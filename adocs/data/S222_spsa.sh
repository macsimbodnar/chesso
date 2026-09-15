#!/usr/bin/env bash
#
# S222's narrow history lane: the pre-registration and the runner. DEC-198.
#
#   nohup adocs/data/S222_spsa.sh > .tuning/spsa_s222.log 2>&1 &
#
# WHAT IS FITTED AND WHY IT IS A LANE OF ITS OWN. S024 built the one-ply
# continuation table on plain history's own coefficients and bound, with no
# constant of its own, and the gainer SPRT accepted H0 at -5.48 +/- 7.20 nElo
# over 8954 games while the census showed the table consulted on 96.19 % of
# quiet reads and non-zero on 27.14 % of those (DEC-194). So the technique as
# the record describes it did not transfer at that scale, and DEC-194 wrote two
# suspects into S222 for a fit to test rather than an argument to settle:
# that summing two terms of equal weight doubled plain history's share of the
# quiet band, and that the shared gravity bound clipped the table. DEC-198 then
# moved S222 in front of S098 -- which scales its reduction by the same sum --
# and gave it this lane, because plain history's own six coefficients are
# themselves a seed: they ship as bonus = malus = depth squared, S085 predates
# S093 and `tools/spsa_s085.json` names no history axis, and nothing before
# S127 fits them. Eleven axes, one night, and S127 still refits everything
# after the block.
#
# **The two suspects are one axis, and that is a finding this lane is built
# on.** Under `history_gravity_update` an entry is `e + b - e|b|/M`, so scaling
# the bonus and the bound together scales every entry and changes nothing the
# search can see; the read then multiplies by the weight. Bonus, malus, bound
# and weight over one table therefore carry three degrees of freedom and not
# four, and the fourth direction is a gauge SPSA would walk without moving the
# engine -- the shape DEC-094 dropped `OrderHistoryMax` for and DEC-200 dropped
# `TmHardPercent` for. So the table's bound is fixed at the int16_t ceiling as
# a definition (`CONT_HIST_BOUND`, src/data_structures.hpp), which is also the
# widest band it can have and the strongest answer available to the clipping
# suspect, and `ContHistWeight` beside `QuietHistoryMax` carries what both
# suspects are actually about: how much of the quiet band each term spans.
#
# THE AXES, and their bounds are the declared bounds from
# `src/search_params.hpp` unnarrowed, which is what `check` compares against
# the binary before a game is played:
#
#   ContHistBonus       15      0 to 1000     c_end 2
#   ContHistMalus       15      0 to 1000     c_end 2
#   ContHistWeight      25      0 to 2000     c_end 4
#   QuietHistoryMax     8192    1 to 32767    c_end 512
#   HistoryBonusQuad    1       0 to 1024     c_end 1
#   HistoryBonusLin     0       0 to 4096     c_end 8
#   HistoryBonusConst   0   -32768 to 32767   c_end 8
#   HistoryMalusQuad    1       0 to 1024     c_end 1
#   HistoryMalusLin     0       0 to 4096     c_end 8
#   HistoryMalusConst   0   -32768 to 32767   c_end 8
#   HistPruneCoeff      576     0 to 16384    c_end 32
#
# `HistPruneCoeff` is here because DEC-205 put it here: S109's history pruning
# reads the same scale and ships nearly inert -- 0.8 % of the nodes at depth 10
# over 300 positions -- because the continuation table it was written for was
# reverted. It reads the raw plain entry and still does; making it read the sum
# is a third change inside one verdict and is left to S098, which scales by
# that sum by design (src/search.cpp says so at the rule).
#
# **No `Tm*` axis and no `TmHardPercent`.** DEC-094 kept the nine
# time-management parameters out of S085 because a run tunes at a control its
# verification does not share, which is the worst regime for them; DEC-200
# additionally found `TmHardPercent` reached by no node-count probe at all and
# excluded it from every config, this one named in that entry.
#
# THE TWO COARSE AXES ARE STATED, NOT HIDDEN. `HistoryBonusQuad` and
# `HistoryMalusQuad` are integer coefficients on `depth * depth` whose useful
# region is a handful of values, so `c_end` is at its floor of 1 and the axis
# has about four reachable settings. That is a property of the shape S093
# shipped and not of this run; the fit can express intermediate rates through
# `QuietHistoryMax` instead, which is in the lane for that reason as well as
# its own. Re-expressing those six in a finer unit -- what S222's own three
# axes do by being shares of a band -- is a step of its own and not an edit
# made inside a lane.
#
# REGIME. S085's, verbatim, because that is what the step's accepts says and
# what this machine has measured: 1250 iterations x 24 pairs = 30000 pairs =
# **60000 games**, `tc=2+0.02`, `Hash=16`, `Threads=1`, concurrency 12
# (DEC-050), alpha 0.602, gamma 0.101, a_ratio 0.1, r_end 0.004, seed 222.
# 24 pairs is the batch S085 measured as the optimum at fixed wall clock
# (DEV_MANUAL "Tune search parameters with SPSA"): more than half of a 6-pair
# iteration is waiting for its slowest game.
#
# **The book is `books/UHO_4060_v3.epd`, not the harness book**, and that is
# deliberate. `adocs/eval_tuning_strategy.md` par.7 and DEV_MANUAL both forbid
# tuning and verifying on the same openings, `fastchess.sh` verifies on
# `books/noob_3moves.epd` since DEC-189, and `books/fetch_book.sh` pins this
# second UHO-class file for exactly this use. 30000 rounds against 242201
# openings, so the book never wraps. The adjudication is `fastchess.sh`'s line
# verbatim including `twosided=true` (S212, DEC-174); the one-sided `extra` in
# `adocs/data/S085_spsa_run.json` is the record of what S085 ran and is not a
# template.
#
# THE ESTIMATE, FROM MEASURED THROUGHPUT AND NOT FROM A GUESS (RUNS, DEC-155).
# S085 ran this exact shape on this machine and took **8 h 21 m** for 60000
# games -- 24.05 s an iteration, 7186 games an hour -- against its own pre-run
# measurement of 23.47 s an iteration, 8.15 h. The engine is not the same
# engine, but a game at a fixed clock costs `2 * (base + moves * inc)` seconds
# whatever the engine's speed, which is S085's own finding, so what could move
# the wall is game length and not node rate: at 10 % longer games the cost
# rises 3.7 %. **Estimate 8 h 30 m, ceiling 17 h** (WATCHERS: at least twice
# the expected run). Cross-check from the other direction, and it lands lower
# rather than higher: this workstation measured 2110 games an hour at 8+0.08 on
# `noob_3moves.epd` (`.moltke.local.md`), and a 2+0.02 game costs about a
# quarter of an 8+0.08 one, which prices 60000 games at about 7.1 h.
#
# **This is a night run** -- eight and a half hours is past DEC-155's four-hour
# line -- and the machine must be idle and on mains before it starts (MACHINE,
# POWER). `ps aux | sort -rnk3 | head` first: the `check` below is
# load-sensitive and fails closed (DEC-200).
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
# MID-RUN READS, at about 25 % and about 50 % of the iterations, off
# `$OUT/trajectory.tsv`: `c_scale` decaying from 2.055 towards 1 as designed,
# `y` centred with real spread rather than the "barely changing" trajectory the
# fishtest wiki calls useless, and the share of iterations each axis spends
# pinned at a bound. All three are recorded whatever they say.
#
# PRE-REGISTERED READINGS, written before a single game is played. An SPSA
# vector is a hypothesis and never a result (`adocs/eval_tuning_strategy.md`
# 4.4, DEC-019): what decides S222 is the gainer SPRT after it, `{0, 5}` nElo
# at the harness regime against the commit before S222's landing, and that run
# gets its own pre-registration with the fitted values named (DEC-143).
#
#   The vector moves       -> round to the integers the shipping build uses,
#                             edit the defaults in `src/search_params.hpp`,
#                             rebuild **Release** -- never measure the tune
#                             build (S073) -- and take the SPRT.
#   The rounded vector
#   equals the incumbent   -> a stuck run. Recorded as one, and no SPRT is owed
#                             because there is no change to measure (S085's own
#                             rule). The step then reports that the scale was
#                             not findable at this budget, which is a different
#                             statement from the technique not working.
#   ContHistWeight ends
#   well above 25          -> the continuation term wanted more of the quiet
#                             band than an equal-authority sum gave it, which
#                             is DEC-194's clipping suspect restated on the
#                             axis that carries it. Read together with
#                             QuietHistoryMax, since only the ratio of the two
#                             spans means anything.
#   ContHistWeight ends
#   well below 25          -> the equal-weight sum was over-weighted, which is
#                             DEC-194's first suspect. Same caveat.
#   ContHistWeight ends
#   at or under 5 and the
#   SPRT says H1           -> the verdict speaks for the eleven-axis vector and
#                             not for the table. At a fifth of its seed the
#                             continuation term spans under a twentieth of the
#                             quiet band, and the gain is then plain history's
#                             six coefficients, fitted for the first time, which
#                             the same vector carries. The step's H1 clause does
#                             not open the two-ply table on that reading: first
#                             a second SPRT, pre-registered in its own script,
#                             the fitted vector against the same vector with
#                             ContHistWeight pinned at 0, gainer {0, 5} nElo at
#                             the harness regime. H1 there is the table's own
#                             gain and opens the two-ply step; H0 records the
#                             table as a zero, kept or dropped with the reason
#                             stated, and the six history coefficients land
#                             either way because the first verdict was theirs.
#                             Threshold and bounds fixed here, before the fit,
#                             so the reading is not chosen after the number.
#   ContHistWeight ends
#   near 25 and the SPRT
#   still says H0          -> the scale was not the cause, the fit has said so
#                             on its own axes, and the technique leaves the
#                             plan with a decision that says why (the step's
#                             own accepts). That is an outcome this file
#                             expects rather than fears: S005, S006 and S015
#                             are the precedent for recording a zero, DEC-194
#                             for not keeping one whose interval sits below it.
#
# WATCHERS, DEC-061. `tools/spsa_driver.py` prints `SPSA-DONE` or
# `SPSA-FAILED` as its last line on every exit path, including a config that
# will not load, and this script prints `SPSA-FAILED` on each of its own. The
# watcher polls the whole log -- never `tail -f | grep` -- with four exits:
# the marker, the process's death, a 17 h ceiling, and a manual stop that is a
# belt and not load-bearing.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPSA-FAILED: cd" >&2; exit 1; }

OUT="${OUT:-.tuning/spsa_s222_$(date +%Y%m%d_%H%M%S)}"
CONFIG="${CONFIG:-tools/spsa_s222.json}"
ENGINE="build-tune/src/chesso"

echo "S222 history lane"
echo "  config   $CONFIG"
echo "  out      $OUT"
echo "  engine   $ENGINE"
echo "  head     $(git rev-parse --short HEAD) $(git log -1 --format=%cd --date=short)"
echo "  dirty    $(git status --porcelain | wc -l) tracked paths"

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
# **The check's own marker does not reach this log. 2026-09-15, S222 phase
# three, and it is the only change made to this file after the run.** The
# driver ends its `check` stage with `SPSA-DONE` too -- it is one of its exit
# paths and DEC-061 asks every one of them for a marker -- so the 2026-09-14
# run's log carried a marker before its first game and two at the end, and the
# watcher armed on it had to be told to count to two instead of exiting on the
# first. Counting is what WATCHERS says a watcher must not have to do. So
# `check` writes its own file beside the log and only its non-marker lines are
# echoed on: the probe evidence still lands in the run log where it is read,
# and the first `SPSA-(DONE|FAILED)` in that log is now the run's. Everything
# above this line is the pre-registration of the run that happened and is left
# exactly as it was written.
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
# path the driver cannot cover. An `exec` that failed would have exited this
# non-interactive shell before any line after it ran, so the fallback it used
# to carry was dead. A driver that printed SPSA-FAILED and exited non-zero
# prints two markers, which the watcher's grep reads as one.
python3 tools/spsa_driver.py run "$CONFIG" --out "$OUT"
rc=$?
[[ "$rc" -eq 0 ]] || { echo "SPSA-FAILED: spsa_driver.py exited $rc" >&2; exit "$rc"; }
exit 0
