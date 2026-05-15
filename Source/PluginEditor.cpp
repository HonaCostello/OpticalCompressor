#include "PluginProcessor.h"
#include "PluginEditor.h"

OpticalCompressorAudioProcessorEditor::OpticalCompressorAudioProcessorEditor(OpticalCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    auto setupSlider = [&](juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, juce::String paramID) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel(&customLookAndFeel);
        addAndMakeVisible(slider);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    setupSlider(inputGainSlider, inputGainAttachment, "inputgain");
    setupSlider(thresholdSlider, thresholdAttachment, "threshold");
    setupSlider(ratioSlider, ratioAttachment, "ratio");
    setupSlider(makeupSlider, makeupAttachment, "makeupgain");
    setupSlider(attackSlider, attackAttachment, "attack");
    setupSlider(releaseSlider, releaseAttachment, "release");
    setupSlider(saturationSlider, saturationAttachment, "saturation");
    setupSlider(wetDrySlider, wetDryAttachment, "wetdry");
    setupSlider(outputGainSlider, outputGainAttachment, "outputgain");

    setupSlider(gateThresholdSlider, gateThresholdAttachment, "gatethreshold");
    setupSlider(gateRangeSlider, gateRangeAttachment, "gaterange");
    setupSlider(gateReleaseSlider, gateReleaseAttachment, "gaterelease");
    setupSlider(delayVolSlider, delayVolAttachment, "delayvol");
    setupSlider(fxWetDrySlider, fxWetDryAttachment, "fxwetdry");

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

    // 1. Glowing Gain Reduction Meter
    auto meterArea = juce::Rectangle<float>(145 * scaleX, 275 * scaleY, 60 * scaleX, 400 * scaleY);
    float gr = audioProcessor.getGainReduction(); 
    float grNormalized = juce::jlimit(0.0f, 1.0f, std::abs(gr) / 24.0f);
    
    if (grNormalized > 0.01f) {
        juce::Colour glowColor = juce::Colours::red.withAlpha(0.6f * grNormalized);
        g.setColour(glowColor);
        for (int i = 1; i <= 5; ++i) {
            g.drawRect(meterArea.expanded(i * scaleX), 1.0f);
        }
        g.setColour(juce::Colours::red);
        float grHeight = meterArea.getHeight() * grNormalized;
        g.fillRect(meterArea.withHeight(grHeight).withY(meterArea.getBottom() - grHeight));
    }

    // 2. Interactive EQ Glowing Bars
    for (int i = 0; i < 13; ++i) {
        if (eqBands[i]) {
            float xPos = (315 + i * 45) * scaleX;
            auto eqBar = juce::Rectangle<float>(xPos, 830 * scaleY, 30 * scaleX, 80 * scaleY);
            g.setColour(juce::Colours::red.withAlpha(0.7f));
            g.fillRect(eqBar);
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.drawRect(eqBar, 1.0f);
        }
    }

    // 3. Limit Light
    if (audioProcessor.apvts.getRawParameterValue("limit")->load() > 0.5f) {
        g.setColour(juce::Colours::red);
        g.fillEllipse(495 * scaleX, 675 * scaleY, 25 * scaleX, 25 * scaleY);
        g.setColour(juce::Colours::red.withAlpha(0.4f));
        g.fillEllipse(485 * scaleX, 665 * scaleY, 45 * scaleX, 45 * scaleY);
    }
}

void OpticalCompressorAudioProcessorEditor::resized()
{
    float scaleX = getWidth() / 1230.0f;
    float scaleY = getHeight() / 930.0f;

    auto setBoundsScaled = [&](juce::Component& c, float x, float y, float w, float h) {
        c.setBounds((int)(x * scaleX), (int)(y * scaleY), (int)(w * scaleX), (int)(h * scaleY));
    };

    // Main Knobs
    setBoundsScaled(inputGainSlider, 320, 280, 110, 110);
    setBoundsScaled(thresholdSlider, 490, 280, 110, 110);
    setBoundsScaled(ratioSlider, 660, 280, 110, 110);
    setBoundsScaled(makeupSlider, 830, 280, 110, 110);

    setBoundsScaled(attackSlider, 280, 490, 100, 100);
    setBoundsScaled(releaseSlider, 420, 490, 100, 100);
    setBoundsScaled(saturationSlider, 560, 490, 100, 100);
    setBoundsScaled(wetDrySlider, 700, 490, 100, 100);
    setBoundsScaled(outputGainSlider, 840, 490, 100, 100);

    // Bottom Panel
    setBoundsScaled(gateThresholdSlider, 65, 830, 70, 70);
    setBoundsScaled(gateRangeSlider, 145, 830, 70, 70);
    setBoundsScaled(gateReleaseSlider, 225, 830, 70, 70);
    
    setBoundsScaled(delayVolSlider, 715, 840, 70, 70);
    setBoundsScaled(fxWetDrySlider, 910, 850, 70, 70);

    setBoundsScaled(limitButton, 450, 670, 120, 40);
}

void OpticalCompressorAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    float scaleX = getWidth() / 1230.0f;
    float scaleY = getHeight() / 930.0f;

    // EQ Band Toggling
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
