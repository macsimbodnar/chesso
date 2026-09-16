#!/usr/bin/env bash
#
# S098 verdict 1's two-axis lane: the pre-registration and the runner. DEC-212.
#
#   nohup adocs/data/S098_v1_spsa.sh > .tuning/spsa_s098v1.log 2>&1 &
#
# WHY THIS LANE EXISTS, AND WHY IT IS TWO AXES. Verdict 1 landed
# `r -= clamp(hist_sum / LmrHistDiv, +/-LmrHistClamp)` on top of the reduction
# table, with S109's gate re-pointed to the same helper. Both constants are
# seeds under DEC-105 (b), and the divisor was seeded twice in one day from two
# defensible derivations over this engine's own data -- and the two do not
# describe the same search:
#
#   8675   half the saturated history band, `QuietHistoryMax +
#          ContHistWeight * CONT_HIST_BOUND / 100` = 17350, over the clamp.
#          A statement about what the tables CAN hold. The step's own by-depth
#          ablation then measured the term inert there: +0.00 % of the bench
#          nodes at depths 9, 10 and 11 and -0.02 % at 12, because a history
#          entry reaches half its band only after many cutoffs on one move.
#   430    the 75th percentile of |hist_sum| where the rule actually reads it,
#          over S024's 400 positions at depth 12 -- 5464717 sites,
#          `adocs/data/S098_v1_hist_census.txt`. A statement about what the
#          tables DO hold. The same census puts |sum| >= 8675 at 0.011 % of
#          those sites. At 430 the term moves a quarter of them and `bench`
#          grows 60.63 %, 5685915 -> 9133516, the largest move in the ledger.
#
# An SPRT at the first seed would have priced an inert constant; at the second
# it prices a search 60 % larger at a fixed depth, which is a real question but
# not the only one available. DEC-212's ruling is that the scale is fitted
# before it is judged: **this lane, then one gainer SPRT of the fitted vector**
# (`adocs/data/S098_v1_sprt.sh`, re-pinned by the coordinator afterwards).
#
# THE AXES. Their bounds are the DECLARED bounds from `src/search_params.hpp`,
# unnarrowed, and that is forced and not chosen: `tools/spsa_driver.py`'s
# `check` refuses a config whose bounds are not equal to the binary's own `uci`
# listing, because a value outside the binary's range is refused rather than
# clamped and the refusal is invisible mid-run. The census's own region is
# carried by the seed and by `c_end` instead, which is where resolution belongs.
#
#   LmrHistDiv     430   1 to 34700   c_end 128
#   LmrHistClamp   2     0 to 4       c_end 1
#
# `c_end` for the divisor is derived from the census and not from the seed: the
# p50 is 174 and the p75 is 430, so 256 of divisor covers a quarter of the
# distribution and 128 is half that gap -- a perturbation that moves the share
# of sites the rule touches by several points, where a tenth of it would move
# that share by a fraction of a point and sit inside a 24-pair iteration's
# noise. **What that budget cannot reach is 8675**, and this file says so
# rather than implying otherwise: the fit explores the census's own p50-to-p90
# region around its seed, the band seed is in range so the axis is never
# clamped, and the inert end is what the SPRT's H0 leg tests by one release
# rebuild. If this lane wanted to walk to 8675 it would need a coarser `c_end`
# than the region it is actually fitting.
#
# **THE CLAMP IS A COARSE AXIS AND ITS ONE WARNING IS STATED, NOT HIDDEN.**
# Five reachable settings, `c_end` at the integer floor of 1 -- below 0.5 the
# driver refuses an axis outright because `round(x+c) == round(x-c)` and the
# perturbation vanishes. `check` warns that the first iteration's `2c` is 4.11
# against a range of 4, so both perturbations clamp to the bounds there and
# that iteration's gradient is noise. Measured rather than left as a worry:
# `c(k) = 1 * (1251/(k+1))**0.101` is 2.0551 at k=0 and 1.9161 at k=1, so `2c`
# is under 4 from the second iteration on. **One iteration of 1250**, and it is
# recorded here because the run's log will carry the warning.
#
# REGIME. S085's, verbatim, as S222's lane used it: 1250 iterations x 24 pairs
# = 30000 pairs = **60000 games**, `tc=2+0.02`, `Hash=16`, `Threads=1`,
# concurrency 12 (DEC-050), alpha 0.602, gamma 0.101, a_ratio 0.1, r_end 0.004,
# seed 98. 24 pairs is the batch S085 measured as the optimum at fixed wall
# clock (DEV_MANUAL "Tune search parameters with SPSA").
#
# **The book is `books/UHO_4060_v3.epd`, not the harness book.**
# `adocs/eval_tuning_strategy.md` par.7, DEV_MANUAL and DEC-209 clause 1 all
# forbid tuning and verifying on the same openings, and `fastchess.sh` verifies
# on `books/noob_3moves.epd` since DEC-189. 30000 rounds against 242201
# openings, so the book never wraps. Adjudication is `fastchess.sh`'s line
# verbatim including `twosided=true` (S212, DEC-174).
#
# THE ESTIMATE, FROM MEASURED THROUGHPUT AND NOT FROM A GUESS (RUNS, DEC-155).
# S222's lane ran this exact shape on this machine at **24.84 s an iteration**,
# so 1250 iterations is **31050 s = 8 h 37 m**. **Ceiling 17 h 15 m** (62100 s),
# twice the expectation, which is what WATCHERS asks of a ceiling. The engine
# is not the same engine -- this tree is 60 % larger at a fixed depth -- but a
# game at a fixed clock costs `2 * (base + moves * inc)` seconds whatever the
# engine's node rate, which is S085's own finding, so what could move the wall
# is game length and not the tree: at 10 % longer games the cost rises 3.7 %.
#
# **This is a night run** -- eight and a half hours is past DEC-155's four-hour
# line -- and the machine must be idle and on mains before it starts (MACHINE,
# POWER). `ps aux | sort -rnk3 | head` first: the `check` below is
# load-sensitive and fails closed (DEC-200).
#
# ABORT RULE, S222's lane's verbatim. Stop the run and report nothing if the
# time-forfeit rate passes **1.0 % on either side** (`tools/forfeit_report.py`
# over the run's own PGN in `$OUT/games.pgn`), or if the driver prints
# `SPSA-FAILED`, or if the machine loses mains or acquires a second load.
# **Nothing else is an abort**: a trajectory that looks sick is read and
# recorded, never adjusted mid-run -- S085's `RfpMinPly` sat at a bound for
# 72.5 % of its iterations and that run was still read, and the adjustment is
# the thing that would make the result unreportable.
#
# MID-RUN READS, at about 25 % and about 50 % of the iterations, off
# `$OUT/trajectory.tsv`: `c_scale` decaying from 2.055 towards 1 as designed,
# `y` centred with real spread rather than the "barely changing" trajectory the
# fishtest wiki calls useless, and the share of iterations each axis spends
# pinned at a bound -- which for `LmrHistClamp` is the figure this file already
# expects to be non-trivial, five settings being what they are. All three are
# recorded whatever they say.
#
# PRE-REGISTERED READINGS, written before a single game is played. An SPSA
# vector is a hypothesis and never a result (`adocs/eval_tuning_strategy.md`
# 4.4, DEC-019): what decides verdict 1 is the gainer SPRT after this,
# `{0, 5}` nElo at the harness regime against the commit before the landing,
# one vector under one verdict as DEC-210 read S222.
#
#   The vector moves        -> round to the integers the shipping build uses,
#                              edit the two defaults in
#                              `src/search_params.hpp`, rebuild **Release** --
#                              never measure the tune build (S073) -- and take
#                              the SPRT.
#   The rounded vector
#   equals the incumbent    -> a stuck run. Recorded as one, and **no SPRT is
#                              owed**, because there is no change to measure:
#                              S085's own rule, and S222's lane restated it.
#                              The step then reports that the scale was not
#                              findable at this budget, which is a different
#                              statement from the technique not working, and
#                              the SPRT of the census seed is the coordinator's
#                              to schedule as a separate decision.
#   LmrHistClamp fitted
#   to 0                    -> **the term is inert by the fit's own word.** At
#                              0 the helper returns the raw table for every sum
#                              and every divisor, so the fitted vector is the
#                              tree before verdict 1 and there is nothing for
#                              an SPRT to price. The SPRT runs on the fitted
#                              vector **only if the clamp is non-zero**;
#                              otherwise the technique leaves the plan with a
#                              decision that says why, which is the step's own
#                              accepts and S005/S006/S015's precedent that a
#                              measured zero is recorded as a zero. Note that
#                              the divisor's own value is then unreadable --
#                              at clamp 0 no divisor changes anything -- so
#                              nothing is concluded about the scale from that
#                              run.
#   LmrHistDiv ends at or
#   above 5362              -> the fit agrees with the **band seed**. 5362 is
#                              the census's own p99 at depth 12: above it the
#                              term reaches a ply for under one site in a
#                              hundred, which is the neighbourhood 8675 was
#                              measured inert in. The technique is then a
#                              near-inert rule this engine does not want at any
#                              scale it can express, and the SPRT is expected
#                              to read as such.
#   LmrHistDiv ends at or
#   below 1442              -> the fit agrees with the **census seed**. 1442 is
#                              the p90: at or under it the rule still touches a
#                              tenth of its sites or more, which is the class
#                              the census seed was derived for. A value under
#                              174, the p50, is the fit asking for more than the
#                              census seed gave, and is reported as that rather
#                              than as agreement.
#   LmrHistDiv ends
#   between 1442 and 5362   -> stated as such and as nothing else: between the
#                              two seeds, agreeing with neither, and the number
#                              the SPRT then prices is the fit's and not either
#                              derivation's. The thresholds are the census's
#                              own percentiles and they are fixed here, before
#                              the fit, so the reading is not chosen after the
#                              number (DEC-063's own discipline applied to a
#                              lane).
#
# WATCHERS, DEC-061. `tools/spsa_driver.py` prints `SPSA-DONE` or
# `SPSA-FAILED` as its last line on every exit path, including a config that
# will not load, and this script prints `SPSA-FAILED` on each of its own. The
# watcher polls the whole log -- never `tail -f | grep` -- with four exits:
# the marker, the process's death, a 17 h 15 m ceiling, and a manual stop that
# is a belt and not load-bearing.

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPSA-FAILED: cd" >&2; exit 1; }

OUT="${OUT:-.tuning/spsa_s098v1_$(date +%Y%m%d_%H%M%S)}"
# The config was tools/spsa_s098v1.json when this ran (2026-09-15) and moved to
# adocs/data/ on 2026-09-16 when DEC-213 removed the two parameters it names;
# the file is the record of a completed run and no longer passes `check`.
CONFIG="${CONFIG:-adocs/data/S098_v1_spsa_config.json}"
ENGINE="build-tune/src/chesso"

echo "S098 verdict 1 history-scale lane"
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
# `check`'s own marker is kept out of this log, S222's lane's fix and its
# reason: the driver ends its `check` stage with `SPSA-DONE` too -- one of its
# exit paths, and DEC-061 asks every one of them for a marker -- so a log that
# carried it would hold a marker before the first game and two at the end, and
# a watcher armed on it would have to count. Counting is what WATCHERS says a
# watcher must not have to do. So `check` writes its own file beside the log
# and only its non-marker lines are echoed on: the probe evidence still lands
# in the run log where it is read, and the first `SPSA-(DONE|FAILED)` in that
# log is the run's.
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
