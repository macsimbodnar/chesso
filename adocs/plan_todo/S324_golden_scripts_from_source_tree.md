id:         S324
goal:       the six golden re-derivers that read `test_eval_positions.hpp`'s fixtures run from any directory, and the build's copy of `tests/assets/` goes once nothing reads it
accepts:    each of `adocs/data/S101_coverage_rows.cpp`, `S101_gradient_rows.cpp`, `S121_coverage_rows.cpp`, `S123_coverage_rows.cpp`, `S125_coverage_rows.cpp` and `S135_gradient_rows.cpp` gets its fixtures and `test_eval_positions.hpp` from the source tree by its build line (`-DCHESSO_SOURCE_DIR`, as `S122_curve_probe.cpp` does) and prints, from the repository root, from `build/tests` and from a third directory, the same output it printed before from `build/tests` (diffed, byte for byte); every golden it re-derives unchanged (DEC-142: a re-derivation, not a re-read); then `tests/CMakeLists.txt`'s `file(COPY ... assets)` removed, with the search for any further reader (S322's patterns and hits) re-run at landing and empty; both fast suites green with no `build*/tests/assets`
touches:    adocs/data/S101_coverage_rows.cpp, adocs/data/S101_gradient_rows.cpp, adocs/data/S121_coverage_rows.cpp, adocs/data/S123_coverage_rows.cpp, adocs/data/S125_coverage_rows.cpp, adocs/data/S135_gradient_rows.cpp, tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   any change to a golden, a case or what a script computes; refactoring `load_json`
decisions:  DEC-171, DEC-142
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-03 by S322's implementer. S322 put `CHESSO_SOURCE_DIR` on every doctest target by construction, but the copy
of `tests/assets/` in `build/tests` had to stay. Six golden re-derivers under `adocs/data/` (DEC-142) are built by `g++`
without the macro and run from `build/tests`. Each calls `all_test_fens()`, which goes through `load_json`'s relative path. They
also include `../../tests/test_eval_positions.hpp` by a path relative to the working directory, so they run two levels below
the root. Filler by DEC-171: the goldens re-derive today; only the directory they need is fragile.
