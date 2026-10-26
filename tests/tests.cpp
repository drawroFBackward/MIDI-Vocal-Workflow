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
        expectEquals(notes[0].pitch, 60);
        expectEquals(notes[0].velocity, 80);
        expectEquals(notes[0].track, 0);
        expectEquals(notes[0].channel, 1);
        expectEquals(notes[0].startTick, 1.0);
        expectEquals(notes[0].duration, 480.0);
        expect(notes[0].isComplete);
    });
}