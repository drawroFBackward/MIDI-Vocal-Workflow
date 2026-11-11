#include "functions.h"

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

int truePositiveNotes(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices)
{
    int count = 0;
    std::vector<bool> matched(benchmarkNotes.size(), false);
    for (auto i : indices)
    {
        const auto& note = originalNotes[i];

        for (int j = 0; j < benchmarkNotes.size(); ++j)
        {
            const auto& benchmarkNote = benchmarkNotes[j];
            if (!matched[j] && note.pitch == benchmarkNote.pitch && note.startTick == benchmarkNote.startTick)
            {
                ++count;
                matched[j] = true;
                break;
            }
        }
    }
    return count;
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

BenchmarkResult evaluate(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices, const juce::String& songName)
{
    int tp = truePositiveNotes(originalNotes, benchmarkNotes, indices);
    int fp = static_cast<int>(indices.size()) - tp;
    int fn = static_cast<int>(benchmarkNotes.size()) - tp;

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

BenchmarkResult runBenchmark(const juce::String& originalMidiFilePath, const juce::String& benchmarkMidiFilePath)
{
    juce::MidiFile originalMidiFile = readMidiFile(originalMidiFilePath);
    juce::MidiFile benchmarkMidiFile = readMidiFile(benchmarkMidiFilePath);

    std::vector<Note> originalNotes = extractNotes(originalMidiFile);
    std::vector<Note> benchmarkNotes = extractNotes(benchmarkMidiFile);

    std::vector<std::size_t> selectedIndices = selectNotes(originalNotes);

    return evaluate(originalNotes, benchmarkNotes, selectedIndices, originalMidiFilePath);
}

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

void BenchmarkReport::addResult(const BenchmarkResult& result)
{
    results.push_back(result);
}

void BenchmarkReport::clearResults()
{
    results.clear();
}