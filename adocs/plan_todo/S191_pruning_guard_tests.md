id:         S191
goal:       every null-move, reverse-futility and late-move-reduction guard has a direct test with a precondition that the guard's condition holds at the node, and the S165 defender set is a registered fixture
accepts:    cases, each with its precondition asserted before the search: an in-check node makes no null move; a pawn-only position (`game_phase` 0) is searched with no null move; every defender node of `adocs/data/S165_defender_set.tsv` with beta inside the mate band takes no null-move cutoff, scored over the whole set with zero tolerance; `prev_move == 0` forbids a second consecutive null; a checking move and a capture are searched at `child_depth` with no reduction; a reduced move that beats alpha is re-searched at full depth; reverse futility does not fire in check, at a PV node, above `RFP_MAX_DEPTH` or inside the mate band; each case observed red under the matching mutant of `adocs/data/2026-09-04_test_review/mutants.py` (M01 to M04, M07 to M09) in a binary other than `test_mate_carry`, then green; the defender TSV is read through `CHESSO_SOURCE_DIR` like the S145 sets; `DEV_MANUAL.md` "Mate safety" lists the defender set as the fifth instrument; fast suite green in both builds
touches:    tests/test_search.cpp, tests/CMakeLists.txt, src/search.hpp, DEV_MANUAL.md, adocs/specs.md
excludes:   changing any guard; the four S109 rules, which arrive with their own cases under DEC-141
decisions:  DEC-139, DEC-141
closes:     2026-09-04_test_review-F02
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F02`. No test exercises the null-move guards at
`src/search.cpp`'s null-move block or the reduction guards in its move loop.
Five guard-removal mutants -- null move in check, the S165 negative mate-band
guard dropped, null move in pawn endings, a null-move mate score returned as
real, LMR reducing captures -- were each caught by exactly one test,
`tests/test_mate_carry.cpp`'s per-game mate-line floor, and by nothing else;
three of the five leave the depth-9 bench identical too. `adocs/data/S165_defender_set.tsv`
holds 104 proved defender nodes built to measure exactly the mate-band guard
and is read by nothing in `tests/`. Ordered before S109, whose four rules then
ship with their own cases and mutants (DEC-141).

## Shape

The S145 fixture style (`tests/test_engine.cpp`'s mate-safety suite: a TSV
read through `CHESSO_SOURCE_DIR`, a precondition per row, a count with zero
tolerance) for the defender set; single constructed positions, tool-verified
(CLAUDE.md, CHESS rule), for the others. `src/search.hpp` is in `touches:`
only for a test hook where a guard's firing cannot be observed from outside
-- a counter or a probe in the tune build, as `search_lmr_reduction_probe`
already is.

## Cost

Machine-free, about a day.
