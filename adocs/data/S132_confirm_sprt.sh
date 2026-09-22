#!/usr/bin/env bash
#
# S132's SECOND-CONTROL CONFIRMATION, DEC-229. The node-fraction time manager
# read H1 at the playing control on 2026-09-21; this run asks the same
# question of the same two binaries at a control four times longer, because
# the time-management family is the one this project has a record of not
# transferring between controls. It is a dedicated night by the owner's
# decision of 2026-09-22 -- "better to spend a night so we are sure the
# results are correct and the change makes us stronger" -- and not a fold into
# the block boundary's estimate, which is what the coordinator had
# recommended.
#
#   nohup adocs/data/S132_confirm_sprt.sh > .tuning/sprt_s132_confirm.log 2>&1 &
#
# WHAT IS BEING CONFIRMED, quoted as what it is and not as an effect size
# (DEC-063): the stopping run's point estimate is upward-biased by
# construction, and what the first verdict established is "at least 5 nElo",
# not 21.
#
#   SPRT | cand 474c288 vs ref 778c7b0, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
#   Elo | 16.28 +/- 8.47, nElo 21.21 +/- 11.01
#   LLR | 2.95 (-2.94, 2.94) -> H1
#   Games | N: 3822 W: 1239 L: 1060 D: 1523, Ptnml [149, 406, 664, 501, 191]
#   Wall | 1 h 48 m, 2115.5 games/h, forfeits 0
#   Log | adocs/data/S132_sprt.log
#
# WHY THIS PAIR. **The same two shas as the first verdict**, which is the only
# pair that isolates S132's own change: candidate `474c288` is the landing,
# reference `778c7b0` is the commit it sits on, and everything between them is
# the buckets, the factor, the gate and the floor on the product. Measuring
# today's tree with the multiplier switched off on one side would need either
# a revert commit or a tune-build pair, and it would price S132 together with
# everything that has landed since. DEC-229's Rejected paragraph records that
# reasoning; DEC-020 is the contamination that made attribution a rule.
#
# WHY THIS CONTROL. `32+0.32` with `Hash=64` is DEC-202's longer control, and
# it is chosen over the 40+0.4-class run the step file named for one reason
# this project can act on: **it has a measured throughput here and 40+0.4 does
# not** -- 534 games an hour, 2000 games in 3 h 44 m 52 s on 2026-09-13
# (`.moltke.local.md`; S151's own first reading was 545). A control with no
# throughput figure cannot be budgeted, and DEC-155 asks for the estimate
# before the run and not after it.
#
# WHY A CAP. At 534 games an hour an uncapped `{0, 5}` walk is two days if the
# truth sits on a bound (below), and the machine is the binding constraint on
# the plan. The cap is one night.
#
# THE CONSTANTS ARE UNCHANGED AND CHESSO'S OWN. Nothing is re-seeded for this
# run: `TmNodeScalePct` 151 and `TmNodeBasePct` 120 are the census's answer
# over chesso's own tree (DEC-105 (b), 300 stratified positions at `go depth
# 12`, median share 0.5350, `adocs/data/S132_node_share_census.tsv`) and
# `TmNodeMinDepth` is `AspirationMinDepth` as this tree compiles it. The
# technique was implemented from the step file's own description (DEC-221).
# **What the published record is worth here is direction and nothing else**
# (DEC-019) -- and the direction it gave is exactly what this run tests: every
# node-TM patch the step read was verified at two to four controls, and one of
# them measured +20.9 at a cyclic control against +9.9 at its own.
#
# THERE IS NOTHING TO MEASURE ON THE TREE BEFORE THE GAMES. Both binaries are
# the ones the first verdict played: `chesso bench` is `5066204` on both -- the
# rule decides when iterations stop and moves no node at any fixed depth -- and
# `tools/search_bench.py` was identical on all three positions at depths 9 and
# 12 when the landing was measured (INV-6, `adocs/plan_done/`'s S132 file). A
# second bench here would restate a number, not check one.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized` -- the same pair the first verdict was taken at, because a
# confirmation that moved the bounds would not be confirming the same claim.
#
# WHAT THE PAIR COSTS AT THIS CONTROL (DEC-143), from the nElo run-length
# formula in `adocs/testing_strategy.md` section 1.1 -- `D / (e1 - e0)^2` with
# `D = C^2 (ln 19)^2` at the interval's midpoint, `639770 / (e1 - e0)^2` on a
# bound, and the Brownian drift form for a truth outside it. Reproduced here
# against that section's own two published figures (41861 and 25591) before
# any other row was computed:
#
#   truth      expected games   at 534 games an hour
#   0 nElo           25591            47.9 h
#   2.5 nElo         41861            78.4 h   (the midpoint, the worst case)
#   5 nElo           25591            47.9 h
#   10 nElo           9475            17.7 h
#   20 nElo           4062             7.6 h
#
# **So the cap decides what this run can answer, and that is stated before it
# starts.** 6400 games is 12.0 h at 534 an hour, and a true effect of about
# **13.6 nElo or more is expected to reach H1 inside it**; anything weaker
# reaches the cap instead, which is why two of DEC-229's three readings are
# about what the interval says at the cap rather than about a verdict. The
# first verdict's point estimate, 21.21 nElo, would finish in about 3800 games
# and 7.1 h if it were the truth -- it is upward-biased and is not the truth,
# and that gap is the whole reason the cap is set where it is rather than at
# the 4062 games a truth of 20 would need.
#
# **THE CAP IS NOT `ROUNDS`, AND THAT IS A FINDING ABOUT THE HARNESS.**
# `ROUNDS=<n>` in `fastchess.sh` replaces the mode's round limit **and empties
# `sprt_args`**, so a run launched that way has no SPRT at all -- the script's
# own comment says a fixed-rounds run "is NEVER a verdict" and prints
# `bounds none` in its banner. DEC-229 asks for an SPRT that is capped, which
# is not a shape the harness can express today. Two ways out, and this file
# takes the second:
#
#   (a) teach `fastchess.sh` to keep the SPRT under a caller-set round limit.
#       That is a harness change, and DEC-143 then owes a fixed-rounds A/A of
#       1000 games before the next verdict -- 28 minutes at the playing
#       control, but it also re-opens the question of whether any verdict
#       taken before it is comparable. Not a price this run should pay.
#   (b) **run the ordinary gainer SPRT and stop it at the cap.** The mode's
#       own limit stands (20000 rounds, which the run will never reach), the
#       bounds are printed and live, and the stop is operational: the
#       coordinator's watcher polls the log and terminates the run once the
#       game count reaches **6400**, with a wall-clock ceiling of 14 h as the
#       belt (DEC-061 wants a ceiling anyway, and 14 h is a little over 2x the
#       7.6 h a 20 nElo truth needs).
#
# **What a stop looks like, so nobody reads it as a crash.** `fastchess.sh`'s
# EXIT trap prints `SPRT-RUN-FAILED: exited 143` when the run is terminated,
# because `completed` is still 0 -- that is the marker for the stop and not a
# broken run. The reading then comes from the last periodic SPRT block in the
# log (fastchess prints one every 20 games) and from the PGN in `$OUT`, which
# is written as the games finish. S068's run 1 is the precedent: a deliberate
# `Terminated`, `status=143`, recorded as no verdict with its interval.
#
# REGIME. `TC=32+0.32`, `HASH=64`, one thread a side, `books/noob_3moves.epd`
# (DEC-189), concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run and not set (DEC-195). The book and the adjudication are
# the playing control's, deliberately: the only thing this run changes against
# the first verdict is the clock. **No harness change since the last
# fixed-rounds A/A** -- same fastchess alpha 1.8.1 20260720-daa3ea2, same book,
# same adjudication, same machine -- so DEC-143's A/A clause owes nothing
# before this run, and (a) above is refused partly to keep that true.
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
# The denominator is each engine's own games and not the run's (the S088
# reading the tool implements). **What a forfeit would mean here, which is not
# what it means at the playing control.** This is a clock change measured at a
# control four times longer: the same 50 ms `MOVE_OVERHEAD_MS` covers moves
# that are four times longer on average, so the margin is proportionally
# wider and the expected rate is nearer zero than at 8+0.08 -- the first
# verdict ran 3822 games with **0 forfeits** at the shorter control. A rate
# that is not near zero here is therefore a stronger signal than the same rate
# would be at 8+0.08, and the shape to look for is a **one-sided** one: both
# engines are chesso, so a thin margin costs them equally, but only the
# candidate carries a rule that can spend more of the allocation on a long
# think. A one-sided rate is the multiplier reaching into the overhead and is
# a reason to stop and look at `TmScaleMinPercent` and the hard clamp, not a
# verdict about strength. A crash or a disconnect on either side voids the run
# outright and `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212).
# Mains, and a second load on the machine. **Nothing else is an abort** -- in
# particular reaching the cap is not one, it is one of the three readings
# below.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171),
# read off `adocs/plan.md`'s Open list and `adocs/data/S188_sprt.sh` on
# 2026-09-22. **No defect reachable in ordinary play, on the UCI surface, or
# able to move a reported score, move or line is open.** What is open is
# test-side and filler:
#
#   1. **S239** -- `tools/mutation_check.py` accepts a baseline that ran no
#      tests and a fixture dirty outside `src/`, the two gaps S097 verdict 2's
#      agent walked into on 2026-09-21. Filler behind S188.
#   2. **`test_mate_breadth` has 2.6 % of headroom under its ceiling** on
#      S188's tree, 109.2 s Release against the 120 s timeout, which is inside
#      this machine's own noise band. The ceiling is not relaxed (TESTS).
#   3. **`X10_extends_two_plies` is scored `unmeasured`** by that same tool,
#      its four failing tests all Timeouts, and DEC-165 refuses to score a
#      Timeout as a kill; the mutant was observed red by assertion twice.
#   4. The three S097 named and re-checked on 2026-09-21: S231 phase one's
#      node-budget label of 17321 against the reverted tree's 16256;
#      `adocs/data/S192_node_budget.py` reporting OUTSIDE for a band it had
#      just derived; and `tests/test_search.cpp` "a reduced move that beats
#      alpha is searched again" pinning depth 6 where
#      `adocs/data/S231_research_witness.py` answers 4.
#
# Items 2 and 3 belong to a tree **later than this pair** -- both binaries here
# predate S188's landing -- so neither can touch either side of this run; item
# 1 is about a tool no game calls; item 4's three sit identically on both
# sides. None of the four can move a reported score, move or line.
#
# PRE-REGISTERED INTERPRETATION. These are DEC-229's three readings, written
# by the owner before a game is played and reproduced here without amendment:
#
#   H1 accepted  -> **confirmed at the second control.** The constants are
#                   called shipped, and S127 may fit them -- with S085's
#                   caveat still written into that lane, because a fit at a
#                   control the verification does not share is exactly what
#                   S085 measured at +23.8 and -22.9. What may be written
#                   about this run is "at least 5 nElo at 32+0.32", not its
#                   point estimate (DEC-063).
#   H0 accepted, or the cap reached with the nElo interval's top **below
#   zero**
#                -> **a regression at the longer control.** `TmNodeScalePct`
#                   flips to 0, which is the step's own pre-registered revert
#                   and a proved off value rather than a range end (DEC-215),
#                   the zero is recorded as a zero, and the counting half
#                   stays -- it is behaviour-neutral and an H0 here does not
#                   price it.
#   H0 accepted, or the cap reached with the interval **reaching above zero**
#                -> **kept on the playing control's verdict**, with the
#                   second-control interval recorded beside it, and S127 fits
#                   the three at the playing control only.
#
# The ledger takes this run as **S132 v2** with DEC-220's block, whichever way
# it goes, and `status.md`'s Parked question closes on it. The step file in
# `adocs/plan_done/` is history and is not edited (AGENTS.md): this run's
# record is DEC-229, this file, and its log.
#
# AGENTS.md WATCHERS and DEC-061: `fastchess.sh` prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker, and
# the stop at the cap is the FAILED one with status 143 as described above.
# COMMITS as amended by DEC-220: the commit that closes this verdict carries
# fastchess's own result block before its `Bench:` line, and `tools/gate.sh`
# refuses a block whose shas do not match the named log's result line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR, pinned 2026-09-22 and identical to the first verdict's.
#
# REF `778c7b0` is the commit S132's landing sits on -- no buckets, no node
# factor, no `TmNode*` settings. CAND `474c288` is the landing itself,
# "Scale the soft time limit by the best move's share of the root", whose
# `Bench:` line is 5066204 and whose tree the first verdict measured.
#
#   git -C /home/max/ws/chesso log --oneline -1 474c288
#   git -C /home/max/ws/chesso rev-parse --short 474c288^   # check it is REF
#
# The `PIN_ME` refusal below stays for anyone who unsets them: a candidate
# guessed from `HEAD` is a candidate nobody checked, and this run's whole
# claim is that it measures the same pair as the first.
REF="${REF:-778c7b0}"
CAND="${CAND:-474c288}"

for pair in "REF=$REF" "CAND=$CAND"; do
  if [[ "${pair#*=}" == "PIN_ME" ]]; then
    echo "SPRT-RUN-FAILED: ${pair%%=*} is unpinned -- pin it in" \
         "adocs/data/S132_confirm_sprt.sh before running (CAND is S132's" \
         "landing commit, REF the commit it sits on)" >&2
    exit 1
  fi
done

[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="$REF" \
     CAND="$CAND" \
     TC="32+0.32" \
     HASH="64" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s132_confirm_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
