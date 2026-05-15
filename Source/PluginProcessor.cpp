#include "PluginProcessor.h"
#include "PluginEditor.h"

OpticalCompressorAudioProcessor::OpticalCompressorAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

OpticalCompressorAudioProcessor::~OpticalCompressorAudioProcessor() {}

void OpticalCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    dsp.prepare(spec);
}

void OpticalCompressorAudioProcessor::releaseResources() {}

void OpticalCompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    dsp.setThreshold(apvts.getRawParameterValue("threshold")->load());
    dsp.setRatio(apvts.getRawParameterValue("ratio")->load());
    dsp.setAttack(apvts.getRawParameterValue("attack")->load());
    dsp.setRelease(apvts.getRawParameterValue("release")->load());
    dsp.setMakeupGain(apvts.getRawParameterValue("makeup")->load());
    dsp.setIsLimit(apvts.getRawParameterValue("limit")->load() > 0.5f);
    dsp.setSaturation(apvts.getRawParameterValue("saturation")->load());
    dsp.setWetDry(apvts.getRawParameterValue("wetdry")->load());
    dsp.setInputGain(apvts.getRawParameterValue("inputgain")->load());
    dsp.setOutputGain(apvts.getRawParameterValue("outputgain")->load());

    dsp.process(buffer);
}

juce::AudioProcessorEditor* OpticalCompressorAudioProcessor::createEditor()
{
    return new OpticalCompressorAudioProcessorEditor(*this);
}

void OpticalCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OpticalCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout OpticalCompressorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("inputgain", "Input Gain", -60.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", -50.0f, 5.0f, -20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ratio", "Ratio", 1.0f, 3.1f, 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.0f, 1000.0f, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("makeup", "Make-up", -100.0f, 100.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("limit", "Limit Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("saturation", "Saturation", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("wetdry", "Wet/Dry", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("outputgain", "Output Gain", -60.0f, 24.0f, 0.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpticalCompressorAudioProcessor();
}
