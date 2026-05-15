#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class OpticalCompressorAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    OpticalCompressorAudioProcessorEditor(OpticalCompressorAudioProcessor&);
    ~OpticalCompressorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    OpticalCompressorAudioProcessor& audioProcessor;

    juce::Slider inputGainSlider, thresholdSlider, ratioSlider, attackSlider, releaseSlider, makeupSlider, saturationSlider, wetDrySlider, outputGainSlider;
    juce::ToggleButton limitButton;
    juce::Label inputGainLabel, thresholdLabel, ratioLabel, attackLabel, releaseLabel, makeupLabel, saturationLabel, wetDryLabel, outputGainLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment, thresholdAttachment, ratioAttachment, attackAttachment, releaseAttachment, makeupAttachment, saturationAttachment, wetDryAttachment, outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limitAttachment;

    float grValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpticalCompressorAudioProcessorEditor)
};
