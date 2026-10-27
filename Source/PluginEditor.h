#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TimeFreezeAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TimeFreezeAudioProcessorEditor(TimeFreezeAudioProcessor&);
    ~TimeFreezeAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    TimeFreezeAudioProcessor& audioProcessor;
    juce::TextButton freezeButton { "FREEZE" }, bypassButton { "Bypass" }, limiterButton { "Limiter" }, resetButton { "Reset" };
    juce::ComboBox modeBox;
    juce::Label titleLabel, timerLabel, presetLabel;
    juce::Slider input, output, overlap, grain, speed, mix, smear, volume, pitch, reducer;
    juce::Label inputLabel, outputLabel, overlapLabel, grainLabel, speedLabel, mixLabel, smearLabel, volumeLabel, pitchLabel, reducerLabel;
    std::unique_ptr<ButtonAttachment> freezeAttachment, bypassAttachment, limiterAttachment;
    std::unique_ptr<ComboAttachment> modeAttachment;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    double elapsed = 0.0;
    void configureSlider(juce::Slider&, juce::Label&, const juce::String&);
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeFreezeAudioProcessorEditor)
};
