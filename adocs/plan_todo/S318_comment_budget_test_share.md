id:         S318
goal:       `test_comment_budget` stops costing the fast suite a third of its length on every commit, without losing what guards the tool
accepts:    what the fast label needs of it, the lexer fixtures at least, stays fast; the rest runs where the tool changes or in `tools/gate_extra.sh`; each of the 78 cuts is still killed somewhere named; no test relaxed; both builds green
touches:    tests/CMakeLists.txt, tests/test_comment_budget.py, tools/gate_extra.sh, DEV_MANUAL.md
excludes:   any change to what the tool does
decisions:  DEC-339, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Read 2026-11-01: 125 cases, about 68 s per build on the MacBook against a fast suite of about 200 s before it, and it runs on every commit of every step. Filler by DEC-171: it moves no score, move or line.
