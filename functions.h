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

std::vector<std::size_t> selectByPitch(const std::vector<Note>& notes, int minPitch);

int truePositiveNotes(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices);

int falseNegatives(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices);