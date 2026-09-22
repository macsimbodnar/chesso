#!/usr/bin/env bash
#
# S188, the check extension: a move that gives check is searched one ply
# deeper, decided inside the move loop after the move is made. Judged against
# the tree without it.
#
#   nohup adocs/data/S188_sprt.sh > .tuning/sprt_s188.log 2>&1 &
#
# **THE FORM IS NOT CHOSEN YET AND THIS FILE IS NOT BOOKED.** DEC-228 re-formed
# the step before a game was played, on the instrument's own reading, and
# stated a bar the run is booked against: at `go nodes 1000000` over the three
# `search_bench` positions the form loses at most one ply on any position and
# at most two in total against the parent's 17 / 13 / 15, `bench` grows by less
# than 30 % over 4493659, and every fast-suite case stays inside its ceiling.
# **FORM 3 IS THE FORM MEASURED AND THE ONE THIS FILE BOOKS.** DEC-228 names it
# as the one further form to try when the gate alone misses, so no new decision
# was needed: form 2 missed on bench alone, form 3 meets all three clauses of
# the bar, and the coordinator's reading of 2026-09-22 is recorded in the step
# file. Three forms were measured that day:
#
#   form 1  every check extended        bench +94.6 %, depths 15/11/14 (40),
#                                       test_mate_breadth 212 s -- bar missed
#                                       on every clause
#   form 2  + `see_ge(board, move, 0)`  bench +72.4 %, depths 17/12/15 (44),
#           the safe-check gate         114.9 s -- **bar missed on bench**
#   form 3  + `depth <= CheckExtMax-     bench +28.1 %, depths 16/13/15 (44),
#           Depth`, the horizon         109.2 s, fast suite 38/40 -- **bar met
#           restriction                 on all three clauses**
#
# The outcomes below are form 3's. `CheckExtMaxDepth` 1 -- `bench` below the
# parent's and a fixed-node total of 46 against 45 -- is recorded as a reading
# for S127's lane and is not the seed (DEC-105 (c), DEC-019).
#
# **THE CANDIDATE IS ONE RULE AND THREE SETTINGS.** `negamax_at`'s move loop
# adds a ply to a move that gives check -- a capture that gives check
# included -- while `CheckExtend` is 1, the node is not the root, and the
# node's ply has not reached `CheckExtPlyFactor * depth`, its remaining depth is
# at most `CheckExtMaxDepth` -- the last that many plies before the horizon --
# **and the move's own exchange at threshold zero holds on the parent board**
# (DEC-228's safe-check gate: a check that hangs the checking piece is not
# extended). The ply is shared with S097's singular extension: a move both
# rules want is searched one ply deeper and not two -- unreachable at the
# shipped seeds, where `SeMinDepth` 10 and `CheckExtMaxDepth` 8 do not overlap,
# which is why the case that holds the budget is in the tune build and the
# site's own assertion holds it in the release one. Nothing else in `src/`
# moves.
#
# DEC-221: implemented from the step file's description and from the Chess
# Programming Wiki's Check Extensions page, which states "typical depth to
# extend is one ply" (https://www.chessprogramming.org/Check_Extensions,
# fetched 2026-09-22). No other project's code was opened. The one ply is that
# form (DEC-105 (a)); `CheckExtPlyFactor` is 4, **(c) the midpoint** of a range
# stated by purpose in `src/search_params.hpp`, because the wiki's Extensions
# page warns that "care must be taken so that the search is not extended
# infinitely" and states no factor, ratio or formula
# (https://www.chessprogramming.org/Extensions, fetched 2026-09-22). No
# engine's number seeds either (DEC-084 as amended by DEC-105, DEC-134).
#
# THE PRIOR IS SMALL OR ZERO, and it is DEC-222's, recorded before the run.
# Three engines have removed a check extension. Ethereal's was the
# **pre-move-loop** form, removed at +4.14 +/- 4.15 and +4.54 +/- 4.13 over
# bounds [-3, 1], and its master still extends a checking move inside the move
# loop. Stormphrax removed one outright with a bench and no Elo, from a record
# far above this band. The open-source record removed one on 2025-04-17 at
# -0.26 +/- 1.49 over 58,032 games on a non-regression pair, its message
# stating no form, and its own 2024 history moved the extension inside the loop
# and back out within a fortnight. **No record establishes the in-loop form
# measured at zero**, and of the hand-crafted engines the plan reads as
# existence proofs, Weiss 1.2 at 3055 and Stash both carry it. The record
# decides that the technique is tried and never what the number is (DEC-019).
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-22, machine idle.
#
#   THE OFF VALUE IS PROVED ON THE TREE and not declared (DEC-215 clause 2):
#   the tune build at `CheckExtend` 0 prints **4493659**, the reference
#   binary's own total, with **all eight `bestmove` replies identical** to a
#   Release build of the parent commit. So an H0 removal is a one-line flip
#   that is behaviour-neutral by the signature and not by argument.
#
#   BENCH -- reference 4493659 -> **8744373 (+94.6 %) at form 1, 7748319
#   (+72.4 %) at form 2, 5756104 (+28.1 %) at form 3**. An extension grows a
#   fixed-depth tree by construction: it buys depth along forcing lines and
#   pays for it everywhere else. What the bar asks is how much.
#
#   tools/search_bench.py, depths 9 and 12, reference -> candidate:
#     depth  9  midgame   21479 ->  45998 (`g5f6` -> `c3d5`)
#               kiwipete 102462 -> 100441 e2a6
#               tactical  33148 ->  27117 d7c8q
#     depth 12  midgame  149688 -> 146741 c3d5
#               kiwipete 459216 -> 662182 (`e2a6` -> `d5e6`)
#               tactical 219544 -> 239407 d7c8q
#     Node counts move by construction, so INV-6's discharge is not available
#     and games are the only thing that can decide this step. **The direction
#     is not uniform** -- two of the six cells fall -- which is the ordinary
#     signature of a reordering on top of a deeper search, and one best move
#     moves at each depth. Of the bench's own eight replies, one moves:
#     kiwipete's `e2a6` -> `d5e6`.
#
#   adocs/data/S097_fixed_node_depth.py, go nodes 1000000, Hash 16 -- **THE
#   ROW TO READ BEFORE SPENDING THE NIGHT**:
#     reference  midgame 17  kiwipete 13  tactical 15   total 45
#     form 1     midgame 15  kiwipete 11  tactical 14   total 40
#     form 2     midgame 17  kiwipete 12  tactical 15   total 44
#     form 3     midgame 16  kiwipete 13  tactical 15   total 44
#     Form 1 lost **five plies over three positions**, which is the explosion
#     signature this instrument exists for and what DEC-228 read (S097 verdict
#     1 lost three and measured a zero). The gate recovered four of them and
#     the horizon restriction holds that while bringing the bench inside the
#     bar. Whether a tree that reaches the same depth on two positions and one
#     less on the third is worth its nodes is what the games answer.
#
#   The ply factor is not what costs form 1: swept over its whole range on the
#   tune build (`adocs/data/S188_cap_sweep.log`), `bench` reads 6883468 at 1 and
#   8744373 at the shipped 4 against 4493659 off. **Under the gate it is**
#   (`adocs/data/S188_cap_sweep_gated.log`): +23.8 % at 1 against +72.4 % at 4.
#   And form 3's own axis is swept in `adocs/data/S188_horizon_sweep.log`, where
#   the two cells at the floor read a smaller tree at a fixed depth and a
#   deeper search at a fixed budget than the parent. Every seed is the range's
#   own midpoint and stays (DEC-105 (c)); re-stating a range to put its midpoint
#   on the cell that passes the bar would be fitting the seed to the bar, S127
#   is where these axes are fitted, and a sweep of depth at a fixed budget is
#   not Elo (DEC-019, with S021's precedent: a sweep chose, the SPRT measured).
#
#   WHAT IT BUYS, measured on the same trees: the S145 mined breadth set at
#   depth 10 goes from **146 exact / 148 right sign** to **189 / 203** at form
#   1 and **180 / 190** at forms 2 and 3, 0 wrong sign at every form, against a
#   floor of 143 (`adocs/data/S145_mined_set.py score --depth 10`). Thirty-four
#   more of this project's own labelled mates found at the labelled distance is
#   still the largest move that instrument has recorded. It costs
#   `test_mate_breadth` 19 s -> 212 s at form 1, which is over its own 120 s
#   ceiling, and 109 s at form 3, which is inside it.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the pair DEC-133 fixed for this step and DEC-222
# affirmed. A `{-5, 0}` pair, which the 2026-09-19 study proposed for the
# removal-shaped steps, accepts H1 on a truth of zero and would ship an
# extension that does nothing after the full walk.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** at the interval's
# midpoint and **25591** with the truth on a bound (`adocs/testing_strategy.md`
# section 1.1, `adocs/plan.md`'s cost table), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. A night, scheduled (DEC-155).
# The candidate searches a bigger tree at the same clock, so the throughput is
# read off the banner and recorded rather than assumed.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). **No harness change since the last
# fixed-rounds A/A**; if fastchess, the book, the adjudication or the machine
# moves before this starts, the A/A of 1000 fixed rounds comes first and is
# read with `adocs/data/S105_pairs.py` (DEC-143).
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# read off `adocs/plan.md`'s Open list on 2026-09-22. **No defect reachable in
# ordinary play, on the UCI surface, or able to move a reported score, move or
# line is open.** What is open is test-side and filler:
#
#   1. **S239** -- `tools/mutation_check.py` accepts a baseline that ran no
#      tests and a fixture dirty outside `src/`. Both were walked into by S097
#      verdict 2's agent on 2026-09-21. Neither can move a reported score,
#      move or line; this step's two mutation runs checked both by hand --
#      fixtures `504a43b` and `a6e0475`, each `diff -rq` clean against the
#      tree for `src/`, `tests/`, `adocs/` and `tools/`, and each baseline
#      line quoted with its real test count (40).
#   2. **`test_mate_breadth` has 2.6 % of headroom under its ceiling.** This
#      landing measured it at 109.2 s Release against the 120 s
#      `CHESSO_MATE_BREADTH_TIMEOUT`; the fast check measured 116.82 s at load
#      0.79 on the same tree, and this project's own noise floor has printed
#      between 0.1 % and 2.0 % depending on the machine's load, so the margin
#      is inside it. The ceiling is not relaxed (TESTS). Test-side under
#      DEC-171 -- it cannot move a reported score, move or line -- and it
#      closes either way this verdict goes: H1 and a step re-derives the
#      ceiling by the rule its CMake comment states or restructures the case;
#      H0 and the case is back at the parent's 19 s.
#   3. **`X10_extends_two_plies` is scored `unmeasured`** by that same tool
#      and it is not a coverage gap: its four failing tests are all Timeouts,
#      DEC-165 refuses to score a Timeout as a kill, and the mutant was
#      observed red by assertion under the step's first form
#      (`adocs/data/S188_form1_reds.log`) **and again by hand on this tree**,
#      `REQUIRE_EQ( record.child_depth[k], depth )` at `values: REQUIRE_EQ( 5,
#      4 )`. Named here because a reader of the kill table will see a row that
#      is not `killed`.
#   4. The three S097 named and re-checked on 2026-09-21, each still open and
#      each unmoved by this landing: S231 phase one's node-budget label of
#      17321 against the reverted tree's 16256; `adocs/data/S192_node_budget.py`
#      reporting OUTSIDE for a band it had just derived; and
#      `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pinning depth 6 where `adocs/data/S231_research_witness.py`
#      answers 4.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063):
#
#   H1 accepted  -> the extension gains at least 5 nElo. **Keep it**, at all
#                   three seeds, and S127 fits `CheckExtPlyFactor` and
#                   `CheckExtMaxDepth` with the rest of the search vector --
#                   the two sweeps say both axes have something to find, and
#                   `CheckExtMaxDepth` 1 is the cell to try first. The stopping
#                   run's Elo is upward-biased and is not the effect size
#                   (DEC-063).
#   H0 accepted  -> **the rule leaves the tree in the same step**: the window,
#                   the exchange gate, the shared capture scan's third term,
#                   the ply, the three settings, the ten cases, the twelve
#                   mutants and the `MANUAL.md` rows. The three repaired cases
#                   go back with it and by the same route they were repaired:
#                   `adocs/data/S188_repair_goldens.py s207` answers 4 on the
#                   reverted tree and `first-mate` answers 9, and the
#                   reported-line invariant returns to an equality. The removal is proved and not argued --
#                   `bench` reads 4493659 and `tools/search_bench.py` is
#                   identical to the reference at depths 9 and 12, node counts
#                   and best moves both, which is INV-6's own discharge -- and
#                   the flip of `CheckExtend` to 0 is the same tree by the
#                   signature recorded above, so the revert is checkable
#                   before it is committed. DEC-194 is the precedent for not
#                   keeping a feature whose interval sits below the bound. A
#                   reason to keep it anyway would have to be stated, and the
#                   mined mate set's +43 is the only candidate on the table.
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero
#                   with the games played and the interval, and take the H0
#                   path above in full -- said in advance so that a long run is
#                   not read as encouragement (DEC-063).
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands the check extension; REF is its parent, the
# tree without it. Both are pinned by the coordinator after the landing commit
# exists, by editing the defaults below:
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
         "adocs/data/S188_sprt.sh before running (CAND is the check" \
         "extension's landing commit, REF is its parent)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s188_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
