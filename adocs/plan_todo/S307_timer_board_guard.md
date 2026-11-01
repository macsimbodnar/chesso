id:         S307
goal:       the timer case in `tests/test_engine.cpp` establishes that its board's first iteration really outlasts the bound, so a dead timer cannot pass on a lighter board
accepts:    the case, or one beside it, asserts its precondition from something a run produces, with the margin stated; a mutant that disarms the timer is killed by it, and a second that lightens the board fails the new assertion rather than passing, each observed; `stop_search_after_ms` is named by a mutant; the added time in the fast suite is stated and the cheaper form chosen if two would do
touches:    tests/test_engine.cpp, tools/mutants/
excludes:   moving the 402 ms bound; any change under `src/`; the comment work S308 holds
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-01 by S306. The case asserts `elapsed_ms <= 402` and a playable move on a board whose
first iteration takes about 6.75 s and 35516352 nodes with the timer disarmed, about 17 times the bound.
Nothing asserts that figure, and it moved from 25933707 with no step noticing. A later change that made
that iteration short would leave the case green with a dead timer. `AGENTS.md`'s TESTS rule asks a test
that a thing does not happen to establish first the precondition that would make it happen, and no
mutant names `stop_search_after_ms`.

The obvious precondition costs 6.75 s in every fast run, so the step weighs a node floor from a cheaper
reading against it. Filler by DEC-171: the shipped timer works on this board, with 1.9 to 2.1 ms over
20 runs in S306's table.
