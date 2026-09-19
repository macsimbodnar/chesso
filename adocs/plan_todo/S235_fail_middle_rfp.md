id:         S235
goal:       a node pruned by reverse futility returns a point between its estimate and beta, weighted by one fitted parameter, instead of the estimate itself -- the first of four fail-middle sites, one site per verdict
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; one parameter in `src/search_params.hpp` with a stated range whose one end returns the estimate (today's behaviour, the off value) and whose other end returns beta, seeded at form (c), the midpoint; at the off value the tree is the parent's exactly, bench signature identical (INV-6), proved on the tree (DEC-215); a test that observes the returned score move between the two ends as the parameter moves, with the precondition counted; a mate-band estimate never interpolates, covered by a case, and `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant that interpolates on the wrong side of beta is killed (`tools/mutation_check.py`); Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tools/mutants/
excludes:   the ProbCut, multi-cut and quiescence stand-pat returns -- each is its own later step once its site exists (S113, S097), one site per verdict; the reverse-futility margin `RFP_MARGIN` and depth ceiling themselves; any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-105, DEC-134, DEC-141, DEC-215
closes:
blocks:
paused_by:
done:

## Why this exists

A node that prunes on a margin returns the margin's own estimate today. The
2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, row N2)
found the engine it read returning a point interpolated between that estimate
and beta at the reverse-futility site, measured at +9.71 over 4654 games, with
three further sites each measured separately afterwards at +1.4 to +4.5. The
site here is the `RFP_MARGIN` test in `src/search.cpp` `negamax`; the
returned value carries how far past beta the node stood instead of the raw
estimate. One weight, fitted here. Reported figures decide what to try, never
what to conclude (DEC-019).

## Shape

At the reverse-futility return in `src/search.cpp` `negamax`, replace the
returned estimate by `beta + (estimate - beta) * w` in integer arithmetic with
`w` the parameter over its declared scale, so that one end of the range is the
estimate and the other is beta. Mate-band estimates return unchanged.

## Seeds (DEC-134)

One parameter, form (c): the midpoint of its declared range, stated as such.
No published value seeds it and no other engine's does.

## Clean room (DEC-221)

The implementing agent's brief carries this file and the study's row N2 in
prose and nothing else; it does not open the clone `.moltke.local.md` names.

## Cost

One verdict at DEC-143's price, 12 to 20 hours.
