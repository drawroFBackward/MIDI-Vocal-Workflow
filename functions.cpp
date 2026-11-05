#include "functions.h"

juce::File filePath(std::string path)
{
    juce::File midiPath(path);
    if (!midiPath.existsAsFile())
    {
        throw std::runtime_error("MIDI file does not exist at the specified path.");
    }
    return midiPath;
}

juce::MidiFile readMidiFile(const std::string& path)
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

juce::MidiFile notesToMidiFile(const std::vector<Note>& notes, int ppq)
{
    if (ppq <= 0)
        throw std::invalid_argument("PPQ must be positive");
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ppq);

    juce::MidiMessageSequence sequence;
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

void exportMidiFile(const juce::MidiFile& midiFile, const std::string& path)
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