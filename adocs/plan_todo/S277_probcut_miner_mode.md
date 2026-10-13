id:         S277
goal:       the ProbCut mate-row miner takes a `--mode` as S097's multicut miner does (DEC-238), so its next re-mine reads the guard-mode witness from its own header
accepts:    (1) `adocs/data/S113_mine_mate_row.py`'s successor (a new script; the S113 evidence is never edited) passes `--mode guard|default` through to S097's `fires` and `pick`, default unchanged; (2) run in guard mode on the tree S259 mined, it reproduces `adocs/data/S259_b04_fires_guard.py`'s four moved-depth lists and `mate:nodes` cells exactly, and the pick is S259's row; (3) the ProbCut row's GOLDEN comment in `tests/test_search.cpp` and `DEV_MANUAL.md`'s golden row name the new script (DEC-142); no assertion changes
touches:    adocs/data/ (the new miner), tests/test_search.cpp (one GOLDEN comment), DEV_MANUAL.md (one golden row), adocs/data/README.md
excludes:   any change to src/, to the ProbCut rule, or to the row itself
decisions:  DEC-142, DEC-171, DEC-238
closes:
blocks:
paused_by:
author:
done:

## Why

S259 re-mined the ProbCut row with `S113_mine_mate_row.py`, which has no
`--mode`; its default-mode witness does not move the tree at d11, the row's own
depth -- the case DEC-238 gave S097's miner a guard mode for. S259 took the
guard-mode witness by calling S097's `fires` through S113's module in a
one-off script, `adocs/data/S259_b04_fires_guard.py`. The next re-mine should
not need a one-off. Proposed in S259's As built; filed by the coordinator.

## Lane

Agent work, tooling and one comment; minutes of machine. Filler under DEC-171:
a test-tooling gap that reaches no play.
