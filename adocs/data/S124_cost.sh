#!/usr/bin/env bash
# S124's research pass, 2026-11-03: what the scale factor's case and cell cost the search when
# they are computed in stage one, at forced non-identity cells into an asm barrier while the
# score stays the landing's (adocs/data/S124_cost_sink.patch; DEC-047's forced-weight method,
# S122_cost.sh's sink form at its smallest). Instructions only, from tools/perf_counters; no
# cycle, wall or nps figure is read. Never a candidate and never applied to a delivered tree:
# the patch is applied to a scratch extract of TREE.
#
#   TREE=<tree-ish> adocs/data/S124_cost.sh
#
# Three binaries, Release, `--target chesso`, each in OUT: base (TREE as it is), eager
# (-DS124_SINK: every count computed at every call) and gated (-DS124_SINK -DS124_GATE: past
# two pawns a side, a knight or a queen, or not one bishop a side, nothing more is read).
# Each must read bench 5482277 and bench 16 14931387 before any count. Five interleaved runs
# of each command; the first is printed, the spread beside it. Ends S124-COST-DONE or
# S124-COST-FAILED.
set -uo pipefail

root="$(git rev-parse --show-toplevel)"
TREE="${TREE:-HEAD}"
OUT="${OUT:-$root/.tuning/s124/cost}"
pc="$root/build/tools/perf_counters"
patch="$root/adocs/data/S124_cost_sink.patch"
fail() { echo "$*"; echo S124-COST-FAILED; exit 1; }

[ -x "$pc" ] || fail "no $pc: build/ first"
rm -rf "$OUT/src_tree" && mkdir -p "$OUT/src_tree" || fail "cannot make $OUT"
git -C "$root" archive "$TREE" | tar -x -C "$OUT/src_tree" || fail "archive $TREE"
cp -r "$root/tests/doctest/." "$OUT/src_tree/tests/doctest/" || fail "doctest"
(cd "$OUT/src_tree" && patch -p1 < "$patch") > "$OUT/patch.log" || fail "patch"

build() {
  local name="$1" flags="$2"
  cmake -S "$OUT/src_tree" -B "$OUT/build-$name" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "-DCMAKE_CXX_FLAGS=$flags" > "$OUT/cfg_$name.log" 2>&1 &&
    nice -n 19 cmake --build "$OUT/build-$name" -j4 --target chesso > "$OUT/build_$name.log" 2>&1
}
build base "" || fail "build base"
build eager "-DS124_SINK" || fail "build eager"
build gated "-DS124_SINK -DS124_GATE" || fail "build gated"

for b in base eager gated; do
  n1=$(nice -n 19 "$OUT/build-$b/src/chesso" bench 2>&1 | tail -1 | awk '{print $1}')
  n16=$(nice -n 19 "$OUT/build-$b/src/chesso" bench 16 2>&1 | tail -1 | awk '{print $1}')
  echo "$b bench $n1 bench16 $n16"
  [ "$n1" = 5482277 ] && [ "$n16" = 14931387 ] || fail "$b moved a node count"
done

rm -f "$OUT"/pc_*.txt
for i in 1 2 3 4 5; do
  for b in base eager gated; do
    nice -n 19 "$pc" -o "$OUT/pc_${b}_1.txt" -- "$OUT/build-$b/src/chesso" bench > /dev/null 2>&1 || fail "run"
    nice -n 19 "$pc" -o "$OUT/pc_${b}_16.txt" -- "$OUT/build-$b/src/chesso" bench 16 > /dev/null 2>&1 || fail "run"
  done
done

python3 - "$OUT" <<'EOF' || fail "read"
import sys
out = sys.argv[1]
def runs(b, w):
    return [int(l.split()[2]) for l in open(f"{out}/pc_{b}_{w}.txt")]
for w, name in (("1", "bench"), ("16", "bench 16")):
    base = runs("base", w)
    print(f"{name}: base {base[0]} (spread {max(base) - min(base)})")
    for b in ("eager", "gated"):
        r = runs(b, w)
        print(f"  {b:6s} {r[0]} (spread {max(r) - min(r)})  {100 * (r[0] / base[0] - 1):+.3f} %")
EOF
echo S124-COST-DONE
