#!/usr/bin/env python3
"""Pick the four truncation positions from a truncation_scan, by a stated rule.

tests/test_eval_trace.cpp's `truncation_positions` are re-derived by
`build/tools/truncation_scan --data <corpus> --min 2.8` after a refit (DEC-057,
DEC-142), "the four ... chosen from the hits at the maximum to span the taper
with both sides to move". Since S083 the four live in that file alone; until
then they were also four rows of tests/test_eval_positions.hpp, where S076's
four stay as coverage rows, that list being test_tuner_gradient's fixture. Until S083 the choosing was by hand; this is that sentence as a rule,
so the choice is re-derived and not re-read:

  1. Keep the hits at the scan's largest difference (69/24 = 2.875 when any
     row reaches it), each FEN once, in the scan's order -- the corpus's.
  2. Targets: the lowest phase among them, the highest, and the two thirds
     between, `lo + (hi - lo) / 3` and `lo + 2 (hi - lo) / 3`.
  3. Sides to move b, w, b, w in that order, the pattern the four before
     S083 had (phases 5, 11, 19, 23).
  4. For each target in turn, the unpicked hit with that side to move whose
     phase is nearest the target; ties to the lower phase, then the scan's
     order. A target with no hit of its side refuses the pick (exit 1).

Usage: S083_truncation_pick.py SCAN.txt   (truncation_scan's stdout)
Prints the four as `difference  phase  side  fen` and then as C++ lines.
"""
import sys


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: S083_truncation_pick.py SCAN.txt")

    hits = []
    for line in open(sys.argv[1]):
        fields = line.rstrip("\n").split("\t")
        if len(fields) != 4 or not fields[1].startswith("phase"):
            continue
        hits.append((float(fields[0]), int(fields[1].split()[1]), fields[2], fields[3]))
    if not hits:
        sys.exit("no hits in %s" % sys.argv[1])

    top = max(h[0] for h in hits)
    seen, at_top = set(), []
    for h in hits:
        if h[0] == top and h[3] not in seen:
            seen.add(h[3])
            at_top.append(h)

    lo = min(h[1] for h in at_top)
    hi = max(h[1] for h in at_top)
    targets = [lo, lo + (hi - lo) / 3.0, lo + 2.0 * (hi - lo) / 3.0, hi]
    sides = ["b", "w", "b", "w"]
    print("%d hits, %d distinct FENs at the maximum %.6f, phases %d..%d, targets %s"
          % (len(hits), len(at_top), top, lo, hi,
             ", ".join("%.2f" % t for t in targets)))

    picked = []
    for target, side in zip(targets, sides):
        candidates = [(abs(h[1] - target), h[1], i, h) for i, h in enumerate(at_top)
                      if h[2] == side and h not in picked]
        if not candidates:
            sys.exit("no hit with %s to move for the target phase %.2f" % (side, target))
        picked.append(min(candidates)[3])

    for h in picked:
        print("%.6f\tphase %2d\t%s\t%s" % h)
    for h in picked:
        print('    "%s",' % h[3])


main()
