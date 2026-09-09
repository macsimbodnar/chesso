# Status

Convenience view, rewritten by hand at the end of any turn that changed plan
state. The filesystem beats this file: on disagreement, `plan_current/` wins.
Nothing generates it since moltke v1 (DEC-109), so a stale line here is a
missed edit and not a tool's opinion.

Updated: 2026-09-08, by hand.

- **S148 is done, 2026-09-09: the reverse futility ceiling stays at 15, and
  the deep mates it loses are its measured price.** `{-5, 0}` nElo at 8+0.08,
  **H0 accepted at LLR -2.95** -- `Elo -5.66 +/- 4.33, nElo -7.31 +/- 5.60`
  over **14808 games in 6 h 19 m 35 s, 0 time forfeits**, LOS 0.52 %,
  `adocs/data/S148_sprt.log`. The candidate was one integer, `RfpMaxDepth`
  15 -> 4, and it is reverted. The reading was written into
  `adocs/data/S148_sprt.sh` before the first game and applied unchanged: a
  ceiling low enough to find the deep mates costs more than five nElo of
  ordinary play, so the mates the engine does not find -- **1 of 16 at four,
  0 of 16 at five** -- are what the pruning costs. DEC-158, and the five
  places 15 is written now carry the price beside it: `specs.md`, both
  `MANUAL.md` sites, `DEV_MANUAL.md`'s mate table (re-taken over 82 rows, it
  still read 48 and S154's counts), `src/search_params.hpp`'s comment and the
  mate suite's own. **A verdict of "keep the incumbent" is a result and the
  step completes on it**, which is the S005 / S006 / S015 precedent read in
  the other direction.
- **Neither deep class was promoted to an asserted floor, and the reason is
  not that they are zero.** At the shipping value mate in four is **1 of 16**,
  which satisfies the accepts' "non-zero" literally -- but a floor of 1 has no
  margin between its ends, which is what DEC-116 rejected for the mate in
  three, and the owner decided on 2026-09-08 that a class at exactly 1 stays
  recorded. Both stay in the `MESSAGE`; the comment above
  `MATE_IN_THREE_FLOOR` now says which, why, and what the refused ceiling
  would have made them.
- **Two facts from the run are worth more than the verdict.** The
  `Incomplete mating PV` class was read from both sides -- **7 lines from the
  candidate against 13 from the reference** -- so the class is not this
  ceiling's and the lower ceiling produced *fewer*; **S202 inherits that**.
  And `test_mate_carry` went red on the candidate and was established as
  budget calibration **before** 18 hours of machine were committed:
  `adocs/data/S203_case_sweep.sh` shows the short lines appearing and
  vanishing with the node budget in both directions -- A short 2 at 1000000
  and 0 at 1200000, 1500000, 2000000, 3000000 and 4000000 -- which is
  DEC-122's eviction class and S203's own knife-edge warning, not a defect
  this change introduced.
- **One thing S187 should know before it starts.** The `src/search_params.hpp`
  comment grew by eight lines here, which shifted every line-number citation
  below it and took `--citations` from 52 flags to 59. The seven are restored
  mechanically -- S098:120, S109:193, S114:87 and :117, S120:89 and :192,
  S132:87, each **+8** -- so the check reads **52 over 66 files** again, the
  same 52 the parent read over 67. It is red on both sides and has been since
  before this step; that is F07 and S187 owns it. One of the seven is a worked
  example of why the unit is wrong: **S109:193 names `LMR_DIVISOR` and points
  at the reverse futility comment**, and the checker cannot see that, because
  all it verifies is that the cited line still holds the text it held.
- **The sweep is the reusable half.** The finer grid was measured before the
  run and the challenger decided by the rule the file pre-registered: `adocs/data/S148_rfp_ceiling_sweep.py --mined` at `192a5a3`,
  every ceiling from 0 to 15 over the 82 constructed rows and the 318 mined
  ones, `RfpMinPly` held at 3 and read from the binary. **C1 = 4**, because the
  mate in five class is the binding one and it is a cliff -- 11 / 9 / 6 / 4 / 1
  of 16 at ceilings 0 to 4 and 0 from 5 up. The whole constructed set moves
  **39 to 52 of 82** between the shipping 15 and 4, the mined set 145 to 159 of
  318, and `short` and `sign` are 0 at all sixteen settings. Two readings the
  coarse grids could not give: the **plateau starts at 10**, not at 15, and the
  mate in four class is 1 of 16 at the shipping value rather than S145's
  "0 of 8", the set having grown.
- **The owner answered all five deferred questions on 2026-09-08, every one as
  the file recommended**, and the answers are in the step file: `{-5, 0}`
  `--nonreg`; **C1 alone**, no C2 = 6 fallback; **keep 15** on a null or a
  rejection; the re-run lands in `S148_rfp_ceiling_sweep.log` because S145's is
  evidence; the excluded dynamic-cutoff sentence reworded off Stockfish's two
  constants (DEC-134). The fifth binds: a class whose count is exactly 1 stays
  in the `MESSAGE`, and at C1 the mate in five count **is** 1, so only the mate
  in four class is promoted when the value ships.
- **The candidate was not behaviour-neutral, which is why INV-6 could not
  discharge it and the games had to.**
  `search_bench.py` depth 9 **124511 / 805638 / 111393** against HEAD's
  121530 / 801481 / 72924, depth 12 **716171 / 3707680 / 560079** against
  636677 / 3520847 / 494098, `bench` **28339749** against 26851183, best moves
  `c3d5` / `e2a6` / `d7c8q` unchanged throughout. Debug self-play **0
  Assertion over 8 games**, 0 disconnect, crash or illegal. Gate green at HEAD
  in both builds, 28/28, before the integer moved.
- **S184 is done, 2026-09-08: the pending documents state the engine as it
  ships, and the parameter checker now reads them.** Every F05 value restored
  from `src/search_params.hpp` at HEAD -- S115's aspiration sweep designed on
  2 / 21 / 437 with the "keep 5" instruction gone, S082's quiescence cap on 19,
  the lazy clamp on 184 in S120, S122 and `plan.md`, S118's cost paragraph
  dated to before S104 with `specs.md`'s 53.90 ns quoted beside it. S042's
  `touches:` names all four sites that build the en-passant key and its body
  states the one rule applied at each. Five F09 sentences and two F10 headers.
  Documents only: `git diff --stat -- src/` empty, 28/28 in both builds.
  Closes `2026-09-04_plan_review-F05`, `-F08`, `-F09`, `-F10`.
- **Three things from S184 are worth knowing before the next step.**
  `tools/plan_prose_check.py --params` now reads the **68 pending step files**
  as well as the four documents, and matches the **C++ symbol** -- with both
  backticks mandatory, because optional they make S114's formula line read as a
  claim about its divisor. So a pending file stating a stale parameter is a red
  fast suite from now on, and the cost is 1.27 s a run against 0.37 s.
  **S171's three F09/F10 items are void, not deferred**: that step completed
  into `plan_done/` on 2026-09-07/08 and `plan_done/` is never edited -- its
  `done:` stamp sits at line 551 rather than in the header, which no longer
  matters. And **an absent `author:` is the corpus convention** for an
  unstarted step, not a defect: 47 of 48 `plan_todo/` headers omit it, which
  is why S024 got one and then had it reverted. The header audit that
  established this found one fresh instance of F10's real class, S202's missing
  `done:`, now added.
- **S180 is done, 2026-09-08: no pending step tells an implementer to start a
  sweep from another engine's number.** Every seed in the `### 4. Constants and
  seeds` section of S095, S097, S098, S109, S113, S114 and S132 is now one of
  three declared forms -- **(a)** a literature value with its URL, **(b)** a
  derivation over chesso's own data or scale written as the procedure its
  owning step runs, **(c)** a declared range's midpoint or off value stated as
  such. F01's ten rows walked plus about a dozen the inventory found.
  Documents only: no `src/`, nothing measured, `GATE-DONE 26851183` unchanged
  from the parent, fast suite 28/28 in both builds. Closes
  `2026-09-04_plan_review-F01`.
- **Three replacements are worth knowing before their steps start.**
  `PROBCUT_MARGIN` is no longer four engines' margins averaged: it is Buro's
  own regression over chesso's positions at the paper's `t` = 1.0, and the
  procedure with its exclusions is written into S113 for S113 to run. **The
  futility margins were wrong in a way nobody had noticed**: the 2026-08-19
  pass read the wiki's "minor" and "rook" as 300 and 500 -- `see_value`'s
  scale -- for a margin compared against `evaluate()`, so `FUT_BASE` /
  `FUT_SLOPE` become **147 / 170** from `piece_value`, not 100 / 200. Every
  rewritten section now carries a units paragraph naming which of the two
  pawns, 94 or 100, its numbers are in. And the clause seeding an LMR
  re-sweep from three engines' curve coefficients republished on the wiki is
  **deleted** -- DEC-105's own PeSTO case -- leaving S085's own 52 / 182.
- **DEC-157 answers the step's three deferred questions, and one of them is a
  standing rule.** `NULL_MOVE_EVAL_CAP` seeds from the range midpoint 8 with
  the measured bound (P4) named beside it, because a step file may state an
  alternative but S180 measures nothing; the stamp lists F01's rows first and
  the inventory's second; and the two stale shipping values sitting inside the
  rewritten sections -- S114's `NULL_MOVE_BASE` "2 ships today" against the 3
  S085 shipped, S132's `TM_NODE_MIN_DEPTH` 5 against the compiled 2 -- were
  fixed here rather than handed to S184, whose file now says so.
- **S203 is done, 2026-09-08: the Zobrist keys are the project generator's, and
  the mating-PV fixture was rebuilt by a rule rather than by a match.**
  `init_zobrist` draws its 851 keys from `project_random_next` seeded with
  `CHESSO_PROJECT_SEED`; `magic_gen zobrist --seed 20260904` reports **`matches
  init_zobrist: yes`** where it reported `no`, quality clean (0 zero keys,
  851/851 distinct, 0 pair XORs equal to a key, 361675/361675 distinct pair
  XORs, minimum Hamming 14). The fast suite asserts all of it. **New baselines:**
  depth 9 **121530 / 801481 / 72924**, depth 12 **636677 / 3520847 / 494098**,
  `bench` **26851183**, **best moves unchanged** -- `specs.md` under INV-6 and
  the handover check below carry them. Perft verified, `test_perft` green, Debug
  self-play 8 games 0 assertions. Closes `2026-09-04_test_review-F08`.
- **The mining run produced nothing, and that is the finding, DEC-156.**
  `ROUNDS=1500 ./fastchess.sh`, 3000 games at 8+0.08, **1 h 18 m 10 s, 0 time
  forfeits** -- and **3** `Incomplete mating PV` warnings over 2 roots, **all
  from the old-keys reference, none from the candidate**. That is not evidence
  the redraw fixed the class: 3 lines landing all on one side has probability
  0.125 under an equal-rate null. What it says is that the class has become far
  rarer than when the set was built -- S147 read 10 lines over 4 games in 3000 --
  so a mining campaign for six fresh cases was not going to converge. The run's
  Elo was not read and the banner says why: fixed rounds, no bounds, not a
  verdict.
- **All six cases came back by re-sweeping their node budget, no match needed.**
  One rule, stated before it was applied and applied to every row: the cheapest
  budget at which the case reports at least its floor of mate lines with all of
  them complete. A `300000 -> 1000000`, C `1000000 -> 1500000`,
  D `1000000 -> 4000000`; B, E, F untouched. Floors re-derived, A 5 -> 6 and
  D 1 -> 3, **no live floor lowered**; F's stale 6 against 4 reported corrected
  to 2. `adocs/data/S203_case_sweep.sh` is the script, which is what DEC-142
  wants beside a golden. **The grid is a knife edge and both files now say so:**
  C reports 13 mate lines at 1500000 nodes and **0 at both 1000000 and
  2000000**, so re-run the sweep after anything that moves the tree, not only
  after a key change.
- **Two reproductions changed hands.** `D_mate_minus6_depth10` between 1200000
  and 3000000 nodes publishes `mate -6` at ply 35 depth 11 with a 10-of-12-ply
  PV, at a depth that also publishes a complete 12/12 -- DEC-122's class, and
  **S202** now carries it as a seconds-long reproduction where its only other
  route was a 3000-game match. In the same move, **S202's existing F
  reproduction is retired**: it printed 6 mate lines and 3 short for two months
  and prints 4 and 0 under the redrawn keys. The old numbers are kept beside the
  new ones, because they are quoted elsewhere. The fast suite costs about 8 s
  more, D's new budget being the price.
- **S179 is done, 2026-09-08: the 128 magic numbers are this project's own
  output and share 0 values with the tutorial set.** `tools/magic_gen` is the
  command and **20260904** the seed -- DEC-132's date, the owner's choice.
  It finds all 128 in **1.56 s** over 9270054 candidates, and the coincidence
  check reads **0 of 128** against the arrays it replaced -- an independent
  regex over both headers, 0 shared values and 0 at the same index -- then
  **128 of 128** from the tool rebuilt against the new one, which is the proof
  that what is committed is what the seed reproduces, not an argument that it
  is. Node-identical by construction and proved rather than asserted:
  `bench_movegen` verifies its perft counts before printing, `test_perft`
  green, `search_bench.py` gives 121512 / 800769 / 62907 and
  639228 / 3430710 / 367858 with `c3d5` / `e2a6` / `d7c8q`, and `gate.sh`
  **GATE-DONE 24880255 (no functional change)**. Interleaved `hyperfine`
  **1.01x** (5.267 +/- 0.016 s against 5.217 +/- 0.075 s), Debug self-play
  **0 assertions**. `magic_is_collision_free` lives in the engine so the
  predicate the generator accepts a candidate with is the one the suite runs
  over all 128, with a zero magic as the precondition proving it can fail.
- **The step was split and the reason is worth more than the split, DEC-154.**
  The Zobrist half was written, measured and then withdrawn: it turned
  `test_mate_carry` red, and not by introducing a defect. `adocs/data/S170_cases.tsv`
  is six eviction reproductions **mined from S147's 3000-game run**, and which
  entries evict which is decided by the keys -- so over four arbitrary seeds
  (20260904, 12345, 999983, 777777777) **every one** left 3, 3, 4 and 3 of the
  six cases reporting no mate at all. There is no seed to pick, and picking one
  would be fitting the seed to the test. Under 20260904 one case also published
  `mate 9` on a 5-ply PV, which is the class DEC-122 leaves short and visible
  and **S202** owns -- not a wrong score, with perft, `test_perft`, the
  hash-versus-recompute test and Debug self-play all green and the 851 keys
  distinct with no three- or four-subset XORing to zero. The measured figures
  are carried into **S203**'s file rather than lost: depth 9
  121530 / 801481 / 72924, depth 12 636677 / 3520847 / 494098, `bench`
  26851183, **best moves unchanged**. `2026-09-04_test_review-F08` goes to S203
  with the half that closes it. The fixture's key-set dependency is now named
  at both its sites, which is DEC-142's rule reaching a fixture that is not a
  number.
- **S198 is done, 2026-09-08: the harness seeds its openings, records nodes and
  clocks, and the workstation is calibrated.** `fastchess.sh` derives `-srand`
  from the run stamp, prints it in the banner and carries it in each PGN's
  `[Event]` header -- fastchess records it on no stream, in no log and in no
  header of its own -- with `SRAND=<n>` to replay a sequence and `ROUNDS=<n>`
  for the fixed-rounds mode nothing had, which drops `-sprt` and prints
  `bounds none -- fixed <n> rounds ... NOT a verdict`. `-pgnout` gains
  `nodes=true timeleft=true`. Three test cases, red first with 5 assertions
  against HEAD's script; eleven properties now.
- **DEC-143's calibration is taken and it stands.** 1000 games at `bbbf8f6`
  between **byte-identical binaries** (sha256 `b047f22d...`), 26 m 21 s,
  **0 time forfeits on either side**, pair score variance **0.2430 +/- 0.0154**
  against S105's after-run 0.2395 +/- 0.0152: **`z = +0.16`, inside the
  pre-registered band**, ratio 1.014, nothing re-run. **2277 games an hour**,
  and the difference from S105's 2322 is game length rather than machine speed
  -- seconds a ply 0.1831 → 0.1791 while plies a game went 98.0 → 102.1. One
  nElo is **0.698 logistic Elo** here, read off the run's own pair. The run is
  also the mode's own argument: with the truth zero by construction it printed
  `Elo: 7.99 +/- 15.03, LOS 85.16 %`, which is why a fixed-rounds run is never
  quoted as a verdict. Records `adocs/data/S198_*`; DEC-153 has the owner's
  three answers. **The next verdict may be taken.**
- **S189 is done, 2026-09-08: the engine has a node signature and a script that
  checks it.** `bench` searches eight fixed positions at `BENCH_DEPTH` 14 and
  prints **`24880255 nodes <nps> nps`** -- identical across three fresh
  processes and across the stdin and argv forms -- and `tools/gate.sh` runs the
  TESTS chain in both builds and then refuses a message whose `Bench:` line is
  not the binary's number. **DEC-140 binds from this commit on**: every commit
  touching `src/` carries `Bench: <n>` or `No functional change`. INV-6 stops
  being a procedure a person performs by eye; it is identical at depths 9 and
  12 and the interleaved timing is 1.00x, so nothing in the search moved.
  DEC-152 records the owner's four answers -- `KILLER_POS` stays in the set
  though it is illegal by piece count, depth 14 by the step's own rule, the
  OpenBench argv form ships now, and `gate.sh` gets a test.
- **The red-then-green could not be taken with a search mutant, and that is
  worth more than the demonstration was.** Three node-moving changes were tried
  -- `M01_nmp_in_check` from the test review's own mutant list, the two killer
  ordering ranks swapped, `MVV_KNIGHT` 300 → 310 -- and the fast suite refused
  all three before the signature was ever compared. The common cause is
  `test_mate_carry`: its anti-vacuity precondition, `mate_lines >=
  expected_mate_lines(name)`, is a search-tree-sensitive golden with no script
  that re-derives it, so any change that reorders the tree trips it. That is
  the test doing its job and it is also exactly the DEC-142 class. **Recorded
  for S192**, which owns naming and scripting the goldens. The demonstration
  used `BENCH_DEPTH` 14 → 13 instead, the case `MANUAL.md` names as a
  deliberate signature change: `GATE-FAILED: message says Bench: 24880255, the
  binary benches 13064004`, then `GATE-DONE 13064004` after the amend.
- **The S189 fast check found a second instance of the first bug, fixed at
  `HEAD`.** The `No functional change` parent walk had no `|| true` either, so
  an ancestry carrying no `Bench:` line anywhere -- a history from before
  DEC-140, which is precisely what `--build-parent` exists for -- died at
  `GATE-FAILED: exited 1` one line above the message telling the reader to use
  it. The one path that most needed to say what to do next was the one that
  lost it. `tests/test_gate_script.sh` gains case 10, observed red on the
  unfixed script and green after; cases 5 and 6 could never have caught it
  because their sandbox has carried a `Bench:` line since its root commit.
- **One bug found and fixed on the way, by the gate's own test.** Case 8 of
  `tests/test_gate_script.sh` -- a binary printing no signature line -- got the
  trap's generic `GATE-FAILED: exited 1` instead of the specific message
  written for it: `grep` exits 1 on no match, `set -o pipefail` fails the
  pipeline, and errexit killed the script one line above its own check.
  `bench_of` is now called `|| true` at both sites.
- **What it cost the suite.** 52 s over 27 tests → **61 s over 28**, all of it
  `test_uci_surface` going 0.03 s → 6.8 s because its new case searches the
  bench set twice at the shipping depth. **In `Debug` that one case is 157 s**,
  measured, which S197's Debug gate inherits. Kept at the shipping depth on
  purpose (DEC-152).
- **Carried forward, cheap to know:** `git worktree` does not bring the
  submodules, so a worktree cannot build the test targets without them.
  `fastchess.sh` and `gate.sh --build-parent` never meet this because both
  build only the `chesso` target. It cost one gate run to find.
- **S171 is done, 2026-09-07/08: the census ran on this machine and its
  residual is measured rather than closed (DEC-150).** The run DEC-128
  postponed here -- `REF=457e355 ./fastchess.sh --fast`, 3000 games at 8+0.08,
  **1 h 17 m 49 s, 0 forfeits**, `Elo +3.24 +/- 8.91` between INV-6 identical
  builds, out `/tmp/chesso_sprt_fast_20260907_224259`. The count it exists for:
  candidate **8** `Incomplete mating PV` lines from **1** search,
  `ref-457e355` **0** from **0**. The `accepts` said 0, and 0 turned out to be
  unreachable inside the step's own `excludes`, so the owner amended it to the
  measured pair.
- **The residual is neither a regression nor a wrong score, and no walk can
  close it.** `stockfish` gives `#+6` for the position at depth 20 and 30, so
  the `mate 6` is right; it is read off the table at depth 3 on 1224 nodes, too
  shallow to build the 11 plies it owes, and the line that iteration built
  continues in the table into a chain proving `mate 8`, so both distance-keyed
  lookups in `complete_mate_pv()` refuse and DEC-122 leaves the line short and
  visible. The same three lines reproduce byte for byte on `457e355`; the
  8-against-0 split is which side met the position. Building the missing line
  means searching, which that path may not do, and every other route is named
  in S171's `excludes`. **S202** owns the class and may alter play under its own
  SPRT. The standing figure in `adocs/specs.md`, `DEV_MANUAL.md` instrument 3
  and now `MANUAL.md` is **8 from 1 search in 3000 games** on this machine, with
  the MacBook's 5 kept attributed to the MacBook.
- **One hypothesis was implemented and reverted on the way, and it is worth the
  line.** That the walk stalled on a position whose single legal move carried an
  upper bound, which `certified_mate_move()` declines by construction. It was
  read off a walk of the match's line over a *differently warmed* table, the
  guard case stayed red under it, and it was backed out: `src/search.cpp` is
  untouched by this step and stands at `136b03f`. A dump of the table taken at a
  different moment than the failure is not evidence about the failure.
- **The instruments were probing a table no game produces, DEC-151.**
  `adocs/data/S170_replay.py` and `tools/mate_trace.cpp` both replayed **every**
  ply through one process; a game gives one engine only the positions it moves
  from. The census case was read back at stride 1 three times and came up clean
  each time, and reproduced at stride 2 on the first attempt -- at
  `nodes 1000000` from ply 56 or 64, *every* mate line the stride-2 replay sees
  is short. Both tools take a stride now, `adocs/data/S170_cases.tsv` has a
  `stride` column, and the guard test refuses a `start`/`stride` pair that steps
  over the last ply. Rows A to E stay stride 1 and still report 0 short lines.
  The class hidden behind stride 1 is unmeasured and S202 inherits it.
- **The POWER rule's literal test does not survive a wireless mouse.**
  `/sys/class/power_supply/` on this machine holds exactly one entry,
  `hidpp_battery_0`, type `Battery` -- a Logitech peripheral. S171's guide said
  "no `Battery` line" satisfies the rule; on a mains-only desktop that reads as
  a refusal. Taken as satisfied and recorded in the stamp. Whoever writes the
  next power check should test for a supply whose `scope` is `System`, not for
  the absence of the word.

- **Back on the workstation, 2026-09-07, and its gate was red on arrival --
  both halves fixed or decided, nothing in the engine changed.** No code moved
  between the MacBook and here; a machine changed and two compiler-and-toolchain
  differences came with it, which is what S167 was in the other direction.
  **(1)** `tools/mate_trace.cpp`'s usage comment wrapped with a trailing
  backslash, which g++ 13.3 -- the reference compiler here, DEC-049 -- calls
  `-Wcomment` and `-Werror` makes fatal; clang does not warn, so it survived
  S171 (`136b03f`) and stopped `build` and `build-tune` at the same object.
  Fixed at `d54f690`, comment only. **(2)** clang-format 23 (DEC-110's pin) is
  not installable from what this machine carries -- the apt line is
  `llvm-toolchain-noble-22` -- so the script resolved Ubuntu's 18.1.3, refused
  it and exited 1, and `test_clang_format_script` failed six assertions in both
  builds: the failure DEC-110's Consequences predicted for this machine, word
  for word. **The owner decided the override, DEC-146**: the pin stays 23,
  this machine exports `CLANG_FORMAT_MAJOR=22`, and `.moltke.local.md` carries
  it. The two majors format this tree identically -- measured, the check prints
  nothing and exits 0. **After both: fast suite 27/27 in both builds, gate
  exit 0.** Also re-checked on arrival: INV-6 reproduces to the node
  (121512 / 800769 / 62907 at depth 9, 639228 / 3430710 / 367858 at depth 12,
  `c3d5` / `e2a6` / `d7c8q`), `src/openings.bin` still digests to
  `77f47f1b...db06b58`, the Open list's 74 entries match `plan_todo/`'s 74
  files id for id with none duplicated and none also in `plan_done/` (72 after
  the day's three completions, re-checked), and
  `plan_prose_check.py --touches` flags 0. **Before the first timed run the
  machine still needs**: the `performance` governor (it boots `powersave`),
  `kernel.perf_event_paranoid=1` for samply, and an idle desktop -- gthumb and
  firefox were between them holding about two cores while this was written.
  **Two facts the workstation settles**: `fastchess` here is
  `alpha 1.8.1 20260720-daa3ea2`, the version S105, S087 and S145 measured
  with, which answers S198's open question about it; and `.ref-builds/` holds
  21 August worktrees at 2.2 GB on a root filesystem at 95 %, prunable with
  `git worktree remove` at the cost of one rebuild if a ref is re-used.
- Last done: **S189, 2026-09-08 -- the `bench` node signature and
  `tools/gate.sh`.** `24880255` at `BENCH_DEPTH` 14 over eight fixed positions;
  the gate runs the TESTS chain in both builds and then checks the binary
  against the commit message. DEC-140 binds from its completing commit on and
  DEC-152 records the four owner answers. Before it: **S200, 2026-09-07 --
  `movetext_to_san()` reads the whitespace
  form of a move number indication, and a cut-short message quotes the token as
  the PGN wrote it.** PGN 8.2.2.1 lets white space sit between the digits and
  the dots, so `1 . e4`, `1 .e4` and `1. ... e5` hand the splitter a token of
  dots with no digits in front of it; S178's block required digits, so the dots
  reached `algebraic_to_move()` whole and the build was refused at ply 0.
  Dropping that one clause -- `dot > 0` -- reads all three forms. The message
  half is the reader's problem and not the engine's: `cannot parse 'Qxf7'`
  matches every line of a PGN that plays the move somewhere, where `2.Qxf7`
  matches the one line that holds it, so `movetext_to_san()` now returns
  `san_token_t {move, as_written}` and `build()` reads the second field for both
  `report_cut_short()` messages. **Red first at `ffe207f`**: two new fixture
  properties gave 13 `FAIL:` lines -- `cannot parse '.' at ply 0`, `'.e4' at ply
  0`, `'...' at ply 1`, and `'Qxf7' at ply 2` failing both the as-written
  assertion and its bare-form negation. Green after. **Numbered 12 and 13, not
  the 9 and 10 the step file named**: S173 landed between the writing and the
  doing and took 9, 10 and 11. **The remainder is still not re-classified**, as
  the step's traps require: `.2` cuts short as `.2` and `1.2` as `1.2`, while a
  bare `2` is still dropped -- silently dropping broken input is the fault class
  S174 closed. **python-chess agrees**: `adocs/data/S175_book_conformance.py`
  reads all three new fixtures natively at `missing 0 extra 0
  weight_mismatch 0`. **Nothing in the engine moved**: no `src/` file in the
  diff, so no `Bench:` line is owed (DEC-140) and `tools/gate.sh` does not exist
  yet; INV-6 reproduces to the node (121512 / 800769 / 62907 and
  639228 / 3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`), and `src/openings.bin`
  rebuilt from `books/8moves_v3.pgn` is byte-identical at `77f47f1b...db06b58`
  -- 34700 games, 0 cut short, 172232 entries. Gate 27/27 in both builds, format
  clean under `CLANG_FORMAT_MAJOR=22`. **Documents**: `DEV_MANUAL.md`'s
  move-number paragraph says the whitespace form is read and what happens to the
  remainder, its cut-short sentence says the token is quoted as written, and
  `adocs/specs.md`'s book clause is widened from S178's wording -- the
  coordinator's own call, this step held the machine; `MANUAL.md` checked, no
  UCI surface touched. **`1 .e4` builds correctly only because S201 fixed the
  parser first**; the other import-format leniencies S178 listed are untouched.
- Previously: **S173, 2026-09-07 -- `make_book build` replaces a book
  atomically.** The bytes go to `<out>.tmp` beside the destination and are
  renamed over `--out` only after the write, `fsync` and `close` all succeed;
  every failure the process lives through unlinks the temporary and exits 1
  with `book not written` and the `errno` string. Before, `std::ofstream
  output(options.out, ...)` truncated the previous book the instant it opened,
  so a write that then failed destroyed it -- and because any prefix of a
  sorted 16-byte-entry book is a valid book, what was left passed `dump` and
  the engine's loader alike. **Two reds on this machine, both new here.** The
  fixture's three new properties (9, 10, 11) gave two `FAIL:` lines --
  destination 0 bytes, refusal message absent -- and by hand `ulimit -f 0`
  exited **153**, SIGXFSZ killing the tool before any guard could run. The
  partial-write form, which S146 had only on macOS, reproduces here too:
  pre-change under `ulimit -f 200` the shipped PGN left **204800 bytes** at
  `--out` and `dump` read it (`heaviest 1: 0844931a6ef4b9a0 g1f3 weight
  2527`). **Green after**: fixture `ok`; by hand exit 1, `wrote 204800 of
  2755712 bytes to '<out>.tmp': File too large -- book not written`, the
  destination still `previous book` at 13 bytes, no temporary. So `SIGXFSZ` is
  now ignored in `main` -- the limit arrives as `EFBIG` from `write` instead of
  as a kill -- and `tools/make_book.cpp` carries the first POSIX headers in the
  tree (`<fcntl.h>`, `<unistd.h>`: nothing standard gives `fsync`).
  **Success path unchanged, measured**: the shipped book rebuilt through the
  new write path is 2755712 bytes, `cmp`-identical to `src/openings.bin` at
  `77f47f1b...db06b58`, 1.15 s. **No `src/` file in the diff**, so no `Bench:`
  line is owed and INV-6 is not owed -- `make_book` is its own executable.
  Gate 27/27 in both builds, format clean under `CLANG_FORMAT_MAJOR=22`,
  `plan_prose_check.py` exit 0 with 0 touches flagged. **The owner chose the
  fixed temporary name over `mkstemp`, DEC-149**: `<out>.tmp`, so at most one
  is ever left behind -- by a kill the process cannot survive -- and the next
  build to the same `--out` truncates it, which lets the `accepts` hold as
  written; the cost is that two concurrent builds to one destination would
  clobber each other, which nothing in the tree does. That choice also answers
  the step's file-mode question by itself: `O_WRONLY|O_CREAT|O_TRUNC` at 0666
  is exactly what the replaced `std::ofstream` asked for, and `rename(2)`
  carries the mode onto the destination. `DEV_MANUAL.md`'s "`loadable` does not
  mean complete" paragraph now states the guarantee and carries the
  `ulimit -f` reproduction as the root-free form of S146's ram disk;
  `MANUAL.md` and `adocs/specs.md` checked, neither moves.
- Before that: **S178, 2026-09-07 -- `movetext_to_san()` reads a move number
  indication glued to the move it introduces.** `1.e4`, `2...Nc6`, `4.O-O`: PGN
  import format, 8.2.2.1. Before, the whole token reached `algebraic_to_move()`,
  got 0 from S174's fail-closed parser, and the build was refused at ply 0 --
  honest and useless for the half of the world's PGN files written that way.
  The splitter now drops a bare number first, then, for digits followed by at
  least one dot, either drops the indication alone or erases it and keeps the
  move. **Red first at `3200a1a`**: two new properties in
  `tests/test_make_book_tools.sh` gave 8 `FAIL:` lines, all on `cannot parse
  '1.e4' at ply 0`; the export-form control was green from the start, so the
  expected entry counts came from its own output. Green after.
  **python-chess agrees**: `adocs/data/S175_book_conformance.py` re-derives both
  glued fixtures at `missing 0 extra 0 weight_mismatch 0`. **Nothing in the
  engine moved**: no `src/` file in the diff, so no `Bench:` line is owed
  (DEC-140), INV-6 reproduces to the node (121512 / 800769 / 62907 and
  639228 / 3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`), and `src/openings.bin`
  rebuilt through the fixed tool is byte-identical at `77f47f1b...db06b58`
  because `books/8moves_v3.pgn` has no glued token. Gate 27/27 in both builds,
  format clean under `CLANG_FORMAT_MAJOR=22`. `DEV_MANUAL.md` and
  `adocs/specs.md` say so; `MANUAL.md` checked, no UCI surface touched.
  **Two owner questions are open and neither is a bug**: whether `1 . e4`
  (whitespace between the digits and the dots, also legal by 8.2.2.1) is worth
  the same few lines or its own step -- today the leading `.` reaches the parser
  and cuts the game short by name -- and whether the cut-short message should
  carry the token as written (`1.e4`) rather than the move part (`e4`).
- **S178's two deferred questions answered by the owner, 2026-09-07, DEC-147;
  both become S200, done the same day.** The whitespace form of a move number
  indication (`1 . e4`, `1 .e4`, `1. ... e5`, which PGN 8.2.2.1 allows) is
  folded into a follow-up rather than left refused, and the cut-short message
  will quote the token as the PGN wrote it -- the owner's condition on the
  second was "if not too complex", and it was sized before the step was
  written: `movetext_to_san()` returns a two-field struct, its one caller in
  the tree reads the second field for the message, about fifteen lines. One
  step and not two, because both edits are in that function and the loop that
  reads it, both are covered by the same fixture file, and neither alters play.
  It sits after S173 to group the two `make_book` tool steps; move it if the
  order should differ.
- **S201, 2026-09-07: the parser defect found while sizing S200, called a bug
  by the owner and fixed before anything else started (BUGS rule); DEC-148.**
  `algebraic_to_move()` reads the piece letter at position 0 only
  (`src/bitboard.cpp`, the `std::isupper(notation[pos])` test): a leading
  character that is not an uppercase piece letter falls to the pawn branch, and
  the disambiguation loop then swallows the real piece letter. **`.Nf3` comes
  back as `f2f3`, a pawn push, where the token means the knight move `g1f3`.**
  python-chess refuses `.Nf3`, `.e4` and `..e4` outright (checked). It is
  reachable: `1 .Nf3` is legal PGN import format, so `make_book` and
  `pgn_to_positions` build silently from a wrong board -- the shape S174 closed
  for a token the parser *cannot* read, still open for one it reads as
  something else. `Zf3` is refused and is what the suite tests; no test covers
  a non-uppercase leading character. **Bounded**: `algebraic_to_move()` is
  called only by the two tools and the tests, never on the engine's UCI move
  path, which is long algebraic; `books/8moves_v3.pgn` has no such token, so
  `src/openings.bin` is unaffected. Interior junk is lenient but *correct* --
  `N.f3` gives `g1f3` -- so only the leading character produces a wrong move.
  **The fix, narrow by the owner's choice**: after the suffix strip and the two
  castling returns, an empty token or one whose first character is not in
  `[a-hKQRBN]` returns 0. The wider gate -- every character in
  `[a-h1-8KQRBNx=]` -- was rejected: the wrong-move class is entirely a
  leading-character effect, and the interior forms it would also close give the
  correct move today. **Red first**: five `CHECK`s, `.Nf3` coming back as
  `2933`. **Verified over the corpus, not argued**: the book rebuilt through
  the changed parser is byte-identical at `77f47f1b...db06b58` -- 34700 games,
  172232 entries, 0 cut short -- so roughly 278000 real SAN tokens parse as
  before. INV-6 identical, gate 27/27 in both builds. No `Bench:` line is owed:
  DEC-140 bound from S189's completing commit and S189 was open. (S189 has
  since landed, 2026-09-08; every `src/` commit after it carries the line.)
  **One thing found in passing and not fixed, for S193**: the new
  `REQUIRE(algebraic_to_move("Nf3", &game) != 0)` is the first positive
  assertion in that TEST_CASE. Every existing subcase asserts `== 0`, so with
  `game_tables()` uninitialised -- which is what running the binary with a
  `-tc=` filter and no earlier case does -- they all pass vacuously. Under
  `ctest` the case is sound; the vacuity is the class S193 was written for.
- In progress: **nothing.** `adocs/plan_current/` is empty. **The enrichment
  pass of DEC-145 is stopped at the owner's word after twenty of the then 74
  files -- S178, since done, through S151; the next file is S181, today Open
  entry 19.** Resume by handing `adocs/data/2026-09-05_enrichment_brief.md` and
  one step path to one agent per file, in Open order, one commit per file; what
  is left is named by
  `grep -L 'Implementation guide (2026-09-05)' adocs/plan_todo/*.md`. The
  session's report, with twenty findings and the owner questions from every
  file, is `adocs/data/2026-09-05_enrichment_pass.md`.
- **Reordered 2026-09-05 for the workstation, DEC-144.** The owner, leaving for
  the day, asked that the plan be re-sorted for the goal under the rules with
  every step assumed to run on the Linux workstation, and that decisions be
  taken. The DEC-112 machine-scope lane is gone from `plan.md` (its text is in
  the file's history at `66cbc54`). The Open list now reads: two tool bugs
  (S178, S173); the census (S171, DEC-128's first run there); S189 and S179 as
  agent-only work while it plays; S198's harness flags and the calibrating A/A
  (DEC-143) before any verdict; then an instrument lane of the sixteen
  document and test steps interleaved with S148, S159 and S151 so the machine
  has a job while documents are written; then blocks 1 to 4 exactly as the
  2026-08-19 review ordered them and DEC-133 corrected them, S152 closing the
  main order, the reserve and the parked network last. **One question is
  deferred to the owner:** S151's re-test of S085's vector at a control at
  least four times 8+0.08 prices a `{-5, 0}` pair near 72 hours worst case by
  DEC-143's formula; whether that pair, a cheaper pair or a fixed-rounds
  reading is wanted decides when it runs -- it sits at Open entry 17 until
  then. Nothing in the engine changed, no run was started, the gate was green
  at `66cbc54` before the edit (27/27 in both builds) and the four prose
  checks pass after it.
- **The enrichment pass, DEC-145.** Also on the owner's instruction: every
  pending step file gets an `## Implementation guide (2026-09-05)` section for
  the agent that will implement it -- the technique as published, chesso's
  form, the symbols it touches at HEAD, seeds in DEC-105 form only, the tests
  DEC-141/142 require, the pair priced per DEC-143, the completion checklist,
  the repository's own traps, every source with its URL or `unverified`, and
  deferred owner questions. Engine-originated seeds in the seven DEC-134 files
  are replaced in place, so S180 becomes a verification; block 3's files are
  brought to S186's accepts, so S186 becomes one too. What the pass has not
  reached: `grep -L 'Implementation guide (2026-09-05)' adocs/plan_todo/*.md`.
  `adocs/data/2026-09-05_enrichment_pass.md` lists what each agent flagged.
  **Findings the owner should read before the workstation runs anything**, none
  fixed today (BUGS rule: the owner says which are bugs): `make_book` is killed
  by `SIGXFSZ` before S146's stream guard and leaves a truncated `loadable`
  book; the `SANITIZER` CMake block lets UBSan recover so a UB report exits 0;
  `adocs/data/S105_pairs.py` reads a side name `fastchess.sh` never prints, so
  S198's A/A read as its accepts says would inflate the variance silently, and
  `fastchess.sh` has no fixed-rounds mode without `-sprt`, so that A/A cannot be
  launched as written; `command_ucinewgame` sets the stop flag and only `go`
  and `test` clear it; `set_position` never clears `proven_mate_line`. Stale
  clauses in pending files: S190's depth-3 walk is about seven times over its
  budget, S159's second candidate is behaviour-neutral and cannot be SPRT'd,
  S148's S145 table predates S154's re-take, S184 assumes `--params` is outside
  the fast suite, S180's inventory found fifteen seeds the review missed.
- **Plan review 2026-09-04 (`adocs/audit/2026-09-04_plan_review.md`), digested
  the same day: no high, 5 medium, 5 low; every finding has a home and every
  home is a document step.** A cold reviewer read the plan against the
  published record, the code at HEAD and the project's rules; an independent
  pass fetched 33 of the plan's figures at source and is tracked as
  `adocs/data/2026-09-04_plan_review_literature_check.md` (DEC-137). Fast suite
  27/27 in both builds, both node-count baselines reproduced, 45 of 46 prior
  plan-review findings closed on re-measurement. The owner decided item by
  item: **F01** seven search steps seed constants from other engines'
  commit-message prose -- stick to DEC-105, reseed, **S180** (DEC-134); **F02**
  S099's "+11.4 at ~2850" was measured at Lynx 3226-3293, so S099 goes to the
  **reserve head** as the family's probe, S110/S111 gated on it, Lynx figures
  re-banded by **S181** (DEC-133); **F03** the cost line's 45-75 minutes per
  verdict against a ledger mean of 4 h 40 m -- **S182**; **F04** the Elo
  arithmetic with no recorded inputs -- **S183** (DEC-136); **F05, F08, F09,
  F10** stale parameter values (S115 would revert a verified SPSA axis), S042's
  narrow touches, five stale sentences, two missing `done:` fields --
  **S184**; **F06** block 3's unsourced ledgers, both located by the literature
  pass -- **S185**, and the DEC-097 enrichment becomes **S186** before block 3
  (DEC-137); **F07** 59 drifted citations three days after S169 -- citations
  into code become symbols, **S187** (DEC-135). From the literature pass, not
  the report: Ethereal removed only its *pre-move-loop* check extension, so
  the in-loop form is reopened as **S188** after S097, S096's id staying
  retired; S133's +65/+88 are release-bundle deltas, kept on adoption breadth
  (both DEC-133). Techniques the surveyed engines carry and the plan gives no
  step are recorded in DEC-138. Nine steps, 63 Open entries; `plan.md` carries
  a section on the review and inline corrections at the DEC-087 rulings; the
  report's findings read `planned`. Nothing in the engine changed, no run was
  started.
- **Test review 2026-09-04 (`adocs/audit/2026-09-04_test_review.md`), digested
  2026-09-05: no high, 3 medium, 7 low; eleven steps, four rule amendments, one
  finding accepted (DEC-139 to DEC-143).** The owner asked for research on how
  a chess engine's tests and measurements are done and an assessment of ours.
  The suite was **measured**, not read: 33 hand-written engine bugs injected
  one at a time into a scratch worktree of `5cffb70`, **31 of 32
  non-equivalent caught**, the survivor a fifty-move draw claimed at 101
  instead of 100 (F04); 98.9 % line and 92 % branch coverage of
  `src/search.cpp`, 100 % of the evaluation, the UCI book path never executed
  (F06); 35 full runs of the fast label, 0 unexplained failures, 41 to 64 s.
  The mediums: INV-2 and INV-4 live in `assert`s both gated builds compile
  out (F01, **S190**); the null-move and reduction guards have no direct test
  and five guard-removal bugs were caught by `test_mate_carry`'s count floor
  and nothing else (F02, **S191**, before S109); the suite's sensitivity
  comes largely from goldens every legitimate search change will redden (F03,
  **S192**, DEC-142). The lows: F04/F05/F09 **S193**; F06 **S194**; F07 the
  bench signature **S189** with DEC-140's `Bench:` commit line and
  `tools/gate.sh`; F08 **S195**, the Zobrist keys folded into **S179**;
  **F10 accepted** -- no harness calibration on this MacBook, the workstation
  is back soon and its A/A is the first run taken there (DEC-143, doubling
  as **S198**'s). From the survey (`adocs/testing_strategy.md`, four
  source-verified literature passes): **S196** the mutation driver as a tool
  and a mutant per new search rule, **S197** `tools/gate_extra.sh` with the
  coverage recipe, **S199** a fixed-rounds drift match against a pinned
  reference after each block; a `NodesTime` clock deferred to S127. The
  bounds now have a price before a run: a `{0,5}` pair is 41861 expected
  games at the midpoint, **17.9 h at the 2337 games an hour the seven
  ledger runs average here** (2328 to 2346), carried into S182 and DEC-143.
  The plan's "~60 % sticks" self-play ratio could not be verified at any
  source; S183 carries it as unverified. Evidence under
  `adocs/data/2026-09-04_test_review/`. 74 Open entries. Nothing in the
  engine changed, no run was started, the machine was on battery throughout.
- **Audit re-run 2026-09-04 (`adocs/audit/2026-09-04_adversarial.md`), the
  closing run for the 2026-09-03 batch: no high, no medium, three low.** All
  four 2026-09-03 findings are **closed on re-measurement from their own
  reproductions**, not from the stamps -- the Polyglot key (registered test
  green, python-chess re-derivation 0/0/0, `OwnBook` answers `d2f3` on one of the
  seven positions), the SAN parser (annotated PGN builds, `Qxf7` refused with
  exit 1 and no file), the scripts (`rating.sh --bracket` prints
  `RATING-RUN-FAILED: ordo not on PATH`) and `position fen` (short forms load, a
  bad FEN after `startpos moves e2e4` keeps the e2e4 board). Rules: nothing.
  Elo behind the literature: nothing outside the plan. Prior findings recounted:
  37 code-audit findings, 30 closed, 4 accepted, 3 planned (S039, S042, S151).
  **The three lows are `open` and wait on the owner** (DEC-035 says a run with
  no high and no medium is where the loop stops, and the lows are discharged by
  one decision or become steps): **F01** `go infinite` prints `bestmove` unasked
  on a root whose tree collapses (KNvK, stalemate, checkmate) because the
  infinite search is the depth loop to `MAX_DEPTH` 126 -- a protocol break, red
  test written and unregistered at `tests/test_audit_go_infinite.cpp`, 3 of 6
  red; **F02** a bad token in `position ... moves` is skipped and the rest
  applied, silently in Release, documented in `MANUAL.md` as intended -- the
  S176 rule applied to the FEN half of the command and not to the moves half;
  **F03** the aborted-iteration best-move logic assumes the root's table entry
  survives the iteration, which the replacement rule does not guarantee --
  mechanism shown, not reproduced, about 6e-5 per iteration by estimate.
- Last done: **S177, 2026-09-04 -- `rating.sh` and `build_release.sh` run on
  macOS as `fastchess.sh` does, and `rating.sh` prints its terminal marker on
  every exit.** On this machine `./rating.sh --bracket` died at
  `all_cores="$(nproc)"` with exit 127 and **no marker**, 90 lines before its
  trap was armed -- the WATCHERS-rule hole S167 closed for `fastchess.sh`. Now
  the marker trap and `fail()` come first, with a `marked` flag rather than `$?`
  because bash 3.2's EXIT trap reads `$? == 0` on a `set -u` abort; cores are
  counted through `sysctl -n hw.physicalcpu` with `nproc` as the fallback and a
  named refusal when neither exists; the GNU `timeout` `identify()` needs is
  resolved once as `timeout` or `gtimeout` and refused by name before any engine
  is probed; and the `command -v x || fail` checks are `if`s, since bash 3.2
  exits before `fail` runs in the old form. `build_release.sh` takes the core
  count the same way (`hw.logicalcpu` for build jobs). **Red first**:
  `tests/test_rating_script.sh`, a sandbox whose PATH holds only the utilities
  it links in -- no `nproc`, `timeout`, `gtimeout`, `sysctl` or `ordo` -- failed
  all five of its properties on the old scripts and passes after; on this
  machine the real script now answers `RATING-RUN-FAILED: ordo not on PATH`,
  exit 1, one marker, snapshot cleaned. Fast suite 27/27 in both builds.
  `DEV_MANUAL.md` (concurrency rows, rating section) and `TOOLCHAIN.md` (new
  "GNU coreutils on macOS" section) say so; `.moltke.local.md` now lists which
  scripts run here. No engine binary changed.
- Before S177: **S176, 2026-09-04 -- `position fen` takes four to six fields, and
  a FEN that does not load changes nothing.** Two defects in `command_position`.
  Fewer than six fields was dropped silently with the board left wherever it
  was, and a four-field FEN followed by `moves` read `moves` as its fifth
  field, failed to load, and applied nothing. A failed load then reloaded
  `initial_position` -- the last *FEN*, not the last *position* -- so the moves
  applied since were gone: after `startpos moves e2e4` one malformed FEN put the
  engine on the start position and applied the moves that followed to it. Now
  the clocks default to `0 1`, reading stops at `moves`, and fewer than four
  fields or a FEN `load_FEN()` rejects is refused with one `info string` line
  (`refused [position fen] <fen>, fewer than four fields` / `, does not load`)
  that ends the whole command; `set_position()` saves the whole `game_t` before
  the load and restores it on failure, about 100 KB and not on a search path.
  **Red first**: three new `test_engine` cases, 4 assertions failed on the
  unfixed tree, green after; the golden surface gained the two refusal shapes
  only after `MANUAL.md` and `specs.md` described them (SURFACE). INV-6
  identical (121512 / 800769 / 62907; 639228 / 3430710 / 367858, same best
  moves). Fast suite 26/26 in both builds. `DEV_MANUAL.md` needed nothing.
- Before S176: **S175, 2026-09-04 -- the Polyglot key follows the format on an
  edge-file en-passant square, and the shipped book is rebuilt to it.**
  `get_key()` looked for the capturing pawn at `en_passant + 7/+9` (White) and
  `-9/-7` (Black); with a8 at index 0 those wrap round the board edge on the a-
  and h-files, so `h6 + 9` read a4 and `a3 - 9` read h5, and a same-side pawn
  there switched the en-passant component on against the format. Now the two
  squares are taken by file with bounds, on the rank the mover's pawns capture
  from. **Red first through ctest**: the audit's `test_audit_polyglot_key`,
  registered, failed 7 of 10 on the unfixed tree and passes 10 of 10 after; the
  nine format example keys in `test_openings` still pass. **The book is
  rebuilt**: `src/openings.bin` is `make_book build books/8moves_v3.pgn` again
  through the gated tool, 0 games cut short, the same 2755712 bytes, 172232
  entries over 129613 positions, **sha256
  `77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58`**, swapped
  into `specs.md`, `MANUAL.md`, `DEV_MANUAL.md`, `src/openings_embedded.S` and
  S173's accepts. **Checked from outside, which the digest never could**:
  `adocs/data/S175_book_conformance.py` re-derives every entry from the PGN with
  python-chess's independent key and reports `missing 0 extra 0
  weight_mismatch 0` against the new file, where the 2026-09-03 file gave
  `missing 7 extra 7` on exactly the audit's seven positions. INV-6 identical
  (121512 / 800769 / 62907; 639228 / 3430710 / 367858; same best moves) -- the
  default configuration never probes the book. End to end: with `OwnBook` on,
  the position `rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w
  KQkq h6 0 8` -- one of the seven -- answers `bestmove d2f3` from the book with
  no `info` line. Fast suite 26/26 in both builds.
- Before S175: **S174, 2026-09-04 -- the SAN parser fails closed and `make_book`
  gates on it.** `algebraic_to_move()`'s three failure paths were `assert(false)`
  with no return, so in Release a token it could not read became a fabricated
  move that `make_move()` applied and the board was rewritten, not left illegal.
  Now each returns 0 in every build, the destination square is range-checked
  before `str_to_index()` (which asserts, and otherwise wraps), and PGN suffix
  annotations (`!`, `?` and their pairs) are stripped in the same loop as
  `+`/`#` -- in the parser, not in `make_book`'s tokenizer as the step first
  said, because `pgn_to_positions` needs it too. `make_book build` refuses to
  write when any game was cut short, names the game and the token on stderr,
  and `--allow-cut-short` is the deliberate form of the old report; a trailing
  flag with no value is an error now instead of silently skipped. **Red
  observed first**: 23 assertions over 2 new `test_chesso` cases and 9 of the
  fixture test's checks failed on the unfixed tree, green after; fast suite
  25/25 in both builds. **INV-6 identical**: 121512 / 800769 / 62907 and
  639228 / 3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`. The shipped book is
  untouched -- digest `3b89a4ad...15b873dd` -- and rebuilding it through the
  gated tool is byte-identical with 0 games cut short, so S146's `0 cut short`
  is a gate's word now. `DEV_MANUAL.md` and `adocs/specs.md` say so;
  `MANUAL.md` needed nothing, no UCI surface moved.
- **Discovered while doing S174, filed as S178, not folded in.** Run on the PGN
  import form (`1.e4 e5 2.Nf3`, no space), the fixed tools refuse at ply 0 and
  name the token -- honest, where before S174 the parser fabricated a move from
  `1.e4` and built from a rewritten board. `movetext_to_san()` should split the
  glued move number; `books/8moves_v3.pgn` has no such token (grep 0), so the
  shipped book is unaffected and the step sits after the audit batch.
- **Audit 2026-09-03, digested 2026-09-04
  (`adocs/audit/2026-09-03_adversarial.md`).** Adversarial, whole engine, cold
  context. **No high, 2 medium, 2 low**; fast suite 24/24 green and both
  `search_bench` baselines reproduce to the node; the project's own rules turned
  up nothing; every technique found behind the literature already has a step
  (S114, S098, S109, S091, S095, S097, S099, S131, S112, S022, S119, S039, S121,
  S122). Both mediums are in the code S172 and S146 shipped and were found by
  re-deriving the shipped book independently with python-chess, not by reading
  the stamps. **F01** `get_key()` wraps round the board edge for an en-passant
  square on the a/h file -- 7 of 172232 shipped entries carry the wrong key, and
  a third-party book is silently abandoned at any such position. **F02**
  `algebraic_to_move()` fabricates a move instead of returning 0 in Release, so
  `make_book` and `pgn_to_positions` (the DEC-023 board tool) run on silently
  on a rewritten board -- `1. e4!? e5` builds a `loadable` book with move `a8a7`
  from the start position, exit 0. The shipped book is nonetheless verified
  correct move for move and weight for weight; only the 7 keys differ. **F03**
  `rating.sh` and `build_release.sh` die on `nproc` before any marker is armed.
  **F04** `position fen` drops a 4/5-field FEN silently, and a bad FEN resets to
  the last FEN without its moves. **All four are steps, S174 to S177, at the
  head of the Open list** (BUGS rule); none owes a match. Of 33 prior code-audit
  findings, 27 are closed on re-measurement, 3 accepted, 3 planned in pending
  steps; two earlier reports still read `planned`/`open` on items now closed
  (2026-08-22 F01, F03), which a re-run records. The unregistered red test
  `tests/test_audit_polyglot_key.cpp` is committed with the report and S175
  registers it. `2026-08-13_plan_review.2-F06` had no trace in S023, whose
  `accepts` already carries its resolution; `closes:` is set now.
- **The magic-number question is answered: DEC-132, 2026-09-04.** The 128
  constants in `src/bb_tables.hpp` were generated in-repo (`a5dbe68`) with the
  tutorial's xorshift seed and coincide with that series' published set. The
  owner keeps them for now -- own generator output, inherited under DEC-013,
  origin in git -- and **S179** recovers or rewrites the generator, commits it,
  and regenerates both arrays under a project seed, proved unchanged on
  `bench_movegen`'s perft verification, `test_perft` and `search_bench` node
  counts. The table cites DEC-132 until then. S179 sits second in the Open
  list, after S178.
- Before S174: **S146, 2026-09-03 -- the book the engine ships with is built by
  this project, from a source it can account for.** The owner took the second of
  DEC-126's three standing options: the unaccounted blob is **deleted**, and
  `src/openings.bin` is now `build/tools/make_book build books/8moves_v3.pgn
  --out src/openings.bin` at the tool's defaults -- **2755712 bytes, 172232
  entries over 129613 positions, sha256 `3b89a4ad9146e266ae9296778067aaedcb7f57
  f3cf0ff2086b9ae6df15b873dd`**. DEC-131 is the ruling. (That digest is history
  since S175 rebuilt the file to the format's keys: `77f47f1b...db06b58`.)
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
- **S171's postponement, kept for the record.** It was postponed here from the
  MacBook by DEC-128 after two attempts died on that machine -- `pmset -g ac`
  said `No adapter attached`, which the POWER rule forbids (DEC-109), and on
  mains `fastchess.sh`'s own load guard reported `about 387% of a core is
  already busy` with Spotlight indexing PDFs. The fix itself was committed and
  green there at `136b03f`: `certified_mate_move()` completes a mate line across
  a table slot the walk has lost, `tests/test_mate_carry.cpp` guards it, INV-6
  discharged. It keeps that evidence -- the five stride-1 cases still report 0
  short lines -- and what the census found is a different cause it never
  claimed. The run itself is the entry at the top of this file.
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
- Last done: **S148, 2026-09-09 -- the reverse futility ceiling re-decided by
  SPRT and kept at 15.** One integer measured and reverted; documents, one
  `src/` comment and one test comment carry the price. `No functional change`.
  (Older `Last done:` lines sit below this one; this is the live pointer.)
- Last done: **S184, 2026-09-08 -- the pending documents at HEAD values and
  `--params` extended over them.** Documents and one checker change, no `src/`.
  (Two earlier `Last done:` lines sit above this one, S189's and S177's, from
  earlier sessions: they are that log's chronology and this is the live
  pointer. Nothing here reconciles them -- a flat list carrying three of the
  same field is a hygiene finding and not S184's scope.)
- Next: **S187**, now Open entry 1 -- citations from pending step files into
  code name a symbol and no line, the checker verifies the symbol, and the 549
  existing ones are converted (F07, DEC-135). Its input is S184's 52 DRIFT
  flags over the pending set. It is agent-only work and owns no run, so the
  machine is free for whichever verdict is taken beside it; **S159** is the
  next entry that wants the machine, and S151's pair is still the owner
  question parked below.

- Blocked: **nothing.**
- Watching: **nothing. No run is armed.** S148's SPRT finished at 01:57 on
  2026-09-09 and its watcher exited on `SPRT-RUN-DONE`, acknowledged and read
  in the same turn; the verdict is recorded above and in DEC-158. Its PGN,
  73 MB at `/tmp/chesso_sprt_nonreg_20260908_193719/games.pgn`, is **not**
  committed and `/tmp` will take it; the log is, and carries every game's
  result. Before it, S198's A/A finished at 02:39 on 2026-09-08.

- Parked:
  - ~~**Calibrate the harness on the workstation.**~~ **Taken 2026-09-08 as
    S198's A/A and retired**: 1000 fixed rounds, 0 forfeits, pair variance
    0.2430 +/- 0.0154 inside S105's band at `z = +0.16`, 2277 games an hour.
    The next machine change owes the next one under DEC-143.
  - **Owner question from the 2026-09-05 reorder: S151's pair.** Its accepts
    asks for S085's vector re-tested against `3488506` at a control at least
    four times `8+0.08`; at a quarter of the **measured 2277** games an hour
    (S198, 2026-09-08, where the estimate was 2337) a `{-5, 0}` pair is about
    74 hours worst case and 45 on a bound. Options: that pair on a
    weekend; a wider pair; or a fixed-rounds reading (2000 games, about 3.5 h,
    +/-8 Elo) which would change the accepts and is therefore a decision. It
    sits at Open entry 17 behind S148 and S159 until answered.
  - **The three lows of the 2026-09-04 audit re-run still wait on the owner**
    (`go infinite` printing `bestmove` unasked; a bad token in `position ...
    moves` skipped silently; the aborted-iteration best move assuming its table
    entry survives) -- one decision or steps, unchanged by the reorder.
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
  - **The corpus is gitignored and does not survive a machine move; the fit
    scripts do -- corrected 2026-09-05.** `.tuning/` holds `selfplay_v2.tsv`
    (715 MB, 11003693 positions), which is gitignored and was not on this
    MacBook. The five scripts S065 leaned on — `apply_fit.py`, `verify_fit.py`,
    `anchors.py`, `reanchor.py`, `diff_fit.py` — **are tracked** since `c56ab41`
    (`.gitignore` carries `!.tuning/*.py`), which this item wrongly called
    gitignored until S192's enrichment agent checked. What is true instead:
    `anchors.py` hard-codes `ROOT = "/home/max/ws/chesso/"` and does not run
    here; patched in a scratch copy it reproduces 10 of 10 pinned values at
    HEAD. S192 owns the fix. This is the
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
