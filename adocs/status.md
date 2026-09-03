# Status

Convenience view, rewritten by hand at the end of any turn that changed plan
state. The filesystem beats this file: on disagreement, `plan_current/` wins.
Nothing generates it since moltke v1 (DEC-109), so a stale line here is a
missed edit and not a tool's opinion.

Updated: 2026-09-03, by hand.

- In progress: **nothing.** `adocs/plan_current/` is empty.
- Last done: **S146, 2026-09-03 -- the book the engine ships with is built by
  this project, from a source it can account for.** The owner took the second of
  DEC-126's three standing options: the unaccounted blob is **deleted**, and
  `src/openings.bin` is now `build/tools/make_book build books/8moves_v3.pgn
  --out src/openings.bin` at the tool's defaults -- **2755712 bytes, 172232
  entries over 129613 positions, sha256 `3b89a4ad9146e266ae9296778067aaedcb7f57
  f3cf0ff2086b9ae6df15b873dd`**. DEC-131 is the ruling.
- **What made it possible was S172, not new evidence about the blob.** DEC-126's
  refutation still stands -- `books/8moves_v3.pgn` shares only 11703 of the old
  book's 154916 positions, 7.6 % -- so this is a **different book, not a
  re-attribution**. What changed in a day is that S172 built `tools/make_book`;
  until 2026-09-03 nothing in this repository could produce a Polyglot book, so
  "build a replacement" had always named a tool that did not exist. The old
  163141-entry file is recoverable from `62d07d4` and from nowhere else.
- **The build is reproducible, which the old file could never be.** The writer
  sorts by key then by weight, so the digest is a function of the PGN and two
  runs are byte-identical (`cmp`, verified). Defaults were used and both are
  right rather than convenient: `--max-ply 16` is the full depth of every line
  in that PGN (**34700 games, 0 cut short, 555200 plies, exactly 34700 x 16**)
  and `--min-games 1` drops nothing. The input is CC0-1.0 and pinned by both
  digests in `books/fetch_book.sh`; the SAN is read by the engine's own
  `algebraic_to_move` and keyed by its own `get_key`, so no other engine's code,
  table or output is anywhere in the path.
- **No SPRT is owed, on S172's reasoning and not a shortcut.** `OwnBook` defaults
  false and S158 established that no measurement this project has taken ever
  played a book move. **INV-6 discharged on the default configuration**:
  121512 / 800769 / 62907 at depth 9 and 639228 / 3430710 / 367858 at depth 12,
  `c3d5` / `e2a6` / `d7c8q` at both. Weight now means something it never did --
  every game in that PGN is recorded `1/2-1/2`, so an entry's weight is the
  count of the 34700 lines that played the move; `Best Book Move` true gives
  `e2e4` at weight 12956.
- **One defect found and fixed in scope, worse than the report of it.** The fast
  check over S172's diff flagged that `make_book build` never checked its output
  stream. Reproduced on a 1 MB HFS ram disk rather than assumed: the tool wrote
  **901120 of 2755712 bytes, printed `bytes 2755712 -> <path>` and exited 0**.
  The part the report missed is why it was invisible -- entries are sixteen
  bytes and sorted, so **any prefix of a valid book is a valid book**, and
  `make_book dump` called the truncated file **`loadable`** with 56320 entries
  while the engine loaded it. Both of the loader's checks are structurally
  incapable of noticing truncation; only a digest is. Fixed, red observed before
  and green after; not a ctest, because the tool has no test target and macOS
  has no `/dev/full`, so the reproduction is in the step file and the hazard is
  in `DEV_MANUAL.md`.
- **The stale citations went with the blob.** `src/openings.book` had not existed
  since `62d07d4` and S146's own reproduction commands still `sed`-ed a
  `#define BOOK` out of it. Re-anchored across `DEV_MANUAL.md`, `adocs/specs.md`,
  `MANUAL.md` (new section "The book it ships with"), `src/openings_embedded.S`,
  `src/openings.cpp` and `tests/test_openings.cpp`. `polyglot_randoms[781]`
  needed nothing -- settled at DEC-121 on 2026-09-02, untouched.
- **S146's own fast check produced one finding, which was wrong, whose fix is
  right.** It reported that the new failure path's `std::remove` destroys a
  pre-existing file at `--out`. It does not: `std::ofstream(path, binary)` opens
  `"wb"` and truncates, so the contents are gone at the open -- verified, a bare
  open with no write and no remove took a nine-byte file to zero. What is real
  is that the whole replacement is not atomic: the tool destroys the old book
  before it knows it can write the new one. **S173 is that fix** -- write beside
  the destination, rename on success -- filed in `plan_todo/` and 49th in the
  Open list, not urgent because `--out` points at `src/openings.bin` about once
  and that file is in git. The other four answers checked out: `close()` then
  `good()` is the right order, `std::remove` binds the `<cstdio>` overload
  unambiguously, and the entry count and sha256 in the comments match the file.
- **The fast check over S172 is done and it took two passes.** The first returned
  "No issues" over 631 new lines of PGN parsing and a rewritten `setoption`; that
  was not accepted, and a second pass answering six named questions with quoted
  code returned the short-write finding above. Recorded because the lesson is
  cheap: a clean review and a shallow one look identical until the questions are
  specific.
- **S171 is postponed to the desktop workstation (DEC-128), not blocked and not
  abandoned.** Its file is in `adocs/plan_todo/` and its Open entry is last in
  the list tagged `postponed`, so it derives as nobody's next step. **The fix is
  committed and green at `136b03f`** -- `certified_mate_move()` completes a mate
  line across a table slot the walk has lost, `tests/test_mate_carry.cpp` guards
  it, INV-6 discharged on the same node counts as above.
- **What S171 owes is a machine, not a change.** One `fastchess.sh --fast`
  census, 3000 games at 8+0.08, about two hours, accepted at **0**
  `Incomplete mating PV` lines from the candidate:

      REF=457e355 nohup ./fastchess.sh --fast > .tuning/sprt_s171_matepv.log 2>&1 &

  `457e355` is the commit before the fix, so the reference prints its own count
  in the same match and nothing is compared across machines. Two attempts here
  died on the machine: `pmset -g ac` said `No adapter attached`, which the POWER
  rule forbids (DEC-109), and on mains `fastchess.sh`'s own load guard reported
  `about 387% of a core is already busy` -- Spotlight indexing PDFs. **That
  indexing has since finished**: load average was 2.4 on eight cores while
  S146's gate ran, against about 15 earlier the same day. The standing figure
  stays 5 lines from 1 search in 3000 games in `adocs/specs.md` and beside
  instrument 3 in `DEV_MANUAL.md`, and neither claims a silence that has not
  been measured.
- Before S146: **S172, 2026-09-03 -- an opening book is loadable over UCI, on
  the option names the protocol actually uses.** `OwnBook` (check, default
  false), `Book File` (string, default `<embedded>`) and `Best Book Move`
  (check, default false) replace `Use Book`, which is removed. A named book that
  will not load leaves the engine **bookless** -- never falling back to the
  built-in one -- and says so as `info string book [<path>] not loaded: <why>`
  on the UCI channel. Selection follows the Polyglot `weight`, which the engine
  had never read. Three things that had never run at all were fixed:
  `load_book_from_file()` had **no caller anywhere** since the `bitboard`
  branch; `setoption` read **one token** as its value, so a path with a space
  arrived truncated; and the probe **scanned every entry** where the format's
  key ordering makes a binary search exact. The container also changed --
  `src/openings.bin` pulled in by `.incbin` instead of a 5.2 MB hex header
  decoded at every startup, **4.7 ms +/- 0.4 to 2.8 ms +/- 0.1, x1.67 +/- 0.15**
  over 574 and 976 `hyperfine -N` runs. DEC-129, DEC-130.
- Before S172: S170 -- **a mate score the search did not itself prove is
  reported with a line that reaches it.** Three causes, one fix each, all
  reporting-only and all under DEC-122; `tests/test_mate_carry.cpp` is the guard
  and `adocs/data/S170_replay.py` with `S170_cases.tsv` the reproduction. Its
  own `--fast` run took 12 lines over 3 searches down to 5 over 1, and the 5
  became S171 above.
- Before it: S147 (**a mate line the search proved reaches its mate**, 54 short
  of 706 before and 0 after; DEC-122, DEC-123), S144 (**a citation in a plan
  document carries its own path**; DEC-120), S169 (**the 97 stale citations are
  re-anchored**; DEC-119), S143 (**the completion gate builds and tests
  `build-tune` beside `build`**; DEC-118).
- **S146's first half was DEC-121 and it still stands**: `polyglot_randoms[781]`
  is format-defining specification, kept, cited at the array, with all 781
  constants verified element by element against the live format description.
  The second half is closed above at DEC-131 and the blob is gone; DEC-126's
  measurement of why the PGN was never the blob's origin -- **11703 positions of
  the book's 154916, 7.6 %** -- is what keeps the replacement honest as a
  replacement rather than an attribution.
- **What the episode did settle, and it is committed.** `books/8moves_v3.pgn`
  is committed, played by `rating.sh`, and had no recorded origin anywhere. It
  is byte-identical to the file in `official-stockfish/books`, which is
  **CC0-1.0**, and is now pinned in `books/fetch_book.sh` by both digests --
  zip `7e1e9dd1...`, unpacked `5835239f...` -- so one file answers where every
  book here came from, and `./books/fetch_book.sh 8moves_v3.pgn` verifies the
  tracked copy in place instead of downloading. `UHO_Lichess_4852_v1.epd`
  needed nothing: that script already carried its digests and the licence
  reasoning. `DEV_MANUAL.md` says the same.
- **`python-chess` was missing on this machine and is installed now**, at the
  owner's decision of 2026-09-01: 1.11.2 on python 3.9.6 in `~/.venv/chess`,
  the path `TOOLCHAIN.md` and every S145 script already name. It had not
  survived the move from the Linux workstation, so
  `adocs/data/S145_rfp_sweep.py` and `S145_mate_set.py` could not run here at
  all and nothing said so. `.moltke.local.md` records it now.
- Next: **S020**, the first entry in `plan.md`'s Open list -- compute the
  in-check state once per node instead of once per call site. It is
  behaviour-neutral and discharges on identical `tools/search_bench.py` node
  counts and best moves plus a `hyperfine` timing, so it is machine-light and
  does not want the idle machine S171 does. S030 is the other half of that
  pair. **On the desktop workstation, S171's census comes first.**
- Blocked: **nothing.**
- Watching: **nothing. No run is armed.** The 2026-09-03 SPRT attempt was killed
  a minute in and no watcher was ever armed for it; the S172 and S146 gates ran
  in the foreground of their own turns and are finished, and S146's fast-check
  subagent has reported and exited.
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
