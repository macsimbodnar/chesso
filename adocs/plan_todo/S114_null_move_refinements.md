id:         S114
goal:       the null move reduction scales with how far the static score is above beta, and the base reduction is re-decided
accepts:    an SPRT verdict, recorded whatever it is; **the static-score term is capped** -- uncapped, a position twenty pawns ahead reduces to depth 0 and re-creates the mate-hiding bug this engine has already shipped twice; the existing zugzwang guard on game_phase is kept, not replaced; the mate cases in the fast suite pass, and the cap is observed to be load-bearing by removing it and watching one go red; the base and divisor seeds are stated with their source and everything ships from our own sweep and S127 (DEC-084)
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   a null-move verification search -- dropped by DEC-087, no evidence below 3000 and the game_phase guard already covers zugzwang; the reduction formula's final values, which S127 fits
decisions:  DEC-071, DEC-084, DEC-087
closes:
blocks:
paused_by:
done:

## Re-scoped 2026-08-19 by the second review, DEC-087

Two halves, one kept and one dropped. **Kept:** the eval-scaled reduction --
`R = base + depth/divisor + min((eval - beta)/scale, cap)` is the surveyed
form, and a deeper base R alone measured **+12.3** at Berserk, with the
eval-term cap worth a further +3.3/+2.3 at Ethereal. The static evaluation
input exists since S108. **Dropped:** the verification search. No engine in
this band shows a gain for it -- it is a Stockfish-depth device -- and this
engine's zugzwang exposure is already guarded by `game_phase(&game->board) > 0`,
which stays.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol if drifted. Every GitHub read
below was a commit message or PR body, never a diff or source file (DEC-016,
DEC-084); numbers quoted from message prose are legal seeds, the S113 pass's
precedent.

### 1. State of the art

**Base R, as published.** CPW's record: Goetsch & Campbell 1988 ran recursive
null move at R=1; Donninger 1993 ("Null Move and Deep Search", ICCA Journal
16(3)) popularised it, his own R not stated on the page; "most typical engines
use recursive null move with an R value of 2 or 3"; a "basic implementation
... uses a fixed reduction of 3 or 4"; Fruit ran R=3 gated on static eval >
beta; Heinz 1999 ("Adaptive Null-Move Pruning") is R=3 when depth > 6 else
R=2 -- the first depth-scaled base. **Convention trap**: CPW's own examples mix
`depth - R` and `depth - R - 1`; chesso searches `depth - 1 - R` (:576), so a
quoted base transfers only next to its engine's depth arithmetic. Modern
re-decides in prose: Weiss #746 "Reduce the depth of NMP searches more"
(2024, +3.86); Lynx #2448 "5 + depth/5" failed STC -2.33 then #2620 passed
the same form +2.89 bundled with an improving simplification; Berserk #552
"less base reduction + more from depth/eval" 0.08/+1.89 at 3300+.

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

- R today: `NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR` = 2 + depth/6
  (src/search.cpp:561; src/search_params.hpp:77-78, ranges 0..16, 1..64).
  With the floor below, NMP fires from depth 4.
- Conditions (:569-571): `!is_pv && !is_in_check && ply > 0 && prev_move != 0
  && depth - 1 - null_reduction >= 1 && beta < MATE_MIN &&
  game_phase(&game->board) > 0`. No eval gate, no eval term.
- **The mate fix is the floor** `depth - 1 - null_reduction >= 1` (:570),
  comment :563-568: a depth-0 null search is pure quiescence, answers with
  the static score, and lost a mate in two at depth 4. Regression test:
  "pruning does not hide a forced mate", tests/test_search.cpp:1274.
- **The zugzwang guard counts both colours**: `game_phase()`
  (src/evaluation.cpp:1050) clamps `board->phase`, accumulated from
  `phase_value[6] = {0,1,1,2,4,0}` (src/eval_tables.hpp:28) for every piece
  of either side -- "some knight/bishop/rook/queen exists on the board",
  weaker than the published side-to-move form. Kept as-is per the accepts.
- Mate-score clamp on the fail-high (:583-586) is published practice exactly
  -- SF 42de93ac (2010) "Do not return unproven mate scores from null move
  search"; Lynx #1790 +1.85. The fail-soft return of `null_score` matches
  Ethereal 255c263 "Return value and not simply beta during NMP"
  (+2.35/+1.47). Both stay.
- No TT store on a null cutoff (:586 returns without storing); the Weiss
  TT-skip entry condition (4596ecf, +4.01) is a deferred extra, not V1.
- Input: post-S108 every non-check node has `static_evals[ply]`; today
  `static_eval` is set only inside the RFP guard (:477, :532).

### 3. Implementation sketch

One SPRT, as the accepts prices:
1. Two new constants beside :77-78 in the X-macro, ranges stated (section 4).
2. Entry gains the gate `static_eval >= beta` (the term is then never
   negative); R gains `min((static_eval - beta) / NULL_MOVE_EVAL_MARGIN,
   NULL_MOVE_EVAL_CAP)`; the floor at :570 tests the **full** R.
3. Base and divisor are re-decided by a node-count sweep over the 300
   positions (the S021/S068 instrument) with margin and cap alongside --
   published practice lands the family together (Weiss #127-#130) -- then
   one SPRT on the winner; S127 fits finals.
4. Tests, red first, printouts recorded:
   - :1274 stays green unmodified at every depth it runs.
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

All in src/search_params.hpp with ranges; every number a **seed -- must be
fitted/SPSA'd here** (DEC-084, S127):
- `NULL_MOVE_BASE` seed 2, sweep 1..4 (ships today; CPW "2 or 3", basic
  "3 or 4", Fruit 3, Heinz 2/3 -- mind the depth-arithmetic convention).
- `NULL_MOVE_DIVISOR` seed 6, sweep 3..8 (ships today; Lynx prose "depth/5").
- `NULL_MOVE_EVAL_MARGIN` seed 100, range 1..2000 (SF 074c7a3 prose: one
  pawn); floor 1 is the div-by-zero guard, the range top parks the term.
- `NULL_MOVE_EVAL_CAP` seed 3, range 0..16 (SF 7ed15af prose: "three plies";
  Ethereal 89ed7eb is the gain record for having one); 0 = term off.
- The entry gate ships as bare `>= beta`; Lynx #1918's margin-30 variant is
  an S127-era sweep, not V1.

### 5. Pitfalls

- **The floor and the cap are different safety devices, and the demolition
  only reddens if both are lifted.** With the floor kept and the term
  uncapped, a +20-pawn node makes R huge, :570 fails, and NMP switches off
  -- fail-safe, green, Elo quietly lost. "Reduces to depth 0" happens only
  in a build where the floor is also gone (then :466 turns the null search
  into quiescence -- the shipped-twice bug). Ship floor + cap; the
  demolition build lifts both, watches the new mate case go red, restores.
- **The floor bites the term at shallow depth**: with cap 3, depths 4..7 have
  `depth - 1 - (2 + depth/6 + 3) < 1`, so a maxed term *skips* NMP where
  today it fires with R=2..3 -- backwards from the term's intent. The sweep
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
- **S113 merge care**: ProbCut's enrichment inserts its block right after
  :588; this step rewrites :561-588. Land in plan order, rebase the later.

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

Two, neither touching goal or accepts. (1) The accepts' "the cap is observed
to be load-bearing by removing it and watching one go red" is unobservable
with the :570 floor kept: cap removal alone fails *safe* (NMP skips; green).
The red observation needs the demolition build to lift floor and cap
together, or the load-bearing claim re-attached to the floor -- owner's
wording call at implementation time; the section 5 arithmetic is the
evidence. (2) DEC-087's "a deeper base R alone measured +12.3 at Berserk"
was not re-traced through Berserk's PR/commit prose this pass; nearest traced
is #286 "NMP returns null search score" +10.94 (with a non-pawn-material
fix), which chesso's fail-soft return already covers.

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
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+null+move+reduction
  -- 314faa9, 074c7a3, 7ed15af, 620cfbb, 08ac4e9.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+hash:9d4090e82685cca447265dcd7093d617cb34a107
  -- the 2026 scaling commit, full message and both SPRT blocks.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+null+move+mate
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
