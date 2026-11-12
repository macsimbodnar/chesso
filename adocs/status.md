# Status

Convenience view, rewritten by hand at the end of any turn that changed plan
state. The filesystem beats this file: on disagreement, `plan_current/` wins.
Nothing generates it since moltke v1 (DEC-109), so a stale line here is a
missed edit and not a tool's opinion.

Updated: 2026-11-12, by hand.

## 2026-11-12 00:55: S124's candidate is built and checked; its SPRT waits for a foreign load to end

**S124's candidate is staged in `../chesso-s124p2` (HEAD `7214ce5`), not committed.** Delivery
`.tuning/coord/S124/S124_candidate.diff`, sha256 `ecca2b17...`, 38 files; under `src/` three array lines and a comment.
`chesso bench` 4707943 -> **4587846**. The step file there, "2026-11-11: phase 2b, the candidate", has every golden.

**Two owner's stops fired (DEC-344 (9)); nothing was moved for either.** The SPRT plays the staged tree in DEC-334's
form and the owner rules only on H1.
- Mate-carry ceiling `C_mate7_depth11`: 10 against 4 on S124's grid (one cell, stride 1 at 2000000 nodes, 10 of 39
  lines short). `test_mate_carry` is green at its landed budgets; S170's new budgets are not applied.
- `test_tuner_gradient`'s bound: 1.69336e-6 at parameter 1002, red. DEC-316's script answers no pair and CLAMP_PUSH's
  no push. B1 (S102's per-column step, with the owner) was not applied.

**Seven tests red at the candidate, the same in both builds: the gradient case and six fixtures written around the old
evaluation.** `test_engine` first-mate (KRR v K reports its mate at 12, the parent at 9), `test_evaluation` "removing a
piece" (mop-up's share past `POSITIONAL_ROOM`), `test_search`'s two planted-bound cases (600 and 500 around the old
585), `test_en_passant_key`'s precondition (-96 against < -200), `test_audit_design`'s first case, and
`test_datagen_repetition`'s precondition. Options and the agent's recommendation for each are in the step file under
"Reds left". **Not repaired before the verdict**: on H0 the bisection's parts read different reds. After an H1, one
agent repairs them, with the fast check's low (the new mop-up case's precondition `> 3` should read `>= 4`).

**Checked on the candidate, 2026-11-12:**
- Fast check, one Opus agent, read-only: nothing blocks the run. The paste equals the screen's fit and P1's final; no
  assertion removed but three re-added with their new values; the SPRT script's guards, markers and arithmetic hold.
  The gates were read on binaries 49 minutes older than `src/evaluation.cpp`'s last write, so `bench` is read again
  on the binary the script rebuilds.
- `STAGES="debug sanitize clang" tools/gate_extra.sh` (`.tuning/coord/S124/gate_extra_cand/`): each stage red on the
  known tests and on nothing else; no sanitizer report; clang compiles the candidate.
  `test_datagen_repetition` is green under clang's library, red under GCC's.
- Debug self-play (DEC-141), 8 games at 4+0.04: 0 `Assertion`, 0 disconnects; the Debug `bench` 4587846.
- Cost, `adocs/data/S124_cost_masks.sh` (`.tuning/coord/S124/cost_masks.log`), instructions, five interleaved runs,
  node counts the parent's on all four builds: **the candidate +1.969 % on `bench`, +1.955 % on `bench 16`**; the
  factor alone +1.112 %, mop-up alone +0.970 %. Spread under 1800 instructions in 1e10. Taken with a foreign load on
  the machine; the counts are per process and the spread says they held.

**The weekly `tools/gate_extra.sh` on `7214ce5` ended `GATE-EXTRA-FAILED: clang`, 2026-11-12 00:09**
(`.tuning/gate_extra_2026-11-11.log`). Prose, citations, debug, sanitize and perft ok. Clang with `-Werror` refuses
`mopup_terms` and `scaled_taper` as not needed while the family sits at its identity; every commit since phase 1's
landing on 2026-11-06 is so, the MacBook's build of them included. **S340 is filed**, filler (DEC-171), second in
Open: it lands when S124's SPRT releases the machine, whatever the verdict. Not fixed first, because the
pre-registered script holds REF's `src/` to the screened tree's byte for byte. The last `GATE-EXTRA-DONE` stays
2026-11-05; the next full run follows S340.

**S340's fix is landed, 01:20, and S340 is paused by S341.** `[[maybe_unused]]` on `mopup_terms` and
`scaled_taper`, `No functional change`: 61 of 61 in both gated builds, bench 4707943, `tools/search_bench.py`
identical at depths 9 and 12, both bisection shapes compile under clang. Clang now compiles the whole tree and reads
a red the two errors had hidden: `test_tuner_gradient`'s factor case, 4.70e-6 over 5696 rows under libc++ against
6.27e-7 over 5717 under GCC. Likely cause, unverified: `placements()` draws with `std::shuffle`, which each library
implements its own way. **S341 is filed in `plan_current/`**, filler, and starts after S124's verdict. The MacBook's
`ctest -L fast` reads that red until then.

**Waiting, not on the owner: a foreign load.** Another session's `go test ./internal/watch/ -run TestSoak` (a 2 h
soak, started 00:38, heavy disk I/O) is running. No A/A and no SPRT starts while it does (MACHINE; the script's abort
rule). A watcher waits for its exit.

**Next, in order:**
1. **The A/A, 1000 games (DEC-143), about 28 minutes**: the kernel moved 7.2.6 -> 7.2.15 since the last one, and
   every pre-registration since S083 named the kernel in "the same machine". `CAND=7214ce5 REF=7214ce5 AA=1
   ROUNDS=500 ./fastchess.sh`. The reading rule is in S124's step file, written before the run.
2. **S124's one `{0, 5}` SPRT**, `adocs/data/S124_sprt.sh` in `../chesso-s124p2`: 12 to 20 h at the worst, cap 41900
   games, watcher ceiling 40 h. Nothing compiles while it plays.
3. S341 (which completes S340), then on H1 the reds' repair and the owner's ruling on the two stops; on H0 the bisection (A mop-up, B F1,
   C F2).
4. S339, filler. S102's landing after the owner's ruling on B1, then its screen (phase 2).

## 2026-11-11 22:50: P1 kept mop-up; S338 done; S124's candidate was being built

**P1 ended 22:40, `S124-P1-KEPT`.** Mop-up `mopup_mg = {96, 149, 0}`, `mopup_eg = {15, 13, 6}`. Fresh starts
converted: head 759, the candidate 862 of 984, exact one-sided p 1.5e-23, no class the other way. KBNvK still
converts 63 of 184. The step file's "2026-11-11: P1's reading" has the table. Evidence in
`../chesso-s124spsa/.tuning/s124/p1/`.

**S124's candidate is F1, F2 and mop-up**: `endgame_scale 45 0 9 29 81 114 86 128 123 19 128 128 128` (K 1.3248,
`../chesso-s124p2/.tuning/s124/screen/fits/f1_f2/full`), F3 dropped by the guard.

**S338 is done (`plan_done/`), this commit.** `test_mutant_anchors` validates the 264 mutants against the tree in
0.1 s; a mutation run excludes it by label. Gate on the idle machine: 61 of 61 in both builds, format clean, bench
4707943. **S339 is filed**, filler: the mutation tool scores a skipped test as a kill, and two crashes.

**S102 phase 1: delivered, staged in `../chesso-s102p1`, not landed, waiting on the owner's ruling on B1.**
- Delivery A (`.tuning/coord/S102/S102_phase1_A.diff`, sha256 `1003329c...`): phase 1, the two fast-check lows fixed,
  PT08 re-anchored. Its one red is `test_tuner_gradient`'s clamp case, 1.29e-6 at 1211 against 1e-6, the owner's
  (DEC-344 (9), DEC-356).
- B1 (`S102_phase1_B1_delta.diff`, sha256 `2a2556d9...`): the finite-difference step scaled per column. Clamp case
  1.72e-7 with `CLAMP_PUSH` +50, gradient case x6.94 under its bound, every pair's script picks its pair, 18 of 18
  planted errors red. A plus the delta reproduces the staged tree. Readings: `adocs/data/S102_clamp_options.txt` and
  `S102_b1_reading.txt` in that worktree.
- At landing: rebase onto S124's candidate, two rows for `S102_clamp_options.*` in `adocs/data/README.md`, B1's text
  takes its DEC id, and `test_mutant_anchors` must read green there.

**For the owner, asked 2026-11-11, nothing blocks on it:** the drift point of 2026-11-01 read +293.62 +/- 17.15
against `1680439` before S101 and S122 landed; S267's gate needs 232 and the path to the mark was priced at 282.
`plan.md`'s "Where chesso stands" is stale on this. The question: read S267's gate and S152's standing gauntlet
(10+0.2, about 3.5 h) after S124 and S102 land, with S266 and S133 moved behind it, or keep the written order. The
coordinator recommends the early reading and continues on the written order until the owner rules (DEC-108 and
DEC-258 are the owner's).

**Disk:** 8.5 GB free at 18:45. `.tuning/s122` holds 22 GB, `.tuning/s082` 9.2 GB, `.tuning/coord` 5.5 GB.

**This file was cut on 2026-11-11.** It had grown to 8863 lines, past what a session can read at its start. The
entries before 2026-11-06 are in history: `git show 5489c25:adocs/status.md`. The Parked block is carried whole, below.

## 2026-11-06 18:15: HANDOVER for a fresh session (the owner shuts the machine down)

Nothing runs. S124's screen was stopped by the coordinator (SIGTERM to its process group) during F3's round 2, and the
S102 agent was stopped at a safe point. Everything below is on disk and survives the shutdown.

**S124 phase 2: the screen, stopped mid-F3.** Worktree `../chesso-s124p2`, branch `s124p2` at `c496403`, clean.
- **Never commit or reset in that worktree before the screen ends.** Its `tools` stage froze the binaries at
  `c496403`, and a resume refuses another HEAD.
- **F1 kept:** d +0.00368 against m 0.0000282. Guard -0.0339 +/- 0.0005. Cells 46 / 0 / 20.
- **F2 kept:** d +0.001851 against m 0.0000209 at K 1.3248, quarters 0.001829 to 0.001859. Guard -0.00686 +/- 0.00027.
  F2's fit: `endgame_scale 45 0 9 29 81 114 86 128 123 19 128 128 128`.
- **F3:** its full fit's round 1 is done (K refit 1.3274). Round 2 was stopped, and its partial `fit.log` has no `best:`,
  so a resume re-runs it. The five quarter fits have not started.
- **Resume:**
  ```
  cd /home/max/ws/chesso-s124p2
  nohup setsid -w nice -n 10 adocs/data/S124_screen.sh all >> .tuning/s124/screen/all.log 2>&1 & echo $!
  ```
  Leave the `cd` out of the backgrounded line, as written. `all` skips F1 and F2 (PART-OUTCOME read), reuses every
  decided fit and every round with `best:`. Watch for `S124-ALL-(DONE|FAILED)` with the four-exit loop, PID check,
  ceiling about 8 h. F3 needs 2 to 3 h.
- **Then P1:** mop-up's SPSA, about 4 h, ceiling 9 h. It runs from `../chesso-s124spsa`, reset to `54c892d` and built.
  The command is in the step file's "The coordinator's run". `--train-starts` is
  `../chesso-s124p2/.tuning/s124/screen/p0/starts.tsv`.
- **Then:** the cost (`S124_phase1_cost.sh` at the candidate), the candidate's paste, and one `{0, 5}` SPRT of the
  survivors (F1, F2, F3's if kept, mop-up if P1 keeps it). Budget 12 to 20 h: a night.

**S102 phase 1: paused** in `../chesso-s102p1`, branch `s102p1` at `42f2aa7`. Its work is staged, not committed.
- State and next steps: the step file's "2026-11-06: phase 1 paused for a shutdown" section, inside that worktree.
- WIP diff: `.tuning/coord/S102/S102_phase1_wip.diff`, with its sidecar.
- Built so far: the engine side at zero weights, with objdiff identical and bench 4707943, and the tuner layout
  1190 -> 1214. Still red: `test_eval_trace` (its coverage block is owed) and `test_tuner_gradient` (ten zero gradients).
- **For the coordinator's review:** the agent proposes ending the endgame factor case's non-vacuity loop at
  `ENDGAME_SCALE_END` rather than `PARAM_COUNT`, since S102's block now follows. It narrows a test's range: rule on
  it before it lands.
- A fresh agent resumes it from that section. It rebases if S124's candidate lands first.

**Done today, committed:**
- S124 phase 1 landed (`96b70fc`). DEC-355 and DEC-356 (the owner's) applied there; DEC-357 and DEC-358 (the
  coordinator's) are the screen's guard and P1's kept rule.
- S124 phase 2a (`c496403`) and P1's instrument (`54c892d`).
- Filler closed: S333, S334, S335, S336, S337.

**Machine notes:**
- Take a run's PID with `setsid -w` (`.moltke.local.md`).
- Disk was about 8 GB free at 18:10. The S102 worktree and the P1 scratch take more.
- A copy of S122's stamped preparation records is in `.tuning/s336/prepared_stamped_backup/`.

## 2026-11-06 18:10: S337 done; S102 phase 1 started behind S124's screen

- **S337 is done** (`ef866de`). The filler queue is empty.
- **Running: S124's screen**, PID 499381, on F2.
- **Running: S102 phase 1**, one Opus 5.5 agent in `../chesso-s102p1` (branch `s102p1`). It builds the family at its
  identity under DEC-351. No fit or match. If S124's candidate lands first, it rebases.
- **P1 waits for the screen:** worktree `../chesso-s124spsa`, built at `54c892d`.
- **Disk:** 8.3 GB free after removing today's closed steps' smoke scratch (about 1 GB). The S102 agent stops under 5 GB.

## 2026-11-06 17:40: S336 done; S124's screen on F2; P1 waits for the screen

**S336 is done (`plan_done/`), this commit.** Later stages' records drop when an earlier stage makes its output, and
`seeds` passes on the real tree. A copy of the stamped preparation's 13 small records is in
`.tuning/s336/prepared_stamped_backup/`, with SHA256SUMS.

**P1's instrument is landed (`54c892d`).** Its run worktree is `../chesso-s124spsa`, reset to `54c892d` and built. It
runs after the screen (about 4 h, ceiling 9 h): the command is in the step file's "The coordinator's run".

**S337 done** (this commit). The filler queue is empty: S124 is the only open step ahead.

## 2026-11-06 16:40: S335 done; S124's F1 kept, the screen on F2; P1's instrument landing

**S335 is done (`plan_done/`), this commit.** The merge's count stands as booked (DEC-296). `read` refuses a bad field on
one line, and the interrupted-merge check tells before the rename from after it.

**S124's screen, F1 kept** (16:15): d +0.00368 against m 0.0000282 over five quarters, at K refit 1.2647.
- The guard passes: the tablebase-labelled loss moves -0.0339 +/- 0.0005.
- Fitted `endgame_scale` cells: P=0 d<=0 46, P=0 d=1 0, OCB_PURE P<=2 20 (128 the identity).
- The screen runs on with F2, then F3.

**P1's instrument** (mop-up SPSA, option (b)), one fast check, DEC-358 its kept rule. It lands next. It runs about 4 h
after the screen, from a worktree at its commit.

## 2026-11-06 15:45: S334 done; S124's screen on F1; P1's instrument being built

**S334 is done (`plan_done/`), this commit.** The ledger's S122 row reads "the king safety family" through
`tools/ledger_overrides.tsv`. The gate ran 60 of 60 in both builds with the screen paused for 6 min (SIGSTOP/SIGCONT on
its process group), because under its load `test_tuner_gradient_plants_threats` passes its 60 s clock only unloaded.
Filed: S337 for the four other garbled rows.

**P0 on head** (the screen's reading): 854 of 1126 starts converted, 0.758. KQvK 200/200, KRNvK 190/200, KRBvK 183/200,
KBBvK 102/126, KRvK 158/200, **KBNvK 21/200** (fifty 171, stalemate 8). So P1 runs.
- Its instrument, option (b), is being built by an Opus 5.5 agent in `../chesso-s124spsa` (branch `s124p1spsa`): SPSA
  that rebuilds per point, no `src/` or UCI change.
- The coordinator runs P1 after the screen.

## 2026-11-06 14:45: S124's screen running; phase 2a landed (`c496403`)

**Running: `S124_screen.sh all`**, started 14:44 from `../chesso-s124p2` (branch `s124p2` at `c496403`, clean). Corpus
read from the main tree by path, so main-tree commits do not void it.
- PID 499381, in its own session, nice 10. Log `../chesso-s124p2/.tuning/s124/screen/all.log`, OUT beside it.
- Stages: locate, tools, identity, k, guard, p0, then parts F1, F2, F3. Typical 6 to 9 h, worst about 30 h a part.
- Watchers armed: one to P0's marker (2 h ceiling), one to `S124-ALL-(DONE|FAILED)` (24 h ceiling). Each exits on
  its marker, a failure marker, the PID's death, or its ceiling.
- Taken while S334, S335 and S336 are open (DEC-171). None reaches the screen: it calls neither `ledger.py`, nor the
  merge, nor `seeds`.
- **Do not reset or commit in `../chesso-s124p2` until the marker**: its `tools` stage froze the binaries at `c496403`.

**Landed: phase 2a** (`c496403`). `feature_audit` reads a local rank of exactly 1142 of 1190. The screen and P0 carry
DEC-357's guard. Two fast checks were run: the first found a blocker, ruled by DEC-357; every finding is fixed.

**Next:** S334 behind the screen, then S335 and S336. If P0 shows non-conversion, mop-up's SPSA instrument, option (b),
is built behind the screen as well.

## 2026-11-06: S124 phase 2a built, its fast check's blocker ruled (DEC-357), fixes in progress

- **Phase 2a delivered** in `../chesso-s124p2` (staged, `.tuning/coord/S124/S124_phase2a.diff`):
  - `feature_audit`'s local rank reads rank exactly 1142 of 1190 at the shipped point, nullity 48, every one accounted for;
  - the screen `S124_screen.sh` and P0's `S124_p0.py`;
  - preconditions on S122's corpus: OCB_PURE P<=2 on 2438 games, so no stop before F1. K at the identity is 1.2171.
- **The fast check's blocker:** the tablebase guard read each vector at its own K, so K alone refused an inert part.
  DEC-357 rules one common K, the part's decided K, with K_0 printed beside. Five lows are being fixed with it.
- **P0's smoke:** 3 and 5 of 12 starts failed to convert (KBNvK among them), so mop-up's SPSA instrument is likely
  needed. Option (b), a driver that rebuilds per point, is recommended, not built.
- **Then:** land phase 2a, run the screen from its own worktree (corpus read from the main tree by path), so main-tree
  commits stay free and S334, S335 and S336 run behind it.

## 2026-11-06: S333 done; next S124 phase 2, the filler behind it

**S333 is done (`plan_done/`), this commit.** Every record of S122's preparation names its pool; `verify` refuses a
pool written before S331; `adopt` stamped `.tuning/s122/prepared/` in place after checking it, K 1.2302 as on record
(old records in `.tuning/s333/prepared_backup/`). One fast check, its real finding fixed. Filed: S336, its two leftovers.

**Order changed by the coordinator:** S124 phase 2 is first in Open, and S334, S335 and S336 run behind it as filler
(DEC-171), one at a time, while its screen holds the machine.

**Machine note:** plain `nohup setsid ... &` gives a dead `$!` under the harness's job control; `.moltke.local.md` and
the watcher recipe now use `setsid -w`.

## 2026-11-06: S124 phase 1 landed at the identity; next S333

**S124 phase 1 is in the tree** (this commit), `No functional change`, bench 4707943 in both builds.
- The owner ruled RED 1: option A, DEC-356. The factor gradient case takes DEC-316's pair, 6.27485e-7 at 739 (x1.59).
- DEC-355 is implemented in the tighter signed form the fast check asked for: each row's taperings' loss
  |smooth - exact - C|, C the curve's signed residual, against the taperings' bound alone. 0 rows past it on 6226944,
  shipped and forced; plants 9 and 10 red; a 1e-6 margin over 69/24 (`LATTICE_MARGIN`).
- Goldens re-derived under dated rule amendments: `cell_truncation_positions` (one hit, 2.906576) and
  `truncation_positions` (phases 5, 11, 17, 23 at 2.875).
- Gate: 59 of 59 in `build/` and `build-tune/`, format clean.
- S124 stays in `plan_current/`. Phase 2 needs `feature_audit`'s local rank first, then the screen at K refit (DEC-352).
- Removed `../chesso-s124p1b` (every blob in history; README and the step file are the merged versions) and
  `../chesso-s124p1` (its staged diff equals `.tuning/coord/S124/S124_phase1.diff`, `c3c8f2d2...`), branches too. Free 12 GB.

**Running: S333**, one Opus 5.5 agent in the main tree, no `src/`. Then S334, S335, then S124 phase 2.

## Parked (human memory, carried forward whole on 2026-11-11)

- Parked:
  - **A full mutation pass over `tools/mutants/` on the current tree, a night
    job (2026-09-22):** S188's 34-mutant pass found S097's E21 unpinned by the
    deeper tree -- its mined row stopped separating the guard-dropped build
    -- and the pass covered only the X and E prefixes, so every other mined
    row and every older guard is unproved on a tree that has moved three
    times since it was killed. 128 mutants at about 240 s each is 8.5 h;
    schedule it for a night the machine has no run, or fold it into
    `tools/gate_extra.sh`'s weekly run as a sixth stage if the tool learns to
    run in the background of nothing. The two mined-row mutants outside S188's
    pass (S095's J row, S230's R01) are re-proved targeted before S188 lands.
  - ~~**S132's question for the owner, 2026-09-21**~~ **decided 2026-09-22 by
    the owner, DEC-229: a dedicated confirmation at 32+0.32, the same pair,
    capped at one night, run when S188's SPRT frees the machine.** The
    question as it was put: the node-fraction time manager read H1 at 8+0.08
    (`Elo 16.28 +/- 8.47`); its constants are census seeds at the playing
    control. Does it get one confirmation at a second control before they are
    called shipped -- a 40+0.4-class run of its own (about 3 h 45 m at the
    block boundary's 534 games an hour for 1000 pairs, or a full `{0, 5}`
    night), or folded into S152's rated run at the list's own control
    (DEC-108)? The reason is S085's: an SPSA'd time manager once measured
    +23.8 at 20+0.2 and -22.9 at 10+0.1, and every published node-TM patch
    the step read was verified at two to four controls. **Coordinator's
    recommendation: fold it into the next block boundary's longer-control
    reading (DEC-202, 1000 pairs at 32+0.32) rather than spend a night now**
    -- that reading is already owed, it runs the whole tree at the second
    control, and a time-manager regression there would show as the boundary
    drift it is designed to catch; a dedicated run is bought only if that
    reading moves. S127 excludes the three from any lane at a control the
    verification does not share either way.
  - ~~pixello's two test sounds~~ **closed 2026-09-13 by DEC-207**: the
    submodule leaves the branch with S227.
  - ~~**Three findings filed by S195, none of them planned.**~~ **Closed
    2026-09-11, DEC-184:** the `ucinewgame` and `nodes` sentences go to
    S210; `command_bench`'s per-position `reset_for_new_game()` is accepted
    and kept -- it is what makes the signature independent of position order.
  - ~~**Calibrate the harness on the workstation.**~~ **Taken 2026-09-08 as
    S198's A/A and retired**: 1000 fixed rounds, 0 forfeits, pair variance
    0.2430 +/- 0.0154 inside S105's band at `z = +0.16`, 2277 games an hour.
    The next machine change owes the next one under DEC-143.
  - ~~**The three lows of the 2026-09-04 audit re-run still wait on the
    owner.**~~ **Closed 2026-09-11, DEC-181:** F01 was already S210's; F02 and
    F03 fold into it, and the report's `Status:` lines say `planned`.
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

    **S203 moved these numbers on 2026-09-08 and the check still works, with
    the new set.** Redrawing the Zobrist keys changes which positions share a
    transposition-table slot, so the counts move once by design; the values
    below are what the engine gave until that step, and from it on they are
    **121530 / 801481 / 72924** at depth 9 and **636677 / 3520847 / 494098** at
    depth 12, best moves unchanged. `adocs/specs.md` under INV-6 carries the
    current pair and is the one to read. The original text follows unedited,
    because what it says about which figures carry across a machine and which do
    not is the standing rule.

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
    can produce. **Answered on 2026-09-10:** all five have been tracked since
    `c56ab41` and S192 moved that one out of `.tuning/` altogether, to
    `adocs/data/S192_anchors.py`. The corpus is what still does not survive a
    move.
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
  - ~~**Nothing checks that the two copies of the completion gate agree.**~~
    **Closed 2026-09-11, DEC-184:** a `--gate` mode in
    `tools/plan_prose_check.py`, registered in the fast suite; S214.
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
  - ~~**A fourth question, from S085's run: what should `RFP_MIN_PLY`'s
    declared minimum be?**~~ **Closed before 2026-09-11 and only now
    removed:** S142 set the floor to 2, the measured one, and S148 re-decided
    the ceiling by SPRT and kept 15; `src/search_params.hpp` carries both
    with their reasons.
  - ~~**Three questions banked for the owner while working overnight,
    2026-08-20.**~~ **Closed 2026-09-11:** (1) the enrichment pass is DEC-184's
    item 7, continuing as filler; (2) S085's goal line is history in
    `plan_done/` and its stamp carries the deviation; (3) the re-measurement
    of deferred numbers was S184.
  - **The enrichment pass continues as filler, DEC-184 (2026-09-11).** Two
    passes have run: the 2026-08-19 research sections (`## Technical details
    (SOTA research`, stopped at S120 by the owner on 2026-08-20) and the
    2026-09-05 implementation guides (DEC-145, twenty instrument-lane files).
    **Snapshot 2026-09-11: 37 of the 59 pending files carry neither**, and the
    recipe is self-updating -- a file with neither section is one
    `grep -L` away. Under the 2026-09-11 delegation the pass continues one
    file at a time as a step nears the top of the Open list, agent-only,
    owning no run and blocking none; it is not a plan step. The scope
    concerns earlier sections flagged are resolved: S097's Lynx banding by
    S181 and DEC-176, S109's LMP exemption by DEC-180.
  - ~~**The 2026-08-16 plan_review findings are planned, not closed, and
    closing them needs a re-run.**~~ **Closed:** the 2026-09-04 plan review
    re-assessed all ten -- nine closed, F08 accepted at DEC-062 -- under its
    "Verdicts on prior findings". Removed 2026-09-11.
  - ~~**The corpus is gitignored and does not survive a machine move.**~~
    **Closed 2026-09-11, DEC-183:** it stays gitignored and is archived with
    its recipe at `/home/max/Synckeeper/chesso_selfplay_v2_corpus_2026-09-11.tar.xz`
    (91999956 bytes, sha256 in the DEC and beside the file); the TSV's digest
    was verified by streaming it back out of the archive. The fit scripts were
    already tracked (S192). A new corpus -- S082, S083 -- gets the same
    treatment in its stamp.
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
    **The book question is closed 2026-09-12, DEC-182, DEC-189, DEC-190:**
    S219 measured four CC0 books and the harness plays the balanced
    `noob_3moves.epd`; the A/A on it read 2110 games an hour and pair variance
    0.2905 +/- 0.0184, the band every later A/A on this book is read against.
    The x1.20 the unbalanced book bought in game length is given back (114
    plies a game against 102), and the balanced book gives 1.13x the score for
    the same strength difference; a verdict costs what it did, +/- 12 %.
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
  - ~~**The `info` line is UCI surface that the golden guard does not
    cover.**~~ **Closed 2026-09-11, DEC-184:** a golden over the field set,
    values excluded, joins `test_uci_surface` in S213.
  - ~~**The tuner's last `--only` group runs to `PARAM_COUNT`, and that is a
    blind spot S041's test cannot cover.**~~ **Accepted 2026-09-11 as to
    structure, DEC-184:** the precondition that does fire gets a message
    naming what it detects, in S214; the group table stays as it is.
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
    **Reaffirmed by the owner on 2026-09-11, DEC-179**, after S183 put the
    whole hand-crafted list at 2658 to 2817: the mark is reached without a
    network, the list is extended (S217), and S029 is un-parked by a decision
    taken when S152 reads the mark -- not before, and not by arithmetic.
