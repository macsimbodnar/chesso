#!/usr/bin/env python3
"""S082's recipe reading, the rule adocs/data/S082_recipe.sh pre-registers.

    python3 adocs/data/S082_recipe_read.py .tuning/s082
    python3 adocs/data/S082_recipe_read.py --smoke .tuning/s082/smoke/datagen.log

The first form reads the fit logs `S082_recipe.sh compare` leaves --
fit_<leaf>_<per game>_s<seed>_<view>.log, each the tuner's own console -- the
sample lines in compare.log and the pool's per-leaf lines in generate.log,
prints one row per fit and every leaf's refusal shares, and applies the rule as
written in the script's header, without judgement:

  1. L(c), the `best:` line's validation figure of c's primary (qs view) fit
  2. the control's L the mean of its five seeds, s their sample standard
     deviation, m = 2.49 * sqrt(2) * s
  3. if L(control) - min L over the eight cells <= m, the control stands
  4. else, of the eight within m of the best, the most rows a game wins, and
     at equal density qs before pv before root
  5. the root view, K, rows a game and the refusal shares are printed and
     decide nothing (DEC-275)

Exit 0 with a decision; 1 when a log is missing or unreadable, or s is above
1e-3 -- outcome (c), nothing chosen.

The second form is the abort rule over a smoke run's datagen log: per walked
leaf, the share of the positions that passed refused as truncated, aborted or
uncertified, against 1 %. Terminal refusals are printed and not gated on.
Exit 1 when the rule says abort or the log carries no leaf line.
"""

import math
import os
import re
import statistics
import sys

LEAVES = ("root", "qs", "pv")
DENSITIES = (0, 46, 8)
CONTROL = ("root", 0)
CONTROL_SEEDS = (1, 2, 3, 4, 5)
PICK_QUANTILE = 2.49  # 95 % point of the largest of eight standard normals
SPREAD_UNREADABLE = 1.0e-03
ABORT_SHARE = 0.01
PREFERENCE = {"qs": 0, "pv": 1, "root": 2}

BEST = re.compile(r"^best: train (\S+)\s+validation (\S+)")
K = re.compile(r"^fitted K = (\S+)")
SAMPLE = re.compile(r"^sample: (\d+) rows from (\d+) games .*?(\S+)$")
REFUSED = re.compile(
    r"^leaf (\w+): not written of (\d+) passed: terminal (\d+) \([^)]*\) "
    r"truncated (\d+) \([^)]*\) aborted (\d+) \([^)]*\) uncertified (\d+)")
WRITTEN = re.compile(r"^leaf (\w+): \d+ rows from")


class Unreadable(Exception):
    pass


def read_fit(path):
    if not os.path.exists(path):
        raise Unreadable(f"missing {path}")
    best = k = None
    with open(path) as handle:
        for line in handle:
            match = BEST.match(line)
            if match:
                best = float(match.group(2))
            match = K.match(line)
            if match:
                k = float(match.group(1))
    if best is None:
        raise Unreadable(f"no best: line in {path}")
    return best, k


def read_samples(path):
    """{corpus file name: (rows, games)} from compare.log's sample lines."""
    out = {}
    if not os.path.exists(path):
        return out
    with open(path) as handle:
        for line in handle:
            match = SAMPLE.match(line.strip())
            if match:
                out[os.path.basename(match.group(3))] = (
                    int(match.group(1)), int(match.group(2)))
    return out


def read_refusals(path):
    """{leaf: (passed, terminal, truncated, aborted, uncertified)} and the
    datagen lines about each leaf, from a datagen log."""
    refusals = {}
    lines = []
    if not os.path.exists(path):
        return refusals, lines
    with open(path) as handle:
        for line in handle:
            line = line.rstrip("\n")
            match = REFUSED.match(line)
            if match:
                refusals[match.group(1)] = tuple(
                    int(match.group(i)) for i in range(2, 7))
            if match or WRITTEN.match(line):
                lines.append(line)
    return refusals, lines


def share(part, whole):
    return part / whole if whole else 0.0


def smoke(path):
    refusals, lines = read_refusals(path)
    for line in lines:
        print(line)
    if not refusals:
        print(f"abort rule: no leaf refusal line in {path}; nothing to read")
        return 1

    abort = False
    for leaf, (passed, terminal, truncated, aborted, uncertified) in \
            refusals.items():
        gated = share(truncated + aborted + uncertified, passed)
        print(f"abort rule, leaf {leaf}: truncated + aborted + uncertified "
              f"{100 * gated:.3f} % of {passed} passed against "
              f"{100 * ABORT_SHARE:.0f} %; terminal "
              f"{100 * share(terminal, passed):.3f} %, printed, not gated")
        abort = abort or gated > ABORT_SHARE

    print("abort rule: " + ("ABORT, the night does not start" if abort
                            else "clear"))
    return 1 if abort else 0


def reading(out):
    samples = read_samples(os.path.join(out, "compare.log"))

    cells = [(leaf, density, 1) for leaf in LEAVES for density in DENSITIES
             if (leaf, density) != CONTROL]
    cells += [(CONTROL[0], CONTROL[1], seed) for seed in CONTROL_SEEDS]

    rows = {}
    try:
        for leaf, density, seed in cells:
            name = f"{leaf}_{density}_s{seed}"
            primary, k = read_fit(os.path.join(out, f"fit_{name}_qs.log"))
            secondary, _ = read_fit(os.path.join(out, f"fit_{name}_root.log"))
            drawn = samples.get(f"cand_{name}.tsv", (0, 0))
            rows[(leaf, density, seed)] = (primary, secondary, k, drawn)
    except Unreadable as why:
        print(f"S082 reading: unreadable, {why}")
        return 1

    print("fit               L (qs view)  root view       K     rows   games"
          "  rows/game")
    for (leaf, density, seed), (primary, secondary, k, drawn) in rows.items():
        per = drawn[0] / drawn[1] if drawn[1] else 0.0
        label = f"{leaf} {density if density else 'all'} s{seed}"
        print(f"{label:<16}  {primary:.6f}    {secondary:.6f}  "
              f"{(k or 0.0):.4f}  {drawn[0]:7d}  {drawn[1]:6d}  {per:9.2f}")

    refusals, lines = read_refusals(os.path.join(out, "generate.log"))
    print("the pool's leaves, from generate.log:")
    for line in lines:
        print(f"  {line}")
    for leaf, (passed, terminal, truncated, aborted, uncertified) in \
            refusals.items():
        print(f"  {leaf}: refused {100 * share(terminal, passed):.3f} % "
              f"terminal, {100 * share(truncated, passed):.3f} % truncated, "
              f"{100 * share(aborted, passed):.3f} % aborted, "
              f"{100 * share(uncertified, passed):.3f} % uncertified, "
              f"of {passed} passed")

    controls = [rows[(CONTROL[0], CONTROL[1], seed)][0]
                for seed in CONTROL_SEEDS]
    control = statistics.mean(controls)
    spread = statistics.stdev(controls)
    margin = PICK_QUANTILE * math.sqrt(2.0) * spread
    print(f"control L = {control:.6f}, mean of {len(controls)}; "
          f"s = {spread:.6f}; m = 2.49 * sqrt(2) * s = {margin:.6f}")

    if spread > SPREAD_UNREADABLE:
        print("S082 reading: outcome (c), s is above 1e-3; nothing is chosen")
        return 1

    scored = {(leaf, density): value[0]
              for (leaf, density, seed), value in rows.items()
              if (leaf, density) != CONTROL}
    best_cell = min(scored, key=scored.get)
    best = scored[best_cell]

    if control - best <= margin:
        print(f"S082 reading: outcome (b), the control stands -- the best "
              f"cell, {best_cell[0]} {best_cell[1]}, is {control - best:.6f} "
              f"below it, within m")
        return 0

    def density_rank(cell):
        # Every row is the densest; then fewer rows a game is cheaper per row.
        return 0 if cell[1] == 0 else 1.0 / cell[1]

    near = [cell for cell, value in scored.items() if value - best <= margin]
    chosen = sorted(near, key=lambda c: (density_rank(c),
                                         PREFERENCE[c[0]]))[0]
    print(f"S082 reading: outcome (a), leaf {chosen[0]}, "
          f"{chosen[1] if chosen[1] else 'every'} row(s) a game -- "
          f"L {scored[chosen]:.6f} against the control's {control:.6f}; "
          f"{len(near)} cell(s) within m of the best")
    return 0


def main(argv):
    if len(argv) == 2 and argv[0] == "--smoke":
        return smoke(argv[1])
    if len(argv) == 1 and not argv[0].startswith("-"):
        return reading(argv[0])
    print(__doc__, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
