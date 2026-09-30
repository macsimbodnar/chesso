id:         S114
goal:       the null move reduction scales with how far the static score is above beta, and the base reduction is re-decided
accepts:    an SPRT verdict, recorded whatever it is; **the static-score term is capped** -- uncapped, a position twenty pawns ahead reduces to depth 0 and re-creates the mate-hiding bug this engine has already shipped twice; the existing zugzwang guard on game_phase is kept, not replaced; the mate cases in the fast suite pass, and the cap is observed to be load-bearing by removing it and watching one go red; the base and divisor seeds are stated with their source and everything ships from our own sweep and S127 (DEC-084). **Read as two verdicts since DEC-243 (2026-09-29)**, each against the commit before it: the first measures the static-score term alone, `NullMoveEvalGate` at 0, and is this step's landing; the second flips the entry gate `static_eval >= beta` to 1 on the tree the first leaves, with DEC-233's second repair of the null-move guard cases the gate takes the premise from ("Stopped" below) made then
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp; as built also src/data_structures.hpp search_node_probe_t (the probe field), tests/test_search_params.cpp (three golden rows), tools/mutants (S114's file and two re-pointed anchors)
excludes:   a null-move verification search -- dropped by DEC-087, no evidence below 3000 and the game_phase guard already covers zugzwang; the reduction formula's final values, which S127 fits
decisions:  DEC-071, DEC-084, DEC-087, DEC-105, DEC-134, DEC-141, DEC-142, DEC-156, DEC-162, DEC-171, DEC-215, DEC-221, DEC-233, DEC-238, DEC-239, DEC-243, DEC-244
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-29 12:37 CEST
done:

## Re-scoped 2026-08-19 by the second review, DEC-087

Two halves, one kept and one dropped. **Kept:** the eval-scaled reduction -- `R
= base + depth/divisor + min((eval - beta)/scale, cap)` is the surveyed form,
and a deeper base R alone measured **+12.3** at Berserk, with the eval-term cap
worth a further +3.3/+2.3 at Ethereal. The static evaluation input exists since
S108. **Dropped:** the verification search. No engine in this band shows a gain
for it -- it is a Stockfish-depth device -- and this engine's zugzwang exposure
is already guarded by `game_phase(&game->board) > 0`, which stays.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol if drifted. Every GitHub read
below was a commit message or PR body, never a diff or source file (DEC-016).
Under DEC-105 a number quoted from such prose is still that engine's tuned
output and never a seed, wherever it is republished: the figures below are
records, and section 4's seeds are derived here.

### 1. State of the art

**Base R, as published.** CPW's record: Goetsch & Campbell 1988 ran recursive
null move at R=1; Donninger 1993 ("Null Move and Deep Search", ICCA Journal
16(3)) popularised it, his own R not stated on the page; "most typical engines
use recursive null move with an R value of 2 or 3"; a "basic implementation ...
uses a fixed reduction of 3 or 4"; Fruit ran R=3 gated on static eval > beta;
Heinz 1999 ("Adaptive Null-Move Pruning") is R=3 when depth > 6 else R=2 -- the
first depth-scaled base. **Convention trap**: CPW's own examples mix `depth -
R` and `depth - R - 1`; chesso searches `depth - 1 - R` (`src/search.cpp`
`negamax`), so a quoted base transfers only next to its engine's depth
arithmetic. Modern re-decides in prose: Weiss #746 "Reduce the depth of NMP
searches more" (2024, +3.86); Lynx #2448 "5 + depth/5" failed STC -2.33 and
then #2620 passed the same form +2.89 bundled with an improving
simplification; Berserk #552 "less base reduction + more from depth/eval"
0.08/+1.89 at 3300+.

**The eval-scaled term, traced.** CPW Advanced Tweaks, verbatim: "a factor
scaled by the difference between (potentially TT-corrected) evaluation and
beta can also be added to the reduction value". Records:
- **Weiss #129** (2020-01-06) "Increase NMP depth reduction based on eval":
  **+5.36 +/-4.21** at 10+0.1, **+11.48 +/-6.92** at 60+0.6, at roughly this
  band, rationale in the PR: "When static eval beats beta by a lot we don't
  need as strong evidence to conclude this is already too good." It sits on
  #127 "Dynamic NMP depth" (+6.92/+16.87) and #128 "nmp at depth >= 2"
  (+7.08/+13.12) from the same week -- the ladder landed together.
- **Stockfish prose**: 314faa9 (2009) "Null move dynamic reduction based on
  value" ~+5 self-play; 074c7a3 (2014) "Variable null-move value based
  reduction" -- grows when eval exceeds beta by more than a pawn; 7ed15af
  (2014) "**Cap evaluation based null move extra reduction to three plies**",
  a zero-Elo safeguard. The form assembled from prose:
  `extra = min((eval - beta) / pawn, 3)`.
- **Ethereal 89ed7eb** (2018-06-24) "Cap NMP formula component from eval
  difference": **+3.28** at 5+0.05, **+2.32** at 30+0.3 -- the header's cap
  number, traced. Distinct object: 039dea6 (2018-01-16) "Do not cap R for
  Null Move Pruning" passed [0,5] -- the cap on *total* R went, the cap on
  the *eval term* came back six months later and gained. Cap the term.
- **The cautionary pair, Stockfish 2026**: 9d4090e8 "Scale Null Move Pruning
  reduction dynamically based on evaluation margin" **passed STC <0.00,2.00>
  and LTC <0.50,2.50>** -- and 9a8dd81d (2026-07-11) reverted it: "causes
  very poor mate finding performance, ultimately causing a CI hang on a mate
  in 2", with a `go mate 2` testcase FEN in the message. Two green SPRTs did
  not certify mate safety; only a mate suite does -- this repo's rule,
  demonstrated at 3600.

**Entry conditions, published vs here.** Non-PV; not in check; no consecutive
passes; beta below mate; non-pawn material for the side to move; and an eval
gate -- Fruit "static evaluation greater than beta" (CPW), Weiss eca86285 "No
NMP if static eval is below beta" +2.51 and ac11d26 "Require static eval to
be higher at low depths" +3.44/+4.41, **Lynx #1918 "staticEval - beta margin
>= 30": +6.13/+6.23**. Chesso has all of these except the eval gate, which
had no input before S108. The dropped verification search has a measured
record: Berserk #524, "Implemented as-is from SF", **-0.68 STC / -0.24 LTC**
-- DEC-087's drop is confirmed, not just argued.

### 2. Shape for chesso

- R today: `NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR` = 3 + depth/6
  (`src/search.cpp` `negamax`; `src/search_params.hpp` `NULL_MOVE_BASE` and
  `src/search_params.hpp` `NULL_MOVE_DIVISOR`, ranges 0..16, 1..64). With the
  floor below, NMP fires from depth 5.
- - Conditions (`src/search.cpp` `negamax`): `!is_pv && !is_in_check && ply > 0
  && prev_move != 0 && depth - 1 - null_reduction >= 1 && beta < MATE_MIN &&
  game_phase(&game->board) > 0`. No eval gate, no eval term.
- - **The mate fix is the floor** `depth - 1 - null_reduction >= 1` in
  `src/search.cpp` `negamax`, with the comment above it: a depth-0 null search
  is pure quiescence, answers with the static score, and lost a mate in two at
  depth 4. Regression test: "pruning does not hide a forced mate",
  `tests/test_search.cpp` "pruning does not hide a forced mate".
- **The zugzwang guard counts both colours**: `game_phase()`
  (`src/evaluation.cpp` `game_phase`) clamps `board->phase`, accumulated from
  `phase_value[6] = {0,1,1,2,4,0}` (`src/eval_tables.hpp` `phase_value`) for
  every piece of either side -- "some knight/bishop/rook/queen exists on the
  board", weaker than the published side-to-move form. Kept as-is per the
  accepts.
- - Mate-score clamp on the fail-high (`src/search.cpp` `negamax`) is published
  practice exactly -- SF 42de93ac (2010) "Do not return unproven mate scores
  from null move search"; Lynx #1790 +1.85. The fail-soft return of
  `null_score` matches Ethereal 255c263 "Return value and not simply beta
  during NMP" (+2.35/+1.47). Both stay.
- - No TT store on a null cutoff (`src/search.cpp` `negamax` returns without
  storing); the Weiss TT-skip entry condition (4596ecf, +4.01) is a deferred
  extra, not V1.
- - Input: post-S108 every non-check node has `static_evals[ply]`; today
  `static_eval` is set only inside the RFP guard in `src/search.cpp` `negamax`.

### 3. Implementation sketch

One SPRT, as the accepts prices:
1. 1. Two new constants beside `src/search_params.hpp` `NULL_MOVE_BASE` and
   `src/search_params.hpp` `NULL_MOVE_DIVISOR` in the X-macro, ranges stated
   (section 4).
2. Entry gains the gate `static_eval >= beta` (the term is then never
   negative); R gains `min((static_eval - beta) / NULL_MOVE_EVAL_MARGIN,
   NULL_MOVE_EVAL_CAP)`; the floor at `src/search.cpp` `negamax` tests the
   **full** R.
3. Base and divisor are re-decided by a node-count sweep over the 300
   positions (the S021/S068 instrument) with margin and cap alongside --
   published practice lands the family together (Weiss #127-#130) -- then
   one SPRT on the winner; S127 fits finals.
4. Tests, red first, printouts recorded:
   - "pruning does not hide a forced mate", `tests/test_search.cpp` "pruning
     does not hide a forced mate" stays green unmodified at every depth it
     runs.
   - New case built the S033 way (python-chess enumeration + Stockfish
     confirmation, DEC-023): one side ~20 pawns ahead statically, opponent
     holding a forced mate inside the null horizon -- the demolition target.
   - Non-vacuous precondition: a position where the term raises R (node
     counts move against cap=0); then eval-below-beta holds counts still.
   - Zugzwang: CPW's "Null Move Test-Positions" page is the published set
     (zugzwang.001-005); most carry pieces, so the game_phase guard does not
     switch NMP off there -- they document what the dropped verification
     covered, not what this step must pass. Fetch fresh and verify any
     position used with stockfish first (DEC-023).

### 4. Constants and seeds

All in the `CHESSO_SEARCH_PARAMS` X-macro in `src/search_params.hpp` with
ranges; every number a **seed -- must be fitted/SPSA'd here** (S127). Under
DEC-105 each is one of three forms and says which: **(a)** a value from a
publication about the technique, with its URL; **(b)** a derivation over
chesso's own data or scale, which includes a value chesso already ships from
its own SPSA run; **(c)** the range midpoint or off value, stated as such.
No engine's shipped R, divisor or margin seeds anything here, wherever it is
republished; those records are in section 1 and, as anti-seeds, in section 5.

**Units, once for this file (P6).** The eval margin is compared against
`evaluate()`, so it is in chesso's material scale, `piece_value` in
`src/eval_tables.hpp`: `PAWN` 94, `KNIGHT` 327, `BISHOP` 308, `ROOK` 487,
`QUEEN` 716. The header's own comment says the split between `piece_value`
and `psqt_mg` / `psqt_eg` is degenerate, so the material term alone is the
unit. "One pawn" therefore means **94**, not 100 -- DEC-134 decided this
treatment by name. Plies have no unit at all, so a ply count quoted from an
engine takes form (c) and nothing else.

- `NULL_MOVE_BASE` seed **3**, sweep 1..4 -- **(b)**, the value chesso ships
  today, which is S085's SPSA output over chesso's own games. (The
  2026-08-19 pass wrote "seed 2, ships today"; **it is 3 since S085**.) The
  sweep range has **(a)** context and no (a) seed: Heinz's rule as the wiki
  states it is "R=3 when normal search depth exceeds 6 plies and R=2
  otherwise", https://www.chessprogramming.org/Depth_Reduction_R (fetched
  2026-09-05) -- mind the depth-arithmetic convention.
- `NULL_MOVE_DIVISOR` seed **6**, sweep 3..8 -- **(b)**, the value chesso
  ships today.
- `NULL_MOVE_EVAL_MARGIN` seed **94**, range 1..2000 -- **(b)**, one pawn in
  chesso's own material scale, `PAWN` in `src/eval_tables.hpp` (units above).
  The wiki's Null Move Pruning page states the eval-scaled factor with no
  number (https://www.chessprogramming.org/Null_Move_Pruning, fetched
  2026-09-05), so there is no (a) to take. Floor 1 is the div-by-zero guard;
  the range top parks the term.
- `NULL_MOVE_EVAL_CAP` seed **8**, range 0..16 -- **(c) midpoint**, stated as
  such, and 0 is the term off. **A midpoint is a poor seed for a safety cap
  and this file says so out loud**: at 8 a large static lead takes eight extra
  plies off the null-move depth, and this step's own `accepts` calls an
  uncapped term the mate-hiding bug this engine has already shipped twice.
  So the mate instruments decide the seed's admissibility *before* any
  sweep -- `TEST_SUITE("engine: mate safety")` in `tests/test_engine.cpp` and
  `TEST_CASE` "pruning does not hide a forced mate" in
  `tests/test_search.cpp`. **The measured alternative, P4**, and this step
  may take it instead of the midpoint if the instruments say 8 is
  inadmissible: with the eval term landed in the tune build, sweep the cap
  downward from the range top, record the largest value at which both suites
  pass, and seed one below it. That is a "measured" bound in the sense
  `src/search_params.hpp`'s own header defines, the kind `RFP_MIN_PLY`'s
  floor is. Whichever is used, the stamp records it and why. (The owner's
  answer of 2026-09-08: seed the midpoint, keep P4 named beside it.)
- The entry gate ships as bare `>= beta`; a margin on it is an S127-era
  sweep, not V1.

seeds re-derived 2026-09-04 under DEC-105 (DEC-134)

### 5. Pitfalls

- **Anti-seeds — records, not seeds.** DEC-019 lets a record say which
  direction is worth trying; DEC-105 forbids any of these numbers starting a
  sweep, which is why section 4 names no engine. Fruit's R=3 and the wiki's
  "2 or 3" / "3 or 4" wordings are engines' shipped reductions; the divisor
  "depth/5" is Lynx's prose; the eval margin "one pawn" is SF 074c7a3's, in
  SF's scale, and the cap "three plies" is SF 7ed15af's, with Ethereal
  89ed7eb the gain record for having a cap at all. Lynx #1918's margin-30
  entry gate is a form to try at S127, not a value to start from.
- - **The floor and the cap are different safety devices, and the demolition
  only reddens if both are lifted.** With the floor kept and the term uncapped,
  a +20-pawn node makes R huge, `src/search.cpp` `negamax` fails, and NMP
  switches off -- fail-safe, green, Elo quietly lost. "Reduces to depth 0"
  happens only in a build where the floor is also gone (then `src/search.cpp`
  `negamax` turns the null search into quiescence -- the shipped-twice bug).
  Ship floor + cap; the demolition build lifts both, watches the new mate case
  go red, restores.
- **The floor bites the term at shallow depth**: with cap 3, depths 4..8 have
  `depth - 1 - (3 + depth/6 + 3) < 1`, so a maxed term *skips* NMP where
  today it fires with R=3..4 -- backwards from the term's intent. The sweep
  must see this; SF's alternative (null search falls to qsearch) is exactly
  what this repo's history forbids. If the sweep prefers a clamp
  (`null_depth = max(1, ...)`) over the skip, that is a recorded choice --
  the never-below-1-ply property is what must survive.
- **A passing SPRT is not mate safety** -- SF 2026 passed two and hid a mate
  in 2. Fast suite (`ctest --test-dir build -L fast`) and the mate cases run
  before and after, not instead.
- **Sentinel**: `static_evals[ply]` is TT_EVAL_NONE in check; NMP is off in
  check anyway, but assert the term never reads a sentinel rather than
  assume it.
- **Zugzwang stays exposed by design**: the both-colours guard admits NMP in
  piece endgames (CPW's zugzwang set is mostly such positions); verification
  was dropped on Berserk's negative measurement. Do not add those positions
  as red tests -- they fail by design without verification.
- - **S113 merge care**: ProbCut's enrichment inserts its block right after
  `src/search.cpp` `negamax`; this step rewrites `src/search.cpp` `negamax`.
  Land in plan order, rebase the later.

### 6. Measurement

One SPRT at the S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`,
fast suite and both mate suites green first, demolition printout recorded in
the stamp. Node counts move by construction, so INV-6 takes the SPRT path;
search_bench depths 9/12 recorded, plus the fixed-node depth check (`go nodes
1000000` over the three bench positions) -- pruning should raise fixed-node
depth. Expectation, direction only (DEC-019): 0..+12 -- Weiss's ladder is the
ceiling at this band, SF-2026's <0,2> is the floor at 3600, and this project
has measured three published figures at 0, 0 and slower.

### 7. Interactions

- **S108 (before, the input)**: supplies `static_evals[ply]`; the term reads
  the raw static, never a search score -- Lynx #2055's TT-corrected variant
  (+1.56) is an S127-era refinement, already noted in S108's section 7.
- **S113 (just before, ladder neighbour)**: separate verdicts by plan order;
  a re-decided R shifts how much fail-high work reaches ProbCut, and both
  live in the same 30 lines of negamax (merge note above).
- **S109 (before, both prune -- compose)**: NMP prunes the node above beta
  before the move loop, S109 prunes moves inside it; no shared lines.
  Improving-modulated NMP (Berserk #303 +1.58) defers with S109's improving
  consumers to S127.
- **S116 (after, the mirror)**: razoring is the same ladder entry below
  alpha, dropping to quiescence where NMP verifies above beta; both read the
  S108 eval; neither touches the other's guards.
- **S097 (before)**: Berserk #45 disables NMP on singular verification
  searches -- gate on `excluded_move == 0` once S097's parameter exists.
- **S127**: base, divisor, margin, cap join the SPSA set; entry margin,
  TT-skip (Weiss 4596ecf +4.01) and threat-conditioned R (Berserk #405
  +3.84/+2.52, Ethereal 291b2f8 +1.68/+1.25) are priced there, not here.

### Scope concern

Two, neither touching goal or accepts. (1) The accepts' "the cap is observed to
be load-bearing by removing it and watching one go red" is unobservable with
the `src/search.cpp` `negamax` floor kept: cap removal alone fails *safe* (NMP
skips; green). The red observation needs the demolition build to lift floor and
cap together, or the load-bearing claim re-attached to the floor -- owner's
wording call at implementation time; the section 5 arithmetic is the evidence.
(2) DEC-087's "a deeper base R alone measured +12.3 at Berserk" was not
re-traced through Berserk's PR/commit prose this pass; nearest traced is #286
"NMP returns null search score" +10.94 (with a non-pawn-material fix), which
chesso's fail-soft return already covers.

### 8. References

- https://www.chessprogramming.org/Null_Move_Pruning -- conditions, basic R,
  Fruit, the eval-scaled-factor sentence, verification quotes.
- https://www.chessprogramming.org/Depth_Reduction_R -- R=2/3, Heinz 1999
  adaptive rule, `depth - R - 1`, Goetsch & Campbell.
- https://www.chessprogramming.org/Null_Move_Test-Positions -- the five
  zugzwang FENs (fetch fresh before use).
- https://api.github.com/repos/TerjeKir/weiss/pulls/129 -- the eval term,
  +5.36/+11.48, rationale quoted.
- https://api.github.com/search/issues?q=repo:TerjeKir/weiss+null+type:pr --
  #127, #128, #245, #422, #572, #643, #688.
- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+nmp -- d5fe301,
  4596ecf, ac11d26, c6ec34b, eca86285, a0c69ef (#746).
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+null+move+reduction
  -- 314faa9, 074c7a3, 7ed15af, 620cfbb, 08ac4e9.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+hash:9d4090e82685cca447265dcd7093d617cb34a107
  -- the 2026 scaling commit, full message and both SPRT blocks.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+null+move+mate
  -- 9a8dd81d revert with the mate-in-2 testcase, 42de93ac, a4fedd81.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+NMP --
  89ed7eb (the cap, +3.28/+2.32), 291b2f8, 255c263.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+null+move
  -- 039dea6, 706ffaa, 90daf70, the 2016 pre-history.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+hash:039dea6
  -- "Do not cap R" full message, passed [0,5] at 60+0.6.
- https://api.github.com/search/issues?q=repo:jhonnold/berserk+nmp+type:pr --
  #552, #549, #524 (verification, negative), #405, #303, #286, #161, #45,
  #21; null/deeper/reduction sweeps found no +12.3 base-R record.
- https://api.github.com/search/commits?q=repo:jhonnold/berserk+null -- the
  2021 introduction trail, 79bdf2c9 (+10.94).
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+type:pr+NMP --
  #1918, #1790, #2448, #2620, #2055, #1971.
- https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+null+move
  -- 2020 messages only, no per-change numbers in prose.

## What the tree already did, 2026-09-29 (checked at `6e8bc63`)

The step file was written at `0edfd26` and reseeded on 2026-09-04; each
assumption was checked on the tree by symbol before any code was written.

- **The null-move block** (`src/search.cpp` `negamax_at`) ran under `!is_pv &&
  !is_in_check && ply > 0 && prev_move != 0 && excluded_move == 0 && depth - 1
  - null_reduction >= 1 && beta < MATE_MIN && beta > -MATE_MIN &&
  game_phase(&game->board) > 0`, with `null_reduction = NULL_MOVE_BASE + depth
  / NULL_MOVE_DIVISOR`, the fail-soft return of `null_score` and `beta` in
  place of a mate-range score, and no table store. **S097's `excluded_move ==
  0` gate was already there**: the brief asked for it only where the tree
  lacked it, so there was nothing to add.
- **The input exists**: `static_eval` is computed at the top of every node
  since S108 and is `TT_EVAL_NONE` (`INT16_MIN`) in check. The null move read
  no static score.
- **ProbCut's block (S113) follows the null move**, then S097's singular
  block; neither is touched here.
- **The seeds as section 4 states them**: `NullMoveBase` 3 and
  `NullMoveDivisor` 6 are S085's SPSA output (its table: base 2 -> 3, divisor
  6 -> 6); `PAWN` is 94 in `src/eval_tables.hpp` `piece_value`.
- **The parent's numbers** (`6e8bc63`, built in `.ref-builds/parent`):
  `bench` 3656950 with c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6; `bench 12`
  1702684; `tools/search_bench.py` 70913 / 71220 / 24293 at 9 and 128099 /
  258594 / 83277 at 12, best c3d5 e2a6 d7c8q and c3d5 d5e6 d7c8q -- S022
  verdict 2's landing, node for node.

## Stopped, 2026-09-29 13:20: the gate takes the premise from nine null-move guard cases

**Resolved the same day by DEC-243, the coordinator's ruling B**: this verdict
measures the term alone with `NullMoveEvalGate` at 0, the nine cases keep their
premises, M02's case takes the repair its arithmetic asks for, and the gate is
S114's second verdict. The section below is the record of why, kept as it was
written (DEC-215 clause 3); "The first verdict" further down is what was built
after it.

**Where the code is.** `src/search_params.hpp` carries `NullMoveEvalMargin`
(94, 1..2000, form (b)), `NullMoveEvalCap` (8, 0..16, form (c)) and a switch,
`NullMoveEvalGate` (1, 0..1); `src/search.cpp` `negamax_at` gates the null
move on `static_eval >= beta` behind the switch, asserts the sentinel is never
read, computes `R = NullMoveBase + depth / NullMoveDivisor + min(max(static_eval
- beta, 0) / NullMoveEvalMargin, NullMoveEvalCap)` past the guards and tests
the floor on the whole R; `src/data_structures.hpp` gains the probe field
`null_reduction`. The zugzwang guard, the mate clamp and the fail-soft return
are unchanged; ProbCut's and the singular block are untouched.

**The switch is a deviation from the brief's off value -- the cap at 0, the
base and the divisor at their shipped seeds -- and it is forced.** The cap at 0
turns the term off, but no value of the four numbers turns the gate off: the
tune build at `NullMoveEvalCap` 0 benches **4904021**, not the parent's
3656950. DEC-215 clause 2 gives the gate a switch; at `NullMoveEvalGate` 0 and
`NullMoveEvalCap` 0 the tune build benches **3656950 with all eight replies
the parent's** (c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6). The clamp at zero
is what makes that identity exact: without it a static score below beta would
take plies off R at gate 0. For the record, niced, counts only: candidate
(gate 1, cap 8) `bench` 3921066, kiwipete's reply d5e6 -> e2a6; term alone
(gate 0, cap 8) 4189351; gate alone (gate 1, cap 0) 4904021.

**The reds.** Both fast suites, Release and tune, are red in exactly four
cases of "search: pruning and reduction guards", each at `REQUIRE(
probe.null_move_made )`: "the node after a null move has no previous move to
index" (H03), "a null-move fail-high against a mate returns the bound" (M04),
"the child after a null move is labelled the type the parent is not" (T08)
and "an excluded node makes no null move" (E10, its control). Each drives a
node whose static score is below its beta -- the castled pawn-wall board reads
0 for the side to move against beta 100, M04's queen-up board 835 against
beta 40000 -- so the gate refuses before the rule under test decides.
Everything else is green in both builds: every mined row of "pruning does not
hide a forced mate", `test_mate_carry`, `test_mate_breadth`, `test_mate_pv`,
`test_engine`'s mate safety, and `test_search_params` once its golden took the
three rows (the deliberate-change edit its header prescribes).
`.tuning/coord/S114_logs/fast_release_1.log`, `fast_tune_1.log`.

**Five more cases stay green and stop killing their mutants**, each mutant
applied by hand to a snapshot of this tree and the guard suite run
(`.tuning/coord/S114_logs/probe_mutants_before_repair.log`; the four red cases
are red in every run and the named case is not among them): **M01** (in check
the static score is the sentinel, `INT16_MIN`, under every ordinary beta, so
the gate refuses first), **M03** and **N01** (static 0 and -55 under beta 100),
**N02** (beta `MATE_MIN`: the static score never reaches the band -- asserted
by `test_evaluation` "the static score never reaches the mate band" -- so at
gate 1 the positive-edge guard can never decide, and N02 is **equivalent** in
the release build) and **M02** (the S165 defender set at depth 5: at beta at
or below `-MATE_MIN` the term is always at its cap, R = 3 + 0 + 8, so the
floor refuses the pass at depth 5 with or without the band guard). And two
anchors moved with the rewritten block: M03 in `tools/mutants/search.py` and
E10 in `tools/mutants/S097_singular_extension.py`.

**Why this stops the step.** The brief's rule: a red that is not a mined
golden stops the step and is reported. These are not goldens. DEC-233 names a
second repair for this class -- a case's drive arranged so its premise holds
again, every assertion kept -- which the brief did not extend to this step.
No test was edited.

**What resumes it -- two ways, the coordinator's call.**

- **A, the step as briefed (gate 1), with DEC-233's premise repair.** Every
  assertion kept; each mutant re-observed killed. (1)
  `require_null_move_preconditions` asserts the two new conditions too -- the
  static score at or above beta, and the whole R leaving exactly one ply at the
  drive depth. (2) H03, T08, E10, N01 and M03's drives move from beta 100 to
  the node's own static score, `evaluate()` on the wiped table, as the
  reverse-futility cases already set theirs; H03's quiet fail-high in the null
  child is re-checked there, not assumed. (3) M01's in-check drive moves to
  beta `TT_EVAL_NONE`, where the gate would admit the sentinel -- which is also
  the brief's sentinel case. (4) M04's case needs a beta at or under its 835
  where the null search still returns a mate score; re-derived by the case's
  own hand-driven precondition, a new board if none exists. (5) N02 declared
  equivalent in the release build at gate 1 in `tools/mutants/S191_guards.py`,
  with its case given a tune-only leg at `NullMoveEvalGate` 0 (S113's B15
  form). (6) M02's defender-set drive moves to the shallowest depth where the
  capped term leaves one ply, 15 at the seeds, or to a tune-only leg at cap 0
  if that is too slow for the fast suite. (7) M03's and E10's anchors
  re-pointed, mutations unchanged (S097's precedent).
- **B, one change at a time: this verdict ships the term with
  `NullMoveEvalGate` 0**, and the gate gets its own verdict later. Measured on
  a snapshot built at gate 0: the whole guard suite green, 82 of 82, and every
  null-move mutant killed by its own case but **M02**, which (6) repairs the
  same way (`.tuning/coord/S114_logs/probe_mutants_gate0.log`). The off value
  is then the cap alone.

Either way the rest is unchanged: the new cases (the demolition row is picked,
`adocs/data/S114_demolition_row.py`: `rbrb4/p1p1p3/P1k1P3/5Q2/2K5/8/8/8 b - - 1
4`, stockfish `#-1`, the threat after a pass Qc5#, quiet, no capture on the
board), the sweep (`adocs/data/S114_null_move_sweep.py`, written, not run), the
mutants, the measurements and the pre-registration.

## Stopped again, 2026-09-29 14:15: the sweep's winner moves the divisor, and the divisor moves the guard cases' drive depth

**Resolved the same day by DEC-244, the coordinator's ruling S**: the winner's
0.8 % pooled lead flips on two of the three samples, inside the instrument's
own disagreement, so the sweep re-decides nothing; the base and the divisor
stay at S085's 3 and 6, the term is measured alone at them, and the table
below is S127's input. `NULL_DRIVE_DEPTH` is derived from the constants all
the same -- 5 at 3 and 6, every assertion the one it was. The winner's
re-derivations were stopped (`.tuning/coord/S114_remine/div5/run_all.out`
records what they had cost). Kept as written below (DEC-215 clause 3).

**The sweep** (`adocs/data/S114_null_move_sweep.py`, the S021/S068 instrument:
S021's 300 positions, three stratified samples of 100 from
`adocs/data/S018_raw.tsv`, `go depth 11` on the tune build, niced; reading
`adocs/data/S114_null_move_sweep_d11.tsv`). Node counts, not Elo (DEC-019).
Grid `NullMoveBase` 1..4 x `NullMoveDivisor` 3..8 at `NullMoveEvalMargin` 94,
`NullMoveEvalCap` 8, `NullMoveEvalGate` 0; pooled against the off row (cap 0,
3, 6 -- the parent's tree), 27924538 nodes:

| rank | base | divisor | nodes | pooled | per sample |
|---|---|---|---|---|---|
| 1, the winner | 3 | 5 | 26351602 | 0.9437 | 0.9423 0.9437 0.9452 |
| 2, runner-up | 2 | 3 | 26507807 | 0.9493 | 0.9521 0.9505 0.9448 |
| 3, the seeds | 3 | 6 | 26572771 | 0.9516 | 0.9325 0.9290 0.9957 |

Every base-1 cell is above the off row (1.02 to 1.25). Two pairs are one
engine at this depth -- (2, 4) with (3, 8) and (3, 4) with (4, 8) give the same
R at every depth up to 11 and the same counts -- which is the instrument
agreeing with itself, not a finding. The winner leads the seeds by 0.8 % pooled,
and the seeds are the lower of the two on two of the three samples.

**What shipping the winner does.** At `NullMoveDivisor` 5 the reduction at
depth 5 is 3 + 1 = 4, so the null search there has no ply and the null move
is never made at depth 5. `NULL_DRIVE_DEPTH` in "search: pruning and reduction
guards" is 5 -- "the only depth at which the block's own `depth - 1 -
null_reduction >= 1` clears by exactly one ply, which each case asserts rather
than assumes" -- and it is a literal. A throwaway build at 3 and 5
(`.tuning/coord/S114_logs/div5_fast.log`) is red in `test_search` in thirteen
cases: every null-move guard case at its own precondition, this step's own
cases at theirs, and "pruning does not hide a forced mate" at the multicut
row; plus `test_search_params` and `test_plan_params` (a golden row and
`MANUAL.md`'s row, the edits a moved default owes). The repair is one number:
the drive depth re-derived from its own definition, 5 -> 6 at (3, 5) --
better, derived in the test from `NULL_MOVE_BASE` and `NULL_MOVE_DIVISOR` so
S127's refit cannot strand it. It is not a mined golden and the brief's rule
stops the step on it; no test was edited.

**Where the step stands.** On the seeds' tree (3, 6) the rest is measured:
the demolition red, the identity, the fixed-node depths, both re-derived
goldens (below). The winner's re-mine and capture-table pass are running on
throwaway worktrees at 3 and 5 so either answer resumes without a wait
(`.tuning/coord/S114_remine/div5/`).

**Measured on the seeds' tree** (3, 6, gate 0, cap 8), niced, counts only:

- **Off value** (DEC-215): the tune build at `NullMoveEvalCap` 0 benches
  3656950 with the parent's eight replies, and `tools/search_bench.py` reads
  the parent's 70913 / 71220 / 24293 at 9 and 128099 / 258594 / 83277 at 12,
  best moves the parent's (`.tuning/coord/S114_logs/identity_cap0_*`).
- **Candidate**: `bench` 3656950 -> 4189351 (+14.56 %), kiwipete's reply d5e6
  -> e2a6, the other seven unchanged; `search_bench` 9 87445 / 69482 / 23937,
  12 148153 / 312807 / 90782, kiwipete's best at 12 d5e6 -> e2a6. Fixed-node
  depth (`adocs/data/S097_fixed_node_depth.py`, `go nodes 1000000`, Hash 16):
  17 / 15 / 16 = 48 -> 17 / 14 / 16 = 47, reach and not a forecast (DEC-239).
- **The demolition** (`.tuning/coord/S114_logs/demolition_red.log`, a
  throwaway copy of this tree, reverted after each variant): shipped green;
  **floor and cap lifted, red** -- `CHECK( last_score <= -MATE_MIN_LOCAL )`,
  `values: CHECK( -715 <= -48000 )` at depth 5 and at 15, R 23 and 25; floor
  alone lifted, the mate lost at 5 and found at 15; cap alone lifted, the mate
  found at both and only the case's reading of R red (23 against 11, 25
  against 13). The cap alone fails safe, as section 5 has it: an uncapped term
  with the floor kept makes R exceed the depth, and the floor refuses.
- **Two mined goldens moved with the tree and were re-derived by their own
  scripts** (DEC-142, DEC-233): the capture-mate table's pass -- profiles
  `d7 d9 d10 d11 d12`, `d7 d8 d9 d10 d11 d12`, `d10 d11 d12`, `d9 d11 d12`,
  depths 7, 7, 10, 9, row 4 now `C02` (`.tuning/coord/S114_remine/capmates/`)
  -- and S097's multicut row, in guard mode (DEC-238): both modes take
  `1R6/8/2p3p1/P5P1/1p2b2P/4k3/6pK/8 b - - 1 54` at depth 14, mate in 5,
  shipped `d11 d13 d14`, guard dropped `d11 d13`, the mutant's multicut firing
  at 13 and 14, 1042946 nodes and 487 ms (`.tuning/coord/S114_remine/`). The
  table's rows are in the test; the multicut row waits for the ruling, since
  the winner's tree will pick its own.
- **M02's repair** (DEC-243): "no defender node inside the mate band makes a
  null move" drives its 104 nodes at the shallowest depth the capped term
  leaves a ply at, derived from the constants -- 15 at (3, 6) -- instead of
  `NULL_DRIVE_DEPTH`; the rows and the assertion are unchanged, and the case
  costs 3.36 s niced against 0.06 s before.
- **Debug**: the guard suite in a Debug build of this tree, 87 of 87, the new
  `assert(static_eval != TT_EVAL_NONE)` never firing
  (`.tuning/coord/S114_logs/debug_guards.log`).

## The first verdict, as built: the term alone at the seeds (DEC-243, DEC-244)

Implemented from the step file's description (sections 2 to 5), written from
the wiki and published prose (DEC-221); no other project's code was opened and
no constant of one is behind any value here.

**What lands.**

- `src/search_params.hpp`: `NullMoveEvalMargin` (94, 1..2000), `NullMoveEvalCap`
  (8, 0..16) and `NullMoveEvalGate` (0, 0..1), beside `NullMoveBase` and
  `NullMoveDivisor`, each seed's form at its site; the base and divisor
  comment records the sweep's non-decision (DEC-244), and the `RfpTtEstimate`
  comment's "null move pruning has no static-score condition" is corrected.
- `src/search.cpp` `negamax_at`, the null-move block only: the gate's clause
  `(NULL_MOVE_EVAL_GATE == 0 || static_eval >= beta)` in the condition, true at
  the shipped 0; `assert(static_eval != TT_EVAL_NONE)` past the guards; `R =
  NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR + min(max(static_eval - beta, 0) /
  NULL_MOVE_EVAL_MARGIN, NULL_MOVE_EVAL_CAP)` computed there; the floor on the
  whole R, with the pass and its fail-soft return inside it unchanged. The
  zugzwang guard, both mate-band edges, the mate clamp and S097's exclusion
  gate stand as they were; ProbCut's block, S097's singular block, S109's
  pruning and the generator are untouched.
- `src/data_structures.hpp`: the probe field `null_reduction` (S113's
  precedent), written only in the probing instantiation.
- `tests/test_search.cpp`, "search: pruning and reduction guards": six cases,
  each observed red -- "the null move reduction adds a ply per margin the
  static score stands above beta", "a static score below beta takes nothing
  off the null move reduction" and "the floor refuses a pass the static-score
  term leaves no ply for" under their NT mutants in the mutation run; "the
  static-score term does not hide a forced mate" (the demolition row) in the
  demolition and under NT02; "the static-score term is never computed at a
  node in check" (the sentinel, beta -40000 under `INT16_MIN`) run alone on a
  release build with M01 applied, since in a whole-suite run M01 crashes
  test_search before it -- `REQUIRE_EQ( probe.null_reduction, -1 )`, values 11
  against -1 (`.tuning/coord/S114_logs/m01_sentinel_case.log`); and the
  tune-only "the static-score term moves the tree above beta and holds it
  still below" (node counts against cap 0, the non-vacuous precondition) in a
  tune build, since `tools/mutation_check.py` builds the release binaries only
  -- under NT01 `CHECK( above_on != above_off )` read 3 against 3 and R at cap
  0 read 23 against 3, under NT03 the above leg 4 against 4 and the below leg
  48 against 51 (`.tuning/coord/S114_logs/tune_case_red.log`). That case's
  below leg shows the cap does nothing below beta and cannot see the clamp --
  without it the term is -2 at either cap -- so the clamp's guard is the
  release case "a static score below beta takes nothing off the null move
  reduction". `NULL_DRIVE_DEPTH` derived from the base and the divisor, 5 at
  the seeds (DEC-244); the null-move preconditions helper also asserts the
  static score stands less than one margin above beta, so the floor it checks
  is the whole R (the fast check's finding, test_search green after it); M02's
  case at the depth the capped term leaves a ply at (DEC-243); the two mined
  goldens re-derived.
- `tests/test_search_params.cpp`: the three golden rows, 74 defaults.
- `tools/mutants/S114_null_move_term.py`, NT01 to NT06; M03's anchor in
  `tools/mutants/search.py` and E10's in `tools/mutants/S097_singular_extension.py`
  re-pointed, mutations unchanged, each re-observed killed below.
- `adocs/data/`: `S114_demolition_row.py`, `S114_null_move_sweep.py` and its
  reading `S114_null_move_sweep_d11.tsv`, `S114_remine_s097.log`,
  `S114_sprt.sh`; the README's rows. `MANUAL.md`'s three option rows and the
  divisor's row; `DEV_MANUAL.md`'s bench ledger and the two golden rows.

**Seeds (DEC-134).** `NullMoveEvalMargin` 94, (b): one pawn in `piece_value`'s
scale, the wiki giving the factor with no number. `NullMoveEvalCap` 8, (c): the
midpoint, the owner's answer of 2026-09-08; the mate instruments did not make
it inadmissible -- both fast suites and every mate case are green at 8 -- so P4
was not needed. `NullMoveBase` 3 and `NullMoveDivisor` 6, (b): S085's SPSA
output, kept by DEC-244 against the sweep's first, (3, 5), whose lead flips on
two of three samples. `NullMoveEvalGate` 0: a verdict switch (DEC-215 clause 2),
no seed. The entry gate's margin, the clamp in place of the skip and the
table-corrected input are S127-era forms (section 4, section 5).

**The cap, read as the pair.** The accepts' "the cap is observed to be
load-bearing by removing it and watching one go red" is unobservable with the
floor kept, because the cap alone fails safe: with the
floor `depth - 1 - R >= 1` tested on the whole R, an uncapped term at twenty
margins makes R 3 + 0 + 20 = 23 at depth 5 and 3 + 2 + 20 = 25 at 15, both
past the depth, so the pass is not made and the node is searched -- section 5's
arithmetic, and the demolition's cap-only row says so (the mate found at both
depths, only the case's reading of R red). The observation the accepts wants
is the demolition that lifts both, and it is red: the node answers -715 in
place of a mate at 5 and at 15. With the floor alone lifted the cap still
earns its place at 15, where the capped R leaves one ply and the pass sees the
mate while the uncapped one would not.

## Proposed `specs.md` sentence (the coordinator edits specs.md)

In the search row, beside the null-move guards: **the null move's reduction
grows with the static score's lead over beta since S114** (its first verdict,
DEC-243): past the block's guards R is `NullMoveBase + depth / NullMoveDivisor`
(3 + depth/6) plus one ply for every whole `NullMoveEvalMargin` (94, a pawn)
the node's raw static evaluation stands above beta, at most `NullMoveEvalCap`
(8), clamped at zero, and the floor `depth - 1 - R >= 1` tests the whole of it,
so a lead that would leave the null search no ply makes the pass impossible
and the node is searched; the cap alone fails safe, and with the floor and the
cap lifted together `tests/test_search.cpp`'s "the static-score term does not
hide a forced mate" goes red. The entry gate `static_eval >= beta` is in the
tree behind `NullMoveEvalGate` at 0, S114's second verdict; `NullMoveEvalCap` 0
is the tree before S114, node for node (DEC-215). And the routing sentence
"this engine's null-move block has no static-score condition" becomes "this
engine's null-move term reads the raw static evaluation, never the estimate".

**Suites, format and checks** (2026-09-29, niced beside S022's second SPRT).
Release `build`: 41 of 41. Tune `build-tune`: 39 of 41 in the gate run, the
two reds ctest ceilings under the SPRT's load and not assertions --
`test_mate_carry` at its 120 s (104.68 s run alone, green; the Release run 106.69
s) and `test_fastchess_script` at its own 60 s (63.09 s run directly, "26
properties hold"; the Release run 55.76 s) -- a load reading, to re-time on the
idle machine (`.tuning/coord/S114_logs/gate_1.log`, `tune_timeouts_rerun.log`,
`fastchess_script_direct.log`). `./clang-format.sh --check` clean;
`tools/plan_prose_check.py --citations`, `--touches`, `--params`: 0 flagged.
The Debug guard suite: 87 of 87.

**Mutation** (DEC-141 clause 2): `tools/mutants/S114_null_move_term.py` on a
clean detached fixture, `.ref-builds/mut` at `3e1b892`, a throwaway commit of
this working tree on no branch: header `baseline green, 41 tests, bench 4189351
nodes via engine`; **mutation score 6 of 6 (100 %)**, wall 3537 s. NT01 to NT05
each red at its named case; NT01 leaves the bench signature unmoved, so only
its cases see it. NT06 was killed in the suite by timeouts -- a static score
below beta gives the null search plies back, the tree explodes and six binaries
time out before they report -- and its named case run alone is red at R 2 and
-2 against 3 (`.tuning/coord/S114_logs/mutation.log`, `nt06_named_case.log`).

**The mutants this step's edits touch, on the same fixture**, `--only M01 M02
M03 E10 E21 C02 R02` over the whole `tools/mutants/` directory -- so every
anchor of the 170 validates, the two re-pointed ones included: **7 of 7
killed**, wall 2178 s. M02 by its case at the new depth; M03 and E10 on their
re-pointed anchors by their own cases; E21 by the new multicut row and S243's
direct case, the two killers the pre-registration's item 9 names; C02 and R02
by the capture-mate rows their labels name as well as by their direct cases;
M01 by crashes in seven binaries -- a null move made in check -- before its
case runs (`.tuning/coord/S114_logs/mutation_extra.log`); run alone on an M01
build, S114's sentinel case is red at R 11 against -1
(`.tuning/coord/S114_logs/m01_sentinel_case.log`).

**Not run here, by the brief**: DEC-141's second tier -- the Debug self-play,
which is a match, and `tools/gate_extra.sh` -- and the SPRT.

**What a reviewer should read first**: the null-move block's diff in
`src/search.cpp` `negamax_at` (the floor moved inside, the assert, the clamp);
"the static-score term does not hide a forced mate" with the demolition log;
the M02 case's new depth and `NULL_DRIVE_DEPTH`'s derivation; the two re-mined
rows' GOLDEN paragraphs.

## Rebased onto the tree without the early-out, 2026-09-29

Written by a fresh Opus agent briefed by the coordinator
(`.tuning/coord/S114_rebase_brief.md`), in the linked worktree `s114b` at
`db1905f` -- achesso's `51f69ff` with S022 verdict 2's node-level delta
early-out taken back out, the removal achesso carries as its own commit, whose
engine is `58585f8`'s node for node -- while S022 verdict 2's SPRT held the
machine: every CPU-bound command niced, builds at `-j4`, no match, no
`tools/gate_extra.sh`. **Every number the sections above took was taken on
`6e8bc63`, the early-out's tree, and they stay as that build's record; every
number in this section is this tree's.** Nothing was committed. The logs are
this worktree's `.tuning/coord/S114b_logs/`, cited below as
`.tuning/coord/S114b_logs/`, for the coordinator to copy to the main tree.

**How.** The first build's patch, `.tuning/coord/S114_landing/working.patch`
against `6e8bc63`, applied with `git apply -3`: the engine files applied clean
and their hunks are the first build's line for line (compared hunk by hunk
against the patch), so `git diff db1905f -- src` is the term and nothing else.
Three files conflicted where the early-out's removal and S114 both wrote, and
each was resolved by keeping the removal's text and adding S114's for this
tree: `tests/test_search.cpp` (the multicut row), `tests/test_search_params.cpp`
(the history comment and the count) and `DEV_MANUAL.md` (the multicut golden
row and the bench ledger). The test diff is S114's six cases, the fixture's
precondition, M02's depth, `NULL_DRIVE_DEPTH`'s derivation and the goldens
below.

### Deviations and findings, first

1. **The multicut row moved again.** S131's row, which the removal brought
   back, stays green on this candidate and **stopped separating E21**: with
   E21 applied the case passes, both builds reporting its mate in 6 at depths
   13 and 14 and neither at 11 or 12 (`.tuning/coord/S114b_logs/e21_case.log`).
   So it was re-mined, as the brief allows on exactly that reading, and the
   pick moves the case's depth with the row, **14 -> 13**: the golden is the
   position and its depth, both re-derived by the script's rule.
2. **The capture-mate table went red nowhere on this tree** -- every row of
   the parent's table still reads its mate at its old depth on the candidate
   -- but the rule's answer moved, and DEC-142 re-derives that. The depths come
   out as the first build's, 7, 7, 10, 9, and **two labels differ from the
   first build's**: row 3 "C05, R02" and row 4 "C02, C05, C07".
3. **A finding older than this step: the parent's own table is stale on the
   parent.** The same seven sweeps on a Release build of `db1905f` put the
   parent's table at 7, 7, 10, 10 with row 3 at "C02, R02" and row 4 at "R02"
   (`.tuning/coord/S114b_logs/capmates/parent_*`), against the committed 7, 9,
   11, 10 -- rows 2 and 3 and row 4's label, green and not the rule's answer;
   when it went stale is not measured here. A test's golden, no play reached:
   **S248, a filler (DEC-171)**, which the coordinator creates at the reading.
   On an H0 the pre-registration restores S113's rows byte for byte, as the
   precedent has it, and this staleness with them.
4. **`DEV_MANUAL.md`'s `golden_defaults` row read 71 against the first
   build's 74**; the first build's fast check did not see it. On this tree the
   table holds 72 (the removal's 69 and S114's three) and the row says so.
5. **The new multicut row's oracle label is #6 and the engine asserts mate in
   5.** Not judged by eye (CLAUDE.md): stockfish at depth 20 and at depth 30,
   each a fresh process through python-chess, report `#+6`; asked for the mate
   itself, `go mate 5` reports `#+5` in 147243 nodes with a line python-chess
   reads as ending in checkmate, and `go mate 4` under a 120 s cap finds
   nothing shorter, its search ending at depth 245 -- every line printed by
   `.tuning/coord/S114b_logs/remine/oracle.py`, a fresh process per question,
   into `remine/oracle.log`. The row asserts the distance the shipped build
   reports, the pick rule's, and that is the oracle's own mate search's.
6. **The demolition's node answers 373 here**, where the first build's tree
   answered -715; every other reading of the demolition is the first build's.
7. **The S170 budgets patch touches two files beyond the brief's three**: the
   comment in `tests/test_mate_carry.cpp` that says E drives 500000, which the
   new budgets make false, and the one sentence of `DEV_MANUAL.md` that says
   which steps re-swept the budgets. Both are in the patch, not the worktree.
8. **The first build's re-mine log stays as `adocs/data/S114_remine_s097.log`**,
   that tree's record as S113's earlier trees' picks were kept; this tree's is
   `adocs/data/S114_rb_remine_s097.log`, and both README rows say which is
   which.
9. **E's re-derived budget cell reports only short lines**, 10 of 10 at
   300000: the rule chooses on the mate count alone and the short lines are
   the ceiling's, 10 against E's 11, so it is taken as the rule says.

### The goldens, each re-derived by its own script (DEC-142, DEC-233, DEC-238)

**The capture-mate table** of `tests/test_search.cpp` "pruning does not hide a
forced mate", `adocs/data/S230_mine_r01_row.py depths` over
`adocs/data/S230_table_fens.txt` at depths 3 to 12, the shipped build and each
of the six mutants of `tools/mutants/S091_capture_see.py` applied to a
throwaway worktree of the candidate and reverted (`.tuning/coord/S114b_logs/capmates/`,
driver `.tuning/coord/S114b_logs/fx1_checks.sh` stage B). Shipped profiles:
`d7 d9 d10 d11 d12`, `d7 d8 d9 d10 d11 d12`, `d10 d11 d12`, `d9 d10 d11 d12`;
the parent's, the same sweeps on its own library: the same but row 4's,
`d10 d11 d12`.

| row | old (S113's pass, the parent's) | new (this tree) |
|---|---|---|
| 1 | 7, 5, "C02, C05, R02, since S112" | 7, 5, "C02, C05, R02, since S112" |
| 2 | 9, 5, "no S091 mutant, since S095" | **7**, 5, "no S091 mutant, since S095" |
| 3 | 11, 4, "R02" | **10**, 4, **"C05, R02, since S114"** |
| 4 | 10, 5, "no S091 mutant, since S113" | **9**, 5, **"C02, C05, C07, since S114"** |

No mate distance moved; C06 and R01 are separated by no row at any depth.

**The multicut row** of the same case. S131's row
`7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42`, depth 14, mate in 6,
stopped separating E21 (deviation 1), so `adocs/data/S097_mine_mate_row.py`
stages 2 to 6 ran in guard mode (DEC-238) and in the default mode for the
record, stage 1 not re-run -- the 269 candidates, byte-identical to the first
build's list, and their oracle did not move. The libraries: the worktree's
Release and tune builds, copied first, and E21's built from a plain copy of
the worktree's `src/` with E21 applied, never the worktree's own (driver
`.tuning/coord/S114b_logs/remine.sh`, the sweeps 34 minutes in parallel).
**One of 269 separates the sweeps and both modes take it**:
`R2Q1bk1/5q2/4bP1p/2p1P3/3pB3/7P/2P3PK/8 w - - 3 50` at **depth 13**, mate in
5 -- shipped `d11 d12 d13 d14`, guard dropped `d11 d12 d14`, the mutant's
multicut and the shipped one both changing the tree at 12, 13 and 14 -- a cell
of 6637574 nodes, about 2.2 s niced under the match. python-chess: is_valid
True, is_check False, 37 legal moves, 2 captures, no promotion, White to move;
the oracle as deviation 5. **Observed red under E21** at `REQUIRE(
result.mate_found )`, "mate the multicut hides, depth 13", **and green
shipped** (`.tuning/coord/S114b_logs/red_e21.log`). Evidence
`adocs/data/S114_rb_remine_s097.log`: the driver, every stage, both rows'
cells, the oracle, S131's row under E21 and the red. S131's row comes back on
an H0 or no verdict, a revert of one line, one depth and one distance.

**`tests/test_search_params.cpp`'s `golden_defaults`**: the three rows
`NullMoveEvalMargin` 94 (1..2000), `NullMoveEvalCap` 8 (0..16) and
`NullMoveEvalGate` 0 (0..1), the count **69 -> 72** on this tree (74 on the
first build's, where the early-out's two rows were in), its history comment
keeping the removal's sentence and adding S114's.

The two repairs DEC-243 and DEC-244 name are the first build's and read the
same here: the defender-set case at depth 15, derived from the constants, and
`NULL_DRIVE_DEPTH` derived from the base and the divisor, 5 at the seeds.

### The off value (DEC-215)

The tune build at `NullMoveEvalCap` 0 is the parent node for node: `bench`
**3429473** with all eight replies (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6
e5e6) and its whole 121-line stream identical, time and nps stripped, to a
Release build of `db1905f`; `bench 12` 1694808 with its 105-line stream
identical the same way; `tools/search_bench.py` 48304 / 71580 / 25413 at 9 and
104784 / 244824 / 117798 at 12, best moves c3d5 e2a6 d7c8q at both -- the
brief's figures, `58585f8`'s (`.tuning/coord/S114b_logs/identity_cap0_*`,
`parent_*`). The tune build at its defaults prints the Release candidate's
bench stream line for line.

### The counts, parent -> candidate (counts only, niced)

| instrument | parent (`db1905f`) | candidate |
|---|---|---|
| `bench` | 3429473 | **4192793** (+22.26 %) |
| `bench` replies | c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 | c3d5 **d5e6** d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 |
| `bench 12` | 1694808 | 1792474 (+5.76 %) |
| `bench 12` replies | c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 | c3d5 **d5e6** d7c8q g7h8q **a7a6** a1b2 e5e6 e5e6 |
| `search_bench` 9 | 48304 / 71580 / 25413, c3d5 e2a6 d7c8q | 23875 / 69842 / 21491, **g5f6** e2a6 d7c8q |
| `search_bench` 12 | 104784 / 244824 / 117798, c3d5 e2a6 d7c8q | 106886 / 237812 / 86217, c3d5 **d5e6** d7c8q |
| fixed-node depth, `go nodes 1000000`, Hash 16 | 17 / 15 / 15 = 47, c3d5 e2a6 d7c8q | 18 / 14 / 16 = **48**, c3d5 d5e6 d7c8q |

The fixed-node depths are `adocs/data/S097_fixed_node_depth.py` on both
binaries, stated as reach and not as a forecast (DEC-239): kiwipete a ply
shallower with its move moving, the other two a ply deeper. INV-6 does not
discharge the change; the SPRT does.

### The demolition, on this tree

`.tuning/coord/S114b_logs/demolition_red.log`, in the throwaway worktree,
reverted after each variant: shipped green; **the floor and the cap lifted
together, red** -- `CHECK( last_score <= -MATE_MIN_LOCAL )`, values 373 against
-48000, at depths 5 and 15, R 23 and 25; the floor alone lifted, the mate lost
at 5 and found at 15; the cap alone lifted, the mate found at both and only the
case's readings of R and of the pass at 15 red. The comment of that case in
`tests/test_search.cpp`, "the static-score term does not hide a forced mate",
named this log and said so until the case left with the term (verdict 1's
removal, below).

### The two cases the fast check found unobserved, observed again here

On a release build of the throwaway worktree with M01 applied, "the
static-score term is never computed at a node in check" run alone is red at
`REQUIRE_EQ( probe.null_reduction, -1 )`, values 11 against -1, and green
shipped (`.tuning/coord/S114b_logs/m01_sentinel_case.log`). In a tune build of
the same worktree, "the static-score term moves the tree above beta and holds
it still below" is green shipped, red under NT01 (`above_on` 3 == `above_off`
3; R at cap 0 23 against 3; the pass not made) and red under NT03 (the above
leg 4 == 4, the below leg 48 against 51)
(`.tuning/coord/S114b_logs/tune_case_red.log`). The first build's readings,
number for number.

### Mutation (DEC-141 clause 2)

**NT01 to NT06**: `tools/mutation_check.py` over
`tools/mutants/S114_null_move_term.py` on a fresh throwaway fixture,
`.ref-builds/mut` at `ab0e58c`, a commit of this working tree on no branch
(its `src/` and `tests/` identical to the worktree's), `--jobs 4`, niced:
header `baseline green, 41 tests, bench 4192793 nodes via engine`;
**mutation score 6 of 6 (100 %)**, wall 3598 s. NT01 1 of 41 with the bench
signature unmoved, so only its cases see it; NT02 3 of 41 (the mate cases of
`test_search`, `test_engine`'s mate safety and `test_mate_carry`'s
`unreached.empty()`); NT03 1; NT04 2 (with `test_mate_carry`'s ceiling);
NT05 1; NT06 6 of 41 by timeouts -- a static score below beta gives the null
search plies back, five binaries time out before they report and
`test_mate_carry`'s majority fails -- and its named case, "a static score below
beta takes nothing off the null move reduction", run alone on the fixture after
the passes, is red at R 2 and -2 against 3 and green shipped
(`.tuning/coord/S114b_logs/mutation.log`, `nt06_named_case.log`).

**The mutants this step's edits touch**, `--only M01 M02 M03 E10 E21 C02 R02`
over the whole `tools/mutants/` directory on the same fixture -- so every
anchor of the 162 validates, the two re-pointed ones included: **7 of 7
killed**, wall 2339 s (`.tuning/coord/S114b_logs/mutation_extra.log`). M02 by
its case at depth 15; M03 and E10 on their re-pointed anchors by their own
cases; **E21 by both of its killers**, the re-mined row's `REQUIRE(
result.mate_found )` and S243's direct case "the multicut never ends a node on
a mate from its verification" -- item 9's two; C02 by the ProbCut row of
"pruning does not hide a forced mate", "a capture that gives check is not
pruned", `test_mate_pv` and `test_mate_carry`'s ceiling; R02 by the multicut
row's distance, which it reads as mate in 6, and its direct cases -- the table
rows their labels name sit after those fatal rows, so the labels are the
sweeps' measurement, as the table's comment says; M01 by crashes in seven
binaries -- a null move made in check -- before S114's sentinel case runs,
which is why that case was observed alone above.

### Suites and checks

Release `build` 41 of 41 (`test_mate_carry` 82.17 s of its 120), tune
`build-tune` 41 of 41 (85.69 s), niced, serial, on the final tree
(`.tuning/coord/S114b_logs/fast_release_final.log`, `fast_tune_final.log`).
**Debug**: the guard suite, "search: pruning and reduction guards", 87 of 87,
and the whole `test_search` binary 187 of 187, 809925 assertions, the new
`assert(static_eval != TT_EVAL_NONE)` never firing
(`.tuning/coord/S114b_logs/debug_guards.log`). `./clang-format.sh --check`
clean with `CLANG_FORMAT_MAJOR=22`; `tools/plan_prose_check.py` `--citations`,
`--touches`, `--params` and `--gate` clean. **Not run here, by the brief**:
DEC-141's second tier -- the Debug self-play, which is a match, and
`tools/gate_extra.sh` -- and the SPRT.

### S170's budgets, re-derived on this tree -- a separate patch (DEC-156 as DEC-162 left it)

`adocs/data/S203_case_sweep.sh` with no argument, niced, on a copy of this
tree's Release build, the full 108-cell grid in 39 minutes. **The rule, stated
before the grid was read** (in the run's own header): each row takes the
cheapest cell at its own stride that reports a mate line, on the mate count
alone, the short lines the ceilings' and never a reason to refuse a cell.

| case | stride | old budget (S245's) | old cell now | new budget | its cell (mates / short) |
|---|---|---|---|---|---|
| A_mate8_shallow | 1 | 500000 | 6 / 0 | **300000** | 1 / 0 |
| B_mate6_shallow | 1 | 100000 | 14 / 0 | 100000 | 14 / 0 |
| C_mate7_depth11 | 1 | 1000000 | **0 / 0** | **1200000** | 2 / 0 |
| D_mate_minus6_depth10 | 1 | 4000000 | 2 / 0 | **1200000** | 2 / 1 |
| E_mate_minus9 | 1 | 500000 | 2 / 0 | **300000** | 10 / 10 |
| F_mate6_inherited_no_line | 2 | 100000 | 2 / 0 | 100000 | 2 / 0 |

`--at` on the new TSV reproduces the six cells exactly
(`.tuning/coord/S114b_logs/s170/at.txt`). **No ceiling moved and none was
asked to**: `--ceilings` over the four recorded grids answers 5, 15, 0, 2, 11,
5, with the new grid added the same six, and the new grid alone 2, 6, 0, 1,
10, 5. `test_mate_carry` at the new budgets is green in both builds, 53.36 s
and 54.96 s (`.tuning/coord/S114b_logs/s170/fx1_carry_check.log`). The TSV
stays S245's in this worktree; the change is
`/home/max/ws/chesso/.tuning/coord/S114_landing/s170_budgets.patch` -- the TSV
with its S114 paragraph quoting the old rows, the grid as
`adocs/data/S114_sweep_s170.txt`, its README row, and the two sentences of
deviation 7 -- to land as its own commit after S114's.

### The pre-registration, re-read for this tree

`adocs/data/S114_sprt.sh`: the REF prose names the removal commit as the
landing's parent, and CAND and REF are pinned as explicit shas -- the landing
and its parent, never `HEAD` and `HEAD^`, since the budgets commit lands after
it; the "measured on S022 v2's early-out" warning is gone; the sweep says it
was taken on `6e8bc63` (`0c0db1b` with S244's screen) and why it stands
(DEC-244); every count is this tree's; the open-findings block is re-read
here -- S022 verdict 2 read and removed (item 13), S244 in the tree with its
second tier pending (item 6), the budgets re-derived as their own commit (item
3), the parent's stale table as S248, a filler (item 4), this verdict's
goldens (item 16) and S247, the stop half's 13.8 ms figure, open as a filler
(item 17). The expected games and the abort rule are the first build's byte
for byte, and so are the three readings but for one addition to the H0 and
the no-verdict rows: the S170 budgets are re-derived on the reverted tree
(DEC-156 as amended by DEC-162), since S245's set was derived on the
early-out's tree and this landing's on the S114 tree.

### Proposed commit (not verdict-closing)

```
Scale the null move reduction with the static score's lead (S114 v1)

S114's first verdict (DEC-243): past the null-move block's guards, R is
NullMoveBase + depth / NullMoveDivisor (3 + depth/6, S085's SPSA output,
kept by DEC-244 because the depth-11 node sweep was inconclusive) plus
one ply for every whole NullMoveEvalMargin (94, a pawn) the node's raw
static evaluation stands above beta, at most NullMoveEvalCap (8),
clamped at zero; the floor depth - 1 - R >= 1 tests the whole of it, so
a lead that would leave the null search no ply makes the pass impossible
and the node is searched. With the floor and the cap lifted together the
new case "the static-score term does not hide a forced mate" goes red at
depths 5 and 15; the cap alone fails safe. Implemented from the step
file's description (DEC-221); margin and cap are seeds in DEC-134's
forms (b) and (c).

Built first on S022 verdict 2's tree and rebased onto the tree its
removal leaves, where every number here was re-taken. The entry gate
static_eval >= beta is in the tree behind NullMoveEvalGate at 0, the
parent's condition, S114's second verdict; NullMoveEvalCap 0 is the tree
before S114 node for node (DEC-215): the tune build at cap 0 benches the
parent's 3429473 with all eight replies and the whole stream, and
reproduces search_bench at depths 9 and 12. The term alone: bench
3429473 -> 4192793 (+22.3 %), kiwipete's reply e2a6 -> d5e6, the other
seven unchanged; fixed-node depths 47 -> 48 over the three positions,
stated as reach (DEC-239). The counts move, so INV-6 does not discharge
it: adocs/data/S114_sprt.sh measures it at {0, 5} nElo against this
commit's parent, with the gate as the second verdict.

Two mined goldens moved and were re-derived by their own scripts
(DEC-142, DEC-233): the capture-mate table's depths 7, 9, 11, 10 -> 7,
7, 10, 9 with two labels, no row red but the rule's answer moved; and
the multicut row of "pruning does not hide a forced mate", S131's again
after the removal, which stopped separating E21 and was re-mined in
guard mode (DEC-238) to a new board at depth 13, mate in 5, red under
E21 and green shipped. M02's case was blind at depth 5 -- the term sat
at its cap on every drive -- and runs at depth 15, derived from the
constants as NULL_DRIVE_DEPTH, and the guard fixture asserts the term is
zero on every drive it makes. Six new cases, each observed red under its
mutant or the demolition; NT01 to NT06 killed 6 of 6, and the seven
touched mutants 7 of 7 on the same fixture, E21 by both its killers.

Bench: 4192793
```

### Verdict 1 landed and pinned (the coordinator, 2026-09-30)

Built on S022 verdict 2's tree, rebased onto its removal, landed as `794e4c3`
on `d946b6f` from the rebase worktree's working tree, two cold fast checks first (two cases observed red that had not been,
a comment's clamp, the fixture's precondition on the term, the decisions
line); `bench` 3429473 -> 4192793. **Second tier on the landing** (DEC-141):
Debug self-play of 8 games at 4+0.04 on `ad18661`, 0 `Assertion`, 0 `disconnect` (`.tuning/coord/S114_debug_selfplay/`); `tools/gate_extra.sh`
four stages green in the full run -- citations, debug 446 s, sanitize 582 s, perft 57 s -- and its prose stage red on one stale `plan.md` sentence that still counted S022's two verdicts as owed, corrected in this commit and the stage re-run green (`.tuning/gate_extra_2026-09-30_S114.log`, `.tuning/gate_extra_2026-09-30_S114_prose2.log`). SPRT pair pinned: `REF` `d946b6f` (the commit the landing sits on),
`CAND` `794e4c3`; open findings re-read at pinning: as the pre-registration
states them. S022 verdict 2 read as a zero at the cap and its early-out removed first (`6893c0f`); S244 and S022 completed; the S170 budgets re-derived on this tree as `ad18661` (DEC-162).

### Verdict 1's outcome (2026-09-30, the coordinator)

**H0, 2026-09-30 07:33: `794e4c3` against `d946b6f`, `Elo -18.54 +/- 10.19`,
`nElo -23.81 +/- 13.06`, LLR -2.95, 2720 games in 1 h 17 m, 0 forfeits**
(`adocs/data/S114_sprt.log`, `adocs/data/S114_sprt_pairs.txt`; marker
`DONE`). **H0 with the interval wholly below zero**: a loss; `NullMoveEvalCap` goes to 0, the proved off value, and the term's code leaves with it on the pre-registration's H0 row (the S238 pattern): its line, the margin and the cap, the probe field, the six S114 cases and the six NT mutants, the manuals with them; the two re-derived rows of item 16 and the defender case's depth go back byte for byte, and S248 re-derives the parent's capture-mate rows; the S170 budgets are re-derived on the reverted tree (DEC-156 as amended by DEC-162). The gate's switch, its clause and its row stay at 0 for S114's second verdict (DEC-243), as does the floor's place inside the block and the derived `NULL_DRIVE_DEPTH`. `Incomplete mating PV` 0 against 3,
an observation for the pre-registration's open findings (CHESS).
No follow-up run and no second pair for this verdict (DEC-063, DEC-019).

## Verdict 1's removal, 2026-09-30

Written by a fresh Opus agent on the linked worktree `s114rm` at `b3876f8`,
briefed from `.tuning/coord/S114_rm_brief.md`. It started beside the SPRT's
last minutes: until the match ended every CPU-bound command ran under `nice
-n 19` with `-j4` builds, and after it `-j8`, still niced, beside the S115
agent's own work. Every number below that decides anything is a node
count, a best move or a pass; the only seconds quoted are wall times. **The
reading, from the coordinator**: `794e4c3` against `d946b6f`, `{0, 5}` nElo at
8+0.08, **accepted H0 on 2026-09-30 at 07:33 -- `Elo -18.54 +/- 10.19`, `nElo
-23.81 +/- 13.06`, LLR -2.95, 2720 games in 1 h 17 m, 0 forfeits** (the run's
log becomes `adocs/data/S114_sprt.log`), an interval wholly below zero. That is
the pre-registration's H0 row exactly, "a loss, and the revert is one default",
and this section executes it: the term's code leaves with `NullMoveEvalCap`'s
off value (the S238 pattern), and what the row names stays. The record commit,
its result block and the shared documents are the coordinator's. Nothing was
committed or staged by this agent. The logs are this worktree's
`.tuning/coord/S114_rm/`, cited below by that path, for the coordinator to
carry into the main tree's. Written against `b3876f8`: the record commit
`22e00c3` has since appended "Verdict 1's outcome" to this file, and this
section follows it at the landing.

**How.** By hand against `b3876f8`: the null-move block of `src/search.cpp`
`negamax_at` edited down to the parent's reduction with the floor left where
S114 put it, `src/data_structures.hpp` restored from `d946b6f` whole, the two
rows and their comment taken out of `src/search_params.hpp`, the six cases
taken out of `tests/test_search.cpp` and its three golden sites restored from
`d946b6f`'s text -- the multicut row and the capture-mate table each with a
record paragraph, the defender case byte for byte -- two rows out of
`tests/test_search_params.cpp`, `tools/mutants/S114_null_move_term.py`
deleted, M03's anchor re-pointed, `MANUAL.md` and `DEV_MANUAL.md` by hand.
`adocs/data/S170_cases.tsv` and `tests/test_mate_carry.cpp` are `b3876f8`'s:
the budgets are their own patch (below).

### Deviations and findings, first

1. **The brief's "what stays" list names two things the row does not, and
   both left.** The brief keeps "M02's depth 15 case as repaired" and "the
   fixture's precondition assert"; the pre-registration's H0 row keeps the
   gate's switch, clause and row, the floor's place and the derived
   `NULL_DRIVE_DEPTH` and nothing else, and its item 16 -- which the brief's
   own first paragraph and the launch message repeat -- sends "the defender
   case's depth" back byte for byte. The defender case *is* M02's case ("no
   defender node inside the mate band makes a null move"), so the two
   sentences conflict, and the row was followed. Both repairs read the term's
   constants, which leave: the depth-15 search is `capped_null_plies` over
   `NULL_MOVE_EVAL_CAP`, and the fixture's assertion is `evaluate() - beta <
   NULL_MOVE_EVAL_MARGIN`. Without the term the reduction the fixture's floor
   checks is the whole reduction, so the assertion guards nothing, and at
   `NULL_DRIVE_DEPTH` the band guard is again the only thing between a
   defender node and a pass, which is what the restored depth needs: **M02
   applied by hand is red there** at `REQUIRE( violations.empty() )` and green
   shipped (`.tuning/coord/S114_rm/hand_checks.log`). If the coordinator reads
   the brief's list as meant, both come back as a small follow-up; nothing
   else here depends on the choice.
2. **`assert(static_eval != TT_EVAL_NONE)` in the null-move block left with
   the term.** Its comment gave the term as its reason ("The term reads the
   number, so that is asserted rather than assumed") and the row does not
   list it. The gate's clause reads `static_eval` too, but inside the
   condition and after `!is_in_check`, whose short-circuit is what keeps the
   sentinel out; a Release build compiles the assert away, so INV-6 cannot
   see the choice either way.
3. **M03's anchor was re-pointed a third time.** The assert's comment was the
   line after the null-move condition that `tools/mutants/search.py`'s M03
   anchored on; it now anchors on the block's first comment line as it
   stands, the mutation unchanged, the file's note saying so. E10's anchor,
   the gate's clause, is untouched and valid. **M03 and E10 applied by hand
   are red** at their own cases, "a node with only kings and pawns makes no
   null move" and "an excluded node makes no null move", and green shipped
   (`hand_checks.log`).
4. **One citation in this file's earlier sections went stale** with the case
   it named: "The demolition, on this tree" cited `tests/test_search.cpp` with
   the demolition case's title, which `--citations` then flagged MISSING. The
   sentence now says in the past tense that the case's comment named that log
   until the case left; no other word of the earlier sections moved.
5. **The S170 patch is at `/home/max/ws/chesso-s114rm/.tuning/coord/S114_rm/s170_budgets.patch`**,
   in this worktree, and not in the main tree's `.tuning/coord/S114_rm/` as the
   brief wrote: the launch message allowed no write under `/home/max/ws/chesso`.
   One `cp` puts it where the brief said.
6. **Kept and amended, not restored**: `src/search_params.hpp`'s paragraph on
   the base and divisor seeds (form (b), S085's SPSA output, and DEC-244's
   sweep that re-decided nothing) stays, since both parameters stay; its
   "S127's input when it fits these two with the two below" now says the table
   was taken with the term in, at 94 and 8, and that the term has left. The
   `RfpTtEstimate` comment's routing sentence reads as `d946b6f`'s with the
   gate named ("no static-score condition ... while `NullMoveEvalGate` is at
   0"), the wording proposed for `specs.md` below.

### What left

| what | where |
|---|---|
| the static-score term -- `null_eval_term`, `min(max(static_eval - beta, 0) / NULL_MOVE_EVAL_MARGIN, NULL_MOVE_EVAL_CAP)` -- and its share of `null_reduction`; the probe write of the reduction; the `assert(static_eval != TT_EVAL_NONE)` and its comment; the floor comment's S114 paragraph ("The whole reduction is tested, the static-score term included") | `src/search.cpp` `negamax_at`, the null-move block |
| `NULL_MOVE_EVAL_MARGIN` ("NullMoveEvalMargin", 94, 1 to 2000) and `NULL_MOVE_EVAL_CAP` ("NullMoveEvalCap", 8, 0 to 16) with their comment block; the three-part R in the base and divisor comment | `src/search_params.hpp`, the one X-macro both builds read |
| the probe field `null_reduction`: the file is `d946b6f`'s byte for byte | `src/data_structures.hpp` `search_node_probe_t` |
| the six S114 cases of "search: pruning and reduction guards" with their fixture `null_eval_drive_t`, `NULL_EVAL_POS` and `NULL_EVAL_NODE_LIMIT`: "the null move reduction adds a ply per margin the static score stands above beta", "a static score below beta takes nothing off the null move reduction", "the floor refuses a pass the static-score term leaves no ply for", "the static-score term is never computed at a node in check", "the static-score term does not hide a forced mate", and the tune-only "the static-score term moves the tree above beta and holds it still below"; the defender case's depth search (`capped_null_plies`, its loop, its `REQUIRE_EQ` and the per-row term-at-cap check); the fixture's term assertion in `require_null_move_preconditions` | `tests/test_search.cpp` |
| the two golden rows, 72 back to 70 | `tests/test_search_params.cpp` `golden_defaults` |
| NT01 to NT06, the file deleted | `tools/mutants/S114_null_move_term.py` |
| the `NullMoveEvalMargin` and `NullMoveEvalCap` option rows; the `NullMoveDivisor` row back to `d946b6f`'s | `MANUAL.md` |

### What stayed, and why

The row's own words, written before the first game: "**One reason to keep
code is stated now**: the gate's switch, its clause and its row stay at 0,
because S114's second verdict flips them on the tree the first leaves and adds
no code then (DEC-243); so does the floor's place inside the block, which at
cap 0 tests the reduction before S114 exactly, and the derived
`NULL_DRIVE_DEPTH`." So:

- `NULL_MOVE_EVAL_GATE` ("NullMoveEvalGate", 0, 0 to 1) in
  `src/search_params.hpp`, its comment rewritten for a tree without the term;
  its clause `(NULL_MOVE_EVAL_GATE == 0 || static_eval >= beta)` in the
  condition of `src/search.cpp` `negamax_at` and its entry in the guard list,
  unchanged; its `golden_defaults` row and its `MANUAL.md` row, whose one
  sentence about the term now says the term left.
- The floor `depth - 1 - null_reduction >= 1` inside the block, the reduction
  computed there as `NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR`, with one
  sentence saying why it sits there: every guard is a pure condition, so where
  the floor sits changes no node. A comment where the term's line was says
  S114 tried it and what its SPRT read, in the form S237's, S238's and S022's
  sites took.
- `NULL_DRIVE_DEPTH`, derived by `null_drive_depth_for` from the base and the
  divisor, 5 at the seeds; its comment loses the term's clauses and says it
  stayed by the row.
- The evidence: `adocs/data/S114_demolition_row.py`,
  `S114_null_move_sweep.py`, `S114_null_move_sweep_d11.tsv`,
  `S114_remine_s097.log`, `S114_rb_remine_s097.log`, `S114_sprt.sh`,
  `S114_sweep_s170.txt` and their `adocs/data/README.md` rows, untouched;
  `DEV_MANUAL.md`'s two landing paragraphs in the bench ledger, which are
  their record; this file's earlier sections.

### The goldens' return (DEC-142, DEC-233)

1. **The multicut row** of `tests/test_search.cpp` "pruning does not hide a
   forced mate" (`mate_the_multicut_hides`). The landing's row,
   `R2Q1bk1/5q2/4bP1p/2p1P3/3pB3/7P/2P3PK/8 w - - 3 50`, depth 13, `mate_in ==
   5`, becomes S131's row as `d946b6f` has it,
   `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42`, depth 14, `mate_in
   == 6`, with the title's depth and the GOLDEN line ("the depth 14 below")
   back too. The GOLDEN block keeps the re-mine as history: the landing's
   paragraph becomes "RE-MINED AT S114'S FIRST VERDICT, IN THE SCRIPT'S GUARD
   MODE, AND RESTORED WHEN THE TERM LEFT", the rows list keeps S114's row as
   its own entry, and S131's entry reads "on `58585f8`, again after S022
   verdict 2's reading, and again after S114 verdict 1's". **S131's row
   separates E21 again on this tree**: E21 applied by hand to a copy of the
   reverted `src/`, and the case fails at the row's `REQUIRE(
   result.mate_found )`, logged "mate the multicut hides, depth 14", after 40
   of its 41 assertions passed; green shipped (`hand_checks.log`).
2. **The capture-mate table** of the same case went back to `d946b6f`'s rows
   first, a revert and not a re-mine -- from the landing's
   `{row 1, 7, 5, "C02, C05, R02, since S112"}`,
   `{row 2, 7, 5, "no S091 mutant, since S095"}`,
   `{row 3, 10, 4, "C05, R02, since S114"}`,
   `{row 4, 9, 5, "C02, C05, C07, since S114"}` to
   `{row 1, 7, 5, "C02, C05, R02, since S112"}`,
   `{row 2, 9, 5, "no S091 mutant, since S095"}`, `{row 3, 11, 4, "R02"}`,
   `{row 4, 10, 5, "no S091 mutant, since S113"}` -- and S248 then re-derived
   those, the next subsection. The landing's S114 paragraph at the table
   becomes the record of both moves.
3. **The defender case's depth**, "no defender node inside the mate band makes
   a null move": from the shallowest depth the capped term leaves a ply at, 15
   at the seeds, back to `NULL_DRIVE_DEPTH`, the case `d946b6f`'s text byte
   for byte (compared in-process), and so is `require_null_move_preconditions`.
   Red under M02 at the restored depth (deviation 1).
4. **`golden_defaults`**: 72 rows back to 70, the gate's row staying; the
   history comment records the three coming and two going.

### S248's rows, re-derived on the reverted tree

`adocs/data/S230_mine_r01_row.py depths` over `adocs/data/S230_table_fens.txt`
at depths 3 to 12, the shipped library and one per mutant of
`tools/mutants/S091_capture_see.py`, each applied to the throwaway worktree
`.ref-builds/d946b6f` after a plain copy of this worktree's `src/` into it, and
reverted by copying `src/search.cpp` back (driver
`.tuning/coord/S114_rm/capmates.sh`, sweeps and build logs in `capmates/`).
The rule the table's GOLDEN block states, applied to every row: the lowest
depth in the shipped profile at which some mutant loses the mate, else the
lowest depth in the profile, said so in the label.

| row | shipped profile | the mutants losing it at that depth | restored (`d946b6f`) | re-derived |
|---|---|---|---|---|
| 1 | `d7 d9 d10 d11 d12` | C02, C05, R02 at 7 | 7, 5, "C02, C05, R02, since S112" | 7, 5, "C02, C05, R02, since S112" |
| 2 | `d7 d8 d9 d10 d11 d12` | none at any depth | 9, 5, "no S091 mutant, since S095" | **7**, 5, "no S091 mutant, since S095" |
| 3 | `d10 d11 d12` | C02 and R02 (`d12` each) at 10 | 11, 4, "R02" | **10**, 4, **"C02, R02, since S248"** |
| 4 | `d10 d11 d12` | R02 (`d8 d9 d11 d12`) at 10 | 10, 5, "no S091 mutant, since S113" | 10, 5, **"R02, since S248"** |

No mate distance moved. C06, C07 and R01 are separated by no row at any depth;
C02 gains row 4's depth 9 reading, a mutant finding the mate earlier and not a
separation. **All seven sweeps are byte for byte the ones S114's rebase took on
its parent** (`.tuning/coord/S114b_logs/capmates/parent_*`, on `db1905f`,
whose `src/` differs from `d946b6f`'s by one comment): the four positions
read the same on both trees under every S091 mutant, evidence on these four
positions and no more.
The site quotes the restored rows and the re-derived ones in a new "Re-derived
at S248" paragraph, row 4's own comment says S114 moved it and S248 keeps it,
and `DEV_MANUAL.md`'s golden row leads with 7, 7, 10, 10. S248's step file is
untouched; the coordinator completes it on this report.

### The proofs

**INV-6 identity to `d946b6f`'s engine** (DEC-215), against a Release build
of `d946b6f` made in the throwaway worktree `.ref-builds/d946b6f` (`git
worktree add --detach`, clean, doctest cloned from the main tree's module),
its binary kept as `.tuning/coord/S114_rm/ref_d946b6f_chesso`, sha256
`dd15ae13...`; this worktree's Release binary `e0276651...`, the same bytes
before and after the site comment took the reading's figures:

| | |
|---|---|
| `bench` | **3429473**, the whole stream -- all 112 `info` lines' depth, score, nodes and PV and all eight `bestmove` replies, c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 -- identical with `time` and `nps` stripped, 121 lines (`bench_ref.txt`, `bench_wt.txt`, the `_stripped` pair) |
| `bench 12` | **1694808**, its 105-line stream identical the same way, the same eight replies |
| `tools/search_bench.py` | node for node and move for move: depth 9 48304 / 71580 / 25413, depth 12 104784 / 244824 / 117798, best c3d5 e2a6 d7c8q at both (`sb9_*`, `sb12_*`) |
| the tune build at defaults | `bench` 3429473 with its whole 121-line stream identical to the reference binary's the same way (`bench_tune.txt`, `tune_checks.log`) |
| the tune build's options | 75 option lines, 70 of them the search table's: `NullMoveBase`, `NullMoveDivisor` and `NullMoveEvalGate` (at 0 on that tree, range 0 to 1) among them and neither `NullMoveEvalMargin` nor `NullMoveEvalCap`; the Release build lists 5 (`tune_uci.txt`) |

The remaining difference to `d946b6f` in `src/`: the gate's row, clause and
guard-list entry, the floor inside the block, the site comment, and in
`src/search_params.hpp` the base and divisor seeds' paragraph, the gate's
comment and the `RfpTtEstimate` sentence. In `tests/`: the derived
`NULL_DRIVE_DEPTH`, the gate's golden row, the record paragraphs and S248's
rows at the goldens, and `ad18661`'s sentence in `tests/test_mate_carry.cpp`,
which the budgets patch moves. In `tools/`: M03's anchor and E10's (S114's,
still valid). `MANUAL.md`: the gate's row. `adocs/data/S170_cases.tsv` is
`ad18661`'s, S114's budgets, until the budgets patch.

### Suites and checks

| | |
|---|---|
| both fast suites | niced, serial `ctest -L fast`, one build at a time, `-j8` builds after the match: **41 of 41 each** -- Release `build` in 109.0 s, `test_mate_carry` 27.36 s against its 120 s ceiling, and `-DCHESSO_TUNE=ON` `build-tune` in 112.3 s, `test_mate_carry` 27.19 s, both at the TSV's S114 budgets (`suite_build_run1.log`, `suite_build-tune_run1.log`). Run again on the final tree, this section in it but for these figures: 41 of 41 each, Release 99.6 s and tune 100.9 s, `test_mate_carry` 25.03 s and 25.42 s (`suite_build_final.log`, `suite_build-tune_final.log`) |
| format | `./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22` |
| prose | `tools/plan_prose_check.py`, one mode per invocation, on the final tree with this section in it: `--citations` 0 flagged over 38 files, this file's 33 code citations among them, `--touches` 0 flagged over 38 files, `--params` exit 0 with no output (`prose--citations.log`, `prose--touches.log`, `prose--params.log`) |
| mutant anchors | `tools/mutation_check.py` has no validate-only flag, so its own `load_mutants` and `validate` were run in-process against the working tree over the whole of `tools/mutants/`: **156 mutants in 19 files**, every anchor unique, ids distinct, no NT id left (`mutant_anchors.log`) -- 162 in 20 on the landing |
| hand-applied mutants | on a copy of the reverted `src/` and `tests/` in the throwaway, each green shipped and red under its mutant at its own case: E21 on S131's restored row, M02 on the defender case at the restored depth, M03 on its re-pointed anchor, E10 on the gate's clause (`hand_checks.log`) |
| the UCI surface | `test_uci_surface` green in both builds with **no refresh**: the Release build's 5 options are unchanged, and the tune build's lines are generated from the same table, 75 with the two gone |

### S170's budgets, re-derived on the reverted tree -- a separate patch (DEC-156 as DEC-162 left it)

`adocs/data/S203_case_sweep.sh` with no argument, niced, on a copy of this
tree's Release build (`.tuning/coord/S114_rm/s170/chesso`, the INV-6
binary's bytes), the full 108-cell grid in 16.5 minutes, 07:34 to 07:51.
**The rule, stated before the grid was read** in the run's own script
header (`s170/run_sweep.sh`): each row takes the cheapest cell at its own
stride that reports a mate line, on the mate count alone, the short lines the
ceilings' and never a reason to refuse a cell. Applied by `s170/pick.py`,
which was checked first on S114's own grid, `adocs/data/S114_sweep_s170.txt`,
where it returns S114's six budgets.

| case | stride | old budget (S114's) | old cell now | new budget | its cell (mates / short) |
|---|---|---|---|---|---|
| A_mate8_shallow | 1 | 300000 | **0 / 0** | **500000** | 2 / 0 |
| B_mate6_shallow | 1 | 100000 | 15 / 0 | 100000 | 15 / 0 |
| C_mate7_depth11 | 1 | 1200000 | **0 / 0** | **500000** | 2 / 0 |
| D_mate_minus6_depth10 | 1 | 1200000 | 3 / 0 | 1200000 | 3 / 0 |
| E_mate_minus9 | 1 | 300000 | 2 / 0 | 300000 | 2 / 0 |
| F_mate6_inherited_no_line | 2 | 100000 | 4 / 0 | 100000 | 4 / 0 |

**At S114's budgets two of the five guarded cases are silent on this tree**,
A and C, which DEC-162 makes green by design -- three silent is red -- so the
removal's `test_mate_carry` is green on the TSV it leaves unchanged (the
suites above), and the budgets commit is what brings all five back. `--at` on
the patched TSV reproduces the six cells exactly (`s170/at.txt`), and
`test_mate_carry` at the new budgets is green in both builds, 20.95 s and
21.50 s, 55 assertions each (`s170/carry_check.log`), the patch applied for
the check and reversed, every file's sha256 back to its value before.
**One ceiling reading moved, and no ceiling was changed**: `--ceilings` over
the four recorded grids answers the shipped 5, 15, 0, 2, 11, 5; with this
grid added it would answer 5, 15, **1**, 2, 11, 5, since C's cell at 2000000
reports 4 lines with 1 short; this grid alone reads 0, 8, 1, 0, 1, 1
(`s170/ceilings_*.txt`). S245's and S114's re-sweep grids are not in the
command `tests/test_mate_carry.cpp` names and neither is this one, so C's
ceiling stays 0 -- adding the grid would raise a ceiling, which that file
calls relaxing a test, a decision -- and C's own budget cell reports 0 short.

The patch, `/home/max/ws/chesso-s114rm/.tuning/coord/S114_rm/s170_budgets.patch`
(the same bytes in `s170/`), holds the TSV with a paragraph quoting S114's
rows, the grid as `adocs/data/S114_rm_sweep_s170.txt`, its
`adocs/data/README.md` row, the sentence of `tests/test_mate_carry.cpp` about
E's cell, which this grid made false (all 10 short on S114's tree, 2 with 0
short here), and `DEV_MANUAL.md`'s sentence naming the steps that re-swept the
budgets. It applies on this worktree as it stands, and on `51ec924` with the
removal's `DEV_MANUAL.md` -- its README hunk carries one line of context so
the record commit's two new rows above it do not stop it -- and lands as its
own commit after the removal's. A commit text for it, if wanted:

```
Re-derive the S170 mate-carry budgets on S114's removal tree (DEC-162)

S114 verdict 1's term left on its H0, so the budgets ad18661 derived on
the term's tree are not this tree's. DEC-156 as amended by DEC-162
re-derives S170_cases.tsv's go budgets after anything that moves the
tree, by one rule stated before the grid was read: each row takes the
cheapest budget at its own stride whose cell reports a mate line,
chosen on the mate count alone. adocs/data/S203_case_sweep.sh over the
full 108-cell grid on a copy of the removal's Release build, tracked as
adocs/data/S114_rm_sweep_s170.txt with its README row.

New budgets: A 300000 -> 500000 and C 1200000 -> 500000, both old cells
silent on this tree; B, D, E, and F at its stride 2, unchanged. --at
reproduces the six cells; test_mate_carry at the new budgets is green in
both builds. --ceilings over the four recorded grids still answers 5,
15, 0, 2, 11, 5; this grid is not in that command. The TSV quotes the
old rows; test_mate_carry.cpp's sentence on E's cell and DEV_MANUAL's
list of the re-sweeps read true again.
```

### Not run here

- DEC-141's second tier -- the Debug self-play, which is a match, and
  `tools/gate_extra.sh` -- by the brief; the coordinator's on the idle
  machine. The Debug guard suite, not required by the brief, was not run.
- `tools/mutation_check.py` on a fixture: its fixture is a clean commit in a
  throwaway worktree, which the landed tree gives the coordinator; the four
  hand-applied mutants above stand in for the touched ones.
- The budgets' landing: the patch below is prepared and checked, not applied.
- Any timing.

### Proposed `adocs/specs.md` edits, for the coordinator

Quoted against `specs.md` at `b3876f8`, both in the `search` row, both for the
removal commit.

**1. The S114 sentence**, from "**The null move's reduction grows with the
static score's lead over beta since S114**" through "<verdict> -- `bench`
3429473 -> 4192793." inclusive -- or whatever the record commit wrote in
place of `<verdict>` -- becomes, in the form S022 verdict 2's took:

> **A static-score term in the null move's reduction was tried and left, S114
> verdict 1, landed and removed 2026-09-30 (DEC-243)**: past the block's
> guards R was `NullMoveBase + depth / NullMoveDivisor` (3 + depth/6, S085's
> fit, kept by DEC-244) plus one ply for every whole `NullMoveEvalMargin` (94,
> a pawn) the node's raw static evaluation stood above beta, at most
> `NullMoveEvalCap` (8), clamped at zero, the floor `depth - 1 - R >= 1`
> testing the whole of it, so a lead that would have left the null search no
> ply made the pass impossible and the node was searched; `NullMoveEvalCap` 0
> gave the tree before S114 node for node (DEC-215). Before any game it moved
> `bench` 3429473 -> 4192793 and the fixed-node depths 47 -> 48 -- reach, not
> a forecast (DEC-239). Its one `{0, 5}` nElo SPRT against the tree without it
> (`adocs/data/S114_sprt.sh`) **accepted H0 at 2720 games on 2026-09-30 --
> `Elo -18.54 +/- 10.19`, `nElo -23.81 +/- 13.06`, LLR -2.95, 0 forfeits**
> (`adocs/data/S114_sprt.log`), an interval wholly below zero, a loss: the
> term's code left with its margin and its cap, and the goldens re-derived for
> the candidate were restored with it -- `bench` 3429473, bench-identical to
> `d946b6f`'s (INV-6). The entry gate `static_eval >= beta` stays in the tree
> behind `NullMoveEvalGate` at 0, S114's second verdict, and the floor stays
> inside the block, where without the term it tests what it tested before
> S114.

**2. The routing clause** in the reverse-futility estimate's sentence: "This
engine's null-move term reads the raw static evaluation, never the estimate
(S114), and razoring does not exist until S116, so reverse futility is the
whole of the routing." becomes:

> This engine's null-move block has no static-score condition until S114's
> second verdict flips its gate, `NullMoveEvalGate`, which reads the raw static
> evaluation and never the estimate, and razoring does not exist until S116,
> so reverse futility is the whole of the routing.

### Proposed commit text, for the coordinator

Not a verdict-closing commit: the record commit carries the DEC-220 block.
The coordinator appends its trailer after the `Bench:` line.

```
Remove S114 v1's static-score null-move term on its H0

The null move's static-score term lost. The SPRT of 794e4c3, the term
at NullMoveEvalCap 8 with NullMoveEvalGate 0, against d946b6f, the tree
without it, {0, 5} nElo at 8+0.08, accepted H0 at 2720 games: nElo
-23.81 +/- 13.06, an interval wholly below zero.
adocs/data/S114_sprt.sh wrote that reading and its consequence before
the first game -- the cap to its off value and the term's code out with
it, the S238 pattern, the goldens the landing re-derived back byte for
byte -- so this is the pre-registration executed.

Out: the term and its share of the reduction in negamax_at, the probe
write of the reduction and the assert that guarded the term's input;
NULL_MOVE_EVAL_MARGIN and NULL_MOVE_EVAL_CAP with their comment;
search_node_probe_t's null_reduction field; the six S114 cases and
their fixture, the defender case's depth search and the guard
fixture's term assertion; the two golden_defaults rows, 72 back to 70;
tools/mutants/S114_null_move_term.py, NT01 to NT06; the two MANUAL
option rows.

Stays, by the row: NullMoveEvalGate at 0 with its clause and its row,
since S114's second verdict flips it on this tree with no code
(DEC-243); the floor inside the block, which without the term tests
what it tested before S114; NULL_DRIVE_DEPTH derived from the base and
the divisor. M03's anchor is re-pointed at the block's first line as
it now stands, the mutation unchanged.

Back to d946b6f's: S131's multicut row of "pruning does not hide a
forced mate" at depth 14, mate in 6, which E21 applied by hand turns
red again; the defender case at NULL_DRIVE_DEPTH, which M02 turns red;
and the capture-mate table's rows, stale on d946b6f already, so S248
re-derived them on this tree by adocs/data/S230_mine_r01_row.py, the
seven sweeps byte for byte the parent sweeps of S114's rebase: 7, 9,
11, 10 -> 7, 7, 10, 10, rows 3 and 4 labelled C02, R02 and R02. Each
site records the move and its return (DEC-142, DEC-233); DEV_MANUAL's
ledger gains the removal's entry and its three golden rows read true
again.

bench is 3429473 with the whole bench stream and all eight bestmove
replies identical to a Release build of d946b6f, bench 12 likewise, and
tools/search_bench.py identical at depths 9 and 12 (INV-6, DEC-215);
the tune build at its defaults prints the same stream and lists
NullMoveEvalGate without the margin and the cap. No second SPRT is
owed. Both fast suites 41 of 41, format and the three prose checks
clean, all 156 remaining mutant anchors unique. The S170 budgets are
re-derived on this tree as their own commit, after this one.

Bench: 3429473
```

### Second tier on the removal tree (the coordinator, 2026-09-30)

DEC-141's second tier on the tree verdict 1's removal leaves, run on
`f5eaa99` (the removal `f3868fb` plus the S170 budgets): Debug self-play of 8
games at 4+0.04, 0 `Assertion`, 0 `disconnect`
(`.tuning/coord/S114rm_debug_selfplay/`); `tools/gate_extra.sh` 5 stages green
in 980 s (`.tuning/gate_extra_2026-09-30_S114rm.log`). S115 landed on this
tree as `eb334e3` and is measured first; this step's second verdict, the gate
flip (DEC-243), follows it.


## Verdict 2, as built: the entry gate

Written by a fresh Opus agent on the linked worktree `s114v2` at `465b43b`,
the tree S115 leaves, briefed from `.tuning/coord/S114_v2_brief.md`, on the
idle machine. Nothing was committed. Logs are under the main tree's
`.tuning/coord/S114_v2/`, cited below by file name. Implemented from this
file's description (DEC-221); no other engine's code was opened.

**The change.** `NullMoveEvalGate` 0 -> 1 in `src/search_params.hpp`'s
X-macro, and the comments that describe it: the gate's own block, the
`RfpTtEstimate` routing sentence, and the guard-list entry above the
null-move condition in `src/search.cpp` `negamax_at`, which now also says
that in check `static_eval` is `TT_EVAL_NONE` and `!is_in_check`
short-circuits before the gate reads it. No other line of `src/` moves.

### Deviations and findings, first

1. **The S170 budgets patch cannot land as the rule gives it.** On this tree
   the rule moves A 500000 -> 100000, C 1000000 -> 500000 and D 3000000 ->
   1000000, and C's new cell reports **4 mate lines, all 4 short**, over its
   ceiling of 0: `test_mate_carry` is red in both builds at `CHECK(
   short_lines <= short_line_ceiling(game.name) )`, `CHECK( 4 <= 0 )`
   (`s170/carry_build.log`, `s170/carry_build-tune.log`). Raising the ceiling
   is relaxing a test, a decision, so the patch is prepared at the rule's
   answer and marked red, not repaired. Measured for the decision, not
   chosen: with C held at its current 1000000 (1 line, 0 short) and A and D
   moved, `test_mate_carry` is green in both builds
   (`s170/carry_optionC1M_*.log`). **At the standing budgets the landing is
   green**: the TSV is unchanged in the worktree and both fast suites pass.
2. **The sentinel assertion lives in the test, not in `src/`.** Item 4 allowed
   keeping or adding `assert(static_eval != TT_EVAL_NONE)`; it was not added,
   since the engine change is the default and its comments. Instead "an
   in-check node makes no null move" now drives beta at `TT_EVAL_NONE`,
   where the gate would admit the sentinel, so `!is_in_check` is the only
   thing that keeps the gate from reading it -- observable in Release, and
   M01 crashes that case (SIGSEGV) where before the gate hid it. If the
   coordinator wants the Debug assert as well, it is one line inside the
   block, placed after the first comment line so M03's anchor holds.
3. **Two mined goldens moved, both re-derived by their own scripts.** The
   multicut row stopped separating E21 and was re-mined in guard mode to a
   new board at the same depth and distance. Capture-mate row 4's label moved;
   its depth did not (below).
4. **M02 was not blind on this tree.** It was blind on the first build's
   tree because of the static-score term. Here the defender case is red
   under M02 at `NULL_DRIVE_DEPTH`, so the case needed no repair.
5. **N02 is declared equivalent in the release build**
   (`tools/mutants/S191_guards.py`), with a tune-only leg at gate 0 observed
   red under N02 (`n02_tune_leg.log`) -- option A's (5), S113's B15 form.
6. **One stale sentence in this file's removal section** -- the gate's
   range quoted with its old default -- turned `test_plan_params` red at gate 1. It now
   reads "(at 0 on that tree, range 0 to 1)". No other word of the earlier
   sections moved.
7. **The fixed-node total does not move, but the positions do**: 48 -> 48,
   the midgame two plies deeper and kiwipete two shallower, with its move
   changing. Reach, not a forecast (DEC-239).

### Every red at gate 1 before any test was touched

`fast_release_1.log`, the Release build at gate 1 on the unrepaired tests.
Three binaries failed:

- `test_search`, four guard cases, each at `REQUIRE( probe.null_move_made )`,
  `values: REQUIRE( false )`: "the node after a null move has no previous
  move to index" (H03), "a null-move fail-high against a mate returns the
  bound" (M04), "the child after a null move is labelled the type the parent
  is not" (T08) and "an excluded node makes no null move" (E10). These are
  the four the "Stopped" section named, re-observed on this tree.
- `test_search_params`: `golden_defaults`, `CHECK( 1 == 0 )`, the gate's row.
- `test_plan_params`: `MANUAL.md`'s row and deviation 6's sentence.

**Green but blind, observed by hand**: each mutant was applied to a
throwaway copy of the gate-1 tree and the guard suite run
(`probe_mutants_before_repair.log`). **M01, M03 and N02**: no case of their
own went red. **N01**: its own case stayed green, and it was killed only
through the multicut and ProbCut cases. **M02**: killed by its own case
(deviation 4). No mate case, and nothing outside the null-move premise, went
red.

### The repairs (DEC-233's second repair; every assertion kept)

- `require_null_move_preconditions` also asserts the gate:
  `REQUIRE((NULL_MOVE_EVAL_GATE == 0 || evaluate(&game.board) >= beta))`.
  This is option A's (1), minus the term.
- H03, T08, E10 and N01 now drive at the node's own static score,
  `evaluate()` on the wiped table, and no longer at `ORDINARY_BETA`. M03's
  kings-and-pawns drive does the same, and asserts the gate by hand.
  `ORDINARY_BETA`'s comment says why the null-move drives no longer use it.
- M01, the in-check case, drives beta at `TT_EVAL_NONE` and asserts `TT_EVAL_NONE >= beta`
  (deviation 2).
- M04 is the same board at the same depth, with beta at its static score,
  835. The hand-driven precondition still reads a mate: the null search
  returns 48996 at that beta, so reverse futility at the mating node does
  not fire. The gate's precondition is asserted beside it.
- N02 keeps its release leg and gains the tune-only leg (deviation 5).
- No anchor moved. `tools/mutation_check.py` validates all 163 mutants.

**Each repaired case's mutant, re-observed killed**, by hand on a throwaway
copy of the repaired tree (`probe_mutants_after_repair.log`;
`probe_shipped_after_repair.log` is the unmutated run, 83 of 83):

| mutant | red at |
|---|---|
| M01 | its own case: SIGSEGV |
| M02 | the defender case |
| M03 | its own case |
| M04 | `REQUIRE_EQ( score, beta )`, 48996 against 835 |
| N01 | its own case |
| H03 | `CHECK_EQ( white_movers_under_prev, 0 )`, 1 against 0 |
| T08 | its own case and the walk |
| E10 | its own case |
| N02 | survives in Release, as declared; red in the tune build |

The first `shipped` entry of `probe_mutants_after_repair.log` is red with
R02's six failures: `capmates.sh` held R02 in the same `.ref-builds/obs` tree
while the probe started. `probe_shipped_after_repair.log` is the clean
baseline, and the R02 capture sweep was re-taken alone
(`capmates/R02_rerun.txt`). The re-run was one build of a fresh throwaway
worktree holding the candidate's `src/` with R02 applied, and its library
was passed to `S230_mine_r01_row.py depths` by absolute path. **Its profile
is byte for byte the original's**: `d9 d10 d11 d12`, `d7 d8 d9 d10 d11
d12`, `d12` and `d8 d9 d10 d11 d12`. So row 4's new label stands, and no
row moved.

A first attempt passed the library by a relative path. The script joins
that path to the repository root, so it swept the worktree's shipped
library and printed the shipped profile. That file was discarded, not kept
as evidence.

### The gate's own guard (DEC-141 clause 2)

The new case "the null move's entry gate refuses a static score below beta"
drives the castled pawn wall twice. At its own static score the pass is
made. One point above it the node reaches its move loop and makes no pass;
every other condition is asserted at that beta too.

`tools/mutants/S114_v2_entry_gate.py` holds three mutants, all red at this
case:

- NG01 drops the clause. Red at the leg one point above.
- NG02 inverts the comparison, `static_eval < beta`.
- NG03 moves the boundary, `static_eval > beta`.

The in-check sentinel is covered by M01's case (deviation 2).

### The off value (DEC-215)

The tune build at `NullMoveEvalGate` 0 is `465b43b`'s engine, node for node,
against a Release build of `465b43b` made in the throwaway worktree
`.ref-builds/465b43b` (deleted after; its binary is kept as
`chesso_465b43b`, sha256 `f5b7e895...`):

| instrument | result |
|---|---|
| `bench` | **3513310** with all eight replies c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6. The whole 121-line stream is identical with time and nps stripped, apart from the final summary line's nps |
| `bench 12` | 1619863, its 105-line stream identical the same way |
| `tools/search_bench.py` | 34236 / 71552 / 25351 at depth 9 and 70283 / 240680 / 80264 at depth 12, best c3d5 e2a6 d7c8q at both. Driven through `search_bench_opt.py`, which runs `search_bench`'s own `run` with `setoption` after `uciok` |

The tune build at its defaults prints the Release candidate's bench stream
line for line (`bench_*`, `sb*_*`).

### The counts, parent -> candidate

| instrument | parent (`465b43b`) | candidate |
|---|---|---|
| `bench` | 3513310 | **4041913** (+15.05 %) |
| `bench` replies | c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 | c3d5 **d5e6** d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 |
| `bench 12` | 1619863 | 1776047 (+9.64 %), replies unchanged |
| `search_bench` 9 | 34236 / 71552 / 25351, c3d5 e2a6 d7c8q | 33312 / 64982 / 25242, same moves |
| `search_bench` 12 | 70283 / 240680 / 80264, c3d5 e2a6 d7c8q | 71035 / 178581 / 110744, same moves |
| fixed-node depth, `go nodes 1000000`, Hash 16 | 17 / 15 / 16 = 48, c3d5 e2a6 d7c8q | 19 / 13 / 16 = 48, c3d5 **d5e6** d7c8q |

The fixed-node depths are `adocs/data/S097_fixed_node_depth.py` (`fnd_*`),
stated as reach (DEC-239).

### The goldens moved (DEC-142, DEC-233, DEC-238)

**The capture-mate table.** Seven sweeps were run: shipped plus C02, C05,
C06, C07, R01 and R02 of `tools/mutants/S091_capture_see.py`. Each was
`adocs/data/S230_mine_r01_row.py depths` at 3 to 12 on a throwaway copy of
the candidate's `src/` (`capmates.sh`, `capmates/`).

The shipped profiles are S248's, row for row: `d7 d9 d10 d11 d12`,
`d7 d8 d9 d10 d11 d12`, `d10 d11 d12`, `d10 d11 d12`. Depths 7, 7, 10 and 10
did not move, and no mate distance moved.

**Row 4's label moved**: `{..., 10, 5, "R02, since S248"}` ->
`{..., 10, 5, "no S091 mutant, since S114"}`. R02 now reads that mate from
8 to 12 without a gap. R02 is still killed by row 1 and by its direct
cases. The site has a "Re-derived at S114's second verdict" paragraph, and
`DEV_MANUAL.md`'s golden row has the same.

**The multicut row.** S131's row, `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1
w - - 2 42`, depth 14, mate in 6, **stayed green under E21**
(`e21_multicut_row.log`).

`adocs/data/S097_mine_mate_row.py` stages 2 to 6 were run in guard mode and
in default mode (`remine.sh`); stage 1 was not re-run, and the 269
candidates were byte-identical to the rebase's list. Two of the 269
separate. Both modes take **`4Q3/p7/2p2p2/P3n2k/7P/2P3P1/5q2/7K b - - 4
42`, depth 14, mate in 6**: shipped `d13 d14`, guard dropped `d13`, the
mutant's multicut firing at 11 to 14. The cell costs 1242164 nodes and
about 0.22 s. The other separator, S188's `1R6/8/2p3p1/P5P1/1p2b2P/4k3/
6pK/8 b - - 1 54`, separates on a run of one.

The new row's position, from tools and not from the board:

- python-chess: `is_valid()` True, `is_check()` True, 4 legal moves, no
  capture, no promotion, Black to move.
- stockfish at depth 20 and at depth 30, each a fresh process through
  python-chess: `#+6` for Black, pv Kg4 Qe6+ Kxg3 Qg8+ Ng4 Qxg4+ Kxg4 h5 Kg3
  c4 Qf1#. python-chess reads the pv as ending in checkmate.

The row was **observed red under E21** at `REQUIRE( result.mate_found )` and
green shipped (`e21_multicut_row_new.log`). Evidence is in
`adocs/data/S114_v2_remine_s097.log`, with its README row.

**`golden_defaults`**: the gate's row is now `{"NullMoveEvalGate", 1, 0, 1}`.
The count stays 72, and the history comment says why.

### Mutation (DEC-141 clause 2)

`tools/mutation_check.py --only` over M01 M02 M03 M04 N01 N02 H03 T08 E10,
NG01 NG02 NG03, E21 and R02. The fixture was a fresh throwaway commit on no
branch, `8ff6acd`, made through a temporary index, in `.ref-builds/mut`
(deleted after). Header: `baseline green, 41 tests, bench 4041913`. All 163
anchors validated.

**Mutation score 13 of 13 (100 %)**: 13 killed, and N02 declared equivalent
(wall 1649 s; `mutation/mutation.log`, `mutation/logs/results.tsv`).

| mutant | killed by |
|---|---|
| E21 | both killers: the new multicut row and S243's direct case |
| NG01 | the new case, and the multicut row -- the row is this tree's, so at gate 0 it reads no mate at 14 |
| NG02, NG03 | the new case and every repaired null-move drive |
| M01 | a SIGSEGV in its own case |
| M02, M03, M04, N01, H03, T08, E10 | each by its own case |
| R02 | capture row 1 and its direct cases |

### The S170 budgets (DEC-156 as amended by DEC-162) -- a separate patch, red as the rule gives it

`adocs/data/S203_case_sweep.sh` was run with no argument on a copy of the
candidate's Release build (`s170/chesso`, sha256 `72749cc0...`): the full
108-cell grid, 16 minutes. **The rule, stated before the grid was read** in
`s170/run_sweep.sh`'s header: each row takes the cheapest cell at its own
stride that reports a mate line, chosen on the mate count alone. It was
applied by `s170/pick.py`, which returns S115's six budgets when given
S115's grid.

| case | stride | old (S115's) | old cell now | new | its cell (mates / short) |
|---|---|---|---|---|---|
| A | 1 | 500000 | 13 / 0 | **100000** | 5 / 0 |
| B | 1 | 100000 | 21 / 0 | 100000 | 21 / 0 |
| C | 1 | 1000000 | 1 / 0 | **500000** | **4 / 4, over ceiling 0** |
| D | 1 | 3000000 | 3 / 1 | **1000000** | 5 / 1 |
| E | 1 | 300000 | 3 / 0 | 300000 | 3 / 0 |
| F | 2 | 500000 | 12 / 0 | 500000 | 12 / 0 |

- `--at` on the patched TSV reproduces the six cells (`s170/at_new.txt`).
- `--ceilings` over the four recorded grids gives 5, 15, 0, 2, 11, 5. With
  this grid added it gives 5, 15, **4**, 2, 11, 5. This grid alone gives 0,
  1, 4, 1, 8, 2.
- `/home/max/ws/chesso/.tuning/coord/S114_v2/s170_budgets.patch` holds the
  TSV at the rule's answer, with a paragraph quoting S115's rows and saying
  it is red; the grid as `adocs/data/S114_v2_sweep_s170.txt`; its README
  row; and `DEV_MANUAL.md`'s list of re-sweeps. It applies on this worktree
  (`git apply --check`). **It is red at C and needs the coordinator's
  decision** (deviation 1). The worktree's TSV is S115's.

### Suites and checks

- **Both fast suites green on the final tree**: 41 of 41 in Release `build`
  and 41 of 41 in tune `build-tune` (`fast_release_final.log`,
  `fast_tune_final.log`).
- `./clang-format.sh --check` is clean with `CLANG_FORMAT_MAJOR=22`.
- `tools/plan_prose_check.py`, one mode per call: `--citations` 0 flagged,
  `--touches` 0 flagged, `--params` exit 0 (`prose--*.log`).
- **Debug**: the guard suite passes 83 of 83. The whole `test_search` binary
  passes 183 of 183, 810321 assertions (`debug_guards.log`,
  `debug_test_search_whole.log`).
- **After the cold fast check's three fixes** (the pre-registration's item
  3, the R02 re-run, and two comments that claimed a measurement not yet
  taken, now in the present tense): both fast suites 41 of 41 again
  (`fast_release_fix.log`, `fast_tune_fix.log`), format clean, and `bench`
  4041913 in both builds.
- Every mate case is green: "pruning does not hide a forced mate",
  `test_mate_carry`, `test_mate_breadth`, `test_mate_pv` and `test_engine`'s
  mate safety.

### Not run here

- DEC-141's second tier: the Debug self-play, which is a match, and
  `tools/gate_extra.sh`. Both were excluded by the brief.
- The SPRT.
- The full mutation pass over all 163 mutants. Only the 14 above were run.
- Any timing.

### The pre-registration

`adocs/data/S114_v2_sprt.sh`, in the form of `S115_sprt.sh`, with its README
row:

- `{0, 5}` nElo at 8+0.08, Hash 16, `noob_3moves.epd`, 12 of 12 cores, OUT
  under `.tuning/`.
- Worst-case games 41861 and 25591, 19.8 h and 12.1 h at 2110 games/h.
- The abort rule.
- `REF` and `CAND` are `PIN_ME` and the script refuses to run until they are
  pinned ("CAND = the landing commit's sha, REF = CAND^").
- The three readings, written before a game. H1 keeps the gate at 1. H0
  with the interval wholly below zero removes the switch, its clause, its
  row, the new case and NG01 to NG03; the re-derived goldens go back byte
  for byte, and the repaired drives stay, since their premise holds without
  the gate. A no-verdict, or an interval reaching above zero, reads as a
  zero the same way.
- The open findings: S247 and S249 are the open fillers, and item 3 names
  the S170 budgets and the `--ceilings` reading.

### Proposed `adocs/specs.md` edits, for the coordinator

1. The removal sentence "The entry gate `static_eval >= beta` stays in the
   tree behind `NullMoveEvalGate` at 0, S114's second verdict, and the floor
   stays inside the block, where without the term it tests what it tested
   before S114." becomes: **"Since S114's second verdict the null move is
   tried only where the node's raw static evaluation stands at or above
   beta, the entry gate `NullMoveEvalGate` at 1 (DEC-243); in check the
   evaluation is `TT_EVAL_NONE` and `!is_in_check` keeps the gate from
   reading it; `NullMoveEvalGate` 0 is the tree before it, node for node
   (DEC-215); `bench` 3513310 -> 4041913; its `{0, 5}` nElo SPRT
   (`adocs/data/S114_v2_sprt.sh`) is not yet read. The floor stays inside
   the block, where without the term it tests what it tested before
   S114."**
2. The routing clause "This engine's null-move block has no static-score
   condition until S114's second verdict flips its gate, `NullMoveEvalGate`,
   which reads the raw static evaluation and never the estimate, ..."
   becomes **"This engine's null-move block has one static-score condition,
   its entry gate `NullMoveEvalGate`, on since S114's second verdict, which
   reads the raw static evaluation and never the estimate, ..."**, the rest
   unchanged.

### Proposed commit (not verdict-closing)

```
Gate the null move on the static score reaching beta (S114 v2)

S114's second verdict (DEC-243): NullMoveEvalGate goes from 0 to 1, so
the null move is tried only where the node's raw static evaluation
stands at or above beta. The switch, its clause in negamax_at and its
row shipped with the first verdict and stayed at 0 through its H0 for
this; the engine change is the default and the comments that describe
it. In check static_eval is TT_EVAL_NONE and !is_in_check keeps the
gate from reading it. Implemented from the step file's description
(DEC-221); a verdict switch, no seed.

NullMoveEvalGate 0 is the parent node for node (DEC-215): the tune
build there prints 465b43b's bench 3513310 with all eight replies and
the whole stream, bench 12 1619863, and search_bench at depths 9 and
12. The gate: bench 3513310 -> 4041913 (+15.05 %), kiwipete's reply
e2a6 -> d5e6; fixed-node depths 48 -> 48, stated as reach (DEC-239).
The counts move, so adocs/data/S114_v2_sprt.sh measures it at {0, 5}
nElo against this commit's parent.

The gate took the premise from eight null-move guard cases (H03, M04,
T08, E10 red; M01, M03, N01, N02 blind), and DEC-233's second repair
restores it with every assertion kept: the drives at the node's own
static score, the in-check case at beta TT_EVAL_NONE where the gate
would admit the sentinel, M04 at its static score 835 where the null
search still returns a mate, N02 declared equivalent in the release
build with a tune-only leg at gate 0. Each mutant was re-observed
killed. A new case, "the null move's entry gate refuses a static score
below beta", and NG01 to NG03 in tools/mutants/S114_v2_entry_gate.py.

Two mined goldens were re-derived by their own scripts: the multicut
row of "pruning does not hide a forced mate" stopped separating E21 and
was re-mined in guard mode (DEC-238) to a new board at depth 14, mate in
6, red under E21 and green shipped; capture-mate row 4 keeps depth 10
and its label goes R02 -> no S091 mutant. Mutation 13 of 13 over the
touched and new mutants, N02 equivalent. Both fast suites 41 of 41;
format and the three prose checks are clean.

Bench: 4041913
```

### Verdict 2 landed and pinned (the coordinator, 2026-10-01)

Landed as `7c7328f` on `465b43b` from the worktree's working tree after a cold
fast check (FIX-FIRST on three text items, fixed by the implementing agent
before the landing; the R02 capture sweep re-taken alone matched byte for
byte). `bench` 3513310 -> 4041913. **Second tier on the landing** (DEC-141):
Debug self-play of 8 games at 4+0.04 on `7c7328f`, 0 `Assertion`, 0
`disconnect` (`.tuning/coord/S114_v2_debug_selfplay/`); `tools/gate_extra.sh`
5 stages green in 1026 s (`.tuning/gate_extra_2026-10-01_S114v2.log`). SPRT
pair pinned: `REF` `465b43b`, `CAND` `7c7328f`. **The S170 budgets are not
re-derived on this tree**: the rule's cell for C carries 4 short lines over its
ceiling of 0, the patch (`.tuning/coord/S114_v2/s170_budgets.patch`) waits on
the owner, and the standing budgets are green; open finding 3 of the
pre-registration says so.

