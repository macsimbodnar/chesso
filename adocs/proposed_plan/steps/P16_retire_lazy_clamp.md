id:         P16 (proposed; the S-id is allocated at adoption)
goal:       evaluate() returns the full score: the ±LAZY_EVAL_MARGIN clamp on mobility plus king safety goes, and quiescence asks the full evaluation
accepts:    (1) before the SPRT, a census decides between removing evaluate_lazy entirely and keeping its shortcut without the clamp (whose bound is then no longer guaranteed); the step ships the census's choice and says which; (2) weights unchanged in this verdict; the clamp leaves P02's trace model; (3) LazyEvalMargin leaves the tune build's option surface: test_uci_surface and MANUAL.md updated first (SURFACE); test_search_params' pins follow; (4) DEC-169's truncation-bound reading retires with the margin by a decision entry, and any test that existed for it is retired by that decision, never deleted silently (TESTS); (5) one SPRT `{-5, 5}` (DEC-063: expected small; the step exists to unblock P17)
touches:    src/evaluation.cpp, src/evaluation.hpp, src/search.cpp, src/search_params.hpp, tools/tuner_model.hpp, tests/test_evaluation.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, MANUAL.md, adocs/specs.md
excludes:   any refit; any new term
closes:
paused_by:
author:
done:

## Why

`specs.md`: "The lazy-evaluation clamp is the ceiling on how much the
evaluation is allowed to say … A king-safety term strong enough to price a
mating attack cannot exist under it." S120 measured on 2026-10-06, clamp kept:
the full evaluation everywhere costs 5.0 % nps for a tree 7.5 % smaller and
2.6 % less wall time. The shortcut no longer pays for itself, and the clamp
blocks P17.

## From the record

S034 (lazy evaluation), DEC-039, DEC-040 (kept at a measured zero as the
platform for the fit), DEC-045 (the guaranteed bound), S085 (the margin's
current 184), S120 and DEC-257 (the evaluation cache retired).
