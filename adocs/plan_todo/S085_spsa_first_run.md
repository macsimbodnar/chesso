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
