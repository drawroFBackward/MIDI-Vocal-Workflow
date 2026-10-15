
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <iostream>

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

    return 0;
}