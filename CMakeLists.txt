cmake_minimum_required(VERSION 3.15)

project(OpticalCompressor VERSION 1.0.0)

# Add JUCE
add_subdirectory(JUCE)

juce_add_plugin(OpticalCompressor
    COMPANY_NAME "ManusAI"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_STEP_FINISHED_ANYWHERE FALSE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE OptC
    FORMATS VST3 AU
    PRODUCT_NAME "Optical Compressor")

target_sources(OpticalCompressor
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/OpticalCompressorDSP.h)

target_compile_definitions(OpticalCompressor
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0)

target_link_libraries(OpticalCompressor
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_audio_basics
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_data_structures
        juce::juce_events
        juce::juce_core
        juce::juce_dsp)

juce_generate_juce_header(OpticalCompressor)
