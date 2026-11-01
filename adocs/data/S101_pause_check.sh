#!/usr/bin/env bash
# Whether S101's paused phase 2 still lands on the tree as it stands (DEC-335). S101's diff is uncommitted
# on the workstation until its verdict, and comment work on `achesso` can overlap it, so each comment
# commit proves the overlap away instead of leaving it for the close. A throwaway worktree of HEAD takes
# adocs/data/S101_phase2_pause.diff three-way. Exit 0 and S101-PAUSE-OK with no conflict; exit 1,
# naming each conflicted path, with one. Touches neither the index nor the working tree here.
#
#   adocs/data/S101_pause_check.sh
set -uo pipefail
root="$(git rev-parse --show-toplevel)" || exit 1
diff="$root/adocs/data/S101_phase2_pause.diff"
[[ -f "$diff" ]] || { echo "S101-PAUSE-FAILED: no $diff"; exit 1; }
tmp="$(mktemp -d)" || exit 1
trap 'git -C "$root" worktree remove --force "$tmp/w" 2>/dev/null; rm -rf "$tmp"' EXIT
git -C "$root" worktree add -q --detach "$tmp/w" HEAD || { echo "S101-PAUSE-FAILED: worktree"; exit 1; }
git -C "$tmp/w" apply --index --3way --whitespace=nowarn "$diff" > "$tmp/apply.log" 2>&1
status=$?
conflicts="$(git -C "$tmp/w" diff --name-only --diff-filter=U)"
if [[ -n "$conflicts" || $status -ne 0 ]]; then
  echo "conflicts with S101's paused diff; leave these hunks alone until S101 closes:"
  git -C "$tmp/w" diff --diff-filter=U | grep -nE '^(diff --cc|@@@)' | cut -c1-120
  [[ -n "$conflicts" ]] || cut -c1-160 "$tmp/apply.log"
  echo "S101-PAUSE-FAILED"
  exit 1
fi
echo "S101-PAUSE-OK"
