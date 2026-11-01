#!/usr/bin/env bash
# The proof a comment-only commit gives on the MacBook, until S314 turns tools/gate.sh green there
# (DEC-336 (4), DEC-337). Run it on the uncommitted batch, before the commit exists. Cheap stages first,
# stops at the first red, touches neither the index nor the working tree.
#
#   adocs/data/S312_local_gate.sh                     every stage
#   adocs/data/S312_local_gate.sh --only s101,format  those stages; the marker says PARTIAL
#   adocs/data/S312_local_gate.sh --exempt PATH       a path that is not comment-only, named by identity;
#                                                     for a tool commit, and never under src/
#   adocs/data/S312_local_gate.sh --read-format FILE STATUS ROOT
#                                                     read a saved `clang-format.sh --check` output
#
# First, always: the index is empty or equal to the tree. The proof reads the tree and a commit takes the index,
# so code staged and then restored in the tree would pass every stage and land.
#
# Stages: identity (tools/comment_budget.py --identical HEAD, which also refuses a `//` comment ending in a
# backslash), anchors, s101, format, suite, bench, citations.
#
# Blind to: a false sentence, which only a reader sees; g++ -Wcomment and major 22's format opinion, until
# the evening's full gate on the workstation; test_fastchess_script, which S314 holds.
#
# Prints S312-GATE-DONE, S312-GATE-PARTIAL or S312-GATE-FAILED as its last line on every exit. Exit 0 on the
# first two, 1 on red or on a stage that could not run.
set -uo pipefail

# DEC-336 (4). A comment commit cannot move it; a code commit moving it says so in its own `Bench:`, and
# EXPECTED_BENCH carries that number here, S101's H1 for one.
expected_bench="${EXPECTED_BENCH:-5707449}"

repo="$(cd -- "$(dirname -- "$0")/../.." && pwd)"
real_git="$(command -v git)"
git()
{
  "$real_git" -C "$repo" "$@"
}

tmp=""
marked=0
completed=0
fail()
{
  marked=1
  echo "S312-GATE-FAILED: $*" >&2
  exit 1
}

cleanup()
{
  if [[ -n "$tmp" ]]; then
    git worktree remove --force "$tmp/candidate" 2> /dev/null
    git worktree prune 2> /dev/null
    rm -rf "$tmp"
  fi
}

trap 'status=$?
      cleanup
      if ((marked == 0 && (status != 0 || completed == 0))); then
        echo "S312-GATE-FAILED: exited $status" >&2
        ((status != 0)) || status=1
      fi
      exit $status' EXIT

# The flagged lines must be tt_cluster_t's by what they are, never by their number: a deleted comment above
# them moves the number, and a line-number match would go red on correct work. Fewer than the two is no red:
# S314 is what removes them.
read_format()
{
  python3 - "$1" "$2" "$3" << 'PY'
import os
import re
import sys

output_file, status, root = sys.argv[1], int(sys.argv[2]), sys.argv[3]
FILE = "src/data_structures.hpp"
HEADER = "struct alignas(64) tt_cluster_t"
BRACE = "{"
MEMBER = "  tt_entry_t entries[TT_CLUSTER_ENTRIES];"
ERROR = re.compile(r"^(.+?):(\d+):(\d+): error: (.*)$")

lines = open(output_file, encoding="utf-8", errors="replace").read().split("\n")
errors = [ERROR.match(line) for line in lines]
errors = [m for m in errors if m]
if status != 0 and not errors:
    print("format: clang-format.sh --check failed and flagged nothing:")
    print("\n".join(lines[:10]))
    sys.exit(1)

source = []
if errors:
    source = open(os.path.join(root, FILE), encoding="utf-8", errors="replace").read().split("\n")


def is_tt_cluster_line(number):
    at = number - 1
    if at < 1 or at >= len(source):
        return False
    if source[at] == BRACE:
        return source[at - 1] == HEADER
    return source[at] == MEMBER and at >= 2 and source[at - 1] == BRACE and source[at - 2] == HEADER


others = []
for m in errors:
    if os.path.normpath(m.group(1)) != FILE or not is_tt_cluster_line(int(m.group(2))):
        others.append(m.group(0))
if others:
    print("format: flagged beyond tt_cluster_t's two lines:")
    print("\n".join(others))
    sys.exit(1)
print(f"format: {len(errors)} flagged, tt_cluster_t's own (S314), no other")
PY
}

check_index()
{
  local staged=0 unstaged=0
  git diff --cached --quiet --ignore-submodules=dirty || staged=$?
  git diff --quiet --ignore-submodules=dirty || unstaged=$?
  ((staged <= 1 && unstaged <= 1)) || fail "the index could not be read: git diff exited $staged and $unstaged"
  ((staged == 0 || unstaged == 0)) \
    || fail "the index is neither empty nor equal to the tree, so what this proves is not what a commit would take: stage the whole batch, or unstage it"
}

stage_identity()
{
  python3 tools/comment_budget.py --identical HEAD ${exempt[@]+"${exempt[@]}"} \
    || fail "identity: a changed file is not proved code-identical to HEAD"
}

stage_anchors()
{
  local out status=0
  out="$(python3 tools/comment_budget.py --anchors)" || status=$?
  printf '%s\n' "$out" | grep -E '^ANCHOR|^COMMENT-BUDGET'
  ((status == 0)) || fail "anchors: a mutant anchor is not found exactly once"
}

# S101_pause_check.sh reads HEAD, so before the commit exists it would read the parent. The batch goes into
# a candidate commit built through a scratch index, reachable from no ref, and the check runs in a throwaway
# worktree of it.
stage_s101()
{
  [[ -n "$tmp" ]] || tmp="$(mktemp -d)" || fail "s101: mktemp"
  local index="$tmp/candidate_index" tree commit
  GIT_INDEX_FILE="$index" git read-tree HEAD || fail "s101: read-tree"
  GIT_INDEX_FILE="$index" git add -A . || fail "s101: staging the batch in the scratch index"
  tree="$(GIT_INDEX_FILE="$index" git write-tree)" || fail "s101: write-tree"
  commit="$(git -c user.name=S312-local-gate -c user.email=none@localhost \
    commit-tree "$tree" -p HEAD -m 'S312 local gate candidate')" || fail "s101: commit-tree"
  git worktree add -q --detach "$tmp/candidate" "$commit" || fail "s101: worktree"
  (cd "$tmp/candidate" && bash adocs/data/S101_pause_check.sh) \
    || fail "s101: the batch conflicts with S101's paused diff; that hunk waits for S101's close"
}

stage_format()
{
  [[ -n "$tmp" ]] || tmp="$(mktemp -d)" || fail "format: mktemp"
  local status=0
  ./clang-format.sh --check > "$tmp/format.txt" 2>&1 || status=$?
  read_format "$tmp/format.txt" "$status" "$repo" || fail "format"
}

stage_suite()
{
  # -j8 as the TESTS rule; test_fastchess_script is red here before any comment (S314).
  cmake --build build -j8 \
    && ctest --test-dir build -L fast -E '^test_fastchess_script$' --output-on-failure \
    && cmake --build build-tune -j8 \
    && ctest --test-dir build-tune -L fast -E '^test_fastchess_script$' --output-on-failure \
    || fail "suite: a fast test or a build is red"
}

stage_bench()
{
  local total
  total="$(printf 'bench\nquit\n' | build/src/chesso | grep -E '^[0-9]+ nodes [0-9]+ nps$' | tail -1 | cut -d' ' -f1 || true)"
  [[ -n "$total" ]] || fail "bench: no '<nodes> nodes <nps> nps' line"
  [[ "$total" == "$expected_bench" ]] || fail "bench: $total, expected $expected_bench"
  echo "bench: $total"
}

stage_citations()
{
  python3 tools/plan_prose_check.py --citations | tail -1
  ((PIPESTATUS[0] == 0)) || fail "citations: tools/plan_prose_check.py --citations flagged"
}

all_stages="identity anchors s101 format suite bench citations"
only=""
exempt=()
while (($# > 0)); do
  case "$1" in
    --only)
      (($# > 1)) || fail "--only needs a list of stages"
      only="${2//,/ }"
      shift 2
      ;;
    --exempt)
      (($# > 1)) || fail "--exempt needs a path"
      exempt+=(--exempt "$2")
      shift 2
      ;;
    --read-format)
      (($# == 4)) || fail "--read-format needs FILE STATUS ROOT"
      read_format "$2" "$3" "$4" || fail "format"
      completed=1
      echo "S312-GATE-PARTIAL read-format"
      exit 0
      ;;
    *) fail "unknown argument [$1]" ;;
  esac
done

cd "$repo" || fail "cd $repo"

stages="${only:-$all_stages}"
for stage in $stages; do
  case " $all_stages " in
    *" $stage "*) ;;
    *) fail "unknown stage [$stage]; stages are: $all_stages" ;;
  esac
done

check_index

started="$(date +%s)"
for stage in $stages; do
  echo "--- $stage"
  began="$(date +%s)"
  "stage_$stage"
  echo "--- $stage $(($(date +%s) - began)) s"
done
elapsed=$(($(date +%s) - started))

completed=1
if [[ -n "$only" ]]; then
  echo "S312-GATE-PARTIAL $stages, $elapsed s"
else
  echo "S312-GATE-DONE $expected_bench, $elapsed s"
fi
