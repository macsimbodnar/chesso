#!/usr/bin/env bash
# S266's research pass, 2026-11-05: every corpus figure of the step file's section of that date.
# Counts, in-sample and across-halves label readings and a collinearity sample: no fit of the
# engine, no match, no timing. Run from a checkout whose build/ has chesso_engine_trace built
# (DEV_MANUAL.md) at a tree whose bench reads 5482277. OUT holds everything; adocs/data/ takes the
# reading, adocs/data/S266_census.txt, which `read` writes.
#
#   adocs/data/S266_census.sh build    # adocs/data/S266_census.cpp against build/'s trace library,
#                                      # and the planted build (-DS266_PLANT)
#   adocs/data/S266_census.sh corpus   # S122's regenerated prepared corpus: counts, collinearity on
#                                      # every 40th row, labels at its own K 1.2302 and again at K
#                                      # refitted at S124's cells to a fixed point (--refit-k)
#   adocs/data/S266_census.sh views    # its three held-out views, counts only, and the rows the
#                                      # corpus's C4x | F12 fit would take to zero there
#   adocs/data/S266_census.sh s083     # S083's corpus, counts only
#   adocs/data/S266_census.sh plant    # the planted build on 20000 corpus rows: the walk must differ
#   adocs/data/S266_census.sh read     # writes adocs/data/S266_census.txt
#   adocs/data/S266_census.sh all
#
# Each stage ends S266-<STAGE>-DONE or S266-<STAGE>-FAILED and exits non-zero on a failure. `read`
# refuses a missing file. Everything runs under `nice -n 19`, one process, at most two threads.
#
# Labels. The result column is read for S122's prepared training corpus at its own K, 1.2302
# (.tuning/s122/prepared/prepared.txt, "k 1.2302"), and for nothing else. The held-out views and
# S083's corpus are read for the FEN and game columns: their labels stay unread. S083's corpus was
# written by the old two-fold rule (DEC-349) and is read for comparison only.
set -u

root="$(git rev-parse --show-toplevel)"
main=/home/max/ws/chesso
OUT="${OUT:-$root/.tuning/s266}"
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
  nice -n 19 g++ "${flags[@]}" "$root/adocs/data/S266_census.cpp" \
    "$root/build/src/libchesso_engine_trace.a" -lpthread -o "$OUT/census" || return 1
  nice -n 19 g++ "${flags[@]}" -DS266_PLANT "$root/adocs/data/S266_census.cpp" \
    "$root/build/src/libchesso_engine_trace.a" -lpthread -o "$OUT/census_plant" || return 1
}

stage_corpus() {
  sha_is "$CORPUS" "$CORPUS_SHA" || return 1
  grep -q "^k $K\$" "$PREP/prepared.txt" || { say "K is not $K"; return 1; }
  nice -n 19 "$OUT/census" "$CORPUS" --k "$K" --refit-k --r2 40 --threads 2 > "$OUT/c_corpus.txt" \
    2> "$OUT/c_corpus.err" || return 1
  grep -q S266-CENSUS-OK "$OUT/c_corpus.txt" || return 1
}

# The proposal's C, C4x | F12 fitted on all of the corpus's rows at the corpus's K, the first of the
# two readings, as --c reads it.
c4_fit() {
  awk '/^C4x \| F12 +in sample/ {getline; for (i = 1; i < NF; ++i) { if ($i == "c0") c0 = $(i + 1);
       if ($i == "cP") cp = $(i + 1); if ($i == "cF") cf = $(i + 1); if ($i == "cE") ce = $(i + 1) }
       print c0 "," cp "," cf "," ce; exit}' "$OUT/c_corpus.txt"
}

stage_views() {
  local v c
  c="$(c4_fit)"
  [ -n "$c" ] || { say "no C4x | F12 fit in $OUT/c_corpus.txt: run corpus first"; return 1; }
  for v in qs root pv; do
    sha_is "$PREP/pool.tsv.$v.validation" "${VIEW_SHA[$v]}" || return 1
    nice -n 19 "$OUT/census" "$PREP/pool.tsv.$v.validation" --c "$c" > "$OUT/c_view_$v.txt" \
      2> "$OUT/c_view_$v.err" || return 1
    grep -q S266-CENSUS-OK "$OUT/c_view_$v.txt" || return 1
  done
}

stage_s083() {
  sha_is "$S083" "$S083_SHA" || return 1
  nice -n 19 "$OUT/census" "$S083" > "$OUT/c_s083.txt" 2> "$OUT/c_s083.err" || return 1
  grep -q S266-CENSUS-OK "$OUT/c_s083.txt" || return 1
}

stage_plant() {
  nice -n 19 "$OUT/census_plant" "$CORPUS" --limit 20000 > "$OUT/c_plant.txt" 2> "$OUT/c_plant.err" || return 1
  grep -q S266-CENSUS-OK "$OUT/c_plant.txt" || return 1
  # Red is the pass: a planted flank the walk does not see would make the walk worthless.
  if grep -q "^walk: 0 rows" "$OUT/c_plant.txt"; then say "the plant left the walk green"; return 1; fi
}

stage_read() {
  local f v
  for f in c_corpus.txt c_view_qs.txt c_view_root.txt c_view_pv.txt c_s083.txt c_plant.txt; do
    [ -s "$OUT/$f" ] || { say "missing $OUT/$f"; return 1; }
  done
  {
    say "# S266 research pass, 2026-11-05: adocs/data/S266_census.sh read, at $(git -C "$root" rev-parse --short HEAD)"
    say "# corpus $CORPUS sha256 $CORPUS_SHA, labels at K $K; views and S083 counts only"
    say "# definitions: adocs/data/S266_census.cpp header"
    say
    say "## S122's regenerated prepared corpus"
    cat "$OUT/c_corpus.txt"
    for v in qs root pv; do
      say
      say "## held-out view $v, counts only; --c is the corpus's C4x | F12 fit, $(c4_fit)"
      cat "$OUT/c_view_$v.txt"
    done
    say
    say "## S083's corpus, counts only (old two-fold rule, DEC-349)"
    cat "$OUT/c_s083.txt"
    say
    say "## the plant: file d counted on neither flank, 20000 corpus rows; the walk must differ"
    grep "^walk:" "$OUT/c_plant.txt"
  } > "$root/adocs/data/S266_census.txt" || return 1
}

run() {
  local s="$1"
  if "stage_$s"; then say "S266-${s^^}-DONE"; else say "S266-${s^^}-FAILED"; exit 1; fi
}

case "${1:-}" in
  build | corpus | views | s083 | plant | read) run "$1" ;;
  all) for s in build corpus views s083 plant read; do run "$s"; done ;;
  *) say "usage: $0 build|corpus|views|s083|plant|read|all"; exit 2 ;;
esac
