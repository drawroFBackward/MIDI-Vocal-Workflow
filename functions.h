#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <exception>
#include <cmath>
#include "melody.h"

// MIDI file access and conversion
// Timestamps are in MIDI ticks; retain source tempo/key/time-signature metadata on export.

juce::File filePath(const juce::String& path);

juce::MidiFile readMidiFile(const juce::String& path);

// Converts MIDI note-on/off messages into Note objects; unmatched note-ons stay incomplete.
std::vector<Note> extractNotes(const juce::MidiFile& midiFile);

juce::MidiFile notesToMidiFile(const std::vector<Note>& notes, const juce::MidiFile& reference);

void exportMidiFile(const juce::MidiFile& midiFile, const juce::String& path);

// Vocal-note selection
// Indices refer to originalNotes; the current baseline uses pitch >= 60.

std::vector<std::size_t> selectByPitch(const std::vector<Note>& notes, int minPitch);

std::vector<std::size_t> selectNotes(const std::vector<Note>& notes);

// Materialize selected note indices as copies, ready for MIDI export.
std::vector<Note> indicesToNotes(const std::vector<Note>& notes, const std::vector<std::size_t>& indices);

// Matching and per-song evaluation
// TP/FP indices refer to original notes; FN indices refer to benchmark notes.
// Note equality is exact pitch + start tick; duplicate matches are one-to-one.

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

bool noteMatches(const Note& note, const Note& benchmarkNote);

SongResult runEvaluation(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices);

// Compatibility count helper; runEvaluation is the source of note-level labels.
int truePositiveNotes(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices);

// Per-song precision, recall and F1; undefined denominators use the existing zero convention.
BenchmarkResult evaluate(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices, const juce::String& songName = "");

// Dataset discovery and benchmark reporting
// Files are paired by their Original_/Modified_ filename prefixes.
// runBenchmark checks matching MIDI time formats before comparing tick positions.

struct FilePair{
    juce::String original;
    juce::String benchmark;
};

// Micro averages weight notes; macro averages weight songs equally.
struct BenchmarkReport{
    std::vector<BenchmarkResult> results;
    void addResult(const BenchmarkResult& result);
    void clearResults();
    BenchmarkResult getMicroAverageResult() const;
    BenchmarkResult getMacroAverageResult() const;
    void printReport() const;
};

std::vector<juce::String> findOriginalMidiFiles(const juce::String& directoryPath);

// Warns about unmatched originals; throws if no complete pairs are found.
std::vector<FilePair> pairFiles(const juce::String& directoryPath);

BenchmarkResult runBenchmark(const juce::String& originalMidiFilePath, const juce::String& benchmarkMidiFilePath);

// Time-window error diagnostics (evaluation only)
// Windows are classified by REFERENCE NOTE ONSETS (TP + FN), not sustained audio activity.
// The final window includes an onset exactly at the song endpoint.

struct songWindow{
    double startTick;
    double endTick;
    int TP;
    int FP;
    int FN;
    songWindow(double start, double end) : startTick(start), endTick(end), TP(0), FP(0), FN(0) {}
    bool isVocal() const { return TP + FN > 0; }
};

// Grouped FP totals for reference-onset versus no-reference-onset windows.
struct SongWindowSummary
{
    std::size_t vocalWindowCount = 0;
    std::size_t nonVocalWindowCount = 0;
    int vocalFP = 0;
    int nonVocalFP = 0;
};

// Window span uses the later MIDI endpoint, including reference notes after the original.
double getSongLength(const juce::MidiFile& midiFile, const juce::MidiFile& benchmarkMidiFile);

// Conservation check: every classified TP/FP/FN must appear in exactly one window.
bool windowCountsMatch(const std::vector<songWindow>& windows, const SongResult& classification);

// Fixed number of equally sized tick windows. Caller must validate matching PPQ.
// Pure note-level computation: no file reads, no change to the selector.
std::vector<songWindow> runSongWindowAnalysis(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<size_t>& selectedIndices, int numberOfWindows, double songLength);

SongWindowSummary summarizeSongWindows(const std::vector<songWindow>& windows);

// Percentage returns 0 for zero FPs; printed reports use N/A for that case.
double percentageFPinVocalWindows(const std::vector<songWindow>& windows);

double averageFPperWindow(const std::vector<songWindow>& windows, bool Vocal = true);

void printSongWindowAnalysis(const std::vector<songWindow>& windows);

// Single-file vocal-note export workflow

// Writes Vocal_<original filename> in the current working directory.
void runAndExport(const juce::String& originalMidiFilePath);
