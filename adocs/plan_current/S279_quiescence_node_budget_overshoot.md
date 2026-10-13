id:         S279
goal:       `go nodes N` never searches more than N nodes: quiescence checks the limit as soon as it counts its node, as `negamax_at` does, so a table cutoff can no longer count the node that reaches the budget without aborting
accepts:    (1) the defect reproduced first with a minimized failing test on achesso's head, observed red, not assumed -- a position and budget where quiescence's table cutoff counts the budget's last node and the search reports N+1 (S261's probe `.tuning/coord/S261/nodes_over.py` found 30 of 1028 budgets over on `fd9e44c`); (2) the cause stated from the code: `quiescence` increments `explored_nodes` and reaches `check_limits` only after its early exits, so the stop flag's 2048-node poll is skipped the same way -- say whether that poll skip is real and what the fix does to it; (3) the fix makes the limit exact -- the minimized test green, the probe's 1028 budgets 0 over, `test_search`'s "the node budget is never exceeded" and "an aborted search reports no move at all" green; (4) what moves is proved, not argued: `chesso bench` (fixed depth, no budget) and `tools/search_bench.py`'s node counts and best moves identical to the parent (INV-6), and the budgeted searches that move are named; (5) whether the change needs an SPRT is proposed with its reason and the precedent found in `adocs/decisions.md` and `adocs/plan_done/` (a stop-path fix whose timed-play effect is the stop flag's poll granularity) -- the coordinator decides; (6) the goldens a node budget drives are re-derived by their scripts where they move (DEC-142): the S170 carry cases and their ceilings (`adocs/data/S203_case_sweep.sh`), any `go nodes` count in `tests/`; the fast suite green in both builds, format clean, DEC-141's second tier (it touches the search)
touches:    src/search.cpp, tests/test_search.cpp or tests/test_engine.cpp, adocs/specs.md and MANUAL.md if the `nodes` wording moves, goldens a node budget drives
excludes:   S261's vector (paused behind this step); any other change to quiescence's order of work; the `info` reporting of an aborted iteration (S195, S210)
decisions:  DEC-171, DEC-141, DEC-142, INV-6
closes:
blocks:     S261
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S279_brief.md), 2026-10-13
done:

## Why this exists

Found 2026-10-13 by S261's phase 2: on the lane's vector `test_search` "the
node budget is never exceeded" and "an aborted search reports no move at all"
read budget 4096, explored 4097. The vector only moves which budget lands on a
quiescence table hit: over UCI `go nodes N` for 1028 values of N reports N+1
in 30 searches on the incumbent `fd9e44c` and 22 on the vector. A throwaway
build with `check_limits` right after quiescence's increment turned both cases
green. It is on the UCI surface, so DEC-171's first clause puts it before
anything else; S261 is paused behind it, and the carry family S261 measured
at node budgets is re-measured after it lands.
