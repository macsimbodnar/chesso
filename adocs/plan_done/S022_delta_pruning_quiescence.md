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
done:       2026-09-30 -- two verdicts, one per change and each against the commit before it (`accepts`): the first an H0, the second a no-verdict zero, and the tree ends where it began at `58585f8`, bench for bench. **Verdict 1, the S015 exchange gate re-measured** (`QsSeeGate` 0 deleted it, 1 the tree before, DEC-215): landed as `a97bc1a` on `d446783`, `{-5, 0}` nElo, **H0 at 900 games on 2026-09-29, `Elo -65.22 +/- 17.66`, `nElo -85.89 +/- 22.70`, LLR -2.96** -- a regression, the gate stays and its code left with the revert `39841ed`; the open item on S015's rerun is discharged. **Verdict 2, the node-level delta early-out** (behind `QsDeltaEarlyOut` 1 with `QsDeltaPhaseMin` 0, out of check and before generation a node whose ceiling cannot reach alpha ends ungenerated; 0 the parent's tree node for node): landed as `0c0db1b` on `58585f8`, `{0, 5}` nElo, **no verdict at fastchess's 40000-game cap on 2026-09-30, `Elo 2.34 +/- 2.63`, `nElo 3.03 +/- 3.40`, LLR 0.87, 0 forfeits, 19 h 3 m** (`858a0ac`, the ledger), read as a zero by its pre-registration's third row (DEC-063) and removed as `6893c0f` on the H0 row's terms, the S238 pattern: the block, the two rows, the eight cases and the eight mutants out, the manuals with them, the three goldens of item 13 back byte for byte (S131's multicut row, the stop half's eight-queens board with its numbers, the timer half's figure), `bench` 3656950 -> 3429473, bench-identical to `58585f8`'s with S244's screen the one engine difference (DEC-242); the removal was prepared beside the run by a fresh agent, cold fast-checked (three text findings fixed, none in code) and S244's ordinary-play census re-taken on it. **DEC-141's second tier on the removal tree**: Debug self-play of 8 games at 4+0.04 on the removal tree `6893c0f`, 0 `Assertion`, 0 `disconnect` (`.tuning/coord/S022rm_S244_debug_selfplay/`); `tools/gate_extra.sh` 5 stages green in 1051 s (`.tuning/gate_extra_2026-09-30_S022rm_S244.log`). **The goal's answer**: deleting delta pruning is the recorded outcome, as a zero; S112's per-move futility is the form that stays, and the exchange gate stays on its H0. No follow-up run and no second pair (DEC-063, DEC-019); S127's fit is where another margin or a gives-check proxy would be tried. Owed and named: the S170 budgets re-derived on the next verdict's tree (DEC-156 as amended by DEC-162, S114's landing carries the sweep), S247 (the stop half's 13.8 ms figure, live again after the restore) and S248 (the parent's stale capture-mate rows) as fillers behind S114 (DEC-171). Implemented from the step file's description (DEC-221); no other project's code was opened. `DEV_MANUAL.md` and `MANUAL.md` checked: the removal's rows stand, no change; `README.md` human-owned, no change.

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

## Verdict 1's revert, 2026-09-29

Written by a fresh Opus agent on the linked worktree `s022rv` at `775dafd`,
briefed from `.tuning/coord/S022_v1_revert_brief.md`, beside the agent
building verdict 2 in `../chesso-s022v2`: every CPU-bound command ran under
`nice -n 19` with `-j4` builds, and every number below is a node count, a best
move or a pass, none a timing. The pre-registration's H0 row, written before
the first game, is executed: **`QsSeeGate` to 1, the parent's tree proved on
the landing, and the switch's code leaves with it** (the S238 pattern). No
reason to keep the switch was found and none is stated. No second SPRT is
owed: the revert is behaviour-neutral against `d446783`, the commit the
landing sat on, and INV-6 discharges it. Nothing was committed or staged.

**How.** `git restore --source=d446783 --worktree` over the nine files
`a97bc1a` touched under `src/`, `tests/` and `tools/` and in `MANUAL.md` --
the pin commit `05ab85d`'s two comment edits in `tests/test_search.cpp` among
them -- which deletes `tools/mutants/S022_v1_see_gate.py`; then one
comment at the gate. `a97bc1a..775dafd` touched nothing else under those paths.

### What left

| what | where |
|---|---|
| the gate's `QS_SEE_GATE != 0 &&` prefix and the paragraph above it; the filter comment's "at QS_SEE_GATE 1" and "At 0 there is no gate to meet"; the store comment back to "since the losing captures were declined" | `quiescence` in `src/search.cpp` |
| the `QS_SEE_GATE` row (`QsSeeGate`, 0, 0 to 1) and its comment; S131's row's "at QsSeeGate 1" | `src/search_params.hpp` |
| the suite "search: quiescence exchange gate" whole -- its three release cases and the tune-only "the gate declines a losing capture at the switch's on value"; in "search: quiescence promotions" the restatement at 0, "a queen promotion onto a defended square is searched at the gate's off value", the premise helper it shared and the tune-only drive of S131's case at 1, S131's case back to its one release form; S112's promotion case's comment back to one reading | `tests/test_search.cpp` |
| the `QsSeeGate` golden row, 70 back to 69, and the header sentence naming it | `tests/test_search_params.cpp` |
| the fifth grid of the `--ceilings` command and the DEC-241 paragraphs | `tests/test_mate_carry.cpp` |
| U01 to U05 | `tools/mutants/S022_v1_see_gate.py`, deleted |
| M18's re-pointed anchor and its release-build equivalence | `tools/mutants/search.py` |
| V07, V09 and V10's re-pointed anchors, V07's equivalence and the note above it | `tools/mutants/S131_quiet_queen_promotions.py` |
| the `QsSeeGate` option row; "at `QsSeeGate` 1" in the `QsFutility` and `QsQueenPromotions` rows | `MANUAL.md` |

**The four goldens of the pre-registration's item 11 are `d446783`'s byte for
byte**, the test files restored whole -- a revert to the rows each GOLDEN block
had quoted for an H0, not a re-derivation:

1. "pruning does not hide a forced mate"'s capture-mate rows: S113's pass,
   depths 7, 9, 11, 10 with `C02, C05, R02, since S112`, `no S091 mutant,
   since S095`, `R02`, `no S091 mutant, since S113`.
2. "ordering keeps the tree small": 65024 and 3251. On the reverted Release
   build `adocs/data/S192_node_budget.py` reads a count of **20427**, inside
   the middle half of that pair, [18694, 49581]
   (`.tuning/coord/S022_v1rv_node_band.log`) -- the parent's count as the
   landing recorded it.
3. S097's multicut row, `mate_the_multicut_hides`: S131's guard-mode row
   (DEC-238), `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42` at depth
   14, mate in 6. **S243's direct case, "the multicut never ends a node on a
   mate from its verification", keeps E21 killed either way**: beside the
   re-mined row on the candidate, beside S131's row here (Mutation, below).
4. `test_mate_carry`'s `short_line_ceiling`: `C_mate7_depth11` back to 0 by
   `adocs/data/S022_v1_sweep.txt` leaving the `--ceilings` command, DEC-241's
   own clause, so the command is the four grids again, and run over them the
   script prints 5, 15, 0, 2, 11, 5, the restored function's six
   (`.tuning/coord/S022_v1rv_ceilings.log`).

The restored test file also undoes one edit of the pin commit's that did not
belong to this verdict. `05ab85d` lengthened a quoted mutation command to
`--only M18 V07 V09 V10 E21 B04` in two GOLDEN paragraphs of the multicut row:
this verdict's own, which left, and S188's older paragraph, whose runs were
`--only E21 J03` and `--only E21` on 2026-09-22 (`adocs/data/S188_revert_e21.log`),
before V07 to V10 existed. S188's paragraph reads its own `--only E21` again.

### What stayed

- The evidence: `adocs/data/S022_v1_sprt.sh`, `S022_v1_sprt.log`,
  `S022_v1_sprt_pairs.txt`, `S022_v1_sweep.txt` -- a recorded grid, out of the
  ceilings' command as `S236_v2_sweep.txt` is -- and `S022_remine_s097.log`,
  with their `adocs/data/README.md` rows, untouched; the census and the
  landing's logs under `.tuning/coord/`.
- `DEV_MANUAL.md`: the bench ledger keeps the landing's entry and **gains the
  revert's, `6049266` -> `3429473`**, beneath it. The five golden-table rows the
  landing moved are `d446783`'s again -- the node band 65024 and 3251, the six
  ceilings 5, 15, 0, 2, 11, 5 with the four-grid command, the capture-mate
  rows, the multicut FEN, the default count 69 -- each with one clause saying
  S022's re-derivation came and went, the shape S238's rows took
  (`capture_mates`: "a revert and not a re-mine"; `golden_defaults`: "came to
  70 and left on its H0, back to 69"; the multicut row says S243's case kills
  E21 on either tree).
- This step file, extended and not rewritten. S015's own `see()` and
  `see_ge()` cases never moved.

### The proofs

**The remaining difference to `d446783` in `src`, `tests`, `tools` and
`MANUAL.md` is one comment.** `git diff --stat d446783 -- src tests tools
MANUAL.md`: `src/search.cpp | 5 +++++`, a paragraph above the gate's condition
in `quiescence` saying S022 tried deleting the gate and what its SPRT read. No
code. S242's and S243's tests are in `d446783` and in this tree alike, so they
are no difference.

| | |
|---|---|
| `bench` | **3429473**, and the whole `bench` stream -- all 112 `info` lines' depth, score, nodes and PV and all eight `bestmove` replies, c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 -- identical to a Release binary built fresh from `d446783` in the throwaway worktree `.ref-builds/parent_d446783` (detached, clean, `git diff --stat d446783 HEAD -- src tests` empty), compared with `time` and `nps` stripped (`.tuning/coord/S022_v1rv_identity.log`, both streams beside it in `.tuning/coord/S022_v1rv_identity/`) |
| `tools/search_bench.py` | identical node for node and move for move on both binaries: depth 9 48304 / 71580 / 25413, depth 12 104784 / 244824 / 117798, best c3d5 e2a6 d7c8q at both -- the numbers the landing's on-value proof recorded at `QsSeeGate` 1 |
| both fast suites | Release `build` and `-DCHESSO_TUNE=ON` `build-tune`, serial `ctest -L fast`: **41 of 41 each**; `test_mate_carry` alone **58.69 s** Release and **59.00 s** tune against its 120 s ceiling (`.tuning/coord/S022_v1rv_suites.log`); run again on the final tree, documents and all, 41 of 41 each, the format check clean and `test_mate_carry` alone 59.41 s and 59.44 s (`.tuning/coord/S022_v1rv_suites_final.log`) |
| format | `./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22` |
| prose | `tools/plan_prose_check.py` one mode per invocation: `--citations` 0 flagged over 39 files, `--touches` 0 flagged, `--params` exit 0, and `--gate` agreeing -- re-run with this section in the file |
| the UCI surface | `MANUAL.md` has no `QsSeeGate` row and is `d446783`'s file byte for byte; `test_uci_surface` green in both builds with **no refresh** |
| the golden | `tests/test_search_params.cpp` is `d446783`'s byte for byte, 69 defaults; DEC-142's re-derivation for `golden_defaults` is its diff against `src/search_params.hpp` |

### Mutation

On a clean detached fixture of this tree, `.ref-builds/mut` at `6ea4176` -- a
throwaway commit of the reverted working tree on no branch, cut through a
temporary index so the worktree's own index was never touched, its `src/`,
`tests/`, `tools/` and `MANUAL.md` equal to this tree's (`git diff --stat
6ea4176 -- src tests tools MANUAL.md` empty; documents apart) -- with doctest
cloned into it from the worktree's own copy. Run from the fixture, so the list
read is the fixture's own, as one run over the whole directory where the brief
named two (the four re-pointed mutants, and E21 over S097's list):
`tools/mutation_check.py tools/mutants .ref-builds/mut --only M18 V07 V09 V10
E21 --jobs 4`, which validates every anchor of the directory's 154 mutants
first -- U01 to U05 gone with their file, M18 and V07 declared `killed` again.
Header `worktree .../.ref-builds/mut at 6ea4176 clean`, `list
.../.ref-builds/mut/tools/mutants clean`, `mutants 5 of 154`, `baseline
green, 41 tests, bench 3429473 nodes via engine`; **mutation score 5 of 5
(100%), killed 5**, wall 885 s, `MUTATION-RUN-DONE`, the fixture clean after
(`.tuning/coord/S022_v1rv_mutation.log`, per-mutant logs in
`.tuning/coord/S022_v1rv_mutation/`).

| mutant | bench | killed by |
|---|---|---|
| E21 | same | "pruning does not hide a forced mate" at the multicut row, S131's row restored, and S243's "the multicut never ends a node on a mate from its verification" -- the two witnesses E21's description names |
| V07 | moved | S131's "the exchange gate declines a queen promotion onto a defended square", in the release build again |
| V09 | moved | "a capturing promotion below the threshold is searched", S131's case above, and "futility does not skip a queen promotion that takes nothing" |
| V10 | moved | "a capturing promotion below the threshold is searched" and S131's case above |
| M18 | moved | `test_mate_pv`'s "the mined set reports no mate it cannot show" and `test_mate_carry`'s "a mate score carried across searches keeps a line that reaches it" |

### Proposed commit text, for the coordinator

No DEC-220 result block: `775dafd` closed the verdict and recorded it. The
coordinator appends its own trailer after the `Bench:` line.

```
Remove S022 v1's QsSeeGate switch on its H0: the exchange gate stays

The deletion of S015's exchange gate in quiescence read a regression.
The non-regression SPRT of a97bc1a, the gate behind QsSeeGate 0,
against d446783, the tree with it, {-5, 0} nElo at 8+0.08, accepted H0
at 900 games: Elo -65.22 +/- 17.66, nElo -85.89 +/- 22.70, LLR -2.96,
0 forfeits. adocs/data/S022_v1_sprt.sh wrote that reading and its
consequence before the first game -- QsSeeGate to 1, the parent's tree
proved on the landing, and the switch's code leaves with it, the S238
pattern -- so this is the pre-registration executed. S015's zero
stands with a current number beside it: the interval [-108.59, -63.19]
nElo, on a tree with per-move futility in it.

Out: the QS_SEE_GATE row and the switch's prefix on the gate in
quiescence, with the comments the verdict changed to read at both
values (the filter and store comments, S131's row, MANUAL's QsFutility
and QsQueenPromotions rows) and the QsSeeGate option row; the suite
"search: quiescence exchange gate", the defended-square case's
restatement at 0 and S131's case's tune-only drive at 1, S131's case
back to its one release form and S112's promotion case's comment to
one reading; the golden_defaults row, 70 back to 69;
tools/mutants/S022_v1_see_gate.py; M18, V07, V09 and V10 back to the
anchors they had, M18 and V07 expected killed again. The four goldens
of the pre-registration's item 11 go back to d446783's byte for byte:
the capture-mate rows, the node band 65024 / 3251, S097's multicut row
to S131's guard-mode row (DEC-238) -- S243's direct case kills E21
either way -- and C_mate7_depth11's ceiling to 0, S022_v1_sweep.txt
dropped from the --ceilings command (DEC-241). The restored test file
also takes the pin commit's lengthened mutation command back out of
S188's older GOLDEN paragraph, where it did not belong.

In: a five-line comment at the gate saying S022 tried deleting it and
what its SPRT read; DEV_MANUAL's ledger gains the revert's entry and
its five golden rows read true again, each saying the re-derivation
came and went. The evidence stays under adocs/data/.

src, tests, tools and MANUAL.md are d446783's apart from that comment.
bench is 3429473 with the whole bench stream and all eight bestmove
replies identical to a fresh build of d446783, and tools/search_bench.py
is identical at depths 9 and 12 (INV-6, DEC-215): no second SPRT is
owed. Both fast suites 41 of 41, format and the three prose checks
clean; M18, V07, V09, V10 and E21 killed, 5 of 5, on a fixture of this
tree.

S022, DEC-063, DEC-019: the record's overlap claim was a direction and
did not transfer here. Verdict 2 measures on the tree this leaves.

Bench: 3429473
```

### Proposed `adocs/specs.md` edits, for the coordinator

Quoted against `specs.md` at `775dafd`. Three edits.

**1. The engine-state table's `exchange evaluation` row**, now:

> `see()` exact, `see_ge()` fast; ProbCut's capture filter and the main
> search's SEE pruning (S091) call both. **Quiescence's exchange gate is
> deleted behind `QsSeeGate` 0 while S022's first verdict measures it**: at 1,
> the tree before S022, quiescence declines losing captures and a queen
> promotion that takes nothing onto a square it cannot hold -- a pawn's
> capture, a capturing promotion among them, is let through by
> `capture_cannot_lose()` before `see_ge()` is asked (S131)

becomes S131's text with the H0 clause this file's landing section wrote for
the open item:

> `see()` exact, `see_ge()` fast; quiescence declines losing captures and a
> queen promotion that takes nothing onto a square it cannot hold -- a pawn's
> capture, a capturing promotion among them, is let through by
> `capture_cannot_lose()` before `see_ge()` is asked (S131); S022 re-measured
> the gate against a tree with per-move futility: H0 at 900 games on
> 2026-09-29, `Elo -65.22 +/- 17.66`, `nElo -85.89 +/- 22.70`, a regression
> (`adocs/data/S022_v1_sprt.log`), and it stays

**2. The `search` row's verdict-1 sentence**, from "**S015's exchange gate in
quiescence deleted, S022 verdict 1 (landed 2026-09-29, DEC-221)**:" through
"<verdict> -- `bench` 6049266." inclusive, becomes, in the form S238's and
S237's sentences took:

> **Deleting S015's exchange gate in quiescence was tried and left, S022
> verdict 1, landed and reverted 2026-09-29 (DEC-221)**: out of check
> quiescence stopped declining a move the exchange evaluation writes off, so
> every capture and queen promotion that passed the filter's drop and S112's
> futility test was searched and quiescence called neither
> `capture_cannot_lose()` nor `see_ge()`, behind `QsSeeGate` 0, whose 1 gave
> the tree before it node for node (DEC-215). Before any game the exchange
> evaluation wrote off 48.8 % of the filter loop's moves past the drop over the
> bench positions at depth 12; per-move futility skipped 7.0 % of those before
> the gate saw them and the gate declined the rest, 45.4 % of the loop's moves
> -- reach, not a forecast (DEC-239). Its one `{-5, 0}` nElo SPRT against the
> tree with the gate (`adocs/data/S022_v1_sprt.sh`) **accepted H0 at 900 games
> on 2026-09-29 -- `Elo -65.22 +/- 17.66`, `nElo -85.89 +/- 22.70`, LLR -2.96,
> 0 forfeits** (`adocs/data/S022_v1_sprt.log`), a regression: `QsSeeGate` went
> to 1 and the switch's code left with it, the four goldens re-derived for the
> candidate restored with it -- `bench` 3429473, `d446783`'s own total node
> for node.

The two clauses of that row that describe the gate as present -- S112's "is
skipped before S015's exchange gate" and S131's "meets S112's promotion
exemption and S015's exchange gate" -- are true again as they stand.

**3. The "Open items" bullet on S015's rerun leaves the list**, discharged
into edit 1 (section 3, item 1: "the open item is discharged either way"):

> - The S015 quiescence SEE pruning measured 0 Elo when `see()` cost 12.1 % more
>   than it does now. Its rerun is S022's first verdict: the gate deleted
>   behind `QsSeeGate` 0 against the tree with it, `{-5, 0}` nElo
>   (`adocs/data/S022_v1_sprt.sh`).

### Owed to the coordinator, not run here

DEC-141's second tier -- Debug self-play and `tools/gate_extra.sh` -- which the
landing's pin ran on `a97bc1a`: the revert touches the search, though only a
comment differs from `d446783`, whose engine this is node for node. Whether it
is owed on a tree identical to a parent's engine is the coordinator's call.
No match, SPSA or timing was run here.

## Verdict 2: the node-level delta early-out, behind a switch (2026-09-29)

Section 3's second item, built after verdict 1 as it recommends, on
`05ab85d` (verdict 1's landing `a97bc1a` and its pin, the exchange gate
deleted behind `QsSeeGate` 0) while verdict 1's SPRT ran. **Verdict 1 read H0
before this was measured** -- `Elo -65.22 +/- 17.66`, `nElo -85.89 +/- 22.70`,
LLR -2.96, 900 games (`.tuning/coord/S022_v1_sprt.log`) -- so S015's gate
stays and its switch's code leaves in the revert. In section 2's naming this
verdict is therefore **F+S+D against F+S**, not F+D against F: the gate is on
both sides of the pair, and REF is the tree the revert leaves. The candidate
asks at `{0, 5}` nElo whether the early-out gains on that tree (section 6).
**Rebased onto `39841ed`, the revert, and then onto `58585f8`, S246 on it,
on 2026-09-29**: `src/`, `tools/` and `MANUAL.md` there are `d446783`'s but
for a five-line comment at the gate, and `tests/` are too but for S246's two
files; the code was reapplied by anchor and every number below re-taken on
`58585f8` unless it names another tree.

**It stopped once before completing, and the stop and its resolution are
first here.** Both fast suites were 40 of 41 on every tree measured before the
ruling: `test_engine`'s first-iteration case was red at its precondition in
both builds. Depth 1 on its eight-queens board,
`q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1`, costs 10187 nodes
and 1 ms with the early-out against 36165 nodes and 6 to 7 ms without it --
the early-out ends 199 of its quiescence nodes at depth 1 -- and the case
asserts 3 ms before it measures anything. Its own comment says what that asks
for: "under that it needs a heavier position, not a smaller floor". A change
of the case's board is not a scripted re-derivation, so the agent made none,
and the step stopped for a ruling.

**The ruling, and a finding on the way to it** (the coordinator, 2026-09-29):
the case's own rule applies -- a heavier position, not a smaller floor -- a
re-derivation by the rule at the site (DEC-233's class), not a relaxation, so
no decision entry; DEC-240 cited for what the case guards. The first ruling
named the case "a stop inside the first iteration cuts it and the hard timer
ends the search within its bound" and its timer half's own board, and **that
case was in no commit**: S242's completing commit `aef0257` had carried its
first redesign -- still "the first iteration honours stop and the hard
timer", both halves on the one eight-queens board, the hard-timer claim
retried up to five times, the form DEC-240 rejected -- while S242's stamp and
commit message described DEC-240's form, which survived only as blob `e88103f`
and in `.tuning/coord/S242_logs/s242_only.patch`. The coordinator's landing had
cut the fillers' patches from their worktree's index, where the first round
was staged. **S246 landed the missing forms (`58585f8`)**, and the ruling was
then applied to the letter on that tree:

- **The stop half's board moved** to
  `r1r1r1r1/1r1r1r1k/8/2n1n3/2N1N3/8/1R1R1R1K/R1R1R1R1 w - - 0 1`, seven
  rooks and two knights a side (python-chess: valid, not in check, 63 legal
  moves, 7 captures; reachable by count), chosen over the rook board the
  first ruling named because it clears the floor about nine times where that
  one clears it twice, and a faster machine is the risk the floor faces. The
  floor, `depth_1_floor_ms = 3`, and `prompt_stop_us`, derived from it, are
  unchanged. The old board is recorded at the site with its numbers and why it
  left.
- **Its golden re-taken on `58585f8`**, `position fen <board>` then `go depth
  1`, the info line's `time`, ten runs each on an idle machine, niced: **146994
  nodes on the parent and on the candidate, 24 to 31 ms**
  (`.tuning/coord/S022_v2_board/site_numbers58_small.log`; the parent is
  `39841ed`'s engine, whose `src/` `58585f8` shares). They agree because the
  early-out never fires on this board at depth 1: the census build's counters
  read 0 there with the exchange gate and without it (`boards.log`).
- **The timer half's board is untouched, and its stated dead-timer figure was
  re-taken** on this tree, since the early-out moves it: depth 1 on
  `rn1qk1nr/qqqqqqqq/8/8/8/8/QQQQQQQQ/RN1QK1NR w - - 0 1` is 18594285 nodes and
  3885 to 3945 ms with the early-out against 25933707 nodes and 5534 to 5602
  ms without it (`site_numbers58_timer.log`); a throwaway build of this tree
  with the hard timer disarmed answers `go movetime 1` after 3915 and 3935 ms,
  depth 1, the same 18594285 nodes, and the engine as shipped after 1.3 to 1.9
  ms (`timer_probe58.log`, S242's own probe). The 402 ms bound now sits about
  ten times below the dead timer, where it sat about fourteen, and a four
  times faster machine still leaves it separating.
- **Separation, observed**: 200 runs of the case, Release, niced, 0 failed,
  the timer half asserted once a run and never retried
  (`.tuning/coord/S022_v2_logs/focused_loop.out`).

Before S246 the boards were measured on the trees then standing, for the
ruling (`boards.log`, `timer_half.log`, `timer_board_depth1.log`), five runs
each:

| board | 05ab85d | its candidate | d446783 | its candidate | early-out fires at depth 1 |
|---|---|---|---|---|---|
| eight queens (the case's) | 36165, 6 ms | 10187, 1 ms | 36165, 6 ms | 10187, 1 ms | 199 times |
| `r1r1r1r1/1r1r1r1k/8/8/8/8/1R1R1R1K/R1R1R1R1 w - - 0 1` | 35163, 6 ms | 35163, 6 ms | 35163, 7 ms | 35163, 7 ms | 0, both trees |
| `r1r1r1r1/1r1r1r1k/8/2n1n3/2N1N3/8/1R1R1R1K/R1R1R1R1 w - - 0 1` | 177816, 33 ms | 177816, 33 ms | 146994, 27 ms | 146994, 27 ms | 0, both trees |
| S242's timer board | 33794579, 7068 ms | 22625582, 4602 ms | 25933707, 5884 ms | 18594285, 4186 ms | -- |

The knight board's cost moves with the exchange gate (177816 without it,
146994 with it) and the rook board's does not; the gate stays, so that is
fine. `go movetime 1` cut depth 1 in 200 of 200 fresh processes on every
engine and each board on the idle machine; under a match the timer half's
premise is a scheduler's, which is why DEC-240's form asserts a bound instead.

### What the tree already did, 2026-09-29 (checked at `05ab85d`)

- **The loop's shape**, by symbol, `src/search.cpp` `quiescence`: the probe
  and S094's static score; S130's stand pat and its substitution; the abort
  and ply-cap returns; `in_check`; out of check the stand pat's fail-high
  store and `alpha` raised to it; the qply cap (`MAX_QSEARCH_DEPTH`),
  returning the stand pat and storing nothing; S112's `qs_futility` and
  `futility_base`, computed from the substituted stand pat; generation --
  `generate_moves` in check, `generate_captures` out of it; then the filter
  loop: S131's drop, S112's block (the promotion exemption, the fold into
  `futility_best`, the make / `is_check` / unmake exemption for a checking
  capture), the exchange gate (`QsSeeGate` 1 on the landing tree, where it is
  unconditional again after the revert), `capture_score()` and the
  compaction; then the search loop with S210's dead-board test, the mate
  declaration and the store.
- **The early-out's site**: after `futility_base` and before `futility_best`
  and the generation call, so below the qply cap and above the generator, out
  of check only. Its compare is `delta_ceiling <= alpha` with `alpha` as the
  stand pat left it; since the ceiling is above the stand pat it can hold
  only where the stand pat did not raise alpha, so that alpha is the node's
  own. Its return stores `normalize_score(delta_ceiling, ply)` at
  `TT_DEPTH_QS` as `TT_ALPHA_NODE`, move 0, `stored_eval`, the shape of the
  node's other stores, and returns the ceiling: the upper bound the fold's
  argument (S112, DEC-102) makes whatever the stand pat's own type.
- **`game_phase()`** (`src/evaluation.cpp` `game_phase`) is the INV-4 phase
  accumulator clamped to `GAME_PHASE_MAX`, 24, the null-move guard's
  predicate in `negamax_at` (`game_phase(&game->board) > 0`). One call per
  node that reaches the site.
- **"A friendly pawn on the seventh"** is the side to move's pawn bitboard
  against `rank_masks` at `a7` for White and `a2` for Black -- square 0 is a8
  (`bb_squares_t`), so the first is squares 8 to 15 and the second 48 to 55.
  A pawn that cannot move still counts, so the allowance only ever keeps the
  early-out from firing.
- **The in-check path is untouched**: the block is guarded by `!in_check`,
  and in check the node generates every evasion and declares mate on
  `legal_moves == 0` as before.
- **ProbCut's preliminary (S113)** enters quiescence at the capture's child
  with the zero window `(-probcut_beta, -probcut_beta + 1)`, and the early-out
  meets it like any caller: where the child's own ceiling is at or below
  `-probcut_beta`, the child ends ungenerated and the preliminary holds;
  deeper in the preliminary's quiescence values move either way, as anywhere.
  The preliminary only decides whether the shallow search is paid, and the
  shallow search, which alone decides a cut, meets the early-out at its own
  leaves. Nothing structural changes, and S244's class (a dead child scored at
  `TT_DEPTH_QS`) is still no wrong cut.
- **The probe pattern does not reach quiescence**: `search_node_probe_t` is
  loaded only by `negamax_at<true>`, and a runtime read in quiescence would
  put a load on every node (S191 measured the negamax one at 1.49 % of nodes
  per second). The cases read the node count and the table instead, as
  S131's suite and verdict 1's do.

### What lands (one commit on `s022v2`, on `58585f8`)

- `src/search.cpp` `quiescence`: one block, above. Nothing else in `src/`:
  S112's block, S131's condition, the gate's line, S113's filter, S091's
  pruning and the generator are untouched.
- `src/search_params.hpp`: `QS_DELTA_EARLY_OUT` ("QsDeltaEarlyOut", **1**,
  0..1), a verdict switch in `QsFutility`'s, `ProbCut`'s and
  `QsQueenPromotions`' shape -- its 0 the parent -- and `QS_DELTA_PHASE_MIN`
  ("QsDeltaPhaseMin", **0**, 0..24) with its seed's form stated at its site.
  No new margin.
- `tests/test_search.cpp`, a new suite "search: quiescence delta early-out":
  "the early-out ends a node no move can lift to alpha", "the early-out's
  value is its ceiling and not the stand pat", "the early-out does not fire
  where a queen's gain reaches alpha", "a pawn on its seventh raises the
  ceiling by the promotion" (both sides to move), "the early-out is not asked
  in a pawn ending", "the early-out is never asked in check", and in the tune
  build "the early-out is off at the switch's off value" and "the endgame
  disable follows its threshold". Every window is the engine's own numbers:
  `evaluate_cheap()`, `LAZY_EVAL_MARGIN`, `QsFutilityMargin` and
  `search_qs_futility_value_probe()`, alpha placed on the ceiling itself or
  one below it. `tests/test_search_params.cpp`: two golden rows, 69 -> 71
  (70 -> 72 on `05ab85d`).
- `tests/test_engine.cpp`, the ruling: the first-iteration case's stop half
  on the rooks-and-knights board, its golden and the old board recorded at
  the site; the timer half's dead-timer figure re-taken. Nothing else in the
  file moves.
- `tests/test_search.cpp`'s multicut row of "pruning does not hide a forced
  mate", re-mined (below): S097's row in place of S131's, depth 14, mate in 5,
  S131's row quoted in the GOLDEN block for an H0; the evidence
  `adocs/data/S022_v2_remine_s097.log` and its README row; `DEV_MANUAL.md`'s
  row for the golden.
- `tools/mutants/S022_v2_delta_early_out.py`, Z01 to Z08.
- `MANUAL.md` two option rows; `DEV_MANUAL.md` the bench ledger entry and the
  golden-defaults count; `adocs/data/S022_v2_sprt.sh` and its
  `adocs/data/README.md` row.
- **The rebase.** Built on `05ab85d`, where the rows sat after `QsSeeGate`
  and the defaults were 70 -> 72; before the revert landed, a scratch of the
  landing tree (`.ref-builds/landing`, `d446783` with the change applied by
  anchor, the rows after `QsQueenPromotions`, 69 -> 71) carried the landing
  numbers. The worktree was then fast-forwarded to `39841ed` and the code
  reapplied by the same anchors (`.tuning/coord/S022_v2_save/rebase_code.py`),
  which reproduces that scratch byte for byte but for the revert's comment at
  the gate; the documents were rebased by hand. Then to `58585f8`, where only
  S246's `tests/test_search.cpp` and `DEV_MANUAL.md` met this change and both
  took it by anchor (the suite and the two DEV_MANUAL edits).

### Seeds (DEC-134)

One. `QS_DELTA_PHASE_MIN` 0 is **(b), chesso's own phase scale**: pawn
endgames only, the boundary the null-move zugzwang guard already keys on; the
wiki names the late-endgame disable and no threshold. Range 0 to 24 by the
scale's own ends: at 24 the early-out is never asked, which is proved below to
be the parent's tree, and at the floor pawn endgames are still disabled, so
section 4's "never disable" is outside the range -- a floor of -1 would admit
it, and that is S127's to want. **No new margin** (section 4): the ceiling is
`futility_base` plus `qs_futility_value` at the queen, 900, and the allowance
that table's queen minus its pawn, 800, both S112's seeds. Implemented from
this file's description (section 2's early-out bullets, section 4; DEC-221);
no other project's code was opened.

### The reach, before any game (stated as reach, DEC-239)

An instrumented copy of the candidate (`.ref-builds/census`, the candidate's
`src/` plus write-only counters from `.tuning/coord/S022_v2_census/apply_census.py`),
its tune build at `QsSeeGate` 1 -- the landing tree, `d446783`'s engine node
for node -- and at 0, each at `QsDeltaEarlyOut` 1 and 0: at all eight
configurations its `bench 12` and `search_bench` 12 totals, replies and best
moves are the uninstrumented tune build's, and the real block's return count
equals the census's own recomputed firing count
(`.tuning/coord/S022_v2_census/census_run.log`, summary
`S022_v2_census_summary.txt`). The site is an out-of-check node past the
stand pat's cutoff and the depth cap, the node that generates on the tree
without the early-out:

| landing tree (`QsSeeGate` 1) | site | ended | no move to generate | every move S112 skips | a move S112 searches | a move the tree searches |
|---|---|---|---|---|---|---|
| `bench 12`, eight positions | 246317 | 31200 (12.67 %) | 22569 | 6340 | 2291 (2273 a checking capture, 18 a promotion) | 804 |
| `search_bench` 12, three positions | 103227 | 4166 (4.04 %) | 753 | 2337 | 1076 | 477 |

At the ended bench nodes a generation holds 23877 moves past the filter's
drop: S112 skips 21255, and of the 2622 it would search (2498 checking
captures, 124 promotions) the gate declines 1631, so **991 moves the tree
without the early-out searches are not searched**. 8247 of the ended nodes
hold at least one capture S112 skips anyway. The allowance is in the ceiling
at 18 ended nodes and 8499 asked ones. **The phase disable reaches nothing
on these positions**: 210 nodes are at phase 0 and the ceiling fires at none
of them, so its seed is untested by the census. On the tree without the
early-out the same condition holds at 31778 of 248994 site nodes (12.76 %),
967 with a move the tree searches. On `05ab85d`, the gate deleted, the
early-out ends 85426 of 743881 (11.48 %) over the bench positions, 14592 with
a move the tree searches. This is how often the class occurs, not what it is
worth: S131's census read 0.85 % and its match H1 at +17.84.

### Measurements (2026-09-29, niced, counts only)

- **The off value** (DEC-215), on the worktree's tree, a parent built from
  `05ab85d` in `.ref-builds/parent`: the tune build at `QsDeltaEarlyOut` 0
  prints `bench` **6049266** with all eight replies the parent's (c3d5 e2a6
  d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), `bench 12` 2393854, and `search_bench`
  reproduces the parent at 9 (39854 / 249885 / 33736) and 12 (97921 / 567770 /
  125308), best c3d5 e2a6 d7c8q (`.tuning/coord/S022_v2_logs/identity.log`).
  **On the rebased tree**, against a build of `39841ed`
  (`.ref-builds/parent_39841ed`): **3429473** with its eight replies (c3d5
  e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), `bench 12` 1694808, `search_bench`
  48304 / 71580 / 25413 and 104784 / 244824 / 117798, best c3d5 e2a6 d7c8q
  (`identity_rebased.log`; the landing scratch against `d446783` read the
  same, `identity_landing.log`). `QsDeltaPhaseMin` 24 with the switch at 1
  prints the parent's totals on both trees.
- **`bench` parent -> candidate**, on the rebased tree: 3429473 -> **3656950** (+6.63 %),
  seven replies unchanged and kiwipete's e2a6 -> d5e6; `bench 12` 1694808 ->
  1702684, the same one reply. On `05ab85d`: 6049266 -> 7722782 (+27.7 %),
  replies two (e2a6 -> d5e6) and five (d8e7 -> a7a6) moving; `bench 12`
  2393854 -> 2826829, reply five moving. The tune build at the defaults prints
  the Release totals on both.
- **`search_bench`**, on the rebased tree: 9: 48304 -> 70913, 71580 -> 71220, 25413 ->
  24293, best moves unchanged; 12: 104784 -> 128099, 244824 -> 258594, 117798
  -> 83277, kiwipete's best move e2a6 -> d5e6. On `05ab85d`: 9: 39854 ->
  39853, 249885 -> 245330, 33736 -> 31515; 12: 97921 -> 97058, 567770 ->
  646712, 125308 -> 183598; best moves unchanged. The change alters play, so
  INV-6 does not discharge it.
- **Tests, red first and observed**, three stages on the worktree, both
  builds (`.tuning/coord/S022_v2_logs/red_stageA.log`, `red_stageB.log`,
  `green_stageC.log`). **A**, the rows and the suite in and the early-out
  not written: red in both builds "the early-out ends a node no move can
  lift to alpha" (3 nodes against 1, the checking capture's child entered,
  its move stored), "the early-out's value is its ceiling and not the stand
  pat" (at its node count) and the allowance case's second half (2 nodes, the
  promotion searched, 1823 against the ceiling 3089, both sides to move); in
  the tune build also "the early-out is off at the switch's off value" at its
  control and "the endgame disable follows its threshold" at 1; green, as
  they must be on a tree without the rule, "does not fire where a queen's
  gain reaches alpha", "not asked in a pawn ending" and "never asked in
  check". **B**, the early-out without the allowance: the allowance case
  alone red in both builds, both halves and both sides to move (the node
  ended at the window between the ceilings, 2289 against 3089 at the
  allowance's ceiling). **C**, the block as it lands: the suite 6 of 6
  Release and 8 of 8 tune. "a side in check may not stand pat" and "mate is
  recognised at depth zero" green.
- **Both fast suites, 41 of 41 each, on `58585f8`**, the ruling applied:
  Release `build` and `-DCHESSO_TUNE=ON` `build-tune`, serial `ctest -L
  fast`, niced, `-j4` builds (`.tuning/coord/S022_v2_logs/final58_suite_release.log`,
  `final58_suite_tune.log`); `./clang-format.sh --check` clean;
  `tools/plan_prose_check.py --citations`, `--touches`, `--params` and
  `--gate` clean. **`test_mate_carry` alone: 57.91 s Release and 57.66 s
  tune** (`final58_mate_carry_build.log`, `_build-tune.log`). **No mined
  row went red**: `test_search` -- "pruning does not hide a forced mate" with
  every mined row and the capture-mate table, the node band of "ordering
  keeps the tree small", count 20357 against the parent's 20427, inside the
  middle half of 65024 / 3251 -- `test_mate_carry` and `test_mate_breadth`
  green in both builds; one row went quiet, the next bullet. Before the ruling
  the suites were 40 of 41 on every
  tree, the one red the stop: on `05ab85d` (`v2a_suite_*.log`, where a
  re-run once also timed out `test_mutation_check`'s sandbox case
  `test_sigterm_reverts_the_mutant_before_exiting` beside another agent's
  builds, green run alone twice), on the landing scratch
  (`landing_suite_*.log`) and on `39841ed` (`rebased_suite_*.log`).
- **S097's multicut row went quiet and was re-mined** (DEC-142, DEC-233, and
  DEC-238's guard mode, a guard row). `tools/mutation_check.py --only E21`
  over the whole mutant directory on the candidate's fixture `4864cca` scored
  E21 killed by S243's direct case alone, where S246's run on `58585f8` had
  listed two, that case and this row -- the sign `DEV_MANUAL.md`'s row names.
  S131's row, `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42`, reads
  its mate at 11 to 14 on both builds of this tree. Stages 2 to 6 of
  `adocs/data/S097_mine_mate_row.py`, stage 1 not re-run (the FEN list is
  identical to verdict 1's), on libraries of the fixture and of the fixture
  with E21 applied (`.ref-builds/pre`, `.ref-builds/e21`): **one of the 269
  candidates separates the sweeps**, and both modes take it, S097's own row,
  `4N3/8/3P1ppk/4p2p/4P2P/1n1P2P1/Q4PK1/3q4 w - - 5 46`, depth 14, mate in 5 --
  shipped `d13 d14`, E21 `d13`, the mutant's multicut changing the tree at 12,
  13 and 14 and the shipped one at 13 and 14 -- a cell of 6580581 nodes.
  Stockfish at depth 20 in a fresh process through python-chess: `#+5` for
  White in 7918 nodes, the label the candidates file held; python-chess:
  valid, not in check, 23 legal moves, 2 captures, no promotion. **Observed
  red under E21, green shipped**: "pruning does not hide a forced mate" fails
  at the row's `REQUIRE( result.mate_found )` on a build with E21 applied and
  passes shipped (`.tuning/coord/S022_v2_remine/red_e21.log`; everything in
  `adocs/data/S022_v2_remine_s097.log`). The row replaces S131's in the case,
  S131's quoted in its GOLDEN block for an H0 or no verdict.
- **Mutation** (DEC-141 clause 2), `tools/mutants/S022_v2_delta_early_out.py`
  on a clean detached fixture of the tree that lands, `.ref-builds/mut` at
  `d3382f9` -- a throwaway commit of the worktree on `58585f8`, on no branch,
  cut after the re-mine, its `src/`, `tests/`, `tools/` and `MANUAL.md` the
  worktree's byte for byte -- header `baseline green, 41 tests, bench 3656950
  nodes via engine`; **mutation score 7 of 7 (100 %), equivalent 1, killed
  7**, wall 1150 s (`.tuning/coord/S022_v2_mutation/final_release.log`):

  | mutant | killed by |
  |---|---|
  | Z01 allowance dropped | "a pawn on its seventh raises the ceiling by the promotion"; "pruning does not hide a forced mate" |
  | Z02 in-check guard dropped | "the early-out is never asked in check"; "pruning does not hide a forced mate" |
  | Z03 bare stand pat returned | "the early-out's value is its ceiling and not the stand pat", the allowance case; "pruning does not hide a forced mate" |
  | Z04 phase disable dropped | "the early-out is not asked in a pawn ending"; "pruning does not hide a forced mate"; `bench` unmoved, the bench positions never reaching the disable |
  | Z05 switch ignored | equivalent in the release build as declared; in the tune build "the early-out is off at the switch's off value" |
  | Z06 compare's sign flipped | "the early-out does not fire where a queen's gain reaches alpha", the allowance case, and 27 more across `test_search`, `test_engine`, `test_mate_breadth` and `test_mate_carry` |
  | Z07 switch inverted | "the early-out ends a node no move can lift to alpha", the value case, the allowance case; "pruning does not hide a forced mate" |
  | Z08 stored with the stand pat's type | "the early-out ends a node no move can lift to alpha", "the table never changes the answer", "pruning does not hide a forced mate", `test_mate_breadth`'s floor |

  "pruning does not hide a forced mate" joined five kill lists with the
  re-mined multicut row, which is S097's row read on this tree and reddens
  under each of those mutants too; on the fixture of the tree before the
  re-mine (`4864cca`, `superseded_4864cca/`) every list was the suite's own
  cases alone. **The tune build**, `--build-dir build-tune --only Z05`: the
  same header; **1 of 1 killed**, by "the early-out is off at the switch's
  off value", wall 289 s (`final_tune.log`); the run closes
  `MUTATION-RUN-FAILED: a verdict differs from what the mutant list expects`
  only because Z05 is declared equivalent for the release build. **The mined
  rows, for the record**, `--only E21 B04 C02 C05 R02` over the whole
  directory, which validates all 162 anchors first: the same header; **5 of 5
  killed**, wall 743 s (`final_extras.log`) -- **E21 by "pruning does not hide
  a forced mate" and by "the multicut never ends a node on a mate from its
  verification"**, two killers again after the re-mine; B04 by the ProbCut
  row and "probcut does not run with beta in the mate band"; C02, C05 and R02
  by "pruning does not hide a forced mate" and their direct guards. The hand
  pre-check on both earlier trees (`.tuning/coord/S022_v2_precheck/`) and a
  pre-run on `4263ebe` (`prerun_4263ebe/`) read the same verdicts.

### The pre-registration (`adocs/data/S022_v2_sprt.sh`)

`{0, 5}` nElo, `fastchess.sh`'s default gainer pair, 8+0.08, Hash 16,
`books/noob_3moves.epd`, concurrency 12; verdict 1's H0 recorded and what it
fixes (F+S+D against F+S); DEC-063's expectation, about zero or negative, with
the record (Weiss #455 gaining by deleting delta once futility existed, Lynx
#731 measuring delta added to a SEE-gated tree negative -- this pair's
configuration); DEC-143's 41861 and 25591 games, 19.8 h and 12.1 h at 2110 an
hour, the midpoint past the 40000-game cap; `REF` and `CAND` `PIN_ME` with the
refusal; `OUT` under `.tuning/`; the abort rule per side; the open findings
carried from verdict 1's block by id, 1 to 12, with 15 new (S246, closed) --
S244 and S245 open fillers -- and this verdict's own as 13 (no mined golden
moved; the two of `tests/test_engine.cpp`'s case re-derived) and 14 (the
stop, resolved by the ruling's re-derivation); the reach stated as reach;
DEC-236's clause answered. **THE REFERENCE IS THE TREE VERDICT 1 LEAVES** --
REF the landing's parent, `58585f8`, whose engine is `39841ed`'s -- and the
promotion
allowance is measured with S131's class in the tree. H1: the early-out stays,
the switch at 1, `QsDeltaPhaseMin` at its seed for S127; H0 with the interval
wholly below zero: the switch to 0 and the code leaves with it, deleting delta
pruning the recorded outcome; no verdict or an interval reaching above zero: a
zero read the same way, no second pair. `bash -n` clean.

### Proposed `specs.md` edits (the coordinator edits specs.md)

Quoted against `39841ed`'s `specs.md`, the tree this lands on.

The `search` row, after verdict 1's sentence (the one ending "-- `bench`
3429473, `d446783`'s own total node for node."); `58585f8` does not touch
`specs.md`:

> **The node-level delta early-out in quiescence, S022 verdict 2 (landed
> <date>, DEC-221)**: out of check, above `QsDeltaPhaseMin` 0 and before
> anything is generated, a node whose ceiling -- S112's futility base, the
> queen's price in S112's victim table, and that table's queen less its pawn
> when a pawn of the side to move stands on its seventh -- is at or below
> alpha ends there, returning the ceiling as an upper bound; it drops the
> checking captures S112 searches, whether a capture gives check being unknown
> before generation, and never runs in check. At 0 `QsDeltaEarlyOut` gives the
> tree before it, node for node (DEC-215). Before any game it ended 12.67 % of
> the out-of-check quiescence nodes that reach generation over the bench
> positions at depth 12, 72.3 % of them with nothing to generate and 804 with
> a move the tree searches -- reach, not a forecast (DEC-239). Decided by one
> `{0, 5}` nElo SPRT against the tree with S015's gate and without the
> early-out (`adocs/data/S022_v2_sprt.sh`): <verdict> -- `bench` 3656950.

The `absent, search` row loses "delta pruning" on an H1 -- "the node-level
internal iterative reduction (S095 shipped the reduction-term form instead),
capture history, correction history" -- and on an H0 or no verdict keeps it,
with "(S022 measured the node-level form and deleted it: <verdict>)" after
it.

### Not run here, by the brief

The SPRT; DEC-141's Debug self-play and `tools/gate_extra.sh` (the
coordinator's, at the landing); any timing claim -- the milliseconds above are
the case's own goldens, taken by the command each names.

### Verdict 2 landed and pinned (the coordinator, 2026-09-29)

Cold fast check over the rebased diff: no real defect in the code; the pre-registration's H0 row and REF prose were corrected in the landing. **Second tier on the landing** (DEC-141): Debug self-play 8 games at
4+0.04, 0 `Assertion`, 0 `disconnect` (`.tuning/coord/S022_v2_debug_selfplay/`);
`tools/gate_extra.sh` 5 stages green in 1119 s (`.tuning/gate_extra_2026-09-29_S022_v2.log`).
Landed as `0c0db1b` on `58585f8`, `bench` 3429473 -> 3656950. SPRT pair pinned:
`REF` `58585f8` (the tree verdict 1's revert and S246 left; its engine
`d446783`'s node for node), `CAND` `0c0db1b`; open findings re-read at
pinning: items 1 to 15 as the pre-registration states them.

### Verdict 2's outcome (2026-09-30, the coordinator)

**No verdict at the cap, 2026-09-30 04:57: `0c0db1b` against `58585f8`, `Elo 2.34 +/- 2.63`,
`nElo 3.03 +/- 3.40`, LLR 0.87, 40000 games in 19 h 3 m, 0 forfeits**
(`adocs/data/S022_v2_sprt.log`, `adocs/data/S022_v2_sprt_pairs.txt`; marker
`DONE`). **No verdict with the nElo interval [-0.37, +6.43] reaching above zero**: read as a zero by the pre-registration's third row -- `QsDeltaEarlyOut` to 0 and the code leaves with it, the three goldens restored; deleting delta pruning is the recorded outcome. `Incomplete mating PV` 20 against 5,
an observation for the pre-registration's open finding 3's class (CHESS).
No follow-up run and no second pair (DEC-063, DEC-019). S022 completes on both verdicts.

## Verdict 2's removal, prepared 2026-09-29 (lands on the reading)

Written by a fresh Opus agent on the linked worktree `s022rm` at `51f69ff`,
briefed from `.tuning/coord/S022_rm_brief.md`, while verdict 2's SPRT
(`0c0db1b` against `58585f8`, `{0, 5}` nElo) held the machine: every
CPU-bound command ran under `nice -n 19` with `-j4` builds, and every number
below that decides anything is a node count, a best move or a pass; the only
seconds quoted are the suites' wall times under the match. **It is prepared
ahead of the reading and lands only on an H0 or no verdict**, the
pre-registration's second and third rows, which read the same way: the switch
goes to 0, the code leaves with it, and the three goldens of its item 13 go
back byte for byte (the S238 pattern). On an H1 it is discarded. The reading,
its result block and the shared documents are the coordinator's, and nothing
here states the verdict. Nothing was committed or staged by this agent.

**How.** `git show 0c0db1b -- src/search.cpp src/search_params.hpp
tests/test_search.cpp MANUAL.md | git apply -R`, which applied cleanly beside
S244's and S245's later hunks in all four; `git restore
--source=58585f8 --worktree -- tests/test_engine.cpp
tests/test_search_params.cpp`, which nothing after `0c0db1b` touched;
`tools/mutants/S022_v2_delta_early_out.py` deleted; then the records, by hand.
Before the hand edits `git diff 58585f8 -- src tests tools MANUAL.md` equalled
`git diff 0c0db1b 51f69ff` over the same paths but for hunk offsets: S244's
screen, S245's comments, cases and checker fix, and nothing else.

### What left

| what | where |
|---|---|
| the early-out block, between `futility_base` and `futility_best`, and its three paragraphs | `quiescence` in `src/search.cpp` |
| `QS_DELTA_EARLY_OUT` ("QsDeltaEarlyOut", 1, 0 to 1) and `QS_DELTA_PHASE_MIN` ("QsDeltaPhaseMin", 0, 0 to 24) with their comment block | `src/search_params.hpp`, the one X-macro both builds read |
| the suite "search: quiescence delta early-out" whole: its six release cases and the two tune-only ones, "the early-out is off at the switch's off value" and "the endgame disable follows its threshold" | `tests/test_search.cpp` |
| the two `golden_defaults` rows, 71 back to 69 | `tests/test_search_params.cpp` |
| Z01 to Z08 | `tools/mutants/S022_v2_delta_early_out.py`, deleted |
| the `QsDeltaEarlyOut` and `QsDeltaPhaseMin` option rows | `MANUAL.md` |

### What came in: records only

- `src/search.cpp` `quiescence`, where the block stood: a seven-line comment
  in the form S237's, S238's and verdict 1's sites took -- S022 tried a
  node-level delta early-out here, what it was, and that its `{0, 5}` SPRT
  showed no gain, so the early-out and its two parameters left. It carries no
  figures because none exist yet; the reading adds them in the precedents'
  form ("accepted H0, `nElo x +/- y` over n games, read as ...").
- `src/chesso.cpp` `iterative_deepening_search`: S245's sentence about the
  eight-queens board said `go depth 1` there reports 10187 nodes and that no
  case uses the board, both false once the early-out is gone. It now says
  36165 nodes, re-taken below, and names the stop half's board again. Not in
  the brief's list of files; a comment the removal made false.
- `tests/test_search_params.cpp`: the history comment says S022's second
  verdict added the two rows and its reading took them out, 69 to 71 and
  back to 69.
- The three goldens' sites each record the move and its return (DEC-142,
  DEC-233), below.
- `DEV_MANUAL.md`: the golden table's `mate_the_multicut_hides` and
  `golden_defaults` rows are `58585f8`'s again with one clause each saying
  S022's second verdict came and went (the shape S238's and verdict 1's rows
  took); the bench ledger keeps the landing's two paragraphs, which are its
  record, and gains the removal's entry, `3656950` -> `3429473`, beneath them.

`adocs/data/S022_v2_sprt.sh`, `S022_v2_remine_s097.log`, their
`adocs/data/README.md` rows and this file's earlier sections are untouched.

### The three goldens of item 13, back byte for byte

1. **S097's multicut row of "pruning does not hide a forced mate"**
   (`mate_the_multicut_hides`). The landing's row,
   `4N3/8/3P1ppk/4p2p/4P2P/1n1P2P1/Q4PK1/3q4 w - - 5 46`, `mate_in == 5`,
   becomes S131's row as `58585f8` has it,
   `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42`, `mate_in == 6`, at
   the same depth 14. The GOLDEN block keeps the re-mine as history: a
   paragraph "RE-MINED AT S022'S SECOND VERDICT, IN THE SCRIPT'S GUARD MODE,
   AND RESTORED WHEN THE EARLY-OUT LEFT" in place of the landing's, and the
   rows list gains "S022 verdict 2, the delta early-out on `58585f8`, guard
   mode: S097's row, depth 14, mate_in 5, 6580581 nodes, about 0.8 s", S131's
   entry reading "on `58585f8`, and again after S022 verdict 2's reading".
   **S131's row separates E21 again on this tree**, S244's screen in it:
   E21 applied by hand to `src/search.cpp` (the S033 protocol), `test_search`
   rebuilt, and "pruning does not hide a forced mate" fails at the row's
   `REQUIRE( result.mate_found )`, logged "mate the multicut hides, depth 14",
   after 40 of its assertions passed (`.tuning/coord/S022_rm/e21_case.log`);
   the file restored byte for byte (sha256) and `build/` rebuilt whole, the
   engine binary's hash the same as before the check. No
   `tools/mutation_check.py` run: its fixture is a clean commit in a throwaway
   worktree, which the landed tree gives the coordinator.
2. **The stop half's board** of `test_engine`'s "a stop inside the first
   iteration cuts it and the hard timer ends the search within its bound".
   The landing's `r1r1r1r1/1r1r1r1k/8/2n1n3/2N1N3/8/1R1R1R1K/R1R1R1R1 w - - 0
   1` with "**Golden**: 146994 nodes and 24 to 31 ms" becomes `58585f8`'s
   `q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1` with "Eight queens
   a side ... **Golden**: 13.8 ms on this machine"; `depth_1_floor_ms` is 3 on
   both. A paragraph beneath the golden records the move by the case's own
   rule, the rook-and-knight board and its numbers, and the return. Depth 1
   there is **36165 nodes** on this tree's Release build and on the reference
   binary alike, three fresh processes each, `bestmove d2h6`; the
   rook-and-knight board 146994 on both.
3. **The timer half's dead-timer figure.** The landing's "18594285 nodes and
   about 3.9 s ... `bestmove` after 3915 and 3935 ms ... The bound sits about
   10 times below it" becomes `58585f8`'s "25933707 nodes and about 5.9 s ...
   `bestmove` after 5913 and 6285 ms ... The bound sits about 14 times below
   it", with one sentence recording the re-take and its return. Depth 1 on the
   timer board, `rn1qk1nr/qqqqqqqq/8/8/8/8/QQQQQQQQ/RN1QK1NR w - - 0 1`, is
   **25933707 nodes** on both binaries, `bestmove f2f7`. The time was not
   re-taken: the machine is the SPRT's.

### The proofs

**Bench-identical to `58585f8`; S244's screen is the one engine difference
(DEC-242).** Compared against the running SPRT's own reference binary,
`.ref-builds/58585f8/build/src/chesso`, run and never rebuilt, with INV-6's
instrument (DEC-215):

| | |
|---|---|
| `bench` | **3429473**, the whole stream -- all 112 `info` lines' depth, score, nodes and PV and all eight `bestmove` replies, c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 -- identical with `time` and `nps` stripped |
| `bench 12` | 1694808, the whole stream identical the same way, the same eight replies |
| `tools/search_bench.py` | identical node for node and move for move: depth 9 48304 / 71580 / 25413, depth 12 104784 / 244824 / 117798, best c3d5 e2a6 d7c8q at both -- the figures this file's verdict-2 section recorded for `39841ed` |

That is identity on the bench positions and no more. S244's screen in
ProbCut's loop changed the search after `58585f8`, and its class -- a capture
that leaves insufficient material -- never reaches these positions, which is
why the streams agree and why INV-6's instrument cannot see it: S244 was
accepted on DEC-242, not on INV-6. **DEC-242's "the census first" is owed on
the landed tree before S244's stamp**: its ordinary-play census, the
coordinator's to run once the machine is free. The remaining difference to
`58585f8` in `src/`, `tests/`, `tools/` and `MANUAL.md` is S244's and S245's
hunks and the records above.

| | |
|---|---|
| both fast suites | niced, serial `ctest -L fast`, one build at a time: **41 of 41 each** -- Release `build` in 284.9 s, `test_mate_carry` 84.07 s against its 120 s ceiling, and `-DCHESSO_TUNE=ON` `build-tune` in 289.1 s, `test_mate_carry` 84.15 s (`release_suite_run1.log`, `tune_suite_run1.log`). Run again on the final tree, this section and the site's E21 sentence in it: 41 of 41 each, Release 278.4 s and tune 285.7 s, `test_mate_carry` 80.86 s and 83.26 s (`release_suite_final.log`, `tune_suite_final.log`) |
| format | `./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22` |
| prose | `tools/plan_prose_check.py`, one mode per invocation: `--citations` 0 flagged over 38 files, `--touches` 0 flagged, `--params` exit 0, `--gate` agreeing |
| mutant anchors | `tools/mutation_check.py` has no validate-only flag, so its own `load_mutants` and `validate` were run against the working tree over the whole of `tools/mutants/`: 156 mutants in 19 files, every anchor unique, no Z id left |
| the tune build | compiles with the two X-macro rows gone, nothing else naming them; at its defaults `bench` 3429473 with the whole stream identical to the reference binary's |
| the UCI surface | `test_uci_surface` green in both builds with **no refresh**; the Release build lists 5 options, the tune build 74, none of them `QsDelta*` |

The logs are in this worktree's `.tuning/coord/S022_rm/`, for the coordinator
to carry into the main tree's `.tuning/coord/` with the landing:
`bench_*.txt`, `bench12_*.txt`, `sb_*.txt` and `bench_tune.txt` (INV-6),
`depth1_boards.txt` and `depth1_timer_board.txt` (the goldens' node counts),
`e21_*` (the hand check), and the suite logs.

### Found, not changed

- **S245's item 2 is live again.** The eight-queens board's restored golden
  reads "13.8 ms on this machine", while the board ran 6 to 7 ms on the idle
  machine at this verdict's ruling -- what S245 found before the landing made
  it moot. The 3 ms floor holds and the case is green in both builds. The
  golden goes back byte for byte, as the pre-registration's row says, so the
  record paragraph names which figure is which and nothing is re-derived
  here; a re-take of that time needs the idle machine.
- `adocs/specs.md` goes false in two rows once this lands: the `search` row's
  verdict-2 sentence, which describes the early-out as present, gives
  `QsDeltaEarlyOut` 0 as its off value and ends "`bench` 3656950", and the
  `absent, search` row's "(its node-level early-out is in the tree behind
  `QsDeltaEarlyOut` 1 while S022 verdict 2 measures it)". Both replacements
  are proposed below, for the removal commit. The coordinator's file.
- `adocs/status.md`'s 09:51 entry calls the running pair `{-5, 0}` nElo
  (`--nonreg`), where the pre-registration, the landing's commit and the run's
  own banner (`bounds elo0=0 elo1=5`) say `{0, 5}`. The coordinator's file.

### Not run here, by the brief

The reading and its result block; DEC-141's second tier -- the Debug
self-play and `tools/gate_extra.sh` -- on the idle machine after the verdict;
`tools/mutation_check.py` on a fixture; any timing. Two more are owed on the
landed tree, both the coordinator's once the machine is free:

- **`adocs/data/S203_case_sweep.sh` over `adocs/data/S170_cases.tsv`, and the
  budgets re-derived from it before the next verdict.** `test_mate_carry`'s
  budgets were re-swept by S245's item 5 on `0c0db1b` with S244's screen -- A
  500000, C 1000000, E 500000, F 100000, B and D unchanged -- the tree this
  removal takes back out, and the TSV's header, `tests/test_mate_carry.cpp`
  and DEC-156 as DEC-162 amended it all say to re-run the sweep after
  anything that moves the tree. DEC-162's rule: each row takes the cheapest
  budget at its own stride whose cell reports a mate line, chosen on the mate
  count alone; `--ceilings` over the recorded grids answers the short-line
  ceilings. The green suite here proves nothing about the budgets: DEC-162
  makes one silent guarded case green by design, and only three of the five
  silent is red.
- **S244's ordinary-play census** (DEC-242's "the census first"), before
  S244's stamp -- the proofs above.

### Proposed `adocs/specs.md` edits, for the coordinator

Quoted against `specs.md` at `51f69ff`. Two edits, both for the removal
commit; `<...>` is the reading's.

**1. The `search` row's verdict-2 sentence**, from "**The node-level delta
early-out in quiescence, S022 verdict 2 (landed 2026-09-29, DEC-221)**:"
through "<verdict> -- `bench` 3656950." inclusive, becomes, in the form
verdict 1's and S238's sentences took:

> **A node-level delta early-out in quiescence was tried and left, S022
> verdict 2, landed 2026-09-29 and removed <date> (DEC-221)**: out of check,
> above `QsDeltaPhaseMin` 0 and before anything was generated, a node whose
> ceiling -- S112's futility base, the queen's price in S112's victim table,
> and that table's queen less its pawn when a pawn of the side to move stood
> on its seventh -- was at or below alpha ended there, returning the ceiling
> as an upper bound; it dropped the checking captures S112 searches, whether
> a capture gives check being unknown before generation, and never ran in
> check, behind `QsDeltaEarlyOut` 1, whose 0 gave the tree before it node for
> node (DEC-215). Before any game it ended 12.67 % of the out-of-check
> quiescence nodes that reach generation over the bench positions at depth
> 12, 72.3 % of them with nothing to generate and 804 with a move the tree
> searches -- reach, not a forecast (DEC-239). Its one `{0, 5}` nElo SPRT
> against the tree with S015's gate and without the early-out
> (`adocs/data/S022_v2_sprt.sh`) **<accepted H0 at n games | reached
> fastchess's 40000-game cap with no verdict> on <date> -- `Elo x +/- y`,
> `nElo x +/- y`, LLR l, f forfeits** (`adocs/data/S022_v2_sprt.log`), <a
> loss | a zero>: `QsDeltaEarlyOut` went to 0 and the code left with it,
> `QsDeltaPhaseMin` with it, the three goldens re-derived for the candidate
> restored with it -- `bench` 3429473, bench-identical to `58585f8`'s, S244's
> screen the one engine difference (DEC-242). Deleting delta pruning is
> S022's recorded outcome; S112's per-move futility is the form that stays.

**2. The `absent, search` row**: "delta pruning (its node-level early-out is
in the tree behind `QsDeltaEarlyOut` 1 while S022 verdict 2 measures it)"
becomes, in the form this file's landing section proposed for an H0 or no
verdict:

> delta pruning (S022 measured the node-level form and deleted it: <H0 at n
> games, `nElo x +/- y` | no verdict at the cap, `nElo x +/- y`>)

### Proposed commit text, for the coordinator

Not a verdict-closing commit: the reading's own commit carries the DEC-220
block. `<...>` is the reading's, and the coordinator appends its trailer
after the `Bench:` line.

```
Remove S022 v2's delta early-out on its <H0 | zero>

The node-level delta early-out in quiescence showed no gain. The SPRT
of 0c0db1b, the early-out behind QsDeltaEarlyOut 1, against 58585f8,
the tree without it, {0, 5} nElo at 8+0.08, <reading: H0 at n games,
nElo x +/- y | no verdict at the cap, nElo x +/- y>.
adocs/data/S022_v2_sprt.sh wrote that reading and its consequence
before the first game -- the switch to 0 and the code leaves with it,
the S238 pattern, three goldens back byte for byte -- so this is the
pre-registration executed: deleting delta pruning is S022's recorded
outcome, S112's per-move futility the form that stays.

Out: the early-out block in quiescence; the QS_DELTA_EARLY_OUT and
QS_DELTA_PHASE_MIN rows and their comment; the suite "search:
quiescence delta early-out", six release cases and two tune-only; the
two golden_defaults rows, 71 back to 69;
tools/mutants/S022_v2_delta_early_out.py, Z01 to Z08; the two MANUAL
option rows. The three goldens of the pre-registration's item 13 go
back to 58585f8's byte for byte: S131's multicut row of "pruning does
not hide a forced mate" in place of the re-mined S097 row, and in
test_engine's first-iteration case the stop half's eight-queens board
with its numbers and the timer half's dead-timer figure.

In: a comment at the site saying S022 tried the early-out and what its
SPRT read; each golden's GOLDEN block records the move and its return
(DEC-142, DEC-233); the params test's history comment records the two
rows coming and going; S245's comment in iterative_deepening_search
gives the eight-queens board's depth 1 as 36165 nodes and names the
stop half again; DEV_MANUAL's ledger gains the removal's entry and its
two golden rows read true again. The evidence stays under adocs/data/.

src, tests, tools and MANUAL.md are 58585f8's apart from S244's and
S245's hunks and those records. bench is 3429473 with the whole bench
stream and all eight bestmove replies identical to the SPRT's own
reference binary of 58585f8, bench 12 likewise, and
tools/search_bench.py identical at depths 9 and 12 (INV-6, DEC-215):
the tree is bench-identical to 58585f8, S244's screen the one engine
difference (DEC-242), and no second SPRT is owed for the removal.
S244's census is owed on this tree before its stamp, and the S170
budgets are re-swept by adocs/data/S203_case_sweep.sh before the next
verdict. Both fast suites 41 of 41, format and the prose checks clean,
every remaining mutant anchor unique; E21 applied by hand turns S131's
restored row red again.

Bench: 3429473
```
