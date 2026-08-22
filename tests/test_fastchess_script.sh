#!/usr/bin/env bash
#
# Smoke test for fastchess.sh. S035, closing 2026-08-13_adversarial-F01.
#
# Eight properties. The second is the one that stops F01 recurring; 3 to 8 are
# the default reference and the A/A guard -- the trap S160 disarmed and the
# defects its own first attempt shipped, each one reproduced before it was fixed:
#
#   1. the script reaches the `fastchess` invocation
#   2. a script that aborts before that point exits non-zero
#   3. with REF unset the reference is HEAD, printed with both commit dates
#   4. with REF unset and a clean tree the run is refused, not played, and
#      nothing is left on disk
#   5. the reference line carries the *reference's* date, not HEAD's
#   6. an explicit ref that resolves to HEAD is refused too: the guard reads
#      state, not who chose the ref
#   7. AA=1 is how the A/A calibration is asked for, and it says so on screen
#   8. a run outside a git checkout still prints a terminal marker
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
  # The book the script reads is 175 MB and gitignored (books/fetch_book.sh
  # fetches it), so the sandbox seeds an empty file of the same name: the
  # script checks that the path is readable, and fastchess is a stub that
  # never opens it. The name has to track fastchess.sh -- a renamed book fails
  # this test at the readability check rather than silently.
  : > "$tmp/books/UHO_Lichess_4852_v1.epd"

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

  # Two commits over one tracked file, and a reference build for each.
  #
  # Tracked, because case 3 needs a working tree that differs from HEAD and the
  # script asks `git diff --quiet HEAD`, which untracked files never answer.
  #
  # Two of them, because case 5 needs a reference whose commit date differs from
  # HEAD's -- with every case at ref == HEAD, printing HEAD's date on the
  # reference line is invisible, which is exactly the mutation that survived the
  # first version of this test. Only the committer date of the older commit is
  # forced, so `%cd` reads 2020-01-02 while `%ad` reads today: a banner built
  # from the author date fails this too.
  git -C "$tmp" init -q
  : > "$tmp/tracked.txt"
  git -C "$tmp" add tracked.txt
  GIT_COMMITTER_DATE="2020-01-02T03:04:05 +0000" \
    git -C "$tmp" -c user.email=smoke@example.invalid -c user.name=smoke \
      commit -q -m "smoke, parent"
  echo second >> "$tmp/tracked.txt"
  git -C "$tmp" add tracked.txt
  git -C "$tmp" -c user.email=smoke@example.invalid -c user.name=smoke \
    commit -q -m "smoke, head"

  for rev in HEAD HEAD~1; do
    sha="$(git -C "$tmp" rev-parse --short "$rev")"
    mkdir -p "$tmp/.ref-builds/$sha/build/src"
    printf '#!/bin/sh\nexit 0\n' > "$tmp/.ref-builds/$sha/build/src/chesso"
    chmod +x "$tmp/.ref-builds/$sha/build/src/chesso"
  done

  echo "$tmp"
}

# Runs the sandboxed script and prints its exit status. Output lands in
# out.txt inside the sandbox so a failing assertion can show it, and OUT keeps
# the per-run pgn/log directory inside the sandbox as well, so a smoke run
# leaves nothing in /tmp.
#
# The second argument is the REF to pass and the third is AA. With neither, both
# are left unset, which is what cases 3, 4 and 7 are about: the default is the
# thing under test, so it cannot be supplied by the harness. They are unset
# explicitly rather than merely not set, so a REF or AA exported into the test's
# own environment cannot decide the result.
run_sandbox()
{
  local tmp="$1" ref="${2-}" aa="${3-}"
  (
    cd "$tmp" || exit 127
    export PATH="$tmp/stub:$PATH" OUT="$tmp/out"
    if [[ -n "$ref" ]]; then
      export REF="$ref"
    else
      unset REF
    fi
    if [[ -n "$aa" ]]; then
      export AA="$aa"
    else
      unset AA
    fi
    ./fastchess.sh --fast
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
#
# REF=HEAD~1 rather than HEAD, here and in case 2: the sandbox tree is clean,
# and a reference resolving to HEAD on a clean tree is the A/A case that cases
# 4, 6 and 7 are about. These two are about reaching fastchess at all, so they
# stay off that path.
reached_dir="$(make_sandbox "$script_under_test")"
reached_status="$(run_sandbox "$reached_dir" HEAD~1)"

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
  abort_status="$(run_sandbox "$abort_dir" HEAD~1)"
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

# 3. With REF unset the reference is HEAD, and the banner says so.
#
# The default it replaced was the fixed sha 7b4d9a4, the 2026-08-08 baseline: a
# bare --fast run reached H1 in minutes whatever the change under test was
# (S160, 2026-08-22_adversarial-F02). The assertion reads the banner because
# the banner is all a run's reader has, and it pins the commit date beside each
# sha -- a sha pair alone never showed how far apart the two sides were.
#
# The tracked file is modified first: REF unset on a clean tree is case 4's
# refusal, so without a real diff this case would assert on the wrong path.
default_dir="$(make_sandbox "$script_under_test")"
echo change >> "$default_dir/tracked.txt"
default_status="$(run_sandbox "$default_dir")"
default_sha="$(git -C "$default_dir" rev-parse --short HEAD)"
date_re='[0-9]{4}-[0-9]{2}-[0-9]{2}'

if ((default_status != 0)); then
  fail "REF unset: the script exited $default_status"
  show "$default_dir"
elif ! grep -qE "^reference  $default_sha  $date_re\$" "$default_dir/out.txt"; then
  fail "REF unset: the banner does not name HEAD ($default_sha) and its date as the reference"
  show "$default_dir"
elif ! grep -qE "^candidate  $default_sha  $date_re  [+] uncommitted changes\$" "$default_dir/out.txt"; then
  fail "REF unset: the banner does not name the candidate's sha, date and uncommitted diff"
  show "$default_dir"
fi

# 4. With REF unset and nothing uncommitted, the run is refused.
#
# Both sides would be the same build, and an SPRT between identical engines
# does not return zero -- it random-walks until a bound is crossed by luck. Case
# 7 is this case's positive control: the same sandbox, the same clean tree, the
# same resolved ref, AA=1 set, and the stub is invoked. So what is asserted here
# is the guard and not some other reason the run could not start.
clean_dir="$(make_sandbox "$script_under_test")"
clean_status="$(run_sandbox "$clean_dir")"

if [[ -e "$clean_dir/fastchess_invoked" ]]; then
  fail "REF unset on a clean tree played a match between two identical builds"
  show "$clean_dir"
elif ((clean_status == 0)); then
  fail "REF unset on a clean tree exited 0 instead of refusing"
  show "$clean_dir"
elif ! grep -q 'SPRT-RUN-FAILED.*both sides are the same build' "$clean_dir/out.txt"; then
  # Pinned to the guard's own sentence, not to the bare marker: every fail() in
  # the script prints SPRT-RUN-FAILED, so a bare grep would keep this case green
  # if the guard were removed and some earlier refusal took its place.
  fail "REF unset on a clean tree refused, but not with the A/A guard's message"
  show "$clean_dir"
elif [[ -e "$clean_dir/out" ]]; then
  # The guard runs before `mkdir -p "$outdir"`, so a refused run leaves no
  # stamped output directory behind. It sat after the mkdir when first written.
  fail "the refused run created its output directory anyway"
  show "$clean_dir"
fi

# 5. The reference line carries the reference's own commit date.
#
# Cases 1 to 4 all run with ref == HEAD, where a banner that printed HEAD's date
# on the reference line would be indistinguishable from a correct one -- and
# that mutation did survive the first version of this test. HEAD~1's committer
# date is forced to 2020-01-02 by make_sandbox, so the two dates cannot collide.
older_dir="$(make_sandbox "$script_under_test")"
older_status="$(run_sandbox "$older_dir" HEAD~1)"
older_sha="$(git -C "$older_dir" rev-parse --short HEAD~1)"
older_head="$(git -C "$older_dir" rev-parse --short HEAD)"

if ((older_status != 0)); then
  fail "REF=HEAD~1: the script exited $older_status"
  show "$older_dir"
elif ! grep -qE "^reference  $older_sha  2020-01-02\$" "$older_dir/out.txt"; then
  fail "REF=HEAD~1: the reference line does not carry HEAD~1's sha and its own date"
  show "$older_dir"
elif ! grep -qE "^candidate  $older_head  $(date +%F)\$" "$older_dir/out.txt"; then
  fail "REF=HEAD~1: the candidate line does not carry HEAD's sha and today's date"
  show "$older_dir"
fi

# 6. An explicit ref that resolves to HEAD is refused as well.
#
# The guard reads state, not provenance. Asking whether REF was passed let
# REF=<HEAD's own sha>, REF=master and REF=$(git rev-parse HEAD) -- the form the
# saved runner scripts under adocs/data/ use -- play the full A/A anyway, which
# is the run that has nothing in it. The full sha is passed here because it is
# the spelling furthest from the word HEAD.
explicit_dir="$(make_sandbox "$script_under_test")"
explicit_status="$(run_sandbox "$explicit_dir" "$(git -C "$explicit_dir" rev-parse HEAD)")"

if [[ -e "$explicit_dir/fastchess_invoked" ]]; then
  fail "an explicit ref resolving to HEAD on a clean tree played the A/A match"
  show "$explicit_dir"
elif ((explicit_status == 0)); then
  fail "an explicit ref resolving to HEAD on a clean tree exited 0 instead of refusing"
  show "$explicit_dir"
fi

# 7. AA=1 is how that match is asked for, and the run says so.
#
# The calibration is a real use -- it measures the harness rather than the
# engine -- so it stays reachable. It is reached by naming it, which is what
# tells it apart from a bare run that has nothing in it.
aa_dir="$(make_sandbox "$script_under_test")"
aa_status="$(run_sandbox "$aa_dir" "" 1)"

if [[ ! -e "$aa_dir/fastchess_invoked" ]]; then
  fail "AA=1 on a clean tree was refused (exit status $aa_status)"
  show "$aa_dir"
elif ((aa_status != 0)); then
  fail "AA=1 reached fastchess but the script exited $aa_status"
  show "$aa_dir"
elif ! grep -q 'A/A CALIBRATION' "$aa_dir/out.txt"; then
  fail "AA=1 played the match without saying it measures the harness"
  show "$aa_dir"
fi

# 8. A run outside a git checkout still prints a terminal marker.
#
# Every git call is fatal there, and the marker is what a watcher breaks on: the
# fallback loop DEV_MANUAL.md teaches has no ceiling of its own, so a silent
# abort spins it forever. The first version of this change moved a git call
# above the trap and lost the marker on exactly this path.
nogit_dir="$(make_sandbox "$script_under_test")"
rm -rf "$nogit_dir/.git"
nogit_status="$(run_sandbox "$nogit_dir" "")"

if ((nogit_status == 0)); then
  fail "a run outside a git checkout exited 0"
  show "$nogit_dir"
elif ! grep -q 'SPRT-RUN-FAILED' "$nogit_dir/out.txt"; then
  fail "a run outside a git checkout aborted without a terminal marker"
  show "$nogit_dir"
fi

rm -rf "$reached_dir" "$default_dir" "$clean_dir" "$older_dir" \
       "$explicit_dir" "$aa_dir" "$nogit_dir"
[[ -n "${abort_dir:-}" ]] && rm -rf "$abort_dir"

if ((failures > 0)); then
  echo "$script_under_test: $failures assertion(s) failed" >&2
  exit 1
fi

echo "$script_under_test: 8 properties hold -- reaches fastchess, aborts non-zero, defaults REF to HEAD, dates each side from its own commit, refuses a clean-tree A/A however the ref is spelled, honours AA=1, marks an abort with no git"
