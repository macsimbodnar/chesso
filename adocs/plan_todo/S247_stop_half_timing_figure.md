id:         S247
goal:       the eight-queens stop half's recorded timing figure in `tests/test_engine.cpp`'s first-iteration case reads what the idle machine reports, or is named as a record and not a golden
accepts:    the GOLDEN paragraph of the stop half either quotes a figure re-taken on the idle machine with the command that took it, or says the number is the landing-day record and the floor of 3 ms is the only assertion (DEC-240); whichever is chosen is stated with the reason; the fast suite is green in both builds
touches:    tests/test_engine.cpp (a comment), DEV_MANUAL.md if its golden row quotes the figure
excludes:   the floor itself, the board, the timer half; any change to what the case asserts
decisions:  DEC-142, DEC-240, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-09-30)

S022 verdict 2's landing moved the stop half to a heavier board because the
early-out had cut the eight-queens board under the case's 3 ms floor, and
S245's item 2 closed the stale "13.8 ms" figure as moot on that board. The
verdict read as a zero and the removal restored the eight-queens board with
its numbers byte for byte, as the pre-registration's H0 row ordered, so the
"13.8 ms" figure is live again while the board ran 6 to 7 ms on the idle
machine at the landing's ruling. The removal's fast check (2026-09-29)
asked for a filler to re-take the time on the idle machine rather than bend
the byte-for-byte restore. A filler behind S114 (DEC-171): it reaches no
play and moves no reported score.
