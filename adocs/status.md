# Status

Convenience view, rewritten by hand at the end of any turn that changed plan
state. The filesystem beats this file: on disagreement, `plan_current/` wins.
Nothing generates it since moltke v1 (DEC-109), so a stale line here is a
missed edit and not a tool's opinion.

Updated: 2026-09-01, by hand.

- Last done: S169 -- **the 97 stale citations in the pending step files are
  re-anchored**, and it exists because doing S144 first would have erased them.
  `--citations` reports **0 flagged**, from 97 (72 DRIFT, 25 ANCHOR, 0 BOUNDS)
  over 20 of the 53 pending files. 94 citations rewritten, 3 held, nothing
  deleted and no range widened.
  **The checker's verdict is not the proof and cannot be.** DRIFT's baseline is
  the commit that last wrote the step file, so after this commit every repaired
  citation is green whether or not it was repaired. The evidence is
  `adocs/data/S169_recitations.tsv`, 97 rows carrying both texts: **83 of 97
  are identical on both sides**, and the 14 that are not are the TITLE, HAND
  and HOLD rows whose reason column says what was read instead. **No BLOCK row
  differs.** `adocs/data/S169_citations_before.txt` is the pre-repair run, kept
  because it cannot be regenerated.
  Method split, from `adocs/data/S169_recite.py`: **72 BLOCK** (the baseline
  text occurs exactly once at HEAD), **6 TITLE**, **16 HAND**, **3 HOLD** (the
  citation was right; the flag was a cosmetic edit to the cited text).
  **The ANCHOR block was one repair repeated seven times**: S091, S095, S097,
  S098, S113, S114 and S116 -- every one a pruning or a reduction step -- cited
  the two mate-safety tests 921 lines short of where they open. That is
  `2026-08-20_plan_review-F01`'s class returned after S138 repaired it once.
  **Two citations were already wrong at their baseline, not drifted**: S020's
  and S082's pointed at a blank line and a bare `}`, so nothing had ever
  checked them. **S082 also says `MAX_QSEARCH_DEPTH` is 8 where the engine
  compiles 19** (`src/search_params.hpp:106`) -- the citation is repaired and
  the number is left, because numeric claims are the step's `excludes` and the
  argument is S082's to remake. DEC-119.
- Before it: S143 (**the completion gate builds and tests `build-tune`
  beside `build`**, the red observed first, 45 s to 93.6 s; DEC-118), S168
  (**the constructed mate set is three motifs and 82
  positions**, the mating piece enforced, `MATE_IN_THREE_FLOOR` re-derived at
  11; DEC-117), S154 (**the mate-in-three floor is 8 and re-derived, not 7 and
  inert** -- 0 positions change verdict over seventeen commits and nine table
  sizes, 5 under one ply of the guard; `-F06` closed and reversed), S156 (**the
  mined breadth set is a gate now**, `test_mate_breadth` at depth 10 with a
  floor of 143; `-F08` closed), S155 (**the constructed set is one motif**,
  counted, which is what DEC-114 was decided on; `-F07` closed), S167
  (fastchess.sh under bash 3.2) and S150 (**document numbers about search
  parameters are checked against the code**, `test_plan_params`; `-F02` closed).
- In progress: nothing. `plan_current/` is empty.
- **`python-chess` was missing on this machine and is installed now**, at the
  owner's decision of 2026-09-01: 1.11.2 on python 3.9.6 in `~/.venv/chess`,
  the path `TOOLCHAIN.md` and every S145 script already name. It had not
  survived the move from the Linux workstation, so
  `adocs/data/S145_rfp_sweep.py` and `S145_mate_set.py` could not run here at
  all and nothing said so. `.moltke.local.md` records it now.
- Next: S144, the step S169 cleared the board for -- the bare `:line`
  continuations get a path of their own. **Its goal line and `plan.md` entry
  both say 467 and the measured count today is 383** over 19 files; the two
  ends move as steps complete, and the number is left for S144's own stamp to
  restate rather than edited from outside the step. After it, S146.
- Blocked: nothing.
- Watching: nothing. No match is running and no watcher is armed.
- Parked:
  - **HANDOVER TO THE MACBOOK, 2026-08-23. Discharged 2026-08-27 -- kept for
    what it explains, not as a thing to do.**

    **The check passed to the node.** `search_bench` at depth 9 gives
    121512 / 800769 / 62907 and at depth 12 gives 639228 / 3430710 / 367858,
    best moves `c3d5` / `e2a6` / `d7c8q` at both, exactly as predicted below.
    `build_lmr_table()`'s `log` was the stated portability exposure and Apple's
    `log` does not move the tree, so every deterministic figure this repository
    records is valid here as written and nothing was regenerated. Throughput in
    games per hour was still unmeasured when this was written and is not any
    more: **2700 games/h on 8 threads at 8+0.08**, from S024's run 3, which is
    the one thing those aborted runs did buy. `.moltke.local.md` is written.
    The commits were pushed. What the move did break was two things the
    handover did not predict, both found by running the check: the tree did not
    compile under Apple clang's `-Werror`, and `fastchess.sh` could not complete
    a run under bash 3.2 -- **S167** is both, and until it landed no SPRT could
    have been taken here at all.

    The original text follows, because its reasoning about which figures carry
    and which do not is the standing rule, not a one-off.

    **Start here, and it takes ten seconds.** Build, then run
    `python3 tools/search_bench.py ./build/src/chesso 9` and the same at 12.
    Expect **121512 / 800769 / 62907** at depth 9 and
    **639228 / 3430710 / 367858** at depth 12, best moves `c3d5` / `e2a6` /
    `d7c8q` at both, exactly. If they match, every deterministic figure this
    repository records is valid on the MacBook **as written**, and nothing needs
    regenerating. If they do not match, stop and read the next paragraph before
    trusting any node count in any document.

    **Why that check and not a re-baseline.** The numbers here fall into two
    classes and only one of them is machine-bound. *Deterministic*: node counts,
    best moves, perft, the reach counts, the counter tallies. At a fixed depth
    with `Threads=1` these are properties of the algorithm and not of the
    machine, so they carry across unchanged -- which is why one comparison
    confirms the whole class instead of re-measuring it. *Time-based*: nps
    ratios, games per hour, Elo verdicts, the CCRL estimate. Those cannot be
    carried and **do not need regenerating either**: DEC-049 already decides it
    -- comparability is a property of a (machine, compiler) pair, and the
    TOOLCHAIN.md warning "bites only when a figure is carried across machines".
    Every such figure stays attributed to the machine that produced it and
    remains valid as that. No re-run makes S108 more honest than it is.

    **The one thing that could break the deterministic class, stated so the
    check has a purpose.** `build_lmr_table()` at `src/search.cpp:102` is the
    engine's only floating-point path: `(LMR_BASE / 100.0) + log(depth) *
    log(move_number) / (LMR_DIVISOR / 100.0)`, truncated to `uint8_t`. glibc's
    `log` and Apple's are not bit-identical, and a 1-ULP difference where that
    expression sits just under an integer flips one reduction by a ply, which
    moves the tree and every node count with it. That is the entire portability
    exposure and the search_bench comparison is what detects it. If it fires,
    the fix is not to re-measure the documents: it is a decision about whether
    the table is built from a portable integer approximation instead, and it
    would be the first decision taken on the MacBook.

    **The only figure the MacBook actually lacks is throughput**, games per hour
    at `8+0.08`, needed for scheduling and nothing else. It arrives free from
    the first hour of the next real SPRT -- do not spend a dedicated calibration
    run on it. This machine did 2340 games/h on 12 threads for comparison.

    **`.moltke.local.md` does not travel and the MacBook has none.** It is
    machine-local and uncommitted by design, so write a fresh one there before
    measuring anything: the tool paths, the core count, the concurrency default
    and the compiler are all different. `TOOLCHAIN.md` is macOS throughout and
    is *correct* there while being stale here, which is the reverse of the
    situation its own Open note describes -- so on the MacBook TOOLCHAIN.md
    becomes the reliable document and `.moltke.local.md`'s Linux notes are what
    is gone.

    **What does not survive the move at all.** `.tuning/` is 1.4 GB and
    gitignored: `selfplay_v2.tsv` is 683 MB and regenerable at the cost of a
    night, and the five fit scripts are **32 KB in total** -- `anchors.py` 16 K,
    `apply_fit.py`, `verify_fit.py`, `reanchor.py`, `diff_fit.py` 4 K each.
    `anchors.py` is still the only executable record of how ten pinned test
    values are derived, including the quiescence composite no evaluation model
    can produce. Committing 32 KB would end that exposure for good and it is the
    owner's call, not the agent's, which is why it is written here and not done.
    `.ref-builds/` is 2.2 GB of gitignored worktrees and is worth nothing --
    `fastchess.sh` rebuilds them on demand.

    **The commits must be pushed before the machine goes down.** The GIT rule
    is that the agent never pushes. At the time of writing `achesso` is
    ahead of `origin/achesso` by three: `bbbd9f4` (DEC-108), `3b39e5a` (S108's
    hoist) and S108's completing commit. Unpushed, the MacBook sees a branch
    that ends at `77ecb6f` and none of S108 exists.

    **Where the work stands.** **S108 is complete.** H1 accepted at `elo0=-5 elo1=0` in 12774 games and 5 h 26 m against `bbbd9f4`, `Elo 2.28 +/- 4.40`, `LLR 2.98`, 0 forfeits -- a regression of 5 nElo or more excluded (**3.64 logistic Elo** at this run's own `nElo 3.13` against `Elo 2.28`; the bounds are normalized Elo, S157), no gain claimed. The engine now computes a static evaluation at every non-check main-search node and the table entry carries it, which is the input S109 reads. The next step in plan.md order is
    **S024**, continuation history -- not S109. It is play-altering and owes an
    SPRT, so it is the step that wants the throughput figure above, and it is
    the one to start with on the MacBook once the search_bench check above has
    passed. **S109 sits behind it** and carries the layer S108 deferred -- a
    direction-certified table score as a pruning margin's input, written up in
    its step file under `## Inherited from S108` -- so that decision is waiting
    where the step that takes it will be read, not here.
  - **Nothing checks that the two copies of the completion gate agree.** S143
    put the command in `AGENTS.md`'s TESTS rule and in DEV_MANUAL.md's Test
    section and made them identical; DEC-118 says they are changed together,
    which is an assumption and not a guard. `tools/plan_prose_check.py
    --params` is the precedent for turning exactly this class of prose drift
    into a `fast` test -- it already reads documents and compares them to the
    code. Parked, not planned: a step is created by a decision and none has
    been taken on this.
  - **The gitignored-evidence exposure was real and S024's share of it is
    discharged, DEC-111.** This item used to say `.tuning/` held the only copy
    of S024's aborted run and that it would die with the machine. It would
    have. All of it now sits on branch `s024_mac_attempt` under
    `adocs/data/S024_mac_attempt/`, gzipped, with digests and a README: both
    PGNs, the three run logs, the fastchess log, the resume config and the
    resume script. The exposure itself is not discharged -- `.tuning/` is still
    gitignored and still holds the corpus and the fit scripts the item below
    describes, which are the larger stakes. What changed is that the pattern
    has a worked example now: a branch nobody merges is a cheap place to put
    evidence that must outlive a machine.
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
    and block 1 (commit c0954ec), plus S020, S055, S117, S120 of block 2.
    **The remainder is derived and is no longer enumerated here.** The
    enumeration this item used to carry — "28 of 48" and twenty ids — was
    already three steps stale the day after it was written, because both ends
    move: every completion shrinks the denominator and every new step grows it.
    The recipe is the answer and it is self-updating:
    `grep -L 'Technical details (SOTA research' adocs/plan_todo/*.md` names
    what is left, `grep -l` the same pattern names what is done, and neither
    reaches a step sitting in `plan_current/`. **Snapshot, 2026-08-21, stated
    as a snapshot and not as a census: 22 enriched, 44 not, over the 66 files
    in `plan_todo/`.** Of the 44, five are excluded by design (reserve
    S023/S025/S110/S111 and parked S029) and nineteen are steps created after
    the pass stopped — S134 to S136 and S141 onward — so what the pass itself
    left behind is the other twenty. Several sections flag scope concerns for
    owner decision — the largest: S097's section notes Lynx's README
    self-rating (3144/3293) against DEC-087's ~2850 banding of Lynx-derived
    records. S109's no-prune-when-giving-check clause was the other one and is
    resolved: S139 split it, and what is left is the owner question the field
    now states, whether to buy late move pruning the exemption with a post-make
    prune.
  - **The 2026-08-16 plan_review findings are planned, not closed, and closing
    them needs a re-run.** Ten findings, one high and three medium: nine went to
    steps (S069 to S072, S074, S078 to S081) and F08 to DEC-062. A finding moves
    to `closed` only when a re-run no longer reports it, so a fourth plan_review
    is owed once those steps land — and the severity profile says the loop has
    not reached its stopping condition, which is a re-run with no high and no
    medium. F05 was this file's own two parked items and S069 has rewritten
    them; it is `planned` until the re-run, like the rest.
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
    stamps as provenance, not as evidence that any completion gate ran.
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
