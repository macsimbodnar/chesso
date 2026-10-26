id:         S296
goal:       a node that skipped moves by a shallow-depth pruning rule no longer returns a mated score it did not prove, so the root never reports a mate shorter than the position's; one form chosen by measurement, decided by a {-5, 0} SPRT
accepts:    S295's chain (`adocs/data/S295_trace.txt`, S145 mined line 168) re-taken on the fixed tree: depth 8 no longer reports `mate 4` (the oracle's mate in 5, Stockfish depth 20), read from the engine's output; both forms of DEC-318, (a) a mated fail-low after a skip returns a bound that does not claim the mate and (b) the move-loop's shallow-depth pruning is not applied while no searched move has escaped the mated band, implemented from their descriptions (DEC-221) and measured before one is chosen -- exact mates gained and lost on S145's set, S289's four mate sets, the S170 cells and S294's grid, the census of printed mate distances shorter than the oracle's on those sets, and S292's ordinary-play census (firings, root moves changed, nodes); if neither form loses no mate the shipped tree reports, stop with both forms' figures for the coordinator; a case observed red first on the shipped tree and green after; a mutant it kills (DEC-141, new pruning-adjacent rule); every golden it moves re-derived by its script (DEC-142), the mate floors and S170 ceilings under DEC-299's guard; Tier 2 (DEC-141) run; S295's node-limited cell in `tests/test_mate_pv.cpp` (the `go nodes 140000` cell on S145 mined line 168, picked by `adocs/data/S295_long_line.py`) exists only because of the short `mate 4`, so it is re-derived by its script on the fixed tree or, if no cell separates, retired by a recorded decision, the cut's guard kept by the constructed-lines case (MW03's other killer); both builds and both fast suites green; then the coordinator's `{-5, 0}` SPRT at 8+0.08, Hash 16, against its parent, pre-registered with its three outcomes
touches:    src/search.cpp (the move loop's fail-low return or its pruning conditions); tests/test_search.cpp or tests/test_mate_pv.cpp (the case); tools/mutants/ (one mutant); the goldens it moves (tests/test_mate_breadth.cpp, tests/test_mate_carry.cpp, tests/test_engine.cpp, tests/test_search.cpp, adocs/data/S170_cases.tsv, each by its script); adocs/data/S296_*; adocs/data/README.md; MANUAL.md (the Known bugs entry S295 wrote); DEV_MANUAL.md
excludes:   reverse futility's margins and ceiling (DEC-158); DEC-310's null-move guard; the reporting layer (S295); the S170 budgets except by S170_pick_budgets.sh under DEC-299's guard
decisions:  DEC-318, DEC-317, DEC-310, DEC-171, DEC-158, DEC-221, DEC-141, DEC-142, DEC-299, DEC-143
closes:
blocks:
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-26, DEC-318)

S295 traced the completed iteration's short mate on S294's cell. A non-PV
node at ply 5, depth 3, searched nine replies, all mated, skipped six by
quiet SEE pruning and late move pruning -- among them g2g3 and g2g4, the
only two replies python-chess finds avoid mate in one -- and returned its
fail-soft mated best as its bound. The table carried that bound up to the
root, which reported `mate 4` where Stockfish reads mate in 5. Returning
alpha there in a scratch build restores mate 5 at depth 8. It moves a
reported score, so it is BUGS-binding under DEC-171 (c) and goes before
S136. It changes play, so it is measured alone and decided by an SPRT, as
DEC-310's null-move guard was.
