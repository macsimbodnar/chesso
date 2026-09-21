#!/usr/bin/env bash
#
# S097 verdict 1, the singular extension: the one move a verification search
# says is much better than every alternative is searched a ply deeper. Judged
# against the tree immediately before the step's landing. Candidate is that
# landing commit -- `src/search.cpp` `negamax_at` gains an `excluded_move`
# parameter with the gates that belong to it, the verification search and the
# extension; `src/search_params.hpp` gains five constants, four settings and
# one switch. Nothing else in `src/` moves.
#
#   nohup adocs/data/S097_v1_sprt.sh > .tuning/sprt_s097_v1.log 2>&1 &
#
# TWO VERDICTS OFF ONE SEARCH, AND THIS IS THE FIRST. The step's accepts
# prices exactly two: the extension here, the multicut in
# `adocs/data/S097_v2_sprt.sh` against this landing. The multicut's code ships
# **inert** with this candidate behind `SeMultiCut`, whose default is 0 and
# whose off value is proved on the tree below rather than declared from the
# range's end (DEC-215). So what this run measures is the extension alone, and
# the second verdict is one default flip away from it -- which is the only
# shape under which the two numbers attribute (MEASUREMENT: one change at a
# time).
#
# DEC-221: the technique reached the implementer as prose -- the step file's
# own sections 1 to 8, written from publications and from the open-source
# record's published measurements -- and was implemented from that description.
# No other project's code was opened. Its constants are seeded in DEC-134's
# three forms and `src/search_params.hpp` says which form each takes:
# `SeMinDepth`, `SeTtDepthMargin`, `SePlyFactor` and `SeMarginPerDepth` are
# all **(c), the midpoint of a range stated by purpose**. The step file offers
# a (b) alternative for `SeMinDepth` -- profiling how often this engine's own
# stored move survives a full-width search at the same depth -- and it was
# **available and not taken**, the step's stamp records it, and S127 refits
# the four.
#
# **What the published record is worth here is direction and nothing else**
# (DEC-019). This project's own published-to-measured transfers read 0, 0,
# wrong sign, 0.10 and 0.33: staged move generation was quoted at 30 to 50 Elo
# and measured 0, SEE pruning in quiescence measured 0, capture ordering
# reported near 150 Elo measured slower, and three of the last five search
# verdicts were H0. The record's figures for this technique cluster from +11
# to +20 at engines between 2450 and 3300 and **none of them is an expectation
# here**; what they decided is that the technique is tried at all.
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON. **Which commit that is
# depends on S095's verdict, and this file is written before it is known**:
# H1 leaves the fifth reduction term in the tree, H0 removes it as a
# behaviour-neutral revert. This candidate's diff is independent of that term
# and rebases onto either tree, so the pair is "the landing and its parent"
# either way -- and the parent is pinned by the coordinator at landing time,
# from `git rev-parse`, never guessed. DEC-020 is the contamination that made
# attribution a rule: one run reported +301 Elo and meant nothing.
#
# `REF` and `CAND` are therefore `PIN_ME` below and **the script refuses to run
# unpinned** rather than defaulting to something plausible. `fastchess.sh`'s
# banner prints both shas with their commit dates before the first game, so
# what the run measures is on screen and never assumed.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019); they are here so
# that "the extension changes the search" is a measurement and not a claim, and
# so that the off value's equality is a number rather than an argument.
#
#   THE OFF VALUE, DEC-215, and it is the multicut's alone. `SeMultiCut` is a
#   switch whose 0 the release build compiles as a constant, so the branch
#   folds away and the binary this run plays holds no multicut at all. Proved
#   on the tree: the tune build at `SeMultiCut` 0 benches this candidate's
#   total to the node, and at 1 it does not.
#
#   THE EXTENSION'S OWN OFF VALUE IS `SeExtend` 0, DEC-215 clause 2. The four
#   settings have none between them and the step file's section 4 is wrong
#   about that -- it calls `SeMinDepth`'s range top the feature's off switch,
#   and the correction is kept beside it (clause 3): at 16 the rule still fires
#   at every depth above 16, which a game at this control reaches, and the only
#   reason it benches the parent's total is that `bench` stops at depth 14. So
#   the block ships behind a switch whose range end really is off, and it is
#   proved on the tree with the full signature:
#
#     tune build at SeExtend 0   4579468, the parent's total **to the node**,
#                                with all eight `bestmove` replies identical to
#                                a build of `5c76ea9` itself
#                                (.tuning/coord/S097_bench_extend_off.log,
#                                .tuning/coord/S097_bench_parent.log)
#
#   That is what makes the H0 reading below a default flip rather than a
#   whole-commit revert.
#
#   BENCH, taken 2026-09-21 on this machine, idle, before a game was played:
#     chesso bench   parent `5c76ea9`               4579468
#                    candidate                      5066204, +10.63 %
#                    tune build at SeMultiCut 0     5066204, the candidate's
#                    own total **to the node**, and with the full signature:
#                    all eight `bestmove` replies identical
#                    tune build at SeMultiCut 1     4493659, so the switch is
#                    live and verdict 2 has something to measure
#
#   The candidate's tree is **larger** at a fixed depth and that is the
#   technique and not a regression: a verification search at half depth runs at
#   every eligible node and the move it calls singular is searched a ply
#   deeper. This is the first entry in the bench ledger that rises for a reason
#   other than a reordering.
#
#   tools/search_bench.py, parent -> candidate:
#     depth 9   midgame   21479 ->  21479, g5f6      identical
#               kiwipete 102462 -> 102462, e2a6      identical
#               tactical  33148 ->  33148, d7c8q     identical
#     depth 12  midgame  154423 -> 154388, c3d5
#               kiwipete 578047 -> 459115, d5e6 -> **e2a6**
#               tactical 173394 -> 239314, d7c8q
#
#   **Depth 9 is byte-identical on all three positions**, which is the depth
#   gate measured rather than argued: `SeMinDepth` is 10 and no node in a
#   depth-9 iteration is deep enough to verify anything.
#
#   THE NODE-EXPLOSION CHECK, and it is this step's own instrument
#   (`adocs/data/S097_fixed_node_depth.py`). An extension trades nodes for
#   depth by construction, so a smaller or larger `bench` total says nothing
#   about whether the trade is a good one. What can say something before the
#   games is the depth reached at a fixed budget: `go nodes 1000000` over the
#   three `search_bench` positions, before and after. **A depth that falls on
#   every position is the explosion signature** and is a reason to look at the
#   caps -- `ply < SePlyFactor * depth`, one ply once per node, no recursive
#   exclusion -- before spending a night on a match.
#
#     position    parent depth   candidate depth   best move
#     midgame          17              16            c3d5 both
#     kiwipete         13              13            d5e6 -> e2a6
#     tactical         17              15            d7c8q both
#     total            47              44
#
#   **Two of three lose depth, one of them by two plies**, and that is the cost
#   side of the trade stated before the games rather than after them. It is not
#   the signature this paragraph defines -- a fall on *every* position -- and
#   kiwipete holds its depth while changing its move, which is the shape the
#   rule is for. The seeds were swept on the tune build for context
#   (`.tuning/coord/S097_seed_ablation.log`): the cost is dominated by
#   `SeMarginPerDepth`, 6639177 at the range floor against 4633274 at the top,
#   so the seed at 9 sits mid-slope where a midpoint should.
#
#   Node counts move by construction here, so INV-6's discharge is not
#   available and games are the only thing that can decide this step.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair, because the step asks for a gainer**:
# S097's accepts names `{0, 5}` nElo per change and named it before a line of
# this was written. DEC-063 sizes bounds from the expectation and there is no
# expectation to size this from that transfers -- see the paragraph above on
# what published figures have been worth here.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices `{0, 5}` at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1,
# `adocs/plan.md`'s cost table). Budgeted at **2110 games an hour**, which is
# `.moltke.local.md`'s standing figure for `8+0.08` on `noob_3moves.epd` under
# this governor: **19.8 h** and **12.1 h**. An effect outside the interval in
# either direction ends far sooner -- the ledger's verdicts mean under five
# hours -- but the wall is what is budgeted and not the mean (DEC-155): this is
# four hours or more, so it is a night the machine is given whole, and the
# coordinator schedules it.
#
# **The extension costs nodes per node, which this budget does not price.** A
# verification search at half depth runs at every eligible node, so the games
# an hour this run actually gets may sit below the standing figure. The
# throughput is read from the run's own banner and recorded; a figure below
# 1900 an hour is a fact about the candidate and not an abort.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). Everything `fastchess.sh` already
# does; nothing here overrides it but the output directory. **No harness change
# since the last fixed-rounds A/A** -- same fastchess alpha 1.8.1
# 20260720-daa3ea2, same book, same adjudication, same machine as the
# 2026-09-12 calibration -- so DEC-143's A/A clause owes nothing before this
# run. If any of the four moves before it starts, the A/A comes first.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot: two runs had to be
# relaunched for it. `.tuning/` is gitignored and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright and
# `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212). Mains, and a second
# load on the machine. **Nothing else is an abort** -- in particular a slow
# games-an-hour figure is not one, and neither is a long walk near the bounds,
# which is what a truth inside the interval looks like (S068 run 1).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **No defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** Three test-side findings are, all
# filler behind the next strength step, all named before a game is played:
#
#   1. S231 phase one labelled `3a649c0`'s node-budget count 17321 where the
#      byte-identical reverted tree reads 16256. One of the two readings was
#      taken wrong; the band in `tests/test_search.cpp` is derived from the
#      second and the first is a label in a done step file, so nothing in the
#      tree depends on it.
#   2. `adocs/data/S192_node_budget.py`'s own drift line reported OUTSIDE for a
#      band it had just derived, which is a defect in that line and not in the
#      band.
#   3. `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pins depth 6 while its own script,
#      `adocs/data/S231_research_witness.py`, answers depth 4 on this tree.
#      S231 moved that pin to 4 and its H0 revert took the move back with
#      everything else. The case is green either way.
#
# None of the three can move a reported score, move or line, and none is
# touched inside this landing. `adocs/plan.md`'s Open list carries no other
# filler behind this entry at the time this file is written; if one is opened
# before the run starts it is named here by id and reach before a game is
# played.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the extension gains at least 5 nElo over the tree without
#                   it. **Keep it, at the seeds it was measured at.** The
#                   stopping run's Elo is upward-biased and is not the effect
#                   size (DEC-063); what may be written is "at least 5 nElo".
#                   The four constants stay seeds and are refitted by S127's
#                   lane and not by this step. **The multicut is then measured**
#                   -- `adocs/data/S097_v2_sprt.sh` against this landing, one
#                   default flip, with its own guard case, its mined mate row
#                   and its three mutants landing with the flip.
#   H0 accepted  -> the extension does not gain 5 nElo over the tree without
#                   it. The accepts then binds: the zero is recorded as a zero
#                   and **the extension is switched off** -- `SeExtend`'s
#                   default moves from 1 to 0, one line, and the tree is this
#                   reference's again, proved by the bench signature above
#                   rather than argued (INV-6). The code, its cases and its
#                   mutants stay in the tree at the off value only if the step
#                   records a reason to keep them; the default reading is that
#                   a rule nothing measures is a rule nobody removes later, so
#                   the flip is followed by the removal in the same step unless
#                   the coordinator states otherwise.
#                   **Whether the multicut is still worth a night on its own is
#                   then a decision and not an automatic yes**, and the step
#                   records it: the multicut needs the verification search the
#                   extension pays for, so an H0 on the extension prices that
#                   search at zero and the multicut would have to pay for it
#                   alone. The recommendation written here, before the number:
#                   **drop both and record the zero**, unless the run's own
#                   reading is a walk near the bounds rather than a measured
#                   loss -- the multicut's published figures are +5.7 and +6.2
#                   at engines three hundred points above this one, one engine
#                   dropped it at this band, and a feature whose carrier
#                   measured zero starts from behind. S005, S006 and S015 are
#                   the precedent that a measured zero is recorded as a zero and
#                   the feature may still be kept with the reason stated;
#                   DEC-194 is the precedent for not keeping one whose interval
#                   sits below the bound.
#   No verdict   -> record as zero and decide with the reason stated, not
#                   implied. S068's run 1 is the precedent for a `{0, 5}` pair
#                   random-walking when the truth sits inside the interval, and
#                   the reading then is the interval, not the point estimate.
#                   DEC-063: a stalled walk is terminated and recorded, and the
#                   default at a zero is to drop, with the reason stated.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.
# COMMITS as amended by DEC-220: the commit that closes this verdict carries
# fastchess's own result block before its `Bench:` line, and `tools/gate.sh`
# refuses a block whose shas do not match the named log's result line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# REF is the commit S097's verdict-1 landing sits on -- the tree with no
# excluded-move plumbing, no verification search and no extension. CAND is the
# landing commit itself. Both are pinned by the coordinator after the landing
# commit is made, by editing the defaults below; `REF` and `CAND` in the
# environment override for any follow-up leg.
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD~1   # check it is REF
#
# Until both are pinned this script refuses: a candidate guessed from `HEAD` is
# a candidate nobody checked, and DEC-020 is what that costs.
# **Pinned 2026-09-21: REF is `5c76ea9`**, "Complete S095: the no-table-move
# reduction term ships on an H1", whose `bench` is 4579468 -- the commit this
# landing sits on, pinned by the coordinator before the landing existed. CAND
# is pinned the same way after the landing commit is made.
REF="${REF:-5c76ea9}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S097_v1_sprt.sh before running (CAND is the landing" \
         "commit, REF the commit it sits on)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s097_v1_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
