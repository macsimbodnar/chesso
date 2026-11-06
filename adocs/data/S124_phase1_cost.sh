#!/usr/bin/env bash
# S124's phase 1, 2026-11-04: what the shipped form of the endgame family
# costs the search once it is live, in instructions (INV-4, DEC-350): the
# factor's cell and its scaled taper, and mop-up's domain and three counts,
# computed in stage one with forced non-zero weights into an asm barrier while
# the score stays the landing's (adocs/data/S124_phase1_cost.patch; DEC-047's
# forced-weight method, S124_cost.sh's sink form). The bodies are the landed
# functions' own, copied with the forced arrays' names, so what is counted is
# the code a fit would ship. Instructions only, from tools/perf_counters; no
# cycle, wall or nps figure is read. Never a candidate: the patch goes onto a
# scratch copy of the working tree.
#
#   adocs/data/S124_phase1_cost.sh        # OUT defaults to .tuning/s124/cost1
#
# Four binaries, Release, `--target chesso`: base (the landing), factor
# (-DS124_SINK=1), mopup (=2) and both (=3). Each must read bench 5482277 and
# bench 16 14931387 before any count. Five interleaved runs of each command;
# the first is printed, the spread beside it. Ends S124-COST1-DONE or
# S124-COST1-FAILED.
set -uo pipefail

root="$(git rev-parse --show-toplevel)"
OUT="${OUT:-$root/.tuning/s124/cost1}"
patch="$root/adocs/data/S124_phase1_cost.patch"
fail() { echo "$*"; echo S124-COST1-FAILED; exit 1; }

rm -rf "$OUT/src_tree" && mkdir -p "$OUT/src_tree" || fail "cannot make $OUT"
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
    nice -n 10 cmake --build "$OUT/build-$name" -j12 --target chesso perf_counters \
      > "$OUT/build_$name.log" 2>&1
}
build base "" || fail "build base"
build factor "-DS124_SINK=1" || fail "build factor"
build mopup "-DS124_SINK=2" || fail "build mopup"
build both "-DS124_SINK=3" || fail "build both"
pc="$OUT/build-base/tools/perf_counters"

for b in base factor mopup both; do
  n1=$(nice -n 10 "$OUT/build-$b/src/chesso" bench 2>&1 | tail -1 | awk '{print $1}')
  n16=$(nice -n 10 "$OUT/build-$b/src/chesso" bench 16 2>&1 | tail -1 | awk '{print $1}')
  echo "$b bench $n1 bench16 $n16"
  [ "$n1" = 5482277 ] && [ "$n16" = 14931387 ] || fail "$b moved a node count"
done

rm -f "$OUT"/pc_*.txt
for i in 1 2 3 4 5; do
  for b in base factor mopup both; do
    nice -n 10 "$pc" -o "$OUT/pc_${b}_1.txt" -- "$OUT/build-$b/src/chesso" bench > /dev/null 2>&1 || fail "run"
    nice -n 10 "$pc" -o "$OUT/pc_${b}_16.txt" -- "$OUT/build-$b/src/chesso" bench 16 > /dev/null 2>&1 || fail "run"
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
    for b in ("factor", "mopup", "both"):
        r = runs(b, w)
        print(f"  {b:6s} {r[0]} (spread {max(r) - min(r)})  {100 * (r[0] / base[0] - 1):+.3f} %")
EOF
echo S124-COST1-DONE
