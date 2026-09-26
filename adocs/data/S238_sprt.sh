#!/usr/bin/env bash
#
# S238: CUTOFF COUNT. Each node counts how many of its children failed high in
# their own move loops so far, and while that count is over a threshold the
# reduction of its later quiet moves rises by a fixed amount: a node whose
# children keep failing high is one whose later moves are not worth the depth.
#
#   nohup adocs/data/S238_sprt.sh > .tuning/sprt_s238.log 2>&1 &
#
# **THE SEEDS' REDS, AND WHAT DEC-233 RESOLVED, SAID HERE FIRST.** This step
# was built on `1a35f16`, where S237's hindsight rule was live; S237 read H0 and
# left (`1680439`), and this step was **rebased onto that removal** and its
# goldens re-derived there, 2026-09-26. On the rebased tree at the seeds below
# the fast suite first read red in one place: "pruning does not hide a forced
# mate" lost its third capture-mate row at depth 10. DEC-233 resolved it
# without an assertion relaxed: all four capture-mate rows re-derived by
# `adocs/data/S230_mine_r01_row.py depths` (9, 9, 10, 10 -> 7, 7, 11, 9; the
# same sweep at `1680439` returns the old rows, so the tree moved them), and
# the four node-type term cases' drives given children that answer from the
# table so the cutoff count is 0 in both, counted. `test_mate_carry` is green
# at `1680439`'s budgets on this tree and its budgets were **not** moved: the
# re-sweep by DEC-156's rule would move C, D and E, but recording its grid
# raises C's short-line ceiling 0 -> 1, which that file calls relaxing a test
# and which DEC-233 does not cover -- the coordinator's decision, recorded in
# the step file. Every old row is kept beside its new one for the revert
# below.
#
# **THE CANDIDATE IS ONE RULE.** A per-ply slot on `search_state_t`,
# `cutoff_counts`; the node clears its children's slot `[ply + 1]` just before
# its move loop, and a node whose own move loop fails high adds one to its
# slot `[ply]` unless it is S097's verification. At each late quiet the node
# reduces, `cutoff_count_ticks` adds `CutoffCountReduction` ticks (1024 to the
# ply) to `lmr_adjusted_reduction`'s sum when the slot is over
# `CutoffCountThreshold`, before the one rounding. The shallow-depth gates do
# not read it. (On `1a35f16` the reduction that results was also what S237's
# hindsight rule was handed as `parent_reduction`; that rule and its
# parameter left with S237's H0, so nothing reads the reduction but the
# search below it.) Two rows in `src/search_params.hpp`:
# `CutoffCountThreshold` 5 [0, 63] and `CutoffCountReduction` 1024 [0, 2048],
# the second the switch, off at 0.
#
# DEC-221: implemented from the description in
# `adocs/data/2026-09-19_search_technique_study.md` row N4, the base rule only,
# and from this step's own file, which is what the implementing agent's brief
# carried. No other project's code was opened and no constant of one is behind
# any value here (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), stated at their site in `src/search_params.hpp`.
# `CutoffCountThreshold` 5 is form (b), the p75 of the count at the sites the
# rule reads it, from `adocs/data/S238_census.py` (output
# `adocs/data/S238_census.txt`) over the eight bench positions at depth 12 on
# the tree with the adjustment at 0 -- re-run on the rebased tree, p75 5 again
# (p50 3 -> 2, p95 12 -> 13 against the first run on `1a35f16`); it fires on
# 21.81 % of those sites, a counterfactual on the tree without the rule. `CutoffCountReduction` 1024 is
# form (c), the midpoint of its range: one ply.
#
# THE PRIOR, recorded before the run (DEC-019, DEC-222).
#
#   **The record's figure is +6.00 over 8688 games** for the base rule, later
#   refined at +3.52 (the study's row N4). A direction and not an expectation.
#   **This engine's own published-to-measured transfers read 0, 0, wrong sign,
#   0.10 and 0.33** (the review's F01, DEC-222), and the recent verdicts on
#   this search read zeros (S235, S236). The bounds are sized from the
#   expectation, not from the record.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES on the rebased working tree,
# 2026-09-26, against a parent built from `1680439` (S237's removal) in a
# throwaway worktree:
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). A Release build with
#   `CutoffCountReduction` 0 prints `bench` 4803214 with the whole `bench`
#   stream -- every `info` line and all eight `bestmove` replies, c3d5 d5e6
#   d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 -- identical to the parent's (times and
#   nps stripped), and `tools/search_bench.py` reproduces it at depth 9 (48522
#   / 85714 / 28080, c3d5 e2a6 d7c8q) and depth 12 (129499 / 411457 / 172984,
#   c3d5 e2a6 d7c8q). That is the equality an H0's revert returns to (INV-6's
#   form).
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` 4803214 -> 4722025,
#   -1.69 %, all eight replies unchanged. `search_bench` depth 9: 48522 ->
#   37598, 85714 -> 84302, 28080 -> 24050; depth 12: 129499 -> 112043, 411457
#   -> 409054, 172984 -> 111754; every best move the parent's. On `1a35f16`,
#   with S237's rule live, the same change read 4845333 -> 3727164, -23.08 %;
#   the difference is S237's rule amplifying it and is no longer in the tree.
#   The counts move by construction, so INV-6 does not discharge this change
#   and nothing here is Elo (DEC-019).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the rule is proposed as a gain.
# DEC-063: the pair straddles the expected effect -- the record's +6.00 sits
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
# carried from `adocs/data/S237_sprt.sh`'s block as the coordinator re-read it
# at pinning on 2026-09-25, and re-read by the rebasing agent on 2026-09-26
# off `adocs/status.md` at `1680439`: **no defect reachable in ordinary play, on the UCI
# surface, or able to move a reported score, move or line is open.** What is
# open is test-side, and none of it is touched here:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding.
#   2. The three S097 named on 2026-09-21: S231 phase one's node-budget label,
#      `adocs/data/S192_node_budget.py` reporting OUTSIDE for a band it had
#      just derived, and "a reduced move that beats alpha is searched again"
#      pinning depth 6 where `adocs/data/S231_research_witness.py` answers 4.
#   3. **Closed by S237's removal, not by a fix**: S237's cold fast check
#      (no mutant moved `handed_reduction` onto another call site, none
#      targeted the give-up `depth >= 2` or the mate-band guards) named code
#      that left the tree with its H0.
#   4. **This step's own goldens now belong to its tree** (DEC-233): the four
#      capture-mate rows were derived on the candidate. R02 separates rows 2,
#      3 and 4 there, C02 rows 1 to 3, C05 row 2; R01 is separated by none.
#   5. **`test_mate_carry`'s budgets are not the DEC-156 rule's answer on
#      either tree**: on this one the rule moves C, D and E, and recording the
#      grid raises C's short-line ceiling 0 -> 1 (a decision, not a
#      re-derivation); at `1680439` itself the rule already moves A, D and E.
#      The test is green at the budgets as they stand. The step file carries
#      both grids' numbers (`adocs/data/S238_carry_sweep_cand.txt`,
#      `S238_carry_sweep_parent.txt`). **Coordinator's ruling, 2026-09-26:**
#      the budgets and ceilings stay as `1680439` has them -- nothing is red,
#      so DEC-233 does not bind, and a ceiling is never raised to admit a
#      grid. The candidate's C reports short mate lines in cells the test does
#      not drive (stride 1 at 4000000: 1 of 1; stride 2 at 2000000: 2 of 6)
#      where the parent reports none: S202's class, open, reachable in play
#      as an `Incomplete mating PV`; the run's per-side count of that warning
#      is read beside the verdict. The TSV's drift from the rule at
#      `1680439` is a test-side filler item for S202's block (DEC-171).
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# THE REFERENCE IS THE TREE S237's VERDICT LEFT: `1680439`, S237's removal,
# unless the coordinator records otherwise. This step was built on `1a35f16`
# with S237's hindsight rule live; S237 read H0, the rule left, and this step
# was rebased onto the removal with its off-value identity proved again
# against it (above) before anything was pinned.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> the rule gains at least 5 nElo at its seeds. **The two
#                   constants stay at their seeds** -- a census percentile and
#                   a midpoint, neither a fit -- and S127 fits them with the
#                   rest of the set; an H1 says the rule is worth having at the
#                   values tried and not where its maximum is. The stopping
#                   run's Elo is upward-biased and is not the effect size.
#
#   H0, interval -> **a loss, and the revert is one default.**
#   wholly         `CutoffCountReduction` to 0, the off value proved above,
#   below zero     so the engine is the reference tree's to the node the moment
#                  it moves -- and **the code leaves with it**: the slot, the
#                  clear, the increment, the read, the two rows, the probe
#                  fields, the cases and the mutants come out in the same step,
#                  behaviour-neutral at the reverted default and discharged by
#                  INV-6. **Unless a reason is stated**; if the code is kept,
#                  the completion stamp says why. The capture-mate rows
#                  DEC-233 re-derived go back byte for byte from the GOLDEN
#                  block that keeps them -- a revert, not a re-mine.
#
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero with
#   or an          the games played and the interval, and read it by the row
#   interval       above: the default goes to its off value with the code
#   reaching       behind it on the same terms. No follow-up run and no second
#   above zero     pair (DEC-063); another seed is a second experiment, which
#                  S127's fit is the place for.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands the cutoff count at its seeds; REF is the tree
# the coordinator pins as its reference -- the landing's parent, which is the
# tree S237's verdict left, `1680439` (above), unless the coordinator records
# otherwise.
# Both are pinned after the landing commit exists, by editing the defaults
# below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-1680439}"
CAND="${CAND:-257c8fe}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S238_sprt.sh before running (CAND is the cutoff" \
         "count's landing commit, REF the tree S237's verdict left)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s238_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
