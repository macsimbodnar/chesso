id:         S240
goal:       chesso's absolute strength is re-measured now, on S088's reference set, so the self-play ledger since 2026-08-18 is read against an outside scale once before the pending order runs on
accepts:    one `./rating.sh` run on the tree S238's verdict leaves, with S088's five anchors from `references.tsv` exactly as installed (md5s checked against the manifest before the first game), the run's settings stated beside S088's (hash, concurrency, time control, opening draw, adjudication) and every difference named rather than assumed away (specs.md's "Measured strength" paragraph lists them); the solved rating per anchor, the five-anchor mean and the anchor spread reported and compared with S088's 2559 and 121.8, the resolution stated before the result is read (about +/-60 from anchor choice and statistics alone, per specs.md); the forfeit rate per engine against DEC-075's 1 %; whatever it returns recorded in `adocs/data/rating_<date>_S240_ccrl_blitz.md`, including a figure inside the forecast band; the run detached with a terminal marker and a self-terminating watcher; `specs.md`'s measured-strength paragraph updated to the new figure with S088's kept as history
touches:    adocs/data/, adocs/specs.md, adocs/plan.md
excludes:   changing the reference manifest or the rating procedure (S087, S088); the second time control and the near-3000 claim, which stay S152's (DEC-108 as amended by DEC-234); any conclusion about an individual patch, which a gauntlet cannot attribute; installing engines, which is the owner's
decisions:  DEC-072, DEC-074, DEC-075, DEC-077, DEC-108, DEC-204, DEC-234
closes:
blocks:
paused_by:
done:

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
