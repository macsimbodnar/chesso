# Status

Convenience view, rewritten at the end of every work turn. The filesystem beats
this file: on disagreement, `plan_current/` wins.

Updated: 2026-08-11, S034 complete.

- Last done: S034
- In progress: none
- Next: S027
- Blocked: none
- S028 measured **+188.74 +/- 32.21 Elo**, the largest single change so far, and
  67.5 % against sgambetto where S018 measured 39.3 % under identical settings —
  so the chained SPRT gains are real against a fixed opponent, not drift.
- **The plan order was re-measured after it and stands** (DEC-035). Everything
  DEC-032 and DEC-033 concluded was measured on constants S028 replaced, so the
  whole profile was run again on the fitted engine: 98 games, 5582 moves, same
  opponent, reference and node limit. Early middlegame still first at 35.2
  cp/move and 38.8 %; 16x search still leaves 70 % of the error, at the same
  10.4 cp per doubling. S027 is next, unchanged, and it now has a tuner to fit
  its terms with.
- **S034 is done: +28.46 +/- 18.61 Elo over 942 games**, H1 accepted, LOS
  99.87 %. Lazy evaluation plus mobility, with the eight mobility weights fitted
  rather than guessed. Three measurements were needed to get there and the order
  is the point: mobility always computed on hand-picked weights measured
  **-14.93**; behind lazy evaluation, same weights, **+4.19 over 498 games and
  stopped**; with the weights fitted, **+28.46 and passed**. The staging is
  worth about 15 Elo and the weights about 25, and the first run measured both
  at once and said the whole thing was a failure.
- **The fit disagreed with hand-picking where it mattered.** Knight mobility
  fitted to nothing on top of the piece-square table; rook middlegame mobility
  to four times the guess. mg {0, 5, 9, 2}, eg {-1, 6, 1, 3}.
- **The margin is 150 and it is a guarantee, not an observation.** The term is
  clamped to it. Any of S027's remaining terms added behind the shortcut has to
  fit inside that bound or move it deliberately, and `test_evaluation` "the lazy
  shortcut cannot change a decision" is what forces the choice.
- **Two tuner bugs are worth remembering because no test caught them.**
  `write_tables()` did not emit the mobility weights and `gradient()` did not
  compute their derivative, so a fit would have run, reported an improvement
  from the other 773 parameters, and returned the eight weights unchanged. Found
  by a smoke run whose output looked too familiar.
- What S028 came to: all 773 evaluation constants are fitted to chesso's own
  self-play, over 1 490 839 positions from 20000 games. Held-out error 0.113852
  to 0.108043; SPRT +188.74 +/- 32.21 Elo over 438 games, H1 accepted. The fit
  was run by the agent under a one-run delegation from the owner, DEC-034 —
  **DEC-015 stands unchanged for every later fit and for the S029 network.**
  Three things went wrong on the way in and all three are in the step file: a
  clang-format gate that had been red since `ae814b6`, a macro collision
  between the ordering values and the evaluation's that only compiled while the
  numbers matched, and a tactics test that had been asserting a preference among
  four moves that all mate in ten.

- Parked:
  - **The plan was reordered by DEC-033 and S019 is retired.** 160 expensive
    moves re-asked at 16 times the search removed 24.1 % of the error and left
    95 of 160 moves unchanged, so the engine is evaluation-limited rather than
    depth-limited on the errors that decide games. S028 moved from 28th to next,
    S027 follows it, and the search block follows that with a measured ceiling
    of about 10 cp per effective doubling. S033 was created for reverse futility
    pruning, which the plan did not contain at all.
  - **Standard search machinery with no step behind it.** Late move pruning,
    check and singular extensions, a quiescence transposition probe, a static
    evaluation in the table entry, an `improving` flag, history malus and
    ageing, correction history. Found while measuring DEC-033. Parked, not
    planned: a step is created by a decision and none has been taken on these.
  - **S015's quiescence SEE pruning has never been re-measured.** It returned
    0 Elo when `see()` cost 12.1 % more than it does now, so the same feature is
    a cheaper trade today than when it was judged. Folded into S022 rather than
    given its own step.
  - **S013's LMR SPRT was never formally concluded.** Killed at 96 % LLR,
    +129.2 +/- 33.8 over 183 games. The trend was unambiguous and the machine was
    needed elsewhere. Nothing depends on closing it; recorded so nobody reads
    "passed" into a run that was stopped.
  - **Measurement capacity is the binding constraint on the whole plan.** At
    10+0.2 with three usable cores an SPRT verdict costs about an hour, the
    opening book is only `8moves_v3.pgn`, and this machine runs `opendirectoryd`
    at half a core often enough to matter. An x86-64 Linux box fixes this and is
    needed for S032 and S029 regardless. S028 spent 95 minutes generating data
    and 75 minutes on its SPRT on the same three cores, and nothing could be
    measured while either ran; S029 will want far more of both.
  - **Steps S001 to S016 were retro-stamped at moltke adoption**, not completed
    under the workflow. Their measurements are transcribed from the commits and
    from the two plan documents they replace (DEC-027). Treat their `done:`
    stamps as provenance, not as evidence that the gates of section 4 ran.
