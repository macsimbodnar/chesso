#!/usr/bin/env bash
#
# S165, null move pruning's negative mate-band guard. Candidate is the working
# tree plus one term in `src/search.cpp`: the null-move condition gains
# `beta > -MATE_MIN`, the mirror of the edge reverse futility has guarded from
# the start. Reference is HEAD, which is what fastchess.sh defaults to and
# prints with its date before the first game (S160).
#
# WHAT IT FIXES. 2026-08-22_adversarial-F04. Reverse futility guards both edges
# of the mate band; null move pruning guarded only the positive one. With
# `beta <= -MATE_MIN` the node is a defender node inside a mate proof, beta is
# a mate bound against the side to move, and any null-move result clears it --
# so the node fails high on a reduced search's word, and the reduced search is
# exactly the instrument that misses mates. This rule hid a mate in two once
# already. No comment, decision or step ever recorded why the two rules
# differed.
#
# MEASURED BEFORE THE GAMES, NOT ARGUED.
#
#   reachability, otherwise-eligible null-move nodes in the negative band
#     400 corpus positions, depth 10           0 of 301620      0.0 %
#     104 proved defender nodes, depth 10   3079 of   6252     49.2 %
#
#   the defender sweep, adocs/data/S165_nmp_defender_sweep.py, 104 proved
#   defender nodes at depth 2k+8, RfpMinPly 3 and RfpMaxDepth 15 held:
#     guard absent   exact 80/104   m1 50/50 d0  m2 28/31 d8  m3 2/15 d8  m4 0/8
#     guard present  exact 82/104   m1 50/50 d0  m2 29/31 d8  m3 3/15 d8  m4 0/8
#     short 0 and sign 0 on both
#
# So it is inert in ordinary play, fires on half the eligible nodes of a mate
# proof, and buys two more mates found there with no delay regression anywhere.
# `tools/search_bench.py` at depth 9 is node-identical -- 121512 / 800769 /
# 62907, `c3d5` / `e2a6` / `d7c8q` -- which is the same fact the reachability
# count states and is NOT a claim of behaviour neutrality: the class it changes
# is mate-bound nodes, and no bench position contains one. INV-6 is therefore
# not available to discharge this and the run is what the accepts asks for.
#
# BOUNDS. `--nonreg`, fastchess.sh's own mode: elo0=-5 elo1=0, alpha=beta=0.05.
# That is the right question for a change that cannot gain measurably in
# ordinary play and must not cost anything: it asks whether this is a
# regression of 5 Elo or more, not whether it gains.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> not a regression of 5 Elo or more. Keep it. The magnitude is
#                  not established and no gain is claimed; the mate evidence is
#                  what the change is for.
#   H0 accepted -> it costs 5 Elo or more. That would be a surprise the
#                  reachability count cannot explain -- 0 of 301620 -- and the
#                  honest response is to treat the count as wrong rather than
#                  the games: re-measure reachability over a real match's
#                  positions before deciding anything, and write the decision.
#                  Do not keep the guard on the strength of the argument.
#   No verdict   -> record as zero and KEEP the guard, with the reason stated
#                  rather than implied: 0 of 301620 eligible nodes in ordinary
#                  play means a null is the expected reading and not a weak
#                  one, the change removes an asymmetry nothing ever argued
#                  for, and it is one term in a condition. S005, S006, S015 and
#                  DEC-103 are the precedent for keeping a measured zero.
#
# A null is the expected and acceptable outcome of this run. It is run anyway
# because AGENTS.md par.0 decides a play-altering change by SPRT and not by
# argument, and because the count that predicts the null is itself a
# measurement that could be wrong.
#
# AGENTS.md par.12 and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.
#
#   nohup adocs/data/S165_sprt.sh > .tuning/sprt_s165.log 2>&1 &

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

exec ./fastchess.sh --nonreg
