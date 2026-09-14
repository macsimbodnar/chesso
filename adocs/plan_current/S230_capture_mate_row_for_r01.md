id:         S230
goal:       the "pruning does not hide a forced mate" table regains a mined position whose line runs through a losing capture and separates S091's R01 mutant (the extra reduction ignoring gives-check) at a measured depth, after S222's ordering moved the row that did
accepts:    one new row in `tests/test_search.cpp` "pruning does not hide a forced mate", a position mined from the repository's own sets (the S145 sets or the S219 A/A corpus) by the rule the table states -- the shipped build reports the mate at the row's depth and the R01 mutant does not -- verified with Stockfish through python-chess (the oracle's line recorded), observed red under `tools/mutants/S091_capture_see.py`'s R01 by hand and reverted; the depth read as a measurement over depths 3 to 12 the way DEC-209's clause 4 describes; the other rows' labels unchanged; no `src/`; `No functional change`
touches:    tests/test_search.cpp, adocs/data/
excludes:   any change to S091's rules or to the mutant; re-picking an existing row's depth
decisions:  DEC-209, DEC-141, DEC-142
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-14 16:16, filler while S091 waits for its SPRT night and S222 for its SPSA night
done:

## Why this exists

S222 (2026-09-14) moved the depth at which one S091 mate row separated its
mutants, and the table's own rule -- the depth is where the shipped build
reports the mate and the mutant does not -- removed the row rather than
re-pick it. R01's kill survives in S091's direct guard "a capture that gives
check is not reduced"; the mate table, the recurring-bug guard of this engine,
lost its second witness. Filler behind S229 (DEC-171): no reach in play.

## Cost

Agent work, an hour or two of mining and oracle checks; no run.
