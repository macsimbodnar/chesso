id:         S335
goal:       S332's three low findings in the corpus merge and its reader are answered: a part-1 game with no row is counted against `GAMES`, a malformed field is refused cleanly, and a stop before the merger's rename is described as one
accepts:    each finding has a fixture seen red at `S332`'s landing and green after: (1) `S122_corpus.sh merge` checks the merged pool's game count against `--games` counting a part-1 game that wrote no row, or states why the count is right as it is; (2) `S331_measure.py read` on a non-number field exits with a one-line refusal naming the file and line, not a traceback; (3) the interrupted-merge check tells a stop before the merger's rename from one after it, by the `.merging` files; `S332_merge_smoke.sh`'s 15 cases still pass and S331's 2400-game reading stays byte-equal
touches:    adocs/data/S331_measure.py, adocs/data/S122_corpus.sh, adocs/data/S332_merge_smoke.sh, adocs/data/S335_*, adocs/data/README.md
excludes:   `src/`; `tools/`; regenerating any corpus; any fit
decisions:  DEC-171, DEC-296, DEC-349
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-05 by S332's implementer ("Findings beyond the step" in S332's done file), each rated low: none occurs
in either real pool, and none moves a label. (1) A merged pool's `GAMES` counts part 1's games only where they wrote a
row, so a part-1 game with no row would make the pool hold more games than `GAMES` says; S332's merge keeps such
games and counts them apart. (2) `read` ends on a traceback (exit 1) for a non-number field, as before S332. (3)
Before S332: the interrupted-merge check also matches the merger's `.merging` files, so a run stopped before the
rename is described as stopped after it; the manual recovery is the same. Filler by DEC-171.
