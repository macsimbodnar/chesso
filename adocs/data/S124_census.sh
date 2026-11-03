#!/usr/bin/env bash
# S124's research pass, 2026-11-03: every figure of the step file's section of that date.
# Counts and tablebase probes only: no fit, no match, no timing. Run from a checkout whose
# build/ is built (DEV_MANUAL.md) at a tree whose bench reads 5482277. OUT holds everything;
# adocs/data/ takes the reading, adocs/data/S124_census.txt, which `read` writes.
#
#   adocs/data/S124_census.sh build    # adocs/data/S124_census.cpp against build/'s trace library
#   adocs/data/S124_census.sh corpus   # S122's prepared corpus, labels at its K, its <= 5-man rows dumped
#   adocs/data/S124_census.sh views    # S082's fixed views of that corpus, counts only
#   adocs/data/S124_census.sh s083     # S083's corpus, counts only
#   adocs/data/S124_census.sh pool     # S122's raw qs pool, the rows past the 1000 filter, <= 5-man rows dumped
#   adocs/data/S124_census.sh tb       # adocs/data/S124_tb_probe.py over both dumps
#   adocs/data/S124_census.sh conversion # adocs/data/S124_conversion.py: the games that reached a lone-king ending,
#                                        # by their root rows, under the fifty-move rule; writes the two-fold list
#   adocs/data/S124_census.sh twofold  # adocs/data/S124_twofold_check.py over that list: an indication
#   adocs/data/S124_census.sh read     # writes adocs/data/S124_census.txt
#   adocs/data/S124_census.sh all
#
# Each stage ends S124-<STAGE>-DONE or S124-<STAGE>-FAILED and exits non-zero on a failure.
# `read` reads what the stages leave and refuses a missing file. Everything runs under
# `nice -n 19`, one process a stage, one thread each; at most two stages at once.
#
# Labels. The result column is read for S122's prepared training corpus at its own K,
# 0.7433 (.tuning/s122/prepared/k.txt), for the raw qs pool's rows of training games only,
# into the tablebase dump, and for the raw root pool's training games in `conversion`. S082's fixed views are read for the FEN, phase, game and ply
# columns: their labels stay unread by anything that chooses a form. The held-out games' ids
# are read from the qs view's game column alone. The tablebase probes the training side's
# rows only.
set -u

root="$(git rev-parse --show-toplevel)"
main=/home/max/ws/chesso
OUT="${OUT:-$root/.tuning/s124}"
PREP="$main/.tuning/s122/prepared"
CORPUS="$PREP/corpus.tsv"
CORPUS_SHA=cd50f961bc25919a90300e308f216f293f8f51c56487b63fef99bd2aa4ed75fd
K=0.7433
S083="$main/.tuning/s083/curve/s0/cand_6226944_s1.tsv"
S083_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
POOL="$main/.tuning/s122/corpus/pool.tsv.qs"
POOL_SHA=1df46c1a9e78e527faf4493753f3b9bfaf91d271fd310058cd7bf9d1bbf524ea
ROOT_SHA=895d006a5356d2aa4f17990aa1298a22b23974dfea69c9a8630afdfb4703dde7
PY="$HOME/.venv/chess/bin/python"
mkdir -p "$OUT"

say() { printf '%s\n' "$*"; }
sha_is() { [ "$(sha256sum "$1" | cut -d' ' -f1)" = "$2" ] || { say "sha256 of $1 is not $2"; return 1; }; }

stage_build() {
  nice -n 19 g++ -std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I "$root/src" \
    -I "$root/tools" "$root/adocs/data/S124_census.cpp" \
    "$root/build/src/libchesso_engine_trace.a" -lpthread -o "$OUT/census" || return 1
}

stage_corpus() {
  sha_is "$CORPUS" "$CORPUS_SHA" || return 1
  nice -n 19 "$OUT/census" "$CORPUS" --k "$K" --dump "$OUT/dump_corpus.tsv" \
    > "$OUT/c_corpus.txt" || return 1
  grep -q S124-CENSUS-OK "$OUT/c_corpus.txt" || return 1
}

stage_views() {
  local v
  for v in qs root pv; do
    grep -q "pool.tsv.$v.validation" "$PREP/prepared.txt" || { say "view $v not named"; return 1; }
    nice -n 19 "$OUT/census" "$PREP/pool.tsv.$v.validation" > "$OUT/c_view_$v.txt" || return 1
    grep -q S124-CENSUS-OK "$OUT/c_view_$v.txt" || return 1
  done
}

stage_s083() {
  sha_is "$S083" "$S083_SHA" || return 1
  nice -n 19 "$OUT/census" "$S083" > "$OUT/c_s083.txt" || return 1
  grep -q S124-CENSUS-OK "$OUT/c_s083.txt" || return 1
}

stage_pool() {
  sha_is "$POOL" "$POOL_SHA" || return 1
  cut -f5 "$PREP/pool.tsv.qs.validation" | sort -un > "$OUT/heldout_games.txt" || return 1
  [ "$(wc -l < "$OUT/heldout_games.txt")" = 12000 ] || { say "held-out games not 12000"; return 1; }
  nice -n 19 "$OUT/census" "$POOL" --pool --k "$K" --exclude "$OUT/heldout_games.txt" \
    --dump "$OUT/dump_pool.tsv" > "$OUT/c_pool.txt" || return 1
  grep -q S124-CENSUS-OK "$OUT/c_pool.txt" || return 1
}

stage_tb() {
  [ -f "$OUT/dump_corpus.tsv" ] && [ -f "$OUT/dump_pool.tsv" ] || { say "a dump is missing"; return 1; }
  nice -n 19 "$PY" "$root/adocs/data/S124_tb_probe.py" --k "$K" "$OUT/dump_corpus.tsv" > "$OUT/tb_corpus.txt" || return 1
  nice -n 19 "$PY" "$root/adocs/data/S124_tb_probe.py" --k "$K" "$OUT/dump_pool.tsv" > "$OUT/tb_pool.txt" || return 1
  grep -q S124-TB-OK "$OUT/tb_corpus.txt" && grep -q S124-TB-OK "$OUT/tb_pool.txt" || return 1
}

stage_conversion() {
  sha_is "$main/.tuning/s122/corpus/pool.tsv.root" "$ROOT_SHA" || return 1
  [ "$(wc -l < "$OUT/heldout_games.txt")" = 12000 ] || { say "run pool first: the held-out list"; return 1; }
  nice -n 19 "$PY" "$root/adocs/data/S124_conversion.py" "$main/.tuning/s122/corpus/pool.tsv.root" \
    "$OUT/heldout_games.txt" "$OUT/twofold_list.tsv" > "$OUT/conversion.txt" || return 1
  grep -q S124-CONVERSION-OK "$OUT/conversion.txt" || return 1
}

stage_twofold() {
  [ -f "$OUT/twofold_list.tsv" ] || { say "run conversion first: the two-fold list"; return 1; }
  sha_is "$main/.tuning/s122/corpus/pool.tsv.root" "$ROOT_SHA" || return 1
  nice -n 19 "$PY" "$root/adocs/data/S124_twofold_check.py" "$OUT/twofold_list.tsv" \
    "$main/.tuning/s122/corpus/pool.tsv.root" "$root/build/src/chesso" > "$OUT/twofold.txt" || return 1
  grep -q S124-TWOFOLD-OK "$OUT/twofold.txt" || return 1
}

stage_read() {
  local f out="$root/adocs/data/S124_census.txt"
  for f in c_corpus c_view_qs c_view_root c_view_pv c_s083 c_pool tb_corpus tb_pool conversion twofold; do
    [ -f "$OUT/$f.txt" ] || { say "missing $OUT/$f.txt"; return 1; }
  done
  {
    say "# S124's research pass, 2026-11-03: adocs/data/S124_census.sh's stages, verbatim."
    say "# Counts, labels at K $K on the training corpus only, and Syzygy probes on training rows."
    for f in c_corpus c_view_qs c_view_root c_view_pv c_s083 c_pool tb_corpus tb_pool conversion twofold; do
      say ""
      say "################ $f"
      sed "s|$main/||g; s|$root/||g" "$OUT/$f.txt"
    done
  } > "$out" || return 1
}

run() {
  local stage="$1"
  local upper
  upper="$(printf '%s' "$stage" | tr '[:lower:]' '[:upper:]')"
  if "stage_$stage"; then
    say "S124-$upper-DONE"
  else
    say "S124-$upper-FAILED"
    exit 1
  fi
}

case "${1:-}" in
  build | corpus | views | s083 | pool | tb | conversion | twofold | read) run "$1" ;;
  all) for s in build corpus views s083 pool tb conversion twofold read; do run "$s"; done ;;
  *) say "S124_census.sh build|corpus|views|s083|pool|tb|conversion|twofold|read|all"; exit 2 ;;
esac
