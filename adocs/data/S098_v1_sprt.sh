#!/usr/bin/env bash
#
# S098 verdict 1 of 3: the late move reduction is scaled by the move's raw
# history. Candidate is the working tree at the commit the coordinator makes
# for this verdict; nothing of verdicts 2 (node type) and 3 (the re-search
# rule) is in it.
#
#   nohup adocs/data/S098_v1_sprt.sh > .tuning/sprt_s098_v1.log 2>&1 &
#
# WHAT IS BEING MEASURED. One rule and one number per move:
#
#   r -= clamp(hist_sum / LmrHistDiv, +/-LmrHistClamp)
#
# on top of the reduction table, in `lmr_adjusted_reduction` in
# `src/search.cpp`, where `hist_sum` is `quiet_history_sum` -- the **raw**
# butterfly entry plus S222's weighted continuation entry, the same sum
# `score_move` returns for a plain quiet and never a killer's or a
# countermove's band. Positive history shrinks the reduction, negative history
# grows it.
#
# Two sites read that helper and they are one change and not two. The reduction
# itself, and **S109's `lmr_depth` gate**, which is re-pointed to it so the
# four shallow-depth rules price a quiet at the depth it is actually searched
# at. S109 is not re-verdicted: its own file and `plan.md` both say S098's SPRT
# prices the interaction and S127 refits its thresholds. The two cannot be
# separated by a bound pair, and they are not meant to be -- they are separated
# by a release rebuild at the off value, which is the bisection below.
#
# THE TWO CONSTANTS AND THEIR SEEDS (DEC-084 as amended by DEC-105; every seed
# is one of the three forms and says which). No engine's coefficient is behind
# either, and the published records that say history scaling is worth trying --
# Lynx #613 +11.40 +/-7.30 in this engine's own band, Weiss #451 +14.11 +/-7.57
# above it -- are records and never seeds.
#
#   LmrHistClamp  3      **this project's own SPSA fit**, theta 2.7319 rounded
#                        by the driver. Seeded at 2, which was **(b)**, a
#                        stated fraction of chesso's own reduction table:
#                        `search_lmr_reduction_probe(11, 63)` at the census
#                        median depth and the last move index is
#                        `0.52 + ln(11) * ln(63) / 1.82` = 5.98, stored floored
#                        as 5, and half of that rounded down is 2. The fit
#                        widened it past that half to three plies.
#   LmrHistDiv    699    **this project's own SPSA fit**, theta 698.6584
#                        rounded by the driver. Seeded at 430, which was
#                        **(b)**, a derivation over chesso's own **measured**
#                        distribution of that sum, and not over the band's
#                        arithmetic. `adocs/data/S098_v1_hist_census.txt`:
#                        the signed sum recorded at every site the rule reads
#                        it, over the 400 positions of
#                        `adocs/data/S024_census_positions.txt`. At depth 12,
#                        5464717 sites, |sum| reads p50 174, **p75 430**, p90
#                        1442, p99 5362; at depth 10, 2105964 sites, 107 /
#                        258 / 689 / 4347. The seed was the depth-12 p75, so
#                        the term reached one full ply at the quartile; the fit
#                        moved it to about 1.6 times that and comfortably under
#                        the p90, a narrower class of moves.
#
# S127 refits both with the whole set after the block.
#
# **BOTH VALUES ARE THE LANE'S, NOT EITHER SEED'S, AND THAT IS DEC-212.**
# `adocs/data/S098_v1_spsa.sh` ran 2026-09-15, 11:49:38 to 20:37:23 -- 1250
# iterations, 60000 games at `2+0.02` on `books/UHO_4060_v3.epd`, 8 h 47 m 45 s
# against an 8 h 37 m estimate, `SPSA-DONE` its log's only marker. Read by that
# file's own pre-registered rows: **not stuck** (both axes moved off their
# seeds, so this SPRT is owed), **the clamp is not 0** (so the term is not
# inert by the fit's own word and the technique does not leave the plan on that
# reading), and **the divisor is at or below the census p90 of 1442** (so the
# fit agrees with the census seed's side rather than the band seed's).
# `LmrHistDiv` never touched a bound in 1250 iterations, running 422 to 747;
# `LmrHistClamp` sat at a bound on 39 of them, five settings being what they
# are. An SPSA vector is a hypothesis and never a result (DEC-019): **this run
# is what decides it.**
#
# **THE DIVISOR WAS RE-SEEDED BEFORE ANY GAME WAS PLAYED, AND THIS IS WHY.**
# It was first seeded at 8675 -- half the saturated sum, the band's own
# arithmetic -- and that is a statement about what the tables *can* hold. The
# census says what they *do* hold: |sum| reaches 8675 at **0.006 %** of sites
# at depth 10 and **0.011 %** at depth 12. The step's own by-depth ablation
# had already seen it from the other end, +0.00 % of the bench nodes at depths
# 9 to 11 and -0.02 % at 12, and an `8+0.08` game lives at depths 10 to 14. A
# run taken at that seed would have priced the seed and not the technique,
# which is what a pre-registration exists to notice before the games and not
# after. The clamp is unchanged at 2 and is not the scale: at 430 it binds on
# real sites -- 14.66 % of them at depth 12 -- rather than only at the band's
# edge, and the 99th percentile reaches two plies twelve times over.
#
# **THE CENSUS WAS ONE PASS, AND THE LANE IS WHAT CLOSES THAT.** It ran on the
# tree at 8675 -- its own header's bench signature, 5968045, is the proof -- so
# 430 was the 75th percentile of a tree the term then changed by 60 %, and a
# fixed point would have needed the two iterated to agreement. That iteration
# was not run and is not owed: SPSA played 60000 games on the tree each
# candidate value actually produces, so **699 is chosen against play and not
# against a distribution measured somewhere else**. The census stays on the
# record as where the lane started and why it was owed, and the percentiles
# above are read as that and not as a derivation of what ships.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019); they are here so
# that "the term changes the search" is a measurement and not a claim.
#
#   chesso bench (depth 14)   5685915 -> 11046420, +94.28 %
#   tools/search_bench.py, depth 9, parent -> candidate
#     midgame      51189 ->  77969   kiwipete  146616 -> 146770
#     tactical     39389 ->  40190   best moves c3d5 / e2a6 / d7c8q, the
#                                    parent's, at both depths
#   tools/search_bench.py, depth 12, parent -> candidate
#     midgame     143205 -> 231052   kiwipete  570238 -> 618264
#     tactical    148060 -> 221800
#
#   At the census seed, 430, the same three read 27434 / 148084 / 30174 and
#   240137 / 772719 / 185931 with midgame's depth-9 move at g5f6; the fit takes
#   that move back to the parent's c3d5 while growing the tree further.
#
# **The tree grows, and by a lot.** A quarter of the sites the rule reads now
# get a ply or two back, and an un-reduced late quiet opens a subtree where an
# extra ply of reduction on an already-reduced one saves very little, so the
# two directions of a signed term do not cost the same. Measured on the tune
# build by the one ablation that build is allowed -- node counts, never
# strength (S073) -- `bench <depth>` at the shipped clamp against the same
# binary at `LmrHistClamp 0`:
#
#   depth   9         10        11        12        13        14
#   on      706352    1185319   2153709   4036614   6296135   11046420
#   off     607842     935536   1634008   2364815   3849812    5685915
#   delta   +16.21 %  +26.70 %  +31.81 %  +70.69 %  +63.54 %   +94.28 %
#
# At the band seed that same table read +0.00 / +0.00 / +0.00 / -0.02 / -1.33 /
# +4.96 %, and at the census seed +0.60 / +28.04 / +23.61 / +41.47 / +35.75 /
# +60.63 %. The fit narrows the class the term touches -- 699 against 430 --
# and widens what it gives each one, three plies against two, and the second
# dominates. **The off column is the parent's totals exactly, at every depth
# and at all three settings**, which is the
# inert-by-rebuild property asserted over the whole engine and not only over
# the helper: the off rebuild restores S109's gate and the reduction together,
# so the bisection below separates "the term is wrong" from "re-pointing the
# gate moved it".
#
# **What the SPRT is being asked, given that.** A 94 % larger tree at a fixed
# depth is roughly a ply given up in the same time, so this run
# is not a free-side test of a refinement: it asks whether searching the quiets
# the history tables like at close to their full depth is worth what it costs
# in depth everywhere else. That is the question the technique poses, and it is
# why the answer is games. Nodes are not Elo and nothing here reads them as
# Elo (DEC-019).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair**, because the step's own section 6
# pre-registered it and sized it from the record: V1 is expected at +5 to +15,
# from Lynx #613 (+11.40 +/-7.30 at 8+0.08, merged inside the 2420-2653 band
# this engine sits in) and Weiss #451 (+14.11 +/-7.57, above the band). DEC-019
# binds that to direction only; DEC-063 is satisfied because the expectation
# sits above the interval rather than inside it, which is what makes {0, 5}
# terminable here.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1): at
# **2150 games an hour** -- the six runs since S212 read 2133, 2162, 2190,
# 2155, 2137 and S222's 2151, averaging 2155 and rounded down to budget from --
# that is **19.5 h** and **11.9 h**, or 19.8 h and 12.1 h at the 2110
# `.moltke.local.md` says to budget from. An effect outside the interval in
# either direction ends far sooner: S222's own run took 2 h 55 m and S091's
# 38 minutes. Budget the wall and not the mean (DEC-155): this is a run the
# machine is given whole, and at four hours or more it is a night's run.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189). Concurrency 12 (DEC-050), governor `performance` at the time this
# file was written, recorded by the run itself (DEC-195). fastchess alpha 1.8.1
# 20260720-daa3ea2, unchanged since S212's A/A, so no new DEC-143 calibration
# is owed. Everything `fastchess.sh` already does; nothing here overrides it
# but the output directory.
#
# DEC-202. This verdict moves a **reduction coefficient** -- two of them -- so
# it is inside the class the longer-control reading binds. That reading is one
# fixed 1000-pair match at `TC=32+0.32 HASH=64` taken **at the block boundary**
# and not per verdict, so it is owed there and not here, and this run is the
# `8+0.08` verdict that decides whether the change ships.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot: two runs had to be
# relaunched for it. `.tuning/` is gitignored and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright and
# `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212). No timed match
# starts on battery (POWER).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **None is open.** `adocs/plan.md`'s Open list has S098 at entry 1 and every
# entry behind it is a strength step, S194 having closed on 2026-09-15; no
# filler sits behind this block. **S231 is not a filler either** -- it is the
# two-ply continuation table S222's H1 opened, a strength step of its own at
# Open entry 2, behind this one because this one reads the sum those two tables
# make.
#
# This file first named `2026-08-22_adversarial-F03` here -- the 50-move draw
# returned before checkmate is tested -- and that reading came from a stale
# `Status:` line and not from the code. **It is closed and has been since
# S162** (`ea9ba2c`, "Score checkmate ahead of the hundredth halfmove"): the
# `halfmove_clock >= 100` return in `negamax_at` tests `is_check` with no legal
# reply first and falls through to the mate score, and
# `tests/test_engine.cpp` "checkmate outranks the hundredth halfmove" is the
# guard. Re-read in the tree before writing this. The report's Status line was
# moved by the coordinator.
#
# `2026-08-22_adversarial-F06` -- `polyglot_randoms[781]` in
# `src/openings.cpp`, the Polyglot format's own constant table -- is answered
# by DEC-121 as format-defining specification and excluded by name from S211's
# originality sweep. No reach into play.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the history-scaled reduction gains at least 5 nElo over the
#                   tree before it. **Keep the term and both constants where
#                   they are**; the stopping run's Elo is upward-biased and is
#                   not the effect size (DEC-063), so what may be written is
#                   "at least 5 nElo". **Verdict 2 (node type) then opens
#                   against this tree**, not against the one before it, and
#                   S127 refits LmrBase, LmrDivisor, LmrHistDiv, LmrHistClamp
#                   and S109's thresholds together after the block.
#   H0 accepted  -> the term does not gain 5 nElo. Recorded as a zero, and the
#                   step's section 6 bisection runs, **two legs at most, each
#                   one value in `src/search_params.hpp` and a release rebuild,
#                   never the tune build** (S073):
#                     leg 1  the sign. The direction is pinned by a case and a
#                            mutant, so this leg is the cheap confirmation and
#                            not the suspect.
#                     leg 2  the divisor's scale, **upward this time**. What
#                            this run prices is the lane's own 699, which
#                            already walked up from the census's p75 while the
#                            clamp widened; a larger divisor still is the same
#                            rule applied to a narrower class, and the census
#                            gives
#                            the next value to try without another run -- p90
#                            at depth 12 is 1442, which is the tenth of sites
#                            rather than the quarter and which **inherits the
#                            p75's one-pass caveat**: it is a percentile of the
#                            tree at 8675, not of the tree a candidate plays.
#                            The re-seed itself is
#                            the record that the scale is the live axis here:
#                            8675 was inert and 430 is loud, and the truth is
#                            between them. The clamp is **not** a leg -- Lynx
#                            #971 measured clamping the same term to [-1, 1]
#                            at +0.63 unresolved, so the clamp is not where
#                            the Elo is.
#                   `LmrHistClamp 0` and a release rebuild is what separates
#                   the term from the gate re-point, and the depth-14 bench at
#                   that value is the parent's exactly, which is the check that
#                   the separation is clean. Dropping the term is the default
#                   at a measured zero; keeping it needs a stated reason
#                   (S005, S006, S015 are the precedent).
#   No verdict   -> record as zero and decide with the reason stated, not
#                   implied. Verdict 2 is then measured against whichever tree
#                   that decision leaves.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR. **Both shas below are the coordinator's to re-pin**, and they are
# **Pinned 2026-09-15: CAND is `0408447`, "Land S098 verdict 1's fitted scale:
# LmrHistDiv 699, LmrHistClamp 3"; REF stays `1db5b8e`, whose src equals the
# commit before verdict 1's landing.**
# written as defaults rather than left blank so the file is runnable and so
# what it will measure is on the record before it is.
#
# REF is the commit before this verdict's landing -- `1db5b8e`, "Start S098:
# late move reduction scaled by history, verdict 1 of 3", whose `src/` is
# `b06d53e`'s, the tree S222's H1 left. The step's own accepts says "an SPRT
# verdict per adjustment, measured separately" and section 6 says each verdict
# is measured "against the commit before it"; if HEAD moves before the landing
# the coordinator re-pins REF to the new parent.
#
# CAND is `HEAD`, which is the landing commit once the coordinator has made it.
# It is left as `HEAD` and not pinned by this file because this file is written
# before that commit exists; the coordinator pins it to the sha after
# committing, and `fastchess.sh`'s banner prints both shas with their commit
# dates before the first game, so what the run measures is on screen and never
# assumed. REF in the environment overrides for any follow-up leg.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-1db5b8e}" \
     CAND="${CAND:-0408447}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s098_v1_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
