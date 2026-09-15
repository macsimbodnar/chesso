#!/usr/bin/env bash
#
# S222, one-ply continuation history retried with a scale of its own, together
# with the eleven-axis history vector its own SPSA lane fitted. Candidate is
# the working tree at the commit the coordinator makes for phase three:
# `src/search_params.hpp` keeps the shape phases one and two landed and every
# one of its eleven history defaults becomes the value the lane returned. No
# rule of the search changes in this commit and no line of `src/search.cpp` or
# `src/evaluation.cpp` moves; what moves is eleven integers.
#
#   nohup adocs/data/S222_sprt.sh > .tuning/sprt_s222.log 2>&1 &
#
# WHAT IS BEING MEASURED, AND WHY IT IS ONE VERDICT AND NOT TWO. The step's
# accepts says "then one gainer SPRT", singular, and the lane's own
# pre-registration (`adocs/data/S222_spsa.sh`) says the same: an SPSA vector is
# a hypothesis and never a result, and what decides S222 is this run. The
# vector is eleven axes and they are not separable here -- the continuation
# table's three shares, plain history's six coefficients, plain history's band
# and the history-pruning threshold all price the same quiet ordering score,
# and each measured against a tree the others are absent from returns a number
# it will not have in the tree that ships. MEASUREMENT's "one change at a time"
# is satisfied at the level a fit is taken at: one lane, one vector, one
# verdict. That is S085's precedent exactly -- twelve axes, one SPRT -- and the
# cost of it is stated in the outcomes below rather than discovered afterwards.
#
# THE VECTOR, incumbent -> fitted, the driver's own rounded JSON at the end of
# `adocs/data/S222_spsa.log` and no number re-rounded by hand:
#
#   ContHistBonus         15 ->   17
#   ContHistMalus         15 ->   18
#   ContHistWeight        25 ->   26
#   QuietHistoryMax     8192 -> 8831
#   HistoryBonusQuad       1 ->    6
#   HistoryBonusLin        0 ->   19
#   HistoryBonusConst      0 ->    2
#   HistoryMalusQuad       1 ->    0
#   HistoryMalusLin        0 ->   17
#   HistoryMalusConst      0 ->   36
#   HistPruneCoeff       576 ->  612
#
# **Not a stuck run.** Every axis moved off its seed, so S085's stuck-run rule
# -- a rounded vector equal to the incumbent is recorded as one and owes no
# SPRT -- does not apply and this run is owed. The largest movements are the
# six plain-history coefficients, which nothing had ever fitted, and
# `QuietHistoryMax`. The fit splits the bonus from the malus for the first time
# and does it asymmetrically: the bonus stays quadratic at 6d^2 + 19d + 2, the
# malus comes back linear at 17d + 36 with its quadratic coefficient at zero.
#
# **`ContHistWeight` ended near 25 (at 26), which is one of the five readings
# the lane pre-registered before a game was played.** Neither of DEC-194's two
# suspects is what the fit found: the term was not over-weighted at an
# equal-authority sum and it was not starved of band either. At 26 it spans
# 26 * 32767 / 100 = 8519 against plain history's fitted 8831, which is the
# same near-equal authority the seed had at 8191 against 8192. The lane's "at
# or under 5" row -- which would have owed a second, attribution SPRT of the
# fitted vector against itself with the weight pinned at 0 -- **is not
# triggered and no pinned-zero run is taken**. `adocs/data/S222_sprt_pinned.sh`
# does not exist for that reason.
#
# WHAT THE TREE DOES, MEASURED BEFORE THE GAMES. Node counts at a fixed depth
# are not Elo and nothing here reads them as Elo (DEC-019); they are here so
# that "the fitted vector changes the search" is a measurement and not a claim.
#
#   chesso bench   5950740 -> 5685915, -4.5 %
#   tools/search_bench.py, depth 9, phases one and two -> fitted
#     midgame      74825 ->  51189    kiwipete   147048 -> 146616
#     tactical     25019 ->  39389    best moves unchanged: c3d5 / e2a6 / d7c8q
#   tools/search_bench.py, depth 12, phases one and two -> fitted
#     midgame     184125 -> 143205    kiwipete   577720 -> 570238
#     tactical     97318 -> 148060    kiwipete's best move moves back
#                                     `d5e6` -> `e2a6`, which is where it sat
#                                     before phase two moved it; the other two
#                                     are unchanged at both depths
#
#   The three positions disagree in direction -- midgame and kiwipete shrink,
#   tactical grows by half -- which is the ordinary signature of a reordering
#   and another reason the verdict is games and not nodes.
#
#   The census is beside it and says the table is not inert at the fitted
#   scale: `adocs/data/S222_census.txt`, 400 positions at depth 10, 96.62 % of
#   history writes have a previous move to index, 95.15 % of quiet score_move()
#   evaluations consult the continuation term and 19.15 % of those read a
#   non-zero entry, against S024's 97.56 / 96.19 / 27.14 on a tree three times
#   the size. The same file carries the control that attributes the third
#   figure: at the incumbent vector on this tree it is 18.94 %, so the fall
#   from 27.14 belongs to S109's and S091's pruning and not to the fit.
#
# BOUNDS. `fastchess.sh`'s own default: elo0=0 elo1=5, alpha=beta=0.05, nElo,
# `model=normalized`. **The gainer pair, because the step asks for a gainer**:
# S222's accepts names `{0, 5}` nElo and DEC-198 repeated it when the lane was
# created, both written before the fit returned. DEC-063 sizes bounds from the
# expectation and there is no published expectation to size this from -- the
# combination being measured is an unfitted table replaced by a fitted one plus
# six coefficients no published source prices for this engine -- so the bound
# stays where the step pre-registered it. S024's own run of the same table at
# plain history's scale closed H0 at -5.48 +/- 7.20 nElo over 8954 games
# (DEC-194), which is the null this is measured against and not a prior.
#
# WHAT THE PAIR COSTS (DEC-143). The nElo run-length formula prices {0, 5} at
# **41861 expected games** with the truth at the interval's midpoint and
# **25591** with it on a bound (`adocs/testing_strategy.md` section 1.1,
# `adocs/plan.md`'s cost table): at **2150 games an hour** -- the five runs
# since S212 (2133, 2162, 2190, 2155 and S091's 2137) average 2155, rounded
# down to budget from -- that is
# **19.5 h** and **11.9 h**, or 19.8 h and 12.1 h at the 2110 `.moltke.local.md`
# says to budget from. An effect outside the interval in either direction ends
# far sooner: the ledger's fourteen verdicts mean 4 h 37 m and S091's took
# 38 minutes. Budget the wall and not the mean (DEC-155): this is a run the
# machine is given whole.
#
# REGIME. 8+0.08, Hash 16, one thread a side, `books/noob_3moves.epd`
# (DEC-189) -- **not the lane's `UHO_4060_v3.epd`**, which is the whole point of
# the two books: a run never verifies on the openings it tuned on
# (`adocs/eval_tuning_strategy.md` par.7, DEC-209 clause 1). Concurrency 12
# (DEC-050), whatever governor the machine is under, recorded by the run
# (DEC-195). Everything `fastchess.sh` already does; nothing here overrides it
# but the output directory.
#
# WHERE THE OUTPUT LANDS. `OUT` is set below because `fastchess.sh`'s default is
# under `/tmp`, which this machine wipes at boot: two runs had to be relaunched
# for it. `.tuning/` is gitignored and survives.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT`). A crash or a disconnect on either side voids it outright and
# `fastchess.sh` says so itself (`SPRT-RUN-INVALID`, S212). No timed match
# starts on battery (POWER).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS rule as scoped by DEC-171).
# **None behind this block.** S194 (Open entry 1, the UCI book path executed by
# the fast suite) is open while this run is taken and is not a filler of this
# block: a harness step with no reach into play or into a reported score. Every
# filler this block carried is closed: S225 (2026-09-14, the
# `gate_extra.sh` STAGES leak), S228 (2026-09-14, the build-stamp freshness
# test), S229 (2026-09-14, json as its MIT single header), S230 (2026-09-14,
# the mined mate row for R01), S213 (stale comments and the dead API), S211
# (the originality sweep over tables and builders) and S224 (the unsourced GUI
# assets). `adocs/plan.md`'s Open list carries no filler behind this entry at
# the time this file is written; if one is opened before the run starts it is
# named here by id and reach before a game is played.
#
# PRE-REGISTERED INTERPRETATION, written before a single game is played:
#
#   H1 accepted  -> S222 whole -- the table on its fitted scale and the eight
#                   axes fitted with it -- gains at least 5 nElo over the tree
#                   before it. **Keep the whole vector**, table and history
#                   scale together, exactly as the lane returned it. The
#                   stopping run's Elo is upward-biased and is not the effect
#                   size (DEC-063); what may be written is "at least 5 nElo".
#                   The step's own H1 clause then applies: **the two-ply
#                   continuation table opens as its own step**, behind S098
#                   and not inside this one, and S098 -- which scales its
#                   reduction by the sum these two tables make -- is measured
#                   against this tree and not the one before it.
#   H0 accepted  -> S222 whole does not gain 5 nElo over the tree before it.
#                   **The scale was not the cause.** DEC-194 reverted this table on plain history's
#                   scale and S222 exists to test whether a scale of its own
#                   was what it lacked; the lane fitted that scale on its own
#                   axes, found the weight where the seed put it, and this run
#                   says the vector built on it does not gain. That is the
#                   lane's fifth pre-registered reading, written before the fit
#                   returned, and the step's accepts then binds: **the
#                   technique leaves the plan with a decision that says why**,
#                   and S005, S006 and S015 are the precedent that a measured
#                   zero is recorded as a zero and may still be kept.
#
#                   **What happens to the other eight axes is decided, not
#                   proposed (DEC-210): the whole vector reverts with the
#                   table.** The six plain-history coefficients,
#                   `QuietHistoryMax` and `HistPruneCoeff` were fitted in the
#                   same vector, with the table present, and have no verdict
#                   of their own; a tree carrying the eight without the three
#                   has been played by no run, and a change of unknown sign is
#                   not shipped on the argument that a fit beats a guess
#                   (MEASUREMENT: one change at a time, decided by SPRT).
#                   `src/` returns to `d785b89`'s search parameters and loses
#                   the table the way DEC-194's revert did. The fit is not
#                   lost: the eight axes become a step of their own -- a lane
#                   without the table, then one SPRT of their own -- seeded
#                   from `adocs/data/S222_spsa_trajectory.tsv`.
#   No verdict   -> record as zero and decide with the reason stated, not
#                   implied. S005, S006, S015 and DEC-103 are the precedent for
#                   keeping a measured zero; DEC-194 is the precedent for not
#                   keeping one whose interval sits below it. The eight-axis
#                   step above is filed in exactly the same terms.
#
# AGENTS.md WATCHERS and DEC-061: fastchess.sh prints SPRT-RUN-DONE or
# SPRT-RUN-FAILED on every exit path, so a watcher has a terminal marker.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPRT-RUN-FAILED: cd" >&2; exit 1; }

# THE PAIR. **Both shas below are placeholders the coordinator re-pins**, and
# they are written as defaults rather than left blank so the file is runnable
# and so what it will measure is on the record before it is.
#
# REF is the tree before S222 -- `d785b89`, S091's landing, whose `src/` is
# the parent of the phase-two landing `96fdc19` (`b0df255` and `f4f70c4`
# between them touch no source). DEC-210: the phase-two landing moved node
# counts and took the SPRT path for INV-6, so it was never a verdict and is
# not a baseline; measured against it, the table itself would stay unpriced.
# This run prices S222 whole -- the table on its fitted scale together with
# the eight axes fitted in the same vector -- which is what the step's accepts
# means by "against the commit before it" and what DEC-194's H0 was measured
# against. `.ref-builds/d785b89` exists from S091's run and answers
# `id name Chesso d785b89 native`. (This file first defaulted REF to
# `bdd82cc`, the commit before phase three; the coordinator re-pinned it
# before any game was played, and DEC-210 records why.)
#
# CAND is `HEAD`, which is the landing commit once the coordinator has made it.
# It is left as `HEAD` and not pinned by this file because this file is written
# before that commit exists; the coordinator pins it to the sha after
# committing, and `fastchess.sh`'s banner prints both shas with their commit
# dates before the first game, so what the run measures is on screen and never
# assumed. REF in the environment overrides for any follow-up leg.
[[ -x ./fastchess.sh ]] || { echo "SPRT-RUN-FAILED: no executable ./fastchess.sh" >&2; exit 1; }
shopt -s execfail
exec env REF="${REF:-d785b89}" \
     CAND="${CAND:-HEAD}" \
     OUT="/home/max/ws/chesso/.tuning/sprt_s222_$(date +%Y%m%d_%H%M%S)" \
     ./fastchess.sh
echo "SPRT-RUN-FAILED: exec env ./fastchess.sh" >&2; exit 127
