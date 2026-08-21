id:         S152
goal:       the engine's absolute rating is re-measured once after the search block, before the speed block, so 45 to 55 verdicts are not accumulated without an end-to-end check
accepts:    one `rating.sh` run after S132, the last step of block 1, before block 2 begins, with the solved rating and the anchor spread both reported and compared against S088's 2559 +/- 25 and its 121.8 Elo spread; the run's resolution is stated before it is read, because +/- 25 plus the family spread means it detects drift above roughly 50 Elo and nothing smaller, and the search block's own forecast of +180 to +280 is the quantity it is sized against; whatever it returns is recorded, including "inside the forecast band", which is the expected outcome and a valid one; it is detached with a terminal marker and a self-terminating watcher
touches:    rating.sh, adocs/data/, adocs/specs.md, adocs/plan.md
excludes:   S128, which asks a different question -- whether the anchor spread is scale compression from the control -- and which stays where it is; changing the reference manifest or the rating procedure, which S087 and S088 fixed; any conclusion about an individual patch, which a gauntlet cannot attribute
decisions:  DEC-019, DEC-020, DEC-077
closes:     2026-08-21_adversarial-F04
blocks:
paused_by:
done:

## Why five hours is the right price

The plan's own budget is "roughly 75 to 110 machine-hours" and S088's run cost
5 h 02 m 49 s, so this is about 5 %. S128's file defers the rated run because it
"buys zero Elo"; what it buys is the only check that the summed per-patch deltas
are the engine's strength. The plan already takes "a fifth off for interaction",
which is an estimate nothing measures, and DEC-020 is the case where an
unattributed run reported +301 Elo and meant nothing.
