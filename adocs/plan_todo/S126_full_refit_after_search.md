id:         S126
goal:       every constant in the evaluation is refitted once the search that consumes them has stopped moving
accepts:    a fit over the corpus S082 and S083 produce, with the held-out figure from the by-game splitter S066 fixed; the emitted table carries the provenance stamp S077 added; an SPRT of the whole refit against the incumbent, recorded whatever it comes back as -- S065 is the precedent and it moved 827 constants in one verdict; **early stopping is on the held-out split and not on training loss**, because the documented overfitting signature is loss still falling while strength falls; any parameter group the fit drives to zero is reported as a corpus finding rather than shipped silently as zero, which is the S100 lesson applied forward
touches:    tools/tuner.cpp, src/evaluation.cpp weights, src/eval_tables.hpp
excludes:   adding or changing any term, which is what every step above this did
decisions:  DEC-084, DEC-041
closes:
blocks:
paused_by:
done:

## Why a refit is a step and not a chore

"Eval parameters are only optimal relative to the search that uses them" is the
second sentence of `adocs/eval_tuning_strategy.md`, and this plan changes the
search more than it has ever been changed. Every weight fitted before S109
lands was fitted against a tree that no longer exists.

The surveyed record puts a full retune repeatedly at +5 to +15 years apart, and
one engine's move from a hand-picked to a fitted evaluation as **the only
change in a release** carried it from 2529 to 2910 on the public list. S028
measured +188.74 here doing the same thing. This is the cheapest large number
on the plan and it costs no engine code.
