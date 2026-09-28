#!/usr/bin/env bash
#
# S131: QUIET QUEEN PROMOTIONS IN QUIESCENCE. Out of check, quiescence searched
# captures only: generate_captures() emits every promotion, and the filter
# dropped the ones that take nothing. The candidate keeps a promotion to a
# queen that takes nothing as well; the three underpromotions that take
# nothing stay out, and a promotion that takes something is searched as the
# capture it always was.
#
#   nohup adocs/data/S131_sprt.sh > .tuning/sprt_s131.log 2>&1 &
#
# **THE CANDIDATE IS ONE CONDITION.** In `quiescence`'s filter loop the drop
# `!in_check && !MOVE_CAPTURE(m)` gains `&& !(QS_QUEEN_PROMOTIONS != 0 &&
# MOVE_PROMOTED(m) == TO_QUEEN)`, and the filter comment and the generation
# comment above it are rewritten to say what is searched now. One row in
# `src/search_params.hpp`: `QsQueenPromotions` 1, the switch. Nothing else in
# `src/`: S112's futility block already exempts the admitted move as a
# promotion; S015's exchange gate prices it through see_ge(), which it reaches
# because capture_cannot_lose() answers false on an empty target -- a
# capturing promotion is a pawn's capture, which capture_cannot_lose() passes,
# and never reaches see_ge() -- and capture_score() orders it at the table's
# existing empty-victim row, -100.
# The generator and INV-3's partition are untouched; the in-check path is
# untouched.
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section, written from published prose and the wiki. No other
# project's code was opened and no constant of one is behind anything here
# (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134): none. The rule has no number in it. The class admitted
# is the wiki's (https://www.chessprogramming.org/Promotions: "In quiescence
# search most programs only consider queening") and the order is a row the
# ordering table already had.
#
# THE PRIOR, recorded before the run (DEC-019), direction only. **The isolated
# delta of adding quiet queen promotions to a captures-only quiescence is
# untraced in the public record**: every surveyed engine carried them in its
# noisy set from the start, so what the record prices is only the
# underpromotion margin -- reported at statistically zero in both directions
# by one engine (+4.90 +/- 4.61 and +1.05 +/- 2.40 for moving them out,
# 0.02 +/- 1.50 and 2.25 +/- 2.57 for moving them back in), and passed as a
# simplification by another. The direction is this engine's own comment,
# written at S001 over the filter ("Letting them through is very likely an
# improvement"), and the practice of every surveyed engine. A zero is an
# ordinary outcome, and the census below predicts one.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES on the tree it lands on,
# 2026-09-28: the step rebased onto `3c7cf84`, S113's tree with ProbCut kept
# on its H1, against a parent built from `3c7cf84` in a throwaway worktree.
# Counts only, on the idle machine. The step was built on `029ff61` and its
# figures there are in the step file; they are replaced here, not kept
# (DEC-233).
#
#   THE ADMISSION RATE, before a game was booked (the step's section 6): over
#   the eight bench positions at depth 12, 795143 moves reach quiescence's
#   filter loop out of check and 6735 of them are promotions to a queen that
#   take nothing, **0.85 %**, at 6709 of the 248994 nodes that reach the
#   loop. S015's exchange gate declines 6499 of the 6735 (96.5 %), 236 are
#   kept, and **143 are searched** -- 0.018 % of the loop's moves and 0.023 %
#   of the 626337 quiescence nodes -- of which 47 raise the node's best and
#   79 cut off. At depth 14 (`bench`): 1618764 moves, 12616 admitted
#   (0.78 %), 12272 declined, 344 kept, 203 searched, 69 raising a best,
#   108 cutting off. The three `search_bench` positions at depth 12: 393796,
#   6446 (1.64 %), 6212, 234, 142, 46, 79.
#   `.tuning/coord/S131_admission_rebased.txt`; the instrumented copy benches
#   the candidate's own 1694808 at depth 12 and 3429473 at 14, so its
#   counters are write-only. **Near zero by DEC-079's measure** -- S094's
#   reach was 0.05 % and its verdict was the zero that predicted -- and lower
#   than on `029ff61`, where 235 were searched, 0.030 % of that tree's
#   quiescence nodes: the census predicts a zero before the match is bought,
#   and the match is booked anyway because the step's accepts is a verdict,
#   recorded whatever it is.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). The rule has no setting, so
#   the switch is `QsQueenPromotions`, the precedent `QsFutility` and `ProbCut`
#   set. The tune build at 0 prints `bench` 3591364 with all eight `bestmove`
#   replies identical to the parent's (c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6
#   e5e6), `bench 12` the parent's 1700466 with its replies, and
#   `tools/search_bench.py` reproduces the parent at depth 9 (34773 / 71512 /
#   25413, c3d5 e2a6 d7c8q) and depth 12 (110496 / 244771 / 117798, c3d5
#   e2a6 d7c8q). `.tuning/coord/S131_identity_rebased.log`.
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` 3591364 -> 3429473,
#   -4.51 %, and **one of the eight replies moves**: the second position's,
#   kiwipete's, d5e6 -> e2a6, where on `029ff61` all eight held; `bench 12`
#   1700466 -> 1694808, replies unchanged. `search_bench` depth 9: 34773 ->
#   48304, 71512 -> 71580, 25413 -> 25413; depth 12: 110496 -> 104784,
#   244771 -> 244824, 117798 -> 117798; every best move unchanged, and the
#   tactical position's count is the parent's at both depths. The counts move
#   by construction, so INV-6 does not discharge this change and nothing here
#   is Elo (DEC-019).
#
# THE REFERENCE IS THE TREE S113's VERDICT LEFT. This step was built on
# `029ff61` -- S112's zero with per-move futility kept (DEC-236), and S113's
# REF -- while S113's SPRT measured ProbCut against that tree. ProbCut's
# preliminary calls `quiescence` directly, so S113's outcome decided one of
# the callers this step's class is reached from, and the pair is taken on the
# tree that outcome left. **S113 accepted H1 on 2026-09-28** (`Elo 7.70 +/-
# 5.41`, `nElo 9.87 +/- 6.93`, 9654 games, `adocs/data/S113_sprt.log`) and
# ProbCut stayed, so the reference is `3c7cf84`, the tree with ProbCut, and
# the step was rebased onto it before pinning: the off-value identity was
# re-proved there -- `QsQueenPromotions` 0 benches the new parent's own
# 3591364 with all eight replies and reproduces its `search_bench` at 9 and
# 12 -- and the candidate's numbers and the census were re-taken there, which
# are the figures above. No golden went red with the combination (open
# finding 7); one stopped separating the mutant it was mined for, and was
# re-derived by its own script in the guard mode DEC-238 gave it (open
# finding 9).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the change is proposed as a
# gain. DEC-063 asks a pair to straddle the expected effect, and stated
# honestly this step has no number to straddle: the isolated delta is
# untraced in the record, and the census above expects it small. The pair is
# the gainer's because the direction is -- the engine's own comment and every
# surveyed engine's move set put the class inside quiescence -- and not
# because any figure says 5 nElo is within reach. A truth near zero walks
# between the bounds without reaching either (DEC-063's case, S112's 40000
# games), and the third row below is written for exactly that.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound (`adocs/testing_strategy.md`
# section 1.1), at **2110 games an hour** from `.moltke.local.md`: **19.8 h**
# and **12.1 h**. A night, scheduled (DEC-155); a walk to fastchess's
# 40000-game cap is the midpoint's figure, about 19 h. The throughput is read
# off the banner and recorded rather than assumed.
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
# (`outdir`, `$repo/.tuning/sprt_<tag>_<stamp>`); set here so the run's
# directory is named for the step.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# carried by id from `adocs/data/S113_sprt.sh`'s block as it stood on
# 2026-09-28 and re-read at the rebase, after S113's H1: **no defect reachable in ordinary play, on the UCI surface, or
# able to move a reported score, move or line is open.** What is open is
# test-side, and none of it is touched here:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding.
#   2. The three S097 named on 2026-09-21: S231 phase one's node-budget label,
#      `adocs/data/S192_node_budget.py` reporting OUTSIDE for a band it had
#      just derived, and "a reduced move that beats alpha is searched again"
#      pinning depth 6 where `adocs/data/S231_research_witness.py` answers 4.
#   3. **`test_mate_carry`'s budgets are not the DEC-156 rule's answer**
#      (S238's item 5, with the coordinator's ruling of 2026-09-26); green on
#      this step's tree at the budgets as they stand, and S202's class of
#      short mate lines is read beside the verdict as an `Incomplete mating
#      PV` count per side -- S112's run read 40 against 25 (candidate against
#      reference, `adocs/data/S112_sprt_pairs.txt`) and S113's 0 against 0
#      (`adocs/data/S113_sprt_pairs.txt`), and S113's is the count this run's
#      is read beside.
#   4. **S113's own golden belongs to its tree** (DEC-233): the mined ProbCut
#      row of "pruning does not hide a forced mate", and the two goldens of
#      other steps its rebase re-derived (S097's multicut row and S112's
#      capture-mate row 4). S113 read H1, so they are this run's reference
#      tree's rows; re-read at the rebase, they are green in both builds with
#      this step in and none went red, but the multicut row stopped
#      separating the mutant it was mined for and was re-derived on this
#      step's tree (item 9); an H0 here puts S113's row back byte for byte.
#   5. **S112's block's items 4 and 5 are resolved** (DEC-236), as S113's
#      block states; S112's verdict adds no open finding.
#   6. **ProbCut's preliminary enters quiescence at a child `negamax_at`
#      never screened** (S113's fast check, 2026-09-28): a capture that
#      leaves insufficient material is scored by quiescence and stored at
#      `TT_DEPTH_QS`; no cut is wrong. A filler (DEC-171), open now that
#      S113's code stayed, and not touched here -- this step's class never
#      leaves a dead board, since the queen it makes is on it.
#   7. **This step's own goldens** (DEC-233): **none, and no golden went red
#      with this step in, on the tree it was built on or on the one it lands
#      on.** "pruning does not hide a forced mate" and `test_mate_carry` are
#      green in both builds on `029ff61` and again on `3c7cf84` with this
#      step in, so no GOLDEN block holds an old row of this step's own; the
#      one golden this step re-derived is S097's multicut row (item 9).
#      `test_mate_carry` read 118 s against its 120 s ceiling beside S113's
#      match, a load reading; alone on the idle machine at the rebase it read
#      55.66 s in the Release build and 57.21 s in the tune build.
#
#   8. **S242, `test_engine`'s capture flake**
#      (`adocs/plan_todo/S242_test_engine_capture_flake.md`, found at this
#      step's mutation baseline on 2026-09-28): the two timing-dependent
#      `deepest_completed_depth(capture) < 1` CHECKs of "the first iteration
#      honours stop and the hard timer" need a stop to land inside a depth-1
#      iteration of about 14 ms and fail under a match's load, and a doctest
#      failure inside a `stdout_capture_t` scope is reported with no text,
#      because the capture swaps `std::cout`'s buffer, which is where doctest
#      writes. Test-side and not reachable in ordinary play, on the UCI surface
#      or in a reported score, move or line: DEC-171's filler class, not
#      touched here.
#
#   9. **E21's coverage, resolved by DEC-238** (found at the rebase,
#      2026-09-28, by a targeted mutation run: `E21_multicut_mate_band_gate_dropped`
#      survived, 0 of 41 red). S097's multicut row separated it on `3c7cf84`
#      and not with this step in, and the miner's default mode found no
#      replacement on the rebased tree: its firing witness asked the shipped
#      build, where the guard is what keeps the rule still. DEC-238 takes a
#      guard row's witness on the mutant's build, and the miner's `--mode
#      guard` took `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42` at
#      depth 14, mate in 6, which the case now carries, red under E21 and
#      green shipped (`adocs/data/S131_remine_s097.log`). On an H0 the row
#      goes back to S113's. What stays open is the corpus dependence, and
#      S243 (item 10) is the filler that ends it.
#
#  10. **S243, a direct guard test for the multicut's mate band**
#      (`adocs/plan_todo/S243_multicut_mate_band_guard_test.md`, created with
#      DEC-238 on 2026-09-28): a test that the verification search's
#      mate-range value is never handed back as the node's, with E21 killed
#      by it, so the guard's coverage no longer depends on a mined row.
#      Test-side and not reachable in ordinary play, on the UCI surface or in
#      a reported score, move or line: DEC-171's filler class, not touched
#      here.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> searching quiet queen promotions in quiescence gains at
#                   least 5 nElo. **The switch stays at 1** and the class
#                   stays queen-only: the underpromotion margin is a second
#                   experiment the record already prices at zero, and the
#                   order is the table's row, which nothing here fitted. The
#                   stopping run's Elo is upward-biased and is not the effect
#                   size.
#
#   H0, interval -> **a loss, and the revert is one default.**
#   wholly         `QsQueenPromotions` to 0, the off value proved above, so
#   below zero     the engine is the reference tree's to the node the moment
#                  it moves -- and **the code leaves with it**: the row, the
#                  condition, the six cases and the ten mutants come out in
#                  the same step, and the filter comment goes back to saying
#                  the class is dropped, with the verdict in place of S001's
#                  "very likely an improvement"; any golden this step
#                  re-derived goes back byte for byte to the row its GOLDEN
#                  block quotes, behaviour-neutral at the reverted default and
#                  discharged by INV-6. S112's "a capturing promotion below
#                  the threshold is searched" keeps what the fast check added
#                  -- its premise and its settled-move assertion hold on the
#                  tree without this step too. No reason to keep the code is
#                  foreseen; if one is given, the completion stamp states it.
#
#   No verdict   -> a stalled walk is a zero. Terminate it at fastchess's cap,
#   or an          record the zero with the games played and the interval, and
#   interval       read it by the row above: the switch goes to 0 with the
#   reaching       code behind it on the same terms, unless a reason is stated
#   above zero     in the completion stamp. No follow-up run and no second
#                  pair (DEC-063); underpromotions, another placement or a
#                  wider pair is a second experiment.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands S131 on the tree S113's H1 left, `3c7cf84`'s
# engine; REF is its parent, that tree. Both are pinned after the landing commit exists, by
# editing the defaults below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S131_sprt.sh before running (CAND is S131's landing" \
         "commit, REF its parent, the tree S113's H1 left)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s131_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
