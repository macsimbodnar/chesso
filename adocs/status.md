# Status

Convenience view, rewritten at the end of every work turn. The filesystem beats
this file: on disagreement, `plan_current/` wins.

Updated: 2026-08-15 by `moltke --step status`.

- Last done: S053
- In progress: S065 regenerate the tuning corpus from today's engine with the tactical-move filter loosened, and fit it
- Next: S065
- Blocked: none
- Parked:
  - **S065's second fit is applied, the suite is green, and the SPRT is the only
    thing left.** The fit is DEC-057's: `tempo` and `piece_placement` held at
    zero *during* the fit through the `--freeze LIST` flag, 817 free of 827,
    K = 0.7624, held out 0.122560 → **0.118460** at epoch 5000, no refusal
    warning, 2329 s, emitted header `.tuning/tuned_v2_frozen.hpp` sha256
    `bb6c68b5…0ce54`.

    `POSITIONAL_ROOM` was what blocked it — the refitted queen on d1 evaluated
    716 against her own `QUEEN` of 1148, a gap of **432** against 150. **DEC-059
    is the owner's answer**: re-anchor rather than widen the guard. 432 off
    `#define QUEEN` and 432 onto all 128 of her squares, which
    `tools/tuner.cpp:26-30` documents as the same evaluation and which measures
    at **one centipawn on one of seven pinned positions** — the residual goes
    432 → 1 and the guard is untouched at 150. Applied to the emitted header
    before the paste, verified **827 of 827**.

    Everything the paste moved was re-derived by `.tuning/anchors.py` rather than
    read off the engine, including `test_search`'s two -491 cases that earlier
    runs left underived: it now reproduces **10 of 10** on the shipped weights.
    The guard-2 re-target finally committed, its constants being in the tree at
    last. Gate green — **12 of 12 fast plus `test_perft`, format clean**, and no
    threshold anywhere was lowered.

    **The SPRT is outstanding and nothing is decided until it returns.**
    `REF=a2f0065 CONCURRENCY=12 ./fastchess.sh`, 3–4.5 h. 827 constants move at
    once; a verdict of zero is recorded as zero and a negative one reverts the
    paste (DEC-019, INV-6).
  - **"Last done" above reads S053 and the newest completion is S067.** Not a
    stale file: `moltke --step status` derives the field from the last completed
    entry in `plan.md` **list order**, and the retention window leaves S053 at
    position 66, after the parked S029, while S054, S038, S066 and S067 sit at
    37 to 40. Hand-correcting the field puts the file back out of agreement with
    its own generator, which the stop hook catches, so it is recorded here
    instead. The newest completion is S067, `d01e61c`. Found while completing
    the S065 paste. Parked, not planned: a step is created by a decision and
    none has been taken on this, and S062 and S064 are the pending steps nearest
    to it.
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
