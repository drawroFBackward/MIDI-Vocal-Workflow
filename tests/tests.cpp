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

void validNoteGenerationTest::runTest() {
    testCase("Verify if valid notes are correctly converted to MIDI", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0)
        };
        notes[0].isComplete = true;
        notes[1].isComplete = true;
        juce::MidiFile reference;
        reference.setTicksPerQuarterNote(480);
        juce::MidiFile midiFile = notesToMidiFile(notes, reference);
        expectEquals(midiFile.getNumTracks(), 1);
        expectEquals(midiFile.getTrack(0)->getNumEvents(), 4); // 2 noteOn + 2 noteOff
        expect(midiFile.getTrack(0)->getEventPointer(0)->message.isNoteOn());
        expect(midiFile.getTrack(0)->getEventPointer(1)->message.isNoteOff());
        expect(midiFile.getTrack(0)->getEventPointer(2)->message.isNoteOn());
        expect(midiFile.getTrack(0)->getEventPointer(3)->message.isNoteOff());
        expectEquals(midiFile.getTrack(0)->getEventPointer(0)->message.getNoteNumber(), 60);
        expectEquals(midiFile.getTrack(0)->getEventPointer(1)->message.getNoteNumber(), 60);
        expectEquals(midiFile.getTrack(0)->getEventPointer(2)->message.getNoteNumber(), 62);
        expectEquals(midiFile.getTrack(0)->getEventPointer(3)->message.getNoteNumber(), 62);
        expectEquals(midiFile.getTrack(0)->getEventPointer(0)->message.getVelocity(), static_cast<juce::uint8>(80));
        expectEquals(midiFile.getTrack(0)->getEventPointer(2)->message.getVelocity(), static_cast<juce::uint8>(85));
        expectEquals(midiFile.getTrack(0)->getEventPointer(0)->message.getTimeStamp(), 0.0);
        expectEquals(midiFile.getTrack(0)->getEventPointer(1)->message.getTimeStamp(), 480.0);
        expectEquals(midiFile.getTrack(0)->getEventPointer(2)->message.getTimeStamp(), 480.0);
        expectEquals(midiFile.getTrack(0)->getEventPointer(3)->message.getTimeStamp(), 960.0);
    });
}

void incompleteNoteRejectionTest::runTest() {
    testCase("Verify if incomplete notes are rejected", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0)
        };
        notes[0].isComplete = true;
        // notes[1].isComplete = false;
        bool exceptionThrown = false;
        try {
            juce::MidiFile reference;
            reference.setTicksPerQuarterNote(480);
            juce::MidiFile midiFile = notesToMidiFile(notes, reference);
        } catch (const std::runtime_error&) {
            exceptionThrown = true;
        }
        expect(exceptionThrown);
    });
}

void invalidPPQTest::runTest() {
    testCase("Verify if invalid PPQ is rejected", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0)
        };
        notes[0].isComplete = true;
        bool exceptionThrown = false;
        try {
            juce::MidiFile reference;
            reference.setTicksPerQuarterNote(0);
            juce::MidiFile midiFile = notesToMidiFile(notes, reference);
        } catch (const std::invalid_argument&) {
            exceptionThrown = true;
        }
        expect(exceptionThrown);
    });
}

void rejectFractionsTest::runTest() {
    testCase("Verify if notes with fractional startTick or duration are rejected", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.5, 480.0)
        };
        notes[0].isComplete = true;
        notes[1].isComplete = true;
        bool exceptionThrown = false;
        try {
            juce::MidiFile reference;
            reference.setTicksPerQuarterNote(480);
            juce::MidiFile midiFile = notesToMidiFile(notes, reference);
        } catch (const std::runtime_error&) {
            exceptionThrown = true;
        }
        expect(exceptionThrown);
    });
}

void rejectNegativeValuesTest::runTest() {
    testCase("Verify if notes with negative startTick or duration are rejected", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, -480.0, 480.0)
        };
        notes[0].isComplete = true;
        notes[1].isComplete = true;
        bool exceptionThrown = false;
        try {
            juce::MidiFile reference;
            reference.setTicksPerQuarterNote(480);
            juce::MidiFile midiFile = notesToMidiFile(notes, reference);
        } catch (const std::runtime_error&) {
            exceptionThrown = true;
        }
        expect(exceptionThrown);
    });
}

void metadataPreservationTest::runTest() {
    testCase("Verify if metadata is preserved in the exported MIDI file", [&] {
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0)
        };
        notes[0].isComplete = true;
        juce::MidiFile reference;
        reference.setTicksPerQuarterNote(480);
        juce::MidiMessageSequence _, sequence;
        juce::MidiMessage keySig = juce::MidiMessage::keySignatureMetaEvent(0, 0);
        juce::MidiMessage timeSig = juce::MidiMessage::timeSignatureMetaEvent(4, 4);
        juce::MidiMessage tempo = juce::MidiMessage::tempoMetaEvent(500000); // 120 BPM
        juce::MidiMessage tempo2 = juce::MidiMessage::tempoMetaEvent(600000); // 100 BPM
        sequence.addEvent(keySig);
        sequence.addEvent(timeSig);
        sequence.addEvent(tempo);
        sequence.addEvent(tempo2, 960);
        reference.addTrack(_);
        reference.addTrack(sequence);
        juce::MidiFile midiFile = notesToMidiFile(notes, reference);
        juce::File outputFile("output_metadata.mid");
        exportMidiFile(midiFile, "output_metadata.mid");
        juce::FileInputStream inputStream(outputFile);
        juce::MidiFile readMidi;
        expect(readMidi.readFrom(inputStream));
        juce::MidiMessageSequence readSequence;
        readMidi.findAllKeySigEvents(readSequence);
        readMidi.findAllTimeSigEvents(readSequence);
        readMidi.findAllTempoEvents(readSequence);
        expect(readSequence.getNumEvents() == 4);
        expectEquals(readSequence.getEventPointer(0)->message.getKeySignatureNumberOfSharpsOrFlats(), keySig.getKeySignatureNumberOfSharpsOrFlats());
        expectEquals(readSequence.getEventPointer(0)->message.getTimeStamp(), static_cast<double>(0));
        int n, d;
        readSequence.getEventPointer(1)->message.getTimeSignatureInfo(n, d);
        expectEquals(n, 4);
        expectEquals(d, 4);
        expectEquals(readSequence.getEventPointer(1)->message.getTimeStamp(), static_cast<double>(0));
        expectEquals(readSequence.getEventPointer(2)->message.getTempoSecondsPerQuarterNote(), tempo.getTempoSecondsPerQuarterNote());
        expectEquals(readSequence.getEventPointer(2)->message.getTimeStamp(), static_cast<double>(0));
        expectEquals(readSequence.getEventPointer(3)->message.getTempoSecondsPerQuarterNote(), tempo2.getTempoSecondsPerQuarterNote());
        expectEquals(readSequence.getEventPointer(3)->message.getTimeStamp(), static_cast<double>(960));
    });
}

void outputMatchedInputTest::runTest() {
    testCase("Verify if exported MIDI matches input notes", [&] {
        std::vector<Note> oldNotes = {
            Note(40, 80, 0, 1, 0.0, 480.0),
            Note(41, 80, 0, 1, 480.0, 480.0),
            Note(42, 80, 0, 1, 960.0, 480.0),
            Note(43, 80, 0, 1, 1440.0, 480.0)
        };
        oldNotes[0].isComplete = true;
        oldNotes[1].isComplete = true;
        oldNotes[2].isComplete = true;
        oldNotes[3].isComplete = true;
        juce::MidiFile reference;
        reference.setTicksPerQuarterNote(480);
        juce::MidiFile oldMidiFile = notesToMidiFile(oldNotes, reference);
        std::vector<Note> notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(60, 80, 0, 2, 0.0, 240.0),
            Note(62, 85, 0, 1, 480.0, 480.0)
        };
        notes[0].isComplete = true;
        notes[1].isComplete = true;
        notes[2].isComplete = true;
        juce::MidiFile outputMidiFile = notesToMidiFile(notes, reference);
        juce::MidiFile createdMidiFile;
        try {
            exportMidiFile(oldMidiFile, "output.mid");
            exportMidiFile(outputMidiFile, "output.mid"); //Overwrite the previous file
            juce::File inputFile("output.mid");
            juce::FileInputStream inputStream(inputFile);
            if(!createdMidiFile.readFrom(inputStream))
                expect(false);
        } catch (const std::exception&) {
            expect(false);
        }
        expectEquals(static_cast<int>(createdMidiFile.getTimeFormat()), 480);
        auto notesInMidiFile = extractNotes(createdMidiFile);
        if(notesInMidiFile.size() != notes.size()) {
            expect(false);
            return;
        }
        for (size_t i = 0; i < notes.size(); ++i) {
            expect(notesInMidiFile[i].pitch == notes[i].pitch);
            expect(notesInMidiFile[i].velocity == notes[i].velocity);
            expect(notesInMidiFile[i].channel == notes[i].channel);
            expect(notesInMidiFile[i].startTick == notes[i].startTick);
            expect(notesInMidiFile[i].duration == notes[i].duration);
        }
    });
}

void evaluate_knownResultTest::runTest() {
    testCase("Evaluate known result", [&] {
        std::vector<Note> originalNotes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0),
            Note(64, 90, 0, 1, 960.0, 480.0),
            Note(65, 95, 0, 1, 1440.0, 480.0)
        };
        std::vector<Note> benchmarkNotes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0)
        };
        for (auto& note : originalNotes) note.isComplete = true;
        for (auto& note : benchmarkNotes) note.isComplete = true;
        std::vector<std::size_t> selectedIndices = {0, 3};
        BenchmarkResult result = evaluate(originalNotes, benchmarkNotes, selectedIndices, "test_song");
        expectEquals(result.TP, 1);
        expectEquals(result.FP, 1);
        expectEquals(result.FN, 1);
        expectEquals(result.precision, 0.5);
        expectEquals(result.recall, 0.5);
        expectEquals(result.f1, 0.5);
    });
}

void evaluate_zeroResultTest::runTest() {
    testCase("Evaluate zero result", [&] {
        std::vector<Note> Notes = {
            Note(60, 80, 0, 1, 0.0, 480.0),
            Note(62, 85, 0, 1, 480.0, 480.0)
        };
        std::vector<std::size_t> selectedIndices = {};
        BenchmarkResult result = evaluate(Notes, Notes, selectedIndices, "test_song");
        expectEquals(result.TP, 0);
        expectEquals(result.FP, 0);
        expectEquals(result.FN, 2);
        expectEquals(result.precision, 0.0);
        expectEquals(result.recall, 0.0);
        expectEquals(result.f1, 0.0);
    });
}

void pairFiles_generalTest::runTest() {
    testCase("Pair Files General Test", [&] {
        juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("pairFilesTest");
        tempDir.createDirectory();
        expect(tempDir.exists());
        juce::File originalFile = tempDir.getChildFile("Original_Test.mid");
        juce::File benchmarkFile = tempDir.getChildFile("Modified_Test.mid");
        expect(originalFile.create().wasOk());
        expect(benchmarkFile.create().wasOk());
        expect(originalFile.existsAsFile());
        expect(benchmarkFile.existsAsFile());
        std::vector<FilePair> filePairs = pairFiles(tempDir.getFullPathName());
        expectEquals(static_cast<int>(filePairs.size()), 1);
        expectEquals(filePairs[0].original, originalFile.getFullPathName());
        expectEquals(filePairs[0].benchmark, benchmarkFile.getFullPathName());
        tempDir.deleteRecursively();
    });
}
void pairFiles_noOriginalFilesTest::runTest() {
    testCase("Pair Files No Original Files Test", [&] {
        juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("pairFilesTestNoOriginal");
        tempDir.createDirectory();
        expect(tempDir.exists());
        juce::File benchmarkFile = tempDir.getChildFile("Modified_Test.mid");
        expect(benchmarkFile.create().wasOk());
        expect(benchmarkFile.existsAsFile());
        bool A = false;
        try {
            std::vector<FilePair> filePairs = pairFiles(tempDir.getFullPathName());
        } catch (...) {
            A = true;
        }
        expect(A);
        tempDir.deleteRecursively();
    });
}

void pairFiles_noBenchmarkFilesTest::runTest() {
    testCase("Pair Files No Benchmark Files Test", [&] {
        juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("pairFilesTestNoBenchmark");
        tempDir.createDirectory();
        expect(tempDir.exists());
        juce::File originalFile = tempDir.getChildFile("Original_Test.mid");
        expect(originalFile.create().wasOk());
        expect(originalFile.existsAsFile());
        bool A = false;
        try {
            std::vector<FilePair> filePairs = pairFiles(tempDir.getFullPathName());
        } catch (...) {
            A = true;
        }
        expect(A);
        tempDir.deleteRecursively();
    });
}

void pairFiles_directoryDoesNotExistTest::runTest() {
    testCase("Pair Files Directory Does Not Exist Test", [&] {
        try {
            std::vector<FilePair> filePairs = pairFiles("non_existent_directory");
        } catch (...) {
            expect(true);
            return;
        }
        expect(false);
    });
}

void pairFiles_unicodeFileNamesTest::runTest() {
    testCase("Pair Files Unicode File Names Test", [&] {
        juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("pairFilesTestUnicode");
        tempDir.createDirectory();
        expect(tempDir.exists());
        juce::File originalFile = tempDir.getChildFile("Original_千本桜.mid");
        juce::File benchmarkFile = tempDir.getChildFile("Modified_千本桜.mid");
        expect(originalFile.create().wasOk());
        expect(benchmarkFile.create().wasOk());
        expect(originalFile.existsAsFile());
        expect(benchmarkFile.existsAsFile());
        std::vector<FilePair> filePairs = pairFiles(tempDir.getFullPathName());
        if (filePairs.size() != 1) 
        {
            expect(false);
            return;
        }
        expectEquals(filePairs[0].original, originalFile.getFullPathName());
        expectEquals(filePairs[0].benchmark, benchmarkFile.getFullPathName());
        tempDir.deleteRecursively();
    });
}