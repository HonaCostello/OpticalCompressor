#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto knobImg = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
        
        if (knobImg.isValid())
        {
            auto centreX = (float)x + (float)width * 0.5f;
            auto centreY = (float)y + (float)height * 0.5f;
            auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            juce::AffineTransform t;
            t = t.rotated(angle, (float)knobImg.getWidth() * 0.5f, (float)knobImg.getHeight() * 0.5f);
            t = t.translated(centreX - (float)knobImg.getWidth() * 0.5f, centreY - (float)knobImg.getHeight() * 0.5f);

            g.drawImageTransformed(knobImg, t);
        }
    }
};

class OpticalCompressorAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    OpticalCompressorAudioProcessorEditor(OpticalCompressorAudioProcessor&);
    ~OpticalCompressorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    CustomLookAndFeel customLookAndFeel;
    OpticalCompressorAudioProcessor& audioProcessor;

    // Sliders
    juce::Slider inputGainSlider, thresholdSlider, ratioSlider, attackSlider, releaseSlider, makeupSlider, saturationSlider, wetDrySlider, outputGainSlider;
    juce::Slider gateThresholdSlider, gateRangeSlider, gateReleaseSlider, delayVolSlider, fxWetDrySlider;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment, thresholdAttachment, ratioAttachment, attackAttachment, releaseAttachment, makeupAttachment, saturationAttachment, wetDryAttachment, outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateThresholdAttachment, gateRangeAttachment, gateReleaseAttachment, delayVolAttachment, fxWetDryAttachment;
    
    juce::ToggleButton limitButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limitAttachment;

    // EQ State
    bool eqBands[13] = {true, true, true, true, true, true, true, true, true, true, true, true, true};

    void setupSlider(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, juce::String paramID);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpticalCompressorAudioProcessorEditor)
};
