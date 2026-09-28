#!/usr/bin/env bash
#
# S113: PROBCUT. At a non-PV node out of check, below the root, at a remaining
# depth of at least 8 and with beta outside the mate band, every capture and
# promotion the exchange evaluation does not write off is tried against a bar
# `probBeta = beta + ProbCutMargin`: a zero-window quiescence first, then a
# zero-window search of the capture's reply at `depth - 5` -- four plies of the
# node's own at depth 8, counting the capture. One that clears the bar ends the
# node with its fail-soft score, stored as a lower bound at that node-level
# depth.
#
#   nohup adocs/data/S113_sprt.sh > .tuning/sprt_s113.log 2>&1 &
#
# **THE CANDIDATE IS ONE BLOCK.** In `negamax_at`, after the null move and
# before S097's singular block: the guards (`ProbCut` != 0, `!is_pv`, not in
# check, `ply > 0`, `excluded_move == 0`, `depth >= ProbCutMinDepth`,
# `depth - ProbCutDepthOffset >= 1`, `beta > -MATE_MIN`, `probBeta <
# MATE_MIN`); the table skip (an entry at least as deep as the shallow search,
# upper bound or exact, scoring under probBeta, read from the copies S097 took
# before the null move recursed); its own capture list filtered by
# `capture_cannot_lose` then `see_ge(0)` and ordered by `capture_score`; per
# move the preliminary quiescence and, where it holds, the shallow search, the
# child labelled as the null move's child is; aborts honoured after each. Four
# rows in `src/search_params.hpp`: `ProbCut` 1, the switch, `ProbCutMargin`
# 49, `ProbCutDepthOffset` 5, `ProbCutMinDepth` 8.
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section, written from published prose, the wiki and Jiang and
# Buro's paper. No other project's code was opened and no constant of one is
# behind any value here (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), stated at their site. `ProbCutMinDepth` 8 and
# `ProbCutDepthOffset` 5 are (a): the depth pair (S, H) = (4, 8) of Jiang and
# Buro, ACG 10, 2003, Figure 2 (https://skatgame.net/mburo/ps/chessmpc.pdf),
# the offset being 5 because the child is searched after the capture, so the
# node-level shallow search is `depth - 4`. The first landing had the offset
# at 4 -- a (5, 8) pair against a (4, 8) fit, t about 1.23 while labelled 1.0
# -- and the coordinator's fast check sent it back.
# `ProbCutMargin` 49 is (b) at an (a) threshold: the paper's regression
# `v = a*v' + b` run over chesso's own positions at `go depth 4` against
# `go depth 8`, mate scores and |v'| > 3 pawns dropped, seeded at
# `round(t * sigma)` with the paper's t = 1.0. The step's 300-position pick
# left 180 survivors, under its own floor of 200, and was widened as the step
# says: 600 positions, 375 kept, a 0.951, b -1.05, sigma 48.98
# (`adocs/data/S113_probcut_fit.tsv`). All three are fitted at S127.
#
# THE PRIOR, recorded before the run (DEC-019), direction only. Reported +6.36
# and +6.65 at its introduction in one engine at about 2950; another engine
# removed it as "currently worthless" at about +1 to +5 and reintroduced it
# three days later at about zero short and small long; a third, near 3300,
# never carried it. The origin paper's own chess result puts plain ProbCut
# beside null move at "no better nor worse". A zero is an ordinary outcome.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES on the tree the step lands
# on, 2026-09-28: the step's working tree rebased onto `308b388`, against a
# parent built from `308b388` in a throwaway worktree. Every figure below was
# re-taken there; the `1e9827d` figures this block carried before the rebase
# are in the step file's history sections and are not this tree's.
#
#   THE FIRE RATE, before a game was booked: over the eight bench positions at
#   depth 12, 240525 nodes reach the block's site, 1345 pass every guard
#   (0.56 %), 213 of those are answered by the table skip, 1450 captures pay
#   the preliminary, 806 the shallow search, and **778 nodes are cut -- 57.8 %
#   of those entered**, 1 of them on a mate score a shallow search returned -- no proof, the step
#   file's section 5 kept that on purpose. At
#   depth 14 (`bench`): 510798 / 3218 / 712 / 3617 / 1734 / 1660, 19 on a
#   mate score. `.tuning/coord/S113_fire_rebased.txt`; the instrumented copy
#   benches the candidate's own 1700466 at depth 12 and 3591364 at 14, so its
#   counters are write-only.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). No value of the three
#   settings is off -- the margin's top still pays the preliminary at every
#   eligible node -- so the switch is `ProbCut`, the precedent `SeExtend`,
#   `RfpTtEstimate` and `QsFutility` set. The tune build at `ProbCut` 0 prints
#   `bench` 4649650 with all eight `bestmove` replies identical to the
#   parent's (c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), `bench 12` the
#   parent's 1981759, and `tools/search_bench.py` reproduces it at depth 9
#   (53598 / 80389 / 25691, c3d5 e2a6 d7c8q) and depth 12 (154098 / 472358 /
#   107876, c3d5 e2a6 d7c8q). `.tuning/coord/S113_identity_rebased.log`.
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` 4649650 -> 3591364,
#   -22.76 %, all eight replies unchanged; `bench 12` 1981759 -> 1700466,
#   replies unchanged. `search_bench` depth 9: 53598 -> 34773, 80389 ->
#   71512, 25691 -> 25413; depth 12: 154098 -> 110496, 472358 -> 244771,
#   107876 -> 117798; every best move unchanged. The counts move by
#   construction, so INV-6 does not discharge this change and nothing here is
#   Elo (DEC-019).
#
# THE REFERENCE IS THE TREE S112's VERDICT LEFT. This step was built on
# `1e9827d` (`3cede8c` after DEC-235's rewrite, `src/` identical) while S112's
# SPRT measured per-move futility in quiescence against it -- the quiescence
# the preliminary calls. S112 read no verdict at the 40000-game cap, a zero,
# and its code stayed on the pre-registration's one exception (DEC-236). So
# the reference is the tree with per-move futility in it, `308b388`, and S113
# was rebased onto it on 2026-09-28: the off-value identity above was
# re-proved against that tree -- `ProbCut` 0 benches its own total with all
# eight replies and reproduces its `search_bench` at 9 and 12 -- and the
# candidate numbers and the fire rate were re-taken there. **Three mined mate
# rows moved with the combination** and each was re-derived by its own script
# under DEC-233, the old rows quoted in their GOLDEN blocks: S097's multicut
# row (`adocs/data/S113_remine_s097.log`), S112's capture-mate row 4 (depth 9
# -> 10, `adocs/data/S230_mine_r01_row.py depths`, all seven sweeps) and this
# step's own ProbCut row (`adocs/data/S113_mine.log`). Each change alone left
# the case green on its own tree's rows.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the rule is proposed as a gain.
# DEC-063: the pair straddles the expected effect -- the record's introduction
# sits above elo1 and its removal and its absence sit at zero.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound (`adocs/testing_strategy.md`
# section 1.1), at **2110 games an hour** from `.moltke.local.md`: **19.8 h**
# and **12.1 h**. A night, scheduled (DEC-155). The throughput is read off the
# banner and recorded rather than assumed.
#
# REGIME. `adocs/data/S237_sprt.sh`'s: 8+0.08, Hash 16, one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 (DEC-050), whatever
# governor the machine is under, recorded by the run and not set (DEC-195).
# **No harness change since the last fixed-rounds A/A**; if fastchess, the
# book, the adjudication or the machine moves before this starts, the A/A of
# 1000 fixed rounds comes first and is read with `adocs/data/S105_pairs.py`
# (DEC-143).
#
# WHERE THE OUTPUT LANDS. `OUT` is set in the exec line below, under
# `.tuning/`, which since DEC-235 is also `fastchess.sh`'s own default
# (`outdir`, `$repo/.tuning/sprt_<tag>_<stamp>`); it was under `/tmp`, which
# this machine wipes at boot, when this file was first written. Set here so
# the run's directory is named for the step.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# carried from `adocs/data/S112_sprt.sh`'s block as it stood on 2026-09-27 and
# re-read at the rebase on 2026-09-28: **no defect reachable in ordinary play,
# on the UCI surface, or able to move a reported score, move or line is
# open.** What is open is test-side, and none of it is touched here:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding.
#   2. The three S097 named on 2026-09-21: S231 phase one's node-budget label,
#      `adocs/data/S192_node_budget.py` reporting OUTSIDE for a band it had
#      just derived, and "a reduced move that beats alpha is searched again"
#      pinning depth 6 where `adocs/data/S231_research_witness.py` answers 4.
#   3. **`test_mate_carry`'s budgets are not the DEC-156 rule's answer**
#      (S238's item 5, with the coordinator's ruling of 2026-09-26); green here
#      at the budgets as they stand, and S202's class of short mate lines is
#      read beside the verdict as an `Incomplete mating PV` count per side --
#      **S112's run read 40 against 25** for that class (candidate against
#      reference, `adocs/data/S112_sprt_pairs.txt`), which is the count this
#      run's is read beside. Re-timed on the idle machine at the rebase: see
#      the step file.
#   4. **This step's own golden belongs to its tree** (DEC-233): the mined
#      ProbCut row of "pruning does not hide a forced mate" is a property of
#      the tree it was mined on -- re-derived when the offset moved from 4 to
#      5, and again at the rebase onto `308b388`, where the fast check's row
#      stopped separating (`adocs/data/S113_mine.log`). The rebase moved two
#      goldens that are not this step's, both re-derived by their own scripts
#      with the old rows quoted for an H0 to restore: S097's multicut row and
#      S112's capture-mate row 4 (THE REFERENCE paragraph above).
#   5. **S112's block's items 4 and 5 are resolved**: its capture-mate rows
#      are the reference tree's now that its code stayed (DEC-236) -- the
#      rows this step re-derived from -- and S238's verdict had closed its
#      items before S112 was pinned. S112's verdict adds no open finding.
#   6. **The preliminary enters quiescence at a child `negamax_at` never
#      screened** (the coordinator's fast check, 2026-09-28): a ProbCut
#      capture that leaves insufficient material is scored by quiescence on
#      the material left and stored at `TT_DEPTH_QS`, which S210's comment
#      in `quiescence` said could not happen. No cut is wrong -- the shallow
#      `negamax_at` that follows returns `DRAW_SCORE` before the cut test,
#      and `negamax_at` answers a dead node before it probes the table -- so
#      the cost is one wasted preliminary and a stale entry no main-search
#      node reads. Filler behind this step (DEC-171), not touched here.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> the block gains at least 5 nElo at its seeds. **The three
#                   settings stay at their seeds** -- two paper values and one
#                   derivation over chesso's own positions, none a fit -- and
#                   S127 fits them together with the rest of the set; the
#                   fit, the seed and the paper agree on the pair (4, 8) at
#                   t = 1.0, and the (5, 8) spread, sigma 39.83 over the 300
#                   set, is on record for a sweep. S114 then takes this tree
#                   as its baseline. The stopping run's Elo is upward-biased
#                   and is not the effect size.
#
#   H0, interval -> **a loss, and the revert is one default.** `ProbCut` to 0,
#   wholly         the off value proved above, so the engine is the reference
#   below zero     tree's to the node the moment it moves -- and **the code
#                  leaves with it**: the four rows, the block, the probe fields,
#                  the eleven cases, the mined mate row and the fifteen mutants
#                  come out in the same step, and the two rows of other steps
#                  the rebase re-derived -- S097's multicut row and S112's
#                  capture row 4 -- go back byte for byte to the ones quoted in
#                  their GOLDEN blocks, behaviour-neutral at the reverted
#                  default and discharged by INV-6. No reason to keep
#                  the code is foreseen; if one is given, the completion stamp
#                  states it.
#
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero with
#   or an          the games played and the interval, and read it by the row
#   interval       above: the switch goes to 0 with the code behind it on the
#   reaching       same terms. No follow-up run and no second pair (DEC-063);
#   above zero     another margin or depth pair is a second experiment, which
#                  S127's fit is the place for.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands ProbCut at its seeds; REF is its parent, the
# tree S112's verdict left (`308b388`'s engine), onto which this step was
# rebased. Both are pinned after the landing commit exists, by editing the
# defaults below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-029ff61}"
CAND="${CAND:-3d97737}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S113_sprt.sh before running (CAND is ProbCut's landing" \
         "commit, REF its parent, the tree S112's verdict left)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s113_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
