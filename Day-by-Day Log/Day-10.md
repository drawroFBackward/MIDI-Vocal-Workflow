# Day 10 — Multi-Song Benchmarking, Aggregation & Evaluation Integrity

**Dates:** 2026-11-10 to 2026-11-12 (JST)  
**Status:** Completed — all four parts passed, local tests reported passing, feature merged  
**Branch:** `feature/benchmarking`  
**Integration:** [PR #3](https://github.com/drawroFBackward/MIDI-Vocal-Workflow/pull/3), squash-merged into `main` (commit `34d5fa9c9489ecd47684000b4d820de8156d4486`).

## Objective
Extend the one-song pitch-based MIDI evaluator into a reliable, repeatable multi-song benchmark, with per-song results, aggregate metrics, directory-based dataset discovery and a PPQ compatibility guard.

## Work completed
**All four parts PASSED:**
1. **Reusable single-song evaluation:** Defined a self-contained `BenchmarkResult` (song identifier, TP, FP, FN, precision, recall and F1), returned by stateless `evaluate()`. Updated the CLI to use the reusable evaluation pipeline. Tests cover known TP/FP/FN expectations and the nonempty-reference/empty-selection case. Zero-denominator convention: metric = 0.
2. **Automatic multi-song discovery:** Built `FilePair`, `findOriginalMidiFiles()`, `pairFiles()` and `runBenchmark()`. The `--dir <dataset>` command discovers exact `Original_<name>.mid` / `Modified_<name>.mid` pairs and benchmarks each without exporting MIDI. Retained a separate one-original-file export mode. Invalid directories, missing originals, and directories with no complete pairs fail instead of silently succeeding; unmatched files are reported. Added directory and Unicode filename tests. The previous two-file CLI invocation was intentionally retired.
3. **Aggregate metrics:** Added `BenchmarkReport` with `getMicroAverageResult()`, `getMacroAverageResult()`, and `printReport()`. Micro averages combine TP/FP/FN totals before deriving metrics; macro averages arithmetic-mean each song's precision, recall and **per-song F1**. Empty aggregation reports throw by design, while printing an empty report is safe. Synthetic regression tests cover normal cases, zero values, empty reports and unchanged per-song results. Routine tests were authored by the assistant and reviewed/executed by the user.
4. **Benchmark integrity:** Added a PPQ/time-format mismatch check in `runBenchmark()` before note extraction and matching. Two synthetic file-based tests verify equal PPQ evaluation and differing PPQ rejection with identifying filenames. User reports passing local tests and unchanged real-song aggregate results.

## Dataset & baseline

**Four complete manually cleaned reference pairs:** This Game, メルト, 千本桜, 空に免じて; `Original_アイドル.mid` is unmatched and explicitly reported. The pitch ≥60 rule was held fixed across all songs.

| Song | TP | FP | FN | Precision | Recall | F1 |
|---|---:|---:|---:|---:|---:|---:|
| This Game | 976 | 845 | 0 | 53.60% | 100.00% | 69.79% |
| メルト | 777 | 297 | 40 | 72.35% | 95.10% | 82.18% |
| 千本桜 | 527 | 969 | 0 | 35.23% | 100.00% | 52.10% |
| 空に免じて | 610 | 467 | 22 | 56.64% | 96.52% | 71.39% |
| **Total** | **2,890** | **2,578** | **62** | — | — | — |

**Aggregate:** micro precision 52.85%, recall 97.90%, F1 **68.65%**; macro precision 54.45%, recall 97.91%, F1 **68.86%**.

The user identified **千本桜** as the hardest arrangement under this selector, hypothesizing that the high-pitched piano introduction and repeated accompaniment passages drive its 969 false positives. This is a useful **hypothesis**, not yet a note-by-note verified explanation.

**Verification:** Developer confirms local Windows tests pass; 29 JUCE test classes registered. Earlier terminal output demonstrates all four benchmark results and matching aggregates. GitHub did not report automated CI workflows. The assistant independently reviewed source/test registration and cross-checked the numeric totals, but did not run the Windows binary.

## C++ / engineering takeaways
- A result-returning, stateless evaluator reduces duplicated state and allows console, tests and future reports to consume the same measurements.
- Pair related source/reference paths in one struct instead of two parallel vectors; enforce the invariant that a successful benchmark processes at least one complete pair.
- Use clear CLI branching and failure codes, not silent partial success. Preserve Unicode through JUCE file/path types.
- **Micro vs. macro:** sum counts for note-weighted micro metrics; mean per-song metrics for equal-weight macro. Macro-F1 is not F1 of macro-precision and macro-recall.
- Meaningful tests isolate behavior through synthetic fixtures and hand-derived expected values. New test-writing delegation policy: user owns test design and review, assistant may implement repeatable test code; new testing techniques remain learning opportunities.
- File-based PPQ regression tests guard benchmark validity at the I/O boundary. Matching tick values across different PPQs is invalid without normalization.
- Safe `std::cout` capture uses RAII to restore stream buffers even when exceptions occur.

## Risks, deferred improvements & next actions
- **Data reproducibility:** The private-repository `Original_千本桜.mid` contained `XFIH`/`XFKM` trailing extension chunks that JUCE rejected. The user normalized the local copy to complete the benchmark; record/provide a repeatable normalization for a fresh dataset checkout, and preserve the original as a backup.
- **Japanese filename display:** Windows terminal sometimes prints Japanese characters incorrectly even while Unicode file matching and benchmark evaluation work. Treat as display diagnosis, not proven path corruption.
- **Exact-onset evaluation limits:** Equal PPQ is necessary but not sufficient to ensure alignment if arrangements start at different offsets, use tempo-map differences, or contain alternate note timing/quantization. Current match rule ignores duration and track as deliberately defined by the baseline.
- **Algorithm quality:** High recall (97.90% micro) coexists with low precision (52.85% micro). Investigate false-positive concentration in piano/high-register accompaniment; compare any new selector to the unchanged four-song baseline.
- **Workflow:** Current CLI supports one-file export and `--dir` benchmarking; sorting, prettier song names, progress/error summaries, configurable output paths and CI automation can follow when valuable.
- **Project scope:** No lyrics, voice-lane assignment or VOCALOID6 editor import is yet validated.

## Next step
Conduct the Day 10 retention/review discussion, then start the next challenge-based day only when requested. New approaches should be benchmarked on all four pairs and should not silently sacrifice high recall for improved precision.
