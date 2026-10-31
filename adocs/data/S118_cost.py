#!/usr/bin/env python3
"""S118's speed verdict: the reading of the pairs adocs/data/S118_cost.sh takes,
and the static check of its builds. adocs/data/S125_cost.py's shape with
DEC-332's rules in place of its cost reading; standard library only, plus
adocs/data/S135_cost.py, imported so the counters, the wall pairs and the paired
CI are S135's to the line (S270's method as DEC-266 adopts it), and
adocs/data/S125_objdiff.py, for its normalised disassembly.

    S118_cost.py code OUT [ROW...]
    S118_cost.py read DIR [ROW...]

ROW is a row S118_cost.sh times, from s8 s10 s12 s14 s16 s18 total slot, and the
default is all eight.

CODE runs before any timing, on OUT/bin and each size row's object files: every
.text pairwise distinct, and each size row's code against s12's, function by
function, printed. A size row is the commit with one source line changed, which
the script has already held to, so the code may differ only where the table's
size is read: the index's mask and the size of what holds the table. It prints the
differing lines, and refuses a function outside SIZE_READERS that differs, which is
the object-level half of "the rows differ by the table's size alone".

READ takes DIR/<comparison>/{bench16,pos11}/{base,cand}_<k>.cnt and
DIR/<comparison>/wall/pair_<k>.json for every comparison, A/A first, and prints
per comparison and instrument the geometric mean ratio cand/base with its 95 % CI,
then applies DEC-332's three rules, written in S118_cost.sh's header before any
run:

  (7) the size: the smallest of s8 to s16 whose cycles on both workloads lie
      within the floor of the best row. s18 is informative, never a candidate.
  (8) the bar, DEC-266 (3), (4) and no other: it ships if the chosen size's
      cycles on both workloads and its wall clear the floor as a gain and its
      instructions move the same way. Zero or a loss: not shipped.
  (5) the shelter: `slot` stays if it does not read a loss beyond the floor.

Rule (7) reads point estimates: the row's change against the best row's, on each
workload, no more than that workload's cycles floor apart. The best row is each
workload's own. It prints what it applied, and decides nothing the coordinator
cannot read back from the table above it.
"""

import glob
import hashlib
import os
import re
import statistics
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S125_objdiff  # noqa: E402  (adocs/data/, by path)

from S135_cost import counters, wall  # noqa: E402

CANDIDATES = ("s8", "s10", "s12", "s14", "s16")
SIZES = CANDIDATES + ("s18",)
ROWS = SIZES + ("total", "slot")

# The binary each row's candidate side is, for the code check.
BINARY = {name: name for name in SIZES}
BINARY.update({"total": "shelter", "slot": "shelter"})

# What may read the table's size, by demangled name: the two forms that index
# the table, and whatever builds, zeroes or frees the struct that holds it. Read
# off the first real comparison, 2026-10-31 (S118's step file), not assumed: at
# every size the same functions, the same instruction counts, and the differing
# lines only the mask and the struct's size.
SIZE_READERS = (
    "evaluate(board_t const*, pawn_table_t*)",
    "evaluate_lazy(board_t const*, int, int, bool*, pawn_table_t*)",
    "iterative_deepening_search(",
    "leaf_walker_t::leaf_walker_t(",
    "leaf_walker_t::~leaf_walker_t(",
)


def rows_of(names):
    """The rows named, in ROWS's order, or None when one is not a row."""
    names = list(names) or list(ROWS)
    bad = [n for n in names if n not in ROWS]
    if bad or len(set(names)) != len(names):
        print("refused: rows %s, not distinct names from %s" % (
            " ".join(names), " ".join(ROWS)))
        return None
    return [n for n in ROWS if n in names]


def objects(out, name):
    """{function: normalised instructions} over one row's engine objects: as
    S125_objdiff reads them, and without the numbers of the compiler's own local
    labels, which an unlinked object shows as call targets and which shift with
    any literal it emits."""
    found = {}
    pattern = os.path.join(out, "tree-" + name, "build", "src", "CMakeFiles",
                           "chesso_engine.dir", "*.o")
    paths = sorted(glob.glob(pattern))
    for path in paths:
        for function, lines in S125_objdiff.functions(path).items():
            found[function] = [re.sub(r"<\.L[A-Z]*\d+>", "<.L>", line)
                               for line in lines]
    return found, len(paths)


def code(out, rows):
    bindir = os.path.join(out, "bin")
    names = ["base"] + sorted({BINARY[r] for r in rows})
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
        print(f"code {name:<8} .text {text[name]}")
    ok = True
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            if text[a] == text[b]:
                print(f"refused: {a} and {b} have the same .text")
                ok = False
    if ok:
        print(f"code: {len(names)} distinct .text sections")

    sizes = [r for r in rows if r in SIZES and r != "s12"]
    if sizes and "s12" in rows:
        reference, count = objects(out, "s12")
        if not count:
            print("refused: no object files for s12")
            return 1
        for name in sizes:
            other, count = objects(out, name)
            if not count:
                print(f"refused: no object files for {name}")
                return 1
            differs = sorted(f for f in set(reference) | set(other)
                             if reference.get(f) != other.get(f))
            print(f"code {name} against s12: {len(differs)} functions differ")
            shown = set()
            for function in differs:
                allowed = any(function.startswith(r) for r in SIZE_READERS)
                before = reference.get(function, [])
                after = other.get(function, [])
                print("  %-7s %5d %5d  %s" % (
                    "size" if allowed else "REFUSED", len(before), len(after),
                    function[:70]))
                if not allowed:
                    ok = False
                shown.update("    - " + line for line in set(before) - set(after))
                shown.update("    + " + line for line in set(after) - set(before))
            for line in sorted(shown, key=lambda text: (text[6:], text[4])):
                print(line[:80])
    return 0 if ok else 1


def verdict(reading, floors):
    """'gain', 'loss', 'zero', or either of the first two owing DEC-266 (4)."""
    bounds = [reading["bench16"]["cycles"], reading["pos11"]["cycles"],
              reading["wall"]]
    slower = all(lo > f for (_, lo, _), f in zip(bounds, floors))
    faster = all(hi < -f for (_, _, hi), f in zip(bounds, floors))
    instructions = [reading["bench16"]["instructions"][0],
                    reading["pos11"]["instructions"][0]]
    if faster:
        return "gain" if all(x < 0 for x in instructions) else "gain, owing (4)"
    if slower:
        return "loss" if all(x > 0 for x in instructions) else "loss, owing (4)"
    return "zero"


def read(directory, rows):
    order = sorted(d for d in os.listdir(directory)
                   if os.path.isdir(os.path.join(directory, d, "bench16")))
    if "aa" not in order:
        sys.exit("no A/A comparison: the floor cannot be set")
    unnamed = [d for d in order if d != "aa" and d not in rows]
    if unnamed:
        print("refused: %s timed here but not named" % " ".join(unnamed))
        return 1
    missing = [r for r in rows if r not in order]
    if missing:
        print("refused: %s named but not timed" % " ".join(missing))
        return 1

    readings = {}
    for name in ["aa"] + rows:
        d = os.path.join(directory, name)
        b16, n1 = counters(os.path.join(d, "bench16"))
        p11, n2 = counters(os.path.join(d, "pos11"))
        w, n3 = wall(os.path.join(d, "wall"))
        readings[name] = {"bench16": b16, "pos11": p11, "wall": w}
        print(f"{name}: {n1} / {n2} counter pairs, {n3} wall pairs")
        for workload in ("bench16", "pos11"):
            for instrument in ("instructions", "cycles"):
                c, lo, hi = readings[name][workload][instrument]
                print(f"  {workload:<8}{instrument:<13}{c * 100:+.3f} %  CI "
                      f"{lo * 100:+.3f} .. {hi * 100:+.3f} %")
        c, lo, hi = w
        print(f"  pos11   wall         {c * 100:+.3f} %  CI {lo * 100:+.3f} .. "
              f"{hi * 100:+.3f} %  (cand time over base, + is slower)")

    aa = readings["aa"]
    floors = [max(abs(aa["bench16"]["cycles"][1]), abs(aa["bench16"]["cycles"][2])),
              max(abs(aa["pos11"]["cycles"][1]), abs(aa["pos11"]["cycles"][2])),
              max(abs(aa["wall"][1]), abs(aa["wall"][2]))]
    print("A/A floor: bench16 cycles %.3f %%, pos11 cycles %.3f %%, wall %.3f %%"
          % tuple(v * 100 for v in floors))

    for name in rows:
        r = readings[name]
        mean = statistics.mean([r["bench16"]["cycles"][0], r["pos11"]["cycles"][0],
                                r["wall"][0]])
        print(f"{name}: {verdict(r, floors)}, mean of the three {mean * 100:+.2f} %")

    # DEC-332 (7): smallest candidate within the floor of the best, both ways.
    timed = [r for r in CANDIDATES if r in rows]
    chosen = None
    if timed:
        best = {w: min(readings[r][w]["cycles"][0] for r in timed)
                for w in ("bench16", "pos11")}
        print("size: best cycles bench16 %+.3f %%, pos11 %+.3f %%, among %s"
              % (best["bench16"] * 100, best["pos11"] * 100, " ".join(timed)))
        within = [r for r in timed
                  if readings[r]["bench16"]["cycles"][0] - best["bench16"]
                  <= floors[0] and
                  readings[r]["pos11"]["cycles"][0] - best["pos11"] <= floors[1]]
        chosen = within[0]
        print(f"size: within the floor of the best {' '.join(within)}; "
              f"DEC-332 (7) chooses {chosen}")

        # DEC-332 (8): the bar.
        answer = verdict(readings[chosen], floors)
        if answer == "gain":
            print(f"bar: {chosen} clears DEC-266 (3), (4): SHIPS")
        elif answer == "gain, owing (4)":
            print(f"bar: {chosen} clears the floor but its instructions do not "
                  "move with it: owes DEC-266 (4)'s two perturbations, zero until "
                  "then")
        else:
            print(f"bar: {chosen} reads {answer}: NOT SHIPPED")

    # DEC-332 (5): the slot's own row.
    if "slot" in rows:
        answer = verdict(readings["slot"], floors)
        if answer == "loss":
            print("shelter: a loss beyond the floor: the slot is dropped and S122 "
                  "decides")
        elif answer == "loss, owing (4)":
            print("shelter: beyond the floor as a loss while its instructions "
                  "fall: owes DEC-266 (4)'s two perturbations before it is read "
                  "as one; it stays until then")
        else:
            print(f"shelter: reads {answer}: the slot stays, S122 being the reason")
        if chosen not in (None, "s12"):
            print(f"shelter: timed at s12 and {chosen} is chosen: `total` and "
                  "`slot` are s12's, and the landing at another size says so")
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
