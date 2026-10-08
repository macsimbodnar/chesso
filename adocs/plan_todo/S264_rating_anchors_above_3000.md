id:         S264
goal:       the rating reference set brackets about 2750 to 3150 on the CCRL Blitz 1CPU scale, so S152's run can read a rating near 3000 without extrapolating past its top anchor
accepts:    (1) at least two engine families with 1CPU entries between about 2850 and 3200 join S088's five anchors; each candidate's CCRL Blitz rating, interval and game count read on the day from the list and recorded with the read date; licence noted; (2) each binary built or downloaded by the agent (DEC-069), its md5 and version string recorded in `references.tsv`; an engine that needs a network file is acceptable as an anchor -- running another engine's binary is a tool use (DEC-016) -- and the file's md5 is recorded with it; (3) one smoke match per new anchor under `rating.sh`'s own settings on the workstation: 0 crashes, 0 illegal moves, forfeits within DEC-075's 1 %, the console trimmed by `tools/trim_console.py`; (4) the cost of S152's two gauntlets with the larger manifest is recomputed (`rating.sh` plays 334 rounds, 668 games, per pairing) and stated in S152's file; (5) no rating is read from the smoke matches (DEC-108)
touches:    references.tsv, rating.sh only if an anchor needs an option, DEV_MANUAL.md, adocs/data/ for the anchors record, adocs/plan_todo/S152_rating_checkpoint_after_search_block.md for the cost line
excludes:   changing `rating.sh`'s regime; the rating run itself, which is S152
decisions:  DEC-258, DEC-016, DEC-069, DEC-072, DEC-075, DEC-077, DEC-108, DEC-234
closes:
blocks:
paused_by:
author:
done:

## Why

S240's top anchor, Leorik 2.4 at 2830, still outscored chesso, and its
record says the next read wants an anchor above about 2850. A 3000 reading
solved from anchors that all sit below 2850 is an extrapolation, and DEC-077
already measured scale compression across this set.

## Candidates

Direction only; ratings are re-read on the day. A family with documented
entries across the band is preferred (the engine-testing guide recommends
Stash as a gauntlet family: S088 already uses Stash v21.0). Other families
in the 2900 to 3150 band on the list read 2026-10-07: CuckooChess,
Gaviota, Crafty, DiscoCheck, Zurichess.

## Lane

Agent work; the smoke matches hold the workstation for minutes and run
between verdicts. Ported from the 2026-10-07 proposal's P04.
