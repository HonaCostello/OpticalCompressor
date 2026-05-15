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
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Optical Compressor", getLocalBounds().removeFromTop(40), juce::Justification::centred, true);

    // Draw Gain Reduction Meter
    auto meterArea = getLocalBounds().removeFromRight(60).reduced(10, 50);
    g.setColour(juce::Colours::black);
    g.fillRect(meterArea);

    float grNormalized = juce::jmap(audioProcessor.apvts.getRawParameterValue("threshold")->load() - 30.0f, -60.0f, 0.0f, 0.0f, 1.0f); // Placeholder logic
    // In a real scenario, we'd pull the actual GR from the processor
    
    g.setColour(juce::Colours::red);
    int grHeight = (int)(meterArea.getHeight() * (std::abs(audioProcessor.apvts.getRawParameterValue("threshold")->load()) / 60.0f)); // Simplified
    g.fillRect(meterArea.withHeight(grHeight).withY(meterArea.getY()));
}

void OpticalCompressorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(40);

    auto row1 = area.removeFromTop(120);
    auto row2 = area.removeFromTop(120);

    auto getSliderArea = [](juce::Rectangle<int>& row) { return row.removeFromLeft(row.getWidth() / 4); };

    auto s1 = row1.removeFromLeft(110);
    inputGainSlider.setBounds(s1.removeFromTop(90));
    inputGainLabel.setBounds(s1);

    auto s2 = row1.removeFromLeft(110);
    thresholdSlider.setBounds(s2.removeFromTop(90));
    thresholdLabel.setBounds(s2);

    auto s3 = row1.removeFromLeft(110);
    ratioSlider.setBounds(s3.removeFromTop(90));
    ratioLabel.setBounds(s3);

    auto s4 = row1.removeFromLeft(110);
    makeupSlider.setBounds(s4.removeFromTop(90));
    makeupLabel.setBounds(s4);

    auto s5 = row2.removeFromLeft(110);
    attackSlider.setBounds(s5.removeFromTop(90));
    attackLabel.setBounds(s5);

    auto s6 = row2.removeFromLeft(110);
    releaseSlider.setBounds(s6.removeFromTop(90));
    releaseLabel.setBounds(s6);

    auto s7 = row2.removeFromLeft(110);
    saturationSlider.setBounds(s7.removeFromTop(90));
    saturationLabel.setBounds(s7);

    auto s8 = row2.removeFromLeft(110);
    wetDrySlider.setBounds(s8.removeFromTop(90));
    wetDryLabel.setBounds(s8);

    limitButton.setBounds(area.removeFromLeft(100).withHeight(30));
    outputGainSlider.setBounds(area.removeFromRight(110).removeFromTop(90));
    outputGainLabel.setBounds(area.removeFromRight(110));
}

void OpticalCompressorAudioProcessorEditor::timerCallback()
{
    repaint();
}
