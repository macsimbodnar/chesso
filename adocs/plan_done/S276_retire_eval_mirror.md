id:         S276
goal:       `feature_audit` and `truncation_scan` read the engine's trace (`tools/tuner_trace.hpp`) instead of the hand-written mirror, and `tools/eval_model.hpp` with the mirror's dataset is deleted
accepts:    (1) `feature_audit`'s report re-taken over the dedup corpus reads `S275_feature_audit.txt`'s figures -- rank 784, the 39 null directions, 0 column disagreements -- with its design rows the trace's coefficients times the taper weights and its `--plant` labels the trace's smooth score; (2) `truncation_scan` reads smooth against exact from the trace and reproduces the pinned four's 2.875; (3) `tools/eval_model.hpp`, the mirror dataset in `tools/tuner_model.hpp` and `test_eval_model`'s mirror cases are deleted, each deletion recorded, the engine-counting cases kept; (4) `adocs/data/S275_walk.cpp`'s build pin -- `f57b92f` or earlier -- is recorded where its row in `adocs/data/README.md` points
touches:    tools/feature_audit.cpp, tools/truncation_scan.cpp, tools/eval_model.hpp, tools/tuner_model.hpp, tools/tuner_trace.hpp, tests/test_eval_model.cpp, tests/test_eval_positions.hpp, tests/CMakeLists.txt, tools/CMakeLists.txt, DEV_MANUAL.md, adocs/data/README.md
excludes:   any change to src/ or to a shipped weight; any change to the tuner's fit path, which S262 settled
decisions:  DEC-142, DEC-170
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-13 -- feature_audit and truncation_scan read evaluate()'s trace; the mirror is deleted. feature_audit loads the corpus the tuner's way (0 of 10795695 rows differ from evaluate()), its design rows the trace's coefficients times the taper weights and its --plant labels the trace's smooth score: rank exactly 784 of 823, the 39 null directions, the seven held gauges spanning them, 0 column disagreements -- every line after the design check byte-identical to S275's mirror run, and the planted 1 M-row slice byte-identical (45305e64). truncation_scan reads smooth against exact from the trace: 105 past 2.8, 33 at 2.875, hit lines byte-identical to the mirror's, the four pins among the 33 and standing (DEC-142 re-derived); the past-2.0 column, 137108 against the mirror's 138335, is ulp noise on 60996 rows at 48/24. Deleted: tools/eval_model.hpp, tools/tuner_model.hpp, seven test_eval_model cases that tested the mirror (each named with its trace-path counterpart); moved: the pinned truncation case to test_eval_trace; kept: twelve engine-checking cases, against the mirror's count walks kept as tests/test_walked_counts.hpp, and two former preconditions that now guard the position set only. S122 and S136's citations of the deleted case re-pointed (DEC-159), S122's now saying it must extend exact_score() to its group and S136's accepts naming the trace's bound. S275_walk.cpp pinned to f57b92f in adocs/data/README.md. Fast suites 45/45 both, format clean, prose checks clean. DEV_MANUAL updated, MANUAL checked. No src/ change; no second tier (tooling only). Fast check: no blocker; six record items (S122 claimed a guard exact_score() does not give, S136 accepts named a deleted case, the plant check logged, the README pin line, two orphaned preconditions named, the design check caveated and the PL_ width asserted) fixed before commit. Landed on d2b0fbe; the coordinator re-pointed ten later steps' touches: lines off the deleted files.

## Why

S262 moved the tuner onto `evaluate()`'s own trace and retired the mirror
from the fit, but `feature_audit` and `truncation_scan` still read
`tools/eval_model.hpp`, the hand-written copy of the evaluation. Two
descriptions of one function drift: every evaluation step after S262 would
otherwise have to update the mirror for those two tools alone, or let them
audit a model the engine no longer computes. Proposed in S262's As built.

## Lane

Agent work, tooling only; no match, no timing.

## As built (2026-10-13)

Written in two phases under DEC-260. Phase 1, 2026-10-12, in the worktree
`/home/max/ws/chesso-s276` on `cead820`, while S119's SPRT held the machine:
code, tests and documents written blind, nothing built or run. Phase 2,
2026-10-13, after the SPRT ended: the phase 1 diff re-applied cleanly onto
`b3cd241` (S263 and S119's record since; neither touches these files) in
`/home/max/ws/chesso-s276b`, built, gated and measured there. Logs are under
that worktree's `.tuning/coord/S276/p2/`.

### Deviations

- **The mirror's count walks are kept, as a test oracle, not deleted with
  `tools/eval_model.hpp`.** Its five walks -- mobility, king safety per colour,
  passed pawns, pawn structure, piece placement -- and their two index enums
  moved to a new test header, `tests/test_walked_counts.hpp`, code unchanged
  (the comment-stripped bodies compared identical) and only comments that
  named deleted parts reworded. They are the one implementation of those
  counts that shares no code with the engine: the trace records the engine's
  own counts and the engine's accessors (`passed_pawn_counts()` and the rest)
  are the same code collecting, so without the walks the four
  engine-against-walk cases in `test_eval_model` and the mobility check in
  `test_eval_trace` would have no oracle -- for king safety and mobility no
  independent check at all, neither having a hand-worked case. Dropping them
  would drop engine coverage, which the brief forbids. They are not a model of
  `evaluate()`: no weight, taper, phase or sum is in the header; the parse into
  a piece list, the phase, the score, the tempo feature and the loader are
  deleted. **The coordinator decides** whether this is the intended reading of
  "the engine-counting cases kept"; the alternative deletes the header and the
  four comparison cases with it.
- **One case moved rather than deleted.** "the pinned positions reach the
  truncation bound" moved from `test_eval_model` to `test_eval_trace` with
  `truncation_positions` and its GOLDEN block, re-targeted from the mirror's
  score against `evaluate()` to the trace's smooth score against
  `exact_score()`, every assertion and threshold unchanged (each pin `> 2.0`,
  the worst `> 2.8`, the tempo precondition). It guards the bound the smooth
  score is held to, not the mirror. Its golden re-derived below.
- **Kept cases renamed** where their name said "the model", which no longer
  exists; assertions unchanged but for the walk's namespace:
  "the model counts what the engine counts" to "the walk counts what the
  engine counts"; "the engine counts the passed pawns the model counts", "...
  the pawn structure the model counts" and "... the piece placement the model
  counts" to "... the walk counts"; "the model and the engine count passed
  pawns by rank", "... pawn structure by hand" and "... piece placement by
  hand" to "the walk and the engine count ..."; the suite to "eval counts: the
  engine against the walk and by hand"; the fixture `model_fixture_t` to
  `counts_fixture_t`.
- **Two pending step files amended, outside `touches:`, on the coordinator's
  authority (DEC-159).** S122's implementation sketch and S136 twice cited the
  deleted case by its title in `test_eval_model`, which turned
  `test_plan_citation_freshness` red (observed: gate exit 8, that test alone,
  `.tuning/coord/S276/p2/gate1.log`). Phase 1's proposed patch applied cleanly
  (S122 at an offset of one line, after the day's amendment above it): S122's
  line cites the trace's identity case, S136's two the two `test_eval_trace`
  cases that hold the guard, and S136's `touches:` names
  `tests/test_eval_trace.cpp` for the tempo precondition, the four pins and
  the threshold, with `tests/test_eval_model.cpp` until S276. **Two more
  edits after the fast check**, also on the coordinator's authority: S122's
  line had claimed the identity case guards the group finaliser, which it does
  not -- exact_score() reads no group -- so it now says nothing holds the two
  together yet and that S122 extends exact_score() to its group, so that the
  per-row identity and the case cover it; and S136's `accepts:` named
  `test_eval_model`'s integer tolerance, deleted here, and now names the bound
  `test_eval_trace` derives from the tempo weights, and the pinned case's
  tempo precondition as the line the step moves (its "tolerance 3 to 4" is
  dropped from the expected numbers).
- **The past-2.0 column of `truncation_scan` moved, and it is ulp noise.**
  137108 on the trace against 138335 for the mirror's tool built at `cead820`
  the same day; the 2.8 and 2.875 columns and every hit line are identical.
  Taken apart under (2) below. The mirror itself had already left S206's
  recorded 138331 by `cead820` (not bisected), so S206's script fails on that
  one column at `HEAD` either way; recorded in `DEV_MANUAL.md` and
  `adocs/data/README.md`.
- **Comments in `src/` now stale, not edited** (`excludes:`), for a separate
  `No functional change` commit if the coordinator wants one. In
  `src/evaluation.cpp`, three times, that the deleted mirror header "is built
  from it and the two have to mean the same thing" (over pawn structure, piece
  placement and tempo; the walk in `tests/test_walked_counts.hpp` is built from
  the first two now, and nothing walks tempo); in the same file, that the
  placement enum is PL and not PP "because tools/eval_model.hpp already spells
  passed pawn PP_" (the PP_ prefix lives in `tools/tuner_layout.hpp` since
  S262); and over the positional taper, "test_eval_model's one-centipawn slack
  against the tuner's floating-point model", and over tempo's taper,
  "test_eval_model allows two" (both stale already; the bound is
  `test_eval_trace`'s). In `src/evaluation.hpp`, over `passed_pawn_counts()`
  and `king_safety_features()`, "comparing the engine's score against the
  tuner's model" (that comparison is gone; the accessors still exist for
  `test_eval_model`, and for `test_eval_trace`). The comment over `evaluate()`
  ("before that through the mirror") and the tempo comment's pointer to "the
  move is worth the same to either side" stay true.
- **New output and refusals.** `feature_audit` prints one new line after the
  corpus line, `trace: <rows> rows, <entries> entries; <n> differ from
  evaluate(), the clamp binds on <c>`, and refuses a corpus where `n` is not 0,
  as the tuner does; the columns line reads `rows traced again one at a time,
  <d> disagree` and the phase line `against the engine's phase`; the
  identities method and design-check lines and the plant's label line are
  reworded; every other line keeps its format. It ranks nothing, and says so,
  if the trace carries per-side entries (none can today). `truncation_scan`
  refuses a row whose `exact_score()` is not `evaluate()`, and an untraceable
  FEN; its output format is unchanged, so `adocs/data/S206_truncation_drift.sh`
  still parses it.
- **Files outside `touches:`**: `tests/test_walked_counts.hpp` (new),
  `tests/test_eval_trace.cpp` (the include, the mobility walk's namespace, the
  moved case), `tests/test_tuner_gradient.cpp` (one comment that said
  `feature_audit` still reads the mirror's loader), `tools/tuner_layout.hpp`
  (its header comment said the mirror stays for the two tools), the two step
  files above, and the two new data files.
- `adocs/data/README.md` had no row for `S206_truncation_drift.sh` (the brief
  expected one); its pin is in the new S276 section instead.

### What each tool reads now

- **`feature_audit`**: the corpus through `tuner_trace::load()` on every
  hardware thread, the identity `exact_score() == evaluate()` checked on every
  row and a corpus with one mismatch refused. *columns*: each row traced again
  from its FEN, one game, one row at a time in file order, and compared with
  the stored row -- entries, per-side entries, phase, side to move -- and the
  stored phase column against the engine's phase. *identities*: each design
  row is the trace's coefficient times its taper weight times 24 (24 for a
  piece value, which is untapered; the phase and 24 minus it for a tapered
  pair's halves), the clamp left out as before; the design check compares
  `x.params / 24` with `tuner_trace::score()` on the same sampled, unclamped
  rows as before. *occurrence*: the rows where the trace's passer-bucket and
  placement features are non-zero. *tempo*: the side to move the trace
  recorded. *collinearity*: `y` is the trace's coefficient, the occupancy
  columns its table entries per piece type. *--plant*:
  `sigmoid(K * tuner_trace::score())` at the planted weights.
- **`truncation_scan`**: per line, `load_FEN()`, `trace_board()`, a one-row
  dataset, `exact_score()` (refused unless it is `evaluate()`) against
  `score()`; output unchanged.
- **What the trace lacks against the mirror, for these tools: nothing.** The
  mirror gave a piece list with a colour bit per piece; the only things read
  off it were the design's material and table entries and the collinearity's
  occupancy, and both already summed a White piece and a mirrored Black one
  onto one signed column -- exactly the trace's table coefficient. The
  measurements below bear it out line for line. One check is weaker and is
  stated: the design check used to hold the design against a score summed
  independently from the mirror's own features; now the design and `score()`
  read the same entries, so it checks the design builder's taper and site
  arithmetic, not an extraction. The extraction's independent check is the
  walk comparisons above.

### Deletions

- `tools/eval_model.hpp`, whole: the mirror -- its piece-list parse and colour
  bit, its phase, the White-minus-Black king safety form, the tempo feature,
  its `evaluate()` and the re-exported layout. Its walks moved (Deviations).
- `tools/tuner_model.hpp`, whole: the mirror's dataset, `load()` and
  `evaluate_position()`. No reader is left; the tuner left it at S262.
- **From `test_eval_model`, seven cases deleted**, each because it tested the
  mirror, which no longer exists:
  - "the model reproduces evaluate() on every phase": the mirror's score
    against `evaluate()`. Superseded by `test_eval_trace`'s exact identity
    ("the trace reconstructs evaluate() exactly, clamp included") and its
    smooth-score bound.
  - "the model's phase is game_phase()": the mirror's phase. The trace's
    phase is checked against `game_phase()` in "every coefficient is the count
    its index names".
  - The five "the mirror dataset: ..." cases -- "the fixture corpus loads
    through its parser", "load() tapers on the placement's phase, not the
    stored column", "load() refuses a row without one king a side", "every
    stored column holds the feature its index names", "the fixture moves every
    term column both ways": the mirror's loader. Their trace-path counterparts
    stand: `test_tuner_gradient`'s "the fixture corpus loads through the
    tuner's trace", "load() tapers on the engine's phase, not the stored
    column", "load() refuses a row without one king a side", "every stored row
    is its FEN's trace, at any thread count", and `test_eval_trace`'s "the
    positions move every term both ways".
- **Moved**: "the pinned positions reach the truncation bound" (Deviations).
- **Kept, fourteen.** Twelve check the engine or hold what those checks
  stand on: the four engine-against-walk comparisons and three
  engine-and-walk hand cases (names under Deviations); the four non-vacuity
  cases "the positions exercise every count", "... every passed pawn bucket",
  "... every pawn structure count", "... every piece placement count", which
  the comparisons need; and "the move is worth the same to either side",
  unchanged. **Two now guard the position set only**, both written as
  preconditions of the deleted mirror comparison: "the positions have both
  sides to move", re-targeted from the mirror's tempo feature to the FEN's own
  field against the engine's board, and "the positions can tell colours
  apart", its two dead lines building an unused parameter vector dropped. No
  remaining case relies on either -- `test_eval_trace` and
  `test_tuner_gradient`'s trace_positions() run every position both ways
  round, the gradient check claims every term parameter itself, and "the move
  is worth the same to either side" flips each position -- and a comment at
  each says so. Kept, not deleted, on the coordinator's instruction.

### (1) feature_audit on the trace

`adocs/data/S276_feature_audit.txt`, the dedup corpus (sha256 `0a6b59b6...`),
`--sample 200000`, 12 threads, idle machine, wall 50.2 s (one run, not a
timing), peak resident 1.96 GB:

- `trace: 10795695 rows, 266907900 entries; 0 differ from evaluate(), the
  clamp binds on 36503` -- S262's figures.
- columns: **10795695 rows traced again one at a time, 0 disagree**; the stored
  phase column against the engine's phase, **0 disagreements**.
- identities: design check 1.36e-12 over the same 202966 rows the mirror's
  read; then **every line byte-identical to `S275_feature_audit.txt`** -- the
  32 zero columns, the five material/table and two king dependencies holding
  on every row, rank modulo 2^61-1 **784 of 823**, 0 further dependencies,
  upper and lower bound meeting, **rank exactly 784, nullity 39**, each of the
  seven held gauges null on every row, `32 zero columns and 7 held gauges span
  39 of at most 39 null dimensions`.
- occurrence, tempo and collinearity: byte-identical to S275's.
- `--plant`, beside it: over the first 1000000 rows (sha256 `379aa7a6...`,
  S275's real-label slice) this tool and the mirror's, built from `cead820` in
  a scratch directory, write byte-identical planted corpora, sha256
  `45305e64...` -- the planted slice S275 and S262 recorded. Re-run after the
  fast check through a script, with both sha256 printed and the result line:
  `.tuning/coord/S276/p2/plant_check.sh` and `plant_check.log`, `RESULT
  byte-identical`.
- The design check's 1.36e-12 is not the mirror's check repeated: the design
  row and `score()` now read the same trace entries, so it covers the design
  builder's assembly, not an independent model. Said so in `DEV_MANUAL.md`
  after the fast check.

### (2) truncation_scan on the trace

`adocs/data/S276_truncation_scan.txt`, the same corpus, `--min 2.8`, wall
27.9 s (one run, not a timing; the mirror's scan was 47 s at S076):

- **10795695 rows, 137108 past 2.0, 105 past 2.8, 33 at 2.875**, worst
  2.875000. The four pinned positions are among the 33, at phases 5, 11, 19
  and 23.
- Against the mirror's tool built from `cead820`, same corpus and day: 138335 /
  105 / 33, and its 105 hit lines **byte-identical** to these.
- The past-2.0 column taken apart, both tools at `--min 1.99`: 192854 hits
  each, the same FENs, every printed difference identical to six decimals;
  131858 rows print above 2.000000 in both and 60996 print 2.000000 exactly,
  residual 48/24. The mirror's float sum puts 6477 of those an ulp above 2.0
  and the trace's 5250: the whole of 138335 against 137108. In exact
  arithmetic the count past 2.0 is 131858. S206 found the mechanism
  (DEC-169); this measures its size.

### Goldens (DEC-142)

- `test_eval_trace` `truncation_positions` and "the pinned positions reach the
  truncation bound": one end moved (the scan), so re-derived by its script --
  the four are still among the 33 rows at 2.875, so **the four stand**,
  nothing re-chosen; the site's comment records the re-derivation. The 2.0
  figure the comment quotes is ulp noise and asserts nothing.
- `test_eval_trace` "the smooth score is within the truncation bound of the
  exact one", `worst > 2.8`: the same four, unchanged.
- `DEV_MANUAL.md`'s golden table: the moved row re-pointed.
- No other golden touched.

### Evidence pins (accepts (4))

S276's parent is the last commit carrying `tools/eval_model.hpp` and
`tools/tuner_model.hpp`. Recorded in `adocs/data/README.md`, the evidence
itself untouched:

- `S275_walk.cpp` builds at `f57b92f` and no other commit: `adam_t` and
  `descend()` arrived in `tools/tuner_model.hpp` with `f57b92f` (absent at
  `0d9faba`, checked with `git show`), and S262's `bee84d8` removed them and
  `gradient()`. Pinned in its own row; the S262-era line under the S262 table
  that said "and earlier only" is corrected to match (the README is an index,
  not evidence).
- `S206_truncation_drift.sh`: its six readings at named shas are untouched;
  its two `HEAD` readings read the mirror before S276 and the trace from S276,
  and its `HEAD` row fails on the past-2.0 column alone (138335 at `cead820`,
  137108 now, against 138331 expected). The margin counterfactual was not
  re-run.
- `S144_paths.py`: two rows relocate onto `tools/eval_model.hpp` at `HEAD`,
  resolvable only before S276.
- `S134_engine_identity.diff`: already pinned to `1472c96`.
- `S100_feature_audit.txt`, `S134_feature_audit.txt`,
  `S134_identity_before.txt`, `S275_feature_audit.txt`: the mirror's tool, at
  their own trees.

### Suites

- Compiler: both builds clean under `-Wall -Wextra -Werror` on the first
  build of the phase 1 code; nothing to fix.
- `./clang-format.sh` reflowed three spots of phase 1's hand wrapping
  (`test_walked_counts.hpp`'s header comment, two `CHECK`s in
  `test_eval_model.cpp` that now fit one line, one `printf` in
  `feature_audit.cpp`) and a trailing blank line; no code change.
- Gate, `CLANG_FORMAT_MAJOR=22`, i7-8700K, g++ 13.3, `-j12`: before the
  citation patch, build 44/45, `test_plan_citation_freshness` red as phase 1
  predicted; after it, **build 45/45, build-tune 45/45, format clean, exit 0**
  (`.tuning/coord/S276/p2/gate2.log`). `test_eval_model` 14 cases, 2631
  assertions; `test_eval_trace` 8 cases, 1196412 assertions;
  `test_tuner_gradient` 13 cases; all green in both builds.
- `tools/plan_prose_check.py`, one mode per call: `--citations` 0 flagged over
  33 files, `--touches` 0 flagged, `--params` silent; exit 0 each.
- After the fast check's six fixes -- the two step-file edits, the plant
  script, the README line, the comments at the two position-set guards, the
  DEV_MANUAL design-check caveat, and a static_assert in `feature_audit`
  tying its three hand-numbered placement indices to the engine's array width
  (the order cannot be tied: the engine's enum sits inside
  `src/evaluation.cpp`; DEV_MANUAL's "the one place that names a parameter by
  hand" corrected to say what is named by hand) -- the gate and the three prose
  checks were run again: build 45/45, build-tune 45/45, format clean, exit 0
  (`.tuning/coord/S276/p2/gate3.log`); `--citations` 0 flagged over 33
  files, `--touches` 0 flagged, `--params` silent, exit 0 each.
- No second tier: tooling only; `src/` untouched, so no bench and no INV-6.
- `MANUAL.md` checked: no mention of the mirror or either tool, no change.
  `DEV_MANUAL.md` updated: the trace library's linkers, the test section, the
  golden table, the truncation section (the S276 reading and the 2.0 column
  taken apart), the feature_audit section, `--plant`, the trace tests and the
  mirror's retirement.

### Proposed `done:` stamp

2026-10-13 -- feature_audit and truncation_scan read evaluate()'s trace; the
mirror is deleted. feature_audit loads the corpus the tuner's way (0 of
10795695 rows differ from evaluate()), its design rows the trace's
coefficients times the taper weights and its --plant labels the trace's smooth
score: rank exactly 784 of 823, the 39 null directions, the seven held gauges
spanning them, 0 column disagreements -- every line after the design check
byte-identical to S275's mirror run, and the planted 1 M-row slice
byte-identical (45305e64). truncation_scan reads smooth against exact from the
trace: 105 past 2.8, 33 at 2.875, hit lines byte-identical to the mirror's,
the four pins among the 33 and standing (DEC-142 re-derived); the past-2.0
column, 137108 against the mirror's 138335, is ulp noise on 60996 rows at
48/24. Deleted: tools/eval_model.hpp, tools/tuner_model.hpp, seven
test_eval_model cases that tested the mirror (each named with its trace-path
counterpart); moved: the pinned truncation case to test_eval_trace; kept:
twelve engine-checking cases, against the mirror's count walks kept as
tests/test_walked_counts.hpp, and two former preconditions that now guard the
position set only. S122 and S136's citations of the deleted case re-pointed
(DEC-159), S122's now saying it must extend exact_score() to its group and
S136's accepts naming the trace's bound. S275_walk.cpp pinned to f57b92f in adocs/data/README.md.
Fast suites 45/45 both, format clean, prose checks clean. DEV_MANUAL updated,
MANUAL checked. No src/ change; no second tier (tooling only).

### Commit text

```
Read the trace in feature_audit and truncation_scan (S276)

S262 moved the tuner onto evaluate()'s own trace, but feature_audit and
truncation_scan still read tools/eval_model.hpp, a hand-written copy of
the evaluation, so every evaluation step would have had to keep a second
description of evaluate() in step for two audit tools. Both now read
the trace: feature_audit loads the corpus the tuner's way and refuses
one where the trace is not evaluate(), builds its design rows from the
trace's coefficients times the taper weights and its planted labels
from the trace's smooth score; truncation_scan reads smooth against
exact from the same record. Over the dedup corpus both reproduce the
mirror's readings: rank 784 with the 39 null directions, every audit
line after the design check unchanged, the same planted slice, the same
105 truncation hits with the four pins among them.

tools/eval_model.hpp and tools/tuner_model.hpp are deleted, with the
seven test_eval_model cases that tested them. The truncation pins move
to test_eval_trace. The mirror's count walks stay, as
tests/test_walked_counts.hpp: they are the only implementation of those
counts that shares no code with the engine, and the kept cases hold
the engine's counts to them and to hand-worked placements. S122 and
S136 cited the deleted case and now cite the trace's; S122 says it
must extend the exact reconstruction to its group.
```
