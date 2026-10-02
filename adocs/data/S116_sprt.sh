#!/usr/bin/env bash
#
# S116: RAZORING AT DEPTH ONE, THE VERIFIED FORM, WITH REVERSE FUTILITY'S PLY
# FLOOR (DEC-248). Between reverse futility and the null move in `negamax_at`:
#
#   at a node with !is_pv && !is_in_check && excluded_move == 0
#               && depth <= RazorDepth (1) && ply >= RfpMinPly (3)
#               && alpha inside (-MATE_MIN, MATE_MIN)
#               && static_eval + RazorMargin (282) <= alpha
#     score = quiescence(alpha, beta)          -- the node's own window
#     if score <= alpha: return score          -- fail soft, the verification
#     else fall through to the move loop
#
# `static_eval` is the node's raw static score, never the table-tightened
# estimate; in check it is `TT_EVAL_NONE` and `!is_in_check` short-circuits
# before the margin reads it.
#
#   nohup adocs/data/S116_sprt.sh > .tuning/sprt_s116.log 2>&1 &
#
# **THE CANDIDATE IS THE RULE.** In `src/search.cpp` the block and its comment;
# in `src/search_params.hpp` two rows, `RazorMargin` 282 (0..2000) and
# `RazorDepth` 1 (0..8), and a sentence on `RfpMinPly`, which the rule reads;
# in `src/data_structures.hpp` three probe fields a test reads and the shipping
# instantiation folds away. Nothing else in `src/` moves.
#
# **THE PLY FLOOR IS DEC-248's, AND THE FORM AS SPECIFIED WAS REFUSED.** The
# step file's form, without `ply >= RfpMinPly`, found 9 of the 26 S145 mates in
# two an iteration late and turned "every mate in two is found on time" and
# "pruning does not hide a mate against the material leader" red: the quiet
# mating move sits at an off-PV depth-1 node at ply 2, which quiescence cannot
# see. With the floor, all 26 are found at iteration 3 and every hard mate
# assertion holds. The owner ruled that form lands (DEC-248).
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section, written from the wiki and published prose. No other
# project's code was opened and no constant of one is behind anything here
# (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), stated at their site. `RazorMargin` 282 is (a)
# literature: the wiki's Razoring page puts the drop-to-quiescence margin at
# "~three pawns", read in chesso's own material scale, where a pawn is 94 --
# 3 * 94 = 282, as `QsFutilityMargin`'s 188 read "around 200 centipawns". The
# step file's 300 was the wiki's number in its own unit. `RazorDepth` 1 is the
# rule as the step specifies it, the depth-one form; not a fitted value. Both
# join S127's set if the rule stays.
#
# THE PRIOR, recorded before the run (DEC-019), direction only and never a
# forecast. The record carries additions read +6 to +13.5 at their bands and
# removals at ~0 in three engines, one of which bought ~1 back at a much higher
# band. No figure here is Elo for this engine.
#
# DEC-063'S EXPECTATION, stated before the run: **small, of either sign.** The
# pair is the gainer pair (DEC-248): an addition ships only on evidence of
# gain. A truth under the interval terminates at H0, a truth near +2.5 walks,
# and the third row below is written for it.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-10-02, counts only, on
# `f82e5a3`'s tree (S250's completion; DEC-248's commit above it moves
# documents only). Binaries: a Release build of `f82e5a3` made in a throwaway
# worktree and deleted after (sha256 `14b6f2b1...`, kept in the S116 worktree
# as `.tuning/coord/S116/chesso_f82e5a3`), this tree's Release build and this
# tree's tune build.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). `RazorDepth` 0: the tune
#   build there prints `bench` **3513310** with all eight replies of the parent
#   (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6) and its whole 121-line stream
#   identical, time and nps stripped, to the parent's Release build; `bench 12`
#   1619863, its 105-line stream identical the same way; `tools/search_bench.py`
#   the parent's counts at depth 9 (34236 / 71552 / 25351) and 12 (70283 /
#   240680 / 80264), best moves c3d5 e2a6 d7c8q at both. No value of
#   `RazorMargin` is off; the depth is the switch. **The coordinator re-proves
#   this on the landing's parent before pinning.**
#
#   AND THE SHIPPED TREE AGAINST THE PARENT: `bench` 3513310 -> **4081329**,
#   +16.17 %, kiwipete's reply e2a6 -> d5e6 and the fifth position's d8e7 ->
#   d8d6, the other six unchanged; `bench 12` 1619863 -> 1860699, +14.87 %,
#   kiwipete d5e6 and the fifth d8e7 -> d8d7. `search_bench` depth 9: 34236 ->
#   32932, 71552 -> 70095, 25351 -> 25178, no move changing; depth 12: 70283 ->
#   67792, 240680 -> 280873, 80264 -> 137893, kiwipete e2a6 -> d5e6. The node
#   is counted twice where the rule drops it, once in negamax and once in
#   quiescence, as the leaf drop always has. The counts move, so INV-6 does not
#   discharge this change and nothing here is Elo (DEC-019).
#
#   THE FIXED-NODE DEPTH, stated as reach and counts (DEC-239):
#   `adocs/data/S097_fixed_node_depth.py`, `go nodes 1000000`, Hash 16, over
#   `tools/search_bench.py`'s three positions: 17 / 15 / 16 = 48 on the parent
#   -> 17 / 14 / 16 = **47**, kiwipete a ply shallower with its move e2a6 ->
#   d5e6.
#
#   THE FIRE RATE, stated as reach (DEC-239), a throwaway census build over the
#   three `search_bench` positions at depth 12, one process each: 28924 nodes
#   met every guard but the margin, **3904 asked quiescence** (13.5 % of those,
#   0.80 % of all 486558 nodes), 3250 of them cut (83.2 %) and 654 fell through
#   (16.8 %). At depth 9 on the floorless form, 80.9 % and 19.1 %. The record's
#   ">40 %" fall-through is another engine's quiescence and direction only.
#
#   THE MATE INSTRUMENTS, before and not instead of the games: both fast suites
#   green, 41 of 41 each, every mate case among them -- "pruning does not hide
#   a forced mate", "pruning does not hide a mate against the material leader",
#   `test_mate_carry`, `test_mate_breadth`, `test_mate_pv`, `test_engine`'s
#   mate safety with all 26 mates in two at iteration 3 -- after the two
#   scripted goldens of item 19 below were re-derived.
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON. `REF` is the landing's
# parent and `CAND` is the landing. No verdict is pending under this tree:
# S114's second verdict was read as a zero and its gate removed (`bd5a6cb`),
# S115 read H1 and completed.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`, 20000 rounds -- the gainer pair, the owner's ruling
# (DEC-248). The step file's section 6 named `{-5, 5}`; DEC-248 rejected it as
# shipping an addition on "not a regression".
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound
# (`adocs/testing_strategy.md` section 1.1), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. The midpoint's figure is past
# fastchess's 40000-game cap, about 19.0 h, and a walk that reaches the cap is
# read as no verdict by the row below. A night, scheduled (DEC-155). The
# throughput is read off the banner and recorded rather than assumed.
#
# REGIME. `adocs/data/S114_v2_sprt.sh`'s: 8+0.08, Hash 16, one thread a side,
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
# carried by id from `adocs/data/S114_v2_sprt.sh`'s block and re-read on the
# tree this lands on: **no defect reachable in ordinary play, on the UCI
# surface, or able to move a reported score, move or line is open.** S247,
# S249 and S250 are done; **S251 and S252 are the open fillers**, items 20
# and 21 below.
#
#   1. **`test_mate_breadth`'s headroom**: S250 re-placed the floor at 145
#      (146 shipping, 143 weakened); on this candidate
#      `adocs/data/S156_mined_floor_sweep.py` reads 149 shipping and 143
#      weakened, the gate at `RfpMinPly` 1 red, so the floor separates with a
#      gap of 6 and is not moved. `MATE_IN_THREE_FLOOR` 11 separates too, 12
#      against 7 (`adocs/data/S154_floor_margin_sweep.py floor`). **S249:
#      done.**
#   2. The three S097 named on 2026-09-21: open, untouched here.
#   3. **`test_mate_carry`'s budgets**: re-swept on this candidate by
#      `adocs/data/S203_case_sweep.sh` (DEC-156 as amended by DEC-162), the
#      rule stated before the grid was read, the grid
#      `adocs/data/S116_sweep_s170.txt`. **Unresolved on this tree, as at
#      S114 v2**: the rule moves C 1000000 -> 1500000, D 3000000 -> 2000000,
#      E 300000 -> 100000 and F 500000 -> 100000, and C's new cell reports 8
#      mate lines, 5 short, over its ceiling of 0, so `test_mate_carry` goes
#      red in both builds at the rule's answer. The patch is held until
#      this verdict reads (DEC-249, DEC-246's rule applied), since raising a
#      ceiling relaxes a test. The standing budgets
#      are green (A, B and D report, the majority), and neither of this pair's
#      engines is touched either way, because the TSV is a test's data. S202's
#      class is still read beside the verdict as an `Incomplete mating PV`
#      count per side.
#   4. **The capture-mate table**: re-derived on this candidate (item 19).
#   5. **S112's block's items 4 and 5 are resolved** (DEC-236): standing.
#   6. **S244: closed.**  7. **S131's own goldens: none.**  8. **S242: closed.**
#   9. **E21's coverage: two killers**, S243's direct case and the multicut row,
#      both green on this candidate.
#  10. **S243: closed.**  11. **S022 verdict 1's goldens**: went back with its
#      revert.  12. **S245: done.**  13. **S022 verdict 2: read and removed.**
#  14. **The first-iteration stop**: untouched. **S247: done.**
#  15. **S246: closed.**  16. **S114 verdict 1's goldens**: went back.
#  17. **S114 verdict 2**: read as a zero; its gate and code left (`bd5a6cb`).
#  18. **S115**: read H1 and completed; `AspirationFailLowPull` 2 in both
#      engines of this pair.
#  19. **This verdict's own goldens** (DEC-142, DEC-233, DEC-248): the
#      `golden_defaults` rows `RazorMargin` and `RazorDepth`, the count 71 ->
#      73; the capture-mate table's seven sweeps re-taken, depths 7, 7, 10, 10
#      -> 9, 8, 10, 10, labels `C02, C05, R02, since S112` -> `no S091 mutant,
#      since S116` (row 1), `C02, R02, since S248` -> `C02, C05, R02, since
#      S116` (row 3), `R02, since S248` -> `no S091 mutant, since S116` (row
#      4); the aspiration case's first `first_mate_depth` 9 -> 10
#      (`adocs/data/S188_repair_goldens.py first-mate`: `d9:cp1135 d10:mate5`).
#      Each site quotes the row before it.
#  20. **New, `RfpMinPly` at 2 is no longer safe**: razoring reads that floor,
#      and at 2 -- inside the declared range, the value S145 measured for
#      reverse futility alone -- the tune build finds 17 of the 26 mates in two
#      at iteration 3. Unreachable in ordinary play (the shipped value is 3),
#      a tuning range's floor: recorded at `RfpMinPly`'s comment and in
#      `MANUAL.md`, the range not moved by this step. Whether the floor goes
#      to 3 or razoring takes a floor of its own is **S251, a filler, open**
#      (DEC-171).
#  21. **New, not this step's: mutant M06a survives** --
#      `M06a_rfp_ply_floor_minus1`, reverse futility's floor at 2 -- on this
#      candidate and on its parent `f82e5a3` alike: no fast-suite case
#      separates RFP at ply 2 any more. A test property reaching no play:
#      **S252, a filler, open** (DEC-171).
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063,
# DEC-248).
#
#   H1 accepted  -> **razoring gains at least 5 nElo at its seeds, and it
#                   stays.** `RazorMargin` 282 and `RazorDepth` 1 join S127's
#                   set; a depth-scaled margin and the multi-depth form are
#                   S127's to price, not this reading's. The stopping run's Elo
#                   is upward-biased and is not the effect size (DEC-063).
#
#   H0, interval -> **a loss, and the rule's code leaves** (the S238 pattern,
#   wholly         DEC-248): `RazorDepth` to 0 is the off value proved above,
#   below zero     so the engine is the reference tree's to the node the moment
#                  it moves -- and then the block in `negamax_at` and its
#                  comment, the two rows in `src/search_params.hpp` and the
#                  `RfpMinPly` sentence, the three probe fields, the
#                  `golden_defaults` rows (73 -> 71), the two `MANUAL.md` rows
#                  and the `RfpMinPly` row's sentence, the ten direct cases
#                  (the razor_drive_t fixture and every "razoring ..." case)
#                  and `tools/mutants/S116_razoring.py` (RZ01 to RZ11); the
#                  re-derived goldens of item 19 go back byte for byte, since
#                  the reverted tree is the reference's node for node -- each
#                  site quotes the row it restores. Item 20 is moot with it.
#                  Behaviour-neutral at the reverted default and discharged by
#                  INV-6 against the reference's own engine.
#
#   No verdict   -> **a zero, read the same way, and the code leaves on the H0
#   or an          row's terms (DEC-248).** Terminate at fastchess's cap,
#   interval       record the zero with the games played and the interval. The
#   reaching       step file's section 6 left this case to "the owner's call";
#   above zero     DEC-248 answers it: the rule adds nodes (+16 % `bench`), so
#                  no node saving argues for keeping it. No follow-up run and
#                  no second pair; another margin or depth is S127's to fit.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands this verdict's candidate, razoring at
# `RazorDepth` 1 with the ply floor; REF is its parent, the commit the landing
# sits on. Both are pinned as explicit shas after the landing commit exists, by
# editing the defaults below: **CAND = the landing commit's sha, REF = CAND^**.
# Never `HEAD` and `HEAD^`: anything that lands after the landing -- the S170
# budgets of item 3, this file's own pinning -- makes `HEAD` a later commit and
# `HEAD^` the landing, one engine against itself.
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
         "adocs/data/S116_sprt.sh before running (CAND = the landing" \
         "commit's sha, REF = CAND^)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s116_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
