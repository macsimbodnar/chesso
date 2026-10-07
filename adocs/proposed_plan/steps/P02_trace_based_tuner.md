id:         P02 (proposed; the S-id is allocated at adoption)
goal:       the tuner fits the evaluation from coefficient traces that evaluate() records under a trace build, so a new term costs its trace call and a parameter declaration, and nonlinear groups are fitted with analytic gradients
accepts:    (1) a trace build (compile-time switch, compiled out of the release and tune builds) records per position the sparse coefficient vector and each nonlinear group's pre-transform sums; recomputing the score from the trace equals evaluate() exactly, after the engine's own rounding, on every row of a >= 1 M-row sample of the current corpus; a fast-label test holds the identity on a small fixture; (2) the gradient matches finite differences over every parameter, worst error stated (S100 measured 4.9e-8 for the current tuner); (3) S100's known-vector recovery test passes on the new tuner; (4) on selfplay_v2 deduplicated, same split by game (S066) and seed, the new tuner's held-out loss on the current model is no worse than the current tuner's by more than a tolerance stated before the run; (5) output is `constexpr int` (DEC-255) with the provenance stamp (S077), and freeze groups by name survive (DEC-057); (6) supports three nonlinear group kinds: a per-side transform (king safety), a sign-preserving endgame adjustment (complexity), a scale factor on the endgame score; (7) release `bench` identical to the parent (INV-6)
touches:    tools/tuner.cpp, tools/tuner_model.hpp, tools/tuner_groups.hpp, tools/tuner_target.hpp, tools/eval_model.hpp, src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tests/test_tuner_gradient.cpp, tests/test_eval_model.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md
excludes:   any new evaluation term; any fit that ships (P03, P26); the corpus format and generator (P03)
closes:
paused_by:
author:
done:

## Why

Every term in block 2 needs fitting. Today `tools/eval_model.hpp`
re-implements the evaluation for the tuner, term by term;
`tools/tuner_model.hpp`'s own header records three occasions when a term's
gradient block was forgotten and the fit returned the weights it was handed. With a trace, the evaluation is its own model and that class
of defect cannot occur. King safety (P17), complexity (P25) and the endgame
scale factor (P24) are nonlinear and need the gradient through the transform;
DEC-044 kept king safety linear only because the tuner could not do that.

## Description it is implemented from

Grant, *Evaluation & Tuning in Chess Engines* (2020): §4.1 evaluation
wrapping (every weight application also records its coefficient), §4.2 the
sparse per-position tuple layout, §3 the derivatives for linear terms, king
safety (§3.5) and complexity (§3.7), §2.3 the base-e sigmoid with the 1/400
absorbed into K. CPW *Texel's Tuning Method*. The optimiser is Adam
(Kingma and Ba, 2014) or AdaGrad (Duchi, Hazan and Singer, 2011), picked by a
comparison the step pre-registers. The step implements from these
descriptions (DEC-221); no engine's tuner source is opened.

## Seeds

Optimiser hyperparameters: the papers' own published defaults (a publication
about the technique, DEC-134 form 1), or a stated sweep over chesso's corpus.

## Measurement

Neutral for the engine (`bench` identity). The loss comparison in accepts (4)
is an offline reading recorded under `adocs/data/`, not a verdict.

## From the record

S028, S065, S075 (WDL/score blend, `--lambda`), S076 (dedupe), S077
(provenance), S100 (gradient and recovery checks), DEC-053 (truncation guard
slack), DEC-169 (truncation reading when LAZY_EVAL_MARGIN moves; P16 retires
the margin).
