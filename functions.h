#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <exception>
#include "melody.h"

juce::File filePath(std::string path);

juce::MidiFile readMidiFile(const std::string& path);

std::vector<Note> extractNotes(const juce::MidiFile& midiFile);
