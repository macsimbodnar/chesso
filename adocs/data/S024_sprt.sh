#!/usr/bin/env bash
#
# S024 verdict 1, the one-ply continuation history (counter-move history).
# Candidate is the working tree; reference is PINNED to 25998fe, the step's
# parent, and not left to fastchess.sh's HEAD default. Two things moved HEAD
# after this run was first written -- the moltke v1 migration (DEC-109) and
# S024's own implementation commit -- and neither is a reference this run may
# measure against: the first is documents only and the second IS the candidate.
# 25998fe is the last commit before any of it. `git diff 25998fe HEAD -- src/
# tools/ CMakeLists.txt` over the migration alone was empty, so the reference
# build under .ref-builds/25998fe is the right binary and is already built.
#
# WHAT THE CANDIDATE DOES. Every quiet move that causes a beta cutoff now
# updates a second table beside the butterfly one, keyed on the pair (the piece
# that played the previous move, where it landed) x (this move's piece, where it
# lands); every quiet the node tried before the cutoff takes the matching malus
# in the same table. score_move sums the two tables at equal weight for a quiet.
# The countermove *heuristic* is untouched and still occupies its own band --
# the two coexist, and removing the table under continuation history is on
# record as a loss (Lynx PR #1563, -6.84, closed).
#
# The table is conditioned on there being a move to reply to, so the root and
# the child of a null move update and read nothing: negamax already passes 0 for
# both, which is the same guard the countermove band uses. The table is
# per-`go`, not carried across one -- S093 verdict 2 measured persistence for
# the butterfly table at Elo -1.65 +/- 4.22 and it was reverted (DEC-101), so
# there is no persistent struct for this one to join.
#
# MEASURED BEFORE THE GAMES, NOT ARGUED.
#
#   node counts, tools/search_bench.py against 25998fe, best move unchanged at
#   all three positions (c3d5 / e2a6 / d7c8q):
#
#     depth  9   121512 / 800769 / 62907  ->  122266 / 794014 / 63484
#     depth 13   944905 / 5228126 / 533227 -> 875013 / 5210372 / 701417
#
#   so the tree moves and INV-6's node-count discharge is not available: this
#   run is the only thing that decides the step.
#
#   throughput on this machine, from run 1's first 1 h 28 m on battery:
#   2276 games, about 1550 games/h at 8+0.08 on 8 cores. This is the figure
#   .moltke.local.md says arrives free from the first real SPRT. Read it as a
#   floor: it was measured on battery with the display on, and run 2 is on AC.
#
#   throughput, two interleaved passes at depth 11, kiwipete as the position
#   with enough work to read: 9655 / 9508 knps reference against 9518 / 9558
#   candidate. Inside the noise this machine resolves (CLAUDE.md rule 5), so
#   the two extra dependent loads per scored quiet and the 1.125 MiB table are
#   not visibly paid for at bench scale. opendirectoryd was taking about 17 %
#   of a core while these were read.
#
# BOUNDS. fastchess.sh's default: elo0=0 elo1=5, alpha=beta=0.05. This is a
# gainer and the pair is the one DEC-063 keeps for a change with a positive
# prior. The prior is wide and that is the point of DEC-019: Weiss PR #477
# measured this exact technique at +44.68 STC / +33.95 LTC in 2021 and Lynx
# PR #645 measured the same technique at +2.16 STC / +9.12 LTC in 2024, the
# widest spread on this plan. The bounds sit at the low end for that reason.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> the one-ply table gains 5 Elo or more. Keep it and commit.
#                  The magnitude is NOT established: an early stop biases the
#                  point estimate upward (DEC-063, S068 is the case where a
#                  pooled estimate fell from +12.18 to +5.02). Verdict 2, the
#                  two-ply follow-up, is then measured against this commit.
#   H0 accepted -> the table does not gain 5 Elo. Do not keep it on the
#                  strength of the published record. Two named candidate
#                  explanations to check before reverting, in this order: the
#                  malus is on record as load-bearing (Lynx PR #645 measured
#                  CMH without one at -12.90 STC / -17.38 LTC, H0), and this
#                  one is applied -- so the live suspect is the sum, since
#                  chesso's countermove band sits ABOVE the summed quiet band
#                  and the published engines this is taken from have no such
#                  band at all. A revert is the default; a second shape is a
#                  new verdict, not a re-run of this one.
#   No verdict   -> record as zero. The keep-or-revert is then a judgement and
#                  the reason is stated either way (S005, S006, S015, DEC-103
#                  are the precedent for keeping a measured zero). What argues
#                  for keeping: S109's history pruning, S098's reduction and
#                  S111 all read this sum, and the table is the input they are
#                  fitted against. What argues for reverting: a zero here means
#                  1.125 MiB and two dependent loads bought nothing, and the
#                  two-ply table would then be measured on top of dead weight.
#
# RUN 1 WAS ABORTED AND THIS IS RUN 2. Nothing about the candidate, the
# reference or the bounds changed; the machine did.
#
# Run 1 started 2026-08-27 22:33:38 on battery. `pmset -g log`:
#
#   2026-08-28 00:01:37  Entering Sleep state due to 'Low Power Sleep' ...
#                        Using Batt (Charge:1%)
#   2026-08-29 17:56:14  Wake from Hibernate ... Using AC (Charge:100%)
#
# so it played for 1 h 28 m, hibernated for about 42 hours with the engines and
# fastchess still resident, and resumed. It was killed at 2549 games rather
# than allowed to reach a bound. Two things were measured before deciding that,
# with `adocs/data/S024_pair_stats.py` over the run's own PGN, and the tool
# reproduces fastchess's printed figures exactly for the same sample:
#
#   1. THE EIGHT TIME LOSSES ARE NOT WHY. All eight sit in rounds 1133 to 1137,
#      the sleep boundary, four in each direction. Dropping all five pairs moves
#      the match from Elo -0.41 +/- 10.51 to -0.68 +/- 10.51 over 1274 pairs.
#      0.27 Elo against a +/- 10.5 interval, and shrinking with the sample. On
#      their own they are a footnote and the run would have been kept.
#
#   2. THE TWO HALVES DO NOT LOOK LIKE THE SAME EXPERIMENT, and that is why.
#
#        rounds 1-1132, battery 78 % falling to 1 %   1132 pairs  -5.06 +/- 11.17
#        rounds 1138 on, AC                            137 pairs  +35.63 +/- 30.99
#
#      About 40 Elo apart, roughly 2.4 sigma, and the mean game length moved
#      with it: 100.0 plies before the sleep against 92.9 after. Two
#      explanations fit -- chance at 137 pairs on a split that was gone looking
#      for, or a machine throttled at a 1 % battery playing a different time
#      control in effect -- and this run cannot separate them. A verdict pooled
#      over both blocks would be a number whose regime is unstated, which is
#      what DEC-020 is about. The evidence is kept:
#      `.tuning/sprt_s024_v1_run1_aborted.log` and
#      `.tuning/sprt_s024_run1_aborted.pgn`.
#
# Run 2 is on mains power and holds the machine awake for the duration.
# `caffeinate -i` prevents idle sleep and `-s` prevents sleep while on AC; the
# match is the payload, so both die together and nothing outlives the run.
# CONCURRENCY is unchanged at all 8 cores (AGENTS.md par.0, DEC-048).
#
# AGENTS.md par.12 and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.
#
#   nohup adocs/data/S024_sprt.sh > .tuning/sprt_s024_v1.log 2>&1 &

set -uo pipefail

cd /Users/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# The POWER rule: a timed match never starts on battery. S024's first run is
# what wrote that rule, so this refuses rather than trusting the operator.
#
# Both tests are written as the NEGATIVE, because the negative is the string
# that was actually observed on this machine -- `pmset -g ac` prints "No
# adapter attached." and `pmset -g batt` prints "Now drawing from 'Battery
# Power'". Matching a positive would mean guessing at output nobody has seen
# here, and guessing wrong would refuse every run with the charger plugged in.
if pmset -g ac 2>/dev/null | grep -qi 'no adapter attached' ||
   pmset -g batt 2>/dev/null | grep -qi "battery power"; then
  echo "SPRT-RUN-FAILED: on battery, no mains adapter; see the POWER rule" >&2
  exit 1
fi

REF=25998fe exec caffeinate -is ./fastchess.sh
