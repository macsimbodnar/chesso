#!/usr/bin/env bash
#
# S331: datagen plays a game on past the second occurrence of a position, and
# its closing `repetition:` line and its `--games-out` file say what the rule
# before S331, a draw at the first recurrence, would have changed. A small run
# of six games at 1000 nodes a move, under a second; tests/test_datagen.cpp
# holds the rule itself on a forced repetition.
#
#   1. precondition: at least one game reached a recurrence, read from the
#      run's own closing line, or nothing below is tested.
#   2. the games file holds one line for each game the run played.
#   3. the closing line's counts are the games file's: the games that recurred
#      after the opening, those of them White won and lost, and the openings
#      that ended on a recurrence.
#   4. its rows are the root file's: the rows of those games, and the ones
#      before each game's first recurrence, by the rows' own ply.
#   5. every row of every leaf file carries its game's result from the games
#      file.
#   6. a game went on past its first recurrence. Not asserted: that a game
#      ending on the ply of its first recurrence ended drawn. A two-fold does
#      not end the game, so that ply is still searched and adjudicated, and a
#      resignation there scores 1 or 0 with nothing wrong; tests/test_datagen.cpp
#      holds the rule itself.
#
#   test_datagen_repetition.sh <datagen binary>
set -uo pipefail

fail()
{
  echo "FAIL: $*" >&2
  exit 1
}

datagen="${1:-}"
[[ -n "$datagen" && -x "$datagen" ]] || fail "no datagen binary given, or '$datagen' is not executable"

work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

games=6
"$datagen" --games "$games" --nodes 1000 --threads 1 --seed 5 --leaf root,qs \
  --out "$work/pool.tsv" --games-out "$work/games.tsv" 2> "$work/run.log"
status=$?
((status == 0)) || fail "the run exited $status; log: $(cat "$work/run.log")"

line="$(grep -E '^repetition: ' "$work/run.log")"
[[ -n "$line" ]] || fail "no repetition: line; log: $(cat "$work/run.log")"
read -r recurred decisive won lost rows before openings < <(
  sed -E 's/^repetition: ([0-9]+) games .*; ([0-9]+) of them ended other than drawn, ([0-9]+) won by White and ([0-9]+) lost; ([0-9]+) positions passed in them, a root row each, ([0-9]+) before the first recurrence; ([0-9]+) openings .*$/\1 \2 \3 \4 \5 \6 \7/' <<< "$line")
[[ "$openings" =~ ^[0-9]+$ ]] || fail "the repetition: line does not parse: $line"

# 1.
((recurred > 0)) || fail "precondition: no game reached a recurrence, so nothing below is tested; $line"
echo "ok: precondition, $recurred games reached a recurrence"

# 2.
[[ "$(wc -l < "$work/games.tsv")" -eq "$games" ]] || fail "the games file holds $(wc -l < "$work/games.tsv") lines for $games games"
[[ "$(cut -f1 "$work/games.tsv" | sort -n | uniq | tr '\n' ' ')" == "$(seq 0 $((games - 1)) | tr '\n' ' ')" ]] ||
  fail "the games file does not number games 0 to $((games - 1)) once each: $(cut -f1 "$work/games.tsv" | tr '\n' ' ')"
echo "ok: one line a game"

# 3.
counted="$(awk -F'\t' '
  $4 >= 0 && $4 == $3 { o++ }
  $4 >= 0 && $4 != $3 { r++; if ($2 == "1.0") w++; if ($2 == "0.0") l++ }
  END { printf "%d %d %d %d %d", r, w + l, w, l, o }' "$work/games.tsv")"
[[ "$counted" == "$recurred $decisive $won $lost $openings" ]] ||
  fail "the games file gives recurred, decisive, won, lost, openings $counted; the line $recurred $decisive $won $lost $openings"
echo "ok: the line's games are the games file's, $counted"

# 4.
read -r file_rows file_before < <(awk -F'\t' '
  FNR == NR { if ($4 >= 0 && $4 != $3) at[$1] = $4; next }
  $5 in at { n++; if ($6 < at[$5]) b++ }
  END { printf "%d %d\n", n, b }' "$work/games.tsv" "$work/pool.tsv.root")
[[ "$file_rows $file_before" == "$rows $before" ]] ||
  fail "the root file gives $file_rows rows in those games, $file_before before the first recurrence; the line $rows and $before"
((before < rows)) || fail "no row after a first recurrence, so 4 did not tell before from after: $before of $rows"
echo "ok: the line's rows are the root file's, $rows, $before before"

# 5.
for leaf in root qs; do
  wrong="$(awk -F'\t' 'FNR == NR { r[$1] = $2; next } !($5 in r) || r[$5] != $2 { n++ } END { print n + 0 }' \
    "$work/games.tsv" "$work/pool.tsv.$leaf")"
  ((wrong == 0)) || fail "$wrong rows of pool.tsv.$leaf carry a result other than their game's"
done
echo "ok: every row carries its game's result"

# 6.
past="$(awk -F'\t' '$4 >= 0 && $5 > $4 { n++ } END { print n + 0 }' "$work/games.tsv")"
((past > 0)) || fail "no game went on past its first recurrence"
echo "ok: $past games went on past their first recurrence"
