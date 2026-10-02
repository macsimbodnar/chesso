id:         S252
goal:       mutant M06a (reverse futility's ply floor at 2) is killed by a fast-suite case again, or declared equivalent with a stated proof
accepts:    `tools/mutation_check.py --only M06a_rfp_ply_floor_minus1` on a fixture of the tree of the day reads killed, by a case that establishes its premise first (a node at ply 2 that reverse futility would prune and a mate or a count that separates), or the mutant is declared equivalent in `tools/mutants/search.py` with the reason it cannot be distinguished; S145's mate set and S156's sweep are the first places to look; both fast suites green; no assertion relaxed
touches:    tests/test_search.cpp or tests/test_engine.cpp (a case), tools/mutants/search.py (only for an equivalence declaration)
excludes:   `RfpMinPly`'s range (S251), any engine change
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-10-02, the coordinator)

S116's mutation run (finding 2) found `M06a_rfp_ply_floor_minus1` surviving on
S116's candidate and on its parent `f82e5a3` alike
(`.tuning/coord/S116/mutation/parent_M06.log`): no fast-suite case separates
reverse futility at ply 2 from ply 3 any more, so the guard that floor protects
is unfenced at that edge. A test property reaching no play: a filler behind
S116 (DEC-171), item 21 of `adocs/data/S116_sprt.sh`'s open findings.
