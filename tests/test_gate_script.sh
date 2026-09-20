#!/usr/bin/env bash
#
# Smoke test for tools/gate.sh, the commit gate. S189, DEC-140; S233, DEC-220.
#
# The gate has two jobs. The first is to compare one number the binary prints
# with one number the commit message claims, and to be right about when the
# number is owed. The second, from S233, is to hold a verdict-closing commit's
# result block to DEC-220's shape and to the log it names. That is seventeen
# cases, and none of them needs a real engine or a real suite: the sandbox is a
# throwaway git repository whose PATH holds stubs for cmake, ctest and
# clang-format.sh that exit 0, and a `build/src/chesso` that prints a canned
# signature line. So this test measures the gate's logic and nothing about the
# machine it runs on.
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
#  10. `No functional change` with no `Bench:` anywhere in the ancestry ->
#      exactly one GATE-FAILED, and it names --build-parent
#
# DEC-220's result block, S233. An `SPRT |` line in the message is the trigger
# and the log it names is the evidence:
#
#  11. a whole block over a tracked log whose `Results of` line carries the
#      same two shas                                        -> GATE-DONE
#  12. one of the six lines missing                         -> GATE-FAILED
#                                                              naming that line
#  13. one of the six malformed                             -> GATE-FAILED
#                                                              naming that line
#  14. one of the six written twice                         -> GATE-FAILED
#  15. a block whose shas are not the log's                 -> GATE-FAILED
#                                                              naming both
#  16. a `Log |` path that is untracked, and one that does not exist at all
#                                                           -> GATE-FAILED each
#  17. the same commit with no `SPRT |` line                -> GATE-DONE, and
#                                                              no block check
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

# 10. `No functional change` over an ancestry that carries no `Bench:` line at
#     all -- a history from before DEC-140, which is the situation
#     `--build-parent` exists for. A second repository, because the one above
#     has carried a `Bench:` line since its root commit.
#
#     This is the case the S189 fast check found missing: the parent walk had
#     no `|| true`, so with no match `grep` exited 1, `pipefail` failed the
#     assignment and errexit killed the script one line above the message
#     written for it -- the generic `GATE-FAILED: exited 1` on the one path
#     that most needs to say what to do next.
mkdir -p "$tmp/repo2/tools" "$tmp/repo2/src" "$tmp/repo2/build/src"
cp "$gate_script" "$tmp/repo2/tools/gate.sh"
chmod +x "$tmp/repo2/tools/gate.sh"
cp "$tmp/repo/clang-format.sh" "$tmp/repo2/clang-format.sh"
cp "$tmp/repo/build/src/chesso" "$tmp/repo2/build/src/chesso"

git -C "$tmp/repo2" init -q
git -C "$tmp/repo2" config user.email smoke@example.invalid
git -C "$tmp/repo2" config user.name smoke
echo "before the rule" > "$tmp/repo2/src/old.cpp"
git -C "$tmp/repo2" add src/old.cpp
git -C "$tmp/repo2" commit -q -m "a commit from before DEC-140"
echo "still nothing functional" >> "$tmp/repo2/src/old.cpp"
git -C "$tmp/repo2" add src/old.cpp
git -C "$tmp/repo2" commit -q -m "A change with no ancestor to compare against

No functional change"

(
  cd "$tmp/repo2" || exit 127
  export PATH="$tmp/stub"
  ./tools/gate.sh
) > "$tmp/out.txt" 2>&1
status=$?
if [[ "$status" -eq 0 ]]; then
  fail "10: 'No functional change' with no ancestor signature exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "10: expected exactly one GATE-FAILED, got $(failed_markers)"; show
fi
if ! grep -q 'build-parent' "$tmp/out.txt"; then
  fail "10: the marker does not say to re-run with --build-parent"; show
fi
if grep -q 'GATE-FAILED: exited' "$tmp/out.txt"; then
  fail "10: the trap's generic marker won over the specific one"; show
fi

# --- DEC-220's result block, cases 11 to 17 (S233) -------------------------
#
# The evidence a block is checked against: a fastchess log, tracked, carrying
# the `Results of cand-<sha> vs ref-<sha>` line the two shas have to match.
# Four lines of it are enough -- the gate reads that one line and nothing else.
mkdir -p "$tmp/repo/adocs/data"
cat > "$tmp/repo/adocs/data/S999_sprt.log" <<'LOG'
--------------------------------------------------
Results of cand-1a2b3c4 vs ref-5d6e7f8 (8+0.08, 1t, 16MB, noob_3moves.epd):
Elo: 5.75 +/- 4.37, nElo: 7.44 +/- 5.65
LLR: 2.97 (100.9%) (-2.94, 2.94) [0.00, 5.00]
--------------------------------------------------
LOG
git -C "$tmp/repo" add adocs/data/S999_sprt.log
git -C "$tmp/repo" commit -q -m "the run log a block points at"

# The six lines, as DEC-220 states them, built one at a time so a case can
# drop, bend or repeat exactly one of them.
line_SPRT="SPRT | cand 1a2b3c4 vs ref 5d6e7f8, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo"
line_Elo="Elo | 5.75 +/- 4.37, nElo 7.44 +/- 5.65"
line_LLR="LLR | 2.97 (-2.94, 2.94) -> H1"
line_Games="Games | N: 14510 W: 4559 L: 4319 D: 5632, Ptnml [603, 1695, 2522, 1729, 706]"
line_Wall="Wall | 6 h 49 m, 2129.6 games/h, forfeits 0"
line_Log="Log | adocs/data/S999_sprt.log"

# block [skip] [extra] -- the whole block, minus one line by name, plus one
# raw line. A verdict-closing commit touches no src/, so these carry no
# signature and the docs-only path is the one under test.
block()
{
  local skip="${1:-}"
  local extra="${2:-}"
  local name value
  echo "Record S999's verdict"
  echo ""
  for name in SPRT Elo LLR Games Wall Log; do
    [[ "$name" == "$skip" ]] && continue
    eval "value=\$line_$name"
    echo "$value"
  done
  [[ -n "$extra" ]] && echo "$extra"
  echo "Co-Authored-By: nobody <nobody@example.invalid>"
}

# 11. The whole block over its own log. The commit touches no src/, so the
#     `Bench:` half is not owed and the block check is the only thing between
#     it and GATE-DONE -- which also shows the check runs before the early
#     exit the no-src/ path takes.
commit "$(block)" docs/manual.md
status="$(run_gate)"
if [[ "$status" -ne 0 ]]; then
  fail "11: a whole block over its own log exited $status"; show
fi
if [[ "$(done_markers)" -ne 1 || "$(failed_markers)" -ne 0 ]]; then
  fail "11: expected exactly one GATE-DONE"; show
fi
if ! grep -q 'SPRT block checked: cand 1a2b3c4 vs ref 5d6e7f8' "$tmp/out.txt"; then
  fail "11: the gate does not say it checked the block"; show
fi

# 12. One line missing. Each of the six in turn, because a loop that checks
#     only the first would not notice a name misspelled in the script.
for missing in SPRT Elo LLR Games Wall Log; do
  # Dropping `SPRT |` drops the trigger with it, so that one is tested by
  # replacing the line rather than removing it.
  if [[ "$missing" == "SPRT" ]]; then
    git -C "$tmp/repo" commit -q --amend \
      -m "$(block SPRT "SPRT | cand 1a2b3c4 vs ref 5d6e7f8")"
  else
    git -C "$tmp/repo" commit -q --amend -m "$(block "$missing")"
  fi
  status="$(run_gate)"
  if [[ "$status" -eq 0 ]]; then
    fail "12/$missing: a block missing its '$missing |' line exited 0"; show
  fi
  if [[ "$(failed_markers)" -ne 1 ]]; then
    fail "12/$missing: expected exactly one GATE-FAILED"; show
  fi
  if ! grep -q "$missing |" "$tmp/out.txt"; then
    fail "12/$missing: the marker does not name the line"; show
  fi
done

# 13. One line bent out of shape: an LLR outcome that is not one of the three.
git -C "$tmp/repo" commit -q --amend -m "$(block LLR "LLR | 2.97 (-2.94, 2.94) -> YES")"
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "13: a malformed LLR line exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "13: expected exactly one GATE-FAILED"; show
fi
if ! grep -q "shape" "$tmp/out.txt" || ! grep -q "LLR |" "$tmp/out.txt"; then
  fail "13: the marker does not say the LLR line is out of shape"; show
fi

# 14. One line twice. Two `Games |` lines and the block stops meaning one run.
git -C "$tmp/repo" commit -q --amend -m "$(block "" "$line_Games")"
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "14: a repeated Games line exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "14: expected exactly one GATE-FAILED"; show
fi

# 15. The shas the block claims are not the shas the log recorded -- the case
#     DEC-220 names, and the one a copy-pasted block from the previous verdict
#     lands in.
line_SPRT="SPRT | cand deadbee vs ref 5d6e7f8, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo"
git -C "$tmp/repo" commit -q --amend -m "$(block)"
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "15: a block whose shas are not the log's exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]]; then
  fail "15: expected exactly one GATE-FAILED"; show
fi
if ! grep -q 'deadbee' "$tmp/out.txt" || ! grep -q '1a2b3c4' "$tmp/out.txt"; then
  fail "15: the marker does not name both the claimed and the recorded sha"; show
fi
line_SPRT="SPRT | cand 1a2b3c4 vs ref 5d6e7f8, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo"

# 16. The log the block names is untracked, then absent. Untracked first: the
#     file is there and the run happened, and the evidence still is not in the
#     repository, which is the whole point of naming it.
cp "$tmp/repo/adocs/data/S999_sprt.log" "$tmp/repo/adocs/data/S998_sprt.log"
line_Log="Log | adocs/data/S998_sprt.log"
git -C "$tmp/repo" commit -q --amend -m "$(block)"
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "16: an untracked log exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]] || ! grep -q 'not tracked' "$tmp/out.txt"; then
  fail "16: expected one GATE-FAILED saying the log is not tracked"; show
fi
rm -f "$tmp/repo/adocs/data/S998_sprt.log"
status="$(run_gate)"
if [[ "$status" -eq 0 ]]; then
  fail "16: an absent log exited 0"; show
fi
if [[ "$(failed_markers)" -ne 1 ]] || ! grep -q 'S998_sprt.log' "$tmp/out.txt"; then
  fail "16: expected one GATE-FAILED naming the missing log"; show
fi
line_Log="Log | adocs/data/S999_sprt.log"

# 17. The same commit with the block removed: a message with no `SPRT |` line
#     takes exactly the path it took before S233, and the gate says nothing
#     about blocks. This is the unaffected-path case in the suite; the
#     twenty-message replay in the step file is the wider proof.
git -C "$tmp/repo" commit -q --amend -m "Record S999's verdict

Co-Authored-By: nobody <nobody@example.invalid>"
status="$(run_gate)"
if [[ "$status" -ne 0 ]]; then
  fail "17: a message with no SPRT line exited $status"; show
fi
if [[ "$(done_markers)" -ne 1 || "$(failed_markers)" -ne 0 ]]; then
  fail "17: expected exactly one GATE-DONE"; show
fi
if grep -q 'SPRT block' "$tmp/out.txt"; then
  fail "17: the gate checked a block that is not there"; show
fi

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
