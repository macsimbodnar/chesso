#!/usr/bin/env bash
#
# S099: PAWN-KEYED STATIC EVALUATION CORRECTION HISTORY -- the pawn key and the
# correction together, one candidate, one verdict (DEC-271).
#
#   nohup adocs/data/S099_sprt.sh > .tuning/s099_sprt.log 2>&1 &
#
# WHAT THE VERDICT COVERS: BOTH PARTS. The step was written as two parts, (a)
# the pawn-structure key and (b) the correction table that reads it, with (a)
# to land alone as node-identical. (a)'s timing under DEC-266 (2026-10-13)
# measured a cost -- cycles +1.96 % on bench 16 and +1.44 % on the 300
# positions, instructions +0.69 % and +0.66 %, about -2 to -3 Elo at DEC-083's
# 1.43 per percent -- so by DEC-271 (a) does not land alone: it was made
# branchless and lands with (b), and this run's REF is the tree **before the
# key**. Whatever (a) still costs and whatever (b) gains are priced here
# together, as the shipped change; nothing else ever measures (a)'s cost.
#
# THE CANDIDATE.
#   (a) `board_t::pawn_hash`, the pawn piece-square keys alone (no side,
#       castling or en passant), kept by add_piece, remove_piece and
#       move_piece and undone by xor in unmake -- branchless since DEC-271:
#       xored for every piece from `zobrist_randoms_t::pawn_randoms`, whose
#       non-pawn rows are zero. Node-identical on its own (INV-6) against
#       S261's landing `bacead1`, 2026-10-14: bench 5636835 and bench 12
#       1937969 with identical streams, search_bench 9 and 12 identical with
#       the same best moves (on f6647dc, before S261, likewise).
#   (b) A table beside the transposition table, `pawn_correction_t`: one
#       int32 per side to move and pawn-key slot, **4096 slots a side** (the
#       census below), owned by the UCI layer, kept across every `go` of a
#       game and cleared by `ucinewgame` (and by `bench` before each position,
#       and by `test` wherever it clears the transposition table).
#       `negamax_at` computes the raw static evaluation once, as S108 made it,
#       and adds the slot's entry over its grain, at most `CorrHistMax` either
#       way, the sum held strictly inside the mate band's edge. Every consumer
#       reads the corrected number -- the improving stack, the futility
#       estimate, reverse futility, razoring -- while both stores and the
#       update read the raw one, so nothing compounds through the table.
#       Beside the node's one store, outside a verification search: a node out
#       of check whose best move takes nothing (a fail-low's best move is only
#       its highest upper bound, so a fail-low's capture does not refuse),
#       whose score is not a mate, and
#       whose bound agrees in direction with the difference teaches the slot a
#       moving-average step of `best - raw`, weight `min((depth + 1)^2,
#       CorrHistWeightCap)` out of 256. Quiescence is untouched. Datagen plays
#       with a table per worker (DEC-270 (2)).
# The step file's "As built" has every file.
#
# DEC-221: implemented from the step file's description, which quotes the Chess
# Programming Wiki's Static Evaluation Correction History page for the shape
# (https://www.chessprogramming.org/Static_Evaluation_Correction_History) and
# commit messages and pull-request prose for the gating, and from nothing
# else. No other engine's code was opened and no constant of one is behind any
# value here (DEC-016, DEC-104, DEC-105, DEC-134); the step file's section 4
# keeps the anti-seeds on record.
#
# THE SEEDS (DEC-134), each with its derivation at its site; the two parameters
# and three definitions are DEC-270 (5)'s reading of the accepts:
#   CorrHistMax 47          (c) midpoint of 0 to one pawn, PAWN 94; 0 is off,
#                           proved: the tune build at 0 prints the parent's
#                           bench stream, 121 lines (DEC-215)
#   CorrHistWeightCap 144   (b) (CONT_HIST_REF_DEPTH + 1)^2, 11 the median
#   CORRHIST_SCALE 256      (b) the power of two at or above the cap
#   CORRHIST_GRAIN 256      (c) geometric midpoint of 64 to 1024
#   entry clamp 12032       (b) CorrHistMax * grain, derived at use
#   4096 slots a side       (b) the census DEC-270 (1) ordered:
#                           adocs/data/S099_pawn_key_census.py, the distinct
#                           pawn keys a `go depth 12` reads per side over S021's
#                           300 positions, cold. On S261's vector the larger
#                           side's p95 reads 2781 at 2048 slots and 2844 at
#                           4096, both giving 4096
#                           (adocs/data/S099_pawn_key_census.txt; on 5ac17bd
#                           it was 1808, 2048 slots). That rule leaves
#                           collisions common (about 50 % of a p95 search's
#                           keys share a slot, uniform hashing, computed); it
#                           is the rule DEC-270 ruled on, and the size is
#                           S127's to sweep.
#
# WHAT IT DEPENDS ON FROM S261. Nothing in this candidate is seeded from a
# value S261's lane fits: the two axes are S127's (S261's own excludes), and
# their seeds rest on PAWN and CONT_HIST_REF_DEPTH, which no search lane moves.
# What S261 does move is the rules the corrected number feeds -- razoring
# (`RazorMargin` 282 -> 242), the move loop's futility on the estimate
# (`FutBase`, `FutSlope`, `FutMaxLmrDepth`) and, through the improving flag,
# late move pruning's count (`LmpBase`, `LmpMaxLmrDepth`); reverse futility's
# margin and the improving flag's reduction term did not move -- all fitted
# without a correction; the step file's section 7 has them re-fitted after
# this lands (S127), not before. No seed reads a value S261 moved. One test
# reads S261's values by name, through the fixture it shares: the probcut
# store case drives S261's ProbCut drive (`ProbCutMinDepth`,
# `ProbCutDepthOffset`, `ProbCutMargin`) and asserts its singular-block
# isolation as S261's drives do; the rest meet S261's vector only as any
# search does. The candidate was rebased onto `22e8df7` blind, then
# fast-forwarded to `6fc7f23` once S261 read H1 (`bacead1`) and S280 landed;
# the census, the goldens and the numbers below were re-taken there,
# 2026-10-14.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, on 6fc7f23 + S099 at 4096
# slots against S261's engine, 2026-10-14:
#   `bench` 5636835 -> 6042933 (+7.2 %), the second of the eight replies
#   e2a6 -> d5e6 and the fifth a7a6 -> d8e7, the rest unchanged; `bench 12`
#   1937969 -> 2339074, its replies unchanged. `tools/search_bench.py` depth
#   9: 43496 / 160453 / 30816 -> 62042 / 125888 / 34107; depth 12: 180917 /
#   509438 / 147594 -> 214973 / 595000 / 217107; best moves c3d5 e2a6 d7c8q
#   and c3d5 d5e6 d7c8q on both. The tune build at `CorrHistMax` 0 prints
#   S261's stream, 121 lines, none differing (DEC-215). The counts move by construction; INV-6 does not
#   discharge this change and nothing here is Elo (DEC-019).
# GOLDENS MOVED WITH IT, each by its script, each landing only with this
# candidate: test_mate_carry's ceilings A 0 -> 2 and E 0 -> 1 (DEC-284) and
# F 1 -> 2 (DEC-272), and the S170 budgets C 1200000, D 500000 and F 300000,
# from adocs/data/S099_sweep_s170.txt; test_mate_breadth's EXACT_FLOOR
# 159 -> 163, a rise, because the correction raised both ends to 164 and 160
# and 159 stopped separating. MATE_IN_THREE_FLOOR 11 and the first-mate
# depths did not move.
#
# THE PREDICTION, labelled as one and not a measurement. The traced record:
# Lynx PR #1662, **+11.35 +/- 5.16 over 7502 games at 8+0.08**, measured at a
# CCRL band of 3224 to 3291 (`adocs/data/S181_lynx_bands.md`), some 300 points
# above this engine; Caissa's introduction +1.74 +/- 1.68, and its re-import of
# the refined update +20.10 +/- 6.82. This engine's published-to-measured
# transfers read 0, 0, wrong sign, 0.10 and 0.33 (DEC-222's F01), so no number
# is predicted here; the direction is positive, less whatever the key costs.
#
# BOUNDS. elo0=0 elo1=5 nElo, alpha=beta=0.05, `model=normalized` --
# fastchess.sh's default, the gainer pair the step file and DEC-258 (4) name.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint, **25591** on a bound (`adocs/testing_strategy.md`
# section 1.1, D / (e1 - e0)^2 with D = 1046535, and 639770 / 25), at **2110
# games an hour** (`.moltke.local.md`): **19.8 h** and **12.1 h**. The cap is
# fastchess.sh's 20000 rounds, 40000 games, 19.0 h. Four hours or more, so a
# night (DEC-155).
#
# REGIME. 8+0.08, Hash 16 (DEC-088), one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 (DEC-050), fastchess.sh's
# adjudication, whatever governor the machine is under, recorded and not set
# (DEC-195). If fastchess, the book, the adjudication or the machine has moved
# since the last fixed-rounds A/A, that A/A of 1000 games comes first
# (DEC-143); the coordinator checks on the day it pins the pair.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Stop it and report nothing, too, if
# the machine loses mains power (POWER) or another load starts on it while the
# run plays (MACHINE). Nothing else stops it -- not the length.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS as scoped by DEC-171),
# re-read against plan.md's Open list at `c5bd0de` (2026-10-14): **none**.
# Closed since this was first written: S278 (the trace sink, `07d9170`), S279
# (`go nodes` one over, `870d210`) and S280 (the mutation recipe's file
# transport, `6fc7f23`). S281, open and conditional on this run's H1, is a
# study of the short mate lines this candidate adds (DEC-284), a reporting
# residue (DEC-122) and not a defect. The coordinator re-reads the list on the
# day it pins the pair.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063),
# in the step file's words (its "Promoted 2026-10-08" section, DEC-258 (4)),
# with DEC-270 and DEC-271's consequences.
#
#   H1 accepted  -> the key and the correction together gain at least 5 nElo
#                   at the harness setting. Both stay, as one commit. **S110
#                   and S111 run next**, each its own verdict, and the two
#                   consumers DEC-222 named -- the correction applied in
#                   quiescence, and the correction's magnitude as a reduction
#                   and margin input -- get step files then, each one verdict.
#                   No 32+0.32 reading: it ships on the verdict (DEC-270 (3)).
#                   The stopping run's Elo is upward-biased (DEC-063) and is
#                   not the effect size.
#
#   H0           -> not shown to gain 5 nElo here. **Revert the whole S099
#                   commit** -- the key with the correction, since the key
#                   alone costs speed and buys nothing in play (DEC-271); S118
#                   re-adds a key when its cache needs one. **S110 and S111
#                   return to the reserve** and no consumer step is created.
#                   Then one DEC-202-shaped estimate at 32+0.32, Hash 64, 1000
#                   pairs, as DEC-269 (2) does for S261: it ships nothing, and a
#                   positive estimate opens a decision for the owner (DEC-270
#                   (3)). No bisection is booked; the owner rules (DEC-270
#                   (4)).
#
#   No verdict   -> the 40000-game cap with neither bound: a zero, read as H0
#                   is, with the games and the interval recorded, and the same
#                   32+0.32 estimate. No follow-up run and no second pair
#                   (DEC-063).
#
# THE CLOSING COMMIT carries DEC-220's block before its `Bench:` line, in
# fastchess's own line names, with the log trimmed into
# `adocs/data/S099_sprt.log` by `tools/trim_console.py`.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path; this script execs it and adds nothing
# after.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR -- a COMMITTED candidate, not a working-tree one (DEC-271):
#
#   REF   `bacead1`, S261's closing commit on its H1: the tree before the key
#   CAND  S099 -- the key and the correction, one commit -- pinned by the
#         coordinator once it is committed
#
# CAND's parent is S280's `6fc7f23` or a docs-only descendant of it, not
# `bacead1` itself; what makes `bacead1` the right REF is that the engine is
# the same, so the script refuses unless `git diff bacead1 CAND^ -- src/` is
# empty.
#
# Both committed, so DEC-220's block names two plain shas that `tools/gate.sh`
# checks against the log and nothing needs a `candidate.diff` blob (DEC-254).
# CAND is pinned by editing its default below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # S099's landing, CAND
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates, and each side's padding, before the first game.
REF="${REF:-bacead1}"
CAND="${CAND:-PIN_ME_CAND}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == PIN_ME* ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S099_sprt.sh before running (REF is bacead1, S261's" \
         "landing, the tree before the key; CAND is S099 committed on" \
         "6fc7f23 or a docs-only descendant)" >&2
    exit 1
  fi
done

if ! git diff --quiet "$REF" "$CAND^" -- src/; then
  echo "SPRT-RUN-FAILED: CAND's parent differs from REF in src/ -- REF is" \
       "the engine the candidate was built on, or the pair measures two" \
       "changes" >&2
  exit 1
fi

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s099_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
