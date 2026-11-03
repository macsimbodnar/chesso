id:         S326
goal:       `adocs/data/S135_gradient_rows.cpp` re-derives the fixture pair it picked, and says so, at the tree it runs on
accepts:    the program takes as its base the fixture as it stood before S135's pair (a stated row count, checked against the pair's own marker in `tests/test_eval_positions.hpp`), applies DEC-316's rule unchanged, and prints whether its pick is the landed pair, exiting non-zero when it is not, in `S101_gradient_rows.cpp`'s shape; run at `HEAD` and at S135's landing commit, each reading stated; if the rule at `HEAD`'s constants picks another pair, that is reported as a finding for the owner (DEC-142: re-derived, never re-read), not fixed by editing the fixture; both fast suites and `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` green
touches:    adocs/data/S135_gradient_rows.cpp, adocs/data/README.md
excludes:   any change to the fixture, a case, a bound, `h` or DEC-316's rule; the other five re-derivers
decisions:  DEC-142, DEC-171, DEC-316
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-03 by S324's implementer. The program takes the whole landed fixture as its base, so S135's own pair
(rows 41 and 42 of 106) counts as known. It then picks the first candidate, a row the fixture does not hold, and exits 0,
because it has no check that its pick is the landed pair. That holds before S324 and after it. So the golden pair
`test_tuner_gradient`'s derivative case reads is, today, a golden its script does not re-derive, which DEC-142 makes a
finding. Filler by DEC-171: the case is green and no reported score, move or line moves.
