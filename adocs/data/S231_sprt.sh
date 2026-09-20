#!/usr/bin/env bash
#
# S231, the two-ply continuation history table on the scale its own SPSA lane
# fitted, judged against the tree before the step's first landing. Candidate is
# the working tree at the commit the coordinator makes for phase three:
# `src/search_params.hpp`'s six continuation defaults become the values the
# lane returned and nothing else in `src/` moves in that commit -- no rule of
# the search changes, no range changes, six integers and the comment blocks
# that explain them do.
#
#   nohup adocs/data/S231_sprt.sh > .tuning/sprt_s231.log 2>&1 &
#
# WHAT IS BEING MEASURED, AND WHY THE REFERENCE IS NOT `HEAD`. `REF` is
# **`3a649c0`**, S098's completing commit and the last tree without the two-ply
# table, pinned in this file because the step file pinned it before phase one
# landed so phase three could not get it wrong. What this run prices is
# therefore the whole step: the second table, the three axes it declared, the
# halved ceiling on `ContHistWeight`, and whatever the lane did to S222's three
# -- one vector under one verdict, which is DEC-210's reading and what the
# step's accepts means by "against the commit before it". Measured against
# `HEAD` instead, the table itself would stay unpriced and only the six
# integers would be on trial.
#
# Between `3a649c0` and the candidate, three commits touch `src/`: `b83fb1d`
# (phase one, the table and its three axes), `af8b9f0` (one arithmetic
# correction inside a comment block, 720873 for 720901) and this phase's
# landing. Nothing else in the range is source.
#
# THE VECTOR, incumbent -> fitted, the driver's own rounded JSON at the end of
# `adocs/data/S231_spsa.log` and no number re-rounded by hand. The unrounded
# theta is `checkpoint.json`'s and is quoted beside it because three of the six
# sit within a sixth of a unit of their rounding boundary: `ContHistBonus` at
# 18.479, `ContHist2Bonus` at 17.564 and `ContHistWeight` at 23.649.
#
#   ContHistBonus     17 -> 18     theta 18.479
#   ContHistMalus     18 -> 17     theta 17.284
#   ContHistWeight    26 -> 24     theta 23.649
#   ContHist2Bonus    17 -> 18     theta 17.564
#   ContHist2Malus    18 -> 20     theta 20.162
#   ContHist2Weight   26 -> 24     theta 23.758
#
# **Not a stuck lane.** All six axes moved off their seeds, so the header's
# stuck-run row does not apply -- and this run would be owed even if it did,
# which is the one place S231's lane differs from S222's: the reference is the
# tree before the step, so the table is priced whether or not the lane moved.
#
# **`ContHist2Weight` ended near 26 (at 24), which is one of the six readings
# the lane pre-registered before a game was played.** The "at or under 5" row
# -- which would have owed a second, attribution SPRT of this vector against
# itself with the weight pinned at 0 -- **is not triggered, and no pinned-zero
# run is taken**; `adocs/data/S231_sprt_pinned.sh` does not exist for that
# reason, the way `S222_sprt_pinned.sh` does not. Equal authority is about
# where the fit wanted the second table, and the seed's derivation is not what
# the step turns on.
#
# **`ContHistWeight` moved 26 -> 24, which is two units and not "a long way"**,
# so the lane's "moves a long way" row is not triggered either. It is named
# anyway, because that row exists to make the SPRT's pre-registration carry
# every value that is no longer the shipping one, and S222's fitted 26 is no
# longer the shipping value. At 24 each the two weighted terms span
# 24 * 32767 / 100 = 7864 apiece against plain history's fitted
# `QuietHistoryMax` of 8831, so the quiet band ships as **8831 / 7864 / 7864**
# over its three terms where it shipped 8831 / 8519 / 8519 at the seeds. The
# two tables still carry very nearly equal authority and plain history still
# carries slightly more than either.
#
# THE LANE, for the record. 2026-09-19 18:41:56 to 2026-09-20 03:22:41,
# **8 h 40 m 45 s** against the 8 h 45 m estimate, 25.00 s an iteration; 1250
# iterations x 24 pairs = 30000 pairs = **60000 games** at 2+0.02, Hash 16,
# concurrency 12, on `books/UHO_4060_v3.epd`; W 20794 L 20774 D 18432;
# `tools/forfeit_report.py` over its own PGN, **0 forfeits on either side of
# 60000**, against the 1.0 % abort line. Evidence:
# `adocs/data/S231_spsa.log`, `S231_spsa_trajectory.tsv`, `S231_spsa_run.json`.
# The three mid-run reads over the whole trajectory: `c_scale` decays
# 2.055 -> 1.150 at a quarter -> 1.072 at a half -> 1.000; `y` mean 0.016,
# standard deviation 5.48, range -17 to +18, 8.1 % of iterations at exactly
# zero; **no axis touched a bound on any iteration**, 0.0 % on all six at both
# ends, where S085's `RfpMinPly` sat at one for 72.5 %.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019); they are here so
# that "the fitted vector changes the search" is a measurement and not a claim.
#
#   chesso bench   4646334 (3a649c0) -> 5443203 (phase one) -> 4393575 (fitted)
#                  The fitted vector takes 19.3 % off phase one's tree and
#                  lands 5.4 % **below** the reference's, which is the sign
#                  phase one did not have: the seeded equal authority was
#                  growing the tree and the fit is what took it back.
#   tools/search_bench.py, depth 9, phase one -> fitted
#     midgame       43974 ->  43949, c3d5       kiwipete 104700 -> 104912, e2a6
#     tactical      29323 ->  28832, d7c8q
#   tools/search_bench.py, depth 12, phase one -> fitted
#     midgame      167581 -> 170594, c3d5       kiwipete 648155 -> 624405,
#     tactical     128534 -> 128731, d7c8q      kiwipete's best move moves
#                                               `e2a6` -> `d5e6`
#
#   The three positions disagree in direction at both depths, which is the
#   ordinary signature of a reordering, and one best move moves. Node counts
#   move by construction, so INV-6's discharge is not available and games are
#   the only thing that can decide this step.
#
#   The census is beside it and says neither table is inert at the fitted
#   scale: `adocs/data/S231_census.txt`, 400 positions at depth 10, **96.53 %
#   of history writes have a previous move to index and 94.24 % have a move
#   two plies back**; 95.30 % of quiet `score_move()` evaluations consult the
#   one-ply term and 95.72 % the two-ply one; of those, **19.89 % read a
#   non-zero one-ply entry and 27.51 % a non-zero two-ply entry**. The same
#   file carries the control at phase one's incumbent vector on the same
#   instrumented binary -- 96.53 / 95.36 / 20.22 and 94.32 / 95.79 / 27.31 --
#   so no share here is the fit's doing to within four tenths of a point, and
#   what the fit moved is the size of the tree.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair, because the step asks for a gainer**:
# S231's accepts names `{0, 5}` nElo and the lane's header repeated it before
# the fit returned. DEC-063 sizes bounds from the expectation and there is no
# expectation to size this from that transfers: the published record treats the
# two-ply table as the second half of the one-ply idea and quotes no figure
# this engine's search can be held to (DEC-019, and the three times a published
# figure did not transfer here).
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1,
# `adocs/plan.md`'s cost table). Budgeted at **2110 games an hour**, which is
# `.moltke.local.md`'s standing figure for `8+0.08` on `noob_3moves.epd` under
# this governor: **19.8 h** and **12.1 h**. The most recent family is the one
# to read: S098's five verdicts measured 2119, 2124, 2126, 2128 and 2128 games
# an hour on a 4.6 M node tree, which would be 19.7 h and 12.0 h -- the same
# figures to a tenth. (The older S212-era five read 2133 to 2190.) The
# conservative figure is the one budgeted from. **A faster run
# would not surprise and a slower one is the thing to watch**: this candidate
# benches 4393575 against the 4.6 M tree the S098 family's rates were measured
# on, about 5 % smaller, where phase one's 5443203 would have been 18 % larger.
# An effect outside the interval in either direction ends far sooner -- the
# ledger's verdicts mean under five hours and S091's took 38 minutes. Budget
# the wall and not the mean (DEC-155): this is a run the machine is given
# whole, and it is a night.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189) -- **not the lane's `UHO_4060_v3.epd`**, which is the whole point
# of the two books: a run never verifies on the openings it tuned on
# (`adocs/eval_tuning_strategy.md` par.7, DEC-209 clause 1). Concurrency 12
# (DEC-050), whatever governor the machine is under, recorded by the run and
# not set (DEC-195). Everything `fastchess.sh` already does; nothing here
# overrides it but the output directory.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default
# is under `/tmp`, which this machine wipes at boot: two runs had to be
# relaunched for it. `.tuning/` is gitignored and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright and
# `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212). Mains, and a second
# load on the machine. **Nothing else is an abort.**
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **No defect reachable in ordinary play, on the UCI surface, or able to move a
# reported score, move or line is open.** Two test gaps are, both named here
# before a game is played and both filler behind the next strength step:
#
#   1. S231's own `I03_null_child_drops_prev2` has no direct guard test. The
#      mutant dies -- by "pruning does not hide a forced mate" -- but only
#      indirectly, and the case written for the null move asserts the negative
#      two plies after the pass, which `I03` leaves true. The step file records
#      what would close it: a drive in which the null child is the only writer
#      of the two-ply table.
#   2. **Found in this phase and recorded rather than repaired**:
#      `tests/test_search.cpp` "ordering keeps the tree small" carries a band
#      of `[3490, 69804]` derived from a count of 17451, and
#      `adocs/data/S192_node_budget.py` -- the band's own script -- reports the
#      count **outside** the middle half of that band and has done since before
#      this step. 17332 at S222, 16256 at phase one, **16246 here**; the middle
#      half is [20068, 53226]. DEC-142's re-derivation trigger has therefore
#      been standing for three steps while two step files recorded it as not
#      fired. The case passes and bounds nothing tighter than 4.3x the count.
#      Not repaired inside this landing on purpose: it is not this phase's
#      doing, it moves a test's numbers in a commit an SPRT is about to judge,
#      and under H0 `src/` reverts to a tree those numbers would no longer have
#      been derived from.
#
# `adocs/plan.md`'s Open list carries no other filler behind this entry at the
# time this file is written; if one is opened before the run starts it is named
# here by id and reach before a game is played.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> S231 whole -- the two-ply table on the scale its lane
#                   fitted, together with what that lane did to S222's three
#                   axes -- gains at least 5 nElo over the tree before it.
#                   **Keep the whole vector and the table.** The stopping
#                   run's Elo is upward-biased and is not the effect size
#                   (DEC-063); what may be written is "at least 5 nElo".
#                   The step then owes DEC-222's two removal verdicts, one at
#                   a time, each `{-5, 0}` nElo and a night: **first the killer
#                   slots** (`src/data_structures.hpp` `killer_moves`), **then
#                   the countermove table** (`counter_moves`), each with
#                   `hyperfine` before the run and the saving stated in its own
#                   pre-registration. A removal that reads H0 stays in the tree
#                   and is recorded as such.
#   H0 accepted  -> S231 whole does not gain 5 nElo over the tree before it.
#                   The step's accepts then binds: the zero is recorded as a
#                   zero and **the two-ply idea leaves the plan with a decision
#                   that says why**. S005, S006 and S015 are the precedent that
#                   a measured zero is recorded as a zero and may still be
#                   kept; DEC-194 is the precedent for not keeping one whose
#                   interval sits below the bound.
#
#                   **The revert is clean here in a way S222's would not have
#                   been, and that is why no axis is stranded**: `src/` goes
#                   back to `3a649c0` whole -- the table, the three new axes,
#                   the halved ceiling and the one-ply three -- and the one-ply
#                   three's pre-S231 values *are* exactly what `3a649c0`
#                   carries (17, 18, 26, S222's own fit, which its own SPRT
#                   already read H1 on). Nothing that was fitted in this lane
#                   is left in the tree without a verdict, and nothing that had
#                   a verdict is thrown away with it.
#
#                   DEC-222's two removals are **not owed** under H0: the
#                   history stack they were to be measured against did not
#                   ship.
#   No verdict   -> record as zero and decide with the reason stated, not
#                   implied. S005, S006, S015 and DEC-103 are the precedent for
#                   keeping a measured zero. The lane's own result stands
#                   either way as what it is -- a fit, never a result (DEC-019).
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.
# COMMITS as amended by DEC-220: the commit that closes this verdict carries
# fastchess's own result block before its `Bench:` line, and `tools/gate.sh`
# refuses a block whose shas do not match the named log's result line.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR.
#
# REF is `3a649c0`, S098's completing commit -- the last tree without the
# two-ply table. It is pinned in this file and not left to default to `HEAD`,
# because the step file pinned it on 2026-09-18, before phase one landed, for
# exactly this reason (DEC-210: one vector under one verdict).
#
# CAND is `HEAD`, which is the landing commit once the coordinator has made
# it. It is left as `HEAD` and not pinned by this file because this file is
# written before that commit exists; the coordinator pins it to the sha after
# committing, and `fastchess.sh`'s banner prints both shas with their commit
# dates before the first game, so what the run measures is on screen and never
# **Pinned 2026-09-20 04:25: CAND is `55891bb`, "Land S231's phase three: the
# lane's fitted vector becomes the defaults" -- the default below carries it.**
# assumed. REF in the environment overrides for any follow-up leg.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-3a649c0}" \
     CAND="${CAND:-55891bb}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s231_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
