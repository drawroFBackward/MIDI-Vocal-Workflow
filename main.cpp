
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <iostream>
#include "melody.h"
#include <vector>

int main()
{
    // Step 1: Find our test file
    juce::File midiPath =
        juce::File::getCurrentWorkingDirectory()
            .getChildFile("test-data")
            .getChildFile("test.mid");

    // Step 2: Open the file
    juce::FileInputStream input(midiPath);

    if (!input.openedOk())
    {
        std::cerr << "Could not open MIDI file.\n";
        return 1;
    }

    // Step 3: Read MIDI data from the file
    juce::MidiFile midiFile;
    int fileType = -1;

    if (!midiFile.readFrom(input, true, &fileType))
    {
        std::cerr << "Failed to read MIDI data.\n";
        return 1;
    }

    // Step 4: Save notes to a vector of Note structs
    std::cout << "MIDI file loaded successfully!\n";
    std::cout << "File type: " << fileType << "\n";
    std::cout << "Tracks: " << midiFile.getNumTracks() << "\n";
    std::cout << "Timing division: "
              << midiFile.getTimeFormat() << "\n";

    std::vector<Note> notes;
    int completeNotesCount = 0;
              
    for (int trackIndex = 0; trackIndex < midiFile.getNumTracks(); ++trackIndex)
    {
        const auto* track = midiFile.getTrack(trackIndex);

        std::cout << "\nTrack " << trackIndex << "\n";

        for (int i = 0; i < track->getNumEvents(); ++i)
        {
            const auto& message = track->getEventPointer(i)->message;

            if (message.isNoteOn() && message.getVelocity() > 0)
            {
                notes.emplace_back(message.getNoteNumber(),
                                   static_cast<int>(message.getVelocity()),
                                   trackIndex,
                                   message.getChannel(),
                                   message.getTimeStamp(),
                                   0.0); // Duration will be calculated later
            }
            else if (message.isNoteOff() || (message.isNoteOn() && message.getVelocity() == 0))
            {
                // Find the corresponding note on event and calculate its duration
                for (auto& note : notes)
                {
                    if (note.pitch == message.getNoteNumber() && !note.isComplete && note.track == trackIndex && note.channel == message.getChannel())
                    {
                        note.duration = message.getTimeStamp() - note.startTick;
                        note.isComplete = true;
                        ++completeNotesCount;
                        break;
                    }
                }
            }
        }
        
    }
    
    // Reading report

    std::cout << "\nTotal notes read: " << notes.size() << "\n";
    std::cout << "Complete notes: " << completeNotesCount << "\n";
    std::cout << "Incomplete notes: " << (notes.size() - completeNotesCount) << "\n";

    // Raise a warning if there are incomplete notes
    if (completeNotesCount < notes.size())
    {
        std::cerr << "Warning: There are incomplete notes in the MIDI file.\n";
    }

    std::cout << "\nOur melody:\n";

    for (const Note& note : notes)
    {
        std::cout << "Pitch: " << note.pitch
                << " | Velocity: " << note.velocity
                << " | Start: " << note.startTick
                << " | Duration: " << note.duration
                << " | Track: " << note.track
                << " | Channel: " << note.channel
                << " | Complete: " << (note.isComplete ? "Yes" : "No")
                << "\n";
    }


    return 0;
}