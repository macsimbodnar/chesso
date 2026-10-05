id:         S254
goal:       every mutant whose `old` string names `is_check_move` matches the tree again, so `tools/mutation_check.py` applies it instead of refusing it
accepts:    `tools/mutation_check.py` applies every mutant in `tools/mutants/` on today's tree with no "old string not found" refusal; S091's C02 and R01, `tools/mutants/S109_shallow_pruning.py` and `tools/mutants/search.py` read killed or survived (a survivor named as a finding), not refused
touches:    tools/mutants/*.py
excludes:   any engine or test change beyond a case a survivor calls for, which is its own step
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
done:

## Why this exists (2026-10-05, the coordinator, from S055's report)

S020 turned `is_check_move` into a memoised call, `is_check_move()`, and the
mutants' `old` strings were not updated: S091 C02/R01,
`tools/mutants/S109_shallow_pruning.py` and `tools/mutants/search.py` no
longer match, so `mutation_check.py` would refuse them and DEC-141's mutant
guard is silently thinner. S055's sweep driver applied C02 and R01 with the
call form by hand. A tooling defect reaching no play: a filler (DEC-171).
