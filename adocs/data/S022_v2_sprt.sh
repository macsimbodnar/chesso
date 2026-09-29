#!/usr/bin/env bash
#
# S022 VERDICT 2: THE NODE-LEVEL DELTA EARLY-OUT IN QUIESCENCE, ADDED. Out of
# check and before anything is generated, quiescence asks whether even the
# largest gain any move could bring reaches alpha -- S112's futility base (the
# stand pat after S130's substitution, plus `QsFutilityMargin`), plus the
# queen's price in S112's victim table `qs_futility_value`, plus that table's
# queen less its pawn when a pawn of the side to move stands on its seventh
# rank -- and where it cannot, the node ends ungenerated. Its fail-soft value
# is that ceiling, stored as an upper bound. Verdict 1 of the same step, the
# deletion of S015's exchange gate, read H0 before this was measured, so the
# gate is on both sides of this pair.
#
#   nohup adocs/data/S022_v2_sprt.sh > .tuning/sprt_s022_v2.log 2>&1 &
#
# **THE CANDIDATE IS ONE BLOCK, ITS SWITCH AND ITS THRESHOLD.** In
# `quiescence`, between the depth cap and the generation call, one block
# behind `QsDeltaEarlyOut`, which ships at **1**, and not asked at
# `game_phase() <= QsDeltaPhaseMin`, seed 0. At `QsDeltaEarlyOut` 0 the engine
# is the tree before this verdict node for node (below). Nothing else in
# `src/` moves: S112's block, S131's condition, S015's gate, ProbCut's own
# filter in `negamax_at` (S113), S091's main-search pruning and the generator
# are untouched. The in-check path is untouched at every value: the block is
# never asked in check.
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section (section 2's early-out bullets, section 4's constants),
# written from the wiki and published prose. No other project's code was
# opened and no constant of one is behind anything here (DEC-016, DEC-104,
# DEC-105, DEC-134).
#
# THE SEEDS (DEC-134). **No new margin** (section 4): the ceiling reuses
# S112's `QsFutilityMargin` (188, form (a), the wiki's ~200 cp read in chesso's
# scale) and S112's victim table read at the queen (900, form (b), chesso's
# exchange scale); the allowance, 900 - 100 = 800, is the same table's and
# agrees with S131's exchange arithmetic for a promotion that takes nothing.
# **`QsDeltaPhaseMin` 0 is form (b), chesso's own phase scale**: pawn endgames
# only, the boundary the null-move zugzwang guard in `negamax_at` already keys
# on (`game_phase() > 0`); the wiki names the late-endgame disable and no
# threshold. Range 0 to 24; at 24 the early-out is never asked (the phase never
# exceeds 24), which is proved below to be the parent's tree; the floor still
# disables pawn endgames, so "never disable" is outside the range. A seed, for
# S127 to fit if the early-out stays.
#
# VERDICT 1 READ H0, AND THAT FIXES WHAT THIS PAIR IS. `a97bc1a` against
# `d446783`, `{-5, 0}` nElo, 2026-09-29: **H0 at 900 games, `Elo -65.22 +/-
# 17.66`, `nElo -85.89 +/- 22.70`, LLR -2.96, 0 forfeits** (the coordinator's
# status entry; `.tuning/coord/S022_v1_sprt.log`). S015's gate stays, and the
# revert puts `QsSeeGate` to 1 and takes its code out. So in the step file's
# section 3 naming this verdict is **F+S+D against F+S** -- futility, S015's
# gate and the early-out against futility and the gate -- and not F+D against
# F. It was built on `05ab85d`, the tree with the gate deleted, as its brief
# said, and rebased onto `39841ed`, the revert -- whose `src/`, `tests/`,
# `tools/` and `MANUAL.md` are `d446783`'s but for a five-line comment at the
# gate -- where every number below that decides was re-taken; the few from
# `05ab85d` say so. "The landing configuration" below is that tree.
#
# THE PRIOR, recorded before the run (DEC-019), direction only and never a
# forecast. **The record is deletion once per-move futility exists**: one
# engine gained by deleting delta pruning after per-move futility, +1.87 +/-
# 3.51 at 8+0.08 and +6.74 +/- 5.26 at 40+0.4 at its simplification bounds
# (Weiss #455), and another measured delta added to a SEE-gated tree negative
# (Lynx #731) -- **which is this pair's configuration exactly**, verdict 1
# having kept the gate. And verdict 1 itself: the gate, which prunes the losing
# captures the early-out's ceiling would otherwise have to cover, measured
# -85.89 nElo for its absence. No figure here is Elo for this engine.
#
# DEC-063'S EXPECTATION, stated before the run: **about zero, or negative.**
# The pair is the gainer pair because an add ships only on evidence of gain
# (section 6); a true zero sits on H0, where the walk terminates. The crawl
# case is a truth near +2.5, between the bounds, and the third row below is
# written for it.
#
# DEC-236: this verdict does not decide S112's per-move futility. Futility is
# on both sides of this pair, as it was of verdict 1's, so no reading of either
# verdict says the rule is worthless; its own worth is S127's fit.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-29, niced. Counts only.
#
#   THE REACH, stated as reach (DEC-239): how many out-of-check quiescence
#   nodes the early-out ends before generation, out of how many, and what a
#   generation there would have held. From an instrumented copy of the
#   candidate with write-only counters (`.tuning/coord/S022_v2_census/`,
#   `apply_census.py`), its tune build at `QsSeeGate` 1 -- the tree verdict 1's
#   H0 leaves, `d446783`'s engine node for node -- and at 0, each at
#   `QsDeltaEarlyOut` 1 and 0; at every one of the eight configurations its
#   bench and `search_bench` totals, replies and best moves are the
#   uninstrumented tune build's, and the real block's return count equals the
#   census's own recomputed firing count (`census_run.log`, summary
#   `S022_v2_census_summary.txt`).
#
#   Over the eight bench positions at depth 12, on the landing configuration:
#   **the early-out ends 31200 of the 246317 out-of-check nodes that reach
#   generation, 12.67 %**, 18 of them with the promotion allowance in the
#   ceiling. Of the 31200, a generation would have kept **no move at all past
#   the filter's drop at 22569 (72.3 %)**; at 6340 every move it kept is one
#   S112 skips anyway (futile, legal, no check), and 8247 hold at least one
#   such capture; **2291 (7.3 %) hold a move S112 searches** -- 2273 a capture
#   that gives check, 18 a promotion -- and **804 (2.6 %) a move the exchange
#   gate lets through as well**, the nodes where the early-out removes a move
#   the tree without it searches: 991 moves, against the 21255 S112 would have
#   skipped. On the tree without the early-out the same condition holds at
#   31778 of 248994 such nodes (12.76 %), 967 of them with a move the tree
#   searches. The three `search_bench` positions at depth 12: 4166 of 103227,
#   4.04 %, 477 with a move the tree searches. **The phase disable touches
#   nothing here**: 210 nodes are at phase 0 and the ceiling fires at none of
#   them. This is how often the class occurs, not what it is worth (DEC-239):
#   S131's census read 0.85 % and its match H1 at +17.84.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). A rule with a threshold and
#   no margin of its own has a switch, `QsDeltaEarlyOut`, in `QsFutility`'s,
#   `ProbCut`'s and `QsQueenPromotions`' shape: its 0 is the parent. **On the
#   worktree's tree** (`05ab85d`, the gate deleted), the tune build at 0 prints
#   `bench` 6049266 with all eight replies a build of `05ab85d` gives (c3d5 e2a6
#   d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), `bench 12` its 2393854, and
#   `tools/search_bench.py` its counts at depth 9 (39854 / 249885 / 33736) and
#   12 (97921 / 567770 / 125308), best moves c3d5 e2a6 d7c8q
#   (`.tuning/coord/S022_v2_logs/identity.log`). **On the landing
#   configuration**, the worktree rebased onto `39841ed`: the tune build at 0
#   prints `bench` **3429473** with the eight replies of a build of `39841ed`
#   (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), `bench 12` 1694808,
#   `search_bench` 48304 / 71580 / 25413 at 9 and 104784 / 244824 / 117798 at
#   12, best c3d5 e2a6 d7c8q (`identity_rebased.log`). `QsDeltaPhaseMin` 24
#   with the switch at 1 prints the same totals on both trees. **The
#   coordinator re-proves this on the landing's parent before pinning.**
#
#   AND THE SHIPPED TREE AGAINST THE PARENT, landing configuration: `bench`
#   3429473 -> **3656950**, +6.63 %, **seven of the eight replies unchanged,
#   the second position's (kiwipete) e2a6 -> d5e6**; `bench 12` 1694808 ->
#   1702684, the same one reply moving. `search_bench` depth 9: 48304 ->
#   70913, 71580 -> 71220, 25413 -> 24293, best moves unchanged; depth 12:
#   104784 -> 128099, 244824 -> 258594, 117798 -> 83277, **kiwipete's best
#   move e2a6 -> d5e6**, the other two unchanged. On the worktree's tree
#   (`05ab85d`): `bench` 6049266 -> 7722782, replies two and five moving;
#   `search_bench` 12 97921 -> 97058, 567770 -> 646712, 125308 -> 183598. The
#   counts move by construction, so INV-6 does not discharge this change and
#   nothing here is Elo (DEC-019).
#
# THE REFERENCE IS THE TREE VERDICT 1 LEAVES. `REF` is the commit this
# verdict's landing sits on: S246 on `39841ed`, the revert of verdict 1's H0 --
# `QsSeeGate` to 1 and its code out, the four goldens restored -- with S242's
# and S243's missing test edits, tests and documents only, so its engine is
# `39841ed`'s, `d446783`'s node for node. `CAND` is this verdict's landing on
# it. The candidate was built on `05ab85d` while verdict 1's SPRT ran and
# rebased onto `39841ed`, where the off-value identity above was re-proved;
# the coordinator re-proves it on the landing's parent before the pair is
# pinned. **The promotion allowance is measured with
# S131's class in the tree**: quiet queen promotions are searched on both sides
# of the pair, and the allowance is what keeps the early-out from ending the
# nodes where one is on offer.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`, 20000 rounds -- the gainer pair, section 6's
# pre-registration for this verdict: an add ships only on evidence of gain;
# accepted at "merely not a regression" it would ship complexity with no
# measured value. With the expectation at about zero the truth sits on H0,
# where the walk terminates (DEC-063).
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound
# (`adocs/testing_strategy.md` section 1.1), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. The midpoint's figure is past
# fastchess's 40000-game cap, about 19.0 h, and a walk that reaches the cap is
# read as no verdict by the row below. A night, scheduled (DEC-155). The
# throughput is read off the banner and recorded rather than assumed.
#
# REGIME. `adocs/data/S022_v1_sprt.sh`'s: 8+0.08, Hash 16, one thread a side,
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
# carried by id from `adocs/data/S022_v1_sprt.sh`'s block as it stood on
# 2026-09-29, after verdict 1's H0 and S246, and re-read on `58585f8`, the tree
# this lands on: **no defect reachable in ordinary play, on the UCI surface,
# or able to move a reported score, move or line is open.** Items 1 to 12 are
# verdict 1's, 15 is new since; 13 and 14 are this verdict's. S244 and S245
# are open fillers, S246 is done:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding.
#   2. The three S097 named on 2026-09-21: S231 phase one's node-budget label,
#      `adocs/data/S192_node_budget.py` reporting OUTSIDE for a band it had
#      just derived, and "a reduced move that beats alpha is searched again"
#      pinning depth 6 where `adocs/data/S231_research_witness.py` answers 4.
#   3. **`test_mate_carry`'s budgets are not the DEC-156 rule's answer**
#      (S238's item 5). S202's class of short mate lines is read beside the
#      verdict as an `Incomplete mating PV` count per side, as S131's run and
#      verdict 1's were; this candidate moves no ceiling (item 13).
#   4. **S113's goldens belong to its tree** (DEC-233).
#   5. **S112's block's items 4 and 5 are resolved** (DEC-236).
#   6. **ProbCut's preliminary enters quiescence at a child `negamax_at`
#      never screened** (S244 is its filler): no cut is wrong. The early-out
#      meets that preliminary like any caller: its child is searched at the
#      zero window `(-probBeta, -probBeta + 1)`, so where the child's own
#      ceiling is at or below `-probBeta` the child ends ungenerated and the
#      preliminary holds; deeper in the preliminary's quiescence it moves
#      values either way, as it does anywhere in quiescence. The preliminary
#      only decides whether the shallow search is paid, and the shallow search
#      -- which alone decides a ProbCut cut -- meets the early-out at its own
#      leaves like any search. Nothing structural changes: the class is still
#      no wrong cut, and a dead child is still a wasted preliminary.
#   7. **S131's own goldens: none.**
#   8. **S242, `test_engine`'s capture flake: closed** (`aef0257`, DEC-240),
#      its DEC-240 form in the tree since S246 (item 15).
#   9. **E21's coverage: two killers** -- S243's direct case, with the
#      returned-score CHECK S246 landed, and the mined multicut row. On this
#      candidate S131's row stopped separating E21 -- `--only E21` on a
#      fixture of it listed S243's case alone, the sign `DEV_MANUAL.md`'s row
#      names -- and the miner's guard mode re-derived the row (item 13): S097's
#      own row came back, and a second `--only E21` kills it by both again.
#  10. **S243, a direct guard test for the multicut's mate band: closed**
#      (`6eb2674`), its second-round CHECK in the tree since S246 (item 15).
#  11. **Verdict 1's own goldens**: the four it re-derived go back byte for
#      byte with the gate in the revert (DEC-241's ceiling to 0 by dropping
#      its grid from the `--ceilings` command). Not this verdict's.
#  12. **S245, a filler for five stale things** (`adocs/plan_todo/S245_*.md`),
#      two of which this verdict moves: its (2), the eight-queens depth-1
#      golden of `tests/test_engine.cpp`'s first-iteration case, is re-derived
#      here (item 14) on another board; its (1), the comment in
#      `iterative_deepening_search` quoting 13.5 ms and 42371 nodes for "the
#      board this repository's case uses", now names a board the case's stop
#      half no longer uses. `src/` outside the early-out is not this verdict's,
#      so the comment is S245's still. None reaches play.
#  13. **This verdict's own goldens.** No mined row went red: on both trees --
#      `05ab85d`, where it was built, and `58585f8`, where it lands --
#      `test_search` (every mined mate row, the capture-mate table, the node
#      band of "ordering keeps the tree small"), `test_mate_carry` and
#      `test_mate_breadth` are green in both builds with the goldens as they
#      stood. **One mined row went quiet and was re-derived by its script**:
#      S097's multicut row of "pruning does not hide a forced mate" stopped
#      separating E21 (item 9), and `adocs/data/S097_mine_mate_row.py` stages
#      2 to 6 in guard mode (DEC-238) took S097's own row,
#      `4N3/8/3P1ppk/4p2p/4P2P/1n1P2P1/Q4PK1/3q4 w - - 5 46`, at depth 14, mate
#      in 5, observed red under E21 and green shipped
#      (`adocs/data/S022_v2_remine_s097.log`). **Two goldens of
#      `tests/test_engine.cpp`'s first-iteration case moved with the early-out
#      and were re-derived by the rule at their site (item 14)**: the stop
#      half's board, and the timer half's dead-timer figure. On an H0 or no
#      verdict all three go back byte for byte with the code, S131's row with
#      them.
#  14. **The stop, resolved by the coordinator's ruling of 2026-09-29.** "a stop
#      inside the first iteration cuts it and the hard timer ends the search
#      within its bound" (DEC-240's form, which S246 put in the tree at
#      `58585f8`, S242's completing commit having carried the first redesign)
#      was red at its precondition: the early-out cut depth 1 on the stop
#      half's eight-queens board to 10187 nodes and 1 ms against 36165 and 6
#      to 7 ms, under the case's 3 ms floor. By the case's own rule -- "a
#      heavier position, not a smaller floor", a re-derivation of DEC-233's
#      class and not a relaxation -- the stop half moved to
#      `r1r1r1r1/1r1r1r1k/8/2n1n3/2N1N3/8/1R1R1R1K/R1R1R1R1 w - - 0 1`, where
#      the early-out never fires at depth 1 (a census build's counters, 0
#      with the exchange gate and without it) and the parent and the
#      candidate agree, 146994 nodes and 24 to 31 ms, about nine times the
#      floor; the floor and the prompt-stop line are unchanged. The timer
#      half's board is untouched and its stated dead-timer figure was
#      re-taken on this tree: the depth-1 iteration 25933707 nodes and about
#      5.9 s before, 18594285 nodes and about 3.9 s with the early-out,
#      `bestmove` after 3915 and 3935 ms with the timer disarmed -- the 402 ms
#      bound still about ten times below it. Both fast suites green in both
#      builds; 200 focused runs of the case, 0 failed, the timer half asserted
#      once a run and never retried.
#  15. **S246, the fillers' second-round forms the landings missed: closed**
#      (`58585f8`): `tests/test_engine.cpp` is S242's DEC-240 form (blob
#      `e88103f`), `tests/test_search.cpp` carries S243's returned-score CHECK,
#      and `DEV_MANUAL.md` S243's sentence; the two earlier stamps are true
#      against the tree again. This verdict's item 14 is re-derived on it.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> **the early-out gains at least 5 nElo on a tree with S112's
#                   futility and S015's gate, and it stays.** The switch stays
#                   at 1, as `QsFutility`'s, `ProbCut`'s and
#                   `QsQueenPromotions`' did; `QsDeltaPhaseMin` stays at its
#                   seed and joins S127's set with the shared margin and
#                   victim table -- an H1 says the rule is worth having at the
#                   values tried and not where its maximum is. The goal's
#                   "decide between delta pruning and per-move futility" is
#                   then answered as both. The stopping run's Elo is
#                   upward-biased and is not the effect size (DEC-063).
#
#   H0, interval -> **a loss, and the revert is one default.** `QsDeltaEarlyOut`
#   wholly         to 0, the off value proved above, so the engine is the
#   below zero     reference tree's to the node the moment it moves -- and
#                  **the code leaves with it** (the S238 pattern): the block,
#                  the two rows, the suite's eight cases and the eight mutants
#                  come out in the same step, `MANUAL.md` and `DEV_MANUAL.md`
#                  with them, behaviour-neutral at the reverted default and
#                  discharged by INV-6 against the reference's own engine; the
#                  three goldens of item 13 go back byte for byte with it --
#                  S131's multicut row in place of the re-mined one, the stop
#                  half's eight-queens board with its numbers, the timer
#                  half's dead-timer figure (the fast check of 2026-09-29
#                  caught the earlier "no golden moved": Z07, the early-out
#                  off, is red at the re-mined row). **The goal's "deleting
#                  delta pruning" is the recorded outcome**, S112's per-move
#                  futility the form that stays. No reason to keep the code is
#                  foreseen; if one is given, the completion stamp states it.
#                  The first suspect for the loss is the one section 5 names,
#                  the checking captures S112 searches and this drops, and the
#                  honest follow-up is a wider margin or no ship -- a second
#                  experiment, not this one.
#
#   No verdict   -> **a zero, read the same way (DEC-063)**: terminate at
#   or an          fastchess's cap, record the zero with the games played and
#   interval       the interval, and the switch goes to 0 with the code behind
#   reaching       it on the H0 row's terms -- deleting delta pruning is the
#   above zero     recorded outcome, as a zero. No follow-up run and no second
#                  pair; another margin, a gives-check proxy or a wider pair is
#                  a second experiment, which S127's fit is the place for.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands this verdict's candidate, `QsDeltaEarlyOut` 1,
# on the tree verdict 1's H0 revert leaves; REF is its parent, `58585f8`
# (S246 on the revert; `src/` is the revert's byte for byte). Both are pinned after the landing commit exists, by editing the
# defaults below:
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
         "adocs/data/S022_v2_sprt.sh before running (CAND is S022 verdict" \
         "2's landing commit, REF its parent, S246 on verdict 1's revert)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s022_v2_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
