#!/usr/bin/env bash
#
# Smoke test for fastchess.sh. S035, closing 2026-08-13_adversarial-F01.
#
# Twenty-six properties. The second is the one that stops F01 recurring; 3 to 8
# are the default reference and the A/A guard -- the trap S160 disarmed and the
# defects its own first attempt shipped, each one reproduced before it was
# fixed; 9 to 11 are S198's seed, PGN fields and fixed-rounds mode; 12 to 15
# are S151's control, hash and commit-candidate overrides, each asserted on
# the argv as well as on the banner, and each with its default asserted beside
# it so a hard-wired override cannot pass; 16 and 17 are the reference build
# itself, which every case before them skips; 18 to 26 are S212's, one per
# finding of the 2026-09-10 audit's harness set -- F04, F06, F05, F31 and F32
# in that order:
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
#   9. the banner prints the seed fastchess is given, and SRAND overrides it
#  10. the pgn records node counts and the clock left after every move
#  11. ROUNDS runs fixed rounds with no SPRT, and the banner says it is not a
#      verdict
#  12. TC reaches the engines and the banner prints it; unset, it is 8+0.08
#  13. HASH reaches the engines and the banner prints it; unset, it is 16
#  14. CAND plays that commit's own build rather than the working tree's, and
#      the candidate line is the commit, dated, with no dirty flag
#  15. CAND resolving to REF is refused with the A/A guard's sentence on a
#      dirty tree, and AA=1 is how it is asked for on purpose
#  16. a commit with no cached worktree is built, and that build is the one
#      played, on both sides of a CAND run
#  17. a build that fails stops the run, with a marker naming the sha, before
#      a game is played -- on both sides
#  18. resignation is adjudicated two-sided, at the score and movecount
#      DEC-174 fixed
#  19. a cached reference whose worktree is dirty is rebuilt before it plays
#  20. a cached reference whose worktree sits at another commit is rebuilt
#  21. a cached reference configured for another CHESSO_ARCH is rebuilt
#  22. a cache that answers all three questions is played, and nothing is built
#  23. the banner prints the candidate's arch and tune, and the reference is
#      configured with them
#  24. a side whose `id name` sha is not the one it was labelled with is
#      refused before a game; an engine predating the stamp plays, with a line
#      saying the check did not happen
#  25. a crash or disconnect termination voids the run -- SPRT-RUN-INVALID,
#      then a terminal marker as the last line, non-zero -- and a PGN holding
#      only expected terminations still reaches SPRT-RUN-DONE
#  26. the busy guard reads the one-minute load average, not `ps`'s lifetime
#      percentages
#
# No game is played and no engine is compiled. `fastchess` is a stub on PATH,
# the candidate and the reference are one-line shell scripts, and the whole run
# happens inside a throwaway git repository, so the real .ref-builds/ and the
# real build/ are never read or written. Cases 16, 17 and 19 to 23 stub `cmake`
# as well and let `git worktree add` run for real inside that repository.
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
  local tmp sha head_sha

  tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-fastchess-smoke.XXXXXX")"
  mkdir -p "$tmp/build/src" "$tmp/books" "$tmp/stub"

  # The book the script reads is 9.4 MB unpacked and gitignored
  # (books/fetch_book.sh fetches it), so the sandbox seeds an empty file of
  # the same name: the script checks that the path is readable, and
  # fastchess is a stub that never opens it. The name has to track
  # fastchess.sh -- a renamed book fails this test at the readability check
  # rather than silently.
  : > "$tmp/books/noob_3moves.epd"

  # The stub records that it ran and plays nothing. It reports its own
  # invocation through a file rather than through stdout, so the assertion
  # cannot be satisfied by the script merely echoing the word fastchess.
  #
  # It also writes its argument vector one argument per line, which is what
  # cases 9 to 11 read: what the banner says and what fastchess is handed are
  # two different claims, and only the second one decides which games are
  # played.
  #
  # And it asks each binary it was pointed at which one it is, in the order the
  # engines were given, into fastchess_engines. See the stub engines above for
  # why that is asked instead of read off the `cmd=` path.
  #
  # And it writes a PGN where it was told to put one, but only when the
  # sandbox left a `stub_pgn` file beside it: the census fastchess.sh runs
  # after the match reads that file and nothing else, so case 25's crash
  # termination has to arrive through it. Without the file the stub writes no
  # PGN at all, which is what every case before 25 assumes -- a census over a
  # missing file reads as zero games.
  cat > "$tmp/stub/fastchess" << 'STUB'
#!/bin/sh
here="$(dirname "$0")/.."
touch "$here/fastchess_invoked"
printf '%s\n' "$@" > "$here/fastchess_args"
: > "$here/fastchess_engines"
pgn=""
prev=""
for arg in "$@"; do
  case "$arg" in
    cmd=*) "${arg#cmd=}" >> "$here/fastchess_engines" ;;
    file=*) [ "$prev" = "-pgnout" ] && pgn="${arg#file=}" ;;
  esac
  prev="$arg"
done
if [ -n "$pgn" ] && [ -f "$here/stub_pgn" ]; then
  cat "$here/stub_pgn" > "$pgn"
fi
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
  #
  # `.gitignore` is tracked and holds `build`, exactly as the real repository's
  # does, because the cached worktrees below are judged by `git status
  # --porcelain` being empty and each one carries its own `build/`. Case 19 is
  # about a *modified tracked file*, which is the state the real
  # `.ref-builds/2b54a4f` was found in.
  git -C "$tmp" init -q
  : > "$tmp/tracked.txt"
  printf 'build\n' > "$tmp/.gitignore"
  git -C "$tmp" add tracked.txt .gitignore
  GIT_COMMITTER_DATE="2020-01-02T03:04:05 +0000" \
    git -C "$tmp" -c user.email=smoke@example.invalid -c user.name=smoke \
      commit -q -m "smoke, parent"
  echo second >> "$tmp/tracked.txt"
  git -C "$tmp" add tracked.txt
  git -C "$tmp" -c user.email=smoke@example.invalid -c user.name=smoke \
    commit -q -m "smoke, head"

  # Each stub engine says which one it is when run, and the stub fastchess
  # above runs every binary it is handed and records what it said. That is the
  # only way to tell the two candidate sources apart: fastchess.sh copies the
  # candidate to a `mktemp` snapshot before the first game (DEC-020) and
  # removes it on exit, so the argv's `cmd=` is a random /tmp path in both
  # modes and the file it named is gone by the time an assertion could read
  # it. What the binary is survives the copy; where it came from does not.
  # Case 14.
  #
  # Each also answers `uci` with the `id name` line the real engine carries
  # since S212 -- `Chesso <sha> <arch>` -- because the script asks both sides
  # who they are before the first game and refuses a mismatch (case 24). The
  # working-tree stub answers HEAD's sha undecorated: `-dirty` is allowed
  # there and not required, so a clean answer passes on a dirty tree, which is
  # what cases 3 and 14 need.
  head_sha="$(git -C "$tmp" rev-parse --short HEAD)"
  stub_engine "$tmp/build/src/chesso" "$head_sha" working-tree

  # The candidate's own configuration, which the banner prints and which the
  # reference worktrees below are checked against.
  printf 'CHESSO_ARCH:STRING=native\nCHESSO_TUNE:BOOL=OFF\n' \
    > "$tmp/build/CMakeCache.txt"

  # A cached reference is a real git worktree in the real thing, so it is one
  # here: `build_ref` asks whether it is clean and at its own sha before
  # playing it, and a plain directory is neither (cases 19 to 22).
  for rev in HEAD HEAD~1; do
    sha="$(git -C "$tmp" rev-parse --short "$rev")"
    git -C "$tmp" worktree add --detach -q "$tmp/.ref-builds/$sha" "$rev"
    mkdir -p "$tmp/.ref-builds/$sha/build/src"
    printf 'CHESSO_ARCH:STRING=native\nCHESSO_TUNE:BOOL=OFF\n' \
      > "$tmp/.ref-builds/$sha/build/CMakeCache.txt"
    stub_engine "$tmp/.ref-builds/$sha/build/src/chesso" "$sha" "ref-build $sha"
  done

  echo "$tmp"
}

# One stub engine: it answers `uci` with an `id name` line carrying the sha it
# is given, and then prints the word that identifies where it came from, which
# is what the stub fastchess records. Both halves matter and neither
# substitutes for the other -- the `id name` is what the script's own identity
# check reads, the word is what the assertions read.
stub_engine()
{
  local path="$1" sha="$2" word="$3"

  printf '#!/bin/sh\nprintf "id name Chesso %s native\\nuciok\\n"\necho %s\nexit 0\n' \
    "$sha" "$word" > "$path"
  chmod +x "$path"
}

# Turns a sandbox into one where `build_ref`'s build branch is actually taken.
#
# Every sandbox above pre-builds `.ref-builds/<sha>/build/src/chesso` for both
# commits, so `if [[ ! -x "$binary" ]]` is false on every path and the three
# commands inside it -- the worktree, the configure, the build -- never run in
# any of cases 1 to 15. That is the gap cases 16 and 17 close: the branch that
# is skipped is exactly the branch the real run takes, because a commit that
# has never been built has no cached worktree (2026-09-12 fast check).
#
# `git worktree add` is left real: the sandbox is a git repository and the
# checkout is one empty file. `cmake` is the stub, and it is the whole
# difference between the two cases -- `ok` produces a binary that says which
# sha it was built from, `fail` produces nothing and exits 1, which is what a
# broken compiler, a missing generator or a failed link all look like from
# here.
unbuild_sandbox()
{
  local tmp="$1" mode="$2"

  # `git worktree remove` and not `rm -rf`: the directories are registered
  # worktrees of the sandbox repository since S212, and a bare removal leaves
  # the registration behind, which makes the `git worktree add` the script is
  # about to run refuse the path.
  local dir
  for dir in "$tmp"/.ref-builds/*; do
    [[ -d "$dir" ]] || continue
    git -C "$tmp" worktree remove --force "$dir" > /dev/null 2>&1 || rm -rf "$dir"
  done
  git -C "$tmp" worktree prune > /dev/null 2>&1 || true
  rm -rf "$tmp/.ref-builds"

  stub_cmake "$tmp" "$mode"
}

# The `cmake` stub, which is what makes a rebuild observable. `ok` produces a
# binary that says which sha it was built from and a CMakeCache.txt carrying
# the configuration it was handed; `fail` produces nothing and exits 1, which
# is what a broken compiler, a missing generator or a failed link all look like
# from here. Either way it records that it ran, which is how case 22 asserts
# that a valid cache is played *without* a build.
stub_cmake()
{
  local tmp="$1" mode="$2"

  if [[ "$mode" == ok ]]; then
    cat > "$tmp/stub/cmake" << 'STUB'
#!/bin/sh
here="$(dirname "$0")/.."
touch "$here/cmake_invoked"

# `cmake --build <dir> --target chesso` is the call that produces the binary.
# <dir> is .ref-builds/<sha>/build, so the sha the binary names itself with is
# read back out of the path -- which is how case 16 tells the two sides of a
# CAND run apart, and how case 24 gets a rebuilt binary whose `id name` matches
# the side it was built for.
if [ "$1" = "--build" ]; then
  sha=$(basename "$(dirname "$2")")
  mkdir -p "$2/src"
  printf '#!/bin/sh\nprintf "id name Chesso %s native\\nuciok\\n"\necho fresh-build %s\nexit 0\n' \
    "$sha" "$sha" > "$2/src/chesso"
  chmod +x "$2/src/chesso"
  exit 0
fi

# The configure call, which is a no-op except for the cache it writes: the
# script hands it the candidate's CHESSO_ARCH and CHESSO_TUNE, and a rebuilt
# worktree that did not record them would be judged stale again on the next
# run -- a rebuild every time. Case 21 reads this file back.
arch=native
tune=OFF
build=""
prev=""
for arg in "$@"; do
  case "$arg" in
    -DCHESSO_ARCH=*) arch="${arg#-DCHESSO_ARCH=}" ;;
    -DCHESSO_TUNE=*) tune="${arg#-DCHESSO_TUNE=}" ;;
  esac
  [ "$prev" = "-B" ] && build="$arg"
  prev="$arg"
done
if [ -n "$build" ]; then
  mkdir -p "$build"
  printf 'CHESSO_ARCH:STRING=%s\nCHESSO_TUNE:BOOL=%s\n' "$arch" "$tune" \
    > "$build/CMakeCache.txt"
fi
exit 0
STUB
  else
    cat > "$tmp/stub/cmake" << 'STUB'
#!/bin/sh
here="$(dirname "$0")/.."
touch "$here/cmake_invoked"
echo "stub cmake: deliberate failure" >&2
exit 1
STUB
  fi
  chmod +x "$tmp/stub/cmake"
}

# Runs the sandboxed script and prints its exit status. Output lands in
# out.txt inside the sandbox so a failing assertion can show it, and OUT keeps
# the per-run pgn/log directory inside the sandbox as well, so a smoke run
# leaves nothing in /tmp.
#
# The second argument is the REF to pass, the third is AA, the fourth SRAND and
# the fifth ROUNDS. With none of them the four are left unset, which is what
# cases 3, 4, 7 and 9 are about: the default is the thing under test, so it
# cannot be supplied by the harness. They are unset explicitly rather than
# merely not set, so a value exported into the test's own environment cannot
# decide the result.
#
# TC, HASH and CAND come through this function's own environment -- written
# `TC=32+0.32 run_sandbox "$dir" HEAD~1` at the call -- rather than as a sixth,
# seventh and eighth positional argument, because five placeholders already
# have to be counted at every call site. They are captured before the subshell
# and then set or unset inside it under the same rule as the five above: the
# default is the thing under test, so the harness never supplies it by
# accident. Cases 12 to 15.
run_sandbox()
{
  local tmp="$1" ref="${2-}" aa="${3-}" srand="${4-}" rounds="${5-}"
  local tc="${TC-}" hash="${HASH-}" cand="${CAND-}"
  (
    cd "$tmp" || exit 127
    export PATH="$tmp/stub:$PATH" OUT="$tmp/out"
    if [[ -n "$tc" ]]; then
      export TC="$tc"
    else
      unset TC
    fi
    if [[ -n "$hash" ]]; then
      export HASH="$hash"
    else
      unset HASH
    fi
    if [[ -n "$cand" ]]; then
      export CAND="$cand"
    else
      unset CAND
    fi
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
    if [[ -n "$srand" ]]; then
      export SRAND="$srand"
    else
      unset SRAND
    fi
    if [[ -n "$rounds" ]]; then
      export ROUNDS="$rounds"
    else
      unset ROUNDS
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
# HEAD's own committer date, read the way the banner reads it. `date +%F` stood
# here until S193 and made the case fail across midnight: the script would print
# the date the commit carries and the test would expect the date the clock says.
# The reference line's forced 2020-01-02 above is what still catches a banner
# printing today's date. S193, 2026-09-04_test_review-F09.
older_head_date="$(git -C "$older_dir" show -s --date=short --format=%cd HEAD)"

if ((older_status != 0)); then
  fail "REF=HEAD~1: the script exited $older_status"
  show "$older_dir"
elif ! grep -qE "^reference  $older_sha  2020-01-02\$" "$older_dir/out.txt"; then
  fail "REF=HEAD~1: the reference line does not carry HEAD~1's sha and its own date"
  show "$older_dir"
elif ! grep -qE "^candidate  $older_head  $older_head_date\$" "$older_dir/out.txt"; then
  fail "REF=HEAD~1: the candidate line does not carry HEAD's sha and its own date"
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

# 9. The banner prints the seed, and fastchess is handed the same one.
#
# fastchess echoes the seed nowhere -- not on stdout, not in its log, not in
# the PGN (measured on alpha 1.8.1 20260720-daa3ea2, S198 section 2) -- so the
# banner is a run's only record of which opening sequence it played. Two
# claims are asserted separately: what the banner says, and what the process
# was actually given. A banner line built from something other than the value
# passed would be invisible to either one alone.
seed_dir="$(make_sandbox "$script_under_test")"
seed_status="$(run_sandbox "$seed_dir" HEAD~1)"
banner_seed="$(sed -n 's/^seed       \([0-9]*\)$/\1/p' "$seed_dir/out.txt")"
passed_seed="$(grep -A1 -x -- '-srand' "$seed_dir/fastchess_args" | tail -1)"

if ((seed_status != 0)); then
  fail "SRAND unset: the script exited $seed_status"
  show "$seed_dir"
elif [[ ! "$banner_seed" =~ ^[0-9]{14}$ ]]; then
  # Fourteen digits is the stamp with its underscore removed, YYYYMMDDHHMMSS,
  # which is what makes the default unique per run and readable as its time.
  fail "SRAND unset: the banner does not print a 14-digit derived seed (got '$banner_seed')"
  show "$seed_dir"
elif [[ "$passed_seed" != "$banner_seed" ]]; then
  fail "the seed passed to fastchess ('$passed_seed') is not the one the banner printed ('$banner_seed')"
  show "$seed_dir"
fi

# The override, which is how a run's openings are replayed. Same two claims.
override_dir="$(make_sandbox "$script_under_test")"
override_status="$(run_sandbox "$override_dir" HEAD~1 "" 424242)"
override_banner="$(sed -n 's/^seed       \([0-9]*\)$/\1/p' "$override_dir/out.txt")"
override_passed="$(grep -A1 -x -- '-srand' "$override_dir/fastchess_args" | tail -1)"

if ((override_status != 0)); then
  fail "SRAND=424242: the script exited $override_status"
  show "$override_dir"
elif [[ "$override_banner" != 424242 || "$override_passed" != 424242 ]]; then
  fail "SRAND=424242 was not honoured (banner '$override_banner', passed '$override_passed')"
  show "$override_dir"
fi

# 10. The PGN records node counts and the clock left after every move.
#
# Both default to false in fastchess, and a census that wants them cannot get
# them back from a match already played. Asserted on the argument vector for
# the same reason as case 9: the stub writes no PGN.
pgn_dir="$(make_sandbox "$script_under_test")"
pgn_status="$(run_sandbox "$pgn_dir" HEAD~1)"

if ((pgn_status != 0)); then
  fail "pgn fields: the script exited $pgn_status"
  show "$pgn_dir"
else
  for field in -pgnout nodes=true timeleft=true; do
    if ! grep -q -x -- "$field" "$pgn_dir/fastchess_args"; then
      fail "fastchess was not passed '$field'"
      show "$pgn_dir"
    fi
  done
fi

# 11. ROUNDS plays fixed rounds with no SPRT, and the banner refuses the word.
#
# A fixed-rounds run is a calibration or a drift reading and never a verdict
# (MEASUREMENT rule): the stopping rule is the round count, so the bounds
# would be a claim nothing tested. The banner has to say that where a reader
# of the log will meet it. AA=1 because a fixed-rounds run of two identical
# builds is what this mode exists for.
rounds_dir="$(make_sandbox "$script_under_test")"
rounds_status="$(run_sandbox "$rounds_dir" "" 1 "" 500)"
passed_rounds="$(grep -A1 -x -- '-rounds' "$rounds_dir/fastchess_args" | tail -1)"

if ((rounds_status != 0)); then
  fail "ROUNDS=500 AA=1: the script exited $rounds_status"
  show "$rounds_dir"
elif [[ "$passed_rounds" != 500 ]]; then
  fail "ROUNDS=500 did not reach fastchess as -rounds 500 (got '$passed_rounds')"
  show "$rounds_dir"
elif grep -q -x -- '-sprt' "$rounds_dir/fastchess_args"; then
  fail "a fixed-rounds run still passed -sprt, so it would stop on a bound"
  show "$rounds_dir"
elif ! grep -q 'fixed 500 rounds' "$rounds_dir/out.txt"; then
  fail "the banner does not say the run is fixed at 500 rounds"
  show "$rounds_dir"
elif ! grep -q 'NOT a verdict' "$rounds_dir/out.txt"; then
  fail "the banner does not say a fixed-rounds run is not a verdict"
  show "$rounds_dir"
fi

# 12. TC=<control> reaches the engines, and the banner says so.
#
# Two claims, asserted separately for the same reason as case 9: a banner line
# built from something other than the value passed would satisfy either one
# alone. The argv is the one that decides which games are played.
#
# The control exists because every verdict this project has taken was taken at
# one control, and a vector tuned at a short one is the class the published
# record says regresses at a longer one (S151).
tc_dir="$(make_sandbox "$script_under_test")"
tc_status="$(TC=32+0.32 run_sandbox "$tc_dir" HEAD~1)"

if ((tc_status != 0)); then
  fail "TC=32+0.32: the script exited $tc_status"
  show "$tc_dir"
elif ! grep -q -x -- 'tc=32+0.32' "$tc_dir/fastchess_args"; then
  fail "TC=32+0.32 did not reach fastchess in the -each line"
  show "$tc_dir"
elif ! grep -qE '^tc 32\+0\.32  ' "$tc_dir/out.txt"; then
  fail "the banner does not print the time control override"
  show "$tc_dir"
fi

# The default is the thing under test as much as the override is: a mutation
# that hard-wires 32+0.32 would pass everything above.
if ! grep -q -x -- 'tc=8+0.08' "$reached_dir/fastchess_args"; then
  fail "TC unset: fastchess was not given the default 8+0.08"
  show "$reached_dir"
fi

# 13. HASH=<n> reaches the engines, and the banner says so.
#
# It travels with TC and not separately: four times the clock is about four
# times the nodes a game writes, so a longer control at the default 16 MB
# quadruples the overwrites per entry that DEC-088 fixed the default to hold.
hash_dir="$(make_sandbox "$script_under_test")"
hash_status="$(HASH=64 run_sandbox "$hash_dir" HEAD~1)"

if ((hash_status != 0)); then
  fail "HASH=64: the script exited $hash_status"
  show "$hash_dir"
elif ! grep -q -x -- 'option.Hash=64' "$hash_dir/fastchess_args"; then
  fail "HASH=64 did not reach fastchess in the -each line"
  show "$hash_dir"
elif ! grep -qE '^tc [^ ]+  hash 64  ' "$hash_dir/out.txt"; then
  fail "the banner does not print the hash override"
  show "$hash_dir"
fi

if ! grep -q -x -- 'option.Hash=16' "$reached_dir/fastchess_args"; then
  fail "HASH unset: fastchess was not given the default 16"
  show "$reached_dir"
fi

# 14. CAND=<ref> plays that commit's build and not the working tree's.
#
# This is the DEC-020 class on the candidate side: a change that resolves CAND
# for the banner and still hands fastchess `build/src/chesso` prints a run that
# is attributable to a commit and plays one that is not. The `cmd=` path cannot
# see it -- the candidate is snapshotted to a random /tmp name in both modes --
# so the assertion reads the identifying line of the binary fastchess was
# handed, which survives the copy. make_sandbox's stub engines carry it.
#
# The tree is dirtied first, and that is not decoration. The dirty flag belongs
# to the working tree and the working tree is not being played, so a candidate
# line carrying `+ uncommitted changes` beside a commit would be saying the
# opposite of what the run measures -- and on a clean tree there is no flag to
# print, which would make the assertion below pass against that mutation.
#
# THE CANDIDATE IS HEAD~1 AND THE REFERENCE IS HEAD, WHICH IS THE WAY ROUND
# THAT CAN FAIL. With CAND=HEAD the candidate's sha is HEAD's sha and its
# commit date is HEAD's date, so `echo "candidate  $head_sha ..."` and
# `commit_date HEAD` both print exactly what the correct line prints and the
# case is blind to either mutant -- both were observed surviving it
# (2026-09-12 fast check over S151's harness). HEAD~1 carries a different sha
# and the forced committer date 2020-01-02, so the two claims separate.
cand_dir="$(make_sandbox "$script_under_test")"
echo change >> "$cand_dir/tracked.txt"
cand_status="$(CAND=HEAD~1 run_sandbox "$cand_dir" HEAD)"
cand_head="$(git -C "$cand_dir" rev-parse --short HEAD)"
cand_older="$(git -C "$cand_dir" rev-parse --short HEAD~1)"
cand_head_date="$(git -C "$cand_dir" show -s --date=short --format=%cd HEAD)"

if ((cand_status != 0)); then
  fail "CAND=HEAD~1 REF=HEAD: the script exited $cand_status"
  show "$cand_dir"
elif ! grep -q -x -- "ref-build $cand_older" "$cand_dir/fastchess_engines"; then
  fail "CAND=HEAD~1: fastchess was not handed the .ref-builds binary for $cand_older"
  show "$cand_dir"
elif grep -q -x -- 'working-tree' "$cand_dir/fastchess_engines"; then
  fail "CAND=HEAD~1: fastchess was handed build/src/chesso, so the tree was played"
  show "$cand_dir"
elif ! grep -qE "^candidate  $cand_older  2020-01-02\$" "$cand_dir/out.txt"; then
  fail "CAND=HEAD~1: the candidate line is not that commit, its own date, undecorated"
  show "$cand_dir"
elif ! grep -qE "^reference  $cand_head  $cand_head_date\$" "$cand_dir/out.txt"; then
  fail "CAND=HEAD~1: the reference line does not carry HEAD's sha and its own date"
  show "$cand_dir"
elif ! grep -q -x -- "name=cand-$cand_older" "$cand_dir/fastchess_args"; then
  fail "CAND=HEAD~1: the engine is not named for the commit it holds"
  show "$cand_dir"
fi

# 15. CAND resolving to REF is refused, and a dirty tree does not rescue it.
#
# Same guard as cases 4 and 6 and the same reason -- an SPRT between identical
# engines random-walks to a bound rather than returning zero -- but it has to
# ask a different question here. The working-tree guard also requires a clean
# tree, and under it this run would be played: the tree is dirty, so the
# candidate "differs" from the reference while the two binaries are one commit.
same_dir="$(make_sandbox "$script_under_test")"
echo change >> "$same_dir/tracked.txt"
same_status="$(CAND=HEAD~1 run_sandbox "$same_dir" HEAD~1)"

if [[ -e "$same_dir/fastchess_invoked" ]]; then
  fail "CAND and REF at the same commit played a match between two identical builds"
  show "$same_dir"
elif ((same_status == 0)); then
  fail "CAND and REF at the same commit exited 0 instead of refusing"
  show "$same_dir"
elif ! grep -q 'SPRT-RUN-FAILED.*both sides are the same build' "$same_dir/out.txt"; then
  fail "CAND and REF at the same commit refused, but not with the A/A guard's message"
  show "$same_dir"
fi

# The positive control for the case above, as case 7 is for case 4: the same
# sandbox and the same two refs with AA=1, which must reach fastchess. Without
# it the refusal could be some other objection to a CAND run entirely.
same_aa_dir="$(make_sandbox "$script_under_test")"
echo change >> "$same_aa_dir/tracked.txt"
same_aa_status="$(CAND=HEAD~1 run_sandbox "$same_aa_dir" HEAD~1 1)"

if [[ ! -e "$same_aa_dir/fastchess_invoked" ]]; then
  fail "AA=1 with CAND and REF at the same commit was refused (exit $same_aa_status)"
  show "$same_aa_dir"
elif ! grep -q 'A/A CALIBRATION' "$same_aa_dir/out.txt"; then
  fail "AA=1 with CAND played the match without saying it measures the harness"
  show "$same_aa_dir"
fi

# 16. A commit with no cached worktree is built, and that build is played.
#
# The branch under test is `build_ref`'s `if [[ ! -x "$binary" ]]`, which every
# case above skips. Both sides are asserted, because the function is called
# twice and a mistake on one call site is invisible from the other: the
# reference here is built from HEAD~1 and the candidate from HEAD, and each
# stub binary names the sha its build directory sat under.
build_dir="$(make_sandbox "$script_under_test")"
unbuild_sandbox "$build_dir" ok
build_status="$(CAND=HEAD run_sandbox "$build_dir" HEAD~1)"
build_head="$(git -C "$build_dir" rev-parse --short HEAD)"
build_older="$(git -C "$build_dir" rev-parse --short HEAD~1)"

if ((build_status != 0)); then
  fail "no cached build: the script exited $build_status"
  show "$build_dir"
elif [[ ! -e "$build_dir/fastchess_invoked" ]]; then
  fail "no cached build: fastchess was never invoked"
  show "$build_dir"
elif ! grep -q -x -- "fresh-build $build_head" "$build_dir/fastchess_engines"; then
  fail "no cached build: the candidate $build_head was not built and played"
  show "$build_dir"
elif ! grep -q -x -- "fresh-build $build_older" "$build_dir/fastchess_engines"; then
  fail "no cached build: the reference $build_older was not built and played"
  show "$build_dir"
elif grep -q -x -- 'working-tree' "$build_dir/fastchess_engines"; then
  fail "no cached build: fastchess was handed build/src/chesso"
  show "$build_dir"
fi

# 17. A build that fails stops the run before a game is played, on both sides.
#
# This is the case the fast check of 2026-09-12 found open, and it is not a
# hypothetical: `set -e` does not reach inside a command substitution, so a
# `cmake` that exits 1 inside `build_ref` left the function running on to
# `echo "$binary"` and the caller taking a path to a file that was never
# produced. Observed against the script as it then stood: the reference side
# reached `SPRT-RUN-DONE` with fastchess handed a non-existent binary, and the
# candidate side died at `cp` with no sentence saying what had gone wrong.
#
# Pinned to the guard's own sentence rather than the bare marker, as case 4 is
# and for the same reason: every fail() prints SPRT-RUN-FAILED, so a bare grep
# would stay green on an unrelated abort -- and on the candidate side one
# already existed.
for side in reference candidate; do
  broken_dir="$(make_sandbox "$script_under_test")"
  unbuild_sandbox "$broken_dir" fail
  if [[ "$side" == candidate ]]; then
    broken_status="$(CAND=HEAD run_sandbox "$broken_dir" HEAD~1)"
    broken_sha="$(git -C "$broken_dir" rev-parse --short HEAD)"
  else
    broken_status="$(run_sandbox "$broken_dir" HEAD~1)"
    broken_sha="$(git -C "$broken_dir" rev-parse --short HEAD~1)"
  fi

  if [[ -e "$broken_dir/fastchess_invoked" ]]; then
    fail "$side build failed and the match was played anyway"
    show "$broken_dir"
  elif ((broken_status == 0)); then
    fail "$side build failed but the script exited 0"
    show "$broken_dir"
  elif ! grep -q "SPRT-RUN-FAILED.*building the $side $broken_sha" \
    "$broken_dir/out.txt"; then
    fail "$side build failed without naming $broken_sha in a terminal marker"
    show "$broken_dir"
  fi
  rm -rf "$broken_dir"
done

# 18. Resignation is adjudicated two-sided, and the other two tokens are where
#     they were.
#
# `twosided` defaults to false in the installed fastchess, so the flag's
# absence was one-sided adjudication under a comment in rating.sh claiming the
# opposite (2026-09-10 adversarial F04). Measured before the change: 11 of 676
# decisive adjudications in self-play were one-sided (1.6 %) and 514 of 2627
# (19.6 %) in the S088 rating run, where the two sides' evaluations are on
# different scales. DEC-174 also fixes `score=400` and `movecount=3`, so both
# are asserted here: this is the line a throughput argument would move next,
# and moving it re-prices every recorded verdict.
for token in -resign movecount=3 score=400 twosided=true; do
  if ! grep -q -x -- "$token" "$reached_dir/fastchess_args"; then
    fail "the resign adjudication did not reach fastchess as '$token'"
    show "$reached_dir"
  fi
done

# 19. A cached reference whose worktree is dirty is rebuilt before it is played.
#
# `.ref-builds/<sha>` was played whenever its binary existed and nothing asked
# anything else of it. Measured over the 30 directories on this machine
# (2026-09-10 adversarial F06): one was dirty and 11 predated S104's arch flag,
# so their binaries carry no hardware popcount -- a +12.62 % nps difference by
# S104's own measurement, on the side of a match whose entire output is the
# difference between the two sides.
#
# The `cmake` stub is what makes a rebuild visible, and its absence in cases 1
# to 15 is what makes those cases evidence that a *valid* cache is not rebuilt.
dirty_ref_dir="$(make_sandbox "$script_under_test")"
stub_cmake "$dirty_ref_dir" ok
dirty_ref_sha="$(git -C "$dirty_ref_dir" rev-parse --short HEAD~1)"
echo dirt >> "$dirty_ref_dir/.ref-builds/$dirty_ref_sha/tracked.txt"
dirty_ref_status="$(run_sandbox "$dirty_ref_dir" HEAD~1)"

if ((dirty_ref_status != 0)); then
  fail "dirty cached reference: the script exited $dirty_ref_status"
  show "$dirty_ref_dir"
elif ! grep -q -x -- "fresh-build $dirty_ref_sha" "$dirty_ref_dir/fastchess_engines"; then
  fail "dirty cached reference: $dirty_ref_sha was not rebuilt before it was played"
  show "$dirty_ref_dir"
elif grep -q -x -- "ref-build $dirty_ref_sha" "$dirty_ref_dir/fastchess_engines"; then
  fail "dirty cached reference: the stale cached binary was played"
  show "$dirty_ref_dir"
fi

# 20. A cached reference whose worktree sits at another commit is rebuilt.
#
# The directory name is the only thing that said which commit was inside it,
# and nothing moves it: a hand-run `git checkout` in one of those worktrees
# leaves `.ref-builds/<sha>` holding something else entirely, and the banner,
# the PGN and the step file all still name <sha>.
moved_ref_dir="$(make_sandbox "$script_under_test")"
stub_cmake "$moved_ref_dir" ok
moved_ref_sha="$(git -C "$moved_ref_dir" rev-parse --short HEAD~1)"
git -C "$moved_ref_dir/.ref-builds/$moved_ref_sha" checkout -q --detach \
  "$(git -C "$moved_ref_dir" rev-parse HEAD)"
moved_ref_status="$(run_sandbox "$moved_ref_dir" HEAD~1)"

if ((moved_ref_status != 0)); then
  fail "moved cached reference: the script exited $moved_ref_status"
  show "$moved_ref_dir"
elif ! grep -q -x -- "fresh-build $moved_ref_sha" "$moved_ref_dir/fastchess_engines"; then
  fail "moved cached reference: $moved_ref_sha was not rebuilt before it was played"
  show "$moved_ref_dir"
fi

# 21. A cached reference configured for another instruction set is rebuilt.
#
# The candidate is whatever `build/` was last configured as and the reference
# used to be configured with bare defaults, so the two agreed by luck --
# `cmake/arch.cmake` defaults CHESSO_ARCH to `native` -- and not by check.
# CHESSO_TUNE is the same question with a sharper edge: DEC-118 calls the tune
# build "deliberately different code".
arch_ref_dir="$(make_sandbox "$script_under_test")"
stub_cmake "$arch_ref_dir" ok
arch_ref_sha="$(git -C "$arch_ref_dir" rev-parse --short HEAD~1)"
printf 'CHESSO_ARCH:STRING=bmi2\nCHESSO_TUNE:BOOL=OFF\n' \
  > "$arch_ref_dir/.ref-builds/$arch_ref_sha/build/CMakeCache.txt"
arch_ref_status="$(run_sandbox "$arch_ref_dir" HEAD~1)"

if ((arch_ref_status != 0)); then
  fail "mis-configured cached reference: the script exited $arch_ref_status"
  show "$arch_ref_dir"
elif ! grep -q -x -- "fresh-build $arch_ref_sha" "$arch_ref_dir/fastchess_engines"; then
  fail "a cached reference built for another CHESSO_ARCH was played unrebuilt"
  show "$arch_ref_dir"
fi

# 22. A cache that answers all three questions is played, and nothing is built.
#
# The negative control for 19 to 21, and the case that stops the check being
# written as "rebuild always": `cmake` is on PATH and must not run. A rebuild
# every time would be minutes per run with a cold ccache, on the one tool that
# decides whether a change ships.
valid_ref_dir="$(make_sandbox "$script_under_test")"
stub_cmake "$valid_ref_dir" ok
valid_ref_sha="$(git -C "$valid_ref_dir" rev-parse --short HEAD~1)"
valid_ref_status="$(run_sandbox "$valid_ref_dir" HEAD~1)"

if ((valid_ref_status != 0)); then
  fail "valid cached reference: the script exited $valid_ref_status"
  show "$valid_ref_dir"
elif [[ -e "$valid_ref_dir/cmake_invoked" ]]; then
  fail "a valid cached reference was rebuilt anyway"
  show "$valid_ref_dir"
elif ! grep -q -x -- "ref-build $valid_ref_sha" "$valid_ref_dir/fastchess_engines"; then
  fail "valid cached reference: the cached binary was not the one played"
  show "$valid_ref_dir"
fi

# 23. The banner prints the candidate's configuration, and the reference is
#     built with it.
#
# Both halves, because either alone is satisfied by a mutant: a banner line
# built from a constant reads right on this machine, where `native` is the
# default, and a reference configured from `build/`'s cache without the banner
# saying so leaves the reader no way to know what was compared. The second
# sandbox configures `build/` for something else so neither can be a constant.
if ! grep -qE '^config     arch native  tune off$' "$reached_dir/out.txt"; then
  fail "the banner does not print the candidate's arch and tune"
  show "$reached_dir"
fi

config_dir="$(make_sandbox "$script_under_test")"
stub_cmake "$config_dir" ok
config_sha="$(git -C "$config_dir" rev-parse --short HEAD~1)"
printf 'CHESSO_ARCH:STRING=bmi2\nCHESSO_TUNE:BOOL=ON\n' \
  > "$config_dir/build/CMakeCache.txt"
config_status="$(run_sandbox "$config_dir" HEAD~1)"

if ((config_status != 0)); then
  fail "CHESSO_ARCH=bmi2 candidate: the script exited $config_status"
  show "$config_dir"
elif ! grep -qE '^config     arch bmi2  tune on$' "$config_dir/out.txt"; then
  fail "the banner does not read the candidate's configuration from build/CMakeCache.txt"
  show "$config_dir"
elif ! grep -q '^CHESSO_ARCH:STRING=bmi2$' \
  "$config_dir/.ref-builds/$config_sha/build/CMakeCache.txt"; then
  fail "the reference was not configured with the candidate's CHESSO_ARCH"
  show "$config_dir"
elif ! grep -q '^CHESSO_TUNE:BOOL=ON$' \
  "$config_dir/.ref-builds/$config_sha/build/CMakeCache.txt"; then
  fail "the reference was not configured with the candidate's CHESSO_TUNE"
  show "$config_dir"
fi

# 24. A side whose `id name` is not the commit it was labelled with is refused.
#
# The two names on the argv -- `candidate` and `ref-<sha>` -- are the script's
# own variables, so every archived PGN's engine names were an assertion about
# what was built and not a property of what played (2026-09-10 adversarial
# F05). `rating.sh` has refused an opponent whose `id name` disagrees with the
# manifest since DEC-068; nothing did that for chesso, and DEC-020's +301 Elo
# is what the class costs. The positive control is every other case in this
# file: the stub engines answer the sha they were made for and all of them
# play.
wrongid_dir="$(make_sandbox "$script_under_test")"
stub_engine "$wrongid_dir/build/src/chesso" deadbee working-tree
wrongid_status="$(run_sandbox "$wrongid_dir" HEAD~1)"
wrongid_head="$(git -C "$wrongid_dir" rev-parse --short HEAD)"

if [[ -e "$wrongid_dir/fastchess_invoked" ]]; then
  fail "a candidate answering another commit's sha was played anyway"
  show "$wrongid_dir"
elif ((wrongid_status == 0)); then
  fail "a candidate answering another commit's sha exited 0 instead of refusing"
  show "$wrongid_dir"
elif ! grep -q "SPRT-RUN-FAILED.*deadbee" "$wrongid_dir/out.txt"; then
  fail "the refusal does not name what the binary said it was"
  show "$wrongid_dir"
elif ! grep -q "$wrongid_head" "$wrongid_dir/out.txt"; then
  fail "the refusal does not name the sha the run labelled that side with"
  show "$wrongid_dir"
fi

# An engine built before S212 answers the bare literal `id name Chesso`, and
# that has to keep playing: every commit the reference mechanism exists to
# reach is one of them, and refusing would make `REF=<any older sha>`
# unrunnable. It plays, and the run says the check did not happen.
oldid_dir="$(make_sandbox "$script_under_test")"
oldid_sha="$(git -C "$oldid_dir" rev-parse --short HEAD~1)"
printf '#!/bin/sh\nprintf "id name Chesso\\nuciok\\n"\necho ref-build %s\nexit 0\n' \
  "$oldid_sha" > "$oldid_dir/.ref-builds/$oldid_sha/build/src/chesso"
chmod +x "$oldid_dir/.ref-builds/$oldid_sha/build/src/chesso"
oldid_status="$(run_sandbox "$oldid_dir" HEAD~1)"

if ((oldid_status != 0)); then
  fail "a reference predating the build stamp was refused (exit $oldid_status)"
  show "$oldid_dir"
elif ! grep -q 'no build stamp' "$oldid_dir/out.txt"; then
  fail "a reference predating the build stamp played without saying so"
  show "$oldid_dir"
fi

# 25. A game ending by crash or disconnect voids the run.
#
# The census counted crashes and disconnects and never voided on them, where
# `rating.sh` prints RATING-RUN-INVALID and exits 1 (2026-09-10 adversarial
# F31). The stated justification -- "both sides here are chesso" -- covers a
# time forfeit, which is a real game result costing both sides about equally,
# and does not extend to a crash: only one side carries the change under test.
#
# The terminal marker is the other half and it is not decoration. Every watcher
# of a detached run breaks on SPRT-RUN-(DONE|FAILED) and on nothing else
# (DEC-061, AGENTS.md WATCHERS), so a void that printed INVALID and stopped
# would leave the watcher spinning to its ceiling -- the S167 and S177 class,
# reintroduced by the fix for a different finding.
crash_dir="$(make_sandbox "$script_under_test")"
cat > "$crash_dir/stub_pgn" << 'PGN'
[Result "1-0"]
[Termination "adjudication"]

1. e4 e5 1-0

[Result "0-1"]
[Termination "disconnect"]

1. e4 0-1
PGN
crash_status="$(run_sandbox "$crash_dir" HEAD~1)"

if [[ ! -e "$crash_dir/fastchess_invoked" ]]; then
  fail "the crash case never reached fastchess, so it proves nothing"
  show "$crash_dir"
elif ((crash_status == 0)); then
  fail "a run with a disconnect termination exited 0"
  show "$crash_dir"
elif ! grep -q 'SPRT-RUN-INVALID' "$crash_dir/out.txt"; then
  fail "a run with a disconnect termination did not print SPRT-RUN-INVALID"
  show "$crash_dir"
elif [[ "$(tail -1 "$crash_dir/out.txt")" != SPRT-RUN-FAILED* ]]; then
  fail "the voided run does not end on a terminal marker a watcher breaks on"
  show "$crash_dir"
fi

# The negative control, and it is what stops the case above passing for the
# wrong reason: the same stub writing the same shape of PGN with nothing
# unexpected in it has to reach SPRT-RUN-DONE.
census_dir="$(make_sandbox "$script_under_test")"
cat > "$census_dir/stub_pgn" << 'PGN'
[Result "1-0"]
[Termination "adjudication"]

1. e4 e5 1-0

[Result "0-1"]
[Termination "time forfeit"]

1. e4 0-1
PGN
census_status="$(run_sandbox "$census_dir" HEAD~1)"

if ((census_status != 0)); then
  fail "a run whose PGN holds only expected terminations exited $census_status"
  show "$census_dir"
elif grep -q 'SPRT-RUN-INVALID' "$census_dir/out.txt"; then
  fail "a normal PGN was voided; the crash case proves nothing"
  show "$census_dir"
elif [[ "$(tail -1 "$census_dir/out.txt")" != SPRT-RUN-DONE* ]]; then
  fail "a completed run does not end on SPRT-RUN-DONE"
  show "$census_dir"
fi

# 26. The busy guard reads the machine's load and not `ps`'s lifetime averages.
#
# Static, because the quantity is the machine's own and there is nothing to
# stub: `/proc/loadavg` is a kernel file. `ps -A -o %cpu=` sums each process's
# average over its whole lifetime, which on this machine read 255 against a
# one-minute load average of 0.79 -- 3.2x over the warning threshold, so the
# guard fired on every run and carried no information (2026-09-10 adversarial
# F32). Comments are filtered out because the replacement's comment names the
# old form on purpose, exactly as tests/test_rating_script.sh case 5 does.
if grep -v '^[[:space:]]*#' "$script_under_test" | grep -q 'ps -A -o %cpu='; then
  fail "the busy guard still sums ps lifetime percentages"
fi
if ! grep -q '/proc/loadavg' "$script_under_test"; then
  fail "the busy guard does not read the one-minute load average"
fi

rm -rf "$reached_dir" "$default_dir" "$clean_dir" "$older_dir" \
       "$explicit_dir" "$aa_dir" "$nogit_dir" "$seed_dir" \
       "$override_dir" "$pgn_dir" "$rounds_dir" "$tc_dir" "$hash_dir" \
       "$cand_dir" "$same_dir" "$same_aa_dir" "$build_dir" \
       "$dirty_ref_dir" "$moved_ref_dir" "$arch_ref_dir" "$valid_ref_dir" \
       "$config_dir" "$wrongid_dir" "$oldid_dir" "$crash_dir" "$census_dir"
[[ -n "${abort_dir:-}" ]] && rm -rf "$abort_dir"

if ((failures > 0)); then
  echo "$script_under_test: $failures assertion(s) failed" >&2
  exit 1
fi

echo "$script_under_test: 26 properties hold -- reaches fastchess, aborts non-zero, defaults REF to HEAD, dates each side from its own commit, refuses a clean-tree A/A however the ref is spelled, honours AA=1, marks an abort with no git, prints and passes the seed, records nodes and time left, runs fixed rounds without an SPRT, passes TC and HASH through to the engines with their defaults intact, plays CAND's own build undecorated, refuses CAND at REF on a dirty tree, builds an uncached commit on either side, stops on a failed build before a game is played, adjudicates resignation two-sided, rebuilds a cached reference that is dirty, moved or configured differently while playing a valid one unbuilt, prints and propagates the candidate's configuration, refuses a side whose id name is another commit, voids a run on a crash and still marks it, and reads the load average"
