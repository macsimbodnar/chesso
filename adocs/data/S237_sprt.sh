#!/usr/bin/env bash
#
# S237: HINDSIGHT REDUCTIONS. A node reached through a reduced first search
# reads the plies its parent took off the move together with how the static
# evaluation moved across that move, from the mover's side, and corrects its
# own depth by one ply: a heavily reduced move whose evaluation got worse for
# the mover gets a ply back, a lightly reduced one whose evaluation improved
# gives one up.
#
#   nohup adocs/data/S237_sprt.sh > .tuning/sprt_s237.log 2>&1 &
#
# **THE CANDIDATE IS ONE RULE WITH TWO BRANCHES.** One block in `negamax_at`,
# after the node's static evaluation is stored and before anything that reads
# `depth`; one trailing parameter, `parent_reduction`, that hands the reduction
# down from the reduced first search and is 0 on every other search of a move;
# one local at the move loop that hands 0 instead wherever the node corrected
# its own depth. Four rows in `src/search_params.hpp`: `HindsightHeavyReduction`
# 3 and `HindsightLightReduction` 1 -- each its branch's switch, off at 126 and
# 0 -- and `HindsightWorseMargin` 24 and `HindsightBetterMargin` 24. The
# branches are measured together, as the description gives them and as the
# step's accepts names one verdict; an H1 attributes to the rule and not to a
# branch.
#
# DEC-221: implemented from the description in
# `adocs/data/2026-09-19_search_technique_study.md` row N5, the base rule only
# (the review's F09), and from this step's own file, which is what the
# implementing agent's brief carried. No other project's code was opened and no
# constant of one is behind any value here (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), stated at their site in `src/search_params.hpp`. Form
# (b) from `adocs/data/S237_census.py` (output `adocs/data/S237_census.txt`),
# over the eight bench positions at depth 12 on the tree with both switches
# off: `HindsightHeavyReduction` 3, the p75 of the reductions a reduced move
# arrives with; both margins 24, the p50 of the mover's |delta| across a
# reduced move. `HindsightLightReduction` 1 is (c), the midpoint of 0 to 2.
# At these seeds the census counts the give-back on 8.79 % of its sites and
# the give-up on 2.52 % -- a counterfactual on the tree without the rule.
#
# THE PRIOR, recorded before the run (DEC-019, DEC-222).
#
#   **The record's figure is +6.03 over 8590 games** for the base rule (the
#   study's row N5). A direction and not an expectation. **This engine's own
#   published-to-measured transfers read 0, 0, wrong sign, 0.10 and 0.33**
#   (the review's F01, DEC-222), and the last two verdicts on this search were
#   zeros: S236's fractional history term and S235's blended return. The
#   bounds are sized from the expectation, not from the record.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES by the implementing agent on
# the landing's working tree, 2026-09-25, against a parent built from `6d9c5ce`
# in a throwaway worktree:
#
#   THE OFF VALUES ARE PROVED ON THE TREE (DEC-215). A Release build with
#   `HindsightHeavyReduction` 126 and `HindsightLightReduction` 0 prints
#   `bench` 4803214 with all eight `bestmove` replies identical to the parent's
#   (c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), and `tools/search_bench.py`
#   reproduces it at depth 9 (48522 / 85714 / 28080, c3d5 e2a6 d7c8q) and
#   depth 12 (129499 / 411457 / 172984, c3d5 e2a6 d7c8q). That is the
#   equality an H0's revert returns to (INV-6's form).
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` 4803214 -> 4845333,
#   +0.88 %, the eight replies unchanged. `search_bench` depth 9: 48522 ->
#   18409 (best c3d5 -> g5f6), 85714 -> 91713, 28080 -> 27401; depth 12:
#   129499 -> 128457, 411457 -> 632820 (best e2a6 -> d5e6), 172984 -> 203399.
#   The counts move by construction, so INV-6 does not discharge this change
#   and nothing here is Elo (DEC-019).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the rule is proposed as a gain.
# DEC-063: the pair straddles the expected effect -- the record's +6.03 sits
# above elo1 and this engine's own transfer record sits at zero.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound (`adocs/testing_strategy.md`
# section 1.1), at **2110 games an hour** from `.moltke.local.md`: **19.8 h**
# and **12.1 h**. A night, scheduled (DEC-155). The throughput is read off the
# banner and recorded rather than assumed.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). **No harness change since the last
# fixed-rounds A/A**; if fastchess, the book, the adjudication or the machine
# moves before this starts, the A/A of 1000 fixed rounds comes first and is read
# with `adocs/data/S105_pairs.py` (DEC-143).
#
# WHERE THE OUTPUT LANDS. `OUT` is set in the exec line below, under
# `.tuning/`, because `fastchess.sh`'s default is under `/tmp`, which this
# machine wipes at boot.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# as `adocs/data/S235_sprt.sh` named them on 2026-09-24, read again off
# `adocs/plan.md`'s Open list and `adocs/status.md` on 2026-09-25 with nothing
# closed or added since: **no defect reachable in ordinary play, on the UCI
# surface, or able to move a reported score, move or line is open.** What is
# open is test-side, and none of it is touched here:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding: the case runs
#      about 19 s Release against its 120 s ceiling. Ceiling untouched (TESTS).
#   2. The three S097 named on 2026-09-21: S231 phase one's node-budget label of
#      17321 against the reverted tree's 16256; `adocs/data/S192_node_budget.py`
#      reporting OUTSIDE for a band it had just derived; and
#      `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pinning depth 6 where `adocs/data/S231_research_witness.py`
#      answers 4.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> the rule gains at least 5 nElo at its seeds. **The four
#                   constants stay at their seeds** -- three census percentiles
#                   and one midpoint, none a fit -- and S127 fits them with the
#                   rest of the set; an H1 says the rule is worth having at the
#                   values tried and not where its maximum is. The stopping
#                   run's Elo is upward-biased and is not the effect size.
#
#   H0, interval -> **a loss, and the revert is one pair of defaults.**
#   wholly         `HindsightHeavyReduction` to 126 and
#   below zero     `HindsightLightReduction` to 0, the off values proved above,
#                  so the engine is the reference tree's to the node the moment
#                  they move -- and **the code leaves with it**: the parameter,
#                  the block, the four rows, the probe fields, the cases and the
#                  four mutants come out in the same step, behaviour-neutral at
#                  the reverted defaults and discharged by INV-6, the shape
#                  S235's and S236's removals already have. **Unless a reason is
#                  stated** -- the only one that would count is S238 or another
#                  step wanting `parent_reduction` as its own plumbing; if the
#                  code is kept, the completion stamp says why.
#
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero with
#   or an          the games played and the interval, and read it by the row
#   interval       above: the defaults go to their off values with the code
#   reaching       behind them on the same terms. No follow-up run and no
#   above zero     second pair (DEC-063); another seed is a second experiment,
#                  which S127's fit is the place for.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands the hindsight rule at its seeds; REF is the tree
# the coordinator pins as its reference -- the landing's parent, the tree the
# rule is added to, unless the coordinator records otherwise. Both are pinned
# after the landing commit exists, by editing the defaults below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
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
         "adocs/data/S237_sprt.sh before running (CAND is the hindsight" \
         "rule's landing commit, REF the tree the coordinator pins)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s237_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
