id:         S291
goal:       find by tool why S083's re-landed constants lose the multicut mined row's mate at depth 10 (found at 9, 11 and 12), and fix the rule at fault as its own measured change if one is
accepts:    the node and the rule that turn `tests/test_search.cpp`'s "mate the multicut hides, depth 10" (`6k1/5pp1/6np/1PP1p3/1Q2P1P1/3r1q1P/1B1R4/6K1 b - - 0 42`, mate in 5) into a non-mate score at depth 10 on S083's re-landed tree (`../chesso-s083r`, bench 3945487) are named from the engine's own output (a trace, a counter, one tune option at a time), never from reading the board (CHESS); the same mechanism's reach at the shipped weights (`cd50178`'s tree) is checked on the mate sets S289 used; if a rule is at fault and reachable at the shipped weights, its fix ships as its own change with a guard test observed red first, a mutant it kills, Debug self-play, gate_extra, every golden it moves re-derived by its script, and an SPRT against its parent; if no rule is at fault, the attribution is recorded for the owner's H1 ruling under DEC-308; the fast suite green in both builds
touches:    adocs/data/S291_*; this file; src/search.cpp and tests/ only if a rule is at fault
excludes:   S083's constants; any lowering of a mate floor; re-mining the row (DEC-308)
decisions:  DEC-308, DEC-307, DEC-306, DEC-305, DEC-171, DEC-158, DEC-142, DEC-141
closes:
blocks:     S083
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-22, DEC-308)

S083's re-landing on S289's guard (`.tuning/coord/S083/s083_reland.diff`)
reads the multicut mined row's mate at depths 9, 11 and 12 and not at 10,
where the shipped engine reports it (`adocs/data/S083r_rows.txt`); a re-mine
by S097's commands over 7..10 found no row separating E21 on that tree
(`adocs/data/S083r_remine_multicut.log`). S289 attributed the paste's lost
mined mates to reverse futility's and the multicut's known price (DEC-158),
but not this row. The machine is held by S289's SPRT until its verdict; this
step runs after it and before S083's own run. S289's trace tooling
(`adocs/data/S289_trace.py`) and its reach method are the precedent.
