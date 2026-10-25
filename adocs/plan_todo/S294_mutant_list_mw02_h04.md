id:         S294
goal:       the mutant list's two off-list verdicts on the shipped tree are closed: MW02 is killed by a guard that still reaches its line, and H04 builds
accepts:    `tools/mutation_check.py --only MW02_aborted_line_no_fallback,H04_cont_hist_unread` on the tree this step leaves reads both `killed`; for MW02 the case that kills it is named in `tools/mutants/S202_mate_walk.py` with why the old killer (`tests/test_mate_carry.cpp`'s F_mate6_inherited_no_line cell, ceiling 0) stopped reaching it, read from the engine's output, not assumed; for H04 the mutant compiles under the tree's warnings (`-Werror=unused-parameter`) without changing what it removes; no assertion loosened (TESTS); the fast suite green in both builds
touches:    tools/mutants/S202_mate_walk.py, tools/mutants/S222_continuation_history.py, tests/ (a guard case or a cell for MW02, if one is needed), adocs/data/README.md, DEV_MANUAL.md if a golden moves
excludes:   any change in src/; the S170 budgets and ceilings (DEC-299); S290's picker
decisions:  DEC-141, DEC-142, DEC-171, DEC-313
closes:
blocks:
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-25)

S083's mutation pass (`adocs/data/S083s_mutation.tsv`, 231 mutants on its
ruled tree) read 224 of 225 killed with two verdicts off the list:
`MW02_aborted_line_no_fallback` (S202) survived and `H04_cont_hist_unread`
(S222) was stillborn, `-Werror=unused-parameter` stopping its build in
`src/evaluation.hpp`. Both read the same with `--only` at the shipped
`e972bd4`, so neither is S083's. The fixes they guard are in the code; what
lapsed is the guard's reach, a test-coverage gap with nothing reachable in
play or on the UCI surface. A DEC-171 filler behind the next strength step,
named in the pre-registration of every run taken while it is open.
