#!/usr/bin/env bash
#
# S235: a node pruned by reverse futility **returns a point between beta and the
# bound its own test argued** -- `RfpReturnWeight` hundredths of the way up from
# the first to the second -- instead of that bound itself. The comparison, the
# margin, the guards and which number the margin is subtracted from are all
# untouched; what moves is the value the parent is handed when the node prunes.
#
#   nohup adocs/data/S235_sprt.sh > .tuning/sprt_s235.log 2>&1 &
#
# **THE CANDIDATE IS ONE CHANGE.** One expression at one site, behind one
# weight. The block's condition is the same condition, so at any node the
# decision to prune does not depend on the weight -- proved by the drive in
# tests/test_search.cpp, which asserts the cutoff at every weight it walks --
# while the set of nodes a search reaches does move, because the value handed
# back moves the parent's window (the node counts below say so); and nothing
# else in the search reads the parameter. So an H1 attributes to the reverse-futility return
# and to nothing else, and there is no bisection to pre-register: a single
# weight has no parts.
#
# **INDEPENDENT OF S234'S VERDICT, AND THAT IS STRUCTURAL.** S234 moved which
# number the margin is subtracted from (`RfpTtEstimate`); this step blends
# whatever that number turns out to be with beta. If S234's verdict leaves the
# switch at 1 the blend runs over the table-tightened estimate, and if it takes
# the switch out the blend runs over the static score -- the same expression,
# the same weight, the same off value. `REF` below is therefore **the tree
# S234's verdict leaves**, pinned by the coordinator after that verdict closes,
# and this candidate is that tree plus one expression.
#
# **WHAT IS IN `src/`, WHETHER OR NOT IT MOVES A GAME.** Two locals inside the
# reverse-futility block's return -- `rfp_bound`, the number the test argued,
# and `rfp_blended`, the point returned -- with three asserts that hold the
# returned value at or above beta, at or below that bound and strictly inside
# the mate band. One compile-time constant, `RFP_RETURN_SCALE`, and its probe.
# One row in `src/search_params.hpp`'s X-macro. Nothing else.
#
# DEC-221: implemented from the description in
# `adocs/data/2026-09-19_search_technique_study.md` row N2 and from this step's
# own file, which is what the implementing agent's brief carried. No other
# project's code was opened and no constant of one is behind any value here
# (DEC-016, DEC-104, DEC-105, DEC-134).
#
# THE SEED (DEC-134), one constant and one form. `RfpReturnWeight` is **(c), the
# midpoint of its declared range**, 50 of a range 0 to 100 stated by purpose:
# 100 returns the site's own bound and is the tree before this step, 0 returns
# beta exactly, and nothing outside those two is a setting of this rule. No
# published number seeds it -- the record carries an interpolation weight and it
# is that engine's tuned output, which is never a seed here wherever it is
# republished (DEC-084 as amended by DEC-105) -- and the midpoint is a midpoint
# and not a value chosen for this engine. S127 fits it after the block.
#
# THE ROUNDING IS A CHOICE AND IT IS STATED. The gap is non-negative at this
# site, so the integer division floors and the point returned is the one
# **below** the exact share: where the weight cannot be honoured exactly the
# node says the weaker of the two available things. `tests/test_search.cpp`
# "the blend rounds toward beta" holds that as a property of the returned value
# rather than as a repeat of the expression.
#
# THE PRIOR, recorded before the run (DEC-019, DEC-222).
#
#   **The record's figure is +9.71 over 4654 games** for this site alone (the
#   2026-09-19 study's row N2). It is a direction and not an expectation. The
#   study's three further fail-middle sites -- the ProbCut return, the multicut
#   return and the quiescence stand pat, +1.4 to +4.5 each -- are other steps
#   under their own verdicts and none of them is in this candidate; two of the
#   three do not exist in this engine yet.
#
#   **This engine has no prior of its own on a fail-middle return**, at any
#   site. What it has is a long record of published figures not transferring:
#   staged move generation quoted at 30 to 50 Elo and measured 0, SEE pruning in
#   quiescence measured 0, capture ordering reported near 150 Elo and measured
#   slower, S188's check extension +13.65 in the record and -13.84 here. The
#   bounds below are sized from the expectation and not from the record.
#
#   **A mechanism note, recorded so the reading is not invented afterwards.**
#   Lowering what a pruned node returns cannot change which nodes prune; it
#   changes what parents do with the answer -- a smaller fail-soft bound is a
#   weaker claim, so a parent's own best score and the entry it stores carry
#   less, and the aspiration window above sees a different number. That is the
#   whole of the effect and it is why node counts move at all.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES -- FILLED IN WHEN THE MACHINE IS
# FREE, and the placeholders below are refused by the coordinator rather than
# run past:
#
#   THE OFF VALUE IS PROVED ON THE TREE and not declared (DEC-215). A Release
#   build with `RfpReturnWeight` defaulted to 100 must print the reference
#   commit's own bench total with all eight `bestmove` replies identical, and
#   `tools/search_bench.py` must reproduce that commit exactly at depths 9 and
#   12. That is the equality an H0's revert returns to and it is INV-6's own
#   form.  MEASURED: <off-value bench total, eight replies, search_bench rows>
#
#   AND THE SHIPPED TREE AGAINST THE PARENT. `bench` <parent> -> <candidate>,
#   <percent>, with <n> of the eight replies moving; `search_bench` at depths 9
#   and 12.  The node counts move by construction -- every parent of a pruned
#   node gets a different number and orders and stores differently afterwards --
#   so INV-6 does not discharge this change and nothing here is Elo (DEC-019).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the gainer pair, because the change is proposed as a
# gain and a `{-5, 0}` pair would accept H1 on a truth of zero and ship a rule
# whose whole content is how much of a static bound a node is allowed to claim.
# DEC-063's rule is that the pair straddles the expected effect: the record's
# +9.71 sits above elo1 and this engine's own transfer record sits at zero, so
# the interval brackets the two priors that exist.
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
# moves before this starts, the A/A of 1000 fixed rounds comes first and is read
# with `adocs/data/S105_pairs.py` (DEC-143).
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
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# read off `adocs/plan.md`'s Open list and `adocs/status.md` on 2026-09-24.
# **No defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** What is open is test-side:
#
#   1. **`test_mate_breadth`'s headroom**, S188's 2.6 % finding: the case runs
#      about 19 s Release against its 120 s ceiling. Ceiling untouched (TESTS).
#   2. The three S097 named and re-checked on 2026-09-21, each still open and
#      none of them touched here: S231 phase one's node-budget label of 17321
#      against the reverted tree's 16256; `adocs/data/S192_node_budget.py`
#      reporting OUTSIDE for a band it had just derived; and
#      `tests/test_search.cpp` "a reduced move that beats alpha is searched
#      again" pinning depth 6 where `adocs/data/S231_research_witness.py`
#      answers 4.
#   3. **S095's mined mate row belongs to whichever tree S234's verdict
#      leaves**, and this run does not move it: on an S234 H1 the row stands at
#      the depth that step re-mined it to, on an S234 H0 it returns with the
#      revert, and this candidate changes neither -- the case is green on the
#      tree this run measures, which is the condition the step completed under.
#      Named here so a reader does not carry S234's item forward as this run's.
#
# PRE-REGISTERED INTERPRETATION, written before a game is played (DEC-063).
# There is no bisection: one weight, one site, one change.
#
#   H1 accepted  -> the fail-middle return at reverse futility gains at least 5
#                   nElo. **`RfpReturnWeight` stays at its seed of 50**, which
#                   is a midpoint and not a fit, and S127 is where it is fitted
#                   with the rest of the set -- an H1 says the rule is worth
#                   having at the one value that was tried and says nothing
#                   about where the maximum is. The other three fail-middle
#                   sites stay separate steps under their own verdicts (S113 and
#                   S097 bring two of them; the quiescence stand pat is the
#                   third). The stopping run's Elo is upward-biased and is not
#                   the effect size (DEC-063).
#
#   H0, interval -> **a loss, and the revert is one default.** `RfpReturnWeight`
#   wholly         moves to 100, its range top and the off value proved above,
#   below zero     so the engine is the reference tree's to the node the moment
#                  that default moves -- and **the code leaves with it**: the
#                  weight, the two locals, the scale and its probe, the five
#                  cases and the four mutants come out in the same step. That
#                  removal is behaviour-neutral at the reverted default and
#                  INV-6 discharges it, which is the shape S098 verdict 3's
#                  leg-1 removal and S236's removal already have, so no second
#                  SPRT is owed for taking it out.
#                  **Unless a reason is stated** -- the only one that would
#                  count is a later fail-middle site wanting the scale and its
#                  probe, and none does today: each of the other three sites
#                  brings its own weight under its own verdict. If the code is
#                  kept, the completion stamp says why, so that a later reader
#                  does not read surviving code as an unmeasured rule.
#
#   No verdict   -> a stalled walk is a zero. Terminate it, record the zero with
#   or an          the games played and the interval, and read it by the row
#   interval       above: a rule that cannot be told from its own off value over
#   reaching       40000 games has not been shown to exist, and the default goes
#   above zero     to 100 with the code behind it on the same terms. There is no
#                  follow-up run and no second pair -- re-running until a bound
#                  is hit is how an alpha of 0.05 stops meaning 0.05 (DEC-063) --
#                  and a second weight is not a follow-up but a second
#                  experiment, which S127's fit is the place for.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path. COMMITS as amended by DEC-220: the commit
# that closes this verdict carries fastchess's own result block before its
# `Bench:` line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# CAND is the commit that lands the blended return with `RfpReturnWeight` at 50;
# REF is its parent, the tree whose reverse futility returns its own bound.
# Both are pinned by the coordinator after the landing commit exists, by editing
# the defaults below:
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
         "adocs/data/S235_sprt.sh before running (CAND is the blended" \
         "return's landing commit, REF is its parent)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s235_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
