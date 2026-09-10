id:         S109
goal:       late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
accepts:    all four rules land in one commit and are measured by **one** SPRT, whatever it returns, recorded as it comes (INV-6); every threshold and margin is a constant in src/search_params.hpp with a stated range and none of them is a number copied from anywhere (DEC-084); the four rules are gated on `depth - lmr_reduction(depth, move_number)` and not on raw depth; the late-move rule sets a skip-quiets flag the staged generator honours rather than `continue`-ing, so the quiet stage is abandoned and not merely skipped over; **a position with a forced mate inside the pruned depth is added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guards removed and the printout recorded**; no quiet is pruned while in check, at a PV node, on the first move, or when alpha or beta is near mate -- all four rules, no exceptions; **the gives-check exemption binds the three per-move rules only** -- futility, history pruning and quiet SEE, which run after `make_move` where `is_check_move` exists (`src/search.cpp` `negamax`) -- **and does not bind late move pruning**, whose skip-quiets flag is honoured at the generation stage before any move is made, where the engine has no pre-make gives-check predicate to consult and the published LMP form carries no such exemption; so LMP may end a quiet stage that still holds checking quiets, which is stated here rather than tested away, and buying it the exemption with a post-make prune is a departure from the published form and the owner's call, not the implementer's; for every rule, the test asserts the precondition that would otherwise prune the move, against the exemptions that bind that rule; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   razoring, which is a node-level rule and is S116; SEE pruning of **captures** in the main search, which is S091; futility inside quiescence, which is S112; the improving flag, which S108 supplies and this step consumes
decisions:  DEC-071, DEC-082, DEC-084, DEC-087, DEC-105, DEC-134
closes:
blocks:
paused_by:
done:

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
  depth`: `FUT_SLOPE` = 487 - 317 = **170** and `FUT_BASE` = 317 - 170 =
  **147**. The 2026-08-19 pass read the same two words in `see_value`'s scale
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
   such exemption either. What is left for the owner is the narrower question
   the field now names: whether to buy LMP the exemption with a post-make
   prune, which departs from the published form -- recorded, not silent.
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
