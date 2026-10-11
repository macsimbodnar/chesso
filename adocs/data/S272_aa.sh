#!/usr/bin/env bash
#
# S272: THE A/A THROUGH THE BARE-FEN RELAY (DEC-143). Every change to the
# harness is followed by a fixed-rounds A/A of 1000 games, read before the next
# verdict. The relay is one: `BARE_FEN=1 ./fastchess.sh` launches both engines
# through `tools/bare_fen_relay.py`, which sends every position as a bare FEN,
# and S272's SPRT (`adocs/data/S272_sprt.sh`) plays through it. So this runs
# first, and its reading decides whether that SPRT starts.
#
#   nohup adocs/data/S272_aa.sh > .tuning/s272_aa.log 2>&1 &
#
# THE RUN. `a7bb10e` (S271's landing) against itself, both sides through the
# relay: `CAND=a7bb10e REF=a7bb10e AA=1 ROUNDS=500 BARE_FEN=1`, 1000 games with
# `-repeat`, no SPRT. 8+0.08, Hash 16, one thread a side, `noob_3moves.epd`,
# concurrency 12, fastchess.sh's adjudication -- the default regime but for the
# relay, and the regime S272's SPRT plays in.
#
# THE ESTIMATE, from `.moltke.local.md`'s 2110 games an hour on this book at
# 8+0.08: 1000 games in **28.4 minutes**, before the relay's own cost, which this
# run measures (games an hour, beside 2110). A watcher's ceiling: **3600 s**.
#
# WHAT THE RELAY COSTS A MOVE, measured before the run
# (`adocs/data/S272_relay_latency.txt`): fastchess starts the clock at `go`,
# after the `readyok` that follows `position`, so the rewrite is off the clock;
# what the clock pays is the hop of `go` in and `bestmove` out -- a median of
# 0.05 ms and a 99th percentile of 0.28 ms with nothing else running (load
# average 0.3), 0.02 ms and 2.4 ms beside twelve busy engines, against an
# 80 ms increment, on both sides alike.
#
# THE READING RULE, written before the run. The SPRT starts when all of these
# hold:
#
#   1. fastchess.sh ends `SPRT-RUN-DONE` with 1000 games and no
#      `SPRT-RUN-INVALID`: no crash and no disconnect through the relay.
#   2. The banner's relay line reads `(HEAD's)`: what played is the committed
#      relay, and its blob is the one S272_sprt.sh's banner must print too.
#   3. The census reads `relay     0 position line(s) passed unrewritten`:
#      every position fastchess sent was rewritten, so the run was the regime
#      it claims. fastchess.sh voids a run where it is not (`SPRT-RUN-INVALID`).
#   4. `tools/forfeit_report.py` at 1.0 % passes: no side over 1.0 % time
#      forfeits, the line every SPRT here aborts at.
#   5. The Elo interval fastchess prints contains 0. Both sides are one binary
#      through one relay snapshot, so an interval that excludes 0 is a seat or
#      timing asymmetry in the harness -- or the one run in twenty an unbiased
#      A/A excludes it by chance. One re-run with a new seed; a second exclusion
#      stops S272 for the owner.
#
# AND WHAT IS RECORDED BUT DOES NOT STOP IT: the pair variance, by
# `adocs/data/S105_pairs.py` (the pentanomial and the variance) and
# `adocs/data/S198_pairs.py` (against S219's band, 0.2905 +/- 0.0184, the
# current book's A/A). A bare FEN takes the game's history from both sides --
# neither engine can see a repetition of an earlier position -- so the
# self-play distribution itself may move, and a variance outside the band is
# read beside the draw rate and the terminations, not as a broken harness. The
# SPRT's bounds are nElo, which normalise by the run's own variance, so its
# expected games do not move with it. The games an hour recorded here replace
# 2110 in S272_sprt.sh's hours.
#
# ABORT RULE. A time-forfeit rate over 1.0 % on either side, counted from the
# run's own PGN with `tools/forfeit_report.py`; a crash or disconnect voids it
# (`SPRT-RUN-INVALID`); mains, and a second load on the machine. Nothing else.
#
# OPEN FINDINGS (DEC-171) this run is taken while open: S259 (the S097 multicut
# and S113 ProbCut rows no longer separate E21 and B04; test-side). Read off
# `adocs/plan.md` on 2026-10-11; the coordinator re-reads it on the day.
#
# MARKERS (WATCHERS, DEC-061). fastchess.sh prints `SPRT-RUN-DONE` or
# `SPRT-RUN-FAILED`; this script then reads the run and its own last line is
# `S272-AA-DONE <out>` or `S272-AA-FAILED: <why>`, on every path. Watch for
# `S272-AA-(DONE|FAILED)`. DONE means fastchess.sh finished and the three
# readers -- forfeit_report.py, S105_pairs.py, S198_pairs.py -- each exited 0;
# a reader exiting non-zero is FAILED, naming it. Whether the reading passes
# the rule above is the coordinator's reading of `<out>/aa_reading.txt`, not
# the marker's.
#
# THE RELAY'S NOTES FILE, for a watcher to poll while the run plays:
# `<out>/relay_notes.txt`, emptied at the start; `<out>` is
# `/home/max/ws/chesso/.tuning/s272_aa_<YYYYmmdd_HHMMSS>` when run from the
# main checkout, and the banner's `notes to` line prints the whole path. Any
# line in it voids the run at its end (rule 3); a watcher may stop it early.
#
# THE LOG THAT IS COMMITTED is the console trimmed by `tools/trim_console.py
# --summary` (DEC-235): a bare FEN hides the game's history from both sides,
# so their PVs run past repetitions and fastchess prints a four-line "PV
# continues after threefold repetition" block each time -- 93 % of the smoke
# console's bytes, about 8 KB a game. The trimmed file keeps every other line
# and the count of the blocks; the untrimmed console goes to the owner's
# archive with the PGN.
#
#   python3 tools/trim_console.py --summary < .tuning/s272_aa.log > adocs/data/S272_aa.log
#
# SMOKE=1 plays 4 games at 1+0.01 on 2 threads into a `smoke_` directory, to
# prove the plumbing; its output is not a reading.

set -uo pipefail

marked=0
fail()
{
  marked=1
  echo "S272-AA-FAILED: $*"
  exit 1
}
trap 'status=$?
      if ((marked == 0)); then echo "S272-AA-FAILED: exited $status"; fi' EXIT

repo="$(cd -- "$(dirname -- "$0")/../.." && pwd)" || fail "cannot find the repository"
cd "$repo" || fail "cd $repo"
[[ -x ./fastchess.sh ]] || fail "no executable ./fastchess.sh in $repo"

# Nothing the caller's environment holds decides the regime: fastchess.sh
# reads each of these.
unset HASH SRAND CONCURRENCY RELAY_PYTHON

stamp="$(date +%Y%m%d_%H%M%S)"
rounds=500
tc=8+0.08
concurrency=""
out="$repo/.tuning/s272_aa_$stamp"
if [[ -n "${SMOKE:-}" ]]; then
  rounds=2
  tc=1+0.01
  concurrency=2
  out="$repo/.tuning/coord/S272/smoke_aa_$stamp"
fi

env CAND=a7bb10e REF=a7bb10e AA=1 ROUNDS="$rounds" BARE_FEN=1 TC="$tc" \
    ${concurrency:+CONCURRENCY="$concurrency"} OUT="$out" ./fastchess.sh
run_status=$?
((run_status == 0)) || fail "fastchess.sh exited $run_status; its marker is above"

pgn="$out/games.pgn"
[[ -s "$pgn" ]] || fail "no games at $pgn"

# The readers, each one's status kept: a reader that exits non-zero -- a side
# over the forfeit line, a PGN a pairs reader refuses -- fails the script, so
# `S272-AA-DONE` means fastchess.sh finished and all three read clean.
reading="$out/aa_reading.txt"
failed=""
reader()
{
  local name="$1" status
  shift
  echo "--- $name" | tee -a "$reading"
  "$@" 2>&1 | tee -a "$reading"
  status=${PIPESTATUS[0]}
  echo "$name exit $status" | tee -a "$reading"
  ((status == 0)) || failed="$failed${failed:+, }$name exited $status"
}

notes="$(grep -c '' "$out/relay_notes.txt" 2> /dev/null || true)"
echo "=== S272 A/A reading, $out ===" | tee "$reading"
echo "--- relay notes (rule 3): ${notes:-0} position line(s) passed unrewritten" | tee -a "$reading"
reader forfeit_report.py python3 tools/forfeit_report.py "$pgn" --max-pct 1.0
reader S105_pairs.py python3 adocs/data/S105_pairs.py "$pgn"
reader S198_pairs.py python3 adocs/data/S198_pairs.py "$pgn"
[[ -z "$failed" ]] || fail "a reader did not read clean ($failed); the reading is $reading"

marked=1
echo "S272-AA-DONE $out"
