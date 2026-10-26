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