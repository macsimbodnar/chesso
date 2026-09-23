#!/usr/bin/env bash
#
# S236 v2: the same fractional history reduction at **half the reach** --
# `LmrHistClamp` 1024 instead of the 2048 verdict 1 measured. Judged against the
# same parent that verdict used, the tree without the term.
#
#   nohup adocs/data/S236_v2_sprt.sh > .tuning/sprt_s236_v2.log 2>&1 &
#
# **THIS IS A PRE-REGISTERED FOLLOW-UP AND NOT A SECOND TRY** (DEC-231, DEC-063).
# S236's own pre-registration named three readings before a game was played. The
# run read the third:
#
#   SPRT | cand 8b1bc79 vs ref 666b5a0, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
#   Elo | -1.60 +/- 4.25, nElo -2.05 +/- 5.44
#   LLR | -2.95 (-2.94, 2.94) -> H0
#   Games | N: 15658 W: 4737 L: 4809 D: 6112, Ptnml [752, 1843, 2665, 1863, 706]
#   Wall | 7 h 31 m, 2081.1 games/h, forfeits 0
#   Log | adocs/data/S236_sprt.log
#
# The nElo interval is **[-7.49, +3.39]**: it reaches above zero, so this is not
# the loss the second reading names but the third -- a zero read as a walk. The
# term at the census clamp does not gain 5 nElo and does not measurably lose,
# while searching half again as many nodes per depth. DEC-231's one follow-up is
# this file, and **there is no third run**: if this reads H0 the term leaves the
# tree with its settings, its cases and its mutants.
#
# WHAT CHANGES, AND IT IS ONE DEFAULT. `LmrHistClamp` 2048 -> 1024 in
# src/search_params.hpp, with its `golden_defaults` row and its MANUAL.md row.
# No other line of `src/` differs from the landing. The term, the divisor
# (`LmrHistDiv` 734), the accumulator and its rounding at `LmrRoundBias` 0 are
# verdict 1's exactly.
#
# WHY THIS VALUE AND WHY IT COULD BE WRITTEN DOWN IN ADVANCE. The shipped 2048
# was never a fit: the census rule asked for 6812 ticks, 6.7 plies, and the
# declared range top bound it. The same census, read before any game, said what
# the cheaper clamp does -- **the same 12.54 % of reduction sites move, every
# one of them by one ply instead of 5.42 % of them by two** -- and the bench
# said what it costs: 5193174 nodes against 6858745, **+15.6 % of tree against
# +52.6 %**. So the follow-up is the same rule at a third of the price, and the
# question it asks is the one verdict 1 left open: whether the term's judgement
# was worth paying for at all, or only worth paying a little for. A number
# chosen off the bench after a verdict would be a search for a passing number
# (DEC-213's own rejection); this one was in `adocs/data/S236_sprt.sh` before
# the first game of verdict 1.
#
# DEC-221: the technique is S098 verdict 1's, implemented from its description
# and this project's own record, seeded in DEC-134's forms. No other project's
# code was opened and no constant of one is behind any value here.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-23:
#
#   THE CENSUS'S PREDICTION HELD TO A TENTH OF A PER CENT. `bench` goes
#   **4493659 -> 5193174, +15.6 %**, against the +15.6 % the census's second
#   pass named before verdict 1 was run. Verdict 1's own tree was 6858745,
#   +52.6 %, so the follow-up buys back two thirds of what the term cost.
#
#   AND THE EIGHT REPLIES ARE THE PARENT'S, ALL OF THEM:
#   c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6. At 2048 two of the eight moved;
#   at 1024 none does. The term at this reach changes how much tree the engine
#   spends and not, on these eight, what it decides -- which is a fact about
#   eight positions and not a prediction about a game (DEC-019).
#
#   `tools/search_bench.py`, parent -> candidate, node counts and best moves:
#     depth  9   21479 / 102462 / 33148  ->  25231 / 104152 / 29307
#     depth 12  149688 / 459216 / 219544 ->  221227 / 485833 / 204838
#     best moves identical at both depths: g5f6 / e2a6 / d7c8q at 9,
#     c3d5 / e2a6 / d7c8q at 12. The third position **shrinks** at both, which
#     one bench total hides: a term that reduces the quiets history likes does
#     not grow every tree.
#
#   THE OFF VALUE IS PROVED ON THE TREE and not declared (DEC-215): at
#   `LmrHistClamp` 0 the tune build prints **4493659 with all eight replies
#   identical** to `666b5a0`, and a release build with that default at 0
#   reproduces `search_bench` exactly at both depths -- 21479 / 102462 / 33148
#   and 149688 / 459216 / 219544. That is the tree an H0's revert returns to and
#   it is INV-6's own form.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, the same one verdict 1 used, because
# the follow-up is proposed as a gain and a `{-5, 0}` pair would accept H1 on a
# truth of zero.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** at the interval's
# midpoint and **25591** with the truth on a bound, at the throughput this
# candidate will run at -- verdict 1 managed 2081.1 games an hour with a tree
# half again the parent's, and this tree is only 15.6 % larger, so the rate
# should sit nearer `.moltke.local.md`'s 2110: **19.8 h** and **12.1 h**. A
# night, scheduled (DEC-155), and the banner's own rate is what gets recorded.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). **No harness change since verdict
# 1**, which ran on this machine with 0 forfeits over 15660 games; if fastchess,
# the book, the adjudication or the machine moves before this starts, the A/A of
# 1000 fixed rounds comes first and is read with `adocs/data/S105_pairs.py`.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default is
# under `/tmp`, which this machine wipes at boot.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **No defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** What is open is test-side:
#
#   1. **`E21_multicut_mate_band_gate_dropped` is killed on this tree, and that
#      closes the finding verdict 1 opened.** It survived there because the row
#      that kills it had been mined on a tree the term does not produce; the
#      flip turned that row false outright, `adocs/data/S097_mine_mate_row.py`
#      was run again on this candidate, and the row it returned kills the
#      mutant: red observed by hand with E21 applied
#      (`.tuning/coord/S236v2_observe_red.log`) and **killed in the pass, 1 of
#      40 cases, with the bench signature unmoved** -- the case catching what
#      the signature cannot see. **What stays open is the dependency**: that row
#      belongs to this tree, so an H0 restores its predecessor with the revert
#      and E21's kill goes back to resting on the old one.
#   2. **Two mate rows now belong to trees this verdict can remove.** S095's was
#      re-mined for the term at 2048 and S097's for the term at 1024; both old
#      rows are kept verbatim in the case's GOLDEN block, with the tree each
#      belongs to, so an H0 restores them rather than re-mining a third time.
#   3. The three S097 named and re-checked on 2026-09-21, each still open and
#      none of them touched here: S231 phase one's node-budget label of 17321
#      against the reverted tree's 16256; `adocs/data/S192_node_budget.py`
#      reporting OUTSIDE for a band it had just derived; and
#      `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pinning depth 6 where `adocs/data/S231_research_witness.py`
#      answers 4.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063), and
# it is DEC-231's own text rather than a new reading:
#
#   H1 accepted  -> the term gains at least 5 nElo at (734, 1024). **Keep it at
#                   that pair** for S127 to fit -- and note that fitting the
#                   clamp upward needs the range widened past its top, which the
#                   parameter's own comment says and this run does not do. And
#                   **before the step completes, re-mine the E21 row** on the
#                   shipping tree with `adocs/data/S097_mine_mate_row.py`, the
#                   new row proved by a targeted `tools/mutation_check.py
#                   --only E21` scoring it killed. The stopping run's Elo is
#                   upward-biased and is not the effect size (DEC-063).
#
#   H0, or a     -> **the term leaves the tree.** `LmrHistDiv`, `LmrHistClamp`,
#   stalled walk    the cases that read them, the three mutants that break them,
#   (DEC-063)       the two re-mined mate rows **and the mate-carry ceiling this
#                   step raised** go with it: the rows those replaced come back
#                   from the GOLDEN block, `C_mate7_depth11`'s ceiling goes back
#                   to 0, and `adocs/data/S236_v2_sweep.txt` drops out of the
#                   `--ceilings` command that derives it -- the first four grids
#                   alone still answer 0. A revert, not a third re-mine. DEC-213 is then reaffirmed on the form
#                   it did not have: the whole-ply term measured zero twice, the
#                   fractional one twice more, and four readings of one idea are
#                   enough. **The accumulator stays**, at `LmrRoundBias` 0: it
#                   is behaviour-neutral by the identity proved at every
#                   landing, and S237 and S238 both need a reduction that can
#                   carry a fraction. **There is no third run.**
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# REF is **666b5a0**, the tree without the term -- verdict 1's own reference and
# the pre-registration's "same parent". It is deliberately not the flip's own
# parent, which already carries the term at 2048: this run asks what the term at
# 1024 is worth against no term at all, which is the only question DEC-231's
# follow-up was written to answer.
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD   # the flip, CAND
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game.
REF="${REF:-666b5a0}"
CAND="${CAND:-85901b3}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S236_v2_sprt.sh before running (CAND is the flip's" \
         "landing commit, REF is 666b5a0, the tree without the term)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s236_v2_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
