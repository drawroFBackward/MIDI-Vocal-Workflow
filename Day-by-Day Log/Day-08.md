# Day 08 — Pitch-Based Vocal Selection & Benchmarking

**Date:** 2026-10-30 (JST)  
**Status:** Completed; feature branch pending integration  
**Branch:** `feature/select-by-pitch`

## Objective
Implement the first measurable vocal-note selector, test its behavior, and benchmark it against a manually cleaned MIDI vocal reference.

## Work completed
**All four parts PASSED:**
1. Implemented `selectByPitch(notes, minPitch)`: returns original-vector indices for pitches ≥ threshold, preserves input and ordering; **O(N)** time, **O(K)** output space.
2. Added five JUCE selection tests: threshold behavior, simultaneous notes, no selections, empty input and input immutability.
3. Registered all tests with the JUCE test runner; verified that failures produce a nonzero process exit code.
4. Implemented `truePositiveNotes()` with pitch/onset matching and one-to-one reference-note consumption; derived FP, FN, precision, recall and F1. Added duplicate-note matching test and refactored pointer arithmetic to an indexed loop.

## Verification and benchmark
**Build succeeded; 11 registered JUCE tests passed (process exit code 0).**

**Dataset:** 空に免じて — original MIDI (1,935 notes) and manually cleaned reference (632 notes). **Selection rule:** pitch ≥ 60.

| Metric | Result |
|---|---:|
| Selected notes | 1,077 |
| True positives (TP) | 610 |
| False positives (FP) | 467 |
| False negatives (FN) | 22 |
| Precision | 56.64% |
| Recall | 96.52% |
| F1 | **71.39%** |

Sanity checks: **TP + FN = 632** reference notes; **TP + FP = 1,077** selected notes. The baseline recovers most vocal notes but also retains substantial accompaniment.

## Engineering lessons
- **Metric validity vs. implementation correctness:** Initially matching track/channel/duration produced misleading results; the agreed onset-based benchmark uses pitch + onset.
- **TP/FP/FN definitions:** False negatives count reference notes missed, not selected notes absent from the reference.
- **One-to-one matching:** Duplicate selected notes must not claim one reference note multiple times.
- **Invariants and testing:** Count identities catch accounting errors; a duplicate-note unit test catches a non-obvious matching bug.
- **Readability and separation of concerns:** Keep evaluation separate from selection; prefer indexed access over unnecessary pointer arithmetic.

## Limitations and decisions
- One-song results do **not** establish generalization; expand to diverse arrangements.
- Exact tick matching assumes compatible MIDI time resolution; future evaluation may normalize PPQ and allow timing tolerance.
- Pitch/onset alone can conflate same-pitch, same-onset notes with different durations; retain duration-aware evaluation as a future metric.
- Current evaluator uses a nested scan, **O(S × R)**; optimization is not yet necessary.
- MIDI benchmarks are stored in a **separate private repository** rather than the public source-code repository.
- Paths are currently machine-dependent; make dataset location configurable later.
- Keep `feature/select-by-pitch` unmerged pending integration review.

## Next step
Proceed to Day 9 with improvements informed by the measured false-positive problem, preserving the baseline for comparison.
