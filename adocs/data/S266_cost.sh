#!/usr/bin/env bash
# S266's research pass, 2026-11-05: what the complexity term costs the search, computed where the
# proposal puts it, in stage one at every call, at forced non-zero weights into an asm barrier while
# the score stays the landing's (adocs/data/S266_cost_sink.patch; DEC-047's forced-weight method,
# S102_cost.sh's form). Instructions only, from tools/perf_counters; no cycle, wall or nps figure is
# read. Never a candidate and never applied to a delivered tree: the patch is applied to a scratch
# extract of TREE, adocs/ left out of it.
#
#   TREE=<tree-ish> adocs/data/S266_cost.sh
#
# Four binaries, Release, `--target chesso`, each in OUT: base (TREE as it is) and S266_SINK at the
# bits adocs/data/S266_cost_sink.patch lists:
#   c1     1    the term on E1 at every stage-one call
#   c3     3    the same skipped at phase 24
#   c5     5    c1 with C as min(C, 0)
# Each must read bench 5482277 and bench 16 14931387 before any count.
# Five interleaved runs of each command, everything under `nice -n 19`, one thread; the first run is
# printed, the spread beside it. Ends S266-COST-DONE or S266-COST-FAILED. The builds are removed at
# the end, the counts kept.
set -uo pipefail

root="$(git rev-parse --show-toplevel)"
TREE="${TREE:-HEAD}"
OUT="${OUT:-$root/.tuning/s266/cost}"
pc="$root/build/tools/perf_counters"
patch="$root/adocs/data/S266_cost_sink.patch"
fail() { echo "$*"; echo S266-COST-FAILED; exit 1; }
names=(base c1 c3 c5)
bits=("" 1 3 5)
flags=()
for b in "${bits[@]}"; do flags+=("${b:+-DS266_SINK=$b}"); done

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
rm -rf "$OUT/src_tree" "$OUT"/build-*
echo S266-COST-DONE
