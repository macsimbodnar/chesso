# Status

Convenience view, rewritten at the end of every work turn. The filesystem beats
this file: on disagreement, `plan_current/` wins.

Updated: 2026-08-10, replan on DEC-033.

- Last done: S018
- In progress: S028 fit every evaluation constant at once against self-play game outcomes
- Next: S028
- Blocked: none
- Where S028 has got to: `datagen`, `tuner`, `eval_model.hpp` and
  `test_eval_model` are built and committed at `ae814b6`. Self-play data is
  generating into `.tuning/selfplay_v1.tsv`, 20000 games at 100000 nodes per
  move, about 1.5 M rows and 95 minutes. The fit itself is the owner's to run
  (DEC-015) and the run is stated at the end of the step file. S027 follows,
  once the returned constants have an SPRT verdict.
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
    needed for S032 and S029 regardless. S028's self-play data generation will
    occupy the same cores, so nothing can be measured while it runs.
  - **Steps S001 to S016 were retro-stamped at moltke adoption**, not completed
    under the workflow. Their measurements are transcribed from the commits and
    from the two plan documents they replace (DEC-027). Treat their `done:`
    stamps as provenance, not as evidence that the gates of section 4 ran.
