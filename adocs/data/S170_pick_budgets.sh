#!/usr/bin/env bash
# S203's budget rule applied by script to a recorded grid, never by hand: for
# each case row of the TSV, the cheapest budget at the row's own stride whose
# cell reports a mate line. Prints the rewritten TSV on stdout and the picks
# on stderr. usage: S170_pick_budgets.sh CASES.tsv GRID.txt > NEW.tsv
#
# S290. This is `.tuning/coord/S099/p2/pick_budgets.sh`, sha256
# 6c18cf5733cab362c0392b7aeac1c58426aef8b2e9434a8be365153f686b4809, which
# applied the rule from S099 until S290 from outside the tree, so the budgets
# it chose could not be re-derived from a clone (DEC-142). Changes
# from it, and only these: the file's name in the usage line above, this
# paragraph and the two below, `set -euo pipefail`, and the usage and
# readability checks before the awk program, which a missing argument or an
# unreadable file used to reach as an awk error or as every row reporting no
# cell. The awk program is byte for byte the original's.
#
# The rule is DEC-156's as DEC-162 left it -- the cheapest budget at the row's
# own stride whose cell reports a mate line, chosen on the mate count alone,
# short lines being the ceiling's business -- and
# `adocs/data/S203_case_sweep.sh` states it beside the grid it records. That
# script makes the grid (no argument, a built engine, one line per cell: case,
# stride, nodes, mate lines, short lines); this one reads a recorded grid and
# rewrites the `go` column of adocs/data/S170_cases.tsv and nothing else --
# comment lines, the header row and every other column pass through untouched:
#
#   adocs/data/S170_pick_budgets.sh adocs/data/S170_cases.tsv \
#       adocs/data/<step>_sweep_s170.txt > NEW.tsv
#
# Exit 0 when every case row found a cell; 1 when one did not, naming it on
# stderr and printing that row unchanged; 2 on a usage error.
# `adocs/data/S290_picker_check.txt` is its proof on S083's grid.
set -euo pipefail
if [[ $# -ne 2 ]]; then
  echo "usage: $0 CASES.tsv GRID.txt > NEW.tsv" >&2
  exit 2
fi
for f in "$1" "$2"; do
  [[ -r "$f" ]] || { echo "cannot read $f" >&2; exit 2; }
done
cases=$1; grid=$2
awk -F'\t' -v OFS='\t' -v grid="$grid" '
  BEGIN {
    while ((getline line < grid) > 0) {
      n = split(line, f, " +")
      # case stride nodes mates short, five fields on a grid row
      if (n == 5 && f[2] ~ /^[0-9]+$/ && f[4] + 0 > 0) {
        key = f[1] SUBSEP f[2]
        if (!(key in best) || f[3] + 0 < best[key] + 0) { best[key] = f[3]; mates[key] = f[4]; short[key] = f[5] }
      }
    }
  }
  /^#/ || NF < 2 || $1 == "name" { print; next }
  {
    key = $1 SUBSEP $6
    if (!(key in best)) { printf "no cell with a mate line for %s at stride %s\n", $1, $6 > "/dev/stderr"; bad = 1; print; next }
    old = $4
    $4 = "nodes " best[key]
    printf "%-26s stride %s  %-14s -> %-14s (%s lines, %s short)\n", $1, $6, old, $4, mates[key], short[key] > "/dev/stderr"
    print
  }
  END { exit bad }
' "$cases"
