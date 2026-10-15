id:         S285
goal:       the extra-ply mate row ("mate the extra ply hides" in tests/test_search.cpp) separates again on the tree it guards, re-mined by S095's procedure
accepts:    (1) on the current tree the row's cells read, for the shipped build and the build with S095's guard opened, the depths at which each finds the mate, recorded, showing the row no longer separates (the precondition, observed, not assumed); (2) the row re-mined by `adocs/data/S095_mine_mate_row.py` over its candidate list, the script's pick taken as given, never chosen by hand, observed green shipped and red with the guard opened; if no candidate separates in the row's range, one band deeper by DEC-288, and none there stops for a ruling; (3) its GOLDEN block names the script, the range and this step; the mate distance confirmed by Stockfish at depth 20 through python-chess per TOOLCHAIN.md; (4) S095's mutant killed by the new row in the mutation recipe; the fast suite green in both builds; the log under `adocs/data/` with a README row
touches:    tests/test_search.cpp (the row and its GOLDEN block), adocs/data/S285_remine_extra_ply.log, adocs/data/README.md
excludes:   any change to search; any other mined row
decisions:  DEC-142, DEC-171, DEC-288
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S023's phase 2 (2026-10-14) found the extra-ply row stale already on the
parent, S261's engine: the shipped build and the build with S095's guard
opened both find its mate at its depth, so the row guards nothing. S023
re-mined it inside its candidate `adb5d78`; S023 verdict 1 read H0 and the
revert put the stale row back. A test defect, not reachable in play, no
score, move or line: filler under DEC-171, scheduled behind the next
strength step and named in every pre-registration while open. S023's
re-mine (`adocs/data/S023_remine_extra_ply.log`) is the model, run on the
reverted tree.
