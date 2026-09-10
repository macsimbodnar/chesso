#!/usr/bin/env bash
#
# Smoke test for tools/gate_extra.sh. S197.
#
# What it protects is the marker, not the stages. gate_extra.sh is detached and
# watched (WATCHERS rule, DEC-061): a run that ends without printing exactly one
# GATE-EXTRA-DONE or GATE-EXTRA-FAILED line leaves its watcher spinning to the
# ceiling, which is the failure S167 removed from fastchess.sh and S177 from
# rating.sh. Both of those were found after the fact; this one is checked from
# the day the script exists.
#
#   1. every stub succeeds: exit 0, exactly one marker and it is DONE, and the
#      five stages ran in the documented order with the documented commands
#   2. the fast label goes red under the sanitizer: exit non-zero, exactly one
#      marker, FAILED naming sanitize -- and the later stages still ran, which
#      is what "every stage runs even after one fails" means
#   3. no cmake on PATH: exit non-zero, exactly one marker, and the trap does
#      not add a second one behind fail()'s
#   4. the two builds disagree on the bench total: the sanitize stage fails and
#      its log names INV-6 (DEC-167 -- the sanitizers are blind to a read that
#      moves the tree without being out of bounds or undefined)
#   5. the sanitizer build prints no bench signature at all: same, and not a
#      silent pass on an empty total
#   6. the script sitting outside the chesso tree: one FAILED marker naming the
#      root, and no stage runs against a tree that is not this one
#   7. static: the script parses, and it does not reach for a bare $(nproc)
#
# Cases 4 and 5 exist because a first version of this file asserted only that
# the comparison had run, and a cut replacing its condition with `false`
# survived every case.
#
# No stage does any real work: cmake, ctest and python3 are stubs that record
# their arguments, and the two engine binaries are one-line scripts printing a
# bench signature. Nothing is built and no test binary runs, so the whole thing
# is seconds and belongs in the fast label even though what it covers does not.
#
# Usage: test_gate_extra_script.sh <gate_extra.sh>

set -uo pipefail

gate_script="${1:?path to gate_extra.sh}"
failures=0

fail()
{
  echo "FAIL: $*" >&2
  failures=$((failures + 1))
}

if [[ ! -r "$gate_script" ]]; then
  echo "FAIL: no script at $gate_script" >&2
  exit 1
fi

tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-gate-extra-smoke.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

mkdir -p "$tmp/stub" "$tmp/tools" "$tmp/build/src" "$tmp/build-sanitize/src" "$tmp/out"

# Everything the script and its stubs reach for, linked in by name. PATH is this
# directory and nothing else, so the result does not depend on what the machine
# has installed -- ccache, nproc and sysctl are absent until a case adds one.
for util in bash sh cat tail head ls date mkdir rm printf grep sed tr awk expr; do
  path="$(command -v "$util")" || { echo "FAIL: no $util on this machine" >&2; exit 1; }
  ln -s "$path" "$tmp/stub/$util"
done

# The script derives the repository root from its own location, so it has to sit
# one directory below the sandbox root.
cp "$gate_script" "$tmp/tools/gate_extra.sh"
chmod +x "$tmp/tools/gate_extra.sh"
: > "$tmp/tools/plan_prose_check.py"

calls="$tmp/calls.log"

make_stub()
{
  local name="$1" body="$2"
  {
    echo '#!/bin/sh'
    echo "echo \"$name \$*\" >> \"$calls\""
    echo "$body"
  } > "$tmp/stub/$name"
  chmod +x "$tmp/stub/$name"
}

make_stub cmake 'exit 0'
make_stub python3 'exit 0'
make_stub nproc 'echo 8'

# The one stub with an opinion: red on the fast label, and only when the case
# asks for it by touching the flag file.
make_stub ctest "
if [ -f \"$tmp/fail_ctest_fast\" ]; then
  for a in \"\$@\"; do
    if [ \"\$a\" = fast ]; then exit 1; fi
  done
fi
exit 0"

# Both engines answer the bench signature with the same total, so the INV-6
# comparison across builds passes -- unless a case asks otherwise by touching a
# flag file. They drain stdin: a stub that exits without reading it gives printf
# an EPIPE, and under `set -o pipefail` the script would read that as the engine
# failing.
printf '#!/bin/sh\ncat > /dev/null\necho "12345 nodes 100 nps"\n' > "$tmp/build/src/chesso"
chmod +x "$tmp/build/src/chesso"

{
  echo '#!/bin/sh'
  echo 'cat > /dev/null'
  echo "if [ -f \"$tmp/no_signature\" ]; then echo 'bestmove e2e4'; exit 0; fi"
  echo "if [ -f \"$tmp/differ_bench\" ]; then echo '99999 nodes 100 nps'; exit 0; fi"
  echo "echo '12345 nodes 100 nps'"
} > "$tmp/build-sanitize/src/chesso"
chmod +x "$tmp/build-sanitize/src/chesso"

# Runs the sandboxed script and prints its exit status; output lands in out.txt
# inside the sandbox.
run_sandbox()
{
  : > "$calls"
  [[ -n "${tmp:-}" ]] && rm -rf "$tmp/out"
  (
    cd "$tmp" || exit 127
    export PATH="$tmp/stub" OUT="$tmp/out" TMPDIR="$tmp"
    ./tools/gate_extra.sh
  ) > "$tmp/out.txt" 2>&1
  echo $?
}

show()
{
  echo "--- out.txt ---" >&2
  sed 's/^/    /' "$tmp/out.txt" >&2
  echo "--- calls.log ---" >&2
  sed 's/^/    /' "$calls" >&2
  echo "--- end ---" >&2
}

markers() { grep -c 'GATE-EXTRA-' "$tmp/out.txt"; }

# The line number of the first calls.log entry matching a pattern, or 0.
call_line() { grep -n -- "$1" "$calls" | head -n 1 | sed 's/:.*//'; }

# 1. Everything green: one DONE marker, and the documented stage order.
status="$(run_sandbox)"
if [[ "$status" -ne 0 ]]; then
  fail "1: exited $status with every stub succeeding"; show
fi
if [[ "$(markers)" -ne 1 ]]; then
  fail "1: expected exactly one marker, got $(markers)"; show
fi
if ! grep -q '^GATE-EXTRA-DONE 5 stages ' "$tmp/out.txt"; then
  fail "1: no DONE marker naming five stages"; show
fi

prose_at="$(call_line 'python3 .*--prose')"
cits_at="$(call_line 'python3 .*--citations')"
dbuild_at="$(call_line 'cmake --build .*build-debug')"
dtest_at="$(call_line 'ctest .*build-debug')"
sconf_at="$(call_line 'cmake -S .* -B .*build-sanitize')"
sbuild_at="$(call_line 'cmake --build .*build-sanitize')"
sfast_at="$(call_line 'ctest .*build-sanitize.*-L fast')"
slow_at="$(call_line 'ctest .*-L slow')"

# Arithmetic on an empty string is a syntax error inside (( )), so a stage that
# never ran reads as line 0 and is reported by the loop below first.
for var in prose_at cits_at dbuild_at dtest_at sconf_at sbuild_at sfast_at slow_at; do
  eval "[[ -n \"\${$var}\" ]] || $var=0"
done

for pair in "prose:$prose_at" "citations:$cits_at" "debug-build:$dbuild_at" \
            "debug-ctest:$dtest_at" "sanitize-configure:$sconf_at" \
            "sanitize-build:$sbuild_at" "sanitize-fast:$sfast_at" \
            "perft-slow:$slow_at"; do
  if [[ "${pair#*:}" == 0 ]]; then
    fail "1: ${pair%%:*} never ran"; show
  fi
done

if ((prose_at > 0 && slow_at > 0)); then
  if ! ((prose_at < cits_at && cits_at < dbuild_at && dbuild_at < dtest_at \
         && dtest_at < sconf_at && sconf_at < sbuild_at && sbuild_at < sfast_at \
         && sfast_at < slow_at)); then
    fail "1: stages ran out of order (prose $prose_at, citations $cits_at," \
         "debug build $dbuild_at, debug ctest $dtest_at, sanitize configure" \
         "$sconf_at, sanitize build $sbuild_at, sanitize fast $sfast_at, slow $slow_at)"
    show
  fi
fi

# The Debug ctest regex is anchored, or it also selects test_search_params.
if ! grep -F -q '^(test_chesso|test_openings|test_movegen|test_evaluation|test_search|test_engine)$' "$calls"; then
  fail "1: the Debug stage's ctest -R is not the anchored six-binary regex"; show
fi
# The Debug directory is configured Debug, or the stage runs the six binaries
# with every assert( in src/ compiled out -- which is the whole reason it exists.
if ! grep -q 'cmake -S .* -B .*build-debug.*-DCMAKE_BUILD_TYPE=Debug' "$calls"; then
  fail "1: build-debug is not configured at CMAKE_BUILD_TYPE=Debug"; show
fi
# The sanitizer directory is configured with the option that makes it one.
if ! grep -q 'cmake -S .* -B .*build-sanitize.*-DSANITIZER=ON' "$calls"; then
  fail "1: build-sanitize is configured without -DSANITIZER=ON"; show
fi
if ! grep -q 'cmake -S .* -B .*build-sanitize.*RelWithDebInfo' "$calls"; then
  fail "1: build-sanitize is not configured at RelWithDebInfo"; show
fi
# The bench comparison of DEC-167 happened and agreed.
if ! grep -rq 'INV-6 across builds: both 12345 nodes' "$tmp/out"; then
  fail "1: the sanitize stage did not compare the two bench totals"; show
fi

# 2. The fast label is red under the sanitizer: one FAILED marker naming
#    sanitize, and the stages after it still ran.
: > "$tmp/fail_ctest_fast"
status="$(run_sandbox)"
rm -f "$tmp/fail_ctest_fast"
if [[ "$status" -eq 0 ]]; then
  fail "2: exited 0 with the sanitizer fast label red"; show
fi
if [[ "$(markers)" -ne 1 ]]; then
  fail "2: expected exactly one marker, got $(markers)"; show
fi
if ! grep -q '^GATE-EXTRA-FAILED: .*sanitize' "$tmp/out.txt"; then
  fail "2: the marker does not name the sanitize stage"; show
fi
sfast_at="$(call_line 'ctest .*build-sanitize.*-L fast')"
slow_at="$(call_line 'ctest .*-L slow')"
if [[ -z "$slow_at" ]]; then
  slow_at=0
fi
[[ -n "$sfast_at" ]] || sfast_at=0
if ((slow_at == 0)); then
  fail "2: the perft stage did not run after the sanitize stage failed"; show
elif ((slow_at < sfast_at)); then
  fail "2: the slow label ran before the stage that failed"; show
fi

# 3. No cmake: the refusal comes from fail(), and the EXIT trap does not add a
#    second marker behind it.
mv "$tmp/stub/cmake" "$tmp/cmake.parked"
status="$(run_sandbox)"
mv "$tmp/cmake.parked" "$tmp/stub/cmake"
if [[ "$status" -eq 0 ]]; then
  fail "3: exited 0 with no cmake on PATH"; show
fi
if [[ "$(markers)" -ne 1 ]]; then
  fail "3: expected exactly one marker, got $(markers) -- the trap doubled it?"; show
fi
if ! grep -q '^GATE-EXTRA-FAILED: .*cmake' "$tmp/out.txt"; then
  fail "3: the marker does not say what is missing (cmake)"; show
fi
if grep -q 'command not found' "$tmp/out.txt"; then
  fail "3: a 'command not found' reached the output"; show
fi

# 4. The two builds disagree on the bench total. DEC-167: they differ only in
#    instrumentation, so a difference is a read that moved the search tree and
#    that neither sanitizer reported.
: > "$tmp/differ_bench"
status="$(run_sandbox)"
rm -f "$tmp/differ_bench"
if [[ "$status" -eq 0 ]]; then
  fail "4: exited 0 with the two builds reporting different bench totals"; show
fi
if ! grep -q '^GATE-EXTRA-FAILED: .*sanitize' "$tmp/out.txt"; then
  fail "4: a bench mismatch did not fail the sanitize stage"; show
fi
if ! grep -q 'INV-6 across builds' "$tmp/out.txt"; then
  fail "4: the failure does not say the two builds disagree"; show
fi

# 5. The sanitizer build prints no signature line at all -- an empty or
#    unparseable total is not a pass.
: > "$tmp/no_signature"
status="$(run_sandbox)"
rm -f "$tmp/no_signature"
if [[ "$status" -eq 0 ]]; then
  fail "5: exited 0 with the sanitizer build printing no bench signature"; show
fi
if ! grep -q '^GATE-EXTRA-FAILED: .*sanitize' "$tmp/out.txt"; then
  fail "5: a missing bench signature did not fail the sanitize stage"; show
fi
if ! grep -q 'no bench signature line' "$tmp/out.txt"; then
  fail "5: the failure does not name the missing signature"; show
fi

# 6. The script somewhere that is not the chesso tree. The root is derived from
#    $0 and then checked rather than trusted: an earlier version derived it with
#    `dirname`, and on a PATH without dirname it silently became "/" and ran
#    every stage against the filesystem root.
mkdir -p "$tmp/elsewhere/tools"
cp "$gate_script" "$tmp/elsewhere/tools/gate_extra.sh"
chmod +x "$tmp/elsewhere/tools/gate_extra.sh"
(
  cd "$tmp/elsewhere" || exit 127
  export PATH="$tmp/stub" OUT="$tmp/out" TMPDIR="$tmp"
  : > "$calls"
  ./tools/gate_extra.sh
) > "$tmp/out.txt" 2>&1
status=$?
if [[ "$status" -eq 0 ]]; then
  fail "6: exited 0 from a directory that is not the chesso tree"; show
fi
if [[ "$(markers)" -ne 1 ]]; then
  fail "6: expected exactly one marker, got $(markers)"; show
fi
if ! grep -q '^GATE-EXTRA-FAILED: .*does not look like the chesso tree' "$tmp/out.txt"; then
  fail "6: the marker does not say the root is wrong"; show
fi
if grep -q 'cmake\|ctest' "$calls"; then
  fail "6: a stage ran against a tree that is not the chesso tree"; show
fi

# 7. Static: the script parses, and it does not reach for a bare $(nproc) --
#    the form rating.sh died on before its marker was armed (S177).
if grep -v '^[[:space:]]*#' "$gate_script" | grep -q '\$(nproc)'; then
  fail "7: gate_extra.sh uses a bare \$(nproc)"
fi
if ! bash -n "$gate_script"; then
  fail "7: gate_extra.sh does not parse"
fi

if [[ $failures -ne 0 ]]; then
  echo "$failures failure(s)" >&2
  exit 1
fi

echo "ok"
