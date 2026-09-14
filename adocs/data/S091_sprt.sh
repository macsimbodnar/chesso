#!/usr/bin/env bash
#
# S091, capture SEE pruning in the main search and the extra reduction for a
# move the exchange evaluation writes off. Candidate is the working tree at the
# commit the coordinator makes for this step: `src/search.cpp` grows one skip
# over captures in `negamax_at`'s move loop, gated on the same
# `lmr_depth = max(0, depth - lmr_reduction(depth, move_number))` S109's four
# quiet rules are gated on, and one extra ply of reduction for any move --
# capture or quiet -- whose exchange evaluation is under zero, inside late move
# reduction's own eligibility.
#
#   nohup adocs/data/S091_sprt.sh > .tuning/sprt_s091.log 2>&1 &
#
# ONE RUN FOR TWO RULES, AND THE FORFEIT IS ON THE RECORD. The step's accepts
# says "an SPRT verdict", singular, and the plan's multi-verdict list does not
# name S091. The two rules act on overlapping moves -- a late losing capture is
# skipped by the first at shallow reduced depth and reduced by the second past
# the first's cap -- so each measured against a tree the other is absent from
# returns a number that is not the number it will have in the tree that ships.
# What replaces attribution is the bisection protocol at the bottom of this
# file, which runs only if the pair fails.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at fixed depth are
# not Elo and this file does not read them as Elo (DEC-019); they are here so
# that "the rules fire" is a measurement and not a claim.
#
#   tools/search_bench.py, depth 9, before -> after
#     midgame     47635 ->  74327   kiwipete  213916 -> 146873
#     tactical    26130 ->  27855   best moves unchanged: c3d5 / e2a6 / d7c8q
#   tools/search_bench.py, depth 12, before -> after
#     midgame    141455 -> 172303   kiwipete 1038779 -> 596588
#     tactical   175684 -> 190823   kiwipete's best move moves d5e6 -> e2a6;
#                                   the other two are unchanged
#   chesso bench   7105111 -> 6267842, -11.8 %
#
#   **A skip does not always shrink the tree, and the sweep below says which
#   rule does what rather than leaving it to be argued.** The midgame position
#   costs 33 % more nodes at depth 12 and kiwipete 43 % fewer; per rule it is
#   the *skip* that adds midgame's nodes -- a node that skips a capture has lost
#   a move that might have cut it off and searches more of what is left -- and
#   the extra ply that takes them off. The three positions disagree in both
#   directions, which is another reason the verdict is games and not nodes.
#
#   Per rule, each alone against both off, the three search_bench positions at
#   depth 12 through the tune build (adocs/data/S091_rule_sweep.txt; the tune
#   build is never SPRT'd, S073, and this is a node count and not Elo):
#     both off    141455 / 1038779 / 175684
#     skip only   187956 /  666527 / 204486
#     extra only  133938 /  714841 / 128080
#     both on     172303 /  596588 / 190823
#
#   Neither rule is inert and neither dominates: the skip takes 36 % of
#   kiwipete's tree and *adds* a third to midgame's, the extra ply takes 31 %
#   of kiwipete's and 27 % of tactical's and takes 5 % off midgame's, and the
#   pair together is below both on kiwipete and between them on the other two.
#   Only `both off` reports a different best move -- `d5e6` against `e2a6` on
#   kiwipete -- and every other cell agrees, at both depths.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. The gainer pair, because this is a gainer: Lynx #1521
# measured a main-search capture SEE skip at **+9.14 +/-4.55 at 8+0.08**, this
# regime exactly, and Stockfish's 2021 removal re-run prices "SEE based pruning"
# at +9.10. DEC-063 is the rule that sizes bounds from the expectation, and an
# effect of that size sits above {0, 5} rather than inside it. The reduction
# half is the suspect: Lynx #2254, the same shape on quiets alone, closed at
# **-5.04 +/-4.33**, which is why the bisection below names it first.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1,
# `adocs/plan.md`'s cost table): **19.8 h** and **12.1 h** at the 2110 games an
# hour `.moltke.local.md` says to budget from, or 19.5 h and 11.9 h at the
# 2150 the five runs since S212 average. If the effect is outside the interval
# in either direction -- which is what the record above expects -- it is far
# less: the ledger's fast runs mean about 2 h 30 m. Budget the night, not the
# mean (DEC-155).
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run (DEC-195). Everything `fastchess.sh` already does;
# nothing here overrides it but the output directory.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default is
# under `/tmp`, which this machine wipes at boot: two runs had to be relaunched
# for it. `.tuning/` is gitignored and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN).
# A crash or a disconnect on either side voids it outright and `fastchess.sh`
# says so itself (`SPRT-RUN-INVALID`, S212).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# None of them is reachable in ordinary play in a way that can move this
# verdict:
#   S225  the STAGES environment leak in `tools/gate_extra.sh` -- a harness-test
#         defect, not reachable in play.
#   S228  a fast-suite test that proves the `id name` build stamp follows the
#         tree without a reconfigure (2026-09-12_plan_adversarial-F02,
#         DEC-206) -- a harness gap, no `src/`.
#   S229  the nlohmann/json dependency carried as its MIT single header instead
#         of a gitlink to a repository whose test tree is partly GPL-3.0 -- a
#         provenance defect in the build, no `src/`.
#   S213  2026-09-10_adversarial-F26, F27, F33 -- stale comments, two dead
#         public entry points and two `<cctype>` calls on a signed char. None
#         alters a reported score, move or line.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted -> the pair gains at least 5 nElo. Keep both rules. The
#                  stopping run's Elo is upward-biased and is not the effect
#                  size (DEC-063); what may be written is "at least 5 nElo".
#   H0 accepted -> the pair costs. **Bisect, do not revert.** Two legs at most,
#                  each by setting one value in `src/search_params.hpp` and
#                  rebuilding Release -- never by measuring the tune build
#                  (S073):
#                    leg 1  `SeeLmrExtra=0`, the extra reduction off, the skip
#                           left on. First because it is the half the published
#                           record scores negative (Lynx #2254) and the half
#                           whose node counts above show a re-search cost.
#                    leg 2  `SeeCaptureMaxLmrDepth=0`, the skip off, the extra
#                           reduction left on.
#                  Each off value is exact: every cap reads `lmr_depth < CAP`,
#                  and the extra ply is added only when `SEE_LMR_EXTRA > 0`.
#   No verdict   -> record as zero and decide with the reason stated, not
#                  implied. S005, S006, S015 and DEC-103 are the precedent for
#                  keeping a measured zero; DEC-194 is the precedent for not
#                  keeping one whose interval sits below it. S015 is the
#                  precedent this step's own file opens with: quiescence SEE
#                  pruning measured 0 here and was kept.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE REFERENCE IS THE COMMIT BEFORE THE CHANGE, pinned here because this script
# runs after the landing commit and `REF=HEAD` would then name the change
# itself, which the A/A guard refuses. The candidate is the working tree at the
# landing commit, rebuilt after it so its id name carries that sha (S212's
# identity check). REF in the environment overrides for a bisection leg, which
# plays a rebuilt working tree against the same commit.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-08461e0}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s091_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
