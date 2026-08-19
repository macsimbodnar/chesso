id:         S103
goal:       reverse futility reads the static evaluation already in the table entry instead of recomputing it
accepts:    identical node counts and identical best moves from tools/search_bench.py against the commit before it, so the change is behaviour-neutral and discharges INV-6 rather than owing an SPRT; if any node count moves, the claim is withdrawn and an SPRT is run instead; a measured speed-up stated as a ratio from one interleaved run rather than as an absolute timing, with the machine's own resolution reported beside it; a test that the value read from the entry equals what a fresh evaluate() returns for that position, observed red by planting a different value; INV-4 still holds -- the stored number is derived from the accumulators and nothing stops maintaining them; the fast suite green
touches:    src/search.cpp at the reverse futility site only, src/search.hpp for a test-only declaration of negamax (amended 2026-08-19 by DEC-080: the field said src/search.cpp alone while `accepts:` demanded a test, and no call to search() can place one on a non-PV node past RFP_MIN_PLY -- negamax already had external linkage, so the declaration changes no emitted code), tests/test_search.cpp
excludes:   quiescence, which S094 measured at zero and left alone; the improving flag, which is S092; correction history, which is S099; widening or re-laying-out the table entry, which S094 already did and proved neutral
decisions:  DEC-079, DEC-080
closes:
blocks:
paused_by:
done:      BEHAVIOUR-NEUTRAL, INV-6 discharged on node counts against 45ec005: 164123 / 670488 / 84351 at depth 9 and 1162576 / 5167100 / 683367 at depth 12, best moves c3d5 / e2a6 / d7c8q at both, identical to the node. No SPRT owed. Speed-up +2.47 %, 95 % CI +1.35 to +3.59 %, ratio new/base 0.9759 geometric mean over one interleaved run of 24 alternating pairs, paired t -4.36; the effect is smaller than a single comparison on this machine resolves (bench_movegen 0.1 % resolution but 2.6 % spread, bench_eval 3.0 %) and the pairing is what resolves it. Hit rate counted, not argued: 3368027 of 14589403 calls at the site, 23.09 %, over 300 positions at depth 10 with a warm table, and 0 disagreements with a fresh evaluate(); 173440 of 1617617, 10.72 %, 0 disagreements over the three search_bench positions at depth 12. Test observed red at the parent commit, REQUIRE_EQ( 488, 288 ), ten assertions passing before it. Suites 16/16 fast in build, build-debug (INV-4 asserted) and build-tune; clang-format clean. Two stale doc claims found by the completion check and fixed in the same commit: DEV_MANUAL.md's tune-build RfpMargin sweep was S068-era and is re-measured (164123 / 223454 / 476911 / 743308), and plan.md's retention paragraph named moltke 0.11.0 where 0.12.0 is installed.

## Why this exists, and why it is not part of S094

S094 measured its own three commits at zero and found the value somewhere it was
not allowed to touch. Instrumented over a depth-12 kiwipete search, the reverse
futility site made **1074051 `evaluate()` calls**, of which **105612 -- 9.8 % --
already had the value in the table entry**, and **0 of them disagreed** with a
fresh call.

That is the opposite profile to the one S094 measured. Its quiescence probe
found an entry on 0.79 % of nodes and the stored value differed from a fresh
lazy evaluation on 0.05 % of them; this site hits twelve times as often and has
never once disagreed.

S094's `touches:` is `src/search.cpp` quiescence, and one change goes in one
commit, so it was left for here.

## What it should buy, and why no SPRT

`evaluate()` costs **83.35 ns a call on the DEC-049 machine** -- measured
2026-08-19, `bench_eval`, 4000000 calls a sweep over 7 sweeps, spread 2.2 %.
Skipping 9.8 % of 1074051 calls is a speed-up and nothing else: the value read
is the value that would have been computed, on every one of the 105612 samples.

So this is an INV-6 change, not an SPRT change. **The node counts decide it.** If
they move at all, something reads a value that is not what a fresh call returns,
the neutrality claim is false, and the step owes a verdict like any other.

## Hazard

The one way this goes wrong is a stale or foreign value: an entry whose `eval`
was stored under a different position that collides on the slot, or stored as a
bound rather than a score. S094 already has a test that a quiescence entry
carries the static score and never a bound, observed red at
`REQUIRE_EQ( 417, -32768 )` where 417 was `cheap - LAZY_EVAL_MARGIN`. The same
property has to hold at this site, and the 0-disagreement measurement above is
the evidence that it currently does -- over one search, which is not a proof.
author:    Maksym Bodnar

## What was measured, 2026-08-19

**Neutrality, INV-6.** `tools/search_bench.py` against `45ec005`, the commit
before this one, both depths and both binaries built by the DEC-049 `g++ 13.3`:

| depth | midgame | kiwipete | tactical | best moves |
|---|---|---|---|---|
| 9  | 164123  | 670488  | 84351  | `c3d5` / `e2a6` / `d7c8q` |
| 12 | 1162576 | 5167100 | 683367 | `c3d5` / `e2a6` / `d7c8q` |

Identical before and after, to the node. No SPRT is owed.

**The hit rate and the disagreement count, instrumented rather than argued.** A
throwaway build counted the calls at the site and compared the stored number
against a fresh `evaluate()` on every hit:

- 300 positions from `adocs/data/S018_raw.tsv` at depth 10, one engine process,
  table left warm across positions as it is in a game: **3368027 of 14589403
  calls, 23.09 %, and 0 disagreements.**
- the three `search_bench.py` positions at depth 12, which that script also
  runs through one process and one table: **173440 of 1617617, 10.72 %, and 0
  disagreements.**

The 9.8 % in DEC-079 was one position on its own; the rate is a property of how
much has already been written to the table, and it is higher in the regime a
game plays in than in either of these. 3.37 million samples and not one
disagreement is what the neutrality claim rests on beyond the node counts.

**The speed-up, one interleaved run.** 24 pairs, base and candidate alternating
order within the pair so a first-or-second bias cancels, each run the 300
positions at depth 10:

    ratio new/base   median 0.9733   geometric mean 0.9759
                     95 % CI 0.9653 .. 0.9867   t = -4.36 over 24 pairs
    speed-up         +2.47 %   95 % CI +1.35 .. +3.59 %

A 12-pair run taken first, before `bench_movegen` was run, read 0.9778 median.

**The machine's resolution beside it, and why the number is believable at this
size.** `bench_movegen` on the same session: **resolution 0.1 %**, spread 2.6 %
over the run. `bench_eval`: **86.41 ns a call, resolution 3.0 %, spread 8.6 %**
-- against the 83.35 ns and 2.2 % this file recorded on 2026-08-19, and the
difference is the machine, not the engine: the governor was `powersave` and a
browser was on it. The per-pair spread here is 2.7 %, larger than the effect.
**It is the pairing that resolves it, not the timer**: 21 of 24 pairs came out
below 1.0 and the paired t is -4.36. A single before-and-after comparison at
this size would have been inside CLAUDE.md rule 5's 3 % and would have been
noise, correctly.

**Cost accounting, for whether +2.47 % is the right order.** 3368027 calls
skipped at 86.41 ns is 291 ms; base median for that run is 11.55 s. 2.52 % --
the same number the clock measured, arrived at from the call count and the
per-call cost independently.

## The test

`tests/test_search.cpp`, `reverse futility prunes on the stored static score`.
It drives one `negamax` node directly, at `RFP_MIN_PLY`, non-PV, depth 1, with
a window the site fails high through, on `4k3/8/8/8/8/8/8/3RK3 w - - 0 1` --
S094's anchor, `evaluate()` 563.

Two preconditions before the assertion, so it is non-vacuous in both
directions: with a cold table the node returns `563 - RFP_MARGIN`, which is the
bound reverse futility argues for and not a score any search of that position
returns; and the same node as a **PV** node, the one place the site may not
fire, returns something else.

Then an entry for the position carrying `563 - 200`, stored at `TT_DEPTH_QS` so
the score in it cannot answer a depth 1 node, and the node must return
`363 - RFP_MARGIN`.

**Observed red at the parent commit**, with the test in place and
`src/search.cpp` at `45ec005`: `REQUIRE_EQ( 488, 288 )` at
`tests/test_search.cpp:1009`, ten assertions passing before it -- so both
preconditions held on the old code and the one failure is the read.

## Suites

`build` (Release) 16/16 fast, `build-debug` 16/16 fast -- INV-4 is asserted
there on every make and unmake and is green -- `build-tune` 16/16 fast,
`./clang-format.sh --check` clean.
