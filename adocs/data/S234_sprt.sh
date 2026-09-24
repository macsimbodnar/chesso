#!/usr/bin/env bash
#
# S234: where a node's table entry holds a score whose bound points the same way
# as the gap between that score and the static evaluation, **the reverse
# futility margin is subtracted from the table's score** instead of from the
# static one, and the bound the rule returns is that same number less the
# margin. The raw static evaluation stays what is stored in the entry's
# evaluation field and what the improving flag compares across plies.
#
#   nohup adocs/data/S234_sprt.sh > .tuning/sprt_s234.log 2>&1 &
#
# **THE CANDIDATE IS ONE CHANGE.** One local at one site, behind one switch.
# The rule that computes the estimate -- which entries may replace the static
# score, and with what -- landed with S109 and is untouched here; the futility
# margin in the move loop has read it ever since and still does at either
# setting of this switch, because that is the tree S109's own verdict measured.
# So an H1 attributes to the reverse-futility site and to nothing else, and
# there is no bisection to pre-register: a single switch has no parts.
#
# **WHAT THE STEP FOUND THAT ITS FILE DID NOT SAY.** The step's `accepts` names
# "the null-move static-score condition" as a second site. **This engine has
# no such condition**: `negamax_at`'s null-move block is guarded on the node
# and the window -- not PV, not in check, `ply > 0`, a real previous move, no
# exclusion, a reduced depth that keeps a ply, both mate-band edges of beta and
# a non-zero game phase -- and it never compares a static score against
# anything. Razoring, the third site the file names, does not exist until S116.
# So the routing this step performs has exactly one destination, which is why
# the switch is named for that site. Recorded here because a reader comparing
# the accepts against the diff would otherwise look for a change that could not
# be made.
#
# **WHAT IS IN `src/`, WHETHER OR NOT IT MOVES A GAME.** One declaration inside
# the reverse-futility block, `rfp_eval`, which is `pruning_eval` at
# `RfpTtEstimate` 1 and `static_eval` at 0; the comparison and the return then
# both read it. One row in `src/search_params.hpp`'s X-macro. Nothing else.
#
# DEC-221: implemented from the description in
# `adocs/data/2026-09-19_search_technique_study.md` row N1 and from this step's
# own file, which is what the implementing agent's brief carried. No other
# project's code was opened and no constant of one is behind any value here
# (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEEDS (DEC-134): **there is no constant to seed.** The tightening is a
# comparison between two numbers the node already has, and the conditions it
# runs under -- the two bound types, the mate band, in check -- are conditions
# and not margins. `RfpTtEstimate` is a switch whose range is 0 to 1 by stated
# purpose. The step looked for a guard worth declaring (a minimum entry depth,
# the obvious candidate) and did not add one: a guard shipped untested inside
# this candidate would be a second change the verdict could not attribute.
#
# THE RETURNED BOUND IS A CHOICE AND IT IS STATED. The site returns
# `rfp_eval - margin` and not `static_eval - margin`. On the lower-bound branch
# that is the weaker of two claims the entry already certifies -- a
# `TT_BETA_NODE` says a search of this position came back at or above its
# score, so the score less a margin is below something already proved -- and on
# the upper-bound branch it hands back the smaller number, which a fail-soft
# return may always do. Returning the static bound while deciding on the
# estimate was the alternative and it was rejected: the node would hand its
# parent a bound its own test did not argue for.
#
# THE PRIOR, recorded before the run (DEC-019, DEC-222).
#
#   **The record's figure is +6.07 over 8414 games** for this tightening at
#   every margin site (the 2026-09-19 study's row N1). It is a direction and
#   not an expectation, and it is a figure for a search with more margin sites
#   than this one has: the record applies it at reverse futility, at a
#   null-move static condition and at razoring, and chesso has only the first.
#
#   **This engine has measured the narrow form of the same idea at zero.**
#   S130 put the table's score at the quiescence stand-pat and read
#   `Elo +1.14 +/- 4.04` over 16784 games, no verdict, recorded as zero and
#   kept (DEC-103). The hypothesis this step tests is that the stand pat was
#   the wrong place -- the substitution fired on 0.23 % of those sites -- and
#   that the main search's margins are where the mechanism has something to
#   move. That is a hypothesis about a recorded chesso result and not a claim
#   about a number.
#
#   Three published figures have transferred to this engine as 0, 0 and the
#   wrong sign (DEC-019). The bounds below are sized from the expectation and
#   not from the record.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES:
#
#   THE OFF VALUE IS PROVED ON THE TREE and not declared (DEC-215), measured
#   2026-09-24. A Release build with `RfpTtEstimate` defaulted to 0 prints
#   **4493659**, the reference commit's own total to the node, with all eight
#   `bestmove` replies identical (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6),
#   and `tools/search_bench.py` reproduces it exactly at both depths:
#   21479 / 102462 / 33148 with g5f6 / e2a6 / d7c8q at depth 9 and
#   149688 / 459216 / 219544 with c3d5 / e2a6 / d7c8q at depth 12. That is the
#   equality an H0's revert returns to and it is INV-6's own form.
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` **4493659 -> 4803214,
#   +6.89 %**, with one of the eight replies moving -- kiwipete's `e2a6` ->
#   `d5e6`. `search_bench` disagrees with the total about the sign, which is the
#   shape of the change rather than a surprise: at depth 9 midgame **grows**
#   21479 -> 48522 with its best move moving `g5f6` -> `c3d5` while kiwipete
#   falls 102462 -> 85714 and tactical 33148 -> 28080; at depth 12 all three
#   shrink, 149688 -> 129499, 459216 -> 411457, 219544 -> 172984, every best
#   move the parent's. The node counts move by construction and in both
#   directions -- a certified lower bound above the static score buys a cutoff
#   the node did not have, a certified upper bound below it takes one away -- so
#   INV-6 does not discharge this change and nothing here is Elo (DEC-019).
#
#   AND WHAT IT COSTS ON A COLD FIXED-DEPTH MATE SWEEP, recorded because it is
#   the one measurement here that points down. Over the 141 positions of S095's
#   mate candidate set at depths 3 to 12 -- 1410 cells, the regime `search_fen()`
#   and the mining driver use and not the iterative deepening a game plays --
#   the candidate reports **489 mate cells against the parent's 497**, sixteen
#   rows losing one somewhere and nine gaining one
#   (`adocs/data/S234_remine.log`). One of the sixteen is the row
#   `tests/test_search.cpp` "pruning does not hide a forced mate" carried for
#   S095: the row was re-mined by its own script on this tree (DEC-142), the
#   position and its distance survived and only the depth moved, 11 to 8. The
#   instruments that are not cold single-depth searches are unmoved --
#   `test_engine`'s mate safety over 48 constructed mates, `test_mate_carry`'s
#   grid, and that row's own mate in 2 at every depth from 3 to 12 through
#   iterative deepening with node counts identical to the parent's. A
#   fixed-depth mate count is a reading of a different tree and not of a better
#   one (DEC-019), and this run is what prices it.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the change is proposed as a
# gain and a `{-5, 0}` pair would accept H1 on a truth of zero and ship a rule
# whose whole content is which of two numbers a margin reads. DEC-063's rule is
# that the pair straddles the expected effect: the record's +6.07 sits above
# elo1 and the engine's own reading of the narrow form sits at zero, so the
# interval brackets the two priors that exist.
#
# WHAT THE PAIR COSTS (DEC-143). **41861 expected games** at the interval's
# midpoint and **25591** with the truth on a bound (`adocs/testing_strategy.md`
# section 1.1, `adocs/plan.md`'s cost table), at **2110 games an hour** from
# `.moltke.local.md`: **19.8 h** and **12.1 h**. A night, scheduled (DEC-155).
# The throughput is read off the banner and recorded rather than assumed.
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
# **1.0 % on either side**, counted per side from the run's own PGN in `$OUT`
# with `tools/forfeit_report.py`. A crash or a disconnect on either side voids
# it outright (`SPRT-RUN-INVALID`, S212). Mains, and a second load on the
# machine. Nothing else is an abort.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# read off `adocs/plan.md`'s Open list and `adocs/status.md` on 2026-09-24.
# **No defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** What is open is test-side:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding. On this tree
#      the case runs 19.3 s Release against its 120 s ceiling; the candidate
#      does not move it materially. Ceiling untouched (TESTS).
#   2. The three S097 named and re-checked on 2026-09-21, each still open and
#      none of them touched here: S231 phase one's node-budget label of 17321
#      against the reverted tree's 16256; `adocs/data/S192_node_budget.py`
#      reporting OUTSIDE for a band it had just derived; and
#      `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pinning depth 6 where `adocs/data/S231_research_witness.py`
#      answers 4.
#   3. **Closed, not open, and named so that a reader of S236's own
#      pre-registration does not carry it forward**:
#      `E21_multicut_mate_band_gate_dropped`, which survived on S236's candidate
#      tree, is killed again on the tree this run measures against -- the row
#      that kills it came back with S236's removal (`0792ef1`), and the removal's
#      own mutation pass scored it killed. S239's two refusals in
#      `tools/mutation_check.py` are landed, and this step's pass ran on the
#      repaired tool.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
# There is no bisection: one switch, one site, one change.
#
#   H1 accepted  -> the estimate at the reverse-futility margin gains at least
#                   5 nElo. **`RfpTtEstimate` stays at 1** and the switch stays
#                   in the parameter set, because it is what a later step needs
#                   to re-ask the question when S116's razoring adds a second
#                   site. S116's own site joins under its own verdict and not
#                   under this one. **The re-mined mate row is then the one that
#                   ships**, and the eight mate cells the cold fixed-depth sweep
#                   loses are what the gain was bought with -- said in the
#                   completion stamp rather than left to the ledger. The
#                   stopping run's Elo is upward-biased and is not the effect
#                   size (DEC-063).
#
#   H0, interval -> **a loss, and the revert is one default.** `RfpTtEstimate`
#   wholly         moves to 0, which is the off value proved above, so the
#   below zero     engine is the parent's to the node the moment that default
#                  moves -- and **the code leaves with it**: the switch, the
#                  `rfp_eval` local, the six cases and the four mutants come
#                  out in the same step. That removal is behaviour-neutral at
#                  the reverted default and INV-6 discharges it, which is the
#                  shape S098 verdict 3's leg-1 removal already has, so no
#                  second SPRT is owed for taking it out.
#                  **Unless a reason is stated** -- the only one that would
#                  count is a later step needing the routing live, and none
#                  does today: S116's razoring brings its own site and prices
#                  it itself. If the code is kept, the completion stamp says
#                  why, so that a later reader does not read surviving code as
#                  an unmeasured rule.
#                  **S095's mate row goes back with it**: the flip restores the
#                  parent's tree to the node, so its depth returns from 8 to 11
#                  as a revert of one literal and not as a second re-mine --
#                  the old depth is kept in the case's own block for exactly
#                  that, which is how S188's and S236's instances closed.
#
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero with
#   or an          the games played and the interval, and read it by the row
#   interval       above: a rule that cannot be told from its own off value over
#   reaching       40000 games has not been shown to exist, the narrow form of
#   above zero     the same idea already has a recorded zero (DEC-103), and the
#                  default goes to 0 with the code behind it on the same terms.
#                  There is no follow-up run and no second pair -- re-running
#                  until a bound is hit is how an alpha of 0.05 stops meaning
#                  0.05 (DEC-063), and the one thing that would make this worth
#                  asking again is a second margin site, which S116 brings.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands the switch at 1; REF is its parent, the tree
# whose reverse futility reads the static score. Both are pinned by the
# coordinator after the landing commit exists, by editing the defaults below:
#
#   git -C /home/max/ws/chesso rev-parse --short HEAD     # the landing, CAND
#   git -C /home/max/ws/chesso rev-parse --short HEAD^    # its parent, REF
#
# Until both are pinned this script refuses (DEC-020). The banner prints both
# shas with their commit dates before the first game, so what the run measures
# is on screen and not assumed.
REF="${REF:-5180a10}"
CAND="${CAND:-169b4cb}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S234_sprt.sh before running (CAND is the switch's" \
         "landing commit, REF is its parent)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s234_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
