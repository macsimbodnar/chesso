id:         S287
goal:       every statement that S039 retires or re-decides the lazy shortcut and the clamp is corrected to S039's H0: both are kept
accepts:    (1) each statement S039's revert left stale is restated for the kept shortcut and clamp, traced to the code and to S039's record (`adocs/plan_done/S039_lazy_margin_redecide.md`, record `ca578de`): DEV_MANUAL.md's two paragraphs that say S039 retires the clamp ("which S039 exists to do", "S039 retires the clamp ... the question is short-lived"), specs.md's sentence "S039 retires the shortcut and the clamp together", and the code comments "S039 re-decides this" (src/evaluation.hpp, two), "S039 re-decides it there" (src/search_params.hpp), "S039 is the step that re-decides it" (tests/test_search.cpp) and "S039 retires the clamp" (tools/tuner_trace.hpp); a grep for S039 over src/, tests/, tools/ and the documents finds no other present-tense claim; (2) comments only in src/: the commit carries `No functional change` and bench stays the parent's; (3) the fast suite green in both builds, format clean, the five prose modes clean
touches:    DEV_MANUAL.md, adocs/specs.md, src/evaluation.hpp, src/search_params.hpp, tests/test_search.cpp, tools/tuner_trace.hpp; widened 2026-10-16 by accepts (1)'s grep (As built): tests/test_search_params.cpp, tools/plan_prose_check.py, adocs/plan_todo/S101_threat_terms.md, adocs/plan_todo/S102_outposts_and_space.md, adocs/plan_todo/S121_mobility_curves.md, adocs/plan_todo/S123_passed_pawn_suite.md, adocs/plan_todo/S124_endgame_scaling_factors.md, adocs/plan_todo/S125_pawn_structure_completion.md, adocs/plan_todo/S126_full_refit_after_search.md, adocs/plan_todo/S135_unfreeze_piece_placement.md
excludes:   S122's design, which waits on the owner's decision before S121 starts (S039's accepts (4)); any change to the shortcut, the clamp or LazyEvalMargin
decisions:  DEC-171, DEC-258, DEC-274
closes:     2026-08-13_adversarial-F05
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent, findings by a second
done:       2026-10-21 11:25 -- the eight statements S039's revert left saying it retires or re-decides the lazy shortcut and the clamp are restated for its H0 (record ca578de, revert 74d0adf: both kept), and 22 more the accepts' grep found with them: DEV_MANUAL.md three (tune build, SPSA driver, --touches), tools/plan_prose_check.py's docstring, tests/test_search_params.cpp's golden_defaults note, and seventeen in eight pending step files (S101, S102, S121, S123, S124, S125, S126, S135), S101's accepts clause among them with its obligation unchanged. S122 is named only as its design waiting on the owner's amendment before S121 starts (DEC-258); S122's file untouched. Fast check: plan.md's part B paragraph restated by the coordinator in this commit; src/evaluation.hpp's margin comment no longer makes tools/eval_spread the measure of the share clamped at 184 (the tuner's load report counts it: 36503 of 10795695 rows at S262); 2026-08-13_adversarial-F05 closed; F16 re-planned to S122, F25 to the new filler S288. Comments and prose only: bench 6042933, the parent's, No functional change. Fast suite 49/49 in both builds (CLANG_FORMAT_MAJOR=22, DEC-146), format clean, the five prose modes clean, tools/gate.sh --message green. MANUAL.md checked, unchanged. Not run: Debug self-play, gate_extra.sh (no make_move, generator or search code). Filler under DEC-171. Written blind by an Opus subagent beside S082's run, findings applied by a second; gate and commit by the coordinator.

## Why this exists

S039 read H0 (`ca578de`) and was reverted; its agent listed, before the
revert commit, eight statements the revert made false again, each saying S039
retires or re-decides the shortcut and the clamp. Documentation and comments
only: no play, no reported score, no UCI surface, so filler under DEC-171,
named in every pre-registration while open.

## As built (2026-10-16, phase 1 blind, the Opus subagent)

Written blind in `/home/max/ws/chesso-s287` (branch `s287` at `df0d3ab`)
while S082's corpus run holds the machine: nothing compiled, tested or run.
Every edit is to a comment, a docstring or prose; no code line moved in
`src/`, `tests/` or `tools/`. Each restatement is traced to S039's record
(the Verdict section and the stamp: H0 at `{-5, 0}`, nElo -16.46 +/- 9.52,
record `ca578de`, revert `74d0adf`), to DEC-258 (5) for what H0 leaves (S122's
design amended by the owner before S121 starts), and to the code it describes:
the clamp is the `std::clamp` in `evaluate_expensive()` over
`evaluate_mobility_and_king_safety()`'s sum, the shortcut is
`evaluate_lazy()`'s two one-sided returns, and `LAZY_EVAL_MARGIN` is 184 in
`src/search_params.hpp`'s list. "S122's design, written for an unclamped king
safety" is traced to S122's goal line ("it is no longer clamped") and DEC-274.

### Deviations and open points, first

- **Scope widened by accepts (1)'s grep: 22 more statements** of the same
  class, restated with the eight. `touches:` names their files. Listed below.
- **Another step's accepts restated.** S101's accepts clause "(S039 retires
  the lazy bound; if S039 read H0, the bound must still hold by construction)"
  now reads "(S039 read H0 and kept the lazy bound, record `ca578de`, so the
  bound must still hold by construction)". The obligation is the H0 branch it
  already named; nothing is added or removed.
- **Dated sections edited in place.** The seven DEC-259 amendment sentences
  (2026-10-08) and the research sections' pitfalls and interactions
  (2026-09-13) carried conditionals or orders that S039's verdict resolved;
  they are restated where they stand rather than under a new dated heading,
  because the step file is the live instruction to its implementer.
- **For the coordinator, outside this step's reach.** `adocs/plan.md`'s part B
  paragraph ("Then on the machine: the lazy shortcut and the clamp retire
  together under one non-regression verdict (S039), so the corpus and every fit
  after it see the exact evaluation") is present tense and stale; plan.md is
  the coordinator's. `adocs/status.md`'s mentions are dated handover history.
  `adocs/plan_current/S082_datagen_qsearch_leaf_labels.md`'s are its own phase
  history (another active step's file). S122's file is excluded by this step.
- **Blind reflow.** `tests/test_search_params.cpp`'s edit leaves one short
  comment line ("S085 moved ten at once -- an SPSA run returns a vector,")
  rather than reflow the paragraph; every changed comment line is 80 columns
  or fewer and the macro comment lines in `src/search_params.hpp` keep their
  backslash at column 80 (counted with awk). Phase 2's format check is the
  proof.

### The eight listed sites, before and after

1. `DEV_MANUAL.md`, the truncation scan's re-take rule. Before: re-taken
   whenever the margin moves, "which S039 exists to do". After: the rule ends
   at "moves"; a sentence after DEC-169 says S039 measured retiring the
   shortcut and the clamp together and read H0, so both stay and the margin
   with them (record, revert).
2. `DEV_MANUAL.md`, the trace tuner's clamp treatment. Before: S039 retires the
   clamp and S122's king safety is not clamped, "so the question is
   short-lived". After: S039 read H0 and kept the clamp, so the treatment
   stays; S122's design, written for an unclamped king safety, waits on an
   owner decision before S121 starts (DEC-258).
3. `adocs/specs.md`, the lazy-evaluation clamp bullet. Before: S039 retires
   both under one verdict and S122 is the rebuild that needs it. After: S039
   measured that under one non-regression verdict and read H0 (record,
   revert; nElo -16.46 +/- 9.52 at `{-5, 0}`), both kept; S122, the rebuild
   that needed the retirement, waits on an owner decision amending its design
   before S121 starts (DEC-258).
4. `src/evaluation.hpp`, the margin comment below `evaluate_expensive_terms`,
   its first paragraph. Before: S039 re-decides the number from measured
   spread and should expect the moved incumbent. After: S039 measured retiring
   the shortcut and this clamp together and read H0 (record, revert): both
   kept, and the number with them.
5. The same comment, its second paragraph. Before: "S039 re-decides the margin
   from measured data, and tools/eval_spread is what measures it". After: only
   "tools/eval_spread is what measures it"; the first paragraph carries the H0.
6. `src/search_params.hpp`, the comment over `LAZY_EVAL_MARGIN`. Before: S039
   re-decides it there. After: S039 read H0 and kept it (record, revert). One
   macro line more.
7. `tests/test_search.cpp`, GOLDEN 383 in "a stand pat that is itself a bound is still capped".
   Before: "S039 is the step that re-decides it". After: S039, which would
   have retired it with the shortcut and the clamp, read H0 (record, revert),
   so it stays. The moves-legitimately list (a refit, an SPSA run, any step
   moving the margin) is unchanged.
8. `tools/tuner_trace.hpp`, the comment over `gradient`. Before: "Its life is
   short in any case: S039 retires the clamp, and S122's king safety is not
   clamped". After: as site 2.

### Found beyond the list (22), before and after

- `DEV_MANUAL.md`, the tune build: "the hand-tunes, S068 and S039" became "the
  hand-tunes then planned, S068 and S039" -- S073's motive, kept as history.
- `DEV_MANUAL.md`, the SPSA driver: "the alternative is what S068 and S039 did
  -- ... one SPRT per number" lost "and S039": S039 tuned no number.
- `DEV_MANUAL.md`, `--touches`: "its whole goal is to re-decide" became "its
  whole goal was then to re-decide" (the F06 case is history).
- `tools/plan_prose_check.py`, the module docstring's `--touches` paragraph:
  the same tense fix, re-wrapped to 80 columns.
- `tests/test_search_params.cpp`, the note over `golden_defaults`: "S068 and
  S039 each move exactly one number" became "S068 moved exactly one number and
  said so here; S039, planned as the second, read H0 and moved none".
- Seven pending files, the DEC-259 amendment's last sentence (S101, S102,
  S121, S123, S124, S125, S135): "S039 runs before this family and retires the
  lazy shortcut and the clamp; if S039 read H0 instead, the owner's amendment
  of that outcome binds here" became "S039 read H0 (record, revert): the lazy
  shortcut and the clamp are kept. S122's design waits on the owner's
  amendment before S121 starts (DEC-258), and that amendment binds here."
- S101's accepts clause (Deviations, above).
- S101, pitfall "The stage decision changes the lazy margin's input": the sum
  "S039 sizes the clamp on" and DEC-172's order became the sum the clamp
  bounds at `LAZY_EVAL_MARGIN`, kept by S039's H0; the choice decides whether
  the clamp can truncate the term, and the step says which stage it takes.
- S101, interaction "S039 (after, before S122): sizes the clamp" became "S039
  (done, H0)": the clamp is kept; DEC-172 had placed S039 after this step to
  size it, DEC-258 re-formed it into the retirement that read H0.
- S102, pitfall "The stage decision": "the sum S039 sizes the clamp on" became
  "the sum the clamp bounds, kept by S039's H0".
- S121, pitfall "The lazy clamp caps this term together with king safety":
  "the sum S039 then measures" became S039's H0 keeping the clamp, this step's
  larger mobility values entering the clamped sum, and S122's design waiting
  on the owner's amendment before this step starts.
- S121, interaction "S039 (before S122, after this): the clamp is sized on the
  sum this step enlarges" became "S039 (done, H0): ... is kept".
- S125, interaction "S039 (before S122): the lazy margin is sized on a sum
  these terms do not enter" became "S039 (done, H0): the lazy margin, kept,
  bounds a sum these terms do not enter".
- S126, the DEC-170 amendment and its pitfall (two): "S039 and S122 may have
  moved or removed the clamp by then" became S039 read H0 and kept it; S122,
  whose design waits on the owner's amendment before S121 starts, may have
  moved or removed it -- the file's own hedge for S122, kept.
- S126, interaction "S039 and S122 (before): both may move or remove the
  clamp" became two bullets: S039 (done, H0) kept it; S122 may move or remove
  it, its design waiting on the owner's amendment.

### Read and left, each true as it stands

- `tests/CMakeLists.txt`, the comment over `test_plan_touches`: S039 is the
  case F06 found, observed red before the repair -- history.
- `tests/test_mate_carry.cpp`, the GOLDEN history line: S039 corrected it and
  its revert keeps the correction -- true.
- `tools/plan_prose_check.py`'s second mention, "the whole of the S039 case",
  and `DEV_MANUAL.md`'s "S039 is the measured case" -- history.
- `DEV_MANUAL.md`'s `golden_defaults` row ("on the reverted tree's 73") and
  `adocs/specs.md`'s search row (floors re-read at S039) -- true.
- S083's research section, the `.tuning/` hazard "S039 records" -- true.
- S124's interaction "S039 (before): the clamp is on the expensive stage" --
  true, and consistent with H0.
- `src/evaluation.hpp` `evaluate_expensive_terms`'s comment, "the margin cannot
  be re-decided from data without this" -- names no step; it says what the
  function is for.
- `MANUAL.md`: no S039 mention; its option-table row for the margin reads
  184, range 0 to 2000, the code's. Checked, unchanged.

### Phase 2, pending (in order)

In `/home/max/ws/chesso-s287`, once the machine is free. Everything is staged;
the gate's `--message` mode refuses unstaged edits.

```
cmake -S . -B build      -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCHESSO_TUNE=ON
cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure
cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure
CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check
./build/src/chesso bench        # must print the parent's total, 6042933
python3 tools/plan_prose_check.py --prose
python3 tools/plan_prose_check.py --citations
python3 tools/plan_prose_check.py --touches
python3 tools/plan_prose_check.py --params
python3 tools/plan_prose_check.py --gate
CLANG_FORMAT_MAJOR=22 tools/gate.sh --message /home/max/ws/chesso/.tuning/coord/S287/commit_msg.txt
```

Expected: no compiler output change (comments only), the fast suite green in
both builds, format clean, bench 6042933, the five modes exit 0. Not owed:
Debug self-play and `tools/gate_extra.sh` -- no `make_move`, generator or
search code changes (DEC-141's second tier).

### Proposed `done:` stamp (HH:MM and the phase-2 figures the coordinator's)

```
done:       2026-10-16 HH:MM -- the eight statements S039's revert left saying it retires or re-decides the lazy shortcut and the clamp are restated for its H0 (record ca578de, revert 74d0adf: both kept), and 22 more the accepts' grep found with them: DEV_MANUAL.md three (tune build, SPSA driver, --touches), tools/plan_prose_check.py's docstring, tests/test_search_params.cpp's golden_defaults note, and seventeen in eight pending step files (S101, S102, S121, S123, S124, S125, S126, S135), S101's accepts clause among them with its obligation unchanged. S122 is named only as its design waiting on the owner's amendment before S121 starts (DEC-258); S122's file untouched. Comments and prose only: bench 6042933, the parent's, No functional change. Fast suite <n>/<n> in both builds, format clean, the five prose modes clean, tools/gate.sh --message green. MANUAL.md checked, unchanged. Not run: Debug self-play, gate_extra.sh (no make_move, generator or search code). plan.md's part B paragraph still says the two retire under S039: the coordinator's. Filler under DEC-171. Written blind by an Opus subagent beside S082's run; phase 2 and commit by the coordinator.
```

### Commit text

The same text is at `.tuning/coord/S287/commit_msg.txt` in the main checkout,
for the gate.

```
Restate S039's stale claims for its H0: both are kept (S287)

S039 read H0 (record ca578de) and its revert (74d0adf) kept the lazy
shortcut and the clamp on mobility plus king safety, but left
statements saying S039 retires or re-decides them. The eight S039
listed before its revert: DEV_MANUAL's truncation-scan rule and its
trace-tuner clamp paragraph, specs.md's clamp bullet,
src/evaluation.hpp's margin comment (two), src/search_params.hpp's
LAZY_EVAL_MARGIN comment, test_search's GOLDEN 383 note and
tuner_trace.hpp's gradient comment. Each now says S039 read H0 and
both stay; where S122 is named, only that its design waits on an
owner decision before S121 starts (DEC-258).

A grep for S039 found 22 more of the same class, restated with them:
DEV_MANUAL's tune-build, SPSA-driver and --touches paragraphs, the same
--touches sentence in plan_prose_check.py's docstring, the
test_search_params note that S039 moves one number, and seventeen in
eight pending step files (S101, S102, S121, S123, S124, S125, S126,
S135), S101's accepts clause among them with its obligation unchanged.
S122's file is excluded: its design waits on the owner.

Comments and prose only; bench is the parent's. Filler under DEC-171.

No functional change
```

## 2026-10-21: the fast check's findings

A read-only fast check over the staged diff found three things.

1. `adocs/plan.md`'s part B paragraph still said the shortcut and the clamp
   retire together under S039. The coordinator fixes it in the same commit;
   this agent did not touch plan.md.
2. `src/evaluation.hpp`'s margin comment, as first restated, made
   `tools/eval_spread` the tool that measures the share clamped at 184. It
   cannot: its candidate margins are 150, 200, 250, 300 and 400
   (`tools/eval_spread.cpp`, `MARGINS`). The comment now says that
   eval_spread measures the spread, and that the share clamped at 184 is what
   the tuner's load report counts ("the clamp binds on", `tools/tuner.cpp`):
   36503 of the 10795695 rows of `selfplay_v2_dedup.tsv`, 0.338 %, at S262
   (`tools/tuner_trace.hpp`'s gradient comment, S262's record). Comment lines
   only, each 80 columns or fewer.
3. Audit findings. `2026-08-13_adversarial-F05` is closed by this step and
   `closes:` names it: its precondition half was restated before S287, and
   its re-decision half goes with S287's rewrite, which no longer promises a
   re-decision. `2026-09-10_adversarial-F16` and `2026-09-10_adversarial-F25`,
   which S039 named and its H0 left open, are the coordinator's bookkeeping;
   F25 becomes its own filler step.
