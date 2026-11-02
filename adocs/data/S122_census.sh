#!/usr/bin/env bash
# S122's research pass, 2026-11-02: every figure of the step file's sections of that date.
# Counts and instructions only: no fit, no match, no timing. Run from a clean checkout of
# ba1cad6 with build/ built (DEV_MANUAL.md). OUT holds everything; adocs/data/ takes the
# reading, adocs/data/S122_census.txt, which `read` writes.
#
#   adocs/data/S122_census.sh head       # head's spread and truncation, the tools as shipped
#   adocs/data/S122_census.sh census     # S122_census.cpp over the corpus and S082's views
#   adocs/data/S122_census.sh prototype  # a private tree at ba1cad6, the patch applied
#   adocs/data/S122_census.sh cost       # the sink rows: instructions and node counts
#   adocs/data/S122_census.sh margins    # the margin rows, real builds, nodes moving
#   adocs/data/S122_census.sh search     # the search census: bench's calls, by attacker
#   adocs/data/S122_census.sh probes     # eval_spread and truncation_scan at forced weights
#   adocs/data/S122_census.sh sample     # head's 12000 games, split at the filter, censused
#   adocs/data/S122_census.sh scans      # DEC-344's Context and (4), flat weights over labels
#   adocs/data/S122_census.sh plants     # what each tool's identity check catches
#   adocs/data/S122_census.sh read       # writes adocs/data/S122_census.txt
#   adocs/data/S122_census.sh all
#
# Each stage ends S122-<STAGE>-DONE or S122-<STAGE>-FAILED, the reason above it, and exits
# non-zero on a failure. `read` reads what those stages leave and nothing else, and it refuses
# a missing file.
#
# K_S083 0.7119 is the zero-epoch K of S101's screen on S083's corpus at c2f2025
# (.tuning/s101/k/full.hpp); head's evaluate asks 0.7147 of those rows, which `scans` reads as
# well. K_SAMPLE 0.7407 is the K the labels ask of the sample at head's evaluate, which `scans`
# checks. Inputs to the label readings alone. Result columns are read for S083's training
# corpus and the sample: S082's fixed set keeps its labels unread by what chooses a form.
set -u

root="$(git rev-parse --show-toplevel)"
main=/home/max/ws/chesso
OUT="${OUT:-$main/.tuning/s122}"
BASE=ba1cad6
CORPUS="$main/.tuning/s083/curve/s0/cand_6226944_s1.tsv"
CORPUS_SHA=2e0e3de9a4beb0e42f13062b748c7dcb86111a74eb0ea7d7f051351f80872c53
POOL="$main/.tuning/s082/pool.tsv"
K_S083=0.7119
K_S083_OWN=0.7147
K_SAMPLE=0.7407
PROBES="32 48 64 96"
mkdir -p "$OUT"

say() { printf '%s\n' "$*"; }

census_build() {
  g++ -std=c++20 -O2 -march=native -DNDEBUG -I "$root/adocs/data" -I "$root/src" \
    "$root/adocs/data/S122_census.cpp" "$root/build/src/libchesso_engine.a" -lpthread \
    -o "$OUT/census" || return 1
}

stage_head() {
  "$root/build/tools/eval_spread" --data "$CORPUS" > "$OUT/spread_head_s083.txt" || return 1
  "$root/build/tools/eval_spread" --data "$POOL.qs.validation" > "$OUT/spread_head_s082qs.txt" || return 1
  "$root/build/tools/truncation_scan" --data "$CORPUS" --min 2.8 --report 5 > "$OUT/trunc_head_s083.txt" || return 1
}

stage_census() {
  [ "$(sha256sum "$CORPUS" | cut -d' ' -f1)" = "$CORPUS_SHA" ] || { say "corpus sha"; return 1; }
  census_build || return 1
  local pids=() names=() i rc=0
  start() {
    "$OUT/census" "$1" ${3:+--k "$3"} > "$OUT/$2.txt" &
    pids+=($!)
    names+=("$2")
  }
  start "$CORPUS" c_s083 "$K_S083"
  start "$POOL.qs.validation" c_s082qs
  start "$POOL.root.validation" c_s082root
  start "$POOL.pv.validation" c_s082pv
  start "$POOL.qs.train" c_pool_qs_train
  start "$POOL.root.train" c_pool_root_train
  start "$POOL.pv.train" c_pool_pv_train
  for i in "${!pids[@]}"; do
    wait "${pids[$i]}" || { say "census failed: ${names[$i]}"; rc=1; }
  done
  return $rc
}

stage_prototype() {
  local p="$OUT/proto"
  [ -e "$p" ] && { say "$p exists; a private tree is not rebuilt over itself"; return 1; }
  mkdir -p "$p" "$p/hdr" "$p/b"
  git -C "$root" archive "$BASE" | tar -x -C "$p" || return 1
  rm -rf "$p/adocs"
  cp -r "$root/tests/doctest/." "$p/tests/doctest/"
  cp "$root/adocs/data/S122_census_core.hpp" "$root/adocs/data/S101_threat_core.hpp" "$p/hdr/"
  (cd "$p" && patch -p1 --quiet < "$root/adocs/data/S122_research_prototype.patch") || return 1
}

# One build of the private tree: NAME "DEFINES" TARGETS...
build_variant() {
  local name=$1 defs=$2 d="$OUT/proto/b/$1"
  shift 2
  [ -x "$d/src/chesso" ] && [ "$*" = chesso ] && return 0
  cmake -S "$OUT/proto" -B "$d" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    "-DCMAKE_CXX_FLAGS=-I$OUT/proto/hdr $defs" > "$d.cmake.log" 2>&1 || { say "cmake failed: $name"; return 1; }
  cmake --build "$d" -j12 --target "$@" > "$d.build.log" 2>&1 || { say "build failed: $name"; return 1; }
}

# A row: instructions and nodes on `bench` and `bench 16`, perf_counters' instructions alone.
count_row() {
  local name=$1 exe="$OUT/proto/b/$1/src/chesso" pc="$root/build/tools/perf_counters" f="$OUT/counters.tmp"
  local n1 i1 n2 i2
  rm -f "$f"
  n1=$("$pc" -o "$f" "$exe" bench 2>/dev/null | tail -1 | awk '{print $1}'); i1=$(awk '{print $3}' "$f")
  rm -f "$f"
  n2=$("$pc" -o "$f" "$exe" bench 16 2>/dev/null | tail -1 | awk '{print $1}'); i2=$(awk '{print $3}' "$f")
  rm -f "$f"
  [ -n "$n1" ] && [ -n "$i1" ] && [ -n "$n2" ] && [ -n "$i2" ] || { say "no count: $name"; return 1; }
  printf '%-10s %10s %14s %10s %14s\n' "$name" "$n1" "$i1" "$n2" "$i2"
}

COST_ROWS="v0:0 v64:64 v1:1 v128:128 v2:2 v4:4 v8:8 v16:16 v256:256 v3:3 v5:5 v9:9 v31:31 v63:63"

stage_cost() {
  local spec name bits rows="$OUT/cost_rows.txt"
  : > "$rows"
  for spec in $COST_ROWS; do
    name=${spec%%:*}; bits=${spec#*:}
    build_variant "$name" "-DS122_VARIANT=$bits" chesso || return 1
  done
  build_variant twice0 "-DS122_VARIANT=0 -DS122_TWICE" chesso || return 1
  build_variant twice31 "-DS122_VARIANT=31 -DS122_TWICE" chesso || return 1
  for spec in $COST_ROWS twice0:x twice31:x; do
    count_row "${spec%%:*}" >> "$rows" || return 1
  done
  # A sink moves no node: a row that reads otherwise is no price.
  [ "$(awk '{print $2, $4}' "$rows" | sort -u | wc -l)" = 1 ] || { say "a sink row moved the nodes"; return 1; }
}

stage_margins() {
  local m
  : > "$OUT/margin_rows.txt"
  for m in 200 250 300 400 500 600 800 32000; do
    build_variant "m$m" "-DS122_VARIANT=0 -DS122_MARGIN_VALUE=$m" chesso || return 1
    count_row "m$m" >> "$OUT/margin_rows.txt" || return 1
  done
  for m in 250 400 600 32000; do
    build_variant "a$m" "-DS122_VARIANT=31 -DS122_MARGIN_VALUE=$m" chesso || return 1
    count_row "a$m" >> "$OUT/margin_rows.txt" || return 1
  done
}

stage_search() {
  build_variant census "-DS122_VARIANT=0 -DS122_CENSUS" chesso || return 1
  "$OUT/proto/b/census/src/chesso" bench > "$OUT/search_bench.out" 2> "$OUT/search_census.txt" || return 1
}

stage_probes() {
  local w m name bench
  : > "$OUT/probe_rows.txt"
  for w in $PROBES; do
    for m in 184 300; do
      name="p${w}m$m"
      build_variant "$name" "-DS122_SCORE_W=$w -DS122_MARGIN_VALUE=$m" chesso eval_spread truncation_scan || return 1
      "$OUT/proto/b/$name/tools/eval_spread" --data "$CORPUS" > "$OUT/$name.spread" 2>&1 || { say "eval_spread: $name"; return 1; }
      "$OUT/proto/b/$name/tools/truncation_scan" --data "$CORPUS" --min 3.3 --report 3 > "$OUT/$name.trunc" 2>&1 || { say "truncation_scan: $name"; return 1; }
      bench=$("$OUT/proto/b/$name/src/chesso" bench 2>/dev/null | tail -1 | awk '{print $1}')
      printf '%-10s bench %s\n' "$name" "$bench" >> "$OUT/probe_rows.txt"
    done
  done
}

stage_sample() {
  # Head's engine, its filter opened so that the rows past 1000 can be counted. No corpus:
  # 3000 games with all three views, then 9000 more with qs, kept under OUT and in no tree.
  # The two runs' rates and start times are in their logs. Generated once: a stage that finds
  # them reads them and plays nothing.
  local s="$OUT/smoke"
  mkdir -p "$s/b"
  if [ ! -s "$s/pool.tsv.qs" ]; then
    { date -Is
      "$root/build/tools/datagen" --out "$s/pool.tsv" --leaf root,qs,pv --games 3000 --nodes 100000 \
        --threads 12 --seed 2026110201 --quiet-limit 30000 || return 1
      date -Is; } > "$s/run.log" 2>&1
  fi
  if [ ! -s "$s/b/pool.tsv" ]; then
    { date -Is
      "$root/build/tools/datagen" --out "$s/b/pool.tsv" --leaf qs --games 9000 --nodes 100000 \
        --threads 12 --seed 2026110202 --quiet-limit 30000 || return 1
      date -Is; } > "$s/b/run.log" 2>&1
  fi
  # Split at today's filter: under 1000 the rows the filter keeps, at or past it the rows it
  # drops. Game numbers repeat between the two runs, so a game count over the two is a lower
  # bound; `scans` keeps the two apart with --run-a.
  rm -f "$s/qs_inside.tsv" "$s/qs_past.tsv"
  cat "$s/pool.tsv.qs" "$s/b/pool.tsv" | awk -F'\t' -v d="$s" '
    { v = $3; if (v < 0) v = -v; print > (d "/qs_" ((v < 1000) ? "inside" : "past") ".tsv") }' || return 1
  census_build || return 1
  "$OUT/census" "$s/qs_inside.tsv" --k "$K_SAMPLE" > "$s/c_qs_inside.txt" || return 1
  "$OUT/census" "$s/qs_past.tsv" --k "$K_SAMPLE" > "$s/c_qs_past.txt" || return 1
}

stage_scans() {
  local d="$OUT/scan" s="$OUT/smoke" run_a point
  mkdir -p "$d"
  [ -s "$s/qs_inside.tsv" ] || { say "no sample: run the sample stage"; return 1; }
  g++ -std=c++20 -O2 -march=native -DNDEBUG -I "$root/adocs/data" -I "$root/src" \
    "$root/adocs/data/S122_scan_dump.cpp" "$root/build/src/libchesso_engine.a" -lpthread \
    -o "$d/scan_dump" || return 1
  g++ -std=c++20 -O2 -march=native -DNDEBUG -I "$root/adocs/data" \
    "$root/adocs/data/S122_scans.cpp" -lpthread -o "$d/scans" || return 1
  "$d/scan_dump" "$CORPUS" "$d/s083.bin" > "$d/dump_s083.txt" || return 1
  "$d/scan_dump" "$s/qs_inside.tsv" "$d/sample.bin" > "$d/dump_sample.txt" || return 1
  run_a=$(awk -F'\t' '{ v = $3; if (v < 0) v = -v; if (v < 1000) n++ } END { print n + 0 }' "$s/pool.tsv.qs")
  "$d/scans" "$d/s083.bin" --k "$K_S083" > "$d/scan_s083.txt" || return 1
  point=$(sed -n 's/^grid-best-point //p' "$d/scan_s083.txt")
  [ -n "$point" ] || { say "no grid point"; return 1; }
  "$d/scans" "$d/sample.bin" --k "$K_SAMPLE" --run-a "$run_a" --cross "$point" > "$d/scan_sample.txt" || return 1
  "$d/scans" "$d/s083.bin" --k "$K_S083_OWN" --quick > "$d/scan_s083_ownk.txt" || return 1
  "$d/scans" "$d/sample.bin" --k "$K_S083" --run-a "$run_a" --quick > "$d/scan_sample_k7119.txt" || return 1
  # The K each reading says it asks, held to the constants above.
  grep -q "K read at $K_S083  K the labels ask at head's evaluate $K_S083_OWN" "$d/scan_s083.txt" \
    || { say "S083's own K moved"; return 1; }
  grep -q "K read at $K_SAMPLE  K the labels ask at head's evaluate $K_SAMPLE" "$d/scan_sample.txt" \
    || { say "the sample's own K moved"; return 1; }
}

stage_plants() {
  local p="$OUT/proto_plants" d out="$OUT/plants.txt" plant rc
  rm -rf "$p"
  mkdir -p "$p/hdr"
  git -C "$root" archive "$BASE" | tar -x -C "$p" || return 1
  rm -rf "$p/adocs"
  cp -r "$root/tests/doctest/." "$p/tests/doctest/"
  cp "$root/adocs/data/S122_census_core.hpp" "$root/adocs/data/S101_threat_core.hpp" "$p/hdr/"
  (cd "$p" && patch -p1 --quiet < "$root/adocs/data/S122_research_prototype.patch" \
    && patch -p1 --quiet < "$root/adocs/data/S122_identity_plants.patch") || return 1
  d="$p/build"
  cmake -S "$p" -B "$d" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    "-DCMAKE_CXX_FLAGS=-I$p/hdr -DS122_SCORE_W=48" > "$p/cmake.log" 2>&1 || { say "cmake failed: plants"; return 1; }
  cmake --build "$d" -j12 --target eval_spread truncation_scan chesso_engine > "$p/build.log" 2>&1 \
    || { say "build failed: plants"; return 1; }
  g++ -std=c++20 -O2 -march=native -DNDEBUG -DS122_SCORE_W=48 -I "$p/hdr" -I "$p/src" \
    "$root/adocs/data/S122_cmp_units.cpp" "$d/src/libchesso_engine.a" -lpthread -o "$d/cmp_units" || return 1
  {
    say "the first 200000 rows of S083's corpus, the scored probe at w 48. 0 no plant; 1 the trace records White's"
    say "count off by one, the engine right; 2 the tuner's exact score divides by the wrong divisor; 3 it adds Black's"
    say "T with the wrong sign; 4 the engine's count is wrong, in the score and the trace alike; 5 T reaches the search's"
    say "instantiation and not the collecting one. rc 0 is a tool that passes the row by row check."
    for plant in 0 1 2 3 4 5; do
      S122_PLANT=$plant "$d/tools/truncation_scan" --data "$CORPUS" --min 9 --report 1 --limit 200000 > /dev/null 2>&1
      rc=$?
      printf 'plant %d  truncation_scan rc %d' "$plant" "$rc"
      S122_PLANT=$plant "$d/tools/eval_spread" --data "$CORPUS" --limit 200000 > /dev/null 2>&1
      printf '  eval_spread rc %d\n' "$?"
    done
    "$d/cmp_units" "$CORPUS" 1000000
  } > "$out" 2>&1
  # Plant 0 must pass both, the units must agree, and each tool must fail where it can.
  grep -q '^plant 0  truncation_scan rc 0  eval_spread rc 0$' "$out" || { say "plant 0 is red"; return 1; }
  grep -q ': 0$' "$out" || { say "the census and the prototype disagree on the units"; return 1; }
  rm -rf "$p"
}

stage_read() {
  ~/.venv/chess/bin/python "$root/adocs/data/S122_census_read.py" "$OUT" > "$OUT/census.new" || return 1
  mv "$OUT/census.new" "$root/adocs/data/S122_census.txt"
}

run() {
  local stage=$1 name
  name=$(printf '%s' "$stage" | tr '[:lower:]' '[:upper:]')
  if "stage_$stage"; then say "S122-$name-DONE"; else say "S122-$name-FAILED"; return 1; fi
}

case "${1:-}" in
  head|census|prototype|cost|margins|search|probes|sample|scans|plants|read) run "$1" || exit 1 ;;
  all)
    for s in head census prototype cost margins search probes sample scans plants read; do
      run "$s" || exit 1
    done
    ;;
  *) say "usage: $0 head|census|prototype|cost|margins|search|probes|sample|scans|plants|read|all"; exit 1 ;;
esac
