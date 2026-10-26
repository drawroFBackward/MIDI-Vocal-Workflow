#pragma once
#include <iostream>
#include "melody.h"
#include "functions.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

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