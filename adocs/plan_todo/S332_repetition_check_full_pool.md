id:         S332
goal:       the `repetition:` check reads a full pool, one run or merged, without holding its rows in memory
accepts:    `adocs/data/S331_measure.py` `read` streams the views, or reads `games.tsv` and the root view alone, and its peak resident memory on S122's regenerated 150000-game pool (`.tuning/s122/corpus/`, read in place) is stated from a run, under 2 GB; its reading on that pool is recorded and agrees with the log's `repetition:` line, or the disagreement is the finding; `S122_corpus.sh merge` writes the merged pool's games file and one combined `repetition:` line beside the two parts' (DEC-296), and a smoke merge of two small runs shows `read` accepting it; S331's 2400-game reading re-run byte-equal by the new reader
touches:    adocs/data/S331_measure.py, adocs/data/S331_measure.sh, adocs/data/S122_corpus.sh, adocs/data/S332_*, adocs/data/README.md
excludes:   `src/`; `tools/datagen.cpp`; regenerating any corpus; any fit
decisions:  DEC-171, DEC-296, DEC-349
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-04 by S122's DEC-349 implementer ("Findings beyond the task, not fixed" in S122's step file).
`--games-out` exists so a pool's `repetition:` line can be checked against its files, but `read_rows()` keeps every
row of the three views: about 56 M tuples with their FENs on 150000 games, inferred at well over the machine's 15 GB.
And after a stop, `merge` leaves part 1's `games.tsv` and the top-up's apart, part 1's closing lines lost, so a merged
pool has no one file to check. Filler by DEC-171: no run's labels depend on it; it is the check that they are right.
