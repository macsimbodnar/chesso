#!/usr/bin/env bash
#
# S261, the search block's own SPSA lane (DEC-222 (8)): the runner.
#
#   nohup adocs/data/S261_spsa.sh > .tuning/spsa_s261.log 2>&1 &
#   echo $! > .tuning/spsa_s261.pid
#
# THE PRE-REGISTRATION IS THE STEP FILE, NOT THIS HEADER.
# `adocs/plan_current/S261_search_block_spsa_lane.md`, "## Pre-registration",
# items (1) to (6): the axis table with every inclusion and exclusion and its
# reason, the sizing and the split rule, the reachability rule, the
# end-of-range rule, the verification SPRT and the block-boundary readings. It
# is not restated here so the two cannot drift apart; what follows is what a
# reader of the run log needs at the moment it is read.
#
# WHAT IS FITTED. `tools/spsa_s261.json`, thirty-one axes (thirty added after
# S085, plus `LmrBase` and `LmrDivisor` by DEC-269, less `SePlyFactor`), each
# at its incumbent and its declared range unnarrowed (`check` compares the
# config's bounds with the binary's and refuses a mismatch). Phase 2's check of
# 2026-10-13 found `SePlyFactor` searching the same node count at 2 and 8 on
# all four rungs; the symbol is read, so it left by item (1)'s refusal rule,
# and the other 31 all separated on a fixed-depth rung (29 at depth 9, two at
# depth 13), none needing the clock. The split test read ONE LANE (0.984).
#
# REGIME. S085's, verbatim, as S222's and S231's lanes ran it: 1250 iterations
# x 24 pairs = 30000 pairs = **60000 games**, `tc=2+0.02`, `Hash=16`,
# `Threads=1`, concurrency 12 (DEC-050), alpha 0.602, gamma 0.101, a_ratio
# 0.1, r_end 0.004, seed 261, `books/UHO_4060_v3.epd` (never the harness book,
# DEC-209 clause 1), `fastchess.sh`'s adjudication verbatim.
#
# THE ESTIMATE, FROM MEASURED THROUGHPUT FOR THIS EXACT SHAPE (RUNS, DEC-155).
# S222's lane took 8 h 37 m 31 s (24.84 s an iteration) and S231's 8 h 40 m
# 45 s (25.00 s) on this machine; the axis count does not enter, two
# evaluations an iteration whatever the dimension. **Estimate 8 h 45 m,
# ceiling 18 h (64800 s)**, at least twice the run (WATCHERS). A night run.
# Game length, not node rate, is what could move it -- a game at a fixed clock
# costs `2 * (base + moves * inc)` -- so a few per cent either way is expected.
# `.moltke.local.md`'s 2110 games an hour is an 8+0.08 figure and is not scaled
# to 2+0.02 (DEV_MANUAL "Never scale a games-per-hour figure between time
# controls"); S231's cross-check from it priced 7.1 h, lower, not higher.
#
# BEFORE LAUNCHING. The machine idle and on mains (MACHINE, POWER):
# `cat /proc/loadavg` and `ps aux | sort -rnk3 | head`. The `check` below is
# load-sensitive only on its clock rung, which none of these 31 axes reached on
# 2026-10-13, and it fails closed (DEC-200). The governor is printed and not
# set (DEC-195).
#
# ABORT RULE. Stop the run and report nothing if the time-forfeit rate passes
# **1.0 % on either side** (`tools/forfeit_report.py "$OUT/games.pgn"`), if the
# driver prints `SPSA-FAILED`, or if the machine loses mains or gains a second
# load. **Nothing else is an abort**: a trajectory that looks sick is read and
# recorded, never adjusted mid-run (S085's `RfpMinPly` sat at a bound for
# 72.5 % of its iterations and that run was still read).
#
# OPEN FINDINGS THIS RUN IS TAKEN WHILE OPEN (BUGS, DEC-171), re-read at
# `1edb3b1`: **S278** unless it lands first -- the sanitizer build's trace
# tests fail under UBSan on `eval_trace::sink`'s store, red since S262; tooling,
# compiled only into the trace library and never into `chesso`, so nothing
# reachable in play. S259 is closed. S099 is in progress and lands after this
# lane (DEC-271). The coordinator re-reads `adocs/plan.md`'s Open list on the
# day and names anything opened since in the launch note, not by editing this
# header.
#
# WATCHERS, DEC-061. `tools/spsa_driver.py` prints `SPSA-DONE` or
# `SPSA-FAILED` as its last line on every exit path, and this script prints
# `SPSA-FAILED` on each of its own. `check`'s output goes to its own log and
# only its non-marker lines are echoed, so the **first** `SPSA-(DONE|FAILED)`
# in the run log is the run's and a watcher never counts to two (S222's
# repair). The watcher polls the whole log -- never `tail -f | grep` -- and has
# four exits: the marker, the process's death, the 64800 s ceiling, and a
# manual stop that is a belt and not load-bearing.
#
# FRESH OR RESUMED, AND NEVER AN OLD DIRECTORY BY ACCIDENT. A fresh run refuses
# an `$OUT` that already exists: an `OUT` inherited from the environment and
# naming an old run would reuse that run's `engine.snapshot` (the driver copies
# the binary only when no snapshot is there) and append to its trajectory.
# `RESUME=1` with `OUT` naming the failed run is the one exception, and only
# under the step file's resume clause (item (3)): a transient `SPSA-FAILED`,
# once. It plays the run's own `engine.snapshot` and probes that same binary
# with `check`, not a rebuild; it writes `$OUT/resumed` before the driver
# starts and refuses a directory that already has one, so a second failure
# aborts. The driver's own `--resume` refuses a checkpoint whose digest is not
# this config's.
#
#   RESUME=1 OUT=<the failed run's directory> \
#     nohup adocs/data/S261_spsa.sh > .tuning/spsa_s261_resume.log 2>&1 &

set -uo pipefail

cd /home/max/ws/chesso || { echo "SPSA-FAILED: cd" >&2; exit 1; }

OUT_GIVEN="${OUT:-}"
RESUME="${RESUME:-}"
OUT="${OUT:-.tuning/spsa_s261_$(date +%Y%m%d_%H%M%S)}"
CONFIG="${CONFIG:-tools/spsa_s261.json}"

if [[ -n "$RESUME" ]]; then
  [[ -n "$OUT_GIVEN" ]] ||
    { echo "SPSA-FAILED: RESUME=1 needs OUT naming the failed run's directory" >&2; exit 1; }
  [[ -f "$OUT/checkpoint.json" && -x "$OUT/engine.snapshot" ]] ||
    { echo "SPSA-FAILED: $OUT holds no checkpoint and engine snapshot to resume" >&2; exit 1; }
  [[ ! -e "$OUT/resumed" ]] ||
    { echo "SPSA-FAILED: $OUT was resumed once already; a second failure aborts the lane" >&2; exit 1; }
  ENGINE="$OUT/engine.snapshot"
  CHECK_LOG="${OUT}.resume.check.log"
else
  [[ ! -e "$OUT" && ! -e "${OUT}.check.log" ]] ||
    { echo "SPSA-FAILED: $OUT exists; a fresh run never reuses a run directory" >&2; exit 1; }
  ENGINE="build-tune/src/chesso"
  CHECK_LOG="${OUT}.check.log"
fi

echo "S261 search-block lane"
echo "  mode     $([[ -n "$RESUME" ]] && echo "RESUME of $OUT" || echo fresh)"
echo "  config   $CONFIG"
echo "  out      $OUT"
echo "  engine   $ENGINE"
echo "  head     $(git rev-parse --short HEAD) $(git log -1 --format=%cd --date=short)"
echo "  dirty    $(git status --porcelain | wc -l) tracked paths"
echo "  load     $(cat /proc/loadavg 2>/dev/null || echo unknown)"
echo "  governor $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo unknown)"

# A fresh run plays the tune build, rebuilt here rather than assumed current: a
# binary older than the config is what nothing else catches. A resumed run
# plays the snapshot the run started with, so nothing is rebuilt for it.
if [[ -z "$RESUME" ]]; then
  cmake --build build-tune -j12 >/dev/null 2>&1 ||
    { echo "SPSA-FAILED: build-tune" >&2; exit 1; }
fi
[[ -x "$ENGINE" ]] || { echo "SPSA-FAILED: no $ENGINE" >&2; exit 1; }

echo "  binary   $(sha256sum "$ENGINE" | cut -c1-64)"

# DEC-200: every run passes `check` on the day, on an idle machine. Every axis
# is probed at both of its own bounds and has to search a different number of
# nodes, or it is refused by name. The check's own marker does not reach the
# run log -- see WATCHERS above.
echo "  check    $CHECK_LOG"
if ! python3 tools/spsa_driver.py check "$CONFIG" --engine "$ENGINE" \
     > "$CHECK_LOG" 2>&1; then
  cat "$CHECK_LOG" >&2
  echo "SPSA-FAILED: check refused the config" >&2
  exit 1
fi
grep -v '^SPSA-DONE$' "$CHECK_LOG" || true

if [[ -n "$RESUME" ]]; then
  date '+%Y-%m-%d %H:%M:%S' > "$OUT/resumed" ||
    { echo "SPSA-FAILED: cannot mark $OUT as resumed" >&2; exit 1; }
  RUN_ARGS=(--resume)
else
  mkdir "$OUT" || { echo "SPSA-FAILED: mkdir $OUT" >&2; exit 1; }
  RUN_ARGS=()
fi

# Not `exec`: this shell stays the watched pid for the run's whole life and
# prints the marker itself when the driver leaves without one -- a signal, an
# uncaught crash -- so WATCHERS' "success and failure both" holds on the one
# path the driver cannot cover.
python3 tools/spsa_driver.py run "$CONFIG" --out "$OUT" "${RUN_ARGS[@]}"
rc=$?
[[ "$rc" -eq 0 ]] || { echo "SPSA-FAILED: spsa_driver.py exited $rc" >&2; exit "$rc"; }
exit 0
