#!/usr/bin/env python3
"""S123's close. Every cell half of the family's fit beside its support.

    python3 adocs/data/S123_cells.py FIT REPORT TIES... > adocs/data/S123_cells.txt

FIT is a tuner header with the ten family arrays (the screen's candidate,
.tuning/s123/fits/ABC/full.hpp, which ships), REPORT the audit's report with
its `passer family (S123) support:` lines (.tuning/s123/audit/report.txt),
TIES the audit's tie files (.tuning/s123/audit/ties_{A,B,C}.txt). One line a
cell half: the fitted value, the value less the mean of its one or two
neighbours in its row (arithmetic, no judgement), the rows and games of its
support, and its tie. Then the cells whose gap is at least `--far` (10 by
default), the cutoff the closer chose, so the record names them; a B slope
multiplies a distance of 0 to 7, so its gap is per step of distance. It
decides nothing: the SPRT decided, and S126 inherits the list.

S121_cells.py's shape, over S123's arrays. Standard library only. Nothing
here asks a chess question.
"""
import argparse
import re
import sys

ARRAYS = ("passed_free", "passed_safe", "passed_king_own",
          "passed_king_enemy", "candidate_passer")
WIDTH = {"candidate_passer": 5}


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
    for array in ARRAYS:
        for half in ("mg", "eg"):
            name = "%s_%s" % (array, half)
            if len(tables.get(name, ())) != WIDTH.get(array, 6):
                sys.exit("%s: %s is missing or not %d wide"
                         % (args.fit, name, WIDTH.get(array, 6)))

    support = {}
    for line in open(args.report):
        m = re.match(r"support (\w+)_(mg|eg)\[(\d+)\] rows (\d+) games (\d+)",
                     line)
        if m and m[1] in ARRAYS:
            support[(m[1], m[2], int(m[3]))] = (int(m[4]), int(m[5]))
    if len(support) != 58:
        sys.exit("%s: %d family support lines, not 58"
                 % (args.report, len(support)))

    # A tie line is `<array>_<half> <index> <index it shares>` (S123_screen.py
    # ties), index 0 the own second rank; comments are #.
    ties = {}
    for path in args.ties:
        for line in open(path):
            words = line.split("#")[0].split()
            if len(words) == 3:
                ties[(words[0], int(words[1]))] = int(words[2])

    far = []
    print("array              half rank value  v-nbr     rows   games  tie")
    for array in ARRAYS:
        for half in ("mg", "eg"):
            name = "%s_%s" % (array, half)
            row = tables[name]
            for c, v in enumerate(row):
                near = [row[i] for i in (c - 1, c + 1) if 0 <= i < len(row)]
                gap = v - sum(near) / len(near)
                rows, games = support[(array, half, c)]
                tie = ties.get((name, c))
                print(f"{array:18} {half:4} {c + 2:4d} {v:5d} {gap:+7.1f} "
                      f"{rows:8d} {games:7d}  "
                      f"{'tied to %d' % tie if tie is not None else '-'}")
                if abs(gap) >= args.far:
                    far.append((name, c + 2, v, gap, rows, games))

    print()
    print("ties: %d" % len(ties))
    print("far, |v-nbr| >= %g (own rank, value, v-nbr, rows, games):"
          % args.far)
    for name, rank, v, gap, rows, games in far:
        print(f"  {name:22} rank {rank}  {v:5d} {gap:+7.1f} {rows:8d} "
              f"{games:7d}")
    thin = min(support.items(), key=lambda kv: kv[1][1])
    print("thinnest support: %s_%s rank %d, %d rows, %d games"
          % (thin[0][0], thin[0][1], thin[0][2] + 2, thin[1][0], thin[1][1]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
