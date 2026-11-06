#!/usr/bin/env bash
# S124's phase 1, 2026-11-04: every guard seen red on its plant and green
# without one (DEC-350 (6)), S122_phase1_plants.sh's form. Never applied to a
# delivered tree: adocs/data/S124_phase1_plants.patch goes onto a scratch copy
# of the tree in OUT, and two builds come out of it -- `plants`, Release, its
# plant picked at run time by S124_PLANT, and `forced`, the same with
# -DS124_FORCED, every scale factor off F (the values tests/test_eval_trace.cpp
# forces), the build DEC-350 (6) reads the factor's identity on.
#
#   adocs/data/S124_phase1_plants.sh          # OUT defaults to .tuning/s124/plants
#   CORPUS=<corpus> adocs/data/S124_phase1_plants.sh
#   adocs/data/S124_phase1_plants.sh scan     # the scans alone, on OUT's forced build,
#                                             # appended to its tally
#
# Its commands run under `nice -n 10` themselves; start it at the shell's own
# priority, or the two stack.
#
# The plants, S124_PLANT:
#   1  the trace records White's candidate cell one off
#   2  exact_parts() leaves the material out of E1
#   3  exact_parts() divides the factor's numerator by 24 and not 24 F
#   4  exact_parts() picks the strong side by E1 + E2, E2 stage two's endgame
#      sum before the clamp
#   5  exact_parts() and smooth() pick the cell by E1's sign at the shipped
#      weights, the stale pick
#   6  the engine reads P from the weak side (the walk's plant)
#   7  gradient() drops the chain rule into material and the positional
#      endgame weights
#   8  the box after each step is left out (tuner_groups::box_scales())
#   9  tuner_trace::truncation_bound() takes the old bound, 3 x 23/24 and the
#      row's curve residual, for every row (tests/test_eval_trace.cpp's own
#      function until DEC-355 moved it to tools/tuner_trace.hpp, 2026-11-06,
#      where tools/truncation_scan reads it too)
#   10 smooth() moves every row at F by 0.03
#   11 the engine's one pick reads the strong side the wrong way round, in
#      both libraries
#   12 the same in the non-trace library alone, `chesso_engine`
#   13 tools/tuner.cpp's in-loop box call left out
#   14 the same call moved before the step
#
# Since the fast check, two guards more: S124_engine_eval.cpp built against the
# forced build's two libraries, `chesso_engine` and `chesso_engine_trace`, their
# evaluate() compared row for row over CORPUS -- the forced identity on the
# library the engine ships -- and tests/test_tuner_box.sh on the plants build's
# tuner.
#
# What each reads: test_eval_model's S124 suite (the walk), test_eval_trace's
# identity and forced-cell cases, test_tuner_gradient's S124 cases, and
# tools/truncation_scan on the forced build over CORPUS (read in place, never
# copied; default S083's corpus) -- each run's doctest tally or the scan's
# summary line goes to OUT/tally.txt, which adocs/data/S124_phase1_guards.txt
# takes. Under `nice -n 10`. Ends S124-PLANTS-DONE or S124-PLANTS-FAILED.
set -uo pipefail

root="$(git rev-parse --show-toplevel)"
OUT="${OUT:-$root/.tuning/s124/plants}"
CORPUS="${CORPUS:-/home/max/ws/chesso/.tuning/s083/curve/s0/cand_6226944_s1.tsv}"
patch="$root/adocs/data/S124_phase1_plants.patch"
fail() { echo "$*"; echo S124-PLANTS-FAILED; exit 1; }

[ -f "$CORPUS" ] || fail "no corpus $CORPUS"
tally="$OUT/tally.txt"

# truncation_scan on the forced build: every row's identity, refused at the
# first it disagrees on.
scan() {
  local p log status
  # Plant 9 since DEC-355: the scan holds every row to truncation_bound(), so
  # the old bound on a forced build's cell rows stops it.
  for p in 0 1 2 3 4 9; do
    log="$OUT/scan_p$p.log"
    S124_PLANT="$p" nice -n 10 "$OUT/build-forced/tools/truncation_scan" \
      --data "$CORPUS" --min 2.875 --report 20 > "$log" 2> "$log.err"
    status=$?
    printf 'forced truncation_scan plant %s: exit %s, %s\n' "$p" "$status" \
      "$(tail -1 "$log.err")" >> "$tally"
  done
}

if [ "${1:-}" = scan ]; then
  [ -x "$OUT/build-forced/tools/truncation_scan" ] || fail "no forced build in $OUT"
  scan
  cat "$tally"
  echo S124-PLANTS-DONE
  exit 0
fi

rm -rf "$OUT/src_tree" && mkdir -p "$OUT/src_tree" || fail "cannot make $OUT"
# The working tree as it stands, tracked and new files alike: the landing is
# staged, not committed, while phase 1 is built.
# The doctest submodule is one gitlink entry, a directory, copied whole below.
(cd "$root" && git ls-files --cached --others --exclude-standard -z |
  grep -zv '^tests/doctest$' |
  xargs -0 -I{} cp --parents {} "$OUT/src_tree/") || fail "copy"
cp -r "$root/tests/doctest/." "$OUT/src_tree/tests/doctest/" || fail "doctest"
(cd "$OUT/src_tree" && patch -p1 < "$patch") > "$OUT/patch.log" || fail "patch"

build() {
  local name="$1" flags="$2"
  cmake -S "$OUT/src_tree" -B "$OUT/build-$name" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "-DCMAKE_CXX_FLAGS=$flags" \
    > "$OUT/cfg_$name.log" 2>&1 &&
    nice -n 10 cmake --build "$OUT/build-$name" -j12 --target tuner test_eval_model \
      test_eval_trace test_tuner_gradient truncation_scan \
      > "$OUT/build_$name.log" 2>&1
}
build plants "" || fail "build plants"
build forced "-DS124_FORCED" || fail "build forced"

: > "$tally"

# One doctest run: its build, its binary, a plant, a case filter; the tally
# line and the first failing assertion's expression, if any.
run() {
  local name="$1" bin="$2" plant="$3" cases="$4"
  local log="$OUT/run_${name}_${bin}_p${plant}.log"
  (cd "$root" && S124_PLANT="$plant" nice -n 10 \
    "$OUT/build-$name/tests/$bin" "--test-case=$cases" > "$log" 2>&1)
  local status=$?
  printf '%s %s plant %s [%s]: exit %s, %s\n' "$name" "$bin" "$plant" "$cases" \
    "$status" "$(grep -E '^\[doctest\] assertions:' "$log" | tr -s ' ')" >> "$tally"
  grep -m1 -E 'ERROR|CHECK\(|REQUIRE\(' "$log" | sed 's/^/    first: /' >> "$tally"
}

walk="the engine counts the endgame family the walk counts,the walk and the engine count the endgame family by hand"
identity="the trace reconstructs evaluate() exactly*"
forced_case="the factor's integer arithmetic is reconstructed at cells that move it"
bound="the smooth score is within the truncation bound of the exact one"
fd="the endgame factor's gradient is the derivative*"
stale="the factor's cell follows E1's sign when a fit moves it"
box="a step boxes the scale factors to*"

for p in 0 6; do run plants test_eval_model "$p" "$walk"; done
for p in 0 1 2 3 4; do run plants test_eval_trace "$p" "$identity,$forced_case"; done
for p in 0 1 2 3 4; do run forced test_eval_trace "$p" "$identity,$bound"; done
for p in 0 5 7 8; do run plants test_tuner_gradient "$p" "$fd,$stale,$box"; done

# The per-row truncation bound (DEC-350 (1)): rows at F keep the old one, red on
# plant 10; a row off F needs the new one, red on plant 9 in the forced-cell case
# and on the forced build.
for p in 0 10; do run plants test_eval_trace "$p" "$bound"; done
for p in 9; do run plants test_eval_trace "$p" "$forced_case"; done
for p in 9 11; do run forced test_eval_trace "$p" "$identity,$bound"; done

# The box's in-loop call site in tools/tuner.cpp, deleted and moved. The call
# before the loop boxes the shipped constants, in [0, F] by a static_assert,
# after ties of scale 1, so no fit can see it.
for p in 0 13 14; do
  log="$OUT/run_plants_box_p$p.log"
  (cd "$root" && S124_PLANT="$p" nice -n 10 bash tests/test_tuner_box.sh \
    "$OUT/build-plants/tools/tuner" > "$log" 2>&1)
  printf 'plants test_tuner_box.sh plant %s: exit %s, %s\n' "$p" "$?" \
    "$(tail -1 "$log")" >> "$tally"
done

# The forced identity on the library the engine ships: evaluate() from
# chesso_engine against chesso_engine_trace, line for line over CORPUS.
for lib in chesso_engine chesso_engine_trace; do
  nice -n 10 g++ -std=c++20 -O2 -march=native -DNDEBUG -I "$OUT/src_tree/src" \
    "$root/adocs/data/S124_engine_eval.cpp" "$OUT/build-forced/src/lib$lib.a" \
    -lpthread -o "$OUT/eval_$lib" || fail "build eval_$lib"
done
for p in 0 11 12; do
  for lib in chesso_engine chesso_engine_trace; do
    S124_PLANT="$p" nice -n 10 "$OUT/eval_$lib" "$CORPUS" > "$OUT/eval_${lib}_p$p.txt" ||
      fail "eval_$lib plant $p"
  done
  differ=$(paste -d' ' "$OUT/eval_chesso_engine_p$p.txt" "$OUT/eval_chesso_engine_trace_p$p.txt" |
    awk '$1 != $2' | wc -l)
  printf 'forced chesso_engine against chesso_engine_trace plant %s: %s rows of %s differ\n' \
    "$p" "$differ" "$(wc -l < "$OUT/eval_chesso_engine_p$p.txt")" >> "$tally"
done

scan

cat "$tally"
echo S124-PLANTS-DONE
