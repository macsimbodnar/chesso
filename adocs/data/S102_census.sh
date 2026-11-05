#!/usr/bin/env bash
# S102's research pass, 2026-11-04: every corpus figure of the step file's section of that date.
# Counts and in-sample label readings only: no fit, no match, no timing. Run from a checkout whose
# build/ has chesso_engine_trace built (DEV_MANUAL.md) at a tree whose bench reads 5482277. OUT
# holds everything; adocs/data/ takes the reading, adocs/data/S102_census.txt, which `read` writes.
#
#   adocs/data/S102_census.sh build    # adocs/data/S102_census.cpp against build/'s trace library,
#                                      # and the planted build (-DS102_PLANT)
#   adocs/data/S102_census.sh corpus   # S122's regenerated prepared corpus: counts, collinearity
#                                      # on every 40th row, labels at its own K 1.2302
#   adocs/data/S102_census.sh views    # its three held-out views, counts only
#   adocs/data/S102_census.sh s083     # S083's corpus, counts only
#   adocs/data/S102_census.sh plant    # the planted build on 20000 corpus rows: the walk must differ
#   adocs/data/S102_census.sh read     # writes adocs/data/S102_census.txt
#   adocs/data/S102_census.sh all
#
# Each stage ends S102-<STAGE>-DONE or S102-<STAGE>-FAILED and exits non-zero on a failure. `read`
# refuses a missing file. Everything runs under `nice -n 19`, one process, one thread.
#
# Labels. The result column is read for S122's prepared training corpus at its own K, 1.2302
# (.tuning/s122/prepared/k.txt, "k 1.2302" in its prepared.txt), and for nothing else. The held-out
# views and S083's corpus are read for the FEN and game columns: their labels stay unread. S083's
# corpus was written by the old two-fold rule (DEC-349) and is read for comparison only.
set -u

root="$(git rev-parse --show-toplevel)"
main=/home/max/ws/chesso
OUT="${OUT:-$root/.tuning/s102}"
PREP="$main/.tuning/s122/prepared"
CORPUS="$PREP/corpus.tsv"
CORPUS_SHA=04c596de726b5a2886df4e7e41c08fdd958529c669ba98ee69befd6a54676509
K=1.2302
S083="$main/.tuning/s083/curve/s0/cand_6226944_s1.tsv"
S083_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
declare -A VIEW_SHA=(
  [qs]=faef5cd415abfb999c77f9754d0d1fbcd81fa572d04680b2b73e1bb9508936eb
  [root]=c2182a342c447b2b51310877fd981c96810a01c225335384ce753a5879aab28d
  [pv]=9d7d4df3657f2ab28a7e34cd536217e796a13d3adeece152bb3076575da86a36
)
mkdir -p "$OUT"

say() { printf '%s\n' "$*"; }
sha_is() { [ "$(sha256sum "$1" | cut -d' ' -f1)" = "$2" ] || { say "sha256 of $1 is not $2"; return 1; }; }

stage_build() {
  local flags=(-std=c++20 -O2 -march=native -DNDEBUG -DCHESSO_TRACE -I "$root/src" -I "$root/tools")
  nice -n 19 g++ "${flags[@]}" "$root/adocs/data/S102_census.cpp" \
    "$root/build/src/libchesso_engine_trace.a" -lpthread -o "$OUT/census" || return 1
  nice -n 19 g++ "${flags[@]}" -DS102_PLANT "$root/adocs/data/S102_census.cpp" \
    "$root/build/src/libchesso_engine_trace.a" -lpthread -o "$OUT/census_plant" || return 1
}

stage_corpus() {
  sha_is "$CORPUS" "$CORPUS_SHA" || return 1
  grep -q "^k $K\$" "$PREP/prepared.txt" || { say "K is not $K"; return 1; }
  nice -n 19 "$OUT/census" "$CORPUS" --k "$K" --r2 40 > "$OUT/c_corpus.txt" 2> "$OUT/c_corpus.err" || return 1
  grep -q S102-CENSUS-OK "$OUT/c_corpus.txt" || return 1
}

stage_views() {
  local v
  for v in qs root pv; do
    sha_is "$PREP/pool.tsv.$v.validation" "${VIEW_SHA[$v]}" || return 1
    nice -n 19 "$OUT/census" "$PREP/pool.tsv.$v.validation" > "$OUT/c_view_$v.txt" 2> "$OUT/c_view_$v.err" || return 1
    grep -q S102-CENSUS-OK "$OUT/c_view_$v.txt" || return 1
  done
}

stage_s083() {
  sha_is "$S083" "$S083_SHA" || return 1
  nice -n 19 "$OUT/census" "$S083" > "$OUT/c_s083.txt" 2> "$OUT/c_s083.err" || return 1
  grep -q S102-CENSUS-OK "$OUT/c_s083.txt" || return 1
}

stage_plant() {
  nice -n 19 "$OUT/census_plant" "$CORPUS" --limit 20000 > "$OUT/c_plant.txt" 2> "$OUT/c_plant.err" || return 1
  grep -q S102-CENSUS-OK "$OUT/c_plant.txt" || return 1
  # Red is the pass: a planted span the walk does not see would make the walk worthless.
  if grep -q "^walk: 0 side-counts" "$OUT/c_plant.txt"; then say "the plant left the walk green"; return 1; fi
}

stage_read() {
  local f
  for f in c_corpus.txt c_view_qs.txt c_view_root.txt c_view_pv.txt c_s083.txt c_plant.txt; do
    [ -s "$OUT/$f" ] || { say "missing $OUT/$f"; return 1; }
  done
  {
    say "# S102 research pass, 2026-11-04: adocs/data/S102_census.sh read, at $(git -C "$root" rev-parse --short HEAD)"
    say "# corpus $CORPUS sha256 $CORPUS_SHA, labels at K $K; views and S083 counts only"
    say "# variant definitions: adocs/data/S102_census.cpp header"
    say
    say "## S122's regenerated prepared corpus"
    cat "$OUT/c_corpus.txt"
    for v in qs root pv; do
      say
      say "## held-out view $v, counts only"
      cat "$OUT/c_view_$v.txt"
    done
    say
    say "## S083's corpus, counts only (old two-fold rule, DEC-349)"
    cat "$OUT/c_s083.txt"
    say
    say "## the plant: the enemy span without its neighbour files, 20000 corpus rows; the walk must differ"
    grep "^walk:" "$OUT/c_plant.txt"
  } > "$root/adocs/data/S102_census.txt" || return 1
}

run() {
  local s="$1"
  if "stage_$s"; then say "S102-${s^^}-DONE"; else say "S102-${s^^}-FAILED"; exit 1; fi
}

case "${1:-}" in
  build | corpus | views | s083 | plant | read) run "$1" ;;
  all) for s in build corpus views s083 plant read; do run "$s"; done ;;
  *) say "usage: $0 build|corpus|views|s083|plant|read|all"; exit 2 ;;
esac
