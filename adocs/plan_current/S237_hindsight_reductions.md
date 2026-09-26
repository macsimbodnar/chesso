id:         S237
goal:       at a child node, the reduction its parent applied to the move that reached it is read together with how the static evaluation moved across that move: a heavily reduced move whose evaluation got worse for the mover gets a ply back, a lightly reduced one whose evaluation improved gives one up
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; the parent's reduction reaches the child as a trailing parameter of `negamax_at` in S231's pattern or as a field on `search_state_t`, the step stating which and pricing the read the way S191 and S231 did; two thresholds and two adjustments in `src/search_params.hpp` with stated ranges, seeded at form (b) from a census of reductions and evaluation deltas at depth 12 the step runs at its start, or at form (c), the step saying which per constant; at the off values the tree is the parent's exactly, bench signature identical (INV-6, DEC-215); a test per branch -- give back, give up, neither -- with the precondition counted; `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant per branch is killed (`tools/mutation_check.py`); the form built is the base case only, no later variant; Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, src/data_structures.hpp, tests/test_search.cpp, tools/mutants/, adocs/data/
excludes:   any change to the reduction formula itself (S236, which lands first so the adjustment can be fractional); reading history in this rule; any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-105, DEC-134, DEC-141, DEC-215, DEC-232
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-25 11:42
done:

## Why this exists

Depth is decided at the parent and never revisited. The 2026-09-19 study
(`adocs/data/2026-09-19_search_technique_study.md`, row N5) found the
open-source record correcting it one ply down: the child reads its parent's reduction
beside the static-evaluation delta across the move and gives a ply back or up,
measured at +6.03 over 8590 games. Its review (`adocs/audit/2026-09-19_study_review.md`,
F09) records that the record later simplified two variants away while keeping
the base rule, so the base rule is what this step builds. S098 made the
reduction a first-class quantity at the parent; S231 showed the cheap way to
hand a per-node value down (a trailing parameter, not a per-ply stack, 1.49 %
of nodes per second for one read per node at S191). Reported figures decide
what to try, never what to conclude (DEC-019).

## Shape

`src/search.cpp` `negamax` receives the reduction its parent applied to the
move that reached it; with `static_eval` at the child and the parent's, the
two conditions fire before the move loop and move `depth` by one ply either
way, bounded so a chain cannot compound (the S097 plumbing when it exists,
a plain cap before).

## Seeds (DEC-134)

Form (b) where a census gives a percentile to seed from -- the reduction
threshold at the p75 of applied reductions, the delta thresholds at the p50 of
absolute evaluation deltas across a reduced move -- and form (c) otherwise,
each stated at its site.

## From the description (DEC-221)

The implementing agent's brief carries this file and the analysis's row N5 in
prose and nothing else; the technique is implemented from that description,
every constant seeded in DEC-134's forms, and the stamp says so.

## Cost

One verdict at DEC-143's price, 12 to 20 hours, plus the census hour.

## What the tree already did, and the two assumptions checked (2026-09-25)

Written by the implementing agent before the code, from `src/search.cpp` at
`6d9c5ce`.

- The parent decides a late move's reduction in the move loop of `negamax_at`
  (`lmr_adjusted_reduction`, whole plies out of S236's fixed-point
  accumulator, plus S091's `SEE_LMR_EXTRA`, clamped to `child_depth - 1`) and
  searches the move once at `child_depth - reduction` on a zero window. Only
  that first search is reduced; S098 verdict 3's re-search and the full-window
  re-search run at their own depths. The child stored its static evaluation in
  `state->static_evals[ply]` (S108) with `TT_EVAL_NONE` in check, and nothing
  handed it the reduction.
- **"S236 lands first so the adjustment can be fractional."** S236's
  accumulator is in at `LmrRoundBias` 0 and its history term is out (DEC-231,
  DEC-213). What this rule moves is the child's `depth`, an integer of plies,
  by one ply, so nothing here is fractional and the accumulator is untouched,
  as `excludes:` says.
- **"The S097 plumbing when it exists, a plain cap before."** S097 is in: an
  extension of one ply applied at the parent to the table move alone, and the
  `excluded_move` parameter for its verification. It left no depth budget a
  second rule could share. Two facts bound the interaction instead: the
  extended move is the table move, searched first and never reduced (reduction
  needs `legal_moves_counter > 3`), so an extended child always arrives with
  reduction 0 and never corrects; and the verification search is handed 0.
  **The bound built is the plain cap**: a node that corrected its own depth
  hands every child 0, so no depth this rule moved is the input to the same
  rule one ply down. Corrections can still occur two plies apart, each on a
  depth its own parent decided; and a give-back never lifts a node past its
  parent's unreduced depth, because a heavy reduction is at least one ply.

**`accepts:` says "two thresholds and two adjustments"; what landed is four
thresholds and no adjustment constant**, the brief's reading, stated here
rather than silently. The description fixes the amount at one ply ("gets a
ply back", "gives one up") and the depth is whole plies, so an adjustment
constant could only be 0 or 1 -- a switch duplicating the off values the two
reduction thresholds already provide. The four are the heavy and light
reduction thresholds and the worse and better evaluation margins.
The owner accepted this reading on 2026-09-25 and it is recorded as DEC-232;
the accepts is read as amended there.

## What landed

- `negamax_at` gains a trailing `int parent_reduction`, S231's pattern; the
  entry points `negamax` and `negamax_probed` default it to 0. The reduced
  first search hands `handed_reduction` -- the reduction, or 0 at a corrected
  node; every other search (first move, both re-searches, null move,
  verification, the root from `search()`) hands 0.
- The rule sits directly after `state->static_evals[ply] = static_eval`, so
  reverse futility, the null move, the extension, the move loop and the store
  all see the corrected depth; the table cutoff and the leaf test above it do
  not, as they run before the node has a static evaluation. Guards: `ply > 0`,
  `parent_reduction > 0`, not in check, the parent's slot not the sentinel,
  both statics strictly inside the mate band; the give-up also needs `depth >=
  2`. The mover's delta is `-static_eval - static_evals[ply - 1]`. The two
  branches are an `if` / `else if` on strict delta tests with non-negative
  margins, so never both.
- Four rows in `src/search_params.hpp` with ranges and seeds stated there:
  `HindsightHeavyReduction` 3 [1, 126], `HindsightLightReduction` 1 [0, 2],
  `HindsightWorseMargin` 24 [0, 716], `HindsightBetterMargin` 24 [0, 716]. The
  two reduction thresholds are each branch's switch, off at 126 and 0. **The
  margins have no off end, deliberately**: widening a range until a tool has
  an off value is DEC-215's own rejected option, and each branch already has
  its switch. That departs from the brief's "ranges whose one end is off" for
  the two margins and is reported as such.
- Probe fields on `search_node_probe_t`: `hindsight`, `parent_reduction`,
  `handed_reduction[]`.

**The plumbing, and its price.** A trailing parameter rather than a field on
`search_state_t`: the reduction is already a local at the one call site that
needs to hand it, so the parameter costs one argument store per call and one
read per node, and nothing on the moves that are not reduced beyond passing a
0; a per-ply field would add the same read per node plus a store per move into
a structure the whole search shares. **Priced by argument, not by a timing
run** -- the brief reserves timing to the coordinator. S191's measured 1.49 %
was for resolving and testing a pointer per move; this is one integer read per
node, and `negamax_at` already took four arguments past the six an x86-64
call passes in registers under the standard convention, so this is a fifth
stack slot of the same kind as `excluded_move`'s. `bench`'s nps carries the
cost; nobody has timed it.

## The census (DEC-134 (b), DEC-212's pattern)

`adocs/data/S237_census.py`, rules in its docstring before the numbers; output
`adocs/data/S237_census.txt`, 2026-09-25, about a minute with its two builds
(not the hour the Cost section allowed). Over the eight bench positions at
depth 12 on the tree with both switches off: 78661 sites, 3.73 % of 2108720
nodes, every arrival through a reduced search reaching the rule. Applied
reduction p25 1, p50 2, **p75 3**, p90 4, p99 5 (1: 36.70 %, 2: 31.24 %, 3:
21.02 %, 4: 9.30 %, 5 and up: 1.75 %). |mover delta| p25 10, **p50 24**, p75
45, p90 87, p99 182. Only 35.78 % of sites have two plies left, which is where
the give-up can fire. Depth 10 gives the same three seeds. **Seeds**:
`HindsightHeavyReduction` 3 (p75), both margins 24 (p50), `HindsightLightReduction`
1 ((c), the midpoint of 0 to 2; no percentile is named for it). Counterfactual
at the seeds: give-back on 8.79 % of sites, give-up on 2.52 %.

## Measurements (2026-09-25, the implementing agent, worktree `s237`)

- Parent `6d9c5ce`, built in a throwaway worktree: `bench` **4803214**,
  replies c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6; `search_bench` depth 9
  48522 / 85714 / 28080 (c3d5 e2a6 d7c8q), depth 12 129499 / 411457 / 172984
  (c3d5 e2a6 d7c8q).
- Candidate at the seeds: `bench` **4845333**, +0.88 %, the eight replies
  unchanged; `search_bench` depth 9 18409 / 91713 / 27401 (g5f6 e2a6 d7c8q),
  depth 12 128457 / 632820 / 203399 (c3d5 d5e6 d7c8q).
- Off values, proved on the tree (DEC-215): the working tree's `src/` in a
  throwaway worktree with `HindsightHeavyReduction` 126 and
  `HindsightLightReduction` 0 prints `bench` **4803214** with the parent's
  eight replies, and `search_bench` reproduces the parent exactly at depths 9
  and 12, counts and best moves (`.tuning/coord/S237_offvalue.log`). The
  census's instrumented and control binaries, built from the same forced
  source, print 4803214 too. The tune build holds it by a case, "hindsight
  reductions never fire at the off values".
- Tests in `tests/test_search.cpp`'s guard suite over `hindsight_drive_t`, a
  PV drive of `guard_fixture_t` at ply 2, depth 4, that plants the reduction
  as `negamax_probed`'s trailing argument and the parent's slot so the
  mover's delta is the named number, and reads both back (the probe's
  `parent_reduction`, the two stack slots): "a heavily reduced move whose
  evaluation got worse gets a ply back", "a lightly reduced move whose
  evaluation improved gives a ply up", "a reduced move outside both
  conditions keeps its depth" (nine legs plus the sentinel, mate-band and
  root guards), "a node that corrected its depth hands its children no
  reduction" (with an uncorrected control and the reduced moves counted), and,
  tune build only, "hindsight reductions never fire at the off values".
  `tests/test_search_params.cpp` gains the four golden rows (62 -> 66) and
  `MANUAL.md` the four options. "pruning does not hide a forced mate" is green
  at the seeds with no row touched.
- Mutation, `tools/mutation_check.py tools/mutants/S237_hindsight.py
  .ref-builds/mut` on a detached fixture commit `aed878b` (rsync of the
  working tree, `tests/doctest` initialised), header `worktree ... at aed878b
  clean`, `baseline green, 40 tests, bench 4845333 nodes via engine`,
  **`mutation score 4 of 4 (100%)   killed 4`**, 645 s: Y01 and Y04 by the
  give-back case, Y02 and Y04 by the give-up case, Y03 by the bound case.
  **Every one of the four also turns "pruning does not hide a forced mate"
  red** -- the recurring fragility, here in the right direction: a wrong
  correction loses a guarded mate. (`.tuning/coord/S237_mutation.log`.)
- Not run by the agent, owed before completion (DEC-141): Debug self-play and
  `tools/gate_extra.sh` -- the coordinator's, with the SPRT.

## Proposed `adocs/specs.md` sentences (the coordinator's to place)

> Hindsight reductions (S237): a node reached through a reduced first search
> reads the reduction its parent took (`negamax_at`'s `parent_reduction`) and
> the mover's static-evaluation delta across the move, and searches one ply
> deeper when the reduction is at least `HindsightHeavyReduction` and the delta
> is below `-HindsightWorseMargin`, or one ply shallower when the reduction is
> between 1 and `HindsightLightReduction`, the delta above
> `HindsightBetterMargin` and at least two plies remain. Never at the root, in
> check, with the parent's slot empty or on a mate-band static score; a node
> that corrected its depth hands its children no reduction. At 126 and 0 the
> two thresholds switch the rule off and the engine is the tree before it.

## Landing and second tier (2026-09-25, the coordinator)

Landed as `4d8c501` on `cd9d0e7`, `bench` 4803214 -> 4845333; the accepts read
as amended by DEC-232 (four thresholds, a fixed ply), the owner's ruling. Cold
fast check: no defect; the delta's sign, the hand-down only on the reduced
first search, the corrected depth's use after the table cutoff and the off
values all read correct. Two test-side gaps, neither reaching play, carried
into the pre-registration's open findings: no mutant moves `handed_reduction`
onto another call site, and none targets the `depth >= 2` or mate-band guards
(the guard cases assert them directly). Both suites 40 of 40 and the format
check on the landing tree. Debug self-play 8 games, 0 `Assertion`, 0
`disconnect` (`.tuning/coord/S237_selfplay/`); `tools/gate_extra.sh` 5 stages
green in 1130 s (`.tuning/gate_extra_2026-09-25_S237.log`) (DEC-141). SPRT
pair pinned: `REF` `cd9d0e7`, `CAND` `4d8c501`.

## The verdict (2026-09-26, the coordinator)

**H0, 2026-09-26 02:01:46: `4d8c501` against `cd9d0e7`, `Elo -5.08 +/- 6.07`,
`nElo -6.34 +/- 7.58`, LLR -2.96, 8072 games in 3 h 50 m, 0 forfeits either
side** (`adocs/data/S237_sprt.log`, `adocs/data/S237_sprt_pairs.txt`). The
nElo interval [-13.92, +1.24] reaches above zero, so the pre-registration's
third row binds and reads it as a zero, not a loss: `HindsightHeavyReduction`
goes to 126 and `HindsightLightReduction` to 0, the off values proved the
parent's tree to the node, and the code leaves with them -- the trailing
parameter, the rule, the four rows, the probe fields, the cases and the
mutants -- as a behaviour-neutral removal INV-6 discharges. No follow-up run
and no second setting (DEC-063, DEC-019): the record's +6.03 was a direction
and did not transfer at this seed. `Incomplete mating PV` 15 against 14, an
observation (CHESS). The removal is a fresh agent's; S238, built on
`4d8c501`, is rebased onto the removal and its off-value identity re-proved
before it lands.
