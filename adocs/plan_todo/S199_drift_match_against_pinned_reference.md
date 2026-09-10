id:         S199
goal:       a fixed-rounds match against a pinned early-S105 reference is played at each block boundary and read as a trend, so the sum of the kept verdicts is measured and not assumed
accepts:    the reference commit is chosen and recorded here -- the first commit after S105 landed the harness regime -- and built by `fastchess.sh`'s worktree machinery; `adocs/data/S199_drift.sh` plays 1000 pairs at the `fastchess.sh` regime, fixed rounds, `-repeat`, detached with a terminal marker, and `adocs/data/S199_drift.py` appends Elo, nElo, the pentanomial and the 95 % interval to `adocs/data/S199_drift.tsv`; the reading rule is written in this file before the first run: a point inside the previous point's interval plus the verdicts landed since is "as expected", a point below it names those verdicts as the suspects for S183's discount, and no point is a verdict on any one of them; the first point is taken after the S109 block lands, on the workstation; `adocs/plan.md`'s Elo paragraph cites the file as its measured check; `DEV_MANUAL.md` "Play games" documents the instrument beside `rating.sh` and says what it is not
touches:    adocs/data/, adocs/plan.md, DEV_MANUAL.md
excludes:   the gauntlet and S152 (DEC-108 stands); any per-patch attribution, which a fixed match cannot give; changing the reference once chosen
decisions:  DEC-139, DEC-108
closes:     2026-09-10_adversarial-F30
blocks:
paused_by:
author:
done:

## Why this exists

R14 of `adocs/testing_strategy.md`. A one-stage `{0,5}` SPRT passes a true
zero one run in twenty, and "the SPRT Elo estimates are only unbiased if one
takes all patches into account, both passed and non-passed ones" (fishtest
FAQ); with about 45 pending verdicts the sum of the kept point estimates
drifts up by construction, which is the mechanism S183 is discounting for.
fishtest's answer is the regression test -- a fixed match against a pinned
reference, repeated, read as a trend. Here that is 1000 pairs, about 52
minutes at the MacBook's 2337 games/h and less on the workstation, with about
a +/- 10 Elo interval at S105's pair variance: enough to see whether the kept
verdicts are in the engine, and not the gauntlet DEC-108 deferred.

## Cost

A script and a reading rule, an hour; about 52 minutes of machine per point,
one point per block boundary.

## Amended 2026-09-11, DEC-170: the reading rule gains a term for timing conversions (F30)

`2026-09-10_adversarial-F30`: the reading rule above prices a point as "inside
the previous point's interval plus the verdicts landed since", and block 2 is
eight speed steps that land no verdict at all -- discharged by an interleaved
timing converted at DEC-083's 1.43 or 2.10 Elo per percent of nps, named as a
conversion. A drift point taken after block 2 would read high against a rule
with no slot for them. The rule therefore counts, beside the verdicts, the
conversions landed since, at their published rate and marked as conversions,
so the expected band has a term for speed; the point then says whether the
conversions were worth what the rate claims, which is a measurement this
project has never had. DEC-172 also places the S151-form longer-control
reading -- one fixed 1000-pair match at `32+0.32` -- beside each drift point,
so a boundary produces two numbers: drift at the regime, and transfer to four
times the control.
