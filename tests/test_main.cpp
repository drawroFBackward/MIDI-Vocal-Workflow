#include "tests.h"

int main() {
    // Tests
    // extractNotes tests
    noteTest A;
    overlapTest B;
    incompleteNoteTest C;
    zeroDurationNoteTest D;
    zeroVelocityNoteOnTest E;
    // selectByPitch tests
    multiplePitchTest F;
    simultaneousNotesAboveMinPitchTest G;
    noNotesAboveMinPitchTest H;
    emptySequenceTest I;
    testNotesUnmodified J;
    // truePositiveNotes tests
    duplicateNotesTest K;
    // notesToMidiFile tests
    validNoteGenerationTest L;
    incompleteNoteRejectionTest M;
    invalidPPQTest N;
    rejectFractionsTest O;
    rejectNegativeValuesTest P;
    metadataPreservationTest Q;
    // exportMidiFile tests
    outputMatchedInputTest R;

    // run all tests
    juce::UnitTestRunner runner;
    runner.runAllTests();
    if (runner.getNumResults() == 0) {
        return 1;
    }
    for (int i = 0; i < runner.getNumResults(); ++i) {
        const auto& result = runner.getResult(i);
        if (result->failures > 0 || result->passes == 0) {
            std::cerr << "Test failed: " << result->unitTestName.toStdString() << "\n";
            for (const auto& failure : result->messages) {
                std::cerr << failure.toStdString() << "\n";
            }
            return 1;
        }
    }
    return 0;
}