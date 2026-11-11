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
        // Optionally, you can print the report here
        for (const auto& result : report.results)
        {
            std::cout << "Benchmark Result for " << result.songName << ":\n";
            std::cout << "True Positives: " << result.TP << "\n";
            std::cout << "False Positives: " << result.FP << "\n";
            std::cout << "False Negatives: " << result.FN << "\n";
            std::cout << "Precision: " << result.precision << "\n";
            std::cout << "Recall: " << result.recall << "\n";
            std::cout << "F1 Score: " << result.f1 << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}