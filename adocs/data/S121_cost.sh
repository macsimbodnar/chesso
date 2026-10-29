#!/usr/bin/env bash
# S121's speed cost: what each of S121's four parts costs the search when its
# count is computed -- the enemy pawn attacks (p1), the mobility area with own
# blocked pawns excluded (p2a), the area with low-rank pawns excluded as well
# (p2b), the pins of both colours (p3), and all of them together (all) -- and
# what the
# table at identity costs against the linear term it replaced (parent). The
# parts are measured on cost-only builds, adocs/data/S121_parts.patch at
# S121_COST=<mask>: the part's count computed and thrown away behind an asm
# barrier while the score is the landing's, so the search is node for node
# the shipping one's and the wall time is the cost and nothing else
# (DEC-047's method; its forced-weight clause is moot here, the identity table
# being non-zero, so nothing folds).
#
#   CAND_SHA=<landing> PARENT_SHA=<its parent> adocs/data/S121_cost.sh check
#   CAND_SHA=<landing> PARENT_SHA=<its parent> \
#     nohup adocs/data/S121_cost.sh run > .tuning/s121_cost.console 2>&1 &
#   CAND_SHA=<landing> PARENT_SHA=<its parent> OUT=<a fresh directory> \
#     ROWS="p5" nohup adocs/data/S121_cost.sh run > CONSOLE 2>&1 &
#
# Run from the main checkout. CAND_SHA is S121's phase 1 landing, the table at
# identity (HEAD when unset); PARENT_SHA the tree before it (CAND^ when
# unset). `run` checks the pair first -- CAND carries the table, PARENT does
# not, PARENT is CAND^ -- and `check` does that alone. Prints S121-COST-DONE
# or S121-COST-FAILED (S121-COST-CHECK-DONE or -FAILED) as its last line, so
# a run goes detached under a four-exit watcher. OUT defaults to
# .tuning/s121/cost.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-28 in S121's phase 1a, blind, before any
# of it ran; S135_cost.sh's shape and its 2026-10-25 guards.
# ============================================================================
#
# WHAT IT DECIDES: NOTHING. The verdict carries the cost: the candidate's
# table is non-zero and its parts are computed in every game it plays
# (DEC-091's reading). The accepts' "the nps cost is recorded next to the
# verdict" is this run's reading, printed beside the SPRT and, on H0, beside
# each bisection part. It is not a speed verdict and moves no decision.
#
# THE BUILDS, all Release, native, padded (CHESSO_BRANCH_PAD; DEC-266 (1)),
# `--target chesso`, each from a throwaway worktree of CAND (or PARENT):
#   zero    CAND as it is: the table at identity, no part
#   copy    zero's binary byte for byte: the A/A
#   parent  PARENT, the linear term: zero against it is the table's own cost
#   p1      CAND + the patch, S121_COST=1: the enemy pawn attacks and their mask
#   p2a     S121_COST=2: the own blocked pawns, the area
#   p2b     S121_COST=10: the area with the own low-rank pawns as well, so
#           p2b less p2a is the low-rank exclusion's own cost
#   p3      S121_COST=4: both colours' pins and the pin-line mask a pinned
#           piece takes, src/bitboard.cpp pinned_pieces() among them
#   p5      S121_COST=5: p1 and p3 together, the shipped count's parts --
#           added 2026-10-29 at S121's close, below
#   all     S121_COST=15: the four together
#   Every patched build carries src/bitboard.cpp pinned_pieces(), called only
#   where bit 4 is set, so p1's code differs from zero's by that
#   function's layout too; DEC-266's floor and its instruction clause are what
#   read that, not this list. So do p2a's and p2b's.
#
# INV-6 FIRST. Every binary prints zero's `bench` total, `bench 12` lines and
# `tools/search_bench.py` node counts and best moves at depths 9 and 12, the
# timing fields stripped; any difference stops the run.
#
# THE CODE CHECK, before any timing: S121_cost.py code, the five forced and
# zero builds' .text sections pairwise distinct (six of them).
#
# THE METHOD, S270's as DEC-266 adopts it: 16 interleaved pairs per
# comparison, the order swapped every pair; build/tools/perf_counters
# (instructions and cycles, user space, the engine's process alone) on `chesso
# bench 16` and on pos11, S020's 300 positions at depth 11 through one engine
# process (S135_cost.py workload); hyperfine wall, one run of each side a pair,
# on pos11. Comparisons against zero: aa (copy), parent, p1, p2a, p2b, p3, all.
#
# THE READING, S121_cost.py read: per comparison and instrument the geometric
# mean ratio and its 95 % CI from the paired t. The floor is the A/A's largest
# CI bound per workload and instrument; a cost counts when its cycles CI on
# both workloads and its wall CI lie wholly beyond the floor the same way and
# its instructions move the same way (DEC-266 (3), (4)); otherwise zero, or
# owing (4)'s perturbations when instructions disagree. It refuses before any
# verdict unless each forced build's instructions lie above zero's, and all's
# above each part's, by more than the A/A's instruction spread.
#
# MACHINE TIME, from S135's run (adocs/data/S135_cost_reading.txt, its five
# comparisons): a comparison about 10.4 min, seven about 73 min; seven builds
# about 1 to 2 min each with ccache, the identity streams about 1 min. About
# 1 h 35 m in all, ceiling 3 h 10 m. Disk: seven worktrees of about 240 MB
# with their builds, 1.7 GB. The machine must be otherwise idle (MACHINE).
# ============================================================================
#
# 2026-10-29, S121's close: THE ROW MASK 5 (p5), added after the run above
# (adocs/data/S121_cost_reading.txt) and after S121's SPRT read H1 on the
# candidate at S121_PARTS 5, which the close folded into src/. That run timed
# no build computing what mask 5 computes; p5 is S121_COST=5, P1's and P3's
# sets forced together, and is read like every other row: INV-6 against zero,
# the code check, the A/A's floor, DEC-266's reading. It is nested inside all
# and holds p1 and p3, so the reading's ordering check gains all > p5, p5 > p1
# and p5 > p3 wherever both ends of a pair are timed. The other rows are as
# they were. ROWS names the rows a run times, default "parent p1 p2a p2b p3
# p5 all"; the A/A, zero, its copy and INV-6 are taken in every run, since
# the floor is the session's (DEC-266). `ROWS=p5` times aa and p5 alone,
# about 25 min (two builds, two comparisons, the streams), ceiling 50 min, in
# a fresh OUT: a run refuses an OUT that holds a reading.txt, since its pairs
# are the record a reading was taken from and `pairs` empties a comparison's
# directory before it times it. A build's stamp now names the patch by the
# sha256 of what `git apply` reads, from its first `--- ` line on, not of the
# whole file: the patch's preamble was edited twice after the run above (the
# candidate's commit and the close), each time leaving the applied text as it
# was, and the old stamp refused a rebuild of the same sources for that.
# Run from the main checkout after the fold, with CAND_SHA=12cab3c and
# PARENT_SHA=6ff7485 (CAND_SHA unset is HEAD, whose parent carries the table,
# and `check` refuses the pair): the cost builds are worktrees of the phase 1
# landing, which the patch applies to, and the fold changes nothing here.

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s121/cost}"
PAIRS=16
patch="$root/adocs/data/S121_parts.patch"
helper="$root/adocs/data/S121_cost.py"
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
      flags="-DS121_COST=$mask"
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

# 0 when COMMIT's src/evaluation.cpp carries S121's table, 1 when it does
# not, 2 when the file cannot be read there. Captured before it is searched
# (S135_cost.sh carries_columns: pipefail and SIGPIPE).
carries_table() {
  local text
  text=$(git -C "$root" show "$1:src/evaluation.cpp" 2> /dev/null) || return 2
  [[ "$text" == *mobility_queen_mg* ]]
}

ROWS="${ROWS:-parent p1 p2a p2b p3 p5 all}"

# The forced rows and their masks.
mask_of() {
  case "$1" in
    p1) echo 1 ;; p2a) echo 2 ;; p2b) echo 10 ;; p3) echo 4 ;; p5) echo 5 ;;
    all) echo 15 ;; *) return 1 ;;
  esac
}

check_rows() {
  local row seen=" "
  [[ -n "${ROWS// /}" ]] || { say "refused: ROWS names no row"; return 1; }
  for row in $ROWS; do
    [[ "$row" == parent ]] || mask_of "$row" > /dev/null ||
      { say "refused: ROWS names '$row', not one of parent p1 p2a p2b p3 p5 all"; return 1; }
    [[ "$seen" == *" $row "* ]] && { say "refused: ROWS names $row twice"; return 1; }
    seen+="$row "
  done
  local shown="aa (zero against its copy)" spec
  for row in $ROWS; do
    if [[ "$row" == parent ]]; then spec="parent (PARENT, unpatched)"
    else spec="$row (S121_COST=$(mask_of "$row"))"; fi
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
  carries_table "$cand"
  status=$?
  case $status in
    0) ;;
    1) say "refused: CAND ${cand:0:12} does not carry S121's table"; return 1 ;;
    *) say "refused: CAND ${cand:0:12} has no src/evaluation.cpp to read"; return 1 ;;
  esac
  carries_table "$parent"
  status=$?
  case $status in
    1) ;;
    0) say "refused: PARENT ${parent:0:12} carries the table already"; return 1 ;;
    *) say "refused: PARENT ${parent:0:12} has no src/evaluation.cpp to read"; return 1 ;;
  esac
  if [[ "$(git -C "$root" rev-parse --verify --quiet "$cand^")" != "$parent" ]]; then
    say "refused: PARENT ${parent:0:12} is not CAND^"
    return 1
  fi
  git -C "$root" show "$cand:adocs/data/S121_parts.patch" > /dev/null 2>&1 ||
    { say "refused: CAND carries no adocs/data/S121_parts.patch"; return 1; }
  say "pair      CAND carries the table, PARENT does not, PARENT is CAND^"
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
    echo "S121 cost"
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
    streams "$OUT/bin/$bin" "$OUT/inv6/$bin"
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
    if check_pair; then echo "S121-COST-CHECK-DONE"; else echo "S121-COST-CHECK-FAILED"; exit 1; fi ;;
  run)
    if run; then echo "S121-COST-DONE"; else echo "S121-COST-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 check | run" >&2
    exit 2 ;;
esac
