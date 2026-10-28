# Day 07 — Week 1 Review

**Date:** 2026-10-28 (JST)  
**Status:** Approved — 28 October 2026  
**Format:** First day-by-day one-pager; Days 1–6 are not backfilled.

## Objective
Assess retention of MIDI, C++, JUCE note extraction, testing and architectural concepts from study Days 1–6; identify what to reinforce before implementing vocal-note selection.

## Work completed
**All five review parts PASSED**, including corrections after feedback:

1. **MIDI:** Reconstructed three notes across two channels, including an incomplete note; corrected PPQ/time conversion: 240 ticks at 480 PPQ and 120 BPM = **0.25 s**.
2. **C++ and algorithms:** Explained references and `break`; established **O(MN)** worst-case note matching; identified ambiguous same-pitch/same-channel overlap.
3. **Testing/invariants:** Found unsafe `vector[]` access and weak channel assertions; distinguished incomplete notes from valid zero-duration notes; rejected negative durations; recognized limited test coverage.
4. **Architecture:** Separated parsing, selection, voice assignment and lyric alignment; identified risks of copied state and invalidated vector pointers; rejected exclusive reliance on highest pitch or chord roots.
5. **Project application:** Computed **TP=20, FP=10, FN=15**: precision **66.7%**, recall **57.1%**, F1 **61.5%**. Proposed arpeggio-overlap, additional-harmony and varied-repeated-chorus test scenarios; prioritized a measurable rule-based baseline.

## Evidence
Five interactive review challenges completed; answers and corrections recorded in the conversation. **No code changed, no new tests were run, and no GitHub commits were made during Day 7.** The five existing unit-test scenarios remain the previously reported test baseline. Proposed selection tests still require implementation and exact expected-note assertions.

## Concepts to reinforce
- Tempo/tick calculations and nonconstant tempo maps.
- Algorithmic complexity, reference semantics, vector element lifetime.
- Data invariants: incomplete versus invalid, including zero-duration completion.
- Compute metrics from **unrounded values** (F1 = 40/65 = 0.61538, or 61.5%).
- Express musical edge cases as deterministic test fixtures with precise expected notes.

## Important decisions and reasons
**<u>Continue with a simple, measured vocal-note selection baseline before advanced ML or polished UI.</u>**  
**Why:** Establish real behavior and failure cases before investing in complexity; consistent with the approved roadmap.

**<u>Preserve source notes; treat high recall as a provisional initial preference.</u>**  
**Why:** Recovering accidentally omitted harmonies may cost more than removing false positives. Validate against actual editing time; no irreversible deletion.

**<u>Defer redesign for ambiguous overlapping Note On/Off pairing.</u>**  
**Why:** The current matching policy is deterministic but cannot infer intended pairing from ambiguous MIDI alone. Document the limitation and test it when warranted by real data.

## Next steps
1. Begin Day 8 with a challenge-based, testable vocal-selection baseline (objective and part count first).
2. Apply the identified learning gaps to code reviews.
3. Assess additional original/cleaned MIDI pairs when provided.