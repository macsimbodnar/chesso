#!/usr/bin/env bash
# S336: the two findings S333 left (S333's done file, "Findings beyond the step"), each
# on fixtures seen red by the corpus script before S336 and green by the tree's, and
# both read on the real tree without a write there.
#
#   adocs/data/S336_records.sh red     # S122_corpus.sh smoke, its cases run by BEFORE's script
#   adocs/data/S336_records.sh green   # S122_corpus.sh smoke, its cases run by itself
#   adocs/data/S336_records.sh real    # the stamped preparation read by every stage, in a mirror
#   adocs/data/S336_records.sh seeds   # `seeds` on the real tree, by BEFORE's script and the tree's
#
# Each prints S336-RECORDS-<STAGE>-DONE or -FAILED as its last line. Scratch is S, by
# default .tuning/s336; the smoke's TUNING is S/tuning, its source S331's measured run
# through a link, so the smoke never writes under .tuning/s122.
#
# `red` and `seeds` write BEFORE's S122_corpus.sh, S336's parent's by default, beside the
# tree's as .S336_before_corpus.sh, so that its root is the tree's, and remove it on exit.
# `red` passes when the smoke fails exactly so: each of S336's cases (NEW) failing and every
# other case as it must be.
#
# `real` is S333_guards.sh's `real` for the preparation as the coordinator stamped it on
# 2026-11-06. It writes nothing under .tuning/s122 and runs no stage there. The mirror,
# S/real_mirror, holds copies of the preparation's files under 1 MB and links to the
# rest. A stage writes a large file only when its record is absent (filter its view's
# marker, sample corpus.tsv.done, split split.txt), so before each stage those records
# are checked present in the mirror. On it verify, filter, split, sample, k, record,
# adopt with ADOPT_DRY=1 and all must each pass, none removing a record. After them the
# markers, split.txt and k.txt must be the real ones' bytes, and verify.txt's view and
# pool lines and prepared.txt the real ones' but for dates and paths. The real
# directory's manifest is held equal from start to end. About 1 min, all of it reading.
#
# `seeds` runs S122_corpus.sh seeds on the real tree: by BEFORE's script, refused, the
# corpus meeting its own line; by the tree's, passing; and by the tree's with SEED_LOGS
# also naming a directory made by mktemp under TMPDIR, outside the repository, that holds
# one planted log: a run inside the corpus's range, then the corpus's own line with its
# quiet limit changed. Each plant must be refused. A plant's line is written by printf
# and never printed, so no file the real `seeds` reads ever holds it. Nothing under
# .tuning or adocs/data may be newer than a stamp taken before the first run.

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
S="${S:-$root/.tuning/s336}"
BEFORE="${BEFORE:-6444f6e}"
corpus_sh="$root/adocs/data/S122_corpus.sh"
before_sh="$root/adocs/data/.S336_before_corpus.sh"
POOL_REAL="$root/.tuning/s122/corpus"
PREP_REAL="$root/.tuning/s122/prepared"
CASES=35
NEW="all_unmarked filter_again split_again split_unrecorded sample_again k_again seeds_self seeds_planted seeds_near_copy seeds_copies_only"
plants=""
wrote_before=0

say() { echo "$*"; }

marker="S336-RECORDS-$(tr '[:lower:]' '[:upper:]' <<< "${stage:-none}")"
finished=0
cleanup() {
  (( wrote_before == 0 )) || rm -f "$before_sh"
  # shellcheck disable=SC2086
  [[ -z "$plants" ]] || rm -rf $plants
  (( finished )) || echo "$marker-FAILED"
}
trap cleanup EXIT

script_line() { say "script    adocs/data/S122_corpus.sh, sha256 $(sha256sum "$corpus_sh" | cut -d' ' -f1)"; }

write_before() {
  wrote_before=1
  git -C "$root" show "$BEFORE:adocs/data/S122_corpus.sh" > "$before_sh" || return 1
  say "before    $BEFORE's S122_corpus.sh, sha256 $(sha256sum "$before_sh" | cut -d' ' -f1)"
}

smoke_tuning() {
  mkdir -p "$S/tuning/s331" || return 1
  ln -sfn "$root/.tuning/s331/measure" "$S/tuning/s331/measure"
}

red() {
  local out name failed=0
  smoke_tuning || return 1
  script_line
  write_before || return 1
  out=$(SMOKE_AGAINST="$before_sh" TUNING="$S/tuning" "$corpus_sh" smoke 2>&1)
  grep -E '^smoke' <<< "$out"
  say "red       the smoke's last line: $(tail -n 1 <<< "$out")"
  [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-SMOKE-FAILED ]] || { say "red: the smoke did not fail"; failed=1; }
  for name in $(sed -n 's/^smoke case \([a-z0-9_]*\): .*/\1/p' <<< "$out"); do
    if [[ " $NEW " == *" $name "* ]]; then
      grep -q "^smoke case $name: FAILED" <<< "$out" || { say "red: case $name was not red"; failed=1; }
    else
      grep -q "^smoke case $name: [a-z]*, as it must be" <<< "$out" || { say "red: case $name did not pass"; failed=1; }
    fi
  done
  local new_n
  new_n=$(wc -w <<< "$NEW")
  grep -q "^smoke     $CASES cases, $(( CASES - new_n )) as they must be, $new_n not$" <<< "$out" ||
    { say "red: not $CASES cases, $new_n red"; failed=1; }
  return "$failed"
}

green() {
  local out
  smoke_tuning || return 1
  script_line
  out=$(TUNING="$S/tuning" "$corpus_sh" smoke 2>&1)
  grep -E '^smoke' <<< "$out"
  say "green     the smoke's last line: $(tail -n 1 <<< "$out")"
  [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-SMOKE-DONE ]] &&
    grep -q "^smoke     $CASES cases, $CASES as they must be, 0 not$" <<< "$out"
}

# Every file of DIR by name, size and mtime, and by sha256 when under 1 MB (S333's).
manifest() {
  find "$1" -maxdepth 1 -type f -printf '%f %s %T@\n' | sort
  find "$1" -maxdepth 1 -type f -size -1024k -print0 | sort -z | xargs -0 -r sha256sum
}

# The records whose presence keeps every stage from writing a large file.
guarded="verify.txt pool.tsv.root.done pool.tsv.qs.done pool.tsv.pv.done split.txt corpus.tsv.done"
# The records a stage reading its output as made before leaves byte-equal.
held="pool.tsv.root.done pool.tsv.qs.done pool.tsv.pv.done split.txt corpus.tsv.done k.txt"

# STAGE on the mirror, after its guard; its output, its last line DONE, and no record removed.
mirror_stage() {
  local m="$1" stage="$2" out name
  shift 2
  for name in $guarded; do
    [[ -f "$m/$name" ]] || { say "real: no $m/$name before $stage: a stage could write through a link, so none runs"; return 1; }
  done
  out=$(env "$@" POOL="$POOL_REAL" OUT="$m" "$corpus_sh" "$stage" 2>&1)
  sed "s/^/  $stage  /" <<< "$out"
  [[ "$(tail -n 1 <<< "$out")" == "S122-CORPUS-${stage^^}-DONE" ]] ||
    { say "real: $stage ended '$(tail -n 1 <<< "$out")'"; return 1; }
  ! grep -q ' runs: removed' <<< "$out" || { say "real: $stage removed a record"; return 1; }
}

real() {
  local real_before m="$S/real_mirror" failed=0 t0 name held_before
  t0=$(date +%s)
  script_line
  [[ -f "$PREP_REAL/prepared.txt" && -f "$POOL_REAL/generate.log" ]] || { say "real: no preparation at $PREP_REAL"; return 1; }
  say "real      $PREP_REAL, prepared from $POOL_REAL and stamped, mirrored; no stage runs on it"
  real_before=$(manifest "$PREP_REAL")
  say "real      the mirror $m: copies of the files under 1 MB, links to the rest"
  rm -rf "$m"; mkdir -p "$m" || return 1
  find "$PREP_REAL" -maxdepth 1 -type f -size -1024k -exec cp -p -t "$m" {} + || return 1
  find "$PREP_REAL" -maxdepth 1 -type f ! -size -1024k -exec ln -s -t "$m" {} + || return 1
  say "real      $(find "$m" -maxdepth 1 -type f | wc -l) copies, $(find "$m" -maxdepth 1 -type l | wc -l) links"
  # shellcheck disable=SC2086
  held_before=$(cd "$m" && sha256sum $held)
  for name in verify filter split sample k record; do
    mirror_stage "$m" "$name" || { failed=1; break; }
  done
  (( failed )) || mirror_stage "$m" adopt ADOPT_DRY=1 || failed=1
  (( failed )) || mirror_stage "$m" all || failed=1
  (( failed == 0 )) || return 1
  # shellcheck disable=SC2086
  if [[ "$held_before" == "$(cd "$m" && sha256sum $held)" ]]; then
    say "real      the mirror's $held: the real ones' bytes after the eight stages"
  else
    say "real: a record changed in the mirror"
    failed=1
  fi
  # adopt appended the real one's pool line; verify writes it second: compared as sets.
  if diff <(grep -E '^(view|pool) ' "$PREP_REAL/verify.txt" | sort) <(grep -E '^(view|pool) ' "$m/verify.txt" | sort) > /dev/null; then
    say "real      verify's verify.txt in the mirror: its view and pool lines the real one's, as a set"
  else
    say "real: the mirror's verify.txt names other views or another pool"
    failed=1
  fi
  if diff <(grep -vE '^(S122 prepared corpus|corpus |view )' "$PREP_REAL/prepared.txt") \
          <(grep -vE '^(S122 prepared corpus|corpus |view )' "$m/prepared.txt") > /dev/null; then
    say "real      record's prepared.txt in the mirror is the real one's but its date and its paths"
  else
    say "real: the mirror's prepared.txt differs from the real one's"
    failed=1
  fi
  if [[ "$real_before" == "$(manifest "$PREP_REAL")" ]]; then
    say "real      $PREP_REAL's manifest is the same at the end as at the start, $(wc -l <<< "$real_before") lines"
  else
    say "real: $PREP_REAL changed"
    failed=1
  fi
  say "real      $(( $(date +%s) - t0 )) s"
  return "$failed"
}

# `S122_corpus.sh seeds` by SCRIPT, SEED_LOGS set to LOGS unless empty; its output, and
# its last line held to WANT and each REGEX printed, `!RE` not.
seeds_run() {
  local want="$1" script="$2" logs="$3" out re ok=1
  shift 3
  if [[ -n "$logs" ]]; then
    out=$(SEED_LOGS="$logs" bash "$script" seeds 2>&1)
  else
    out=$(bash "$script" seeds 2>&1)
  fi
  sed 's/^/  /' <<< "$out"
  [[ "$(tail -n 1 <<< "$out")" == "S122-CORPUS-SEEDS-$want" ]] || { say "seeds: ended '$(tail -n 1 <<< "$out")', not $want"; ok=0; }
  for re in "$@"; do
    case "$re" in
      '!'*) ! grep -qE -- "${re:1}" <<< "$out" || { say "seeds: printed '${re:1}'"; ok=0; } ;;
      *) grep -qE -- "$re" <<< "$out" || { say "seeds: did not print '$re'"; ok=0; } ;;
    esac
  done
  (( ok ))
}

# W, a directory outside the repository holding one log of datagen's settings line, for
# GAMES games at SEED under QUIET; in `plants`, removed on exit. Not run in a subshell.
plant() {
  w=$(mktemp -d "${TMPDIR:-/tmp}/s336_seeds.XXXXXX") || return 1
  plants+=" $w"
  printf 'datagen: %d games, %d nodes per move, %d threads, seed %d, quiet-limit %d, leaf root,qs,pv\n' \
    "$1" 100000 12 "$2" "$3" > "$w/generate.log"
  echo "$w"
}

seeds() {
  local stamp="$S/seeds.stamp" failed=0 w newer
  local own='\[2026500349, 2026650349\)'
  mkdir -p "$S" || return 1
  script_line
  write_before || return 1
  touch "$stamp"
  say "seeds     by $BEFORE's script, on the real tree: refused, the corpus meeting its own line"
  seeds_run FAILED "$before_sh" "" "refused: the corpus's $own .* meet $own" || failed=1
  say "seeds     by the tree's, on the real tree: passes, prepared/verify.txt's line read as the corpus's own"
  seeds_run DONE "$corpus_sh" "" "corpus $own, workers .*: meets none" \
    "in .*\.tuning/s122/prepared/verify\.txt.*: datagen: 150000 games, .* seed 2026500349, quiet-limit 30000, leaf root,qs,pv$" || failed=1
  plant 600 2026600000 30000 || return 1
  say "seeds     by the tree's, a plant of 600 games at seed 2026600000 under SEED_LOGS: refused, naming the plant alone"
  seeds_run FAILED "$corpus_sh" "$root/.tuning $root/adocs/data $w" \
    "refused: the corpus's $own .* meet \[2026600000, 2026600600\)" "!meet.*$own" || failed=1
  plant 150000 2026500349 1000 || return 1
  say "seeds     by the tree's, a plant of the corpus's own line with quiet-limit 1000: refused, another run"
  seeds_run FAILED "$corpus_sh" "$root/.tuning $root/adocs/data $w" \
    "refused: the corpus's $own .* meet $own" || failed=1
  newer=$(find "$root/.tuning" "$root/adocs/data" -path "$S" -prune -o -newer "$stamp" -print)
  if [[ -z "$newer" ]]; then
    say "seeds     nothing under $root/.tuning (but $S) or $root/adocs/data is newer than the stamp taken before the first run"
  else
    say "seeds: newer than the stamp:"; sed 's/^/  /' <<< "$newer"
    failed=1
  fi
  return "$failed"
}

case "$stage" in
  red | green | real | seeds) if "$stage"; then finished=1; echo "$marker-DONE"; else exit 1; fi ;;
  *) finished=1; echo "usage: $0 red | green | real | seeds" >&2; exit 2 ;;
esac
