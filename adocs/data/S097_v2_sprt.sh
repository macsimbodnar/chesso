#!/usr/bin/env bash
#
# S097 verdict 2, the multicut: the verification search the extension already
# pays for is allowed to **return**. Where it fails high at or above this
# node's own beta, some move other than the table's already reaches a bar just
# under the entry's score and the node is taken to fail high without being
# searched. Judged against S097 verdict 1's landing.
#
#   nohup adocs/data/S097_v2_sprt.sh > .tuning/sprt_s097_v2.log 2>&1 &
#
# **THE CANDIDATE IS ONE DEFAULT AND WHAT IT TURNS ON.** `SeMultiCut` moves
# from 0 to 1 in `src/search_params.hpp`; the rule it enables shipped inert
# with verdict 1 and no line of it moves here. What lands beside the flip is
# the coverage the flip makes possible and nothing else: the rule's direct
# guard case, the accepts' mate row, and the three mutants E20 to E22 named in
# `tools/mutants/S097_singular_extension.py`'s header. **Why they could not
# land with verdict 1**: `SeMultiCut` is `inline constexpr int` in the release
# build, which is the build the gate runs and the only build
# `tools/mutation_check.py` compiles, so at 0 the compiler folds the branch
# away -- a case cannot drive a rule that is not in the binary and a mutant of
# it is equivalent by construction. Verdict 1's own case asserts exactly that
# much, that every condition of the rule holds at a drive and the node searches
# anyway.
#
# ONE CHANGE AT A TIME, which is what makes the two numbers attribute. Verdict
# 1 measured the verification search and the extension against the tree with
# neither. This measures the multicut against the tree with both. Nothing else
# in `src/` moves between the two references, and the pair below is checked
# against that: `REF` is verdict 1's landing commit.
#
# DEC-221: implemented from the step file's description -- its section 1's
# published prose and section 3's sketch -- and from no project's code. The
# two choices the step file leaves open are recorded in it and are this
# project's own: **the fail-soft score is returned and never `singular_beta`**,
# and the rule is gated on `!is_pv` like every other bound-returning rule in
# `negamax_at`. The record is direction only (DEC-019) and seeds nothing
# (DEC-105); the multicut adds no constant beyond its switch.
#
# WHAT THE RECORD SAYS, AND WHY THE EXPECTATION IS LOWER HERE. The multicut is
# traced at +5.7 and +6.2 at engines in the 3300 band and was **dropped by one
# engine at roughly the band this one is heading for**, which is the honest
# split the step file's section 1 records. A `{0, 5}` pair over an effect of
# that size is a pair the truth may well sit inside, and DEC-063 says what to
# do then rather than leaving it to the day: terminate a stalled walk, record
# the zero, and drop by default.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES.
#
#   THE OFF VALUE IS THE REFERENCE, which is the one thing this pair gets for
#   free (DEC-215): `REF`'s binary **is** the tune build at `SeMultiCut` 0, and
#   verdict 1's pre-registration records that equality to the node. So the
#   question this run's bench answers is the other one -- that the flip is not
#   inert.
#
#   BENCH -- TO BE FILLED BEFORE THE RUN, on the landing tree, machine idle:
#     chesso bench   reference (verdict 1's landing) <total>
#                    candidate (SeMultiCut 1)        <total>
#     and the two must differ, or the flip changed nothing and there is
#     nothing to measure.
#   tools/search_bench.py, depths 9 and 12, reference -> candidate: <rows>
#   adocs/data/S097_fixed_node_depth.py, both trees: <rows>
#
#   A multicut **prunes**, so the expected direction here is a smaller tree at
#   a fixed depth and a deeper search at a fixed node budget -- the opposite of
#   the extension's trade. A candidate whose bench does not fall is a candidate
#   whose rule fires nowhere, which is a reason to look at the gates before
#   spending a night on it (DEC-212 and DEC-214 are what an inert rule costs a
#   verdict).
#
#   Node counts move by construction, so INV-6's discharge is not available and
#   games are the only thing that can decide this step.
#
# THE MATE ROW, AND THE RED IT WAS OBSERVED AT. The accepts asks for a forced
# mate inside the multicut's pruned depth added beside "pruning does not hide a
# forced mate" in `tests/test_search.cpp`, observed red with the mate-range
# guard removed. Multicut is pruning, and pruning that hides a mate is this
# repository's recurring bug -- null move pruning hid a mate in two, late move
# reduction reduced the mating move at the root, and both were caught by a case
# like that one and by no benchmark. The row is **mined and not chosen**
# (CHESS), over this project's own labelled mates, and the mining is run on
# this candidate because the rule does not exist on the reference. The
# observation is recorded in the step file with the failing line.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the pair S097's accepts names for each of its two
# changes.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** at the interval's
# midpoint and **25591** with the truth on a bound
# (`adocs/testing_strategy.md` section 1.1, `adocs/plan.md`'s cost table), at
# **2110 games an hour** from `.moltke.local.md`: **19.8 h** and **12.1 h**. A
# night, scheduled (DEC-155). The reference here carries the verification
# search, so both sides pay for it and the throughput should be the reference
# run's rather than the standing figure; it is read from the banner and
# recorded either way.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). **No harness change since the last
# fixed-rounds A/A**; if fastchess, the book, the adjudication or the machine
# moves before this starts, the A/A of 1000 fixed rounds comes first and is
# read with `adocs/data/S105_pairs.py` (DEC-143).
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright
# (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the machine. Nothing
# else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# The three test-side findings `adocs/data/S097_v1_sprt.sh` names, unless they
# have been closed by then, and any finding S097's own fast check or verdict-1
# run opened -- named here by id and reach before a game is played. **No defect
# reachable in ordinary play, on the UCI surface, or able to move a reported
# score, move or line may be open when this starts** (BUGS).
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the multicut gains at least 5 nElo over the tree with the
#                   extension alone. **Keep it**, with the default at 1, and
#                   the step completes with two verdicts. The stopping run's
#                   Elo is upward-biased and is not the effect size (DEC-063).
#   H0 accepted  -> **drop the multicut and keep the extension.** The default
#                   goes back to 0 and the rule, its case, its mate row and its
#                   three mutants leave with it rather than shipping at an off
#                   value -- a rule nothing measures is a rule nobody will
#                   remove later. That removal is behaviour-neutral by
#                   construction and proved by the bench signature against this
#                   reference. S005, S006 and S015 are the precedent that a
#                   measured zero is recorded as a zero; DEC-194 is the
#                   precedent for not keeping a feature whose interval sits
#                   below the bound.
#   No verdict   -> **a stalled walk near +3 straddles these bounds and is what
#                   the record predicts here** (DEC-063). Terminate it, record
#                   the zero with the games played and the interval, and drop
#                   by default -- the same reading as H0, stated in advance so
#                   that a long run is not read as encouragement.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# REF is S097 verdict 1's landing commit -- the tree with the verification
# search and the extension and with `SeMultiCut` at 0. CAND is the commit that
# moves that default to 1. Both are pinned by the coordinator after the flip is
# committed, by editing the defaults below.
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the flip, CAND
#
# Until both are pinned this script refuses (DEC-020).
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S097_v2_sprt.sh before running (CAND is the SeMultiCut" \
         "flip, REF is S097 verdict 1's landing)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s097_v2_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
