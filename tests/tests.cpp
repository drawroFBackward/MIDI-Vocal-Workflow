#include "tests.h"

void noteTest::runTest() {
    testCase("Verify if note is read correctly", [&] {
        juce::MidiMessageSequence t;
        t.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)80), 1.0);
        t.addEvent(juce::MidiMessage::noteOff(1, 60), 481.0);
        juce::MidiFile midiFile;
        midiFile.addTrack(t);
        auto notes = extractNotes(midiFile);
        expectEquals(static_cast<int>(notes.size()), 1);
        if (notes.empty()) return;
        expectEquals(notes[0].pitch, 60);
        expectEquals(notes[0].velocity, 80);
        expectEquals(notes[0].track, 0);
        expectEquals(notes[0].channel, 1);
        expectEquals(notes[0].startTick, 1.0);
        expectEquals(notes[0].duration, 480.0);
        expect(notes[0].isComplete);
    });
}

void overlapTest::runTest() {
    testCase("Verify if overlapping notes are handled correctly", [&] {
        juce::MidiMessageSequence t;
        // Channel 1 note starts at tick 0 and ends at tick 960
        // Channel 2 note starts at tick 1 and ends at tick 481
        t.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)80), 0.0);
        t.addEvent(juce::MidiMessage::noteOn(2, 60, (juce::uint8)90), 1.0); 
        t.addEvent(juce::MidiMessage::noteOff(2, 60), 481.0);
        t.addEvent(juce::MidiMessage::noteOff(1, 60), 960.0);
        juce::MidiFile midiFile;
        midiFile.addTrack(t);
        auto notes = extractNotes(midiFile);
        expectEquals(static_cast<int>(notes.size()), 2);
        if (notes.size() < 2) return;
        for (const auto& note : notes) {
            expectEquals(note.pitch, 60);
            if (note.channel == 1) {
                expectEquals(note.startTick, 0.0);
                expectEquals(note.duration, 960.0);
                expect(note.isComplete);
            } else if (note.channel == 2) {
                expectEquals(note.startTick, 1.0);
                expectEquals(note.duration, 480.0);
                expect(note.isComplete);
            } else {
                expect(false); // Unexpected channel
            }
        }
    });
}

void incompleteNoteTest::runTest() {
    testCase("Verify if incomplete notes are caught correctly", [&] {
        juce::MidiMessageSequence t;
        t.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)80), 1.0);
        juce::MidiFile midiFile;
        midiFile.addTrack(t);
        auto notes = extractNotes(midiFile);
        expectEquals(static_cast<int>(notes.size()), 1);
        if (notes.empty()) return;
        expect(!notes[0].isComplete);
        expectEquals(notes[0].pitch, 60);
        expectEquals(notes[0].velocity, 80);
        expectEquals(notes[0].track, 0);
        expectEquals(notes[0].channel, 1);
        expectEquals(notes[0].startTick, 1.0);
    });
}