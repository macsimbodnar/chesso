id:         S126
goal:       every constant in the evaluation is refitted once the search that consumes them has stopped moving
accepts:    a fit over the corpus S082 and S083 produce, with the held-out figure from the by-game splitter S066 fixed; the emitted table carries the provenance stamp S077 added; an SPRT of the whole refit against the incumbent, recorded whatever it comes back as -- S065 is the precedent and it moved 827 constants in one verdict; **early stopping is on the held-out split and not on training loss**, because the documented overfitting signature is loss still falling while strength falls; any parameter group the fit drives to zero is reported as a corpus finding rather than shipped silently as zero, which is the S100 lesson applied forward
touches:    tools/tuner.cpp, tools/tuner_model.hpp, tests/test_tuner_gradient.cpp, src/evaluation.cpp weights, src/eval_tables.hpp
excludes:   adding or changing any term, which is what every step above this did
decisions:  DEC-084, DEC-041
closes:     2026-09-10_adversarial-F24
blocks:
paused_by:
done:

## Why a refit is a step and not a chore

"Eval parameters are only optimal relative to the search that uses them" is the
second sentence of `adocs/eval_tuning_strategy.md`, and this plan changes the
search more than it has ever been changed. Every weight fitted before S109
lands was fitted against a tree that no longer exists.

The surveyed record puts a full retune repeatedly at +5 to +15 years apart,
and one engine's move from a hand-picked to a fitted evaluation as **the only
change in a release** carried it from 2529 to 2910 on the public list.
**Both are unverified**: the 2026-09-04 literature check located neither the
+5-to-+15 range nor the 2529-to-2910 release, and neither is needed -- this
step's argument is the local one below it, S028's **+188.74** measured in this
engine, and `adocs/eval_tuning_strategy.md`'s opening sentence. S186 owns the
two figures (DEC-137). S028
measured +188.74 here doing the same thing. This is the cheapest large number
on the plan and it costs no engine code.

## Amended 2026-09-11, DEC-170: the gradient's clamp treatment is decided before the refit (F24)

`2026-09-10_adversarial-F24`: `tools/tuner_model.hpp`'s gradient ignores the
model's clamp on the mobility-plus-king-safety sum, defended by a stale
sentence -- "the term's maximum over 149084 real positions was 143 against a
bound of 150", a mobility-only figure at the old margin -- while
`tools/eval_model.hpp` already says the opposite. Measured at the shipping
weights over 400000 rows: 0.33 % of rows are clamped, carrying 0.44 % of the
stage-two gradient magnitude, and `tests/test_tuner_gradient.cpp`'s fixtures
contain no clamped row, so the finite-difference check has zero coverage of
the region where the gradient is knowingly not the derivative. Before this
step's fit: decide and write whether the clamp's boundary is treated as flat
(the derivative of a clamped term is zero) or ignored as today, state the
share of rows it touches at the then-current weights, and give the
finite-difference test one fixture row that is clamped so the choice is under
test either way. S039 and S122 may have moved or removed the clamp by then,
which changes the share and not the obligation.
