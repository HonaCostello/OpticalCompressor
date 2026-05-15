#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Draw a glowing red indicator line with a gold border
        juce::Path p;
        auto pointerLength = radius * 0.9f;
        auto pointerThickness = 4.0f;
        p.addRoundedRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength, 2.0f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(juce::Colours::gold.withAlpha(0.5f));
        g.strokePath(p, juce::PathStrokeType(1.0f));
        
        g.setColour(juce::Colours::red);
        g.fillPath(p);
        
        // Add a bright glow at the tip
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillEllipse(centreX + std::sin(angle) * radius * 0.95f - 1.5f, centreY - std::cos(angle) * radius * 0.95f - 1.5f, 3, 3);
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

private:
    CustomLookAndFeel customLookAndFeel;
    OpticalCompressorAudioProcessor& audioProcessor;

    juce::Slider inputGainSlider, thresholdSlider, ratioSlider, attackSlider, releaseSlider, makeupSlider, saturationSlider, wetDrySlider, outputGainSlider;
    juce::ToggleButton limitButton;
    juce::Label inputGainLabel, thresholdLabel, ratioLabel, attackLabel, releaseLabel, makeupLabel, saturationLabel, wetDryLabel, outputGainLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment, thresholdAttachment, ratioAttachment, attackAttachment, releaseAttachment, makeupAttachment, saturationAttachment, wetDryAttachment, outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limitAttachment;

    float grValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpticalCompressorAudioProcessorEditor)
};
