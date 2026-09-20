#!/usr/bin/env bash
set -euo pipefail

# The commit gate. Runs the TESTS rule's suite in both builds and then checks
# the node signature the built binary prints against the `Bench:` line of the
# commit message under test (DEC-140, S189).
#
#   tools/gate.sh                        check HEAD
#   tools/gate.sh <ref>                  check any other commit
#   tools/gate.sh --message FILE         check a message not yet committed,
#                                        against the staged tree
#   tools/gate.sh --build-parent ...     for `No functional change`, build the
#                                        parent and run its bench instead of
#                                        reading its message
#
# WHAT IT IS FOR. INV-6 -- a change claimed behaviour-neutral proves it with
# identical node counts -- was discharged by a person running
# `tools/search_bench.py` on two binaries and comparing three counts by eye,
# and nothing recorded the number where a script could read it. Every CI
# surveyed in `adocs/testing_strategy.md` 3.1 checks a bench signature from the
# commit message before anything else runs, and the fault-injection pass
# measured what it is worth here: 21 of 33 injected bugs moved the depth-9
# counts, including a one-ply reverse-futility floor drift that two of the
# three mate gates did not see.
#
# THE RULE IT ENFORCES. A commit that touches `src/` carries exactly one of:
#
#   Bench: <n>              the total `chesso bench` prints on that tree
#   No functional change    only when that total equals the parent's
#
# A commit that touches no `src/` file needs neither and may carry neither.
# "Ends with" in the COMMITS rule is read as "carries in its trailer block":
# commits here end with `Co-Authored-By:`, so the whole message is searched.
#
# THE SECOND RULE, DEC-220 and S233. A commit that closes an SPRT verdict --
# H1, H0 or no verdict -- carries the run's result block in fastchess's own
# line names, so that `git log` is the measurement ledger and
# `tools/ledger.py` can regenerate `plan.md`'s table from it:
#
#   SPRT | cand <sha> vs ref <sha>, <tc>, Hash=<n>, <book>, {e0, e1} nElo
#   Elo | <x> +/- <y>, nElo <x> +/- <y>
#   LLR | <l> (<a>, <b>) -> H1|H0|none
#   Games | N: <n> W: <w> L: <l> D: <d>, Ptnml [<5>]
#   Wall | <h> h <m> m, <g> games/h, forfeits <f>
#   Log | adocs/data/<file>
#
# An `SPRT |` line is the trigger: with one in the message all six are owed,
# each in that shape, and the two shas have to be the `cand-<sha>` and
# `ref-<sha>` of the named log's own `Results of` line. A message with no
# `SPRT |` line is not touched by any of this.
#
# ON THIS MACHINE: `clang-format.sh` pins major 23 and the workstation has 22,
# so `export CLANG_FORMAT_MAJOR=22` before running this (`.moltke.local.md`,
# DEC-146). The pin is not overridden here: a gate that silences its own
# version check is not a gate.

# A MARKER ON EVERY EXIT PATH, SUCCESS AND FAILURE BOTH, armed before the first
# thing that can fail -- the same contract fastchess.sh's header states, for the
# same reason: a gate run detached is watched by something that has to be able
# to stop (WATCHERS rule, DEC-061). `marked` keeps the trap from printing a
# second marker after fail() has spoken; `completed` is what the trap asks,
# because bash 3.2 hands an EXIT trap `$? == 0` when the shell aborts under
# `set -u` and the status cannot be trusted (S167).
marked=0
completed=0
fail()
{
  marked=1
  echo "GATE-FAILED: $*" >&2
  exit 1
}

trap 'status=$?
      if ((marked == 0 && (status != 0 || completed == 0))); then
        echo "GATE-FAILED: exited $status" >&2
        ((status != 0)) || status=1
      fi
      exit $status' EXIT

# Anchored to the script and not to the caller, so a run from inside another
# checkout reads this repository -- the bug S160 fixed in fastchess.sh, avoided
# here by construction.
repo="$(cd -- "$(dirname -- "$0")/.." && pwd)"
real_git="$(command -v git)"
git()
{
  "$real_git" -C "$repo" "$@"
}

cd "$repo"

message_file=""
build_parent=0
ref=""

while (($# > 0)); do
  case "$1" in
    --message)
      shift
      (($# > 0)) || fail "--message needs a file"
      message_file="$1"
      ;;
    --build-parent) build_parent=1 ;;
    -*) fail "unknown option [$1]" ;;
    *)
      [[ -z "$ref" ]] || fail "two refs given: [$ref] and [$1]"
      ref="$1"
      ;;
  esac
  shift
done

if [[ -n "$message_file" ]]; then
  [[ -z "$ref" ]] || fail "--message and a ref are exclusive"
  [[ -f "$message_file" ]] || fail "no such message file [$message_file]"
else
  ref="${ref:-HEAD}"
fi

# THE SUITE FIRST. Verbatim the TESTS rule's command, and read as one status:
# piping ctest into anything hides the status behind the pipe's last stage
# (DEV_MANUAL.md, "Test"). A failure here is a failure of the gate, not a
# signature question, so it is reported as itself.
cmake --build build -j8 \
  && ctest --test-dir build -L fast --output-on-failure \
  && cmake --build build-tune -j8 \
  && ctest --test-dir build-tune -L fast --output-on-failure \
  && ./clang-format.sh --check \
  || fail "TESTS rule command failed"

# THE SIGNATURE, off the binary the suite just built. `bench` runs
# synchronously, so `quit` written after it on the same pipe is read only once
# it has returned -- unlike `go`, where the same shape kills the search
# (TOOLCHAIN.md, "The chess oracle"). The last matching line is taken so a
# future `info string` cannot be mistaken for it.
#
# The `|| true` is load-bearing. `grep` exits 1 when a binary prints no
# signature line, and under `set -o pipefail` that fails the whole pipeline and
# the assignment with it, so errexit killed the script one line above the check
# written for exactly that case and the trap reported the generic
# `GATE-FAILED: exited 1`. Caught by case 8 of tests/test_gate_script.sh.
line_re='^[0-9]+ nodes [0-9]+ nps$'
bench_of()
{
  printf 'bench\nquit\n' | "$1" | grep -E "$line_re" | tail -1 | cut -d' ' -f1
}

total="$(bench_of build/src/chesso || true)"
[[ -n "$total" ]] || fail "bench printed no '<nodes> nodes <nps> nps' line"

# WHAT THE TREE UNDER TEST IS. In message mode it is the staged tree, so an
# unstaged edit would make the bench above measure something the commit will
# not contain. Refused rather than warned about: a signature taken from the
# wrong tree is worse than no signature.
if [[ -n "$message_file" ]]; then
  git diff --quiet || fail "working tree has unstaged changes; the bench above would not be the committed tree"
  touched="$(git diff --cached --name-only -- src/)"
  message="$(cat "$message_file")"
else
  touched="$(git diff-tree --no-commit-id --name-only -r "$ref" -- src/)"
  message="$(git log -1 --format=%B "$ref")"
fi

# DEC-220'S RESULT BLOCK, checked here -- immediately after the message is
# read and before the `Bench:` line it sits above -- rather than ahead of the
# suite. The order of everything else is left exactly as it was and the whole
# of S233's diff to this script is this block; the price is that a typo in a
# block is reported after the build rather than before it.
sprt_lines="$(printf '%s\n' "$message" | grep -cE '^SPRT \|' || true)"

if ((sprt_lines != 0)); then
  # One of each of the six, in any order. The order is DEC-220's prose and
  # nothing downstream reads it -- `tools/ledger.py` parses by line name. Two
  # `SPRT |` lines are a different matter: "the block's shas" would stop
  # meaning one thing, so a repeat of any line is refused.
  for name in SPRT Elo LLR Games Wall Log; do
    case "$name" in
      SPRT)
        shape='^SPRT \| cand [0-9a-f]{7,40} vs ref [0-9a-f]{7,40}, [^,]+, Hash=[0-9]+, [^,]+, \{-?[0-9]+(\.[0-9]+)?, -?[0-9]+(\.[0-9]+)?\} nElo$' ;;
      Elo)
        shape='^Elo \| [-+]?[0-9]+(\.[0-9]+)? \+/- [0-9]+(\.[0-9]+)?, nElo [-+]?[0-9]+(\.[0-9]+)? \+/- [0-9]+(\.[0-9]+)?$' ;;
      LLR)
        shape='^LLR \| [-+]?[0-9]+(\.[0-9]+)? \([-+]?[0-9]+(\.[0-9]+)?, [-+]?[0-9]+(\.[0-9]+)?\) -> (H1|H0|none)$' ;;
      Games)
        shape='^Games \| N: [0-9]+ W: [0-9]+ L: [0-9]+ D: [0-9]+, Ptnml \[[0-9]+, [0-9]+, [0-9]+, [0-9]+, [0-9]+\]$' ;;
      Wall)
        shape='^Wall \| [0-9]+ h [0-9]+ m, [0-9]+(\.[0-9]+)? games/h, forfeits [0-9]+$' ;;
      Log)
        shape='^Log \| adocs/data/[A-Za-z0-9._/-]+$' ;;
    esac

    present="$(printf '%s\n' "$message" | grep -cE "^$name \|" || true)"
    ((present != 0)) \
      || fail "the message carries an 'SPRT |' line, so DEC-220's block is owed, and its '$name |' line is missing"
    ((present == 1)) \
      || fail "the message carries $present '$name |' lines; DEC-220's block has exactly one of each"

    shaped="$(printf '%s\n' "$message" | grep -cE "$shape" || true)"
    ((shaped == 1)) \
      || fail "the '$name |' line is not in DEC-220's shape: [$(printf '%s\n' "$message" | grep -E "^$name \|" | head -1)]"
  done

  # The block against its own evidence. Without this the six lines are only a
  # claim; with it a verdict in `git log` is pinned to the log that produced
  # it, which is what DEC-020 made attribution depend on.
  sprt_line="$(printf '%s\n' "$message" | grep -E '^SPRT \|' | head -1)"
  cand_sha="$(printf '%s\n' "$sprt_line" | sed -E 's/^SPRT \| cand ([0-9a-f]+) vs ref ([0-9a-f]+),.*/\1/')"
  ref_sha="$(printf '%s\n' "$sprt_line" | sed -E 's/^SPRT \| cand ([0-9a-f]+) vs ref ([0-9a-f]+),.*/\2/')"
  log_path="$(printf '%s\n' "$message" | grep -E '^Log \|' | head -1 | sed -E 's/^Log \| //')"

  [[ -f "$log_path" ]] \
    || fail "the 'Log |' line names [$log_path], which is not a file in this repository"
  git ls-files --error-unmatch "$log_path" > /dev/null 2>&1 \
    || fail "the log [$log_path] is not tracked; a verdict's evidence is committed with it"

  if ! grep -qE "^Results of cand-$cand_sha vs ref-$ref_sha( |\$)" "$log_path"; then
    results_line="$(grep -E '^Results of ' "$log_path" | head -1 || true)"
    [[ -n "$results_line" ]] \
      || fail "[$log_path] carries no 'Results of cand-<sha> vs ref-<sha>' line; it is not a fastchess run log"
    fail "the block says cand $cand_sha vs ref $ref_sha, [$log_path] says [$results_line]"
  fi

  echo "SPRT block checked: cand $cand_sha vs ref $ref_sha against $log_path"
fi

bench_lines="$(printf '%s\n' "$message" | grep -cE '^Bench: [0-9]+$' || true)"
nfc_lines="$(printf '%s\n' "$message" | grep -cxF 'No functional change' || true)"

if [[ -z "$touched" ]]; then
  # Nothing under src/ moved, so neither line is owed. One may still be there
  # -- a documentation commit that states the unchanged signature is not wrong
  # -- and it is checked if it is.
  if ((bench_lines == 0 && nfc_lines == 0)); then
    completed=1
    echo "GATE-DONE $total (no src/ change, no signature owed)"
    exit 0
  fi
fi

((bench_lines + nfc_lines != 0)) \
  || fail "commit touches src/ but carries neither 'Bench: <n>' nor 'No functional change'"
((bench_lines + nfc_lines == 1)) \
  || fail "message carries $bench_lines 'Bench:' and $nfc_lines 'No functional change' lines; exactly one is allowed"

if ((bench_lines == 1)); then
  claimed="$(printf '%s\n' "$message" | grep -E '^Bench: [0-9]+$' | cut -d' ' -f2)"
  ((claimed == total)) \
    || fail "message says Bench: $claimed, the binary benches $total"

  completed=1
  echo "GATE-DONE $total"
  exit 0
fi

# `No functional change` is a claim about the parent, so the parent's total has
# to come from somewhere. By induction every `src/` commit from S189's onward
# carries a verified line, so the nearest ancestor `Bench:` is the parent's
# total; --build-parent is the escape hatch for an ancestry that predates the
# rule or was rewritten.
if [[ -n "$message_file" ]]; then
  parent="HEAD"
else
  parent="$ref^"
fi

if ((build_parent == 1)); then
  parent_sha="$(git rev-parse --short "$parent")"
  parent_dir="$repo/.ref-builds/$parent_sha"
  parent_binary="$parent_dir/build/src/chesso"

  if [[ ! -x "$parent_binary" ]]; then
    echo "Building parent $parent_sha ..."
    git worktree add --detach "$parent_dir" "$parent_sha" > /dev/null
    cmake -S "$parent_dir" -B "$parent_dir/build" -DCMAKE_BUILD_TYPE=Release > /dev/null
    cmake --build "$parent_dir/build" --target chesso -j8 > /dev/null
  fi

  parent_total="$(bench_of "$parent_binary" || true)"
  [[ -n "$parent_total" ]] \
    || fail "parent $parent_sha printed no signature line; it predates bench"
else
  # The first `Bench:` at or below the parent. `-n 100` is the same window
  # Stockfish's CI takes its reference from.
  #
  # `|| true` for the reason the bench pipeline above has it: with no match
  # `grep` exits 1, `pipefail` fails the assignment and errexit kills the
  # script before the check written for this exact case -- an ancestry that
  # predates the rule, which is the situation `--build-parent` exists for, so
  # the one path that most needs its message was the one that lost it. Found by
  # the S189 fast check; case 10 of tests/test_gate_script.sh covers it.
  parent_total="$(git log --format=%B -n 100 "$parent" \
    | grep -E '^Bench: [0-9]+$' | head -1 | cut -d' ' -f2 || true)"
  [[ -n "$parent_total" ]] \
    || fail "'No functional change' but no ancestor carries a 'Bench:' line in the last 100 commits; re-run with --build-parent"
fi

((parent_total == total)) \
  || fail "'No functional change' but the binary benches $total against the parent's $parent_total"

completed=1
echo "GATE-DONE $total (no functional change)"
