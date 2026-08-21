id:         S022
goal:       decide between delta pruning and the per-move futility S112 adds, by measurement -- deleting delta pruning is a valid recorded outcome
accepts:    two SPRT verdicts, one per change and each against the commit before it, in either order: delta pruning, and the re-measure of the S015 quiescence SEE pruning, which measured 0 Elo when see() cost 12.1 % more than it does now (an Apple-machine figure, pre-DEC-049)
touches:    src/search.cpp quiescence
excludes:
decisions:
closes:
blocks:
paused_by:
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

- **Current truth: chesso has no delta pruning, in either form.** No delta
  test exists in `quiescence()` -- the only skips today are the non-capture
  drop at src/search.cpp:320 and S015's SEE gate at :332-335 (`!in_check &&
  !capture_cannot_lose && !see_ge(move, 0)`), and specs.md's "absent, search"
  row lists delta pruning absent. The goal's "deleting delta pruning"
  therefore means the trial add is reverted or never bought and the zero
  recorded -- no shipped code comes out on that arm.
- **The tree this step measures against**: after S112 (per-move futility in
  the filter loop above :322, fail-soft raise into :332, promotion /
  gives-check / in-check exemptions, QS_FUTILITY_MARGIN plus a dedicated
  `qs_futility_value[]`) and S131 (quiet queen promotions pass :310). Both
  land first in plan order, their verdicts recorded whatever they were.
- **The re-target's three configurations, named.** Baseline F+S = futility
  plus S015's SEE gate, as S112/S131 leave the loop. **Verdict 1** = delete
  the SEE gate: F vs F+S -- the S015 re-measure the accepts fold in.
  **Verdict 2** = add the node-level early-out D on top of verdict 1's
  winner: F+D vs F, or F+S+D vs F+S. Two sequential SPRTs, each against the
  commit before it (accepts; DEC-020), walk one path through the three; the
  third pairwise comparison is not bought and not owed.
- **The early-out's site and arithmetic**: under `!in_check`, between the
  qply cap at :287 and generation at :289. Reuse S112's numbers rather than
  minting new ones: skip when `futility_base + qs_futility_value[QUEEN] +
  promo_allowance <= alpha`, where promo_allowance = queen minus pawn from
  the same table when a friendly pawn stands on the seventh, else 0.
  Fail-soft return is that same ceiling -- the node-level analog of S112's
  per-move raise; returning bare stand_pat would claim an upper bound the
  pruned moves can exceed -- stored TT_ALPHA_NODE via the :387-390 pattern.
  A fired node is a certified fail-low, unlike the :287 cap return, which is
  a truncation and rightly stores nothing.
- **Endgame disable**: `game_phase()` (src/evaluation.cpp:1111-1115, the
  INV-4 phase accumulator clamped to 24) is the O(1) predicate the null-move
  zugzwang guard already keys on (`> 0`, src/search.cpp:581). Disable the
  early-out at `game_phase() <= QS_DELTA_PHASE_MIN`, seed in section 4.

### 3. Implementation sketch -- the decision protocol

Two SPRTs, priced as two verdicts by plan.md ("S097 and S022 two each");
either order per the accepts. Recommended order, reason stated:

1. **Verdict 1, the S015 re-measure: delete the SEE gate** (:322-325, one
   condition block, capture_cannot_lose's call goes with it). H1 at the
   non-regression bounds -> deleted: the gate re-measured ~0 even with
   futility present and a see() 12.1 % cheaper; specs' quiescence row and
   its Open items entry are rewritten in the same commit, and a decisions.md
   entry records the deletion superseding S015's kept-at-zero (AGENTS.md
   section 8, before or alongside). H0 -> kept, now with a current number;
   the open item is discharged either way. Run first so verdict 2's
   condition is written against the loop that survived.
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
- **The SEE skip must not take the futility raise.** Moves reaching :322
  survived S112's test, so their futility_value exceeds alpha; raising
  best_value to it on a SEE skip fakes a fail-high through the :387-390
  store. S112's asymmetry -- futility skip raises, SEE skip does not -- is
  correct, not an oversight. Verdict 1 deletes the gate or keeps it as-is;
  "fixing" the raise is a bug, and this answers the question S112 section 5
  deferred here.
- **game_phase and the disable.** The phase is an INV-4 accumulator,
  promotion-clamped at 24, already the zugzwang boundary at :571 -- reusing
  it costs one compare. What it does not capture: low-material positions at
  phase > 0 (minor-piece endings), which is the threshold's range and S127's
  business if delta ships.
- **Two changes, one function, strict sequence.** Verdict 1's commit is only
  the gate deletion (or nothing); verdict 2's only the early-out. The fast
  suite's two quiescence mate cases -- "a side in check may not stand pat"
  (tests/test_search.cpp:701) and "mate is recognised at depth zero"
  (tests/test_search.cpp:775) -- gate each commit; pruning hiding mate is
  the recurring bug -- and the early-out never fires in check by
  construction, so the :370 mate path stays reachable.

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
  stand_pat is an alpha-side overstatement (src/evaluation.hpp:24-27), so
  futility and the early-out both fire less -- conservative, sound. S039
  moves how often that happens, not the arithmetic.
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
- https://api.github.com/repos/official-stockfish/Stockfish/commits/3dfbc5de25705fadcb4b5b7a551eacb3eb75d171
  -- "Remove non-pawn material check in qsearch pruning", 2025-02-05, passed
  simplification STC and LTC at <-1.75, 0.25>; message only.
- https://talkchess.com/viewtopic.php?t=75059 -- hgm on delta pruning
  pre-empting the stand-pat cutoff and needing a margin over a post-move
  evaluation ceiling; no measurements.
