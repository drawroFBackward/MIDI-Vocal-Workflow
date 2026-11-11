#pragma once
#include <iostream>
#include "melody.h"
#include "functions.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

// extractNotes() test functions
std::vector<Note> testExtractNotes(const juce::MidiMessageSequence& sequence);

class noteTest : public juce::UnitTest{
    public:
        noteTest() : juce::UnitTest("Note Test") {}

        void runTest() override;
};

class overlapTest : public juce::UnitTest{
    public:
        overlapTest() : juce::UnitTest("Overlap Test") {}

        void runTest() override;
};

class incompleteNoteTest : public juce::UnitTest{
    public:
        incompleteNoteTest() : juce::UnitTest("Incomplete Note Test") {}

        void runTest() override;
};

class zeroDurationNoteTest : public juce::UnitTest{
    public:
        zeroDurationNoteTest() : juce::UnitTest("Zero Duration Note Test") {}

        void runTest() override;
};

class zeroVelocityNoteOnTest : public juce::UnitTest{
    public:
        zeroVelocityNoteOnTest() : juce::UnitTest("Zero Velocity Note On Test") {}

        void runTest() override;
};

// selectByPitch() test functions
class multiplePitchTest : public juce::UnitTest{
    public:
        multiplePitchTest() : juce::UnitTest("Multiple Pitch Test") {}

        void runTest() override;
};

class simultaneousNotesAboveMinPitchTest : public juce::UnitTest{
    public:
        simultaneousNotesAboveMinPitchTest() : juce::UnitTest("Simultaneous Notes Above Min Pitch Test") {}

        void runTest() override;
};

class noNotesAboveMinPitchTest : public juce::UnitTest{
    public:
        noNotesAboveMinPitchTest() : juce::UnitTest("No Notes Above Min Pitch Test") {}

        void runTest() override;
};

class emptySequenceTest : public juce::UnitTest{
    public:
        emptySequenceTest() : juce::UnitTest("Empty Sequence Test") {}

        void runTest() override;
};

class testNotesUnmodified : public juce::UnitTest{
    public:
        testNotesUnmodified() : juce::UnitTest("Test Notes Unmodified") {}

        void runTest() override;
};

// truePositiveNotes() test functions
class duplicateNotesTest : public juce::UnitTest{
    public:
        duplicateNotesTest() : juce::UnitTest("Duplicate Notes Test") {}

        void runTest() override;
};

// notesToMidiFile() test functions
class validNoteGenerationTest : public juce::UnitTest{
    public:
        validNoteGenerationTest() : juce::UnitTest("Valid Note Generation Test") {}

        void runTest() override;
};

class incompleteNoteRejectionTest : public juce::UnitTest{
    public:
        incompleteNoteRejectionTest() : juce::UnitTest("Incomplete Note Rejection Test") {}

        void runTest() override;
};

class invalidPPQTest : public juce::UnitTest{
    public:
        invalidPPQTest() : juce::UnitTest("Invalid PPQ Test") {}

        void runTest() override;
};

class rejectFractionsTest : public juce::UnitTest{
    public:
        rejectFractionsTest() : juce::UnitTest("Reject Fractions Test") {}

        void runTest() override;
};

class rejectNegativeValuesTest : public juce::UnitTest{
    public:
        rejectNegativeValuesTest() : juce::UnitTest("Reject Negative Values Test") {}

        void runTest() override;
};

class metadataPreservationTest : public juce::UnitTest{
    public:
        metadataPreservationTest() : juce::UnitTest("Metadata Preservation Test") {}

        void runTest() override;
};

// exportMidiFile() test functions
class outputMatchedInputTest : public juce::UnitTest{
    public:
        outputMatchedInputTest() : juce::UnitTest("Output Matched Input Test") {}

        void runTest() override;
};

// evaluate() test functions
class evaluate_knownResultTest : public juce::UnitTest{
    public:
        evaluate_knownResultTest() : juce::UnitTest("Evaluate Known Result Test") {}

        void runTest() override;
};

class evaluate_zeroResultTest : public juce::UnitTest{
    public:
        evaluate_zeroResultTest() : juce::UnitTest("Evaluate Zero Result Test") {}

        void runTest() override;
};

// pairFiles() test functions

class pairFiles_generalTest : public juce::UnitTest{
    public:
        pairFiles_generalTest() : juce::UnitTest("Pair Files General Test") {}

        void runTest() override;
};

class pairFiles_noOriginalFilesTest : public juce::UnitTest{
    public:
        pairFiles_noOriginalFilesTest() : juce::UnitTest("Pair Files No Original Files Test") {}

        void runTest() override;
};

class pairFiles_noBenchmarkFilesTest : public juce::UnitTest{
    public:
        pairFiles_noBenchmarkFilesTest() : juce::UnitTest("Pair Files No Benchmark Files Test") {}

        void runTest() override;
};

class pairFiles_directoryDoesNotExistTest : public juce::UnitTest{
    public:
        pairFiles_directoryDoesNotExistTest() : juce::UnitTest("Pair Files Directory Does Not Exist Test") {}

        void runTest() override;
};

class pairFiles_unicodeFileNamesTest : public juce::UnitTest{
    public:
        pairFiles_unicodeFileNamesTest() : juce::UnitTest("Pair Files Unicode File Names Test") {}

        void runTest() override;
};