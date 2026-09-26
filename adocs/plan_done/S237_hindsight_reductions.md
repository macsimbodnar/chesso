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
done:       2026-09-26 -- one verdict, H0 read as a zero, and the code left. **What was tried**: a node reached through a reduced first search read its parent's reduction (a trailing `parent_reduction` on `negamax_at`, S231's pattern) beside the mover's static-evaluation delta and corrected its depth by one ply -- back for a reduction of at least `HindsightHeavyReduction` whose delta fell below `-HindsightWorseMargin`, up for one of at most `HindsightLightReduction` whose delta rose above `HindsightBetterMargin` with two plies left; a corrected node handed its children no reduction. Four thresholds and a fixed ply, the accepts read as amended by DEC-232 (the owner's ruling); seeds 3 / 24 / 24 from a depth-12 census (DEC-134 (b)) and 1 at its midpoint (c). Landed as `4d8c501`, `bench` 4803214 -> 4845333; off values 126 and 0 proved the parent's tree to the node (eight replies, `search_bench` at 9 and 12; DEC-215, INV-6); five cases, mutants Y01 to Y04 4 of 4 killed; cold fast check no defect, two test-side gaps carried as open findings; Debug self-play 8 games 0 `Assertion`, `gate_extra` 5 stages 1130 s (DEC-141). **Decided at `{0, 5}` nElo against `cd9d0e7`: H0, `Elo -5.08 +/- 6.07`, `nElo -6.34 +/- 7.58`, LLR -2.96, 8072 games in 3 h 50 m, 0 forfeits either side** (`d9bac46`; the ledger's thirty-second row in `211fe04`, which also stopped `tools/ledger.py` counting S235's verdict twice); the interval [-13.92, +1.24] reaches above zero, so the pre-registration read it as a zero, not a loss (DEC-063). **The code left** (`1680439`): `src/`, `tests/` and `tools/` are `cd9d0e7`'s apart from a four-line comment at the site and the later ledger fix, proved by `bench` 4803214 with the whole bench stream identical to a fresh build of `cd9d0e7` and `search_bench` identity at both depths; both suites 40 of 40; S234's mutant list 4 of 4 on a clean fixture; cold fast check no defect; Debug self-play 8 games 0 `Assertion`, `gate_extra` 5 stages 1221 s on the removal (DEC-141). No follow-up run: the record's +6.03 over 8590 was a direction and did not transfer at this seed (DEC-019). `specs.md`'s search row states the trial; `DEV_MANUAL.md`'s bench ledger carries the landing and the removal; `MANUAL.md` lost the four rows. Implemented from row N5's description (DEC-221); no other project's code was opened. S238, built on `4d8c501`, is rebased onto the removal under its own step. `bench` 4803214 -> 4845333 -> 4803214.

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

## The removal, 2026-09-26

Written by a fresh Opus 5 agent on the linked worktree `s237rm` at `211fe04`,
briefed from `.tuning/coord/S237_removal_brief.md`. The pre-registration's H0
row and its "interval reaching above zero" row were written before the first
game and read the same way: **`HindsightHeavyReduction` to 126 and
`HindsightLightReduction` to 0, the off values, and the code leaves with
them.** That is executed here. No second SPRT is owed: the removal is
behaviour-neutral at the reverted defaults and INV-6 discharges it, the shape
S235's and S236's removals already have.

### What left

| what | where |
|---|---|
| the trailing `parent_reduction` parameter of `negamax_at` and of the two entry points, with its defaults and its header paragraph; every call site's extra `0` | `src/search.cpp`, `src/search.hpp` |
| the rule after `state->static_evals[ply] = static_eval`, its comment block, the `hindsight` local and its probe writes; `handed_reduction` and its comment | `negamax_at`, `src/search.cpp` |
| the four X-macro rows `HindsightHeavyReduction`, `HindsightLightReduction`, `HindsightWorseMargin`, `HindsightBetterMargin` and their seed comments | `src/search_params.hpp` |
| the probe fields `hindsight`, `parent_reduction`, `handed_reduction[]` | `search_node_probe_t`, `src/data_structures.hpp` |
| `hindsight_drive_t` and this step's five cases | `tests/test_search.cpp` |
| the four golden rows and the count, 66 back to 62, with the header sentence | `tests/test_search_params.cpp` |
| the mutant list Y01 to Y04 | `tools/mutants/S237_hindsight.py`, deleted |
| the four option rows | `MANUAL.md` |

The landing changed no existing test: its diff to `tests/test_search.cpp`
removes no line, so nothing needed restoring beyond taking its additions out.
`src/`, `tests/test_search.cpp` and `tests/test_search_params.cpp` were
restored from `cd9d0e7` whole. **No reason to keep any of the code was found
and none was stated.**

### What stayed

The evidence: `adocs/data/S237_census.py` and `.txt`, `S237_sprt.sh`,
`S237_sprt.log`, `S237_sprt_pairs.txt` and their `adocs/data/README.md` rows.
`DEV_MANUAL.md`'s node-signature ledger **gains** an entry for this removal,
`4845333` -> `4803214`, beneath S237's landing entry. Its `golden_defaults`
row, which read 62 throughout S237's landing (stale while the four rows were
in), now says they came and went. This step file is extended, not rewritten.

### The proofs

**The remaining difference to `cd9d0e7` in `src`, `tests` and `tools` is one
comment plus `211fe04`'s ledger fix.** `git diff --stat cd9d0e7 -- src tests
tools`:

- `src/search.cpp | 5 +++++` -- a four-line comment and its blank line after
  `state->static_evals[ply] = static_eval` in `negamax_at`, saying the rule
  was tried there and what its SPRT read. No code.
- `tests/test_ledger.py | 17 +` and `tools/ledger.py | 18 +-` -- `211fe04`'s
  "count a verdict once" fix to the ledger generator, a commit after the
  landing and not part of it. It stays.

| | |
|---|---|
| `bench` | **4803214**, and the whole `bench` stream -- every `info` line's depth, score, nodes and PV and all eight `bestmove` replies, c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 -- identical to a binary built fresh from `cd9d0e7` in this step's own throwaway worktree `.ref-builds/parent` (compared with times and nps stripped). That worktree was checked before being called the parent: HEAD `cd9d0e7` detached and clean, `git diff --stat cd9d0e7 <its HEAD> -- src tests` empty |
| `tools/search_bench.py` | identical at both depths, node for node and move for move: 48522 / 85714 / 28080 with c3d5 / e2a6 / d7c8q at 9, and 129499 / 411457 / 172984 with c3d5 / e2a6 / d7c8q at 12 (INV-6) |
| both fast suites | **40 of 40**, Release `build` and `-DCHESSO_TUNE=ON` `build-tune`; `./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22` |
| prose | `tools/plan_prose_check.py`, one mode per invocation: `--citations` 0 flagged over 42 files, `--touches` 0 flagged over 42 files, `--params` no finding (exit 0, no output) |
| the UCI surface | `MANUAL.md` has no `Hindsight*` row and the options are gone from the parameter table with them, so `test_uci_surface` is green in both builds with **no refresh** |
| the golden | `tests/test_search_params.cpp` is `cd9d0e7`'s file byte for byte; DEC-142's re-derivation for `golden_defaults` is its diff against `src/search_params.hpp`, as the block names |
| mutation | S234's list re-run, below |

Nothing above is a timing: every number is a node count, a best move or a
pass. No match, fit or timing run was started here.

**Mutation: S234's list on a clean detached fixture.** The fixture is
`.ref-builds/mut` at `cd9d0e7`, detached and clean, since nothing here is
committed. It is this removal's tree for the purpose: `src`, `tests`, `tools`
differ from it only by `211fe04`'s ledger test and the four-line comment
above, which no anchor in the list reaches and no compiler emits, and the
mutant list read is this worktree's, unchanged since `cd9d0e7`. The baseline's
`4803214` is this tree's own total. Header:

```
worktree /home/max/ws/chesso-s237rm/.ref-builds/mut at cd9d0e7 clean
list     /home/max/ws/chesso-s237rm/tools/mutants/S234_tt_estimate_margins.py   clean
baseline green, 40 tests, bench 4803214 nodes via engine
```

**Mutation score 4 of 4 (100%), killed 4, wall 690 s**. Each of G01 to G04
was caught by one fast case and moved the bench
(`.tuning/coord/S237rm_mutation.log`, `MUTATION-RUN-DONE`). S234's list still
validates on the tree the removal leaves.

### Proposed commit text, for the coordinator

Subject 46 characters. No DEC-220 result block: `d9bac46` closed the verdict
and recorded it, and repeating the block here would count the verdict twice in
the ledger (the defect `211fe04` fixed after S235's removal).

```
Remove S237's hindsight reductions on their H0

Hindsight reductions read a zero. Their gainer SPRT of 4d8c501 against
cd9d0e7, {0, 5} nElo at 8+0.08, accepted H0 at 8072 games: nElo -6.34
+/- 7.58, an interval [-13.92, +1.24] reaching above zero, which
adocs/data/S237_sprt.sh reads as a zero and not a loss. That reading and
its consequence were written before the first game -- the two thresholds
go to their off values, 126 and 0, and the code leaves with them -- so
this is the pre-registration executed, not a judgement after a number.

Out: negamax_at's trailing parent_reduction and its defaults on negamax
and negamax_probed; the rule after the static-eval store and
handed_reduction; the four Hindsight* X-macro rows; the probe fields;
hindsight_drive_t and the step's five cases; the four golden_defaults
rows (66 back to 62); tools/mutants/S237_hindsight.py; the four MANUAL
option rows. The landing changed no existing test.

In: a four-line comment at the site saying the rule was tried and what
it read; the evidence (census, pre-registration, log, pairs file and
their README rows); DEV_MANUAL's ledger gains the removal's entry.

src, tests and tools are cd9d0e7's apart from that comment and
211fe04's ledger fix. bench is 4803214 with the whole bench stream and
all eight bestmove replies identical to a fresh build of cd9d0e7, and
tools/search_bench.py is identical at depths 9 and 12 (INV-6, DEC-215):
no second SPRT is owed. Both fast suites 40 of 40, the format check and
the three prose checks clean, S234's mutant list 4 of 4 on a clean
fixture.

S237, DEC-063, DEC-019: the record's +6.03 was a direction and did not
transfer at this seed.

Bench: 4803214
```

### Proposed `adocs/specs.md` edit, for the coordinator

One edit in the search row. The whole S237 sentence, from
"**Hindsight reductions, S237 (landed 2026-09-25, DEC-222, DEC-232)**:"
through "Decided by one `{0, 5}` nElo SPRT against the tree without it
(`adocs/data/S237_sprt.sh`): <verdict>." inclusive, becomes:

> **Hindsight reductions were tried and left, S237, 2026-09-25 to 2026-09-26
> (DEC-222, DEC-232)**: a node reached through a reduced first search corrected
> its depth by one ply from the parent's reduction and the mover's
> static-evaluation delta, seeded from a census at depth 12
> (`adocs/data/S237_census.txt`); its `{0, 5}` nElo SPRT against the tree
> without it (`adocs/data/S237_sprt.sh`) **accepted H0 at 8072 games on
> 2026-09-26 -- `Elo -5.08 +/- 6.07`, `nElo -6.34 +/- 7.58`, LLR -2.96, 0
> forfeits either side** (`adocs/data/S237_sprt.log`); the nElo interval
> reaches above zero, so the pre-registration reads it as a zero and not a
> loss, and the two thresholds went to their off values with the code behind
> them -- `bench` 4803214, `cd9d0e7`'s own total node for node.

The sentence after it, "**A two-ply continuation history table was tried and
left, S231 ...", is untouched.

### Owed to the coordinator, not run here

The second tier (DEC-141): the removal touches the search, so Debug self-play
and `tools/gate_extra.sh` are owed on the landing tree; both load the machine
the way a match does and the brief reserves that to the coordinator. No guard
test or mutant is owed: the removal takes a rule away, and the identity above
is its proof. S238, built on `4d8c501`, is rebased onto this removal by its
own agent.
