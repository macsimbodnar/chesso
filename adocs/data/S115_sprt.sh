#!/usr/bin/env bash
#
# S115: THE ASPIRATION LOOP'S FAIL-LOW PULL, MEASURED ALONE. When a root search
# fails low, the window's top comes down toward its bottom before the bottom is
# pushed below the returned score -- in `aspiration_after_fail`, the schedule
# `iterative_deepening_search` re-searches a failed root with:
#
#   fail low:   beta  -= (beta - alpha) * AspirationFailLowPull / 4
#               alpha  = score - delta
#   fail high:  beta   = score + delta          (unchanged)
#   escape:     a mate score, or delta > AspirationMaxDelta -> the full window
#   widening:   delta  = max(delta + 1, delta * AspirationWidenPct / 100)
#
# never computed while either bound is infinite -- the escape replaces both --
# and every re-search at the iteration's own depth, as before.
#
#   nohup adocs/data/S115_sprt.sh > .tuning/sprt_s115.log 2>&1 &
#
# **THE CANDIDATE IS THE PULL.** In `src/chesso.cpp`: the loop's window update
# moved into the pure `aspiration_after_fail`, the pull added there with its
# infinity guard, the doubling written as `AspirationWidenPct` (200, the
# parent's `delta += delta` exactly), and a record of every root search the
# loop makes, `uci_last_aspiration_searches`, which nothing in the engine reads
# -- the tests' only view of a window. Two rows in `src/search_params.hpp`:
# `AspirationWidenPct` 200 (100..400) and `AspirationFailLowPull` 2 (0..4); the
# declarations in `src/uci.hpp` (the S089/S132 pattern, accepted by the
# coordinator). Nothing else in `src/` moves: the triple 2 / 21 / 437, the mate
# escape, the disarming of the next band after a mate, the time manager (which
# reads nothing new and gains no check inside the loop) and the search are
# untouched.
#
# **THE PUBLISHED COMPANION IS NOT IN IT (DEC-245).** S115 built a root
# fail-high depth reduction beside the pull -- a ply off each consecutive root
# fail-high's re-search, floored at 1, reset by a fail-low -- and it turned
# three of the fast suite's mate guards red in every form tried: the S074 gate
# case's first mate at iteration 12 against 9, 5 of the 26 mates in two on
# time against 26, the mined set 102 exact against its floor of 143. The rule
# is refused on this engine as its guards stand and its code left before
# landing; whether those guards should assert what they do is the owner's
# question, parked. Its code and the three variants tried are
# `adocs/data/S115_reduction_as_built.diff` and `S115_reduction_variant_*.diff`.
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section, written from the wiki and published prose. No other
# project's code was opened and no constant of one is behind anything here
# (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), stated at their site. `AspirationFailLowPull` 2 is (c),
# the midpoint of 0 to 4 quarters, stated as such; that one half is also a form
# the published record carries is a record and not the number's source.
# `AspirationWidenPct` 200 is (b), a derivation over chesso's own positions:
# the sweep below, which moves it only on a lead held on every sample
# (DEC-244), and none did. The triple is S085's fit, not touched (DEC-244's
# precedent: a sweep does not re-decide a fitted value).
#
# THE SWEEP, THE VERDICT COUNT AND S127'S INPUT, NOT A FORECAST.
# `adocs/data/S115_aspiration_sweep.py` over S021's 300 positions -- S021's own
# picker at its recorded offsets 0, 37 and 71 -- at `go depth 11` on the tune
# build, the off row at the shipped triple re-read from `src/search_params.hpp`.
# First with both rules the step built, on the first tree it was built on
# (`cd50c7a`, S114 verdict 1's candidate; `--rows first`,
# `adocs/data/S115_aspiration_sweep_d11.tsv`): the pull alone 1.0250 of the off
# row's 40704024 nodes pooled with 44 of 300 best moves changed, the reduction
# alone 0.9185 with 62 -- neither inert apart, so DEC-082's condition failed and
# the verdict count was two before DEC-245 took the reduction out, leaving this
# one. Then the tree that ships, the tree S114 verdict 1's removal leaves
# (`--rows pull`, `adocs/data/S115_aspiration_sweep_pull_d11.tsv`, on a copy of
# its tune build), the ratio under the pull:
#
#   row       pull  ratio  nodes     pooled  per sample (0 / 37 / 71)
#   off       0     200    40114760  1.0000  1.0000 1.0000 1.0000   the parent
#   pull      2     200    40892425  1.0194  1.0144 1.0430 1.0104   shipped
#   pull150   2     150    40098837  0.9996  1.0245 1.0024 0.9860
#   pull300   2     300    40957623  1.0210  0.9905 1.0594 1.0174
#
# 150 leads 200 on samples 37 and 71 and not on 0, 300 on sample 0 alone, so
# the ratio stays 200 (DEC-244: a lead held on every sample, and neither is).
# The pull costs more nodes than the parent on every sample, 33 of 300 best
# moves changed -- stated as reach, not read as a forecast either way
# (DEC-239). The same rows on the first tree, where the reading was the same
# and the counts were not, are
# `adocs/data/S115_aspiration_sweep_pull_d11_cd50c7a.tsv` (the pull 1.0250).
#
# THE PRIOR, recorded before the run (DEC-019), direction only and never a
# forecast. The record's figure for this family is a bundle: one engine shipped
# the pull together with a root fail-high reduction and a slower widening as
# one patch, +6.78 +/- 4.87 at 10+0.1 and +8.19 +/- 5.31 at 60+0.6 (Weiss #183),
# and the reduction is the part this engine refused. **The pull alone is
# unmeasured anywhere in the record this step read.** No figure here is Elo for
# this engine.
#
# DEC-063'S EXPECTATION, stated before the run: **small, of either sign.** The
# pair is the gainer pair, the coordinator's ruling for this verdict: an add
# ships only on evidence of gain. A truth under the interval terminates at H0,
# a truth near +2.5 walks, and the third row below is written for it.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-30, counts only, on
# the tree S114 verdict 1's removal leaves (`f5eaa99`'s engine; `3f9ffa5` above
# it moves documents only). Three binaries, copied aside so no rebuild could
# replace one mid-run and named by hash in
# `.tuning/coord/S115b_logs/binaries.sha256`: the parent's Release build
# (`f5eaa99`), this tree's Release build and this tree's tune build.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). `AspirationFailLowPull` 0,
#   with `AspirationWidenPct` at its 200: the tune build there prints `bench`
#   **3429473** with all eight replies of the parent (c3d5 e2a6 d7c8q g7h8q
#   d8e7 a1b2 e5e6 e5e6) and its whole 121-line stream identical, time and nps
#   stripped, to a Release build of the parent; `bench 12` 1694808, its
#   105-line stream identical the same way; and `tools/search_bench.py` the
#   parent's counts at depth 9 (48304 / 71580 / 25413) and 12 (104784 / 244824
#   / 117798), best moves c3d5 e2a6 d7c8q at both -- `f5eaa99`'s engine, node
#   for node (`.tuning/coord/S115b_logs/counts.out`). **The coordinator
#   re-proves this on the landing's parent before pinning.**
#
#   AND THE SHIPPED TREE AGAINST THE PARENT: `bench` 3429473 -> **3513310**,
#   +2.44 %, **all eight replies unchanged**; `bench 12` 1694808 -> 1619863,
#   -4.42 %, all eight replies unchanged. `search_bench` depth 9: 48304 ->
#   34236, 71580 -> 71552, 25413 -> 25351; depth 12: 104784 -> 70283, 244824
#   -> 240680, 117798 -> 80264; no best move moves at either depth. The counts
#   move, and in both directions, so INV-6 does not discharge this change and
#   nothing here is Elo (DEC-019).
#
#   THE FIXED-NODE DEPTH, stated as reach and counts (DEC-239):
#   `adocs/data/S097_fixed_node_depth.py`, `go nodes 1000000`, Hash 16, over
#   `tools/search_bench.py`'s three positions: 17 / 15 / 15 = 47 on the parent
#   -> 17 / 15 / 16 = **48**, the tactical position a ply deeper, no move
#   changing. Over the sweep's 300 positions the pull costs +1.94 % nodes at a
#   fixed depth and on these three a fixed budget buys a ply more on one; the
#   two do not agree on a sign and neither is a verdict forecast.
#
#   THE MATE INSTRUMENTS, before and not instead of the games: both fast suites
#   green at the seeds, 41 of 41 each. The three guards that refused the
#   reduction, each re-taken over UCI on **this tree's pull-only Release
#   build** (`.ref-builds/bin/cand_release` of the rebase's worktree, sha256
#   `e85baab1...`, the bytes `build/src/chesso` holds) and on the parent's: the
#   S074 gate case's first mates 9 and 9 (`adocs/data/S188_repair_goldens.py
#   first-mate`), all 26 mates in two first reported at iteration 3, and the
#   mined set 146 exact at depth 10 against its floor of 143 -- the parent
#   reads 9 and 9, 26 and 146, so the pull moves none of them on this tree; the
#   constructed set's mates in three 12 of 24 on both, against
#   `MATE_IN_THREE_FLOOR` 11 (`.tuning/coord/S115b_logs/mates.out` and the
#   four files it names).
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON. `REF` is the landing's
# parent and `CAND` is the landing, `AspirationFailLowPull` 2. **No verdict is
# pending under this tree.** S115 was first built on `cd50c7a`, S114 verdict
# 1's candidate; that verdict read H0 (`22e00c3`: `794e4c3` against `d946b6f`,
# 2720 games, `Elo -18.54 +/- 10.19`), its static-score term left as `f3868fb`
# and the S170 budgets were re-derived as `f5eaa99`. This step was rebased
# onto that tree and every number above re-taken there -- the S114 precedent
# -- so the landing's parent is achesso's head after the removal, whose engine
# is `d946b6f`'s node for node on the bench positions with `NullMoveEvalGate`
# at 0 on both sides of this pair.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`, 20000 rounds -- the gainer pair: accepted at "merely not
# a regression" it would ship a change of the loop with no measured value.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound
# (`adocs/testing_strategy.md` section 1.1), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. The midpoint's figure is past
# fastchess's 40000-game cap, about 19.0 h, and a walk that reaches the cap is
# read as no verdict by the row below. A night, scheduled (DEC-155). The
# throughput is read off the banner and recorded rather than assumed.
#
# REGIME. `adocs/data/S114_sprt.sh`'s: 8+0.08, Hash 16, one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 (DEC-050), whatever
# governor the machine is under, recorded by the run and not set (DEC-195).
# **No harness change since the last fixed-rounds A/A** as this is written; if
# fastchess, the book, the adjudication or the machine moves before this
# starts, the A/A of 1000 fixed rounds comes first and is read with
# `adocs/data/S105_pairs.py` (DEC-143).
#
# WHERE THE OUTPUT LANDS. `OUT` is set in the exec line below, under
# `.tuning/`, which since DEC-235 is also `fastchess.sh`'s own default; set
# here so the run's directory is named for the step.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# carried by id from `adocs/data/S114_sprt.sh`'s block as it stood on
# 2026-09-30 and re-read on the tree this lands on: **no defect reachable in
# ordinary play, on the UCI surface, or able to move a reported score, move or
# line is open.** Items 1 to 17 are S114 verdict 1's, re-read on this tree; 18
# is S114 itself, its first verdict read and its second pending; 19 is this
# verdict's own and 20 is new since. **S247 and S249 are the open fillers**
# (items 17, and 1 and 20); S248 is done:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding: open, and this
#      candidate is green in it at the golden as it stands, 146 exact against
#      143, the parent's own count. **Its tabled shipping end is stale** --
#      the golden's comment reads 145 where this tree and its parent read 146
#      -- a test's comment, green, reaching no play, older than this step:
#      **S249, a filler, open** (DEC-171), not fixed here. S249 was written on
#      the first tree's readings, 147 there, and also names
#      `MATE_IN_THREE_FLOOR`'s comment, 12 written against 13 read on
#      `cd50c7a`, the first build's parent: on this tree both builds read 12,
#      what the comment says, so that half of S249 is not stale here.
#   2. The three S097 named on 2026-09-21: open, untouched here.
#   3. **`test_mate_carry`'s budgets**: re-swept on this tree by
#      `adocs/data/S203_case_sweep.sh` (DEC-156 as amended by DEC-162), the
#      full grid on a copy of the candidate's Release build, the rule stated
#      before the grid was read (`adocs/data/S115_sweep_s170.txt`). At the
#      budgets as they stand, the removal's (`f5eaa99`), the test is green in
#      both builds with its majority at exactly three of five -- C and D
#      report no mate line at their cells, and nor does F, which is unguarded
#      -- and the rule moves three of the six: C 500000 -> 1000000, D 1200000
#      -> 3000000, F 500000 at its stride 2 from 100000; A, B and E stay.
#      **They land as their own commit after the landing** (the S114
#      precedent), prepared as a separate patch, so this pair's two engines
#      are not touched by it -- the TSV is a test's data. With this grid added
#      the `--ceilings` command would read C 6 where it reads 0 (its 4000000
#      cell, 15 lines, 6 short), not acted on by the S245 and S114 precedent
#      that leaves re-sweep grids out of that command. S202's class is still
#      read beside the verdict as an `Incomplete mating PV` count per side.
#   4. **S113's goldens and the capture-mate table**: S248 re-derived the rows
#      on the removal's tree (7, 9, 11, 10 -> 7, 7, 10, 10, in `f3868fb`) and
#      is **done** (`3f9ffa5`); out of this change's reach by construction --
#      those rows call `search()` at one depth with the full window, where no
#      aspiration schedule runs -- and both fast suites are green with them
#      unchanged.
#   5. **S112's block's items 4 and 5 are resolved** (DEC-236): standing.
#   6. **S244: closed**, its second tier green (`c16f523`).
#   7. **S131's own goldens: none.**
#   8. **S242: closed** (`aef0257`, DEC-240).
#   9. **E21's coverage: two killers**, S243's direct case and S131's multicut
#      row, which S114 verdict 1's removal put back byte for byte (`f3868fb`)
#      and E21 applied by hand turns red on that tree
#      (`.tuning/coord/S114_rm/hand_checks.log`); the row is driven through
#      `search()` like item 4's and out of this change's reach.
#  10. **S243: closed** (`6eb2674`).
#  11. **S022 verdict 1's own goldens**: went back with its revert.
#  12. **S245: done** (`6e8bc63`).
#  13. **S022 verdict 2: read and removed** (`6893c0f`).
#  14. **The first-iteration stop** runs on the eight-queens board; untouched --
#      depth 1 has no window.
#  15. **S246: closed** (`58585f8`).
#  16. **S114 verdict 1's own goldens**: went back byte for byte with the
#      term's removal (`f3868fb`); none stands in this tree.
#  17. **S247, a filler**: the stop half's "13.8 ms" golden, open, needs the
#      idle machine.
#  18. **S114**: its first verdict read H0 and its term is out (`22e00c3`,
#      `f3868fb`); **its second verdict, the entry gate's flip (DEC-243), is
#      pending and runs after this one, on this tree**. `NullMoveEvalGate` is
#      at 0 in both engines of this pair, so nothing of it is measured here.
#  19. **This verdict's own goldens** (DEC-142): `tests/test_search_params.cpp`'s
#      `golden_defaults`, 70 -> 72, the two rows, a deliberate-change detector
#      with no script. Re-read by their own scripts on this tree's pull-only
#      Release build and **not moved**: the S074 case's `first_mate_depth`, 9
#      and 9 (`adocs/data/S188_repair_goldens.py first-mate`); the mined
#      set's shipping count, 146 as on the parent, against the floor 143;
#      `MATE_IN_THREE_FLOOR`'s shipping end, 12 as on the parent. No other
#      golden went red: both fast suites are green with every other row as
#      the parent has it.
#  20. **S132's docstring** calls offsets 0, 1 and 2 "the same 300 positions
#      src/search_params.hpp's aspiration rows were chosen over", where S021
#      recorded 0, 37 and 71 (its TSV and `adocs/data/README.md`); S113 and
#      S114 sampled 0, 1 and 2 as well. A data script's prose, no play reached:
#      **S249, a filler, open** (DEC-171), not fixed here.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> **the pull gains at least 5 nElo at its seed, and it
#                   stays.** `AspirationFailLowPull` 2 and `AspirationWidenPct`
#                   200 join S127's set, the sweep's table above as that lane's
#                   input -- an H1 says the pull is worth having at the weight
#                   tried and not where its maximum is. The stopping run's Elo
#                   is upward-biased and is not the effect size (DEC-063).
#
#   H0, interval -> **a loss, and the revert is one default.**
#   wholly         `AspirationFailLowPull` to 0, the off value proved above,
#   below zero     so the engine is the reference tree's to the node the
#                  moment it moves -- and **the pull's code leaves with it**
#                  (the S238 pattern): its lines in `aspiration_after_fail`,
#                  its row, its cases ("a fail-low pulls beta toward alpha
#                  and pushes alpha below the score", "the pull is never
#                  computed while either bound is infinite", the tune-only
#                  "at AspirationFailLowPull 0 a fail-low moves alpha alone",
#                  and point 3 of the loop case), AW01 to AW03, `MANUAL.md`
#                  and `DEV_MANUAL.md` with them, behaviour-neutral at the
#                  reverted default and discharged by INV-6 against the
#                  reference's own engine. **The loop case keeps a
#                  precondition**: its `REQUIRE(pulled > 0)` is the only
#                  proof that a re-search happened at all, and without one
#                  points 2 and 4 pass vacuously the day the four positions
#                  stop failing, so it is replaced and not deleted -- a count
#                  of the failures the four positions produce (fail-lows and
#                  fail-highs together), required above zero, in place of the
#                  count of pulls. The kept `AspirationWidenPct` row of
#                  `MANUAL.md` is reworded with it: its sweep was taken with
#                  the pull at 2, and the row then says the pull has left
#                  and the table is the record of a rule not in the tree.
#                  **One reason to
#                  keep code is stated now**: `AspirationWidenPct`, the pure
#                  `aspiration_after_fail`, the record of root searches and
#                  their cases stay, at 200 the parent's doubling node for
#                  node. The ratio is the schedule's own axis and not the
#                  pull's -- S021 shipped the doubling unswept and S127 needs
#                  it as a parameter to fit it, with this step's sweep as its
#                  input -- and the function and the record are what hold the
#                  loop's schedule to a test at all; AW04 and its case stay
#                  with them. Any golden re-derived on this tree goes back
#                  byte for byte, since the reverted tree is the reference's
#                  node for node.
#
#   No verdict   -> **a zero, read the same way (DEC-063)**: terminate at
#   or an          fastchess's cap, record the zero with the games played and
#   interval       the interval, and the pull goes to its off value with its
#   reaching       code on the H0 row's terms, the ratio and the schedule's
#   above zero     function staying for the reason given there. No follow-up
#                  run and no second pair; another weight is S127's to fit,
#                  and the refused reduction is the owner's question (DEC-245),
#                  not a second experiment of this step.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands this verdict's candidate, the fail-low pull at
# `AspirationFailLowPull` 2; REF is its parent, the commit the landing sits on.
# Both are pinned as explicit shas after the landing commit exists, by editing
# the defaults below: **CAND = the landing commit's sha, REF = CAND^**, which is
# `3f9ffa5`, S248's completion commit, whose `src/` tree is `f5eaa99`'s. Never
# `HEAD` and `HEAD^`: anything that lands after the landing -- the S170 budgets
# of item 3, this file's own pinning -- makes `HEAD` a later commit
# and `HEAD^` the landing, one engine against itself.
#
#   git -C /home/max/ws/chesso rev-parse --short <landing>    # CAND
#   git -C /home/max/ws/chesso rev-parse --short <landing>^   # REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-3f9ffa5}"
CAND="${CAND:-eb334e3}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S115_sprt.sh before running (CAND is S115's landing" \
         "commit, REF the commit it sits on)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s115_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
