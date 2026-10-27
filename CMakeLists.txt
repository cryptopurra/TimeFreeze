cmake_minimum_required(VERSION 3.22)
project(TimeFreeze VERSION 1.0.0 LANGUAGES CXX)

if(NOT DEFINED JUCE_DIR)
  message(FATAL_ERROR "Set JUCE_DIR to the JUCE source folder.")
endif()
add_subdirectory("${JUCE_DIR}" JUCE)

juce_add_plugin(TimeFreeze
  COMPANY_NAME "TimeFreeze Audio"
  BUNDLE_ID "audio.timefreeze.plugin"
  PLUGIN_MANUFACTURER_CODE Tfze
  PLUGIN_CODE TFrz
  FORMATS VST3 Standalone
  PRODUCT_NAME "TimeFreeze"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(TimeFreeze PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h
  Source/PluginEditor.cpp
  Source/PluginEditor.h
)
target_compile_features(TimeFreeze PUBLIC cxx_std_17)
target_compile_definitions(TimeFreeze PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(TimeFreeze PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
