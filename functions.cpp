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

int truePositiveNotes(const std::vector<Note>& originalNotes, const std::vector<Note>& benchmarkNotes, const std::vector<std::size_t>& indices)
{
    int count = 0;
    std::vector<bool> matched(benchmarkNotes.size(), false);
    for (auto i : indices)
    {
        const auto& note = originalNotes[i];

        for (const auto& benchmarkNote : benchmarkNotes)
        {
            if (!matched[&benchmarkNote - &benchmarkNotes[0]] && note.pitch == benchmarkNote.pitch && note.startTick == benchmarkNote.startTick)
            {
                ++count;
                matched[&benchmarkNote - &benchmarkNotes[0]] = true;
                break;
            }
        }
    }
    return count;
}