#!/usr/bin/env python3
"""S125's phase 2. Every cell half of the family's fit beside its support.

    python3 adocs/data/S125_cells.py FIT REPORT TIES... > adocs/data/S125_cells.txt

FIT is a tuner header with S125's fourteen arrays (the screen's candidate,
.tuning/s125/fits/02567/full.hpp), REPORT the audit's report with its
`structure family (S125) support:` lines (.tuning/s125/audit/report.txt), TIES
the audit's tie files (.tuning/s125/audit/ties_P{1..7}.txt). One line a cell
half: the fitted value, the value less the mean of its one or two neighbours in
its row (arithmetic, no judgement), the rows and games of its support, and its
tie. A row of one cell has no neighbour, so its gap reads 0. Then the cells
whose gap is at least `--far` (10 by default, S123's cutoff) and the thinnest
support. An array of a dropped group is all zero and is printed as such. It
decides nothing: the SPRT decides, and S126 inherits the list.

S123_cells.py's shape, over S125's arrays; the cell is labelled as the array
indexes it, by file or by own rank. Standard library only. Nothing here asks a
chess question.
"""
import argparse
import re
import sys

# array -> width, and the label of cell 0 onward: the files b to h after the
# dropped a file, or own ranks (DEC-331 (4)).
ARRAYS = {"isolated_backward": ["-"],
          "isolated_file": list("bcdefgh"),
          "doubled_file": list("bcdefgh"),
          "backward_rank": ["3", "4", "5"],
          "phalanx": ["2", "3", "4", "5", "6", "7"],
          "supported": ["3", "4", "5", "6", "7"],
          "weak_unopposed": ["-"]}
CELLS = sum(2 * len(labels) for labels in ARRAYS.values())


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("fit")
    ap.add_argument("report")
    ap.add_argument("ties", nargs="+")
    ap.add_argument("--far", type=float, default=10.0)
    args = ap.parse_args()

    text = open(args.fit).read()
    tables = {name: [int(v) for v in body.split(",") if v.strip()]
              for name, body in re.findall(
                  r"constexpr int (\w+)\[\d+\]\s*=\s*\{([^}]*)\}", text)}
    for array, labels in ARRAYS.items():
        for half in ("mg", "eg"):
            name = "%s_%s" % (array, half)
            if len(tables.get(name, ())) != len(labels):
                sys.exit("%s: %s is missing or not %d wide"
                         % (args.fit, name, len(labels)))

    support = {}
    for line in open(args.report):
        m = re.match(r"support (\w+)_(mg|eg)\[(\d+)\] rows (\d+) games (\d+)",
                     line)
        if m and m[1] in ARRAYS:
            support[(m[1], m[2], int(m[3]))] = (int(m[4]), int(m[5]))
    if len(support) != CELLS:
        sys.exit("%s: %d family support lines, not %d"
                 % (args.report, len(support), CELLS))

    # A tie line is `<array>_<half> <index> <index it shares>` (S125_screen.py
    # ties); comments are #.
    ties = {}
    for path in args.ties:
        for line in open(path):
            words = line.split("#")[0].split()
            if len(words) == 3:
                ties[(words[0], int(words[1]))] = int(words[2])

    far = []
    print("array              half cell value  v-nbr     rows   games  tie")
    for array, labels in ARRAYS.items():
        for half in ("mg", "eg"):
            name = "%s_%s" % (array, half)
            row = tables[name]
            for c, v in enumerate(row):
                near = [row[i] for i in (c - 1, c + 1) if 0 <= i < len(row)]
                gap = v - sum(near) / len(near) if near else 0.0
                rows, games = support[(array, half, c)]
                tie = ties.get((name, c))
                print(f"{array:18} {half:4} {labels[c]:>4} {v:5d} {gap:+7.1f} "
                      f"{rows:8d} {games:7d}  "
                      f"{'tied to %d' % tie if tie is not None else '-'}")
                if abs(gap) >= args.far:
                    far.append((name, labels[c], v, gap, rows, games))

    print()
    print("ties: %d" % len(ties))
    print("far, |v-nbr| >= %g (cell, value, v-nbr, rows, games):" % args.far)
    for name, label, v, gap, rows, games in far:
        print(f"  {name:22} cell {label}  {v:5d} {gap:+7.1f} {rows:8d} "
              f"{games:7d}")
    thin = min(support.items(), key=lambda kv: kv[1][1])
    print("thinnest support: %s_%s cell %s, %d rows, %d games"
          % (thin[0][0], thin[0][1], ARRAYS[thin[0][0]][thin[0][2]],
             thin[1][0], thin[1][1]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
