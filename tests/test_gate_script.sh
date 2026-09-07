#!/usr/bin/env bash
#
# Smoke test for tools/gate.sh, the commit gate. S189, DEC-140.
#
# The gate's whole job is to compare one number the binary prints with one
# number the commit message claims, and to be right about when the number is
# owed. That is nine cases, and none of them needs a real engine or a real
# suite: the sandbox is a throwaway git repository whose PATH holds stubs for
# cmake, ctest and clang-format.sh that exit 0, and a `build/src/chesso` that
# prints a canned signature line. So this test measures the gate's logic and
# nothing about the machine it runs on.
#
#   1. `Bench: <n>` matching the binary, src/ touched      -> GATE-DONE, exit 0
#   2. `Bench: <n>` not matching                           -> GATE-FAILED, both
#                                                             numbers named
#   3. src/ touched, neither line                          -> GATE-FAILED
#   4. no src/ change, neither line                        -> GATE-DONE
#   5. `No functional change` over a matching parent       -> GATE-DONE
#   6. `No functional change` over a non-matching parent   -> GATE-FAILED
#   7. both lines at once                                  -> GATE-FAILED
#   8. a binary that prints no signature line              -> exactly one
#                                                             GATE-FAILED
#   9. static: parses under `bash -n`, and uses none of the bash 4 forms the
#      MacBook's bash 3.2 does not have
#
# Usage: test_gate_script.sh <gate.sh>

set -uo pipefail

gate_script="${1:?path to gate.sh}"
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

tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-gate-smoke.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

mkdir -p "$tmp/stub" "$tmp/repo/tools" "$tmp/repo/src" "$tmp/repo/build/src" \
         "$tmp/repo/docs"

# The utilities the gate reaches for, linked in by name. git is real: the gate
# reads a commit's message and its touched paths, which is the half of its
# logic a stub would hide.
for util in bash sh dirname basename mkdir rm cat chmod grep sed cut head tail \
            printf git env tr wc; do
  path="$(command -v "$util")" || { echo "FAIL: no $util on this machine" >&2; exit 1; }
  ln -s "$path" "$tmp/stub/$util"
done

cp "$gate_script" "$tmp/repo/tools/gate.sh"
chmod +x "$tmp/repo/tools/gate.sh"

# The suite the gate runs, stubbed green. A red one is not a case here: the
# gate reports it as itself and the branch is one `|| fail`.
for stub in cmake ctest; do
  printf '#!/bin/sh\nexit 0\n' > "$tmp/stub/$stub"
  chmod +x "$tmp/stub/$stub"
done
printf '#!/bin/sh\nexit 0\n' > "$tmp/repo/clang-format.sh"
chmod +x "$tmp/repo/clang-format.sh"

# The binary. `bench` is reached over stdin, so the stub reads its input away
# and prints the eight bestmove lines and the signature, which is the shape
# `bench_of` parses.
set_bench()
{
  if [[ "$1" == "none" ]]; then
    printf '#!/bin/sh\ncat > /dev/null\necho "bestmove e2e4"\n' \
      > "$tmp/repo/build/src/chesso"
  else
    printf '#!/bin/sh\ncat > /dev/null\necho "bestmove e2e4"\necho "%s nodes 999 nps"\n' \
      "$1" > "$tmp/repo/build/src/chesso"
  fi
  chmod +x "$tmp/repo/build/src/chesso"
}
set_bench 12345

git -C "$tmp/repo" init -q
git -C "$tmp/repo" config user.email smoke@example.invalid
git -C "$tmp/repo" config user.name smoke

commit()
{
  # commit <message> [path ...]
  local message="$1"
  shift

  for path in "$@"; do
    echo "$RANDOM $path" >> "$tmp/repo/$path"
    git -C "$tmp/repo" add "$path"
  done

  git -C "$tmp/repo" commit -q --allow-empty -m "$message"
}

run_gate()
{
  (
    cd "$tmp/repo" || exit 127
    export PATH="$tmp/stub"
    ./tools/gate.sh "$@"
  ) > "$tmp/out.txt" 2>&1
  echo $?
}

show()
{
  echo "--- out.txt ---" >&2
  sed 's/^/    /' "$tmp/out.txt" >&2
  echo "--- end ---" >&2
}

done_markers()   { grep -c 'GATE-DONE'   "$tmp/out.txt"; }
failed_markers() { grep -c 'GATE-FAILED' "$tmp/out.txt"; }

# The root of the ancestry, so cases 5 and 6 have a parent to read.
commit "root

Bench: 12345
Co-Authored-By: nobody <nobody@example.invalid>" src/a.cpp

# 1. A matching Bench line over a src/ change.
commit "a functional change

Bench: 12345
Co-Authored-By: nobody <nobody@example.invalid>" src/b.cpp
status="$(run_gate)"
if [[ "$status" -ne 0 ]]; then
  fail "1: matching Bench line exited $status"; show
fi
if [[ "$(done_markers)" -ne 1 || "$(failed_markers)" -ne 0 ]]; then
  fail "1: expected exactly one GATE-DONE"; show
fi
if ! grep -q 'GATE-DONE 12345' "$tmp/out.txt"; then
  fail "1: GATE-DONE does not carry the total"; show
fi

# 2. The same, with the previous signature left in the message: the case the
#    accepts asks to be observed red, and the reason the gate exists.
commit "a functional change carrying a stale signature

Bench: 12344
Co-Authored-By: nobody <nobody@example.invalid>" src/c.cpp
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "2: a stale Bench line exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "2: expected exactly one GATE-FAILED, got $(failed_markers)"; show
fi
if ! grep -q '12344' "$tmp/out.txt" || ! grep -q '12345' "$tmp/out.txt"; then
  fail "2: the marker does not name both numbers"; show
fi

# Amended to the total the binary benches, which is what a developer does when
# the gate refuses -- red, then green, on the same commit. It also keeps the
# ancestry consistent for cases 5 and 6, whose parent walk reads the nearest
# `Bench:` line and would otherwise read this rejected one.
git -C "$tmp/repo" commit -q --amend -m "a functional change, signature corrected

Bench: 12345
Co-Authored-By: nobody <nobody@example.invalid>"
status="$(run_gate)"
if [[ "$status" -ne 0 ]]; then
  fail "2: the amended commit did not go green, exited $status"; show
fi

# 3. src/ touched, no line at all.
commit "a functional change with no signature at all" src/d.cpp
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "3: a missing signature exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "3: expected exactly one GATE-FAILED"; show
fi

git -C "$tmp/repo" commit -q --amend -m "a functional change with its signature

Bench: 12345
Co-Authored-By: nobody <nobody@example.invalid>"

# 4. Documentation only: nothing under src/ moved, so nothing is owed.
commit "a documentation change" docs/manual.md
status="$(run_gate)"
if [[ "$status" -ne 0 ]]; then
  fail "4: a docs-only commit exited $status"; show
fi
if [[ "$(done_markers)" -ne 1 ]]; then
  fail "4: expected exactly one GATE-DONE"; show
fi

# 5. `No functional change` where the binary agrees with the ancestry.
commit "a rename that changes no search line

No functional change
Co-Authored-By: nobody <nobody@example.invalid>" src/e.cpp
status="$(run_gate)"
if [[ "$status" -ne 0 ]]; then
  fail "5: a true 'No functional change' exited $status"; show
fi
if [[ "$(done_markers)" -ne 1 ]]; then
  fail "5: expected exactly one GATE-DONE"; show
fi

# 6. The same claim against a binary that no longer benches the parent's total
#    -- a change that was thought neutral and was not. INV-6's whole point.
set_bench 11111
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "6: a false 'No functional change' exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "6: expected exactly one GATE-FAILED"; show
fi
if ! grep -q '11111' "$tmp/out.txt" || ! grep -q '12345' "$tmp/out.txt"; then
  fail "6: the marker does not name both numbers"; show
fi
set_bench 12345

# 7. Both lines at once: the message contradicts itself and the gate does not
#    pick a winner.
commit "a change that claims both things

Bench: 12345
No functional change
Co-Authored-By: nobody <nobody@example.invalid>" src/f.cpp
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "7: a message carrying both lines exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "7: expected exactly one GATE-FAILED"; show
fi

# 8. A binary that prints no signature at all -- a `bench` that regressed, or a
#    build that is not this engine. One marker, not a bare shell error.
set_bench none
commit "a functional change

Bench: 12345
Co-Authored-By: nobody <nobody@example.invalid>" src/g.cpp
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "8: a binary with no signature line exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "8: expected exactly one GATE-FAILED, got $(failed_markers)"; show
fi
if ! grep -q "GATE-FAILED.*nodes" "$tmp/out.txt"; then
  fail "8: the marker does not say the signature line is missing"; show
fi
set_bench 12345

# 9. Static. bash 3.2 on the MacBook has none of these (S167, S177), and the
#    comments in the script name no such form, so the whole file is searched.
if ! bash -n "$gate_script"; then
  fail "9: gate.sh does not parse"
fi
for form in 'mapfile' 'readarray' 'declare -A' '&>>' '\$(nproc)' '\btimeout '; do
  if grep -qE "$form" "$gate_script"; then
    fail "9: gate.sh uses [$form], which bash 3.2 does not have"
  fi
done
if grep -qE '\$\{[A-Za-z_]+,,\}' "$gate_script"; then
  fail "9: gate.sh uses \${x,,}, which bash 3.2 does not have"
fi

if [[ $failures -ne 0 ]]; then
  echo "$failures failure(s)" >&2
  exit 1
fi

echo "ok"
