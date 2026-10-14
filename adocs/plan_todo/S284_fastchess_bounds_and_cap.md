id:         S284
goal:       `fastchess.sh` takes an SPRT's bounds and a round cap that keeps the SPRT from the environment, so a pre-registration can state both and the run plays them
accepts:    (1) `fastchess.sh` reads `SPRT_BOUNDS` ("elo0=<x> elo1=<y> alpha=<a> beta=<b>", each field validated, refused when malformed) and `SPRT_ROUNDS` (an unsigned round cap; the SPRT stays on), refuses `SPRT_ROUNDS` beside `ROUNDS`, and refuses either beside a mode argument (`--fast`, `--nonreg`); (2) the banner prints the bounds and, when `SPRT_ROUNDS` is set, the cap; (3) `tests/test_fastchess_script.sh` gains a case per new path and per refusal, observed red before the change where a case can be, and one case showing the default path's fastchess command line byte-identical to `4e6ca63`'s; (4) the header comment and `DEV_MANUAL.md`'s SPRT section state the two variables, traced to the lines that consume them; (5) it lands only after S023 verdict 1's run ends; no DEC-143 A/A is owed, because no played setting moves, and the stamp says so
touches:    fastchess.sh, tests/test_fastchess_script.sh, DEV_MANUAL.md
excludes:   any change to a mode's bounds, rounds, time control, book, adjudication or concurrency; the run itself (S283)
decisions:  DEC-289, DEC-286, DEC-143, DEC-020, DEC-061
closes:
blocks:     S283
paused_by:
author:
done:

## Why this exists

S283's pre-registration (DEC-286) cannot reach `fastchess.sh` at `4e6ca63`:
the mode argument fixes the bounds, and `ROUNDS` drops the SPRT. Found by
S283's phase-1 agent (2026-10-15), checked by the coordinator by reading the
mode block (`fastchess.sh` around lines 331-378). DEC-289 rules the step.
