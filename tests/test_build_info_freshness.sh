#!/usr/bin/env bash
#
# The build stamp follows the tree, with no reconfigure. S228, closing
# 2026-09-12_plan_adversarial-F02; DEC-204 (a) is the form it checks.
#
# What it protects is a property of a *generation step*, not of a build
# directory. `id name` carries the commit the binary was compiled from, and
# fastchess.sh refuses a run whose engine answers a sha other than the one it
# labelled that side with -- so a stamp captured once and reused refuses a
# perfectly good run, and worse, a table or a game record names a commit its
# binary was not built from. The trap is configure time: `cmake -S . -B build`
# runs once and never again.
#
# The test drives cmake/build_info.cmake the way the chesso_build_info target
# does -- `cmake -D... -P <script>` -- over a throwaway git repository it
# creates in a temp dir. There is no configure step at all in `-P` mode, which
# is exactly the property: the header must follow the tree from the script's
# own re-run. Cases 1 to 5 are that property. Case 6 is the wiring that runs
# the script on every build, which no run of the script can show.
#
#   1. a fresh tree: the header carries `git rev-parse --short HEAD` with no
#      -dirty, and the arch and tune values it was passed -- TUNE=ON gives
#      " tune" with the leading space the C++ side concatenates, TUNE=OFF an
#      empty string
#   2. dirty, then committed: modifying a tracked file and re-running gives
#      <sha>-dirty; committing it and re-running gives the *new* short sha and
#      no suffix. Both read from rev-parse at that moment, never from a value
#      typed in here
#   3. an untracked file is not dirty -- the `git diff --quiet HEAD`
#      convention the script states and fastchess.sh shares, because adocs/
#      and .tuning/ carry untracked evidence constantly and none of it
#      compiles into anything
#   4. an unchanged tree does not rewrite the header: the mtime set to 2001
#      survives a re-run, so the translation units that include it do not
#      recompile on every build. Then the commit moves and the mtime does too
#   5. outside a git checkout the commit reads "unknown" and carries no -dirty
#   6. the wiring, read statically out of the two CMake files rather than run:
#      the top-level CMakeLists.txt calls the script from an
#      `add_custom_target(chesso_build_info ALL ...)` and passes OUTPUT,
#      SOURCE_DIR, ARCH and TUNE; no execute_process() or configure_file()
#      there names the script or the generated header, which is the
#      configure-time form the script's own header says it avoids; and
#      src/CMakeLists.txt keeps `add_dependencies(chesso_engine
#      chesso_build_info)`. Drop ALL, move the call to configure time or drop
#      the dependency and cases 1 to 5 stay green while the stale header F02
#      is about comes straight back
#
# Observed red against .tuning/coord/S228_stale_build_info.cmake, a copy that
# captures the whole stamp on its first run the way a configure-time capture
# would: five failures across cases 2, 3 and 4, with 1 and 5 green -- the shape
# of the defect, a stamp that is right the first time and wrong from then on.
# Case 3 goes red there as a consequence rather than on its own property: it
# compares against the sha rev-parse answers at that moment, which the frozen
# copy stopped matching in case 2. Case 6 was observed red the same way, against
# copies of the two CMake files under .tuning/coord/ with the word ALL deleted
# from the target and the add_dependencies line deleted from the library.
#
# Seconds, no build products read, nothing compiled: it belongs in the fast
# label even though what it covers is the build system.
#
# Usage: test_build_info_freshness.sh <build_info.cmake> <cmake>
#        <top-level CMakeLists.txt> <src/CMakeLists.txt>

set -uo pipefail

script="${1:?path to cmake/build_info.cmake}"
cmake_bin="${2:?path to the cmake executable}"
top_cmake="${3:?path to the top-level CMakeLists.txt}"
src_cmake="${4:?path to src/CMakeLists.txt}"
failures=0

fail()
{
  echo "FAIL: $*" >&2
  failures=$((failures + 1))
}

for readable in "$script" "$top_cmake" "$src_cmake"; do
  if [[ ! -r "$readable" ]]; then
    echo "FAIL: no readable file at $readable" >&2
    exit 1
  fi
done
if [[ ! -x "$cmake_bin" ]] && ! command -v "$cmake_bin" > /dev/null; then
  echo "FAIL: no cmake at $cmake_bin" >&2
  exit 1
fi

tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-build-info.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

# The machine's git stays out: no ~/.gitconfig, no XDG config, no /etc
# /gitconfig, no hook templates, and no repository above the sandbox -- case 5
# asks git for a sha in a directory that must not be in a checkout, and this
# file's own repository is one, a few levels up from wherever TMPDIR points on
# a machine that sets it oddly. The ceiling is a belt; case 5 checks the
# precondition itself rather than trusting it.
export HOME="$tmp"
export XDG_CONFIG_HOME="$tmp/xdg"
export GIT_CONFIG_NOSYSTEM=1
export GIT_CEILING_DIRECTORIES="$tmp"

# And the variables that point git somewhere else regardless of -C: a hook,
# `git bisect run` and `git rebase --exec` all run their command with some of
# these exported, and an inherited GIT_DIR aims every sandbox command at this
# repository.
unset GIT_DIR GIT_WORK_TREE GIT_INDEX_FILE GIT_CONFIG_GLOBAL GIT_CONFIG_COUNT

repo="$tmp/repo"
header="$tmp/generated/chesso_build_info.hpp"
mkdir -p "$repo" "$tmp/generated" "$tmp/xdg" "$tmp/template"

if ! git -c init.defaultBranch=main init -q --template="$tmp/template" \
         "$repo" > "$tmp/git.log" 2>&1; then
  echo "FAIL: could not create the sandbox repository" >&2
  sed 's/^/    /' "$tmp/git.log" >&2
  exit 1
fi

printf 'one\n' > "$repo/tracked.txt"

# Identity on the command line, never in a config file: the sandbox has no
# global config to write into and the machine's must not be read.
sandbox_commit()
{
  git -C "$repo" add "$1" > "$tmp/git.log" 2>&1 \
    && git -C "$repo" -c user.name='Chesso Test' \
           -c user.email='test@chesso.invalid' \
           commit -q -m "$2" >> "$tmp/git.log" 2>&1
}

if ! sandbox_commit tracked.txt "the tracked file"; then
  echo "FAIL: could not commit in the sandbox repository" >&2
  sed 's/^/    /' "$tmp/git.log" >&2
  exit 1
fi

# The chesso_build_info target's command line, argument for argument: every -D
# before the -P, which is what script mode requires.
generate()
{
  local out="$1" source_dir="$2" arch="$3" tune="$4"
  if ! "$cmake_bin" -DOUTPUT="$out" -DSOURCE_DIR="$source_dir" \
                    -DARCH="$arch" -DTUNE="$tune" \
                    -P "$script" > "$tmp/cmake.log" 2>&1; then
    fail "the generation step exited non-zero (OUTPUT=$out ARCH=$arch" \
         "TUNE=$tune)"
    return 1
  fi
  if [[ ! -f "$out" ]]; then
    fail "the generation step wrote no header at $out"
    return 1
  fi
  return 0
}

# The value of one macro in a generated header, with the quotes stripped.
defined()
{
  sed -n "s/^#define $2 \"\\(.*\\)\"\$/\\1/p" "$1"
}

short_sha()
{
  git -C "$repo" rev-parse --short HEAD
}

mtime_of()
{
  stat -c %Y "$1" 2> /dev/null || stat -f %m "$1"
}

show()
{
  echo "--- ${1:-$header} ---" >&2
  sed 's/^/    /' "${1:-$header}" >&2
  echo "--- cmake.log ---" >&2
  sed 's/^/    /' "$tmp/cmake.log" >&2
  echo "--- end ---" >&2
}

# 1. A fresh tree. The sha is read from the sandbox at this moment and
#    compared; the arch and the tune flag are configure-time facts that reach
#    the script on its command line and have to come out the other side
#    unchanged, the tune one with its leading space.
#
#    The tree is clean here by construction -- one commit, nothing touched
#    since -- and the case establishes that rather than assuming it, the way
#    case 5 establishes its own. Without it a bare sha would also be what a
#    script that never looks at the working tree prints, and the absence of the
#    -dirty suffix would prove nothing. `rev-parse --short` never answers a
#    suffix, so comparing against it covers both halves once the tree is known
#    clean.
if ! git -C "$repo" diff --quiet HEAD; then
  fail "1: the sandbox tree is not clean before the first generation, so a" \
       "stamp with no -dirty suffix would prove nothing"
fi

if generate "$header" "$repo" avx2 OFF; then
  sha="$(short_sha)"
  got="$(defined "$header" CHESSO_BUILD_COMMIT)"
  if [[ "$got" != "$sha" ]]; then
    fail "1: a clean tree stamped '$got', not the short sha '$sha'"; show
  fi
  if [[ "$(defined "$header" CHESSO_BUILD_ARCH)" != "avx2" ]]; then
    fail "1: ARCH=avx2 came out as" \
         "'$(defined "$header" CHESSO_BUILD_ARCH)'"; show
  fi
  if [[ -n "$(defined "$header" CHESSO_BUILD_TUNE)" ]]; then
    fail "1: TUNE=OFF came out as" \
         "'$(defined "$header" CHESSO_BUILD_TUNE)', not empty"; show
  fi
fi

if generate "$header" "$repo" native ON; then
  if [[ "$(defined "$header" CHESSO_BUILD_TUNE)" != " tune" ]]; then
    fail "1: TUNE=ON came out as" \
         "'$(defined "$header" CHESSO_BUILD_TUNE)', not ' tune' with the" \
         "leading space"; show
  fi
  if [[ "$(defined "$header" CHESSO_BUILD_ARCH)" != "native" ]]; then
    fail "1: ARCH=native came out as" \
         "'$(defined "$header" CHESSO_BUILD_ARCH)'"; show
  fi
fi

# 2. The tree moves under a header that already exists, and no configure runs
#    in between -- there is none to run. First the working tree is dirtied,
#    then the change is committed, and the stamp has to follow both times.
printf 'two\n' >> "$repo/tracked.txt"
if generate "$header" "$repo" avx2 OFF; then
  dirty_sha="$(short_sha)"
  got="$(defined "$header" CHESSO_BUILD_COMMIT)"
  if [[ "$got" != "$dirty_sha-dirty" ]]; then
    fail "2: a modified tracked file stamped '$got', not" \
         "'$dirty_sha-dirty' -- the stamp did not follow the tree"; show
  fi
fi

if ! sandbox_commit tracked.txt "the tracked file, modified"; then
  fail "2: could not commit the modification"
  sed 's/^/    /' "$tmp/git.log" >&2
fi

if generate "$header" "$repo" avx2 OFF; then
  committed_sha="$(short_sha)"
  got="$(defined "$header" CHESSO_BUILD_COMMIT)"
  if [[ "$committed_sha" == "$dirty_sha" ]]; then
    fail "2: the sandbox commit did not move HEAD, so the case proves nothing"
  fi
  if [[ "$got" != "$committed_sha" ]]; then
    fail "2: after the commit the stamp is '$got', not the new short sha" \
         "'$committed_sha' -- a rebuild after a commit does not pick it up"
    show
  fi
fi

# 3. An untracked file is not a dirty tree. `git diff --quiet HEAD` is the
#    convention the script states and fastchess.sh shares.
printf 'evidence\n' > "$repo/untracked.txt"
if generate "$header" "$repo" avx2 OFF; then
  got="$(defined "$header" CHESSO_BUILD_COMMIT)"
  if [[ "$got" != "$(short_sha)" ]]; then
    fail "3: an untracked file stamped '$got', not the plain short sha" \
         "'$(short_sha)' -- untracked is not dirty"; show
  fi
fi
rm -f "$repo/untracked.txt"

# 4. An unchanged value does not rewrite the header. Every translation unit
#    that includes it recompiles when it does, so a script that wrote on every
#    build would rebuild the engine from scratch each time. The mtime is set
#    far into the past first: without that, "unchanged" and "rewritten a
#    second ago" are the same second on a coarse filesystem clock.
if generate "$header" "$repo" avx2 OFF; then
  touch -t 200101010000 "$header"
  before="$(mtime_of "$header")"
  now="$(date +%s)"
  if [[ -z "$before" ]]; then
    # An empty reading is arithmetic zero, which reads as 1970 and would pass
    # the past-mtime guard below while proving nothing about a rewrite.
    fail "4: neither stat form could read the header's mtime, so an unchanged" \
         "mtime would prove nothing"
  elif ((before > now - 86400)); then
    fail "4: the header's mtime could not be set into the past (mtime" \
         "$before, now $now), so an unchanged mtime would prove nothing"
  elif generate "$header" "$repo" avx2 OFF; then
    after="$(mtime_of "$header")"
    if [[ "$after" != "$before" ]]; then
      fail "4: an unchanged tree rewrote the header (mtime $before ->" \
           "$after) -- every dependent translation unit recompiles"; show
    fi
  fi

  # And the other half: when the value does move, the file does.
  printf 'three\n' >> "$repo/tracked.txt"
  if ! sandbox_commit tracked.txt "the tracked file, again"; then
    fail "4: could not commit the second modification"
    sed 's/^/    /' "$tmp/git.log" >&2
  elif generate "$header" "$repo" avx2 OFF; then
    after="$(mtime_of "$header")"
    moved_sha="$(short_sha)"
    if [[ "$after" == "$before" ]]; then
      fail "4: the commit moved but the header was not rewritten (mtime" \
           "still $before)"; show
    fi
    if [[ "$(defined "$header" CHESSO_BUILD_COMMIT)" != "$moved_sha" ]]; then
      fail "4: after the commit the stamp is" \
           "'$(defined "$header" CHESSO_BUILD_COMMIT)', not '$moved_sha'"
      show
    fi
  fi
fi

# 5. Outside a git checkout. DEC-204 (a): the sha reads "unknown" rather than
#    a guess, and nothing is dirty. The precondition is established, not
#    assumed -- if git finds a repository above this directory the case would
#    pass or fail for a reason that has nothing to do with the script.
plain="$tmp/plain"
plain_header="$tmp/plain_build_info.hpp"
mkdir -p "$plain"
printf 'not a checkout\n' > "$plain/file.txt"

if git -C "$plain" rev-parse --short HEAD > /dev/null 2>&1; then
  fail "5: $plain is inside a git checkout, so the case cannot mean anything"
elif generate "$plain_header" "$plain" portable OFF; then
  got="$(defined "$plain_header" CHESSO_BUILD_COMMIT)"
  if [[ "$got" != "unknown" ]]; then
    fail "5: outside a checkout the stamp is '$got', not 'unknown'"
    show "$plain_header"
  fi
  if [[ "$(defined "$plain_header" CHESSO_BUILD_ARCH)" != "portable" ]]; then
    fail "5: outside a checkout ARCH=portable came out as" \
         "'$(defined "$plain_header" CHESSO_BUILD_ARCH)'"
    show "$plain_header"
  fi
fi

# 6. The wiring, read out of the two CMake files rather than run. Cases 1 to 5
#    prove a property of the generation step; not one of them proves that
#    anything invokes it. Delete the word ALL from the custom target, move the
#    call to a configure-time execute_process(), or drop the library's
#    dependency on the target, and every case above stays green while the
#    binary carries whatever stamp the last configure left -- which is the
#    whole of F02. Three anchors, tolerant of whitespace and not of the names.
cat > "$tmp/wiring.py" << 'PY'
"""The three wiring anchors of the build stamp, read as text.

Comments go first -- CMake's are `#` to end of line outside a quoted string --
so a paragraph describing the wiring cannot stand in for the wiring. The quote
state is per line, which is enough for these two files: neither carries a
string across a line break. Then each command's body is read to the parenthesis
that closes it, which is enough because neither file quotes a parenthesis.
"""

import re
import sys


def strip_comments(path):
    lines = []
    for line in open(path, encoding="utf-8", errors="replace"):
        quoted = False
        cut = len(line)
        i = 0
        while i < len(line):
            char = line[i]
            if char == "\\":
                i += 2
                continue
            if char == '"':
                quoted = not quoted
            elif char == "#" and not quoted:
                cut = i
                break
            i += 1
        lines.append(line[:cut].rstrip("\n"))
    return "\n".join(lines)


def body_of(text, start):
    """Everything between the parenthesis opening the command at `start` and
    the one that closes it, or None if it is never closed."""
    open_at = text.find("(", start)
    if open_at < 0:
        return None
    depth = 0
    for i in range(open_at, len(text)):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return text[open_at + 1:i]
    return None


def check_target(top, top_path, problems):
    """(i) the target exists, carries ALL, and runs the script with all four
    values on its command line."""
    target = re.search(
        r"add_custom_target\s*\(\s*chesso_build_info\s+ALL\b", top)
    if target is None:
        if re.search(r"add_custom_target\s*\(\s*chesso_build_info\b", top):
            problems.append(
                "the chesso_build_info target in %s has lost ALL, so an "
                "ordinary build never runs it and the header stays whatever "
                "the last run left" % top_path)
        else:
            problems.append(
                "no add_custom_target(chesso_build_info ALL ...) in %s, so "
                "nothing regenerates the stamp on a build" % top_path)
        return
    body = body_of(top, target.start())
    if body is None:
        problems.append(
            "the chesso_build_info target in %s is never closed, so its "
            "command line cannot be read" % top_path)
        return
    if re.search(
            r"-P\s+\$\{CMAKE_SOURCE_DIR\}/cmake/build_info\.cmake", body
    ) is None:
        problems.append(
            "the chesso_build_info target in %s does not run "
            "`-P ${CMAKE_SOURCE_DIR}/cmake/build_info.cmake`" % top_path)
    for flag in ("-DOUTPUT=", "-DSOURCE_DIR=", "-DARCH=", "-DTUNE="):
        if flag not in body:
            problems.append(
                "the chesso_build_info target in %s passes no %s, so the "
                "script runs without one of the values it stamps"
                % (top_path, flag))


def check_no_configure_time_form(top, top_path, problems):
    """(ii) nothing captures the stamp at configure time."""
    for command in ("execute_process", "configure_file"):
        for call in re.finditer(r"\b%s\s*\(" % command, top):
            body = body_of(top, call.start())
            if body is None:
                continue
            if "build_info.cmake" in body or "chesso_build_info.hpp" in body:
                problems.append(
                    "a call to %s() in %s names the build stamp: that is the "
                    "configure-time capture cmake/build_info.cmake's own "
                    "header says it avoids" % (command, top_path))


def check_dependency(src, src_path, problems):
    """(iii) the engine library still waits for the target."""
    if re.search(
            r"add_dependencies\s*\(\s*chesso_engine\s[^)]*\b"
            r"chesso_build_info\b", src) is None:
        problems.append(
            "no add_dependencies(chesso_engine chesso_build_info) in %s, so "
            "the library can be compiled against a header nothing "
            "regenerated" % src_path)


def main():
    top_path, src_path = sys.argv[1], sys.argv[2]
    problems = []
    top = strip_comments(top_path)
    check_target(top, top_path, problems)
    check_no_configure_time_form(top, top_path, problems)
    check_dependency(strip_comments(src_path), src_path, problems)
    for problem in problems:
        print(problem)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
PY

if ! command -v python3 > /dev/null 2>&1; then
  fail "6: no python3 on PATH, so the build wiring was not checked"
elif ! python3 "$tmp/wiring.py" "$top_cmake" "$src_cmake" \
     > "$tmp/wiring.txt" 2>&1; then
  if [[ ! -s "$tmp/wiring.txt" ]]; then
    fail "6: the wiring check exited non-zero and said nothing"
  fi
  while IFS= read -r problem; do
    fail "6: $problem"
  done < "$tmp/wiring.txt"
fi

if [[ $failures -ne 0 ]]; then
  echo "$failures failure(s)" >&2
  exit 1
fi

echo "ok"
