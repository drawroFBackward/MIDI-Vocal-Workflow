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
