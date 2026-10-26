#include "tests.h"

int main() {
    noteTest A;
    juce::UnitTestRunner runner;
    runner.runAllTests();
    if (runner.getNumResults() == 0) {
        return 1;
    }
    for (int i = 0; i < runner.getNumResults(); ++i) {
        const auto& result = runner.getResult(i);
        if (result->failures > 0 && result->passes == 0) {
            return 1;
        }
    }
    return 0;
}