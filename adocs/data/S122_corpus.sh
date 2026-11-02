#!/usr/bin/env bash
# S122's corpus, prepared from head's own games (DEC-344 (1)): the 150000-game pool
# .tuning/s122/corpus/ generated with `--leaf root,qs,pv --quiet-limit 30000`, cut by
# S083's recipe into the training corpus and the fixed held-out set
# adocs/data/S122_screen.sh fits and reads.
#
#   adocs/data/S122_corpus.sh verify     # the pool against its own log
#   adocs/data/S122_corpus.sh filter     # the recipe's band, offline
#   adocs/data/S122_corpus.sh split      # the fixed set, 12000 games
#   adocs/data/S122_corpus.sh sample     # every training game at 46 qs rows
#   adocs/data/S122_corpus.sh k          # K on the new rows
#   adocs/data/S122_corpus.sh record     # prepared.txt, the screen's input
#   adocs/data/S122_corpus.sh all        # the six in order; resumable
#   adocs/data/S122_corpus.sh smoke      # all six on a slice of under 20000 rows
#
# Each stage prints S122-CORPUS-<STAGE>-DONE or -FAILED as its last line. POOL
# defaults to .tuning/s122/corpus, OUT to .tuning/s122/prepared (DEC-235). Nothing
# here writes into POOL.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-11-02 in S122's phase 1, while the pool was
# still generating, before any of it ran on the pool. Nothing below changes once
# `filter` has run on it; a change is a dated section in the step file.
# ============================================================================
#
# S083'S RECIPE, AND EACH DEPARTURE FROM IT, STATED:
#   - The pool: datagen at `ba1cad6` (bench 5482277), `--games 150000 --nodes 100000
#     --threads 12 --seed 2026110203 --leaf root,qs,pv --quiet-limit 30000`, every
#     other setting datagen's default, as S082's. DEPARTURE (DEC-344 (1)): the quiet
#     limit is 30000 and not 1000, so that the rows past the recipe's band are kept and
#     can be read. datagen records a position only when |its search score| < the limit
#     (tools/datagen.cpp, inside_band), and column 3 of every view's row is that
#     search score, White relative (sample_leaf()), so `filter` keeping |column 3| <
#     1000 is the recipe's band exactly, row for row; the rows past it go to
#     pool_past.tsv.* and into no corpus.
#   - A top-up: if the pool did not finish -- a power cut, a crash -- S082's merge is
#     the precedent (DEC-296, adocs/data/S082_pool_merge.py), and it is the
#     coordinator's: `verify` refuses a pool whose log lacks S122-CORPUS-DONE.
#   - The fixed held-out set: S082's rule, 12000 games chosen by
#     tools/corpus_split.py split before anything is sampled, every row of those
#     games, the qs view primary. DEPARTURE: drawn from this pool and not S082's, under
#     seed 20261102, because the fit is on head's games and DEC-344 (1) reads the
#     family on head's games; S082's set stays S082's, read only by DEC-344 (1)'s side
#     reading (a).
#   - The training corpus: every game of the training side at 46 qs rows a game
#     (S083's top rung), `corpus_split.py sample --per-game 46 --seed 1`, the three
#     views of the set named so no corpus row stands on a set position, the rows the
#     capacity counts (S083_corpus.py capacity). No dedupe, S082's setting.
#   - K: fitted on the new rows at the shipped constants, a zero-epoch tuner run
#     against the set, and recorded; adocs/data/S122_screen.sh fits it again per sample
#     at the screen's margin.
#   - The split is S066's by game, which `sample` and the tuner's --holdout keep.
#
# THE SMOKE: `smoke` runs the six stages on the first games of
# .tuning/s122/smoke/pool.tsv.* (the 12000-game sample's pool, head's engine) whose
# root rows stay under 20000, 20 games held out, under OUT=.tuning/s122/phase1/
# corpus_smoke: the pipeline's shape, no reading taken. The main checkout's .tuning is read through TUNING, so a
# worktree runs it with TUNING=/home/max/ws/chesso/.tuning.
#
# MACHINE TIME, inferred from S082's and S083's stages on pools of this size: verify
# and filter about 10 min (three 13 M-row views read twice), split about 10 min, sample
# about 5 min, K one zero-epoch load, about 3 min at 12 threads. About 30 min; it
# shares the machine with nothing heavier than reads.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
# The main checkout's .tuning, which a worktree does not have (DEC-235).
TUNING="${TUNING:-$root/.tuning}"
POOL="${POOL:-$TUNING/s122/corpus}"
OUT="${OUT:-$TUNING/s122/prepared}"
SPLIT_SEED=20261102
VALIDATION_GAMES="${VALIDATION_GAMES:-12000}"
GAMES="${GAMES:-150000}"
PER_GAME=46
BAND=1000
THREADS=12
VIEWS_ALL="root qs pv"

split="$root/tools/corpus_split.py"
capacity_helper="$root/adocs/data/S083_corpus.py"
tuner="${TUNER:-$root/build/tools/tuner}"

say() { echo "$*"; }

verify() {
  local v rows games bad
  [[ -f "$POOL/generate.log" ]] || { say "no $POOL/generate.log"; return 1; }
  grep -q '^S122-CORPUS-DONE$' "$POOL/generate.log" || {
    say "refused: $POOL/generate.log has no S122-CORPUS-DONE: the pool is still generating, or failed; a top-up is S082's merge (DEC-296), the coordinator's"
    return 1
  }
  {
    echo "S122 corpus verify, $(date -Iseconds)"
    grep -E '^(engine|datagen|command)' "$POOL/generate.log"
  } > "$OUT/verify.txt"
  for v in $VIEWS_ALL; do
    [[ -f "$POOL/pool.tsv.$v" ]] || { say "no $POOL/pool.tsv.$v"; return 1; }
    read -r rows games bad < <(awk -F'\t' '{ if (NF != 7) bad++; seen[$5] = 1 }
        END { n = 0; for (g in seen) n++; print NR, n, bad + 0 }' "$POOL/pool.tsv.$v")
    echo "view $v: $rows rows, $games games, $bad rows without seven fields, sha256 $(sha256sum "$POOL/pool.tsv.$v" | cut -d' ' -f1)" | tee -a "$OUT/verify.txt"
    (( rows > 0 )) || { say "refused: $v holds no row"; return 1; }
    (( bad == 0 )) || { say "refused: $v has rows without seven fields"; return 1; }
    (( games <= GAMES )) || { say "refused: $v names $games games, more than the $GAMES played"; return 1; }
  done
}

filter() {
  local v kept past
  [[ -f "$OUT/verify.txt" ]] || { say "refused: run verify first"; return 1; }
  for v in $VIEWS_ALL; do
    if [[ -f "$OUT/pool.tsv.$v.done" ]]; then say "$v filtered before"; continue; fi
    awk -F'\t' -v band="$BAND" -v keep="$OUT/pool.tsv.$v" -v past="$OUT/pool_past.tsv.$v" '
        { v = $3 + 0; if (v < band && v > -band) print > keep; else print > past }' \
        "$POOL/pool.tsv.$v" || { say "the filter failed on $v"; return 1; }
    kept=$(wc -l < "$OUT/pool.tsv.$v")
    past=$(wc -l < "$OUT/pool_past.tsv.$v" 2>/dev/null || echo 0)
    echo "filter $v: |column 3| < $BAND keeps $kept rows, $past past it" | tee -a "$OUT/filter.txt"
    sha256sum "$OUT/pool.tsv.$v" | cut -d' ' -f1 > "$OUT/pool.tsv.$v.done"
  done
}

split_stage() {
  local v
  for v in $VIEWS_ALL; do
    [[ -f "$OUT/pool.tsv.$v.done" ]] || { say "refused: run filter first"; return 1; }
  done
  if [[ -f "$OUT/split.txt" ]] && [[ -f "$OUT/pool.tsv.qs.validation" ]]; then
    say "split before"; cat "$OUT/split.txt"; return 0
  fi
  python3 "$split" split --seed "$SPLIT_SEED" --validation-games "$VALIDATION_GAMES" \
      "$OUT/pool.tsv.root" "$OUT/pool.tsv.qs" "$OUT/pool.tsv.pv" | tee "$OUT/split.txt"
  (( PIPESTATUS[0] == 0 )) || { rm -f "$OUT/split.txt"; say "the split refused"; return 1; }
}

sample() {
  local rows
  [[ -f "$OUT/pool.tsv.qs.train" ]] || { say "refused: run split first"; return 1; }
  if [[ -f "$OUT/corpus.tsv.done" ]]; then say "sampled before"; return 0; fi
  python3 "$capacity_helper" capacity --in "$OUT/pool.tsv.qs.train" \
      --validation "$OUT/pool.tsv.qs.validation" --validation "$OUT/pool.tsv.root.validation" \
      --validation "$OUT/pool.tsv.pv.validation" --per-game "$PER_GAME" | tee "$OUT/capacity.txt"
  (( PIPESTATUS[0] == 0 )) || { say "the capacity refused"; return 1; }
  rows=$(sed -n "s/^capacity: per-game $PER_GAME, \([0-9]*\) rows.*/\1/p" "$OUT/capacity.txt")
  [[ -n "$rows" ]] || { say "no capacity read"; return 1; }
  rm -f "$OUT/corpus.tsv"
  python3 "$split" sample --in "$OUT/pool.tsv.qs.train" \
      --validation "$OUT/pool.tsv.qs.validation" --validation "$OUT/pool.tsv.root.validation" \
      --validation "$OUT/pool.tsv.pv.validation" --rows "$rows" --per-game "$PER_GAME" \
      --seed 1 --out "$OUT/corpus.tsv" | tee "$OUT/sample.txt"
  (( PIPESTATUS[0] == 0 )) || { say "the sample refused"; return 1; }
  sha256sum "$OUT/corpus.tsv" | cut -d' ' -f1 > "$OUT/corpus.tsv.done"
}

k_stage() {
  [[ -f "$OUT/corpus.tsv.done" ]] || { say "refused: run sample first"; return 1; }
  if [[ -s "$OUT/k.txt" ]]; then say "K before: $(cat "$OUT/k.txt")"; return 0; fi
  [[ -x "$tuner" ]] || { say "no tuner at $tuner"; return 1; }
  # A free set the tuner accepts, S122's groups: K is fitted at the shipped
  # constants before any step, whatever the set (S101_screen.sh's K probes).
  "$tuner" --data "$OUT/corpus.tsv" --holdout "$OUT/pool.tsv.qs.validation" \
      --out "$OUT/k.hpp" --epochs 0 --only ks_checks,ks_weak,ks_defenders,ks_storm,ks_curve \
      --threads "$THREADS" > "$OUT/k.log" 2>&1 ||
    { tail -5 "$OUT/k.log"; say "the K run failed"; return 1; }
  sed -n 's/^fitted K = \([0-9.]*\)$/\1/p' "$OUT/k.log" > "$OUT/k.txt"
  [[ -s "$OUT/k.txt" ]] || { say "the K run printed no K"; return 1; }
  grep -E '^trace:' "$OUT/k.log"
  say "K $(cat "$OUT/k.txt") at the shipped constants and margin"
}

record() {
  local v
  [[ -s "$OUT/k.txt" ]] || { say "refused: run k first"; return 1; }
  {
    echo "S122 prepared corpus, $(date -Iseconds)"
    echo "corpus $OUT/corpus.tsv"
    echo "sha256 $(sha256sum "$OUT/corpus.tsv" | cut -d' ' -f1)"
    echo "rows $(wc -l < "$OUT/corpus.tsv")"
    echo "games $(awk -F'\t' '{ seen[$5] = 1 } END { n = 0; for (g in seen) n++; print n }' "$OUT/corpus.tsv")"
    for v in qs root pv; do
      echo "view $v $OUT/pool.tsv.$v.validation $(sha256sum "$OUT/pool.tsv.$v.validation" | cut -d' ' -f1)"
    done
    echo "k $(cat "$OUT/k.txt")"
    echo "pool $POOL"
    echo "recipe filter |column 3| < $BAND, split seed $SPLIT_SEED, $VALIDATION_GAMES games held out, $PER_GAME qs rows a game, sample seed 1"
    echo "S122-PREPARED"
  } > "$OUT/prepared.txt"
  cat "$OUT/prepared.txt"
}

all_stages() {
  verify && filter && split_stage && sample && k_stage && record
}

# The six stages on a slice of the 12000-game sample's pool, under 20000 rows a view.
smoke() {
  local src="$TUNING/s122/smoke" slice games
  OUT="$TUNING/s122/phase1/corpus_smoke"
  rm -rf "$OUT"; mkdir -p "$OUT/pool" || return 1
  slice="$OUT/pool"
  # The first games whose root rows stay under 20000; every view cut to them.
  games=$(awk -F'\t' '{ if (!($5 in seen)) { if (NR > 19000) exit; seen[$5] = 1; print $5 } }' \
            "$src/pool.tsv.root" | sort -n | tr '\n' ' ')
  for v in $VIEWS_ALL; do
    [[ -f "$src/pool.tsv.$v" ]] || { say "no $src/pool.tsv.$v"; return 1; }
    awk -F'\t' -v keep=" $games" 'index(keep, " " $5 " ") > 0' "$src/pool.tsv.$v" > "$slice/pool.tsv.$v"
  done
  printf 'smoke slice of %s\nS122-CORPUS-DONE\n' "$src" > "$slice/generate.log"
  POOL="$slice"; VALIDATION_GAMES=20; GAMES=12000
  all_stages
}

mkdir -p "$OUT" || { echo "cannot create $OUT"; echo "S122-CORPUS-FAILED"; exit 1; }

case "$stage" in
  verify) if verify; then echo "S122-CORPUS-VERIFY-DONE"; else echo "S122-CORPUS-VERIFY-FAILED"; exit 1; fi ;;
  filter) if filter; then echo "S122-CORPUS-FILTER-DONE"; else echo "S122-CORPUS-FILTER-FAILED"; exit 1; fi ;;
  split) if split_stage; then echo "S122-CORPUS-SPLIT-DONE"; else echo "S122-CORPUS-SPLIT-FAILED"; exit 1; fi ;;
  sample) if sample; then echo "S122-CORPUS-SAMPLE-DONE"; else echo "S122-CORPUS-SAMPLE-FAILED"; exit 1; fi ;;
  k) if k_stage; then echo "S122-CORPUS-K-DONE"; else echo "S122-CORPUS-K-FAILED"; exit 1; fi ;;
  record) if record; then echo "S122-CORPUS-RECORD-DONE"; else echo "S122-CORPUS-RECORD-FAILED"; exit 1; fi ;;
  all) if all_stages; then echo "S122-CORPUS-ALL-DONE"; else echo "S122-CORPUS-ALL-FAILED"; exit 1; fi ;;
  smoke) if smoke; then echo "S122-CORPUS-SMOKE-DONE"; else echo "S122-CORPUS-SMOKE-FAILED"; exit 1; fi ;;
  *) echo "usage: $0 verify | filter | split | sample | k | record | all | smoke" >&2; exit 2 ;;
esac
