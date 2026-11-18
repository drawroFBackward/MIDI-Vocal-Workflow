# Day 11 — Per-Note Classification, Time-Window Diagnostics & False-Positive Distribution

**Dates:** 2026-11-16 to 2026-11-18 (JST)  
**Status:** Completed — all four parts passed; user confirmed local JUCE tests and executed the six-song CLI benchmark  
**Branch:** `feature/tp-fp-fn-analysis` (not yet merged into `main` as of this entry)  
**Baseline selector:** Retain notes with pitch ≥60; intentionally unchanged throughout Day 11.

## Objective

Move beyond one overall TP/FP/FN score per song to identify **when** the pitch-only selector produces false positives. In particular, investigate the hypothesis that the high-register piano introduction and instrumental passages in **千本桜** explain much of its low precision. Diagnose errors using reference labels only for **evaluation**, not as information available to the deployed selector.

## Work completed — four parts passed

1. **Note-level TP/FP/FN classification.** Introduced `SongResult` storing original-note indices for TP and FP and reference-note indices for FN. `runEvaluation()` performs one-to-one pitch/onset matching; existing numerical evaluation consumes its counts, avoiding divergent classification implementations. Fixed the source-vector mix-up for FN indices. Added eight synthetic JUCE regression scenarios.
2. **Twenty normalized time windows.** Implemented `runSongWindowAnalysis()` as an in-memory, testable function. The user chose a fixed **count** of equal-relative-time windows rather than a fixed duration so different songs can be viewed on a comparable 0–100% timeline. A window is assigned by **note onset**; the final window includes an onset at the song endpoint. Window counts must reconcile exactly with overall classification through `windowCountsMatch()`. The analysis rejects invalid window counts, nonpositive/nonfinite song lengths, invalid window size, and classified onset positions outside the timeline. Uses the later of the original and reference MIDI endpoints. Window assignment was reduced from rescanning notes per window, roughly O(W×N), to direct assignment, roughly O(W+N). Seven synthetic JUCE regression cases were added.
3. **Reference-onset category summaries.** Windows are classified as **reference-active** when `TP + FN > 0`, otherwise **no-reference-onset**; an FN-only window is reference-active. The derived `isVocal()` accessor avoids cached, mutable duplicate state. Introduced `SongWindowSummary` and `summarizeSongWindows()` with window and FP totals for each category, plus percentages, averages and console printing. Fixed the category-selection condition and the no-FP reporting case (both FP shares display N/A when denominator is zero). Five regression cases were added. The assistant implemented requested mechanical validation, refactor and test changes; the user owned the design and reviewed the commits.
4. **Real-song benchmark integration.** Extended `--dir` to report labeled 20-window diagnostics for **all six paired songs**, after validating matching MIDI PPQ/time formats. Files are loaded and notes extracted once per song, and the original benchmark precision/recall/F1 report remains enabled. User executed the Windows program and supplied full per-window output. All window TP/FP/FN totals matched the corresponding baseline results; **no selector behavior was altered**.

**Tests:** Day 11 introduced 8 classification + 7 window-counting + 5 summary regression cases, bringing the registered JUCE unit-test classes to **32**. The user reported local tests passing; the assistant reviewed GitHub code/test registration but did not run the Windows executable independently.

## Dataset and six-song benchmark

Two additional manually paired songs were added since Day 10: **からくりピエロ** and **アイのシナリオ**. The repository also contains `Original_アイドル.mid` without a paired modified benchmark; it is explicitly skipped and reported. Song labels are based on filenames in the private reference-data repository, and numerical results below come from the user's CLI output. Precision, recall and F1 were computed from summed window TP/FP/FN counts.

| Song | TP | FP | FN | Precision | Recall | F1 |
|---|---:|---:|---:|---:|---:|---:|
| This Game | 976 | 845 | 0 | 53.60% | 100.00% | 69.79% |
| からくりピエロ | 413 | 1,508 | 2 | 21.50% | 99.52% | 35.36% |
| アイのシナリオ | 512 | 801 | 0 | 38.99% | 100.00% | 56.11% |
| メルト | 777 | 297 | 40 | 72.35% | 95.10% | 82.18% |
| 千本桜 | 527 | 969 | 0 | 35.23% | 100.00% | 52.10% |
| 空に免じて | 610 | 467 | 22 | 56.64% | 96.52% | 71.39% |
| **Total** | **3,815** | **4,887** | **64** | — | — | — |

**Six-song micro:** precision **43.84%**, recall **98.35%**, F1 **60.65%**. This is a **new dataset baseline**, not a regression in the algorithm versus Day 10's four-song benchmark.

## Where the false positives occur

Each song was divided into **20 equal portions of its MIDI tick timeline**. The category criterion uses the onset of *reference* notes, not audio or continuous note sustain.

| Song | Reference-active windows | No-reference-onset windows | FP in active | FP in no-onset | Share of song FP in no-onset windows |
|---|---:|---:|---:|---:|---:|
| This Game | 16 | 4 | 508 | 337 | 39.88% |
| からくりピエロ | 14 | 6 | 942 | 566 | 37.53% |
| アイのシナリオ | 17 | 3 | 588 | 213 | 26.59% |
| メルト | 20 | 0 | 297 | 0 | 0.00% |
| 千本桜 | 14 | 6 | 442 | 527 | 54.39% |
| 空に免じて | 18 | 2 | 373 | 94 | 20.13% |
| **Total** | **99** | **21** | **3,150** | **1,737** | **35.54% of all FPs** |

**Cross-song observation:** No-reference-onset windows represent **21 / 120 = 17.5%** of all examined windows yet contain **1,737 / 4,887 = 35.5%** of FPs. Their pooled average is **82.71 FP/window**, versus **31.82 FP/window** in reference-active windows (approximately **2.60×**). For each of the five songs having both window categories, average FP/window is greater in the no-reference-onset category. This is descriptive, **not an inference of causality**, and is sensitive to candidate-note density.

**千本桜-specific observation:** Introduction windows 0–1 contain **131 FPs** and no reference note onsets, but middle windows 12–13 contain **234 FPs**, a larger concentration. Window 7 contains **94 FPs** despite having reference onsets. Thus the hypothesis that instrumental-type regions contribute heavily is supported, but **the introduction is not the only or largest error region**.

**New hardest reference:** `からくりピエロ` has **1,508 FP** and **21.50% precision**, with **942 FP in reference-active windows**. Fixing no-onset regions alone cannot solve the broader selection problem. `メルト` has a reference onset in every coarse window; this does *not* establish the absence of shorter instrumental gaps.

## Engineering / C++ takeaways

- **Indexes need an explicit source invariant:** TP/FP refer to original notes; FN refers to reference notes. Keep note-level labels rather than only aggregate counts.
- **Pure analysis functions simplify testing:** MIDI I/O/PPQ checks belong at the data-loading boundary, while windows and summaries operate on note vectors and tick positions.
- **Protect partition invariants:** Sum of each category across windows must equal its overall classification count. Final-endpoint handling and floating-point underflow guards prevent silent omissions or unsafe vector access.
- **Lambdas and captures:** A local `findWindow` lambda captures the song/window context by reference; vector indexing is clamped to the last valid index.
- **Derived vs stored state:** `isVocal()` derives reference-onset activity from TP/FN rather than storing a potentially stale flag.
- **Separate reporting from calculation:** A reusable summary structure lets later comparisons use numerical results without parsing printed text.
- **Selective delegation:** The user implemented the core diagnostic logic, while the assistant completed low-learning-value mechanical changes and JUCE regression tests when explicitly requested.

## Methodological caveats and deferred risks

1. **Evaluation-only reference information.** Whether a window contains reference vocal-note onsets cannot be used by a deployable selector without an independent activity detector; otherwise it is label leakage.
2. **Not proof of instrumental-only audio.** A no-reference-onset window may contain sustained vocals from a preceding window. A reference-active window may still contain instrumental breaks. Activity via note-duration overlap and finer windows would yield a more precise diagnostic.
3. **FP/window is not a normalized error rate.** Different songs/windows can have different amounts of source polyphony or candidate notes. A future comparison could normalize FP by number of candidates and report onset/overlap conditions.
4. **Reference validity and missing notes.** A reference note not present in the source is impossible for a selection-only algorithm to choose. Distinguish source-selection performance from eventual complete vocal reconstruction; flag dataset misalignment or suspicious reference extensions rather than assume every FN is recoverable.
5. **Integration duplication.** `main.cpp` now repeats much of the loading/extraction logic of `runBenchmark()`. It avoids repeat file reads in the CLI but is a candidate for shared pipeline extraction in a later cleanup.
6. **Reference data/console hygiene.** Preserve the original/normalized 千本桜 MIDI parsing provenance noted on Day 10. Windows console output still sometimes mojibakes Japanese filenames; this is a display issue until proven otherwise.
7. **Scope remains focused.** No vocal-note generator, instrument/vocal-activity detector, voice-lane grouping, lyric alignment or VPR export has yet been validated.

## Day review and next direction

**Day 11 accomplished its diagnostic objective:** The pitch ≥60 selector suffers notably elevated false-positive counts in windows with no reference-note onsets, but a substantial number of false positives occur during reference-active windows too. Future experiments should test candidate-only (non-reference-leaking) techniques for detecting vocal activity and/or distinguishing melody from accompaniment, with results scored against this six-song benchmark while preserving high recall. No new algorithm or threshold change was introduced during Day 11.

**Next lesson:** Begin Day 12 only when requested; first present its overall objective and part count, then progress one coding challenge at a time. A structured weekly review follows on study Day 14.
