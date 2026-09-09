#!/usr/bin/env bash
#
# S148, the reverse futility depth ceiling. Candidate is the working tree with
# one integer moved in `src/search_params.hpp`:
#
#   X(RFP_MAX_DEPTH, "RfpMaxDepth", 15, 0, 63)  ->  ... 4 ...
#
# Reference is HEAD, `192a5a3` plus this step's document commit, which is what
# fastchess.sh defaults to and prints with its date before the first game
# (S160). No rule is added and no code path is touched; the declared range
# stays 0 to 63 (DEC-095).
#
# WHERE 4 COMES FROM. The rule was pre-registered in the step file before the
# grid was run and it is DEC-105 form (b), a derivation over chesso's own data:
#
#   C1 = the largest ceiling at which both the mate in four and the mate in
#        five exact counts over the 82 constructed rows are non-zero
#
# adocs/data/S148_rfp_ceiling_sweep.log, at HEAD, `RfpMinPly` held at 3, every
# value from 0 to 15 -- a grid no earlier sweep had, the set having been 82
# rows since S168:
#
#   RfpMaxDepth   exact/82   m3      m4      m5      mined exact
#   0             70         20/24   13/16   11/16   177
#   1             70         21/24   14/16    9/16   175
#   2             62         20/24   10/16    6/16   175
#   3             60         20/24   10/16    4/16   166
#   4  <- C1      52         18/24    7/16    1/16   159
#   5             47         17/24    4/16    0/16   149
#   6             46         15/24    5/16    0/16   149
#   10 to 15      39         12/24    1/16    0/16   145
#
#   short 0 and sign 0 at all sixteen settings -- the two defect columns.
#
# The mate in five class is the binding one and it is a cliff: 11, 9, 6, 4, 1
# and then nothing. The plateau starts at 10, not at 15, so the shipping value
# confines nothing that three lower values do not also fail to confine.
#
# WHAT IS NOT CLAIMED. No Elo. The ceiling is what confines the pruning to the
# nodes nearest the leaves, so 4 searches more nodes per iteration and reaches
# fewer plies under a clock. Measured, both builds, before the games:
#
#   tools/search_bench.py, depth 9    121530 / 801481 /  72924   HEAD
#                                     124511 / 805638 / 111393   candidate
#   tools/search_bench.py, depth 12   636677 / 3520847 / 494098  HEAD
#                                     716171 / 3707680 / 560079  candidate
#   chesso bench                      26851183                   HEAD
#                                     28339749                   candidate
#   best moves c3d5 / e2a6 / d7c8q on both, at both depths.
#
# So the change is not behaviour-neutral, INV-6 is not available to discharge
# it, and this run is what decides it. S085's SPSA moved this axis the other
# way and the vector that held 15 was verified at +21.02; that figure is joint,
# upward-biased like every early-stopped estimate (DEC-063), and it decides
# what to try and never what to conclude (DEC-019).
#
# MATE-PV READING, AND A KNOWN CLASS THAT WILL SHOW. The run carries
# `-check-mate-pvs`. Count `Incomplete mating PV` lines per side in the log,
# as S171 does. The class is open under S202 and DEC-122 and it is sensitive
# to the node budget rather than to the ceiling: at the candidate,
# adocs/data/S203_case_sweep.sh reads case A short 2 of 9 lines at 1000000
# nodes and short 0 at 1200000, 1500000, 2000000, 3000000 and 4000000, and
# case B short 4 at stride 2 / 2000000 alone. A candidate count above the
# reference's is recorded as a finding against S202 and is read before the
# verdict is; it is not silently attributed to this ceiling.
#
# BOUNDS, `{-5, 0}` nElo, `--nonreg`, alpha = beta = 0.05, 8+0.08, Hash 16, the
# UHO book, `-repeat`. Decided by the owner on 2026-09-08, the step file's own
# recommendation. No gain is claimed, so `{0, 5}` cannot contain the effect and
# random-walks to the cap (S068 spent 6 h 36 m that way); `{-5, 0}` straddles
# it and is the harness's mode for a constant moved for another reason.
#
# COST, DEC-143, at the 2277 games an hour S198 measured on this machine on
# 2026-09-08. D / (e1 - e0)^2 with D = 1046535 gives 41861 games with the truth
# at the midpoint, 18.4 h; 25591 games and 11.2 h with it on a bound.
#
# ABORT RULE. `--nonreg`'s 20000 rounds cap the run at 40000 games, about
# 17.6 h, which is below the midpoint case, so a run that reaches the cap ends
# as "no verdict" by construction and is read as one. The watcher's hard
# ceiling is 36 h. The run is stopped early for exactly two things, both
# findings and not results: a forfeit rate over 1.0 % on a side
# (`python3 tools/forfeit_report.py <dir>/games.pgn --max-pct 1.0`), or an
# `Incomplete mating PV` line from the candidate that the reference does not
# match and that the budget evidence above does not explain.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> 4 is not a regression of 5 nElo or more (about 3.5 to 4.0
#                  logistic Elo; take the ratio off the run's own Elo and nElo
#                  lines). SHIP 4. The magnitude is not established and no gain
#                  is claimed: the mate counts are what the change is for --
#                  39 to 52 of 82 on the constructed set, 145 to 159 of 318 on
#                  the mined one, and the mate in four class promoted from a
#                  recorded MESSAGE to an asserted floor. The verdict is inside
#                  S151's scope, so the number is provisional until a control
#                  at least four times 8+0.08 has spoken.
#
#   H0 accepted -> 4 costs 5 nElo or more. REJECT it and KEEP 15, with the
#                  deep-mate loss recorded as the measured price of S085's
#                  ceiling: 1 of 16 mates in four and 0 of 16 in five at the
#                  shipping value, against 7 and 1 at the ceiling that was
#                  refused. No C2 = 6 fallback is bought -- the owner decided
#                  C1 alone on 2026-09-08 -- so a rejection ends the step.
#
#   No verdict   -> at the 40000-game cap, record it as zero (S005, S006,
#                  S015, DEC-103) and KEEP 15. The incumbent has a verdict
#                  behind it and the candidate is not inert in play, so
#                  "cannot tell" does not displace it. Decided by the owner on
#                  2026-09-08 with the alternative -- ship the challenger for
#                  its mate property on a null -- put to them and declined. No
#                  re-run at other bounds without a decisions.md entry.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.
#
#   nohup adocs/data/S148_sprt.sh > .tuning/sprt_s148.log 2>&1 &

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

exec ./fastchess.sh --nonreg
