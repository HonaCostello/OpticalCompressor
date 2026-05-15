#include "PluginProcessor.h"
#include "PluginEditor.h"

OpticalCompressorAudioProcessorEditor::OpticalCompressorAudioProcessorEditor(OpticalCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    auto setupKnob = [&](juce::Slider& knob, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, juce::String paramID) {
        knob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knob.setLookAndFeel(&customLookAndFeel);
        addAndMakeVisible(knob);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, knob);
    };

    setupKnob(inputGainSlider, inputGainAttachment, "inputgain");
    setupKnob(thresholdSlider, thresholdAttachment, "threshold");
    setupKnob(ratioSlider, ratioAttachment, "ratio");
    setupKnob(makeupSlider, makeupAttachment, "makeupgain");
    setupKnob(attackSlider, attackAttachment, "attack");
    setupKnob(releaseSlider, releaseAttachment, "release");
    setupKnob(saturationSlider, saturationAttachment, "saturation");
    setupKnob(wetDrySlider, wetDryAttachment, "wetdry");
    setupKnob(outputGainSlider, outputGainAttachment, "outputgain");

    setupKnob(gateThresholdSlider, gateThresholdAttachment, "gatethreshold");
    setupKnob(gateRangeSlider, gateRangeAttachment, "gaterange");
    setupKnob(gateReleaseSlider, gateReleaseAttachment, "gaterelease");
    setupKnob(delayVolSlider, delayVolAttachment, "delayvol");
    setupKnob(fxWetDrySlider, fxWetDryAttachment, "fxwetdry");

    limitButton.setButtonText("");
    limitButton.setAlpha(0.0f);
    addAndMakeVisible(limitButton);
    limitAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "limit", limitButton);

    setResizable(true, true);
    setResizeLimits(600, 450, 2400, 1800);
    getConstrainer()->setFixedAspectRatio(1230.0 / 930.0);

    setSize(1230, 930);
    startTimerHz(30);
}

OpticalCompressorAudioProcessorEditor::~OpticalCompressorAudioProcessorEditor()
{
    stopTimer();
}

void OpticalCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto skin = juce::ImageCache::getFromMemory(BinaryData::skin_clean_jpg, BinaryData::skin_clean_jpgSize);
    if (skin.isValid())
        g.drawImageWithin(skin, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);

    float scaleX = getWidth() / 1230.0f;
    float scaleY = getHeight() / 930.0f;

    // Gain Reduction Meter
    auto meterArea = juce::Rectangle<float>(145 * scaleX, 275 * scaleY, 60 * scaleX, 400 * scaleY);
    float gr = audioProcessor.getGainReduction(); 
    float grNormalized = juce::jlimit(0.0f, 1.0f, std::abs(gr) / 24.0f);
    
    if (grNormalized > 0.01f) {
        g.setColour(juce::Colours::red.withAlpha(0.6f * grNormalized));
        g.fillRect(meterArea.withHeight(meterArea.getHeight() * grNormalized).withY(meterArea.getBottom() - meterArea.getHeight() * grNormalized));
    }

    // EQ Bars
    for (int i = 0; i < 13; ++i) {
        if (eqBands[i]) {
            float xPos = (315 + i * 45) * scaleX;
            g.setColour(juce::Colours::red.withAlpha(0.7f));
            g.fillRect(xPos, 830 * scaleY, 30 * scaleX, 80 * scaleY);
        }
    }

    // Limit Light
    if (audioProcessor.apvts.getRawParameterValue("limit")->load() > 0.5f) {
        g.setColour(juce::Colours::red);
        g.fillEllipse(495 * scaleX, 675 * scaleY, 25 * scaleX, 25 * scaleY);
    }
}

void OpticalCompressorAudioProcessorEditor::resized()
{
    float scaleX = getWidth() / 1230.0f;
    float scaleY = getHeight() / 930.0f;

    auto setSquareBounds = [&](juce::Component& c, float x, float y, float size) {
        c.setBounds((int)(x * scaleX), (int)(y * scaleY), (int)(size * scaleX), (int)(size * scaleX)); // Forced square for perfect circular rotation
    };

    // Main Knobs (Forced Square)
    setSquareBounds(inputGainSlider, 320, 280, 110);
    setSquareBounds(thresholdSlider, 490, 280, 110);
    setSquareBounds(ratioSlider, 660, 280, 110);
    setSquareBounds(makeupSlider, 830, 280, 110);

    setSquareBounds(attackSlider, 280, 490, 100);
    setSquareBounds(releaseSlider, 420, 490, 100);
    setSquareBounds(saturationSlider, 560, 490, 100);
    setSquareBounds(wetDrySlider, 700, 490, 100);
    setSquareBounds(outputGainSlider, 840, 490, 100);

    // Bottom Panel Knobs (Forced Square)
    setSquareBounds(gateThresholdSlider, 65, 830, 70);
    setSquareBounds(gateRangeSlider, 145, 830, 70);
    setSquareBounds(gateReleaseSlider, 225, 830, 70);
    setSquareBounds(delayVolSlider, 715, 840, 70);
    setSquareBounds(fxWetDrySlider, 910, 850, 70);

    limitButton.setBounds((int)(450 * scaleX), (int)(670 * scaleY), (int)(120 * scaleX), (int)(40 * scaleY));
}

void OpticalCompressorAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    float scaleX = getWidth() / 1230.0f;
    float scaleY = getHeight() / 930.0f;

    for (int i = 0; i < 13; ++i) {
        float xPos = (315 + i * 45) * scaleX;
        auto eqBar = juce::Rectangle<float>(xPos, 830 * scaleY, 30 * scaleX, 80 * scaleY);
        if (eqBar.contains(event.position)) {
            eqBands[i] = !eqBands[i];
            repaint();
            return;
        }
    }
}

void OpticalCompressorAudioProcessorEditor::timerCallback()
{
    repaint();
}
