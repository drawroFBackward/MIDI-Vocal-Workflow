#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <exception>
#include <cmath>
#include "melody.h"

struct BenchmarkResults{
    std::vector<std::string> songNames; // Names of the songs being evaluated
    std::vector<int> TP; //True Positives
    std::vector<int> FP; //False Positives
    std::vector<int> FN; //False Negatives
    std::vector<double> precision; // Precision
    std::vector<double> recall; // Recall
    std::vector<double> f1Score; // F1 Score
    BenchmarkResults();
    void evaluate(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices, const std::string& songName = "...");
    void printResults();
};

juce::File filePath(std::string path);

juce::MidiFile readMidiFile(const std::string& path);

std::vector<Note> extractNotes(const juce::MidiFile& midiFile);

std::vector<std::size_t> selectByPitch(const std::vector<Note>& notes, int minPitch);

std::vector<Note> indicesToNotes(const std::vector<Note>& notes, const std::vector<std::size_t>& indices);

int truePositiveNotes(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices);

juce::MidiFile notesToMidiFile(const std::vector<Note>& notes, const juce::MidiFile& reference);

void exportMidiFile(const juce::MidiFile& midiFile, const std::string& path);