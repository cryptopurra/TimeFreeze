#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    constexpr int maxFreezeSeconds = 8;
    float getParam (juce::AudioProcessorValueTreeState& state, const char* id)
    {
        if (auto* p = state.getRawParameterValue (id)) return p->load();
        return 0.0f;
    }
}

TimeFreezeAudioProcessor::TimeFreezeAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout TimeFreezeAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APA = juce::AudioParameterBool;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add (std::make_unique<APA> ("freeze", "Freeze", false));
    p.add (std::make_unique<APA> ("bypass", "Bypass", false));
    p.add (std::make_unique<APA> ("limiter", "Limiter", true));
    p.add (std::make_unique<APF> ("input", "Input Gain", juce::NormalisableRange<float>(-24.f, 24.f, .01f), 0.f));
    p.add (std::make_unique<APF> ("output", "Output Gain", juce::NormalisableRange<float>(-24.f, 24.f, .01f), 0.f));
    p.add (std::make_unique<APF> ("overlap", "Overlap", juce::NormalisableRange<float>(0.f, 100.f, .1f), 50.f));
    p.add (std::make_unique<APF> ("grain", "Grain Size", juce::NormalisableRange<float>(64.f, 8192.f, 1.f), 2048.f));
    p.add (std::make_unique<APF> ("speed", "Speed", juce::NormalisableRange<float>(25.f, 200.f, .1f), 100.f));
    p.add (std::make_unique<APF> ("mix", "Spectral Mix", juce::NormalisableRange<float>(0.f, 100.f, .1f), 100.f));
    p.add (std::make_unique<APF> ("smear", "Smear", juce::NormalisableRange<float>(0.f, 100.f, .1f), 0.f));
    p.add (std::make_unique<APF> ("volume", "Volume", juce::NormalisableRange<float>(-24.f, 12.f, .01f), 0.f));
    p.add (std::make_unique<APF> ("pitch", "Pitch", juce::NormalisableRange<float>(-24.f, 24.f, .01f), 0.f));
    p.add (std::make_unique<APF> ("reducer", "Reducer", juce::NormalisableRange<float>(1.f, 100.f, .1f), 100.f));
    p.add (std::make_unique<juce::AudioParameterChoice> ("mode", "Mode", juce::StringArray { "Normal", "2 Bars", "Loop", "Ping Pong" }, 0));
    return p;
}

void TimeFreezeAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    freezeBuffer.setSize (2, juce::jmax (2, (int) std::round (sampleRate * maxFreezeSeconds)));
    freezeBuffer.clear();
    writePosition = 0;
    readPosition = 0.0;
    freezeWasOn = false;
    freezeSeconds.store (0.0);
}

void TimeFreezeAudioProcessor::releaseResources() {}

bool TimeFreezeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void TimeFreezeAudioProcessor::resetFreeze()
{
    freezeBuffer.clear();
    writePosition = 0;
    readPosition = 0.0;
    freezeSeconds.store (0.0);
}

void TimeFreezeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels(), n = buffer.getNumSamples();
    if (channels == 0 || freezeBuffer.getNumSamples() < 2) return;
    const bool bypass = getParam(parameters, "bypass") > 0.5f;
    const bool freeze = getParam(parameters, "freeze") > 0.5f;
    const bool limiter = getParam(parameters, "limiter") > 0.5f;
    const float inGain = juce::Decibels::decibelsToGain(getParam(parameters, "input"));
    const float outGain = juce::Decibels::decibelsToGain(getParam(parameters, "output") + getParam(parameters, "volume"));
    const float wet = getParam(parameters, "mix") / 100.0f;
    const float rate = juce::jlimit(0.1f, 4.0f, (getParam(parameters, "speed") / 100.0f) * std::pow(2.0f, getParam(parameters, "pitch") / 12.0f));
    const float smear = getParam(parameters, "smear") / 100.0f;
    const float reducer = juce::jmax(1.0f, getParam(parameters, "reducer"));
    const int grain = juce::jlimit(64, freezeBuffer.getNumSamples(), (int)getParam(parameters, "grain"));
    float inPeak = 0.0f, outPeak = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float originalL = buffer.getSample(0, i);
        const float originalR = channels > 1 ? buffer.getSample(1, i) : originalL;
        const float inL = originalL * inGain, inR = originalR * inGain;
        inPeak = juce::jmax(inPeak, juce::jmax(std::abs(inL), std::abs(inR)));
        if (!freeze)
        {
            freezeBuffer.setSample(0, writePosition, inL);
            freezeBuffer.setSample(1, writePosition, inR);
            writePosition = (writePosition + 1) % freezeBuffer.getNumSamples();
            freezeSeconds.store(juce::jmin(8.0, (double)freezeBuffer.getNumSamples() / currentSampleRate));
            readPosition = (double)writePosition;
        }
        else if (!freezeWasOn)
            readPosition = (double)writePosition;
        freezeWasOn = freeze;

        float wetL = inL, wetR = inR;
        if (freeze)
        {
            const int idx = ((int)readPosition % freezeBuffer.getNumSamples() + freezeBuffer.getNumSamples()) % freezeBuffer.getNumSamples();
            const int idx2 = (idx + 1) % freezeBuffer.getNumSamples();
            const float frac = (float)(readPosition - std::floor(readPosition));
            wetL = freezeBuffer.getSample(0, idx) + frac * (freezeBuffer.getSample(0, idx2) - freezeBuffer.getSample(0, idx));
            wetR = freezeBuffer.getSample(1, idx) + frac * (freezeBuffer.getSample(1, idx2) - freezeBuffer.getSample(1, idx));
            readPosition += rate;
            const double loopStart = (double)writePosition;
            if (readPosition >= loopStart + grain || readPosition >= freezeBuffer.getNumSamples())
                readPosition = std::fmod(readPosition, (double)freezeBuffer.getNumSamples());
            const int delay = juce::jmax(1, (int)(smear * 0.04 * currentSampleRate));
            const int di = (idx - delay + freezeBuffer.getNumSamples()) % freezeBuffer.getNumSamples();
            wetL = wetL * (1.0f - smear * 0.35f) + freezeBuffer.getSample(0, di) * smear * 0.35f;
            wetR = wetR * (1.0f - smear * 0.35f) + freezeBuffer.getSample(1, di) * smear * 0.35f;
        }
        if (reducer < 99.9f)
        {
            const float levels = juce::jmax(2.0f, reducer * 2.56f);
            wetL = std::round(wetL * levels) / levels;
            wetR = std::round(wetR * levels) / levels;
        }
        float outL = (inL * (1.0f - wet) + wetL * wet) * outGain;
        float outR = (inR * (1.0f - wet) + wetR * wet) * outGain;
        if (bypass) { outL = originalL; outR = originalR; }
        else if (limiter) { outL = std::tanh(outL); outR = std::tanh(outR); }
        outPeak = juce::jmax(outPeak, juce::jmax(std::abs(outL), std::abs(outR)));
        buffer.setSample(0, i, outL);
        if (channels > 1) buffer.setSample(1, i, outR);
    }
    inputPeak.store(inPeak);
    outputPeak.store(outPeak);
}

juce::AudioProcessorEditor* TimeFreezeAudioProcessor::createEditor() { return new TimeFreezeAudioProcessorEditor(*this); }

void TimeFreezeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TimeFreezeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TimeFreezeAudioProcessor(); }
