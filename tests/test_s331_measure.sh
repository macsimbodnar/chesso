#!/usr/bin/env bash
#
# S331: adocs/data/S331_measure.sh, the measurement's wrapper, over a stub
# datagen that records its arguments and writes a two-game run the reader
# accepts. No game is played; a few seconds, most of it case 5's sleep.
#
#   1. the run stage's arguments reach datagen intact, once, and the run ends
#      S331-MEASURE-DONE with its reading written.
#   2. a run already finished is read again, not replayed: the stub is not
#      called a second time and generate.log is unchanged.
#   3. a run that did not finish is refused, S331-MEASURE-FAILED, unreplayed.
#   4. a seed range that meets a logged run's is refused before anything runs.
#   5. an edit to the script while it runs does not reach the run. Precondition:
#      the edit lands while the stub is still running. The measurement's first
#      run, 2026-11-03, failed after a complete datagen run with `line 128:
#      s-out: command not found`: the script's header had been rewritten in
#      place while it played, and bash reads a script as it goes. Observed red
#      on the script as first delivered; the body is one brace group since.
#
#   test_s331_measure.sh <repository root> [script]
set -uo pipefail

fail()
{
  echo "FAIL: $*" >&2
  exit 1
}

root="${1:-}"
[[ -n "$root" && -r "$root/adocs/data/S331_measure.py" ]] || fail "no repository root given, or '$root' holds no adocs/data/S331_measure.py"
script="${2:-$root/adocs/data/S331_measure.sh}"
[[ -r "$script" ]] || fail "no script '$script'"
cd "$root" || fail "cannot cd to $root"

work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

# The stub: --help names --games-out, as S331's datagen does; a run appends
# its arguments to $STUB_CALLS, sleeps $STUB_SLEEP with markers either side,
# and writes two games -- game 0 won by White with no recurrence, game 1 lost
# with its first recurrence at ply 20 -- and the closing lines that count them.
cat > "$work/datagen" <<'STUB'
#!/usr/bin/env bash
if [[ "${1:-}" == --help ]]; then echo "  --games-out FILE   one line a game" >&2; exit 0; fi
printf '%s\n' "$*" >> "$STUB_CALLS"
out="" games_out="" games="" threads="" seed="" quiet="" leaf=""
while (($#)); do
  case "$1" in
    --out) out="$2" ;; --games-out) games_out="$2" ;; --games) games="$2" ;;
    --threads) threads="$2" ;; --seed) seed="$2" ;; --quiet-limit) quiet="$2" ;; --leaf) leaf="$2" ;;
  esac
  shift 2
done
touch "$STUB_CALLS.started"
sleep "${STUB_SLEEP:-0}"
for kind in root qs pv; do
  printf '8/8/8/8/8/8/8/K6k w - - 0 1\t1.0\t30\t0\t0\t10\t0\n8/8/8/8/8/8/8/K6k w - - 0 1\t0.0\t-30\t0\t1\t12\t0\n8/8/8/8/8/8/8/K6k w - - 0 1\t0.0\t-40\t0\t1\t24\t0\n' > "$out.$kind"
done
printf '0\t1.0\t8\t-1\t40\n1\t0.0\t8\t20\t60\n' > "$games_out"
echo "datagen: $games games, 100000 nodes per move, $threads threads, seed $seed, quiet-limit $quiet, leaf $leaf" >&2
for kind in root qs pv; do
  echo "leaf $kind: 3 rows from 2 games written to $out.$kind, 1.50 a game; the leaf differs from the root on 0 (0.00 %), 0.000 plies on average" >&2
done
echo "repetition: 1 games reached a position that had occurred before, which drew a game until S331; 1 of them ended other than drawn, 0 won by White and 1 lost; 2 positions passed in them, a root row each, 1 before the first recurrence; 0 openings ended on one, discarded until S331" >&2
touch "$STUB_CALLS.ended"
STUB
chmod +x "$work/datagen"

mkdir -p "$work/logs"
echo "datagen: 10 games, 100 nodes per move, 1 threads, seed 5, quiet-limit 1000, leaf qs" > "$work/logs/other.log"

run_script()  # $1 the script, $2 OUT, $3 the console; the rest is env
{
  local file="$1" out="$2" console="$3"
  shift 3
  env DATAGEN="$work/datagen" STUB_CALLS="$out.calls" LOG_DIRS="$work/logs" OUT="$out" GAMES=2 SEED=900 "$@" \
    bash "$file" run > "$console" 2>&1
}

calls() { [[ -f "$1.calls" ]] && wc -l < "$1.calls" || echo 0; }

# 1.
run_script "$script" "$work/a" "$work/a.console"
[[ "$(tail -n 1 "$work/a.console")" == S331-MEASURE-DONE ]] || fail "1: the run did not end S331-MEASURE-DONE: $(cat "$work/a.console")"
[[ "$(calls "$work/a")" -eq 1 ]] || fail "1: datagen was called $(calls "$work/a") times, not once"
want="--out pool.tsv --leaf root,qs,pv --games 2 --nodes 100000 --threads 12 --seed 900 --quiet-limit 30000 --games-out games.tsv"
[[ "$(cat "$work/a.calls")" == "$want" ]] || fail "1: datagen was handed '$(cat "$work/a.calls")', not '$want'"
grep -q '^  of the recurred, ended other than drawn 1 ' "$work/a/reading.txt" || fail "1: no reading in $work/a/reading.txt: $(cat "$work/a/reading.txt")"
echo "ok: the arguments reach datagen intact, once, and the run is read"

# 2.
before="$(sha256sum < "$work/a/generate.log")"
run_script "$script" "$work/a" "$work/a2.console"
[[ "$(tail -n 1 "$work/a2.console")" == S331-MEASURE-DONE ]] || fail "2: the second run did not end S331-MEASURE-DONE: $(cat "$work/a2.console")"
grep -q '^run       finished before, read again' "$work/a2.console" || fail "2: the second run did not say it read again: $(cat "$work/a2.console")"
[[ "$(calls "$work/a")" -eq 1 ]] || fail "2: datagen was called again, $(calls "$work/a") calls"
[[ "$(sha256sum < "$work/a/generate.log")" == "$before" ]] || fail "2: generate.log changed"
echo "ok: a finished run is read again, not replayed"

# 3.
mkdir -p "$work/b" && echo "S331 run, unfinished" > "$work/b/generate.log"
run_script "$script" "$work/b" "$work/b.console"
[[ "$(tail -n 1 "$work/b.console")" == S331-MEASURE-FAILED ]] || fail "3: an unfinished run did not end S331-MEASURE-FAILED: $(cat "$work/b.console")"
grep -q 'did not finish' "$work/b.console" || fail "3: no line says the run did not finish: $(cat "$work/b.console")"
[[ "$(calls "$work/b")" -eq 0 ]] || fail "3: datagen was called"
echo "ok: an unfinished run is refused"

# 4.
run_script "$script" "$work/c" "$work/c.console" SEED=10
[[ "$(tail -n 1 "$work/c.console")" == S331-MEASURE-FAILED ]] || fail "4: an overlapping seed did not end S331-MEASURE-FAILED: $(cat "$work/c.console")"
grep -q 'refused: seeds \[10, 12) meet a run' "$work/c.console" || fail "4: no line names the overlap: $(cat "$work/c.console")"
[[ "$(calls "$work/c")" -eq 0 ]] || fail "4: datagen was called"
echo "ok: an overlapping seed range is refused"

# 5.
cp "$script" "$work/edited.sh" || fail "5: cannot copy $script"
run_script "$work/edited.sh" "$work/d" "$work/d.console" STUB_SLEEP=2 &
pid=$!
for _ in $(seq 100); do [[ -f "$work/d.calls.started" ]] && break; sleep 0.05; done
[[ -f "$work/d.calls.started" ]] || fail "5: the stub never started: $(cat "$work/d.console")"
# Rewritten in place, the way the first run's header edit was: the same inode,
# three lines longer above everything bash has still to read.
python3 - "$work/edited.sh" <<'EOF'
import sys
path = sys.argv[1]
text = open(path).read()
lines = text.split("\n", 1)
open(path, "w").write(lines[0] + "\n# one\n# two, a little longer\n# three, longer again than two\n" + lines[1])
EOF
[[ ! -f "$work/d.calls.ended" ]] || fail "5: precondition: the stub had ended before the edit landed"
wait "$pid"
[[ "$(tail -n 1 "$work/d.console")" == S331-MEASURE-DONE ]] || fail "5: the run edited mid-way did not end S331-MEASURE-DONE: $(cat "$work/d.console")"
! grep -q 'command not found' "$work/d.console" || fail "5: the edit reached the run: $(cat "$work/d.console")"
echo "ok: an edit to the script mid-run does not reach the run"
