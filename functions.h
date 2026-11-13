#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <exception>
#include <cmath>
#include "melody.h"

struct FilePair{
    juce::String original;
    juce::String benchmark;
};

struct SongResult{
    std::vector<size_t> truePositives;
    std::vector<size_t> falsePositives;
    std::vector<size_t> falseNegatives;
};

struct BenchmarkResult{
    juce::String songName; // Song name
    int TP; // True Positives
    int FP; // False Positives
    int FN; // False Negatives
    double precision; // Precision
    double recall; // Recall
    double f1; // F1 Score
    BenchmarkResult(): songName(""), TP(0), FP(0), FN(0), precision(0), recall(0), f1(0) {}
};

BenchmarkResult evaluate(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices, const juce::String& songName = "");

BenchmarkResult runBenchmark(const juce::String& originalMidiFilePath, const juce::String& benchmarkMidiFilePath);

struct BenchmarkReport{
    std::vector<BenchmarkResult> results;
    void addResult(const BenchmarkResult& result);
    void clearResults();
    BenchmarkResult getMicroAverageResult() const;
    BenchmarkResult getMacroAverageResult() const;
    void printReport() const;
};

juce::File filePath(const juce::String& path);

juce::MidiFile readMidiFile(const juce::String& path);

std::vector<Note> extractNotes(const juce::MidiFile& midiFile);

//selection functions

std::vector<std::size_t> selectByPitch(const std::vector<Note>& notes, int minPitch);

// run all selection functions and evaluation

std::vector<std::size_t> selectNotes(const std::vector<Note>& notes);

std::vector<Note> indicesToNotes(const std::vector<Note>& notes, const std::vector<std::size_t>& indices);

bool noteMatches(const Note& note, const Note& benchmarkNote);

int truePositiveNotes(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices);

SongResult runEvaluation(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices);

juce::MidiFile notesToMidiFile(const std::vector<Note>& notes, const juce::MidiFile& reference);

void exportMidiFile(const juce::MidiFile& midiFile, const juce::String& path);

void runAndExport(const juce::String& originalMidiFilePath);

std::vector<juce::String> findOriginalMidiFiles(const juce::String& directoryPath);

std::vector<FilePair> pairFiles(const juce::String& directoryPath);