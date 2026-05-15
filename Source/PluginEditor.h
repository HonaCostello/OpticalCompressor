#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 2.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Draw the rotating knob body (simulating the rose/gun icons rotating)
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2);

        // Draw a high-visibility rotating indicator line
        juce::Path p;
        auto pointerLength = radius * 0.9f;
        auto pointerThickness = 5.0f;
        p.addRoundedRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength, 2.0f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // Glow effect for the indicator
        juce::Graphics::ScopedSaveState save(g);
        g.addTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        
        juce::ColourGradient cg(juce::Colours::red, 0, -radius, juce::Colours::red.withAlpha(0.0f), 0, 0, false);
        g.setGradientFill(cg);
        g.fillPath(p);

        g.setColour(juce::Colours::red);
        g.fillPath(p);
        
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillEllipse(-1.5f, -radius - 1.5f, 3, 3);
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
