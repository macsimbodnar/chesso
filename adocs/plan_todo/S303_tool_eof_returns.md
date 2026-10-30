id:         S303
goal:       `tools/depth_vs_eval.py` and `tools/spsa_driver.py` stop with an error when the engine closes its output, instead of returning a placeholder move or a partial node count
accepts:    `depth_vs_eval.py`'s `best_move()` raises at end of file instead of returning `-`, and `spsa_driver.py`'s `probe_nodes()` raises instead of returning a partial count, each with a message naming what it waited for, shown on a planted stand-in engine that exits and one that closes stdout and stays alive; each tool's output on the real binary is unchanged (nodes and moves equal, timings stripped); every caller of either function is checked and none depends on the old return; no timing is taken
touches:    tools/depth_vs_eval.py, tools/spsa_driver.py
excludes:   any change to a tool's protocol, output or workload; the frozen scripts under `adocs/data/`; any timing run
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

S301's subagent found two more end-of-file returns in live tools.
`depth_vs_eval.py`'s `best_move()` returns `-` when the engine closes its
output, so a dead engine reads as a move. S301's fast check traced the
worst case: if the engine closes during a worker's last deep search,
`cost_of` sends `position fen ... moves -` to the reference, costs the row
from the wrong position, and the run exits 0. `spsa_driver.py`'s
`probe_nodes()` can return a partial node count, which an SPSA run would
take as a measurement. Both can pass a wrong number from a tool. Neither is reachable in ordinary play or on the UCI
surface, and neither moves the engine's own score, move or line (DEC-171).
Filler, behind the next strength step. Filed by the coordinator,
2026-10-30.
