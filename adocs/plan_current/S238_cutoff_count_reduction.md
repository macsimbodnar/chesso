id:         S238
goal:       each node counts the beta cutoffs that happened at it, and the parent reduces its later moves more when the child's count is high -- a child that keeps failing high is a node whose siblings are not worth depth
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; a per-ply cutoff counter on `search_state_t` or handed back through `negamax_at`'s return path, cleared on entry to a node, the step stating which and pricing the cost the way S191 and S231 did; one threshold and one adjustment in `src/search_params.hpp` with stated ranges, seeded at form (c), the midpoint, or at form (b) from a census of per-node cutoff counts at depth 12, the step saying which; at the off value the tree is the parent's exactly, bench signature identical (INV-6, DEC-215); a test drives a child to a stated number of cutoffs and observes the parent's reduction of its next move, with the precondition counted; `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant is killed (`tools/mutation_check.py`); Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, src/data_structures.hpp, tests/test_search.cpp, tools/mutants/, adocs/data/
excludes:   any use of the count outside the reduction; the accumulator (S236, first); any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-105, DEC-134, DEC-141, DEC-215
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-25 on 1a35f16; rebased onto 1680439 by a fresh Opus subagent, 2026-09-26
done:

## Why this exists

The 2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, row
N4) found the open-source record counting beta cutoffs per node and reading the
child's count from the parent to reduce later siblings more, measured at
+6.00 over 8688 games and refined later at +3.52. The mechanism is one
counter and one comparison inside `src/search.cpp` `negamax`'s move loop,
beside `lmr_adjusted_reduction`. Reported figures decide what to try and
never what to conclude (DEC-019); the price is DEC-143's.

## Shape

A counter incremented where the node returns on `beta`, read by the parent
after the child returns and compared against the threshold; over it, the
parent's reduction of its later quiet moves rises by the adjustment, in
S236's fixed-point unit.

## Seeds (DEC-134)

Form (c), the midpoint of each declared range, unless the census the step
may run gives a percentile to seed from -- then form (b), stated at the site.

## From the description (DEC-221)

The implementing agent's brief carries this file and the analysis's row N4 in
prose and nothing else; the technique is implemented from that description,
every constant seeded in DEC-134's forms, and the stamp says so.

## Cost

One verdict at DEC-143's price, 12 to 20 hours.

## What the tree already did, and the three assumptions checked (2026-09-25)

Written by the implementing agent before the code, from `src/search.cpp` at
`1a35f16`.

- A node leaves on a bound in seven places: the table cutoff
  (`tt_entry_answers`), reverse futility, the null move, S097's multicut, the
  move loop's `score >= beta` break, and quiescence's stand-pat and capture
  cutoffs. The move loop breaks on its first fail-high, so **a node's own
  move loop fails high at most once per visit**; nothing counted cutoffs.
- The parent reduces a late quiet through `lmr_adjusted_reduction` (S236's
  ticks, rounded once at `LmrRoundBias` 0) plus S091's `SEE_LMR_EXTRA`,
  clamped to `child_depth - 1`. The shallow-depth gates read the same
  adjusted reduction through `lmr_depth_of`. S237's rule hands the child the
  reduction actually taken (`handed_reduction`).

**1. "A counter incremented where the node returns on `beta`", and which
count.** Read literally -- each node counts its own fail-highs, cleared on its
own entry -- the count is 0 or 1 at every node, so a threshold has nothing
to compare and "keeps failing high" cannot happen. **The reading built**: the
count lives in a per-ply slot the *parent* clears, and each child adds one to
its own ply's slot when its move loop fails high; while the parent walks its
moves the slot holds how many of its children so far failed high. "The
child's count, seen from the parent" is that slot, `cutoff_counts[ply + 1]`.
Only the **move-loop** fail-high counts -- a searched move of the child
reached its beta, which is what "beta cutoff" names; the table cutoff,
reverse futility, the null move and the multicut return a bound without a
move having refuted anything, and quiescence is a different function whose
nodes this rule never reduces. S097's verification is excluded: it is a
search of the node's own position less a move, at the node's own ply, and
counting it would add a phantom child to the grandparent's slot.

**"Cleared on entry to a node"** is kept in substance and moved in place: the
node clears its children's slot **just before its move loop**, not at its top,
because the null move's child and S097's verification's children also search
at `ply + 1` before the loop, and a cutoff there is not one of the loop's
children failing high. The mutant `Q03_not_cleared` holds the clear and
`Q06_cleared_per_move` the rejected per-child reading.

**The plumbing: a per-ply array on `search_state_t`, not a return value.**
The parent reads the slot after each child returns without any of the four
recursion sites carrying anything, and a re-search of the same move adds to
the same slot. A value handed back would need `negamax_at` to return a pair
or take an out-parameter at every recursion site, the null move's and the
verification's included, only to discard it there. **Priced by argument, not
by a timing run** (the brief reserves timing to the coordinator): per node one
store (the clear) and at most one increment, both to the slot beside the
node's own in a 512-byte array next to `static_evals`; per reduced late quiet
one load. S191's measured 1.49 % was a pointer resolved and tested per move;
S231 chose a parameter because a per-ply stack costs a store per move *and* a
load per node, where this costs one store per node and one load per reduced
quiet, the site already computing a table lookup and a sum. Nobody has timed
it.

**2. "Later moves".** Every late quiet the node reduces after the count has
passed the threshold, read per move, so each sees every child searched before
it, the one just returned included. Not "the previous child's count": under
this reading that is 0 or 1.

**3. "In S236's fixed-point unit".** `cutoff_count_ticks` returns ticks, added
to `node_adjustment` in the sum `lmr_adjusted_reduction` rounds once, so the
adjustment may be a fraction of a ply. The clamp to `child_depth - 1` then
binds as for every reduction.

**S237's interaction, stated.** The adjusted reduction is what the reduced
first search hands its child as `parent_reduction`, so a child reached under
this rule may meet `HindsightHeavyReduction` where it would not have. That is
intended: the child is told what was actually taken off it, which is the
quantity S237's rule is defined on.

## What landed (in the worktree, uncommitted)

- `search_state_t::cutoff_counts[MAX_PLY]` (`src/data_structures.hpp`).
- In `negamax_at`: the clear of `[ply + 1]` before the move loop; the
  increment of `[ply]` at the move loop's fail-high when `excluded_move == 0`;
  at the late quiet's reduction, `cutoff_count_ticks(cutoff_counts[ply + 1])`
  added to `node_adjustment` before the rounding. The shallow-depth gates are
  untouched (`excludes:`).
- `CutoffCountThreshold` 5 [0, 63] and `CutoffCountReduction` 1024 [0, 2048]
  in `src/search_params.hpp`, ranges and seeds stated there. **The off value
  is `CutoffCountReduction` 0**; the threshold has no off end and is not
  given one (DEC-215's rejected option; S237's margins, DEC-232).
- Probe fields `cutoff_count[]` (the slot, read from the slot and not from
  the rule's local) and `node_adjustment`.
- Implemented from the description (DEC-221), base rule only.

## The census (DEC-134 (b), DEC-212's pattern)

`adocs/data/S238_census.py`, the rule in its docstring before the numbers;
output `adocs/data/S238_census.txt`, 2026-09-25, about two minutes at `-j4`
under `nice -n 19`. Instrumented and control `bench` both 4845333, the
parent's. At depth 12: 244621 sites (late quiets reduced through
`lmr_adjusted_reduction`), 10.49 % of 2332561 nodes; count p25 1, p50 3,
**p75 5**, p90 9, p95 12, p99 24; move number p50 14, p75 22. Every earlier
child counted (`count == move - 1`) on 5.47 % of sites. **Seed:
`CutoffCountThreshold` 5** (p75), firing on 21.82 % of sites as a
counterfactual; depth 10 reads 6. `CutoffCountReduction` 1024 is (c), the
midpoint of 0 to 2048: one ply.

## Measurements (2026-09-25, the implementing agent, worktree `s238`)

All under `nice -n 19` beside S237's SPRT; node counts, best moves and
pass/fail only, no timing.

- Parent `1a35f16`, built in a throwaway worktree: `bench` **4845333**,
  replies c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6; `search_bench` depth 9
  18409 / 91713 / 27401 (g5f6 e2a6 d7c8q), depth 12 128457 / 632820 / 203399
  (c3d5 d5e6 d7c8q).
- **Off value, proved on the tree (DEC-215)**: the working tree's `src/` with
  `CutoffCountReduction` 0 prints `bench` **4845333** with the parent's eight
  replies, and `search_bench` reproduces the parent exactly at 9 and 12,
  counts and best moves (`.tuning/coord/S238_search_bench.log`). The census's
  instrumented and control binaries, from the same forced source, print
  4845333 too.
- Candidate at the seeds: `bench` **3727164**, -23.08 %, replies c3d5 **e2a6**
  d7c8q g7h8q **a7a6** a1b2 e5e6 e5e6; `search_bench` depth 9 47576 / 85179 /
  28316 (**c3d5** e2a6 d7c8q), depth 12 115640 / 524209 / 121935 (c3d5
  **e2a6** d7c8q).
- Tests in the guard suite over `cutoff_count_drive_t` -- a PV node at ply 1,
  depth 5, on FAIL_LOW_BETA's window, a stale 99 planted in the children's
  slot: "a node counts every child that failed high, from zero" (the count
  move k sees is k, every k, the stated number counted off the probe),
  "children failing high raise the reduction of the later quiets" (every
  replayable late quiet's reduction equals the replay with the term exactly
  where its count is over the threshold; at least one where the term moved
  the rounded, clamped reduction and one reduced quiet at or under the
  threshold that did not), "a node's own cutoff counts once in its parent's
  slot" (fail high +1, fail low 0, fail high as a verification 0), and, tune
  build only, "the cutoff count never moves a reduction at the off value".
  All four green in both builds. `tests/test_search_params.cpp` gains the two
  golden rows (66 -> 68); `MANUAL.md` the two options.
- `./clang-format.sh --check` clean at `CLANG_FORMAT_MAJOR=22`;
  `tools/plan_prose_check.py` `--citations` 0 flagged over 42 files,
  `--touches` 0 flagged, `--params` exit 0 with no output.

## THE FAST SUITE IS RED AT THE SEEDS -- the finding

Both builds, 38 of 40: `test_search` 161 of 166 cases and `test_mate_carry`.
Nothing was edited to pass (TESTS).

1. **"pruning does not hide a forced mate" loses the third capture-mate row**,
   `3N1bk1/3Q3p/6p1/p3Bp1n/1p6/3P1P1P/1q5K/8 w - - 0 33` at depth 10, #4
   (label R02). This is the recurring hazard CLAUDE.md names and what DEC-231
   recorded for S236: an upward move of the reduction losing a guarded mate.
2. **`test_mate_carry`**: 2 of 5 guarded cases report a mate line, 3 needed
   (silent: C_mate7_depth11, D_mate_minus6_depth10, E_mate_minus9). The
   case's own message names `adocs/data/S203_case_sweep.sh` as the remedy,
   not relaxing the assertion; not run by the agent.
3. **Four node-type term cases** ("... by LmrCutNode / LmrNotImproving /
   LmrTtCapture / LmrNoTtMove more") fail their precondition
   `reduction[k] < NODE_TYPE_DEPTH - 2`, 4 < 4 at the seeds. Their premise is
   "the other conditions are false in both drives, so the difference is this
   term alone"; a sixth term that fires in their drives breaks it. S095 met
   the same kind of premise and the cases took a condition for it ("the entry
   carries a move, so S095's term is off in both"); the analogous condition
   for this term is a test edit and is left to the coordinator.

**The sweep** (Release, threshold 5, one build per value,
`.tuning/coord/S238_sweep.log`):

| `CutoffCountReduction` | `bench` | forced mate case | node-type (4) | `test_mate_carry` |
|---|---|---|---|---|
| 0 (off) | 4845333 | green | green | green |
| 256 | 4442391 | red: "mate the multicut hides, depth 14" | 4 red | green |
| 512 | 4767336 | red: "mate the multicut hides, depth 14" | 4 red | green |
| 768 | 3793663 | red: capture mate row 3, depth 10 | 4 red | red (2 of 5) |
| 1024 (seed) | 3727164 | red: capture mate row 3, depth 10 | 4 red | red (2 of 5) |

**No non-zero adjustment tried keeps the suite green**, the shape DEC-231
recorded for S236's rounding. The multicut row is a mined row ("a property of
the tree it was mined on") and the capture rows carry DEC-209 clause 4's
depth rule, so re-deriving them is a procedure the tree has -- but the accepts
says the mate case "stays green", and whether a re-derivation is that or a
weakening is the owner's call, not the agent's. **No pair is pinned and no
SPRT can honestly start until it is decided.**

## Mutation

`tools/mutation_check.py` **refuses** this tree: its baseline must be green
over `-L fast` and it is not. The kills were taken by hand instead, the same
mutants file applied one at a time to a throwaway copy of the working tree at
the seeds (`.ref-builds/off`), `test_search` rebuilt and the three Release
cases run by name (`.tuning/coord/S238_mutkill.log`): baseline 3 of 3 green;
**7 of 7 killed** -- Q01, Q02, Q04, Q05 by "children failing high raise the
reduction of the later quiets"; Q03, Q06 by that case and "a node counts
every child that failed high, from zero"; Q07 by "a node's own cutoff counts
once in its parent's slot". Q04 and Q05 are the call-site class S237's fast
check found missing there: the count read from the grandchildren's slot, and
read once at the loop's start. The tool's run is owed once the suite is
green.

Not run by the agent, owed before completion (DEC-141): the tool's mutation
run, Debug self-play and `tools/gate_extra.sh` -- the coordinator's, with the
machine.

## Proposed `adocs/specs.md` sentences (the coordinator's to place)

> Cutoff count (S238): each node clears a per-ply count of its children's
> cutoffs just before its move loop, and every child whose own move loop fails
> high -- not a table cutoff, reverse futility, the null move, the multicut or
> S097's verification -- adds one. A late quiet the node reduces while the
> count is above `CutoffCountThreshold` gets `CutoffCountReduction` more, in
> ticks of 1024 to the ply, before the one rounding; the shallow-depth gates
> do not read it. At `CutoffCountReduction` 0 the engine is the tree before it.

## Phase 2 under DEC-233 (2026-09-25, a second Opus subagent)

DEC-233 is the contract: a mined golden moves with the tree and is re-derived
by its own script, the old row kept beside it; the node-type cases keep every
assertion and get their premise back. No assertion was relaxed, no seed moved,
no `src/` line changed. Everything CPU-bound ran under `nice -n 19` beside
S237's SPRT; every number below is a node count, a mate distance or a
pass/fail, none a timing.

### Red 1 -- "pruning does not hide a forced mate", capture-mate row 3

A mined golden: the GOLDEN block at `capture_mates` names
`adocs/data/S230_mine_r01_row.py depths` over `adocs/data/S230_table_fens.txt`,
depths 3 to 12, once shipped and once per mutant of
`tools/mutants/S091_capture_see.py`, and the rule "lowest depth of the shipped
profile at which some mutant loses the mate; if none separates, the lowest
depth, and the label says so". Run as written, seven sweeps on a copy of this
tree at the seeds (`.ref-builds/off`, `src/` byte-identical to the worktree's),
output in `.tuning/coord/S238_capmates/` and `S238_capsweep.log`:

| row | shipped | C02 | C05 | C06 | C07 | R01 | R02 | old row | new row |
|---|---|---|---|---|---|---|---|---|---|
| 1 `3krb1r/...` | d9-d12 | same | +d7 | same | same | same | same | 9, 5, none | 9, 5, none (unchanged) |
| 2 `2b5/4k2P/...` | d9-d12 | same | same | same | same | same | same | 9, 5, none | 9, 5, none (unchanged) |
| 3 `3N1bk1/...` | **d12** | d11-d12 | d12 | d12 | d10-d12 | **none** | d12 | 10, 4, R02 | **12, 4, R01** |
| 4 `1r3r1k/...` | d9-d12 | same | same | same | same | same | same | 10, 5, R02 | **9, 5, none** |

The script derives a separating row for the row that went red: the shipped
build reports #4 at depth 12 and R01 reports no mate at any depth. Confirmed
through the case itself, not only the sweep: with R01 applied to the same copy
the case fails `REQUIRE( result.mate_found )` on "capture mate, 3N1bk1/...,
depth 12, red under R01", and passes without it. R01's incidental kill is back
in the table for the first time since S222; R02's is gone (its direct guard
stands). No mate distance moved. The old rows are kept in the GOLDEN block for
a revert; `DEV_MANUAL.md`'s DEC-142 row says the same.

### Red 2 -- `test_mate_carry`, 2 of 5 guarded cases reporting

What it guards (DEC-162): a published line at its claimed length ends in mate;
short lines under a per-case ceiling; a majority of the guarded cases report a
mate line at all. The budgets in `adocs/data/S170_cases.tsv` are goldens with a
script -- `adocs/data/S203_case_sweep.sh`, DEC-156's rule, "run after anything
that moves the tree" -- and the failing assertion's own message sends a red
here and not to the assertion. So it is a re-derivation, not a stop.

The whole grid, 108 cells, one process per case (`adocs/data/S238_sweep.txt`,
per-case logs in `.tuning/coord/S238_mc/`). Every case still reports a mate
line somewhere -- stride-1 cells reporting A 8, B 9, C 4, D 1, E 7 of 9 -- so
the mates are not lost; the cells moved. **D is the thin one**: one stride-1
cell (1500000, 4 lines, 1 short) and none at stride 2, the shape DEC-162
recorded for C at 1/9.

The rule applied, and the reading taken (stated, not assumed silently): the
cheapest budget at the row's own stride that reports **a** mate line. DEC-162
deleted the floors the rule named and removed completeness from the choice
("a future re-sweep is choosing on the mate count alone"), so one line is the
floor. Applied to the three silent rows only, as S203 left the rows it did not
re-sweep ("re-deriving them would only churn the record"): C 1500000 ->
**1200000** (9 lines), D 4000000 -> **1500000** (4 lines, 1 short), E 1500000
-> **300000** (2 lines). `S203_case_sweep.sh --at` on the new TSV reproduces
the grid's cells exactly: A 7/0, B 9/0, C 9/0, D 4/1, E 2/0, F 0/0 -- 5 of 5
guarded cases reporting. The old budgets are in the TSV's S238 note.

The ceilings: `--ceilings` over the four tracked grids answers 5, 15, 0, 2, 11,
5, the case's current numbers, and over the five with `S238_sweep.txt` added
the same six; this grid's own worst cells are A 0, B 4, C 0, D 1, E 1, F 1.
The grid joins the command in the GOLDEN block and in `DEV_MANUAL.md`.

The case green alone under `nice -n 19` in 87 s wall against its 120 s
ceiling, while loaded; the suite runs below are the gate.

### Red 3 -- four node-type cases, clamp precondition

`LmrCutNode`, `LmrNotImproving`, `LmrTtCapture`, `LmrNoTtMove`. Every assertion
kept, `reduction[k] < NODE_TYPE_DEPTH - 2` included. Why the count fired: the
drives search inside the mate band, where every child's own move loop fails
high on its first move, so the count is k at move k and passes 5 before the
index the cases read. The premise restored the way S095's term is held off by a
planted entry: `node_type_drive_t::children_answer_from_table` plants, at every
child of the node, a lower-bound entry of 0 at `NODE_TYPE_DEPTH` -- above the
child's beta `-BAND_ALPHA`, deep enough for any child search the node starts --
so each child takes a table cutoff before its move loop, which the rule never
counts, and the node reads 0 back as a fail low, so nothing is re-searched or
re-stored. Each plant is read back after the node's own entry, so a collision
fails the drive instead of silently counting. Non-PV drives only (a PV child
never takes a table cutoff; the flag `REQUIRE`s it), which covers the four.

Counted, not assumed: `require_cutoff_count_off` asserts
`cutoff_count[i] == 0` at every move of every drive in all four cases, beside a
comment naming S238 next to S095's. **Red first**: with the flag left false the
four cases fail that assertion, `REQUIRE_EQ( 1, 0 )`; with it set all four are
green in both builds, their original assertions unchanged. T05 (`LmrPv`) was
green throughout and is not touched: its PV drive's first child is PV and can
not be answered this way, and its difference holds as it is.

### What changed, files

`tests/test_search.cpp` (the drive flag, the plants and their read-back,
`require_cutoff_count_off`, four cases opting in; capture-mate rows 3 and 4 and
their GOLDEN paragraph), `tests/test_mate_carry.cpp` (GOLDEN command and
paragraph; no ceiling moved), `adocs/data/S170_cases.tsv` (three budgets and
the note), `adocs/data/S238_sweep.txt` (new), `adocs/data/S238_sprt.sh`
(header and open findings; the H0 revert now names the goldens),
`DEV_MANUAL.md` (two DEC-142 rows, the bench entry), `adocs/data/README.md`.

### The gate on the phase-2 tree

- Both fast suites **40 of 40**, Release `build` and tune `build-tune`, run
  serially under `nice -n 19` (`test_mate_carry` 76.8 s against its 120 s
  ceiling), and `./clang-format.sh --check` clean at `CLANG_FORMAT_MAJOR=22`
  (`.tuning/coord/S238_suites_phase2.log`).
- `tools/plan_prose_check.py` `--citations` 0 flagged over 42 files,
  `--touches` 0 flagged, `--params` exit 0.
- `tools/mutation_check.py tools/mutants/S238_cutoff_count.py .ref-builds/mut
  --jobs 4` under `nice -n 19`, on a fixture that is this tree committed
  detached (`91c3873` on `1a35f16`, `tests/doctest` initialised):
  `worktree .../.ref-builds/mut at 91c3873 clean`, `baseline green, 40 tests,
  bench 3727164 nodes via engine`, **`mutation score 7 of 7 (100%) killed 7`**,
  every mutant's bench moved, 2311 s (`.tuning/coord/S238_mutation.log`). The
  list read was the worktree's own, untracked, as the header says. A first
  attempt was refused at the baseline because that shell lacked
  `CLANG_FORMAT_MAJOR` (`test_clang_format_script`), kept as
  `S238_mutation_refused_noenv.log`.
- **The off-value identity was not re-run**: no `src/` line changed in phase 2
  (`src/` is byte-identical to the tree phase 1 proved it on), and the
  candidate's `bench` still prints 3727164, phase 1's figure.
- `touches:` does not name `tests/test_mate_carry.cpp`, `DEV_MANUAL.md`,
  `MANUAL.md` or `tests/test_search_params.cpp`, which the step reached; the
  completion stamp should say so (S204's form).

### Fast-check fix-ups (2026-09-26, a third Opus subagent)

The Tier-1 fast check over phase 2 found five things; these and nothing else
were changed. `src/` did not move.

1. **Every row re-swept, not the red ones.** Phase 2 applied the rule to C, D
   and E, the rows that went red. DEC-156 states the rule once and applies it
   to every row, and choosing the rows by which went red is close to the
   per-case choice it refuses; DEC-162 keeps that re-sweep for the budgets.
   Applied from the existing grid `adocs/data/S238_sweep.txt`, no new sweep,
   same reading (the cheapest budget at the row's own stride reporting a mate
   line): A 1000000 -> **300000** (4 lines; 100000 reports 0), B unchanged at
   100000 (9 lines, its cheapest cell), C/D/E as phase 2 left them, F 300000
   -> **1000000** (stride 2: 100000, 300000, 500000 report 0, 1000000 reports
   4). `S170_cases.tsv`'s S238 note, `test_mate_carry.cpp`'s S238 paragraph,
   `DEV_MANUAL.md`'s golden row, the `S238_sweep.txt` row in
   `adocs/data/README.md` and `adocs/data/S238_sprt.sh`'s header and open
   finding 4 carry the six budgets; the note keeps all old values for the
   revert. `S203_case_sweep.sh --at`: A 4/0, B 9/0, C 9/0, D 4/1, E 2/0, F 4/0,
   every row equal to its grid cell, **5 of 5 guarded cases reporting**
   (`.tuning/coord/S238_fixup_at.log`). `--ceilings` over the five grids:
   5, 15, 0, 2, 11, 5, unchanged -- the rule reads the stride, not the budget.
   `test_mate_carry` alone under `nice -n 19`: passed, 53.3 s.
2. `test_mate_carry.cpp`'s paragraph on E's cell said 1500000 "is the cell
   this case drives"; E now drives 300000. The paragraph now names 1500000 as
   the cell E drove until S238, attributes the 22/11 reading to S095's tree,
   and says it does not carry: on S238's tree 1500000 reports 0 lines and
   300000 reports 2 with 0 short.
3. Mutant Q06's description said the count read is "the child just returned
   alone -- 0 or 1". The clear sits at `pick_next_move`, before the move's
   reduction is read and with no search between, so the read is always 0,
   near-equivalent to Q05. Description and the file's header only; the
   mutation is unchanged and was not re-run (no mutation run was allowed).
4. `DEV_MANUAL.md`'s golden row said the budgets come from the script "with
   no argument"; it now says the no-argument run prints the grid, the budgets
   are read from it by hand per row under the rule for all six rows, and
   `--at` confirms them.
5. This subsection.

Gate: both fast suites **40 of 40**, Release `build` and tune `build-tune`,
serially under `nice -n 19` (`test_mate_carry` 54.1 s and 55.3 s), and
`./clang-format.sh --check` clean at `CLANG_FORMAT_MAJOR=22`
(`.tuning/coord/S238_suites_fixup2.log`; the first run,
`S238_suites_fixup.log`, was 40/40 twice and red on format over a comment
line this pass had left unwrapped). `tools/plan_prose_check.py`
`--citations`, `--touches`, `--params` all exit 0, 0 flagged.
`git diff --stat HEAD -- src` is phase 1's: three files, 99 insertions, 2
deletions.

## Rebased onto 1680439 after S237's removal (2026-09-26, a fourth Opus subagent)

S237's SPRT read H0 and its hindsight reductions left the tree at `1680439`
(`src/` is `cd9d0e7`'s plus one comment). This step was built on `1a35f16`,
with that rule live, so it was ported onto the removal in the linked worktree
`s238b` from `.tuning/coord/S238_rebase_brief.md`; the old work stays
untouched in `../chesso-s238` as its record. Everything above this section is
the old tree's and is kept as written; every number below is the new tree's.
Nothing here is a timing: node counts, best moves, mate depths and pass/fail.

### What the port changed in the design

- **The code is the same rule, line for line.** The slot, the clear before
  the move loop, the move-loop increment that skips S097's verification, the
  per-move read at each late quiet into `lmr_adjusted_reduction`'s sum, the
  two rows, the probe fields `cutoff_count[]` and `node_adjustment`. The
  three-way apply conflicted only where S237's lines were the context: the
  probe's `hindsight`, `parent_reduction` and `handed_reduction[]` fields, the
  `probe->handed_reduction[k]` write beside the new `cutoff_count[k]` write,
  and the four `Hindsight*` rows above the two new ones. Those were dropped;
  nothing of S238 was among them.
- **What S237's removal takes out of the design**: the paragraph "S237's
  interaction, stated" above no longer applies. On `1a35f16` the reduction
  this rule adjusts was also handed to the reduced child as
  `parent_reduction`, so a child reached under the new term could meet
  `HindsightHeavyReduction`; that parameter and rule are gone, so the adjusted
  reduction is read by nothing but the reduced search itself. **Nothing in
  S238 depended on it**: no line of the rule, no case, no mutant anchor and no
  census patch reads S237's names (the census forces only
  `CutoffCountReduction`). One test comment cited "the hindsight drives'
  position" for `cutoff_count_drive_t`'s FEN and now says it is the position
  those drives used before the rule left.
- **The size of the change is not the same.** `bench` moves -1.69 % here
  against -23.08 % on `1a35f16`, with every reply unchanged: S237's rule was
  compounding this one there. That is an observation, not a reading of
  strength (DEC-019).
- `tests/test_search_params.cpp`: 62 -> **64** rows (on `1a35f16` it was 66 ->
  68); `DEV_MANUAL.md`'s `golden_defaults` row says 64.

### The identity, against the new parent (INV-6, DEC-215)

Parent: `.ref-builds/parent`, a detached worktree of this tree at **`1680439`**,
clean, built fresh. `bench` **4803214**, replies c3d5 d5e6 d7c8q g7h8q d8e7
a1b2 e5e6 e5e6. The working tree's `src/` with `CutoffCountReduction` 0 (a copy
under `.ref-builds/off`, the one row changed) prints **4803214** and the whole
`bench` stream -- every `info` line and all eight `bestmove` replies -- is
identical to the parent's with times and nps stripped; re-checked after the
last `src/` edit (a comment). `tools/search_bench.py`, identical node for node
and move for move: depth 9 48522 / 85714 / 28080 (c3d5 e2a6 d7c8q), depth 12
129499 / 411457 / 172984 (c3d5 e2a6 d7c8q) (`.tuning/coord/S238b_search_bench.log`).

Candidate at the seeds: `bench` **4722025**, -1.69 %, all eight replies the
parent's; `search_bench` depth 9 37598 / 84302 / 24050, depth 12 112043 /
409054 / 111754, every best move the parent's.

### The census, re-run on the new tree

`adocs/data/S238_census.py census`, the seed rule in its docstring unchanged,
2026-09-26, instrumented and control `bench` both 4803214. Output replaces
`adocs/data/S238_census.txt` (`.tuning/coord/S238b_census.log`):

| depth 12 | `1a35f16` (S237 live) | `1680439` + rule off |
|---|---|---|
| nodes | 2332561 | 2108720 |
| sites | 244621 (10.49 %) | 238386 (11.30 %) |
| count p25 / p50 / **p75** / p90 / p95 / p99 | 1 / 3 / **5** / 9 / 12 / 24 | 1 / 2 / **5** / 9 / 13 / 25 |
| `count == move - 1` | 5.47 % | 5.02 % |
| fires at the seed | 21.82 % | 21.81 % |
| depth 10 p75 | 6 | 6 |

**The seed does not move: `CutoffCountThreshold` 5.** The site comment in
`src/search_params.hpp` now quotes 21.81 % and says the census was taken again
on this tree. `CutoffCountReduction` stays (c), 1024.

### The goldens (DEC-142, DEC-233)

Started from the goldens as `1680439` has them -- phase 2's re-derivations
were made on S237's tree and were set aside, not carried. First fast suite on
the rebased candidate: Release 39 of 40, **one red**: "pruning does not hide a
forced mate", capture-mate row 3 at depth 10. `test_mate_carry` green at
`1680439`'s budgets; the four node-type cases green with the premise restored.

**Capture-mate rows**, by `adocs/data/S230_mine_r01_row.py depths --fens
adocs/data/S230_table_fens.txt --lo 3 --hi 12`, shipped plus each of the six
mutants of `tools/mutants/S091_capture_see.py` applied by hand
(`.tuning/coord/S238b_capmates.sh`), on a copy of the tree at the seeds, and
the same seven sweeps at `1680439` to tell the tree's moves from the rule's
(`.tuning/coord/S238b_capmates/`, `S238b_capmates_parent/`). The rule as the
GOLDEN block states it: the lowest depth of the shipped profile at which some
mutant loses the mate; if none, the lowest depth, and the label says so.

| row | shipped, parent | rule at parent (= old row) | shipped, candidate | new row |
|---|---|---|---|---|
| 1 `3krb1r/...` | d9-d12 | 9, 5, none | d7 d9-d12 | **7, 5, C02** |
| 2 `2b5/4k2P/...` | d9-d12 | 9, 5, none | d7 d9-d12 | **7, 5, C02, C05 and R02** |
| 3 `3N1bk1/...` | d10 d12 | 10, 4, R02 (and C05, unlabelled) | d11 d12 | **11, 4, C02 and R02** |
| 4 `1r3r1k/...` | d9-d12 | 10, 5, R02 | d9-d12 | **9, 5, R02** |

At the parent the rule returns the old depths, so all four moves are the
tree's. No mate distance moved; R01 separates no row on either tree. The old
rows are in the GOLDEN block for a revert. Through the case itself: green at
the new rows; under C02 it fails on "capture mate, 3krb1r/..., depth 7, red
under C02"; under C05 and R02 it fails first on "mate the multicut hides,
depth 14", a row earlier in the same case, so their capture-row separations
are the sweep's evidence (`.tuning/coord/S238b_capcase.log`).

**Node-type drives' premise.** Carried from phase 2 unchanged
(`children_answer_from_table`, `require_cutoff_count_off`). Red first on this
tree: with the four drives' flag left false, all four cases fail
`REQUIRE_EQ( drive.cutoff_count[i], 0 )` with `1, 0`; with it set, green in
both builds.

**`test_mate_carry` -- a stop, not re-derived.** The whole grid was taken on
the candidate and on the parent, 108 cells each, one process per case
(`.tuning/coord/S238b_sweep_cand.txt`, `S238b_sweep_parent.txt`). At the TSV's
budgets the candidate reports A 6/0, B 7/0, C 1/0, D 5/0, E 5/5, F 1/0, five of
five guarded cases, so the test is green. DEC-156's rule as phase 2 read it
(the cheapest budget at the row's stride reporting a mate line), applied to
every row: A 1000000, B 100000 and F 300000 unchanged; **C 1500000 -> 500000
(2/0), D 4000000 -> 1000000 (2/1), E 1500000 -> 1000000 (6/0)**. But the
`--ceilings` rule is over every recorded sweep, and adding this grid raises
**C's ceiling 0 -> 1** (stride 1, 4000000 nodes: 1 mate line, 1 short); the
other five are unmoved. The file says "a step that raises one is relaxing a
test and needs a decision", S109, S095 (DEC-225) and S236 each took one, and
DEC-233 covers re-derivation, not a raised ceiling. So the TSV, the ceilings
and `test_mate_carry.cpp` are left exactly as `1680439` has them and the grid
is not tracked; the coordinator decides. Two facts for that decision: at
`1680439` itself the same rule already moves A (its own cell reports 0 lines
there), D (-> 1200000) and E (-> 500000), and adding the parent's grid moves
no ceiling -- the TSV was not the rule's answer on the tree without S238
either.

### Measurements

- Both fast suites **40 of 40**, Release `build` and tune `build-tune`, and
  `./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22`
  (`.tuning/coord/S238b_suites.log`).
- `tools/mutation_check.py tools/mutants/S238_cutoff_count.py .ref-builds/mut`
  on a detached fixture commit of this tree, `035aba8` on `1680439`, doctest
  initialised; every anchor resolves once on the new tree, none edited:
  `worktree .../.ref-builds/mut at 035aba8 clean`, `baseline green, 40 tests,
  bench 4722025 nodes via engine`, **`mutation score 7 of 7 (100%) killed 7`**,
  every mutant's bench moved, 1112 s (`.tuning/coord/S238b_mutation.log`). The
  list read is the worktree's own, untracked, as the header says. The rule's
  own cases catch each one as on `1a35f16`; Q01 to Q06 also turn the
  re-derived capture-mate rows red.
- `tools/plan_prose_check.py`, one mode per invocation: `--citations` 0
  flagged over 42 files, `--touches` 0 flagged, `--params` exit 0 with no
  output (`.tuning/coord/S238b_prose.log`).

### Documents

`MANUAL.md` the two option rows (S237's four dropped from the merge);
`DEV_MANUAL.md` the bench-ledger entry `4803214` -> `4722025` with the off
value, the `capture_mates` golden row, `golden_defaults` 64;
`adocs/data/README.md` the census rows re-stated for the re-run, the
pre-registration's row, no `S238_sweep.txt` row (the grid is not tracked);
`adocs/data/S238_sprt.sh` re-headed for the rebase: REF is `1680439`, the tree
S237's verdict left, still `PIN_ME`; the open findings re-read (S237's cold
fast-check finding closed by its removal; this step's goldens and the budget
question added). `README.md`, `specs.md`, `plan.md`, `status.md`,
`decisions.md` not edited. The proposed `specs.md` sentence above stands as
written.

## The coordinator's landing notes (2026-09-26)

**`test_mate_carry`, the stop, ruled.** The budgets and ceilings stay as
`1680439` has them: the test is green on the candidate, so DEC-233's
re-derivation does not bind, and a short-line ceiling is never raised to
admit a grid. The candidate's C reports short mate lines in two cells the
test does not drive where the parent reports none; that is S202's open class
and is carried in the pre-registration's open finding 5, both grids kept as
`adocs/data/S238_carry_sweep_cand.txt` and `S238_carry_sweep_parent.txt`.

**Cold fast check over the rebased tree**: no defect in `src/`, no assertion
removed or loosened, `test_mate_carry` and `S170_cases.tsv` byte-identical to
`1680439`, anchors unique. Two items fixed by the coordinator: the
`adocs/data/README.md` row for `S238_sprt.sh` still quoted the first tree's
numbers (now 4803214 -> 4722025, REF `1680439`); and T05, "a principal
variation node reduces its late quiets by LmrPv less", did not count its
premise -- the PV drive cannot take the table-answer plant. A premise
assertion "neither side over the threshold" was written first and **failed**
(the ALL side reads 11), so the case now asserts what its difference needs:
the count is over the threshold on both sides or on neither
(`REQUIRE_EQ` of the two firing states), green.

**Second tier on the landing** (DEC-141): Debug self-play 8 games, 0
`Assertion`, 0 `disconnect` (`.tuning/coord/S238_selfplay/`);
`tools/gate_extra.sh` 5 stages green in 1117 s
(`.tuning/gate_extra_2026-09-26_S238.log`). Landed as `257c8fe` on `1680439`,
`bench` 4803214 -> 4722025. SPRT pair pinned: `REF` `1680439`, `CAND`
`257c8fe`; open findings re-read at pinning, nothing closed or added since
the rebase agent's block.

## The verdict (2026-09-27, the coordinator)

**H0, 2026-09-27 06:25: `257c8fe` against `1680439`, `Elo 0.25 +/- 2.96`,
`nElo 0.31 +/- 3.77`, LLR -2.95, 32574 games in 15 h 29 m, 0 forfeits**
(`adocs/data/S238_sprt.log`, `adocs/data/S238_sprt_pairs.txt`). The nElo
interval [-3.46, +4.08] reaches above zero, so the pre-registration reads it
as a zero, not a loss: `CutoffCountReduction` goes to 0, the proved off value,
and the code leaves with it -- the slot, the clear, the increment, the read,
the two rows, the cases, the mutants -- and the capture-mate rows DEC-233
re-derived on the candidate are restored byte for byte from `1680439`. No
follow-up run (DEC-063, DEC-019): the record's +6.00 was a direction and did
not transfer here. The census and both carry sweeps stay as evidence.

## The removal, 2026-09-27

Written by a fresh Opus agent on the linked worktree `s238rm` at `fccb7e0`,
briefed from `.tuning/coord/S238_removal_brief.md`, beside S240's rating
gauntlet, which held every core: everything CPU-bound ran under `nice -n 19`,
and every number below is a node count, a best move or a pass, none a timing.
The pre-registration's H0 row, written before the first game, is executed:
**`CutoffCountReduction` to 0, the proved off value, and the code leaves with
it.** No reason to keep any of it was found and none is stated. No second SPRT
is owed: the removal is behaviour-neutral against `1680439` and INV-6
discharges it.

### What left

| what | where |
|---|---|
| `cutoff_count_ticks`; the clear of the children's slot before the move loop and the `node_adjustment` probe write beside it; the move-loop increment at the fail-high; the read at each reduced late quiet; the `cutoff_count[]` probe write | `negamax_at` and above it, `src/search.cpp` |
| `search_state_t::cutoff_counts` and the probe fields `cutoff_count[]`, `node_adjustment` | `src/data_structures.hpp` |
| the two X-macro rows `CutoffCountThreshold`, `CutoffCountReduction` and their seed comments | `src/search_params.hpp` |
| `cutoff_count_drive_t` and this step's cases; the node-type drives' `children_answer_from_table` plant, its read-back and `require_cutoff_count_off`; T05's premise assertion | `tests/test_search.cpp` |
| the two golden rows and the count, 64 back to 62, with the header sentence | `tests/test_search_params.cpp` |
| the mutant list Q01 to Q07 | `tools/mutants/S238_cutoff_count.py`, deleted |
| the two option rows | `MANUAL.md` |

**The capture-mate rows DEC-233 re-derived are back to `1680439`'s byte for
byte** -- 9, 9, 10, 10 with `no S091 mutant, since S095` twice and `R02` twice
-- a revert from the GOLDEN block and not a re-mine. The landing had edited
existing lines of `tests/test_search.cpp` (those rows, their GOLDEN paragraph,
three comments in the node-type cases) and `tests/test_search_params.cpp`'s
header, so both were restored from `1680439` whole, as were
`src/data_structures.hpp`, `src/search.cpp` and `src/search_params.hpp`, and
the comment below was then added.

### What stayed

The evidence: `adocs/data/S238_census.py` and `.txt`, `S238_sprt.sh`,
`S238_sprt.log`, `S238_sprt_pairs.txt`, `S238_carry_sweep_cand.txt` and
`_parent.txt`, and their `adocs/data/README.md` rows. `DEV_MANUAL.md`'s bench
ledger **gains** the removal's entry, `4722025` -> `4803214`, beneath the
landing's. Its two golden rows are corrected to what is true: `capture_mates`
reads 9, 9, 10, 10 again and says S238's re-derivation came and went;
`golden_defaults` reads 62 and says S238's two rows came to 64 and left. This
step file is extended, not rewritten.

### The proofs

**The remaining difference to `1680439` in `src`, `tests` and `tools` is one
comment.** `git diff --stat 1680439 -- src tests tools`: `src/search.cpp | 4
++++`, a four-line comment at the head of `if (may_reduce)` in `negamax_at`,
where the read sat, saying the rule was tried there and what its SPRT read. No
code. (`257c8fe..fccb7e0` touched nothing under `src`, `tests` or `tools`.)

| | |
|---|---|
| `bench` | **4803214**, and the whole `bench` stream -- every `info` line's depth, score, nodes and PV and all eight `bestmove` replies, c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 -- identical to a Release binary built fresh from `1680439` in the throwaway worktree `.ref-builds/parent` (detached, clean, `git diff --stat 1680439 HEAD -- src tests` empty), compared with times and nps stripped; the only differing line is the final total's nps (`.tuning/coord/S238rm_bench_tree.txt`, `_parent.txt`) |
| `tools/search_bench.py` | identical node for node and move for move: depth 9 48522 / 85714 / 28080 (c3d5 e2a6 d7c8q), depth 12 129499 / 411457 / 172984 (c3d5 e2a6 d7c8q), on both binaries (`.tuning/coord/S238rm_search_bench.log`) |
| both fast suites | Release `build` and `-DCHESSO_TUNE=ON` `build-tune`: **39 of 40 each under ctest, the one red `test_mate_carry` as `Timeout` at its 120 s ceiling**, not an assertion; run alone through ctest it timed out again in both builds (load average 17 on 12 cores, the gauntlet). Run directly without the ceiling: **1 of 1 case, 37 of 37 assertions passed, both builds, 122 s and 124 s wall**. Its budgets are node counts, so the pass is the tree's and the ceiling the load's; `1680439` itself times out the same way (below). `./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22` (`.tuning/coord/S238rm_suites.log`, `S238rm_matecarry.log`, `S238rm_matecarry_direct.log`) |
| prose | `tools/plan_prose_check.py`, one mode per invocation: `--citations` 0 flagged over 42 files, `--touches` 0 flagged over 42 files, `--params` exit 0, no output (`.tuning/coord/S238rm_prose.log`) |
| the UCI surface | `MANUAL.md` has no `CutoffCount*` row and is `1680439`'s file byte for byte; `test_uci_surface` green in both builds with **no refresh** |
| the golden | `tests/test_search_params.cpp` is `1680439`'s file byte for byte; DEC-142's re-derivation for `golden_defaults` is its diff against `src/search_params.hpp` |

**Mutation: not obtained, owed.** `tools/mutation_check.py
tools/mutants/S234_tt_estimate_margins.py .ref-builds/mut --jobs 4` under
`nice -n 19` on a clean detached fixture at `1680439`, doctest initialised,
header `worktree .../.ref-builds/mut at 1680439 clean`, `list
.../tools/mutants/S234_tt_estimate_margins.py clean`, `mutants 4 of 4`, was
**refused at the baseline**: "the unmutated worktree is red: 1 of 40 failed
(test_mate_carry)", `Timeout 120.15 sec` in the baseline's ctest log
(`.tuning/coord/S238rm_mutation.log`). The fixture is `1680439` itself, so the
same timeout on the parent is what says the red is the load's. The ceiling was
not raised to get past it. It is owed to the coordinator once S240's gauntlet
frees the machine, same command, same fixture (left in place).

### Proposed commit text, for the coordinator

No DEC-220 result block: `95ea28d` closed the verdict and recorded it.

```
Remove S238's cutoff count on its H0

The cutoff count read a zero. Its gainer SPRT of 257c8fe against
1680439, {0, 5} nElo at 8+0.08, accepted H0 at 32574 games: nElo 0.31
+/- 3.77, an interval [-3.46, +4.08] reaching above zero, which
adocs/data/S238_sprt.sh reads as a zero and not a loss. That reading
and its consequence were written before the first game --
CutoffCountReduction goes to its off value, 0, and the code leaves with
it -- so this is the pre-registration executed.

Out: search_state_t::cutoff_counts, the clear before the move loop, the
move-loop increment, the read at each reduced late quiet and
cutoff_count_ticks; the two CutoffCount* X-macro rows; the probe fields;
cutoff_count_drive_t and the step's cases, the node-type drives'
table-answer plant with require_cutoff_count_off, and T05's premise
assertion; the two golden_defaults rows (64 back to 62);
tools/mutants/S238_cutoff_count.py; the two MANUAL option rows. The
capture-mate rows DEC-233 re-derived go back to 1680439's byte for byte.

In: a four-line comment at the site saying the rule was tried and what
it read; DEV_MANUAL's ledger gains the removal's entry and its two
golden rows read true again. The evidence stays under adocs/data/.

src, tests and tools are 1680439's apart from that comment. bench is
4803214 with the whole bench stream and all eight bestmove replies
identical to a fresh build of 1680439, and tools/search_bench.py is
identical at depths 9 and 12 (INV-6, DEC-215): no second SPRT is owed.
Format and the three prose checks clean; both fast suites green but
for test_mate_carry's wall-clock ceiling under the S240 gauntlet's
load, the case passing whole when run without it.

S238, DEC-063, DEC-019: the record's +6.00 was a direction and did not
transfer here.

Bench: 4803214
```

### Proposed `adocs/specs.md` edit, for the coordinator

One edit in the search row. The whole S238 sentence, from
"**The cutoff count, S238 (landed 2026-09-26, DEC-222, DEC-233)**:" through
"Decided by one `{0, 5}` nElo SPRT against the tree without it
(`adocs/data/S238_sprt.sh`): <verdict>." inclusive, becomes:

> **A cutoff count was tried and left, S238, 2026-09-25 to 2026-09-27
> (DEC-222, DEC-233)**: a late quiet was reduced one ply more once more than
> `CutoffCountThreshold` 5 of the node's children (the census p75,
> `adocs/data/S238_census.txt`) had failed high in their own move loops; its
> `{0, 5}` nElo SPRT against the tree without it (`adocs/data/S238_sprt.sh`)
> **accepted H0 at 32574 games on 2026-09-27 -- `Elo 0.25 +/- 2.96`, `nElo
> 0.31 +/- 3.77`, LLR -2.95, 0 forfeits** (`adocs/data/S238_sprt.log`); the
> nElo interval reaches above zero, so the pre-registration reads it as a zero
> and not a loss, and the adjustment went to its off value with the code
> behind it, the capture-mate rows re-derived under DEC-233 restored with it
> -- `bench` 4803214, `1680439`'s own total node for node.

### Owed to the coordinator, not run here

The mutation run above, and the second tier (DEC-141): the removal touches the
search, so Debug self-play and `tools/gate_extra.sh` are owed on the landing
tree; both load the machine the way a match does. A clean `ctest -L fast` pass
of `test_mate_carry` under its ceiling on an unloaded machine, both builds.

### Owed items, discharged by the coordinator on the idle machine (2026-09-27)

After S240's first gauntlet ended: `test_mate_carry` passed inside its 120 s
ceiling under ctest in both builds (55.10 s Release, 56.68 s tune); the
mutation proof over S234's list on the fixture at `1680439`: baseline green, 40
tests, `bench` 4803214, **mutation score 4 of 4 (100%), killed 4**, 644 s
(`.tuning/coord/S238rm_mutation_idle.log`). Cold fast check over the removal:
no defect; the specs sentence keeps the landing date it would have dropped.
