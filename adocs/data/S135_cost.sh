#!/usr/bin/env bash
# S135's speed cost: what the seven piece placement columns cost the search
# when they are computed, which at the zero weights S135 lands with they are
# not -- the compiler deletes a zero-weighted term (DEC-047). Measured on a
# cost-only build, adocs/data/S135_cost_only.patch: the counts weighed at
# forced weights and thrown away behind an asm barrier, so the search is node
# for node the shipping one's and the wall time is the cost and nothing else.
#
#   CAND_SHA=<landing> PARENT_SHA=<its parent> adocs/data/S135_cost.sh check
#   CAND_SHA=<landing> PARENT_SHA=<its parent> \
#     nohup adocs/data/S135_cost.sh run > .tuning/s135_cost.console 2>&1 &
#
# Run from the main checkout. The pair is explicit since 2026-10-25 (a fix
# commit on top of the landing moves HEAD): CAND_SHA is S135's zero-weight
# landing, HEAD when unset; PARENT_SHA the tree before it, CAND^ when unset
# (HEAD^ when neither is set). `run` checks the pair before anything else --
# CAND carries the four columns, PARENT does not, PARENT is CAND^ -- and
# `check` does that alone, building and timing nothing. Prints S135-COST-DONE or
# S135-COST-FAILED (S135-COST-CHECK-DONE or -FAILED) as its last line, so a
# run goes detached under a four-exit watcher. OUT defaults to
# .tuning/s135/cost.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-22 in S135's phase 1, blind, before any
# of it ran.
# ============================================================================
#
# WHAT IT DECIDES: NOTHING. The verdict carries the cost: S135_sprt.sh's
# candidate has the fitted, non-zero weights, so the compiler keeps the term
# and every game pays for it (DEC-091, DEC-047). This timing is owed as the
# step's record of what that verdict paid, read before the SPRT is reported
# and printed beside it, and on H0 beside each bisection part, where a cost
# concentrated in one half is what the split would be read with. It is not a
# speed verdict and moves no decision.
#
# THE BUILDS, all Release, native, padded (CHESSO_BRANCH_PAD, S273's default
# on this machine; DEC-266 (1)), `--target chesso`, each from a throwaway
# worktree of CAND (or PARENT) under $OUT:
#   zero    CAND as it is: the seven at zero, deleted
#   copy    zero's binary copied byte for byte: the A/A
#   parent  PARENT, the tree before S135: zero against it is the check that
#           the landing is free, the compiler deleting the four new columns as
#           it deletes the three -- instructions equal within the A/A's spread
#   all     CAND + the patch, S135_COST_MASK=0x7f: the seven forced
#   old3    the same at 0x07: S027's three forced, bisection part A
#   new4    the same at 0x78: S135's four forced, bisection part B
#
# INV-6 FIRST. Every binary prints zero's `bench` total, `bench 12` lines and
# `tools/search_bench.py` node counts and best moves at depths 9 and 12, the
# timing fields stripped; any difference stops the run, since a timing of two
# different trees measures the trees.
#
# THE METHOD, S270's as DEC-266 adopts it: 16 interleaved pairs per
# comparison, the order swapped every pair; build/tools/perf_counters
# (instructions and cycles, user space, the engine's process alone) on `chesso
# bench 16` and on pos11, S020's 300 positions at depth 11 through one engine
# process (S135_cost.py workload); hyperfine wall, one run of each side a pair,
# on pos11. Comparisons against zero: aa (copy), parent, all, old3, new4.
#
# THE READING, S135_cost.py read: per comparison and instrument the geometric
# mean ratio and its 95 % CI from the paired t. The floor is the A/A's largest
# CI bound per workload and instrument; a cost counts when its cycles CI on
# both workloads and its wall CI lie wholly beyond the floor the same way and
# its instructions move the same way (DEC-266 (3), (4)); otherwise it is
# recorded as zero, or as owing (4)'s two perturbations when instructions
# disagree. The parent comparison reads zero when the landing is free.
#
# MACHINE TIME, from S273's measured rate (adocs/data/S273_timing.txt, the
# base's median 4547095 nps on `bench 16`) and the node counts S083's weights
# give, read at S135's rebase: `bench 16` 11370791 nodes (S273's tree
# 10214429), about 2.5 s a run; pos11 38413692 nodes (S273's about 26.5 M),
# about 8.5 s. A pair of counter runs on both workloads about 22 s, a wall
# pair about 17 s: a comparison about 10.4 min, five 52 min. Six builds about
# 1 to 2 min each with ccache (the 0x7f build, evaluation.cpp and the link,
# under 2 min at the rebase), the identity streams about 1 min. About 1 h 05 m
# in all, ceiling 2 h 30 m. Disk: five worktrees of about 240 MB with their
# builds, 1.2 GB. The machine must be otherwise idle (MACHINE): a match, a fit
# or datagen running beside it makes the floor meaningless.
# ============================================================================
#
# AMENDED 2026-10-25, after run 1 (the step file's section "2026-10-25: the
# cost run's forced builds did not separate"). The method and the reading
# above are unchanged; the patch and three guards are new. Run 1's 0x78
# build was its 0x7f build's code to the byte: the patch's single barrier
# over the whole dot product let g++ 13.3 vectorise columns 0 to 3 into one
# vpmulld against a constant vector, a zero lane still multiplied, so new4
# computed the seven and old3 computed PL_BAD_BISHOP beside its three. The
# patch now hands each forced column to a barrier of its own and never reads
# a clear one. The guards:
#   stamp   a binary under $OUT/bin is reused only when $OUT/bin/NAME.stamp
#           names the commit, the mask and the patch's sha256 it would be
#           built from now, and a tree under $OUT is built only when its HEAD
#           is that commit and its files are the commit's, or the commit's
#           with this patch, exactly; run 1's binaries carry no stamp and its
#           patched trees the old patch, so a re-run takes a fresh OUT and
#           cannot time the old builds
#   code    before any timing, S135_cost.py code: zero, all, old3 and new4
#           four distinct .text sections, and the forced builds' popcnt
#           counts in evaluate_cheap() beyond zero's adding up, old3 + new4 =
#           all, which a part computing a column outside its mask breaks
#   read    S135_cost.py read refuses before any verdict unless, on both
#           workloads, each forced build's instructions lie above zero's and
#           all's above old3's and new4's by more than the A/A's instruction
#           spread; it prints, ungated, the parts' sum against all's. Coarse:
#           run 1's old3 overlap passes it, and only `code` refuses that
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s135/cost}"
PAIRS=16
patch="$root/adocs/data/S135_cost_only.patch"
helper="$root/adocs/data/S135_cost.py"
pc="$root/build/tools/perf_counters"
python_chess="$HOME/.venv/chess/bin/python"

say() { echo "$*"; }

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
  [[ -n "$mask" ]] && stamp="$commit $mask $(sha256sum "$patch" | cut -c1-64)"
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
      flags="-DS135_COST_MASK=$mask"
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

# The timing-stripped INV-6 streams of one binary.
streams() {
  local bin="$1" dir="$2" d
  mkdir -p "$dir"
  strip() { sed -E 's/ time [0-9]+//; s/ nps [0-9]+//; s/ hashfull [0-9]+//; s/^([0-9]+ nodes) [0-9]+ nps/\1/'; }
  "$bin" bench 2>&1 | strip > "$dir/bench.txt"
  "$bin" bench 12 2>&1 | strip > "$dir/bench12.txt"
  for d in 9 12; do
    "$python_chess" "$root/tools/search_bench.py" "$bin" "$d" 2>&1 |
      awk '{print $1, $3, $4, $(NF)}' | grep -v total > "$dir/sb$d.txt"
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
        python3 "$helper" workload "$bin" 11 > "$dir/pos11/${who}_$k.out" 2>&1 ||
        { say "$name pos11 $who $k failed"; return 1; }
    done
    if (( k % 2 == 1 )); then
      hyperfine -N -w 0 -r 1 --export-json "$dir/wall/pair_$k.json" \
          -n base "python3 $helper workload $base 11" \
          -n cand "python3 $helper workload $cand 11" > /dev/null 2>&1
    else
      hyperfine -N -w 0 -r 1 --export-json "$dir/wall/pair_$k.json" \
          -n cand "python3 $helper workload $cand 11" \
          -n base "python3 $helper workload $base 11" > /dev/null 2>&1
    fi || { say "$name wall pair $k failed"; return 1; }
  done
  say "$name: $PAIRS pairs"
}

# 0 when COMMIT's src/evaluation.cpp names PL_TRAPPED_ROOK, 1 when it does
# not, 2 when the file cannot be read there. The text is captured before it is
# searched: under pipefail `git show ... | grep -q` reads 141 whenever grep
# exits on its first match before git show has written the file, git show
# dying of SIGPIPE, so the landing was refused as not carrying the columns it
# carries, and a parent that carried them would have passed (2026-10-25).
carries_columns() {
  local text
  text=$(git -C "$root" show "$1:src/evaluation.cpp" 2> /dev/null) || return 2
  [[ "$text" == *PL_TRAPPED_ROOK* ]]
}

# The pair, resolved to full shas into $cand and $parent, and checked: CAND
# carries S135's columns, PARENT does not, PARENT is CAND^. Otherwise the
# parent build measures something other than the landing.
cand=""
parent=""
check_pair() {
  local status
  cand=$(git -C "$root" rev-parse --verify --quiet "${CAND_SHA:-HEAD}^{commit}") ||
    { say "refused: CAND_SHA '${CAND_SHA:-HEAD}' names no commit"; return 1; }
  parent=$(git -C "$root" rev-parse --verify --quiet "${PARENT_SHA:-$cand^}^{commit}") ||
    { say "refused: PARENT_SHA '${PARENT_SHA:-CAND^}' names no commit"; return 1; }
  say "pair      cand $cand, parent $parent"
  carries_columns "$cand"
  status=$?
  case $status in
    0) ;;
    1) say "refused: CAND ${cand:0:12} does not carry S135's columns (no PL_TRAPPED_ROOK)"; return 1 ;;
    *) say "refused: CAND ${cand:0:12} has no src/evaluation.cpp to read"; return 1 ;;
  esac
  carries_columns "$parent"
  status=$?
  case $status in
    1) ;;
    0) say "refused: PARENT ${parent:0:12} carries PL_TRAPPED_ROOK already, so it is not the tree before S135"; return 1 ;;
    *) say "refused: PARENT ${parent:0:12} has no src/evaluation.cpp to read"; return 1 ;;
  esac
  if [[ "$(git -C "$root" rev-parse --verify --quiet "$cand^")" != "$parent" ]]; then
    say "refused: PARENT ${parent:0:12} is not CAND^, so the pair would measure more than S135's landing"
    return 1
  fi
  say "pair      CAND carries the four columns, PARENT does not, PARENT is CAND^"
}

run() {
  mkdir -p "$OUT" || return 1
  [[ -x "$pc" ]] || { say "no $pc; build the tools first"; return 1; }
  command -v hyperfine > /dev/null || { say "no hyperfine"; return 1; }
  command -v objdump > /dev/null && command -v objcopy > /dev/null ||
    { say "no objdump or objcopy, which the masks' code check reads with"; return 1; }
  check_pair || return 1
  {
    echo "S135 cost"
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
  build_one parent "$parent" | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  local name mask
  for spec in all:0x7f old3:0x07 new4:0x78; do
    name=${spec%%:*}; mask=${spec#*:}
    build_one "$name" "$cand" "$mask" | tee -a "$OUT/cost.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  done

  # INV-6 before any timing: every binary searches zero's tree.
  local bin
  for bin in zero parent all old3 new4; do
    streams "$OUT/bin/$bin" "$OUT/inv6/$bin"
  done
  for bin in parent all old3 new4; do
    if ! diff -r -q "$OUT/inv6/zero" "$OUT/inv6/$bin" > /dev/null; then
      diff -r "$OUT/inv6/zero" "$OUT/inv6/$bin" | head -20
      say "INV-6: $bin does not search zero's tree; no timing is taken"
      return 1
    fi
  done
  say "INV-6: bench $(tail -n 1 "$OUT/inv6/zero/bench.txt"), every binary identical" |
    tee -a "$OUT/cost.log"

  # Each mask built the code it names, before an hour of timing is spent on it.
  python3 "$helper" code "$OUT/bin" | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || { say "the forced builds do not compute what their masks name; no timing is taken"; return 1; }

  echo "loadavg before pairs: $(cat /proc/loadavg)" >> "$OUT/cost.log"
  pairs aa copy | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  for bin in parent all old3 new4; do
    pairs "$bin" "$bin" | tee -a "$OUT/cost.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  done
  echo "loadavg after pairs: $(cat /proc/loadavg)" >> "$OUT/cost.log"

  python3 "$helper" read "$OUT/pairs" | tee "$OUT/reading.txt"
  (( PIPESTATUS[0] == 0 )) || return 1
  echo "finished  $(date -Iseconds)" >> "$OUT/cost.log"
  say "the five worktrees under $OUT stay for a re-run; git worktree remove each when the reading is recorded"
}

case "$stage" in
  check)
    if check_pair; then echo "S135-COST-CHECK-DONE"; else echo "S135-COST-CHECK-FAILED"; exit 1; fi ;;
  run)
    if run; then echo "S135-COST-DONE"; else echo "S135-COST-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 check | run" >&2
    exit 2 ;;
esac
