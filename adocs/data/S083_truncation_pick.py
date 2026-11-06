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

AMENDED 2026-11-05, S122, written before it ran on S122's scan. S122's curve
(src/evaluation.hpp) adds four truncating divisions, T per king and per half,
before king safety's taper: each half's two lose under one unit between them,
White's T taken off and Black's added, so the curve's share of the residual is
under 1 and off the 1/24 lattice. The bound becomes 69/24 + 1 = 3.875 while the
curve is live, the scan's largest difference is one row, and step 1 keeps one
FEN. So, when any hit passes 72/24 = 3.0, step 1 keeps the hits past 3.0
instead, each FEN once: two taperings and the curve reach at most 46/24 + 1 =
70/24, so on each of them all three taperings truncated and the curve lost a
part of its unit. Step 4 then breaks a tie on phase by the larger difference
before the lower phase and the scan's order, so each target's row stands as
near the bound as its phase allows. With the curve not live no hit passes
69/24, and the rule is S083's as written above.

AMENDED 2026-11-06, S124, DEC-355, written before it ran on S124's rebased
tree. truncation_scan prints each row's difference less its own share of
S122's curve since DEC-355 (tuner_trace::curve_residual(): the two kings'
rounding losses in T(x), through king safety's taper, under 1 and zero at the
curve's seed). That column, a lower bound on what the three taperings lost
(the next amendment says why), has its maximum at 69/24, which no row at F
passes by more than an ulp. So step 1 keeps the
hits at the scan's largest reading, each FEN once, as S083's rule wrote it;
S122's branch past 3.0 cannot fire, no reading passing 2.9163, and the code
is unchanged. The command is unchanged too: truncation_scan --min 2.8 on
build/, this script on its stdout.

AMENDED again 2026-11-06, S124, the fast check's findings 1 and 2, written
before it ran. The residual is signed since: the scan's first column is
|smooth - exact - C|, C the curve's share of smooth - exact with its sign,
which is exactly what the three taperings lost, on the 1/24 lattice, wherever
stage two's clamp does not bind, and is bounded by the same, off the lattice,
where it does. The abs form before it, |smooth - exact| - |C|, was only a
lower bound on that loss. Step 1 keeps the hits at the scan's largest
reading as above; the code and the command are unchanged.
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
    curve = top > 3.0  # S122's amendment: past 72/24, the curve is live
    seen, at_top = set(), []
    for h in hits:
        if (h[0] > 3.0 if curve else h[0] == top) and h[3] not in seen:
            seen.add(h[3])
            at_top.append(h)

    lo = min(h[1] for h in at_top)
    hi = max(h[1] for h in at_top)
    targets = [lo, lo + (hi - lo) / 3.0, lo + 2.0 * (hi - lo) / 3.0, hi]
    sides = ["b", "w", "b", "w"]
    print("%d hits, %d distinct FENs %s %.6f, phases %d..%d, targets %s"
          % (len(hits), len(at_top), "past 3.0, the maximum" if curve else "at the maximum",
             top, lo, hi, ", ".join("%.2f" % t for t in targets)))

    picked = []
    for target, side in zip(targets, sides):
        candidates = [(abs(h[1] - target), -h[0] if curve else 0, h[1], i, h)
                      for i, h in enumerate(at_top) if h[2] == side and h not in picked]
        if not candidates:
            sys.exit("no hit with %s to move for the target phase %.2f" % (side, target))
        picked.append(min(candidates)[-1])

    for h in picked:
        print("%.6f\tphase %2d\t%s\t%s" % h)
    for h in picked:
        print('    "%s",' % h[3])


main()
