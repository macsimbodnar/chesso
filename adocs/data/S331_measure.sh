#!/usr/bin/env bash
# S331: what datagen's two-fold rule, a draw at the first recurrence of a position, did to the
# games and labels of every corpus written from S207 (DEC-173) to S331, read from one run of
# the step's datagen, which ends a game on the third occurrence and prints and writes what the
# old rule would have changed. adocs/data/S331_measure.py is the reading and says how.
#
#   adocs/data/S331_measure.sh [run]               # the measurement, the coordinator's
#   BASE=<datagen before S331> adocs/data/S331_measure.sh premise
#
# RUN plays GAMES (2400) at the corpus's settings, `--leaf root,qs,pv --nodes 100000
# --quiet-limit 30000 --threads 12`, seed SEED (2026400331), with `--games-out`, into OUT
# (.tuning/s331/measure): OUT/generate.log, OUT/pool.tsv.root, .qs, .pv, OUT/games.tsv; then
# reads them into OUT/reading.txt. About 20 minutes on 12 threads with the machine to itself:
# its run of 2026-11-03 took 1160 s (S331_reading.txt). The estimate before it was 25: S122's
# corpus ran about 9136 and 8858 games an hour at these settings (its two parts' started and
# ended lines), 16 minutes for 2400 games under the old rule, and the smoke's games under the
# new rule cost 1.54 times the old rule's search nodes and wrote 1.52 times its root rows
# (S331_smoke.txt's two runs). Ceiling for a watcher: 1 h. Refused before it starts if [SEED, SEED +
# GAMES) or its worker seeds [SEED, SEED + THREADS) meet any run's range named by a datagen
# settings line in a log under LOG_DIRS, by default .tuning/ and adocs/data/ (S122_screen.sh's
# check, over every log): at writing the nearest are S122's top-up, [2026300203, 2026347791),
# and its sample, [2026260203, 2026272203). A run
# already finished is read again, not replayed; one that did not finish is refused, the
# coordinator's. Its last line is S331-MEASURE-DONE or S331-MEASURE-FAILED.
#
# PREMISE is the step's smoke: 24 games at 10000 nodes a move, 2 threads, nice 10, seed
# 2026410331, `--leaf root,qs,pv --quiet-limit 30000`, once by BASE and once by the step's
# datagen, into OUT_PREMISE (.tuning/s331/premise); S331_measure.py premise then checks that the
# old run's rows are the new run's before each first recurrence, the property the reading rests
# on. Its last line is S331-PREMISE-DONE or S331-PREMISE-FAILED.
set -uo pipefail

# The whole body is one brace group, read before any of it runs: bash reads a script as it
# goes, so an edit to this file during a run used to land mid-command. The measurement's first
# run, 2026-11-03, printed `line 128: s-out: command not found` and S331-MEASURE-FAILED after a
# complete datagen run, the header having been rewritten in place while it played.
{

root="$(git rev-parse --show-toplevel)"
STAGE="${1:-run}"
DATAGEN="${DATAGEN:-$root/build/tools/datagen}"
reader="$root/adocs/data/S331_measure.py"
NODES=100000
QUIET=30000
LEAVES=root,qs,pv

if [[ "$STAGE" == premise ]]; then
  marker=S331-PREMISE
  GAMES=24 NODES=10000 THREADS=2 SEED=2026410331 NICE=10
  OUT="${OUT_PREMISE:-$root/.tuning/s331/premise}"
else
  marker=S331-MEASURE
  GAMES="${GAMES:-2400}" THREADS=12 SEED="${SEED:-2026400331}" NICE=0
  OUT="${OUT:-$root/.tuning/s331/measure}"
fi

finished=0
trap '(( finished )) || echo "$marker-FAILED"' EXIT
fail() { echo "$*"; exit 1; }

[[ "$STAGE" == run || "$STAGE" == premise ]] || fail "unknown stage '$STAGE': run or premise"
[[ -x "$DATAGEN" ]] || fail "no $DATAGEN: build/ first"
"$DATAGEN" --help 2>&1 | grep -q -- '--games-out' ||
  fail "$DATAGEN has no --games-out: not S331's datagen"
sha=$(sha256sum "$DATAGEN" | cut -d' ' -f1)

# Every run a log names, by its own settings line, against this run's ranges.
check_seeds() {
  local seed games threads n=0 log_dirs
  read -ra log_dirs <<< "${LOG_DIRS:-$root/.tuning $root/adocs/data}"
  while read -r games threads seed; do
    n=$((n + 1))
    if (( SEED < seed + games && seed < SEED + GAMES )); then
      fail "refused: seeds [$SEED, $((SEED + GAMES))) meet a run's [$seed, $((seed + games)))"
    fi
    if (( SEED < seed + threads && seed < SEED + THREADS )); then
      fail "refused: worker seeds [$SEED, $((SEED + THREADS))) meet a run's [$seed, $((seed + threads)))"
    fi
  done < <(find "${log_dirs[@]}" -path "$OUT" -prune -o -type f -name '*.log' -print0 2>/dev/null |
           xargs -0 -r grep -hoE 'datagen: [0-9]+ games, [0-9]+ nodes per move, [0-9]+ threads, seed [0-9]+' |
           sed -E 's/datagen: ([0-9]+) games, [0-9]+ nodes per move, ([0-9]+) threads, seed ([0-9]+)/\1 \2 \3/' |
           sort -u)
  (( n > 0 )) || fail "refused: no datagen settings line in any log under ${log_dirs[*]}, so no range was checked"
  echo "seeds     [$SEED, $((SEED + GAMES))), workers [$SEED, $((SEED + THREADS))): meet none of the $n runs named in logs under ${log_dirs[*]} (its own OUT apart)"
}

# One datagen run into DIR, its stderr and its frame in DIR/generate.log, ending RUN-DONE.
play() {
  local binary="$1" dir="$2" extra=("${@:3}") started rc
  mkdir -p "$dir" || return 1
  started=$(date +%s)
  {
    echo "S331 $STAGE, $(date -Is)"
    echo "load      $(cat /proc/loadavg 2>/dev/null || sysctl -n vm.loadavg)"
    echo "datagen   sha256 $(sha256sum "$binary" | cut -d' ' -f1)  $binary"
    echo "command   datagen --out pool.tsv --leaf $LEAVES --games $GAMES --nodes $NODES --threads $THREADS --seed $SEED --quiet-limit $QUIET ${extra[*]}"
    ( cd "$dir" && nice -n "$NICE" "$binary" --out pool.tsv --leaf "$LEAVES" --games "$GAMES" \
        --nodes "$NODES" --threads "$THREADS" --seed "$SEED" --quiet-limit "$QUIET" "${extra[@]}" )
    rc=$?
    echo "exit      $rc"
    echo "elapsed   $(( $(date +%s) - started )) s"
    (( rc == 0 )) && echo RUN-DONE
  } > "$dir/generate.log" 2>&1
  grep -q '^RUN-DONE$' "$dir/generate.log"
}

if [[ "$STAGE" == premise ]]; then
  [[ -n "${BASE:-}" && -x "$BASE" ]] || fail "premise needs BASE, the datagen before S331"
  ! "$BASE" --help 2>&1 | grep -q -- '--games-out' || fail "$BASE has --games-out: not the datagen before S331"
  check_seeds
  rm -rf "$OUT/old" "$OUT/new"
  play "$BASE" "$OUT/old" || fail "the old run failed: $(tail -3 "$OUT/old/generate.log")"
  play "$DATAGEN" "$OUT/new" --games-out games.tsv || fail "the new run failed: $(tail -3 "$OUT/new/generate.log")"
  for dir in old new; do
    echo "== $dir: $(sed -n 's/^datagen   //p' "$OUT/$dir/generate.log")"
    grep -E '^(filter|leaf [a-z]+: [0-9]|walk|repetition|elapsed)' "$OUT/$dir/generate.log"
  done
  python3 "$reader" premise "$OUT/old/pool.tsv" "$OUT/new/pool.tsv" "$OUT/new/games.tsv" ||
    fail "the premise does not hold"
  finished=1
  echo "$marker-DONE"
  exit 0
fi

if [[ -f "$OUT/generate.log" ]] && ! grep -q '^RUN-DONE$' "$OUT/generate.log"; then
  fail "refused: $OUT/generate.log is a run that did not finish; the coordinator's, never resumed"
fi

if [[ ! -f "$OUT/generate.log" ]]; then
  check_seeds
  echo "run       $GAMES games into $OUT, datagen sha256 $sha, started $(date -Is)"
  play "$DATAGEN" "$OUT" --games-out games.tsv || fail "datagen failed: $(tail -5 "$OUT/generate.log")"
else
  echo "run       finished before, read again: $OUT/generate.log"
fi

grep -E '^(datagen|command|load|filter|leaf [a-z]+: [0-9]|walk|repetition|exit|elapsed)' "$OUT/generate.log"
python3 "$reader" read "$OUT" > "$OUT/reading.txt" || fail "$(cat "$OUT/reading.txt")"
cat "$OUT/reading.txt"
finished=1
echo "$marker-DONE"
exit 0
}
