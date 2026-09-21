#!/usr/bin/env bash
#
# S132, the node-fraction time manager: the soft limit is scaled by the share
# of the root's own nodes the move it is about to play consumed, so a choice
# that was never in doubt does not buy the next iteration and a position whose
# nodes are spread does. Judged against the tree immediately before the step's
# landing. Candidate is that landing commit -- `src/search.cpp` `negamax`
# gains a per-root-move node snapshot at `ply == 0`, `src/data_structures.hpp`
# gains the buckets and their three accessors, `src/chesso.cpp` gains
# `search_time_node_factor_percent` and the block in
# `iterative_deepening_search` where the three scalers now meet, and
# `src/search_params.hpp` gains three settings. Nothing else in `src/` moves.
#
#   nohup adocs/data/S132_sprt.sh > .tuning/sprt_s132.log 2>&1 &
#
# ONE VERDICT OVER TWO INCREMENTS, AND THAT IS DELIBERATE. The counting half
# is behaviour-neutral -- it reads a counter and writes a bucket nothing in
# the search consults -- and DEC-083 says a behaviour-neutral change is
# accepted on identical node counts and an interleaved timing, never on an
# SPRT. So the games measure the multiplier alone, and the counting's own
# proof is stated below with the numbers that discharge INV-6. Two changes in
# one run that could not be attributed would break MEASUREMENT; a run per
# increment would spend a night proving that a counter nobody reads is worth
# zero.
#
# DEC-221: the technique reached the implementer as prose -- the step file's
# own sections 1 to 8, written from publications and from the open-source
# record's published measurements -- and was implemented from that
# description. No other project's code was opened. Its constants are seeded in
# DEC-134's three forms and `src/search_params.hpp` says which form each
# takes: `TmNodeBasePct` and `TmNodeScalePct` are **(b), a derivation over
# chesso's own tree** -- the census in `adocs/data/S132_node_share_census.py`
# over the 300-position stratified pick at `go depth 12`, solved against two
# constraints that are chesso's own (the factor is neutral at chesso's median
# share, and lands exactly on chesso's own `TmScaleMinPercent` at a share of
# 100 %). The census ran 2026-09-21 over those 300 positions on the release
# build with the counting half in it: **median share 0.5350**, quartiles 35.8
# and 73.0, deciles 22 31 40 47 54 59 69 78 89, one position at 100 % and none
# at 0 % (`adocs/data/S132_node_share_census.tsv`). The two constraints then
# give **`TmNodeScalePct` 151 and `TmNodeBasePct` 120**, both inside their
# declared ranges -- the slope the second constraint asks for passes the
# range's top only above a median of about 0.77, and chesso's is nowhere near
# it. `TmNodeMinDepth` is **(b)** as well, `AspirationMinDepth` as this
# tree compiles it. **(c) was available for all three and was not taken**, and
# for the gate it was refused outright: the midpoint of its range is above
# every depth this engine reaches, so it would have seeded the feature off.
#
# **What the published record is worth here is direction and nothing else**
# (DEC-019). The figures the step file's section 1 traces -- +5.1 and +6.7 at
# one engine, +9.9 and +9.7 at another, +11.4 and +11.8 at a third, +3.6 to
# +10.2 at a fourth -- are records at engines this project is not, and this
# project's own published-to-measured transfers read 0, 0, wrong sign, 0.10
# and 0.33. **None of them is an expectation here**; what they decided is that
# the technique is tried at all, and that it is tried early.
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON, and which commit that is
# depends on S097 verdict 1, whose SPRT was still running when this file was
# written: H1 leaves the singular extension in the tree, H0 flips `SeExtend`
# to 0. This candidate's diff is independent of that flip and rebases onto
# either tree, so the pair is "the landing and its parent" either way -- and
# the parent is pinned by the coordinator at landing time, from `git
# rev-parse`, never guessed. DEC-020 is the contamination that made
# attribution a rule: one run reported +301 Elo and meant nothing.
#
# `REF` and `CAND` are therefore `PIN_ME` below and **the script refuses to
# run unpinned** rather than defaulting to something plausible.
# `fastchess.sh`'s banner prints both shas with their commit dates before the
# first game, so what the run measures is on screen and never assumed.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019). They are here for
# one reason: this candidate must leave the tree **identical**, because the
# only thing it changes is when iterations stop, and a search that also
# changed shape would not be the experiment this run is pre-registering.
#
#   INV-6, the counting half, measured 2026-09-21 against `9fdd9fb`, whose
#   `src/` and `tests/` are `88ec74f`'s to the byte:
#
#     tools/search_bench.py   depth 9    21479 / 102462 / 33148 nodes,
#                                        g5f6 / e2a6 / d7c8q
#                             depth 12  154388 / 459115 / 239314 nodes,
#                                        c3d5 / e2a6 / d7c8q
#
#   **identical on both sides at both depths, counts and best moves**
#   (.tuning/coord/S132_search_bench.log).
#
#   `chesso bench`: **5066204 on both, the parent's total to the node, with
#   all eight `bestmove` replies identical** -- the only difference between
#   the two transcripts is the nps figure (.tuning/coord/S132_bench_parent.log
#   and S132_bench_candidate.log). This is the ledger entry that is an
#   equality rather than a number.
#
#   WHAT THE COUNTING COSTS, DEC-083. One `if (ply == 0)` in the move loop
#   every node runs. Priced over `chesso bench`, whose node count is identical
#   on the two binaries, so its nps is a pure timing: **12 interleaved pairs,
#   paired delta -0.10 %, sd 1.65 %, se 0.48 %, 95 % [-1.03, +0.83]**, parent
#   mean 3583659 nps against candidate 3579527. `hyperfine -w 1 -r 10` over
#   the same pair, which does not interleave, agrees: 1.434 s +/- 0.023
#   against 1.438 s +/- 0.019. One-minute load average 1.1 to 1.5 on twelve
#   threads throughout, nothing running but the desktop
#   (.tuning/coord/S132_timing_paired.log). **Inside the project's own 3 %
#   noise floor and inside its own interval**: the branch predicts to
#   not-taken everywhere but the root, and "expected to be free" was still not
#   taken on trust.
#
#   THE OFF VALUE, DEC-215, and it belongs to the multiplier. `TmNodeScalePct`
#   0 is not a range end that merely looks inert: the factor's own formula
#   reads 0 there, which would floor every soft limit, so
#   `search_time_node_factor_percent` answers 100 at 0 before it computes
#   anything. **What the tree can and cannot show, measured rather than
#   argued.** The rule moves no node at any depth by construction -- it
#   decides when iterations stop -- and the tune build says so: `bench` is
#   5066204 at `TmNodeScalePct` 0, at its shipped 151 and at `TmNodeBasePct`
#   400 alike, the parent's total in every case. A bench equality is therefore
#   necessary here and carries no information, and the probe is the only
#   instrument that can see the switch: in the tune build at `TmNodeScalePct`
#   0 the factor reads **100 at every share tested (0, 37, 50, 99, 100)**, and
#   with the factor at 100 the loop's combined scale is S089's own scale to
#   the point, which is the parent's time decision exactly
#   (`tests/test_engine.cpp` "the node factor falls as the best move takes
#   more of the tree" and "the iteration loop scales its soft limit by the
#   share it measured"). That is what makes the H0 reading below **one default
#   flip** rather than a revert of the commit, and the suite is green at that
#   flip by construction: every case this step adds asserts the off behaviour
#   when the setting is 0.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair, because the step asks for a gainer**:
# S132's accepts names an SPRT verdict at an increment control and the step is
# a strength change with nothing to trade against it. DEC-063 sizes bounds
# from the expectation and there is no expectation that transfers -- see the
# paragraph above on what published figures have been worth here.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices `{0, 5}`
# at **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1,
# `adocs/plan.md`'s cost table). Budgeted at **2110 games an hour**, which is
# `.moltke.local.md`'s standing figure for `8+0.08` on `noob_3moves.epd` under
# this governor: **19.8 h** and **12.1 h**. An effect outside the interval in
# either direction ends far sooner -- the ledger's verdicts mean under five
# hours -- but the wall is what is budgeted and not the mean (DEC-155): this
# is four hours or more, so it is a night the machine is given whole, and the
# coordinator schedules it.
#
# **A time-management change is measured at the control it will be judged at,
# and this run is one control.** The candidate's own cost per node is nil --
# it moves no node -- so the standing throughput figure applies unchanged, and
# a games-an-hour figure far from 2110 is a fact about the machine and not
# about the candidate.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). Everything `fastchess.sh` already
# does; nothing here overrides it but the output directory. **The step file's
# section 6 says "UHO book" and that sentence is older than the harness**: it
# was written on 2026-08-19, and DEC-189 moved the book to `noob_3moves.epd`
# after S219 measured it. The regime this run takes is the harness's own, which
# is what every verdict in the ledger since DEC-189 was taken at; the step
# file keeps its sentence with this correction beside it, in the DEC-215
# clause-3 style.
#
# **No harness change since the last fixed-rounds A/A** -- same fastchess
# alpha 1.8.1 20260720-daa3ea2, same book, same adjudication, same machine as
# the 2026-09-12 calibration -- so DEC-143's A/A clause owes nothing before
# this run. If any of the four moves before it starts, the A/A comes first.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot: two runs had to be
# relaunched for it. `.tuning/` is gitignored and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, read per side from the run's own PGN in `$OUT`:
#
#   tools/forfeit_report.py "$OUT"/*.pgn --max-pct 1.0
#
# The denominator is each engine's own games and not the run's, which is the
# S088 reading the tool implements. **This is the S089 lesson and it is not a
# formality here**: this is a time-management change, the one class of change
# whose failure mode is arriving late rather than playing badly, and a
# candidate that forfeits is not a candidate that lost. A crash or a
# disconnect on either side voids the run outright and `fastchess.sh` says so
# itself (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the machine.
# **Nothing else is an abort** -- in particular a long walk near the bounds is
# not, which is what a truth inside the interval looks like (S068 run 1).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **No defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** The three test-side findings S097's
# pre-registration names are still open, all filler behind the next strength
# step, and none of them is touched by this landing:
#
#   1. S231 phase one labelled `3a649c0`'s node-budget count 17321 where the
#      byte-identical reverted tree reads 16256.
#   2. `adocs/data/S192_node_budget.py`'s own drift line reported OUTSIDE for
#      a band it had just derived.
#   3. `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pins depth 6 while `adocs/data/S231_research_witness.py` answers
#      depth 4 on this tree.
#
# None of the three can move a reported score, move or line. If `adocs/plan.md`
# opens another before this run starts, it is named here by id and reach before
# a game is played.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the multiplier gains at least 5 nElo over the tree without
#                   it. **Keep it, at the census seeds it was measured at.**
#                   The stopping run's Elo is upward-biased and is not the
#                   effect size (DEC-063); what may be written is "at least 5
#                   nElo". The three constants stay seeds and are refitted by
#                   S127's lane and not by this step -- **with S085's caveat
#                   doubled and written down at the same time**: the time
#                   management family is the one S085 recommends excluding
#                   from a tune at a control the verification does not share,
#                   because an SPSA'd time manager measured +23.8 at 20+0.2
#                   and -22.9 at 10+0.1. A fit of these three wants the
#                   playing control or a second-control verification, and
#                   S127's file says so before it runs.
#                   **And the step then asks the owner one question rather
#                   than deciding it**: before the constants are called
#                   shipped, one confirmation at a second control -- a
#                   40+0.4-class run, or folded into S152's rated run at the
#                   list's own control (DEC-108) -- for the same S085 reason
#                   and because every published node-TM patch this step read
#                   was verified at two to four controls. It is a
#                   recommendation in the step file and it stays one; the
#                   completion states it as a question and does not spend the
#                   night on its own authority.
#   H0 accepted  -> the multiplier does not gain 5 nElo over the tree without
#                   it. The accepts then binds: the zero is recorded as a zero
#                   and **the multiplier is switched off** -- `TmNodeScalePct`
#                   moves from its census seed to 0, one line, and the engine
#                   is this reference's again, proved by the off-value
#                   signature above rather than argued (INV-6). The counting
#                   half stays: it is behaviour-neutral, it is the thing an
#                   H0 does not price, and a later step that wants per-root-
#                   move nodes -- a node-effort stop rule is the obvious one,
#                   and the record has that variant too -- would otherwise
#                   rebuild it. The code, its cases and its mutants stay at
#                   the off value on that reason and it is written here rather
#                   than decided afterwards.
#   No verdict   -> record as zero and decide with the reason stated, not
#                   implied. S068's run 1 is the precedent for a `{0, 5}` pair
#                   random-walking when the truth sits inside the interval,
#                   and the reading then is the interval, not the point
#                   estimate. DEC-063: a stalled walk is terminated and
#                   recorded, and the default at a zero is to switch the
#                   multiplier off by the same one-line flip, with the reason
#                   stated.
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
# REF is the commit S132's landing sits on -- the tree with no per-root-move
# buckets, no node factor and no `TmNode*` settings. CAND is the landing
# commit itself. Both are pinned by the coordinator after the landing commit
# is made, by editing the defaults below; `REF` and `CAND` in the environment
# override for any follow-up leg.
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD~1   # check it is REF
#
# Until both are pinned this script refuses: a candidate guessed from `HEAD`
# is a candidate nobody checked, and DEC-020 is what that costs.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S132_sprt.sh before running (CAND is the landing" \
         "commit, REF the commit it sits on)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s132_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
