#pragma once
#include <JuceHeader.h>

class TimeFreezeAudioProcessor : public juce::AudioProcessor
{
public:
    TimeFreezeAudioProcessor();
    ~TimeFreezeAudioProcessor() override = default;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void resetFreeze();
    float getInputPeak() const noexcept { return inputPeak.load(); }
    float getOutputPeak() const noexcept { return outputPeak.load(); }
    double getFreezeSeconds() const noexcept { return freezeSeconds.load(); }
private:
    juce::AudioBuffer<float> freezeBuffer;
    int writePosition = 0;
    double currentSampleRate = 44100.0;
    double readPosition = 0.0;
    bool freezeWasOn = false;
    std::atomic<float> inputPeak { 0.0f }, outputPeak { 0.0f };
    std::atomic<double> freezeSeconds { 0.0 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeFreezeAudioProcessor)
};
