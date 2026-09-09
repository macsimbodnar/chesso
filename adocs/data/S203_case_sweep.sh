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
#   adocs/data/S203_case_sweep.sh --ceilings F... # short_line_ceiling() from
#                                                 # recorded sweep files
#
# S204, DEC-162 added `--ceilings`. `tests/test_mate_carry.cpp` no longer holds
# a per-case floor on mate lines -- one cell of this grid is one sample and not
# a property -- and holds a per-case ceiling on short lines instead. The rule,
# stated once and applied to every row: the ceiling is the largest short-line
# count any cell of the recorded grid shows for that case, at the stride its TSV
# row carries, across every recorded sweep given. Re-derive it from the tracked
# grids, never from a failing run:
#
#   adocs/data/S203_case_sweep.sh --ceilings \
#       adocs/data/S204_sweep_head.txt adocs/data/S204_sweep_killer_iter_clear.txt
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

if [[ "${1:-}" == "--ceilings" ]]; then
  shift
  [[ $# -gt 0 ]] || { echo "--ceilings needs at least one recorded sweep file" >&2; exit 1; }
  strides=$(mktemp)
  awk -F'\t' '!/^#/ && NF > 1 && $1 != "name" { print $1 "\t" $6 }' "$CASES" > "$strides"
  echo "The short-line ceiling per case: the worst cell of the recorded grid at"
  echo "the stride the case itself carries. tests/test_mate_carry.cpp holds these."
  printf "%-26s %8s\n" "case" "ceiling"
  # A case with no row at its own stride is an error and not an omission: the
  # ceilings are a golden and a short table would quietly ship the wrong one.
  # Reachable whenever a row's stride moves or a sweep file is partial.
  awk -v strides="$strides" '
    BEGIN { while ((getline line < strides) > 0) { split(line, f, "\t"); want[f[1]] = f[2] } }
    NF == 5 && $2 ~ /^[0-9]+$/ && ($1 in want) && $2 == want[$1] {
      if ($5 + 0 > max[$1]) { max[$1] = $5 + 0 }
      seen[$1] = 1
    }
    END {
      missing = ""
      for (n in want) { if (!(n in seen)) { missing = missing " " n } }
      if (missing != "") {
        printf "no row at its own stride for:%s\n", missing > "/dev/stderr"
        exit 1
      }
      for (n in seen) printf "%-26s %8d\n", n, max[n]
    }
  ' "$@" | sort
  status=${PIPESTATUS[0]}
  rm -f "$strides"
  exit "$status"
fi

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
