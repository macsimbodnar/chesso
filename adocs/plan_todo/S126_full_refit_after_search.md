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
and that range is now traced: one engine's own ledger of seven retunes reads
2.8, 4.0, 5.8, 10.2, 12.8, 24.6 and 39.4, median 10.2, each over 32000 games
(section 1). A rating jump this file used to quote for a release whose only
change was a fitted evaluation carried no source through two searches and is
deleted (DEC-203). The local evidence is what this step's argument rests on
anyway: S028 measured **+188.74** here doing the same thing. This is the
cheapest large number on the plan and it costs no engine code.

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

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The "+5 to +15 for a full retune" range is now traced to a published ledger,
and the ledger says it is right.** CPW *Texel's Tuning Method*
(https://www.chessprogramming.org/Texel%27s_Tuning_Method, fetched 2026-09-13)
carries Peter Osterlund's own running total for his engine: "Since 2014-01-01
when the first evaluation change based on this algorithm was included in my
engine, the Elo improvements caused by evaluation weight tuning (measured using
32000 games at 1+0.08) have been: 24.6 + 4.0 + 5.8 + 2.8 + 12.8 + 39.4 + 10.2 =
99.6 Elo". Seven retunes of one engine, each measured over 32000 games at a
fixed control: **2.8, 4.0, 5.8, 10.2, 12.8, 24.6 and 39.4**. Five of the seven
sit at or below 12.8 and the median is 10.2, which is the "+5 to +15" claim
with a source and a spread instead of a bare range -- and with a tail on both
sides that a range hides.

**The rating-jump figure this file used to quote survived two searches
unsourced and is deleted (DEC-203).** It was written as a release whose only
change was a fitted evaluation, moving the engine by several hundred points on
a public list. Searched 2026-09-04 by the plan review: not located. Searched
this pass: a web search for such a release and for the two ratings it named,
which surfaced Zurichess's evaluation write-up -- and
Medium returns HTTP 403 to every fetch route tried, so the article could not be
read; the CPW *Zurichess* page describes its releases in
release-delta terms ("about 100 Elo stronger", "about 80 Elo stronger") and
attaches none of them to an evaluation-only change. Not located. Nothing in
this step rests on it: the argument is the local one -- S028's **+188.74**
measured in this engine -- plus the sourced ledger above.

**The one sentence this step is built on is unsourced where it is written, and
this pass did not find a source for it either.** `adocs/eval_tuning_strategy.md`
opens with "Eval parameters are only optimal relative to the search that uses
them" and cites nothing. CPW *Texel's Tuning Method* and *Automated Tuning*
(https://www.chessprogramming.org/Automated_Tuning, fetched 2026-09-13) both
state the tuning method without saying anything about re-tuning after a search
change. The claim is kept because it is the strategy document's own and
because the seven-retune ledger is consistent with it, and it is marked here as
**unverified as a published claim**.

**Early stopping on the held-out split has no published figure and one published
warning.** CPW *Automated Tuning* names regularisation only for Vladimir
Medvedev's logistic regression and is silent on overfitting, holdout splits and
dataset size; CPW *Texel's Tuning Method* is silent on all three too. So the
accepts' rule -- stop on the held-out split, not on training loss -- is this
project's own and rests on S066's splitter fix and on DEC-170's F24 work, not
on a citation. **One engine record does state a threshold** and it is recorded
in S083's section 1, not here: Stash's changelog, v32.0 (2021-12-02), "the eval
doesn't overfit for datasets > 500k positions"
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md). One author,
one parameter count, one corpus; it is direction and not a rule this step
adopts (DEC-019).

### 2. Shape for chesso

A refit of every constant, on the corpus S082 and S083 produce, with the
held-out figure from the by-game splitter S066 fixed and the provenance stamp
S077 added. `tools/tuner.cpp` is full-batch Adam over the closed-form gradient
in `tools/tuner_model.hpp`; its `--epochs` option is documented in its own
help text as "maximum full-batch steps", which is the fact
`adocs/eval_tuning_strategy.md`'s mini-batch paragraph diverges from -- see
that document's dated note of 2026-09-13.

**Departures from the published method, stated:**

- CPW's formulation fits against the **quiescence score** of the position in
  the corpus; chesso fits a linear model of `evaluate()` over stored features
  (`tools/eval_model.hpp`), which is the same objective reached through a
  design matrix and is what makes an epoch cheap. The model-equals-engine test
  is what keeps the two the same function.
- CPW's method has no validation split and no early stopping; this step has
  both, deliberately.
- CPW's `K` is fitted once and then held. This step does the same and says so.

### 3. Implementation sketch

- Fit `K` first, then the parameters, then quantise to integers **last** and
  re-measure -- rounding a few hundred parameters can cost measurable Elo if
  any sit near a decision boundary, which is
  `adocs/eval_tuning_strategy.md` section 2.5's warning and is why the SPRT is
  over the quantised vector and not the float one.
- The clamp decision the F24 amendment above requires is made **before** the
  fit, written down, with the share of clamped rows stated at the
  then-current weights and one clamped fixture row in
  `tests/test_tuner_gradient.cpp`.
- Any group the fit drives to zero is reported as a corpus finding, not
  shipped silently as zero.

### 4. Constants and seeds

**Every constant in the engine is a seed here in the only sense that matters:
the fit starts from what ships and ends where the data puts it.** No external
value enters. Two knobs are chosen rather than fitted and both are declared:

- **The learning rate and the epoch budget** -- **(c) the defaults
  `tools/tuner.cpp` already documents**, stated as such, moved only if the
  run's loss curve requires it and the move recorded.
- **The early-stopping patience** -- **(b) derived from this project's own
  runs**: the number of report intervals over which the held-out figure has
  historically stopped improving in this engine's fits, read at this step's
  start from the recorded runs rather than chosen.

`adocs/eval_tuning_strategy.md` section 2.5's "Learning rate around 1.0 on
centipawn-scale parameters" is a statement in this project's own strategy
document, not an engine's constant, and `tools/tuner.cpp`'s help text already
gives 1.0 as the default; it is form (c) and is recorded as the incumbent.

### 5. Pitfalls

- **Training loss is not the criterion.** The documented overfitting signature
  is loss still falling while strength falls; the accepts makes early stopping
  read the held-out split for that reason.
- **The splitter must be by game.** Positions from one game are correlated by
  construction -- at the published density, about 140 per game -- so a
  position-wise split leaks and reports a held-out figure that is not one.
  S066 fixed it; this step must not undo it.
- **Quantisation is a second change.** Fit in floats, round last, re-measure.
- **One verdict over 823 constants attributes nothing on a failure.** S065 is
  the precedent for the form and the bisect discipline (DEC-082) is what a
  negative verdict is followed by.
- **The clamp's gradient is knowingly not the derivative** in the clamped
  region. F24 measured 0.33 % of rows carrying 0.44 % of the stage-two
  gradient magnitude; S039 and S122 may have moved or removed the clamp by
  then, which changes the share and not the obligation.
- **A published total is not a target.** The 99.6 Elo above is seven retunes of
  a different engine over years; it says the direction pays and nothing about
  the size here (DEC-019).

### 6. Measurement

Held-out error, reported. One SPRT of the whole refit against the incumbent at
the S105 regime, bounds stated in advance with the nElo worst case and the
abort rule (DEC-143), verdict recorded whatever it is. S065 is the precedent:
827 constants in one verdict. The expected size from the located record is the
+2.8-to-+39.4 band of a single retune at another engine, and the local
precedent is S028's +188.74 from hand-written starting values -- which is a
different situation and is not a prediction.

### 7. Interactions

- **S082 and S083 (before)**: the corpus and its size.
- **S134 (before)**: the parameter count this fits, 823 rather than 827, with
  the two exact degeneracies removed. DEC-170's ruling against regularisation
  reopens here **only if this fit shows a second ridge**, and
  `tools/feature_audit.cpp`'s report over the remaining columns is what that
  is read against.
- **S039 and S122 (before)**: both may move or remove the clamp the F24
  amendment is about.
- **S133 (immediately before)**: the tables this fit has to cover.
- **S127 (block 4)**: SPSA over the search parameters, after this.

### 8. References

- - https://www.chessprogramming.org/Texel%27s_Tuning_Method -- the seven-retune
  ledger quoted above, 24.6 + 4.0 + 5.8 + 2.8 + 12.8 + 39.4 + 10.2 = 99.6 Elo,
  each measured over 32000 games at 1+0.08; the objective, the sigmoid and `K`
  fitted once; **no treatment of validation splits, early stopping,
  regularisation or overfitting**. Fetched 2026-09-13.
- - https://www.chessprogramming.org/Automated_Tuning -- regularisation named
  only for Medvedev's logistic regression; overfitting, holdout splits and
  dataset size absent; **no Elo figure**. Fetched 2026-09-13.
- - `adocs/eval_tuning_strategy.md` -- the opening sentence this step is built
  on (**unverified as a published claim**), section 2.5 on the optimiser and
  on quantising last, and the dated note of 2026-09-13 recording where the
  document and `tools/tuner.cpp` diverge. Local, not literature.
- - `adocs/plan_done/S028_*` -- +188.74 measured in this engine. Local
  evidence, and the one the step's argument rests on.
