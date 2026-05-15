#pragma once

#include <JuceHeader.h>

class OpticalCompressorDSP
{
public:
    OpticalCompressorDSP()
    {
        // Initialize parameters
        threshold = -20.0f;
        ratio = 2.0f;
        attack = 5.0f;
        release = 5.0f;
        makeupGain = 0.0f;
        isLimit = false;
        saturation = 0.0f;
        wetDry = 1.0f;
        inputGain = 0.0f;
        outputGain = 0.0f;

        envelope = 0.0f;
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        
        juce::dsp::ProcessSpec monoSpec = spec;
        monoSpec.numChannels = 1;
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        auto numChannels = buffer.getNumChannels();
        auto numSamples = buffer.getNumSamples();

        float currentThreshold = threshold;
        float currentRatio = isLimit ? 20.0f : ratio;
        float attCoeff = 1.0f - std::exp(-1.0f / (sampleRate * (attack / 1000.0f)));
        float relCoeff = 1.0f - std::exp(-1.0f / (sampleRate * (release / 1000.0f)));

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float inputL = buffer.getSample(0, sample) * juce::Decibels::decibelsToGain(inputGain);
            float inputR = numChannels > 1 ? buffer.getSample(1, sample) * juce::Decibels::decibelsToGain(inputGain) : inputL;

            // Sidechain signal (mono sum)
            float sidechain = (std::abs(inputL) + std::abs(inputR)) * 0.5f;
            float sidechainDb = juce::Decibels::gainToDecibels(sidechain, -120.0f);

            // Optical envelope follower (non-linear)
            if (sidechainDb > envelope)
                envelope += (sidechainDb - envelope) * attCoeff;
            else
                envelope += (sidechainDb - envelope) * relCoeff;

            // Gain reduction calculation
            float over = std::max(0.0f, envelope - currentThreshold);
            float grDb = over * (1.0f / currentRatio - 1.0f);
            float grGain = juce::Decibels::decibelsToGain(grDb);

            lastGR = grDb; // For metering

            // Apply compression and makeup
            float compressedL = inputL * grGain * juce::Decibels::decibelsToGain(makeupGain);
            float compressedR = inputR * grGain * juce::Decibels::decibelsToGain(makeupGain);

            // Limit mode logic: force high ratio and faster response if needed
            if (isLimit) {
                // Additional limiting logic could go here
            }

            // Wet/Dry mix
            float mixedL = compressedL * wetDry + inputL * (1.0f - wetDry);
            float mixedR = compressedR * wetDry + inputR * (1.0f - wetDry);

            // Saturation (Soft clipping)
            mixedL = applySaturation(mixedL, saturation);
            mixedR = applySaturation(mixedR, saturation);

            // Output gain
            buffer.setSample(0, sample, mixedL * juce::Decibels::decibelsToGain(outputGain));
            if (numChannels > 1)
                buffer.setSample(1, sample, mixedR * juce::Decibels::decibelsToGain(outputGain));
        }
    }

    float getGainReduction() const { return lastGR; }

    // Parameter setters
    void setThreshold(float val) { threshold = val; }
    void setRatio(float val) { ratio = val; }
    void setAttack(float val) { attack = val; }
    void setRelease(float val) { release = val; }
    void setMakeupGain(float val) { makeupGain = val; }
    void setIsLimit(bool val) { isLimit = val; }
    void setSaturation(float val) { saturation = val; }
    void setWetDry(float val) { wetDry = val; }
    void setInputGain(float val) { inputGain = val; }
    void setOutputGain(float val) { outputGain = val; }

private:
    float applySaturation(float x, float s)
    {
        if (s <= 0.0f) return x;
        return (1.0f - s) * x + s * std::tanh(x * 1.5f);
    }

    double sampleRate = 44100.0;
    float threshold, ratio, attack, release, makeupGain, saturation, wetDry, inputGain, outputGain;
    bool isLimit;
    float envelope;
    float lastGR = 0.0f;
};
