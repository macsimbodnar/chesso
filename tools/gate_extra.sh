#!/usr/bin/env bash
set -uo pipefail

# The second tier of the gate. S197, DEC-141 clause 3, DEC-167.
#
#   tools/gate_extra.sh                    all five stages
#   STAGES="sanitize perft" tools/gate_extra.sh    a subset, after a fix
#   JOBS=6 OUT=/tmp/run tools/gate_extra.sh        override cores and outdir
#
# WHAT THIS IS FOR, AND WHAT tools/gate.sh IS FOR. gate.sh is the automatic
# gate: two Release directories, the `fast` label, the format check and the
# commit's bench signature, about 108 s, run before every commit. DEC-025 keeps
# it to exactly that. This script is everything that gate cannot hold, and it is
# NOT automatic:
#
#   * the Debug binaries, whose assert( lines are the only enforcement of INV-2
#     and INV-4 at every make and unmake -- both gated builds are Release, where
#     every assert in src/ is dead;
#   * a sanitizer build, which existed as a CMake option nobody ran since the
#     2026-08-13 audit;
#   * deep perft, INV-1 at depths the fast label cannot reach;
#   * the two tools/plan_prose_check.py modes kept out of the fast label.
#
# WHEN IT RUNS, DEC-141 clause 3: before a step that touched make_move,
# unmake_move, the generator or the search completes, and otherwise weekly. The
# date and sha of the last GATE-EXTRA-DONE live in adocs/status.md.
#
# EVERY STAGE RUNS EVEN AFTER ONE FAILS. The weekly run's value is the whole
# picture in one pass, so a red stage is recorded and the next one starts; the
# marker names every stage that failed. Nothing is piped into a pager or a tail
# -- DEV_MANUAL.md "Test": a gate written that way reports success on a failed
# suite -- each stage's whole output goes to its own log and its exit status is
# read from the stage function.
#
# THE MARKER. One line on every exit path, WATCHERS rule and DEC-061, so a
# detached run's watcher always terminates:
#
#   GATE-EXTRA-DONE <n> stages <total> s <outdir>
#   GATE-EXTRA-FAILED: <stage> <stage>... <outdir>
#
# IT PRESUMES THE AUTOMATIC GATE IS GREEN. Stage 4 runs the whole `fast` label
# under the sanitizer, so any red in that label fails the sanitize stage -- the
# marker then says "sanitize" for something that has nothing to do with the
# sanitizers, after the build has already been paid for. The first real run of
# this script, 2026-09-10, failed exactly that way: 514 s spent to be told
# test_clang_format_script could not resolve its pinned major. On the machine
# .moltke.local.md describes that means exporting DEC-146's override, and a
# detached run does not inherit an interactive shell's environment:
#
#   export CLANG_FORMAT_MAJOR=22
#
# Launch it detached and poll the whole log; never `tail -f | grep`:
#
#   nohup tools/gate_extra.sh > .tuning/gate_extra_$(date +%F).log 2>&1 &

# The terminal marker, armed before the first command that can fail -- before
# the repository root is even resolved. Same shape as rating.sh and fastchess.sh
# after S167 and S177: `marked` is read rather than `$?`, because bash 3.2's EXIT
# trap sees `$? == 0` for a `set -u` abort, and a detached run that dies with no
# marker leaves its watcher spinning to the ceiling.
marked=0
fail()
{
  marked=1
  echo "GATE-EXTRA-FAILED: $*" >&2
  exit 1
}
trap 'status=$?
      if ((marked == 0)); then
        echo "GATE-EXTRA-FAILED: exited $status before the run could report" >&2
        ((status != 0)) || status=1
      fi
      exit $status' EXIT

# The repository root, through parameter expansion rather than `dirname`.
# dirname is an external command, and `$(dirname "$0")` on a PATH without it
# expands to nothing: the root silently became "/" and every stage ran against
# a tree that is not this one. Found by tests/test_gate_extra_script.sh on the
# day the script was written -- the same class as rating.sh's bare $(nproc)
# (S177), and the reason the root is then checked rather than trusted.
here="$0"
case "$here" in
  */*) here="${here%/*}" ;;
  *)   here="." ;;
esac
repo="$(cd "$here/.." && pwd)" || fail "cannot resolve the repository root from $0"
[[ -r "$repo/tools/plan_prose_check.py" ]] \
  || fail "$repo does not look like the chesso tree: no tools/plan_prose_check.py"

# Counted through nproc where it exists and sysctl otherwise. Never a bare
# $(nproc): rating.sh died on exactly that on a machine without GNU coreutils,
# before its marker was armed (S177, 2026-09-03_adversarial-F03).
all_cores="$(nproc 2> /dev/null || sysctl -n hw.logicalcpu 2> /dev/null || true)"
[[ -n "$all_cores" ]] || fail "cannot count cores: neither nproc nor sysctl on PATH"
jobs_n="${JOBS:-$all_cores}"

# ccache is a speed-up and not a requirement. Passed only when it is there, so
# a machine without it configures instead of failing at the first compile.
launcher=""
if command -v ccache > /dev/null; then
  launcher="-DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
fi

stamp="$(date +%Y%m%d_%H%M%S)"
outdir="${OUT:-/tmp/chesso_gate_extra_${stamp}}"
mkdir -p "$outdir" || fail "cannot create $outdir"

if ! command -v cmake > /dev/null; then fail "cmake not on PATH"; fi
if ! command -v ctest > /dev/null; then fail "ctest not on PATH"; fi
if ! command -v python3 > /dev/null; then fail "python3 not on PATH"; fi

# ---------------------------------------------------------------------------
# Stages. Each is a function; the driver redirects its whole output to a log and
# reads its return value. Bodies are explicit && chains with an explicit return,
# because `set -e` is off inside an `if` condition and the driver calls them
# from one.
# ---------------------------------------------------------------------------

# --no-tests=error (ctest 3.20 and later) is on every ctest call below: a filter
# that matches nothing otherwise exits 0, so a mistyped regex or a label that
# stopped being applied reads as a green stage. DEC-165 met the same class from
# the other side -- a non-zero run that ran nothing is not a kill.

# Configures <dir> only when it has no cache, so a re-run reuses the objects.
configure_if_needed()
{
  local dir="$1"
  shift
  if [[ -f "$repo/$dir/CMakeCache.txt" ]]; then
    echo "-- $dir already configured, reusing it"
    return 0
  fi
  # shellcheck disable=SC2086
  cmake -S "$repo" -B "$repo/$dir" "$@" $launcher
  return $?
}

# Stale tense in adocs/plan.md. Kept out of the fast label because it goes red
# on commits that touch no code.
stage_prose()
{
  python3 "$repo/tools/plan_prose_check.py" --prose
  return $?
}

# A code citation in a pending step file that no longer resolves. Red here is
# the signal to fix the citation by symbol (DEC-135), never to relax the check.
stage_citations()
{
  python3 "$repo/tools/plan_prose_check.py" --citations
  return $?
}

# INV-2 and INV-4, the PV assert in src/search.cpp, and every other assert( in
# src/. The six binaries are the ones that drive make_move and unmake_move; the
# tuner, script, audit and surface tests make no moves, and the three mate
# binaries are out on cost -- test_mate_breadth alone is about 697 s in Debug
# (DEC-167 question 2).
#
# The regex is anchored because an unanchored `test_search` also matches
# test_search_params.
stage_debug()
{
  configure_if_needed build-debug -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build "$repo/build-debug" -j"$jobs_n" \
    && ctest --test-dir "$repo/build-debug" --output-on-failure --no-tests=error \
         -R '^(test_chesso|test_openings|test_movegen|test_evaluation|test_search|test_engine)$'
  return $?
}

# Out-of-bounds reads, use-after-free, signed overflow, bad shifts, misaligned
# loads and -- on Linux, where LeakSanitizer is on by default -- leaks.
#
# RelWithDebInfo, not Release and not Debug: -O2 -g is the "-O1 or higher" ASan
# asks for plus symbolised reports, and it falls on the 600 s side of
# CHESSO_TEST_TIMEOUT, so a 2x slowdown cannot hit the 60 s Release ceiling and
# read as a failure that is the clock (the DEC-052 class).
#
# The last two commands are DEC-167 question 1: the sanitizer build's bench
# total must equal the Release build's. The two differ only in instrumentation,
# so a difference is a read of uninitialised or out-of-bounds memory that
# changed the search tree and that neither sanitizer reported -- the class they
# are blind to. It is a third reading of INV-6, and the first that compares two
# builds of one commit rather than two commits.
stage_sanitize()
{
  local san_line rel_line san_total rel_total

  export ASAN_OPTIONS=detect_leaks=1
  export UBSAN_OPTIONS=print_stacktrace=1

  configure_if_needed build-sanitize -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSANITIZER=ON \
    && cmake --build "$repo/build-sanitize" -j"$jobs_n" \
    && ctest --test-dir "$repo/build-sanitize" -L fast --output-on-failure --no-tests=error \
    || return $?

  san_line="$(printf 'bench\nquit\n' | "$repo/build-sanitize/src/chesso" | tail -n 1)" \
    || return $?
  echo "sanitizer bench: $san_line"
  if [[ ! "$san_line" =~ ^[0-9]+\ nodes\ [0-9]+\ nps$ ]]; then
    echo "the sanitizer build printed no bench signature line" >&2
    return 1
  fi

  # The Release reference, built here so the comparison is the same commit.
  cmake --build "$repo/build" -j"$jobs_n" || return $?
  rel_line="$(printf 'bench\nquit\n' | "$repo/build/src/chesso" | tail -n 1)" || return $?
  echo "release bench:   $rel_line"

  san_total="${san_line%% *}"
  rel_total="${rel_line%% *}"
  if [[ "$san_total" != "$rel_total" ]]; then
    echo "INV-6 across builds: sanitizer $san_total nodes, Release $rel_total nodes" >&2
    echo "the two builds differ only in instrumentation, so this is a read the" >&2
    echo "sanitizers did not report. BUGS rule: fix it before anything else." >&2
    return 1
  fi
  echo "INV-6 across builds: both $san_total nodes"
  return 0
}

# INV-1 at the depths the fast label does not reach. test_perft exits non-zero
# on a missing or unparseable asset and on a mismatch in any column it compares
# since S193; before that it passed vacuously on both.
stage_perft()
{
  cmake --build "$repo/build" -j"$jobs_n" \
    && ctest --test-dir "$repo/build" -L slow --output-on-failure --no-tests=error
  return $?
}

# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

all_stages="prose citations debug sanitize perft"
stages="${STAGES:-$all_stages}"

for name in $stages; do
  found=0
  for known in $all_stages; do
    [[ "$name" == "$known" ]] && found=1
  done
  ((found == 1)) || fail "no stage named '$name' -- have: $all_stages"
done

echo "gate_extra: repo $repo"
echo "gate_extra: stages $stages"
echo "gate_extra: jobs $jobs_n, logs in $outdir"
echo

# Space-separated rather than arrays: bash 3.2 errors on ${arr[@]} for an empty
# array under `set -u`, and the failed list is empty on a green run.
failed=""
summary=""
index=0
run_start="$(date +%s)"

for name in $stages; do
  index=$((index + 1))
  log="$(printf '%s/%02d_%s.log' "$outdir" "$index" "$name")"
  echo "-- $name ..."
  start="$(date +%s)"
  if "stage_$name" > "$log" 2>&1; then
    status=0
  else
    status=$?
  fi
  elapsed=$(($(date +%s) - start))
  if ((status == 0)); then
    echo "   ok, ${elapsed}s"
    summary="${summary}${name}|ok|${elapsed}|${log}"$'\n'
  else
    echo "   FAILED (exit $status), ${elapsed}s"
    summary="${summary}${name}|exit ${status}|${elapsed}|${log}"$'\n'
    failed="${failed}${failed:+ }${name}"
  fi
done

total=$(($(date +%s) - run_start))

echo
echo "stage        status     seconds  log"
printf '%s' "$summary" | while IFS='|' read -r s_name s_status s_sec s_log; do
  [[ -n "$s_name" ]] || continue
  printf '%-12s %-10s %7s  %s\n' "$s_name" "$s_status" "$s_sec" "$s_log"
done

if [[ -n "$failed" ]]; then
  for name in $failed; do
    echo >&2
    echo "=== $name: last 40 lines ===" >&2
    log="$(ls "$outdir"/[0-9][0-9]_"$name".log 2> /dev/null | head -n 1)"
    if [[ -n "$log" ]]; then
      tail -n 40 "$log" >&2
    else
      echo "(no log at $outdir for $name)" >&2
    fi
  done
  marked=1
  echo "GATE-EXTRA-FAILED: $failed $outdir" >&2
  exit 1
fi

marked=1
echo
echo "GATE-EXTRA-DONE $index stages $total s $outdir"
