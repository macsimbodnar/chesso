#!/usr/bin/env bash
#
# S098 **verdict 3**: the depth the reduced fail-high re-search runs at. One
# gainer SPRT for the two paths, against the tree verdict 2's H1 left.
#
#   nohup adocs/data/S098_v3_sprt.sh > .tuning/sprt_s098_v3.log 2>&1 &
#
# WHAT IS BEING MEASURED. A late quiet that was reduced and came back above
# alpha is owed a zero-window repeat, and until this verdict that repeat always
# ran at `child_depth`. It now answers the reduced search
# (`lmr_research_depth` in src/search.cpp):
#
#   child_depth - 1   the score beat alpha by less than LmrShallowerMargin and
#                     the reduction was at least 2
#   child_depth + 1   the score cleared the node's own fail-soft best by
#                     LmrDeeperMargin and the reduction was at least
#                     LmrDeeperMinReduction
#   child_depth       otherwise
#
# `best_so_far` before the move and not alpha is the deeper margin's base. The
# shallower path wins where both conditions hold, which is every scout node
# whose fail-soft best sits more than a margin below alpha. The **full-window**
# re-search below it is untouched and still runs at `child_depth`, and so is the
# alpha-raising path: this verdict moves one depth argument and nothing else.
# Nothing of verdict 1 returns, verdict 2's four terms are untouched, and no
# history is updated from the re-search's outcome (routed onward, the step's
# Scope concerns).
#
# THE STRICT INEQUALITY. The re-search depth is always greater than the reduced
# depth `child_depth - reduction`, or the re-search would be the reduced search
# run a second time -- a move believed on the very search that was not believed.
# The shallower path's `reduction >= 2` is what keeps it and it is arithmetic,
# not a setting. Asserted in the Debug build and held by
# `tests/test_search.cpp` "the re-search depth stays inside its cap, floor and
# inequality" over the rule's whole declared domain.
#
# THE SEEDS, AND THE CENSUS BESIDE EACH. Both margins are DEC-105 **(c)**, the
# midpoint of a range stated by purpose -- 0 to PAWN, whose top is the point
# past which the depth would be decided on more material than a pawn of window
# -- and no engine's coefficient seeds either, wherever it is republished
# (DEC-084 as amended by DEC-105). `LmrDeeperMinReduction` is DEC-105 **(b)**, a
# derivation over this engine's own site: the re-search exists only where the
# reduction is at least 1, so 1 is a guard that says nothing and 2 is the
# smallest value at which it does. What the published record supplies is
# direction only (DEC-019) and it is in the step file, not here.
#
# What the census supplies is whether each path fires:
# `adocs/data/S098_v3_research_census.txt`, 106610 re-search sites at depth 12
# and 44871 at depth 10 over S024's 400 positions, taken on the off tree.
#
#   constant                 default  form   the PATH fires at d12 / d10
#   LmrDeeperMargin          47       (c)     4.60 % /  3.97 %
#   LmrShallowerMargin       47       (c)    47.77 % / 44.36 %
#   LmrDeeperMinReduction    2        (b)    r >= 2 on 52.41 % / 48.38 %
#
# **A path's share and its condition's share are different numbers and this run
# is priced on the first.** The shallower path is tested first, so a site where
# both conditions hold belongs to it: at depth 12 the deeper *condition* holds
# on 6.89 % of sites, the shallower on 47.77 %, and they overlap on 2.29 %, so
# the deeper *path* fires on 4.60 % and 47.64 % of sites are left unchanged. The
# first version of this file published 6.89 % as the path share; the Tier-1 fast
# check caught it and the census was re-run with joint counters.
#
# DEC-214's inert threshold is one per cent and the smaller path share is over
# four times it, so **neither path ships at its off value** and both are
# measured. The seed rule was written before the census ran: the (c) seed stands
# for a path firing at one per cent or more, a path under it is re-seeded by
# derivation (b) at the quantile that makes it fire at about a tenth of sites,
# and a path that cannot reach one per cent inside 0 to PAWN ships inert. Both
# paths are in the first case and neither was re-seeded.
#
# The census is not the only evidence that neither is inert. A release rebuild
# at each off value says the same from the tree: deeper-only
# (`LmrShallowerMargin` 0) benches **4646334** against the off tree's 5469072,
# and shallower-only (`LmrDeeperMinReduction` 126) benches **4794294** against
# the shipped 4025871. Both paths move the tree on their own, which is what the
# H0 bisection below needs of them.
#
# **ONE THING THE CENSUS CORRECTED BEFORE A GAME WAS PLAYED.** The step's
# section 4 expected `LmrDeeperMargin` at its range top to be the deeper path's
# off value. It is not: the fail-soft best sits below alpha at every scout node,
# so `score > best + 94` is still true on **2.86 %** of sites at depth 12. The
# deeper path's off value is `LmrDeeperMinReduction` at its own range top, 126,
# which is above every reduction the clamp to `[0, child_depth - 1]` admits --
# the census saw no reduction above 6. That is the value the H0 bisection's
# second leg uses.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019).
#
#   chesso bench (depth 14)   5469072 -> 4025871, -26.39 %
#   tools/search_bench.py, depth 9, reference -> candidate
#     midgame      22078 ->  51048   best move **g5f6 -> c3d5**
#     kiwipete    104682 -> 104849   e2a6
#     tactical     29842 ->  29842   d7c8q
#   tools/search_bench.py, depth 12, reference -> candidate
#     midgame     205096 -> 115959   kiwipete  646466 -> 471175
#     tactical    149330 -> 134331   all three best moves the reference's,
#                                    c3d5 / e2a6 / d7c8q
#
# The by-depth ablation on the tune build, the shipped seeds against the two off
# values -- node counts, never strength (S073):
#
#   depth   9         10        11        12        13        14
#   on      535688    836148    1399862   2002440   2746511   4025871
#   off     475717    841594    1344741   2453847   3447081   5469072
#   delta   +12.61 %  -0.65 %   +4.10 %   -18.40 %  -20.32 %  -26.39 %
#
# **The off column is the reference's totals exactly at every depth**, which is
# the property the bisection rests on and is measured rather than argued: at
# `LmrShallowerMargin` 0 and `LmrDeeperMinReduction` 126 the re-search runs at
# `child_depth` everywhere and the engine is the one before this landing, bench
# signature included. The same equality is asserted a second way by the census
# script, whose instrumented Release binary is built at those two values and
# refuses to take a census unless it prints 5469072.
#
# **The ablation is not monotone and the crossover is the reading.** The tree is
# *larger* at depths 9 and 11, flat at 10, and 18 to 26 per cent smaller at 12,
# 13 and 14 -- so the rule costs at shallow depths and pays at the depths a game
# at this control actually reaches. Both directions are the same two paths: the
# shallower path fires on nearly half of all re-search sites and saves a ply of
# verification at each, while the deeper path spends one on 7 % and every ply it
# spends opens a subtree that grows with depth. Which wins at a given depth is a
# property of that depth's tree and not of the rule, exactly as verdict 2's
# non-monotone row was. `search_bench` says the same from the other side and
# disagrees with the bench about the sign at depth 9: midgame **grows** 22078 ->
# 51048 there with its best move moving `g5f6` -> `c3d5`, the value the tree had
# before verdict 2, while kiwipete and tactical do not move at all; at depth 12
# all three shrink. A quarter of the tree at the depth `bench` reads is a large
# trade to put under a {0, 5} pair, and whether it is a better engine is what
# the games decide and not this number. One best move moves and it is recorded
# rather than explained -- a move at a fixed depth is not a chess judgement this
# file is allowed to make (CHESS).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. The gainer pair and the step's own section 6. **The
# expectation is 0 to +3 and it is direction only** (DEC-019): the one record
# below 3100 for this device is a single guarded pass at the boundary
# (+3.11 +/-2.35 in the 3138-3224 band, with the bare form at -7.07 +/-7.65 in
# the same place), one unmerged pass above it, and one wholesale revert on the
# negative side. **A zero is a named outcome here and not a surprise** --
# DEC-176 says a verdict resting on 3100-band evidence may not be argued for on
# a band this engine is below, and the step's section 6 says a random-walked V3
# is terminated and recorded as zero. DEC-063's straddle rule is satisfied from
# above in the same way verdict 2's was: the pair does not bracket a +3 truth,
# which is why no verdict is a live outcome and is written out below.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1). The
# most recent run on this machine measured **2126 games an hour**
# (`adocs/data/S098_v2_sprt.log`), so that is **19.7 h** and **12.0 h**. Budget
# the wall and not the mean (DEC-155): **this is a night run.**
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189). Concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run (DEC-195). fastchess alpha 1.8.1 20260720-daa3ea2,
# unchanged since S212's A/A, so no new DEC-143 calibration is owed.
#
# DEC-202. This verdict moves a **reduction adjustment**, so it is inside the
# class the longer-control reading binds; that reading is one fixed 1000-pair
# match at the block boundary and not per verdict, so it is owed there and not
# here. Nothing in this verdict was fitted at any control -- two midpoints, one
# derivation and a census -- so it adds no point to that reading either way.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default is
# under `/tmp`, which this machine wipes at boot. `.tuning/` is gitignored and
# survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright and
# `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212). No timed match
# starts on battery (POWER).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **None is open**, re-read on this tree rather than copied from verdict 2's:
# `adocs/plan.md`'s Open list has S098 at entry 1 and every entry behind it is a
# strength step, with S231 -- the two-ply continuation table S222's H1 opened --
# a strength step of its own at entry 2 and not a filler.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the re-search rule gains at least 5 nElo over the tree
#                   verdict 2's H1 left. **Both paths stay**, at the seeds
#                   above, and **S098 completes**: the three verdicts are then
#                   spent and the step takes its `done:` stamp. S127 refits
#                   `LmrDeeperMargin`, `LmrShallowerMargin` and
#                   `LmrDeeperMinReduction` beside `LmrBase`, `LmrDivisor`, the
#                   four node-type terms and S109's thresholds after the block.
#                   The stopping run's Elo is upward-biased and is not the
#                   effect size (DEC-063); what may be written is "at least
#                   5 nElo".
#   H0 accepted  -> the bisection the step's section 6 pre-registers, **two legs
#                   at most, each one release rebuild and never the tune build**
#                   (S073), one path at a time:
#                     leg 1  `LmrShallowerMargin` to **0**, keeping the deeper
#                            path. The shallower path first because the record's
#                            argument is about the guard: the guarded form is
#                            what measured positive, the shallower path is the
#                            unguarded half of the device by volume -- 48 % of
#                            sites against 7 % -- and it is what the -26 % tree
#                            is mostly made of.
#                     leg 2  `LmrDeeperMinReduction` to **126**, its range top,
#                            with `LmrShallowerMargin` restored -- the deeper
#                            path off and the shallower one on. Only if leg 1
#                            also reads H0.
#                   **Before either leg is blamed on a path, the firing census
#                   is re-read**: a path that stopped firing is not a path that
#                   was measured, which is what verdict 1 cost the plan
#                   (DEC-212). If both legs read H0 the verdict is **recorded as
#                   a zero** and the rule leaves the tree in DEC-194's shape --
#                   `lmr_research_depth`, its probe, the three constants, the
#                   probe's four `research_*` fields and
#                   `tools/mutants/S098_research_rule.py` go, and the re-search
#                   returns to `child_depth`, which is a revert to a bench
#                   signature this tree already knows (5469072).
#   No verdict   -> recorded as zero and the same removal. Two legs is the
#                   budget this bisection is given, and a run that cannot
#                   resolve inside it has spent it; re-running until a bound is
#                   hit is how an alpha of 0.05 stops meaning 0.05 (DEC-063).
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR. **REF is pinned here and CAND is the coordinator's to pin**, and
# **Pinned 2026-09-17: CAND is `cb40afd`, "Land S098 verdict 3: the re-search
# depth answers the reduced search"; REF stays `efdbc9b`.**
# both are written as defaults rather than left blank so the file is runnable
# and what it will measure is on the record before it is.
#
# REF is `efdbc9b`, `HEAD` at this landing -- the tree verdict 2's H1 left. Its
# `src/` is `a771260`'s byte for byte (`git diff a771260 efdbc9b -- src/` is
# empty; `efdbc9b` records the verdict and touches no source), and its bench is
# 5469072. That is the tree the step's section 6 names: each verdict is measured
# against the commit before it.
#
# CAND is `HEAD`, which is verdict 3's landing commit once the coordinator has
# made it; the coordinator re-pins REF if HEAD moves before the run.
# `fastchess.sh`'s banner prints both shas with their commit dates before the
# first game, so what the run measures is on screen and never assumed.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-efdbc9b}" \
     CAND="${CAND:-cb40afd}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s098_v3_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
