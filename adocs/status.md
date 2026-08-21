# Status

Convenience view, rewritten at the end of every work turn. The filesystem beats
this file: on disagreement, `plan_current/` wins.

Updated: 2026-08-21 by `moltke --step status`.

- Last done: S138
- In progress: none
- Next: S142
- Blocked: none
- Parked:
  - **`AGENTS.md` par.12 and the stop hook's own message both name
    `bin/moltke.py`, which does not exist in this repository.** The tool ships
    with the plugin, at
    `~/.claude/plugins/cache/moltke/moltke/<version>/bin/moltke.py`, so every
    command written the documented way fails at invocation -- it cost one failed
    call at the start of the 2026-08-20 session and it is why plan_review F18
    was filed against S085's copy of the same string. The S085 copy is corrected;
    this one is not, because `AGENTS.md` is the moltke ruleset file that a plugin
    upgrade rewrites, and par.0 already says local overrides there have to be
    re-applied after an upgrade. Recorded rather than edited: whether to patch
    the ruleset locally, or carry the real path in `.moltke.local.md` where a
    machine-specific path belongs, is the owner's call. Parked, not planned.
  - **A fourth question, from S085's run: what should `RFP_MIN_PLY`'s declared
    minimum be?** Measured 2026-08-20: the tested floor is **2**, not the 3 the
    comment argues for, and **0 and 1 are the same engine** because `!is_pv`
    exempts the root, not this parameter. 3 of 18 mate cases fail at 0 and 1;
    all pass at 2. Narrowing to 2 is measurement-backed; narrowing to 3 matches
    the stated purpose but rests on an argument no test exercises. The wrong
    claims in `src/search_params.hpp` are corrected; the bound is left at 0
    because changing it is a decision. `RFP_MAX_DEPTH` has the same shape and no
    red test -- its comment says "the last few plies" and its max of 63 permits
    every depth. Full measurement in S085's step file.
  - **Three questions banked for the owner while working overnight, 2026-08-20.**
    Asked here rather than blocking the run. (1) **Resume the SOTA enrichment
    pass?** 20 pending steps are still unenriched and it was stopped at S120 by
    your instruction earlier the same day; it is machine-free and parallelises
    one agent per step, but it is not a plan step, so it did not fit the
    instruction that plan steps run in sequence. (2) **S085's goal line says
    "the twenty search parameters that exist today" and the frozen run is 12** --
    your decision at freeze time, with the nine `Tm*` and `OrderHistoryMax`
    excluded for reasons recorded in the step file. Amend the goal line, or let
    the `done:` stamp carry the deviation? The step's own Scope concern already
    flags that the live surface is 22 rather than the goal's twenty. (3) **The
    fourth plan_review is running documents-and-citations only**, because its
    method re-measures every numeric claim from the tool the step names and the
    machine is committed to S085; each unverifiable number is being recorded as
    deferred with the command that would settle it. Schedule the re-measurement
    pass as its own run, or fold each deferred number into the step that owns it?
  - **The SOTA enrichment pass over the 3000-Elo steps stopped at S120, by the
    owner's instruction, 2026-08-20.** One research agent per pending step
    appends a `## Technical details (SOTA research, 2026-08-19)` section —
    published form, traced records, file:line grounding, seeds per DEC-084,
    measurement plan per DEC-083/S105. Done: all of block 0 (commit 0edfd26)
    and block 1 (commit c0954ec), plus S020, S055, S117, S120 of block 2 —
    28 of 48. Not yet enriched: S119, S042, S032, S030, then blocks 3 and 4
    (S082, S083, S039, S121, S123, S125, S118, S101, S122, S124, S102, S133,
    S126, S127, S128, S129). Derivable from the filesystem:
    `grep -L 'Technical details (SOTA research' adocs/plan_todo/*.md` names
    the remainder (reserve S023/S025/S110/S111 and parked S029 were excluded
    by design). Several sections flag scope concerns for owner decision — the
    largest: S097's section notes Lynx's README self-rating (3144/3293)
    against DEC-087's ~2850 banding of Lynx-derived records, and S109's
    accepts contains a no-prune-when-giving-check clause its own section
    argues is unenforceable as written.
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
    came after it. the paragraph beginning "It also prunes completed entries" in
    `plan.md` states the rule against the code that implements it — read `:173-187` until 2026-08-17, off by one at both ends: 173
    is blank and 188 is the paragraph's last word, and `:174-188` until 2026-08-19,
    by which time the paragraph had moved bodily and the range named a sentence
    about S087 — and this item does not restate the census: the window moves on
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
  - **DEC-033's ordering conclusion is superseded by DEC-081; its measurement
    is not.** 160 expensive moves re-asked at 16 times the search removed 24.1 %
    of the error and left 95 of 160 unchanged, and that is still true. What it
    priced was *nodes*, and sixteen times the nodes is about four plies in this
    tree: measured 2026-08-19, chesso reaches depth 12 on 1448572 nodes where
    Stockfish reaches depth 15 to 16 on 158837. So the evaluation was the
    binding constraint in 2026-08 — S028 at +188.74 and S065 at +21.10 are that
    cashed — and the tree shape is the constraint now. The search block leads
    the evaluation block. S019 is still retired.
  - **S015's quiescence SEE pruning has never been re-measured.** It returned
    0 Elo when `see()` cost 12.1 % more than it does now, so the same feature is
    a cheaper trade today than when it was judged. Folded into S022 rather than
    given its own step.
  - **S013's LMR SPRT was never formally concluded.** Killed at 96 % LLR,
    +129.2 +/- 33.8 over 183 games. The trend was unambiguous and the machine was
    needed elsewhere. Nothing depends on closing it; recorded so nobody reads
    "passed" into a run that was stopped.
  - **Measurement capacity is still the binding constraint, and S105 bought
    less than it was priced at.** Landed 2026-08-20: `tc=8+0.08`, `Hash=16`
    (DEC-088, not DEC-083's 128), `UHO_Lichess_4852_v1.epd`, all 12 threads of
    the DEC-049 machine. **Measured x1.67, 23.1 to 38.7 games a minute**,
    against the x3 DEC-083 predicted — x1.41 from the control and x1.20 from
    shorter games. **The unbalanced book bought nothing at the pair level**
    (variance ratio 1.022) and Pohl's 45 % draw floor is unreachable at this
    strength: the *balanced* book measured 40.3 % draws. It is kept on the
    x1.20 and on DEC-083, and whether to keep it is the owner's to revisit —
    the argument is in `adocs/data/S105_calibration_pairs.txt` and
    `adocs/plan_done/S105_sprt_harness_regime.md`. Find the source paragraph in
    `specs.md` under "Open items" by the phrase "Measurement capacity is the
    binding constraint" — this item carried a line range into that file three
    times and it was stale all three times, which is why it is a phrase now.
    A verdict that stops early is cheaper — S033's took 44 m 10 s for 1012
    games — and one that goes the distance costs the full window. Nothing else
    is measured while a match runs, and data generation and fits compete for the
    same machine.
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
    decision, not a drift. The standard search machinery that used to be parked
    beside it is no longer: **DEC-071 is the decision that was missing**, and
    the 2026-08-19 review (DEC-081 to DEC-086) is what ordered it — a search
    block that leads, an evaluation block that follows it, and one step, S109,
    where four pruning rules that are inert apart are measured together.
