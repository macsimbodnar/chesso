id:         S198
goal:       `fastchess.sh` seeds the opening order and records node counts and clock margins in the PGN, and one A/A shows the distribution unchanged
accepts:    `-srand <seed>` is passed, the seed derived from the run stamp unless `SRAND` overrides it, and printed in the banner so a run's opening sequence is reproducible; `-pgnout` gains `nodes=true timeleft=true`; `tests/test_fastchess_script.sh` gains a case for each; one fixed-rounds `AA=1` run of 1000 games -- the DEC-143 calibration that follows the machine change to the workstation doubles as this step's A/A -- is read with `adocs/data/S105_pairs.py` and `tools/forfeit_report.py`, its pair variance and forfeit rate recorded in this file beside S105's; `DEV_MANUAL.md` "Play games" says what the PGN now carries and how a run's openings are reproduced; no bound, control, book or adjudication setting moves
touches:    fastchess.sh, tests/test_fastchess_script.sh, DEV_MANUAL.md, adocs/data/
excludes:   any change to the bounds, the time control, the hash, the book or the adjudication
decisions:  DEC-139, DEC-143
closes:
blocks:
paused_by:
author:
done:

## Why this exists

R11(b) of `adocs/testing_strategy.md`. fishtest and OpenBench both pass a
seed and `order=random` so a run is reproducible and two runs on one change
share their openings; the PGN fields let a census read node counts and clock
margins without a log at `trace`. Both are `fastchess` 1.8.1 options on this
machine. The A/A is DEC-143's rule applied for the first time: the workstation
is a new machine for the harness, and the calibration it owes is the run that
also shows the seed changes nothing about the pair distribution.

## Cost

Two flags, two script cases, a manual paragraph; the run is the workstation's
calibration, about 25 minutes there.
