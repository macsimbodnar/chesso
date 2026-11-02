#!/usr/bin/env bash
# S122's speed cost: what each of S122's parts costs the search when it is computed --
# P1, safe checks by type (p1); P2, weak squares (p2); P3, defenders by square (p3);
# P4, the storm on a slot miss (p4); P5, the curve over its core (p5), and over P1 to
# P3's counts as well wherever their bits are set; and the five together (all) -- and
# that the seed landing costs nothing against the tree before it (parent). At the seed
# the compiler never sees the parts (src/evaluation.cpp, the liveness flags over S122's
# weights), so each is measured on a cost-only build, adocs/data/S122_cost_only.patch
# at S122_COST=<mask>: the part computed at forced non-zero weights into a sum an asm
# barrier consumes, while the score is the landing's, so the search is node for node
# the shipping one's and the cost is the counts' and nothing else (DEC-047's
# forced-weight method, S101's form). The storm is computed on a slot miss, where it
# would ship, and its score never enters the slot.
#
#   CAND_SHA=<landing> PARENT_SHA=<its parent> adocs/data/S122_cost.sh check
#   CAND_SHA=<landing> PARENT_SHA=<its parent> \
#     nohup adocs/data/S122_cost.sh run > .tuning/s122_cost.console 2>&1 &
#   CAND_SHA=<landing> PARENT_SHA=<its parent> ROWS=cand CAND_MASK=<mask> \
#     OUT=.tuning/s122/cost_cand nohup adocs/data/S122_cost.sh run \
#     > .tuning/s122_cost_cand.console 2>&1 &
#
# Run from the main checkout. CAND_SHA is S122's phase 1 landing, the family at its
# seed (HEAD when unset); PARENT_SHA the tree before it (CAND^ when unset). Prints
# S122-COST-DONE or S122-COST-FAILED (S122-COST-CHECK-DONE or -FAILED) as its last
# line. OUT defaults to .tuning/s122/cost.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-11-02 in S122's phase 1, before any of it ran;
# S122_cost.sh's shape, which this script is, with S122's masks.
# ============================================================================
#
# WHAT IT DECIDES: NOTHING, and there is no bar (DEC-259, DEC-344 (4)). The verdict
# carries the cost. This run's reading is recorded beside the verdict and, on H0,
# beside each bisection part. Instructions and cycles are read here; nps, the
# search's own rate, is the coordinator's on these binaries, with the machine idle
# (the corpus held it while this was written).
#
# THE BUILDS, all Release, native, padded (CHESSO_BRANCH_PAD; DEC-266 (1)), `--target
# chesso`, each from a throwaway worktree of CAND (or PARENT):
#   zero    CAND as it is: the family at its seed, in no expression
#   copy    zero's binary byte for byte: the A/A
#   parent  PARENT, the tree before S122: zero against it is the check that the
#           landing is free
#   p1..p5  CAND + the patch, S122_COST=1, 2, 4, 8, 16: one part forced
#   all     S122_COST=31: the five together, the curve over every input
#   cand    S122_COST=CAND_MASK, the screen's survivors, named after the screen.
#           P6's wide zone and P7's retirement are not rows: P6's cost is its
#           variant tree's, priced by the cand row of a tree that carries it, and
#           P7 removes code that costs nothing at zero.
#   Phase 1 built every mask in a scratch copy of the landing, 2026-11-02: bench
#   5482277 at each (the S122 step file records it).
#
# INV-6 FIRST, THE CODE CHECK, THE METHOD AND THE READING: S122_cost.sh's, unchanged --
# zero's bench, bench 12 and search_bench.py streams equal on every binary; the forced
# .text sections pairwise distinct; 16 interleaved pairs per comparison of
# perf_counters on `bench 16` and pos11 and hyperfine wall; S122_cost.py read.
#
# MACHINE TIME, from S101's run (ten comparisons, about 2 h): here eight comparisons --
# aa, parent, p1 to p5, all -- and eight builds, about 1 h 40 m, ceiling 3 h 30 m. The
# machine must be otherwise idle (MACHINE).
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s122/cost}"
PAIRS=16
patch="$root/adocs/data/S122_cost_only.patch"
helper="$root/adocs/data/S122_cost.py"
workload="$root/adocs/data/S135_cost.py"
pc="$root/build/tools/perf_counters"
python_chess="$HOME/.venv/chess/bin/python"

say() { echo "$*"; }

# The patch as `git apply` reads it, from its first `--- ` line on: a build's
# stamp names this, so an edit to the preamble alone does not refuse a
# rebuild of the same sources.
patch_sha() {
  sed -n '/^--- /,$p' "$patch" | sha256sum | cut -c1-64
}

# The tree git would hold for COMMIT with the cost patch applied, written from
# a scratch index in TREE's repository and printed as its id; the working tree
# and its own index are not touched.
patched_tree_id() {
  local tree="$1" commit="$2" scratch id status
  scratch=$(mktemp -d) || return 1
  GIT_INDEX_FILE="$scratch/index" git -C "$tree" read-tree "$commit" &&
    GIT_INDEX_FILE="$scratch/index" git -C "$tree" apply --cached "$patch" 2> /dev/null &&
    id=$(GIT_INDEX_FILE="$scratch/index" git -C "$tree" write-tree)
  status=$?
  rm -rf "$scratch"
  (( status == 0 )) && echo "$id"
}

# A worktree of COMMIT at $OUT/tree-NAME, the patch applied when asked, built
# Release into its own build directory; the binary copied to $OUT/bin/NAME,
# and beside it NAME.stamp: what it was built from. A binary already there is
# reused only when its stamp is the one this build would write. A tree already
# there is built only when its HEAD is COMMIT and its tracked files are COMMIT's
# exactly, or for a patched build COMMIT's with this patch applied exactly --
# compared as whole trees, `git diff --quiet` against the tree id
# patched_tree_id prints -- so a binary is stamped only with what was built.
# Anything else is refused, never cleaned: the old tree may be a run's record.
build_one() {
  local name="$1" commit="$2" mask="${3:-}" tree="$OUT/tree-$1" flags=""
  local stamp="$commit unpatched" had head want applied=""
  [[ -n "$mask" ]] && stamp="$commit $mask applied $(patch_sha)"
  if [[ -e "$OUT/bin/$name" ]]; then
    had=$(cat "$OUT/bin/$name.stamp" 2> /dev/null) || had="no stamp"
    if [[ "$had" != "$stamp" ]]; then
      say "refused: $OUT/bin/$name was built from '$had', not '$stamp'; take a fresh OUT, or remove that binary and $tree"
      return 1
    fi
  else
    if [[ -d "$tree" ]]; then
      head=$(git -C "$tree" rev-parse --verify --quiet HEAD) || head="no commit"
      if [[ "$head" != "$commit" ]]; then
        say "refused: $tree is at $head, not $commit; take a fresh OUT, or remove that tree"
        return 1
      fi
      if ! git -C "$tree" diff --quiet HEAD; then
        want=""
        [[ -n "$mask" ]] && want=$(patched_tree_id "$tree" "$commit")
        if [[ -z "$want" ]] || ! git -C "$tree" diff --quiet "$want"; then
          if [[ -n "$mask" ]]; then
            say "refused: $tree carries changes other than exactly the cost patch; take a fresh OUT, or remove that tree"
          else
            say "refused: $tree carries changes, and an unpatched build needs none; take a fresh OUT, or remove that tree"
          fi
          return 1
        fi
        applied=yes
      fi
    else
      git -C "$root" worktree add -q --detach "$tree" "$commit" || return 1
    fi
    if [[ -n "$mask" ]]; then
      if [[ -z "$applied" ]]; then
        git -C "$tree" apply --check "$patch" 2> /dev/null && git -C "$tree" apply "$patch" ||
          { say "the cost patch does not apply to $commit"; return 1; }
      fi
      flags="-DS122_COST=$mask"
    fi
    cmake -S "$tree" -B "$tree/build" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "-DCMAKE_CXX_FLAGS=$flags" \
        > "$OUT/cfg_$name.log" 2>&1 || { say "configure $name failed"; return 1; }
    cmake --build "$tree/build" --target chesso -j12 > "$OUT/build_$name.log" 2>&1 ||
      { tail -20 "$OUT/build_$name.log"; say "build $name failed"; return 1; }
    grep -q '^CHESSO_BRANCH_PAD:INTERNAL=ON$' "$tree/build/CMakeCache.txt" ||
      { say "$name is not padded (CHESSO_BRANCH_PAD), and DEC-266 (1) wants both sides padded"; return 1; }
    mkdir -p "$OUT/bin" && cp "$tree/build/src/chesso" "$OUT/bin/$name" || return 1
    echo "$stamp" > "$OUT/bin/$name.stamp" || return 1
  fi
  say "$name $(sha256sum "$OUT/bin/$name" | cut -c1-16) $commit ${mask:-unpatched}"
}

# The timing-stripped INV-6 streams of one binary. Each command's status is
# checked and each bench must end on its total line: a bench or
# search_bench.py failing the same way for every binary would otherwise write
# the same error text everywhere and pass the diff below. DIR holds the four
# stripped streams and nothing else, since `run` compares whole directories:
# it is emptied first, and search_bench.py's raw output, timings and knps
# included, goes to a sibling tree, $OUT/inv6_raw/<bin>, which nothing
# compares (2026-10-30: the first run kept it in DIR, and zero against parent
# read different on seconds and knps alone, nodes and moves identical).
streams() {
  local bin="$1" dir="$2" d b raw
  raw="${dir%/*}_raw/${dir##*/}"
  rm -rf "$dir" "$raw"
  mkdir -p "$dir" "$raw"
  strip() { sed -E 's/ time [0-9]+//; s/ nps [0-9]+//; s/ hashfull [0-9]+//; s/^([0-9]+ nodes) [0-9]+ nps/\1/'; }
  "$bin" bench 2>&1 | strip > "$dir/bench.txt"
  (( PIPESTATUS[0] == 0 )) || { say "$bin bench failed"; return 1; }
  "$bin" bench 12 2>&1 | strip > "$dir/bench12.txt"
  (( PIPESTATUS[0] == 0 )) || { say "$bin bench 12 failed"; return 1; }
  for b in bench bench12; do
    tail -n 1 "$dir/$b.txt" | grep -qE '^[0-9]+ nodes$' ||
      { say "$bin: $dir/$b.txt does not end on a bench total"; return 1; }
  done
  for d in 9 12; do
    "$python_chess" "$root/tools/search_bench.py" "$bin" "$d" > "$raw/sb$d.raw" 2>&1 ||
      { say "$bin: search_bench.py at depth $d failed"; return 1; }
    awk '{print $1, $3, $4, $(NF)}' "$raw/sb$d.raw" | grep -v total > "$dir/sb$d.txt"
    [[ "$(grep -c ' nodes ' "$dir/sb$d.txt")" == 3 ]] ||
      { say "$bin: search_bench.py at depth $d printed no three positions"; return 1; }
  done
}

pairs() {
  local name="$1" base="$OUT/bin/zero" cand="$OUT/bin/$2" dir="$OUT/pairs/$1" k who bin
  # perf_counters appends to its -o file, so a re-run over an old directory
  # would be read as the first run's lines; every comparison starts empty.
  rm -rf "$dir"
  mkdir -p "$dir/bench16" "$dir/pos11" "$dir/wall"
  for k in $(seq 1 "$PAIRS"); do
    local order="base cand"
    (( k % 2 == 0 )) && order="cand base"
    for who in $order; do
      bin=$base; [[ "$who" == cand ]] && bin=$cand
      "$pc" -o "$dir/bench16/${who}_$k.cnt" "$bin" bench 16 > "$dir/bench16/${who}_$k.out" 2>&1 ||
        { say "$name bench16 $who $k failed"; return 1; }
      PERF_COUNTERS="$pc" PERF_OUT="$dir/pos11/${who}_$k.cnt" \
        python3 "$workload" workload "$bin" 11 > "$dir/pos11/${who}_$k.out" 2>&1 ||
        { say "$name pos11 $who $k failed"; return 1; }
    done
    if (( k % 2 == 1 )); then
      hyperfine -N -w 0 -r 1 --export-json "$dir/wall/pair_$k.json" \
          -n base "python3 $workload workload $base 11" \
          -n cand "python3 $workload workload $cand 11" > /dev/null 2>&1
    else
      hyperfine -N -w 0 -r 1 --export-json "$dir/wall/pair_$k.json" \
          -n cand "python3 $workload workload $cand 11" \
          -n base "python3 $workload workload $base 11" > /dev/null 2>&1
    fi || { say "$name wall pair $k failed"; return 1; }
  done
  say "$name: $PAIRS pairs"
}

# 0 when COMMIT's src/evaluation.cpp carries S122's family, 1 when it does
# not, 2 when the file cannot be read there. Captured before it is searched
# (S135_cost.sh carries_columns: pipefail and SIGPIPE).
carries_family() {
  local text
  text=$(git -C "$root" show "$1:src/evaluation.cpp" 2> /dev/null) || return 2
  [[ "$text" == *king_danger_counts* ]]
}

ROWS="${ROWS:-parent p1 p2 p3 p4 p5 all}"
export CAND_MASK="${CAND_MASK:-}"

# The forced rows and their masks.
mask_of() {
  case "$1" in
    p1) echo 1 ;; p2) echo 2 ;; p3) echo 4 ;; p4) echo 8 ;;
    p5) echo 16 ;; all) echo 31 ;;
    cand) [[ "$CAND_MASK" =~ ^[0-9]+$ ]] && (( CAND_MASK >= 1 && CAND_MASK <= 31 )) &&
            echo "$CAND_MASK" ;;
    *) return 1 ;;
  esac
}

check_rows() {
  local row seen=" "
  [[ -n "${ROWS// /}" ]] || { say "refused: ROWS names no row"; return 1; }
  for row in $ROWS; do
    [[ "$row" == parent ]] || mask_of "$row" > /dev/null ||
      { say "refused: ROWS names '$row', not one of parent p1 p2 p3 p4 p5 all, or cand with CAND_MASK 1 to 31"; return 1; }
    [[ "$seen" == *" $row "* ]] && { say "refused: ROWS names $row twice"; return 1; }
    seen+="$row "
  done
  local shown="aa (zero against its copy)" spec
  for row in $ROWS; do
    if [[ "$row" == parent ]]; then spec="parent (PARENT, unpatched)"
    else spec="$row (S122_COST=$(mask_of "$row"))"; fi
    shown+=", $spec"
  done
  say "rows      $shown, each against zero"
}

cand=""
parent=""
check_pair() {
  local status
  cand=$(git -C "$root" rev-parse --verify --quiet "${CAND_SHA:-HEAD}^{commit}") ||
    { say "refused: CAND_SHA '${CAND_SHA:-HEAD}' names no commit"; return 1; }
  parent=$(git -C "$root" rev-parse --verify --quiet "${PARENT_SHA:-$cand^}^{commit}") ||
    { say "refused: PARENT_SHA '${PARENT_SHA:-CAND^}' names no commit"; return 1; }
  say "pair      cand $cand, parent $parent"
  carries_family "$cand"
  status=$?
  case $status in
    0) ;;
    1) say "refused: CAND ${cand:0:12} does not carry S122's family"; return 1 ;;
    *) say "refused: CAND ${cand:0:12} has no src/evaluation.cpp to read"; return 1 ;;
  esac
  carries_family "$parent"
  status=$?
  case $status in
    1) ;;
    0) say "refused: PARENT ${parent:0:12} carries the family already"; return 1 ;;
    *) say "refused: PARENT ${parent:0:12} has no src/evaluation.cpp to read"; return 1 ;;
  esac
  if [[ "$(git -C "$root" rev-parse --verify --quiet "$cand^")" != "$parent" ]]; then
    say "refused: PARENT ${parent:0:12} is not CAND^"
    return 1
  fi
  git -C "$root" show "$cand:adocs/data/S122_cost_only.patch" > /dev/null 2>&1 ||
    { say "refused: CAND carries no adocs/data/S122_cost_only.patch"; return 1; }
  say "pair      CAND carries the family, PARENT does not, PARENT is CAND^"
  [[ -n "$(patched_tree_id "$root" "$cand")" ]] ||
    { say "refused: $patch does not apply to CAND ${cand:0:12}"; return 1; }
  say "patch     applies to CAND, applied text sha256 $(patch_sha | cut -c1-16)"
  check_rows
}

run() {
  if [[ -e "$OUT/reading.txt" ]]; then
    say "refused: $OUT holds a reading.txt, a finished run's record whose pairs a new run would empty; take a fresh OUT"
    return 1
  fi
  mkdir -p "$OUT" || return 1
  [[ -x "$pc" ]] || { say "no $pc; build the tools first"; return 1; }
  command -v hyperfine > /dev/null || { say "no hyperfine"; return 1; }
  command -v objcopy > /dev/null || { say "no objcopy, which the code check reads with"; return 1; }
  check_pair || return 1
  {
    echo "S122 cost"
    echo "rows      aa $ROWS"
    echo "started   $(date -Iseconds)"
    echo "cand      $cand"
    echo "parent    $parent"
    echo "loadavg   $(cat /proc/loadavg)"
    echo "governor  $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null)"
  } | tee "$OUT/cost.log"

  build_one zero "$cand" | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  [[ -x "$OUT/bin/copy" ]] || cp "$OUT/bin/zero" "$OUT/bin/copy" || return 1
  cmp -s "$OUT/bin/zero" "$OUT/bin/copy" || { say "the A/A copy differs"; return 1; }
  local name
  for name in $ROWS; do
    if [[ "$name" == parent ]]; then
      build_one parent "$parent" | tee -a "$OUT/cost.log"
    else
      build_one "$name" "$cand" "$(mask_of "$name")" | tee -a "$OUT/cost.log"
    fi
    (( PIPESTATUS[0] == 0 )) || return 1
  done

  local bin
  for bin in zero $ROWS; do
    streams "$OUT/bin/$bin" "$OUT/inv6/$bin" || { say "INV-6: no streams from $bin; no timing is taken"; return 1; }
  done
  for bin in $ROWS; do
    if ! diff -r -q "$OUT/inv6/zero" "$OUT/inv6/$bin" > /dev/null; then
      diff -r "$OUT/inv6/zero" "$OUT/inv6/$bin" | head -20
      say "INV-6: $bin does not search zero's tree; no timing is taken"
      return 1
    fi
  done
  say "INV-6: bench $(tail -n 1 "$OUT/inv6/zero/bench.txt"), every binary identical" |
    tee -a "$OUT/cost.log"

  # shellcheck disable=SC2086  # ROWS is a list of names
  python3 "$helper" code "$OUT/bin" $ROWS | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || { say "the forced builds do not differ as their masks do; no timing is taken"; return 1; }

  echo "loadavg before pairs: $(cat /proc/loadavg)" >> "$OUT/cost.log"
  pairs aa copy | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  for bin in $ROWS; do
    pairs "$bin" "$bin" | tee -a "$OUT/cost.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  done
  echo "loadavg after pairs: $(cat /proc/loadavg)" >> "$OUT/cost.log"

  # shellcheck disable=SC2086
  python3 "$helper" read "$OUT/pairs" $ROWS | tee "$OUT/reading.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  echo "finished  $(date -Iseconds)" >> "$OUT/cost.log"
  say "the worktrees under $OUT stay for a re-run; git worktree remove each when the reading is recorded"
}

case "$stage" in
  check)
    if check_pair; then echo "S122-COST-CHECK-DONE"; else echo "S122-COST-CHECK-FAILED"; exit 1; fi ;;
  run)
    if run; then echo "S122-COST-DONE"; else echo "S122-COST-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 check | run" >&2
    exit 2 ;;
esac
