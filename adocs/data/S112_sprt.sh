#!/usr/bin/env bash
#
# S112: PER-MOVE FUTILITY IN QUIESCENCE. Out of check, a capture whose best
# case -- the stand pat, a margin, and the value of what it takes -- is still
# at or below alpha is skipped in quiescence's filter loop, before S015's
# exchange gate is asked, and that best case is folded into the node's
# fail-soft value instead of being dropped.
#
#   nohup adocs/data/S112_sprt.sh > .tuning/sprt_s112.log 2>&1 &
#
# **THE CANDIDATE IS ONE RULE.** In `quiescence`: `futility_base` once per node
# from the stand pat after S130's substitution, under `!in_check`; in the
# filter loop, above the exchange gate, a capture that is not a promotion is
# priced by `qs_futility_value` (a table owned by the search, indexed by
# piece_t, en passant's victim a pawn), and if `futility_base + victim <=
# alpha` it is made, asked `is_check`, unmade, and skipped unless it checks;
# the largest skipped best case is folded into `best_value`, stored as an
# upper bound. Two rows in `src/search_params.hpp`: `QsFutility` 1, the switch,
# and `QsFutilityMargin` 188. S015's gate is untouched and no delta pruning
# exists in the tree before or after (S022 re-decides both on this verdict).
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section, which was written from published prose and the wiki. No
# other project's code was opened and no constant of one is behind any value
# here (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134), stated at their site. `QsFutilityMargin` 188 is (a):
# the wiki's Delta Pruning page, "some safety margin (typically around 200
# centipawns)", read as two pawns in chesso's own material scale, 2 * 94. The
# victim table {100, 300, 300, 500, 900} is (b): chesso's own exchange scale,
# `see_value`, copied and not shared. Both are fitted at S127, together.
#
# THE PRIOR, recorded before the run (DEC-019). **The record reads 0 to +30,
# both endpoints published**, and it is direction only: one engine without a
# quiescence SEE gate measured the rule large; one engine with one -- chesso's
# configuration, S015's gate being live -- measured it at about zero three
# times. This engine's own published-to-measured transfers read mostly zero.
# The bounds are sized from the expectation, not from the record.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES by the implementing agent on
# the landing's working tree, 2026-09-26, against a parent built from
# `1680439` in a throwaway worktree:
#
#   THE FIRE RATE, before a game was booked (the step file's section 6): over
#   the eight bench positions at depth 12, 992064 captures reach the filter
#   loop out of check, 954340 are tested (37724 promotions exempt), 116176
#   fall below the threshold, 12319 of those give check and are kept, and
#   **103857 are skipped -- 10.47 % of filter-loop captures**. Not near zero,
#   so the zero is not predicted before it is bought.
#   `.tuning/coord/S112_fire_d12.txt`; the instrumented copy benches the
#   candidate's own 1981759 at depth 12, so its counters are write-only.
#
#   THE OFF VALUE IS PROVED ON THE TREE (DEC-215). No value of the margin is
#   off -- at 0 the rule still skips a capture whose victim alone cannot reach
#   alpha -- so the switch is `QsFutility`, the precedent `SeExtend` and
#   `RfpTtEstimate` set. The tune build at `QsFutility` 0 prints `bench`
#   4803214 with all eight `bestmove` replies identical to the parent's
#   (c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6), and `tools/search_bench.py`
#   reproduces it at depth 9 (48522 / 85714 / 28080, c3d5 e2a6 d7c8q) and
#   depth 12 (129499 / 411457 / 172984, c3d5 e2a6 d7c8q).
#   `.tuning/coord/S112_identity.log`.
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` 4803214 -> 4649650,
#   -3.20 %, the eight replies unchanged. `search_bench` depth 9: 48522 ->
#   53598, 85714 -> 80389, 28080 -> 25691; depth 12: 129499 -> 154098,
#   411457 -> 472358, 172984 -> 107876; every best move unchanged. The counts
#   move by construction, so INV-6 does not discharge this change and nothing
#   here is Elo (DEC-019).
#
# THE REFERENCE IS THE TREE S238's VERDICT LEAVES. This step was built on
# `1680439` while S238's SPRT measured the cutoff count against it. The
# coordinator rebases S112 onto whatever S238's verdict leaves -- the cutoff
# count kept, or removed back to `1680439` -- and **re-proves the off-value
# identity above against that tree before pinning**: `QsFutility` 0 must bench
# the reference's own total with all eight replies and reproduce its
# `search_bench` at 9 and 12. The candidate numbers above are then re-taken
# on the rebased tree, and DEC-233's capture-mate re-derivation is re-run if
# the rebase moves the tree the rows were mined on.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the rule is proposed as a gain.
# DEC-063: the pair straddles the expected effect -- the record's top sits far
# above elo1 and the configuration-matched record sits at zero.
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
# carried from `adocs/data/S238_sprt.sh`'s block as it stood on 2026-09-26:
# **no defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** What is open is test-side, and none
# of it is touched here:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding.
#   2. The three S097 named on 2026-09-21: S231 phase one's node-budget label,
#      `adocs/data/S192_node_budget.py` reporting OUTSIDE for a band it had
#      just derived, and "a reduced move that beats alpha is searched again"
#      pinning depth 6 where `adocs/data/S231_research_witness.py` answers 4.
#   3. **`test_mate_carry`'s budgets are not the DEC-156 rule's answer** at
#      `1680439` (S238's item 5, with the coordinator's ruling of 2026-09-26);
#      green here at the budgets as they stand, and S202's class of short mate
#      lines is read beside the verdict as an `Incomplete mating PV` count per
#      side.
#   4. **This step's own goldens now belong to its tree** (DEC-233): the four
#      capture-mate rows of "pruning does not hide a forced mate" were
#      re-derived on the candidate -- depths 7, 9, 11, 9, labels C02/C05/R02,
#      none, R02, C05/C07, every distance unchanged -- and S095's rows are
#      quoted in the case's GOLDEN block for an H0 to restore.
#   5. Whatever S238's verdict adds or closes; the coordinator merges its
#      block in at the rebase.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> the rule gains at least 5 nElo at its seeds. **The margin
#                   and the victim table stay at their seeds** -- one
#                   literature value in chesso's scale and one copy of the
#                   exchange scale, neither a fit -- and S127 fits them
#                   together with the rest of the set; an H1 says the rule is
#                   worth having at the values tried and not where its maximum
#                   is. S022 then takes this tree as its baseline. The stopping
#                   run's Elo is upward-biased and is not the effect size.
#
#   H0, interval -> **a loss, and the revert is one default.** `QsFutility` to
#   wholly         0, the off value proved above, so the engine is the
#   below zero     reference tree's to the node the moment it moves -- and
#                  **the code leaves with it**: the two rows, the table and its
#                  probe, the block in `quiescence`, the seven cases and the six
#                  mutants come out in the same step, the capture-mate rows go
#                  back to the ones quoted in their GOLDEN block, behaviour-
#                  neutral at the reverted default and discharged by INV-6.
#                  **Unless a reason is stated** -- the only one that would
#                  count is S022 wanting the rule present to measure S015's
#                  gate and delta pruning against it; if the code is kept, the
#                  completion stamp says why. The first bisect lever, if one
#                  is ever wanted, is the step file's: `futility_base` from the
#                  pre-substitution static score instead of S130's stand pat.
#
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero with
#   or an          the games played and the interval, and read it by the row
#   interval       above: the switch goes to 0 with the code behind it on the
#   reaching       same terms. No follow-up run and no second pair (DEC-063);
#   above zero     another margin is a second experiment, which S127's fit is
#                  the place for. S022 takes the zero as its baseline.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands per-move futility at its seeds; REF is the tree
# S238's verdict leaves, onto which this step is rebased before pinning. Both
# are pinned after the landing commit exists, by editing the defaults below:
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
         "adocs/data/S112_sprt.sh before running (CAND is per-move futility's" \
         "landing commit, REF the tree S238's verdict leaves)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s112_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
