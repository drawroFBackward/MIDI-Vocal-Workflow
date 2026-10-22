cmake_minimum_required(VERSION 3.22)

project(MIDIVocalWorkflow VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_console_app(MIDIVocalWorkflow
    PRODUCT_NAME "MIDI Vocal Workflow"
)

target_sources(MIDIVocalWorkflow
    PRIVATE
        main.cpp
        functions.cpp
)

target_link_libraries(MIDIVocalWorkflow
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
)