#!/usr/bin/env bash
# S333: the guards adocs/data/S122_corpus.sh gained, seen red and green, each untested
# refusal killed by a planted break, and S122's regenerated preparation read by them.
#
#   adocs/data/S333_guards.sh red       # S122_corpus.sh smoke, its cases run by the script before S333
#   adocs/data/S333_guards.sh green     # S122_corpus.sh smoke, its cases run by itself
#   adocs/data/S333_guards.sh mutants   # the smoke run by copies with one check broken each
#   adocs/data/S333_guards.sh real      # .tuning/s122/prepared's records read in a mirror, and adopted there
#
# Each prints S333-GUARDS-<STAGE>-DONE or -FAILED as its last line. Scratch is S, by
# default .tuning/s333; the smoke's TUNING is S/tuning, its source S331's measured run
# through a link, so the smoke never writes under .tuning/s122.
#
# `red` writes the script at BEFORE, S333's parent, beside S122_corpus.sh as
# .S333_before_corpus.sh, so that its root is the tree's, and removes it on exit. It
# passes when the smoke fails exactly so: every case but the two controls failing
# (refusals not refused, and adopt's passes on a stage it lacks), and the controls,
# control_all and adopted_reads, passing.
#
# `mutants` writes, one at a time beside S122_corpus.sh, a copy with one check of adopt
# broken (MUTANTS below: the refusal turned off, or verify.txt's stamp written in a form
# no stage reads) and runs the smoke by it and against it. It passes when each copy's
# smoke fails on exactly the case named for it, every other case as it must be.
#
# `real` writes nothing under .tuning/s122 and runs no stage there. It builds a mirror,
# S/real_mirror, of copies of the preparation's files under 1 MB and links to the rest,
# which no stage writes while their markers are present. On the mirror verify, filter,
# split, sample and k each refuse it as made before S333, the mirror unchanged; adopt
# runs with ADOPT_DRY=1, the mirror unchanged, then for real; and filter, split, sample,
# k and record read it after. verify then reads the pool into S/real_verify. The real
# directory's manifest (every file's name, size and mtime, and the sha256 of each under
# 1 MB) is held equal from start to end. About 2 min, all of it reading.

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
S="${S:-$root/.tuning/s333}"
BEFORE=9daee21
corpus_sh="$root/adocs/data/S122_corpus.sh"
before_sh="$root/adocs/data/.S333_before_corpus.sh"
mutant_sh="$root/adocs/data/.S333_mutant_corpus.sh"
POOL_REAL="$root/.tuning/s122/corpus"
PREP_REAL="$root/.tuning/s122/prepared"
CASES=25
CONTROLS="control_all adopted_reads"

say() { echo "$*"; }

script_line() { say "script    adocs/data/S122_corpus.sh, sha256 $(sha256sum "$corpus_sh" | cut -d' ' -f1)"; }

smoke_tuning() {
  mkdir -p "$S/tuning/s331" || return 1
  ln -sfn "$root/.tuning/s331/measure" "$S/tuning/s331/measure"
}

red() {
  local out name failed=0
  smoke_tuning || return 1
  script_line
  git -C "$root" show "$BEFORE:adocs/data/S122_corpus.sh" > "$before_sh" || return 1
  trap 'rm -f "$before_sh"' EXIT
  say "red       the cases run by $BEFORE's S122_corpus.sh, sha256 $(sha256sum "$before_sh" | cut -d' ' -f1)"
  out=$(SMOKE_AGAINST="$before_sh" TUNING="$S/tuning" "$corpus_sh" smoke 2>&1)
  grep -E '^smoke' <<< "$out"
  say "red       the smoke's last line: $(tail -n 1 <<< "$out")"
  [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-SMOKE-FAILED ]] || { say "red: the smoke did not fail"; failed=1; }
  for name in $(sed -n 's/^smoke case \([a-z0-9_]*\): .*/\1/p' <<< "$out"); do
    if [[ " $CONTROLS " == *" $name "* ]]; then
      grep -q "^smoke case $name: passes, as it must be" <<< "$out" || { say "red: control $name did not pass"; failed=1; }
    else
      grep -q "^smoke case $name: FAILED" <<< "$out" || { say "red: case $name was not red"; failed=1; }
    fi
  done
  grep -q "^smoke     $CASES cases, 2 as they must be, $(( CASES - 2 )) not$" <<< "$out" ||
    { say "red: not $CASES cases, $(( CASES - 2 )) red"; failed=1; }
  rm -f "$before_sh"
  return "$failed"
}

green() {
  local out
  smoke_tuning || return 1
  script_line
  out=$(TUNING="$S/tuning" "$corpus_sh" smoke 2>&1)
  grep -E '^(smoke|k |rows |games |S122-PREPARED)' <<< "$out"
  say "green     the smoke's last line: $(tail -n 1 <<< "$out")"
  [[ "$(tail -n 1 <<< "$out")" == S122-CORPUS-SMOKE-DONE ]] &&
    grep -q "^smoke     $CASES cases, $CASES as they must be, 0 not$" <<< "$out"
}

# The mutant named NAME of S122_corpus.sh into mutant_sh, and the case that must kill it
# printed: one text of the script, which must occur once, replaced.
mutate() {
  python3 - "$corpus_sh" "$mutant_sh" "$1" <<'PY'
import sys
src, dst, name = sys.argv[1:4]
MUTANTS = {
    # Finding 1: verify.txt's stamp in a form named_pool does not read.
    "verify_stamp": ("adopted_reads",
        '      binary_line pool "$POOL/generate.log"\n      echo "adopted',
        '      if [[ "$name" == verify.txt ]]; then echo "pool      $now"; else binary_line pool "$POOL/generate.log"; fi\n      echo "adopted'),
    "pool_lines": ("adopt_pool_lines",
        """    if diff <(sed 1d "$OUT/verify.txt" | grep -vE '^(pool|view|adopted) ') <(pool_lines) > /dev/null; then""",
        "    if true; then"),
    "prepared_sha": ("adopt_prepared_sha",
        """  if [[ "$(sed -n 's/^pool //p' "$prep")" != "$POOL" ]] ||\n     [[ "$(sed -n 's/^sha256 //p' "$prep")" != "$(head -n 1 "$OUT/corpus.tsv.done")" ]]; then""",
        "  if false; then"),
    "pool_log": ("adopt_pre_s331",
        """  check_pool_log || { say "refused: $POOL fails verify's checks of its log, so nothing is adopted by it"; return 1; }""",
        "  :"),
    "prepared_view": ("adopt_validation",
        """    if [[ -z "$recorded" || "$recorded" != "$(sha256sum "$OUT/pool.tsv.$v.validation" 2>/dev/null | cut -d' ' -f1)" ]]; then""",
        "    if false; then"),
    "no_prepared": ("adopt_no_prepared",
        """    [[ -f "$OUT/$p" ]] || { say "refused: no $OUT/$p, so corpus.tsv's preparation cannot be read"; ok=0; }""",
        "    :"),
    "k": ("adopt_k",
        """  if [[ "$(sed -n 's/^k //p' "$prep")" != "$(cat "$OUT/k.txt")" ]]; then""",
        "  if false; then"),
}
if name == "list":
    print(" ".join(MUTANTS))
    sys.exit(0)
case, old, new = MUTANTS[name]
text = open(src).read()
if text.count(old) != 1:
    sys.exit(f"mutate: the text of {name} occurs {text.count(old)} times")
open(dst, "w").write(text.replace(old, new))
print(case)
PY
}

mutants() {
  local name killer out failed=0
  smoke_tuning || return 1
  script_line
  trap 'rm -f "$mutant_sh"' EXIT
  for name in $(mutate list); do
    killer=$(mutate "$name") || { say "mutants: $name not written"; failed=1; continue; }
    out=$(TUNING="$S/tuning" bash "$mutant_sh" smoke 2>&1)
    say "mutant    $name, sha256 $(sha256sum "$mutant_sh" | cut -d' ' -f1), must be killed by $killer:"
    grep -E "^smoke case $killer: |^smoke     [0-9]+ cases" <<< "$out" | sed 's/^/  /'
    if grep -q "^smoke case $killer: FAILED" <<< "$out" &&
       grep -q "^smoke     $CASES cases, $(( CASES - 1 )) as they must be, 1 not$" <<< "$out"; then
      say "mutant    $name killed by $killer alone"
    else
      say "mutants: $name not killed by $killer alone"
      failed=1
    fi
    rm -f "$mutant_sh"
  done
  return "$failed"
}

# Every file of DIR by name, size and mtime, and by sha256 when under 1 MB (find's
# -size -1M would round up and match empty files only).
manifest() {
  find "$1" -maxdepth 1 -type f -printf '%f %s %T@\n' | sort
  find "$1" -maxdepth 1 -type f -size -1024k -print0 | sort -z | xargs -0 -r sha256sum
}

# STAGE on POOL and OUT; its output, and its last line held to WANT.
run_stage() {
  local want="$1" pool="$2" out_dir="$3" stage="$4" out
  shift 4
  out=$(env "$@" POOL="$pool" OUT="$out_dir" "$corpus_sh" "$stage" 2>&1)
  sed "s/^/  $stage  /" <<< "$out"
  [[ "$(tail -n 1 <<< "$out")" == "S122-CORPUS-${stage^^}-$want" ]] ||
    { say "real: $stage ended '$(tail -n 1 <<< "$out")', not $want"; return 1; }
}

real() {
  local real_before before name m="$S/real_mirror" failed=0 t0
  t0=$(date +%s)
  script_line
  [[ -f "$PREP_REAL/prepared.txt" && -f "$POOL_REAL/generate.log" ]] || { say "real: no preparation at $PREP_REAL"; return 1; }
  say "real      $PREP_REAL, prepared from $POOL_REAL, mirrored; no stage runs on it"
  real_before=$(manifest "$PREP_REAL")
  say "real      the mirror $m: copies of the files under 1 MB, links to the rest"
  rm -rf "$m"; mkdir -p "$m" || return 1
  find "$PREP_REAL" -maxdepth 1 -type f -size -1024k -exec cp -p -t "$m" {} + || return 1
  find "$PREP_REAL" -maxdepth 1 -type f ! -size -1024k -exec ln -s -t "$m" {} + || return 1
  before=$(manifest "$m")
  for name in verify filter split sample k; do
    run_stage FAILED "$POOL_REAL" "$m" "$name" || failed=1
  done
  [[ "$before" == "$(manifest "$m")" ]] && say "real      the mirror is unchanged by the five refusals" ||
    { say "real: a refusal changed the mirror"; failed=1; }
  (( failed == 0 )) || return 1
  run_stage DONE "$POOL_REAL" "$m" adopt ADOPT_DRY=1 || failed=1
  [[ "$before" == "$(manifest "$m")" ]] && say "real      the mirror is unchanged by the dry run" ||
    { say "real: the dry run changed the mirror"; failed=1; }
  run_stage DONE "$POOL_REAL" "$m" adopt || failed=1
  for name in filter split sample k record; do
    run_stage DONE "$POOL_REAL" "$m" "$name" || failed=1
  done
  for name in verify.txt pool.tsv.root.done pool.tsv.qs.done pool.tsv.pv.done corpus.tsv.done; do
    say "real      the mirror's $name, its last lines:"
    tail -n 3 "$m/$name" | sed 's/^/  /'
  done
  diff <(grep -vE '^(S122 prepared corpus|corpus |view )' "$PREP_REAL/prepared.txt") \
       <(grep -vE '^(S122 prepared corpus|corpus |view )' "$m/prepared.txt") > /dev/null &&
    say "real      record's prepared.txt in the mirror is the real one's but its date and its paths" ||
    { say "real: the mirror's prepared.txt differs from the real one's"; failed=1; }

  rm -rf "$S/real_verify"; mkdir -p "$S/real_verify" || return 1
  run_stage DONE "$POOL_REAL" "$S/real_verify" verify || failed=1
  if [[ "$real_before" == "$(manifest "$PREP_REAL")" ]]; then
    say "real      $PREP_REAL's manifest is the same at the end as at the start, $(wc -l <<< "$real_before") lines"
  else
    say "real: $PREP_REAL changed"
    failed=1
  fi
  say "real      $(( $(date +%s) - t0 )) s"
  return "$failed"
}

case "$stage" in
  red) if red; then echo "S333-GUARDS-RED-DONE"; else echo "S333-GUARDS-RED-FAILED"; exit 1; fi ;;
  green) if green; then echo "S333-GUARDS-GREEN-DONE"; else echo "S333-GUARDS-GREEN-FAILED"; exit 1; fi ;;
  mutants) if mutants; then echo "S333-GUARDS-MUTANTS-DONE"; else echo "S333-GUARDS-MUTANTS-FAILED"; exit 1; fi ;;
  real) if real; then echo "S333-GUARDS-REAL-DONE"; else echo "S333-GUARDS-REAL-FAILED"; exit 1; fi ;;
  *) echo "usage: $0 red | green | mutants | real" >&2; exit 2 ;;
esac
