#!/usr/bin/env bash
#
# S109, the shallow-depth pruning block. Candidate is the working tree at the
# commit the coordinator makes for this step: `src/search.cpp` grows four rules
# over quiet moves in `negamax_at`'s move loop -- late move pruning, futility,
# history pruning and quiet SEE -- all gated on the reduction-adjusted depth
# `lmr_depth = max(0, depth - lmr_reduction(depth, move_number))`, plus S108's
# deferred layer (c), the table score as the futility margin's input.
# Reference is `HEAD`, which is what `fastchess.sh` defaults to and prints with
# its date before the first game (S160).
#
#   nohup adocs/data/S109_sprt.sh > .tuning/sprt_s109.log 2>&1 &
#
# ONE RUN FOR FOUR RULES, AND THE FORFEIT IS ON THE RECORD. DEC-082 as narrowed
# by DEC-087: per-part attribution is deliberately given up here. The parts
# prune overlapping sets of moves, so each measured against a tree the other
# three are absent from returns a number that is not the number it will have in
# the tree that ships -- Stockfish's own removal test prices move-count pruning
# at ~0 alone and the block it belongs to at ~204. Four verdicts on effects
# Lynx measured near +4.7 each would sit inside the bounds and crawl (DEC-063).
# What replaces attribution is the bisection protocol at the bottom of this
# file, which runs only if the block fails.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at fixed depth are
# not Elo and this file does not read them as Elo (DEC-019); they are here so
# that "the block prunes" is a measurement and not a claim.
#
#   tools/search_bench.py, depth 9, before -> after
#     midgame    121515 ->  47635   kiwipete  801408 -> 213916
#     tactical    72895 ->  26130   best moves unchanged: c3d5 / e2a6 / d7c8q
#   tools/search_bench.py, depth 12, before -> after
#     midgame    638719 -> 141455   kiwipete 3514653 -> 1038779
#     tactical   341715 -> 175684   kiwipete's best move moves e2a6 -> d5e6;
#                                   the other two are unchanged
#   chesso bench   27322394 -> 7111579, -74.0 %
#
#   Per rule, over the 300-position stratified pick at depth 10, each rule
#   alone against all four off (adocs/data/S109_lmp_census.py sweep):
#     all off   57276904 nodes                  0 of 300 best moves changed
#     Lmp       24675444  0.4308               72
#     Fut       42250613  0.7377               13
#     HistPrune 56807807  0.9918                3
#     SeeQuiet  45805722  0.7997               49
#     all on    20321365  0.3548               65
#
#   **History pruning is nearly inert at its first setting** -- 0.8 % of the
#   nodes and 3 of 300 best moves -- and that is expected rather than
#   surprising: DEC-194 reverted S024's continuation history, so the sum this
#   rule reads is plain butterfly history alone and most entries at a fresh
#   node are zero. The block's verdict therefore prices late move pruning,
#   futility and quiet SEE, with history pruning along for the ride. S222 is
#   where the other term returns, fitted.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. The gainer pair, because this is a gainer: the published
# record prices the block far above the bounds (Stockfish's 2019 removal test
# at ~204 Elo, its 2021 post-NNUE re-run at +98.63 against its in-block parts'
# +26 summed) and the plan's own expectation is +40 to +120 self-play. DEC-063
# is the rule that sizes bounds from the expectation, and an effect of that
# size sits far outside {0, 5} rather than inside it.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound: **19.8 h** and **12.1 h** at the 2110 games an
# hour measured on this workstation at 8+0.08 on `noob_3moves.epd`
# (`.moltke.local.md`, S219's DEC-143 A/A of 2026-09-12). If the effect is
# outside the interval in either direction -- which is what the record above
# expects -- it is far less: the ledger's four fast runs mean 2 h 28 m. Budget
# the night, not the mean (DEC-155).
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050). Everything `fastchess.sh` already does;
# nothing here overrides it.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN).
# A crash or a disconnect on either side voids it outright and `fastchess.sh`
# says so itself (`SPRT-RUN-INVALID`, S212).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# Three steps hold defects that are scheduled rather than fixed, and none of
# them is reachable in ordinary play in a way that can move this verdict:
#   S210  2026-09-10_adversarial-F17 to F23, 2026-09-04_adversarial-F01 to F03
#         -- protocol and rules edge cases: the halfmove clock's wrap, a full
#         history, `go infinite`, `movestogo 0`, the first iteration's stop,
#         quiescence scoring a dead position, one comment. F19 and F20 are
#         inputs no harness here sends; F22 alters the tree and is counted
#         before it is booked.
#   S223  2026-09-12_adversarial-F01 -- `load_FEN` accepts a placement with the
#         wrong number of kings or the side not to move in check. Reachable
#         only through a `position fen` line no harness sends: fastchess sends
#         `position startpos moves ...`.
#   S213  2026-09-10_adversarial-F26, F27, F33 -- stale comments, two dead
#         public entry points and two `<cctype>` calls on a signed char. None
#         alters a reported score, move or line.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> the block gains at least 5 nElo. Keep all four rules. The
#                  stopping run's Elo is upward-biased and is not the effect
#                  size (DEC-063); what may be written is "at least 5 nElo",
#                  and the magnitude is the block boundary's longer control to
#                  state (S151, DEC-202 -- this verdict moves pruning
#                  parameters, so its class owes that reading once per block
#                  and not once per verdict).
#   H0 accepted -> the block costs. **Bisect, do not revert** (DEC-082): the
#                  failing block is evidence that one part is wrong, which is
#                  the attribution question the block form was not asked.
#                  Halves first, by evidence strength, each by setting a cap to
#                  its off value in `src/search_params.hpp` and rebuilding
#                  Release -- never by measuring the tune build (S073):
#                    leg 1  HistPruneMaxLmrDepth=0 and SeeQuietMaxLmrDepth=0,
#                           the two weakest sub-3000 records (+2.06 and none)
#                    leg 2  LmpMaxLmrDepth=0 and FutMaxLmrDepth=0 instead
#                  One further run inside the failing half isolates the rule:
#                  three runs worst case against four one-at-a-time. First
#                  suspects, and this file names them before the run rather
#                  than after: **late move pruning's count**, which this step's
#                  own census put at 7.33 moves flat where every published
#                  sub-3000 form is looser and which the sweep above shows
#                  changing the best move on 72 of 300 positions at depth 10;
#                  and the gates' axis, since the one engine that A/B'd
#                  lmr-depth gating near this band measured it negative twice
#                  for history pruning (`HistPruneMaxLmrDepth` keeps the raw
#                  depth variant one edit away).
#   No verdict   -> record as zero and decide with the reason stated, not
#                  implied. A block that cannot be separated from zero at these
#                  bounds after 41861 games is a block whose parts are not
#                  where the published record says they are, and the bisection
#                  above is then the diagnosis rather than the appeal. S005,
#                  S006, S015 and DEC-103 are the precedent for keeping a
#                  measured zero; DEC-194 is the precedent for not keeping one
#                  whose interval sits below it.
#
# WHAT IS NOT IN THIS RUN. The gives-check exemption on late move pruning is
# **in** it, and that is S109's own finding rather than its plan: the accepts
# shipped the published form without the exemption and DEC-180 reserved the
# exemption to S218 *unless* this step's own mate case went red without it. It
# did -- `4K3/q7/8/4k3/8/8/8/8 b`, mate in two, lost at depth 3 at every count
# below 30 moves, which is a count that never fires -- so the exemption is this
# step's fix under the TESTS rule and S218 folds into it. DEC-180's own words:
# a red guard is a bug, not an option. S218 is re-scoped by the coordinator.
#
# OPEN WHILE THIS RUNS, added 2026-09-13 before launch: S225, the STAGES
# environment leak in tools/gate_extra.sh found by this step's second-tier
# gate -- a harness-test defect, not reachable in play (DEC-171).
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE REFERENCE IS THE COMMIT BEFORE THE BLOCK, 50e3661 ("Start S109"), pinned
# here because this script runs after the landing commit and `REF=HEAD` would
# then name the block itself, which the A/A guard refuses. The candidate is the
# working tree at the landing commit, rebuilt after it so its id name carries
# that sha (S212's identity check). REF in the environment overrides for a
# bisection leg, which plays a rebuilt working tree against the same 50e3661.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-50e3661}" ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
