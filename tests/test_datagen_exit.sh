#!/usr/bin/env bash
#
# S082: datagen's exit status says whether the corpus it wrote is the one its
# closing lines describe. Until S082's phase 2 it exited 0 when a write or a
# flush failed -- the rows counted and printed, the file short of them -- and
# when a worker stopped at its attempt cap with fewer games than booked. A
# night's pool read as complete on either.
#
#   1. a run whose output cannot be written exits non-zero and says so. The
#      output is /dev/full, which takes the open and fails the flush; the
#      precondition is that the run counted rows to write, read from its own
#      closing line. Skipped where /dev/full does not exist.
#   2. a run that plays fewer games than booked exits non-zero and says so.
#      `--opening-limit -1` discards every opening, so the one worker reaches
#      its attempt cap with no game; the precondition, 0 games, is read from
#      the run's own closing line.
#   3. the control: a small run that writes what it counts exits 0, so the two
#      cases above do not pass on a datagen that always fails.
#
# Observed red against the datagen before the change: cases 1 and 2 exited 0.
#
#   test_datagen_exit.sh <datagen binary>
set -uo pipefail

fail()
{
  echo "FAIL: $*" >&2
  exit 1
}

datagen="${1:-}"
[[ -n "$datagen" && -x "$datagen" ]] || fail "no datagen binary given, or '$datagen' is not executable"

work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

small=(--games 1 --nodes 200 --threads 1 --seed 3 --leaf root)

# 3, first: the control.
"$datagen" "${small[@]}" --out "$work/ok.tsv" 2> "$work/ok.log"
status=$?
((status == 0)) || fail "control: a writable run exited $status; log: $(cat "$work/ok.log")"
rows="$(grep -oE '^leaf root: [0-9]+ rows' "$work/ok.log" | grep -oE '[0-9]+')"
[[ -n "$rows" && "$rows" -gt 0 ]] || fail "control: the run counted no rows, so nothing below is tested; log: $(cat "$work/ok.log")"
[[ "$(wc -l < "$work/ok.tsv")" -eq "$rows" ]] || fail "control: the file holds $(wc -l < "$work/ok.tsv") rows, the log $rows"
echo "ok: control, $rows rows written and exit 0"

# 1. A failed write.
if [[ -e /dev/full ]]; then
  "$datagen" "${small[@]}" --out /dev/full 2> "$work/full.log"
  status=$?
  rows="$(grep -oE '^leaf root: [0-9]+ rows' "$work/full.log" | grep -oE '[0-9]+')"
  [[ -n "$rows" && "$rows" -gt 0 ]] || fail "write: the run counted no rows, so it had nothing to fail on; log: $(cat "$work/full.log")"
  ((status != 0)) || fail "write: $rows rows to /dev/full and exit 0; log: $(cat "$work/full.log")"
  grep -q 'write' "$work/full.log" || fail "write: exit $status but no line names the failed write; log: $(cat "$work/full.log")"
  echo "ok: write, $rows rows to /dev/full and exit $status"
else
  echo "skipped: write, no /dev/full here"
fi

# 2. Fewer games than booked.
"$datagen" "${small[@]}" --nodes 100 --opening-limit -1 --out "$work/short.tsv" 2> "$work/short.log"
status=$?
grep -qE '^leaf root: 0 rows from 0 games' "$work/short.log" \
  || fail "short: the run played a game, so the cap was not reached; log: $(cat "$work/short.log")"
((status != 0)) || fail "short: 0 of 1 games and exit 0; log: $(cat "$work/short.log")"
grep -q '0 of 1 games' "$work/short.log" || fail "short: exit $status but no line says how many games were played; log: $(cat "$work/short.log")"
echo "ok: short, 0 of 1 games and exit $status"
