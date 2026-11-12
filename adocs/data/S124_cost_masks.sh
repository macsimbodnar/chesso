#!/usr/bin/env bash
# S124 phase 2b, 2026-11-11: what the candidate's own masks of the endgame family cost the
# search, in instructions (INV-4, DEC-350's screen, "The cost"), adocs/data/S124_phase1_cost.sh
# widened to the candidate. Phase 1's sink (adocs/data/S124_phase1_cost.patch) computes the
# landed mop-up and scaled taper with forced weights into an asm barrier while the score stays
# the landing's, so the search is node for node REF's (DEC-047's form). Here the sink's three
# forced arrays carry the candidate's own values, read from this worktree's
# src/evaluation.cpp, the staged paste, so what is counted is the code the candidate ships:
# the cell's gate and pick, and the scaled taper, at the screen's F1 and F2 cells with F3's at
# F; mop-up's domain test, and its counts where it holds, at P1's six. Instructions only, from
# tools/perf_counters; no cycle, wall or nps figure is read. Never a candidate: the patch goes
# onto a scratch export of REF.
#
#   adocs/data/S124_cost_masks.sh              # OUT defaults to .tuning/s124/cost_masks
#   SMOKE=1 adocs/data/S124_cost_masks.sh      # one `bench` run of each, to show it runs
#
# REF defaults to HEAD, the family at its identity (S124_screen.py landing-check refuses any
# other). Four binaries, Release, `--target chesso perf_counters`: parent (no sink), factor
# (-DS124_SINK=1), mopup (=2) and cand (=3, both). Each must read the parent's own `bench` and
# `bench 16` node counts before any count is taken. Five interleaved runs of each command
# (one, and `bench` alone, under SMOKE); the first is printed, the spread beside it. Ends
# S124-COST-MASKS-DONE or S124-COST-MASKS-FAILED.
set -uo pipefail

root="$(git rev-parse --show-toplevel)"
OUT="${OUT:-$root/.tuning/s124/cost_masks}"
REF="${REF:-HEAD}"
SMOKE="${SMOKE:-0}"
patch="$root/adocs/data/S124_phase1_cost.patch"
fail() { echo "$*"; echo S124-COST-MASKS-FAILED; exit 1; }

python3 "$root/adocs/data/S124_screen.py" landing-check <(git -C "$root" show "$REF:src/evaluation.cpp") ||
  fail "refused: REF $REF is not the family at its identity, so a sink there would move the score"

# The candidate's three arrays, from the working tree.
read_array() {
  python3 - "$root/src/evaluation.cpp" "$1" <<'EOF'
import re, sys
text = open(sys.argv[1]).read()
m = re.search(r"^constexpr int %s\[\w+\] = \{([^}]*)\};" % sys.argv[2], text, re.M)
if not m:
    sys.exit("no %s in %s" % (sys.argv[2], sys.argv[1]))
print(", ".join(re.findall(r"-?\d+", m.group(1))))
EOF
}
scale=$(read_array endgame_scale) || fail "no endgame_scale in the working tree"
mg=$(read_array mopup_mg) || fail "no mopup_mg in the working tree"
eg=$(read_array mopup_eg) || fail "no mopup_eg in the working tree"
echo "the candidate's arrays, from $root/src/evaluation.cpp:"
echo "  endgame_scale {$scale}"
echo "  mopup_mg {$mg}  mopup_eg {$eg}"

rm -rf "$OUT/src_tree" && mkdir -p "$OUT/src_tree" || fail "cannot make $OUT"
git -C "$root" archive "$REF" | tar -x -C "$OUT/src_tree" || fail "export $REF"
cp -r "$root/tests/doctest/." "$OUT/src_tree/tests/doctest/" || fail "doctest"
(cd "$OUT/src_tree" && patch -p1 < "$patch") > "$OUT/patch.log" || fail "patch"
python3 - "$OUT/src_tree/src/evaluation.cpp" "$scale" "$mg" "$eg" <<'EOF' || fail "the sink's arrays were not set"
import re, sys
path, scale, mg, eg = sys.argv[1:]
text = open(path).read()
for name, values in (("sink_endgame_scale", scale), ("sink_mopup_mg", mg), ("sink_mopup_eg", eg)):
    text, n = re.subn(r"(constexpr int %s\[\w+\] = \{)[^}]*(\};)" % name,
                      lambda m: m.group(1) + values + m.group(2), text)
    assert n == 1, (name, n)
open(path, "w").write(text)
EOF
grep -A1 -E 'constexpr int sink_(endgame_scale|mopup_mg|mopup_eg)' "$OUT/src_tree/src/evaluation.cpp" |
  sed 's/^/  sink: /'

build() {
  local name="$1" flags="$2"
  cmake -S "$OUT/src_tree" -B "$OUT/build-$name" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "-DCMAKE_CXX_FLAGS=$flags" \
    > "$OUT/cfg_$name.log" 2>&1 &&
    nice -n 10 cmake --build "$OUT/build-$name" -j12 --target chesso perf_counters \
      > "$OUT/build_$name.log" 2>&1
}
build parent "" || fail "build parent"
build factor "-DS124_SINK=1" || fail "build factor"
build mopup "-DS124_SINK=2" || fail "build mopup"
build cand "-DS124_SINK=3" || fail "build cand"
pc="$OUT/build-parent/tools/perf_counters"

commands=("bench" "bench 16")
runs=5
if [[ "$SMOKE" == 1 ]]; then commands=("bench"); runs=1; fi

count() { nice -n 10 "$OUT/build-$1/src/chesso" $2 2>&1 | tail -1 | awk '{print $1}'; }
declare -A want
for c in "${commands[@]}"; do want[$c]=$(count parent "$c"); done
for b in parent factor mopup cand; do
  line="$b"
  for c in "${commands[@]}"; do
    n=$(count "$b" "$c")
    line="$line, $c $n"
    [[ "$n" == "${want[$c]}" ]] || fail "$b reads $c $n against the parent's ${want[$c]}: a node count moved"
  done
  echo "$line"
done

rm -f "$OUT"/pc_*.txt
for ((i = 1; i <= runs; i++)); do
  for b in parent factor mopup cand; do
    for c in "${commands[@]}"; do
      tag="${c// /_}"
      nice -n 10 "$pc" -o "$OUT/pc_${b}_${tag}.txt" -- "$OUT/build-$b/src/chesso" $c > /dev/null 2>&1 ||
        fail "perf_counters failed on $b $c"
    done
  done
done

python3 - "$OUT" "${commands[@]}" <<'EOF' || fail "read"
import sys
out, commands = sys.argv[1], sys.argv[2:]
def runs(b, tag):
    return [int(l.split()[2]) for l in open(f"{out}/pc_{b}_{tag}.txt")]
for c in commands:
    tag = c.replace(" ", "_")
    base = runs("parent", tag)
    print(f"{c}: parent {base[0]} (spread {max(base) - min(base)}, {len(base)} runs)")
    for b in ("factor", "mopup", "cand"):
        r = runs(b, tag)
        print(f"  {b:6s} {r[0]} (spread {max(r) - min(r)})  {100 * (r[0] / base[0] - 1):+.3f} %")
EOF
echo S124-COST-MASKS-DONE
