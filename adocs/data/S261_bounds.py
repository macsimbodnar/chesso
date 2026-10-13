#!/usr/bin/env python3
"""S261: where each axis ended, and how long it sat at a bound.

    python3 adocs/data/S261_bounds.py <run dir> [--config tools/spsa_s261.json]

The reading item (4) of `adocs/plan_current/S261_search_block_spsa_lane.md`
pre-registers, taken at the mid-run reads and at the end. One line per axis:
the incumbent, the value the latest trajectory row holds (the driver writes the
rounded UCI value, which at the last row is its final vector), and the share of
iterations so far at the declared minimum and at the declared maximum. An axis
whose last value is a bound is flagged `END`; the step file says how each kind
of end is decided. Reads the trajectory only; stdlib only.
"""

import argparse
import csv
import json
import os
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("rundir")
    ap.add_argument("--config", default=str(ROOT / "tools" / "spsa_s261.json"))
    args = ap.parse_args()

    with open(args.config) as fh:
        params = json.load(fh)["params"]
    with open(os.path.join(args.rundir, "trajectory.tsv")) as fh:
        rows = list(csv.DictReader(fh, delimiter="\t"))
    if not rows:
        sys.exit(f"{args.rundir}/trajectory.tsv has no iteration yet")

    print(f"{len(rows)} iterations read")
    for p in params:
        name = p["name"]
        if name not in rows[0]:
            sys.exit(f"{name} is in the config and not in the trajectory")
        values = [int(r[name]) for r in rows]
        at_min = sum(v == p["min"] for v in values) / len(values)
        at_max = sum(v == p["max"] for v in values) / len(values)
        last = values[-1]
        flag = "END" if last in (p["min"], p["max"]) else ""
        moved = "" if last == p["start"] else "moved"
        print(f"{name:24s} {p['start']:>6} -> {last:>6}  "
              f"at min {100 * at_min:5.1f} %  at max {100 * at_max:5.1f} %  "
              f"{moved:5s} {flag}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
