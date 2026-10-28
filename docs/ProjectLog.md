# MIDI Vocal Workflow — Project Log

**Version:** 0.3 — Approved for publication  
**Prepared:** 28 October 2026 (JST)  
**Approval:** Approved for publication by user on 28 October 2026 (JST)  
**Cadence:** One-page daily Markdown logs from study Day 7; weekly review every 7 study days; comprehensive delivery review every 8 calendar weeks.  
**Canonical document:** `docs/ProjectLog.md` in GitHub; generate `ProjectLog.pdf` from the approved master.

> **Purpose:** Preserve evidence, engineering reasoning, and decisions for a 40–60-week portfolio project reducing manual preparation of virtual-singer performances. This version was approved by the user on 28 October 2026.


## Executive summary / TL;DR

**Product vision.** Build one modular workflow that starts with **jumbled MIDI -> identified vocal notes -> separate singing-voice lanes -> lyric-aware refinement -> usable MIDI/VPR**. Later, add audio-to-MIDI via existing tools (Project 1.5) and then our own audio/ML research (Project 2). The objective is to **reduce manual producer work**, not claim perfect automation.

**What the first files establish.** `Original_空に免じて.mid` contains **1,935 notes**. The manually cleaned vocal reference, `modified.mid`, contains **632 notes** (a **67.3% reduction**). All retained notes have an original matching pitch/onset; 630 also match duration exactly.

**Critical clarification.** The reference deliberately retains simultaneous notes that may represent **different vocal performers/layers**. They must not be indiscriminately collapsed into a single monophonic melody. Each *individual* vocal lane should eventually be monophonic, but the total vocal arrangement can be polyphonic. The present reference combines those notes in one MIDI note-bearing track; it does **not** label which note belongs to which vocal lane.

**Baseline / caution.** Keeping all notes from the source's upper-register track reaches **77.7% pitch/onset F1** against this reference, but this is a *single-song selection baseline*, not evidence of robust vocal separation. Lane assignment cannot yet be scored without separate part annotations.

**Pipeline and timing.** Initial stage: identify and preserve plausible vocal notes (including harmonies). Later: group notes into separate vocal lanes, align supplied lyrics to phrases/morae, and make targeted edits (e.g., splitting a note when a Japanese `ん` needs separate timing). Splitting is **context-dependent**, not a blanket rule for every `ん`. Keep MIDI export and measurable reduction of editing work as the **first ~8-week POC**, with a small VPR interoperability investigation early. Plan around **~11 hours/week**, milestone demonstrations every **6–10 calendar weeks**, and a flexible 40–60-week portfolio horizon. Winter Comiket is optional.

**Current status.** Study Days 1–6 complete; Day 7 weekly assessment still pending. More paired examples are expected over the weekend, **but development continues meanwhile**. Five named engineering decisions have been approved, and this version is **approved for publication**. Day-specific notes will be kept in `Day-by-Day Log/` beginning with Day 7, not accumulated here.

---

## 1. Project charter and constraints

**North star:** Convert MIDI, lyrics, and ultimately audio/recordings into editable singing-synthesis project material, initially for VOCALOID6 and VPR. The core user outcome is less manual work, **not guaranteed perfect automation**. The stage-one extractor must retain plausible notes for **lead and additional vocal/harmony lanes**, not assume one monophonic output for the full arrangement.

**Inputs and workstreams:**

- **Project 1 — MIDI + lyrics -> VPR:** Extract likely vocal notes from difficult polyphonic/mixed MIDI (including harmonies), then infer **separate, individually monophonic vocal lanes**, align lyrics and make timing/note segmentation edits, and export an editable singing-synthesis project.
- **Project 1.5 — audio -> third-party transcription -> Project 1:** Integrate an external audio-to-MIDI engine first; do not reinvent transcription prematurely.
- **Project 2 — audio -> own analysis/refinement -> VPR:** Explore signal processing, then learned models and/or fine-tuning; share the same data and output core as Project 1.

**Technical starting point:** Windows, C++17, JUCE, CMake, GitHub, Cubase, VOCALOID6. A console MIDI reader/extractor and five JUCE unit-test scenarios have been implemented; the `feature/unit-tests` branch was merged into `main` after the user confirmed successful build and test execution.

**Time:** Around **11 development hours per calendar week** (1 hour on each weekday and around 3 hours per weekend day), or roughly **88 hours in an eight-week milestone**. These are planning estimates, not guaranteed capacity.

**Target cadence:** Useful and demonstrable output every 6–10 calendar weeks. Winter Comiket in late December 2026 is an *optional* feedback opportunity, not a non-negotiable launch date. Long-range plan: approximately 40–60 calendar weeks. Day numbers for lessons are separate from calendar weeks.

## 2. Initial source files and evaluation method

**Evaluated pair:** `Original_空に免じて.mid` (original arrangement) and **`modified.mid`** (user's manually selected vocal-note reference). The analysis uses MIDI event parsing, pairing Note On with Note Off (including velocity-zero Note On), and note tuples represented as pitch, start tick, and duration. The measured values refer to this pair; no external files were used.

**Important evaluation assumptions:**

1. The manually filtered file is a *reference*, not perfect ground truth. **Simultaneous retained notes are intentionally allowed** because they can belong to separate singing voices (lead, harmony, doubles); a few erroneous inclusions or omissions may still exist. Do **not** classify polyphony itself as a labeling error.
2. Matching by `(pitch, start tick)` is useful for this first paired comparison because both files use the same 480 ticks-per-quarter resolution and aligned event positions. It does **not** prove that a note is a vocal note, and it ignores duration differences unless separately noted.
3. The first pair is one song, and its original already contains a strong higher-register track. Its baseline scores **must not** be taken as general performance on arbitrary mixed MIDI. Voice-lane identity is *unlabeled* in the reference and cannot be evaluated from pitch/onset membership alone.
4. A note with duration zero and a missing Note Off are different states; completion information must be retained in the data model.

### 2.1 File-level observations

| Attribute | Original | Manually filtered reference |
|---|---:|---:|
| MIDI type | 1 | 1 |
| Resolution | 480 PPQ | 480 PPQ |
| Tracks in file | 2 | 2 (first is metadata-only; all 632 notes in one track) |
| Matched note instances | 1,935 | 632 |
| Note pitch span | 24–95 | 56–90 |
| Median pitch | 62 | 68 |
| Maximum simultaneous sounding notes | 7 | 4 |
| Fraction of sounding time with overlapping notes | 95.4% | 42.1% |
| Simultaneous-onset note instances (groups of 2+) | 1,468 | 359 |
| Unmatched Note On / Note Off events | 0 / 0 | 0 / 0 |

**Comparison:** The manual edit retained **632 / 1,935 = 32.7%** of original note instances, a **67.3% reduction**. All **632** reference notes match at least one original note at exactly the same pitch and onset tick. **630** also match duration exactly. The **two duration changes** are: tick 38,640, pitch 60, original 719 ticks vs reference 240; and tick 262,080, pitch 62, original 959 ticks vs reference 1,439. Velocity patterns are broadly retained, and the tempo events and time-signature events correspond across files.

**Where retained notes came from:** The original has two tracks, both labeled `Piano`, containing **959** and **976** note instances respectively. Of the 632 reference notes, **618** occur in original track 0 and **14** in original track 1 (using exact pitch/onset matches). There are four duplicated pitch/onset note keys across original note instances, so unique-key-based baseline counts can differ slightly from the 1,935 event-pair count.

**Vocal-layer interpretation (confirmed by user):** The reference is **intentionally polyphonic**: 359 of its 632 notes share their onset with at least one other note, and up to **four notes sound concurrently**. The user intends simultaneous notes to be sung by **separate vocal tracks**, because each individual singing track is treated as one note at a time. Thus, a four-way overlap implies **at least four monophonic lanes at that instant**, if every retained note is valid and overlaps are interpreted literally; it does *not* tell us how to assign the rest of the song to those lanes. The cleaned MIDI currently puts all selected notes in a single note-bearing track: **note selection is done; lane assignment has not been encoded in this reference**. Some residual edits may still be needed.

### 2.2 Measured simple baselines (this pair only)

The following metrics compare **unique (pitch, onset) keys** from a proposed output against the user's reference: `precision = matched proposed keys / all proposed keys`; `recall = matched reference keys / all reference keys`; F1 is their harmonic mean. These scores assess whether the **set of vocal notes** was selected, *including legitimate simultaneous notes*. They do not account for duration correctness, **which note belongs to which vocal track**, vocal perceptual quality, or actual editing time. Future voice separation requires a separate reference or manual listening assessment.

| Baseline selection | Output note keys | Precision | Recall | F1 |
|---|---:|---:|---:|---:|
| Keep every original note | 1,931 | 32.7% | 100.0% | 49.3% |
| Keep original track 0 | 959 | 64.4% | 97.8% | **77.7%** |
| Keep original track 1 | 976 | 1.4% | 2.2% | 1.7% |
| Highest pitch per simultaneous onset, both tracks | 931 | 46.2% | 68.0% | 55.0% |
| Every original note with pitch >= 60 | 1,077 | 56.6% | 96.5% | 71.4% |

**Interpretation:** Picking the higher-register track is a powerful shortcut for *this* song. It retains 618 of 632 reference notes but leaves 341 additional track-0 notes to remove. A naive highest-pitch-only strategy loses too many reference notes. These numbers justify building and measuring a stronger intra-track note selector, but they also show why we need other difficult examples before concluding that our algorithm handles “jumbled MIDI.”

**Not a fair benchmark:** If we trim the higher-register track using the first onset in the *reference* (known only after manual cleanup), the F1 increases to about 81.5%. This uses reference information and must not be advertised as an independently deployable method.

## 3. Interpretation and engineering consequences

**Finding F-01 — The first problem is note selection, not transcription.** Of 632 manually retained notes, all preserve original pitch/onset, and almost all preserve duration. The early algorithm can initially score and keep/drop existing notes rather than trying to synthesize new pitches or timings.

**Finding F-02 — Track selection is useful but inadequate.** Most reference notes originate in the original's higher-register track, yet 14 come from the other track, and hundreds of notes in the high track were discarded. We need mixed-track and intra-track selection, not merely “choose the vocal track.”

**Finding F-03 — Support multiple distinct monophonic vocal lanes.** The user clarified that simultaneous notes in the cleaned reference can represent different vocal tracks, **not one singer singing a chord**. Preserve polyphonic vocal content at the arrangement level; ultimately partition it into separate individually monophonic lanes. Overlap alone is not a reason to reject a note. Current reference labels note inclusion, **not lane assignment**.

**Finding F-04 — Baselines must be established before complex algorithms.** Track choice, pitch cutoff, and highest-note heuristics provide a meaningful comparison for deterministic graph/path methods and eventual ML candidates.

**Finding F-05 — This is an imperfect-label problem.** Review uncertain note segments and use measured editing-time savings alongside note-level F1; do not equate reference mismatches with musical errors automatically. Simultaneous notes are not, by themselves, errors.

**Finding F-06 — Separate selection, voice assignment, and lyric-guided editing.** First select the plausible vocal *note set*; next attribute notes to distinct singing parts; then align lyric units to phrases and adjust durations, boundaries, or split notes only where needed. A single raw MIDI note may later map to multiple sung morae, and a single sung syllable may span multiple notes.

**Finding F-07 — Preserve uncertainty and annotations.** A single reference file with mixed parts cannot establish whether a note is lead or harmony. Our system should preserve source-note IDs and allow manual labeling/correction, so later evaluations and ML targets are meaningful.

## 4. Decision register — dated and approval-gated

**Formatting rule:** Every major decision is displayed in **bold and underline**, followed by its rationale, status, and approval date where applicable. New daily, weekly, and eight-week updates remain drafts until reviewed and approved by the user. Git provides document change history; a redundant in-document revision list is deliberately omitted.

**<u>D-001 — Build one modular vocal-preparation system, not two independent apps.</u>**  
**Status:** AGREED IN DISCUSSION.  
**Why:** MIDI and audio pathways can share a canonical internal note/phrase representation, lyric alignment, and exporters. This reduces duplication and supports a more coherent portfolio.

**<u>D-002 — Focus the initial eight-week proof of concept on extracting candidate vocal notes from difficult polyphonic MIDI.</u>**  
**Status:** AGREED IN DISCUSSION.  
**Why:** This is the highest-value immediate user problem; a focused MIDI export offers a demonstrable result for a potential Winter Comiket conversation. VPR and GUI polish must not block the first useful milestone.

**<u>D-003 — Treat manually cleaned MIDI as imperfect reference data, not unquestionable ground truth.</u>**  
**Status:** AGREED IN DISCUSSION.  
**Why:** The objective is to reduce manual cleanup rather than promise perfect transcription; label errors and polyphony are expected.

**<u>D-004 — Preserve original notes and distinguish retained, rejected, and uncertain candidates.</u>**  
**Status:** APPROVED by user — 28 October 2026.  
**Why:** It enables reversibility, candidate comparison, annotation, debugging, and downstream ML without losing evidence.

**<u>D-005 — Preserve intentional vocal polyphony, while supporting one-note-at-a-time within each future vocal lane.</u>**  
**Status:** AGREED IN DISCUSSION; implementation details pending.  
**Why:** The user specifies that double/triple retained notes can belong to different vocal parts. The current reference stores them together; later separation into individually monophonic lanes is necessary for singing-synthesis editing. Avoid discarding harmonies simply because notes overlap.

**<u>D-006 — Measure each milestone using note-selection metrics and actual manual correction time.</u>**  
**Status:** AGREED IN DISCUSSION.  
**Why:** A model can have impressive note accuracy without improving the producer's workflow. Speed and remaining error severity matter.

**<u>D-007 — Require note-selection algorithms to be compared against the 77.7% F1 upper-track baseline on this pair, and generalize on additional songs; evaluate voice assignment separately.</u>**  
**Status:** APPROVED by user — 28 October 2026.  
**Why:** Avoid building complex algorithms that perform worse than a simple rule. A one-song score is diagnostic, not a claim of generalization. Since this pair has no per-voice labels, the note-set score cannot verify that lead/harmony notes were assigned to the right virtual singers.

**<u>D-008 — Run a VPR compatibility feasibility spike early, but keep it off the initial POC critical path.</u>**  
**Status:** AGREED IN DISCUSSION.  
**Why:** VOCALOID interoperability is a central product goal; testing a small real import early reduces a potentially expensive format risk.

**<u>D-009 — Document major decisions and rationale, using distinct daily one-pagers and periodic strategic reviews.</u>**  
**Status:** AGREED IN DISCUSSION.  
**Why:** Daily one-pagers make progress easy to browse without bloating the main log; the main log retains strategic decisions, measurements, and weekly/eight-week summaries. Git captures revision history.

**<u>D-010 — Use deterministic candidates as the measured baseline; introduce ML when real data and a controlled evaluation can demonstrate improvement.</u>**  
**Status:** AGREED IN DIRECTION; implementation specifics TBD.  
**Why:** Maintains interpretability, provides learning value in algorithms, and creates a research comparison that is relevant to AI engineering.

**<u>D-011 — Maintain `docs/ProjectLog.md` as the editable master; create `ProjectLog.pdf` as a derived review snapshot, with publication approval required.</u>**  
**Status:** APPROVED by user — 28 October 2026.  
**Why:** Git commits already record exact changes and rationale over time. Markdown is straightforward to review and diff, while PDF serves as a stable presentation/review copy. Publication of this version was separately approved by the user on 28 October 2026.

**<u>D-012 — Use a staged MIDI-to-vocal workflow: inclusive note selection -> separate vocal lanes -> lyric-driven refinement -> project export.</u>**  
**Status:** AGREED IN DIRECTION; implementation details TBD.  
**Why:** Mixed-source MIDI can contain multiple vocal parts. Lyric timing and phonetic constraints are valuable *after* an initial extraction, rather than forcing a single melody or deleting harmonies prematurely.

**<u>D-013 — Treat Japanese mora/phoneme-specific note splitting (including possible `ん` adjustments) as a context-dependent alignment rule, not an unconditional mutation.</u>**  
**Status:** APPROVED by user — 28 October 2026.  
**Why:** The user correctly identifies `ん` as a common alignment difficulty, but the necessary edits depend on the note structure, singing engine, pronunciation and intended timing. Test against actual VOCALOID6 results before encoding a rule.

**<u>D-014 — Continue Week 2 development using the current reference while collecting additional pairs over the coming weekend.</u>**  
**Status:** AGREED IN DISCUSSION.  
**Why:** An expanding dataset improves confidence, but code for export, fixtures, metadata preservation, evaluation, and multi-voice-safe processing can progress now without waiting for additional files.

**<u>D-015 — Distinguish note-set selection quality from correctness of voice-lane assignment.</u>**  
**Status:** APPROVED by user — 28 October 2026.  
**Why:** The current reference contains intended vocal polyphony in one track. It supports measuring retained-note membership, but not assigning each note to the correct singer without new annotations.

**<u>D-016 — Keep one-page daily records in the GitHub folder `Day-by-Day Log/`, beginning with Day 7; retain weekly and eight-week rollups in the master Project Log.</u>**  
**Status:** USER-DIRECTED / APPROVED — 28 October 2026.  
**Why:** Separate daily files give a concise, navigable account of what was done, learned, tested, and decided, without making the strategic log unwieldy. No retroactive Day 1–6 files are required. Start the Day 7 one-pager only when actual Day 7 work has been completed. The folder can be created with a `README.md` on the first approved GitHub publication.  

## 5. Updated eight-week delivery plan (approximately 88 hours)

This is a **working proposal**, not a fixed feature contract. Study days and calendar weeks are tracked separately.

| Calendar window | Main deliverable | Exit check |
|---|---|---|
| Foundation completed (study Days 1–6) | JUCE MIDI reader, structured notes, five tests, merged branch | Existing unit-test target runs green locally; **study days are not calendar weeks** |
| Week 2 | Reliable MIDI writer/round trip; reproducible paired-data inspection, basic voice-safe data representation; small VPR feasibility spike | Round-trip preserves pitch/onset/duration/tempo; input polyphony not accidentally collapsed |
| Week 3 | Baselines, support for mixed vocal notes, ingest additional examples when supplied; inspect voice overlap structures | Note-selection metrics reproducible; no assumption that simultaneous notes are necessarily accompaniment |
| Week 4 | First deterministic vocal-*note-set* selection algorithm, including potential harmonies | Export candidate note set; compare against 77.7% upper-track baseline on this example |
| Week 5 | Continuity/rhythm-aware candidate ranking; retain uncertain/rejected notes and source IDs | Error analysis includes false-positive accompaniment and omitted harmony notes |
| Week 6 | Optional preliminary voice-lane separation and simple controls; ranked candidate note sets (stretch) | CLI exports vocal notes without enforcing global monophony; trial lane separation measured qualitatively |
| Week 7 | Minimal “import -> analyze -> export -> review in Cubase” demo, including retained harmonies | Usable end-to-end local run; note layers remain accessible for editing |
| Week 8 | POC packaging, before/after examples, demo recording, prospective creator feedback | Shareable Windows executable/demo and measured results |

**Scope-control rule:** The initial POC requires a usable MIDI output and benchmark, with **lead and harmony notes retained** when plausible; it does **not** require correct assignment to every vocal lane, a production GUI, complete VPR output, perfect mixed-song extraction, a complete lyric alignment engine, or trained ML. VPR feasibility remains an early technical investigation. If data collection is slower than expected, prioritize a reproducible CLI demo over speculative architecture.

**Proposed acceptance gate:** At least one reproducible sample from the supplied pair and ideally two or more distinct arrangements; all regression tests pass; exported MIDI opens in Cubase; the algorithm's **full vocal note set** is scored against simple baselines, including harmony preservation; manual correction time is measured on a representative passage. A 20–30% editing-time saving is an *aspirational experiment target*, not a promised result. Metrics should include limitations and cases where the algorithm is worse.

## 6. Medium- and long-range milestones

| Weeks | Proposed outcome | Evidence to deliver |
|---|---|---|
| 9–16 | Known vocal notes + lyrics -> VOCALOID-compatible project; begin voice-lane assignment | Minimal VPR/alternative export and multi-part arrangement validated in actual editor |
| 17–24 | Integrated mixed-MIDI selection + per-voice separation + lyric-aware note edits | Measured multi-voice workflow, Japanese mora alignment tests, including conditional `ん` splits |
| 25–32 | Audio -> third-party MIDI -> existing pipeline | Comparative experiment with external transcription tools |
| 33–40 | Producer-oriented UI and multiple melody candidates | Usability feedback, repeatable demonstration |
| 41–48 | Own audio/ML refinement experiment | Dataset protocol, training/inference, baseline comparison |
| 49–56 | Portfolio-quality integrated release | Documentation, tests/CI, benchmarks, demo recordings |
| 57–60 | Optional reserve for research, polish and creator feedback | Release or revised milestone based on evidence |

**Architectural direction:** C++/JUCE core; clean separation of MIDI parsing, note representation, **vocal-note-set selection**, **voice-lane assignment**, **lyric/mora alignment and note splitting**, exporters, and user interface. Python/PyTorch may be used as a distinct research component when the data and evaluation method justify it. Reuse candidate notes and confidence metadata across MIDI- and audio-derived inputs. Preserve original sources and never silently destroy candidate evidence.

## 7. Risk register and experiments

| Risk | Evidence / reason | Mitigation / next experiment |
|---|---|---|
| One sample too easy or unrepresentative | Track 0 alone gives 77.7% F1 on this song | Add more songs with truly mixed notes and different arrangements |
| Imperfect reference labels | Manual selection may have residual errors; polyphony is often intentional | Mark genuinely ambiguous phrases, not all chords; review representative sections by ear |
| Vocal-vs-accompaniment intrinsically ambiguous | Instrument and vocal parts share pitch/timing space, including simultaneous notes | Rank note sets without suppressing harmonies by default; allow user overrides |
| Vocal-lane labels missing | Reference combines all 632 selected vocal notes on one note-bearing MIDI track | Separate track assignment as a second task; test on annotated phrases or have user assign representative voice parts |
| Premature lyric splitting | Japanese `ん` and extended vowels depend on pronunciation and phrase timing | Test rules against VOCALOID6 and a manually labeled example before automatic edits |
| MIDI event ambiguity | Same pitch/channel overlaps, note-off pairing, tempo maps | Add tests for overlapping same-pitch and multitrack event timing |
| VPR ecosystem dependency | Format compatibility and editor version behavior may differ | Early five-note import/export smoke test in VOCALOID6 |
| Schedule overrun | ~11 h/week over a large scope | Protect 8-week output, defer UI/ML until evidence justifies them |
| ML data leakage/overfitting | Repeated sections of the same song are correlated | Split evaluations by song, record train/test data provenance |
| Copyright and sharing | Real songs and arrangements may not be redistributable | Keep private evaluation files separate from public original/licensed demos |

## 8. Documentation workflow and review cadence

**Source of truth and repository locations:**

| Repository artifact | Role | Update timing |
|---|---|---|
| `docs/ProjectLog.md` | Project charter, evidence, benchmarks, roadmap, bold-and-underlined decision register, and periodic consolidated summaries | After the user approves a proposed edit |
| `Day-by-Day Log/README.md` | Naming convention, one-page template and navigation for the daily logs | Created with the initial approved publication |
| `Day-by-Day Log/Day-07.md`, then `Day-08.md`, ... | One-page record per study day; start on Day 7, with no backfill for Days 1–6 | After each study day, pending user approval |
| `ProjectLog.pdf` | Readable export generated from the approved Markdown master | After approval and major documentation updates |

**Daily one-page template:** Date and study-day number; objective; completed challenges/commits; evidence of tests/results; technical concepts and blockers; **<u>important decisions</u>** with reasons; next steps; approval status. Short entries are preferable to filling a page artificially. A day is recorded when it actually happens; do not mark Day 7's assessment complete prematurely.

**Weekly summary (every seven study days):** Add a concise rollup to `docs/ProjectLog.md`, referencing the daily files where relevant: work delivered, retained C++/algorithmic understanding, weak areas, tests, decisions, and changes to the next week's lesson plan. The Day 7 one-pager may also note the weekly assessment, but the main log retains the cross-week summary.

**Milestone summary (every eight calendar weeks):** Add a comprehensive rollup to `docs/ProjectLog.md`: runnable demo/version; reproducible dataset, metrics, and baselines; manual-work reduction; significant failures; creator feedback; decisions/pivots; approved next milestone. The Git commit history replaces a duplicative revision record inside the PDF.

**Approval protocol:** Prepare updated Markdown and PDF drafts; present relevant changes; obtain user approval **before** committing them to GitHub or marking the PDF approved. Do not add copyrighted song MIDI examples to the public repository without checking permissions. The repository can be updated during the active chat, but daily entries will not be generated automatically without an active interaction or configured automation.

## 9. Open questions and near-term validation

1. **Known:** `modified.mid` is the manually cleaned vocal-note reference; `Original_空に免じて.mid` is the original arrangement. This report evaluates that pair only.
2. **Known:** Double/triple notes can represent different singing voices. **Unknown:** the correct singer/lane assignment of every retained note; we will eventually need manually tagged example passages.
3. **Known:** More reference pairs are expected over the upcoming weekend. Implementation and the Day 7 review **continue without waiting**.
4. **To validate later:** Which Japanese lyric timing/phoneme constraints—including `ん`—actually require splitting a note versus adjusting phonemes within the existing note in VOCALOID6?
5. **Documentation:** Begin the first one-page day entry **after Day 7 work**. Keep song-specific data out of public GitHub absent redistribution rights.


**Change control:** Future strategic decisions are appended to the register with dates and reasons; corrections to factual measurements should identify the corrected evidence and date. All updates are reviewed by the user before publication.
