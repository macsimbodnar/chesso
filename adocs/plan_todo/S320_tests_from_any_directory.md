id:         S320
goal:       the six test binaries that still fail from the repository root read their fixtures from the source tree, so a binary's tally does not depend on the directory it runs from
accepts:    `test_eval_trace`, `test_invariants`, `test_leaf_walk`, `test_eval_model`, `test_evaluation` and `test_movegen` each read the same tally from the repository root, from `build/tests` and from a third directory, before and after shown; every other test binary checked the same way, with what it read; no bound moves and no case is dropped; `test_tuner_gradient`'s `CAPTURE(tuner_optimiser::kind_name(kind))` prints the optimiser's name, seen on a failure; both builds green
touches:    tests/CMakeLists.txt, tests/test_tuner_gradient.cpp, DEV_MANUAL.md
excludes:   any change to what a case asserts; refactoring the fixture loader beyond S310's `CHESSO_SOURCE_DIR` form
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-02 by S310's implementer and reproduced by its fast check. S310 made `load_json` in `tests/test_helpers.hpp` read from the source tree under `CHESSO_SOURCE_DIR`, defined on `test_tuner_gradient` and already on `test_search`. The binaries without the macro still open `assets/...` from the working directory. Green under ctest, which runs them from `build/tests`, so filler by DEC-171: not reachable in play, and it costs whoever runs a binary by hand.

| binary | cases failing from the root at S310, every red `Cannot open` |
|---|---|
| `test_evaluation` | 17 of 52 |
| `test_movegen` | 5 of 22 |
| `test_eval_trace` | 6 of 10 |
| `test_leaf_walk` | 4 of 8 |
| `test_eval_model` | 4 of 21 |
| `test_invariants` | 1 of 2 |

Also seen and left by S310: `CAPTURE(tuner_optimiser::kind_name(kind))` in the hold case prints an address and not the name, so a failure there does not say which optimiser it is. Filler for the same reason.
