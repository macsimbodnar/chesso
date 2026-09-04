#!/usr/bin/env bash
#
# Smoke test for rating.sh, and a static check on build_release.sh. S177,
# closing 2026-09-03_adversarial-F03.
#
# Five properties. The first is the one that stops F03 recurring: at 1d8cbac
# `./rating.sh --bracket` on a machine without GNU coreutils died at
# `all_cores="$(nproc)"` with exit 127 and printed no marker, because the
# RATING-RUN-* trap was armed 90 lines later. Under the WATCHERS rule a detached
# run that dies with no marker leaves its watcher spinning to the ceiling -- the
# class S167 removed for fastchess.sh.
#
#   1. with no way to count cores on PATH, the script prints exactly one
#      RATING-RUN-FAILED line, naming cores, and exits non-zero
#   2. with a core count and no `ordo`, the one marker names ordo -- which is
#      also the bash 3.2 case: `command -v x || fail` exits before fail runs
#      there, so the check has to be written as an if
#   3. with `ordo` and no GNU `timeout` or `gtimeout`, the one marker names
#      timeout, before any engine is asked anything
#   4. a run that gets past every check and then dies -- fastchess is a stub
#      that writes no PGN -- still ends on a terminal marker line
#   5. neither script uses a bare `$(nproc)`, and both parse under `bash -n`
#
# No game is played. Everything happens in a throwaway directory whose PATH
# holds only the utilities linked in below, so the result does not depend on
# what the machine has installed: nproc, timeout, gtimeout, sysctl and ordo are
# absent until a case adds a stub for one of them.
#
# Usage: test_rating_script.sh <rating.sh> <build_release.sh>

set -uo pipefail

rating_script="${1:?path to rating.sh}"
release_script="${2:?path to build_release.sh}"
failures=0

fail()
{
  echo "FAIL: $*" >&2
  failures=$((failures + 1))
}

for script in "$rating_script" "$release_script"; do
  if [[ ! -r "$script" ]]; then
    echo "FAIL: no script at $script" >&2
    exit 1
  fi
done

tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-rating-smoke.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

mkdir -p "$tmp/stub" "$tmp/build/src" "$tmp/books" "$tmp/tools" "$tmp/out"

# The utilities rating.sh needs up to and through its checks, linked in by name.
for util in bash sh dirname basename date mkdir mktemp cp chmod rm ps awk sed \
            head tr cat tee grep sort uniq git env sleep printf; do
  path="$(command -v "$util")" || { echo "FAIL: no $util on this machine" >&2; exit 1; }
  ln -s "$path" "$tmp/stub/$util"
done

cp "$rating_script" "$tmp/rating.sh"
chmod +x "$tmp/rating.sh"

# What the script derives from its own location: a candidate, a manifest and
# the book. Three stub engines answer `uci` with the name the manifest gives
# them, which is all identify() asks.
printf '#!/bin/sh\nexit 0\n' > "$tmp/build/src/chesso"
chmod +x "$tmp/build/src/chesso"
: > "$tmp/books/8moves_v3.pgn"

for name in alpha bravo charlie; do
  printf '#!/bin/sh\nprintf "id name %s\\nuciok\\n"\n' "$name" > "$tmp/engine_$name"
  chmod +x "$tmp/engine_$name"
  printf '%s\t%s\t%s\tsmoke\ttag\tmd5\n' "$name" "$tmp/engine_$name" "$name-ccrl" \
    >> "$tmp/references.tsv"
done

# The banner asks git for the candidate's commit.
git -C "$tmp" init -q
: > "$tmp/tracked.txt"
git -C "$tmp" add tracked.txt
git -C "$tmp" -c user.email=smoke@example.invalid -c user.name=smoke \
  commit -q -m "smoke"

# Runs the sandboxed script and prints its exit status; output lands in
# out.txt inside the sandbox. PATH is the stub directory and nothing else.
run_sandbox()
{
  (
    cd "$tmp" || exit 127
    export PATH="$tmp/stub" OUT="$tmp/out/run" TMPDIR="$tmp/out"
    ./rating.sh --bracket
  ) > "$tmp/out.txt" 2>&1
  echo $?
}

show()
{
  echo "--- out.txt ---" >&2
  sed 's/^/    /' "$tmp/out.txt" >&2
  echo "--- end ---" >&2
}

markers() { grep -c 'RATING-RUN-' "$tmp/out.txt"; }
failed_markers() { grep -c 'RATING-RUN-FAILED' "$tmp/out.txt"; }

# 1. No way to count cores: one FAILED marker, naming cores.
status="$(run_sandbox)"
if [[ "$status" -eq 0 ]]; then
  fail "1: exited 0 with no way to count cores"; show
fi
if [[ "$(markers)" -ne 1 || "$(failed_markers)" -ne 1 ]]; then
  fail "1: expected exactly one RATING-RUN-FAILED line, got $(markers) marker(s)"; show
fi
if ! grep -q 'RATING-RUN-FAILED:.*core' "$tmp/out.txt"; then
  fail "1: the marker does not say what is missing (cores)"; show
fi
if grep -q 'command not found' "$tmp/out.txt"; then
  fail "1: a 'command not found' reached the output"; show
fi

# 2. A core count and no ordo: the one marker names ordo.
printf '#!/bin/sh\necho 8\n' > "$tmp/stub/nproc"
chmod +x "$tmp/stub/nproc"
printf '#!/bin/sh\nexit 0\n' > "$tmp/stub/fastchess"
chmod +x "$tmp/stub/fastchess"
status="$(run_sandbox)"
if [[ "$status" -eq 0 ]]; then
  fail "2: exited 0 with no ordo"; show
fi
if [[ "$(markers)" -ne 1 ]]; then
  fail "2: expected exactly one marker, got $(markers)"; show
fi
if ! grep -q 'RATING-RUN-FAILED:.*ordo' "$tmp/out.txt"; then
  fail "2: the marker does not name ordo"; show
fi

# 3. ordo present, no timeout and no gtimeout: the one marker names timeout,
#    and no engine was asked anything (the marker comes before the manifest
#    walk, so no 'id name' complaint appears).
printf '#!/bin/sh\nexit 0\n' > "$tmp/stub/ordo"
chmod +x "$tmp/stub/ordo"
status="$(run_sandbox)"
if [[ "$status" -eq 0 ]]; then
  fail "3: exited 0 with no GNU timeout"; show
fi
if [[ "$(markers)" -ne 1 ]]; then
  fail "3: expected exactly one marker, got $(markers)"; show
fi
if ! grep -q 'RATING-RUN-FAILED:.*timeout' "$tmp/out.txt"; then
  fail "3: the marker does not name timeout"; show
fi
if grep -q 'id name' "$tmp/out.txt"; then
  fail "3: an engine was probed before the timeout check"; show
fi

# 4. gtimeout present: every check passes, the manifest is walked through the
#    stub engines, the stub fastchess writes no PGN, and whatever kills the
#    run after that still ends it on a terminal marker line.
printf '#!/bin/sh\nshift 3\nexec "$@"\n' > "$tmp/stub/gtimeout"
chmod +x "$tmp/stub/gtimeout"
status="$(run_sandbox)"
if [[ "$status" -eq 0 ]]; then
  fail "4: exited 0 with a fastchess that played nothing"; show
fi
if ! grep -qE 'RATING-RUN-(DONE|FAILED|INVALID)' "$tmp/out.txt"; then
  fail "4: no terminal marker after the checks passed"; show
fi
if ! grep -q 'opponents   alpha bravo charlie' "$tmp/out.txt"; then
  fail "4: the run did not get past the checks to the banner"; show
fi

# 5. Static: no bare nproc, and both scripts parse.
for script in "$rating_script" "$release_script"; do
  # Code only: the comments in both scripts name the old form on purpose.
  if grep -v '^[[:space:]]*#' "$script" | grep -q '\$(nproc)'; then
    fail "5: $(basename "$script") still uses a bare \$(nproc)"
  fi
  if ! bash -n "$script"; then
    fail "5: $(basename "$script") does not parse"
  fi
done

if [[ $failures -ne 0 ]]; then
  echo "$failures failure(s)" >&2
  exit 1
fi

echo "ok"
