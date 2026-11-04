#include "tests.h"

std::vector<Note> testExtractNotes(const juce::MidiMessageSequence& sequence) {
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
        expect(!(notes[0].channel == notes[1].channel));// no need to verify that they are Ch1 and Ch2 as later the test will fail with unexpected channel
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

void multiplePitchTest::runTest() {
    testCase("Verify if multiple pitches are handled correctly", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0),
            Note(64, 90, 0, 1, 960.0, 480.0)
        };
        auto selectedIndices = selectByPitch(notes, 62);
        expectEquals(static_cast<int>(selectedIndices.size()), 2);
        if (selectedIndices.size() < 2) return;
        expectEquals(selectedIndices[0], static_cast<std::size_t>(1));
        expectEquals(selectedIndices[1], static_cast<std::size_t>(2));
    });
}

void simultaneousNotesAboveMinPitchTest::runTest() {
    testCase("Verify if simultaneous notes above min pitch are handled correctly", [&] {
        std::vector<Note> notes = {
            Note(65, 80, 0, 1, 0.0, 480.0),
            Note(67, 85, 0, 1, 0.0, 480.0),
            Note(69, 90, 0, 1, 0.0, 480.0)
        };
        auto selectedIndices = selectByPitch(notes, 66);
        expectEquals(static_cast<int>(selectedIndices.size()), 2);
        if (selectedIndices.size() < 2) return;
        expectEquals(selectedIndices[0], static_cast<std::size_t>(1));
        expectEquals(selectedIndices[1], static_cast<std::size_t>(2));
    });
}

void noNotesAboveMinPitchTest::runTest() {
    testCase("Verify if no notes above min pitch are handled correctly", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(61, 85, 0, 1, 480.0, 480.0),
            Note(62, 90, 0, 1, 960.0, 480.0)
        };
        auto selectedIndices = selectByPitch(notes, 63);
        expectEquals(static_cast<int>(selectedIndices.size()), 0);
    });
}

void emptySequenceTest::runTest() {
    testCase("Verify if empty sequence is handled correctly", [&] {
        std::vector<Note> notes; // Empty vector
        auto selectedIndices = selectByPitch(notes, 60);
        expectEquals(static_cast<int>(selectedIndices.size()), 0);
    });
}

void testNotesUnmodified::runTest() {
    testCase("Verify if notes are unmodified after selectByPitch", [&] {
        std::vector<Note> originalNotes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0),
            Note(64, 90, 0, 1, 960.0, 480.0)
        };
        std::vector<Note> notes = originalNotes; // Copy of original notes
        auto selectedIndices = selectByPitch(notes, 62);
        expectEquals(static_cast<int>(notes.size()), static_cast<int>(originalNotes.size()));
        if (notes.size() != originalNotes.size()) return;
        for (std::size_t i = 0; i < notes.size(); ++i) {
            expectEquals(notes[i].pitch, originalNotes[i].pitch);
            expectEquals(notes[i].velocity, originalNotes[i].velocity);
            expectEquals(notes[i].track, originalNotes[i].track);
            expectEquals(notes[i].channel, originalNotes[i].channel);
            expectEquals(notes[i].startTick, originalNotes[i].startTick);
            expectEquals(notes[i].duration, originalNotes[i].duration);
            if (notes[i].isComplete != originalNotes[i].isComplete) {
                expect(false); // isComplete should be the same
            }
        }
    });
}

void duplicateNotesTest::runTest() {
    testCase("Verify if duplicate notes are handled correctly", [&] {
        std::vector<Note> originalNotes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0)
        };
        std::vector<Note> benchmarkNotes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0)
        };
        std::vector<std::size_t> indices = {0, 1, 2};
        int tp = truePositiveNotes(originalNotes, benchmarkNotes, indices);
        expectEquals(tp, 2);
    });
}