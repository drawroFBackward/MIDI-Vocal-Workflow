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
            BenchmarkResult result = runBenchmark(originalMidiFilePath, benchmarkMidiFilePath);
            report.addResult(result);
        }
        report.printReport();
        // Song Window Analysis Summary for 千本桜
        juce::String original;
        juce::String benchmark;
        for (const auto& pair : filePairs)
        {
            if (pair.original.contains("千本桜"))
            {
                original = pair.original;
                benchmark = pair.benchmark;
                break;
            }

        }
        std::vector<Note> originalNotes = extractNotes(readMidiFile(original));
        std::vector<Note> benchmarkNotes = extractNotes(readMidiFile(benchmark));
        std::vector<std::size_t> selectedIndices = selectNotes(originalNotes);
        std::vector<songWindow> windows = runSongWindowAnalysis(originalNotes, benchmarkNotes, selectedIndices, 20, getSongLength(readMidiFile(original), readMidiFile(benchmark)));
        SongWindowSummary summary = summarizeSongWindows(windows);
        std::cout << "Vocal windows: " << summary.vocalWindowCount << "\n";
        std::cout << "Non-vocal windows: " << summary.nonVocalWindowCount << "\n";
        std::cout << "FP in vocal windows: " << summary.vocalFP << "\n";
        std::cout << "FP in non-vocal windows: " << summary.nonVocalFP << "\n";
        printSongWindowAnalysis(windows);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}