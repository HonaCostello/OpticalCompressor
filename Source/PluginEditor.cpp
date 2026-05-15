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

    limitButton.setButtonText("");
    limitButton.setAlpha(0.0f); // Make it invisible but clickable
    addAndMakeVisible(limitButton);
    limitAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "limit", limitButton);

    // Enable resizability
    setResizable(true, true);
    setResizeLimits(400, 300, 2048, 1536);
    getConstrainer()->setFixedAspectRatio(1024.0 / 768.0);

    // Apply custom look and feel to all sliders
    auto applyLAF = [&](juce::Slider& s) {
        s.setLookAndFeel(&customLookAndFeel);
    };
    applyLAF(inputGainSlider);
    applyLAF(thresholdSlider);
    applyLAF(ratioSlider);
    applyLAF(makeupSlider);
    applyLAF(attackSlider);
    applyLAF(releaseSlider);
    applyLAF(saturationSlider);
    applyLAF(wetDrySlider);
    applyLAF(outputGainSlider);

    setSize(1024, 768); // Match skin aspect ratio
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
    auto meterArea = juce::Rectangle<int>(145, 305, 55, 465); // Refined based on epic skin
    
    // Get actual GR from the processor
    float gr = audioProcessor.getGainReduction(); 
    float grNormalized = juce::jlimit(0.0f, 1.0f, std::abs(gr) / 24.0f); // Scale 0-24dB
    
    g.setColour(juce::Colours::red.withAlpha(0.8f));
    int grHeight = (int)(meterArea.getHeight() * grNormalized);
    g.fillRect(meterArea.withHeight(grHeight).withY(meterArea.getY())); // GR meter usually goes down from top

    // Draw Limit Light
    if (audioProcessor.apvts.getRawParameterValue("limit")->load() > 0.5f)
    {
        g.setColour(juce::Colours::red.withAlpha(0.9f));
        g.fillEllipse(495, 675, 25, 25); // Positioned over the switch light
    }
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

    // Position sliders over the skin's knobs (Epic Skin coordinates - recalibrated)
    setBoundsScaled(inputGainSlider, 315, 335, 105, 105);
    setBoundsScaled(thresholdSlider, 490, 335, 105, 105); // "Peak Reduction"
    setBoundsScaled(ratioSlider, 660, 335, 105, 105);
    setBoundsScaled(makeupSlider, 830, 335, 105, 105);

    setBoundsScaled(attackSlider, 285, 645, 95, 95);
    setBoundsScaled(releaseSlider, 425, 645, 95, 95);
    setBoundsScaled(saturationSlider, 565, 645, 95, 95);
    setBoundsScaled(wetDrySlider, 705, 645, 95, 95);
    setBoundsScaled(outputGainSlider, 845, 645, 95, 95);

    setBoundsScaled(limitButton, 485, 835, 90, 45); // Positioned over the switch
    
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
