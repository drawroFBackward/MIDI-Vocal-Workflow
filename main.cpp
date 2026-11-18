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
        std::cout << "Please check input arguments.\nUsage: <program> <original_midi_file>\nUsage for training mode: <program> --dir <directory_path>\n";
        return 0;
    }
    if (argc == 2)
    {
        // Export mode
        try
        {
            runAndExport(juce::String(argv[1]));
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return 1;
        }
        return 0;
    }
    if (argc != 3 || std::wstring(argv[1]) != L"--dir")
    {
        std::cout << "Incorrect arguments. \nUsage: <program> <original_midi_file>\nUsage for directory mode: <program> --dir <directory_path>\n";
        return 1;
    }
    
    // benchmark mode
    std::vector<FilePair> filePairs;
    try
    {
        filePairs = pairFiles(juce::String(argv[2]));
        BenchmarkReport report;
        for (const auto& pair : filePairs)
        {
            juce::String originalMidiFilePath = pair.original;
            juce::String benchmarkMidiFilePath = pair.benchmark;
            // BenchmarkResult result = runBenchmark(originalMidiFilePath, benchmarkMidiFilePath);
            juce::MidiFile originalMidiFile = readMidiFile(originalMidiFilePath);
            juce::MidiFile benchmarkMidiFile = readMidiFile(benchmarkMidiFilePath);
            if (originalMidiFile.getTimeFormat() != benchmarkMidiFile.getTimeFormat())
                throw std::runtime_error(("MIDI files: " + originalMidiFilePath + " and " + benchmarkMidiFilePath + " have different PPQ (time format)").toStdString());
            std::vector<Note> originalNotes = extractNotes(originalMidiFile);
            std::vector<Note> benchmarkNotes = extractNotes(benchmarkMidiFile);
            std::vector<std::size_t> selectedIndices = selectNotes(originalNotes);
            BenchmarkResult result = evaluate(originalNotes, benchmarkNotes, selectedIndices, originalMidiFilePath);
            report.addResult(result);
            std::vector<songWindow> windows = runSongWindowAnalysis(originalNotes, benchmarkNotes, selectedIndices, 20, getSongLength(originalMidiFile, benchmarkMidiFile));
            SongWindowSummary summary = summarizeSongWindows(windows);
            std::cout << "Song: " << originalMidiFilePath << "\n";
            std::cout << "Vocal windows: " << summary.vocalWindowCount << "\n";
            std::cout << "Non-vocal windows: " << summary.nonVocalWindowCount << "\n";
            std::cout << "FP in vocal windows: " << summary.vocalFP << "\n";
            std::cout << "FP in non-vocal windows: " << summary.nonVocalFP << "\n";
            printSongWindowAnalysis(windows);
        }
        report.printReport();
        
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}