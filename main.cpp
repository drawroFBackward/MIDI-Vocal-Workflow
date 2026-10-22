#pragma once
#include <iostream>
#include "melody.h"
#include "functions.h"
#include <vector>
#include <algorithm>

int main()
{
    // Step 1: Specify the path to your MIDI file
    std::string midiFilePath = "C:\\Users\\C30413\\source\\repos\\MIDI-Vocal-Workflow\\test.mid";

    // Step 2: Read the MIDI file
    auto midiFile = readMidiFile(midiFilePath);

    // Step 3: Extract notes from the MIDI file
    std::vector<Note> notes = extractNotes(midiFile);

    // Reading report

    std::cout << "\nTotal notes read: " << notes.size() << "\n";
    int incompleteNotes = std::count_if(notes.begin(), notes.end(), [](const Note& n) { return !n.isComplete; });
    std::cout << "Complete notes: " << (notes.size() - incompleteNotes) << "\n";
    std::cout << "Incomplete notes: " << incompleteNotes << "\n";

    // Raise a warning if there are incomplete notes
    if (incompleteNotes)
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