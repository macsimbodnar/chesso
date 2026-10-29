#!/usr/bin/env python3
"""S121's speed cost: the reading of the pairs adocs/data/S121_cost.sh takes,
and the static check of its forced builds. Standard library only, plus
adocs/data/S135_cost.py, imported so the workload, the counters, the wall
pairs and the paired CI are S135's to the line (S270's method as DEC-266
adopts it).

    S121_cost.py code BINDIR
    S121_cost.py read DIR

CODE runs before any timing on BINDIR/{zero,p1,p2a,p2b,p3,all}: the six .text
sections pairwise distinct -- a forced build whose code is zero's computes
nothing, and two forced builds with one .text computed the same parts (S135's
run 1, where a mask's zero lanes were still multiplied). It cannot see a part
computing more than its mask names; READ's ordering is the coarse check of
that.

READ takes DIR/<comparison>/{bench16,pos11}/{base,cand}_<k>.cnt and
DIR/<comparison>/wall/pair_<k>.json for every comparison, A/A first, prints
per comparison and instrument the geometric mean ratio cand/base with its 95 %
CI, and DEC-266's reading against the A/A's floor: the floor is the A/A's
largest CI bound per workload and instrument; a cost counts when its cycles CI
on both workloads and its wall CI lie wholly beyond the floor the same way and
its instructions move the same way, and reads zero otherwise. It refuses
before any verdict unless, on both workloads, each forced build's
instructions CI lies wholly above the A/A's instruction floor, all's (the
four parts forced together) lies above each part's, and p2b's (the area with
both own-pawn exclusions) above p2a's, each by more than that floor. `parent` -- the tree before S121 against the table at identity -- is
read like any other comparison and is expected to read zero or a small
negative (a table load in place of a multiply).
"""

import os
import statistics
import subprocess
import hashlib
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S135_cost  # noqa: E402  (adocs/data/, by path)

from S135_cost import counters, wall  # noqa: E402

FORCED = ("p1", "p2a", "p2b", "p3", "all")
PARTS = ("p1", "p2a", "p2b", "p3")
# The pairs whose first computes everything the second does and more.
NESTED = [("all", name) for name in PARTS] + [("p2b", "p2a")]


def code(bindir):
    names = ("zero",) + FORCED
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
        print("code: six distinct .text sections")
    return 0 if ok else 1


def separates(readings, floor):
    missing = [name for name in FORCED if name not in readings]
    if missing:
        print("refused: no %s comparison" % " and ".join(missing))
        return False
    ok = True
    for workload_name in ("bench16", "pos11"):
        fl = floor(workload_name, "instructions")
        ci = {name: readings[name][workload_name]["instructions"]
              for name in FORCED}
        print(f"forced {workload_name}: A/A instruction floor {fl * 1e9:.1f} "
              "ppb; " + ", ".join(f"{n} {ci[n][1] * 100:+.4f} .. "
                                  f"{ci[n][2] * 100:+.4f} %" for n in FORCED))
        for name in FORCED:
            if not ci[name][1] > fl:
                print(f"refused: {workload_name} {name}'s instructions CI "
                      f"starts at {ci[name][1] * 1e9:+.1f} ppb, not above the "
                      f"A/A floor: the forced build computes nothing zero "
                      "does not")
                ok = False
        for big, small in NESTED:
            gap = ci[big][1] - ci[small][2]
            if not gap > fl:
                print(f"refused: {workload_name} {big} exceeds {small} by "
                      f"{gap * 1e9:+.1f} ppb at the nearest CI bounds, not "
                      f"more than the floor: {big} forces parts {small} does "
                      f"not, so {small} computed them too")
                ok = False
        parts = sum(ci[name][0] for name in ("p1", "p2b", "p3"))
        print(f"forced {workload_name}: p1 + p2b + p3 {parts * 100:+.3f} % "
              f"against all {ci['all'][0] * 100:+.3f} % (printed, not gated)")
    return ok


def read(directory):
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

    if not separates(readings, floor):
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
    if len(argv) == 2 and argv[0] == "code":
        return code(argv[1])
    if len(argv) == 2 and argv[0] == "read":
        return read(argv[1])
    print(__doc__, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
