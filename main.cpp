#pragma once
#include <iostream>
#include "melody.h"
#include "functions.h"
#include <vector>
#include <algorithm>

int wmain(int argc, wchar_t* argv[])
{
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
    
    if (std::wstring(argv[1]) == L"--benchmark-dir")
    {
        //
    }

    // Step 2: Read the MIDI file, extract notes, and handle exceptions
    juce::MidiFile originalMidiFile, benchmarkMidiFile;
    std::vector<Note> notes, correctNotes;
    std::string originalMidiFilePath = juce::String(argv[1]).toStdString();
    std::string benchmarkMidiFilePath = juce::String(argv[2]).toStdString();

    std::cout << "Original MIDI File: " << originalMidiFilePath << "\n";
    std::cout << "Benchmark MIDI File: " << benchmarkMidiFilePath << "\n";

    BenchmarkResult result;

    try
    {
        result = runBenchmark(originalMidiFilePath, benchmarkMidiFilePath);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
    std::cout << "Benchmark Result for " << result.songName << ":\n";
    std::cout << "True Positives: " << result.TP << "\n";
    std::cout << "False Positives: " << result.FP << "\n";
    std::cout << "False Negatives: " << result.FN << "\n";
    std::cout << "Precision: " << result.precision << "\n";
    std::cout << "Recall: " << result.recall << "\n";
    std::cout << "F1 Score: " << result.f1 << "\n";

    // Step 4: Export the selected notes to a new MIDI file
    try
    {
        runAndExport(originalMidiFilePath);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    /*
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
    */   

    return 0;
}