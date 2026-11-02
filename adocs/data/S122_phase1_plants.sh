#!/usr/bin/env bash
# S122 phase 1: each guard against the plant it exists for, in a scratch tree of the
# landing with adocs/data/S122_phase1_plants.patch applied, `build/` at the seed and
# `build-forced/` configured with -DCMAKE_CXX_FLAGS=-DS122_FORCED (the curve at forced
# weights). Never run on the main checkout: the patch is a scratch tree's alone.
#
#   adocs/data/S122_phase1_plants.sh TREE [SLICE]
#
# SLICE is a corpus slice for truncation_scan at the forced weights, under 20000 rows.
# Prints one line a run, `<guard> plant <n>: exit <s>, <doctest's tally>`, and
# S122-PLANTS-DONE last. A guard is green at plant 0 and red at its own plant; the
# mirror is green on the two-colour plants 5 and 6, which only the walk can see.
set -u
tree="${1:?usage: S122_phase1_plants.sh TREE [SLICE]}"
slice="${2:-}"
T="$tree/build/tests"
F="$tree/build-forced"
O="$tree/../plants"
mkdir -p "$O"

run() {  # plant label dir binary args...
  local plant="$1" label="$2" dir="$3" bin="$4"
  shift 4
  ( cd "$dir" && S122_PLANT=$plant timeout 3600 "./$bin" "$@" ) > "$O/$label.txt" 2>&1
  local st=$?
  echo "$label plant $plant: exit $st, $(grep -E '^\[doctest\] test cases' "$O/$label.txt" | tr -s ' ')"
}

MIRROR="-ts=evaluation: king safety family symmetry"
WALK="-ts=eval counts: S122*"
CLAMP="-tc=the clamp's derivative is exact where it binds"
FORCED_CASE="-tc=the curve's integer arithmetic is reconstructed at weights that move it"
IDENTITY="-tc=the trace reconstructs evaluate*"

run 0 mirror_p0 "$T" test_evaluation "$MIRROR"
for p in 1 2 3 4 5 6; do run "$p" "mirror_p$p" "$T" test_evaluation "$MIRROR"; done
run 0 walk_p0 "$T" test_eval_model "$WALK"
for p in 5 6; do run "$p" "walk_p$p" "$T" test_eval_model "$WALK"; done
run 0 trace_p0 "$T" test_eval_trace
for p in 7 8 9; do run "$p" "trace_p$p" "$T" test_eval_trace; done
run 0 clamp_p0 "$T" test_tuner_gradient "$CLAMP"
for p in 10 11; do run "$p" "clamp_p$p" "$T" test_tuner_gradient "$CLAMP"; done

if [[ -d "$F/tests" ]]; then
  run 0 forced_identity_p0 "$F/tests" test_eval_trace "$IDENTITY"
  run 13 forced_identity_p13 "$F/tests" test_eval_trace "$IDENTITY"
  run 0 forced_case_p0 "$F/tests" test_eval_trace "$FORCED_CASE"
fi
if [[ -n "$slice" && -x "$F/tools/truncation_scan" ]]; then
  for p in 0 13; do
    S122_PLANT=$p "$F/tools/truncation_scan" --data "$slice" --min 3.9 > "$O/scan_p$p.txt" 2>&1
    echo "truncation_scan forced plant $p: exit $?, $(tail -n 1 "$O/scan_p$p.txt")"
  done
fi
echo S122-PLANTS-DONE
