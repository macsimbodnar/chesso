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
  1 490 839 rows. The fit is running, executed by the agent under a one-run
  delegation from the owner (DEC-034; DEC-015 stands for every later fit).
  S027 follows, once the returned constants have an SPRT verdict.

## The overnight run, and how to resume it

Written so an interrupted session picks up from the filesystem alone. Check
each state in order and act on the first that matches.

1. **Tuner still running** — `pgrep -f "build/tools/tuner"` returns a pid.
   Wait. Progress is in `.tuning/tuner_v1.log`. Do not start a timed match
   alongside it; a match sharing the cores measures the tuner.
2. **Tuner finished, `.tuning/tuned_tables.hpp` exists** — read the last line
   of `.tuning/tuner_v1.log`. It prints `best: train ... validation ...
   (start ... / ...)`. Held-out must be below the starting figure, and the
   tuner prints a refusal warning if it is not. If it refused, stop: the fit
   failed, record that and do not touch `eval_tables.hpp`.
3. **Fit good, `src/eval_tables.hpp` unmodified** — paste the tuned
   definitions over the corresponding ones, `cmake --build build -j8`,
   `ctest --test-dir build -L fast`, `./clang-format.sh --check`. Leave it
   uncommitted so the SPRT reference is the hand-picked constants.
4. **Tables pasted, no match running** — `REF=e0c338e ./fastchess.sh`, full
   bounds, wrapped in `caffeinate -is` and detached. That commit is the
   hand-picked constants with a green gate. Log at `/tmp/fastchess_full.log`.
5. **Match finished** — the verdict goes into the step file and into
   `testing.md` whatever it says. Zero is recorded as zero and the constants
   are still discardable. Commit only on a green suite; never push.

The tuned constants are worthless until step 5, and the owner may discard the
whole run on DEC-034 grounds without anything downstream depending on it.

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
