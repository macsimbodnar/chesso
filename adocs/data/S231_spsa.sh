#!/usr/bin/env bash
#
# S231's narrow continuation-history lane: the pre-registration and the runner.
#
#   nohup adocs/data/S231_spsa.sh > .tuning/spsa_s231.log 2>&1 &
#
# WHAT IS FITTED AND WHY IT IS A LANE OF ITS OWN. S231 adds a second
# continuation history table beside S222's -- keyed on the (piece, to) of the
# move **two** plies back and this move's, written at every quiet cutoff and
# summed into the same quiet ordering score on a weight of its own. S222's H1
# is what it was waiting on (DEC-210: +11.13 +/- 6.90 Elo over 6278 games,
# one vector under one verdict), and its own accepts asks for the fit before
# the verdict for DEC-019's reason: this technique's published figures are the
# widest spread on the plan and a figure decides what to try, never what to
# conclude.
#
# SIX AXES, and the six are the step's accepts verbatim: the three new ones
# together with S222's three. Their bounds are the declared bounds from
# `src/search_params.hpp` unnarrowed, which is what `check` compares against
# the binary before a game is played.
#
#   ContHistBonus       17      0 to 1000     c_end 2
#   ContHistMalus       18      0 to 1000     c_end 2
#   ContHistWeight      26      0 to 1000     c_end 4
#   ContHist2Bonus      17      0 to 1000     c_end 2
#   ContHist2Malus      18      0 to 1000     c_end 2
#   ContHist2Weight     26      0 to 1000     c_end 4
#
# Each starts at its incumbent, which for the one-ply three is S222's own
# fitted 17/18/26 and for the two-ply three is a seed derived inside this
# engine (`src/search_params.hpp` carries each derivation; DEC-084 as amended
# by DEC-105, and no number here came from another engine).
#
# **The one-ply three are in the lane and plain history's are not**, and both
# halves of that are deliberate. The two tables share one quiet band and one
# reference depth, so their scales are meaningful only against each other and
# a lane that moved one pair while pinning the other would be fitting a ratio
# it had fixed by hand. Plain history's six coefficients, `QuietHistoryMax` and
# `HistPruneCoeff` were all fitted by S222's lane on 2026-09-14 over 60000 of
# this project's own games and S127 refits everything after the block, so
# putting them here would spend this night's resolution on axes that already
# have a fit and would widen the vector one SPRT has to carry.
#
# **No `Tm*` axis and no `TmHardPercent`.** DEC-094 kept the nine
# time-management parameters out of S085 because a run tunes at a control its
# verification does not share; DEC-200 additionally found `TmHardPercent`
# reached by no node-count probe at all and excluded it from every config, this
# one named in that entry.
#
# **No second bound.** DEC-209's gauge argument applies to the second table
# word for word: under `history_gravity_update` an entry is `e + b - e|b|/M`,
# so scaling a table's bonus and bound together scales every entry and changes
# nothing the search can see, and the read then multiplies by the weight --
# (bonus, malus, bound, weight) over one table is three degrees of freedom and
# the fourth is a gauge SPSA would walk without moving the engine. So both
# tables' bound is the same definition, `CONT_HIST_BOUND` at the int16_t
# ceiling, and three axes per table is the parameterisation.
#
# **The two weights' declared ceiling is 1000 each and was 2000 for the one-ply
# weight alone.** Two weighted terms now share one quiet band, the clearance
# against the countermove band at 700000 is a property of their *total*, and
# 1000 + 1000 spans exactly the 655340 that 2000 alone did: the widest band the
# ranges admit is 32767 + 327670 + 327670 = 688107 and the clearance is 11893,
# the same number S222 asserted. The fitted 26 is nowhere near either edge.
# Consequence worth knowing before a mid-run read: `tools/spsa_s222.json`
# declares `ContHistWeight` 0 to 2000 and would now be refused by `check` as a
# config whose bounds disagree with the binary's. That is correct -- it is the
# frozen record of a run already taken, not a template, and it is not edited.
#
# REGIME. S085's, verbatim, because that is what the step's accepts says and
# what this machine has measured: 1250 iterations x 24 pairs = 30000 pairs =
# **60000 games**, `tc=2+0.02`, `Hash=16`, `Threads=1`, concurrency 12
# (DEC-050), alpha 0.602, gamma 0.101, a_ratio 0.1, r_end 0.004, seed 231.
# 24 pairs is the batch S085 measured as the optimum at fixed wall clock
# (DEV_MANUAL "Tune search parameters with SPSA"): more than half of a 6-pair
# iteration is waiting for its slowest game. The seed is this step's own id and
# nothing else -- traceable, and a seed chosen for its trajectory would be a
# result chosen after the number.
#
# **The book is `books/UHO_4060_v3.epd`, not the harness book**, as DEC-209
# clause 1 settled for every lane: `adocs/eval_tuning_strategy.md` par.7 and
# DEV_MANUAL both forbid tuning and verifying on the same openings, and
# `fastchess.sh` verifies on `books/noob_3moves.epd` since DEC-189. 30000
# rounds against 242201 openings, so the book never wraps. The adjudication is
# `fastchess.sh`'s line verbatim including `twosided=true` (S212, DEC-174).
#
# THE ESTIMATE, FROM MEASURED THROUGHPUT AND NOT FROM A GUESS (RUNS, DEC-155).
# S222's lane ran **this exact shape** -- same iterations, same pairs, same
# control, same book, same concurrency, on this machine -- and took **8 h 37 m
# 31 s** for 60000 games, 24.84 s an iteration, against its own 8 h 30 m
# estimate. The axis count does not enter: SPSA plays two evaluations an
# iteration whatever the dimension. What can move the wall is game length and
# not node rate, because a game at a fixed clock costs `2 * (base + moves *
# inc)` seconds whatever the engine's speed, which is S085's own finding -- and
# this engine orders quiets differently, so a few per cent either way is
# expected. **Estimate 8 h 45 m, ceiling 18 h** (WATCHERS: at least twice the
# expected run). Cross-check from the other direction, and it lands lower
# rather than higher: this workstation measured 2110 games an hour at 8+0.08 on
# `noob_3moves.epd` (`.moltke.local.md`), and a 2+0.02 game costs about a
# quarter of an 8+0.08 one, which prices 60000 games at about 7.1 h.
#
# **This is a night run** -- eight and three quarter hours is past DEC-155's
# four-hour line -- and the machine must be idle and on mains before it starts
# (MACHINE, POWER). `ps aux | sort -rnk3 | head` and `cat /proc/loadavg` first:
# the `check` below is load-sensitive and fails closed (DEC-200), and the
# one-minute figure is what to read against the core count, never the sum of
# the percentages (CLAUDE.md, S212). The governor is recorded and not set
# (DEC-195); this script prints it.
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py` over the run's own PGN in
# `$OUT/games.pgn`), or if the driver prints `SPSA-FAILED`, or if the machine
# loses mains or acquires a second load. **Nothing else is an abort**: a
# trajectory that looks sick is read and recorded, never adjusted mid-run --
# S085's `RfpMinPly` sat at a bound for 72.5 % of its iterations and that run
# was still read, and the adjustment is the thing that would make the result
# unreportable.
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN, named here because BUGS (DEC-171)
# requires every run's pre-registration to name them: **none at the time this
# file was written.** The coordinator re-checks `adocs/plan.md`'s Open list
# before launch and names anything that has opened since, in the launch note
# rather than by editing this header.
#
# MID-RUN READS, at about 25 % and about 50 % of the iterations, off
# `$OUT/trajectory.tsv`: `c_scale` decaying from its start towards 1 as
# designed, `y` centred with real spread rather than the "barely changing"
# trajectory the fishtest wiki calls useless, and the share of iterations each
# axis spends pinned at a bound. All three are recorded whatever they say.
#
# PRE-REGISTERED READINGS, written before a single game is played. An SPSA
# vector is a hypothesis and never a result (`adocs/eval_tuning_strategy.md`
# 4.4, DEC-019): what decides S231 is the gainer SPRT after it, `{0, 5}` nElo
# at the harness regime, **against `3a649c0`** -- the tree before this step's
# first landing, which is where the two-ply table does not exist. One vector
# under one verdict, DEC-210's reading, and that run gets its own
# pre-registration with the fitted values named (DEC-143).
#
#   The vector moves       -> round to the integers the shipping build uses,
#                             edit the defaults in `src/search_params.hpp`,
#                             rebuild **Release** -- never measure the tune
#                             build (S073) -- and take the SPRT.
#
#   The rounded vector
#   equals the incumbent   -> a stuck lane, recorded as one. **The SPRT is
#                             still owed**, and this is where S231 differs from
#                             S222's header: the reference is the tree before
#                             this step, so what the run prices is the table
#                             itself and not the lane's movement. The step then
#                             also reports that the scale was not findable at
#                             this budget, which is a different statement from
#                             the technique not working.
#
#   ContHist2Weight ends
#   at or under 5          -> the fit put the second table under a twentieth of
#                             the quiet band, which is the lane saying the
#                             two-ply term is not worth authority beside the
#                             one-ply one. **If the SPRT then reads H1 the
#                             verdict does not belong to the two-ply table**:
#                             at that weight the gain is whatever else in the
#                             vector moved, and the step's accepts may not be
#                             read as "the two-ply table works". A second
#                             SPRT is then owed, pre-registered in its own
#                             script -- the fitted vector against the same
#                             vector with `ContHist2Weight` pinned at 0,
#                             gainer `{0, 5}` nElo at the harness regime. H1
#                             there is the second table's own gain; H0 records
#                             it as a zero and the two-ply idea leaves the plan
#                             with a decision that says why, the one-ply axes'
#                             fitted values landing either way because the
#                             first verdict was theirs. Threshold and bounds
#                             fixed here, before the fit, so the reading is not
#                             chosen after the number.
#
#   ContHist2Weight ends
#   well above 26          -> the two-ply term wanted more of the quiet band
#                             than equal authority with the one-ply table gave
#                             it. Read together with `ContHistWeight`, since
#                             only the ratio of the two spans means anything,
#                             and against plain history's fitted
#                             `QuietHistoryMax` of 8831, which this lane does
#                             not move. Not a verdict: the SPRT is.
#
#   ContHist2Weight ends
#   near 26                -> equal authority was about where the fit wanted
#                             it, and the seed's derivation is not what the
#                             step turns on. The SPRT alone decides, exactly as
#                             it would have without this row.
#
#   ContHistWeight moves
#   a long way              -> the two tables compete for one band and the fit
#                             re-split it. Recorded, and it is not a verdict on
#                             either table; it is also the one outcome that
#                             makes S222's fitted 26 no longer the shipping
#                             value, which the SPRT's pre-registration must
#                             name among the values it carries.
#
#   The SPRT reads H0      -> the step's accepts binds: the zero is recorded as
#                             a zero and **the two-ply idea leaves the plan
#                             with a decision that says why**. The revert is
#                             clean here in a way S222's would not have been --
#                             `src/` goes back to `3a649c0` whole, table, three
#                             new axes and the one-ply three with it, and no
#                             axis is stranded without a verdict, because the
#                             one-ply three's pre-S231 values are exactly what
#                             `3a649c0` carries. S005, S006 and S015 are the
#                             precedent for recording a zero; DEC-194 is the
#                             precedent for not keeping one whose interval sits
#                             below the bound.
#
# WATCHERS, DEC-061. `tools/spsa_driver.py` prints `SPSA-DONE` or
# `SPSA-FAILED` as its last line on every exit path, including a config that
# will not load, and this script prints `SPSA-FAILED` on each of its own. The
# `check` stage writes its own log beside the run log and only its non-marker
# lines are echoed on, so the **first** `SPSA-(DONE|FAILED)` in the run log is
# the run's and a watcher never has to count to two -- S222's lane had to be
# repaired for exactly that, and counting is what WATCHERS says a watcher must
# not do. The watcher polls the whole log -- never `tail -f | grep` -- with
# four exits: the marker, the process's death, an 18 h ceiling, and a manual
# stop that is a belt and not load-bearing.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPSA-FAILED: cd" >&2; exit 1; }

OUT="${OUT:-.tuning/spsa_s231_$(date +%Y%m%d_%H%M%S)}"
CONFIG="${CONFIG:-tools/spsa_s231.json}"
ENGINE="build-tune/src/chesso"

echo "S231 continuation-history lane"
echo "  config   $CONFIG"
echo "  out      $OUT"
echo "  engine   $ENGINE"
echo "  head     $(git rev-parse --short HEAD) $(git log -1 --format=%cd --date=short)"
echo "  dirty    $(git status --porcelain | wc -l) tracked paths"
echo "  load     $(cat /proc/loadavg 2>/dev/null || echo unknown)"
echo "  governor $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo unknown)"

# The tune build is the binary this plays, and it is rebuilt here rather than
# assumed current: a config whose axes the binary does not carry is what
# `check` is for, but a binary older than the config is what nothing catches.
cmake --build build-tune -j12 >/dev/null 2>&1 ||
  { echo "SPSA-FAILED: build-tune" >&2; exit 1; }
[[ -x "$ENGINE" ]] || { echo "SPSA-FAILED: no $ENGINE" >&2; exit 1; }

echo "  binary   $(sha256sum "$ENGINE" | cut -c1-64)"

# DEC-200: every run writes its own config and passes `check` on the day, on an
# idle machine. Every axis is probed at both of its own bounds and has to
# search a different number of nodes, or it is refused by name.
#
# The check's own marker does not reach the run log -- see WATCHERS above.
CHECK_LOG="${OUT}.check.log"
echo "  check    $CHECK_LOG"
if ! python3 tools/spsa_driver.py check "$CONFIG" --engine "$ENGINE" \
     > "$CHECK_LOG" 2>&1; then
  cat "$CHECK_LOG" >&2
  echo "SPSA-FAILED: check refused the config" >&2
  exit 1
fi
grep -v '^SPSA-DONE$' "$CHECK_LOG" || true

mkdir -p "$OUT" || { echo "SPSA-FAILED: mkdir $OUT" >&2; exit 1; }

# Not `exec`: this shell stays the watched pid for the run's whole life and
# prints the marker itself when the driver leaves without one -- a signal, an
# uncaught crash -- so WATCHERS' "success and failure both" holds on the one
# path the driver cannot cover.
python3 tools/spsa_driver.py run "$CONFIG" --out "$OUT"
rc=$?
[[ "$rc" -eq 0 ]] || { echo "SPSA-FAILED: spsa_driver.py exited $rc" >&2; exit "$rc"; }
exit 0
