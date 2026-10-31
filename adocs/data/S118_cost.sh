#!/usr/bin/env bash
# S118's speed verdict: what the pawn table buys at each size, and what the
# shelter slot adds to it. Identical trees by INV-6, so this is the verdict
# itself and no SPRT follows it (DEC-332 (6), DEC-083).
#
#   CACHE_SHA=<the cache's commit> SHELTER_SHA=<the slot's commit> \
#     adocs/data/S118_cost.sh check
#   CACHE_SHA=<the cache's commit> SHELTER_SHA=<the slot's commit> \
#     adocs/data/S118_cost.sh build
#   CACHE_SHA=<the cache's commit> SHELTER_SHA=<the slot's commit> \
#     nohup adocs/data/S118_cost.sh run > .tuning/s118_cost.console 2>&1 &
#
# Run from the checkout holding this file, branch s118's: DEC-332 (9) has the
# timing come before any landing, so both commits sit on that branch and neither
# is on achesso. No commit in this checkout while it runs. The base is CACHE^.
# `check` holds the commits: CACHE carries the table at 2^12 and its parent does
# not; SHELTER carries the slot and CACHE does not; CACHE is SHELTER^. `build` does
# that, every build, INV-6 and the code check, and times nothing. `run` does what
# `build` does, reusing its binaries, and then the pairs. Each prints
# S118-COST-CHECK-, S118-COST-BUILD- or S118-COST-(DONE|FAILED) as its last line,
# so a run goes detached under a four-exit watcher. OUT defaults to
# .tuning/s118/cost. ROWS names what a run times, default all; the A/A and INV-6 are
# taken in every run, since the floor is the session's (DEC-266). A run refuses an
# OUT that holds a reading.txt.
#
# ============================================================================
# PRE-REGISTRATION -- written 2026-10-31 in S118's phase B, before any of it ran.
# ============================================================================
#
# WHAT IT DECIDES, by DEC-332, and nothing else:
#   (7) the size: the smallest of s8 to s16 whose cycles on both workloads lie
#       within the floor of the best row. s18 is informative, never a candidate
#       (the step file: a table past the last-level cache is the published
#       failure, and 8 MiB is two thirds of this machine's 12).
#   (8) the bar: DEC-266 (3), (4) and no other. The cache ships if the chosen
#       size's cycles on both workloads and its wall clear the floor as a gain and
#       its instructions move the same way. Zero or a loss: not shipped, and
#       DEC-046 stands, with S122's accepts amended to read the shelter from stage
#       two (DEC-332 (1)). No margin beyond the floor: DEC-332 rejected one.
#   (5) the shelter: it stays if `slot` does not read a loss beyond the floor, S122
#       being the reason; a loss drops it and S122 decides.
# What was predicted, and it is no reading: a scratch prototype read -8.42 %
# instructions on bench 16 and -8.87 % on pos11 at 2^12, with -0.43 % and -0.63 %
# more from the slot. DEC-046 read zero at an 89.8 % hit rate. Cycles are this
# run's to say. DEC-083 converts a credited gain to Elo, named as a conversion.
#
# THE BUILDS, all Release, native, padded (CHESSO_BRANCH_PAD; DEC-266 (1)), one
# cmake line and `--target chesso`, each from a throwaway worktree:
#   base     CACHE^, the tree before the table
#   copy     base's binary byte for byte: the A/A
#   s12      CACHE as it is: PAWN_TABLE_BITS = 12, 128 KiB
#   s8 s10 s14 s16 s18   CACHE with that one line changed in the worktree: 8 KiB,
#            32 KiB, 512 KiB, 2 MiB, 8 MiB
#   shelter  SHELTER as it is, at 12: 128 KiB and 64 KiB of slots
# **A size row differs by the table's size alone.** Same commit, same cmake line,
# no define: `git diff --numstat` in the row's tree must read exactly one line
# added and one removed in src/data_structures.hpp, and that diff goes in the log.
# Then S118_cost.py code compares each size row's objects with s12's, function by
# function, prints the lines that differ, and refuses a difference outside what
# reads the table's size. Taken on the two commits by `build`, 2026-10-31, before any
# timing: seven functions at every size, the two forms' mask (`and $0xfff,%eax`; at
# s8 and s16 a narrower load in its place, one instruction fewer) and the struct's
# size in iterative_deepening_search and leaf_walker_t, and nothing else.
#
# INV-6 FIRST. Every binary prints base's `bench` total, `bench 12` lines and
# `tools/search_bench.py` node counts and best moves at depths 9 and 12, the
# timing fields stripped; any difference stops the run: that is a defect, and it
# is reported, not read.
#
# THE METHOD, S270's as DEC-266 adopts it: 16 interleaved pairs per comparison,
# the order swapped every pair; build/tools/perf_counters (instructions and
# cycles, user space, the engine's process alone) on `chesso bench 16` and on
# pos11, S020's 300 positions at depth 11 through one engine process (S135_cost.py
# workload); hyperfine wall, one run of each side a pair, on pos11. Comparisons:
#   aa      base against copy
#   s8 s10 s12 s14 s16 s18   base against each size: the cache over its parent
#   total   base against shelter: s12 and the slot over their parent
#   slot    s12 against shelter: the slot's own
#
# THE READING, S118_cost.py read: per comparison and instrument the geometric mean
# ratio and its 95 % CI from the paired t. The floor is the A/A's largest CI bound
# per workload and instrument; a row counts when its cycles CI on both workloads
# and its wall CI lie wholly beyond the floor the same way and its instructions
# move the same way (DEC-266 (3), (4)); otherwise zero, or owing (4)'s
# perturbations when instructions disagree. It prints (7), (8) and (5) applied.
# bench_eval is not read: it calls the pure form, which has no table, and DEC-046
# and DEC-332 say why it could not price one.
#
# OPEN FINDINGS (DEC-171): none at 85b7847 but S304's named residuals, which sit in
# its step file: SIGTERM to depth_vs_eval.py's main process, reads without a
# timeout, `run`'s marker past the load. Tools, none in anything this run executes.
#
# MACHINE TIME, from S125's run (adocs/data/S125_cost_reading.txt: ten comparisons
# and ten builds in 1 h 58 m, 03:44:37 to 05:43:05): about 12 min a comparison with
# its build and streams. Here nine comparisons and eight builds, about 1 h 50 m;
# under DEC-155's four hours, so it starts when ready. Watcher ceiling 4 h,
# terminal marker `S118-COST-(DONE|FAILED)`. Disk: eight worktrees of about 240 MB
# with their builds, 1.9 GB. The machine must be otherwise idle (MACHINE): a match,
# a fit or datagen beside it makes the floor meaningless.
# ============================================================================

set -uo pipefail

stage="${1:-}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${OUT:-$root/.tuning/s118/cost}"
PAIRS=16
helper="$root/adocs/data/S118_cost.py"
workload="$root/adocs/data/S135_cost.py"
pc="$root/build/tools/perf_counters"
python_chess="$HOME/.venv/chess/bin/python"
size_file="src/data_structures.hpp"
size_line="inline constexpr int PAWN_TABLE_BITS = 12;"

say() { echo "$*"; }

ROWS="${ROWS:-s8 s10 s12 s14 s16 s18 total slot}"

# The table's bits of a size row, or nothing for a row that is not one.
bits_of() {
  case "$1" in
    s8) echo 8 ;; s10) echo 10 ;; s14) echo 14 ;; s16) echo 16 ;; s18) echo 18 ;;
    *) return 1 ;;
  esac
}

check_rows() {
  local row seen=" "
  [[ -n "${ROWS// /}" ]] || { say "refused: ROWS names no row"; return 1; }
  for row in $ROWS; do
    case "$row" in
      s8|s10|s12|s14|s16|s18|total|slot) ;;
      *) say "refused: ROWS names '$row', not one of s8 s10 s12 s14 s16 s18 total slot"; return 1 ;;
    esac
    [[ "$seen" == *" $row "* ]] && { say "refused: ROWS names $row twice"; return 1; }
    seen+="$row "
  done
  say "rows      aa $ROWS"
}

# The binaries the rows need, base always: a row's two sides.
binaries_needed() {
  local row need=" base "
  for row in $ROWS; do
    case "$row" in
      total) need+="shelter " ;;
      slot) need+="s12 shelter " ;;
      *) need+="$row " ;;
    esac
  done
  # shellcheck disable=SC2086  # a list of names
  printf '%s\n' $need | awk '!seen[$0]++'
}

# 0 when the commit's file carries the text, 1 when not, 2 when unreadable.
# Captured before it is searched (S135_cost.sh carries_columns: pipefail and
# SIGPIPE).
carries() {
  local text
  text=$(git -C "$root" show "$1:$size_file" 2> /dev/null) || return 2
  [[ "$text" == *"$2"* ]]
}

parent=""
cache=""
shelter=""
check_commits() {
  [[ -n "${CACHE_SHA:-}" && -n "${SHELTER_SHA:-}" ]] ||
    { say "refused: CACHE_SHA and SHELTER_SHA name the two commits"; return 1; }
  cache=$(git -C "$root" rev-parse --verify --quiet "$CACHE_SHA^{commit}") ||
    { say "refused: CACHE_SHA '$CACHE_SHA' names no commit"; return 1; }
  shelter=$(git -C "$root" rev-parse --verify --quiet "$SHELTER_SHA^{commit}") ||
    { say "refused: SHELTER_SHA '$SHELTER_SHA' names no commit"; return 1; }
  parent=$(git -C "$root" rev-parse --verify --quiet "$cache^") ||
    { say "refused: CACHE ${cache:0:12} has no parent"; return 1; }
  say "commits   base $parent, cache $cache, shelter $shelter"
  [[ "$(git -C "$root" rev-parse --verify --quiet "$shelter^")" == "$cache" ]] ||
    { say "refused: CACHE ${cache:0:12} is not SHELTER^"; return 1; }
  carries "$cache" "$size_line" ||
    { say "refused: CACHE ${cache:0:12} does not read '$size_line'"; return 1; }
  carries "$shelter" "$size_line" ||
    { say "refused: SHELTER ${shelter:0:12} does not read '$size_line'"; return 1; }
  if carries "$parent" "pawn_table_t"; then
    say "refused: CACHE^ ${parent:0:12} carries the table already"
    return 1
  fi
  carries "$shelter" "pawn_shelter_t" ||
    { say "refused: SHELTER ${shelter:0:12} does not carry the slot"; return 1; }
  if carries "$cache" "pawn_shelter_t"; then
    say "refused: CACHE ${cache:0:12} carries the slot already"
    return 1
  fi
  say "commits   the table is CACHE's, the slot SHELTER's, CACHE is SHELTER^"
  check_rows
}

# A worktree of COMMIT at $OUT/tree-NAME, its one size line changed when asked,
# built Release into its own build directory; the binary copied to $OUT/bin/NAME,
# and beside it NAME.stamp: what it was built from. A binary already there is
# reused only when its stamp is the one this build would write. A tree already
# there is built only when its HEAD is COMMIT and its diff is exactly what this
# row wants. Anything else is refused, never cleaned: the old tree may be a run's
# record.
build_one() {
  local name="$1" commit="$2" bits="${3:-}" tree="$OUT/tree-$1"
  local stamp="$commit bits ${bits:-12}" had head numstat want=""
  [[ -n "$bits" ]] && want=$'1\t1\t'"$size_file"
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
    else
      git -C "$root" worktree add -q --detach "$tree" "$commit" || return 1
    fi
    if [[ -n "$bits" ]]; then
      sed -i "s/^$size_line\$/inline constexpr int PAWN_TABLE_BITS = $bits;/" "$tree/$size_file" ||
        return 1
    fi
    numstat=$(git -C "$tree" diff --numstat HEAD)
    if [[ "$numstat" != "$want" ]]; then
      say "refused: $tree differs from $commit by '$numstat', and $name wants '${want:-nothing}'"
      return 1
    fi
    [[ -n "$bits" ]] && git -C "$tree" diff -U0 HEAD | grep -E '^[-+]inline' | tee "$OUT/$name.size_diff"
    cmake -S "$tree" -B "$tree/build" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        > "$OUT/cfg_$name.log" 2>&1 || { say "configure $name failed"; return 1; }
    cmake --build "$tree/build" --target chesso -j12 > "$OUT/build_$name.log" 2>&1 ||
      { tail -20 "$OUT/build_$name.log"; say "build $name failed"; return 1; }
    grep -q '^CHESSO_BRANCH_PAD:INTERNAL=ON$' "$tree/build/CMakeCache.txt" ||
      { say "$name is not padded (CHESSO_BRANCH_PAD), and DEC-266 (1) wants both sides padded"; return 1; }
    mkdir -p "$OUT/bin" && cp "$tree/build/src/chesso" "$OUT/bin/$name" || return 1
    echo "$stamp" > "$OUT/bin/$name.stamp" || return 1
  fi
  say "$name $(sha256sum "$OUT/bin/$name" | cut -c1-16) $commit ${bits:-12}"
}

# The timing-stripped INV-6 streams of one binary. Each command's status is
# checked and each bench must end on its total line: a bench or search_bench.py
# failing the same way for every binary would otherwise write the same error text
# everywhere and pass the diff below. DIR holds the four stripped streams and
# nothing else, since `run` compares whole directories; search_bench.py's raw
# output, timings and knps included, goes to a sibling tree nothing compares.
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

# 16 interleaved pairs of one comparison, base and cand by binary name.
pairs() {
  local name="$1" base="$OUT/bin/$2" cand="$OUT/bin/$3" dir="$OUT/pairs/$1" k who bin
  # perf_counters appends to its -o file, so a re-run over an old directory would
  # be read as the first run's lines; every comparison starts empty.
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

# Everything before the first timing: the commits, the builds, INV-6, the code.
prepare() {
  if [[ -e "$OUT/reading.txt" ]]; then
    say "refused: $OUT holds a reading.txt, a finished run's record whose pairs a new run would empty; take a fresh OUT"
    return 1
  fi
  mkdir -p "$OUT" || return 1
  [[ -x "$pc" ]] || { say "no $pc; build the tools first"; return 1; }
  command -v hyperfine > /dev/null || { say "no hyperfine"; return 1; }
  command -v objcopy > /dev/null || { say "no objcopy, which the code check reads with"; return 1; }
  [[ -x "$python_chess" ]] || { say "no $python_chess, which search_bench.py runs under"; return 1; }
  check_commits || return 1
  {
    echo "S118 cost"
    echo "rows      aa $ROWS"
    echo "started   $(date -Iseconds)"
    echo "base      $parent"
    echo "cache     $cache"
    echo "shelter   $shelter"
    echo "loadavg   $(cat /proc/loadavg)"
    echo "governor  $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null)"
  } | tee "$OUT/cost.log"

  local name bits
  for name in $(binaries_needed); do
    case "$name" in
      base) build_one base "$parent" ;;
      s12) build_one s12 "$cache" ;;
      shelter) build_one shelter "$shelter" ;;
      *) bits=$(bits_of "$name"); build_one "$name" "$cache" "$bits" ;;
    esac | tee -a "$OUT/cost.log"
    (( PIPESTATUS[0] == 0 )) || return 1
  done
  [[ -x "$OUT/bin/copy" ]] || cp "$OUT/bin/base" "$OUT/bin/copy" || return 1
  cmp -s "$OUT/bin/base" "$OUT/bin/copy" || { say "the A/A copy differs"; return 1; }

  for name in $(binaries_needed); do
    streams "$OUT/bin/$name" "$OUT/inv6/$name" || { say "INV-6: no streams from $name; no timing is taken"; return 1; }
  done
  for name in $(binaries_needed); do
    if ! diff -r -q "$OUT/inv6/base" "$OUT/inv6/$name" > /dev/null; then
      diff -r "$OUT/inv6/base" "$OUT/inv6/$name" | head -20
      say "INV-6: $name does not search base's tree; a defect, and no timing is taken"
      return 1
    fi
  done
  say "INV-6: bench $(tail -n 1 "$OUT/inv6/base/bench.txt"), every binary identical" |
    tee -a "$OUT/cost.log"

  # shellcheck disable=SC2086  # ROWS is a list of names
  python3 "$helper" code "$OUT" $ROWS | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || { say "the builds do not differ as their rows do; no timing is taken"; return 1; }
}

run() {
  prepare || return 1
  local name
  echo "loadavg before pairs: $(cat /proc/loadavg)" >> "$OUT/cost.log"
  pairs aa base copy | tee -a "$OUT/cost.log"
  (( PIPESTATUS[0] == 0 )) || return 1
  for name in $ROWS; do
    case "$name" in
      total) pairs total base shelter ;;
      slot) pairs slot s12 shelter ;;
      *) pairs "$name" base "$name" ;;
    esac | tee -a "$OUT/cost.log"
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
    if check_commits; then echo "S118-COST-CHECK-DONE"; else echo "S118-COST-CHECK-FAILED"; exit 1; fi ;;
  build)
    if prepare; then echo "S118-COST-BUILD-DONE"; else echo "S118-COST-BUILD-FAILED"; exit 1; fi ;;
  run)
    if run; then echo "S118-COST-DONE"; else echo "S118-COST-FAILED"; exit 1; fi ;;
  *)
    echo "usage: $0 check | build | run" >&2
    exit 2 ;;
esac
