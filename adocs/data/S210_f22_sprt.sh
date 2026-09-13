#!/usr/bin/env bash
#
# S210, F22 -- quiescence scores a dead position as a draw. Candidate is the
# working tree at the commit the coordinator makes for the step's second half:
# one test in `quiescence()`'s move loop in `src/search.cpp`, after `make_move`,
# returning `DRAW_SCORE` where the move left a position
# `is_insufficient_material()` calls dead. Nothing else in `src/` moves.
# Reference is **the first half's landing commit**, pinned below, because this
# script runs after the F22 commit and `REF=HEAD` would then name the change
# itself, which `fastchess.sh`'s A/A guard refuses.
#
#   nohup adocs/data/S210_f22_sprt.sh > .tuning/sprt_s210_f22.log 2>&1 &
#
# WHY THIS RUN EXISTS AT ALL, AND WHY THE CENSUS DID NOT DISCHARGE IT.
# DEC-107's precedent is that an insurance run whose only reachable outcome is
# the one already pre-declared is not a measurement, and that a census over the
# games carrying the effect decides it in ninety seconds instead of four hours.
# That precedent was tested here first and it does not apply: the census found
# the class reached, and reached often enough to move the engine's answer.
#
#   adocs/data/S210_f22_census.py, over 11503 positions systematically sampled
#   from all 115021 of `adocs/data/S219_aa_calibration.pgn` (the DEC-143 A/A,
#   1000 games at 8+0.08), each searched to depth 10 on the tune build, whose
#   `bench` is the release build's 7111579 to the node:
#
#     quiescence nodes          492702453      49.5 % of all 996242087 nodes
#     moves made in quiescence  254361785
#     ... onto a dead board        158685       0.062 % of them
#     root best move moved            194 of 11503   1.687 %
#     root score moved                444 of 11503   3.860 %
#     total nodes             996242087 -> 996421227   x1.000180
#
#   S162's census read 0 firings in 3314 games. This one reads 194 changed root
#   moves in 11503 searches, and the Debug self-play for this step ended 7 of
#   80 games "Draw by insufficient mating material" -- the class arrives on the
#   board, not only inside the tree. **A change that moves the played move on
#   1.7 % of real positions is decided by SPRT** (MEASUREMENT, DEC-143), and
#   DEC-107's own last sentence says so: it applies to a correctness fix on a
#   rare boundary and not to a change that alters the tree.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Not Elo and not read as Elo
# (DEC-019); here so that "the rule fires" is a measurement and not a claim.
#
#   chesso bench                7111579 -> 7105111,  -0.09 %
#   tools/search_bench.py       all six rows identical at depths 9 and 12,
#                               node counts and best moves both -- the three
#                               positions never trade down far enough to reach
#                               the class, which is exactly why the census was
#                               taken over games instead of over them.
#
# BOUNDS. `--nonreg`: elo0=-5 elo1=0, alpha=beta=0.05, nElo, `model=normalized`.
# The non-gainer pair, because this is a correctness fix and not a gainer: the
# change replaces a wrong score with the right one in positions the game is
# already over in, and the expectation pre-registered here is **no measurable
# change**. DEC-063 sizes bounds from the expectation, and an expectation of
# zero is what `{-5, 0}` is for -- it asks only that the fix does not cost.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula
# (`adocs/testing_strategy.md` 1.1) prices `{-5, 0}` at **41861 expected games**
# with the truth at the interval's midpoint and **25591** with it on a bound:
# **19.8 h** and **12.1 h** at the 2110 games an hour measured on this
# workstation at 8+0.08 on `noob_3moves.epd` (`.moltke.local.md`, S219's
# DEC-143 A/A of 2026-09-12). A true zero sits on the H1 boundary of this pair
# and drifts, so the midpoint figure is the one to budget: **this is a night
# run** (DEC-155), not a daytime one.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050). Everything `fastchess.sh` already does;
# nothing here overrides it.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN).
# A crash or a disconnect on either side voids it outright and `fastchess.sh`
# says so itself (`SPRT-RUN-INVALID`, S212). And one specific to this change:
# **read, after the run, whether either side reported a mate score in a game
# the adjudicator ended as drawn by insufficient material** -- that is the
# shape a draw rule that hides a mate would take, and it is the recurring bug
# class here. No instrument checks it mid-run (tools/forfeit_report.py covers
# the forfeit clause only), so it is not an abort trigger: it is read from the
# PGN's score comments and termination strings when the run has ended, and a
# hit is a finding before the verdict is banked. The direct guard is the unit
# case "a quiescence capture into a dead position is a draw" plus the mate
# cases in the fast suite, all green.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# None is reachable in ordinary play in a way that can move this verdict:
#   S223  2026-09-12_adversarial-F01 -- `load_FEN` accepts a placement with the
#         wrong number of kings or the side not to move in check. Reachable
#         only through a `position fen` line no harness sends: fastchess sends
#         `position startpos moves ...`.
#   S213  2026-09-10_adversarial-F26, F27, F33 -- stale comments, two dead
#         public entry points and two `<cctype>` calls on a signed char. None
#         alters a reported score, move or line.
#   S225  the `STAGES` leak in `tools/gate_extra.sh` -- a harness-test defect,
#         outside `src/` entirely.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> the fix does not cost 5 nElo or more. **Keep it.** That is
#                  the whole question this run asks; the stopping run's Elo is
#                  upward-biased and is not an effect size (DEC-063), and no
#                  gain may be claimed from a `{-5, 0}` verdict whatever the
#                  printed number says.
#   H0 accepted -> the fix costs at least 5 nElo. **Keep it anyway and open a
#                  step**, because the score it replaces is wrong by the laws
#                  of chess and a wrong score that happens to win games is a
#                  bug the engine is profiting from, not a feature. What that
#                  step would investigate is named now rather than after: the
#                  most likely mechanism is that scoring a dead line 0 makes
#                  the engine *accept* a trade into a drawn ending it was
#                  previously walking away from with a positive static score --
#                  a contempt question (a drawn position is worth 0 and the
#                  engine has no contempt term at all), not a question about
#                  this test. S005, S006 and S015 are the precedent for
#                  recording a measured cost and keeping the change with the
#                  reason stated.
#   No verdict   -> record as zero and keep the fix. A correctness fix that
#                  cannot be separated from zero at these bounds after 41861
#                  games has been shown not to cost, which is all that was
#                  asked of it. Do not re-run at wider bounds: a wider band
#                  measures the same zero more cheaply and no decision hangs
#                  on the magnitude.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# 9075bf8 is "Land S210's first half: nine protocol and rules defects, none
# reachable in play" -- the commit immediately before F22, and `No functional
# change` from its own parent, so the reference tree is the one every figure
# above was measured against. REF in the environment overrides. CAND, which
# `fastchess.sh` reads since S151, pins the candidate to a commit built into
# `.ref-builds/` instead of the working tree; the coordinator sets it to F22's
# own commit after committing it, so neutral steps landing during the day
# cannot enter the candidate (default below, filled at commit time).
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-9075bf8}" CAND="${CAND:-9ef06f3}" ./fastchess.sh --nonreg
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
