id:         S336
goal:       S122's corpus preparation drops a later stage's "before" record when an earlier stage runs, and `seeds` does not read the corpus's own preparation as a meeting
accepts:    a stage that runs removes the records of every stage after it (`split.txt`, `k.txt` and their markers), so that with `verify.txt` and the four markers removed by hand and `split.txt` left, `all` on another pool no longer reads "split before", seen red on a fixture at S333's landing; `S122_corpus.sh seeds` passes on the current tree with the stamped `.tuning/s122/prepared/` in place (today it refuses, "the corpus meets itself", reading the corpus's settings line from `prepared/verify.txt`), while still refusing a planted range that meets the corpus; `bash -n`, `smoke` and `adocs/data/S333_guards.sh green` green
touches:    adocs/data/S122_corpus.sh, adocs/data/S333_guards.sh, adocs/data/S336_*, adocs/data/README.md
excludes:   `src/`; `tools/`; re-running the preparation over the 150000-game pool; any fit
decisions:  DEC-171, DEC-349
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-06 by S333's implementer ("Findings beyond the step" in S333's done file). (1) `split.txt` and `k.txt` are
bound to their pool only through the records beside them: removing `verify.txt` and the markers by hand and running
`all` on another pool still reads the old split. Low, fails only after a manual deletion. (2) `seeds` skips `POOL`,
`TOPUP` and `SAMPLE_DIR` but not `OUT`, so the corpus's own settings line in `prepared/verify.txt` is read as another
run meeting the corpus, and `seeds` refuses. Predates S333; a top-up or a fresh side sample for S124's or S126's fits
would meet it first. Filler by DEC-171: no play, no reported score.
