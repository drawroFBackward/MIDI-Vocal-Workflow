# Day 09 — MIDI Export, Round-Trip Verification & Musical Timing

**Dates:** 2026-11-04 to 2026-11-06 (JST)  
**Status:** Four challenge parts passed in code review; latest local test-suite execution not independently confirmed  
**Development branch:** `feature/midi-export` (pending cleanup and merge)  
**Integration:** Previous `feature/select-by-pitch` work was squash-merged into `main` through PR #1.

## Objective
Integrate the pitch-based selection baseline and generate reimportable vocal MIDI files, preserving note events, MIDI timing resolution, and musical tempo/time-signature metadata.

## Work completed
**All four parts PASSED in iterative code review:**
1. **CLI and integration:** Accepted two Unicode-friendly MIDI input paths on Windows using `wmain` and JUCE string conversion, retained original/reference benchmark evaluation and error handling. Integrated the previously developed selector into `main`.
2. **MIDI generation:** Implemented `notesToMidiFile()` with a one-track Note On/Off sequence, preserving pitch, velocity, MIDI channel, onset and duration. Rejects incomplete notes, nonpositive PPQ and invalid/fractional/negative note timing values.
3. **Disk output and round-trip:** Implemented `exportMidiFile()` with write-result checking and overwrite/truncation behavior. Added a disk write/reimport test including overlapping same-pitch notes on different channels, and a longer-file-to-shorter-file overwrite regression scenario. Improved failure reporting in the JUCE test runner.
4. **Musical timing preservation and real-song export:** Updated `notesToMidiFile(notes, reference)` to derive PPQ from the source and preserve key-signature, tempo and time-signature events collected from all reference tracks. Added a metadata round-trip test featuring 120 BPM at tick 0, 100 BPM at tick 960 and 4/4 metadata on a nonzero track. Registered this test in the test runner. The real-song CLI exported and reimported a MIDI file with the expected 1,077 selected notes.

## Verification and benchmark
**Code-review evidence:** `feature/midi-export` contains 18 registered JUCE tests, including the newly registered metadata-preservation test. The user earlier reported successful local tests, and provided a real-song export screenshot. **The latest suite was not independently executed by the assistant, and a new local pass after registration was not explicitly shown.**

**Dataset:** 空に免じて — source 1,935 notes, vocal reference 632 notes; selection rule pitch ≥ 60.

| Metric | Result |
|---|---:|
| Selected and reimported output notes | 1,077 |
| True positives (TP) | 610 |
| False positives (FP) | 467 |
| False negatives (FN) | 22 |
| Precision | 56.64% |
| Recall | 96.52% |
| F1 | **71.39%** |

The successful reimport verifies output readability and note count for this song, **not** perfect note-level or editor-specific fidelity. Editor import into VOCALOID6 remains to be tested.

## Engineering lessons
- **Evaluation contracts differ:** Pitch/onset matching is suitable for the current manually edited vocal-reference benchmark; exporter round trips require stricter comparison of event attributes, with original track indices intentionally not preserved by the one-track exporter.
- **C++ references and JUCE APIs:** `findAll...Events(sequence)` and `getTimeSignatureInfo(numerator, denominator)` write into supplied output parameters. Directly collecting metadata into the output sequence avoids an unnecessary intermediate `addSequence()`.
- **Musical time:** PPQ governs ticks per quarter note; tempo governs real-time duration; time signature governs metrical grouping. Mid-song tempo events must survive export even when no vocal notes occur at their ticks.
- **Testing discipline:** Register every new test; use meaningful property assertions rather than comparing unsupported `MidiMessage` objects or only their `0xFF` meta-event status byte. Test file overwrite with a *shorter* replacement.
- **Debugging:** The MSVC compiler's long C++ template-error report traced back to an unsupported `expectEquals<juce::MidiMessage>` call in the project's own `tests.cpp`, not to a JUCE implementation fault.

## Risks and deferred follow-ups
- The generated `output_metadata.mid` was accidentally committed to the branch; remove it from tracking and add it to `.gitignore` before merge. Test output files should eventually use temporary paths and cleanup.
- Confirm the newly registered metadata test and full suite return exit code 0 locally. Add an explicit PPQ assertion to the metadata test for self-contained coverage (PPQ is already checked in the separate output round-trip test).
- The CLI currently writes to hardcoded `Vocal_.mid`; support a user-provided output path and safer file-naming later.
- The exporter deliberately combines note events into one track and does **not** yet perform voice-lane assignment or lyric alignment. VOCALOID6 import must be evaluated separately; preserving tempo metadata alone does not prove all editor behaviors.
- Some edge cases (same-pitch/same-channel overlap, zero-duration ordering, metadata conflicts, invalid pitch/velocity/channel values, incomplete-source notes) need broader fixtures and/or clearer handling when encountered.
- Current selection remains a deliberately crude high-recall pitch threshold, not a reliable vocal isolator; the measured false-positive burden motivates the next selection improvements.

## Next steps
1. Run the full registered suite, remove the committed test artifact and open/review a PR from `feature/midi-export` to `main` when ready.
2. Perform a first editor import of exported MIDI when the personal music workstation is available.
3. Continue challenge-based improvements against additional reference MIDI pairs, while retaining the Day 8/9 baseline for comparison.
