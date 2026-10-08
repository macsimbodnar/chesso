id:         S262
goal:       the tuner fits the evaluation from coefficient traces that evaluate() records in a trace build, so a new term costs its trace call and a parameter declaration, and nonlinear groups are fitted through their own analytic gradients
accepts:    (1) a trace build -- a compile-time switch compiled out of the release and tune builds -- records per position the sparse coefficient vector, **each side's** pre-transform sum for every nonlinear group (a per-side transform needs both sides' coefficients, not their difference), and the inputs of each endgame adjustment; (2) **exact reconstruction**: the score recomputed from a trace at the weights under test equals `evaluate()` after the engine's own integer tapers, rounding and clamp, on every row of a sample of at least one million corpus rows, and a fast-label test holds the identity on a small fixture -- the identity is the contract that replaces `tools/eval_model.hpp`'s hand-written mirror; (3) the analytic gradient is checked against finite differences of the **smooth** objective the optimiser minimises, over every parameter, with the treatment of truncation and clamp stated (S100 measured 4.9e-8 worst error for the current tuner); (4) S100's planted-vector recovery passes on the new tuner, and a recovery case covers each nonlinear group kind; (5) a nonlinear group whose transform has zero gradient at its seed -- `max(0, x)^2` at x = 0 is the case -- is seeded from a nondegenerate derivation over chesso's own data (DEC-134 form b), and recovery from that seed is shown before any long fit; (6) on the current corpus, same split by game (S066) and seed, the new tuner's held-out loss on the current model is no worse than the current tuner's by more than a tolerance stated before the run, recorded under `adocs/data/`; (7) output is `constexpr int` (DEC-255) with the provenance stamp (S077), and freeze groups by name survive (DEC-057); (8) trace extraction time and storage per million rows are measured and stated; (9) the src/ instrumentation is behaviour-neutral by INV-6 in full -- `bench` and `tools/search_bench.py` node counts and best moves identical at two depths -- and its release-build cost is a timing against the parent; (10) `DEV_MANUAL.md`'s tuner section describes the new path and retires the old one only after (6) holds
touches:    tools/tuner.cpp, tools/tuner_model.hpp, tools/tuner_groups.hpp, tools/tuner_target.hpp, tools/eval_model.hpp, src/evaluation.cpp, src/evaluation.hpp, tests/test_tuner_gradient.cpp, tests/test_eval_model.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md
excludes:   any new evaluation term; any fit that ships weights (S083, S126 and the term steps own those); the corpus format and generator, which are S082's
decisions:  DEC-258, DEC-259, DEC-255, DEC-057, DEC-221
closes:
blocks:
paused_by:
author:
done:

## Why

Every evaluation family in the new order needs fitting, three of them
through a nonlinear transform -- king safety (S122), the endgame scale
factor and mop-up (S124), complexity (S266). Today `tools/eval_model.hpp`
re-implements the evaluation term by term for the tuner, and
`tools/tuner_model.hpp`'s own header records three occasions when a term's
gradient block was forgotten and the fit silently returned its input. A trace
makes the evaluation its own feature extractor. It does **not** make a
missing gradient impossible: a nonlinear group still needs its model, its
derivative and a test, which is what (3) to (5) are for. DEC-044 kept king
safety linear only because the tuner could not fit through a transform.

## Description it is implemented from

Grant, *Evaluation & Tuning in Chess Engines* (2020), the paper that
describes the method: evaluation wrapping (every weight application also
records its coefficient), the sparse per-position layout, the derivatives for
linear terms, for a per-side king-safety transform and for complexity, and
the base-e sigmoid. CPW *Texel's Tuning Method*. The optimiser is Adam
(Kingma and Ba, 2014) or AdaGrad (Duchi, Hazan and Singer, 2011), chosen by a
comparison pre-registered in this step. Implemented from these descriptions
(DEC-221); no engine's tuner source is opened. Optimiser hyperparameters: the
papers' own published defaults (DEC-134 form a) or a stated sweep over
chesso's corpus (form b).

## Lane

Agent work under DEC-260: written in a separate worktree while matches hold
the workstation, compiled, tested and timed only between runs. It lands
before S134's fold is fitted on and before S082's corpus is selected, so
every later fit uses it.

## Measurement

Neutral for the engine; no match. (6) is an offline reading, not a verdict.

## From the record

S028, S065, S075 (the WDL/score blend, `--lambda`), S076 (dedupe), S077
(provenance), S100 (gradient and recovery checks), DEC-053 (truncation guard
slack), DEC-169 (truncation reading when the margin moves; S039 retires the
clamp). Ported from the 2026-10-07 proposal's P02 with the corrected
comparison's conditions (exact integer reconstruction, smooth-objective
gradients, per-side sums, nondegenerate seed) folded in.
