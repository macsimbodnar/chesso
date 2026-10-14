id:         S281
goal:       the short mate lines S099's correction table adds to the S170 grid are classified, and any that a reporting fix closes are closed, so case A's and E's carry ceilings come back down by script
accepts:    (1) on the tree after the correction family (S099, and S110 and S111 if they land), every short line `adocs/data/S203_case_sweep.sh` finds in case A's and E's cells is reproduced deterministically and classified by S202's method (which walk or fallback publishes it, and why the line stops short of the distance its score claims); (2) a class a reporting-only fix closes is fixed with INV-6 proved (`bench` and `search_bench` node-identical, same best moves), a minimized failing test first; (3) the six ceilings are re-derived by `--ceilings` and `tests/test_mate_carry.cpp` takes what the script answers; (4) a class no reporting fix closes is recorded with its reason, and the ceilings stay where the script puts them
touches:    src/search.cpp (the PV completion walk, reporting only), tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv
excludes:   any change to what the search finds or scores (that is a strength step, SPRT); the correction table itself
decisions:  DEC-284, DEC-272, DEC-251, DEC-122, DEC-142
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S099's phase 2 on `6fc7f23` raised case A's carry ceiling 0 -> 2 and case
E's 0 -> 4 as the script answered (DEC-284). E's own 300000-node cell is 4 of
4 lines short, so E's case now guards only that a mate line is published at
its budget. S202 closed 14 of 33 short lines with two reporting-only fixes on
an earlier tree (DEC-251); the same method may close some of these.
Conditional on S099's H1: on H0 the revert takes the ceilings out and this
step is void. Placed after S110 and S111 because each adds a correction table
that moves the grid again.
