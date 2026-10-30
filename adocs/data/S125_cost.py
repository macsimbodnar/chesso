#!/usr/bin/env python3
"""S125's speed cost: the reading of the pairs adocs/data/S125_cost.sh takes,
and the static check of its forced builds. adocs/data/S123_cost.py's reading
with S125's rows; standard library only, plus adocs/data/S135_cost.py,
imported so the workload, the counters, the wall pairs and the paired CI are
S135's to the line (S270's method as DEC-266 adopts it).

    S125_cost.py code BINDIR [ROW...]
    S125_cost.py read DIR [ROW...]

ROW is a row S125_cost.sh times, from parent p1 to p7 and all, and the default
is all nine; the forced ones among them -- p1 to p7 and all,
adocs/data/S125_cost_only.patch at S125_COST 1, 2, 4, 8, 16, 32, 64 and 127 --
are the ones checked.

CODE runs before any timing on BINDIR/zero and the forced rows' binaries:
their .text sections pairwise distinct -- a forced build whose code is
zero's computes nothing, and two forced builds with one .text computed the
same groups (S135's run 1). It cannot see a group computing more than its
mask names; READ's ordering is the coarse check of that.

READ takes DIR/<comparison>/{bench16,pos11}/{base,cand}_<k>.cnt and
DIR/<comparison>/wall/pair_<k>.json for every comparison, A/A first, prints
per comparison and instrument the geometric mean ratio cand/base with its 95 %
CI, and DEC-266's reading against the A/A's floor: the floor is the A/A's
largest CI bound per workload and instrument; a cost counts when its cycles CI
on both workloads and its wall CI lie wholly beyond the floor the same way and
its instructions move the same way, and reads zero otherwise. It refuses
before any verdict unless, on both workloads, each forced build's
instructions CI lies wholly above the A/A's instruction floor and all's (the
seven groups forced together) above each group's, by more than that floor,
for every such pair whose two rows are both named; and it refuses a
comparison timed in DIR that the rows do not name. `parent` -- the tree
before S125 against the landing -- is read like any other comparison and is
expected to read zero: the landing is in no expression (adocs/data/
S125_inv6.txt).
"""

import os
import statistics
import subprocess
import hashlib
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S135_cost  # noqa: E402  (adocs/data/, by path)

from S135_cost import counters, wall  # noqa: E402

PARTS = ("p1", "p2", "p3", "p4", "p5", "p6", "p7")
FORCED = PARTS + ("all",)
ROWS = ("parent",) + FORCED
# The pairs whose first computes everything the second does and more.
NESTED = [("all", name) for name in PARTS]


def rows_of(names):
    """The rows named, in ROWS's order, or None when one is not a row."""
    names = list(names) or list(ROWS)
    bad = [n for n in names if n not in ROWS]
    if bad or len(set(names)) != len(names):
        print("refused: rows %s, not distinct names from %s" % (
            " ".join(names), " ".join(ROWS)))
        return None
    return [n for n in ROWS if n in names]


def code(bindir, rows):
    forced = [n for n in rows if n in FORCED]
    names = ["zero"] + forced
    text = {}
    for name in names:
        path = os.path.join(bindir, name)
        if not os.access(path, os.X_OK):
            print(f"refused: no binary {path}")
            return 1
        section = subprocess.run(
            ["objcopy", "-O", "binary", "--only-section=.text", path,
             "/dev/stdout"], capture_output=True, check=True).stdout
        text[name] = hashlib.sha256(section).hexdigest()[:16]
        print(f"code {name:<5} .text {text[name]}")
    ok = True
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            if text[a] == text[b]:
                print(f"refused: {a} and {b} have the same .text, so two "
                      "builds compute the same thing")
                ok = False
    if ok:
        print(f"code: {len(names)} distinct .text sections")
    return 0 if ok else 1


def separates(readings, floor, forced):
    missing = [name for name in forced if name not in readings]
    if missing:
        print("refused: no %s comparison" % " and ".join(missing))
        return False
    if not forced:
        print("refused: no forced row named, so nothing is ordered")
        return False
    ok = True
    for workload_name in ("bench16", "pos11"):
        fl = floor(workload_name, "instructions")
        ci = {name: readings[name][workload_name]["instructions"]
              for name in forced}
        print(f"forced {workload_name}: A/A instruction floor {fl * 1e9:.1f} "
              "ppb; " + ", ".join(f"{n} {ci[n][1] * 100:+.4f} .. "
                                  f"{ci[n][2] * 100:+.4f} %" for n in forced))
        for name in forced:
            if not ci[name][1] > fl:
                print(f"refused: {workload_name} {name}'s instructions CI "
                      f"starts at {ci[name][1] * 1e9:+.1f} ppb, not above the "
                      f"A/A floor: the forced build computes nothing zero "
                      "does not")
                ok = False
        for big, small in NESTED:
            if big not in ci or small not in ci:
                continue
            gap = ci[big][1] - ci[small][2]
            if not gap > fl:
                print(f"refused: {workload_name} {big} exceeds {small} by "
                      f"{gap * 1e9:+.1f} ppb at the nearest CI bounds, not "
                      f"more than the floor: {big} forces parts {small} does "
                      f"not, so {small} computed them too")
                ok = False
        for whole, sum_of in (("all", PARTS),):
            if whole not in ci or any(n not in ci for n in sum_of):
                continue
            parts = sum(ci[name][0] for name in sum_of)
            print(f"forced {workload_name}: {' + '.join(sum_of)} "
                  f"{parts * 100:+.3f} % against {whole} "
                  f"{ci[whole][0] * 100:+.3f} % (printed, not gated)")
    return ok


def read(directory, rows):
    order = sorted(d for d in os.listdir(directory)
                   if os.path.isdir(os.path.join(directory, d, "bench16")))
    if "aa" not in order:
        sys.exit("no A/A comparison: the floor cannot be set")
    order = ["aa"] + [d for d in order if d != "aa"]
    readings = {}
    for name in order:
        d = os.path.join(directory, name)
        b16, n1 = counters(os.path.join(d, "bench16"))
        p11, n2 = counters(os.path.join(d, "pos11"))
        w, n3 = wall(os.path.join(d, "wall"))
        readings[name] = {"bench16": b16, "pos11": p11, "wall": w}
        print(f"{name}: {n1} / {n2} counter pairs, {n3} wall pairs")
        for workload_name in ("bench16", "pos11"):
            for inst in ("instructions", "cycles"):
                c, lo, hi = readings[name][workload_name][inst]
                print(f"  {workload_name:<8}{inst:<13}{c * 100:+.3f} %  CI "
                      f"{lo * 100:+.3f} .. {hi * 100:+.3f} %")
        c, lo, hi = w
        print(f"  pos11   wall         {c * 100:+.3f} %  CI {lo * 100:+.3f} .. "
              f"{hi * 100:+.3f} %  (cand time over base, + is slower)")

    aa = readings["aa"]

    def floor(workload_name, inst):
        value = aa[workload_name] if inst == "wall" else aa[workload_name][inst]
        _, lo, hi = value
        return max(abs(lo), abs(hi))

    floors = [floor("bench16", "cycles"), floor("pos11", "cycles"),
              floor("wall", "wall")]
    print("A/A floor: bench16 cycles %.3f %%, pos11 cycles %.3f %%, wall %.3f %%"
          % tuple(v * 100 for v in floors))

    unnamed = [d for d in order[1:] if d not in rows]
    if unnamed:
        print("refused: %s timed here but not named" % " ".join(unnamed))
        return 1
    if not separates(readings, floor, [n for n in rows if n in FORCED]):
        return 1

    for name in order[1:]:
        r = readings[name]
        bounds = [r["bench16"]["cycles"], r["pos11"]["cycles"], r["wall"]]
        slower = all(lo > f for (_, lo, _), f in zip(bounds, floors))
        faster = all(hi < -f for (_, _, hi), f in zip(bounds, floors))
        instr = [r["bench16"]["instructions"][0],
                 r["pos11"]["instructions"][0]]
        same_way = (slower and all(x > 0 for x in instr)) or \
                   (faster and all(x < 0 for x in instr))
        if (slower or faster) and same_way:
            cost = statistics.mean([b[0] for b in bounds])
            verdict = (f"COST {cost * 100:+.2f} % (cycles and wall beyond the "
                       "floor, instructions the same way)")
        elif slower or faster:
            verdict = ("beyond the floor but instructions do not agree: owes "
                       "DEC-266 (4)'s two perturbations before it is credited; "
                       "zero until then")
        else:
            verdict = "zero (inside the A/A's floor)"
        print(f"{name}: {verdict}")
    return 0


def main(argv):
    if len(argv) >= 2 and argv[0] in ("code", "read"):
        rows = rows_of(argv[2:])
        if rows is None:
            return 1
        if argv[0] == "code":
            return code(argv[1], rows)
        return read(argv[1], rows)
    print(__doc__, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
