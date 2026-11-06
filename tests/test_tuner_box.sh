#!/usr/bin/env bash
#
# S124, the fast check's finding: tools/tuner boxes every endgame scale factor
# to [0, F] after each step and its ties (DEC-350 (5)), and nothing guarded
# those call sites. A small fit whose step drives two cells past the box, read
# from what the tuner emits:
#
#   rows A  king, rook and pawn against king and bishop, White to move: White
#           is E1's strong side, cell P=1 d=1 (endgame_cell_t 4), labelled 0.
#           The fit lowers White's score there, so the factor falls from F to
#           0 and the step would carry it below.
#   rows B  king and queen against king and rook, cell P=0 d>=2 (2), labelled
#           1: the factor would rise past F from its start.
#
# Each game holds two of each, so any game the split holds out reads both.
# The cells are counts and E1's sign; the labels are a fixture's and no
# reading of the positions. Every row VALID by python-chess, and each row's
# cell is the one named, by adocs/data/S124_cell_check.cpp (2026-11-04).
#
# Asserted: both the header's `endgame_scale` and the `--vector` file's
# factors, the best epoch's -- `--report 1` makes every epoch a candidate, so
# the emitted vector is an in-loop one -- lie in [0, 128] and agree; and the
# precondition, that the two cells sit at the box's edges, 0 and 128, so the
# box is what held them. Observed red with the in-loop box call deleted, and
# with it moved before the step (the S124 step file).
#
#   test_tuner_box.sh <tuner binary>
set -uo pipefail

fail()
{
  echo "FAIL: $*" >&2
  exit 1
}

tuner="${1:-}"
[[ -n "$tuner" && -x "$tuner" ]] || fail "no tuner binary given, or '$tuner' is not executable"

work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

# group, placement, move number; White to move.
while read -r group placement move; do
  label=0
  [[ "$group" == B ]] && label=1
  printf '%s w - - 0 %s\t%s\t0\t3\n' "$placement" "$move" "$label"
done > "$work/corpus.tsv" <<'ROWS'
A 8/8/8/8/5b2/8/2kP3R/7K 1
B 4kr2/8/8/4KQ2/8/8/8/8 2
A 1K1k4/8/8/8/R7/8/5P2/6b1 3
B 3K4/2r5/8/1Q6/8/8/7k/8 4
A 1k6/8/8/4RP2/8/5K2/b7/8 1
B 3Q4/4K3/8/1r6/8/4k3/8/8 2
A 4k3/8/8/8/8/3P4/K6R/b7 3
B 8/8/2r5/8/5k2/1Q6/4K3/8 4
A 7k/P7/8/8/3R4/4b3/1K6/8 1
B 7r/5k2/2K5/8/8/8/4Q3/8 2
A 8/8/7R/1b1K4/8/6P1/8/4k3 3
B 8/6k1/5r2/8/8/8/8/2QK4 4
A 8/1KP5/8/8/2b5/5k2/8/2R5 1
B 8/8/8/3Q4/5K1k/8/8/6r1 2
A 6k1/8/R4P2/8/1b6/8/7K/8 3
B 8/8/8/1r6/2k5/5Q2/K7/8 4
A 8/7k/8/8/2P5/8/5K2/b2R4 1
B 8/3Q4/4r3/6k1/8/8/6K1/8 2
A 8/7P/4k3/4b3/1K6/3R4/8/8 3
B 8/7k/1Q6/8/8/8/3K4/2r5 4
A 5R2/k7/8/8/8/6P1/5K2/5b2 1
B 3k4/K7/8/8/8/4r3/6Q1/8 2
A 8/2K5/7b/8/4R3/6P1/8/6k1 3
B 1k6/8/K1Q5/8/8/8/8/5r2 4
A k7/8/8/1R6/b7/7K/4P3/8 1
B 8/8/8/1k6/7r/8/2KQ4/8 2
A 1R6/7k/4b3/8/3P4/K7/8/8 3
B 6K1/Q7/5r2/8/8/8/8/2k5 4
A 8/3k4/8/5P2/8/R7/6K1/3b4 1
B 1r6/8/2k5/8/8/1Q6/8/2K5 2
A 3K4/8/8/1k6/8/3PR3/8/3b4 3
B 8/2Q5/2K5/8/8/8/k6r/8 4
A 2K5/7R/8/8/3P4/4b3/8/2k5 1
B 7Q/8/8/8/4kr2/1K6/8/8 2
A 8/6k1/8/8/6PR/1K6/8/5b2 3
B 2r5/4Q3/8/8/3k4/8/8/1K6 4
A 8/8/1R5b/8/K2k3P/8/8/8 1
B 8/8/1k4K1/8/8/2r5/8/5Q2 2
A 8/8/8/8/4b1K1/8/k5P1/1R6 3
B 1QK5/4r3/8/2k5/8/8/8/8 4
A 5b2/8/7R/1k3K2/8/8/3P4/8 1
B 5K2/8/5k2/8/Q7/8/8/6r1 2
A 7K/2P5/2R4b/8/8/7k/8/8 3
B 8/8/7Q/1k6/8/6r1/3K4/8 4
A 8/8/8/1P4bk/8/8/6R1/1K6 1
B 7Q/8/8/r7/4K3/8/5k2/8 2
A 8/3P4/4R3/8/8/k1bK4/8/8 3
B 8/K7/8/8/7r/1k6/6Q1/8 4
ROWS

"$tuner" --data "$work/corpus.tsv" --out "$work/fit.hpp" --vector "$work/fit.vec" \
  --only endgame_scale --k 1 --lr 4 --epochs 200 --report 1 --patience 1000 \
  --validation 0.25 --seed 1 --threads 1 > "$work/fit.log" 2>&1 ||
  { cat "$work/fit.log" >&2; fail "the tuner failed"; }

header="$(grep -o 'constexpr int endgame_scale\[ENDGAME_CELL_COUNT\] = {[^}]*}' "$work/fit.hpp" |
  sed 's/.*{//; s/}//; s/,//g')"
read -r -a cells <<< "$header"
[[ ${#cells[@]} -eq 13 ]] || fail "the header holds ${#cells[@]} factors, not 13: '$header'"

# The vector: `#` lines, then one integer a parameter; the factors are the last 13.
mapfile -t vec < <(grep -v '^#' "$work/fit.vec" | tail -n 13)
[[ ${#vec[@]} -eq 13 ]] || fail "the vector holds ${#vec[@]} trailing factors"

for c in "${!cells[@]}"; do
  f="${cells[$c]}"
  (( f >= 0 && f <= 128 )) || fail "header factor $c is $f, outside [0, 128]"
  [[ "${vec[$c]}" == "$f" ]] || fail "factor $c: header $f, vector ${vec[$c]}"
done

# The precondition: the step drove both cells to the box's edges.
[[ "${cells[4]}" == 0 ]] || fail "precondition: cell 4 (P=1 d=1) is ${cells[4]}, not 0"
[[ "${cells[2]}" == 128 ]] || fail "precondition: cell 2 (P=0 d>=2) is ${cells[2]}, not 128"

echo "ok: every factor in [0, 128], cells 4 and 2 at 0 and 128: ${cells[*]}"
