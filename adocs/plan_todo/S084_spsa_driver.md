id:         S084
goal:       an SPSA driver over the exposed search parameters, verified against an objective whose optimum is known
accepts:    a tracked tool implements the Spall iteration -- `a_k = a/(k+1+A)^alpha`, `c_k = c/(k+1)^gamma`, Rademacher perturbation, two objective evaluations per iteration -- reads a parameter file naming each parameter, its bounds and its `c_end`, and drives paired games through `fastchess` against the tune build S073 produces; a test runs the driver against a synthetic noisy quadratic with a known optimum and asserts it converges to within a stated tolerance, and fails if the sign of the update is flipped; the driver checkpoints to disk every iteration so an interrupted run resumes rather than restarts; it prints a terminal marker as its last line, so a watcher exits on the run rather than on a turn boundary (DEC-061); nothing under `src/` changes and no verdict is owed
touches:    tools/, tests/, DEV_MANUAL.md
excludes:   running it on the engine, which is S085's; choosing which parameters to tune and their ranges, which S085 decides and records; any change to `src/`; CLOP, CMA-ES and every other black-box optimiser
decisions:  DEC-019
closes:
blocks:
paused_by:
done:

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
