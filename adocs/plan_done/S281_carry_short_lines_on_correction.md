id:         S281
goal:       the short mate lines S099's correction table adds to the S170 grid are classified, and any that a reporting fix closes are closed, so the carry ceilings S099 raised (A, E, F) come back down by script
accepts:    (1) on the tree after the correction family (S099, and S110 and S111 if they land), every short line `adocs/data/S203_case_sweep.sh` finds in case A's, E's and F's cells is reproduced deterministically and classified by S202's method (which walk or fallback publishes it, and why the line stops short of the distance its score claims); (2) a class a reporting-only fix closes is fixed with INV-6 proved (`bench` and `search_bench` node-identical, same best moves), a minimized failing test first; (3) the six ceilings are re-derived by `--ceilings` and `tests/test_mate_carry.cpp` takes what the script answers; (4) a class no reporting fix closes is recorded with its reason, and the ceilings stay where the script puts them
touches:    src/search.cpp (the PV completion walk, reporting only), tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv
excludes:   any change to what the search finds or scores (that is a strength step, SPRT); the correction table itself
decisions:  DEC-284, DEC-272, DEC-251, DEC-122, DEC-142
closes:
blocks:
paused_by:
author:
done:       2026-10-14 -- VOID, not run. Conditional on S099's H1 (DEC-284); S099 read H0 (`adocs/data/S099_sprt.log`, nElo -4.71 +/- 6.84, LLR -2.96) and `8758622` is reverted whole, so the short mate lines it was to classify leave with the correction table and the carry ceilings return to S261's 0, 5, 4, 0, 0, 1 by the revert. Nothing was implemented; DEC-284 is moot. Moved by the coordinator.

## Why this exists

S099's phase 2 raised test_mate_carry's ceilings above S261's as the script
answered: A 0 -> 2 and E 0 -> 1 (DEC-284, as amended after the fast check's
gate fix), and F 1 -> 2 (DEC-272). Before that fix E's own 300000-node cell
was 4 of 4 lines short, which shows how far one change to the tree can move
this residue. S202 closed 14 of 33 short lines with two reporting-only fixes
on an earlier tree (DEC-251); the same method may close some of these.
Conditional on S099's H1: on H0 the revert takes the ceilings out and this
step is void. Placed after S110 and S111 because each adds a correction table
that moves the grid again.
