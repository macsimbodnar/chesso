id:         S128
goal:       the gauntlet is replayed at a time control near the rating list's own, to test whether the anchor spread is scale compression from 10+0.2
accepts:    a rated run at a control materially closer to the list's 2 min plus 1 s than 10+0.2 is, with the control stated and its cost recorded before it is committed to; the solved rating and the **anchor spread** are both reported, and the spread is compared against the 121.8 Elo S088 measured and the 30 the procedure allows; the reference manifest names the version each result was played against; the run is detached with a terminal marker and a self-terminating watcher; whatever it returns is recorded, including "the spread did not move", which would falsify the hypothesis
touches:    rating.sh, adocs/data/, adocs/specs.md
excludes:   changing the reference set, which DEC-077 says no reference set fixes
decisions:  DEC-074, DEC-077, DEC-072
closes:
blocks:
paused_by:
done:

## The number the whole plan is measured against is soft, and this is the named suspect

`specs.md` carries 2559, 95 % +/-25, and calls it soft because **the five
references disagree internally by 121.8 Elo, four times what the procedure
allows**. DEC-077 established that adding a third family identified the spread
rather than closing it, that the solved rating rises monotonically with the
anchor's own rating and flattens at the top, and that across a CCRL span of 440
Elo the measured differences span 375.3 -- a ratio of 0.853. That is scale
compression, and DEC-077 names **the time control as the leading candidate and
says attacking it is nobody's step**.

It is this step. Chesso is rated at 10+0.2 against a list played at 120+1,
which is a twelve-fold difference in base time, and the surveyed record is
explicit that Elo per doubling of time falls as the control lengthens -- so a
short-control gauntlet compresses exactly the way the measurement compresses.

It goes near the end because it costs about five hours and buys zero Elo, and
because DEC-074 leaves re-running the gauntlet to the owner's judgement rather
than to a threshold. It is on the plan because "430 Elo to go" is a claim that
rests on it.
