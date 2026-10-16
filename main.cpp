
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <iostream>
#include "melody.h"

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

    // Step 4: Display information
    std::cout << "MIDI file loaded successfully!\n";
    std::cout << "File type: " << fileType << "\n";
    std::cout << "Tracks: " << midiFile.getNumTracks() << "\n";
    std::cout << "Timing division: "
              << midiFile.getTimeFormat() << "\n";
              
    for (int trackIndex = 0; trackIndex < midiFile.getNumTracks(); ++trackIndex)
    {
        const auto* track = midiFile.getTrack(trackIndex);

        std::cout << "\nTrack " << trackIndex << "\n";

        for (int i = 0; i < track->getNumEvents(); ++i)
        {
            const auto& message = track->getEventPointer(i)->message;

            if (message.isNoteOn())
            {
                std::cout << "Note On  | Pitch: "
                        << message.getNoteNumber()
                        << " | Velocity: " << static_cast<int>(message.getVelocity())
                        << " | Tick: " << message.getTimeStamp()
                        << "\n";
            }
            else if (message.isNoteOff())
            {
                std::cout << "Note Off | Pitch: "
                        << message.getNoteNumber()
                        << " | Tick: " << message.getTimeStamp()
                        << "\n";
            }
        }

        Note testNote(60, 100, 0.0, 1.0);

        std::cout << "\nTest Note:\n";
        std::cout << "Pitch: " << testNote.pitch
                    << " | Velocity: " << testNote.velocity
                    << " | Start Tick: " << testNote.startTick
                    << " | Duration: " << testNote.duration
                    << "\n";    
    }

    return 0;
}