#!/usr/bin/env bash
#
# Format-check coverage test for clang-format.sh. S054.
#
# `git ls-files` with no flags lists the index, so before S054 the check was
# blind to a source file that had not been added yet -- and it is one of the
# three commands in the step-completion gate (AGENTS.md's TESTS rule; it was
# .moltke.json's until DEC-109), so every step that added a file got a vacuous
# pass for exactly the files it added. S041's
# completion commit sealed that claim over two files with lines past the column
# limit.
#
# Four assertions, in order. The first two are preconditions: without them a
# script that always exits non-zero, or a fixture that is not actually
# misformatted, would satisfy the two that matter.
#
#   1. a sandbox whose only source file is formatted correctly exits 0
#   2. a misformatted TRACKED file exits non-zero and is named in the output
#   3. a misformatted UNTRACKED file exits non-zero and is named in the output
#   4. a misformatted file inside a gitignored directory exits 0
#   5. a file in the index but deleted from the worktree is not opened
#   6. a code-shaped file under adocs/ is not scanned, tracked or not
#
# 4 is the negative half. Selecting the whole worktree instead of asking git
# would pass 1 to 3 and fail this one, and would then try to format the
# generated sources under build/ and the checked-out engine copies under
# .ref-builds/.
#
# Nothing is read or written outside a throwaway git repository, so the real
# tree is untouched: run `git status --porcelain` before and after and compare.
#
# Usage: test_clang_format_script.sh [path-to-clang-format.sh]
# The argument exists so a past revision of the script can be run through the
# same assertions, which is how this was observed failing against
# `git ls-files` with no flags.

set -uo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
script_under_test="${1:-$repo_root/clang-format.sh}"
failures=0

# Every sandbox is a subdirectory of one root, removed by an EXIT trap. Both
# halves matter: make_sandbox is called in a command substitution, so a
# subshell, and anything it recorded in a variable would be lost by the time
# the cleanup ran -- and a failing assertion must not leave the sandbox it was
# shown from behind either.
sandbox_root="$(mktemp -d "${TMPDIR:-/tmp}/chesso-clang-format.XXXXXX")"
trap 'rm -rf "$sandbox_root"' EXIT
sandbox_count=0

fail()
{
  echo "FAIL: $*" >&2
  failures=$((failures + 1))
}

if [[ ! -r "$script_under_test" ]]; then
  echo "FAIL: no script at $script_under_test" >&2
  exit 1
fi

if [[ ! -r "$repo_root/.clang-format" ]]; then
  echo "FAIL: no .clang-format at $repo_root" >&2
  exit 1
fi

# Text that no clang-format style leaves alone. Written by printf so the file
# this test reasons about is not itself subject to the repository's formatter.
misformatted()
{
  printf 'int  main( ){int    x=1;return\nx;}\n' > "$1"
}

# A sandbox is a git repository holding a copy of the script under test, the
# repository's own .clang-format so the style is the real one, the
# CMakeLists.txt the script requires in its working directory, a .gitignore
# naming the trees the real one ignores, and one correctly formatted tracked
# source file so the selection is never empty. Prints the directory.
make_sandbox()
{
  local tmp
  tmp="$(mktemp -d "$sandbox_root/case.XXXXXX")"

  mkdir -p "$tmp/src" "$tmp/build" "$tmp/.ref-builds"
  : > "$tmp/CMakeLists.txt"
  cp "$repo_root/.clang-format" "$tmp/.clang-format"
  cp "$script_under_test" "$tmp/clang-format.sh"
  chmod +x "$tmp/clang-format.sh"

  printf 'build\n.ref-builds\n' > "$tmp/.gitignore"

  # Deliberately dull: a declaration and a comment, with no brace, no body and
  # no line for a style option to rewrite. A short function body is not safe
  # here -- AllowShortBlocksOnASingleLine collapses it under clang-format 22
  # and leaves it alone under 18, which would make assertion 1 depend on which
  # version the script resolved.
  printf '// Formatted, and nothing here for a style option to rewrite.\nint format_fixture_ok = 0;\n' \
    > "$tmp/src/ok.cpp"

  git -C "$tmp" init -q
  git -C "$tmp" config user.email format@example.invalid
  git -C "$tmp" config user.name format
  # The check's file selection reads the standard exclude sources. Pinning the
  # global one to empty keeps the sandbox's own .gitignore the only thing that
  # decides what is ignored, whatever the machine's git config says.
  git -C "$tmp" config core.excludesFile /dev/null
  git -C "$tmp" add CMakeLists.txt .clang-format .gitignore src/ok.cpp
  git -C "$tmp" commit -q -m sandbox

  echo "$tmp"
}

# Runs the sandboxed script with --check and prints its exit status. Output
# lands in out.txt inside the sandbox so a failing assertion can show it.
run_check()
{
  local tmp="$1"
  (
    cd "$tmp" || exit 127
    ./clang-format.sh --check
  ) > "$tmp/out.txt" 2>&1
  echo $?
}

show()
{
  echo "--- $1/out.txt ---" >&2
  sed 's/^/    /' "$1/out.txt" >&2
  echo "--- end ---" >&2
}

# 1. Precondition. A clean sandbox passes.
#
# Every assertion below reads a non-zero status as "the misformatted file was
# seen". That only follows if the same sandbox without it exits 0 -- otherwise a
# script that cannot find its clang-format, or one that always fails, satisfies
# 2 and 3 by accident.
clean_dir="$(make_sandbox)"
clean_status="$(run_check "$clean_dir")"

if ((clean_status != 0)); then
  fail "a sandbox with only correctly formatted sources exited $clean_status;" \
       "nothing below this proves anything"
  show "$clean_dir"
fi

# 2. Precondition. A misformatted tracked file is caught.
#
# This is what the selection has always done. It is asserted so that the
# untracked case below is evidence about the selection rather than about the
# fixture: if this text is not misformatted under this .clang-format, or the
# check does not run, assertion 3 would pass for the wrong reason.
tracked_dir="$(make_sandbox)"
misformatted "$tracked_dir/src/bad.cpp"
git -C "$tracked_dir" add src/bad.cpp
git -C "$tracked_dir" commit -q -m bad
tracked_status="$(run_check "$tracked_dir")"

if ((tracked_status == 0)); then
  fail "a misformatted TRACKED file was not caught;" \
       "the fixture is not misformatted or the check did not run"
  show "$tracked_dir"
elif ! grep -q 'src/bad\.cpp' "$tracked_dir/out.txt"; then
  fail "the tracked case failed without naming src/bad.cpp;" \
       "the non-zero status came from somewhere else"
  show "$tracked_dir"
fi

# 3. The defect. A misformatted untracked file is caught.
untracked_dir="$(make_sandbox)"
misformatted "$untracked_dir/src/bad.cpp"
untracked_status="$(run_check "$untracked_dir")"

if ((untracked_status == 0)); then
  fail "a misformatted UNTRACKED source file passed the check (exit 0);" \
       "the file selection lists the index only"
  show "$untracked_dir"
elif ! grep -q 'src/bad\.cpp' "$untracked_dir/out.txt"; then
  fail "the untracked case failed without naming src/bad.cpp;" \
       "the non-zero status came from somewhere else"
  show "$untracked_dir"
fi

# 4. The negative half. A misformatted file under a gitignored path is skipped.
#
# build/ and .ref-builds/ hold generated and checked-out sources that this
# repository does not format. A selection that walked the worktree would fail
# here, and would rewrite them in the script's formatting mode.
ignored_dir="$(make_sandbox)"
misformatted "$ignored_dir/build/generated.cpp"
misformatted "$ignored_dir/.ref-builds/copy.hpp"
ignored_status="$(run_check "$ignored_dir")"

if ((ignored_status != 0)); then
  fail "a misformatted file under a gitignored path was scanned (exit" \
       "$ignored_status); build/ and .ref-builds/ must stay excluded"
  show "$ignored_dir"
fi

# 5. A file in the index but gone from the worktree is not handed to
# clang-format, which cannot open it. `git ls-files --cached` still names it, so
# without the filter the check fails with "No such file or directory" on a path
# nobody asked about. The old selection had the same weakness.
deleted_dir="$(make_sandbox)"
printf '// Formatted.\nint format_fixture_gone = 0;\n' > "$deleted_dir/src/gone.cpp"
git -C "$deleted_dir" add src/gone.cpp
git -C "$deleted_dir" commit -q -m gone
rm "$deleted_dir/src/gone.cpp"

# Non-vacuous by construction: the status assertion below says nothing unless
# the index really still names the missing file.
if ! git -C "$deleted_dir" ls-files | grep -q '^src/gone\.cpp$'; then
  fail "the index no longer names the deleted file;" \
       "the deleted-file case proves nothing"
else
  deleted_status="$(run_check "$deleted_dir")"
  if ((deleted_status != 0)); then
    fail "a file deleted from the worktree but still in the index was handed" \
         "to clang-format (exit $deleted_status)"
    show "$deleted_dir"
  fi
fi

# 6. adocs/ is evidence, not source, and is never formatted. S075.
#
# `tools/tuner` emits a pasteable C++ header, and a step that keeps one as
# evidence keeps it under adocs/data/ -- six of them at S075. The value of that
# file is that it is byte for byte what the tool wrote: reformatting it destroys
# the thing it is kept for, and --check turns it into a step-completion failure
# nobody can fix without corrupting the record. The tracked half matters as much
# as the untracked one, since evidence is committed.
#
# Assertion 2 is this case's precondition: the same bytes in src/ exit non-zero,
# so a zero here is the path being excluded rather than the fixture being
# formatted or the check not running.
adocs_dir="$(make_sandbox)"
mkdir -p "$adocs_dir/adocs/data"
misformatted "$adocs_dir/adocs/data/emitted_tracked.hpp"
misformatted "$adocs_dir/adocs/data/emitted_untracked.hpp"
git -C "$adocs_dir" add adocs/data/emitted_tracked.hpp
git -C "$adocs_dir" commit -q -m evidence
adocs_status="$(run_check "$adocs_dir")"

if ((adocs_status != 0)); then
  fail "a code-shaped file under adocs/ was scanned (exit $adocs_status);" \
       "emitted evidence must not be reformatted"
  show "$adocs_dir"
fi

# Non-vacuous by construction, once more: six cases were meant to run, and a
# count below that means an assertion was skipped rather than satisfied.
sandbox_count="$(find "$sandbox_root" -mindepth 1 -maxdepth 1 -type d | wc -l)"
if ((sandbox_count != 6)); then
  fail "$sandbox_count of 6 sandboxes were built; a case did not run"
fi

if ((failures > 0)); then
  echo "$script_under_test: $failures assertion(s) failed" >&2
  exit 1
fi

echo "$script_under_test: catches misformatted tracked and untracked sources," \
     "skips gitignored ones, passes a clean tree"
