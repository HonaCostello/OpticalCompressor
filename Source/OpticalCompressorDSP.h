#pragma once

#include <JuceHeader.h>

class OpticalCompressorDSP
{
public:
    void prepare(double sampleRate, int samplesPerBlock)
    {
        this->sampleRate = sampleRate;
        
        // Prepare Reverb
        juce::Reverb::Parameters reverbParams;
        reverb.setParameters(reverbParams);
        reverb.setSampleRate(sampleRate);

        // Prepare Delay
        delayBuffer.setSize(2, (int)(sampleRate * 2.0)); // 2 seconds max delay
        delayBuffer.clear();
        writePos = 0;

        // Prepare EQ (Simplified 10-band)
        for (int i = 0; i < 10; ++i) {
            filters[i].reset();
        }
    }

    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts)
    {
        auto inputGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputgain")->load());
        auto threshold = apvts.getRawParameterValue("threshold")->load();
        auto ratio = apvts.getRawParameterValue("ratio")->load();
        auto attack = apvts.getRawParameterValue("attack")->load() / 1000.0f;
        auto release = apvts.getRawParameterValue("release")->load() / 1000.0f;
        auto makeupGain = apvts.getRawParameterValue("makeupgain")->load();
        auto saturation = apvts.getRawParameterValue("saturation")->load() / 100.0f;
        auto wetDry = apvts.getRawParameterValue("wetdry")->load() / 100.0f;
        bool isLimit = apvts.getRawParameterValue("limit")->load() > 0.5f;

        // New Parameters
        auto gateThreshold = apvts.getRawParameterValue("gatethreshold")->load();
        auto gateRange = apvts.getRawParameterValue("gaterange")->load();
        auto gateRelease = apvts.getRawParameterValue("gaterelease")->load() / 1000.0f;
        auto delayVol = apvts.getRawParameterValue("delayvol")->load() / 100.0f;
        auto fxWetDry = apvts.getRawParameterValue("fxwetdry")->load() / 100.0f;

        if (isLimit) ratio = 20.0f;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float input = channelData[i] * inputGain;
                
                // 1. GATE
                float absInput = std::abs(input);
                if (absInput < juce::Decibels::decibelsToGain(gateThreshold)) {
                    input *= juce::Decibels::decibelsToGain(-gateRange);
                }

                // 2. COMPRESSOR (Optical Style)
                float env = std::abs(input);
                if (env > envelope) envelope += (env - envelope) * (1.0f - std::exp(-1.0f / (sampleRate * attack)));
                else envelope += (env - envelope) * (1.0f - std::exp(-1.0f / (sampleRate * release)));

                float grDb = 0.0f;
                if (juce::Decibels::gainToDecibels(envelope) > threshold) {
                    grDb = (threshold - juce::Decibels::gainToDecibels(envelope)) * (1.0f - 1.0f / ratio);
                }
                lastGR = grDb;
                float grGain = juce::Decibels::decibelsToGain(grDb);
                float compressed = input * grGain * juce::Decibels::decibelsToGain(makeupGain);

                // 3. SATURATION
                if (saturation > 0.0f) {
                    compressed = std::tanh(compressed * (1.0f + saturation * 2.0f));
                }

                // 4. EQ (Simplified)
                // In a real VST we'd use 10 peaking filters here
                
                // 5. FX (Delay & Reverb)
                float fxOut = compressed;
                // Add Reverb/Delay logic here...

                channelData[i] = compressed * wetDry + input * (1.0f - wetDry);
            }
        }
    }

    float getGainReduction() const { return lastGR; }

private:
    double sampleRate = 44100.0;
    float envelope = 0.0f;
    float lastGR = 0.0f;
    juce::Reverb reverb;
    juce::AudioBuffer<float> delayBuffer;
    int writePos = 0;
    juce::IIRFilter filters[10];
};
