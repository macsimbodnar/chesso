# Status

Convenience view, rewritten at the end of every work turn. The filesystem beats
this file: on disagreement, `plan_current/` wins.

Updated: 2026-08-17 by `moltke --step status`.

- Last done: S053
- In progress: none
- Next: S075
- Blocked: none
- Parked:
  - **The 2026-08-16 plan_review findings are planned, not closed, and closing
    them needs a re-run.** Ten findings, one high and three medium: nine went to
    steps (S069 to S072, S074, S078 to S081) and F08 to DEC-062. A finding moves
    to `closed` only when a re-run no longer reports it, so a fourth plan_review
    is owed once those steps land — and the severity profile says the loop has
    not reached its stopping condition, which is a re-run with no high and no
    medium. F05 was this file's own two parked items and S069 has rewritten
    them; it is `planned` until the re-run, like the rest.
  - **"Last done" above can name an older step than the newest completion.** Not
    a stale file: `moltke --step status` derives the field from the last
    completed entry in `plan.md` **list order**, and retention is a window over
    list positions, so an entry sitting low in the list outlives completions that
    came after it. `plan.md:174-188` states the rule against the code that
    implements it — read `:173-187` until 2026-08-17, off by one at both ends: 173
    is blank and 188 is the paragraph's last word — and this item does not restate
    the census: the window moves on
    every completion, so a worked example here reads as a measurement while being
    a memory. Hand-correcting the field puts the file back out of agreement with
    its own generator, which the stop hook catches, so it is recorded here
    instead. Found while completing S065. Parked, not planned: a step is created
    by a decision and none has been taken on this.
  - **The corpus and the fit tooling are gitignored and do not survive a machine
    move.** `.tuning/` holds `selfplay_v2.tsv` (715 MB, 11003693 positions) and
    the scripts S065 leaned on — `apply_fit.py`, `verify_fit.py`, `anchors.py`,
    `reanchor.py`, `diff_fit.py`. `anchors.py` in particular is now the only
    executable record of how ten pinned test values are derived, including the
    quiescence composite that no evaluation model can produce; the step file
    records the transformations in prose but nothing re-runs them. This is the
    same class of loss `2026-08-13_plan_review.2-F02` recorded when
    `selfplay_v1.tsv` did not survive DEC-049, and it cost S065 a night of
    regeneration. Found while completing S065. Parked, not planned: a step is
    created by a decision and none has been taken on this.
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
  - **Measurement capacity is the binding constraint on the whole plan.** An
    SPRT verdict costs three to four and a half hours at the DEC-048/DEC-050
    settings — all 12 threads of the DEC-049 machine — and the opening book is
    only `8moves_v3.pgn`. `specs.md:199-204` is the source; this restates it
    because it is the paragraph a session reads before deciding whether it can
    afford a measurement. (The citation read `:189-194` until 2026-08-17 and was
    stale by one commit: it was correct when S069 wrote it at `e8a3dca` and
    `7f15ac4` added ten lines above the paragraph.) A verdict that stops early is
    cheaper — S033's took 44 m 10 s for 1012 games — and one that goes the
    distance costs the full window. Nothing else is measured while a match runs,
    and data generation and fits compete for the same machine.
  - **The bounds decide whether a night buys a verdict at all, DEC-063.** S068
    measured one constant twice with the same binaries: `elo0=0 elo1=5` ran
    6 h 36 m over 9036 games and returned nothing, `elo0=-5 elo1=5` returned
    H1 in 1 h 41 m over 2312. Both at 1371 games/h, so the cost of a verdict is
    set by the hypothesis pair and not only by the hardware. Not parked as work:
    it is a rule, and the pending steps it constrains are S021, S023, S024 and
    S026.
  - **Steps S001 to S016 were retro-stamped at moltke adoption**, not completed
    under the workflow. Their measurements are transcribed from the commits and
    from the two plan documents they replace (DEC-027). Treat their `done:`
    stamps as provenance, not as evidence that the gates of section 4 ran.
  - **The `info` line is UCI surface that the golden guard does not cover.**
    `test_uci_surface` holds the command set, the option lines and the `go` and
    `position` tokens against `MANUAL.md`; the search output has never been in
    it, which is how S037 could add an `nps` field and change what `nodes` and
    `time` mean without any test noticing. `MANUAL.md` now documents the fields,
    so a golden check has something to hold them against. Found while doing
    S037. Parked, not planned: a step is created by a decision and none has been
    taken on this.
  - **The tuner's last `--only` group runs to `PARAM_COUNT`, and that is a blind
    spot S041's test cannot cover.** `tempo` ends at `PARAM_COUNT`
    (`tools/tuner_groups.hpp`), so a parameter block appended after it and given
    no group of its own is covered by `tempo` and all three partition properties
    still hold. `test_tuner_groups` catches it through a precondition instead —
    `TEMPO_EG_BASE + TEMPO_COUNT == PARAM_COUNT`, the bases chain against
    `eval_model.hpp`'s independent width sum — which fires but names the wrong
    thing. The structural fix is a group table the last entry cannot outrun, and
    it would delete the four historical comments that are the only record of the
    defect. Found while doing S041, whose `excludes:` puts it out of reach.
    Parked, not planned: a step is created by a decision and none has been taken
    on this.
  - **NNUE is deferred and S029 is parked, DEC-054.** The owner's decision:
    strength comes from search and from the hand-crafted evaluation instead.
    Parked is not retired — `plan_todo/S029_nnue.md` is kept whole, its id is
    not reused, and its plan.md entry moved to the end of the pending order
    marked parked, so nothing derives it as the next step. Resuming it is a
    decision, not a drift. This makes the "standard search machinery" list above
    the place the next steps have to come from; it still has no step behind it,
    on the same rule, and DEC-033's finding that the engine is
    evaluation-limited now has to be answered by hand-crafted terms and fits.
