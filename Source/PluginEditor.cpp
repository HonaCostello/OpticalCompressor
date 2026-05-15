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

    float scaleX = getWidth() / 1230.0f; // Original image width
    float scaleY = getHeight() / 930.0f; // Original image height

    // Draw Gain Reduction Meter
    auto meterArea = juce::Rectangle<float>(175 * scaleX, 305 * scaleY, 65 * scaleX, 465 * scaleY);
    float gr = audioProcessor.getGainReduction(); 
    float grNormalized = juce::jlimit(0.0f, 1.0f, std::abs(gr) / 24.0f);
    
    g.setColour(juce::Colours::red.withAlpha(0.8f));
    float grHeight = meterArea.getHeight() * grNormalized;
    g.fillRect(meterArea.withHeight(grHeight).withY(meterArea.getY()));

    // Draw Limit Light
    if (audioProcessor.apvts.getRawParameterValue("limit")->load() > 0.5f)
    {
        g.setColour(juce::Colours::red.withAlpha(0.9f));
        g.fillEllipse(595 * scaleX, 855 * scaleY, 30 * scaleX, 30 * scaleY);
    }
}

void OpticalCompressorAudioProcessorEditor::resized()
{
    float scaleX = getWidth() / 1230.0f;
    float scaleY = getHeight() / 930.0f;

    auto setBoundsScaled = [&](juce::Component& c, float x, float y, float w, float h) {
        c.setBounds((int)(x * scaleX), (int)(y * scaleY), (int)(w * scaleX), (int)(h * scaleY));
    };

    // Hide labels
    inputGainLabel.setVisible(false);
    thresholdLabel.setVisible(false);
    ratioLabel.setVisible(false);
    makeupLabel.setVisible(false);
    attackLabel.setVisible(false);
    releaseLabel.setVisible(false);
    saturationLabel.setVisible(false);
    wetDryLabel.setVisible(false);
    outputGainLabel.setVisible(false);

    // Precise coordinates for Epic Skin
    setBoundsScaled(inputGainSlider, 380, 330, 130, 130);
    setBoundsScaled(thresholdSlider, 590, 330, 130, 130);
    setBoundsScaled(ratioSlider, 800, 330, 130, 130);
    setBoundsScaled(makeupSlider, 1010, 330, 130, 130);

    setBoundsScaled(attackSlider, 345, 645, 115, 115);
    setBoundsScaled(releaseSlider, 515, 645, 115, 115);
    setBoundsScaled(saturationSlider, 685, 645, 115, 115);
    setBoundsScaled(wetDrySlider, 855, 645, 115, 115);
    setBoundsScaled(outputGainSlider, 1025, 645, 115, 115);

    setBoundsScaled(limitButton, 580, 840, 120, 60);
    
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
