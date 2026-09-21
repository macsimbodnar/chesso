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
# guard case, the accepts' mate row, and the mutants E20 to E23 named in
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
# neither, and **it read H0 on 2026-09-21**: `Elo -0.81 +/- 3.70`,
# `nElo -1.06 +/- 4.81`, LLR -2.96 over 20080 games with 0 forfeits
# (`adocs/data/S097_v1_sprt.log`). A zero and not a loss -- the nElo interval
# [-5.87, +3.75] is centred within one nElo of zero and the bound was reached
# in fewer games than a truth at zero expects -- so DEC-227 keeps the block in
# the tree at `SeExtend` 1, recorded as a zero and not as a gain, as the
# carrier of this verdict. This measures the multicut against the tree with
# both. Nothing else in `src/` moves between the two references, and the pair
# below is checked against that: `REF` is the tree with the extension as it
# stands when the flip lands.
#
# WHY THE MULTICUT IS STILL WORTH A NIGHT AFTER THAT ZERO, written here rather
# than assumed. The H0 text of verdict 1's own pre-registration recommended
# dropping both, and its argument was that the multicut would then have to buy
# the verification search alone. With the extension in the tree that search is
# already paid for at zero -- its gain matched its cost, which is what the
# interval says -- so this run is the pure-margin question and the most
# favourable form the pair can be measured in. DEC-227 is the decision and the
# owner's standing priority of strength and correctness over machine time
# (2026-09-19) is why the night is spent.
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
#   free (DEC-215): `REF`'s binary **is** the tree at `SeMultiCut` 0, and
#   verdict 1's landing recorded that equality to the node -- the tune build at
#   `SeMultiCut` 0 printed `5066204`, the Release candidate's total, with all
#   eight `bestmove` replies identical. That equality is re-taken on whatever
#   tree this flip lands on, because the rebase moves the total and an equality
#   quoted from a different tree proves nothing. So the question this run's
#   bench answers is the other one -- that the flip is not inert.
#
#   BENCH -- TO BE FILLED BEFORE THE RUN, on the landing tree, machine idle,
#   and **re-taken after the rebase**: this landing is written in a worktree
#   while S132 holds the machine and is rebased onto S132's landing before it
#   commits, so every number below is taken once on the pre-rebase tree and
#   again on the tree the SPRT actually measures. A row that moves across the
#   rebase is S132's and not this flip's, and the pair is what says so.
#     chesso bench   reference `474c288`      5066204
#                    candidate (SeMultiCut 1)  4493659   -11.30 %
#     The two differ, so the flip is not inert. The off value is proved on the
#     tree with the full signature beside it: the tune build at `SeMultiCut` 0
#     prints 5066204 with all eight `bestmove` replies identical to the
#     reference binary's.
#   tools/search_bench.py, depths 9 and 12, reference -> candidate:
#     depth  9  midgame  21479 ->  21479 g5f6   kiwipete 102462 -> 102462 e2a6
#               tactical 33148 ->  33148 d7c8q  -- identical on all three,
#               because no node of a depth-9 root has `ply > 0` at a remaining
#               depth of `SeMinDepth`
#     depth 12  midgame 154388 -> 149688 c3d5   kiwipete 459115 -> 459216 e2a6
#               tactical 239314 -> 219544 d7c8q -- no best move moves
#   adocs/data/S097_fixed_node_depth.py, go nodes 1000000, Hash 16:
#     reference  midgame 16  kiwipete 13  tactical 15   total 44
#     candidate  midgame 17  kiwipete 13  tactical 15   total 45
#     The predicted direction: a smaller tree at a fixed depth and one ply more
#     at a fixed budget, which is one of the two the extension cost.
#
#   RE-TAKEN AFTER THE SECOND REBASE, onto `f02f59a` ("Complete S132"). The
#   bench pair reproduced to the node -- 5066204 and 4493659, with all eight
#   `bestmove` replies identical on the reference, on the candidate and on the
#   tune build at the off value -- so the off value is proved with the full
#   signature on the tree this pair is actually measured on. The
#   `search_bench` and fixed-node rows above were **not** re-taken and stand:
#   `git diff 474c288 f02f59a -- src tests` is empty, `src/` under the flip is
#   one X-macro row, the build flags are unchanged, and the bench reproducing
#   its total and its replies is the instrument saying the tree is the one
#   those rows were read on.
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
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# read off `adocs/plan.md`'s Open list and this step's own record on
# 2026-09-21, when the landing was written. **No defect reachable in ordinary
# play, on the UCI surface, or able to move a reported score, move or line is
# open.** `adocs/plan.md`'s Open list names no finding step at all -- its
# entries 1 to 48 are strength, speed and evaluation steps -- so what is open
# is the three test-side findings `adocs/data/S097_v1_sprt.sh` named, each
# re-checked against the tree here and each still open, all filler behind the
# next strength step:
#
#   1. S231 phase one labelled `3a649c0`'s node-budget count 17321 where the
#      byte-identical reverted tree reads 16256. One reading was taken wrong;
#      the band in `tests/test_search.cpp` is derived from the second and the
#      first is a label in a step file now in `plan_done/`, so nothing in the
#      tree depends on it.
#   2. `adocs/data/S192_node_budget.py`'s own drift line reported OUTSIDE for a
#      band it had just derived, which is a defect in that line and not in the
#      band.
#   3. `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pins depth 6 while its own script,
#      `adocs/data/S231_research_witness.py`, answers depth 4 on this tree.
#      S231 moved that pin to 4 and its H0 revert took the move back with
#      everything else. The case is green either way and the pin is unmoved in
#      this landing.
#
# S097's own fast check and its verdict-1 run opened no finding: the check's
# one real problem -- the verification gate reading the entry through the raw
# slot pointer after the null-move recursion -- was repaired inside the
# landing, and the run reported 0 forfeits and no crash or disconnect. The
# `Incomplete mating PV` split, 14 candidate against 9 reference, is an
# observation recorded in the step file and not a diagnosis (CHESS).
#
# None of the three can move a reported score, move or line, and none is
# touched inside this landing.
#
# PRE-REGISTERED INTERPRETATION, **amended 2026-09-21 under DEC-227 and before
# a single game is played**. The H0 text this file carried until then read
# "drop the multicut and keep the extension", and that sentence is wrong now
# for a reason that has nothing to do with the number this run will produce:
# it was written while verdict 1's own verdict was still open, and verdict 1
# read H0. The extension is in the tree as a **measured zero**, kept only as
# this verdict's carrier, so there is nothing left for an H0 here to fall back
# to -- keeping a rule that measured zero underneath a rule that measured zero
# is two unmeasured features and a plan entry nobody will ever remove. This
# verdict therefore decides the block whole, and the reason is written before
# the number (DEC-063):
#
#   H1 accepted  -> the multicut gains at least 5 nElo over the tree with the
#                   extension alone. **Keep both**, with both defaults at 1.
#                   The extension's zero stays on the record and the reason it
#                   is kept is that it carries a measured gain -- S005, S006
#                   and S015 are the precedent for a feature kept with its own
#                   zero stated. The step completes with two verdicts. The
#                   stopping run's Elo is upward-biased and is not the effect
#                   size (DEC-063).
#   H0 accepted  -> **the whole block leaves**, not the multicut alone: the
#                   verification search, the extension, the five gates on an
#                   excluded node, the six settings, the sixteen cases and the
#                   twenty-two mutants, in **one revert to `5c76ea9`'s
#                   `src/`** and not a default flip. Proved and not argued:
#                   `bench` reads **4579468** and `tools/search_bench.py` is
#                   identical to `5c76ea9` at depths 9 and 12, node counts and
#                   best moves both, which is INV-6's own discharge of the
#                   removal. `MANUAL.md`'s six option rows, the `MANUAL.md`
#                   and `specs.md` passages and the mutant file go with it.
#                   DEC-194 is the precedent for not keeping a feature whose
#                   interval sits below the bound, and DEC-227 is the decision
#                   that this verdict is where the block is decided.
#   No verdict   -> **a stalled walk near +3 straddles these bounds and is what
#                   the record predicts here** (DEC-063). Terminate it, record
#                   the zero with the games played and the interval, and take
#                   the H0 path above in full -- the same reading, stated in
#                   advance so that a long run is not read as encouragement.
#
# The removal is one commit and it is behaviour-neutral against `5c76ea9`'s
# search by construction, so the commit that takes it carries this run's result
# block (DEC-220) and `Bench: 4579468`.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# REF is **the tree the flip lands on**: the tree carrying the verification
# search and the extension with `SeMultiCut` at 0, as it stands at the
# landing. That is not verdict 1's own landing commit `88ec74f` any more --
# S132 landed on `achesso` between the two verdicts as `474c288` ("Scale the
# soft time limit by the best move's share of the root", whose own commit line
# reads `Bench: 5066204`, verdict 1's total unmoved) and this landing is
# rebased onto it. So REF is `474c288` or whatever descendant of it the
# coordinator pins at the landing, and it is the flip's parent either way.
# CAND is the commit that moves the default to 1. Both are pinned by the
# coordinator after the flip is committed, by editing the defaults below.
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the flip, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-PIN_ME}"
CAND="${CAND:-PIN_ME}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S097_v2_sprt.sh before running (CAND is the SeMultiCut" \
         "flip, REF is its parent: the tree with the extension and" \
         "SeMultiCut 0)" >&2
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
