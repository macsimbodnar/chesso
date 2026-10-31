#!/usr/bin/env bash
# S125's speed cost: what each of S125's seven groups costs the search when it
# is computed -- P1, the isolated and backward increment (p1); P2, isolated
# pawns by file (p2); P3, doubled pawns by file (p3); P4, backward pawns not
# isolated by rank (p4); P5, phalanx pawns (p5); P6, supported pawns (p6); P7,
# weak unopposed pawns (p7); and the seven together (all) -- and that the
# zero-weight landing costs nothing against the tree before it (parent). P0,
# the three flats refitted, has no row: the flats are computed today and a
# refit moves no instruction. At zero the compiler never sees the groups
# (src/evaluation.cpp, the liveness over each block in evaluate_pawns()), so
# each is measured on a cost-only build, adocs/data/S125_cost_only.patch at
# S125_COST=<mask>: the group computed at forced non-zero weights into a sink
# an asm barrier consumes, while the score is the landing's, so the search is
# node for node the shipping one's and the wall time is the cost and nothing
# else (DEC-047's forced-weight method, S135's and S123's cost form, DEC-331
# (12)).
#
#   CAND_SHA=<landing> PARENT_SHA=<its parent> adocs/data/S125_cost.sh check
#   CAND_SHA=<landing> PARENT_SHA=<its parent> \
#     nohup adocs/data/S125_cost.sh run > .tuning/s125_cost.console 2>&1 &
#
# Run from the main checkout. CAND_SHA is S125's phase 1 landing, the family
# at zero (HEAD when unset); PARENT_SHA the tree before it (CAND^ when unset).
# `run` checks the pair first -- CAND carries the family and the patch,
# PARENT does not carry the family, PARENT is CAND^, the patch applies to CAND
# -- and `check` does that alone. Prints S125-COST-DONE or S125-COST-FAILED
# (S125-COST-CHECK-DONE or -FAILED) as its last line, so a run goes detached
# under a four-exit watcher. OUT defaults to .tuning/s125/cost.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-30 in S125's phase 1, before any of it
# ran; S123_cost.sh's shape with its two fixes (the streams' commands checked,
# the raw timings kept out of the compared directory).
# ============================================================================
#
# WHAT IT DECIDES: NOTHING. The verdict carries the cost: the candidate's
# survivors are non-zero and computed in every game it plays (DEC-091's
# reading, DEC-331 (12)). This run's reading is recorded beside the verdict
# and, on H0, beside each bisection part, where a cost concentrated in one
# group is what the split is read with; it is also the per-call baseline S118
# is later asked to reclaim (the accepts). It is not a speed verdict.
#
# THE BUILDS, all Release, native, padded (CHESSO_BRANCH_PAD; DEC-266 (1)),
# `--target chesso`, each from a throwaway worktree of CAND (or PARENT):
#   zero    CAND as it is: the family at zero, in no expression
#   copy    zero's binary byte for byte: the A/A
#   parent  PARENT, the tree before S125: zero against it is the check that
#           the landing is free -- instructions equal within the A/A's spread
#   p1..p7  CAND + the patch, S125_COST=1, 2, 4, 8, 16, 32, 64: one group
#           forced
#   all     S125_COST=127: the seven together
#   cand    S125_COST=114: P2, P5, P6 and P7, the set the candidate computes,
#           added 2026-10-31 (below)
#   Phase 1 built every mask in a scratch copy of the landing: bench 5434393
#   at each, nine distinct .text sections with zero's, and evaluate_pawns()'s
#   out-of-line clone grows from 699 instructions to 899 at mask 127
#   (adocs/data/S125_objdiff.py).
#
# INV-6 FIRST. Every binary prints zero's `bench` total, `bench 12` lines and
# `tools/search_bench.py` node counts and best moves at depths 9 and 12, the
# timing fields stripped; any difference stops the run.
#
# THE CODE CHECK, before any timing: S125_cost.py code, the forced and zero
# builds' .text sections pairwise distinct.
#
# THE METHOD, S270's as DEC-266 adopts it: 16 interleaved pairs per
# comparison, the order swapped every pair; build/tools/perf_counters
# (instructions and cycles, user space, the engine's process alone) on `chesso
# bench 16` and on pos11, S020's 300 positions at depth 11 through one engine
# process (S135_cost.py workload); hyperfine wall, one run of each side a pair,
# on pos11. Comparisons against zero: aa (copy), parent, p1 to p7, all.
#
# THE READING, S125_cost.py read: per comparison and instrument the geometric
# mean ratio and its 95 % CI from the paired t. The floor is the A/A's largest
# CI bound per workload and instrument; a cost counts when its cycles CI on
# both workloads and its wall CI lie wholly beyond the floor the same way and
# its instructions move the same way (DEC-266 (3), (4)); otherwise zero, or
# owing (4)'s perturbations when instructions disagree. It refuses before any
# verdict unless each forced build's instructions lie above zero's, and all's
# above each group's, by more than the A/A's instruction spread; p1 + ... + p7
# against all is printed, not gated. `bench_eval`'s ns per call may be printed
# beside it for S118; it decides nothing either.
#
# MACHINE TIME, from S123's run (adocs/data/S123_cost_reading.txt: six
# comparisons -- aa, parent, a, b, c, all -- six builds and their identity
# streams in 67 min, 03:29:42 to 04:37): about 11.2 min a comparison with its
# build and streams. Here ten comparisons -- aa, parent, p1 to p7, all -- and
# ten builds -- zero, parent, p1 to p7, all; the copy is zero's bytes -- about
# 1 h 55 m in all, ceiling 4 h. Disk: ten worktrees of about 240 MB with their
# builds, 2.4 GB.
# The machine must be otherwise idle (MACHINE): a match, a fit or datagen
# beside it makes the floor meaningless.
# ============================================================================
#
# 2026-10-31, S125's phase 2: THE ROW `cand`, added after the run above
# (adocs/data/S125_cost_reading.txt). The screen kept P2, P5, P6 and P7 and
# dropped P1, P3 and P4, so `all` forces three groups the candidate does not
# compute, and that run timed no build computing what the candidate computes.
# `cand` is S125_COST=114, those four groups forced together: the candidate's
# set, which holds for CAND and not after the close. The close's fold (DEC-331
# (3), (15)) makes isolated_backward live, so the engine then also computes P1
# and the `& ~isolated`; the fold adds P1's forced row, +0.37 % instructions,
# read zero with its cycles and wall CIs reaching inside the A/A floor, and
# S118's baseline is read with both rows. `cand` is read like every other row:
# INV-6 against zero, the code check, the A/A's floor, DEC-266's reading. It is
# nested inside all and holds p2, p5, p6 and p7, so the reading's ordering check
# gains all > cand and cand > each of the four wherever both ends of a pair are
# timed. The
# other rows are as they were. ROWS names the rows a run times, default "parent
# p1 p2 p3 p4 p5 p6 p7 cand all"; the A/A, zero, its copy and INV-6 are taken in
# every run, since the floor is the session's (DEC-266). `ROWS=cand` times aa
# and cand alone, about 25 min (two builds, two comparisons, the streams; S121's
# one-row run took 23 min), ceiling 50 min, in a fresh OUT: a run refuses an OUT
# that holds a reading.txt.

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s125/cost}"
PAIRS=16
patch="$root/adocs/data/S125_cost_only.patch"
helper="$root/adocs/data/S125_cost.py"
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
      flags="-DS125_COST=$mask"
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

# 0 when COMMIT's src/evaluation.cpp carries S125's family, 1 when it does
# not, 2 when the file cannot be read there. Captured before it is searched
# (S135_cost.sh carries_columns: pipefail and SIGPIPE).
carries_family() {
  local text
  text=$(git -C "$root" show "$1:src/evaluation.cpp" 2> /dev/null) || return 2
  [[ "$text" == *structure_family_counts* ]]
}

ROWS="${ROWS:-parent p1 p2 p3 p4 p5 p6 p7 cand all}"

# The forced rows and their masks.
mask_of() {
  case "$1" in
    p1) echo 1 ;; p2) echo 2 ;; p3) echo 4 ;; p4) echo 8 ;;
    p5) echo 16 ;; p6) echo 32 ;; p7) echo 64 ;; cand) echo 114 ;;
    all) echo 127 ;;
    *) return 1 ;;
  esac
}

check_rows() {
  local row seen=" "
  [[ -n "${ROWS// /}" ]] || { say "refused: ROWS names no row"; return 1; }
  for row in $ROWS; do
    [[ "$row" == parent ]] || mask_of "$row" > /dev/null ||
      { say "refused: ROWS names '$row', not one of parent p1 p2 p3 p4 p5 p6 p7 cand all"; return 1; }
    [[ "$seen" == *" $row "* ]] && { say "refused: ROWS names $row twice"; return 1; }
    seen+="$row "
  done
  local shown="aa (zero against its copy)" spec
  for row in $ROWS; do
    if [[ "$row" == parent ]]; then spec="parent (PARENT, unpatched)"
    else spec="$row (S125_COST=$(mask_of "$row"))"; fi
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
    1) say "refused: CAND ${cand:0:12} does not carry S125's family"; return 1 ;;
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
  git -C "$root" show "$cand:adocs/data/S125_cost_only.patch" > /dev/null 2>&1 ||
    { say "refused: CAND carries no adocs/data/S125_cost_only.patch"; return 1; }
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
    echo "S125 cost"
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
    if check_pair; then echo "S125-COST-CHECK-DONE"; else echo "S125-COST-CHECK-FAILED"; exit 1; fi ;;
  run)
    if run; then echo "S125-COST-DONE"; else echo "S125-COST-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 check | run" >&2
    exit 2 ;;
esac
