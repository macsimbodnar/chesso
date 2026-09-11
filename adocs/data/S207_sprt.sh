#!/usr/bin/env bash
#
# S207, a repetition before the root is not a draw. Candidate is the working
# tree: `src/bitboard.cpp` grows `classify_repetition()`, which compares the
# matching history index against the root's own entry, and `negamax_at` scores
# `DRAW_SCORE` only on its `DRAW` class. Reference is HEAD, which is what
# fastchess.sh defaults to and prints with its date before the first game
# (S160).
#
# WHAT IT FIXES. 2026-09-10_adversarial-F08, the one high finding of that audit
# that fires in ordinary play. `is_position_repeated()` returned true on the
# first hash match anywhere in the halfmove window, and its one caller scored
# every match as a draw. Nothing distinguished a match the search itself walked
# into from one played in the game before the root.
#
# Driven through python-chess's chess.engine, which waits for `bestmove` --
# never a bare pipe (TOOLCHAIN.md) -- the start position less Black's queen with
# the history `g1f3 g8f6 f3g1`, `go depth 10`:
#
#   reference ae4eed4  with history            score 0      nodes 5387    pv f6g8
#                      same board, no history  score -1229  nodes 412680  pv e7e5
#   candidate          with history            score -1229  nodes 412680  pv e7e5
#                      same board, no history  score -1229  nodes 412680  pv e7e5
#
# Measured here, on this machine, both binaries built the same way -- not
# carried over from the audit, whose tree was two commits older. The candidate
# answers the position identically whether the moves that reached it are there
# or not, which is what the finding asked for; the reference answers `cp 0`
# behind a tree 77 times smaller.
#
# Oracles, neither of them chesso: python-chess 1.11.2 on the position after
# `f6g8` reports `is_repetition(2) True`, `is_repetition(3) False`,
# `can_claim_threefold_repetition() False`, `is_game_over(claim_draw=True)
# False`; stockfish at depth 18 with the identical board and move history scores
# it **-703**, playing `b8c6` (the audit read -687 from the same invocation on
# 2026-09-10; both are re-taken here rather than quoted). No draw exists.
#
# THE RULE, AS PUBLISHED. A draw is returned if the position repeats once
# earlier but strictly after the root, or repeats twice before or at it (CPW
# *Repetitions*; Stockfish PR #925, read as prose -- no source was opened,
# DEC-016). The convention it replaces is not wrong chess, it is a weaker
# convention that no decision in this repository ever recorded and that the
# engine's own oracles contradict. DEC-173 is the decision, and it is what lets
# an agent re-state the two fast-suite cases that pinned the old rule.
#
# MEASURED BEFORE THE GAMES, NOT ARGUED.
#
#   `tools/search_bench.py` depth 9, node-identical to HEAD:
#     121530 / 801481 / 72924, best `c3d5` / `e2a6` / `d7c8q`.
#   That is NOT a claim of behaviour neutrality and INV-6 is not available
#   here. It says only that no bench tree returns to its own root position:
#   the three positions are loaded as bare FENs, so there is no pre-root
#   history for the changed class to fire on except the root's own entry.
#
#   `chesso bench`, which loads eight bare FENs the same way, does move:
#     26851183 -> 26491479, -359704 nodes, -1.34 %.
#   The only class that can move from a bare FEN is the root position
#   recurring once inside the tree, which the old rule called a draw and the
#   published one does not. So the boundary this step turns on is reachable at
#   depth 14 from a bare FEN, before any game history is involved.
#
#   In a game it is the pre-root history that fires, and fastchess sends every
#   game as `position startpos moves ...`, so the pre-root history *is* the
#   game. How often a pre-root two-fold is reached inside a search is the count
#   the H0 reading below demands, and it is deliberately not taken here: this
#   run's own PGN is the population to take it over.
#
# BOUNDS. `--nonreg`, fastchess.sh's own mode: elo0=-5 elo1=0, alpha=beta=0.05,
# nElo. The right question for a change whose motive is a wrong score in
# ordinary play rather than a claimed gain: it asks whether this costs 5 nElo or
# more, not whether it gains. Direction is not obvious in either sign -- the
# reference threw away won positions by taking a false draw, and the candidate
# searches trees the reference pruned, which costs time.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices a `{-5,0}`
# pair at **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound: **18.4 h** and **11.2 h** at the 2277 games an
# hour measured on this machine (S198, `.moltke.local.md`). If the effect is
# outside the interval in either direction it is far less. Budgeted as a night
# run under DEC-155, launched 2026-09-11.
#
# ABORT RULE. Stop the run and report nothing if the forfeit rate passes 1.0 %
# on either side (`tools/forfeit_report.py` over the PGN). A crash on either
# side voids it as well.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> not a regression of 5 nElo or more. Keep it. No magnitude is
#                  claimed; what the change is for is the score, and the oracles
#                  above are what establish that.
#   H0 accepted -> it costs 5 nElo or more. Before believing it, count how often
#                  the changed class fires in this run's own PGN -- a pre-root
#                  two-fold reached inside a search -- because a rule that fires
#                  rarely cannot cost that much and a wrong count is likelier
#                  than a wrong oracle. Then a decision, and it is not "restore
#                  the old convention" by default: a measured cost for a correct
#                  score is a finding about the search around it, in the shape
#                  of DEC-019's three published figures that did not transfer.
#   No verdict   -> record as zero and KEEP the change, with the reason stated
#                  rather than implied: the engine's own oracles say the old
#                  score is wrong on a position that occurs in ordinary play,
#                  the tool CLAUDE.md mandates for chess judgement reads those
#                  scores, and S005, S006, S015 and DEC-103 are the precedent
#                  for keeping a measured zero.
#
# A null is a likely outcome and an acceptable one. The run happens because
# AGENTS.md par.0 decides a play-altering change by SPRT and not by argument.
#
# AGENTS.md par.12 and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.
#
#   nohup adocs/data/S207_sprt.sh > .tuning/sprt_s207.log 2>&1 &

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

exec ./fastchess.sh --nonreg
