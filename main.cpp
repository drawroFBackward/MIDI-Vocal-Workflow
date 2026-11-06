#pragma once
#include <iostream>
#include "melody.h"
#include "functions.h"
#include <vector>
#include <algorithm>

int wmain(int argc, wchar_t* argv[])
{
    // Step 1: Specify the path to your MIDI file
    //std::string originalMidiFilePath = "..\\MIDI-Vocal-Workflow-Test-Data\\Original_空に免じて.mid";
    //std::string benchmarkMidiFileName = "..\\MIDI-Vocal-Workflow-Test-Data\\Modified_空に免じて.mid";

    // Step 1: Exit on no input files
    
    if (argc == 1)
    {
        std::cout << "Please check input arguments. Usage: <program> <original_midi_file> <benchmark_midi_file>\n";
        return 0;
    }
    if (argc != 3)
    {
        std::cout << "Incorrect number of arguments. Usage: <program> <original_midi_file> <benchmark_midi_file>\n";
        return 1;
    }
    
    

    // Step 2: Read the MIDI file, extract notes, and handle exceptions
    juce::MidiFile originalMidiFile, benchmarkMidiFile;
    std::vector<Note> notes, correctNotes;
    std::string originalMidiFileName = juce::String(argv[1]).toStdString();
    std::string benchmarkMidiFileName = juce::String(argv[2]).toStdString();

    std::cout << "Original MIDI File: " << originalMidiFileName << "\n";
    std::cout << "Benchmark MIDI File: " << benchmarkMidiFileName << "\n";

    try
    {
        originalMidiFile = readMidiFile(originalMidiFileName);
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

    // Step 4: Export the selected notes to a new MIDI file
    auto selectedNotes = indicesToNotes(notes, selectedIndices);
    std::string outputMidiFileName = "Vocal_";
    // outputMidiFileName += originalMidiFileName;
    outputMidiFileName += ".mid";
    try
    {
        juce::MidiFile outputMidiFile = notesToMidiFile(selectedNotes, originalMidiFile);
        exportMidiFile(outputMidiFile, outputMidiFileName);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // reimport test

    try
    {
        juce::MidiFile reimportedMidiFile = readMidiFile(outputMidiFileName);
        auto reimportedNotes = extractNotes(reimportedMidiFile);
        std::cout << "Reimported Notes Count: " << reimportedNotes.size() << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}