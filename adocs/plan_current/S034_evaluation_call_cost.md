id:         S034
goal:       compute the cheap evaluation terms first and skip the expensive ones when the score is already outside the window
accepts:    lazy evaluation, delivered with one expensive term behind it so there is something to skip, measured by `./fastchess.sh --fast` against the same hand-picked mobility weights that measured -14.93, so the comparison isolates the staging; full bounds if that is inconclusive
touches:    src/evaluation.cpp, src/evaluation.hpp, src/search.cpp, tests/
excludes:   fitting any weight -- that follows and is measured separately; the transposition-entry static eval and the quiescence cache, which this step's own measurement retired
decisions:  DEC-036, DEC-037, DEC-038, DEC-039
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

## What the step became, 2026-08-11

The owner chose to skip the work rather than remember it: DEC-039. The cache is
retired on its own measurement and this step is now lazy evaluation.

**The shape.** `evaluate()` splits in two. Stage one is what it computes today,
material and the tapered tables, straight off the accumulators at 1.36 ns. If
that score is far enough outside the caller's window that no plausible stage two
could bring it back, the function returns stage one and the expensive terms are
never computed. Otherwise stage two runs and the full score is returned.

That needs `evaluate()` to know the window, which it does not today: the
signature is `int evaluate(const board_t*)` and both call sites are in
`search.cpp`. The window has to be passed in, and the margin has to be a
constant that is bigger than stage two can plausibly be worth.

**The margin is the whole risk.** A margin that is too small returns a stage-one
score in a position where mobility would have changed the decision, which is a
wrong score rather than a slow one. The Chess Programming Wiki carries a warning
about exactly this and names an endgame case, K vs KBN. The mate tests in the
fast suite are the guard: pruning that hides a mate is this project's recurring
bug and both previous instances were caught there rather than by a benchmark.

**What it is measured with, and why that is clean.** Lazy evaluation alone has
nothing to skip, so it lands with mobility behind it as stage two, using the
same hand-picked weights that DEC-037 measured at -14.93 Elo. The only
difference between this candidate and that one is the staging, so the comparison
attributes cleanly:

| candidate | verdict |
|---|---|
| mobility, always computed | -14.93 +/- 16.44, DEC-037 |
| mobility, behind lazy evaluation | this step |

If the second is much better than the first, the staging works and the cost was
the problem. If it is the same, the cost was not the problem and the weights
are, which is the next experiment and not this one.

**Fitting the weights comes after and separately**, so that a term, its price
and its weights are never three unknowns in one number again. The tuner already
links the engine and the model stays linear -- mobility counts are a property of
the position, so they enter as features. DEC-037 has the sizing.

## How it is judged

Not behaviour-neutral: returning a stage-one score where the full score would
have differed changes what the engine plays, deliberately. So node counts will
move and INV-6's identical-nodes route does not apply. The verdict is
`./fastchess.sh --fast` against `HEAD`, full bounds if that is inconclusive,
read alongside the -14.93 above.

Before any of that: the fast suite green, all eleven mate tests included, and
the static-eval anchors recomputed rather than relaxed, the same way S028 did
it. A margin bug that hides a mate must fail a test, not a match.
