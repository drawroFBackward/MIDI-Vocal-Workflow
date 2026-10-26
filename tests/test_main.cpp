#include "tests.h"

int main() {
    noteTest A;
    juce::UnitTestRunner runner;
    runner.runAllTests();
    return 0;
}