id:         S022
goal:       decide between delta pruning and the per-move futility S112 adds, by measurement -- deleting delta pruning is a valid recorded outcome
accepts:    two SPRT verdicts, one per change and each against the commit before it, in either order: delta pruning, and the re-measure of the S015 quiescence SEE pruning, which measured 0 Elo when see() cost 12.1 % more than it does now (an Apple-machine figure, pre-DEC-049)
touches:    src/search.cpp quiescence, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_mate_carry.cpp, tools/mutants/
excludes:
decisions:
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-29 01:06 CEST
done:

## Outstanding question this step answers

S015's quiescence pruning measured 0 Elo as a trade against a `see()` that was
subsequently made 12.1 % cheaper (measured on the Apple machine, pre-DEC-049).
Nobody has rerun it. Do that here rather than as a separate step, since both
changes are in the same function -- but measure them one at a time, one SPRT
each, or neither number means anything. The accepts asks for exactly that:
two verdicts, either order.


## Re-targeted 2026-08-19

The step used to be "add delta pruning". It is now "choose", and the reason is
that one engine **gained** by deleting delta pruning once per-move futility
(S112) existed -- the two prune overlapping sets and having both was worse than
having the better one. `specs.md` already folds the S015 re-run into this step;
that re-run belongs here too, for the same reason: SEE pruning in quiescence
measured 0 here at a time when per-move futility was absent, which is exactly
when the surveyed record expects it to measure 0.

So this step now decides three things against one another with S112 in the
tree: per-move futility alone, futility plus SEE, futility plus delta. Deleting
is a valid outcome and gets recorded as one (DEC-019, and S005/S006/S015 are
the precedent for keeping or dropping on a measured zero).

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

- **Delta pruning has two published forms** (CPW Delta Pruning). The
  **per-move** rule: "test whether the captured piece value plus some safety
  margin (typically around 200 centipawns) are enough to raise alpha". The
  **node-level "big delta"** early-out: skip move generation entirely when
  even the biggest possible gain cannot help -- the page's sample tests the
  static score against `alpha - BIG_DELTA` with BIG_DELTA 975 (a queen),
  += 775 when a promoting pawn exists. One stated disable: "switched off in
  the late endgame", because piece values there understate insufficient
  material and transitions into won endgames bought with material.
- **The per-move form is S112 under its modern name** -- S112 section 1 is
  the taxonomy and the traced record (Weiss #188/#355, Lynx #731/#752/#1142,
  SF fail-soft-raise prose); none of it is re-derived here. hgm's framing
  (talkchess t=75059) matches: delta pruning "pre-empts the stand-pat cutoff"
  and needs a margin over an upper limit of the post-move evaluation.
- **The published trend is deletion once per-move futility exists.** Weiss
  #455 (2021-06-25, re-verified): +1.87 +/- 3.51 at 8+0.08 and +6.74 +/- 5.26
  at 40+0.4 *for deleting delta*, at Weiss's simplification bounds
  [-4.00, 1.00], rationale quoted in S112 section 1. Lynx #731 measured delta
  adds negative in a SEE-gated tree. Which delta form Weiss deleted is not
  readable from prose (the diff is off limits), but the rationale sentence
  itself distinguishes it from the per-move futility that stayed.
- **What remains for delta once S112 exists is exactly the node-level
  early-out**: one compare before `generate_captures()` that skips the whole
  generate-filter-search machinery where no capture can reach alpha. Per-move
  futility cannot buy that -- it runs after generation, per move. But it is
  **not** a pure speed transform of S112's test: S112 exempts promotions and
  checking captures, and a node-level test cannot see gives-check at all
  (promotions it can, via the allowance term). So it prunes a superset and
  owes an SPRT, never an INV-6 identity claim.
- **The endgame disable has a counter-record**: Stockfish removed the
  non-pawn-material check from its qsearch pruning as a passed simplification,
  STC and LTC at <-1.75, 0.25> (3dfbc5d, 2025-02-05, message only) -- an NNUE
  engine, so direction only (DEC-019). The guard ships in the candidate per
  CPW; its threshold is a swept constant, not a belief.

### 2. Shape for chesso

Line numbers at 0edfd26 (`src/` byte-identical to cf89e22); re-locate by
symbol once S112/S131 have edited the function.

- - **Current truth: chesso has no delta pruning, in either form.** No delta
  test exists in `quiescence()` -- the only skips today are the non-capture
  drop and S015's SEE gate, both in `src/search.cpp` `quiescence`
  (`!in_check && !capture_cannot_lose && !see_ge(move, 0)`), and specs.md's
  "absent, search" row lists delta pruning absent. The goal's "deleting delta
  pruning" therefore means the trial add is reverted or never bought and the
  zero recorded -- no shipped code comes out on that arm.
- - **The tree this step measures against**: after S112 (per-move futility in
  the filter loop above the SEE gate of `src/search.cpp` `quiescence`, with the
  fail-soft raise into `best_value`, promotion / gives-check / in-check
  exemptions, QS_FUTILITY_MARGIN plus a dedicated `qs_futility_value[]`) and
  S131 (quiet queen promotions pass the same filter). Both land first in plan
  order,
  their verdicts recorded whatever they were.
- **The re-target's three configurations, named.** Baseline F+S = futility
  plus S015's SEE gate, as S112/S131 leave the loop. **Verdict 1** = delete
  the SEE gate: F vs F+S -- the S015 re-measure the accepts fold in.
  **Verdict 2** = add the node-level early-out D on top of verdict 1's
  winner: F+D vs F, or F+S+D vs F+S. Two sequential SPRTs, each against the
  commit before it (accepts; DEC-020), walk one path through the three; the
  third pairwise comparison is not bought and not owed.
- - **The early-out's site and arithmetic**: under `!in_check`, between the
  qply cap and the generation call in `src/search.cpp` `quiescence`. Reuse
  S112's numbers rather than minting new ones: skip when `futility_base +
  qs_futility_value[QUEEN] + promo_allowance <= alpha`, where promo_allowance =
  queen minus pawn from the same table when a friendly pawn stands on the
  seventh, else 0. Fail-soft return is that same ceiling -- the node-level
  analog of S112's per-move raise; returning bare stand_pat would claim an
  upper bound the pruned moves can exceed -- stored TT_ALPHA_NODE via the
  `src/search.cpp` `quiescence` pattern. A fired node is a certified fail-low,
  unlike the `src/search.cpp` `quiescence` cap return, which is a truncation
  and rightly stores nothing.
- **Endgame disable**: `game_phase()` (`src/evaluation.cpp` `game_phase`, the
  INV-4 phase accumulator clamped to 24) is the O(1) predicate the null-move
  zugzwang guard already keys on (`> 0`, `src/search.cpp` `negamax`). Disable
  the early-out at `game_phase() <= QS_DELTA_PHASE_MIN`, seed in section 4.

### 3. Implementation sketch -- the decision protocol

Two SPRTs, priced as two verdicts by plan.md ("S097 and S022 two each");
either order per the accepts. Recommended order, reason stated:

1. 1. **Verdict 1, the S015 re-measure: delete the SEE gate** (`src/search.cpp`
   `quiescence`, one condition block, capture_cannot_lose's call goes with it).
   H1 at the non-regression bounds -> deleted: the gate re-measured ~0 even
   with futility present and a see() 12.1 % cheaper; specs' quiescence row and
   its Open items entry are rewritten in the same commit, and a decisions.md
   entry records the deletion superseding S015's kept-at-zero (AGENTS.md
   section 8, before or alongside). H0 -> kept, now with a current number; the
   open item is discharged either way. Run first so verdict 2's condition is
   written against the loop that survived.
2. **Verdict 2, the delta early-out, on verdict 1's winner.** Before booking
   the match, the DEC-079/S094 pre-check: instrument the condition's fire
   rate over the three search_bench positions at depth 12 plus one kiwipete
   run -- a near-zero fire rate predicts the zero before it is bought, a real
   one says what fraction of `generate_captures()` calls the early-out saves.
   The SPRT is still owed by the accepts; the probe sizes expectation, not
   whether to run. H1 -> shipped; constants join S127. H0 -> not shipped,
   recorded as zero -- the goal's "deleting delta pruning" outcome.
3. **Artifacts**: each verdict lands in this step's stamp and in specs.md's
   current-state rows in the same commit as its change; a DEC entry only
   where a recorded outcome is superseded (deleting the S015 gate).

### 4. Constants and seeds

- **Margin: none new.** The early-out consumes S112's QS_FUTILITY_MARGIN --
  CPW's "typically around 200" is that seed, already marked **seed -- must be
  fitted if kept** (https://www.chessprogramming.org/Delta_Pruning). A
  delta-specific margin is minted only if S127's sweep says the two tests
  want different slack.
- **Queen ceiling and promotion allowance**: from S112's
  `qs_futility_value[]` -- 900, and 900 - 100 = 800 with a friendly pawn on
  the seventh. CPW's form is BIG_DELTA 975 += 775 on their scale; ours is
  internal reuse of the table S112 seeds from see_value, no provenance
  question, and the 800 agrees with S131's SEE arithmetic for a quiet
  promotion (+800) -- the consistency the siblings require.
- **Endgame disable**: `QS_DELTA_PHASE_MIN` in src/search_params.hpp, **seed
  0** (pawn endgames only -- the boundary the null-move guard already uses),
  range 0..24 for SPSA. **Seed -- must be fitted if kept.** CPW names the
  disable but no threshold; SF's 3dfbc5d removed its equivalent guard, so a
  sweep returning "never disable" or "disable wider" is a legitimate fit.

### 5. Pitfalls

- **Measuring near-zero at DEC-063's bounds.** The record: `elo0=0 elo1=5`
  on a ~+3 truth random-walked 9036 games and 6 h 36 m, with 22.5 h more
  owed to the 40000-game cap -- "crawl at the bounds". Both of this step's
  effects are expected within a few Elo of zero, so both pairs put **zero on
  a hypothesis, not mid-interval**: deletion at `elo0=-5 elo1=0` (truth 0
  sits on H1, terminates), add at `elo0=0 elo1=5` (truth 0 sits on H0,
  terminates). The crawl case left is a truth near -2.5 or +2.5:
  pre-register a games cap (one night, ~20000 games at S105 throughput --
  plan.md prices a normal verdict at 45-75 min) and the no-bound default,
  which is the null action -- keep the gate, do not ship delta -- per the
  S094/DEC-079 precedent of recording a no-bound run as zero.
- **Promotion gain in the ceiling.** S131 lands first: quiet queen
  promotions are in the loop, exempt from S112's futility, worth +800 on the
  internal scale. An early-out without the allowance prunes the exact class
  S131 shipped, at every node with a pawn on the seventh. CPW's sample
  carries the term; Ethereal b7f142a is delta's recorded promotion bug (S112
  section 1).
- **Gives-check is invisible at node level.** S112 searches checking
  captures that fail its futility test; the early-out cannot, short of
  generating. It is the one exemption the node-level form structurally
  drops, the reason this owes an SPRT, and the first suspect if verdict 2
  reads negative -- the honest fallback is a wider margin or no ship, not a
  cheap gives-check oracle that does not exist (S112 section 2).
- - **The SEE skip must not take the futility raise.** Moves reaching
  `src/search.cpp` `quiescence` survived S112's test, so their futility_value
  exceeds alpha; raising best_value to it on a SEE skip fakes a fail-high
  through the `src/search.cpp` `quiescence` store. S112's asymmetry -- futility
  skip raises, SEE skip does not -- is correct, not an oversight. Verdict 1
  deletes the gate or keeps it as-is; "fixing" the raise is a bug, and this
  answers the question S112 section 5 deferred here.
- - **game_phase and the disable.** The phase is an INV-4 accumulator,
  promotion-clamped at 24, already the zugzwang boundary at `src/search.cpp`
  `negamax` -- reusing it costs one compare. What it does not capture:
  low-material positions at phase > 0 (minor-piece endings), which is the
  threshold's range and S127's business if delta ships.
- **Two changes, one function, strict sequence.** Verdict 1's commit is only
  the gate deletion (or nothing); verdict 2's only the early-out. The fast
  suite's two quiescence mate cases -- "a side in check may not stand pat"
  (`tests/test_search.cpp` "a side in check may not stand pat") and "mate is
  recognised at depth zero" (`tests/test_search.cpp` "mate is recognised at
  depth zero") -- gate each commit; pruning hiding mate is the recurring bug --
  and the early-out never fires in check by construction, so the
  `src/search.cpp` `quiescence` mate path stays reachable.

### 6. Measurement

Both at the S105 regime -- 8+0.08, Hash=16, UHO book -- fast suite and mate
cases green first, each verdict against the commit before it, expectation
stated before each run (DEC-063):

- **Verdict 1 (SEE-gate deletion): `elo0=-5 elo1=0`**, S105's stated
  non-regression pair -- a deletion earns its place by proving
  non-regression. Expected ~0 (S015's zero, see() cheaper since, futility
  eating the overlap). Keeping H0 live: Weiss #355 measured SEE added on top
  of futility at +35.7/+24.9 (S112 section 1), so a regression on deletion
  is a published outcome, not a straw man.
- **Verdict 2 (delta early-out add): `elo0=0 elo1=5`**, S105's gainer pair
  -- an add ships only on evidence of gain; accepted at "merely not a
  regression" it would ship complexity with no measured value. Expected ~0
  or negative (Weiss #455 gained by deleting, Lynx #731 negative on adding);
  a true zero terminates on H0.
- **A ~0 verdict is recorded as zero and is a valid outcome on both arms**
  (DEC-019; S005/S006/S015), and on verdict 2 that outcome *is* the goal's
  "deleting delta pruning". Record alongside: search_bench depth-12 node
  counts before/after each commit (play-altering, no identity claim -- the
  deltas are the datum) and verdict 2's pre-check fire rate.

### 7. Interactions

- **S112 (input)**: supplies futility_base, `qs_futility_value[]`, the
  exemption set and the fail-soft raise the early-out's return mirrors; its
  section 7 names this step as its verdict's consumer.
- **S131 (input)**: quiet queen promotions are why the allowance term is
  load-bearing; its section 7 asks this step to price promotion gain --
  priced in sections 2 and 4.
- **S015 (subject of verdict 1)**: filter-loop test order stays futility ->
  capture_cannot_lose -> see_ge, cheapest first (S112 section 2); the gate's
  skip correctly does not raise best_value (section 5).
- **S039 (different layer, note only)**: when the lazy shortcut fired,
  stand_pat is an alpha-side overstatement (`src/evaluation.hpp` "one it would
  have taken, and both are true statements"), so futility and the early-out
  both fire less -- conservative, sound. S039 moves how often that happens, not
  the arithmetic.
- **S130 (input, via S112)**: a TT-tightened stand-pat lowers futility_base
  and fires the early-out more -- honest, the entry certifies value <= s;
  the pre-substitution base stays the bisect lever.
- **S127 (after)**: QS_DELTA_PHASE_MIN and the shared margin join the sweep
  only if verdict 2 ships; a deleted SEE gate removes no constant.

### 8. References

Every URL read for this section; commit and PR text read as prose only, no
diffs (DEC-016, DEC-084). The sibling records built on here -- Weiss
#188/#355, Lynx #731/#752/#1142, Ethereal b7f142a, the SF qsearch-futility
messages -- are cited in S112's and S131's sections 8 and not re-fetched.

- https://www.chessprogramming.org/Delta_Pruning -- the per-move rule and
  ~200 cp margin; the big-delta sample (975, += 775 on a promoting pawn);
  the late-endgame switch-off and its stated reason.
- https://github.com/TerjeKir/weiss/pull/455 -- re-verified: "Remove delta
  pruning", 2021-06-25, +1.87 +/- 3.51 at 8+0.08 / +6.74 +/- 5.26 at 40+0.4,
  simplification bounds [-4.00, 1.00].
- -
  https://api.github.com/repos/official-stockfish/Stockfish/commits/3dfbc5de25705fadcb4b5b7a551eacb3eb75d171
  -- "Remove non-pawn material check in qsearch pruning", 2025-02-05, passed
  simplification STC and LTC at <-1.75, 0.25>; message only.
- https://talkchess.com/viewtopic.php?t=75059 -- hgm on delta pruning
  pre-empting the stand-pat cutoff and needing a margin over a post-move
  evaluation ceiling; no measurements.

## Verdict 1: S015's exchange gate deleted, behind a switch (2026-09-29)

Section 3's first item, built first as it recommends. The candidate deletes
S015's gate from quiescence's filter loop behind a switch and asks at
`{-5, 0}` nElo whether the tree is not worse without it (section 6). Verdict 2,
the node-level delta early-out, is later work on the tree this one leaves and
is not built here.

### What the tree already did, 2026-09-29 (checked at `cbaa699`)

The step file was written on 2026-08-19; the implementing agent checked each
assumption on the tree before writing code, by symbol. `cbaa699` is S131's
tree, H1 on 2026-09-29; its `src/` and `tests/` are `d664a8a`'s.

- **The filter loop's shape** (`src/search.cpp` `quiescence`): out of check
  `generate_captures`, in check `generate_moves`; S130's stand pat and its
  substitution above the loop; then, per move, (1) S131's drop, `!in_check &&
  !MOVE_CAPTURE(m) && !(QS_QUEEN_PROMOTIONS != 0 && MOVE_PROMOTED(m) ==
  TO_QUEEN)`; (2) S112's block, `qs_futility && !MOVE_PROMOTED(m)` -- the
  promotion exemption before any arithmetic, the best case folded into
  `futility_best`, a capture that gives check exempt by make, `is_check` and
  unmake, an illegal one dropped there; (3) S015's gate, `!in_check &&
  !capture_cannot_lose(m) && !see_ge(m, 0)`; (4) `capture_score` and the
  compaction; then the search loop, where S210's dead-board test sits. Section
  2's "the only skips today are the non-capture drop and S015's SEE gate" is
  S112's and S131's pre-landing truth and is kept as written.
- **The same pair elsewhere, untouched**: ProbCut's own capture filter in
  `src/search.cpp` `negamax_at` (S113) declines on `capture_cannot_lose` and
  `see_ge(0)`, and S091's capture SEE pruning and extra reduction ask
  `see_ge` in the main search. Neither function changes; only quiescence's
  call goes, which is section 3's "capture_cannot_lose's call goes with it".
- **S112's asymmetry (section 5)**: the gate's skip does not take the futility
  raise, and with the gate deleted there is no second skip to take one -- every
  move that survives the futility test is searched and the fold stays
  futility's alone. Nothing was "fixed".
- **S015's own cases** in `tests/test_search.cpp`, "the exchange comes out at
  the right value", "a quiet move into an attack loses material" and "over
  every legal move of every test position", assert `see()` and `see_ge()` and
  never the gate, so they stay true untouched: the switch touches neither
  function. The one case that asserted the gate's decision in quiescence is
  S131's "the exchange gate declines a queen promotion onto a defended square"
  (one node, the static score); S112's "a capturing promotion below the
  threshold is searched" leans on the gate for what its count means (the push's
  four promotions declined, so a child is a capturing promotion's), and its
  settled-move assertion is what carries it once the gate is gone.

### What landed

- `src/search.cpp` `quiescence`: the gate gains a leading `QS_SEE_GATE != 0 &&`
  and a paragraph saying so; the filter comment's S015 sentence and the store
  comment's "since the losing captures were declined" say what holds at each
  value. Nothing else in `src/`: S112's block, S131's condition, S113's filter,
  S091's pruning and the generator are untouched.
- `src/search_params.hpp`: `QS_SEE_GATE` ("QsSeeGate", **0**, 0..1), a verdict
  switch in `QsFutility`'s, `ProbCut`'s and `QsQueenPromotions`' shape the other
  way round -- its 1 is the parent -- its comment stating the class, the value
  that ships, that there is no seed, and what each reading does with the row;
  S131's row names the gate "at QsSeeGate 1".
- `tests/test_search.cpp`, a new suite "search: quiescence exchange gate": "a
  losing capture is searched at the gate's off value", "a capture the exchange
  evaluation clears is searched", "a losing evasion is searched in check", and
  in the tune build "the gate declines a losing capture at the switch's on
  value". In "search: quiescence promotions", S131's gate case is kept as
  written and driven at `QsSeeGate` 1 in the tune build, beside its restatement
  at 0, "a queen promotion onto a defended square is searched at the gate's off
  value". S112's promotion case's comment says what its count means at each
  value. `tests/test_search_params.cpp`: the golden row, 69 -> 70 defaults.
- Four goldens re-derived by their own scripts (below), in
  `tests/test_search.cpp` and `tests/test_mate_carry.cpp`.
- `tools/mutants/S022_v1_see_gate.py`, U01 to U05; M18 in
  `tools/mutants/search.py` and V07, V09, V10 in
  `tools/mutants/S131_quiet_queen_promotions.py` re-pointed at the switched
  condition, the same edits, M18 and V07 declared equivalent in the release
  build while the switch ships at 0.
- `MANUAL.md` the option row, and the `QsFutility` and `QsQueenPromotions` rows
  saying the exchange evaluation is asked at `QsSeeGate` 1; `DEV_MANUAL.md` the
  bench ledger entry and the four golden rows; `adocs/data/S022_v1_sprt.sh`,
  `adocs/data/S022_v1_sweep.txt`, `adocs/data/S022_remine_s097.log` and their
  `adocs/data/README.md` rows.
- `touches:` above gained the files the switch, its cases, the goldens and the
  mutants live in, which the step as written did not name.

### Seeds (DEC-134)

None. A deletion has no number in it, and the gate's own bar, 0, is "the
exchange loses material", a definition; the switch's comment says so.
Implemented from this file's description (section 3, item 1; DEC-221); no
other project's code was opened.

### The reach, before any game (stated as reach, DEC-239)

An instrumented copy of the candidate (`.ref-builds/census`, the fixture's
`src/` plus write-only counters, `.tuning/coord/S022_census_instrumentation.patch`),
its tune build at `QsSeeGate` 1 and at 0, benches the uninstrumented builds'
own totals at both values -- 1694808 and 2393854 at depth 12, `search_bench`
12 at 104784 / 244824 / 117798 and 97921 / 567770 / 125308, every reply
identical -- so its counters change nothing. The gate's set is every move past
the filter's drop, out of check, that the exchange evaluation writes off,
asked of each such move whether or not the gate reaches it
(`.tuning/coord/S022_census.txt`, `.tuning/coord/S022_census_summary.txt`):

| run, today (`QsSeeGate` 1) | loop moves past the drop | written off | skipped by futility first | declined by the gate | of those, futile but checking |
|---|---|---|---|---|---|
| `bench 12`, eight positions | 774938 | 378178 (48.8 %) | 26425 (7.0 % of the set) | 351753 (45.4 % of the loop) | 5283 |
| `search_bench` 12, three positions | 374458 | 153258 (40.9 %) | 14331 (9.4 %) | 138927 (37.1 %) | 2957 |

**The overlap the re-target is about is small from the gate's side**: of the
moves the exchange evaluation writes off, per-move futility takes 7.0 % before
the gate sees them over the bench positions and 9.4 % over the `search_bench`
three. From futility's side it is larger: 40.4 % and 37.5 % of futility's own
skips (65328 and 38218) are moves the gate would have declined too. Today the
gate costs 709610 `capture_cannot_lose()` calls and 504765 `see_ge()` calls
over the bench positions at depth 12. The declined include 6499 queen
promotions that take nothing (S131's class, the 96.5 % of it S131's census
recorded) and 345254 captures. **With the gate deleted** (`QsSeeGate` 0) the
same eight positions at depth 12 keep 685767 moves the exchange evaluation
writes off, search 367882 of them, and 96122 of those raise the node's best and
5484 cut off; quiescence nodes go from 626337 to 1410143. This is how often
the class occurs, not what it is worth: S131's census read 0.85 % and the
match read H1 at +17.84 (DEC-239).

### Measurements (2026-09-29, niced beside the fillers; counts only)

- **The on value** (DEC-215), a parent built from `cbaa699` in
  `.ref-builds/parent`: the tune build at `QsSeeGate` 1 prints `bench`
  **3429473** with all eight replies the parent's (c3d5 e2a6 d7c8q g7h8q d8e7
  a1b2 e5e6 e5e6), `bench 12` 1694808 with its replies, and `search_bench`
  reproduces the parent at 9 (48304 / 71580 / 25413) and 12 (104784 / 244824 /
  117798), best moves c3d5 e2a6 d7c8q at both (`.tuning/coord/S022_identity.log`,
  its driver `.tuning/coord/S022_identity.sh`).
- `bench` 3429473 -> **6049266** (+76.4 %), **the eight replies unchanged**;
  the tune build at the defaults prints the same 6049266. `bench 12` 1694808 ->
  2393854, replies unchanged.
- `search_bench` 9: 48304 -> 39854, 71580 -> 249885, 25413 -> 33736; 12:
  104784 -> 97921, 244824 -> 567770, 117798 -> 125308; best moves unchanged.
  Counts only: the change alters play, so INV-6 does not discharge it.
- **Tests, red first and observed** (`.tuning/coord/S022_logs/red_stageA.log`):
  with the switch, its golden row, the `MANUAL.md` row and the cases in place
  and the gate not yet reading the switch, "a losing capture is searched at
  the gate's off value" was red in both builds (the child not entered, nodes 1
  against at least 2), "a queen promotion onto a defended square is searched
  at the gate's off value" red in both (the same two), and the tune build's
  "the gate declines a losing capture at the switch's on value" red at its
  control; "a capture the exchange evaluation clears is searched", "a losing
  evasion is searched in check" and S131's case driven at 1 green, as they must
  be on a tree whose gate still runs. With the gate reading the switch all of
  them green in both builds (`.tuning/coord/S022_logs/green_stageB.log`). The
  first board for the evasion case, `k3r3/8/8/8/8/8/3Q2PP/4r2K w - - 0 1`, was
  mate in one after its forced reply and made "not a mate" false; it was
  replaced before the change by `k7/8/5Q2/8/8/4p3/4nnPP/7K w - - 0 1`, whose
  line the premise pins at three nodes, the leaf standing pat.
- **Three goldens went red with the deletion in both builds, and each was
  re-derived by the script its GOLDEN block names** (DEC-233, DEC-142) -- a
  fourth, S097's multicut row, stayed green and stopped separating the mutant
  it was mined for, which the targeted mutation run below found --
  re-observed red on a fixture of the candidate with the old tests
  (`.tuning/coord/S022_logs/golden_reds_observed.log`):
  1. "pruning does not hide a forced mate", capture-mate row 1 at depth 7, no
     mate. The seven sweeps of `adocs/data/S230_mine_r01_row.py depths`, the
     shipped tree and each of the six S091 mutants over depths 3 to 12
     (`.tuning/coord/S022_capmates/`, driver `.tuning/coord/S022_capmates.sh`):
     shipped `d9 d10 d11 d12`, `d7 d8 d9 d10 d11 d12`, `d10 d11 d12`,
     `d9 d10 d11 d12`; R02 loses row 3 at every depth and R01 row 4 at 12
     alone, C05 and R02 gain early readings on row 1, and nothing else
     separates. The rule puts the rows at **9, 7, 10, 12**, `no S091 mutant,
     since S022`, `no S091 mutant, since S095`, `R02`, `R01` -- R01's
     incidental kill back for the first pass since S230 mined row 4 for it --
     and no mate distance moved. S113's rows are quoted in the GOLDEN block
     for an H0.
  2. "ordering keeps the tree small" aborted at its 65024 budget. The count
     was taken again by `adocs/data/S192_node_budget.py` on a scratch build
     whose case carried a budget the search could not reach (the script reads
     only a passing case): **127342**, against the parent's 20427, which sat
     inside the old pair's middle half. The pair is **509368 / 25468**, 4x and
     a fifth (`.tuning/coord/S022_node_budget.txt`).
  3. `test_mate_carry`: `C_mate7_depth11` 6 short mating PVs of 19 mate lines
     against a ceiling of 0. `adocs/data/S203_case_sweep.sh` over its 108
     cells on the candidate's engine, committed as
     `adocs/data/S022_v1_sweep.txt`; `--ceilings` over the four grids the
     golden names reproduces 5, 15, 0, 2, 11, 5 exactly, and with this grid
     added moves **C alone, 0 -> 6**, its own cell the worst (1500000 nodes,
     stride 1, 19 with 6 short, the count the failing run reported). **The
     test's own comment says a raised ceiling needs a decision, and DEC-241
     is it** (the coordinator, 2026-09-29, in the form DEC-225 set for
     S095's; S236's raise of the same case, 0 -> 2, went the same route): C's
     ceiling moves to 6 in the landing commit, this grid committed as the
     fifth recorded sweep and named in the golden's re-derivation line, and
     on an H0 or no verdict the ceiling returns to 0 by dropping the grid
     from the `--ceilings` command. `unreached.empty()`, the clause that a
     published full-length line ends in mate, holds on all six cases in both
     builds.
- **Both fast suites green, 41 of 41 each** -- Release `build` and
  `-DCHESSO_TUNE=ON` `build-tune`, serial `ctest -L fast`, niced, `-j4`
  builds -- and `./clang-format.sh --check` clean
  (`.tuning/coord/S022_logs/suite_release_B2_green.log`,
  `suite_tune_B2_green.log`). "a side in check may not stand pat" and "mate is
  recognised at depth zero" green in both, run by name as well
  (`.tuning/coord/S022_logs/goldens_named.log`). `test_mate_carry` read 51.32 s
  Release and 50.15 s tune inside the suites. `test_uci_surface` green with the
  new row. `tools/plan_prose_check.py --citations`, `--touches`: 0 flagged;
  `--params` exit 0. **Re-run on the final tree**, after the multicut row
  below: both suites 41 of 41 again and the format check clean
  (`.tuning/coord/S022_logs/suite_release_final.log`, `suite_tune_final.log`);
  `test_search` itself went from 7.4 s to 14.1 s in the suite, the new row's
  depth-14 cell, and `test_mate_carry` read 51.61 s and 50.58 s.
- **Every mutant anchor in `tools/mutants/` validates on the tree**, 159 of
  them, after M18 (`tools/mutants/search.py`) and V07, V09, V10
  (`tools/mutants/S131_quiet_queen_promotions.py`) were re-pointed at the
  switched condition -- the four the switch's text moved, and the tool's own
  validation is what named them. Each makes the same edit it made; M18 and V07,
  whose whole edit is the gate, are declared equivalent in the release build
  while the switch ships at 0, with the argument at each, and the tune run
  below kills both.
- **Mutation** (DEC-141 clause 2), `tools/mutants/S022_v1_see_gate.py` on a
  clean detached fixture, `.ref-builds/mut` at `d0d21ae` -- a throwaway commit
  of this working tree on no branch, cut before the multicut row below moved
  -- header `baseline green, 41 tests, bench 6049266 nodes via engine`; **mutation score 4 of 4 (100 %), equivalent 1, killed 4**, wall
  920 s (`.tuning/coord/S022_mutation.log`, per-mutant logs in
  `.tuning/coord/S022_logs/mutation_main/`). Which case kills which, as a
  pre-check against the named cases had predicted
  (`.tuning/coord/S022_precheck.log`) and the mutant file's header states:

  | mutant | killed by |
  |---|---|
  | U01 switch ignored | "a losing capture is searched at the gate's off value", "a queen promotion onto a defended square is searched at the gate's off value", and the floor of "ordering keeps the tree small" |
  | U02 switch inverted | the same three |
  | U03 gate's sign flipped at 1 | equivalent in the release build as declared; in the tune build "the gate declines a losing capture at the switch's on value" and "the exchange gate declines a queen promotion onto a defended square" |
  | U04 quiet promotions still gated at 0 | "a queen promotion onto a defended square is searched at the gate's off value" alone |
  | U05 captures still gated at 0 | "a losing capture is searched at the gate's off value", and the floor of "ordering keeps the tree small" |

  **The tune build, by the tool**: `--build-dir` a `-DCHESSO_TUNE=ON` build of
  the same commit (`.ref-builds/mut2`), `--only U03 M18 V07` over the whole
  directory, which validates all 159 anchors first: header `baseline green, 41
  tests, bench 6049266 nodes via engine`; **3 of 3 killed** -- U03 by the two
  cases in its row, V07 by S131's case at `QsSeeGate` 1, M18 by the on-value
  case's losing evasion, declined in check and read as a mate -- wall 666 s
  (`.tuning/coord/S022_mutation_tune.log`). The tool closes that run with
  `MUTATION-RUN-FAILED: a verdict differs from what the mutant list expects`,
  because the three are declared equivalent for the release build the tool is
  written for; the kills are what the run is for.

  **Targeted extras**, release, on the same fixture, `--only M18 V07 V09 V10
  E21 B04` over the whole directory -- the four re-pointed mutants and the two
  whose kills rest on mined rows of "pruning does not hide a forced mate":
  header `baseline green, 41 tests, bench 6049266 nodes via engine`; **3 of 4,
  equivalent 2, killed 3, survived 1** -- M18 and V07 equivalent as declared,
  V09 and V10 killed by their futility halves, B04 killed, and
  **`E21_multicut_mate_band_gate_dropped` survived, 0 of 41 red, bench
  unchanged** (`.tuning/coord/S022_mutation_extras.log`). S131's multicut row
  had stopped separating it with the gate deleted: the shipped and the
  guard-dropped builds both report its mate at 12, 13 and 14. **Resolved by
  DEC-238's procedure, not by a new ruling**: `adocs/data/S097_mine_mate_row.py`
  stages 2 to 6 on the candidate with the witness and the pick in guard mode
  (`.tuning/coord/S022_mine_guard.sh`, every stage's output in
  `adocs/data/S022_remine_s097.log`) -- one of the 269 candidates separates the
  sweeps, `2r4r/kq3pb1/N3p1p1/QPp1Pn1p/2PPRP2/7P/5BP1/R5K1 w - - 1 31`, shipped
  `d13 d14` against E21's `d13`, the mutant's rule firing at 14 and the shipped
  one nowhere, which the default mode refuses and the guard mode takes at
  depth 14, mate in 6 -- S236 v2's row come back on another tree, at a cell of
  53836648 nodes, about 7 s as the miner timed it, against S131's row's 2179530
  here. Stockfish at depth 20 in a fresh process: `#+6` for White in 70199
  nodes, the label the candidates file held; python-chess: valid, not in
  check, 38 legal moves, 2 captures, no promotion. The row replaces S131's in
  "pruning does not hide a forced mate", same depth and distance, S131's row
  quoted in the GOLDEN block for an H0; **observed red under E21, green
  shipped** (`.tuning/coord/S022_mine_red.log`). This is the fourth tree in a
  row to move that row; S243's direct guard test is what ends it.

  **The final pass, on the tree that lands**: fixtures re-cut from this
  working tree once every edit to `src/`, `tests/` and `tools/` was in,
  `.ref-builds/mut`, `mut2` and `mut3` at `887f042`, byte-equal to the
  landing's three directories, each run's header `baseline green, 41 tests,
  bench 6049266 nodes via engine`. `tools/mutants/S022_v1_see_gate.py`:
  **mutation score 4 of 4 (100 %), equivalent 1, killed 4**, the kills the
  table above names, wall 945 s (`.tuning/coord/S022_mutation_final.log`).
  The extras, `--only E21 B04 M18 V07 V09 V10`: **4 of 4, equivalent 2,
  killed 4** -- **E21 killed**, by "pruning does not hide a forced mate" at
  the new multicut row, B04 there and at "probcut does not run with beta in
  the mate band", V09 and V10 killed, M18 and V07 equivalent as declared --
  wall 1120 s (`.tuning/coord/S022_mutation_final_extras.log`). The tune
  build, `--only U03 M18 V07`: **3 of 3 killed**, the same cases as before,
  wall 617 s (`.tuning/coord/S022_mutation_final_tune.log`).

### The pre-registration (`adocs/data/S022_v1_sprt.sh`)

`{-5, 0}` nElo through `fastchess.sh --nonreg` (the script's own
non-regression mode, 20000 rounds), 8+0.08, Hash 16, `books/noob_3moves.epd`,
concurrency 12; DEC-063's expectation about zero with H0 live; DEC-143's
41861 games at the midpoint and 25591 on a bound, 19.8 h and 12.1 h at 2110 an
hour, the midpoint past the 40000-game cap; `REF` and `CAND` `PIN_ME` with the
refusal, tested on a copy pointed at the worktree; `OUT` under `.tuning/`; the
abort rule per side; the open findings carried from S131's block by id, 1 to
10, with this verdict's goldens as item 11; the prior (S015's zero, the
overlap claim, SEE on top of futility's published gain); the reach stated as
reach (DEC-239); DEC-236's clause answered -- **neither of S022's verdicts
decides S112's futility**, which is on both sides of both pairs as this file
writes them, so its worth stays S127's fit. **REF is the commit the landing
sits on.** H1: the deletion stands, the gate, the switch and the switch's own
cases leave in a removal commit proved by INV-6 against the candidate, and the
coordinator records the decision that supersedes S015's kept-at-zero; H0: the
gate stays, `QsSeeGate` to 1 and the switch's code out (the S238 pattern), the
goldens back byte for byte; no verdict: the null action, the same as H0,
recorded as a zero. `bash -n` clean.

### Proposed `specs.md` edits (the coordinator edits specs.md)

Quoted against the main checkout's `specs.md` as it stands with S131's
completion staged.

The engine-state table's `exchange evaluation` row, now:

> `see()` exact, `see_ge()` fast; quiescence declines losing captures and a
> queen promotion that takes nothing onto a square it cannot hold -- a pawn's
> capture, a capturing promotion among them, is let through by
> `capture_cannot_lose()` before `see_ge()` is asked (S131)

proposed at the landing:

> `see()` exact, `see_ge()` fast; ProbCut's capture filter and the main
> search's SEE pruning (S091) call both. **Quiescence's exchange gate is
> deleted behind `QsSeeGate` 0 while S022's first verdict measures it**: at 1,
> the tree before S022, quiescence declines losing captures and a queen
> promotion that takes nothing onto a square it cannot hold -- a pawn's
> capture, a capturing promotion among them, is let through by
> `capture_cannot_lose()` before `see_ge()` is asked (S131)

and on each reading: **H1** -- "`see()` exact, `see_ge()` fast; ProbCut's
capture filter and the main search's SEE pruning (S091) call both, and
quiescence neither since S022 deleted S015's gate (<verdict>)"; **H0 or no
verdict** -- the row as it stands now.

The `search` row, after S131's sentence (the one ending "-- `bench`
3429473."):

> **S015's exchange gate in quiescence deleted, S022 verdict 1 (landed <date>,
> DEC-221)**: out of check quiescence no longer declines a move the exchange
> evaluation writes off, so every capture and queen promotion that passes the
> filter's drop and S112's futility test is searched, and quiescence calls
> neither `capture_cannot_lose()` nor `see_ge()`. At 1 `QsSeeGate` gives the
> tree before it, node for node (DEC-215). Before any game the exchange
> evaluation wrote off 48.8 % of the filter loop's moves past the drop over
> the bench positions at depth 12; per-move futility skipped 7.0 % of those
> before the gate saw them and the gate declined the rest, 45.4 % of the
> loop's moves -- reach, not a forecast (DEC-239). Decided by one `{-5, 0}`
> nElo SPRT against the tree with the gate (`adocs/data/S022_v1_sprt.sh`):
> <verdict> -- `bench` 6049266.

Two clauses already in that row describe the gate as present -- S112's
"is skipped before S015's exchange gate" and S131's "meets S112's promotion
exemption and S015's exchange gate" -- and read as history on an H1, where
the coordinator may want "(until S022)" after each.

The "Open items" bullet, now:

> - The S015 quiescence SEE pruning measured 0 Elo when `see()` cost 12.1 % more
>   than it does now. The rerun has not happened; it is folded into S022.

proposed at the landing:

> - The S015 quiescence SEE pruning measured 0 Elo when `see()` cost 12.1 % more
>   than it does now. Its rerun is S022's first verdict: the gate deleted
>   behind `QsSeeGate` 0 against the tree with it, `{-5, 0}` nElo
>   (`adocs/data/S022_v1_sprt.sh`).

and on every reading the bullet leaves the list, the open item discharged
(section 3, item 1): **H1** into the two rows above, the deletion standing;
**H0** into the `exchange evaluation` row as "... (S131); S022 re-measured
the gate against a tree with per-move futility: <verdict>, and it stays";
**no verdict** the same with the zero and the games played.

### Rebased onto `6eb2674`, 2026-09-29

Four commits landed on `achesso` after this verdict was built on `cbaa699`:
S131's record and completion, DEC-239 and DEC-240 (documents), S242
(`tests/test_helpers.hpp`'s listener that repeats a failure on stderr from
inside a `stdout_capture_t`; `tests/test_engine.cpp`'s first-iteration case
redesigned and renamed) and S243 ("the multicut never ends a node on a mate
from its verification" in `tests/test_search.cpp`, E21's description in
`tools/mutants/S097_singular_extension.py`, the `mate_the_multicut_hides` row
in `DEV_MANUAL.md`). **`src/` is byte-identical between `cbaa699` and
`6eb2674`** (`git diff --stat cbaa699 6eb2674 -- src` is empty), so the
identity proof at 1 and every engine number above stand; the tests were
re-run. Everything here was measured and nothing is quoted from the old tree
but those engine numbers.

**The rebase.** The work committed as a WIP on `s022` on top of `cbaa699`,
rebased onto `6eb2674` with one conflict, both sides kept: `DEV_MANUAL.md`'s
`mate_the_multicut_hides` row, S243's "since S243 the property this row
stands in for has its own case" first and this verdict's re-mine after it.
`tests/test_search.cpp`, `tools/mutants/S097_singular_extension.py` and
`adocs/data/README.md` merged on their own: S243's direct case and this
verdict's re-mined row both live in "pruning does not hide a forced mate"'s
suite, and **E21 is now killed by both** -- S243's direct case and the
re-mined row, the second witness S243 keeps. E21's description, S243's
wording, already names the two and holds for the new row, so it is unchanged;
the row's own GOLDEN paragraph says the survivor reading was taken on
`cbaa699`, before S243. One miss of this verdict's was caught at the rebase and
fixed: `DEV_MANUAL.md`'s `golden_defaults` row still read 69; it reads 70,
`QsSeeGate` the seventieth.

**The ceiling is ruled, DEC-241** (the coordinator, 2026-09-29, in the form
DEC-225 set for S095's): `C_mate7_depth11`'s ceiling moves from 0 to 6 in the
landing commit, `adocs/data/S022_v1_sweep.txt` committed as the fifth
recorded sweep and named in the golden's re-derivation line; on an H0 or no
verdict it returns to 0 by dropping the grid from the `--ceilings` command.
The test's comment, `DEV_MANUAL.md`'s golden list and the pre-registration
cite it.

**Re-run on the rebased tree** (niced, `-j4` builds):

- Both fast suites **41 of 41** -- Release `build` and `-DCHESSO_TUNE=ON`
  `build-tune`, serial `ctest -L fast` -- and `./clang-format.sh --check`
  clean (`.tuning/coord/S022_logs/suite_release_rebased.log`,
  `suite_tune_rebased.log`, `format_rebased.log`). `test_engine`, S242's
  redesigned case among it, green in both. **`test_mate_carry` alone: 49.44 s
  Release and 50.52 s tune** against its 120 s ceiling
  (`.tuning/coord/S022_logs/mate_carry_alone_rebased.log`).
- `tools/plan_prose_check.py --citations` and `--touches` 0 flagged,
  `--params` exit 0, `--gate` agrees.
- **Every anchor of every mutant file this verdict touched occurs exactly
  once** -- `tools/mutants/S022_v1_see_gate.py`, `tools/mutants/search.py`,
  `tools/mutants/S131_quiet_queen_promotions.py`, and the whole directory with
  them, 159 mutants, validated by `tools/mutation_check.py`'s own `validate`
  on the rebased tree.
- **Mutation, on fresh fixtures of the rebased tree**: `.ref-builds/mut`,
  `mut2` and `mut3` at `d4e80bf`, the squashed landing commit on `s022`, clean
  at their own HEAD -- the commit that carries this paragraph differs from it
  in this paragraph and in no file under `src/`, `tests/` or `tools/`. Every
  run's header `baseline green, 41 tests, bench 6049266 nodes via engine`.

  | run | score | wall | log |
  |---|---|---|---|
  | `tools/mutants/S022_v1_see_gate.py`, release | **4 of 4 (100 %), equivalent 1, killed 4** | 985 s | `.tuning/coord/S022_mutation_rebased.log` |
  | `--only E21 B04 M18 V07 V09 V10`, release | **4 of 4 (100 %), equivalent 2, killed 4** | 1113 s | `.tuning/coord/S022_mutation_rebased_extras.log` |
  | `--only U03 M18 V07`, tune build (`--build-dir build-tune`) | **3 of 3 killed** | 704 s | `.tuning/coord/S022_mutation_rebased_tune.log` |

  The kills are the final pass's table's and the mutant file's header's, with
  two additions from S243: **E21 is killed by "pruning does not hide a forced
  mate" at the re-mined multicut row and by "the multicut never ends a node on
  a mate from its verification"**, and B04 by S113's ProbCut row and "probcut
  does not run with beta in the mate band". V09 and V10 are killed by their
  futility halves (S112's "a capturing promotion below the threshold is
  searched", and for V09 S131's "futility does not skip a queen promotion that
  takes nothing") and by "pruning does not hide a forced mate" as well. M18,
  V07 and U03 are equivalent in the release build as declared and killed in
  the tune build: U03 by "the gate declines a losing capture at the switch's
  on value" and S131's case at 1, V07 by S131's case at 1, M18 by the on-value
  case's losing evasion. The tune run closes `MUTATION-RUN-FAILED: a verdict
  differs from what the mutant list expects` only because the list declares
  the three equivalent for the release build.

### Not run here, by the brief

The SPRT; DEC-141's Debug self-play and `tools/gate_extra.sh` (the
coordinator's, at the landing); any timing.

### Verdict 1 landed and pinned (the coordinator, 2026-09-29)

Cold fast check over the rebased diff: nothing real; its six document items land with this pin. **Second tier on the landing** (DEC-141): Debug self-play 8 games at
4+0.04, 0 `Assertion`, 0 `disconnect` (`.tuning/coord/S022_v1_debug_selfplay/`);
`tools/gate_extra.sh` 5 stages green in 1524 s (`.tuning/gate_extra_2026-09-29_S022_v1.log`).
Landed as `a97bc1a` on `d446783`, `bench` 3429473 -> 6049266. SPRT pair
pinned: `REF` `d446783` (the tree S131's H1 left, with S242 and S243's tests;
its engine `13caf43`'s node for node), `CAND` `a97bc1a`; open findings re-read
at pinning: items 1 to 11 as the pre-registration states them.

### Verdict 1's outcome (2026-09-29, the coordinator)

**H0, 2026-09-29 05:32: `a97bc1a` against `d446783`, `Elo -65.22 +/- 17.66`,
`nElo -85.89 +/- 22.70`, LLR -2.96, 900 games in 0 h 25 m, 0 forfeits**
(`adocs/data/S022_v1_sprt.log`, `adocs/data/S022_v1_sprt_pairs.txt`; marker
`DONE`). **H0**: a regression; the gate stays (the pre-registration's second row). `QsSeeGate` goes to 1, the parent's tree proved on the landing, and the switch's code leaves with it, the four goldens of item 11 restored byte for byte; S015's zero stands with a current number beside it, this run's interval [-108.59, -63.19] nElo against a tree with per-move futility in it. `Incomplete mating PV` 0 against 0,
an observation for the pre-registration's open finding 3's class (CHESS).
No follow-up run and no second pair (DEC-063, DEC-019). Verdict 2 measures
on the tree this leaves.
