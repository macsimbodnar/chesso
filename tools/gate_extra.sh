#!/usr/bin/env bash
set -uo pipefail

# The second tier of the gate. S197, DEC-141 clause 3, DEC-167.
#
#   tools/gate_extra.sh                    all six stages
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
#   * a clang build of the whole tree against libc++, Release and tune: both
#     gated builds are GCC, which reads -Wall -Wextra -Werror differently from
#     the Apple clang the owner's Mac builds with (S297, S298);
#   * tools/plan_prose_check.py --prose, the one mode the fast label does not
#     already run. --citations is in it as test_plan_citation_freshness since
#     S187 (DEC-159); stage 2 re-asserts it here for a second's cost rather
#     than reaching anything the automatic gate cannot.
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
# test_clang_format_script could not resolve its pinned major. Stage 6 runs the
# same label twice under clang and is read the same way. On the machine
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
#
# Every marker is written to fd 9, which is this script's real stderr saved
# before anything is redirected. The driver runs each stage with its whole
# output redirected to that stage's log, and a function is not a subshell: a
# `fail` reached from inside a stage would `exit` with stderr still pointing at
# the stage log, so the marker would land in a file no watcher polls and the
# EXIT trap would stay silent because `marked` is already 1. Measured before the
# fix: the terminal log was empty and `GATE-EXTRA-FAILED` was in the stage log.
# No stage calls `fail` today -- this keeps the header's "one line on every exit
# path" true for whoever adds stage 7.
exec 9>&2

marked=0
fail()
{
  marked=1
  echo "GATE-EXTRA-FAILED: $*" >&9
  exit 1
}
trap 'status=$?
      if ((marked == 0)); then
        echo "GATE-EXTRA-FAILED: exited $status before the run could report" >&9
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

# Configures <dir> and then proves the cache says what this stage needs.
#
# The first version returned early on any existing CMakeCache.txt, and that is a
# vacuous green waiting to happen: a `build-sanitize` configured by hand without
# -DSANITIZER=ON builds an uninstrumented tree, runs the whole fast label with
# no sanitizer at all, matches the Release bench total trivially, and reports
# `sanitize ok` -- the most expensive stage returning nothing, which is the
# class this script exists to remove. `build-debug` left at Release is the same
# defect on the stage whose only job is the asserts. It is not hypothetical:
# DEC-052 records VS Code's CMake Tools configuring a directory of this tree on
# open and writing a compiler into it that nobody chose.
#
# cmake is idempotent and costs about a second on a correct cache, so the
# configure always runs; a directory whose compiler has changed makes it fail
# here by name instead of later and elsewhere. `expect` is the grep-ready
# strings the cache must carry, one per argument, before the shift.
configure_and_verify()
{
  local dir="$1" expect="$2"
  shift 2
  # shellcheck disable=SC2086
  cmake -S "$repo" -B "$repo/$dir" "$@" $launcher || return $?

  local cache="$repo/$dir/CMakeCache.txt"
  if [[ ! -f "$cache" ]]; then
    echo "$dir has no CMakeCache.txt after configuring it" >&2
    return 1
  fi
  local want
  for want in $expect; do
    if ! grep -q -- "$want" "$cache"; then
      echo "$dir is not the directory this stage needs: $cache carries no $want" >&2
      echo "remove $repo/$dir and run again" >&2
      return 1
    fi
  done
  return 0
}

# `<total> nodes <nps> nps`, the line S189's bench command ends on.
signature_or_fail()
{
  local which="$1" line="$2"
  if [[ ! "$line" =~ ^[0-9]+\ nodes\ [0-9]+\ nps$ ]]; then
    echo "the $which build printed no bench signature line: $line" >&2
    return 1
  fi
  return 0
}

# Stale tense in adocs/plan.md, which is the one plan_prose_check.py mode the
# fast label does not already run: `--citations` has been in it since S187 as
# test_plan_citation_freshness (DEC-159), so stage 2 below re-asserts a fast
# test rather than reaching something the automatic gate cannot.
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
  configure_and_verify build-debug "CMAKE_BUILD_TYPE:STRING=Debug" \
      -DCMAKE_BUILD_TYPE=Debug \
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

  configure_and_verify build-sanitize \
      "CMAKE_BUILD_TYPE:STRING=RelWithDebInfo SANITIZER:BOOL=ON" \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSANITIZER=ON \
    && cmake --build "$repo/build-sanitize" -j"$jobs_n" \
    && ctest --test-dir "$repo/build-sanitize" -L fast --output-on-failure --no-tests=error \
    || return $?

  san_line="$(printf 'bench\nquit\n' | "$repo/build-sanitize/src/chesso" | tail -n 1)" \
    || return $?
  echo "sanitizer bench: $san_line"
  signature_or_fail sanitizer "$san_line" || return $?

  # The Release reference, built here so the comparison is the same commit.
  cmake --build "$repo/build" -j"$jobs_n" || return $?
  rel_line="$(printf 'bench\nquit\n' | "$repo/build/src/chesso" | tail -n 1)" || return $?
  echo "release bench:   $rel_line"
  # Checked on both sides, and the second one is not symmetry for its own sake.
  # Unvalidated, a stale build/ whose last line is `bestmove e2e4` made
  # rel_total the word "bestmove", the totals differed, and the stage accused
  # the tree of a memory bug the sanitizers had missed and escalated it to the
  # BUGS rule -- from a build directory that simply predated `bench`.
  signature_or_fail Release "$rel_line" || return $?

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

# True when the compiler at $1 says it is clang. Captured rather than piped into
# `grep -q`, which exits at its first match: under pipefail a compiler still
# writing its last lines would turn the yes into a SIGPIPE and a no.
is_clang()
{
  local said
  said="$("$1" --version 2> /dev/null)" || return 1
  [[ "$said" == *"clang version"* ]]
}

# The clang the stage builds with, into clang_cxx and clang_cc: the first pair
# whose C++ and C drivers are both on PATH and both say they are clang.
#
#   c++ and cc            the machine's default, when that is clang. It is on
#                         the Mac: Apple clang, the compiler the owner's own
#                         build/ uses and the one S297's break came from, on a
#                         machine where clang++-22 does not exist. TOOLCHAIN.md
#                         appends Homebrew LLVM to PATH there so that c++
#                         stays /usr/bin/c++: Apple clang, not Homebrew's.
#   clang++-22, clang-22  the workstation, whose c++ is g++ 13.3 (DEC-049).
#   clang++, clang        any other machine with a clang.
#
# Asked rather than trusted by name, because c++ is g++ on one machine and
# Apple clang on the other. C as well as C++: the project enables C and ASM,
# and ASM takes the C compiler (CMakeLists.txt). No pair found is red, not
# skipped -- a machine without clang cannot run this check, and a green that
# compiled nothing under clang is the class this script exists to remove.
find_clang()
{
  local pair cxx cc cxx_path cc_path
  clang_cxx=""
  clang_cc=""
  for pair in c++:cc clang++-22:clang-22 clang++:clang; do
    cxx="${pair%%:*}"
    cc="${pair#*:}"
    cxx_path="$(command -v "$cxx")" || continue
    cc_path="$(command -v "$cc")" || continue
    if ! is_clang "$cxx_path" || ! is_clang "$cc_path"; then
      continue
    fi
    clang_cxx="$cxx_path"
    clang_cc="$cc_path"
    return 0
  done
  echo "no clang on PATH: tried c++ and cc, clang++-22 and clang-22, clang++ and clang" >&2
  return 1
}

# The compilers a clang directory's cache records, asked what they are. The
# cache strings configure_and_verify greps for cannot say "is clang": the path
# is /usr/bin/clang++-22 here and /usr/bin/c++ on the Mac. For this stage it is
# a backstop. The configure always passes both compilers by -D, so a cache
# holding anything else means CMake did not do what it was told -- and a
# directory holding g++ would build, pass both fast labels and report
# `clang ok` with no clang having run. The hazards that make such a directory,
# CXX ignored by a directory that exists and DEC-052's VS Code writing a
# compiler into one, belong to S297's hand recipe and not to this stage.
clang_cache_or_fail()
{
  local dir="$1" var value
  for var in CMAKE_CXX_COMPILER CMAKE_C_COMPILER; do
    value="$(sed -n "s/^${var}:[A-Z]*=//p" "$repo/$dir/CMakeCache.txt")"
    if [[ -z "$value" ]] || ! is_clang "$value"; then
      echo "$dir is not the directory this stage needs: its $var is '$value', which is not clang" >&2
      echo "remove $repo/$dir and run again" >&2
      return 1
    fi
  done
  return 0
}

# One clang directory: configured and verified, built with every error shown,
# then the fast label. $2 is CHESSO_TUNE, passed either way, so a directory
# configured by hand with the other value comes back to this one.
clang_half()
{
  local dir="$1" tune="$2" expect
  expect="CMAKE_GENERATOR:INTERNAL=Ninja CMAKE_BUILD_TYPE:STRING=Release"
  expect="$expect CHESSO_TUNE:BOOL=$tune CMAKE_CXX_FLAGS:STRING=-stdlib=libc++"
  expect="$expect CMAKE_EXE_LINKER_FLAGS:STRING=-stdlib=libc++"
  configure_and_verify "$dir" "$expect" \
      -G Ninja -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE="$tune" \
      -DCMAKE_C_COMPILER="$clang_cc" -DCMAKE_CXX_COMPILER="$clang_cxx" \
      -DCMAKE_CXX_FLAGS=-stdlib=libc++ -DCMAKE_EXE_LINKER_FLAGS=-stdlib=libc++ \
    && clang_cache_or_fail "$dir" \
    && cmake --build "$repo/$dir" -j"$jobs_n" -- -k 0 \
    && ctest --test-dir "$repo/$dir" -L fast --output-on-failure --no-tests=error
  return $?
}

# A compile break only clang reports, found here weekly instead of on the
# owner's Mac (S297, S298). The same -Wall -Wextra -Werror reads differently in
# the two front ends: GCC 13 does not flag a variable that is only incremented
# and clang does (-Wunused-but-set-variable), so the Mac's build stopped in
# tools/tuner_trace.hpp while both gated GCC builds were green.
#
# Two directories, as the automatic gate has two: build-clang, and
# build-clang-tune with CHESSO_TUNE=ON, because src/search_params.hpp is
# different code in the tune build (DEC-118). Release, because Debug turns the
# unused-* warnings off (CMakeLists.txt). libc++ in both, the Mac's standard
# library: clang here compiles against GCC's libstdc++ unless told otherwise,
# and an #include one library supplies transitively and the other does not is
# the same kind of break. Ninja, because `-k 0` is its flag: every error in the
# tree reaches the log, not only the first, and a machine without ninja is
# refused by name -- the owner's Mac builds with it. The second directory is
# built and tested even when the first fails, for the reason the driver runs
# every stage.
#
# A compile check, never a measured binary: no bench total is read from either
# directory (DEV_MANUAL.md "Build"). Last in the order so that no stage before
# it was renumbered -- stages 3 and 4 are cited by number in DEV_MANUAL.md,
# adocs/specs.md and finished step files.
stage_clang()
{
  local status=0
  find_clang || return $?
  if ! command -v ninja > /dev/null; then
    echo "no ninja on PATH: the clang stage configures with -G Ninja for -k 0" >&2
    return 1
  fi
  echo "clang stage: C++ $clang_cxx, C $clang_cc"
  "$clang_cxx" --version || return $?
  clang_half build-clang OFF || status=$?
  clang_half build-clang-tune ON || status=$?
  return $status
}

# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

all_stages="prose citations debug sanitize perft clang"
stages="${STAGES:-$all_stages}"

# Read once, then taken out of the environment before any stage runs. Stages 4
# and 6 run the whole `fast` label, and tests/test_gate_extra_script.sh is in
# it: that test drives a nested gate_extra.sh over a fake tree and its cases
# expect the default six stages. Inherited, `STAGES="sanitize" tools/gate_extra.sh`
# gave the nested script one stage, nine of its checks over two cases went red,
# and the documented subset invocation failed its own gate with a green C++
# build. `unset`, not `export -n`: the variable is only exported when it was
# passed as a command prefix, and unset covers both. Case 11 of that test is
# the guard (S225).
unset STAGES

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
  echo "GATE-EXTRA-FAILED: $failed $outdir" >&9
  exit 1
fi

marked=1
echo
echo "GATE-EXTRA-DONE $index stages $total s $outdir"
