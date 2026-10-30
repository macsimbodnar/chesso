id:         S299
goal:       `adocs/data/S121_cost.sh` and `adocs/data/S135_cost.sh` check every command their INV-6 streams run, as S123's does, so a failure common to every binary cannot pass the diff; and `S121_screen.sh` and `S123_screen.sh` refuse an empty or non-numeric K probe in `part()`
accepts:    each script's `streams()` checks the exit status of `bench`, `bench 12` and `tools/search_bench.py` at both depths, requires each bench stream to end on its total line and each search_bench stream to print its three positions, and keeps raw output out of the directory `run` compares; each script refuses a planted failing engine (exit 1, or junk output) and still passes its own zero against parent on real binaries, shown without a timing; `check` still prints its DONE marker at the shas each script pins; each screen script's `part()` refuses an empty or non-numeric `full.k` before any fit, shown on a planted empty and a planted junk file without a fit
touches:    adocs/data/S121_cost.sh, adocs/data/S135_cost.sh, adocs/data/S121_screen.sh, adocs/data/S123_screen.sh, adocs/data/README.md
excludes:   any timing run; any change to S123_cost.sh, which already has both fixes (S123's phase 1 fast check and `1a231dd`)
decisions:  DEC-047, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

S123's phase 1 fast check found that `streams()` in its cost script did not
check the commands it ran. A `bench` or `search_bench.py` failing the same
way for every binary would write the same error text everywhere and pass
the INV-6 `diff -r`. S123 fixed its own copy, then found the fix's second
defect, raw timings in the compared directory, at its first run (`1a231dd`).
S121's and S135's copies have the first gap. They are pinned to their
steps' shas and are re-run only to re-time those steps, so the defect is
not reachable in ordinary play, on the UCI surface, or in a reported score
(DEC-171). Filler, behind the next strength step. Filed by the coordinator,
2026-10-30.

**Widened 2026-10-30 (S125's phase 1 fast check).** `part()` in
`S121_screen.sh` and `S123_screen.sh` writes the K probe's `full.k` by `sed`
and passes `--k "$(cat ...)"` unchecked. An empty file passes `--k ""`,
which `atof` reads as 0, and 0 means "fit K from the data": a bisection
part would silently leave the pre-registered K. Both steps closed at H1, so
neither `part()` has run or is due to run; S125's copy is fixed in its own
phase 1.
