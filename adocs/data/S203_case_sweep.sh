#!/bin/bash
# Re-derive adocs/data/S170_cases.tsv's `go` budgets and the
# expected_mate_lines() floors in tests/test_mate_carry.cpp.
#
# S203, DEC-156. The case set is eviction reproductions and the Zobrist keys
# decide eviction, so a redraw of the keys retires the budgets the rows carry
# (DEC-154). This is the script that chooses new ones, and the rule it applies
# is stated once and applied to every row rather than tuned per case:
#
#   the cheapest budget at which the case reports at least its floor of mate
#   lines with all of them complete.
#
# "Cheapest" matters: a budget is not chosen because it is green. A case that is
# green only far past a window where it is red has that window recorded as an
# S202 reproduction, not hidden behind the budget that clears it.
#
# Run from the repository root with a built engine:
#
#   adocs/data/S203_case_sweep.sh                 # every case, the sweep grid
#   adocs/data/S203_case_sweep.sh --at            # every case at its TSV setting
#
# Each line is: case, stride, nodes, mate lines reported, short lines. A row is
# usable when mate lines clears its floor and short is 0.
set -uo pipefail

cd "$(dirname "$0")/../.." || exit 1

ENGINE="${ENGINE:-build/src/chesso}"
CASES="${CASES:-adocs/data/S170_cases.tsv}"
PYTHON="${PYTHON:-$HOME/.venv/chess/bin/python}"
REPLAY="adocs/data/S170_replay.py"

[[ -x "$ENGINE" ]] || { echo "no engine at $ENGINE, build it first" >&2; exit 1; }
[[ -r "$CASES" ]] || { echo "no cases at $CASES" >&2; exit 1; }

names=$(awk -F'\t' '!/^#/ && NF > 1 && $1 != "name" { print $1 }' "$CASES")

# One run of the replay tool; prints "<mate lines> <short lines>".
counts() {
  local out
  out=$("$PYTHON" "$REPLAY" --engine "$ENGINE" --cases "$CASES" --only "$1" \
          --hash 16 --verbose "${@:2}" 2>&1)
  echo "$(grep -cE '^  ply +[0-9]+ depth' <<< "$out") $(grep -c '^SHORT' <<< "$out")"
}

if [[ "${1:-}" == "--at" ]]; then
  echo "Each case at the budget and stride its TSV row carries."
  printf "%-24s %6s %6s\n" "case" "mates" "short"
  for name in $names; do
    read -r mates short < <(counts "$name")
    printf "%-24s %6d %6d\n" "$name" "$mates" "$short"
  done
  exit 0
fi

echo "The sweep grid. Stride 2 is one side's own turns, which is what a game"
echo "gives an engine's table; stride 1 is a table no game produces (DEC-151),"
echo "and the existing rows use it, so both are swept."
printf "%-24s %7s %9s %6s %6s\n" "case" "stride" "nodes" "mates" "short"
for name in $names; do
  for stride in 1 2; do
    for n in 100000 300000 500000 1000000 1200000 1500000 2000000 3000000 4000000; do
      read -r mates short < <(counts "$name" --go "nodes $n" --stride-override "$stride")
      printf "%-24s %7d %9d %6d %6d\n" "$name" "$stride" "$n" "$mates" "$short"
    done
  done
done
