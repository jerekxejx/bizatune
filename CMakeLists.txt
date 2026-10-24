cmake_minimum_required(VERSION 3.22)
project(Bizatune VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Bizatune
  COMPANY_NAME "Bizarra"
  PLUGIN_MANUFACTURER_CODE Bzrr
  PLUGIN_CODE Bztn
  FORMATS VST3 Standalone
  PRODUCT_NAME "Bizatune"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  VST3_CATEGORIES Fx
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(Bizatune)

target_sources(Bizatune PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)

target_compile_definitions(Bizatune PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_options(Bizatune PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(Bizatune
  PRIVATE juce::juce_audio_utils
  PUBLIC juce::juce_recommended_config_flags
         juce::juce_recommended_lto_flags
         juce::juce_recommended_warning_flags)
