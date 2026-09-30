#!/usr/bin/env bash
#
# S114 VERDICT 1: THE NULL MOVE REDUCTION GROWS WITH THE STATIC SCORE'S LEAD
# OVER BETA, AND THE TERM IS MEASURED ALONE. Past the null-move block's guards
# in `negamax_at`, the reduction gains one ply for every whole
# `NullMoveEvalMargin` the node's static evaluation stands above beta, at most
# `NullMoveEvalCap`, clamped at zero, and the floor tests the whole of it:
#
#   R = NullMoveBase + depth / NullMoveDivisor
#     + min(max(static_eval - beta, 0) / NullMoveEvalMargin, NullMoveEvalCap)
#
#   the pass is made only where depth - 1 - R >= 1
#
# so a lead that leaves the null search no ply makes the pass impossible, not
# blind: the node is searched. The entry gate `static_eval >= beta` is in the
# tree behind `NullMoveEvalGate` and ships at 0 -- it is **S114's second
# verdict**, measured on the tree this one leaves, with DEC-233's second repair
# of the null-move guard cases then (DEC-243).
#
#   nohup adocs/data/S114_sprt.sh > .tuning/sprt_s114.log 2>&1 &
#
# **THE CANDIDATE IS THE TERM AND ITS FLOOR.** In `negamax_at`'s null-move
# block: the gate's clause `(NULL_MOVE_EVAL_GATE == 0 || static_eval >= beta)`
# in the condition, true at the shipped 0; `assert(static_eval !=
# TT_EVAL_NONE)` past the guards, since the in-check guard is what keeps the
# sentinel out; the term and the reduction computed there; the floor `depth - 1
# - null_reduction >= 1` on the whole R, the pass and its fail-soft return
# inside it unchanged. Three rows in `src/search_params.hpp`:
# `NullMoveEvalMargin` 94, `NullMoveEvalCap` 8 and `NullMoveEvalGate` 0; and a
# probe field, `null_reduction`, written only in the probing instantiation.
# Nothing else in `src/` moves: the zugzwang guard on `game_phase`, both edges
# of the mate band, the mate-score clamp, S097's `excluded_move` gate,
# ProbCut's block (S113), S097's singular block, S109's pruning and the
# generator are untouched.
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section (sections 2 to 5), written from the wiki and published
# prose. No other project's code was opened and no constant of one is behind
# anything here (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), stated at their site. `NullMoveEvalMargin` 94 is (b):
# one pawn in this engine's own material scale, `PAWN` in src/eval_tables.hpp;
# the wiki's Null Move Pruning page states the eval-scaled factor with no
# number, so there is no (a) to take. `NullMoveEvalCap` 8 is (c), the midpoint
# of 0 to 16, the owner's answer of 2026-09-08, with the measured alternative
# named beside it (the largest cap at which both mate suites pass, less one);
# the mate instruments did not make 8 inadmissible -- both fast suites are
# green at it -- so the midpoint stands. `NullMoveBase` 3 and `NullMoveDivisor`
# 6 are (b): S085's SPSA output over this engine's own games, which the sweep
# below did not re-decide (DEC-244). A seed each, for S127 to fit.
#
# THE SWEEP, S127'S INPUT AND NOT A DECISION (DEC-244).
# `adocs/data/S114_null_move_sweep.py` over S021's 300 positions (three
# stratified samples of 100, `go depth 11`, the tune build), the grid
# `NullMoveBase` 1..4 x `NullMoveDivisor` 3..8 at margin 94, cap 8, gate 0,
# node counts against the off row (cap 0, 3, 6: 27924538), not Elo. **Taken
# on `6e8bc63` (`0c0db1b` with S244's screen)** -- the first build's tree,
# with S022 verdict 2's early-out in quiescence; S244's identity with
# `0c0db1b` is shown on the bench positions only, not on S021's 300 -- and
# not re-run on the tree this lands on: DEC-244's reason
# the seeds stand is a lead inside the instrument's own sample-to-sample
# disagreement, which a rebase does not turn into a decision, and S127 refits
# all four anyway:
#
#   base  divisor  nodes     pooled  per sample
#   3     5        26351602  0.9437  0.9423 0.9437 0.9452   the sweep's first
#   2     3        26507807  0.9493  0.9521 0.9505 0.9448   second
#   3     6        26572771  0.9516  0.9325 0.9290 0.9957   the seeds, shipped
#
# The first's 0.8 % pooled lead flips on two of the three samples, inside the
# instrument's own disagreement, so the seeds stay; every base-1 cell costs
# more than the off row (1.02 to 1.25). `adocs/data/S114_null_move_sweep_d11.tsv`.
#
# THE PRIOR, recorded before the run (DEC-019), direction only and never a
# forecast. The eval-scaled term has a record at roughly this band -- one
# engine measured it at +5.36 +/- 4.21 at 10+0.1 and +11.48 +/- 6.92 at
# 60+0.6 (Weiss #129), another measured a cap on the term at +3.28 and +2.32
# (Ethereal 89ed7eb) -- and a cautionary pair at 3600, two passing SPRTs
# reverted for "very poor mate finding performance" (Stockfish 9a8dd81d), which
# is why the mate suites and the demolition below came first. The step file's
# range is 0 to +12; this project has measured three published figures at 0, 0
# and slower (DEC-019). No figure here is Elo for this engine.
#
# DEC-063'S EXPECTATION, stated before the run: **small and positive, or zero.**
# The pair is the gainer pair because an add ships only on evidence of gain
# (step file section 6); a truth under the interval terminates at H0, a truth
# near +2.5 walks, and the third row below is written for it.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-29, niced, counts only.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). The term's off value is
#   `NullMoveEvalCap` 0, and the gate's is its switch at 0, where it ships: no
#   value of the four numbers turns the gate off, so it has a switch, and the
#   clamp at zero is what makes cap 0 exact. The tune build at
#   `NullMoveEvalCap` 0 prints `bench` **3429473** with all eight replies of
#   the parent (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6) and its whole
#   121-line stream identical, time and nps stripped, to a Release build of
#   the parent; `bench 12` 1694808, its 105-line stream identical the same
#   way; and `tools/search_bench.py` the parent's counts at depth 9 (48304 /
#   71580 / 25413) and 12 (104784 / 244824 / 117798), best moves c3d5 e2a6
#   d7c8q at both -- `58585f8`'s engine, node for node
#   (`.tuning/coord/S114b_logs/identity_cap0_*`, `parent_*`). **The
#   coordinator re-proves this on the landing's parent before pinning.**
#
#   AND THE SHIPPED TREE AGAINST THE PARENT: `bench` 3429473 -> **4192793**,
#   +22.26 %, **seven of the eight replies unchanged, kiwipete's e2a6 ->
#   d5e6**; `bench 12` 1694808 -> 1792474, kiwipete's e2a6 -> d5e6 and the
#   fifth position's d8e7 -> a7a6. `search_bench` depth 9: 48304 -> 23875,
#   71580 -> 69842, 25413 -> 21491, **the midgame's best move c3d5 -> g5f6**;
#   depth 12: 104784 -> 106886, 244824 -> 237812, 117798 -> 86217,
#   **kiwipete's best move e2a6 -> d5e6**. The counts move by construction,
#   so INV-6 does not discharge this change and nothing here is Elo
#   (DEC-019).
#
#   THE FIXED-NODE DEPTH, stated as reach and counts (DEC-239):
#   `adocs/data/S097_fixed_node_depth.py`, `go nodes 1000000`, Hash 16, over
#   `tools/search_bench.py`'s three positions: 17 / 15 / 15 = 47 on the parent
#   -> 18 / 14 / 16 = **48**, kiwipete a ply shallower with its move e2a6 ->
#   d5e6, the other two a ply deeper. The term raises R, and at shallow depth
#   it also makes the floor refuse passes the parent made, so the tree can
#   grow as well as shrink; this is how far each goes on these three
#   positions, not a verdict forecast.
#
#   THE MATE INSTRUMENTS, before and not instead of the games (step file
#   section 5): both fast suites green at the seeds, "pruning does not hide a
#   forced mate" at every depth it runs, `test_mate_carry`, `test_mate_breadth`,
#   `test_mate_pv` and `test_engine`'s mate safety green. **The demolition**:
#   with the floor and the cap lifted together in a throwaway copy, S114's case
#   "the static-score term does not hide a forced mate" goes red -- the node
#   answers 373 in place of a mate at depths 5 and 15; with the floor alone
#   lifted the mate is lost at 5 and found at 15; with the cap alone lifted it
#   is found at both (`.tuning/coord/S114b_logs/demolition_red.log`; the first
#   build's tree answered -715, `.tuning/coord/S114_logs/demolition_red.log`).
#   The cap alone fails safe; the floor is the device that keeps the mate.
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON. `REF` is the landing's
# parent -- the commit that takes S022 verdict 2's early-out back out on its
# reading, whose engine is `58585f8`'s node for node -- and `CAND` is the
# landing, `NullMoveEvalCap` 8 and `NullMoveEvalGate` 0. S114 was first built
# on `6e8bc63`, the early-out's tree, and was rebased onto the removal's tree
# before this pair is pinned -- the S113 precedent: every number above was
# re-taken there and the goldens of item 16 re-derived on it.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`, 20000 rounds -- the gainer pair, section 6's
# pre-registration for this verdict: an add ships only on evidence of gain;
# accepted at "merely not a regression" it would ship a pruning rule with no
# measured value.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound
# (`adocs/testing_strategy.md` section 1.1), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. The midpoint's figure is past
# fastchess's 40000-game cap, about 19.0 h, and a walk that reaches the cap is
# read as no verdict by the row below. A night, scheduled (DEC-155). The
# throughput is read off the banner and recorded rather than assumed.
#
# REGIME. `adocs/data/S022_v2_sprt.sh`'s: 8+0.08, Hash 16, one thread a side,
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
# carried by id from `adocs/data/S022_v2_sprt.sh`'s block as it stood on
# 2026-09-29 and re-read on the tree this lands on, the one S022 verdict 2's
# removal leaves: **no defect reachable in ordinary play, on the UCI surface,
# or able to move a reported score, move or line is open.** Items 1 to 15 are
# S022 verdict 2's, re-read; 16 is this verdict's own and 17 is new since:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding: open, and this
#      candidate is green in it at the golden as it stands.
#   2. The three S097 named on 2026-09-21: open, untouched here.
#   3. **`test_mate_carry`'s budgets**: S245's item 5 re-derived them under
#      DEC-156 as DEC-162 left it on the early-out's tree, which the removal
#      takes back out, and this candidate moves the tree again, so the
#      budgets are re-derived on it by `adocs/data/S203_case_sweep.sh`
#      (`adocs/data/S114_sweep_s170.txt`) and land as their own commit after
#      this one, a test change that moves no engine. Green here at the budgets
#      as they stand and at the new ones, no ceiling moved. S202's class is
#      still read beside the verdict as an `Incomplete mating PV` count per
#      side.
#   4. **S113's goldens belong to its tree** (DEC-233): standing. **The
#      capture-mate table they left was already stale on the parent**, rows 2
#      and 3 and row 4's label (item 16): a test's golden, green, reaching no
#      play, and older than this step -- open as **S248, a filler (DEC-171)**.
#   5. **S112's block's items 4 and 5 are resolved** (DEC-236): standing.
#   6. **ProbCut's preliminary at a child `negamax_at` never screened:
#      resolved in the tree by S244** (`192732f`), accepted without a run on
#      DEC-242's four conditions; **its second tier -- DEC-141's Debug
#      self-play and `tools/gate_extra.sh` -- is pending on the idle
#      machine**, and its step stays in `plan_current/` until it completes.
#      Nothing here touches ProbCut's block.
#   7. **S131's own goldens: none.** Its multicut row came back with the
#      removal and is item 16's old row.
#   8. **S242: closed** (`aef0257`, DEC-240).
#   9. **E21's coverage: two killers**, again after item 16's re-mine: S243's
#      direct case and the mined multicut row, observed red under E21 on this
#      tree.
#  10. **S243: closed** (`6eb2674`).
#  11. **S022 verdict 1's own goldens**: went back with its revert; not this
#      verdict's.
#  12. **S245: done** (`6e8bc63`).
#  13. **S022 verdict 2: read and removed.** Its early-out left on its reading,
#      and the three goldens it had re-derived went back to `58585f8`'s byte
#      for byte with the code -- S131's multicut row and, in `test_engine`'s
#      first-iteration case, the stop half's eight-queens board and the timer
#      half's dead-timer figure. Not this verdict's; a depth-1 iteration makes
#      no null move.
#  14. **The first-iteration stop**: the ruling of 2026-09-29 went with the
#      early-out, and the stop half runs on the eight-queens board again.
#  15. **S246: closed** (`58585f8`).
#  16. **This verdict's own goldens**, re-derived on this tree by their own
#      scripts (DEC-142, DEC-233); each site quotes the old row. **The
#      capture-mate table** of "pruning does not hide a forced mate",
#      `adocs/data/S230_mine_r01_row.py depths`, the seven sweeps: no row went
#      red -- each still reads its mate at its old depth -- but the rule's
#      answer moved, depths 7, 9, 11, 10 -> **7, 7, 10, 9**, rows 3 and 4's
#      labels "R02" -> "C05, R02, since S114" and "no S091 mutant, since
#      S113" -> "C02, C05, C07, since S114", no mate distance moving
#      (`.tuning/coord/S114b_logs/capmates/`); the same sweeps on the parent
#      read 7, 7, 10, 10 (item 4). **The multicut row**: S131's row
#      `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42` stayed green and
#      stopped separating E21, both builds reading its mate in 6 at 13 and 14,
#      so `adocs/data/S097_mine_mate_row.py` stages 2 to 6 ran in guard mode
#      (DEC-238): one separator of 269, taken by both modes,
#      `R2Q1bk1/5q2/4bP1p/2p1P3/3pB3/7P/2P3PK/8 w - - 3 50` at **depth 13**,
#      mate in 5 -- stockfish's `go mate 5` in a fresh process reads #+5 where
#      its depth-20 label reads #6 -- observed red under E21 and green shipped
#      (`adocs/data/S114_rb_remine_s097.log`). One guard case took the repair
#      DEC-243 names: "no defender node inside the mate band makes a null
#      move" drives its 104 nodes at the shallowest depth the capped term
#      leaves a ply at, 15 at the seeds, since at every beta in the band the
#      term is at its cap. And `NULL_DRIVE_DEPTH` is derived from the base and
#      the divisor, 5 at the seeds, every assertion the one it was (DEC-244).
#      On an H0 or no verdict the table's rows, the multicut row and its depth,
#      and the defender case's depth go back byte for byte with the term's
#      code. The first build's re-derivations on `6e8bc63` are that tree's
#      record (`.tuning/coord/S114_remine/`, `adocs/data/S114_remine_s097.log`).
#  17. **S247, a filler**: the stop half's "13.8 ms" golden, back with the
#      removal, reads 6 to 7 ms on the idle machine (S245's item 2 live
#      again); the 3 ms floor holds and the case passes. A test's golden, no
#      play reached; it needs the idle machine.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> **the term gains at least 5 nElo at its seeds, and it
#                   stays.** `NullMoveEvalMargin` 94, `NullMoveEvalCap` 8,
#                   `NullMoveBase` 3 and `NullMoveDivisor` 6 stay at their
#                   seeds and join S127's set, the sweep's table above as that
#                   lane's input -- an H1 says the term is worth having at the
#                   values tried and not where its maximum is. The gate then
#                   takes S114's second verdict on this tree (DEC-243). The
#                   stopping run's Elo is upward-biased and is not the effect
#                   size (DEC-063).
#
#   H0, interval -> **a loss, and the revert is one default.**
#   wholly         `NullMoveEvalCap` to 0, the off value proved above, so the
#   below zero     engine is the reference tree's to the node the moment it
#                  moves -- and **the term's code leaves with it** (the S238
#                  pattern): its line, the margin and the cap, the probe
#                  field, the six S114 cases that read the term and the six
#                  NT mutants, `MANUAL.md` and `DEV_MANUAL.md` with them,
#                  behaviour-neutral at the reverted default and discharged by
#                  INV-6 against the reference's own engine; the two re-derived
#                  rows of item 16 and the defender case's depth go back byte
#                  for byte. **The S170 budgets are re-derived on the reverted
#                  tree** by `adocs/data/S203_case_sweep.sh` (DEC-156 as
#                  amended by DEC-162): S245's were derived on the early-out's
#                  tree and this landing's on the S114 tree, so neither set is
#                  the reverted tree's. **One reason to keep code is stated
#                  now**: the gate's switch, its clause and its row stay at 0,
#                  because S114's second verdict flips them on the tree the
#                  first leaves and adds no code then (DEC-243); so does the
#                  floor's place inside the block, which at cap 0 tests the
#                  reduction before S114 exactly, and the derived
#                  `NULL_DRIVE_DEPTH`. The first suspect for the loss is the
#                  one section 5 names -- a capped term makes the floor refuse
#                  passes the parent made at shallow depth -- and the honest
#                  follow-up is a clamp in place of the skip or no ship: a
#                  second experiment, not this one.
#
#   No verdict   -> **a zero, read the same way (DEC-063)**: terminate at
#   or an          fastchess's cap, record the zero with the games played and
#   interval       the interval, and the term goes to its off value with its
#   reaching       code on the H0 row's terms, **the S170 budgets re-derived
#   above zero     on the reverted tree** with them (DEC-156 as amended by
#                  DEC-162), since neither S245's set nor this landing's is
#                  that tree's. No follow-up run and no second pair; another
#                  margin, the clamp in place of the skip or a wider pair is a
#                  second experiment, which S127's fit is the place for.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands this verdict's candidate, the static-score term
# at `NullMoveEvalCap` 8 with `NullMoveEvalGate` 0; REF is its parent, the
# commit the landing sits on. Both are pinned as explicit shas after the
# landing commit exists, by editing the defaults below: **CAND = the landing
# commit's sha, REF = CAND^ (the removal commit)**. Never `HEAD` and `HEAD^`:
# the S170 budgets land as their own commit after the landing, and once they
# have, `HEAD` is that commit and `HEAD^` the landing -- one engine against
# itself.
#
#   git -C /home/max/ws/chesso rev-parse --short <landing>    # CAND
#   git -C /home/max/ws/chesso rev-parse --short <landing>^   # REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S114_sprt.sh before running (CAND is S114 verdict 1's" \
         "landing commit, REF the commit it sits on)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s114_v1_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
