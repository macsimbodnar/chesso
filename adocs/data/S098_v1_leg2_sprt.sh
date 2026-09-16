#!/usr/bin/env bash
#
# S098 verdict 1, **bisection leg 2**: the same rule at a larger divisor.
# One value in `src/search_params.hpp` and a release rebuild, measured against
# the same reference the verdict was.
#
#   nohup adocs/data/S098_v1_leg2_sprt.sh > .tuning/sprt_s098_v1_leg2.log 2>&1 &
#
# WHAT VERDICT 1 READ, AND WHY THIS LEG EXISTS. The history-scaled reduction
# with S109's gate re-pointed to the same helper was fitted in its own two-axis
# SPSA lane (DEC-212) to `LmrHistDiv` 699 and `LmrHistClamp` 3, and that vector
# was measured at the harness regime against `1db5b8e`, the tree before the
# landing: **H0 accepted, LLR -2.96, `Elo -2.92 +/- 5.02`, `nElo -3.70 +/-
# 6.34` over 11524 games**, W 3691 L 3788 D 4045, `Ptnml [578, 1352, 1985,
# 1283, 564]`, 0 time forfeits on either side, 2124 games an hour, 5 h 25 m 46 s
# (`adocs/data/S098_v1_sprt.log`, read in `adocs/data/S098_v1_sprt_pairs.txt`).
# The term at that scale does not gain 5 nElo and is recorded as a zero.
#
# `adocs/data/S098_v1_sprt.sh` pre-registered the H0 bisection before a game of
# that run was played, **two legs at most, each one value and a release
# rebuild, never the tune build** (S073):
#
#   leg 1  the sign. Already pinned by two direct cases -- "a quiet the history
#          tables like is reduced less" and "... written off is reduced more"
#          -- and by mutant `L01_lmr_history_sign`, which those cases kill. It
#          is the cheap confirmation and was never the suspect, so it costs no
#          run.
#   leg 2  the divisor's scale, upward. **This file.**
#
# THE VALUE, AND WHAT IT IS NOT. `LmrHistDiv` 699 -> **1442**, the clamp
# unchanged at 3. 1442 is the 90th percentile of |hist_sum| at depth 12 in
# `adocs/data/S098_v1_hist_census.txt` -- 5464717 sites over S024's 400
# positions -- so the term reaches a full ply for **a tenth** of the moves the
# rule sees where 699 reached it for something under a quarter and the lane's
# seed of 430 reached it for exactly a quarter. The direction is the one the
# verdict's own pre-registration named: the same rule applied to a narrower
# class.
#
# **It is not a fit and this file does not pretend otherwise.** The lane
# already fitted this axis against 60000 games and returned 699; 1442 is a
# pre-registered bisection point taken from the census, and it **inherits the
# census's one-pass caveat** -- the census ran on the tree at divisor 8675, so
# its p90 is a percentile of that tree and not of the tree this value builds.
# What that means for the reading is written into the outcomes below: this run
# can say the term does not gain at either scale, and it cannot say that no
# scale would.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019).
#
#   chesso bench (depth 14)   5685915 -> 9268371, +63.01 %
#   tools/search_bench.py, depth 9, reference -> candidate
#     midgame      51189 ->  60840   kiwipete  146616 -> 146616
#     tactical     39389 ->  42517   best moves c3d5 / e2a6 / d7c8q, the
#                                    reference's, at both depths
#   tools/search_bench.py, depth 12, reference -> candidate
#     midgame     143205 -> 243165   kiwipete  570238 -> 641165
#     tactical    148060 -> 199344
#
#   Beside the two trees this step has already measured: the fitted 699 read
#   11046420 on the bench and 77969 / 146770 / 40190 at depth 9, the lane's
#   seed of 430 read 9133516 and 27434 / 148084 / 30174. A larger divisor
#   touches fewer moves and the tree comes back toward the reference's, which
#   is the whole shape of this leg.
#
# The by-depth ablation on the tune build, shipped clamp against
# `LmrHistClamp 0` -- node counts, never strength (S073):
#
#   depth   9        10        11        12        13        14
#   on      632553   1074018   1867606   3023865   5018624   9268371
#   off     607842    935536   1634008   2364815   3849812   5685915
#   delta   +4.07 %  +14.80 %  +14.30 %  +27.87 %  +30.36 %  +63.01 %
#
# The off column is the reference's totals exactly at every depth, as it has
# been at every setting this step has measured: the term switched off at its
# own constant is the tree before verdict 1, gate and reduction together, which
# is what makes an H0 removal below a revert of a known shape and not a guess.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair, unchanged from the verdict it
# bisects**, which is what makes the two runs comparable: a leg measured at
# different bounds would answer a different question than the one it is a leg
# of. DEC-063's straddle rule is satisfied for the same reason it was there --
# the published expectation for this technique is +5 to +15 and sits above the
# interval rather than inside it.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1): at
# **2124 games an hour**, which is what the verdict's own run measured on this
# machine three hours ago, that is **19.7 h** and **12.0 h**. The verdict it
# bisects took 11524 games and 5 h 26 m to reach a bound, and a leg whose
# effect is smaller rather than larger can take longer; budget the wall and not
# the mean (DEC-155). **This is a night run.**
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189). Concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run (DEC-195). fastchess alpha 1.8.1 20260720-daa3ea2,
# unchanged since S212's A/A, so no new DEC-143 calibration is owed.
#
# DEC-202. This leg moves a **reduction coefficient**, so it is inside the
# class the longer-control reading binds; that reading is one fixed 1000-pair
# match at the block boundary and not per verdict, so it is owed there and not
# here.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot. `.tuning/` is gitignored
# and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright and
# `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212). No timed match
# starts on battery (POWER).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **None is open.** `adocs/plan.md`'s Open list has S098 at entry 1 and every
# entry behind it is a strength step; **S231 is not a filler** -- it is the
# two-ply continuation table S222's H1 opened, a strength step of its own at
# Open entry 2, behind this one because this one reads the sum those two tables
# make. `2026-08-22_adversarial-F03` was named as open by verdict 1's first
# draft and is closed, and has been since S162 (`ea9ba2c`); F06 is answered by
# DEC-121 as format-defining specification.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the term at 1442 and a clamp of 3 gains at least 5 nElo
#                   over the tree before verdict 1. **It stays**, at those two
#                   values, and the verdict's H1 clause applies: **verdict 2
#                   (node type) opens against this tree**, and S127 refits
#                   LmrBase, LmrDivisor, LmrHistDiv and LmrHistClamp together
#                   with S109's thresholds after the block. The stopping run's
#                   Elo is upward-biased and is not the effect size (DEC-063);
#                   what may be written is "at least 5 nElo".
#   H0 accepted  -> **the two legs are spent and the term leaves the tree.**
#                   The sign was pinned by cases and a mutant, the scale was
#                   fitted against 60000 games and measured at -3.70 nElo, and
#                   a second scale a census points at measures no better; a
#                   third value would be a run chosen after seeing two numbers,
#                   which is what a pre-registered bisection exists to stop.
#                   Recorded as a zero, and the removal is DEC-194's shape,
#                   stated here so it is a revert and not a redesign:
#                     - `lmr_adjusted_reduction` goes, and `lmr_depth_of` and
#                       the reduction site in `negamax_at` read
#                       `lmr_reduction` again -- **S109's gate reads the raw
#                       table**, which the off column above shows is the tree
#                       before verdict 1 exactly, bench signature included;
#                     - `LmrHistDiv` and `LmrHistClamp` leave
#                       `src/search_params.hpp`, `tests/test_search_params.cpp`
#                       and `MANUAL.md`;
#                     - the six cases that hold the term, the probes they read
#                       and `tools/mutants/S098_lmr_history.py` go with it;
#                     - `quiet_history_sum` **stays**: it is the one probe path
#                       `score_move` reads and it is behaviour-neutral, proved
#                       by the signature at the time it was factored out.
#                   What does **not** leave is the evidence: the census, the
#                   lane, both pre-registrations and their logs stay as the
#                   record of what was measured and why, and the step file
#                   carries the zero. **Verdict 2 is then measured against the
#                   tree the removal leaves**, not against this one, and the
#                   step's own accepts -- an SPRT verdict per adjustment --
#                   is satisfied for verdict 1 by this zero. S005, S006 and
#                   S015 are the precedent that a measured zero is recorded as
#                   a zero; DEC-194 is the precedent for not keeping one whose
#                   interval sits below.
#   No verdict   -> recorded as zero and the same removal, for the reason the
#                   H0 clause gives: two legs is the budget this bisection was
#                   given, and a run that cannot resolve inside it has spent
#                   it. The reading is not re-opened at other bounds without a
#                   decision -- re-running until a bound is hit is how an alpha
#                   of 0.05 stops meaning 0.05 (DEC-063).
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR. **REF is pinned and CAND is the coordinator's to pin**, and both
# **Pinned 2026-09-16: CAND is `73fbf05`, "Land S098 verdict 1's bisection leg
# 2: LmrHistDiv 1442"; REF stays `1db5b8e`.**
# are written as defaults rather than left blank so the file is runnable and so
# what it will measure is on the record before it is.
#
# REF is `1db5b8e`, the commit before verdict 1's landing -- **the same
# reference the verdict used**, deliberately: a leg measured against the tree
# it is bisecting would price the difference between two of its own candidates
# and say nothing about whether the term is worth having. Its `src/` is
# `b06d53e`'s, the tree S222's H1 left, and `.ref-builds/1db5b8e` is already
# built and clean from the verdict's own run.
#
# CAND is `HEAD`, which is the leg's landing commit once the coordinator has
# made it. `fastchess.sh`'s banner prints both shas with their commit dates
# before the first game, so what the run measures is on screen and never
# assumed.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-1db5b8e}" \
     CAND="${CAND:-73fbf05}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s098_v1_leg2_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
