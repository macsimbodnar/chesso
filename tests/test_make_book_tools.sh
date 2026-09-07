#!/usr/bin/env bash
#
# Fixture test for the two tools that sit on algebraic_to_move(): make_book and
# pgn_to_positions. S174, closing 2026-09-03_adversarial-F02.
#
# The parser's failure paths were `assert(false)` with no return, so in Release
# a token it could not parse became a fabricated move that make_move() applied.
# Observed at 1d8cbac: `1. e4!? e5 2. Nf3 Nc6 *` built a book whose first entry
# was the start position with move `a8a7`, reported `games cut short 0`, exited
# 0, and `dump` called it loadable; pgn_to_positions on the same line printed a
# FEN with a white bishop on a7 and exited 0. Six properties:
#
#   1. a clean game builds: exit 0, four entries
#   2. the same game with a suffix annotation builds byte-identically to it
#   3. a game with an unparseable token is refused: exit non-zero, no file at
#      --out, and the report names the token
#   4. --allow-cut-short turns the refusal into a report: exit 0, the game is
#      dropped from the bad token on, and the count says so
#   5. pgn_to_positions prints the same positions for the annotated line as for
#      the clean one, starting from the start position with `e2e4` and ending
#      with the position after the last move
#   6. pgn_to_positions on an unparseable token exits non-zero
#
# S178 added two more, over PGN import format (8.2.2.1): a move number
# indication may be glued to the move it introduces, `1.e4`, and a black
# indication `2...Nc6` may follow commentary.
#
#   7. a game written in the glued form builds byte-identically to the same
#      game written in export form
#   8. the same across a comment, a black indication and glued castling
#
# Everything happens in a throwaway directory. No engine is searched.
#
# Usage: test_make_book_tools.sh <make_book> <pgn_to_positions>

set -uo pipefail

make_book="${1:?path to make_book}"
pgn_to_positions="${2:?path to pgn_to_positions}"
failures=0

fail()
{
  echo "FAIL: $*" >&2
  failures=$((failures + 1))
}

for tool in "$make_book" "$pgn_to_positions"; do
  if [[ ! -x "$tool" ]]; then
    echo "FAIL: no executable at $tool" >&2
    exit 1
  fi
done

tmp="$(mktemp -d "${TMPDIR:-/tmp}/chesso-make-book.XXXXXX")"
trap 'rm -rf "$tmp"' EXIT

write_pgn()
{
  printf '[Event "fixture"]\n[Result "*"]\n\n%s\n' "$1" > "$2"
}

write_pgn '1. e4 e5 2. Nf3 Nc6 *' "$tmp/control.pgn"
write_pgn '1. e4!? e5 2. Nf3 Nc6 *' "$tmp/annotated.pgn"
write_pgn '1. e4 e5 2. Qxf7 Nc6 *' "$tmp/illegal.pgn"
write_pgn '1.e4 e5 2.Nf3 Nc6 *' "$tmp/glued.pgn"
write_pgn '1. e4 e5 2. Nf3 Nc6 3. Bc4 Nf6 4. O-O *' "$tmp/castle_spaced.pgn"
write_pgn '1.e4 e5 2.Nf3 {c} 2...Nc6 3.Bc4 Nf6 4.O-O *' "$tmp/castle_glued.pgn"

# 1. The control.
if ! "$make_book" build "$tmp/control.pgn" --out "$tmp/control.bin" \
    > "$tmp/control.out" 2>&1; then
  fail "control: build exited non-zero: $(cat "$tmp/control.out")"
fi
grep -q 'games cut short      0' "$tmp/control.out" \
  || fail "control: expected 0 games cut short: $(cat "$tmp/control.out")"
grep -q 'entries written      4' "$tmp/control.out" \
  || fail "control: expected 4 entries: $(cat "$tmp/control.out")"

# 2. Annotated, byte-identical to the control.
if ! "$make_book" build "$tmp/annotated.pgn" --out "$tmp/annotated.bin" \
    > "$tmp/annotated.out" 2>&1; then
  fail "annotated: build exited non-zero: $(cat "$tmp/annotated.out")"
fi
grep -q 'games cut short      0' "$tmp/annotated.out" \
  || fail "annotated: the annotation cut the game short: $(cat "$tmp/annotated.out")"
if [[ -f "$tmp/control.bin" && -f "$tmp/annotated.bin" ]]; then
  cmp -s "$tmp/control.bin" "$tmp/annotated.bin" \
    || fail "annotated: book differs from the control's"
fi

# 3. An unparseable token refuses the build.
"$make_book" build "$tmp/illegal.pgn" --out "$tmp/illegal.bin" \
  > "$tmp/illegal.out" 2>&1
status=$?
[[ $status -ne 0 ]] || fail "illegal: build exited 0"
[[ ! -e "$tmp/illegal.bin" ]] || fail "illegal: a book was written at --out"
grep -q 'Qxf7' "$tmp/illegal.out" \
  || fail "illegal: the report does not name the token: $(cat "$tmp/illegal.out")"

# 4. --allow-cut-short: the game is dropped from the bad token on.
if ! "$make_book" build "$tmp/illegal.pgn" --out "$tmp/allowed.bin" \
    --allow-cut-short > "$tmp/allowed.out" 2>&1; then
  fail "allow-cut-short: build exited non-zero: $(cat "$tmp/allowed.out")"
fi
grep -q 'games cut short      1' "$tmp/allowed.out" \
  || fail "allow-cut-short: expected 1 game cut short: $(cat "$tmp/allowed.out")"
grep -q 'entries written      2' "$tmp/allowed.out" \
  || fail "allow-cut-short: expected the 2 entries before the bad token: $(cat "$tmp/allowed.out")"
[[ -f "$tmp/allowed.bin" ]] || fail "allow-cut-short: no book written"

# 7. Import-format move numbers: glued builds byte-identically to the control.
if ! "$make_book" build "$tmp/glued.pgn" --out "$tmp/glued.bin" \
    > "$tmp/glued.out" 2>&1; then
  fail "glued: build exited non-zero: $(cat "$tmp/glued.out")"
fi
grep -q 'games cut short      0' "$tmp/glued.out" \
  || fail "glued: the glued move number cut the game short: $(cat "$tmp/glued.out")"
grep -q 'entries written      4' "$tmp/glued.out" \
  || fail "glued: expected 4 entries: $(cat "$tmp/glued.out")"
# Unlike property 2, a missing book is a failure here and not a skipped check.
{ [[ -f "$tmp/control.bin" ]] && [[ -f "$tmp/glued.bin" ]] \
  && cmp -s "$tmp/control.bin" "$tmp/glued.bin"; } \
  || fail "glued: book differs from the control's"

# 8. A black indication after a comment, and castling glued to its number.
if ! "$make_book" build "$tmp/castle_spaced.pgn" --out "$tmp/castle_spaced.bin" \
    > "$tmp/castle_spaced.out" 2>&1; then
  fail "castle_spaced: build exited non-zero: $(cat "$tmp/castle_spaced.out")"
fi
grep -q 'entries written      7' "$tmp/castle_spaced.out" \
  || fail "castle_spaced: expected 7 entries: $(cat "$tmp/castle_spaced.out")"
if ! "$make_book" build "$tmp/castle_glued.pgn" --out "$tmp/castle_glued.bin" \
    > "$tmp/castle_glued.out" 2>&1; then
  fail "castle_glued: build exited non-zero: $(cat "$tmp/castle_glued.out")"
fi
grep -q 'games cut short      0' "$tmp/castle_glued.out" \
  || fail "castle_glued: the game was cut short: $(cat "$tmp/castle_glued.out")"
grep -q 'entries written      7' "$tmp/castle_glued.out" \
  || fail "castle_glued: expected 7 entries: $(cat "$tmp/castle_glued.out")"
{ [[ -f "$tmp/castle_spaced.bin" ]] && [[ -f "$tmp/castle_glued.bin" ]] \
  && cmp -s "$tmp/castle_spaced.bin" "$tmp/castle_glued.bin"; } \
  || fail "castle_glued: book differs from the spaced twin's"

# 5. pgn_to_positions, annotated against clean.
printf 'e4 e5 Nf3 Nc6\n' | "$pgn_to_positions" > "$tmp/p_control.tsv" \
  || fail "positions control: exited non-zero"
printf 'e4!? e5 Nf3 Nc6\n' | "$pgn_to_positions" > "$tmp/p_annotated.tsv" \
  || fail "positions annotated: exited non-zero"
# The second column echoes the token as given, `e4!?` against `e4`, so the
# comparison is over everything but it: ply, long algebraic, FEN and phase.
cmp -s <(cut -f1,3- "$tmp/p_control.tsv") <(cut -f1,3- "$tmp/p_annotated.tsv") \
  || fail "positions annotated: positions differ from the clean line's"
expected_first="0	e4	e2e4	rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
first="$(head -n 1 "$tmp/p_control.tsv" | cut -f1-4)"
[[ "$first" == "$expected_first" ]] \
  || fail "positions control: first line is '$first'"
# Four moves and the position after the last one: five lines.
[[ "$(wc -l < "$tmp/p_control.tsv" | tr -d ' ')" == "5" ]] \
  || fail "positions control: expected 5 lines"

# 6. pgn_to_positions on an unparseable token.
printf 'e4 e5 Qxf7\n' | "$pgn_to_positions" > "$tmp/p_illegal.tsv" 2> "$tmp/p_illegal.err"
status=$?
[[ $status -ne 0 ]] || fail "positions illegal: exited 0"

if [[ $failures -ne 0 ]]; then
  echo "$failures failure(s)" >&2
  exit 1
fi

echo "ok"
