#include "functions.h"

// -----------------------------------------------------------------------------
// MIDI file access and conversion
// -----------------------------------------------------------------------------

juce::File filePath(const juce::String& path)
{
    juce::File midiPath(path);
    if (!midiPath.existsAsFile())
    {
        throw std::runtime_error("MIDI file does not exist at the specified path.");
    }
    return midiPath;
}

juce::MidiFile readMidiFile(const juce::String& path)
{
    juce::FileInputStream input(filePath(path));
    if (!input.openedOk())
    {
        throw std::runtime_error("Failed to open MIDI file");
    }

    juce::MidiFile midiFile;
    int fileType = -1;

    if (!midiFile.readFrom(input, true, &fileType))
    {
        throw std::runtime_error("Failed to read MIDI data");
    }
    return midiFile;
}

std::vector<Note> extractNotes(const juce::MidiFile& midiFile)
{
    std::vector<Note> notes;

    for (int trackIndex = 0; trackIndex < midiFile.getNumTracks(); ++trackIndex)
    {
        const auto* track = midiFile.getTrack(trackIndex);

        for (int i = 0; i < track->getNumEvents(); ++i)
        {
            const auto& message = track->getEventPointer(i)->message;

            if (message.isNoteOn())
            {
                notes.emplace_back(message.getNoteNumber(),
                                   static_cast<int>(message.getVelocity()),
                                   trackIndex,
                                   message.getChannel(),
                                   message.getTimeStamp(),
                                   0.0); // Duration will be calculated later
            }
            else if (message.isNoteOff())
            {
                for (auto& note : notes)
                {
                    if (!note.isComplete && note.pitch == message.getNoteNumber() &&
                        note.track == trackIndex && note.channel == message.getChannel())
                    {
                        note.duration = message.getTimeStamp() - note.startTick;
                        note.isComplete = true;
                        break;
                    }
                }
            }
        }
    }

    return notes;
}

juce::MidiFile notesToMidiFile(const std::vector<Note>& notes, const juce::MidiFile& reference)
{
    int ppq = reference.getTimeFormat();
    if (ppq <= 0)
        throw std::invalid_argument("PPQ must be positive");
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ppq);
    juce::MidiMessageSequence sequence;
    reference.findAllKeySigEvents(sequence);
    reference.findAllTempoEvents(sequence);
    reference.findAllTimeSigEvents(sequence);
    for (const auto& note : notes)
    {
        if (!note.isComplete){
            throw std::runtime_error("Incomplete note encountered");
        }
        if ((note.startTick != std::floor(note.startTick)) || (note.duration != std::floor(note.duration)) || (note.startTick < 0) || (note.duration < 0) || !std::isfinite(note.startTick) || !std::isfinite(note.duration))
            throw std::runtime_error("Note startTick and duration must be whole numbers and non-negative");
        sequence.addEvent(juce::MidiMessage::noteOn(note.channel, note.pitch, static_cast<juce::uint8>(note.velocity)), note.startTick);
        sequence.addEvent(juce::MidiMessage::noteOff(note.channel, note.pitch), note.startTick + note.duration);
    }

    sequence.sort();

    midiFile.addTrack(sequence);

    return midiFile;
}

void exportMidiFile(const juce::MidiFile& midiFile, const juce::String& path)
{
    juce::File file(path);
    juce::FileOutputStream outputStream(file);
    if (!outputStream.openedOk())
        throw std::runtime_error("Failed to open output file");
    outputStream.setPosition(0);
    outputStream.truncate();
    if (!midiFile.writeTo(outputStream))
        throw std::runtime_error("Failed to write MIDI file");
}

// -----------------------------------------------------------------------------
// Vocal-note selection
// -----------------------------------------------------------------------------

std::vector<std::size_t> selectByPitch(const std::vector<Note>& notes, int minPitch)
{
    std::vector<std::size_t> selectedIndices;
    for (std::size_t i = 0; i < notes.size(); ++i)
    {
        if (notes[i].pitch >= minPitch)
        {
            selectedIndices.push_back(i);
        }
    }
    return selectedIndices;
}

std::vector<std::size_t> selectNotes(const std::vector<Note>& notes)
{
    std::vector<std::size_t> selectedIndices = selectByPitch(notes, 60);
    // Add more selection in future
    return selectedIndices;
}

std::vector<Note> indicesToNotes(const std::vector<Note>& notes, const std::vector<std::size_t>& indices)
{
    std::vector<Note> selectedNotes;
    for (auto i : indices)
    {
        selectedNotes.push_back(notes[i]);
    }
    return selectedNotes;
}

// -----------------------------------------------------------------------------
// Reference matching and per-song evaluation
// -----------------------------------------------------------------------------

bool noteMatches(const Note& note, const Note& benchmarkNote)
{
    return note.pitch == benchmarkNote.pitch && note.startTick == benchmarkNote.startTick;
}

SongResult runEvaluation(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices)
{
    SongResult result;
    std::vector<bool> matched(benchmarkNotes.size(), false);
    for (auto i : indices)
    {
        const auto& note = originalNotes[i];
        bool found = false;
        for (int j = 0; j < benchmarkNotes.size(); ++j)
        {
            const auto& benchmarkNote = benchmarkNotes[j];
            if (!matched[j] && noteMatches(note, benchmarkNote))
            {
                result.truePositives.push_back(i);
                matched[j] = true;
                found = true;
                break;
            }
        }
        if (!found)
            result.falsePositives.push_back(i);
    }
    for (int j = 0; j < benchmarkNotes.size(); ++j)
    {
        if (!matched[j])
            result.falseNegatives.push_back(j);
    }
    return result;
}

int truePositiveNotes(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices)
{
    SongResult result = runEvaluation(originalNotes, benchmarkNotes, indices);
    int count = static_cast<int>(result.truePositives.size());
    return count;
}

BenchmarkResult evaluate(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices, const juce::String& songName)
{
    SongResult classification = runEvaluation(originalNotes, benchmarkNotes, indices);
    int tp = static_cast<int>(classification.truePositives.size());
    int fp = static_cast<int>(classification.falsePositives.size());
    int fn = static_cast<int>(classification.falseNegatives.size());

    double p = (tp + fp > 0) ? static_cast<double>(tp) / (tp + fp) : 0;
    double r = (tp + fn > 0) ? static_cast<double>(tp) / (tp + fn) : 0;
    double f = (p + r > 0) ? 2.0 * (p * r) / (p + r) : 0;

    BenchmarkResult result;
    result.songName = songName;
    result.TP = tp;
    result.FP = fp;
    result.FN = fn;
    result.precision = p;
    result.recall = r;
    result.f1 = f;
    return result;
}

// -----------------------------------------------------------------------------
// Dataset discovery and benchmark reporting
// -----------------------------------------------------------------------------

std::vector<juce::String> findOriginalMidiFiles(const juce::String& directoryPath)
{
    std::vector<juce::String> originalFiles;
    juce::File directory(directoryPath);
    if (!directory.isDirectory())
        throw std::runtime_error("Directory does not exist");
    // File is an original MIDI file if it has the .mid extension and it has Original_ as a prefix in the file name
    juce::Array<juce::File> files = directory.findChildFiles(juce::File::findFiles, false, "*.mid");
    for (auto& file : files)
    {
        if (file.getFileName().startsWith("Original_"))
        {
            originalFiles.push_back(file.getFullPathName());
        }
    }
    if (originalFiles.empty())
        throw std::runtime_error("No original MIDI files found in the directory");
    return originalFiles;
}

std::vector<FilePair> pairFiles(const juce::String& directoryPath)
{
    std::vector<FilePair> filePairs;
    std::vector<juce::String> originalFiles = findOriginalMidiFiles(directoryPath);
    for (const auto& originalFile : originalFiles)
    {
        juce::String originalFileName = juce::File(originalFile).getFileName();
        
        juce::String benchmarkFileName = "Modified_" + originalFileName.substring(juce::String("Original_").length());
        juce::File benchmarkFile(directoryPath + "/" + benchmarkFileName);
        if (benchmarkFile.existsAsFile())
        {
            FilePair pair;
            pair.original = originalFile;
            pair.benchmark = benchmarkFile.getFullPathName();
            filePairs.push_back(pair);
        }
        else
        {
            std::cout << "Benchmark file not found for " << originalFileName << "\n";
        }
    }
    if (filePairs.empty())
        throw std::runtime_error("No benchmark files found for the original MIDI files in the directory");
    return filePairs;
}

BenchmarkResult runBenchmark(const juce::String& originalMidiFilePath, const juce::String& benchmarkMidiFilePath)
{
    juce::MidiFile originalMidiFile = readMidiFile(originalMidiFilePath);
    juce::MidiFile benchmarkMidiFile = readMidiFile(benchmarkMidiFilePath);
    if (originalMidiFile.getTimeFormat() != benchmarkMidiFile.getTimeFormat())
        throw std::runtime_error(("MIDI files: " + originalMidiFilePath + " and " + benchmarkMidiFilePath + " have different PPQ (time format)").toStdString());

    std::vector<Note> originalNotes = extractNotes(originalMidiFile);
    std::vector<Note> benchmarkNotes = extractNotes(benchmarkMidiFile);

    std::vector<std::size_t> selectedIndices = selectNotes(originalNotes);

    return evaluate(originalNotes, benchmarkNotes, selectedIndices, originalMidiFilePath);
}

void BenchmarkReport::addResult(const BenchmarkResult& result)
{
    results.push_back(result);
}

void BenchmarkReport::clearResults()
{
    results.clear();
}

BenchmarkResult BenchmarkReport::getMicroAverageResult() const
{
    if (results.empty())
        throw std::runtime_error("No benchmark results available for micro average calculation");
    BenchmarkResult microAverage;
    for (const auto& result : results)
    {
        microAverage.TP += result.TP;
        microAverage.FP += result.FP;
        microAverage.FN += result.FN;
    }
    microAverage.precision = (microAverage.TP + microAverage.FP) > 0 ? static_cast<double>(microAverage.TP) / (microAverage.TP + microAverage.FP) : 0;
    microAverage.recall = (microAverage.TP + microAverage.FN) > 0 ? static_cast<double>(microAverage.TP) / (microAverage.TP + microAverage.FN) : 0;
    microAverage.f1 = (microAverage.precision + microAverage.recall) > 0 ? 2 * (microAverage.precision * microAverage.recall) / (microAverage.precision + microAverage.recall) : 0;
    return microAverage;
}

BenchmarkResult BenchmarkReport::getMacroAverageResult() const
{
    BenchmarkResult macroAverage;
    if (results.empty())
        throw std::runtime_error("No benchmark results available for macro average calculation");

    for (const auto& result : results)
    {
        macroAverage.precision += result.precision;
        macroAverage.recall += result.recall;
        macroAverage.f1 += result.f1;
    }
    macroAverage.precision /= results.size();
    macroAverage.recall /= results.size();
    macroAverage.f1 /= results.size();
    return macroAverage;
}

void BenchmarkReport::printReport() const
{
    if (results.empty())
    {
        std::cout << "No benchmark results available.\n";
        return;
    }
    for (const auto& result : results)
    {
        std::cout << "Song: " << result.songName << "\n";
        std::cout << "TP: " << result.TP << ", FP: " << result.FP << ", FN: " << result.FN << "\n";
        std::cout << "Precision: " << result.precision << ", Recall: " << result.recall << ", F1: " << result.f1 << "\n";
        std::cout << "--------------------------\n";
    }
    BenchmarkResult microAverage = getMicroAverageResult();
    BenchmarkResult macroAverage = getMacroAverageResult();
    std::cout << "Micro Average - Precision: " << microAverage.precision << ", Recall: " << microAverage.recall << ", F1: " << microAverage.f1 << "\n";
    std::cout << "Macro Average - Precision: " << macroAverage.precision << ", Recall: " << macroAverage.recall << ", F1: " << macroAverage.f1 << "\n";
}

// -----------------------------------------------------------------------------
// Time-window error diagnostics (reference-onset based)
// -----------------------------------------------------------------------------

double getSongLength(const juce::MidiFile& midiFile, const juce::MidiFile& benchmarkMidiFile)
{
    return std::max(midiFile.getLastTimestamp(), benchmarkMidiFile.getLastTimestamp());
}

bool windowCountsMatch(const std::vector<songWindow>& windows, const SongResult& classification)
{
    std::size_t tpCount = 0;
    std::size_t fpCount = 0;
    std::size_t fnCount = 0;

    for (const auto& window : windows)
    {
        if (window.TP < 0 || window.FP < 0 || window.FN < 0)
            return false;

        tpCount += static_cast<std::size_t>(window.TP);
        fpCount += static_cast<std::size_t>(window.FP);
        fnCount += static_cast<std::size_t>(window.FN);
    }

    return tpCount == classification.truePositives.size()
        && fpCount == classification.falsePositives.size()
        && fnCount == classification.falseNegatives.size();
}

std::vector<songWindow> runSongWindowAnalysis(const std::vector<Note>& originalNotes,
                                              const std::vector<Note>& benchmarkNotes,
                                              const std::vector<size_t>& selectedIndices,
                                              int numberOfWindows, double songLength)
{
    if (numberOfWindows <= 0)
        throw std::invalid_argument("Number of windows must be positive");

    if (!std::isfinite(songLength) || songLength <= 0.0)
        throw std::invalid_argument("Song length must be finite and positive");

    const SongResult classification = runEvaluation(originalNotes, benchmarkNotes, selectedIndices);
    const double windowSize = songLength / numberOfWindows;

    if (!std::isfinite(windowSize) || windowSize <= 0.0)
        throw std::invalid_argument("Window size must be finite and positive");

    std::vector<songWindow> windows;
    windows.reserve(static_cast<std::size_t>(numberOfWindows));
    for (int i = 0; i < numberOfWindows; ++i)
        windows.emplace_back(windowSize * i, windowSize * (i + 1));

    // Each note is assigned directly to its window, rather than rescanning
    // all classified notes for every window (O(W + N) instead of O(W * N)).
    const auto findWindow = [&](const Note& note) -> std::size_t
    {
        const double tick = note.startTick;
        if (!std::isfinite(tick) || tick < 0.0 || tick > songLength)
            throw std::out_of_range("Classified note onset is outside the song timeline");

        // The final window includes the song endpoint; intermediate windows
        // remain half-open [startTick, endTick).
        if (tick == songLength)
            return windows.size() - 1;

        return std::min(
            static_cast<std::size_t>(tick / windowSize),
            windows.size() - 1
        );
    };

    for (const auto index : classification.truePositives)
        ++windows[findWindow(originalNotes.at(index))].TP;

    for (const auto index : classification.falsePositives)
        ++windows[findWindow(originalNotes.at(index))].FP;

    for (const auto index : classification.falseNegatives)
        ++windows[findWindow(benchmarkNotes.at(index))].FN;

    if (!windowCountsMatch(windows, classification))
        throw std::logic_error("Window counts disagree with song classification");

    return windows;
}

SongWindowSummary summarizeSongWindows(const std::vector<songWindow>& windows)
{
    SongWindowSummary summary;
    for (const auto& window : windows)
    {
        if (window.isVocal())
        {
            ++summary.vocalWindowCount;
            summary.vocalFP += window.FP;
        }
        else
        {
            ++summary.nonVocalWindowCount;
            summary.nonVocalFP += window.FP;
        }
    }
    return summary;
}

double percentageFPinVocalWindows(const std::vector<songWindow>& windows)
{
    const SongWindowSummary summary = summarizeSongWindows(windows);
    const int totalFP = summary.vocalFP + summary.nonVocalFP;
    // For calculations, use 0 when there are no FPs; printed reports show N/A.
    return (totalFP > 0) ? 100.0 * summary.vocalFP / totalFP : 0.0;
}

double averageFPperWindow(const std::vector<songWindow>& windows, bool Vocal)
{
    const SongWindowSummary summary = summarizeSongWindows(windows);
    const std::size_t count = Vocal ? summary.vocalWindowCount : summary.nonVocalWindowCount;
    const int totalFP = Vocal ? summary.vocalFP : summary.nonVocalFP;
    return (count > 0) ? static_cast<double>(totalFP) / count : 0.0;
}

void printSongWindowAnalysis(const std::vector<songWindow>& windows)
{
    const SongWindowSummary summary = summarizeSongWindows(windows);

    std::cout << "Song Window Analysis:\n";
    std::cout << "Index\tStart\tEnd\tTP\tFP\tFN\tVocal\n";
    for (std::size_t i = 0; i < windows.size(); ++i)
    {
        const auto& window = windows[i];
        std::cout << i << "\t"
                  << window.startTick << "\t"
                  << window.endTick << "\t"
                  << window.TP << "\t"
                  << window.FP << "\t"
                  << window.FN << "\t"
                  << (window.isVocal() ? "Yes" : "No") << "\n";
    }

    std::cout << "Total vocal windows: " << summary.vocalWindowCount << "\n";
    std::cout << "Total FP in vocal windows: " << summary.vocalFP << "\n";
    std::cout << "Total non-vocal windows: " << summary.nonVocalWindowCount << "\n";
    std::cout << "Total FP in non-vocal windows: " << summary.nonVocalFP << "\n";

    const int totalFP = summary.vocalFP + summary.nonVocalFP;
    if (totalFP == 0)
    {
        std::cout << "Percentage of false positives in vocal windows: N/A (no false positives)\n";
        std::cout << "Percentage of false positives in non-vocal windows: N/A (no false positives)\n";
    }
    else
    {
        const double vocalPercentage = 100.0 * summary.vocalFP / totalFP;
        const double nonVocalPercentage = 100.0 * summary.nonVocalFP / totalFP;
        std::cout << "Percentage of false positives in vocal windows: " << vocalPercentage << "%\n";
        std::cout << "Percentage of false positives in non-vocal windows: " << nonVocalPercentage << "%\n";
    }

    const double vocalAverage = (summary.vocalWindowCount > 0)
        ? static_cast<double>(summary.vocalFP) / summary.vocalWindowCount : 0.0;
    const double nonVocalAverage = (summary.nonVocalWindowCount > 0)
        ? static_cast<double>(summary.nonVocalFP) / summary.nonVocalWindowCount : 0.0;
    std::cout << "Average false positives per vocal window: " << vocalAverage << "\n";
    std::cout << "Average false positives per non-vocal window: " << nonVocalAverage << "\n";
}

// -----------------------------------------------------------------------------
// Single-file vocal-note export workflow
// -----------------------------------------------------------------------------

void runAndExport(const juce::String& originalMidiFilePath)
{
    juce::MidiFile originalMidiFile = readMidiFile(originalMidiFilePath);
    std::vector<Note> originalNotes = extractNotes(originalMidiFile);
    auto selectedNotes = indicesToNotes(originalNotes, selectNotes(originalNotes));
    juce::MidiFile outputMidiFile = notesToMidiFile(selectedNotes, originalMidiFile);
    juce::String outputMidiFilePath = "Vocal_";
    outputMidiFilePath += juce::File(originalMidiFilePath).getFileName();
    exportMidiFile(outputMidiFile, outputMidiFilePath);
}
