id:         S085
goal:       the first SPSA run, over the twenty search parameters that exist today, and an independent SPRT of what it returns
accepts:    the run's parameter list, bounds, `c_end` per parameter, schedule constants, time control, opening book and game budget are written down before it starts and are not changed while it runs; the game budget is at least 30000 paired games or the run is not read at all; what it returns is rounded to the integers the shipping build uses and put through an SPRT of the **shipping** build against the commit before it, at a time control and an opening book the run did not use; the verdict is recorded whatever it is, including a rejection, and a rejected vector is kept in the step file rather than discarded
touches:    src/search.cpp, src/evaluation.hpp, adocs/plan_done/ on completion
excludes:   tuning the evaluation weights, which the Texel fit owns; adding parameters to the set, which is S073's; a second run, which is a new step if this one earns it
decisions:  DEC-019, DEC-041, DEC-048, DEC-050
closes:
blocks:
paused_by:
done:

## Why this exists

S084 builds the driver; this is the run. It is a separate step because the run is
where the machine time goes and because the two fail differently: a driver bug is
found by a synthetic objective in milliseconds, a run that was budgeted wrong is
found four hours in.

## The rule this step exists to obey

`adocs/eval_tuning_strategy.md` section 4.4: "SPSA output is a point estimate
from a noisy process. A meaningful fraction of SPSA runs produce parameter
vectors that do **not** pass a subsequent SPRT. Treat SPSA as a hypothesis
generator, not a result."

This project has the same rule from its own history, three times over: staged
move generation quoted at 30-50 Elo and measured 0, SEE pruning in quiescence
measured 0, capture ordering reported around 150 Elo and measured slower
(DEC-019). An SPSA vector is one more reported figure until an SPRT says
otherwise.

**The verification match must not reuse the tuning conditions.** Section 7: never
tune and test on the same opening set. The engine has one book,
`books/8moves_v3.pgn`, so this step either finds a second one or states plainly
that its verification shares the book and is weaker for it. The time control
should differ too.

## Budget, against what this machine is

Section 4.3: "30k to 200k games at fast time control (e.g. 5+0.05 or 10+0.1) per
run. Below ~30k games the result is indistinguishable from noise."

S033's SPRT was 1012 games in 44 m 10 s at 12 threads and 10+0.2. Thirty thousand
games at that rate is roughly 22 hours; at 5+0.05 it is a few hours, and that is
one run before the verification match. This is the most expensive item in the
plan after S083 and the two compete for the same nights.

Section 4.3 also caps the width: 10 to 30 parameters per run. S073's set is ten,
which fits in one run whole.

## What a rejection means

Not that SPSA does not work. The likely readings are that the budget was too
small, that `c_end` was set below what the parameter can express, or that the
tune build's timing differs from the shipping build's enough to matter -- the
cost S073 accepted on purpose. All three are recoverable and all three are worth
more written down than a second run started immediately.

## Cost

A night for the run, three to four and a half hours for the verification SPRT.


## Moved to the front, 2026-08-19

The old plan put both SPSA steps last, on the argument that they tune what the
steps above them add. That is right for S127 and wrong for this run: **the
twenty parameters that exist today are demonstrably mis-set**, and the tuner is
the cheapest way to find out which.

The evidence, measured on the tune build at depth 12: `MaxQsearchDepth` is 8
and the bound binds. At 16 the Ruy Lopez position after `e4 e5 Nf3 Nc6 Bb5 a6`
drops from 1038972 nodes to 940880 -- 9.4 % fewer -- and its score moves from
20 to 33 on a different line; kiwipete goes the other way, 5167100 to 6061763;
an endgame position is unchanged at both. 16 and 32 are identical, so the
natural depth is under 16. One untuned parameter, position-dependent, changing
both the tree and the score. `ASPIRATION_DELTA` at 50 is the next candidate --
the engines surveyed run 10 to 25 and widen.

## Technical details (SOTA research, 2026-08-19)

Algorithm, driver design, hyperparameters and their sources live in S084's
Technical details section and are not repeated; this section is the run.

### 1. State of the art: how a run is sized and judged

**Run budget.** Kiiski's original Stockfish method (CPW "Stockfish's Tuning
Method"): "we used 30000-100000 super-fast games, before reading final result",
over **7 to 35 variables at once**, apply_factor 0.002. Our strategy doc
(par.4.3): 30k-200k games at fast TC, 10-30 parameters per run, "below ~30k
games the result is indistinguishable from noise" -- the accepts' floor.
fishtest freezes the budget up front: "SPSA hyperparams are based off the
initial number of games", so a run is never extended past its planned K -- the
schedules `a_k`, `c_k` are defined by it. Fishtest RFC #535 set one iteration
to one pair in principle, batched only for worker mechanics.

**Tuning TC.** fishtest wiki: short TC "to get faster approximations or for
checking the tune's correctness", normal TC "usually preferred" for search
parameters, 60+0.6 "to get better scaling values"; tuning at 20+0.2 uses
Hash=16. RFC #535 compared one tune at 20+0.2 against local reruns at 2+0.02,
1+0.01 and 0.5+0.01 and the "results are similar" -- SPSA endpoints for
search-shape parameters are fairly TC-insensitive, which is what licenses
tuning cheaper than verifying. The exception is time management (par.5).

**The verification norm.** SPSA output is a point estimate from a noisy walk
(strategy par.4.4: "a meaningful fraction of SPSA runs produce parameter
vectors that do not pass a subsequent SPRT"). Kiiski says why: the method
"doesn't converge and it needs to be stopped at 'suitable moment'" -- important
variables approach their optima while the rest "take 'a random walk'", and run
long enough "the strength starts to decrease". Stockfish practice ships a tune
only through an ordinary SPRT patch: PR #5491 is the shape -- an SPSA tune,
then STC `<0.00,2.00>` over 61376 games, then LTC `<0.50,2.50>` over 100662.
Published near-zero and negative verifications exist: Triumviratus's devlog
records an SPSA bake whose measured effect kept "decaying toward zero", and an
SPSA-tuned time manager at **+23.8 Elo at 20+0.2 that measured -22.9 at
10+0.1** -- the tuned-at-one-TC hazard as a number. DEC-019 in someone else's
ledger.

**Stuck and diverged, judged from the trajectory.** Stuck: "if after a few
thousand games the values are barely changing at all, the tuning run is
useless and should be stopped", usually c too small (fishtest wiki). Diverged:
oversized c/r "the SPSA diverges" (RFC #535) -- values pinned at a bound for
the whole run mean c or the bound is mis-set. Rarely-exercised parameters
"can have a hard time moving at all" and their endpoint is noise, not signal.
Inspection is read-only: the accepts freeze the config, so a sick run is
stopped and recorded, never re-tuned mid-flight.

### 2. Shape for chesso

**The set.** The live surface is **22**, not the goal's twenty
(`src/search_params.hpp:41-195`; full list with bounds in S084 par.2): 13
search-shape parameters and the 9 `Tm*` time-management parameters S089 added.
None of the 22 is one of the 827 fitted evaluation weights; the one boundary
case is `LazyEvalMargin`, consumed inside `evaluation.cpp` (:984, :1038-1039)
but owned by S039, not the Texel fit. Decisions to freeze at run time, with
recommendations:

- **Exclude the 9 `Tm*` parameters (recommended).** They are the published
  TC-overfit family (the +23.8/-22.9 record above); this run tunes at a TC the
  verification deliberately does not share, which is the worst possible regime
  for them; and they are the only parameters in the set with a fresh SPRT
  behind their current values (S089, H1 accepted 2026-08-18). S127 retunes
  everything at the S105 control after the search block. That leaves **13**.
- **`OrderHistoryMax` is the weakest of the 13**: it binds only when history
  saturates (`search.cpp:752`), so it is the "rarely used, hardly moves"
  case; its upper bound 899999 is itself the ordering-band guard (a killer
  scores 900000). Keep or drop at run time; if kept, expect a random walk.
- **`LazyEvalMargin` stays in**: mis-set costs Elo now, and S039 re-decides it
  later with spread data -- note there that the incumbent may have moved.
- Tuning all 22 in one run is the documented alternative (Kiiski co-tuned up
  to 35); its cost is TM noise in the vector and a muddier rejection analysis.

**Ranges and c_end.** Run bounds = the declared bounds unless the frozen
config narrows them with a reason. **Do not derive c_end as range/20 from the
declared bounds** -- they are legality bounds, deliberately loose (`RfpMargin`
[0,2000]/20 = 100 cp, an absurd perturbation). Use "±c costs a few Elo":
seeds (each **seed -- re-decided here**): centipawn margins (`RfpMargin`,
`AspirationDelta`, `LazyEvalMargin`, `AspirationMaxDelta`) c_end 4-8; depth/ply
ints (`MaxQsearchDepth`, `RfpMaxDepth`, `RfpMinPly`, `NullMoveBase`,
`NullMoveDivisor`, `AspirationMinDepth`) c_end 0.5-1 (int floor 0.5); LMR
hundredths (`LmrBase`, `LmrDivisor`) c_end 5-10; `OrderHistoryMax` c_end
~30000 if kept. `r_end` 0.002, alpha 0.602, gamma 0.101, A = 0.1 x iterations
-- S084's seeds, unchanged.

**TC, book, budget, machine.** Tune at **5+0.05** (seed; strategy par.4.3's
example) with `Hash=16 Threads=1` pinned both sides and `fastchess.sh`'s
adjudication frozen verbatim in the config. Book: **not** the S105
verification book. Prefer a second UHO-class file from the CC0
`official-stockfish/books` repo via S105's pinned-fetch machinery (unbalanced
signal for the tune, disjoint from `UHO_Lichess_4852_v1.epd`); fallback is
`books/8moves_v3.pgn`, stated with its cost -- a balanced book's higher draw
rate thins the pair-score gradient. Budget: **30000 games** (accepts floor) =
15000 pairs = **2500 iterations at 6 pairs each** (one 12-thread wave,
DEC-050), A = 250. Throughput arithmetic: S033 measured 1375 games/h at
10+0.2 on 12 threads; DEC-083 prices 8+0.08 at about half a 10+0.2 game, so
~2700/h, and 5+0.05 at 5/8 of that cost, ~4300/h -- **30k games ≈ 7 h, one
night**. 60k is a night and a day; start at 30k and let the trajectory argue
for a bigger second run (a new step per excludes). Detached overnight
(DEC-041), `SPSA-DONE`/`SPSA-FAILED` marker last, watcher via
`python3 bin/moltke.py --watch <log> 'SPSA-(DONE|FAILED)' --ceiling 14h --pid <pid>`
(ceiling 2x the budget, AGENTS.md par.12, DEC-061).

### 3. Implementation sketch

Pre-run, in order: (a) S084 gates green -- synthetic convergence, sign-flip,
clamp, integer-floor, resume; (b) S084's dry-run against `build-tune`, no
`Rejected` in the engine log; (c) **bounds-extremes smoke**: for each tuned
parameter one `setoption` at min and at max accepted over UCI, plus one short
game pair at the nastiest corners (`MaxQsearchDepth=64`,
`AspirationMaxDelta=48000`; `TmHardPercent=1000` only if TM is in) -- no
crash, no illegal move, forfeits counted from the PGN; (d) machine idle check
(`ps aux | sort -rnk3 | head`, CLAUDE.md rule 4); (e) the frozen config --
list, bounds, c_end per parameter, schedule constants, TC, book, budget --
written into this step file **before** the first game (accepts).
Then the run, detached, watcher armed. Post-run: forfeit count from the PGN;
trajectory copied to `adocs/data/`; floats rounded to ints at the UCI
boundary; if the rounded vector equals the incumbent, record a stuck run and
owe no SPRT -- there is no change to measure. Otherwise edit the defaults in
`src/search_params.hpp` (the X-macro rows -- the touches line predates S073;
`search.cpp`/`evaluation.hpp` only consume them), rebuild the **shipping**
build, one SPRT against the commit before the edit, verdict recorded whatever
it is; on rejection the vector and the trajectory stay in this file and the
default edit is reverted, with the reading (budget, c_end, tune-build timing)
written down per "What a rejection means".

### 4. Constants and seeds

All marked **seed -- re-decided here**: tune TC 5+0.05 and budget 30k games
(strategy par.4.3; CPW Kiiski "30000-100000 super-fast games"); 6 pairs per
iteration (OpenBench default 8, reshaped to the 12-thread wave); c_end
classes above (OpenBench "1/20th the reasonable range" + fishtest "a few
Elo per ±c", int floor 0.5); tuning Hash=16 (fishtest tunes 20+0.2 at
Hash=16; DEC-088's pressure logic); alpha/gamma/A-ratio/r_end per S084 par.4
with URLs. Fixed, not seeds: verification regime 8+0.08 Hash=16 UHO gainer
`elo0=0 elo1=5` (S105, DEC-063, DEC-088).

### 5. Pitfalls

- **Tuning at one TC, playing at another** -- real and published (+23.8 →
  -22.9 above), concentrated in the TM family; excluding TM is the mitigation
  here. For search-shape parameters RFC #535 measured endpoints similar from
  20+0.2 down to 0.5+0.01.
- **Stuck run** (c too small) and **divergence** (c/r oversized, values pinned
  at bounds) -- read the trajectory at ~25 % and ~50 %, stop-and-record, never
  adjust mid-run. **Run too long** is its own failure: Kiiski's random-walk
  decay -- the budget is the stopping rule, chosen up front.
- **Oversized c crashing or forfeiting games**: this engine *refuses* an
  out-of-range `setoption` (`chesso.cpp:1043-1071`) and plays the default
  silently -- the driver clamps theta and theta±c (S084); the extremes smoke
  test exists so the first illegal value is found in minutes, not at 3 am.
- **Tuning to the book or the draw rate**: adjudication identical both sides
  and frozen; the gradient only sees decisive pairs, so a balanced book thins
  signal (Pohl's ~91 % draw figure is top-engine; at 2559 it is milder).
- **MaxQsearchDepth=8 binds** -- verified at `src/search.cpp:287`
  (`qply >= MAX_QSEARCH_DEPTH` returns stand-pat); the measurement in "Moved
  to the front" above. Expect this axis to move; its c_end must let it
  (>= 0.5, arguably 1).
- **Move-ordering bands**: the band constants themselves are excluded from
  the exposed set by design (`search_params.hpp:29-32`); the one in-set
  band-adjacent value is `OrderHistoryMax`, whose declared max 899999 is the
  guard -- run bounds must never widen past declared bounds.
- **SPSA tunes around bugs**: Triumviratus found margins that had been
  SPSA-tuned around a broken scale and "absorbed most of the error" -- fixing
  the bug then measured ~0. S106's correctness sweep sits before this run in
  plan order for exactly that reason; do not reorder.
- **Multi-hour discipline**: detached, self-terminating watcher with ceiling,
  no bare `tail -f` (AGENTS.md par.12, DEC-061); resume from checkpoint, and
  a resume continues to the planned K, never past it.

### 6. Measurement

This step owes: the frozen run config and per-iteration `trajectory.tsv`
recorded, TSV copied to `adocs/data/`; forfeit count from the PGN; then **one
independent SPRT** -- shipping build with the rounded vector vs the commit
before it, at 8+0.08, Hash=16, the S105 UHO book, `elo0=0 elo1=5
alpha=0.05 beta=0.05`, 12 concurrency (typical 45-75 min at these settings
per plan.md). Verdict recorded whatever it is; a rejected vector is kept in
this file. **Expected verdict count: 1** (0 if the run is stuck and the
rounded vector is the incumbent -- recorded as such, per CLAUDE.md rule 8).

### 7. Interactions

- **S084** supplies the driver, checkpointing, marker and watcher contract;
  this file freezes what S084 leaves as config (list, bounds, c_end, TC,
  book, budget). Any disagreement is resolved in S084's favour on mechanism
  and this file's favour on run choices.
- **S105 must land first** (it does, plan order 7 vs 12): it supplies the
  verification regime and the UHO book, and its book-fetch machinery is how
  a second tuning book arrives. **S106 must land first too** -- tuning around
  a latent bound-sign bug bakes it in (pitfall above).
- **S100 is independent**: verified, none of the 22 is an evaluation weight;
  the 827 live in the tuner's domain. `LazyEvalMargin` is the boundary and
  belongs to S039, which should expect a moved incumbent.
- **Parameters added by S093-S132 are not in this run**; S127 owns the full
  post-search-block retune, at the S105 control, where the TM family gets
  its proper shot.

### 8. References

- https://www.chessprogramming.org/Stockfish%27s_Tuning_Method -- Kiiski 2011:
  30k-100k games, 7-35 variables, apply_factor 0.002, random-walk decay,
  "stopped at suitable moment".
- https://www.chessprogramming.org/SPSA -- pair-match objective in {-2..+2},
  alpha/gamma recommendation.
- https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html
  and https://github.com/official-stockfish/fishtest/wiki/Creating-my-first-test
  -- SPSA fields, TC guidance (STC/normal/60+0.6), Hash=16 at 20+0.2, frozen
  initial games, stuck-run wording, rarely-used-values warning.
- https://github.com/AndyGrant/OpenBench/wiki/SPSA-Tuning-Workloads -- pairs-per
  8, c_end range/20 + few-Elo rule, int C >= 0.5, r_end 0.0020, A-ratio 0.1.
- https://github.com/official-stockfish/fishtest/wiki/Fishtest-mathematics --
  normalised update, wins-losses gradient signal.
- https://github.com/official-stockfish/fishtest/issues/535 -- TC-insensitivity
  reruns (20+0.2 vs 2+0.02/1+0.01/0.5+0.01 "results are similar"), divergence
  on oversized c/r, one-pair iterations. Prose only.
- https://github.com/official-stockfish/Stockfish/pull/5491 -- a tune shipped
  through STC (61376 games, <0.00,2.00>) then LTC (100662, <0.50,2.50>).
- https://talkchess.com/forum3/viewtopic.php?t=63632 -- SPSA failure modes in
  the wild: synthetic-function verification advice, hyperparameter traps.
- Triumviratus `archive/DEVELOPMENT_6.0.md` (github.com/Tors3/Triumviratus,
  devlog markdown read as prose, no source opened): 50-param co-tune verified
  +15.8 ±10.9 at 10+0.1 over 1058 games; TMv2 +23.8 at 20+0.2 / -22.9 at
  10+0.1; an SPSA bake decaying toward zero; margins tuned around a bug.
- https://tests.stockfishchess.org/tests/view/5612a3af0ebc597e4f23e593 --
  a live fishtest SPSA record; **unreachable** (bot check), not used.

### Scope concern

The goal line says "the twenty search parameters that exist today"; the live
surface is **22** (S089 added nine `Tm*` entries after the goal was written;
S084 par.2 lists all 22 with bounds). And this section recommends tuning
**13** of them, excluding the TM family on the published TC-overfit record --
the accepts accommodate that (the frozen list is a run-time artifact) but the
goal's letter reads as all of them; the owner decides at freeze time. Also:
`touches` names `src/search.cpp` and `src/evaluation.hpp`, but since S073 the
shipping defaults live in `src/search_params.hpp` -- the post-run edit lands
there.
author:    Maksym Bodnar

## The frozen run configuration, 2026-08-20 19:45

`tools/spsa_s085.json`, committed before the first game and not edited while the
run is up (accepts). Every number below is measured here, not inherited: the
step's own seeds were re-decided against the simulator and against this machine,
and where a seed lost, the measurement that beat it is recorded beside it.

**Parameter set: 12.** The owner's decision at freeze time, from the three
options this file's research section left open. The 9 `Tm*` parameters are out
(the published TC-overfit family, and this run deliberately tunes at a TC the
verification does not share; they also have a fresh SPRT behind them from S089,
and S127 retunes them at the S105 control). `OrderHistoryMax` is out: it binds
only when history saturates, so it random-walks, and a meaningless endpoint
would land in the shipping vector the SPRT judges.

| parameter | start | min | max | `c_end` |
|---|---|---|---|---|
| `MaxQsearchDepth` | 8 | 1 | 64 | 4 |
| `RfpMargin` | 75 | 0 | 2000 | 24 |
| `RfpMaxDepth` | 6 | 0 | 63 | 4 |
| `RfpMinPly` | 3 | 0 | 63 | 1 |
| `NullMoveBase` | 2 | 0 | 16 | 1 |
| `NullMoveDivisor` | 6 | 1 | 64 | 4 |
| `LmrBase` | 75 | 0 | 400 | 24 |
| `LmrDivisor` | 225 | 1 | 2000 | 32 |
| `LazyEvalMargin` | 150 | 0 | 2000 | 24 |
| `AspirationMinDepth` | 5 | 2 | 64 | 2 |
| `AspirationDelta` | 50 | 1 | 2000 | 16 |
| `AspirationMaxDelta` | 400 | 1 | 48000 | 32 |

Bounds are the declared bounds from `src/search_params.hpp`, unnarrowed.

**Schedule.** `alpha` 0.602, `gamma` 0.101, `a_ratio` 0.1 (S084's seeds, kept),
`r_end` **0.004**, seed 85.

**Shape.** 1250 iterations x 24 pairs = 30000 pairs = **60000 games**, twice the
accepts' floor. `tc=2+0.02`, `Hash=16`, `Threads=1`, concurrency 12 (DEC-050),
book `books/UHO_4060_v3.epd`, `fastchess.sh`'s adjudication verbatim. 30000
rounds against 242201 openings, so the book never wraps.

### What the simulator said, and what it overturned

S084 left the driver verified on a 5-axis objective 225 Elo deep and said
explicitly that its constants were not inheritable. Re-measuring them at this
run's shape overturned three of the four numbers this file had seeded.

**1. The budget floor is a resolution floor, and it is higher than the plan
assumed.** 12 axes, each optimum `4*c_end` off, depth swept, 5 seeds, mean gain
of what was available (`scratchpad/depth_grid.py`, 2500 x 6 pairs = 30000 games):

| objective depth | `r_end` 0.002 | 0.008 | 0.032 | 0.128 |
|---|---|---|---|---|
| 6 Elo | -0.04 | -0.44 | -8.62 | -56.88 |
| 15 Elo | +0.32 | **+2.34** | -6.97 | -45.18 |
| 36 Elo | +5.91 | **+17.43** | +14.53 | -24.91 |
| 96 Elo | +37.92 | **+80.20** | +80.14 | +37.47 |
| 240 Elo | +179.55 | **+230.79** | +224.29 | +190.87 |

30000 games resolves an objective about 36 Elo deep and up. At 15 Elo it
recovers a sixth and goes backwards on an unlucky seed; at 6 Elo it does
nothing at any step size, and the signal run and a zero-weight flat run drift
by the same amount -- a walk with no gradient in it. The bottom row reproduces
S084's own regime and its choice of `r_end`, which is the check that the
simulator is the same one.

**2. `c_end` was the largest lever, and the step's seeds were sized for the
wrong thing.** The seeds (4-8 cp for margins, 0.5-1 for depths) are the smallest
change that could matter -- endpoint precision. At a 15 Elo objective they get
+2.34 of 15. Four times larger gets **+10.54**, and at 4x the cost of sitting
`+/-c` off the optimum is 1.25 Elo, which is fishtest's own "`+/-c` should cost a
few Elo" rule arrived at from the other end.

That result is width-dependent -- the best multiplier tracks a distance to the
optimum nobody knows -- but the loss is asymmetric, and that is what decides it
(`scratchpad/width_grid.py`, mean gain of 15.0):

| width (in seed `c_end`) | 1x | 2x | 4x | 8x | 16x |
|---|---|---|---|---|---|
| 1 | +6.61 | +7.08 | +6.33 | **-38.97** | **-250.27** |
| 2 | +6.64 | +9.54 | +9.81 | +2.10 | -51.44 |
| 4 | +2.34 | +7.10 | +10.54 | +10.99 | -2.35 |
| 8 | +0.21 | +2.87 | +8.21 | +11.31 | +10.13 |
| 16 | -0.57 | +1.68 | +5.04 | +10.04 | +11.65 |

A `c` too small costs a couple of Elo. A `c` too large costs tens, and the
parameters most likely to be at width 1 are the ones already fitted or already
swept -- `LmrBase` and `LmrDivisor` from the reduction table, `AspirationMaxDelta`
which S021 measured flat from 100 to 2000. **4x is the only multiplier positive
at every width**, and within about 2 Elo of the best where this project has any
evidence about the width at all: `MaxQsearchDepth`'s optimum is between 8 and 16
by the measurement in "Moved to the front" above, and `AspirationDelta` at 50
against a surveyed 10-25, both of which are width 4 to 8.

Two of the twelve are capped below 4x, for a cliff a quadratic objective cannot
express. `RfpMinPly` 3 +/- 2 reaches ply 1, where S033 measured the mate cases in
`test_search` going red -- a perturbation that makes the engine hide mates is not
a worse engine, it is a broken one. `NullMoveBase` 2 +/- 2 spans its entire useful
range in one perturbation. The capped list costs about 0.8 Elo at wide widths
and buys 0.9 at width 1 against a blanket 4x (`scratchpad/verify_frozen.py`), so
the caps are close to free.

**3. The time control was arithmetic and the arithmetic was wrong by 2.4x.**
This file priced 5+0.05 at ~4300 games/h by chaining two ratios off S033, giving
30000 games in about 7 h. Measured: a 12-game wave at 5+0.05 takes 24.4 s, so
2500 iterations is **17.0 h**, not 7. The cause is that a game's duration is set
by its clock and not by how fast the engine is -- 12 games on 12 SMT threads each
still spend `(base + moves*inc)*2` seconds -- so a ratio taken from a
games-per-hour figure at another control does not transfer. Fixed cost per
`fastchess` invocation is only 1.4 s, and the 16 MB book costs 0.05 s of it, so
neither is worth attacking.

Measured on this machine, 12-game waves, `Hash=16 Threads=1` concurrency 12:

| tc | s per wave | median depth | p10 depth | 2500 iterations |
|---|---|---|---|---|
| 1+0.01 | 6.9 | 9 | 8 | 4.8 h |
| 2+0.02 | 10.9 | 11 | 9 | 7.6 h |
| 3+0.03 | 11.9 | 11 | 9 | 8.3 h |
| 5+0.05 | 24.4 | 12 | 10 | 17.0 h |

**2+0.02** is the choice: it reaches median depth 11 against 5+0.05's 12, so
`RfpMaxDepth` 6 and `AspirationMinDepth` 5 are exercised either way, and RFC #535
measured search-shape endpoints similar from 20+0.2 down to 0.5+0.01. 0 forfeits
in 120 games at 2+0.02 and 120 at 3+0.03 (`tools/forfeit_report.py`). 1+0.01
was refused rather than measured further: median depth 9 starts to compress the
depth-gated parameters this run exists to move.

**4. The batch size is decided at fixed wall clock, not fixed games, and that
inverts the answer.** At a fixed game count the simulator prefers small batches
-- 2 or 3 pairs beat 6 (+3.43 against +2.34) -- which is textbook SPSA. But the
driver runs one `fastchess` per iteration and cannot finish until the slowest of
its games does, so a small batch pays that straggler every iteration. Measured
at 2+0.02, three reps each:

| pairs | s/iteration | s/pair |
|---|---|---|
| 3 | 7.60 | 2.53 |
| 6 | 9.14 | 1.52 |
| 12 | 15.14 | 1.26 |
| 24 | 23.47 | 0.98 |
| 48 | 40.97 | 0.85 |

More than half of a 6-pair iteration is waiting. Priced in 8 h of machine rather
than in games (`scratchpad/shape_at_wallclock.py`, mean over widths 2, 4 and 8):

| pairs | iterations | games | `r_end` 0.002 | 0.004 | 0.008 |
|---|---|---|---|---|---|
| 3 | 3789 | 22734 | -- | +9.48 | +9.11 |
| 6 | 3150 | 37800 | -- | +10.32 | +9.45 |
| 12 | 1902 | 45648 | +9.82 | +10.64 | +9.70 |
| 24 | 1227 | 58896 | +11.07 | **+11.61** | +10.52 |
| 48 | 702 | 67392 | +10.77 | +11.52 | +9.57 |

24 pairs at `r_end` 0.004 is the optimum and 48 is tied with half the
iterations, so the turnover is found rather than assumed. **1250 x 24 = 60000
games in 8.15 h** -- one night, and twice the accepts' floor for the same
machine time the old 30000-game plan at 5+0.05 would have spent twice over.

`r_end` 0.004 rather than S084's 0.008 for the reason S084 wrote down: the step
sums `(wins - losses)` over the iteration's pairs, so displacement tracks total
pairs, and this run has 30000 of them against the 20000 that calibrated 0.008.

### Pre-run gates, all green

- S084's synthetic gate: `ctest --test-dir build -L fast` 18/18, `test_spsa_driver` 3.19 s.
- `spsa_driver.py check --engine build-tune/src/chesso tools/spsa_s085.json`:
  12 parameters accepted, no warnings, node probe `RfpMargin` 75 -> 164302
  nodes, 2000 -> 742729, so `setoption` reaches the search.
- **Bounds extremes.** All 24 corners -- every parameter at its min and at its
  max -- sent over UCI to `build-tune`: **0 `info string refused` lines**, which
  is what S137 made observable. Then `MaxQsearchDepth=64`,
  `AspirationMaxDelta=48000`, `RfpMinPly=0` together, `go depth 10`: a legal
  `bestmove`, empty stderr, exit 0.
- **A game pair at the nastiest corner.** All 12 parameters at whichever bound
  is worse, against the defaults, 6 games at 5+0.05: 0-5-1, no crash, and
  **0 forfeits of 6** by `tools/forfeit_report.py`. The corner engine loses,
  which is the point -- it does not fall over.
- Machine idle (`load average: 0.55`, nothing above 4 % CPU), governor
  `performance`.
- Second book fetched and pinned: `books/UHO_4060_v3.epd`, 242201 openings,
  zip sha256 `62fe32cd...`, unpacked `419844f8...`, verified end to end through
  `books/fetch_book.sh` -- so the tuning book and the verification book are
  disjoint, which `adocs/eval_tuning_strategy.md` par.7 requires.

### Where the time-control arithmetic came from, and where else it is

The 2.4x error above was this file's own research section chaining two ratios
off S033's 1375 games/h at 10+0.2 -- 8+0.08 at "about half a 10+0.2 game", then
5+0.05 at "5/8 of that". The first link is DEC-083's, and **S105 already
measured it wrong once**: DEC-083 predicted x3 and S105 measured **x1.67**,
which is the parked item in `status.md`. So this is the second instance of one
failure, not two failures.

What generalises is the cause, and it is worth stating once: **a game's duration
is set by its clock, not by how fast the engine searches.** Twelve games on
twelve SMT threads each still spend `(base + moves*inc)*2` seconds, so a
games-per-hour figure measured at one control cannot be scaled to another by any
ratio -- it has to be measured. The wave times in the table above took four
minutes to measure and would have cost nine hours to assume.

Checked, so it is not left as a suspicion: `grep` over `adocs/plan_todo/` finds
no other step carrying a chained throughput ratio. The one throughput figure
there is S135's, and it uses S105's *measured* 23.1 to 38.7 games a minute at
8+0.08, which is sound. S127 inherits this run's shape rather than an estimate,
and S128 sets its own control by definition.

## Mid-run read at 39 %, 2026-08-20 22:39

The pitfalls above ask for the trajectory to be read at about 25 % and 50 % and
for a sick run to be stopped and recorded rather than adjusted. Read at 491 of
1250 iterations, 11784 pairs, 3 h 16 m elapsed -- **23.96 s per iteration against
the 23.47 measured, so the 8.15 h estimate holds** and the run is due at about
03:43.

**Not stuck and not diverged.** `c_scale` has decayed 2.055 -> 1.099 as designed.
`y` over 491 iterations: mean -0.15, sd 7.22, range -33 to +24, first fifty
-2.14 and last fifty +1.52 -- centred, with real spread, and no sign of the
"barely changing" trajectory the fishtest wiki calls useless. Every one of the
twelve axes has moved.

| parameter | start | at 39 % | move / `c_end` | % of iterations at a bound |
|---|---|---|---|---|
| `MaxQsearchDepth` | 8 | 17 | +2.25 | 0.0 |
| `RfpMargin` | 75 | 61 | -0.58 | 0.0 |
| `RfpMaxDepth` | 6 | 14 | +2.00 | 0.0 |
| `RfpMinPly` | 3 | **0** | -3.00 | **30.8** |
| `NullMoveBase` | 2 | 3 | +1.00 | 0.0 |
| `NullMoveDivisor` | 6 | 7 | +0.25 | 0.0 |
| `LmrBase` | 75 | 68 | -0.29 | 0.0 |
| `LmrDivisor` | 225 | 199 | -0.81 | 0.0 |
| `LazyEvalMargin` | 150 | 170 | +0.83 | 0.0 |
| `AspirationMinDepth` | 5 | 2 | -1.50 | 23.6 |
| `AspirationDelta` | 50 | 24 | -1.62 | 0.0 |
| `AspirationMaxDelta` | 400 | 454 | +1.69 | 0.0 |

Two axes want reading rather than just recording. `MaxQsearchDepth` at 17
agrees with this file's own pre-run measurement that the natural depth is above
8 and under 16 -- and since 16 and 32 measured identical there, anything at or
above 16 is the same engine, so 17 is that finding and not a contradiction of
it. `AspirationDelta` at 24 has walked into the 10-25 band the surveyed engines
use, which is an independent arrival at a published range from a noisy process
that was told nothing about it.

### The `RfpMinPly` problem, found by the run

**`RfpMinPly` has walked to 0 and spends 30.8 % of its iterations at that
bound.** `src/search_params.hpp` says the root, ply 1 and ply 2 are exempt from
reverse futility pruning because "a mate two moves away lives exactly that far
down", and records as measured fact that "at ply 1 the mate cases in test_search
go red" (S033). A value below 3 is therefore a value the test suite rejects, and
a step cannot complete on a red suite.

So the declared minimum of 0 contradicts the sentence the same file writes two
lines above it, and contradicts the list's own claim that every bound "is either
arithmetic ... or the constant's own stated purpose ..., never a guess". For
`RfpMinPly` the stated purpose implies a floor of 3; the declared bound is 0.
The cost is already paid: about a third of one axis's gradient budget has been
spent exploring values that cannot ship.

`AspirationMinDepth` sitting at its bound 23.6 % of the time is **not** the same
problem -- its declared floor of 2 is arithmetic ("depth 1 has no previous score
to build a band around"), so 2 is admissible and the tuner simply wants
aspiration windows from the shallowest depth that can have them.

**Not acted on mid-run.** The accepts freeze the configuration and the pitfalls
say a sick run is recorded, never re-tuned in flight, so nothing was touched.
What this changes is the post-run handling: the safe floor for `RfpMinPly` is
being measured separately against `build-tune` over UCI while the run continues,
and if the rounded vector lands below it, the reading is recorded here and the
axis is not shipped at an inadmissible value. Whether that means rejecting the
vector, shipping it with that one axis held at its safe floor -- which is then a
vector nobody measured -- or narrowing the declared bound and owing a rerun, is
a decision and not a judgement call, so it is written up for the owner rather
than taken here.

## What the `RfpMinPly` floor actually is, measured 2026-08-20 23:10

Measured against `build-tune/src/chesso` over UCI and, independently, against
`build-tune/tests/test_search` with `RFP_MIN_PLY` set through `gdb` at `main` --
two mechanisms because `go depth N` is not what the tests do (UCI deepens 1..N
with a warm table and aspiration from depth 5; the tests call `search(depth)`
once from a cold one). Both agree. No build, no match, no timing: the SPSA run
kept the machine throughout.

| mate case in `tests/test_search.cpp` | `RfpMinPly` 0 | 1 | 2 | 3 |
|---|---|---|---|---|
| `mate in two is found at the right distance` :125 | FAIL | FAIL | pass | pass |
| `pruning does not hide a forced mate` :1887 | FAIL | FAIL | pass | pass |
| `pruning does not hide a mate against the material leader` :1926 | FAIL | FAIL | pass | pass |
| the other 15 mate cases | pass | pass | pass | pass |
| `search results are well formed everywhere` (123769 assertions) | pass | pass | pass | pass |

**The tested floor is 2.** Three findings come with it, and two of them correct
`src/search_params.hpp`, which is why that file was edited in the same commit.

1. **0 and 1 are the same engine.** The guard is `!is_pv && ... ply >=
   RFP_MIN_PLY` (`src/search.cpp:521`) and the root is entered at :853 with ply
   0 and `is_pv` true, so `!is_pv` exempts the root at every setting and this
   parameter never sees ply 0. Byte-identical node counts confirm it: TRICKY
   329568, CMK 260802, KILLER 53310 at depth 8, both values. So the axis had a
   duplicate value in it, and the comment's "the root is exempt because its
   answer is the one that gets played" credited the wrong guard.
2. **"the mate cases go red" was 3 of 18.** All three assert `mate_found` and a
   mate distance, none asserts a move, and each fails on its black or
   material-leader arm at depth 3. The material-leader position -- the one S033
   wrote *for* this rule -- is the worst: no mate at any depth 3 through 6 at
   `RfpMinPly` 0 or 1, and a different move played.
3. **The ply-2 exemption has no test behind it.** RFP fires at ply 2 when the
   setting is 2 -- TRICKY 329598 against 375687 at 3, so the tree really does
   change -- and everything still passes. `src/search.cpp:517` already concedes
   the gap: "A mate deeper than ply 3 can still be missed for an iteration, and
   no test covers that."

**Not fixed, because it is a decision.** The declared minimum stays 0. Narrowing
it to 2 is measurement-backed and narrowing it to 3 matches the stated purpose
while resting on an argument no test exercises -- and either way it is the
owner's call, so it is banked rather than taken. What is settled is that 0 and 1
cannot ship, which is what the rounded vector will be held against.

`RFP_MAX_DEPTH` has the same shape and is softer: its comment says the bound
"keeps the assumption to the last few plies", and a declared max of 63 permits
the assumption at every depth in the tree. No red test and no number for "few",
so it is recorded and not acted on. The tuner has walked this axis 6 -> 14.

Checked and not findings: the other 20 rows hold, including three that needed
arithmetic rather than reading -- `TmStabilityMax` 126 is `MAX_DEPTH`,
`TmFallingPercent` 400 makes the scaled soft limit meet `TmHardPercent` exactly,
`OrderHistoryMax` 899999 sits under the killer's 900000.
