id:         S034
goal:       stop paying for evaluate() at every node: static eval in the table entry, and a quiescence eval cache
accepts:    the feasibility measurement first, and it is allowed to end the step; then, for whatever survives it, one change at a time, each shown behaviour-neutral by identical node counts and best moves (INV-6) and each measured by SPRT for the speed it buys
touches:    src/search.cpp, src/transposition_table.hpp, src/transposition_table.cpp, tests/
excludes:   adding any evaluation term -- that is S027; lazy evaluation, which is a different answer to the same problem and gets its own step if this one fails
decisions:  DEC-036, DEC-037, DEC-038
closes:
blocks:
paused_by:
done:

## Why this exists, and why it is ahead of S027

S027's first term measured -14.93 Elo while paying a third of the nodes per
second (DEC-037). That verdict bundled two causes, unfitted weights and the
cost, and this step attacks the cost -- for every S027 term at once rather than
for mobility alone.

`evaluate()` is the first statement of every quiescence node, `search.cpp:123`,
before `is_check` and before any generation. A node that stands pat returns at
`search.cpp:135` having done nothing else. Quiescence never probes or stores the
transposition table at all. So the evaluation is computed at the highest-traffic
point in the search and its result is thrown away every time.

Both parts of this step are on the parked list in `status.md` and neither had a
decision behind it until DEC-038.

## The measurement that comes first, and may end the step

**The literature's premise does not obviously hold here.** The Chess Programming
Wiki and Arasan's separate evaluation cache both rest on "it is usually
expensive to call the evaluation function". Chesso's `evaluate()` is **1.36 ns**,
measured by `bench_eval` at resolution 0.1 %. A probe into a table large enough
to be useful is a memory access, and a memory access that misses cache is
plausibly slower than 1.36 ns of arithmetic on values already in registers.

If that is true, this step is a pessimisation today and its value is entirely
conditional on an expensive term existing -- which is circular, because the
expensive term was rejected for being expensive.

So the first task is not to build the cache. It is to measure what a probe
costs against what the evaluation costs, on this machine:

- the cost of a probe into a table of the size the transposition table actually
  uses, at a realistic hit rate and a realistic access pattern
- the same for a small direct-mapped eval cache sized to stay in L2
- both against the 1.36 ns baseline, and against the 15.93 ns an expensive term
  costs, since the second is the case the cache is really for

A result showing the probe costs more than the evaluation ends the step with a
recorded number, and the answer becomes lazy evaluation, which skips the
expensive work rather than remembering it. That outcome is a success for this
step, not a failure: it costs an hour and saves building the wrong thing.

## What the measurement said, 2026-08-11

**The concern above was wrong, and something worse is true.**

`sizeof(tt_entry_t)` is 24 bytes with 20 used, so a 16-bit static evaluation
fits in existing tail padding. No entry growth, no change in entry count, and
the behaviour risk this file warned about does not exist.

Probe costs, best of seven sweeps, independent random accesses:

| | ns |
|---|---|
| index loop only | 0.08 |
| access into the 524288-entry table the engine allocates at Hash=16 | 1.08 |
| access into a 256 KB direct-mapped cache | 0.36 |
| `evaluate()` today | 1.36 |

So a probe is **cheaper** than the evaluation, not dearer. The literature's
premise survives in the sense that mattered.

**But the saving is below the noise floor.** A search node costs about 116 ns.
Replacing a 1.36 ns evaluation with a 0.36 ns cache hit saves 1 ns, which is
under 1 % of a node, against a 3 % noise floor. There is nothing here an SPRT
could see. The cache is not a pessimisation; it simply has nothing to buy while
the evaluation is this cheap.

**And `bench_eval` understates an occupancy term by about four times.** The
kiwipete position searched nearly the same number of nodes in both builds --
9095066 against 8860613 -- so the per-node cost is comparable: 115.7 ns against
172.1 ns, a difference of **56.4 ns per node**. `bench_eval` priced the same
term at +14.6 ns per call. The gap is the working set: `bench_eval` evaluates
ten positions in a loop and keeps a small slice of the magic tables hot, while a
real search walks 2 MB of rook attacks and 256 KB of bishop attacks at random.
An occupancy-based term pays cache misses that the isolated benchmark never
sees, and that is a limitation of the instrument, recorded here rather than
discovered again later.

That also revises DEC-036 and DEC-037 downward in mobility's favour on cost and
upward on the difficulty: the term was not costing 14.6 ns, it was costing 56.

**Where this leaves the step.** The cache cannot pay for itself against a 1.36 ns
evaluation, and against an expensive one the cheaper answer is not to remember
the work but to skip it -- lazy evaluation costs no memory traffic at all, where
a cache costs 0.36 ns and a miss path. Part 1 keeps a reason that is not speed:
S033's reverse futility pruning and an `improving` flag both want a static score
they did not pay for, and it is free in bytes.

## If it survives, the two changes, separately

**1. Static evaluation in the transposition entry.** Full-depth nodes already
probe the table, so the read is close to free -- the line is in cache because
the entry was just read for the move and the bound. This is also the
prerequisite for the `improving` flag and for S033's reverse futility pruning,
both of which want a static score they did not pay for. Watch the entry size:
the table is sized in entries rounded down to a power of two, so a wider entry
means fewer of them for the same megabytes, and that is a behaviour change
hiding inside a speed change.

**2. A quiescence eval cache.** Quiescence does not probe the table today.
Arasan keeps a separate evaluation cache rather than putting q-nodes in the
transposition table, because there are far more q-nodes than useful entries and
they would evict the ones the main search needs. If this part is built at all it
is built that way, and separately from part 1, because two caches at once make
neither number attributable.

## How each part is judged

Caching is behaviour-neutral by construction: a cache that returns anything
other than what `evaluate()` would have returned is a bug, not a feature. So
each part proves neutrality with identical node counts and identical best moves
from `tools/search_bench.py` (INV-6) before any timing is believed, and then the
speed it buys is measured by SPRT like anything else. A part that measures zero
is recorded as zero and reverted unless there is a stated reason to keep it --
part 1 has one, that S033 needs it.
