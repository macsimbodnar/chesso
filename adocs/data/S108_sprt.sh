#!/usr/bin/env bash
#
# S108, the static evaluation at every node. Candidate is the working tree,
# reference is bbbd9f4 -- the step's parent and NOT fastchess.sh's HEAD default,
# which by now is the step's own first commit.
#
# WHY THE WHOLE STEP AND NOT THE SECOND COMMIT. The step file's par.6 books this
# run against the first commit, so that it prices the store and the read-back
# alone. That is measured here against bbbd9f4 instead, deliberately:
#
#   - the first commit is not independently keepable. It hoists the evaluation
#     to the top of the node, costs -1.21 % nps and buys nothing until the
#     entry carries the number. There is no outcome in which it ships alone.
#   - so its cost would otherwise never be measured by anything. Booked against
#     the first commit, this run reads ~0 and the step still ships an unpriced
#     -1.7 Elo of throughput.
#   - the step is the unit that ships or does not, and this is the question that
#     decides it.
#
# Recorded as a deviation from the step file rather than taken silently
# (AGENTS.md par.4). One change at a time is not weakened: the two commits are
# one change with one purpose, and the bisect the step file names -- overwrite
# fix alone against compute-and-store -- is still what a failure buys.
#
# WHAT THE CANDIDATE DOES. Every non-check main-search node computes its static
# evaluation once at the top of the node, reads the entry's number where the
# probe it already paid for found one, and stores what it computed;
# tt_store_entry() no longer overwrites a stored evaluation with TT_EVAL_NONE
# for the same position. Quiescence therefore stands pat on an exact score at
# nodes where it used to use a lazy bound.
#
# MEASURED BEFORE THE GAMES, NOT ARGUED. adocs/data/S108_node_reach.py, and the
# counters it describes, over 200 book positions at depth 12, Hash 16:
#
#   main-search stores carrying an evaluation      92389379 of 159810384  57.8 %
#   the preserve firing (a sentinel store kept)       259867
#   quiescence stand-pat sites                     137392769
#     reached with an entry carrying an evaluation     794270   0.578 %
#     of those, from a main-search entry               313851   0.228 %
#     of those, differing from evaluate_lazy()          29492   0.021 %
#
#   node counts, same 200 positions at depth 12, Hash 16
#     3 of 200 differ, 0 best moves differ
#     347297369 -> 346772409, -0.15 %, the three at -4.0 %, -24.5 %, -2 nodes
#
#   and at the engine's default hash the same change is invisible: 60 positions
#   at depth 11 read 63680446 nodes on both sides, 0 differences. The reach is a
#   function of replacement pressure, so INV-6 is NOT available here and a
#   bench-sized sample would have called this neutral and been wrong.
#
#   the throughput cost of the hoist alone, first commit against bbbd9f4:
#     -1.21 % nps, sigma 0.96 %, 12 interleaved pairs at go movetime 3000 from
#     the start position, the candidate faster in 1 pair of 12. About -1.7 Elo
#     at DEC-083's conversion, and it is the thing this run has to earn back.
#
# BOUNDS. --nonreg, fastchess.sh's own mode: elo0=-5 elo1=0, alpha=beta=0.05.
# The gainer pair is wrong here and DEC-063 is why. Every traced positive for
# this plumbing had its consumers already present (Weiss #653, +5.89/+4.28) or a
# reuse trick bundled in (#135, +8.63/+7.87); the consumers arrive at S109, not
# here. The local precedent is S094's two zeros for the same field. So the
# expected effect is a read-back saving minus a throughput cost plus a stand-pat
# tightening that measured zero at S130 -- which sits between 0 and +5 and would
# random-walk a gainer run to the round limit.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> not a regression of 5 Elo or more. Keep the step. No gain is
#                  claimed and the magnitude is not established; what the step
#                  is for is the input S109's four pruning rules read, and this
#                  run says buying it did not cost strength.
#   H0 accepted -> the step costs 5 Elo or more. Bisect before deciding
#                  anything: the overwrite fix alone against compute-and-store,
#                  which separates "the preserved evaluation is wrong somewhere"
#                  from "the hoist's throughput is not earned back". Do not keep
#                  it on the strength of S109 needing it -- S109 can read a
#                  number computed at its own consumers instead.
#   No verdict   -> record as zero and KEEP, with the reason stated: the reach
#                  count predicts a null (0.021 % of stand-pat sites change
#                  their number at all), the input is infrastructure four
#                  pending steps read, and S005, S006, S015 and DEC-103 are the
#                  precedent for keeping a measured zero. An unpriced -1.21 %
#                  nps is the known cost of doing so and S109 is where it is
#                  earned back or is not.
#
# AGENTS.md par.12 and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.
#
#   nohup adocs/data/S108_sprt.sh > .tuning/sprt_s108.log 2>&1 &

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

REF=bbbd9f4 exec ./fastchess.sh --nonreg
