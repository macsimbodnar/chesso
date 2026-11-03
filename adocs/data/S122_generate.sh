#!/usr/bin/env bash
# S122's corpus generated again, DEC-349: the pool every S122 stage reads,
# .tuning/s122/corpus/, played by S331's datagen -- a game ends on a repetition only
# at a third occurrence -- built at HEAD and frozen in the pool directory, at
# DEC-344 (1)'s command with DEC-349's seed and `--games-out games.tsv`; and, only if
# that run is stopped, its top-up in DEC-296's form, the rest of the games under the
# pre-assigned TOPUP_SEED. The void pool, its top-up, preparation and sample, written
# by the datagen before S331, are under .tuning/s122/void_twofold/, moved aside by the
# coordinator before `freeze`.
#
#   adocs/data/S122_generate.sh freeze   # build/'s datagen at HEAD, frozen into POOL
#   adocs/data/S122_generate.sh corpus   # the pool: POOL/generate.log and its files
#   adocs/data/S122_generate.sh topup    # a stopped pool's rest: TOPUP/topup.log
#
# The games, seeds and settings are adocs/data/S122_corpus.sh's (GAMES, NODES,
# THREADS, CORPUS_SEED, TOPUP_SEED), its own lines read here, so `merge`, `verify` and
# S122_screen.sh sample hold the runs to the numbers that played them. Run from the
# main checkout. `corpus` and `topup` run for hours: each goes detached under a
# four-exit watcher (WATCHERS). The run's log ends on the marker the void runs'
# run.sh wrote, the form `merge` and `verify` read: POOL/generate.log on
# S122-CORPUS-DONE or -FAILED after `ended ..., exit N`; TOPUP/topup.log on
# S122-TOPUP-DONE or -FAILED after an `exit N` line. Every stage's last stdout line is
# S122-GENERATE-<STAGE>-DONE or -FAILED.
#
# STOPPING A RUN, for a shutdown: SIGTERM to datagen alone, by the PID the run saved,
# `kill -TERM "$(cat .tuning/s122/corpus/datagen.pid)"`, so that the wrapper writes
# `exit 143` and S122-CORPUS-FAILED and `merge` can take the pool (DEC-296), as the
# void pool was stopped. Never the wrapper: its log would end on no marker.
#
# MACHINE TIME, from S331's measurement (adocs/data/S331_reading.txt): S331's datagen at
# these settings played 2400 games in 1160 s on 12 threads with the machine to itself,
# 7448 games an hour, so 150000 games take about 20 h 10 m. DEC-349's 26 h was the
# estimate before that run: S122's old rate times the smoke's 1.54 nodes a game. A
# top-up of G games takes about G / 7448 hours. Fixed nodes: contention slows a run
# and moves no row.
#
# DISK. S331's games wrote 125.3 root rows a game against the void pool's 92.5, so the
# pool is about 1.35 x the void's 3.18 GB, about 4.3 GB, and `corpus` and `topup`
# refuse to start under FREE_GB_FLOOR, 6 GB, free. The preparation after it needs
# about 8.5 GB more (the void's 6.2 GB x 1.35), which this script does not check.
set -uo pipefail

# The whole body is one brace group, read before any of it runs: bash reads a script as
# it goes, and S331's measurement failed once on an edit made to its script mid-run.
{

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
TUNING="${TUNING:-$root/.tuning}"
POOL="${POOL:-$TUNING/s122/corpus}"
TOPUP="${TOPUP:-$TUNING/s122/topup}"
export TUNING POOL TOPUP
corpus_sh="$root/adocs/data/S122_corpus.sh"
merger="$root/adocs/data/S082_pool_merge.py"
self="$root/adocs/data/S122_generate.sh"
FREE_GB_FLOOR=6
QUIET=30000
LEAVES=root,qs,pv
# What a run writes, none of which may exist before it.
OUTPUTS="generate.log topup.log pool.tsv.root pool.tsv.qs pool.tsv.pv games.tsv datagen.pid run.sh"
# The commit that holds S331, the datagen DEC-349 asks for.
S331_COMMIT=402e80c

say() { echo "$*"; }

# S122_corpus.sh's constants, its own assignment lines evaluated here, so a smoke's
# overrides (GAMES, CORPUS_SEED, TOPUP_SEED, each overridable for a smoke and nothing
# else) reach this script and the stages it calls alike.
constants() {
  local lines name
  lines=$(grep -E '^(GAMES|NODES|THREADS|CORPUS_SEED|TOPUP_SEED)=' "$corpus_sh") ||
    { say "refused: no constants in $corpus_sh"; return 1; }
  eval "$lines"
  for name in GAMES NODES THREADS CORPUS_SEED TOPUP_SEED; do
    [[ "${!name:-}" =~ ^[0-9]+$ ]] || { say "refused: $corpus_sh gives no $name"; return 1; }
  done
}

enough_disk() {
  local free_gb
  free_gb=$(df --output=avail -BG "$1" | tail -n 1 | tr -dc '0-9')
  if (( free_gb < FREE_GB_FLOOR )); then
    say "refused: $free_gb GB free under $1, under the $FREE_GB_FLOOR GB a pool needs (about 4.3 GB at 150000 games)"
    return 1
  fi
  say "disk      $free_gb GB free under $1"
}

# A pool is never overwritten: none of a run's outputs may be in DIR.
fresh() {
  local dir="$1" name
  for name in $OUTPUTS; do
    [[ ! -e "$dir/$name" ]] ||
      { say "refused: $dir/$name exists: a pool is never overwritten; the void runs' outputs go to .tuning/s122/void_twofold/ first"; return 1; }
  done
}

# POOL's frozen datagen: the sha256 `freeze` recorded, and S331's --games-out, which a
# datagen before S331 refuses as an unknown option.
frozen() {
  local dg="$POOL/datagen"
  [[ -x "$dg" && -s "$POOL/datagen.sha256" && -s "$POOL/datagen.built" ]] ||
    { say "refused: no frozen datagen in $POOL: run freeze first"; return 1; }
  [[ "$(sha256sum "$dg" | cut -d' ' -f1)" == "$(cat "$POOL/datagen.sha256")" ]] ||
    { say "refused: $dg is not the datagen freeze recorded"; return 1; }
  [[ "$("$dg" --help 2>&1)" == *--games-out* ]] ||
    { say "refused: $dg has no --games-out: not S331's datagen"; return 1; }
}

# build/'s datagen, built at HEAD from a tree whose src/, tools/ and build files are
# HEAD's, HEAD holding S331, copied into POOL with its sha256 and the commit and bench
# it was built at (datagen.built, the log's `engine` line). Never replaced.
freeze() {
  local head untracked bench
  [[ ! -e "$POOL/datagen" ]] || { say "refused: $POOL/datagen exists: a frozen datagen is never replaced"; return 1; }
  fresh "$POOL" || return 1
  git -C "$root" merge-base --is-ancestor "$S331_COMMIT" HEAD ||
    { say "refused: HEAD does not hold S331 ($S331_COMMIT)"; return 1; }
  untracked=$(git -C "$root" ls-files --others -- src tools | grep -v '/__pycache__/')
  [[ -z "$untracked" ]] || { say "refused: untracked files under src/ or tools/:"; say "$untracked"; return 1; }
  git -C "$root" diff --quiet HEAD -- src tools CMakeLists.txt cmake ||
    { say "refused: src/, tools/ or the build files differ from HEAD; the datagen must be HEAD's"; return 1; }
  grep -qx 'CMAKE_BUILD_TYPE:STRING=Release' "$root/build/CMakeCache.txt" 2>/dev/null ||
    { say "refused: $root/build is not configured Release"; return 1; }
  head=$(git -C "$root" rev-parse --short HEAD)
  mkdir -p "$POOL" || return 1
  cmake --build "$root/build" -j"$THREADS" --target datagen chesso > "$POOL/freeze.log" 2>&1 ||
    { tail -20 "$POOL/freeze.log"; say "the build failed: $POOL/freeze.log"; return 1; }
  bench=$("$root/build/src/chesso" bench 2>&1 | tail -n 1 | awk '{ print $1 }')
  [[ "$bench" =~ ^[0-9]+$ ]] || { say "refused: chesso bench printed no node total"; return 1; }
  [[ "$("$root/build/tools/datagen" --help 2>&1)" == *--games-out* ]] ||
    { say "refused: $root/build/tools/datagen has no --games-out: not S331's datagen"; return 1; }
  cp "$root/build/tools/datagen" "$POOL/datagen.tmp" && chmod +x "$POOL/datagen.tmp" || return 1
  sha256sum "$POOL/datagen.tmp" | cut -d' ' -f1 > "$POOL/datagen.sha256" || return 1
  echo "$head, bench $bench" > "$POOL/datagen.built" || return 1
  mv "$POOL/datagen.tmp" "$POOL/datagen" || return 1
  frozen || return 1
  say "frozen    $POOL/datagen, sha256 $(cat "$POOL/datagen.sha256"), built at $head, bench $bench"
}

# A run's last lines for the console, never datagen's settings line: on a stopped run
# it is among them, and `seeds` reads every text file under .tuning, the console too,
# so a top-up's check would read the corpus's own range from it and refuse (the
# coordinator's fast check of 2026-11-04).
last_lines() {
  tail -n 3 "$1" | grep -vE '^datagen: [0-9]+ games, '
}

# The seeds against every logged run (S122_corpus.sh seeds), its lines for the log.
check_seeds() {
  local reading rc
  reading=$("$corpus_sh" seeds 2>&1)
  rc=$?
  # Its stage marker is the console's, not the log's.
  grep -v '^S122-CORPUS-SEEDS-' <<< "$reading"
  (( rc == 0 ))
}

# One datagen run in DIR, framed into LOG as the void runs' run.sh framed theirs.
# KIND is corpus or topup; GAMES_RUN and SEED_RUN its games and seed; EXTRA a line
# for the log before the command.
play() {
  local kind="$1" dir="$2" log="$3" games_run="$4" seed_run="$5" seeds="$6" extra="$7"
  local rc dpid
  (
    cd "$dir" || exit 1
    {
      if [[ "$kind" == corpus ]]; then echo "S122 corpus"; else echo "S122 corpus top-up"; fi
      echo "started   $(date -Is)"
      echo "engine    $(cat "$POOL/datagen.built")"
      echo "datagen   sha256 $(sha256sum "$POOL/datagen")"
      echo "script    sha256 $(sha256sum "$dir/run.sh" | cut -d' ' -f1), adocs/data/S122_generate.sh as it ran, copied to $dir/run.sh"
      [[ -n "$extra" ]] && echo "$extra"
      echo "$seeds"
      echo "command   datagen --out pool.tsv --leaf $LEAVES --games $games_run --nodes $NODES --threads $THREADS --seed $seed_run --quiet-limit $QUIET --games-out games.tsv"
      "$POOL/datagen" --out pool.tsv --leaf "$LEAVES" --games "$games_run" --nodes "$NODES" \
          --threads "$THREADS" --seed "$seed_run" --quiet-limit "$QUIET" --games-out games.tsv &
      dpid=$!
      echo "$dpid" > "$dir/datagen.pid"
      wait "$dpid"
      rc=$?
      if [[ "$kind" == corpus ]]; then
        echo "ended     $(date -Is), exit $rc"
        if (( rc == 0 )); then echo S122-CORPUS-DONE; else echo S122-CORPUS-FAILED; fi
      else
        echo "ended     $(date -Is)"
        echo "exit $rc"
        if (( rc == 0 )); then echo S122-TOPUP-DONE; else echo S122-TOPUP-FAILED; fi
      fi
    } > "$log" 2>&1
  )
}

corpus() {
  local seeds
  fresh "$POOL" || return 1
  frozen || return 1
  enough_disk "$POOL" || return 1
  seeds=$(check_seeds) || { echo "$seeds"; say "refused: the seeds"; return 1; }
  echo "$seeds"
  cp "$self" "$POOL/run.sh" || return 1
  say "run       $GAMES games into $POOL, seed $CORPUS_SEED, started $(date -Is); datagen's PID in $POOL/datagen.pid"
  play corpus "$POOL" "$POOL/generate.log" "$GAMES" "$CORPUS_SEED" "$seeds" ""
  last_lines "$POOL/generate.log"
  [[ "$(tail -n 1 "$POOL/generate.log")" == S122-CORPUS-DONE ]]
}

# The rest of a stopped pool: GAMES less the games S082_pool_merge.py part1 keeps of
# it, by POOL's frozen datagen at the corpus's settings under TOPUP_SEED, into TOPUP,
# for `S122_corpus.sh merge`.
topup() {
  local log="$POOL/generate.log" sha kept games_run seeds
  fresh "$TOPUP" || return 1
  frozen || return 1
  [[ -f "$log" && "$(head -n 1 "$log")" == "S122 corpus" && "$(tail -n 1 "$log")" == S122-CORPUS-FAILED ]] ||
    { say "refused: $log is not a stopped run of this script: a top-up completes a stopped pool only"; return 1; }
  grep -qE "^command .* --seed $CORPUS_SEED " "$log" ||
    { say "refused: $log's run is not seed $CORPUS_SEED's"; return 1; }
  sha=$(sed -n 's/^datagen *sha256 \([0-9a-f]\{64\}\) .*/\1/p' "$log" | head -n 1)
  [[ "$sha" == "$(cat "$POOL/datagen.sha256")" ]] ||
    { say "refused: $log's datagen is not $POOL/datagen"; return 1; }
  mkdir -p "$TOPUP" || return 1
  enough_disk "$TOPUP" || return 1
  python3 "$merger" part1 "$POOL/pool.tsv" "$log" > "$TOPUP/part1.txt" 2>&1 ||
    { cat "$TOPUP/part1.txt"; say "refused: S082_pool_merge.py part1 does not cut $POOL"; return 1; }
  kept=$(sed -n 's/^part 1    cut: the \([0-9][0-9]*\) games before game .*/\1/p' "$TOPUP/part1.txt")
  [[ "$kept" =~ ^[0-9]+$ ]] && (( kept < GAMES )) ||
    { say "refused: no kept games read from $TOPUP/part1.txt"; return 1; }
  games_run=$(( GAMES - kept ))
  seeds=$(check_seeds) || { echo "$seeds"; say "refused: the seeds"; return 1; }
  echo "$seeds"
  cp "$self" "$TOPUP/run.sh" || return 1
  say "run       $games_run games into $TOPUP, seed $TOPUP_SEED, started $(date -Is); datagen's PID in $TOPUP/datagen.pid"
  play topup "$TOPUP" "$TOPUP/topup.log" "$games_run" "$TOPUP_SEED" "$seeds" \
    "kept      $kept games of $POOL by S082_pool_merge.py part1 ($TOPUP/part1.txt); this run plays $GAMES - $kept"
  last_lines "$TOPUP/topup.log"
  [[ "$(tail -n 1 "$TOPUP/topup.log")" == S122-TOPUP-DONE ]]
}

marker="S122-GENERATE-$(tr '[:lower:]' '[:upper:]' <<< "${stage:-none}")"
case "$stage" in
  freeze|corpus|topup)
    if constants && "$stage"; then echo "$marker-DONE"; exit 0; else echo "$marker-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 freeze | corpus | topup" >&2
    exit 2 ;;
esac
}
