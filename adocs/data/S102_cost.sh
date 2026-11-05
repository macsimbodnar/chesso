#!/usr/bin/env bash
# S102's research pass, 2026-11-04: what the outpost and space counts cost the search, computed
# where the proposal puts them, at forced non-zero weights into an asm barrier while the score stays
# the landing's (adocs/data/S102_cost_sink.patch; DEC-047's forced-weight method, S124_cost.sh's
# form). Instructions only, from tools/perf_counters; no cycle, wall or nps figure is read. Never a
# candidate and never applied to a delivered tree: the patch is applied to a scratch extract of TREE,
# adocs/ left out of it.
#
#   TREE=<tree-ish> adocs/data/S102_cost.sh
#
# Twelve binaries, Release, `--target chesso`, each in OUT: base (TREE as it is) and S102_SINK at
# the bits adocs/data/S102_cost_sink.patch lists, one change at a time so each step prices one thing:
#   o1     1    knights on an outpost by cell and on a hole, stage one, every call
#   o3     3    the same behind the narrow gate (a knight in either side's region)
#   o3w    259  the same behind the wide gate (a knight anywhere, or a bishop in the region)
#   oX     291  + the no-exchange cell (P2)
#   oXB    307  + bishops on an outpost (P5)
#   oXBl   371  + reach by a loop over the knights (P6, the census's first form)
#   oXBs   435  + reach set-wise instead (P6 as proposed)
#   oXBsF  947  + the rank-4 flank cell (P4): every outpost part as proposed
#   o2s    696  every outpost part in stage two, set-wise reach, no gate
#   s4     4    space through the entry's spare bytes (P7 and P8 on a miss, P9 every call)
#   all    951  every outpost part and space
# Each must read bench 5482277 and bench 16 14931387 before any count.
# Five interleaved runs of each command, everything under `nice -n 19`, one thread; the first run is
# printed, the spread beside it. Ends S102-COST-DONE or S102-COST-FAILED.
set -uo pipefail

root="$(git rev-parse --show-toplevel)"
TREE="${TREE:-HEAD}"
OUT="${OUT:-$root/.tuning/s102/cost}"
pc="$root/build/tools/perf_counters"
patch="$root/adocs/data/S102_cost_sink.patch"
fail() { echo "$*"; echo S102-COST-FAILED; exit 1; }
names=(base o1 o3 o3w oX oXB oXBl oXBs oXBsF o2s s4 all)
bits=("" 1 3 259 291 307 371 435 947 696 4 951)
flags=()
for b in "${bits[@]}"; do flags+=("${b:+-DS102_SINK=$b}"); done

[ -x "$pc" ] || fail "no $pc: build/ first"
rm -rf "$OUT/src_tree" && mkdir -p "$OUT/src_tree" || fail "cannot make $OUT"
paths=$(git -C "$root" ls-tree --name-only "$TREE" | grep -v '^adocs$')
# shellcheck disable=SC2086
git -C "$root" archive "$TREE" $paths | tar -x -C "$OUT/src_tree" || fail "archive $TREE"
cp -r "$root/tests/doctest/." "$OUT/src_tree/tests/doctest/" || fail "doctest"
(cd "$OUT/src_tree" && patch -p1 < "$patch") > "$OUT/patch.log" || fail "patch"

for i in "${!names[@]}"; do
  n="${names[$i]}"
  cmake -S "$OUT/src_tree" -B "$OUT/build-$n" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "-DCMAKE_CXX_FLAGS=${flags[$i]}" > "$OUT/cfg_$n.log" 2>&1 &&
    nice -n 19 cmake --build "$OUT/build-$n" -j2 --target chesso > "$OUT/build_$n.log" 2>&1 ||
    fail "build $n"
done

for n in "${names[@]}"; do
  n1=$(nice -n 19 "$OUT/build-$n/src/chesso" bench 2>&1 | tail -1 | awk '{print $1}')
  n16=$(nice -n 19 "$OUT/build-$n/src/chesso" bench 16 2>&1 | tail -1 | awk '{print $1}')
  echo "$n bench $n1 bench16 $n16"
  [ "$n1" = 5482277 ] && [ "$n16" = 14931387 ] || fail "$n moved a node count"
done

rm -f "$OUT"/pc_*.txt
for i in 1 2 3 4 5; do
  for n in "${names[@]}"; do
    nice -n 19 "$pc" -o "$OUT/pc_${n}_1.txt" -- "$OUT/build-$n/src/chesso" bench > /dev/null 2>&1 || fail "run"
    nice -n 19 "$pc" -o "$OUT/pc_${n}_16.txt" -- "$OUT/build-$n/src/chesso" bench 16 > /dev/null 2>&1 || fail "run"
  done
done

python3 - "$OUT" "${names[@]}" <<'PY' || fail "read"
import sys
out, names = sys.argv[1], sys.argv[2:]
def runs(b, w):
    return [int(l.split()[2]) for l in open(f"{out}/pc_{b}_{w}.txt")]
for w, name in (("1", "bench"), ("16", "bench 16")):
    base = runs("base", w)
    print(f"{name}: base {base[0]} (spread {max(base) - min(base)})")
    for b in names[1:]:
        r = runs(b, w)
        print(f"  {b:5s} {r[0]} (spread {max(r) - min(r)})  {100 * (r[0] / base[0] - 1):+.3f} %")
PY
echo S102-COST-DONE
