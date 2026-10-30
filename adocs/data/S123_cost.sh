#!/usr/bin/env bash
# S123's speed cost: what each of S123's three groups costs the search when it
# is computed -- A, the free and safe tests, is_attacked() once per passer
# whose push square is empty (a); B, both kings' distances to each passer's
# push square (b); C, the candidate sets (c); and the three together (all) --
# and that the zero-weight landing costs nothing against the tree before it
# (parent). At zero the compiler never sees the groups (src/evaluation.cpp,
# the liveness over passer_family_side()), so each is measured on a cost-only
# build, adocs/data/S123_cost_only.patch at S123_COST=<mask>: the group
# computed at forced non-zero weights into a sink an asm barrier consumes,
# while the score is the landing's, so the search is node for node the
# shipping one's and the wall time is the cost and nothing else (DEC-047's
# forced-weight method, S135's cost form).
#
#   CAND_SHA=<landing> PARENT_SHA=<its parent> adocs/data/S123_cost.sh check
#   CAND_SHA=<landing> PARENT_SHA=<its parent> \
#     nohup adocs/data/S123_cost.sh run > .tuning/s123_cost.console 2>&1 &
#
# Run from the main checkout. CAND_SHA is S123's phase 1 landing, the family
# at zero (HEAD when unset); PARENT_SHA the tree before it (CAND^ when unset).
# `run` checks the pair first -- CAND carries the family and the patch,
# PARENT does not carry the family, PARENT is CAND^, the patch applies to CAND
# -- and `check` does that alone. Prints S123-COST-DONE or S123-COST-FAILED
# (S123-COST-CHECK-DONE or -FAILED) as its last line, so a run goes detached
# under a four-exit watcher. OUT defaults to .tuning/s123/cost.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-30 in S123's phase 1, before any of it
# ran; S121_cost.sh's shape and S135_cost.sh's 2026-10-25 guards.
# ============================================================================
#
# WHAT IT DECIDES: NOTHING. The verdict carries the cost: the candidate's
# survivors are non-zero and computed in every game it plays (DEC-091's
# reading, DEC-329 (9)). This run's reading is recorded beside the verdict
# and, on H0, beside each bisection part, where a cost concentrated in one
# group is what the split is read with. It is not a speed verdict.
#
# THE BUILDS, all Release, native, padded (CHESSO_BRANCH_PAD; DEC-266 (1)),
# `--target chesso`, each from a throwaway worktree of CAND (or PARENT):
#   zero    CAND as it is: the family at zero, in no expression
#   copy    zero's binary byte for byte: the A/A
#   parent  PARENT, the tree before S123: zero against it is the check that
#           the landing is free -- instructions equal within the A/A's spread
#   a       CAND + the patch, S123_COST=1: A forced
#   b       S123_COST=2: B forced
#   c       S123_COST=4: C forced
#   all     S123_COST=7: the three together
#   The forced builds move evaluate_pawns() out of line (evaluate_cheap()
#   disassembles to 43 instructions at masks 1, 2 and 7 against 381 at zero,
#   read at phase 1 on g++ 13.3), as a fitted survivor would; that is part of
#   what is timed.
#
# INV-6 FIRST. Every binary prints zero's `bench` total, `bench 12` lines and
# `tools/search_bench.py` node counts and best moves at depths 9 and 12, the
# timing fields stripped; any difference stops the run. Phase 1 read bench
# 6016079 at all four masks.
#
# THE CODE CHECK, before any timing: S123_cost.py code, the forced and zero
# builds' .text sections pairwise distinct.
#
# THE METHOD, S270's as DEC-266 adopts it: 16 interleaved pairs per
# comparison, the order swapped every pair; build/tools/perf_counters
# (instructions and cycles, user space, the engine's process alone) on `chesso
# bench 16` and on pos11, S020's 300 positions at depth 11 through one engine
# process (S135_cost.py workload); hyperfine wall, one run of each side a pair,
# on pos11. Comparisons against zero: aa (copy), parent, a, b, c, all.
#
# THE READING, S123_cost.py read: per comparison and instrument the geometric
# mean ratio and its 95 % CI from the paired t. The floor is the A/A's largest
# CI bound per workload and instrument; a cost counts when its cycles CI on
# both workloads and its wall CI lie wholly beyond the floor the same way and
# its instructions move the same way (DEC-266 (3), (4)); otherwise zero, or
# owing (4)'s perturbations when instructions disagree. It refuses before any
# verdict unless each forced build's instructions lie above zero's, and all's
# above each group's, by more than the A/A's instruction spread; a + b + c
# against all is printed, not gated.
#
# MACHINE TIME, from S121's run (adocs/data/S121_cost_reading.txt, seven
# comparisons in 1 h 35 m) and S135's (a comparison about 10.4 min): six
# comparisons about 63 min; six builds about 1 to 2 min each with ccache, the
# identity streams about 1 min. About 1 h 15 m in all, ceiling 2 h 30 m.
# Disk: six worktrees of about 240 MB with their builds, 1.5 GB. The machine
# must be otherwise idle (MACHINE): a match, a fit or datagen beside it makes
# the floor meaningless.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s123/cost}"
PAIRS=16
patch="$root/adocs/data/S123_cost_only.patch"
helper="$root/adocs/data/S123_cost.py"
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
      flags="-DS123_COST=$mask"
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
# the same error text everywhere and pass the diff below.
streams() {
  local bin="$1" dir="$2" d b
  mkdir -p "$dir"
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
    "$python_chess" "$root/tools/search_bench.py" "$bin" "$d" > "$dir/sb$d.raw" 2>&1 ||
      { say "$bin: search_bench.py at depth $d failed"; return 1; }
    awk '{print $1, $3, $4, $(NF)}' "$dir/sb$d.raw" | grep -v total > "$dir/sb$d.txt"
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

# 0 when COMMIT's src/evaluation.cpp carries S123's family, 1 when it does
# not, 2 when the file cannot be read there. Captured before it is searched
# (S135_cost.sh carries_columns: pipefail and SIGPIPE).
carries_family() {
  local text
  text=$(git -C "$root" show "$1:src/evaluation.cpp" 2> /dev/null) || return 2
  [[ "$text" == *passer_family_side* ]]
}

ROWS="${ROWS:-parent a b c all}"

# The forced rows and their masks.
mask_of() {
  case "$1" in
    a) echo 1 ;; b) echo 2 ;; c) echo 4 ;; all) echo 7 ;; *) return 1 ;;
  esac
}

check_rows() {
  local row seen=" "
  [[ -n "${ROWS// /}" ]] || { say "refused: ROWS names no row"; return 1; }
  for row in $ROWS; do
    [[ "$row" == parent ]] || mask_of "$row" > /dev/null ||
      { say "refused: ROWS names '$row', not one of parent a b c all"; return 1; }
    [[ "$seen" == *" $row "* ]] && { say "refused: ROWS names $row twice"; return 1; }
    seen+="$row "
  done
  local shown="aa (zero against its copy)" spec
  for row in $ROWS; do
    if [[ "$row" == parent ]]; then spec="parent (PARENT, unpatched)"
    else spec="$row (S123_COST=$(mask_of "$row"))"; fi
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
    1) say "refused: CAND ${cand:0:12} does not carry S123's family"; return 1 ;;
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
  git -C "$root" show "$cand:adocs/data/S123_cost_only.patch" > /dev/null 2>&1 ||
    { say "refused: CAND carries no adocs/data/S123_cost_only.patch"; return 1; }
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
    echo "S123 cost"
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
    if check_pair; then echo "S123-COST-CHECK-DONE"; else echo "S123-COST-CHECK-FAILED"; exit 1; fi ;;
  run)
    if run; then echo "S123-COST-DONE"; else echo "S123-COST-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 check | run" >&2
    exit 2 ;;
esac
