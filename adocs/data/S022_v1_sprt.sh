#!/usr/bin/env bash
#
# S022 VERDICT 1: S015'S EXCHANGE GATE IN QUIESCENCE, DELETED. Out of check,
# quiescence's filter loop drops a non-capture that is not a queen promotion
# (S131), skips a capture whose best case cannot reach alpha (S112's per-move
# futility), and then declines a move the exchange evaluation writes off --
# S015's gate, `!capture_cannot_lose(...) && !see_ge(..., 0)`. The candidate
# deletes the gate and asks whether the tree is not worse without it. Verdict 2
# of the same step, the node-level delta early-out, is measured later on the
# tree this one leaves.
#
#   nohup adocs/data/S022_v1_sprt.sh > .tuning/sprt_s022_v1.log 2>&1 &
#
# **THE CANDIDATE IS ONE CONDITION AND ITS SWITCH.** In `quiescence`'s filter
# loop the gate gains a leading `QS_SEE_GATE != 0 &&`, and one row in
# `src/search_params.hpp`, `QsSeeGate`, ships at **0**: the gate deleted, and
# neither exchange function called in quiescence. At 1 the engine is the tree
# before this verdict node for node (below). Nothing else in `src/` moves but
# comments: S112's block, S131's condition, ProbCut's own filter in
# `negamax_at` (S113) and S091's main-search SEE pruning are untouched, and so
# are `see()` and `see_ge()`, which those two still call. The in-check path is
# untouched at either value: the gate never ran in check.
#
# DEC-221: implemented from the description in the step file's "Technical
# details" section (section 3's item 1), written from published prose and the
# wiki. No other project's code was opened and no constant of one is behind
# anything here (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134): none. A deletion has no number in it; the gate's own
# bar, 0, is "the exchange loses material", a definition.
#
# THE PRIOR, recorded before the run (DEC-019), direction only and never a
# forecast. **This engine's own record: S015 measured the gate at zero** and
# kept it, when `see()` cost 12.1 % more than it does now and per-move
# futility did not exist (an Apple-machine figure, pre-DEC-049). **The
# surveyed record's overlap claim** (the step file's re-target): once per-move
# futility exists, an older rule that prunes an overlapping set may measure
# nothing, and deleting it can gain -- one engine gained by deleting delta
# pruning after per-move futility, +1.87 +/- 3.51 and +6.74 +/- 5.26 at its
# simplification bounds (Weiss #455). **And the record that keeps H0 live**:
# the same engine measured SEE pruning in quiescence added *on top of*
# futility at +35.73 +/- 12.73 and +24.89 +/- 9.66 (Weiss #355), and another
# reported a futility variant without SEE at -384 (Lynx #1142), both read as
# S112's section 1 records them. A regression on deletion is a published
# outcome, not a straw man, and no figure here is Elo for this engine.
#
# DEC-063'S EXPECTATION, stated before the run: **about zero**, with a real
# chance of a loss. The pair below is the non-regression pair because a
# deletion earns its place by proving it does not cost, and a truth at zero
# sits on H1, where the walk terminates.
#
# DEC-236: this verdict does not decide S112's per-move futility. Futility is
# on both sides of this pair, and of verdict 2's as the step file writes it
# (F+D against F, or F+S+D against F+S), so no reading of either verdict says
# the rule is worthless; it stays through this verdict on every row below,
# and its own worth is S127's fit, as DEC-236 names it.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES, 2026-09-29, on `cbaa699`'s
# engine (S131's tree; `src/` is byte-identical at `6eb2674` and at `d446783`,
# the landing's parent -- DEC-241's commit, documents only), niced. Counts
# only.
#
#   THE REACH, stated as reach (DEC-239): what the gate declines today, and
#   how much of what it would decline per-move futility skips first. From an
#   instrumented copy of the candidate with write-only counters
#   (`.tuning/coord/S022_census_instrumentation.patch`), its tune build at
#   `QsSeeGate` 1 and 0, whose bench totals are the uninstrumented builds' at
#   both values (`.tuning/coord/S022_census.txt`, summary
#   `.tuning/coord/S022_census_summary.txt`). The gate's set is every move
#   past the filter's drop that the exchange evaluation writes off, asked of
#   each such move whether or not the gate reaches it.
#
#   Over the eight bench positions at depth 12, today (`QsSeeGate` 1): 774938
#   moves pass the filter's drop out of check at 248994 nodes -- 768203
#   captures and 6735 queen promotions that take nothing -- and the exchange
#   evaluation writes off **378178 of them, 48.8 %** (371679 captures, 6499
#   promotions). **Per-move futility skips 26425 of those first, 7.0 % of the
#   gate's set** -- 40.4 % of futility's own 65328 skips -- so **the gate
#   declines 351753 moves today, 93.0 % of its set and 45.4 % of the loop's
#   moves** (345254 captures, 6499 promotions), 5283 of them captures that
#   futility would have skipped had they not given check. The gate's own work
#   there is 709610 calls of `capture_cannot_lose()` and 504765 of `see_ge()`.
#   The three `search_bench` positions at depth 12: 374458 moves, 153258
#   written off (40.9 %), 14331 of those skipped by futility first (9.4 %),
#   138927 declined today. With the gate deleted (`QsSeeGate` 0), the bench
#   positions at depth 12 keep 685767 moves the exchange evaluation writes
#   off; 367882 are searched, 96122 raise the node's best and 5484 cut off,
#   and the quiescence nodes go from 626337 to 1410143.
#
#   THE ON VALUE IS PROVED ON THE TREE (DEC-215). A deletion has no setting,
#   so the gate has a switch, in `QsFutility`'s, `ProbCut`'s and
#   `QsQueenPromotions`' shape, the other way round: its 1 is the parent. The
#   tune build at 1 prints `bench` **3429473** with all eight `bestmove`
#   replies identical to a build of `cbaa699` (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2
#   e5e6 e5e6), `bench 12` the parent's 1694808 with its replies, and
#   `tools/search_bench.py` reproduces the parent at depth 9 (48304 / 71580 /
#   25413, c3d5 e2a6 d7c8q) and depth 12 (104784 / 244824 / 117798, c3d5 e2a6
#   d7c8q). `.tuning/coord/S022_identity.log`.
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` 3429473 -> **6049266**,
#   +76.4 %, **all eight replies unchanged**; `bench 12` 1694808 -> 2393854,
#   replies unchanged. `search_bench` depth 9: 48304 -> 39854, 71580 ->
#   249885, 25413 -> 33736; depth 12: 104784 -> 97921, 244824 -> 567770,
#   117798 -> 125308; every best move unchanged. The counts move by
#   construction, so INV-6 does not discharge this change and nothing here is
#   Elo (DEC-019).
#
# THE REFERENCE IS THE COMMIT THE LANDING SITS ON. `REF` is the landing's
# parent, `d446783` (DEC-241's entry, documents only, on `6eb2674`, S243's
# completion, the tree this verdict was rebased onto), whose `src/` is
# `cbaa699`'s byte for byte -- the commit every
# engine number above was measured against; S242 and S243 moved tests and
# documents only, and the tests were re-run on the rebased tree. If anything
# lands between that tree and this verdict's landing, the identity above is
# re-proved on the new parent before the pair is pinned.
#
# BOUNDS. `fastchess.sh --nonreg`, the script's own non-regression mode:
# elo0=-5 elo1=0, alpha=beta=0.05, nElo, `model=normalized`, 20000 rounds.
# The step file's section 6 pre-registers exactly this pair, S105's stated
# non-regression pair: a deletion earns its place by proving non-regression,
# and with the expectation at about zero the truth sits on H1, where the walk
# terminates (DEC-063). The crawl case is a truth near -2.5, between the
# bounds, and the third row below is written for it.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** with the truth at the
# interval's midpoint and **25591** with it on a bound
# (`adocs/testing_strategy.md` section 1.1), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. The midpoint's figure is past
# `--nonreg`'s 40000-game cap, which is about 19.0 h. **The cap is that one**:
# the step file's section 5 (2026-08-19) asked for a cap of about 20000 games
# as "one night" at the throughput then assumed; DEC-143's pricing on this
# machine and `fastchess.sh`'s own cap supersede it, and a walk that reaches
# 40000 games is read as no verdict by the row below. A night, scheduled
# (DEC-155). The throughput is read off the banner and recorded rather than
# assumed.
#
# REGIME. `adocs/data/S131_sprt.sh`'s: 8+0.08, Hash 16, one thread a side,
# `books/noob_3moves.epd` (DEC-189), concurrency 12 (DEC-050), whatever
# governor the machine is under, recorded by the run and not set (DEC-195).
# **No harness change since the last fixed-rounds A/A**; if fastchess, the
# book, the adjudication or the machine moves before this starts, the A/A of
# 1000 fixed rounds comes first and is read with `adocs/data/S105_pairs.py`
# (DEC-143).
#
# WHERE THE OUTPUT LANDS. `OUT` is set in the exec line below, under
# `.tuning/`, which since DEC-235 is also `fastchess.sh`'s own default; set
# here so the run's directory is named for the step.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# carried by id from `adocs/data/S131_sprt.sh`'s block as it stood on
# 2026-09-29, after S131's H1, and re-read on the tree this landing sits on,
# `d446783` (`6eb2674` plus DEC-241's entry), after S242 and S243 completed: **no defect reachable in ordinary
# play, on the UCI surface, or able to move a reported score, move or line is
# open.** What is open is test-side, and none of it is touched here but items
# 9 and 11; items 8 and 10 are closed and kept under their ids:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding.
#   2. The three S097 named on 2026-09-21: S231 phase one's node-budget label,
#      `adocs/data/S192_node_budget.py` reporting OUTSIDE for a band it had
#      just derived, and "a reduced move that beats alpha is searched again"
#      pinning depth 6 where `adocs/data/S231_research_witness.py` answers 4.
#   3. **`test_mate_carry`'s budgets are not the DEC-156 rule's answer**
#      (S238's item 5, with the coordinator's ruling of 2026-09-26). S202's
#      class of short mate lines is read beside the verdict as an
#      `Incomplete mating PV` count per side; S131's run read 1 against 17
#      (candidate against reference, `adocs/data/S131_sprt_pairs.txt`), and
#      this run's is read beside it. This candidate raises one of the
#      budgets' ceilings (item 11).
#   4. **S113's goldens belong to its tree** (DEC-233): the mined ProbCut row
#      of "pruning does not hide a forced mate", S097's multicut row (item 9)
#      and the capture-mate table, which item 11 re-derives here.
#   5. **S112's block's items 4 and 5 are resolved** (DEC-236).
#   6. **ProbCut's preliminary enters quiescence at a child `negamax_at`
#      never screened** (S113's fast check; S244 is its filler): a capture
#      that leaves insufficient material is scored by quiescence and stored at
#      `TT_DEPTH_QS`; no cut is wrong. Not touched here; the deletion lets
#      more captures through that preliminary's quiescence, so it reaches the
#      class more often, and the class is still no wrong cut.
#   7. **S131's own goldens: none**, and none went red on S131's trees.
#   8. **S242, `test_engine`'s capture flake: closed** (`aef0257`, DEC-240).
#      A doctest listener in `tests/test_helpers.hpp` now repeats a failure on
#      stderr from inside a `stdout_capture_t`, and the first-iteration case
#      asserts what holds under any scheduling -- its hard-timer half a bound
#      on when the search ends, once, never retried until it passes.
#   9. **E21's coverage: two killers now, and neither is open.** S131's
#      guard-mode row, `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2
#      42`, stopped separating E21 on this candidate -- on `cbaa699`, before
#      S243 landed, a targeted mutation run scored E21 a survivor, 0 of 41
#      red -- and the miner's guard mode re-derived the row (DEC-238):
#      `2r4r/kq3pb1/N3p1p1/QPp1Pn1p/2PPRP2/7P/5BP1/R5K1 w - - 1 31` at depth
#      14, mate in 6, red under E21 and green shipped
#      (`adocs/data/S022_remine_s097.log`). On the rebased tree E21 is killed
#      by S243's direct case, "the multicut never ends a node on a mate from
#      its verification", and by that row, which stays as the second witness
#      S243 keeps. An H0 puts S131's row back; the direct case holds either
#      way.
#  10. **S243, a direct guard test for the multicut's mate band: closed**
#      (`6eb2674`). The guard's coverage no longer rests on a mined row.
#  11. **This verdict's own goldens** (DEC-233, DEC-142). Four moved with the
#      deletion and were re-derived by the scripts their GOLDEN blocks name,
#      each block keeping the rows it replaced: S097's multicut row (item 9),
#      the capture-mate table of
#      "pruning does not hide a forced mate" (the seven sweeps of
#      `adocs/data/S230_mine_r01_row.py depths`, depths 9, 7, 10, 12 in place
#      of S113's 7, 9, 11, 10, labels `no S091 mutant, since S022`, `no S091
#      mutant, since S095`, `R02`, `R01`), the band of "ordering keeps the
#      tree small" (a count of 127342 against the parent's 20427, the pair
#      509368 / 25468 in place of 65024 / 3251), and `test_mate_carry`'s
#      `short_line_ceiling` for `C_mate7_depth11` (the grid
#      `adocs/data/S022_v1_sweep.txt` added to the `--ceilings` command, as
#      S236 v2's grid once was): **DEC-241** moves C's ceiling from 0 to 6 in
#      the landing commit, the grid the fifth recorded sweep and named in the
#      golden's re-derivation line, in the form DEC-225 set for S095's. On an
#      H0 or no verdict all four go back byte for byte with the gate, the
#      ceiling by dropping the grid from the `--ceilings` command (DEC-241).
#  12. **S245, a filler for five stale things** the fillers of 2026-09-28/29
#      noted and did not touch (`adocs/plan_todo/S245_*.md`): a source comment
#      quoting 42371 nodes where the engine reports 36165; the eight-queens
#      13.8 ms depth-1 golden reading 6 to 7 ms today, its floor holding;
#      `tools/plan_prose_check.py` reporting a citation MISSING when its quoted
#      title wraps across two string literals; the S237/S238 "needs fractional
#      reductions" comments; `S170_cases.tsv`'s budgets against the DEC-156
#      rule. None reaches play or a reported score; none is touched here.
#
#   The coordinator re-reads the list on the day it pins the pair and amends
#   this block if it has moved.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
#
#   H1 accepted  -> **not a regression of 5 nElo or more, and the deletion
#                   stands.** The gate, the switch and the switch's own cases
#                   leave in one removal commit, proved by INV-6 against the
#                   candidate's tree: `bench` 6049266 with the eight replies
#                   above and `search_bench` identical at 9 and 12. The cases
#                   that assert the deletion -- a losing capture searched, a
#                   queen promotion onto a defended square searched -- stay,
#                   their switch assertions dropped; the tune-only cases at 1
#                   leave with the switch, S131's "the exchange gate declines
#                   a queen promotion onto a defended square" among them;
#                   S015's own `see()` and `see_ge()` cases stay, ProbCut and
#                   S091 still calling both. Mutants whose target leaves with
#                   the gate -- U01 to U05, M18, V07, and the gate halves of
#                   V09 and V10 -- are retired or re-cut, ids never reused.
#                   The goldens of item 11 stay, being the candidate's
#                   tree's.
#                   The coordinator records the decision that supersedes
#                   S015's kept-at-zero, and specs.md's rows say the gate is
#                   gone. The stopping run's Elo is upward-biased and is not
#                   the effect size (DEC-063).
#
#   H0 accepted  -> **a regression, and the gate stays.** `QsSeeGate` to 1,
#                   the parent's tree proved above, and **the switch's code
#                   leaves with it** (the S238 pattern): `src/`, `tests/`,
#                   `tools/` and `MANUAL.md` go back to the landing's parent,
#                   the goldens of item 11 byte for byte with them and
#                   `DEV_MANUAL.md`'s golden table with those (the node band,
#                   the six ceilings, the capture-mate rows, the multicut
#                   row, the default count), proved by
#                   INV-6 against that parent (`bench` 3429473 with its eight
#                   replies, `search_bench` identical at 9 and 12). S015's
#                   zero stands, with a current number beside it: this run's
#                   interval, against a tree with per-move futility in it.
#                   Keeping the switch is not foreseen; if a reason is given,
#                   the completion stamp states it.
#
#   No verdict   -> **the null action: the gate stays, the same way as H0**,
#   at the cap     and the run is recorded as a zero with the games played and
#                  the interval (DEC-063, the S094/DEC-079 precedent). No
#                  follow-up run and no second pair; a wider pair, another
#                  bar or a deletion by class is a second experiment.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands this verdict's candidate, `QsSeeGate` 0; REF is
# its parent, the commit the landing sits on. Both are pinned after the landing
# commit exists, by editing the defaults below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-d446783}"
CAND="${CAND:-a97bc1a}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S022_v1_sprt.sh before running (CAND is S022 verdict" \
         "1's landing commit, REF its parent, the commit the landing sits" \
         "on)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s022_v1_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh --nonreg
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
