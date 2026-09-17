#!/usr/bin/env bash
#
# S098 verdict 3, **bisection leg 1**: the deeper path alone. One value in
# `src/search_params.hpp` and a release rebuild, measured against the same
# reference the verdict was.
#
#   nohup adocs/data/S098_v3_leg1_sprt.sh > .tuning/sprt_s098_v3_leg1.log 2>&1 &
#
# WHAT VERDICT 3 READ, AND WHY THIS LEG EXISTS. The re-search depth rule --
# `lmr_research_depth` in src/search.cpp, one ply shallower where the reduced
# score beat alpha by less than `LmrShallowerMargin` with a reduction of at
# least 2, one ply deeper where it cleared the node's own fail-soft best by
# `LmrDeeperMargin` with a reduction of at least `LmrDeeperMinReduction` --
# was measured at the harness regime against `efdbc9b`, the tree verdict 2's H1
# left: **H0 accepted, LLR -2.96, `Elo -9.97 +/- 7.56`, `nElo -13.40 +/-
# 10.16` over 4496 games**, W 1282 L 1411 D 1803, `Ptnml(0-2) [185, 595, 794,
# 512, 162]`, LOS 0.48 %, 0 time forfeits on either side, 2128 games an hour,
# 2 h 06 m 45 s (`adocs/data/S098_v3_sprt.log`, read in
# `adocs/data/S098_v3_sprt_pairs.txt`). **The whole interval sits below zero,
# so the rule as shipped is a measured loss and not a null.**
#
# `adocs/data/S098_v3_sprt.sh` pre-registered the H0 bisection before a game of
# that run was played, **two legs at most, each one value and a release
# rebuild, never the tune build** (S073), one path at a time:
#
#   leg 1  `LmrShallowerMargin` to **0**, keeping the deeper path. **This
#          file.** The shallower path first because the record's argument is
#          about the guard: the guarded form is what measured positive
#          elsewhere, the shallower path is the unguarded half of the device by
#          volume -- 47.77 % of re-search sites against the deeper path's
#          4.60 % -- and it is what the -26.39 % tree is mostly made of.
#   leg 2  `LmrDeeperMinReduction` to **126**, its range top, with
#          `LmrShallowerMargin` restored to 47 -- the deeper path off and the
#          shallower one on. Only if this leg also reads H0.
#
# THE VALUE, AND WHAT IT IS NOT. `LmrShallowerMargin` 47 -> **0**, its own off
# value, proved on the tree and not declared from a range's end (DEC-215):
# `score < alpha + 0` is false where the site already requires `score > alpha`,
# and the release rebuild at that value benches **4646334** where the shipped
# verdict-3 tree benches 4025871 and the reference benches 5469072. The other
# two constants do not move: `LmrDeeperMargin` stays 47 and
# `LmrDeeperMinReduction` stays 2. **No code path is deleted** -- the shallower
# branch stays in `lmr_research_depth` and is simply never taken -- which is
# what makes this one release rebuild and a leg that leg 2 can reverse in one
# more.
#
# **It is not a fit and it is not a new idea.** It is the first of the two
# points the verdict's own pre-registration named before it had a number, and
# it exists to answer one question: whether the deeper path alone gains where
# the pair measured a loss.
#
# **THE FIRING CENSUS WAS RE-READ BEFORE THE LEG**, as the pre-registration
# demands -- a path that stopped firing is not a path that was measured, which
# is what verdict 1 cost the plan (DEC-212). With the shallower path off,
# precedence no longer takes anything from the deeper path, so the deeper path
# fires at exactly its condition's share:
# **6.89 % of re-search sites at depth 12 and 5.85 % at depth 10**
# (`adocs/data/S098_v3_research_census.txt`, 106610 sites at depth 12 and
# 44871 at depth 10 over S024's 400 positions), against the 4.60 % / 3.97 %
# the same census measured for the path after precedence at verdict 3. That is
# six times DEC-214's one per cent threshold, so this leg measures a path that
# fires, and it fires on **more** sites than it did in the tree that lost.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019).
#
#   chesso bench (depth 14)   4025871 -> 4646334, +15.41 % against the shipped
#                             verdict-3 tree, and -15.04 % against the
#                             reference's 5469072
#   tools/search_bench.py, depth 9, reference -> candidate
#     midgame      22078 ->  21995   kiwipete  104682 -> 104682
#     tactical     29842 ->  29842   best moves g5f6 / e2a6 / d7c8q, the
#                                    reference's, at both depths
#   tools/search_bench.py, depth 12, reference -> candidate
#     midgame     205096 -> 155612   kiwipete  646466 -> 683624
#     tactical    149330 -> 152138
#
#   The reference's own numbers are `adocs/data/S098_v3_sprt.sh`'s, taken on
#   the same commit for the same run. Depth 9 barely moves -- two of the three
#   positions are node-identical to the reference and midgame is 0.4 % under
#   it -- and at depth 12 the three disagree in direction: midgame is 24.1 %
#   smaller while kiwipete and tactical are 5.7 % and 1.9 % larger.
#   **Verdict 3's one moved best move comes back**: it moved midgame's depth-9
#   answer `g5f6` -> `c3d5`, and this leg reads `g5f6`, the reference's, with
#   every other best move the reference's at both depths.
#
# The by-depth ablation on the tune build, the shipped seeds against
# `LmrDeeperMinReduction` 126 -- the deeper path's off value, which is now the
# whole rule's -- node counts, never strength (S073):
#
#   depth   9         10        11        12        13        14
#   on      475656    812822    1294648   2406743   3238952   4646334
#   off     475717    841594    1344741   2453847   3447081   5469072
#   delta   -0.01 %   -3.42 %   -3.73 %   -1.92 %   -6.04 %   -15.04 %
#
# **The off column is the reference's totals exactly at every depth**, which is
# the property the bisection rests on and is measured rather than argued: at
# `LmrShallowerMargin` 0 and `LmrDeeperMinReduction` 126 the re-search runs at
# `child_depth` everywhere and the engine is the one before verdict 3's
# landing, bench signature included.
#
# **The sign change is gone with the shallower path, and that attributes it.**
# Verdict 3's ablation read +12.61, -0.65, +4.10, -18.40, -20.32, -26.39 per
# cent over depths 9 to 14 -- the pair cost at shallow depths and paid at deep
# ones. The deeper path alone reads -0.01, -3.42, -3.73, -1.92, -6.04,
# -15.04: **smaller at every depth, flat at 9, and no crossover to read**. The
# only thing that moved between the two rows is the shallower path, so the
# depth-9 growth verdict 3 measured was the shallower path's. What is left is
# the shape the census predicts of a path that fires on 7 % of re-search sites
# and spends one ply at each: the ply buys a table entry a ply deeper, and what
# that saves grows with the depth it is re-used at. Whether a smaller tree is a
# better engine is what the games decide and not this number (DEC-019).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair, unchanged from the verdict it
# bisects**, which is what makes the two runs comparable: a leg measured at
# different bounds would answer a different question than the one it is a leg
# of. DEC-063's straddle rule is satisfied from above the same way verdict 3's
# was -- the published expectation for the guarded form is +3 and sits at or
# above the interval rather than inside it, and DEC-176 says a verdict resting
# on 3100-band evidence may not be argued for on a band this engine is below.
# **A zero is a named outcome here and not a surprise.**
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1). The
# verdict this leg bisects measured **2128 games an hour** on this machine
# (`adocs/data/S098_v3_sprt.log`), so that is **19.7 h** and **12.0 h**. The
# verdict itself reached a bound in 4496 games and 2 h 07 m, and a leg whose
# effect is smaller rather than larger can take much longer; budget the wall
# and not the mean (DEC-155). **This is a night run.**
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189). Concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run (DEC-195). fastchess alpha 1.8.1 20260720-daa3ea2,
# unchanged since S212's A/A, so no new DEC-143 calibration is owed.
#
# DEC-202. This leg moves a **reduction adjustment**, so it is inside the class
# the longer-control reading binds; that reading is one fixed 1000-pair match
# at the block boundary and not per leg, so it is owed there and not here.
# Nothing in this leg was fitted at any control -- one constant moved to its
# own off value -- so it adds no point to that reading either way.
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
# **None is open**, re-read on this tree rather than copied from verdict 3's:
# `adocs/plan.md`'s Open list has S098 at entry 1 and every entry behind it is
# a strength step, with S231 -- the two-ply continuation table S222's H1
# opened -- a strength step of its own at entry 2 and not a filler.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> **the deeper path alone gains at least 5 nElo over the
#                   tree verdict 2's H1 left, and the shallower path was the
#                   loss.** The shallower path then leaves: `LmrShallowerMargin`
#                   ships at 0 and the branch it switches off is removed with
#                   it, together with the `reduction >= 2` guard that exists
#                   only for it, the constant's row in
#                   `tests/test_search_params.cpp` and `MANUAL.md`, the two
#                   arms every case carries for it, and the four mutants this
#                   configuration makes equivalent (`D04`, `D05`, `D07`,
#                   `D10`) -- a removal that has to keep the bench at 4646334,
#                   which is what makes it a revert of a known shape and not a
#                   redesign. **S098 then completes on the deeper path**: the
#                   three verdicts are spent and the step takes its `done:`
#                   stamp, and S127 refits `LmrDeeperMargin` and
#                   `LmrDeeperMinReduction` beside `LmrBase`, `LmrDivisor`,
#                   the four node-type terms and S109's thresholds after the
#                   block. The stopping run's Elo is upward-biased and is not
#                   the effect size (DEC-063); what may be written is "at
#                   least 5 nElo".
#   H0 accepted  -> **leg 2**, and it is the last: `LmrDeeperMinReduction` to
#                   126 with `LmrShallowerMargin` restored to 47 -- the deeper
#                   path off and the shallower one on -- measured against this
#                   same reference. The firing census is re-read before that
#                   leg too, and the shallower path's share at a margin of 47
#                   is the census's 47.77 % / 44.36 % because nothing takes
#                   sites from it. **If leg 2 also reads H0 the verdict is
#                   recorded as a zero and the whole rule leaves the tree in
#                   DEC-194's shape**: `lmr_research_depth`, its probe, the
#                   three constants, the probe's four `research_*` fields and
#                   `tools/mutants/S098_research_rule.py` go, and the
#                   re-search returns to `child_depth`, which is a revert to a
#                   bench signature this tree already knows (5469072). S005,
#                   S006 and S015 are the precedent that a measured zero is
#                   recorded as a zero; DEC-194 is the precedent for not
#                   keeping one whose interval sits below.
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

# THE PAIR. **REF is pinned here and CAND is the coordinator's to pin**, and
# both are written as defaults rather than left blank so the file is runnable
# and what it will measure is on the record before it is.
#
# REF is `efdbc9b`, the commit before verdict 3's landing -- **the same
# reference the verdict used**, deliberately: a leg measured against the tree
# it is bisecting would price the difference between two of its own candidates
# and say nothing about whether the rule is worth having. Its `src/` is
# `a771260`'s byte for byte, its bench is 5469072, and `.ref-builds/efdbc9b` is
# already built and clean from the verdict's own run.
#
# CAND is `HEAD`, which is this leg's landing commit once the coordinator has
# made it; the coordinator re-pins REF if HEAD moves before the run.
# `fastchess.sh`'s banner prints both shas with their commit dates before the
# first game, so what the run measures is on screen and never assumed.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-efdbc9b}" \
     CAND="${CAND:-HEAD}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s098_v3_leg1_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
