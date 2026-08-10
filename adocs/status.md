# Status

Convenience view, rewritten at the end of every work turn. The filesystem beats
this file: on disagreement, `plan_current/` wins.

Updated: 2026-08-11, overnight S028 run.

- Last done: S018
- In progress: S028 fit every evaluation constant at once against self-play game outcomes
- Next: S028
- Blocked: none
- Where S028 has got to: `datagen`, `tuner`, `eval_model.hpp` and
  `test_eval_model` are built and committed at `ae814b6`, and formatted to the
  pinned clang-format at `e0c338e` — `ae814b6` had shipped them unformatted, so
  the section 5 gate was red at that commit and at every commit after it. The
  self-play data is generated: `.tuning/selfplay_v1.tsv`, 20000 games,
  1 490 839 rows. The fit ran, executed by the agent under a one-run
  delegation from the owner (DEC-034; DEC-015 stands for every later fit), and
  took held-out error from 0.113852 to 0.108043 on a 149083-position split it
  never saw. The constants are in `src/eval_tables.hpp` at `c31b995`, the
  anchor tests are re-anchored against a second implementation of `evaluate()`,
  and the whole suite is green including deep perft. **Only the SPRT is left.**
  S027 follows the verdict.

## The overnight run, and how to resume it

Written so an interrupted session picks up from the filesystem alone. Check
each state in order and act on the first that matches.

1. **A match is running** — `pgrep -x fastchess` returns a pid. Wait, and
   start nothing else: a second timed match measures the first. Progress is in
   `/tmp/sprt_s028_console.log`, and `/tmp/fastchess_full.log` is fastchess's
   own.
2. **No match running and the step file has no verdict** — the match was
   interrupted. Read the last SPRT block in `/tmp/sprt_s028_console.log`; if it
   holds a few hundred games or more, that is a partial result and is recorded
   as partial, not as a verdict. Otherwise restart it:
   `REF=9fc6fdf ./fastchess.sh`, full bounds, wrapped in `caffeinate -is` and
   detached. `9fc6fdf` is the hand-written constants with a green gate, one
   commit before the tuned ones.
3. **Match finished** — the verdict goes into the step file and into
   `testing.md` whatever it says. Zero is recorded as zero. A pass completes
   the step; a fail reverts `c31b995` and the step stays open with the fit
   recorded as measured and rejected.

Earlier states, kept because they say what has already been checked: the tuner
did not print its refusal warning, `.tuning/tuned_tables.hpp` is the fit it
produced, and the paste is committed rather than sitting in the working tree.

The tuned constants are worth nothing until the verdict, and the owner may
discard the whole run on DEC-034 grounds without anything downstream depending
on it.

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
    needed for S032 and S029 regardless. S028's self-play data generation and
    its fit occupy the same cores, so nothing can be measured while either runs.
  - **Steps S001 to S016 were retro-stamped at moltke adoption**, not completed
    under the workflow. Their measurements are transcribed from the commits and
    from the two plan documents they replace (DEC-027). Treat their `done:`
    stamps as provenance, not as evidence that the gates of section 4 ran.
