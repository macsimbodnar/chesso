#!/bin/bash

# clang-format changes its output between major versions, so the version is
# pinned rather than "whatever is on PATH". An unpinned formatter rewrites files
# nobody touched and buries the real change in the diff.
REQUIRED_MAJOR=${CLANG_FORMAT_MAJOR:-22}

if [ $# -gt 0 -a "$1" != "--check" ]; then
    echo "Usage: $0 [--check]"
    echo "Set CLANG_FORMAT_MAJOR to use a different major version."
    exit 0
fi

if [ ! -f CMakeLists.txt ]; then
    echo "Please run this script from the root folder."
    exit 1
fi

# Homebrew ships the binary unsuffixed inside a keg-only prefix; Debian and
# Ubuntu ship it suffixed. Try both, and confirm the version either way.
CANDIDATES="
clang-format-$REQUIRED_MAJOR
/opt/homebrew/opt/llvm/bin/clang-format
/usr/local/opt/llvm/bin/clang-format
/usr/lib/llvm-$REQUIRED_MAJOR/bin/clang-format
clang-format
"

TOOL=""
FOUND=""
for CANDIDATE in $CANDIDATES; do
    RESOLVED="$(command -v "$CANDIDATE" 2> /dev/null)"
    [ -x "$RESOLVED" ] || continue

    case "$FOUND" in
        *" $RESOLVED "*) continue ;;  # same binary reached by two candidate names
    esac

    VERSION="$("$RESOLVED" --version | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
    FOUND="$FOUND  $RESOLVED ($VERSION)\n"

    if [ "${VERSION%%.*}" = "$REQUIRED_MAJOR" ]; then
        TOOL="$RESOLVED"
        break
    fi
done

if [ -z "$TOOL" ]; then
    echo "clang-format $REQUIRED_MAJOR not found."
    if [ -n "$FOUND" ]; then
        echo "Found, but wrong version:"
        printf "$FOUND"
    fi
    echo "  macOS:  brew install llvm"
    echo "          fish_add_path --append /opt/homebrew/opt/llvm/bin"
    echo "  Ubuntu: apt install clang-format-$REQUIRED_MAJOR"
    exit 1
fi

# S054: `git ls-files` with no flags lists the index, so a source file that has
# not been added yet is invisible to the check -- and --check is one of the three
# commands in .moltke.json's step-completion gate, so every step that added a
# file got a vacuous pass for exactly the files it added. --others adds the
# untracked ones. --exclude-standard is load-bearing: it is what keeps the
# generated sources under build*/ and the checked-out engine copies under
# .ref-builds/ out of the list. --cached can also name a file deleted from the
# worktree but still in the index, which clang-format cannot open, so the list
# is filtered to paths that exist.
FILES=""
while IFS= read -r FILE; do
    [ -f "$FILE" ] && FILES="$FILES $FILE"
done < <(git ls-files --cached --others --exclude-standard |
    grep -E '\.(c|cc|cpp|h|hpp|hh)$' | sort -u)

EXTRA_ARGS=""
if [ "$1" == "--check" ]; then
    EXTRA_ARGS="--dry-run -Werror"
fi

$TOOL $EXTRA_ARGS -i $FILES
