#!/usr/bin/env bash
#
# S098 **verdict 2**: late move reduction adjusted by the node's type. One
# gainer SPRT for four terms, against the tree verdict 1's removal left.
#
#   nohup adocs/data/S098_v2_sprt.sh > .tuning/sprt_s098_v2.log 2>&1 &
#
# WHAT IS BEING MEASURED. `negamax_at` now carries `cut_node` beside `is_pv`, so
# the pair names CPW's three node types instead of two, and the reduction a late
# quiet is searched with becomes
#
#   r = lmr_reduction(depth, move_number)
#       + LmrCutNode        at a node predicted to fail high
#       + LmrNotImproving   where improving_at() is false
#       + LmrTtCapture      where the entry's own move is a capture
#       - LmrPv             at a principal variation node
#
# clamped to `[0, child_depth - 1]` exactly where the raw table was clamped
# before. **S109's shallow-depth gate reads the same adjusted number**
# (`lmr_depth_of`), so the gate and the reduction are one value; the PV term
# never reaches the gate, because a PV node is not a pruning node. Nothing of
# verdict 1 returns and nothing of verdict 3 is here: the re-search still repeats
# at `child_depth` and no deeper.
#
# ONE SPRT FOR FOUR TERMS, DEC-063 and DEC-214. Each is a +/-1-class effect, and
# four pairs at 41861 worst-case games each is a week of machine for effects the
# pair cannot separate anyway. They sit behind four constants whose off value is
# 0 and inside the declared range, so a failing verdict bisects by **release
# rebuild** -- not by the tune build, which is different code (S073).
#
# THE SEEDS, AND THE CENSUS BESIDE EACH. Every default is DEC-105 **(c)**, the
# midpoint of a declared 0 to 2, and no engine's ply count seeds any of them
# wherever it is republished (DEC-084 as amended by DEC-105). What the published
# record supplies is direction only (DEC-019) and it is in the step file, not
# here. What the census supplies is whether the condition ever fires:
# `adocs/data/S098_v2_node_census.txt`, 5478549 reduction sites at depth 12 and
# 2103446 at depth 10 over S024's 400 positions, taken on the off tree.
#
#   constant           default  form    fires at depth 12 / depth 10
#   LmrCutNode         1        (c)     21.12 % / 17.74 %
#   LmrNotImproving    1        (c)     52.75 % / 52.73 %
#   LmrTtCapture       1        (c)     24.83 % / 25.37 %
#   LmrPv              1        (c)     26.72 % / 36.19 %
#
# DEC-214's inert threshold is one per cent and the lowest share is twenty times
# it, so **no term ships at 0** and all four are measured. The four sum per site:
# at depth 12 the reduction moves by nothing on 27.85 % of sites, by +1 on
# 41.95 %, +2 on 18.15 %, +3 on 1.45 % and **-1 on 10.60 %**, which is the PV
# term's own share. This is a rule that touches about seven reduction sites in
# ten, which is the opposite of verdict 1's first seed and is why the census
# comes before the match (DEC-212's lesson, DEC-214's clause 2).
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019).
#
#   chesso bench (depth 14)   5685915 -> 5469072, -3.81 %
#   tools/search_bench.py, depth 9, reference -> candidate
#     midgame      51189 ->  22078   best move c3d5 -> **g5f6**
#     kiwipete    146616 -> 104682   e2a6
#     tactical     39389 ->  29842   d7c8q
#   tools/search_bench.py, depth 12, reference -> candidate
#     midgame     143205 -> 205096   kiwipete  570238 -> 646466
#     tactical    148060 -> 149330   all three best moves the reference's,
#                                    c3d5 / e2a6 / d7c8q
#
# The by-depth ablation on the tune build, the shipped seeds against all four at
# 0 -- node counts, never strength (S073):
#
#   depth   9         10        11        12        13        14
#   on      475717    841594    1344741   2453847   3447081   5469072
#   off     607842    935536    1634008   2364815   3849812   5685915
#   delta   -21.74 %  -10.04 %  -17.70 %  +3.77 %   -10.46 %  -3.81 %
#
# **The off column is the reference's totals exactly at every depth**, which is
# the property the bisection rests on and is measured rather than argued: with
# the four constants at 0 the helper returns the raw table, S109's gate is the
# one it had, and the engine is the one before this landing, bench signature
# included. The same equality is asserted a third way by the census script,
# whose instrumented Release binary is built at the off values and must print
# 5685915 before a single position is driven.
#
# The shape is worth stating because it is the opposite of verdict 1's, and
# because it is not uniform. Three of the four terms lengthen the reduction and
# one shortens it, so the tree is **mostly smaller** -- -21.7 % at depth 9,
# -10.0 % at 10, -17.7 % at 11, -10.5 % at 13, -3.8 % at 14 -- with **depth 12
# the exception at +3.8 %**. A signed rule does not cost the same in both
# directions: an unreduced late quiet at a PV node opens a subtree that an extra
# ply elsewhere does not pay for, and which of the two wins at a given depth is
# a property of that depth's tree and not of the rule. `search_bench.py`'s three
# positions say the same from the other side -- every one smaller at depth 9,
# two of three larger at depth 12. The one reading worth having from all of it
# is that the rule is **live at every depth a game reaches**, which is what the
# census predicted and what verdict 1's first seed was not. Whether a mostly
# smaller tree is a better one is what the games decide and not this number.
#
# One best move moves: midgame at depth 9, `c3d5` -> `g5f6`. At depth 12 all
# three are the reference's again. Recorded rather than explained -- a move at a
# fixed depth is not a chess judgement this file is allowed to make (CHESS).
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. The gainer pair, the step's own section 6, and DEC-063's
# straddle rule is satisfied from above: the published expectation for this
# family is +3 to +10 (cutnode +9.34, !improving +4.64, TT-capture +1.87 to
# +3.33, PV +3.78 -- records and not seeds, DEC-019), which sits above the
# interval rather than inside it. Those records are all at or above the 3119 to
# 3138 band and this engine is below it, so a zero here is an expected outcome
# and not a surprise (DEC-176).
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1). The
# two most recent runs on this machine measured **2124 and 2119 games an hour**
# (`adocs/data/S098_v1_sprt.log`, `adocs/data/S098_v1_leg2_sprt.log`), so that
# is **19.7 h** and **12.0 h**. Budget the wall and not the mean (DEC-155):
# **this is a night run.**
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189). Concurrency 12 (DEC-050), whatever governor the machine is under,
# recorded by the run (DEC-195). fastchess alpha 1.8.1 20260720-daa3ea2,
# unchanged since S212's A/A, so no new DEC-143 calibration is owed.
#
# DEC-202. This verdict moves a **reduction adjustment**, so it is inside the
# class the longer-control reading binds; that reading is one fixed 1000-pair
# match at the block boundary and not per verdict, so it is owed there and not
# here. Verdict 1 already left one data point for it: a scale fitted at 2+0.02
# did not transfer to 8+0.08 (DEC-213 clause 4). Nothing here was fitted at any
# control -- four midpoints and a census -- so this run adds no second point.
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
# **None is open.** `adocs/plan.md`'s Open list has S098 at entry 1 and every
# entry behind it is a strength step; S231 is the two-ply continuation table
# S222's H1 opened and is a strength step of its own at Open entry 2, not a
# filler. `2026-08-22_adversarial-F03` is closed and has been since S162
# (`ea9ba2c`); F06 is answered by DEC-121 as format-defining specification.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> the four terms together gain at least 5 nElo over the tree
#                   verdict 1's removal left. **All four stay**, at 1 ply each,
#                   and **verdict 3 (the re-search rule) opens against this
#                   tree**. S127 refits LmrBase, LmrDivisor and these four
#                   together with S109's thresholds after the block -- the
#                   thresholds especially, since the gate now reads a number
#                   that moves with the node type. The stopping run's Elo is
#                   upward-biased and is not the effect size (DEC-063); what may
#                   be written is "at least 5 nElo".
#   H0 accepted  -> the bisection the step's section 6 pre-registers, **two legs
#                   at most, each one release rebuild and never the tune build**
#                   (S073), weakest records first:
#                     leg 1  `LmrTtCapture` and `LmrPv` to 0, keeping
#                            `LmrCutNode` and `LmrNotImproving` at 1. Those two
#                            carry the thinnest records (+1.87 and +3.78) and
#                            the PV term is the only one that fights the other
#                            three, so a pair that cancels is the first thing to
#                            rule out.
#                     leg 2  the other half -- `LmrCutNode` and
#                            `LmrNotImproving` to 0, the first pair restored --
#                            only if leg 1 also reads H0.
#                   **Before either leg is blamed on a term, the `cut_node`
#                   alternation is re-checked**: a wrong prediction is noise
#                   with no symptom, and the two cases that hold it are "the
#                   node type of every child is the one the published rules
#                   predict" and "the node labels its first child by the
#                   first-child rule and the rest by the scout rule", with
#                   `tools/mutants/S098_node_type.py` T01, T07 and T09 as the
#                   mutants they kill. If both legs read H0 the verdict is
#                   **recorded as a zero** and the four terms leave the tree in
#                   DEC-194's shape, down to what stays: the `cut_node`
#                   parameter and the prediction functions are what verdict 3
#                   does not need, so they leave with the terms unless verdict 3
#                   is re-scoped to want them, which is a decision and not this
#                   file's.
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
# **Pinned 2026-09-16: CAND is `a771260`, "Land S098 verdict 2: late move
# reduction adjusted by node type"; REF stays `50fd965`.**
# both are written as defaults rather than left blank so the file is runnable
# and what it will measure is on the record before it is.
#
# REF is `50fd965`, the commit before verdict 2's landing -- the tree DEC-213's
# removal left, whose `src/` is `30a3be2`'s and whose bench is 5685915. That is
# the tree the step's section 6 and DEC-213 clause 3 both name: verdict 2 is
# measured against the tree verdict 1 left and not against verdict 1's own
# candidate.
#
# CAND is `HEAD`, which is verdict 2's landing commit once the coordinator has
# made it; the coordinator re-pins REF if HEAD moves before the run.
# `fastchess.sh`'s banner prints both shas with their commit dates before the
# first game, so what the run measures is on screen and never assumed.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-50fd965}" \
     CAND="${CAND:-a771260}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s098_v2_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
