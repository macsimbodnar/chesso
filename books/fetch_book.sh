#!/usr/bin/env bash
set -euo pipefail

# Fetches an opening book that is too large to commit, and verifies it.
#
#   ./books/fetch_book.sh                          the default book
#   ./books/fetch_book.sh UHO_Lichess_4852_v1.epd  a named one
#   ./books/fetch_book.sh --list                   what is pinned here
#
# WHY THIS EXISTS. fastchess.sh plays an unbalanced book (S105, DEC-083):
# balanced openings draw about 91 % between engines of equal strength, and a
# drawn pair carries no signal, so a balanced book spends a night of the
# machine to say less than it could have. The unbalanced books that do the job
# are large -- UHO_Lichess_4852_v1.epd is 175 MB against the 8.0 MB of
# books/8moves_v3.pgn -- and a 175 MB blob in the history, for a file that is
# reproducible from a URL and a digest, is the trade .tuning/ already refused.
#
# WHERE IT COMES FROM AND UNDER WHICH LICENCE. official-stockfish/books is
# CC0-1.0, which is why it and not sp-cc.de is the source. Stefan Pohl's own
# UHO pages carry "(C) 2024 Stefan Pohl (SPCC)" and no usage licence, and this
# repository bundles nothing whose licence is unstated (CLAUDE.md's first
# foundation, DEC-016). The Pohl-derived books redistributed *by the CC0
# repository* are fine to fetch from there; his site is not.
#
# WHY BOTH DIGESTS ARE PINNED. The zip is what the network returned and the
# unpacked file is what the match reads. Checking only the zip trusts unzip;
# checking only the unpacked file cannot say whether a mismatch was the
# download or the unpack. A book is a measurement input: a silently different
# one moves every verdict taken with it and leaves no trace in the engine.

repo="$(cd "$(dirname "$0")/.." && pwd)"
books="$repo/books"
base_url="https://github.com/official-stockfish/books/raw/master"

default_book="UHO_Lichess_4852_v1.epd"

# name <tab> zip sha256 <tab> unpacked sha256
#
# Only books this repository has actually run are listed. An entry is added by
# fetching the file, recording both digests, and saying in the step which
# measurement adopted it -- not in advance, because an unverified pin is a
# number nobody has checked.
#
# UHO_Lichess_4852_v1.epd is what fastchess.sh plays and every SPRT verdict is
# taken on.  UHO_4060_v3.epd was added by S085: an SPSA run must not tune on
# the book its verification match plays (eval_tuning_strategy.md par.7), and
# 242201 openings is more than the 30000 rounds that run walks through -- 1250
# iterations of 24 pairs, one round per pair -- so it never wraps.
#
# 8moves_v3.pgn is the exception: it is 8.0 MB, committed rather than fetched
# (.gitignore says why), and rating.sh plays it. It is pinned here anyway so
# that one file answers where every book in this repository came from, and so
# the committed copy can be checked against upstream without a download being
# the only way to find out. Its origin was unrecorded until 2026-09-02, when
# the owner named the source and the tracked file was compared against it: the
# sha256 below is the tracked copy, byte for byte. Running the script on it
# verifies rather than fetches, since the file is already in place.
pinned="$(
  cat <<'PINS'
UHO_Lichess_4852_v1.epd	4e298f11e8acfa106babe02968f2e61582145e7874c59284690b20b9650e0e07	7a7f6470615a69c6cf23d565417701d38732876f480af90d67b42abade35644a
UHO_4060_v3.epd	62fe32cda02f605acd5938887d574730c91208812f2bb1e839f28eee10869af8	419844f8c43a9c1fa3e279518bb79e89a5ed3d181f27c180ea9eb7444a1b9885
8moves_v3.pgn	7e1e9dd118b4bb97d8a8b5b8a790c86e21f8509d59a27d2883767d94477be02e	5835239f88cc2c7511b177c32392a69f3ede21819cf0616f80a7f907cd21d17e
PINS
)"

fail() { echo "FETCH-BOOK-FAILED: $*" >&2; exit 1; }

if [[ "${1:-}" == "--list" ]]; then
  echo "pinned books, in $books/ -- fetched and gitignored, except the"
  echo "committed 8moves_v3.pgn, which is only verified against its pin:"
  awk -F'\t' '{ printf "  %s\n", $1 }' <<< "$pinned"
  echo "default: $default_book"
  exit 0
fi

book="${1:-$default_book}"

line="$(awk -F'\t' -v b="$book" '$1 == b { print; found = 1 } END { exit !found }' \
  <<< "$pinned")" || fail "no pinned digest for '$book'; run --list"

IFS=$'\t' read -r _ zip_sha file_sha <<< "$line"

command -v unzip > /dev/null || fail "unzip is not on PATH"

# GNU coreutils ships sha256sum; the BSD userland macOS ships does not, and has
# `shasum -a 256` instead. Both print "<digest>  <name>", so the callers below
# are unchanged. Found on the MacBook: without this the script died
# `sha256sum: command not found` after the 43 MB download, having verified
# nothing, which is the S167 class of portability failure and not a new one.
if command -v sha256sum > /dev/null; then
  sha256() { sha256sum "$1"; }
elif command -v shasum > /dev/null; then
  sha256() { shasum -a 256 "$1"; }
else
  fail "neither sha256sum nor shasum is on PATH; a book cannot be verified"
fi

target="$books/$book"
if [[ -r "$target" ]]; then
  got="$(sha256 "$target" | cut -d' ' -f1)"
  if [[ "$got" == "$file_sha" ]]; then
    echo "$book is already here and matches its pin."
    exit 0
  fi
  fail "$target exists and its sha256 is $got, not the pinned $file_sha"
fi

# Download to a scratch name and move into place only after both digests
# check, so an interrupted fetch never leaves a short file that looks like a
# book.
tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-book.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

echo "fetching $book.zip from official-stockfish/books (CC0-1.0) ..."
curl -fsSL -o "$tmp/book.zip" "$base_url/$book.zip" \
  || fail "download of $base_url/$book.zip failed"

got="$(sha256 "$tmp/book.zip" | cut -d' ' -f1)"
[[ "$got" == "$zip_sha" ]] \
  || fail "zip sha256 is $got, pinned is $zip_sha"

unzip -q -o -d "$tmp" "$tmp/book.zip" || fail "unzip failed"
[[ -r "$tmp/$book" ]] || fail "the zip does not contain $book"

got="$(sha256 "$tmp/$book" | cut -d' ' -f1)"
[[ "$got" == "$file_sha" ]] \
  || fail "unpacked sha256 is $got, pinned is $file_sha"

mv "$tmp/$book" "$target"
echo "$target"
echo "sha256 $file_sha, verified against the pin in $(basename "$0")"
