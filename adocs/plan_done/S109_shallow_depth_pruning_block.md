id:         S109
goal:       late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
accepts:    all four rules land in one commit and are measured by **one** SPRT, whatever it returns, recorded as it comes (INV-6); every threshold and margin is a constant in src/search_params.hpp with a stated range and none of them is a number copied from anywhere (DEC-084); the four rules are gated on `depth - lmr_reduction(depth, move_number)` and not on raw depth; the late-move rule sets a skip-quiets flag the staged generator honours rather than `continue`-ing, so the quiet stage is abandoned and not merely skipped over; **a position with a forced mate inside the pruned depth is added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guards removed and the printout recorded**; no quiet is pruned while in check, at a PV node, on the first move, or when alpha or beta is near mate -- all four rules, no exceptions; **the gives-check exemption binds the three per-move rules only** -- futility, history pruning and quiet SEE, which run after `make_move` where `is_check_move` exists (`src/search.cpp` `negamax`) -- **and does not bind late move pruning**, whose skip-quiets flag is honoured at the generation stage before any move is made, where the engine has no pre-make gives-check predicate to consult and the published LMP form carries no such exemption; so LMP may end a quiet stage that still holds checking quiets, which is stated here rather than tested away, and buying it the exemption with a post-make prune is a departure from the published form that the owner sent to its own step and SPRT, **S218**, behind this one (DEC-180) -- if this step's mate guard goes red without it, the exemption is this step's fix under the TESTS rule and S218 folds into it; for every rule, the test asserts the precondition that would otherwise prune the move, against the exemptions that bind that rule; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   razoring, which is a node-level rule and is S116; SEE pruning of **captures** in the main search, which is S091; futility inside quiescence, which is S112; the improving flag, which S108 supplies and this step consumes
decisions:  DEC-071, DEC-082, DEC-084, DEC-087, DEC-105, DEC-134
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); the SPRT is the coordinator's; started 2026-09-13 04:51 on the idle machine
done:       2026-09-13 09:20. **H1: the block is worth +46.90 +/- 15.43 Elo at `8+0.08`, decided in 1364 games.** `adocs/data/S109_sprt.sh` ran as pre-registered -- candidate `1952c56` (the block, `600f448`, plus the tests-only sanitizer-build fix) against the pre-block `50e3661`, both identity lines printed, `{0, 5}` nElo, `8+0.08`, `Hash=16`, `noob_3moves.epd`, seed `20260913081118`, 12 cores, governor `performance` -- from 08:11:18 to 08:49:09: **LLR 2.97, `Elo 46.90 +/- 15.43`, `nElo 56.76 +/- 18.44`, LOS 100 %, `Ptnml(0-2) [55, 116, 214, 185, 112]`, W 556 L 373 D 435, 1364 games in 37 m 51 s at 2162 an hour**, 0 time forfeits either side, 1028 adjudications and 339 natural ends, the crash census quiet, `Incomplete mating PV` 1 from the candidate and 2 from the reference (recorded, not a stop), pair variance 0.3372 (`adocs/data/S109_sprt_pairs.txt`; a stronger side widens the pair score, as expected). Read as pre-registered: H1 at the gainer pair, the block's parts not attributed (one verdict for four rules, by design, DEC-082), the bisection protocol not needed. The `8+0.08` figure sits inside the published +40 to +120 self-play band the file quoted and DEC-019 keeps it from meaning more than it says; the block's transfer to four times the control is DEC-202's block-boundary reading, pre-registered as `adocs/data/S109_ltc.sh` by S199 and taken beside S199's first drift point, both coordinator-run on the idle machine after this commit. A first launch at 08:10 was stopped after 9 games because its output had defaulted to `/tmp`, which this machine wipes at boot; the relaunch one minute later carried `OUT` under `.tuning/`, and the new busy guard's warning at that moment was the stopped run's own decaying load. What the block is, what it departed from and why, its constants, tests, mutants and node counts are the "Landed" and "Repaired before the commit" sections above and DEC-205; evidence `adocs/data/S109_sprt.log`. Implemented by an Opus 5 subagent and two repair agents; run, read and stamped by the coordinator (DEC-185, DEC-199).

## Why this is one step and not four

**Per-part attribution is deliberately forfeited here, and DEC-082 is the
decision that permits it.** The four rules prune overlapping sets of moves, so
each one measured against a tree the other three are absent from returns
nothing. Stockfish's own removal test is the evidence: move-count pruning
measures **~0 Elo alone** and the block it belongs to measures **~204**. Four
steps would spend four verdicts to record four zeros and leave this plan
believing a real +60 to +120 does not exist.

If the block fails, *then* it is bisected -- a failing block is evidence that
one part is wrong, which is the attribution question the block form was not
asked.

**The second review corrected the evidence without changing the shape,
DEC-087.** Lynx measured late move pruning and futility individually at
**+4.7 each** when added to a tree without the others, so "inert apart" is too
strong -- the parts are small-positive apart. The block is kept anyway, on
measurement-budget grounds: four verdicts on true effects near +5 sit at the
bounds and crawl (DEC-063), where one verdict on a +40 to +120 block resolves
in an hour. The bisect-on-failure clause is unchanged.

**This step absorbs S090 and S026, both retired.** Their ids are not reused.
S090 was late move pruning alone; S026 was "drop nodes near the horizon that
cannot reach alpha", whose razoring half is now S116 and whose futility half is
here. The mate clause below is S026's own, carried forward, and the retired
S060 existed to put it in S026's `accepts:` where it now sits in this step's.

## The four rules

For a quiet move, at `lmr_depth = depth - lmr_reduction(depth, move_number)`:

1. **Late move pruning.** Past a move count that grows with depth, stop
   generating quiets. The count is **doubled when improving** -- that is the
   single highest-leverage use of the flag S108 supplies.
2. **Futility.** Static score plus a base plus a slope times `lmr_depth` at or
   below alpha, skip the remaining quiets.
3. **History pruning.** History below a margin scaled by depth, skip.
4. **Quiet SEE pruning.** The exchange evaluation says the move loses more than
   a margin scaled by `lmr_depth` squared, skip.

The functional forms are read from the published record; **every constant is
ours and is fitted** -- a first setting from a sweep here, and SPSA at S127.
DEC-084.

## Hazard, three times observed and once more expected

Null move pruning hid a mate in two by reducing to depth 0. Late move reduction
reduced the mating move at the root. Reverse futility returns a static bound
and therefore cannot see a mate at all, which is why S033 bounded it to depth 6
and ply 3. All three were caught by a mate test rather than by a benchmark.
This is the fourth, fifth, sixth and seventh pruning rule in the engine and
they get the same treatment before any of them is called done -- the clause is
in the `accepts:` above rather than left for a later step to add.

## Inherited from S108: the table score as the margin's input

**S108's layer (c) is deferred here by the owner's decision, 2026-08-23.** S108
supplies the static evaluation at every non-check node; what it deliberately
did not take is the next layer above it -- where the table entry's *score*
certifies a direction, using that adjusted number as the input to a pruning
margin instead of the raw static evaluation. The bound type is what licenses
it: a `TT_BETA_NODE` whose de-normalised score is above the static evaluation
may raise the input, a `TT_ALPHA_NODE` whose score is below it may lower it,
and the mate band is excluded. The stack and the stored evaluation keep the raw
static either way -- improving compares statics and never search scores, or the
correction compounds through storage (S099 inherits the same rule).

It is deferred rather than dropped because S108 already owed one verdict and
one change at a time is the rule. It arrives here as this step's first line,
which is the cheaper place for it: the margin consumers this step adds are what
the adjusted input feeds, so it is measured with them present rather than
against reverse futility alone.

The published record prices the sites individually and far above 3000: Lynx
#1973 measured the reverse-futility site alone at **+2.07 +/-1.50** over 80488
games at 8+0.08, and #2055 the null-move condition at **+1.56 +/-1.27** over
91990, after #1971 removed a global version and #1975's first null-move cut
failed. Weiss cca90ea (#336, 2020) measured **+10.78/+12.09** for the family,
scoped by its own message to "pruning heuristics". An effect of +2 is not
resolvable by a run this harness can afford, which is the thing to settle
before booking one: fold it into this step's single SPRT, or give it a
non-regression pair of its own. S130 is the local precedent and it is not
encouraging -- the quiescence half of the same idea measured no verdict over
16784 games.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `cf89e22`; re-locate by symbol if drifted. Every GitHub read
below was a commit message or PR body, never a diff or source file (DEC-016,
DEC-084).

### 1. State of the art

**The block, traced.** The plan's ~204 is Stockfish PR #2401 (2019-11, removal
tests at 10+0.1, 20000 games): "step 14 (pruning at shallow depth) going from
~170 to ~204 Elo"; same message, "step 7 (futility pruning) going from ~30 to
~49" on code "actually identical" both times -- "the value of early futility
pruning increased significantly due to changes elsewhere in search." The
post-NNUE re-run (PR #3868, 2021-12) prices the block at +98.63 against its
in-block parts alone: FutilityMoveCount +6.91, futility parent +8.71, SEE
based pruning +9.10, cont-hist pruning +1.7 -- the block is ~5x its parts'
sum, DEC-082's whole case. The step's Lynx +4.7s: PR #512 "Add basic LMP",
merged 2023-11-24, **+4.7 +/-3.9**; PR #733 "Futility pruning", merged
2024-05-14, **+4.72 +/-3.97 STC / +2.07 +/-2.85 LTC** -- chesso's band.

**(a) Late move pruning.** Skip the remaining quiets past a move count that
grows with depth. Weiss PR #104 (2019-12, +20.17/+24.67): "after trying some
quiet moves we give up". The quadratic FORM `base + depth^2` is Stockfish's
shape (source, not citable) and in PR prose **at a band S181 puts 180 points
lower than the "~2600" written here before 2026-09-11** it **failed**: Lynx
#512 merged 2023-11-24, between v1.0.1 and v1.1.0, band **2420 to 2430**
(`adocs/data/S181_lynx_bands.md`), and measured `3 + depth*depth` at **-23.8 +/-15.0** while linear
`moves >= depth*10` capped at depth <= 3 passed +4.7; re-tries #783 (-0.9/-8.61) and
#1343/#1344/#1345 all closed; #1551 later removed the depth cap for
+0.87/+2.60. The aggressive count is a >3000 setting; the seed starts loose.
**Improving doubles the count**: Lynx #1129 "Improving: LMP, multiplying by
2" merged **+8.39 +/-4.48**; the halving direction (#1128) closed. Counter
semantics: Ethereal #160 (84ec1707, 2020-12) counted **all** moves toward the
threshold and set skipQuiets "at any point", +4.79/+6.31; Lynx #2492 (legal
instead of pseudo-legal count, +2.21) closed unmerged. The published mechanism
is a **skip-quiets flag** the move picker honours -- captures keep coming,
the quiet stage ends.
**(b) Futility pruning.** CPW: "adding a safety margin to the evaluation of
the current position. If this does not exceed alpha then futility pruning is
triggered to skip this move." Exempt per CPW: captures and moves that give
check; "not used when the side to move is in check, or when either alpha or
beta are close to the mate value"; requires "the existence of at least one
legal move to avoid returning erroneous stalemate scores". Margin scale per
CPW: a minor piece at depth 1, "more like the value of a rook" at depth 2.
The **lmrDepth** margin is Stockfish commit prose: dbd7f60 (2021) dropped the
second FP depth limit as "already capped by margin that is a function of
lmrDepth"; 910f779 (2020) raised the "LMRdepth threshold for futility pruning
at parent nodes". Improving in the move-loop margin is untraced in prose (its
futility role is RFP's margin, S108); it enters this block via LMP only.
**(c) History pruning.** CPW History Leaf Pruning (Fruit 05/11/03, Fabien
Letouzey): prune at "depth <= 0 after reductions" -- the lmrDepth idea in its
original form -- guarded !PV, not in check, at least 5 moves played, no
extension; two thresholds, reduce below the first, prune below the lower.
Modern records: Weiss #446 (2021-06) "Skip moves with bad history at low
depths" **+9.05/+10.50**, #483 lowered the margin, +6.33/+3.48; Lynx #972
(2024-09) **+2.06 +/-1.67**. Negative controls: captures included failed
(Lynx #1096, -7.23); `break` -> `continue` failed (Lynx #1828, -3.08 -- with
quiets ordered by history, everything after the first sub-threshold quiet is
sub-threshold too, so stopping is free); and the lmrDepth gate failed **twice
at Lynx** (#1303 -8.84, #1532 -0.94) where Stockfish prose keeps it (d37de3c
raises the "lmrDepth threshold for continuation history based pruning";
de2bf1a drops the depth limit as "naturally constrained by history
thresholds"). See Scope concerns.
**(d) Quiet SEE pruning.** Skip a quiet whose exchange evaluation loses more
than a margin scaled by lmrDepth. Stockfish prose: a834bfe (2018) allows "see
pruning on quiets to be done up to an lmrDepth of 9" -- the gate is lmrDepth
by record. The margin's power is source-only and both shapes appear in
surveyed descriptions (this file's goal says lmrDepth squared; S091's survey
says linear for quiets, squared for captures) -- ship the power as a
parameter, the fit decides. Weiss #742 (2024, ~3300) merged one shared
threshold for quiets and noisies, +1.44/+0.46, so the split is not
load-bearing either. No individual sub-3000 record for the quiet half was
found; SF #3868 prices "SEE based pruning" at +9.10.

**Exemptions every published form keeps**: nothing prunes while in check (the
list is evasions; pruning one risks a false mate) or near mate bounds;
futility exempts captures and checking moves; the TT move is structurally
exempt everywhere (searched first, before any count or flag trips); history
pruning exempts PV nodes (CPW). A killers exemption is untraced in prose.
LMP is the one rule with no per-move exemption list -- it ends a stage.

### 2. Shape for chesso

The move loop: `for (size_t i = 0;; ++i)` at `src/search.cpp` `negamax`; staged
quiet generation inside it at `src/search.cpp` `negamax` (`quiets_generated`,
the branch the skip-quiets flag must gate); `pick_next_move` `src/search.cpp`
`negamax`; `make_move` `src/search.cpp` `negamax`; `is_capture`
`src/search.cpp` `negamax`; `is_check_move` `src/search.cpp` `negamax` --
**computed only after make_move**, from the child position;
`legal_moves_counter++` `src/search.cpp` `negamax`; LMR eligibility and
`reduction = lmr_reduction(depth, legal_moves_counter)` at `src/search.cpp`
`negamax`; the fail-high update block `src/search.cpp` `negamax`; the
no-legal-moves mate/stalemate return `src/search.cpp` `negamax`. `is_pv` is a
parameter (`src/search.cpp` `negamax`). RFP's guard row to copy the mate clause
from: `src/search.cpp` `negamax` (`beta < MATE_MIN && beta > -MATE_MIN`).

- **lmrDepth before S098**: the reduction today is the static table
  `lmr_reduction(depth, move_number)` (`src/search.cpp`
  `history_gravity_update` and `src/search.cpp` `history_on_quiet_cutoff`),
  `LMR_BASE 52` / `LMR_DIVISOR 182` (`src/search_params.hpp`
  `CHESSO_SEARCH_PARAMS`), axes capped at 63. The accepts' `lmr_depth = depth -
  lmr_reduction(depth, move_number)`, clamped to >= 0, computed per candidate
  move with `move_number = legal_moves_counter + 1` when testing before the
  counter increments. When S098 rebuilds the reduction, this gate shifts with
  it by construction -- see Interactions.
- **Exists after S108**: `static_evals[ply]` (TT_EVAL_NONE while in check --
  so futility is off in check by data as well as by guard) and the
  `improving_at()` helper whose first in-search call site is this step.
- **Exists after S093/S024**: signed quiet history `[-M, +M]` on butterfly
  indexing plus continuation entries, read through the single probe path
  those steps build; the sum's range is `[-3M, +3M]` after S024. The history
  threshold reads the **raw table sum**, never `score_move`'s return -- a
  killer's 900000 band value would silently exempt it (see Pitfalls).
- - **Quiet classification in the loop**: `!MOVE_CAPTURE(m) &&
  !MOVE_PROMOTED(m)`, the same pair LMR uses (`src/search.cpp` `negamax`). En
  passant is capture-flagged. Non-capture promotions are not "quiet" here.
- - **SEE for quiets exists today**: `see_ge(board, move, threshold)`
  (`src/bitboard.cpp` `see_ge`) scores a quiet move -- `captured == EMPTY`
  gives gain 0 -- and takes a negative threshold, so quiet SEE pruning is
  `!see_ge(&game->board, moves[i], -margin)`. `see()` `src/bitboard.cpp` `see`
  is the exact reference; `see_value` `src/bitboard.cpp` `see_value`;
  `capture_cannot_lose` `src/bitboard.cpp` `capture_cannot_lose` is
  capture-only.
- - **Does NOT exist**: a pre-make gives-check predicate. `is_check_move` is
  known only after make_move, so the per-move rules (futility, history, SEE)
  run **after** `src/search.cpp` `negamax` and prune by `unmake_move +
  continue` -- the subtree saving dominates the wasted make. LMP's flag is
  pre-make by construction and cannot see gives-check (Scope concerns). Lynx
  #1520 measured moving rules before make at -0.6 +/-2.6, so nothing is lost by
  staying after.

### 3. Implementation sketch

One commit (accepts), assembled and tested in this order, each rule behind
its own constant whose declared range includes an **off value** -- that is
the kill-switch that keeps a failing block bisectable without four SPRTs and
without taking strength numbers on the tune build (forbidden, S073):

1. 1. **Scaffold**: `lmr_depth` computation plus a `skip_quiets` flag wired
   into the generation branch (`src/search.cpp` `negamax`: when set, do not
   generate quiets, break instead) and into the staged-up-front corner -- when
   `tt_move_is_quiet` put both stages in the array (`src/search.cpp`
   `negamax`), already-generated quiets are filtered by the same flag. Flag
   never set yet: node counts identical, search_bench proves the scaffold inert
   before any rule lands.
2. **LMP**: `legal_moves_counter + 1 > lmp_threshold(depth, improving)` sets
   `skip_quiets` (never `continue`, per accepts). Threshold doubled when
   improving. Guards: `!is_pv`, `!is_in_check`, depth cap, `>= 1` legal move
   searched.
3. **Futility**: at `static_evals[ply] != TT_EVAL_NONE`, quiet, `!is_pv`,
   `!is_in_check`, `!is_check_move`, not first legal move, alpha and beta
   outside the mate band, `lmr_depth <= FUT_MAX_LMRDEPTH`: prune when
   `static_evals[ply] + FUT_BASE + FUT_SLOPE * lmr_depth <= alpha`. The
   margin shrinks as `lmr_depth` falls with move number, so later quiets
   prune more easily -- monotone at a fixed node.
4. **History pruning**: same guards, `depth <= HP_MAX_DEPTH` (raw depth --
   the lmrDepth gate is the accepts' rule; keep the raw-depth variant one
   constant-flip away, Scope concerns): prune when the S093/S024 history sum
   `< -HP_COEFF * depth`.
5. **Quiet SEE**: same guards, `lmr_depth <= SEE_QUIET_MAX_LMRDEPTH`: prune
   when `!see_ge(board, move, -(SEE_QUIET_COEFF * lmr_depth * lmr_depth))`
   (power parameterised; linear is the one-line alternative).
6. Tests red-first, per rule, before its rule lands:
   - Extend "pruning does not hide a forced mate" (`tests/test_search.cpp`
     "pruning does not hide a forced mate") with a position whose mating move
     is a **late, low-history, negative-SEE quiet** inside the pruned depth --
     observed red with the in-check and near-mate guards removed, printout
     recorded (accepts). Built the S033 way: python-chess enumeration plus
     Stockfish confirmation (DEC-023).
   - - Stalemate edge: a position whose only legal moves are late quiets; the
     first-legal-move guard keeps `src/search.cpp` `negamax` unreachable --
     assert no false mate/stalemate score.
   - Pure-helper tests: `lmp_threshold` doubles exactly when improving says
     so; each formula at its off constant is provably unreachable.
   - Precondition tests (accepts, non-vacuous): a position where a rule fires
     (node counts move against the off value), then the exempt variants --
     in check, PV, first move, gives check, near-mate bounds -- search
     identically to the off build.
7. Fast suite green, one SPRT, verdict recorded whatever it is.

### 4. Constants and seeds

All in the `CHESSO_SEARCH_PARAMS` X-macro in `src/search_params.hpp` with
stated ranges; every number below is a **seed -- must be fitted here (sweep)
and SPSA'd at S127**. Off values sit inside the declared ranges on purpose.
Under DEC-105 each seed is one of three forms and says which: **(a)** a value
from a publication about the technique, with its URL; **(b)** a derivation
over chesso's own data or scale; **(c)** the range midpoint or off value,
stated as such. Where a midpoint is not an integer this step takes the
integer below it and says so. No engine's shipped threshold, margin or depth
cap seeds anything here, wherever it is republished; those records are in
section 1 and, as anti-seeds, in section 5.

**Units, once for this file (P6).** A margin compared against `evaluate()` --
futility -- is in chesso's material scale, `piece_value` in
`src/eval_tables.hpp`: `PAWN` 94, `KNIGHT` 327, `BISHOP` 308, `ROOK` 487,
`QUEEN` 716. The header's own comment says the split between `piece_value`
and `psqt_mg` / `psqt_eg` is degenerate, so the material term alone is the
unit. A threshold compared against a SEE result -- the quiet SEE gate -- is
in the exchange scale instead, `see_value` in `src/bitboard.cpp`, where a
pawn is 100. **The two pawns differ and the 2026-08-19 pass conflated them.**

- `LMP_BASE`, `LMP_DEPTH_COEFF`, `LMP_MAX_DEPTH`: threshold `LMP_BASE +
  LMP_DEPTH_COEFF * depth` -- **(b)**, a census over chesso's own tree,
  **P2**, run by this step at its start. Tune build, instrumented under
  `#ifdef CHESSO_TUNE` only: at every beta cutoff on a quiet move in
  `negamax` in `src/search.cpp`, record `(depth, legal_moves_counter)`; run
  `go depth 10` over the 300-position stratified pick
  (`adocs/data/S021_aspiration_sweep.py`). Per remaining depth 1..8 take the
  **95th percentile** of the cutoff index -- the count past which 19 of 20
  quiet cutoffs have already happened, which is what "late enough to skip"
  means; the percentile is this file's choice and the sweep decides. Fit
  `threshold = LMP_BASE + LMP_DEPTH_COEFF * depth` by least squares over
  depths 1..3. Seed `LMP_MAX_DEPTH` as the largest depth at which the fitted
  threshold is below the median number of quiet moves generated at that depth
  in the same census -- a rule that never binds is not measurable. Record the
  percentile table in this step's stamp. The quadratic `base + depth^2` stays
  the alternative **form**, tried only after the linear fit has a verdict.
  Off: `LMP_BASE` at `MAX_MOVES` (270, `src/data_structures.hpp`).
- `FUT_BASE` **147**, `FUT_SLOPE` **170** -- **(a) literature**, Heinz's
  margins as the wiki states them:
  https://www.chessprogramming.org/Futility_Pruning (fetched 2026-09-05)
  gives the depth-1 margin as one that "should not exceed the value of a
  minor piece" and the depth-2 margin as "more like the value of a rook". In
  chesso's material scale (units above) a minor is `(KNIGHT + BISHOP) / 2` =
  317 and a rook is `ROOK` = 487, so with `margin = FUT_BASE + FUT_SLOPE *
  depth` the slope is 487 - 317 = **170** and the base is 317 - 170 = **147**.
  (Written with the words rather than the symbols since 2026-09-13: a symbol
  followed by an equals sign and the first term of its own derivation reads to
  `tools/plan_prose_check.py --params` as a claim that the parameter is that
  term, which is its NEAR rule doing its job on an intermediate.)
  The 2026-08-19 pass read the same two words in `see_value`'s scale
  and got 100 / 200; that was the wrong pawn. `FUT_MAX_LMRDEPTH` **8** --
  **(c) midpoint** of a range declared by purpose, 0..16: 0 is off, and 16 is
  above the median depth 11 the `RFP_MAX_DEPTH` comment in
  `src/search_params.hpp` records, where the gate stops binding. Off:
  `FUT_BASE` at the range top (>= 2 * `QUEEN`).
- `HP_COEFF` **576**, `HP_MAX_DEPTH` **8**. `HP_COEFF` is **(c)**, the
  arithmetic midpoint of the per-depth-ply region this file declares on
  chesso's own history scale, `M/64 .. M/8` with `M` = `QUIET_HISTORY_MAX`
  (8192): `(128 + 1024) / 2` = `9M/128` = **576**, swept from there.
  `HP_MAX_DEPTH` is **(c) midpoint** of 0..16, declared by the same purpose
  as `FUT_MAX_LMRDEPTH` above. Off: `HP_COEFF` at max (threshold below
  `-3M`).
- `SEE_QUIET_COEFF` -- **(b)**, chesso's own exchange scale: seeded so the
  bar at lmrDepth 1 is under one pawn of `see_value` in `src/bitboard.cpp`
  (100 there, not 94). `SEE_QUIET_MAX_LMRDEPTH` **8** -- **(c) midpoint** of
  0..16, declared by purpose as above. Off: coefficient at max.

seeds re-derived 2026-09-04 under DEC-105 (DEC-134)

### 5. Pitfalls

- **Anti-seeds — records, not seeds.** DEC-019 lets a record say which
  direction is worth trying; DEC-105 forbids any of these numbers starting a
  sweep, which is why section 4 names no engine. The linear threshold's
  shipped form at the only sub-3000 pass is Lynx #512's (base 0, coeff 10,
  cap 3) and the quadratic `3 + depth^2` failed at a banded **2420-2430**
  there (#512/#783; S181) --
  a record of which *form* to try first, never of where to start it;
  doubling the threshold when improving is Lynx #1129 and is S092-era, not
  this step's. The depth cap "3-4, low depths" is Weiss #446's phrase, and
  the quiet-SEE gate lmrDepth <= 9 is SF a834bfe prose: both are those
  engines' tuned output. Section 4 declares its own ranges and takes their
  midpoints instead.
- **The recurring repo bug, now four ways.** Null move hid a mate in 2, LMR
  reduced the mating root move, RFP cannot see mates (S033). Every rule here
  carries the in-check + near-mate-bounds + PV + first-move guards, and the
  accepts' red-first mate test is the enforcement, not the comment.
- - **Pruning the last legal move.** Rules skip moves without making them, so
  `legal_moves_counter` can end at 0 with legal moves on the board, and
  `src/search.cpp` `negamax` would return a false mate/stalemate. The `>= 1
  legal move searched` guard (CPW's own clause) is load-bearing; the stalemate
  edge test pins it. Zugzwang: no surveyed move-loop rule adds a phase guard
  (null move keeps that job); the depth caps bound the damage -- do not invent
  one silently.
- **Improving misread through S108's sentinel.** `static_evals[ply]` is
  TT_EVAL_NONE in check; `improving_at` falls back ply-2 -> ply-4 -> default
  true. A wrong-side default doubles the LMP count where it should not --
  Elo loss with no crash. The consumer-side test (LMP threshold doubles
  exactly when the helper says improving) is the guard; S108's own fallback
  tests cover the helper.
- - **History sign and band errors.** After S093 history is signed; the
  threshold is negative (`< -HP_COEFF * depth`); a sign slip prunes the *good*
  quiets -- silent regression, the S093 band hazard's sibling. And the rule
  must read the raw table sum through the S093/S024 probe path: killers score
  900000 and counters 700000 in `score_move` (`src/evaluation.cpp`
  `ORDER_TT_MOVE` to `src/evaluation.cpp` `ORDER_COUNTER`, `src/evaluation.cpp`
  `score_move`), so testing the ordering score instead of the table silently
  exempts killers/counters and nothing else -- decide the exemption, never
  inherit it from the wrong variable.
- **The skip-quiets flag kills killers and checking quiets in the tail.**
  Chesso's killers live inside the quiet stage (no separate emission stage),
  so an abandoned stage drops them; they order at the band top and are
  usually searched before the count trips, but the exposure exists. S107's
  newly-remembered checking quiets are what LMP would prune -- the mate test
  is the net. See Scope concerns.
- **Double-pruning with S091.** This step prunes quiets only; S091 prunes
  captures and adds an extra reduction for negative-SEE moves, after, with
  its own verdict. S091's accepts still names "separate margins for captures
  and for quiets" from before the re-scope -- the quiet margin lands **here**,
  not twice. When S091 arrives, one see_ge call per quiet, result shared.
- **Tune-build strength numbers.** The off values make per-rule ablation
  builds one-line release rebuilds; never SPRT the tune build (S073).

### 6. Measurement

- **One SPRT** at the S105 regime (8+0.08, Hash 16, UHO book), gainer bounds
  `elo0=0 elo1=5` (DEC-063: expected effect +40 to +120 self-play, far above
  the pair). Fast suite plus both mate cases green first, red observations
  recorded before the guards land. INV-6 takes the SPRT path -- node counts
  move by construction.
- **Bisection protocol on a failing block** (DEC-082/DEC-087): halves first,
  by evidence strength -- leg 1 disables {history, quiet SEE} (weakest
  sub-3000 records: +2.06, none) via their off constants, release rebuild,
  SPRT; leg 2 disables {LMP, futility} instead. One more run inside the
  failing half isolates the rule: three runs worst case against four
  one-at-a-time. First suspects within a half: LMP threshold too tight
  (Lynx's quadratic failed at -23.8) and the history threshold's sign/scale.
- **Node counts, recorded not argued**: tools/search_bench.py at depths 9
  and 12 before/after (expect large drops; the numbers go in the step
  stamp), and the DEC-081 probe re-run -- depth reached after `e4 e5 Nf3 Nc6
  Bb5 a6` at `go movetime 250` against the 12-on-1.45M baseline. Direction
  only; DEC-019 keeps expectations out of the verdict.

### 7. Interactions

- **S108 (required input)**: `static_evals[ply]`, `improving_at` -- this step
  is the helper's first in-search call site. S108's deferred layer (c),
  TT-score-adjusted eval, becomes "S109's first line" per that file: decide
  explicitly whether the futility comparison uses the adjusted value, and
  say so in the commit.
- **S093/S024 (history inputs)**: signed sum through one probe path;
  thresholds fitted against the **post-S024** shape (plan order guarantees
  it). Stockfish d37de3c records cont-hist pruning as strongly TC-sensitive
  -- the 8+0.08 verdict may not transfer upward; S152, which absorbed S128's
  question (DEC-108), eventually reads that.
- **S098 (after)**: rebuilds the reduction this gate is computed from.
  plan.md gives S098 three verdicts of its own, measured with the block
  live, so the block is **not** re-verdicted -- S098's SPRTs price the
  interaction and S127 refits the constants against the new lmrDepth.
- **S091 (after)**: capture SEE pruning + extra reduction; boundary above.
- **S112 (separate)**: per-move futility on **captures inside quiescence**;
  SF #3868 prices the quiescence pair separately (+4.95 futility+movecount,
  +5.14 negative-SEE).
- **S116 (separate)**: razoring is node-level, before the move loop, depth 1.
- **S107 (before)**: checking quiets now earn history; they are also what
  LMP's flag cannot exempt -- the mate test covers the join.
- **S127**: the eight-plus new constants enter with stated ranges; the
  quadratic LMP form and the SEE power are re-tried there, where the
  post-block tree can price them. Berserk 587fee4 (history in the futility
  margin, +4.34) and SF 2b62c44/93b14a1 are that phase's material.

### Scope concerns

1. **lmrDepth gating vs the sub-3000 record.** The accepts gates all four
   rules on `depth - lmr_reduction(...)`. Stockfish prose supports it
   (dbd7f60, a834bfe, d37de3c) -- at ~3600. The one engine that A/B'd the
   gate near this band measured it **negative twice for history pruning**
   (Lynx #1303 -8.84, #1532 -0.94; their merged rules gate on raw depth or
   move count). The accepts stands; but a failing block's bisection should
   include the gating axis, and the sketch's HP_MAX_DEPTH keeps raw depth
   one flip away.
2. **The gives-check exemption binds three of the four rules, and since S139
   the accepts says which.** It used to be one undivided clause over all four.
   It is testable for futility, history and SEE, which run after `make_move`
   where the flag exists -- `const bool is_check_move = is_capture ? false :
   is_check(game);` at `src/search.cpp` `negamax` -- and structurally
   impossible for a skip-quiets flag honoured at the generation stage: `grep
   -rn
   "gives_check\|is_check_move" src/` returns that line and its single
   consumer, the LMR guard at `:710`, and nothing else, so there is no
   pre-make predicate to gate generation on. The published LMP form carries no
   such exemption either. The owner answered the narrower question on
   2026-09-11, DEC-180: the published form ships here, and the post-make
   exemption is S218, its own step and SPRT behind this one -- recorded, not
   silent.
3. **"Quadratic in depth" is the >3000 form, not the entry form.** Both
   sub-3000 LMP passes traced (Weiss #104, formula unstated; Lynx #512,
   linear capped at depth 3) are conservative; every quadratic attempt at
   Lynx's **2420-2430** band failed (S181; this read "~2600" until
   2026-09-11). The FORM ships parameterised either way; the seed is loose.

### 8. References

- - https://github.com/official-stockfish/Stockfish/pull/2401 -- removal tests
  2019: step 14 pruning at shallow depth ~170 -> ~204; step 7 futility ~30 ->
  ~49; TC-sensitivity note. The plan's ~204.
- - https://github.com/official-stockfish/Stockfish/pull/3868 -- 2021 re-run:
  block +98.63; FutilityMoveCount +6.91; futility parent +8.71; SEE based
  pruning +9.10; cont-hist pruning +1.7; qsearch pair +4.95/+5.14.
- - **The plan's "~0 alone" is pull request #2401 above, not #4294.** The
  literature check of 2026-09-04 fetched #2401's comment updates and the
  number is there: "Move count based pruning **~0** Elo", added in the same
  diff that moved step 14 from ~170 to ~204, 20000 games per test at 10+0.1,
  +/-3 Elo. Re-pointed by S185 (2026-09-04_plan_review-F06);
  https://github.com/official-stockfish/Stockfish/pull/4294 is a later update
  and is **unverified** as the source of that figure.
- - https://github.com/lynx-chess/Lynx/pull/512 -- basic LMP +4.7 +/-3.9;
  formula ablations in prose: 3+depth^2 -23.8, depth*10 cap-3 merged.
- - https://github.com/lynx-chess/Lynx/pull/733 -- futility pruning +4.72 STC /
  +2.07 LTC.
- - https://github.com/lynx-chess/Lynx/pull/1129 -- LMP count x2 when
  improving, +8.39 +/-4.48 (merged; #1128 divide-by-2 closed).
- - https://github.com/lynx-chess/Lynx/pull/1551, /pull/783, /pull/2492 --
  depth-cap removal +0.87/+2.60; depth^2 re-try failed; legal-count variant
  +2.21 closed.
- - https://github.com/lynx-chess/Lynx/pull/972, /pull/1303, /pull/1532,
  /pull/1096, /pull/1828 -- history pruning +2.06; lmrDepth gate -8.84 and
  -0.94; captures -7.23; break->continue -3.08.
- - https://github.com/TerjeKir/weiss/pull/104 -- LMP 2019: +20.17 STC / +24.67
  LTC, "after trying some quiet moves we give up".
- -
  https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22history+pruning%22
  -- #446 +9.05/+10.50 "skip moves with bad history at low depths"; #483 margin
  -> +6.33/+3.48.
- - https://github.com/TerjeKir/weiss/pull/742 -- one SEE threshold for quiets
  and noisies, +1.44/+0.46 at ~3300.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+lmrDepth
  -- dbd7f60 FP margin is a function of lmrDepth; 910f779 FP lmrDepth
  threshold; a834bfe quiet SEE up to lmrDepth 9; de2bf1a quiet HP depth limit
  removed; d37de3c cont-hist pruning lmrDepth, TC-sensitive; 2b62c44 history
  sum in FP.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+skipQuiets
  -- 84ec1707 (#160) count all moves toward LMP, set skipQuiets at any point,
  +4.79/+6.31; 85bbcd3 (2017) LMP added.
- - https://www.chessprogramming.org/Futility_Pruning -- condition, exemptions
  (captures, checks, in-check, mate bounds, one-legal-move), minor/rook margin
  wording.
- - https://www.chessprogramming.org/History_Leaf_Pruning -- Fruit origin;
  "depth <= 0 after reductions"; !PV / not in check / >= 5 moves / no
  extension; two-threshold reduce-then-prune cascade.
- - https://www.chessprogramming.org/Late_Move_Reductions -- exemption list
  (tactical, in-check, gives-check, extensions, PV, killers, passers);
  published log-log reduction formulas (Obsidian, Weiss) as prose.
- - https://github.com/jhonnold/berserk (commit 587fee4, message only) --
  history value in futility pruning +4.34: S127-phase material.
- -
  https://github.com/official-stockfish/Stockfish/commit/93b14a17d168e87e7f05fc09e3ba93e737b0757e
  -- title only ("Don't direct prune a move if it's a retake"): a later
  exemption refinement, not opened.

## Landed 2026-09-13 06:10

Everything in `accepts:` except the SPRT, which is the coordinator's. The
`done:` stamp is theirs to write with the verdict.

### The one deviation, and it is the one the accepts pre-authorised

**The gives-check exemption binds late move pruning too.** The published form
shipped first, with the flag ending the quiet stage at the generation branch
and no exemption, exactly as the `accepts` and DEC-180 say. The mate case then
went red -- three of them did -- and DEC-180's own clause fires: *a red guard
is a bug, not an option*, the exemption is this step's fix and **S218 folds
into it**. Recorded rather than argued:

- `4K3/q7/8/4k3/8/8/8/8 b` (`MATE_IN_2_B_POS`), mate in two by a7b8 after
  e5e6 e8d8, **lost at depth 3** in `tests/test_search.cpp` "pruning does not
  hide a forced mate" -- `REQUIRE( black.mate_found ) values: REQUIRE( false )`,
  and the same position red in "mate in two is found at the right distance"
  and `4K1R1/q7/5P2/4k3/8/1P6/2P5/1B2N3 b` red in "pruning does not hide a mate
  against the material leader", all at depth 3.
- Which rule: one release rebuild per cap at its off value. **Late move pruning
  alone.** With `LmpMaxLmrDepth` 0 the case is green; with `FutMaxLmrDepth`,
  `HistPruneMaxLmrDepth` or `SeeQuietMaxLmrDepth` 0 instead it stays red.
- Not the count being too tight: a sweep of `LmpBase` over one release rebuild
  each reads **red at 733, 1200 and 2000 hundredths and green at 3000, 5000 and
  27000** -- 30 moves, at a node whose median quiet count is 25, is a rule that
  never fires. The mating move is one of twenty-eight queen moves at a node
  whose history table has never seen any of them, so no useful count reaches
  it. The choice was the exemption or the rule.
- The shape is the first of the two DEC-180 names: the flag still decides, at
  the generation stage, and the skip happens **after `make_move`** where
  `is_check_move` exists. The price is stated rather than hidden -- the quiet
  stage can no longer be left ungenerated, because a stage that is never
  generated cannot be searched for the checking move inside it, so what late
  move pruning saves here is the subtrees and not the move list, and every
  skipped quiet costs one make/unmake and one `is_check`.
- **S218 is now empty of its own content** and is the coordinator's to re-scope
  or retire; its id is not reused.

### Per accepts clause

| clause | where |
|---|---|
| four rules, one commit, one SPRT | `adocs/data/S109_sprt.sh`, pre-registration written, **not run** |
| every threshold a constant with a stated range, none copied | `src/search_params.hpp`, ten parameters, derivations below |
| gated on `depth - lmr_reduction(depth, move_number)` | `lmr_depth_of()` in `src/search.cpp`, clamped at 0, `move_number = legal_moves_counter + 1` |
| the late-move rule sets a skip-quiets flag the staged generator honours, so the quiet stage is abandoned and not merely skipped over | **departed from, under the clause DEC-180 wrote for it (DEC-205)**, see below |
| a forced mate inside the pruned depth, red first | "pruning does not hide a forced mate" gains `6qk/7p/2p2p1B/4R2P/4P1Q1/1p4P1/5P2/6K1 w - - 1 43` |
| no quiet pruned in check, at a PV node, on the first move, near mate | `pruning_node` and `may_prune`, one case each |
| the gives-check exemption | binds all four now, see above |
| the fast suite green | 37 of 37 in both builds |

**The skip-quiets clause, departed from and what is left of it.** The flag
itself is there and still decides: `skip_quiets` in `negamax_at` is set at the
top of the move loop, before any move of that iteration is made, by a count
read from the reduced depth, and past it no quiet is searched. What the clause
asked for on top of that -- a flag *the staged generator honours*, so the quiet
stage is **abandoned** -- is what the exemption cost, and `src/search.cpp`
carries the whole of the price in one place: nothing between the second
generation stage and the skip reads the flag. Every quiet past the count is
still generated, scored, `make_move`d and `is_check`-scanned, and only then
skipped, because `is_check_move` exists nowhere earlier. So late move pruning
saves this node's *subtrees* and not its move list, and the make/unmake and
attack scan per skipped quiet are part of the 7.58 M -> 4.31 M nodes per second
below. The deviation is the first of the two shapes DEC-180 names and it was
taken under that decision's own clause, the mate guard having gone red without
it; DEC-205 records it, and the section above -- "The one deviation, and it is
the one the accepts pre-authorised" -- is the evidence and the measurements.

### Layer (c): which comparisons read the adjusted value

`pruning_eval` in `negamax_at`, computed once beside `static_eval`. A
`TT_BETA_NODE` whose de-normalised score is above the static evaluation raises
it; a `TT_ALPHA_NODE` whose score is below it lowers it; the mate band is
excluded. **Exactly one comparison reads it: the futility margin,**
`pruning_eval + FUT_BASE + FUT_SLOPE * lmr_depth <= alpha`. Everything else
reads the raw static evaluation -- `improving_at()` compares
`state->static_evals[]` across plies, reverse futility compares `static_eval`
against beta, and `tt_store_entry` stores `static_eval`. `TT_PV_NODE` is
deliberately **not** taken: an exact score certifies both directions, but the
licence this file states covers the two bound types and widening it is a change
with its own verdict.

### The ten constants

| name | default | range | off | form |
|---|---|---|---|---|
| `LmpBase` | 733 | 0..27000 | 27000 | (b) census, 95th percentile of the quiet-cutoff index, hundredths of a move |
| `LmpDepthCoeff` | 0 | 0..27000 | 0 | (b) census, **at its floor**: the fitted slope is negative and the range is non-negative by purpose |
| `LmpMaxLmrDepth` | 8 | 0..16 | 0 | (c) midpoint; the census criterion does not discriminate under a flat threshold |
| `FutBase` | 147 | 0..48000 | 48000 | (a) CPW's minor piece in chesso's material scale, 317 - 170 |
| `FutSlope` | 170 | 0..2000 | 0 | (a) CPW's rook less its minor, 487 - 317 |
| `FutMaxLmrDepth` | 8 | 0..16 | 0 | (c) midpoint of a range declared by purpose |
| `HistPruneCoeff` | 576 | 0..16384 | its cap at 0 | (c) midpoint of `M/64 .. M/8` on chesso's own history scale |
| `HistPruneMaxLmrDepth` | 8 | 0..16 | 0 | (c) midpoint |
| `SeeQuietCoeff` | 50 | 0..10000 | its cap at 0 | (b) chesso's exchange scale: under one pawn of `see_value` at lmr depth 1 |
| `SeeQuietMaxLmrDepth` | 8 | 0..16 | 0 | (c) midpoint |

**Every cap reads `lmr_depth < CAP` and not `<=`**, so 0 is an exact off value;
`lmr_depth` reaches 0 and `<= 0` would still fire. That is what the bisection
protocol needs: one release rebuild per half, never an SPRT on the tune build.

**The two coefficients have no off value of their own, and their range top is
not one.** Both margins are the coefficient times the reduced depth --
`-HP_COEFF * lmr_depth` and `-(SEE_QUIET_COEFF * lmr_depth * lmr_depth)` -- so
at `lmr_depth == 0` each is `C * 0` whatever `C` holds: history pruning at
16384 still skips every quiet whose entry is negative, and quiet SEE at 10000
still skips every quiet that loses material. What the range top buys is
silence at lmr_depth 1 and above, which is a range top and not an off switch.
The two additive margins are different and their off values stand: `FutBase`
at 48000 and `LmpBase` at 27000 hundredths are unreachable at every lmr depth,
0 included. So the bisection turns a rule off by its cap, never by its
coefficient. `src/search_params.hpp` says the same beside each parameter.

**The census, and its finding.** `adocs/data/S109_lmp_census.py`, 300
stratified positions of `adocs/data/S018_raw.tsv` at depth 10 with all four
caps off, **904472 quiet beta cutoffs**; `adocs/data/S109_lmp_census.tsv` is
the census and `_fit.txt` the reading. The 95th percentile of the cutoff index
per remaining depth 1..10 is **8 8 6 5 6 6 4 5 3 3** -- it *falls* with depth,
so the published shape "a count that grows with depth" is **not supported by
chesso's own tree**: a deeper node here cuts off earlier in the move order
because its table move and killers are better. The unconstrained line is
`10.00 - 2.00 * lmr_depth`; the range is non-negative by stated purpose, since
a negative coefficient prunes harder the deeper the node and inverts the
mechanism; fitted subject to that bound it is flat at 7.33 moves. Read on the
lmr-depth axis directly the first pass returned `12.67 - 3.50 * lmr_depth`,
which is the axis being a function of the move number and not the tree -- that
wrong table is recorded in the script, because it is the plausible-looking one.

### What the block does to the tree, measured

`tools/search_bench.py`, before -> after, `HEAD` = `50e3661`:

| position | depth 9 | depth 12 |
|---|---|---|
| midgame | 121515 -> 47635 | 638719 -> 141455 |
| kiwipete | 801408 -> 213916 | 3514653 -> 1038779 |
| tactical | 72895 -> 26130 | 341715 -> 175684 |

Best moves at depth 9 unchanged (`c3d5` / `e2a6` / `d7c8q`); at depth 12
kiwipete moves `e2a6` -> `d5e6`. `chesso bench` **27322394 -> 7111579**, -74.0 %,
which is the `Bench:` line the commit carries (DEC-140). Nodes per second fall
with it, 7.58 M -> 4.31 M on the bench: the block adds a `see_ge` and two table
reads per candidate quiet and a make/unmake per skipped one, and what it removes
is mostly cheap quiescence leaves. Wall time for the same bench is 3.60 s ->
1.65 s.

Per rule, each alone against all four off, 300 positions at depth 10
(`adocs/data/S109_lmp_census.py sweep`; nodes, relative, best moves changed):

    all off   57276904  1.0000   0
    Lmp       24675444  0.4308  72
    Fut       42250613  0.7377  13
    HistPrune 56807807  0.9918   3
    SeeQuiet  45805722  0.7997  49
    all on    20321365  0.3548  65

**History pruning is nearly inert at its first setting** -- 0.8 % of the nodes,
3 of 300 best moves -- and that is DEC-194's shadow: with S024's continuation
history reverted the sum this rule reads is plain butterfly history alone, and
most entries at a fresh node are zero. The block's verdict prices late move
pruning, futility and quiet SEE. S222 is where the other term returns.

### Tests, and the red each was observed at

New cases in `tests/test_search.cpp`, all in the S191 suite "search: pruning
and reduction guards", each killed by the mutant named beside it in
`tools/mutants/S109_shallow_pruning.py` -- the last row excepted, which is
breadth and kills nothing, see under the table.

Every line below is the printout doctest produced, copied from a run and never
paraphrased: `.tuning/coord/S109_mutant_kills.txt` from the landing, and
`.tuning/coord/S109_reobserve.log` from the re-observation the fast check asked
for, one release rebuild per mutant, the two agreeing line for line.

| case | mutant | observed red |
|---|---|---|
| the late move pruning count doubles exactly when improving | P06 | `REQUIRE_EQ( doubled, 2 * flat )` values `( 366, 1466 )` |
| history pruning skips the quiet the table has written off | P07 | `REQUIRE_EQ( rule_that_pruned(probe, planted), PRUNE_HISTORY )` values `( 0, 2 )` |
| quiet SEE pruning skips the quiets that lose material and no others | P08 | `REQUIRE_NE( rule_that_pruned(probe, safe), PRUNE_SEE )` values `( 3, 3 )` |
| futility pruning skips a quiet that cannot reach alpha | P09 | `REQUIRE_EQ( futile, 0 )` values `( 40, 0 )` |
| a quiet move that gives check is not pruned | P05 | `REQUIRE_EQ( rule_that_pruned(probe, checking), PRUNE_NONE )` values `( 1, 0 )` |
| no quiet is pruned at a PV node | P01 | `REQUIRE_EQ( probe.pruned_count, 0 )` values `( 40, 0 )` |
| no quiet is pruned at a node in check | P02 | `REQUIRE( probe.move_count > 1 )` values `( 1 >  1 )` |
| a node's only legal move is never pruned | P03 | `REQUIRE_EQ( probe.move_count, 1 )` values `( 0, 1 )` |
| no quiet is pruned against a beta inside the mate band | P04 | `REQUIRE_EQ( probe.pruned_count, 0 )` values `( 40, 0 )` |
| no quiet is pruned against an alpha inside the mate band | P0A | `REQUIRE_EQ( static_cast<size_t>(probe.move_count), legal_count )` values `( 4, 5 )` |
| no defender node inside the mate band prunes a quiet | breadth, none | 104 rows, 0 violations |

**Two of these reds are the assertion before the one the rule is about, and
that is a property of the case rather than an accident.** P0A and P02 both go
red at a whole-loop or precondition count, because a move a rule skips is a
move the node never searches: the move list falls short before `pruned_count`
is read. **And P09 can only be killed by its case's second half.** The first
drive's window is `FUTILE_ALPHA, FUTILE_ALPHA + 1`, so comparing the margin
with beta instead of alpha is the same comparison one point wider and the
case's own precondition makes both true -- the wide window is what separates
them. The three in-test comments said otherwise until the fast check over this
step caught them; they now carry the printouts above.

**The last row kills nothing, and that too is measured.** All ten mutants
leave "no defender node inside the mate band prunes a quiet" green. Its drives
pass `beta = alpha + 1` with the S165 defender set's alpha between -48997 and
-48991, so `pruning_node`'s `beta > -MATE_MIN` refuses every node before
`may_prune`'s alpha clause is read, and P0A -- which drops exactly that clause
-- cannot show here. The case is reach over 104 positions, and the single-node
case above it is the guard test DEC-141 clause 2 asks for.

**The mate case's own red.** The position added to "pruning does not hide a
forced mate" is `6qk/7p/2p2p1B/4R2P/4P1Q1/1p4P1/5P2/6K1 w - - 1 43`, whose key
is a late quiet rook move onto a square the black queen attacks. Oracle, and
not a reading of the board: stockfish through python-chess at depth 20 reports
`#+2`, 1918 nodes, pv `e5e8 g8e8 g4g7`; python-chess reports `is_valid() True`,
`is_check() False`, 36 legal moves of which 1 is a capture, `Re8` in its list of
quiet moves onto an attacked square and not in its list of quiet moves that give
check. Found by filtering `adocs/data/S145_mate_set.tsv` and
`S145_mined_set.tsv` for that shape, not invented. **Observed red with the
exemption removed** (mutant P05, the whole block otherwise as it ships):

    TEST CASE:  pruning does not hide a forced mate
    FATAL ERROR: REQUIRE( result.mate_in == 2 ) is NOT correct!
      values: REQUIRE( 3 == 2 )
      logged: mate by a hanging quiet, depth 6

The mating move `Qg7#` is the checking quiet the exemption keeps; without it
the engine reports mate in three.

Mutants: `tools/mutants/S109_shallow_pruning.py`, ten, **10 of 10 killed**,
verified by applying each pair to the working tree and running its case
(`.tuning/coord/S109_mutant_kills.txt`). `tools/mutation_check.py` wants a
worktree at a commit holding the rule, so the tool's own pass is the
coordinator's after the commit -- `python3 tools/mutation_check.py tools/mutants
.ref-builds/mut --only P01 P02 P03 P04 P05 P06 P07 P08 P09 P0A`.

**Three cases of S191's own suite were re-stated, not relaxed.** "a capture is
not reduced", "a quiet move that gives check is not reduced" and "a reduced move
that beats alpha is searched again at full depth" all need the node to run its
whole move loop, and `FAIL_LOW_BETA`'s fail-low window is exactly the window
futility fires on: at a non-PV node the first read 8 of 48 moves. They drive a
**PV node** now, where the block is off and late move reduction -- which does
not read `is_pv` -- decides exactly what it decided before; the assertions are
unchanged. The third also needed a different position, because with the block
live in the children no late quiet at perft position 2 comes back from its
reduced search worth more than the ordering thought (40 reduced, 0 re-searched,
at every depth from 4 to 8). It uses
`6k1/1p1b1pb1/1r1p2p1/3Pp2p/1B1p4/3P1BP1/2P2PKP/1R6 w - - 2 28`, row 234 of
`adocs/data/S024_census_positions.txt`, found by scanning that committed set --
3 to 5 of its 26 reduced moves re-search at every depth from 4 to 8.

### Goldens re-derived (DEC-142)

- `tests/test_search_params.cpp` `golden_defaults`: 28 rows -> **38**, the ten
  new parameters with their ranges. Derivation is `src/search_params.hpp` and
  the diff of the two is the re-derivation; `DEV_MANUAL.md`'s table row moved
  with it.
- `tests/test_mate_carry.cpp` `short_line_ceiling`: re-derived with
  `adocs/data/S203_case_sweep.sh --ceilings` over the two S204 grids **and this
  step's own**, `adocs/data/S109_sweep_block.txt` (108 cells, the script's full
  grid, taken with the block live). **Three ceilings rise**: D 1 -> 2, E 8 -> 9,
  F 2 -> 5. A and B are unchanged and C stays at its earned 0. **A rise is a
  decision and not a re-derivation, and this one is proposed rather than
  assumed**: a block that takes 74 % of the tree stores fewer lines, so the walk
  certifies fewer, and DEC-122's guarantee beside them -- a line published at
  its claimed length ends in checkmate -- is **0 unreached across the whole of
  this grid**, as it was across S204's. What rose is the residue, not the
  promise. S202 still owns closing the class.

### Debug self-play (DEC-141 clause 1)

`cmake --build build-debug -j12`, then four rounds at 4+0.04 on
`noob_3moves.epd` with `-log ... level=trace engine=true`: **8 games in 15 s, 0
`Assertion`, 0 `disconnect`**, in both the trace log and the tee'd stdout.

### Gate

`cmake --build build -j12 && ctest --test-dir build -L fast` **37 of 37**;
`cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast` **37 of
37**; `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` clean. Run again after
this section was written. `tools/gate_extra.sh` is the coordinator's.

**The export is not optional on this machine**: `clang-format.sh` pins major
23, this machine has 18 and 22 only, and without `CLANG_FORMAT_MAJOR=22` the
script resolves Ubuntu's unsuffixed 18.1.3, refuses it by version and takes
`test_clang_format_script` red in both builds (`.moltke.local.md`, DEC-146).

### The pre-registration

`adocs/data/S109_sprt.sh`, written and **not run**. `./fastchess.sh` at its own
default bounds, `elo0=0 elo1=5`, alpha = beta = 0.05, nElo, against `HEAD`;
8+0.08, Hash 16, `noob_3moves.epd`, concurrency 12. Worst case **41861 games /
19.8 h** with the truth at the interval's midpoint and **25591 / 12.1 h** with
it on a bound, at the measured 2110 games an hour. Abort at a time-forfeit rate
over 1.0 % a side; a crash voids. The three outcomes, the bisection protocol on
H0 with its two legs and its first suspects, and the open findings the run is
taken while open -- S210, S223, S213 by finding id -- are all in the header.

### What still needs a decision

1. **The three raised mate-carry ceilings** above: `tests/test_mate_carry.cpp`
   says in as many words that raising one needs a decision.
2. **S218's re-scope or retirement**, its content having landed here.
3. One prose line of this file was reworded so `tools/plan_prose_check.py
   --params` stops reading a derivation's intermediate as a parameter value;
   no number changed.

### Repaired before the commit, after the Tier-1 fast check

Documentation only -- no value, no behaviour, `chesso bench` still **7111579**
and the two builds still 37 of 37.

1. **Three in-test recorded reds were wrong and are now the real printouts**,
   re-observed one release rebuild per mutant and logged to
   `.tuning/coord/S109_reobserve.log`: P09 (its recorded assertion cannot fail
   at all), P0A and P01. Three more were repaired with them, found by the same
   pass: P02 named a different assertion, P04 a different number, P07 a
   different case. The remaining four were spelled as `REQUIRE( a == b )` where
   doctest prints `REQUIRE_EQ( a, b )` and now read as printed. The breadth
   case claimed a P0A red that no mutant here can produce; it says what it is
   instead. The table above is the record and it needed no number changed.
2. **The two coefficients' `off` column said their range top.** It is not one:
   both margins are the coefficient times the reduced depth, so at
   `lmr_depth == 0` they are 0 whatever the coefficient holds and both rules
   still prune. Corrected in the table, in the paragraph under it and in
   `src/search_params.hpp`, where the history comment also contradicted itself.
3. **The skip-quiets accepts row claimed the clause was met.** It is departed
   from under DEC-180's own clause (DEC-205); the row says so and the paragraph
   under that table says what is honoured, what is not and what it costs.
4. `src/data_structures.hpp`: `enum prune_rule_t` was reading as the tail of
   `search_node_probe_t`'s doc block. It has its own comment now.
