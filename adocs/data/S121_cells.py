#!/usr/bin/env python3
"""S121 phase 2. Every mobility cell half of a fit beside its support.

    python3 adocs/data/S121_cells.py FIT REPORT TIES > adocs/data/S121_cells.txt

FIT is a tuner header with the eight `mobility_<piece>_<half>` tables (the
screen's candidate, .tuning/s121/fits/table_m5/full.hpp), REPORT the audit's
report with its `support` lines (.tuning/s121/audit/m5/report.txt), TIES the
audit's tie file (.tuning/s121/audit/m5/ties.txt). One line a cell half: the
fitted value, the value less the mean of its one or two neighbours in its row
(arithmetic, no judgement), the rows and games of its support, its tie, and
`named` on the cells the coordinator's phase 2 brief listed as far from their
neighbours. It decides nothing; the record shows which cells are thin.

Standard library only. Nothing here asks a chess question.
"""
import re
import sys

PIECES = ("knight", "bishop", "rook", "queen")
NAMED = {("queen", "mg"): range(20, 26), ("queen", "eg"): (0, 24),
         ("knight", "eg"): (8,), ("bishop", "mg"): range(11, 14)}


def main(fit, report, ties_path):
    text = open(fit).read()
    tables = {name: [int(v) for v in body.replace("\n", " ").split(",")
                     if v.strip()]
              for name, body in re.findall(
                  r"constexpr int (mobility_\w+)\[\d+\]\s*=\s*\{([^}]*)\}", text)}
    if len(tables) != 8:
        sys.exit("%s: %d mobility tables, not 8" % (fit, len(tables)))

    support = {}
    for line in open(report):
        m = re.match(r"support mobility_(mg|eg)\[(\w+)\]\[(\d+)\] rows (\d+) "
                     r"games (\d+)", line)
        if m:
            support[(m[2], m[1], int(m[3]))] = (int(m[4]), int(m[5]))
    if len(support) != 132:
        sys.exit("%s: %d support lines, not 132" % (report, len(support)))

    ties = {}
    for line in open(ties_path):
        words = line.split("#")[0].split()
        if len(words) == 3:
            ties[(words[0], int(words[1]))] = int(words[2])

    print("piece  half cell value  v-nbr     rows   games  tie        brief")
    for piece in PIECES:
        for half in ("mg", "eg"):
            name = "mobility_%s_%s" % (piece, half)
            row = tables[name]
            for c, v in enumerate(row):
                near = [row[i] for i in (c - 1, c + 1) if 0 <= i < len(row)]
                rows, games = support[(piece, half, c)]
                tie = ties.get((name, c))
                line = (f"{piece:6} {half:4} {c:4d} {v:5d} "
                        f"{v - sum(near) / len(near):+7.1f} {rows:8d} {games:7d}  "
                        f"{'tied to %d' % tie if tie is not None else '-':10} "
                        f"{'named' if c in NAMED.get((piece, half), ()) else ''}")
                print(line.rstrip())
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    sys.exit(main(*sys.argv[1:]))
