#pragma once
#include <iostream>
#include "melody.h"
#include "functions.h"
#include <vector>
#include <algorithm>

int main()
{
    // Step 1: Specify the path to your MIDI file
    std::string originalMidiFilePath = "..\\MIDI-Vocal-Workflow-Test-Data\\Original_空に免じて.mid";
    std::string benchmarkMidiFileName = "..\\MIDI-Vocal-Workflow-Test-Data\\modified.mid";

    // Step 2: Read the MIDI file, extract notes, and handle exceptions
    juce::MidiFile originalMidiFile, benchmarkMidiFile;
    std::vector<Note> notes, correctNotes;
    try
    {
        originalMidiFile = readMidiFile(originalMidiFilePath);
        benchmarkMidiFile = readMidiFile(benchmarkMidiFileName);
        notes = extractNotes(originalMidiFile);
        correctNotes = extractNotes(benchmarkMidiFile);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    // Step 3: Run selection function and compare results
    auto selectedIndices = selectByPitch(notes, 60);

    int tp = truePositiveNotes(notes, correctNotes, selectedIndices);
    int fp = static_cast<int>(selectedIndices.size()) - tp;
    int fn = static_cast<int>(correctNotes.size()) - tp;

    std::cout << "True Positives: " << tp << "\n";
    std::cout << "False Positives: " << fp << "\n";
    std::cout << "False Negatives: " << fn << "\n";

    double precision = tp + fp > 0 ? static_cast<double>(tp) / (tp + fp) : 0.0;
    double recall = tp + fn > 0 ? static_cast<double>(tp) / (tp + fn) : 0.0;
    double f1 = precision + recall > 0 ? 2 * (precision * recall) / (precision + recall) : 0.0;

    std::cout << "Precision: " << precision << "\n";
    std::cout << "Recall: " << recall << "\n";
    std::cout << "F1 Score: " << f1 << "\n";


    return 0;
}