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