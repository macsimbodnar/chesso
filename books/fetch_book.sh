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
pinned="$(
  cat <<'PINS'
UHO_Lichess_4852_v1.epd	4e298f11e8acfa106babe02968f2e61582145e7874c59284690b20b9650e0e07	7a7f6470615a69c6cf23d565417701d38732876f480af90d67b42abade35644a
PINS
)"

fail() { echo "FETCH-BOOK-FAILED: $*" >&2; exit 1; }

if [[ "${1:-}" == "--list" ]]; then
  echo "pinned books, fetched into $books/ and gitignored there:"
  awk -F'\t' '{ printf "  %s\n", $1 }' <<< "$pinned"
  echo "default: $default_book"
  exit 0
fi

book="${1:-$default_book}"

line="$(awk -F'\t' -v b="$book" '$1 == b { print; found = 1 } END { exit !found }' \
  <<< "$pinned")" || fail "no pinned digest for '$book'; run --list"

IFS=$'\t' read -r _ zip_sha file_sha <<< "$line"

command -v unzip > /dev/null || fail "unzip is not on PATH"

target="$books/$book"
if [[ -r "$target" ]]; then
  got="$(sha256sum "$target" | cut -d' ' -f1)"
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

got="$(sha256sum "$tmp/book.zip" | cut -d' ' -f1)"
[[ "$got" == "$zip_sha" ]] \
  || fail "zip sha256 is $got, pinned is $zip_sha"

unzip -q -o -d "$tmp" "$tmp/book.zip" || fail "unzip failed"
[[ -r "$tmp/$book" ]] || fail "the zip does not contain $book"

got="$(sha256sum "$tmp/$book" | cut -d' ' -f1)"
[[ "$got" == "$file_sha" ]] \
  || fail "unpacked sha256 is $got, pinned is $file_sha"

mv "$tmp/$book" "$target"
echo "$target"
echo "sha256 $file_sha, verified against the pin in $(basename "$0")"
