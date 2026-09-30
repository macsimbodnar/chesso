#!/usr/bin/env bash
#
# S114 VERDICT 2: THE NULL MOVE'S ENTRY GATE, MEASURED ALONE. The null move is
# tried only at a node whose raw static evaluation stands at or above beta:
#
#   the pass needs, beside the block's other guards,
#     NullMoveEvalGate == 0 || static_eval >= beta
#
# and `NullMoveEvalGate` goes from 0 to 1. `static_eval` is the node's raw
# static score -- never the table-tightened estimate -- and is `TT_EVAL_NONE`
# in check, where `!is_in_check` short-circuits before the gate reads it.
#
#   nohup adocs/data/S114_v2_sprt.sh > .tuning/sprt_s114v2.log 2>&1 &
#
# **THE CANDIDATE IS ONE DEFAULT.** The switch, its clause in `negamax_at`'s
# null-move condition and its row in `src/search_params.hpp` were in the tree
# already: S114's first verdict built them, and its H0 kept them at 0 for this
# one (DEC-243). The landing moves `NullMoveEvalGate` 0 -> 1 in the X-macro and
# the comments that describe it; nothing else in `src/` moves -- no margin on
# the gate (an S127-era form, section 4 of the step file), no term, no base or
# divisor change (DEC-244).
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section, written from the wiki and published prose. No other
# project's code was opened and no constant of one is behind anything here
# (DEC-016, DEC-104, DEC-105, DEC-134). No seed: a verdict switch (DEC-215
# clause 2) whose 1 is the published bare form and whose 0 is the parent.
#
# THE PRIOR, recorded before the run (DEC-019), direction only and never a
# forecast. The wiki names the gate as a form engines have used -- its Fruit
# tried the null move only on a static evaluation greater than beta -- and
# carries no figure for it that this step read. No figure here is Elo for this
# engine.
#
# DEC-063'S EXPECTATION, stated before the run: **small, of either sign.** The
# gate narrows the null move to nodes that already stand above beta, so it
# removes passes -- and their cutoffs -- where the static score is below beta;
# whether the nodes searched in their place buy more than they cost is what
# the games are for. The pair is the gainer pair: a narrowing ships only on
# evidence of gain.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-30, counts only, on
# the tree S115 leaves (`465b43b`). Three binaries: a Release build of
# `465b43b` made in a throwaway worktree and deleted after (sha256
# `f5b7e895...`, kept as `.tuning/coord/S114_v2/chesso_465b43b`), this tree's
# Release build and this tree's tune build.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). `NullMoveEvalGate` 0: the
#   tune build there prints `bench` **3513310** with all eight replies of the
#   parent (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6) and its whole 121-line
#   stream identical, time and nps stripped, to the parent's Release build;
#   `bench 12` 1619863, its 105-line stream identical the same way; and
#   `tools/search_bench.py` the parent's counts at depth 9 (34236 / 71552 /
#   25351) and 12 (70283 / 240680 / 80264), best moves c3d5 e2a6 d7c8q at both
#   (`.tuning/coord/S114_v2/bench_*`, `sb*_*`). **The coordinator re-proves
#   this on the landing's parent before pinning.**
#
#   AND THE SHIPPED TREE AGAINST THE PARENT: `bench` 3513310 -> **4041913**,
#   +15.05 %, kiwipete's reply e2a6 -> d5e6 and the other seven unchanged;
#   `bench 12` 1619863 -> 1776047, +9.64 %, all eight replies unchanged.
#   `search_bench` depth 9: 34236 -> 33312, 71552 -> 64982, 25351 -> 25242;
#   depth 12: 70283 -> 71035, 240680 -> 178581, 80264 -> 110744; no best move
#   moves at either depth. The counts move, in both directions, so INV-6 does
#   not discharge this change and nothing here is Elo (DEC-019).
#
#   THE FIXED-NODE DEPTH, stated as reach and counts (DEC-239):
#   `adocs/data/S097_fixed_node_depth.py`, `go nodes 1000000`, Hash 16, over
#   `tools/search_bench.py`'s three positions: 17 / 15 / 16 = 48 on the parent
#   -> 19 / 13 / 16 = **48**, the midgame two plies deeper, kiwipete two
#   shallower with its move e2a6 -> d5e6. Reach, not a verdict forecast.
#
#   THE MATE INSTRUMENTS, before and not instead of the games: both fast suites
#   green at gate 1, 41 of 41 each, every mate case among them -- "pruning
#   does not hide a forced mate", `test_mate_carry`, `test_mate_breadth`,
#   `test_mate_pv`, `test_engine`'s mate safety -- after the repairs and the
#   re-derivations of item 19 below.
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON. `REF` is the landing's
# parent and `CAND` is the landing, `NullMoveEvalGate` 1. No verdict is pending
# under this tree: S115 read H1 (`d544872`) and completed (`465b43b`).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`, 20000 rounds -- the gainer pair.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound
# (`adocs/testing_strategy.md` section 1.1), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. The midpoint's figure is past
# fastchess's 40000-game cap, about 19.0 h, and a walk that reaches the cap is
# read as no verdict by the row below. A night, scheduled (DEC-155). The
# throughput is read off the banner and recorded rather than assumed.
#
# REGIME. `adocs/data/S115_sprt.sh`'s: 8+0.08, Hash 16, one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 of 12 cores (DEC-050),
# whatever governor the machine is under, recorded by the run and not set
# (DEC-195). **No harness change since the last fixed-rounds A/A** as this is
# written; if fastchess, the book, the adjudication or the machine moves before
# this starts, the A/A of 1000 fixed rounds comes first and is read with
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
# carried by id from `adocs/data/S115_sprt.sh`'s block and re-read on the tree
# this lands on: **no defect reachable in ordinary play, on the UCI surface,
# or able to move a reported score, move or line is open.** **S247 and S249 are
# the open fillers.**
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding: open, and this
#      candidate is green in it; its tabled comment's staleness is **S249, a
#      filler, open** (DEC-171), not fixed here.
#   2. The three S097 named on 2026-09-21: open, untouched here.
#   3. **`test_mate_carry`'s budgets**: re-swept on this candidate by
#      `adocs/data/S203_case_sweep.sh` (DEC-156 as amended by DEC-162), the
#      rule stated before the grid was read, the grid
#      `adocs/data/S114_v2_sweep_s170.txt`. **Unresolved on this tree**: the
#      rule moves A 500000 -> 100000, C 1000000 -> 500000 and D 3000000 ->
#      1000000, and C's new cell reports 4 mate lines, all 4 short, over its
#      ceiling of 0, so `test_mate_carry` goes red in both builds at the rule's
#      answer. The patch is held for the owner's decision, since raising a
#      ceiling relaxes a test. The standing budgets (S115's) are green, and
#      neither of this pair's engines is touched either way, because the TSV is
#      a test's data. The `--ceilings` reading of row C (6 with S115's grid, 4
#      with this one, where the test holds 0) is still not acted on: re-sweep
#      grids stay out of that command. S202's class is still read beside the
#      verdict as an `Incomplete mating PV` count per side.
#   4. **The capture-mate table**: S248's rows re-derived on this candidate
#      (item 19), depths unmoved, row 4's label moved.
#   5. **S112's block's items 4 and 5 are resolved** (DEC-236): standing.
#   6. **S244: closed.**  7. **S131's own goldens: none.**  8. **S242: closed.**
#   9. **E21's coverage: two killers**, S243's direct case and the multicut row
#      re-mined on this candidate (item 19).
#  10. **S243: closed.**  11. **S022 verdict 1's goldens**: went back with its
#      revert.  12. **S245: done.**  13. **S022 verdict 2: read and removed.**
#  14. **The first-iteration stop**: untouched.  15. **S246: closed.**
#  16. **S114 verdict 1's goldens**: went back byte for byte with its removal.
#  17. **S247, a filler**: the stop half's "13.8 ms" golden, open, needs the
#      idle machine.
#  18. **S115**: read H1 and completed; `AspirationFailLowPull` 2 in both
#      engines of this pair.
#  19. **This verdict's own goldens and repairs** (DEC-142, DEC-233): the
#      `golden_defaults` row `NullMoveEvalGate` 0 -> 1, the count 72; the
#      capture-mate table's seven sweeps re-taken, depths 7, 7, 10, 10
#      unmoved and row 4's label `R02, since S248` -> `no S091 mutant, since
#      S114`; the multicut row re-mined in guard mode (DEC-238), S131's row
#      -> `4Q3/p7/2p2p2/P3n2k/7P/2P3P1/5q2/7K b - - 4 42` at depth 14, mate
#      in 6 (`adocs/data/S114_v2_remine_s097.log`); eight null-move guard
#      cases' drives arranged so their premise holds under the gate, every
#      assertion kept, each mutant re-observed killed; the new case "the null
#      move's entry gate refuses a static score below beta" with NG01 to NG03;
#      N02 declared equivalent in the release build with a tune-only leg.
#  20. **S132's docstring offsets**: **S249, a filler, open**, not fixed here.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> **the gate gains at least 5 nElo, and it stays at 1.**
#                   `NullMoveEvalGate` 1 is the engine; a margin on the gate
#                   and a table-corrected input are S127-era forms (section 4)
#                   and not part of this reading. The stopping run's Elo is
#                   upward-biased and is not the effect size (DEC-063).
#
#   H0, interval -> **a loss, and the revert is one default.**
#   wholly         `NullMoveEvalGate` to 0, the off value proved above, so the
#   below zero     engine is the reference tree's to the node the moment it
#                  moves -- and **the switch's code leaves with it** (the S238
#                  pattern): the clause in `negamax_at`'s condition and its
#                  guard-list comment, the row in `src/search_params.hpp`, the
#                  `golden_defaults` row (72 -> 71), the `MANUAL.md` row, the
#                  new case "the null move's entry gate refuses a static score
#                  below beta" and `tools/mutants/S114_v2_entry_gate.py`
#                  (NG01 to NG03); E10's anchor re-pointed off the clause, its
#                  mutation unchanged; the re-derived goldens back byte for
#                  byte (S131's multicut row, the capture table's row 4 label),
#                  since the reverted tree is the reference's node for node.
#                  **What stays, and why**: the eight repaired guard cases keep
#                  their drives at the node's own static score (and the
#                  in-check case at `TT_EVAL_NONE`), because each premise holds
#                  without the gate as well -- a beta at the static score is an
#                  ordinary window -- and every assertion is the one it was;
#                  the helper's gate assertion and the N02 tune-only leg and
#                  its equivalence declaration leave with the switch they
#                  read. Behaviour-neutral at the reverted default and
#                  discharged by INV-6 against the reference's own engine.
#
#   No verdict   -> **a zero, read the same way (DEC-063)**: terminate at
#   or an          fastchess's cap, record the zero with the games played and
#   interval       the interval, and the gate goes to its off value with its
#   reaching       code on the H0 row's terms, the repaired cases staying for
#   above zero     the reason given there. No follow-up run and no second
#                  pair; a margin on the gate is S127's to try.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands this verdict's candidate, `NullMoveEvalGate` 1;
# REF is its parent, the commit the landing sits on. Both are pinned as
# explicit shas after the landing commit exists, by editing the defaults
# below: **CAND = the landing commit's sha, REF = CAND^**, pinned as `7c7328f`
# and `465b43b`, S115's completion commit, whose engine is S115's landing
# `eb334e3` byte for byte. Never `HEAD` and
# `HEAD^`: anything that lands after the landing -- the S170 budgets of item
# 3, this file's own pinning -- makes `HEAD` a later commit and `HEAD^` the
# landing, one engine against itself.
#
#   git -C /home/max/ws/chesso rev-parse --short <landing>    # CAND
#   git -C /home/max/ws/chesso rev-parse --short <landing>^   # REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-465b43b}"
CAND="${CAND:-7c7328f}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S114_v2_sprt.sh before running (CAND = the landing" \
         "commit's sha, REF = CAND^)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s114v2_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
