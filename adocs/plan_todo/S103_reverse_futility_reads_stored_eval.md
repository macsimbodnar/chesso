id:         S103
goal:       reverse futility reads the static evaluation already in the table entry instead of recomputing it
accepts:    identical node counts and identical best moves from tools/search_bench.py against the commit before it, so the change is behaviour-neutral and discharges INV-6 rather than owing an SPRT; if any node count moves, the claim is withdrawn and an SPRT is run instead; a measured speed-up stated as a ratio from one interleaved run rather than as an absolute timing, with the machine's own resolution reported beside it; a test that the value read from the entry equals what a fresh evaluate() returns for that position, observed red by planting a different value; INV-4 still holds -- the stored number is derived from the accumulators and nothing stops maintaining them; the fast suite green
touches:    src/search.cpp at the reverse futility site only
excludes:   quiescence, which S094 measured at zero and left alone; the improving flag, which is S092; correction history, which is S099; widening or re-laying-out the table entry, which S094 already did and proved neutral
decisions:  DEC-079
closes:
blocks:
paused_by:
done:

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
