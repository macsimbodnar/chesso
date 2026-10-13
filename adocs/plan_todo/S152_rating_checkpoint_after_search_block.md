id:         S152
goal:       the engine's absolute rating is re-measured once, near the 3000 mark, and it carries both time controls so the number it returns can be read
accepts:    one `rating.sh` run when the pending order is close to exhausted and a
            3000 claim is in reach -- **not** at a block boundary, DEC-108 -- with the
            solved rating and the anchor spread both reported and compared against
            S088's 2559 +/- 25 and its 121.8 Elo spread and S240's 2766 and its 94.1;
            **the 3000 claim is read as the anchor-mean point estimate at
            `rating.sh`'s standing control, 10+0.2, with its interval and the second
            control's reading reported beside it, not as the interval's lower bound
            (DEC-258)**; the manifest includes S264's anchors above 2850; the run's resolution is stated
            before it is read, because +/- 25 plus the family spread means it detects
            drift above roughly 50 Elo and nothing smaller; the same gauntlet is played
            at a second control near the rating list's own, which is S128's question
            folded in here by DEC-108, and the two anchor spreads are reported side by
            side so scale compression is separated from engine drift rather than
            assumed either way; whatever it returns is recorded, including "inside the
            forecast band", which is the expected outcome and a valid one; both runs
            are detached with a terminal marker and a self-terminating watcher
touches:    rating.sh, adocs/data/, adocs/specs.md, adocs/plan.md
excludes:   changing the reference manifest or the rating procedure, which S087 and
            S088 fixed; any conclusion about an individual patch, which a gauntlet
            cannot attribute
decisions:  DEC-019, DEC-020, DEC-072, DEC-074, DEC-077, DEC-108, DEC-258
closes:     2026-08-21_adversarial-F04
blocks:
paused_by:
done:

## Amended 2026-10-08, DEC-258

The owner fixed the claim standard before any result exists: the mark is met
when the gauntlet's anchor-mean point estimate at `rating.sh`'s standing
control reads 3000 or more. The interval, the anchor spread and the second
control are reported beside it and qualify the claim; they do not move the
bar. S264 installs the anchors this run needs and recomputes its cost with
the larger manifest. S267's drift gate sits before S126, so this run is
reached only if that gate passed or the owner ruled past it.

## Cost with S264's manifest (written by S264, 2026-10-13)

S264 adds Weiss 1.1 (2987) and Stash v30.0 (3153), so the gauntlet has seven
pairings. `rating.sh` plays 334 rounds, 668 games, per pairing:

    games = 668 x P                  P = pairings = 7  ->  4676 games
    hours = 668 x P / G              G = games an hour at the control played

**G at 10+0.2 is measured: 1349 games an hour.** S240's valid run 2 played
3340 games in 2 h 28 m 31 s (fastchess's own total time) on this machine at
concurrency 12, hash 128. Run 1 took 2 h 30 m 45 s but was void and shared
the machine for its first hour, so it is not the reference. That is **29 m 42 s
a pairing**, and the standing control's gauntlet is **4676 games, about 3 h
28 m** -- against 2 h 29 m for S240's five anchors. One added pairing is about
30 minutes at 10+0.2.

**G at the second control is not measured, and is estimated by scaling.**
Throughput falls about in proportion to the control: the harness's own
self-play read 2110 games an hour at 8+0.08 and 534 to 545 at 32+0.32, a
factor 3.87 to 3.95 for a control four times as long. So at a control `k`
times the length of 10+0.2 -- `k = (base + 60 x inc) / 22` when the two do
not scale together, the nominal clock a side gets over 60 moves -- the
second gauntlet costs about `3.47 x k` hours and the pair `3.47 x (1 + k)`:

| second control | k | second gauntlet | both |
|---|---|---|---|
| 20+0.4 | 2 | 6.9 h | 10.4 h |
| 30+0.6 | 3 | 10.4 h | 13.9 h |
| 40+0.4 | 2.91 | 10.1 h | 13.5 h |
| 60+0.6 | 4.36 | 15.1 h | 18.6 h |
| 120+1, the list's nominal | 8.18 | 28.4 h | 31.8 h |

The 10+0.2 gauntlet is under DEC-155's four hours and starts when it is
ready; every second control above is a night run, and the last is two. The
second control's games-an-hour is re-read from the first 1000 games of its
own run before its finish time is promised. The anchors near chesso's
strength may lengthen games a little (fewer early adjudications). S264's
smoke run, 600 games against Leorik 2.4 and the two new anchors at 10+0.2,
read **1299 games an hour**: 1663 s from its start stamp to `report.txt`'s
last write (fastchess's own total, 27 m 42 s, agrees), 3.7 % under 1349 and
inside a run more than seven times shorter. At 1299 the standing control's
gauntlet is 3 h 36 m; budget 3 h 30 m to 3 h 40 m.

## Why five hours is the right price -- and why not yet

The plan's own budget is "roughly 75 to 110 machine-hours" and S088's run cost
5 h 02 m 49 s, so this is about 5 %. What it buys is the only check that the
summed per-patch deltas are the engine's strength. The plan already takes "a
fifth off for interaction", which is an estimate nothing measures, and DEC-020
is the case where an unattributed run reported +301 Elo and meant nothing.

**Deferred to near the goal by the owner, DEC-108, 2026-08-23.** The argument
this step was written on -- that 45 to 55 verdicts should not accumulate without
an end-to-end check -- is not withdrawn and is why the step is deferred rather
than retired. What the owner decided is that the checkpoint is not worth five
hours *at the block boundary*: it buys information and no strength, and no
result it could return would change the pending order, which was itself set by
an adversarial review against the published record. So the check is owed once,
before the 3000 claim, and not twice.

Two things follow and they are the reason this file is not just moved. **S128 is
folded in**, its substance carried below rather than lost with its file, because
its question is a property of how the final number is read and answering it on
the *current* engine and then again on the final one is the same night twice.
And **the estimate stays an estimate**:
asked on 2026-08-23, the honest answer was 2600 to 2650 -- the kept positive
point estimates sum to about +90, DEC-063's own correction factor (S068's pooled
estimate falling from +12.18 to +5.02) cuts that to about +49, and S104's
+18.22 % is unpriced because what a ply is worth here has never been measured.
None of that may be written into `specs.md` as the engine's rating. 2559 +/-25
soft is the last measured number until this step runs.

## What S128 carried in, folded here by DEC-108

S128 was "the gauntlet is replayed at a time control near the rating list's own,
to test whether the anchor spread is scale compression from 10+0.2". Its file is
gone and its id is not reused; this section is the substance, and the second
control in the `accepts:` above is the step it asked for.

**The number the whole plan is measured against is soft, and the control is the
named suspect.** `specs.md` carries 2559, 95 % +/-25, and calls it soft because
the references disagree internally by **121.8 Elo over five**, and **64.7 with
Leorik 2.1 dropped** -- against the 30 the procedure allows, so there is no
single dissenter to discard. S088 was written on "a third family can arbitrate a
disagreement between two"; a third family did not close the spread, it took it
from 83.1 over four anchors to 121.8 over five.

**DEC-077 is what makes the control the suspect rather than one misrated
engine.** The solved rating rises monotonically with the anchor's own rating and
flattens at the top, and across a CCRL span of 440 Elo the measured differences
span 375.3 -- a ratio of **0.853**. That is scale compression. Chesso is rated at
10+0.2 against a list played at 120+1, a twelvefold difference in base time, and
the surveyed record is explicit that Elo per doubling of time falls as the
control lengthens, so a short-control gauntlet compresses exactly the way the
measurement compresses. DEC-077 named the control as the leading candidate and
said attacking it was nobody's step. It is this one's.

Three requirements come from S128's own `accepts:` and are kept: the second
control is **materially closer to the list's 2 min plus 1 s than 10+0.2 is**, and
its cost is recorded before it is committed to; the **anchor spread** is reported
alongside the solved rating for both controls and compared against 121.8 and
against the 30 the procedure allows; and the reference manifest names the version
each result was played against. "The spread did not move" falsifies the
compression hypothesis and is a valid, recorded outcome. Its decisions were
DEC-074, DEC-077 and DEC-072 -- DEC-074 in particular leaves re-running the
gauntlet to the owner's judgement rather than to a threshold, which is the rule
DEC-108 has just exercised.

The reference set is out of scope here as it was there: DEC-077 says no reference
set fixes this.