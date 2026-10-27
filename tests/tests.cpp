#include "tests.h"

std::vector<Note> testExtractNotes(const juce::MidiMessageSequence& sequence, int trackIndex) {
    juce::MidiFile midiFile;
    midiFile.addTrack(sequence);
    auto notes = extractNotes(midiFile);
    return notes;
}

void noteTest::runTest() {
    testCase("Verify if note is read correctly", [&] {
        juce::MidiMessageSequence t;
        t.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)80), 1.0);
        t.addEvent(juce::MidiMessage::noteOff(1, 60), 481.0);
        auto notes = testExtractNotes(t);
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
        auto notes = testExtractNotes(t);
        expectEquals(static_cast<int>(notes.size()), 2);
        if (notes.size() < 2) return;
        expect(notes[0].channel ^ notes[1].channel == 1);
        // expect(notes[0].channel ^ notes[1].channel == 2); // for channel 2, commented out due to redundancy
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
        auto notes = testExtractNotes(t);
        expectEquals(static_cast<int>(notes.size()), 1);
        if (notes.empty()) return;
        expect(!notes[0].isComplete);
        expectEquals(notes[0].pitch, 60);
        expectEquals(notes[0].velocity, 80);
        expectEquals(notes[0].track, 0);
        expectEquals(notes[0].channel, 1);
        expectEquals(notes[0].startTick, 1.0);
        expectEquals(notes[0].duration, 0.0);
    });
}

void zeroDurationNoteTest::runTest() {
    testCase("Verify if zero duration notes are handled correctly", [&] {
        juce::MidiMessageSequence t;
        t.addEvent(juce::MidiMessage::noteOn(1, 62, (juce::uint8)75), 240.0);
        t.addEvent(juce::MidiMessage::noteOff(1, 62), 240.0); // Note off at the same tick
        expect(t.getEventPointer(0)->message.isNoteOn());
        expect(t.getEventPointer(1)->message.isNoteOff());
        auto notes = testExtractNotes(t);
        expectEquals(static_cast<int>(notes.size()), 1);
        if (notes.empty()) return;
        expect(notes[0].isComplete);
        expectEquals(notes[0].pitch, 62);
        expectEquals(notes[0].velocity, 75);
        expectEquals(notes[0].track, 0);
        expectEquals(notes[0].channel, 1);
        expectEquals(notes[0].startTick, 240.0);
        expectEquals(notes[0].duration, 0.0); // Duration should be zero
    });
}

void zeroVelocityNoteOnTest::runTest() {
    testCase("Verify if zero velocity note on is treated as note off", [&] {
        juce::MidiMessageSequence t;
        t.addEvent(juce::MidiMessage::noteOn(3, 64, (juce::uint8)95), 120.0);
        t.addEvent(juce::MidiMessage::noteOn(3, 64, (juce::uint8)0), 600.0); // Zero velocity note on
        auto notes = testExtractNotes(t);
        expectEquals(static_cast<int>(notes.size()), 1);
        if (notes.empty()) return;
        expect(notes[0].isComplete);
        expectEquals(notes[0].pitch, 64);
        expectEquals(notes[0].velocity, 95);
        expectEquals(notes[0].track, 0);
        expectEquals(notes[0].channel, 3);
        expectEquals(notes[0].startTick, 120.0);
        expectEquals(notes[0].duration, 480.0); // Duration should be from tick 120 to tick 600
    });
}