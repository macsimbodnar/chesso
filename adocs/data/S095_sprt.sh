#!/usr/bin/env bash
#
# S095, one more ply of late move reduction at a node whose transposition-table
# entry carries no move, judged against the tree immediately before the step's
# landing. Candidate is that landing commit: `src/search.cpp`
# `lmr_node_adjustment` gains a fifth term, `src/search_params.hpp` gains
# `LmrNoTtMove` (default 1, range 0 to 2), and the one call site in `negamax`
# hands it `tt_move == 0`. Nothing else in `src/` moves.
#
#   nohup adocs/data/S095_sprt.sh > .tuning/sprt_s095.log 2>&1 &
#
# THE STEP WAS RE-FORMED BEFORE IT WAS IMPLEMENTED (DEC-222 clause 2). S095 was
# written in 2026-08-19 as a **node-level depth cut** -- a node with no table
# move searched a ply shallower before the pruning block -- which is the form
# the published record describes. The 2026-09-19 analysis found that the
# open-source record which priced that cut later measured it against a **term
# inside the reduction** and kept the term, removing the cut as a free simplification.
# The owner chose the surviving form; chesso already had the site, since S098
# verdict 2 put four node-type terms there. The node-level cut stays available
# as a later step with its own file if this reads H0.
#
# **What that record measured is direction and nothing else** (DEC-019). This
# project's own published-to-measured transfers read 0, 0, wrong sign, 0.10 and
# 0.33; staged move generation was quoted at 30 to 50 Elo and measured 0, SEE
# pruning in quiescence measured 0, and capture ordering reported near 150 Elo
# measured slower. No figure from any engine is an expectation here and none of
# them seeds anything: `LmrNoTtMove` is **DEC-105 (c)**, the midpoint of the
# declared 0 to 2, and that every traced introduction of the older form also
# shipped one ply is a coincidence of that arithmetic (DEC-084 as amended by
# DEC-105, DEC-134). DEC-221: the technique reached the implementer as prose
# from the analysis of literature and open-source resources and was
# implemented from that description.
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON -- the tree with the four
# terms and without the fifth. That is **`b25452a`**, the head after S231's completion,
# and what sits under this step is therefore `3a649c0`'s `src/` byte for byte:
# S231's own gainer SPRT was running while this file was first written, it read
# **H0** (`Elo -2.65 +/- 4.82`, `nElo -3.40 +/- 6.20`, 12070 games)
# and its pre-registered revert landed (`bench` 4646334 to the node).
# So the two-ply continuation table is not in the tree this term is measured on
# and nothing of it is stranded underneath the reading.
#
# CAND is the landing commit and is **not** pinned here, because the file is
# written before that commit exists. The coordinator pins it after committing,
# and the script **refuses to run unpinned** rather than defaulting to something
# plausible: DEC-020 is the contamination that made attribution a rule, and a
# reference that is merely probable is how a run measures the wrong pair for six
# hours. `fastchess.sh`'s banner prints both shas with their commit dates before
# the first game, so what the run measures is on screen and never assumed.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019); they are here so
# that "the term changes the search" is a measurement and not a claim, and so
# that the off value's equality is a number rather than an argument.
#
#   THE OFF VALUE, DEC-215. `LmrNoTtMove` is a single addend, so at 0 the sum
#   is the four-term sum and both consumers -- the reduction a late quiet is
#   searched with, and S109's shallow-depth gate -- see exactly what they saw
#   before this step. Proved on the tree and not declared from the range's end:
#   the tune build at `setoption name LmrNoTtMove value 0` benches the parent's
#   total to the node.
#
#     chesso bench   parent 4646334 (`b25452a`, which is `3a649c0`'s src)
#                    tune build at LmrNoTtMove 0: **4646334**, the parent's
#                    total to the node, and a Release rebuild with the X-macro
#                    default forced to 0 prints the same -- the off value is
#                    proved on the tree twice over and in the build that ships
#                    Release build at the seed 1: **4579468**, 1.4 % below the
#                    parent. The term reduces one ply more at every node the
#                    table has no move for, and the tree it leaves is smaller
#   tools/search_bench.py, depth 9, parent -> seed
#     midgame       21995 ->  21479, g5f6      kiwipete 104682 -> 102462, e2a6
#     tactical      29842 ->  33148, d7c8q
#   tools/search_bench.py, depth 12, parent -> seed
#     midgame      155612 -> 154423, c3d5      kiwipete 683624 -> 578047,
#     tactical     152138 -> 173394, d7c8q     kiwipete's best move moves
#                                              `e2a6` -> `d5e6`
#
#   Two positions shrink and one grows at both depths, which is the ordinary
#   signature of a reduction change rather than of a saving, and one best move
#   moves at depth 12.
#
#   Node counts move by construction at the seed, so INV-6's discharge is not
#   available and games are the only thing that can decide this step.
#
#   Every number above was taken on this machine, idle, after S231's verdict
#   landed and before a game of this run was played.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair, because the step asks for a gainer**:
# S095's accepts names `{0, 5}` nElo and named it before a line of the term was
# written. DEC-063 sizes bounds from the expectation and there is no expectation
# to size this from that transfers -- see the paragraph above on what published
# figures have been worth here.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices `{0, 5}` at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1,
# `adocs/plan.md`'s cost table). Budgeted at **2110 games an hour**, which is
# `.moltke.local.md`'s standing figure for `8+0.08` on `noob_3moves.epd` under
# this governor: **19.8 h** and **12.1 h**. The S098 family's five verdicts
# measured 2119 to 2128 games an hour on a 4.6 M node tree, which is the same
# figure to a tenth. An effect outside the interval in either direction ends far
# sooner -- the ledger's verdicts mean under five hours and S091's took 38
# minutes -- but the wall is what is budgeted and not the mean (DEC-155): this
# is four hours or more, so it is a night the machine is given whole, and the
# coordinator schedules it.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). Everything `fastchess.sh` already
# does; nothing here overrides it but the output directory. **No harness change
# since the last fixed-rounds A/A** -- same fastchess alpha 1.8.1
# 20260720-daa3ea2, same book, same adjudication, same machine as the
# 2026-09-12 calibration -- so DEC-143's A/A clause owes nothing before this
# run.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default is
# under `/tmp`, which this machine wipes at boot: two runs had to be relaunched
# for it. `.tuning/` is gitignored and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright and
# `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212). Mains, and a second
# load on the machine. **Nothing else is an abort.**
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **No defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** Two test-side findings are, both
# filler behind the next strength step, both named before a game is played.
# S231's own two are gone with its revert: `I03_null_child_drops_prev2` left
# the tree with the mutant file, and the node-budget golden this file first
# named as deferred was re-derived on the reverted tree (65024 / 3251 from a
# count of 16256) by the script DEC-142 asks for. What is left is what that
# re-derivation exposed:
#
#   1. S231 phase one labelled `3a649c0`'s node-budget count 17321, and the
#      byte-identical reverted tree reads **16256**. One of the two readings
#      was taken wrong; the band is derived from the second and the first is a
#      label in a done step file, so nothing in the tree depends on it.
#   2. `adocs/data/S192_node_budget.py`'s own drift line reported OUTSIDE for a
#      band it had just derived, on the reverted tree, which is a defect in
#      that line and not in the band. On this candidate the same script reads
#      the count at 22217 against a middle half of [18694, 49581] and says
#      inside, so DEC-142's trigger has not fired here and the golden is read
#      and not moved.
#   3. Found by this step and named here rather than repaired: the case
#      `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pins depth 6 while its own script,
#      `adocs/data/S231_research_witness.py`, answers depth 4 on this tree and
#      on the parent alike -- S231 moved that pin to 4 and its H0 revert took
#      the move back with everything else. The case is green either way.
#   4. **The one that has to be settled before this run can be launched at
#      all**, because it is what keeps the landing from being green:
#      `tests/test_mate_carry.cpp` reports 11 short mating PVs of 22 mate lines
#      on `E_mate_minus9` against a ceiling of 9. The grid its protocol asks
#      for was re-taken on this tree (`adocs/data/S095_sweep_block.txt`) and
#      the ceiling rule's answer over all four recorded grids is A 5, B 15,
#      C 0, D 2, E 11, F 5. Raising a ceiling is a decision (DEC-162), the
#      decision is the owner's or the coordinator's, and this run is launched
#      only once it has been taken and the suite is green.
#
# Neither can move a reported score, move or line, and neither is touched
# inside this landing: DEC-142's trigger is **read** here -- the count is taken
# on the tree this run measures and recorded in the step -- and a re-derivation
# that moved a test's numbers inside a commit an SPRT is about to judge is
# exactly what S231's header refused to do.
#
# `adocs/plan.md`'s Open list carries no other filler behind this entry at the
# time this file is written; if one is opened before the run starts it is named
# here by id and reach before a game is played.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the term gains at least 5 nElo over the tree without it.
#                   **Keep it, at the seed it was measured at.** The stopping
#                   run's Elo is upward-biased and is not the effect size
#                   (DEC-063); what may be written is "at least 5 nElo". The
#                   constant stays a seed and is refitted with the four terms
#                   beside it by S127's lane, not by this step. The node-level
#                   depth cut is **not** taken up afterwards: the study's own
#                   reading is that the two are alternatives, and a second
#                   verdict on the cut would be a new step with its own file and
#                   its own reason.
#   H0 accepted  -> the term does not gain 5 nElo over the tree without it. The
#                   accepts then binds: the zero is recorded as a zero and
#                   **the term is removed**, which is behaviour-neutral by
#                   construction here in a way most removals are not -- the off
#                   value already proved the tree is the parent's, bench
#                   signature included, so the removal commit is a revert whose
#                   equality is a measurement (INV-6) and not an argument.
#                   `LmrNoTtMove` leaves `src/search_params.hpp`, its golden row
#                   leaves `tests/test_search_params.cpp` and the two cases and
#                   three mutants leave with it. **The node-level depth cut
#                   then becomes available as a later step with its own file**,
#                   which is what the step's `excludes` line says. S005, S006
#                   and S015 are the precedent that a measured zero is recorded
#                   as a zero and the feature may still be kept with the reason
#                   stated; DEC-194 is the precedent for not keeping one whose
#                   interval sits below the bound.
#   No verdict   -> record as zero and decide with the reason stated, not
#                   implied. S005, S006, S015 and DEC-103 are the precedent for
#                   keeping a measured zero; S068's run 1 is the precedent for a
#                   `{0, 5}` pair random-walking when the truth sits inside the
#                   interval, and the reading then is the interval, not the
#                   point estimate.
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
# REF is the commit the landing sits on -- the tree with S098 verdict 2's four
# node terms and without S095's fifth. CAND is the landing commit itself.
#
# **Pinned 2026-09-20: REF is `b25452a`, the head after "Complete S231: the
# two-ply continuation history leaves on a measured zero"**, whose `src/` is
# `3a649c0`'s byte for byte and whose `bench` is 4646334. CAND is pinned by the
# coordinator after the landing commit is made, by editing the default below;
# `REF` and `CAND` in the environment override for any follow-up leg. Until
# CAND is pinned this script refuses: a candidate guessed from `HEAD` is a
# candidate nobody checked, and DEC-020 is what that costs.
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#
# Pinned <date>: CAND is `<sha>` "<subject>".
REF="${REF:-b25452a}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S095_sprt.sh before running (CAND is the landing" \
         "commit, REF the commit it sits on)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s095_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
