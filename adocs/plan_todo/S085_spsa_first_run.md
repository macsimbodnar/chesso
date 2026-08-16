id:         S085
goal:       the first SPSA run on the search parameters, and an independent SPRT of what it returns
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
