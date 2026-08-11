# Status

Convenience view, rewritten at the end of every work turn. The filesystem beats
this file: on disagreement, `plan_current/` wins.

Updated: 2026-08-11, S028 complete.

- Last done: S028
- In progress: S034 stop paying for evaluate() at every node: static eval in the table entry, and a quiescence eval cache
- Next: S034
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
- **S027's first term has been tried and rejected, and the step has not started.**
  Mobility cannot go through the S014 accumulators — it is a function of
  occupancy, not of a piece and a square. `tests/bench_eval` priced recomputing
  it at 12.2 times the cost per call and 33 % of nodes per second, worse than
  the 25 % S014 removed (DEC-036). Measured in play anyway, because nodes per
  second is not Elo: **−14.93 +/- 16.44 over 1164 games, H0 accepted**
  (DEC-037). Reverted, nothing in the tree.
  That verdict bundles two things — four weights that were never fitted, and a
  third of the machine — so it does not close mobility. The two experiments that
  would are in DEC-037: fit the eight weights through the tuner (the model stays
  linear, `PARAM_COUNT` 773 to 781), or make `evaluate()` run less often first.
  **The owner chose the cost work** (DEC-038), which is now S034 and sits
  between S028 and S027.
- **S034's opening measurement is done and the step needs a decision.** The
  concern that a probe would cost more than the 1.36 ns evaluation was wrong:
  0.80–1.08 ns into the 12 MB table, 0.36 ns into a 256 KB cache, and
  `sizeof(tt_entry_t)` is 24 bytes with 20 used so a 16-bit static eval is free.
  What kills the speed rationale is the size of the prize: a node costs about
  116 ns, so trading 1.36 ns for 0.36 ns saves under 1 % against a 3 % noise
  floor. Nothing an SPRT could see. `tools/probe_cost` is kept because the
  answer changes as the evaluation gets more expensive.
- **`bench_eval` understates an occupancy term 3.9x, and that revises DEC-036
  and DEC-037.** Kiwipete searched 9095066 nodes against 8860613 in the two
  mobility builds, so per-node cost compares directly: 115.7 ns against 172.1,
  a difference of 56.4 ns per node where `bench_eval` measured 14.6 per call.
  The benchmark keeps a slice of the magic tables hot; a real search walks 2 MB
  of rook attacks at random. Mobility was costing about 56 ns, not 14.6.
- Three ways out of S034, none started: close it keeping only the static eval in
  the table entry as an enabler for S033 and an `improving` flag, which is free
  in bytes and justified by what it unlocks rather than by speed; convert it to
  lazy evaluation; or drop it and go at fitted mobility weights knowing the real
  cost. The agent leans the first. **Owner's call.**
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
