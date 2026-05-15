#include "PluginProcessor.h"
#include "PluginEditor.h"

OpticalCompressorAudioProcessorEditor::OpticalCompressorAudioProcessorEditor(OpticalCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    auto setupSlider = [&](juce::Slider& slider, juce::Label& label, const juce::String& name, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        addAndMakeVisible(slider);
        
        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    setupSlider(inputGainSlider, inputGainLabel, "Input", inputGainAttachment, "inputgain");
    setupSlider(thresholdSlider, thresholdLabel, "Threshold", thresholdAttachment, "threshold");
    setupSlider(ratioSlider, ratioLabel, "Ratio", ratioAttachment, "ratio");
    setupSlider(attackSlider, attackLabel, "Attack", attackAttachment, "attack");
    setupSlider(releaseSlider, releaseLabel, "Release", releaseAttachment, "release");
    setupSlider(makeupSlider, makeupLabel, "Make-up", makeupAttachment, "makeup");
    setupSlider(saturationSlider, saturationLabel, "Saturation", saturationAttachment, "saturation");
    setupSlider(wetDrySlider, wetDryLabel, "Wet/Dry", wetDryAttachment, "wetdry");
    setupSlider(outputGainSlider, outputGainLabel, "Output", outputGainAttachment, "outputgain");

    limitButton.setButtonText("Limit");
    addAndMakeVisible(limitButton);
    limitAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "limit", limitButton);

    setSize(600, 400);
    startTimerHz(30);
}

OpticalCompressorAudioProcessorEditor::~OpticalCompressorAudioProcessorEditor()
{
    stopTimer();
}

void OpticalCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto skin = juce::ImageCache::getFromMemory(BinaryData::skin_jpg, BinaryData::skin_jpgSize);
    if (skin.isValid())
        g.drawImageWithin(skin, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    else
        g.fillAll(juce::Colours::black);

    // Draw Gain Reduction Meter (aligned with the skin's meter area)
    auto meterArea = juce::Rectangle<int>(105, 125, 45, 450); // Approximate based on image
    
    float gr = audioProcessor.apvts.getRawParameterValue("threshold")->load(); // Simplified for now
    float grNormalized = juce::jmap(gr, -50.0f, 5.0f, 0.0f, 1.0f);
    
    g.setColour(juce::Colours::red.withAlpha(0.8f));
    int grHeight = (int)(meterArea.getHeight() * grNormalized);
    g.fillRect(meterArea.withHeight(grHeight).withY(meterArea.getBottom() - grHeight));
}

void OpticalCompressorAudioProcessorEditor::resized()
{
    // Adjusting sizes to match the 1024x768 aspect ratio of the skin
    float scaleX = getWidth() / 1024.0f;
    float scaleY = getHeight() / 768.0f;

    auto setBoundsScaled = [&](juce::Component& c, int x, int y, int w, int h) {
        c.setBounds((int)(x * scaleX), (int)(y * scaleY), (int)(w * scaleX), (int)(h * scaleY));
    };

    // Hide labels as they are part of the skin
    inputGainLabel.setVisible(false);
    thresholdLabel.setVisible(false);
    ratioLabel.setVisible(false);
    makeupLabel.setVisible(false);
    attackLabel.setVisible(false);
    releaseLabel.setVisible(false);
    saturationLabel.setVisible(false);
    wetDryLabel.setVisible(false);
    outputGainLabel.setVisible(false);

    // Position sliders over the skin's knobs
    setBoundsScaled(inputGainSlider, 280, 220, 120, 120);
    setBoundsScaled(thresholdSlider, 460, 220, 120, 120); // "Peak Reduction" in skin
    setBoundsScaled(ratioSlider, 640, 220, 120, 120);
    setBoundsScaled(makeupSlider, 820, 220, 120, 120);

    setBoundsScaled(attackSlider, 280, 480, 120, 120);
    setBoundsScaled(releaseSlider, 460, 480, 120, 120);
    setBoundsScaled(saturationSlider, 640, 480, 120, 120);
    setBoundsScaled(wetDrySlider, 820, 480, 120, 120);

    setBoundsScaled(outputGainSlider, 820, 620, 120, 120);
    setBoundsScaled(limitButton, 440, 680, 150, 50);
    
    // Make sliders transparent to show the skin's knobs
    auto makeTransparent = [](juce::Slider& s) {
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
        s.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    };

    makeTransparent(inputGainSlider);
    makeTransparent(thresholdSlider);
    makeTransparent(ratioSlider);
    makeTransparent(makeupSlider);
    makeTransparent(attackSlider);
    makeTransparent(releaseSlider);
    makeTransparent(saturationSlider);
    makeTransparent(wetDrySlider);
    makeTransparent(outputGainSlider);
}

void OpticalCompressorAudioProcessorEditor::timerCallback()
{
    repaint();
}
