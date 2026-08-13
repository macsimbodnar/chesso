#!/usr/bin/env bash
#
# Smoke test for fastchess.sh. S035, closing 2026-08-13_adversarial-F01.
#
# Two properties, and the second is the one that stops the finding recurring:
#
#   1. the script reaches the `fastchess` invocation
#   2. a script that aborts before that point exits non-zero
#
# No game is played and no engine is built. `fastchess` is a stub on PATH, the
# candidate and the reference are one-line shell scripts, and the whole run
# happens inside a throwaway git repository, so the real .ref-builds/ and the
# real build/ are never read or written.
#
# Usage: test_fastchess_script.sh [path-to-fastchess.sh]
# The argument exists so a past revision of the script can be run through the
# same assertions, which is how this was observed failing at 44877c4.

set -uo pipefail

script_under_test="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/fastchess.sh}"
failures=0

fail()
{
  echo "FAIL: $*" >&2
  failures=$((failures + 1))
}

if [[ ! -r "$script_under_test" ]]; then
  echo "FAIL: no script at $script_under_test" >&2
  exit 1
fi

# A sandbox is a git repository holding a copy of the script under test and
# every path the script derives from its own location: the candidate binary,
# the opening book, and a pre-built reference for HEAD so the worktree build is
# skipped. Prints the directory.
make_sandbox()
{
  local script="$1"
  local tmp sha

  tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-fastchess-smoke.XXXXXX")"
  mkdir -p "$tmp/build/src" "$tmp/books" "$tmp/stub"

  printf '#!/bin/sh\nexit 0\n' > "$tmp/build/src/chesso"
  chmod +x "$tmp/build/src/chesso"
  : > "$tmp/books/8moves_v3.pgn"

  # The stub records that it ran and plays nothing. It reports its own
  # invocation through a file rather than through stdout, so the assertion
  # cannot be satisfied by the script merely echoing the word fastchess.
  cat > "$tmp/stub/fastchess" << 'STUB'
#!/bin/sh
touch "$(dirname "$0")/../fastchess_invoked"
exit 0
STUB
  chmod +x "$tmp/stub/fastchess"

  cp "$script" "$tmp/fastchess.sh"
  chmod +x "$tmp/fastchess.sh"

  git -C "$tmp" init -q
  git -C "$tmp" -c user.email=smoke@example.invalid -c user.name=smoke \
    commit -q --allow-empty -m smoke
  sha="$(git -C "$tmp" rev-parse --short HEAD)"

  mkdir -p "$tmp/.ref-builds/$sha/build/src"
  printf '#!/bin/sh\nexit 0\n' > "$tmp/.ref-builds/$sha/build/src/chesso"
  chmod +x "$tmp/.ref-builds/$sha/build/src/chesso"

  echo "$tmp"
}

# Runs the sandboxed script and prints its exit status. Output lands in
# out.txt inside the sandbox so a failing assertion can show it.
run_sandbox()
{
  local tmp="$1"
  (
    cd "$tmp" || exit 127
    PATH="$tmp/stub:$PATH" REF=HEAD ./fastchess.sh --fast
  ) > "$tmp/out.txt" 2>&1
  echo $?
}

show()
{
  echo "--- $1/out.txt ---" >&2
  sed 's/^/    /' "$1/out.txt" >&2
  echo "--- end ---" >&2
}

# 1. The script reaches the fastchess invocation.
reached_dir="$(make_sandbox "$script_under_test")"
reached_status="$(run_sandbox "$reached_dir")"

if [[ ! -e "$reached_dir/fastchess_invoked" ]]; then
  fail "fastchess was never invoked (exit status $reached_status)"
  show "$reached_dir"
elif ((reached_status != 0)); then
  fail "fastchess was invoked but the script exited $reached_status"
  show "$reached_dir"
fi

# 2. A failure before the invocation is visible in the exit status.
#
# The injected line reproduces the shape of F01: an unbound variable under
# `set -u`, placed after the EXIT trap is armed and before fastchess runs. A
# trap that rewrites the status turns this into a silent success.
anchors="$(grep -c '^fastchess \\$' "$script_under_test")"
if ((anchors != 1)); then
  fail "expected exactly one 'fastchess \\' line to inject before, found $anchors"
else
  aborting="$(mktemp "${TMPDIR:-/tmp}/chesso-fastchess-aborting.XXXXXX")"
  awk '/^fastchess \\$/ && !injected {
         print "echo \"$smoke_deliberately_unset_variable\""
         injected = 1
       }
       { print }' "$script_under_test" > "$aborting"

  abort_dir="$(make_sandbox "$aborting")"
  abort_status="$(run_sandbox "$abort_dir")"
  rm -f "$aborting"

  # Non-vacuous by construction. The status assertion below is only evidence
  # about the trap if the run really stopped at the injected line: a script
  # that dies earlier for its own reasons would satisfy it by accident.
  if [[ -e "$abort_dir/fastchess_invoked" ]]; then
    fail "injected failure did not stop the script; the abort case proves nothing"
    show "$abort_dir"
  elif ! grep -q smoke_deliberately_unset_variable "$abort_dir/out.txt"; then
    fail "the run stopped before the injected line; the abort case proves nothing"
    show "$abort_dir"
  elif ((abort_status == 0)); then
    fail "script aborted before fastchess but exited 0 (the EXIT trap masks it)"
    show "$abort_dir"
  fi
fi

rm -rf "$reached_dir"
[[ -n "${abort_dir:-}" ]] && rm -rf "$abort_dir"

if ((failures > 0)); then
  echo "$script_under_test: $failures assertion(s) failed" >&2
  exit 1
fi

echo "$script_under_test: reaches fastchess, and an abort before it exits non-zero"
