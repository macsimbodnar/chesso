id:         S084
goal:       an SPSA driver over the exposed search parameters, verified against an objective whose optimum is known
accepts:    a tracked tool implements the Spall iteration -- `a_k = a/(k+1+A)^alpha`, `c_k = c/(k+1)^gamma`, Rademacher perturbation, two objective evaluations per iteration -- reads a parameter file naming each parameter, its bounds and its `c_end`, and drives paired games through `fastchess` against the tune build S073 produces; a test runs the driver against a synthetic noisy quadratic with a known optimum and asserts it converges to within a stated tolerance, and fails if the sign of the update is flipped; the driver checkpoints to disk every iteration so an interrupted run resumes rather than restarts; it prints a terminal marker as its last line, so a watcher exits on the run rather than on a turn boundary (DEC-061); nothing under `src/` changes and no verdict is owed
touches:    tools/, tests/, DEV_MANUAL.md
excludes:   running it on the engine, which is S085's; choosing which parameters to tune and their ranges, which S085 decides and records; any change to `src/`; CLOP, CMA-ES and every other black-box optimiser
decisions:  DEC-019, DEC-093
closes:
blocks:
paused_by:
done:      tools/spsa_driver.py with a 22-assertion synthetic gate at 3.5 s in -L fast (18/18 green, format clean). Convergence 27.8 of 300 initial axis error and -0.86 Elo; the sign-flipped run 799 and -1594 Elo. r_end 0.002 measured 293 of 300, so 0.008 is used -- the seed constant is not inheritable (DEC-084). Four mutations of the driver checked, the fourth exposed a clamp test that only covered the sent integers and it now asserts the float theta. Dry run: 3 parameters, 2x2 pairs at 1+0.01, 8 games parsed, killed and resumed byte-identical, 0 forfeits; node probe 164123 -> 743308 proves setoption reaches the search. No src change, no SPRT owed. Two findings became S137: the tune build cannot report a refused setoption (LOG_W is compiled out of Release) and uci gives no readback, so MANUAL.md and DEV_MANUAL.md were both wrong and are corrected.

## Why this exists

`adocs/eval_tuning_strategy.md` section 4: search parameters -- LMR
coefficients, futility margins, null-move reduction formulas, aspiration window
widths, history bonus scaling -- "have no differentiable objective. The only
measurable objective is win rate, which is extremely noisy. This is where
black-box optimization belongs, and nowhere else."

The engine's current method for one such parameter is a step, a source edit, a
rebuild and an SPRT per candidate: that is S068 for `RFP_MARGIN` and S039 for
`LAZY_EVAL_MARGIN`, two steps and two verdicts for two numbers. S073's parameter
set has ten.

Section 4.2 is why SPSA and not a coordinate method: "SPSA needs exactly two
objective evaluations per iteration regardless of dimensionality. With 40
parameters, a coordinate method needs 80 evaluations per gradient estimate; SPSA
needs 2. When each evaluation costs thousands of games, this dominates every
other consideration."

## What the driver has to get right

- **Paired games.** Same opening, both colours, theta+ against theta- directly.
  Section 4.3: "Variance reduction here is worth more than any tuning of `a` and
  `c`."
- **`c_end` per parameter**, set to the smallest change in that parameter that
  could plausibly matter, and `r_end = a_end / c_end^2` for the final step size.
- **Clamping to legal ranges** every iteration. `RFP_MIN_PLY` below 1 or
  `MAX_QSEARCH_DEPTH` at 0 is not a worse engine, it is a different one.
- **The sign.** `theta = theta + a_k * g` because win rate is maximised. A sign
  error produces a run that looks like a run and walks downhill, which is the
  single easiest way to spend a night and learn nothing -- hence the synthetic
  test in the gate.

## Why the gate is a synthetic objective and not a game

A driver tested by playing games cannot distinguish a bug in itself from noise
in the objective, at any budget this machine can afford. A noisy quadratic with a
known optimum costs milliseconds, isolates the iteration from the engine
entirely, and fails loudly on the sign, the decay schedules and the clamp. The
games come in S085, where they are the point.

## Cost

No match. Development and a test that runs in the fast suite.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

**Spall's algorithm** (1992; the 1998 implementation paper is the cookbook).
Minimise a loss `L(theta)` observable only as `y(theta) = L(theta) + noise`.
Per iteration k: draw `Delta_k` with each component Rademacher +/-1 —
**uniform and normal perturbations are invalid** (the estimator divides by
`Delta_i`, and those have infinite inverse moments); evaluate `y(theta + c_k
Delta_k)` and `y(theta - c_k Delta_k)`; estimate `ghat_i = (y+ - y-) / (2 c_k
Delta_i)`; update `theta <- theta - a_k ghat` (minimising; win rate is
maximised, hence `+` here — the step file's sign warning). Gains `a_k = a /
(k+1+A)^alpha`, `c_k = c / (k+1)^gamma`. Spall 1998: `alpha = 0.602`, `gamma =
0.101` are "the lowest allowable satisfying the theoretical conditions" and
beat the asymptotically optimal 1.0 and 1/6 in finite samples; in high noise
pick a smaller `a` and larger `c`; set `c` about equal to the standard
deviation of the measurement noise at `theta_0`; set `A` to 10 % or less of the
expected iteration count; pick `a` so the first steps move `theta` by the
smallest change worth having. Averaging 2-4 gradient estimates per iteration
stabilises the early phase when noise is high.

**The chess adaptation** (CPW SPSA page; fishtest wiki; OpenBench wiki). The
two evaluations are **one paired mini-match**: theta+ plays theta- directly,
same opening, both colours, so `y+ - y-` is the pair score in {-2..+2} — the
five outcomes are exactly the pentanomial classes (LL, LD, WL/DD, WD, WW) that
paired-game statistics are built on, and pairing cancels opening bias, the
dominant variance term. Per-parameter scaling replaces Spall's global `c`:
fishtest normalises `phi_i = theta_i / c_i` so each axis is perturbed by one
Elo-meaningful unit — choose `c_i` so `theta_i +/- c_i` costs "a small,
measurable Elo gap (a few Elo)" — and updates `Delta theta_i = r_i * c_i *
g_phi_i` with `g_phi_i ~ (wins - losses) * Delta_i`. Both frameworks
parametrise by **end values**: `c_end` and `r_end = a_end / c_end^2` at the
final iteration, from which `c = c_end * (K+1)^gamma` and `a = r_end * c_end^2
* (K+1+A)^alpha` recover Spall's constants (K = last iteration index).
Integers are kept as **floats internally and rounded only at the UCI
boundary**; OpenBench enforces an effective `C >= 0.5` for int parameters so
`round(x+c) != round(x-c)` — below that the parameter never moves. Every
update is clamped to `[min, max]`. Output is a hypothesis: fishtest and this
plan (S085) both SPRT the returned vector before believing it.

### 2. Shape for chesso

**No native SPSA in the installed fastchess.** `/usr/games/fastchess` reports
`fastchess alpha 1.8.2 20260729-74deac2`; its `-help` has no SPSA or tuning
mode (checked 2026-08-19). So the driver is a **python tool in `tools/`**
(`tools/spsa_driver.py`, matching `tools/search_bench.py` in style) that
shells **one short fastchess match per iteration**: two `-engine` entries,
both the tune build, theta+ options on one and theta- on the other via
`option.<UciName>=<int>` (supported per-engine, fastchess help), `-openings
file=<book> order=random` with a fresh seed or `order=sequential
start=<k*pairs>` so iterations do not replay openings, `-games 2 -repeat
-rounds <pairs_per_iter>`, `-each tc=<tc> option.Hash=16 option.Threads=1`, no
`-sprt`, `-concurrency <n>`; parse the final wins/losses (or the pentanomial
line, `-report penta` defaults true) from stdout. Per-iteration process spawn
is noise against 8+0.08 games. `fastchess.sh` stays untouched — it is the SPRT
harness (invocation at `fastchess.sh:122-131`), not this tool.

**The engine side already exists (S073).** `build-tune` (`-DCHESSO_TUNE=ON`,
DEV_MANUAL "The tune build") turns every parameter into a UCI spin option
(`src/chesso.cpp:934-944`). Critical: `setoption` **refuses** an out-of-range
value rather than clamping — `search_param_set` returns false and the engine
logs and keeps the old value (`src/chesso.cpp:1043-1071`,
`src/search_params.hpp:231-239`) — so a driver that fails to clamp theta+/-
itself plays games with a **silently-default parameter** and the gradient
measures the wrong vector. The driver clamps before every send, and its
parameter file's bounds must match `search_param_info()`; the dry-run mode
should diff them against the `option name ... min ... max ...` lines the
binary prints at `uci`. Derived state rebuilds inside the setter
(`search_params_rebuild_derived()` rebuilds the LMR table,
`src/search_params.hpp:239-243`), so per-match option setting is safe.

**The surface today: 22 parameters** in `src/search_params.hpp` (X-macro,
lines 41-195) — the plan's "twenty" predates S089's nine TM entries:
`OrderHistoryMax` 600000 [0,899999] :45; `MaxQsearchDepth` 8 [1,64] :51;
`RfpMargin` 75 [0,2000] :60; `RfpMaxDepth` 6 [0,63] :61; `RfpMinPly` 3 [0,63]
:70; `NullMoveBase` 2 [0,16] :77; `NullMoveDivisor` 6 [1,64] :78; `LmrBase` 75
[0,400] :85; `LmrDivisor` 225 [1,2000] :86; `LazyEvalMargin` 150 [0,2000] :91;
`AspirationMinDepth` 5 [2,64] :123; `AspirationDelta` 50 [1,2000] :124;
`AspirationMaxDelta` 400 [1,48000] :125; `TmSoftPercent` 60 [1,100] :145;
`TmHardPercent` 300 [100,1000] :146; `TmSuddenDeathPercent` 5 [1,100] :164;
`TmIncrementPercent` 50 [0,100] :165; `TmStabilityMax` 8 [0,126] :191;
`TmStabilityPercent` 4 [0,50] :192; `TmFallingMaxCp` 100 [1,2000] :193;
`TmFallingPercent` 50 [0,400] :194; `TmScaleMinPercent` 30 [1,100] :195.
Which subset a run tunes is S085's decision, recorded there.

**State and logging.** A run directory (gitignored, like `.ref-builds/`)
holding: the frozen run config (params, bounds, c_end/r_end, schedule, tc,
book, budget — S085 requires it written before the run); `checkpoint.json`
rewritten atomically every iteration (k, float theta, RNG state, cumulative
pair counts) so an interrupted run **resumes**; `trajectory.tsv` appended per
iteration (k, c_k, r_k, pair score, rounded theta) so the run is inspectable
and DEC-019-style flatness is visible; the fastchess PGN per run for the
forfeit check (`tools/forfeit_report.py` exists; the fastchess log is
WARN-only and comes back empty — S089 lesson). Last line printed:
`SPSA-DONE` or `SPSA-FAILED` (DEC-061); a multi-hour run is detached and
watched by the self-terminating primitive with a ceiling at 2x the budgeted
time (AGENTS.md par.12). The trajectory TSV that decides anything gets copied
to `adocs/data/` by the step that reads it.

### 3. Implementation sketch

**(a) Synthetic objective harness first.** The objective is injectable: an
interface the fastchess runner and a simulator both implement, returning a
pair score for (theta+, theta-). Simulator: true strength `s(theta) = -sum_i
w_i ((theta_i - theta*_i)/sigma_i)^2` in Elo; a pair is two games sampled at
`d = s(theta+) - s(theta-)`: per game `p_win = (1 - p_draw) /(1 + 10^(-d/400)) `
shaped variants are fine — with `p_draw ~ 0.5` (the S105 UHO band's
neighbourhood), pair-score sd is ~1.0-1.4, which is the real noise regime.
Validation numbers, deterministic seed, milliseconds per iteration:
- **Convergence**: 5 params, `theta*` offset ~30 % of range from `theta_0`,
  initial gap >= 20 Elo, K = 20000 iterations x 1 pair, `c_end = sigma_i`,
  `r_end = 0.002`: mean theta over the last 10 % of iterations within 25 % of
  the initial error on every axis, and `s(theta_final)` within 2 Elo of
  `s(theta*)`.
- **Sign**: same run with the update negated must FAIL that criterion and end
  with `s` no better than the start — the accepts' sign-flip test.
- **Clamp**: `theta*` outside a bound on one axis: every recorded theta stays
  in `[min, max]`, trajectory pins at the bound.
- **Integer floor**: an int param with `c_end < 0.5` is refused at config
  load (or raised with a warning), because +/-c rounds to the same value.
- **Resume**: kill after iteration n, restart from checkpoint, trajectory
  byte-identical to an uninterrupted seeded run.
The whole set must fit the fast suite (`ctest -L fast`; register like
`test_fastchess_script`, `tests/CMakeLists.txt:114-117`, label fast, tight
timeout; `.moltke.json` test_command runs it at completion).
**(b) Dry-run mode**: 2 iterations x 2 pairs at `tc=1+0.01` against
`build-tune`, asserting options were accepted (no `Rejected` in the engine
log), pair results parsed, checkpoint written and resumable.
**(c) Real driver**: config-file driven, detached, watcher armed, forfeit
count from the PGN. Running it is S085.

### 4. Constants and seeds

Every one **seed — must be verified on the synthetic objective here**
(DEC-084: form and constants from open literature may seed, then our own run
decides):
- `alpha = 0.602`, `gamma = 0.101` — Spall 1998 (jhuapl.edu PDF); OpenBench
  documents the same defaults.
- `A = 0.1 x planned iterations` — Spall: "10 % (or less) of the maximum
  number of expected/allowed iterations"; OpenBench field `A-Ratio`, default
  0.1.
- `r_end = 0.002` — OpenBench default; the same figure circulates from
  Stockfish's original tuner era (CPW/talkchess).
- `c_end` per parameter: the smallest change that plausibly matters; OpenBench
  guidance ~1/20 of the sane range; fishtest guidance: +/-c should cost a few
  Elo; `c_end >= 0.5` for integers. For chesso seeds: ~4 for `RfpMargin` and
  `AspirationDelta`-class centipawn params, 0.5-1 for depth/ply ints.
- Batch: OpenBench default 8 pairs per SPSA point; on this 12-thread machine
  (DEC-050) **6 pairs = 12 games = one concurrent wave** is the zero-idle
  shape; Spall's gradient averaging is the same lever.
- Budget: >= 30000 paired games before a run is read at all
  (eval_tuning_strategy.md par.4.3; S085's accepts) — S085's constant, the
  driver just must not hardcode any of these.
- Spall's semiautomatic cross-check: `c ~ sd of the pair score at theta_0`
  (measure it in the simulator), `a` such that the first iterations move each
  theta by about its `c_end` — use to sanity-check the derived `a = r_end *
  c_end^2 * (K+1+A)^alpha` before trusting a night to it.

### 5. Pitfalls

- **Sign flip**: a downhill run looks exactly like a run (the accepts' own
  warning). The synthetic test is the guard.
- **Unclamped theta+/-**: this engine **refuses** instead of clamping
  (`src/chesso.cpp:1058-1062`), so the game silently measures a default —
  worse than a crash because it converges confidently to nonsense. Clamp
  theta, and theta+c and theta-c, before every send; dry-run greps the engine
  log for `Rejected`.
- **Too-large `c_end`/`r_end` diverges** — fishtest RFC #535's operational
  complaint: end-value parametrisation means oversized values do not decay
  away. Too-small `c_k`: "if after a few thousand games the values are barely
  changing at all, the tuning run is useless" (fishtest wiki).
- **Integer sub-0.5 c never moves** (rounding eats the perturbation) — refuse
  at config load.
- **Flat objective drift**: DEC-019 — three published figures measured 0 here;
  a no-effect parameter random-walks inside its bounds and its final value is
  noise, not signal. The trajectory TSV is what shows it; S085's SPRT is what
  decides it.
- **Correlated parameters** (`LmrBase`/`LmrDivisor`, the TM soft/hard pair):
  SPSA walks ridges slowly and the per-axis endpoints are not independently
  meaningful — read the vector, never a coordinate.
- **Time-control reward hacking**: 9 of the 22 are time-management
  parameters; a game-result objective at 8+0.08 tunes them for 8+0.08 and
  toward the forfeit edge. Count forfeits from the PGN every run; leaving the
  TM family out of the first run is a legitimate S085 choice.
- **Adjudication interaction**: keep `-draw`/`-resign` identical for both
  sides and frozen in the run config, or scores shift around thresholds.
- **Tune build is not the shipping build** (specs.md: no strength number is
  taken on it). The gradient is fair — both sides carry the same
  variable-load cost — but the returned vector is only real after S085's SPRT
  of the **shipping** build.
- **Multi-hour run discipline**: detached, terminal marker as last line,
  self-terminating watcher with ceiling (DEC-061, AGENTS.md par.12); resume
  from checkpoint rather than restart.
- Framework internals beyond the wikis (fishtest's exact batching/A choice,
  OpenBench's update code) live **only in their source — not read; derive
  independently** and let the synthetic objective arbitrate (DEC-084).

### 6. Measurement

Driver = code + tests: the synthetic-objective numbers from par.3 recorded in
this file at completion, fast suite green (`.moltke.json` test_command), no
`src/` change so node counts are trivially identical (INV-6 has nothing to
discharge) and **no SPRT is owed** — S085 owes the SPRT of what a run returns.

### 7. Interactions

- **S085** consumes this immediately: it freezes the parameter list, bounds,
  `c_end`s, budget, tc and book, runs overnight (DEC-041), and SPRTs the
  result. Note for it: the live surface is 22, not the 20 its goal line says.
- **S105** lands first in plan order and sets the game regime the driver
  should default to (8+0.08, Hash 16, UHO book); the driver takes tc/book/hash
  from config so S105's values are data, not code.
- **S127** reruns the driver after the search block; S093-S132 will widen
  `search_params.hpp`, so the driver reads its parameter file and the `uci`
  option listing rather than baking in today's 22.
- **S073** built the tune build and `test_search_params` holds the two builds
  equal member by member; this step adds no parameter and must not touch that.

### 8. References

- https://www.chessprogramming.org/SPSA — CPW: update rule, Rademacher, chess
  match(theta+,theta-) in {-2..+2}, Kiiski's 2011 Stockfish method.
- https://www.jhuapl.edu/spsa/ — Spall's SPSA site (paper index).
- https://www.jhuapl.edu/spsa/PDF-SPSA/Spall_Implementation_of_the_Simultaneous.PDF
  — Spall 1998, read in full: gains, 0.602/0.101, A <= 10 % of iterations, c ~
  noise sd, `a` from desired first-step size, gradient averaging, blocking.
- https://official-stockfish.github.io/docs/fishtest-wiki/Creating-my-first-test.html
  — fishtest SPSA test fields (name, start, min, max, c_end, r_end), `r_k =
  a_k / c_k^2`, default range [0, 2v], stuck-run warning, TC guidance.
- https://github.com/official-stockfish/fishtest/wiki/Fishtest-mathematics —
  normalised phi_i = theta_i/c_i, g_phi ~ (wins-losses)*Delta, per-axis
  quasi-Newton reading, async worker updates.
- https://github.com/AndyGrant/OpenBench/wiki/SPSA-Tuning-Workloads — field
  set and defaults (alpha 0.602, gamma 0.101, A-ratio 0.1, r_end 0.002,
  pairs-per 8), int C >= 0.5, float-internal/round-at-UCI, c_end ~ range/20,
  reporting/distribution notes.
- https://github.com/AndyGrant/OpenBench/wiki — wiki index (page discovery).
- https://github.com/official-stockfish/fishtest/issues/535 — SPSA
  improvements RFC read as prose: divergence on oversized end values, rounding
  at the worker, TC-insensitivity experiments (20+0.2 down to 0.5+0.01).
- Surfaced but **not read** (engine/framework source): zamar/spsa,
  fairy-stockfish/spsa, fishtest server code — DEC-084.
author:    Maksym Bodnar

## What landed

- `tools/spsa_driver.py` — the driver. `check`, `run`, `run --resume`.
- `tests/test_spsa_driver.py` — 22 assertions on a synthetic objective, no games.
- `tests/CMakeLists.txt` — `test_spsa_driver`, label `fast`, timeout 120.
- `tools/spsa_dryrun.json` — the smoke config, and the format DEV_MANUAL quotes.
- `DEV_MANUAL.md` — "Tune search parameters with SPSA", and a correction to the
  tune build section. `MANUAL.md` — the same correction to its own copy of it.
- `.gitignore` — `.spsa`.
- No change under `src/`. INV-6 has nothing to discharge and no SPRT is owed.

## Two things the tune build will not tell you

Both found here, both change what the step file planned, and together they are
S137.

**The refusal is invisible.** `setoption` with an out-of-range value is refused
rather than clamped (`src/search_params.hpp:231-239`) and the refusal goes to
`LOG_W`, which is `if (false) std::clog` under `NDEBUG` (`src/log.hpp:35`).
`build-tune` is `CMAKE_BUILD_TYPE=Release`. So this step file's plan — "the
dry-run mode should diff them against the `option name ...` lines" and "dry-run
greps the engine log for `Rejected`" — is half unimplementable: there is no
`Rejected` line to grep, in the log or on either stream.

**There is no readback either.** `uci` re-prints the compiled default:

```
setoption name RfpMargin value 120
uci                     # option name RfpMargin type spin default 75 ...
```

What replaced the grep, in `check`:

1. every config name against the binary's own `uci` listing, and every bound
   against the binary's bounds — the half of the plan that does work, and now
   the only guard the engine side offers;
2. a node-count probe, because it is the one remaining observable. Measured
   2026-08-20: `RfpMargin` 75 searched **164123** nodes at depth 9 on the
   `search_bench.py` midgame position and 2000 searched **743308** — the same
   two figures DEV_MANUAL's tune build section quotes, so `setoption` demonstrably
   reaches the search.

The probe cost one bug of its own on the way: piping the script in and closing
stdin makes the engine take EOF for `quit` and abandon the search, which
reported **230** nodes at depth 9 and read exactly like "setoption does not
work". `search_bench.py` holds the pipe open for the same reason.

## The gate, and what it measures

`ctest -R test_spsa_driver` — 22 assertions, **3.99 s**, inside `-L fast`
(10.9 s before this step). It plays no games.

The synthetic objective: 5 parameters on [0, 1000] starting at 500, optimum 300
away in alternating directions, strength `-sum w_i ((theta_i - opt_i)/sigma_i)^2`
with `w = 500` and `sigma = 1000`, so the start point is **225 Elo** below the
optimum. Pairs are two games at a 50 % draw rate, giving a pair-score spread of
about 1.0 — the real regime.

**Convergence**, 5000 iterations x 4 pairs, `c_end = 50`, `r_end = 0.008`:
worst axis error **27.8** of the 300 it started at, final strength **-0.86 Elo**.
Over seeds 1 to 10 the worst case was **65.9** and **-3.96 Elo**, which is what
the tolerances are set from (75, and 5 Elo) rather than from the seed the test
happens to use. **Flipped sign**, same everything: every axis **799** off,
strength **-1594 Elo**, pinned against the far bound. The two are not close.

**`r_end` is not inheritable.** At 20000 pairs on this objective:

| `r_end` | max axis error | final strength |
|---|---|---|
| 0.002 | 293 of 300 | -21.8 Elo |
| 0.008 | 28 of 300 | -0.9 Elo |
| 0.050 | 221 of 300 | -6.2 Elo |

0.002 is the OpenBench and fishtest-era seed and it moves this objective **2 %**
of the way — the "barely changing after a few thousand games" run the fishtest
wiki calls useless, reproduced. The bottom row is RFC #535's divergence
complaint: an oversized end value does not decay away, because the end-value
parametrisation shrinks the step by only about 1.6x across a whole run (`r_k`
*rises* by 1.74x while `c_k` falls by 2.72x). DEC-084 as intended: the seed
chose what to try and the measurement chose what to use.

**Iterations against pairs is close to a wash at a fixed game budget.** 20000
pairs converged the same objective to comparable error as 20000x1, 10000x2,
5000x4 and 2500x8 — max axis error 39.9, 49.2, 27.8, 32.8. That is what let the
gate be a quarter of the wall time it started at, and it is a fact S085 can
spend: pairs per iteration can be sized to fill the 12 threads without paying
for it in convergence.

## The gate was checked by breaking the driver

Four mutations, each reverted:

| mutation | caught |
|---|---|
| step ignores the perturbation direction (`* delta[i]` dropped) | yes, 2 failures |
| `gamma` 0.101 -> 0.6, so `c_k` decays asymptotically | yes |
| resume stops truncating the torn trajectory tail | yes |
| clamp removed from the theta update | **no** |

The last one is why the clamp test now asserts the **float** theta the run
returns and not only the integers the trajectory records: `uci_value()` clamps
at the boundary, so an internal value winding up thousands outside its range
prints a tidy bound while the axis sits unresponsive for as many iterations as
it took to drift out. With that assertion the mutation is caught.

## The dry run, with real games

`tools/spsa_dryrun.json`, 3 parameters, 2 iterations x 2 pairs at `tc=1+0.01`
against `build-tune`:

- `check` passed, including the node probe above.
- 8 games played and parsed, `Games:` and `Ptnml(0-2):` both, from the real
  `fastchess alpha 1.8.2 20260729-74deac2` — whose output block is pasted
  verbatim into the test, so the parser is covered without playing anything.
- killed after iteration 0 (`--crash-after 0`, exit 9), resumed from
  `checkpoint.json` at k=1, finished. `tools/forfeit_report.py` over the run's
  PGN: **0 forfeits of 8 games**.
- **Nothing moved**, and that is correct: two iterations at `r_end` 0.002 step
  `RfpMargin` by 0.02, which rounds back to 75. A smoke run is not a small
  tuning run and cannot be read as one.

Resume is byte-identical, tested at both interruption points — after the
checkpoint, and between the trajectory row and the checkpoint, which is the case
that needs the tail truncated.

## One deliberate deviation

Checkpoint and trajectory `fsync` only when the objective is a real match. Two
fsyncs per iteration is 13 ms, which is nothing against a mini-match and is 26 s
against 2000 simulated iterations — it would have kept the gate out of the fast
suite. The atomic rename is unconditional, so a killed process still cannot
leave a checkpoint that will not parse; what the fsync buys on top is the
machine losing power mid-iteration, and a simulated iteration is reproducible
from the seed anyway.

## For S085

- The live surface is **22** parameters, not the 20 its goal line says.
- `r_end` and `pairs_per_iter` are chosen together: `(wins - losses)` is summed
  over the iteration's pairs, so doubling the pairs doubles the step.
- The trajectory TSV is what shows a stuck run and DEC-019 flatness. Copy the
  one that decides anything into `adocs/data/`.
- `c_end` below 0.5 is refused at config load. A narrow range relative to the
  first iteration's `2c` warns rather than refuses.
- S137 lands before it, so the run does not have to trust the driver's clamp
  alone.

## Documents

`README.md` checked — owner-written, no change needed. `adocs/specs.md` checked:
no behaviour changed, nothing under `src/` was touched, and it names no tooling
this step alters. `MANUAL.md` and `DEV_MANUAL.md` both carried the claim that a
refused value is logged; both corrected.
