id:         S240
goal:       chesso's absolute strength is re-measured now, on S088's reference set, so the self-play ledger since 2026-08-18 is read against an outside scale once before the pending order runs on
accepts:    one `./rating.sh` run on the tree S238's verdict leaves, with S088's five anchors from `references.tsv` exactly as installed (md5s checked against the manifest before the first game), the run's settings stated beside S088's (hash, concurrency, time control, opening draw, adjudication) and every difference named rather than assumed away (specs.md's "Measured strength" paragraph lists them); the solved rating per anchor, the five-anchor mean and the anchor spread reported and compared with S088's 2559 and 121.8, the resolution stated before the result is read (about +/-60 from anchor choice and statistics alone, per specs.md); the forfeit rate per engine against DEC-075's 1 %; whatever it returns recorded in `adocs/data/rating_<date>_S240_ccrl_blitz.md`, including a figure inside the forecast band; the run detached with a terminal marker and a self-terminating watcher; `specs.md`'s measured-strength paragraph updated to the new figure with S088's kept as history
touches:    adocs/data/, adocs/specs.md, adocs/plan.md
excludes:   changing the reference manifest or the rating procedure (S087, S088); the second time control and the near-3000 claim, which stay S152's (DEC-108 as amended by DEC-234); any conclusion about an individual patch, which a gauntlet cannot attribute; installing engines, which is the owner's
decisions:  DEC-072, DEC-074, DEC-075, DEC-077, DEC-108, DEC-204, DEC-234
closes:
blocks:
paused_by:
author:     the coordinator, which holds the machine and runs rating.sh itself (a run, not an implementation); started 2026-09-27 06:27
done:       2026-09-27 -- chesso ~= 2766 on the CCRL Blitz scale, five-anchor mean, spread 94.1, +207 over S088 on the same anchors. `./rating.sh` on `1680439` (`bench` 4803214), S088's five anchors md5-checked, 10+0.2, hash 128, concurrency 12, 3340 games. Run 1 void (Blunder 8.5.5 disconnected; recorded, not read); run 2 on an idle machine valid: 0 crashes or disconnects, Stash 3 forfeits (0.45 %, inside DEC-075). Per anchor 2762.0 / 2702.6 / 2794.2 / 2773.1 / 2796.7; without Leorik 2.1 2781.5, spread 34.7. Settings differing from S088 named, not patched (excludes). Record `adocs/data/rating_2026-09-27_S240_ccrl_blitz.md`; `specs.md` updated, S088 kept as history. The forecast of 2600 to 2720 was low. Leorik 2.4 (2830) still scores 54.7 % against chesso, so the set brackets it only just: the next read wants an anchor above ~2850, which is the owner's to install. No `src/` change.

## Why this exists

The owner asked on 2026-09-26 for the current rating to be re-measured next,
on the engines installed on this machine (DEC-234). The last measurement is
S088's, 2026-08-18: about 2559 on the CCRL Blitz scale, soft, 121.8 Elo of
anchor spread. Since then the SPRT ledger has kept eleven gainers and an SPSA
fit whose point estimates sum to roughly +250 self-play Elo; DEC-063's
early-stopping correction and the unmeasured self-play-to-gauntlet factor put
the expected outside gain far below that. This run is the only thing that
reads it.

## Shape

- Machine: the whole machine, nothing else running -- it waits for S238's
  SPRT to finish. About five hours at concurrency 12 (DEC-073, DEC-075).
- Reference set: S088's five, all installed under `/home/max/ws/engines/` with
  md5s matching `references.tsv` on 2026-09-26 -- Blunder 7.1.0 (2389),
  Leorik 2.1 (2568), Blunder 8.5.5 (2664), Stash v21.0 (2713), Leorik 2.4
  (2829). Ratings re-read from the live list at run time by
  `tools/ccrl_rating.py`, as S088 did.
- If the forecast (2600 to 2720) holds, chesso sits in the upper half of the
  set and the top anchor is only about 100 to 200 above it; S088 found the
  solve flattening at the top anchors (DEC-077). An anchor near 2900 to 3000
  would bracket it properly -- **that needs the owner to install one**, and
  this step runs on the set as it is if none is added.
- `rating.sh` at HEAD does not replay S088 exactly (specs.md: hash 128 not
  64, concurrency, no `-srand`, two-sided adjudication since DEC-174 / S212).
  Name each difference in the record; do not patch the script (excludes).

## Cost

About five hours of machine, one night or one day slot.

## Run 1, 2026-09-27 06:27 to 08:58 -- void (the coordinator)

`./rating.sh` from a detached worktree at `1680439` (`bench` 4803214, the
engine S238's removal leaves), S088's five anchors, 10+0.2, hash 128,
concurrency 12, 3340 games in 2 h 30 m 45 s. **`RATING-RUN-DONE rated
INVALID`**: Blunder 8.5.5 disconnected in game 6 (`Blunder 8.5.5 vs chesso`,
"White disconnects", one termination "abandoned"), and the script voids any
crash or disconnect at zero, so no rating was solved. Recorded, not read:
`adocs/data/S240_rating_run1_INVALID_report.txt` and `_forfeits.txt`. Chesso
itself: 0 forfeits, 0 crashes. Stash v21.0 overran the clock 5 times (0.75 %,
inside DEC-075's 1 %). The machine had the S238 removal agent running
beside it at `nice -n 19` for the first hour; its load is named here because
it was present, and the re-run is taken on an otherwise idle machine. **Run 2
follows on the same tree and settings**; the pooled table of run 1 is not
used for any figure.

## Run 2, 2026-09-27 09:33 to about 12:05 -- valid (the coordinator)

Same tree (`1680439`, `bench` 4803214), same settings, on an idle machine
(load 1.13 at launch, nothing else running). **`RATING-RUN-DONE rated OK`**:
0 crashes, 0 disconnects; Stash v21.0 3 time forfeits (0.45 %), every other
engine 0. **Five-anchor mean 2765.7, spread 94.1** (2762.0 / 2702.6 / 2794.2 /
2773.1 / 2796.7); without Leorik 2.1 2781.5, spread 34.7. Against S088's 2558.5
and 121.8: **+207**. The record is `adocs/data/rating_2026-09-27_S240_ccrl_blitz.md`
with the run's report, anchors and forfeits beside it; `specs.md`'s measured
strength paragraph now opens on it, S088's kept below as history. The forecast
band stated before the run, 2600 to 2720, was too low.
