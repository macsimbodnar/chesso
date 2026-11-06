#!/usr/bin/env bash
# S124's phase 2a, 2026-11-06: the guards over tools/feature_audit's local form
# (tools/audit_design.hpp), each seen red on a plant and green without one.
# Never applied to a delivered tree: adocs/data/S124_phase2_plants.patch goes
# onto a scratch copy of the tree under OUT, one Release build, the plant picked
# at run time by S124_P2_PLANT:
#
#   0  none
#   1  the local form takes each side's x itself, its max(x, 0) dropped
#   2  Black's side read at White's x
#   3  the endgame half's column at the middlegame half's x
#   4  the slope squared, max(x, 0)^2 for max(x, 0)
#   5  local_form_refusal() refuses nothing (the fast check's finding 6), read by the
#      test's refusal case; the engine's model is the supported one, so the corpus
#      reading does not move
#
# What each run reads: tests/test_audit_design.cpp's three cases (the doctest
# tally), and tools/feature_audit over SLICE, the first 300000 rows of S122's
# regenerated prepared corpus, its design check and linearisation check lines
# and its rank line. Run from the worktree; OUT defaults to
# .tuning/s124p2/plants. Under `nice -n 10`. Ends S124-P2-PLANTS-DONE or
# S124-P2-PLANTS-FAILED; the tally is OUT/tally.txt, which
# adocs/data/S124_phase2_plants.txt takes.
set -uo pipefail

root="$(git rev-parse --show-toplevel)"
OUT="${OUT:-$root/.tuning/s124p2/plants}"
CORPUS="${CORPUS:-/home/max/ws/chesso/.tuning/s122/prepared/corpus.tsv}"
patch="$root/adocs/data/S124_phase2_plants.patch"
fail() { echo "$*"; echo S124-P2-PLANTS-FAILED; exit 1; }

mkdir -p "$OUT" || fail "cannot create $OUT"
tree="$OUT/tree"
rm -rf "$tree"
mkdir -p "$tree" || fail "cannot create $tree"
# The working tree as it stands, tracked and new files alike, the build
# directories and .tuning/ left out.
( cd "$root" && git ls-files --cached --others --exclude-standard -z |
    tar --null -T - -cf - ) | tar -xf - -C "$tree" || fail "the copy failed"
rm -rf "$tree/tests/doctest"
cp -r "$root/tests/doctest" "$tree/tests/doctest" || fail "doctest not copied"
( cd "$tree" && patch -p1 --no-backup-if-mismatch < "$patch" ) > "$OUT/patch.log" 2>&1 ||
  fail "the plants patch does not apply: $OUT/patch.log"
grep -q 'offset\|fuzz' "$OUT/patch.log" && fail "the plants patch applied with offset or fuzz"

( cd "$tree" &&
  nice -n 10 cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache > /dev/null &&
  nice -n 10 cmake --build build -j12 --target test_audit_design feature_audit ) \
  > "$OUT/build.log" 2>&1 || fail "the plants build failed: $OUT/build.log"

slice="$OUT/slice300k.tsv"
[[ -f "$slice" ]] || head -n 300000 "$CORPUS" > "$slice" || fail "no slice"

: > "$OUT/tally.txt"
for plant in 0 1 2 3 4 5; do
  {
    echo "== S124_P2_PLANT=$plant"
    S124_P2_PLANT=$plant nice -n 10 "$tree/build/tests/test_audit_design" 2>&1 |
      grep -E 'TEST CASE|ERROR|MESSAGE|^\[doctest\] (test cases|assertions|Status)' |
      sed "s|$tree/||g"
    S124_P2_PLANT=$plant nice -n 10 "$tree/build/tools/feature_audit" --data "$slice" \
        --free endgame_scale --no-columns --no-occurrence --no-collinearity 2>&1 |
      grep -E '^identities: (design check|linearisation check|rank over|rank modulo)'
  } >> "$OUT/tally.txt"
done
cat "$OUT/tally.txt"
echo S124-P2-PLANTS-DONE
