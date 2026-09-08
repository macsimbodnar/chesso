#!/usr/bin/env python3
"""S148. The reverse-futility ceiling over the whole set, on the finer grid.

    ~/.venv/chess/bin/python adocs/data/S148_rfp_ceiling_sweep.py --mined

WHY THIS EXISTS AND WHAT IT ADDS TO S145 AND S154.

S145 swept `RfpMaxDepth` over [0, 3, 6, 10, 15, 63] against the 48 rows the
constructed set held then; S154 re-took the same six values at `fc5526e`. The
set is 82 rows since S168 and the ceiling has never been swept over them, and
neither sweep resolves the elbow: the deep classes empty out somewhere between
3 and 10 and the coarse grid cannot say where. S148 decides the shipping
default, so it needs the value the rule names --

    C1 = the largest ceiling at which both the mate in four and the mate in
         five exact counts over the 82 rows are non-zero

-- which is a statement about every integer from 0 to 15, not about six of
them. This is that table.

Nothing here re-implements the measurement. `adocs/data/` is append-only
(README.md), so S145's script is imported and called, not edited and not
copied: the axis, the value list, the held options and the rows are `sweep()`'s
arguments and that is the whole difference between this run and S145's. The
floor is read from the binary rather than written down, so the run states the
value it held rather than assuming the shipping one.

63 is not on the grid. It is the ceiling's declared maximum and S145 and S154
both read it as identical to 15 on every count; the question here is where the
curve bends, which is below 15.

Run against the tune build, which is the only one with the options.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S145_rfp_sweep as s145
import S145_mate_set as constructed
import S145_mined_set as mined

CEILING_VALUES = list(range(0, 16))


def main():
    engine_path = s145.TUNE_ENGINE

    declared = s145.read_defaults(engine_path)
    print("shipping defaults read from the binary: %s"
          % {name: spec[0] for name, spec in declared.items()})
    print("declared ranges: %s"
          % {name: [spec[1], spec[2]] for name, spec in declared.items()})

    rows = constructed.read_tsv()
    mined_rows = mined.read_tsv() if "--mined" in sys.argv else []

    s145.sweep(engine_path, "RfpMaxDepth",
               s145.admissible(CEILING_VALUES, declared["RfpMaxDepth"],
                               "RfpMaxDepth"),
               {"RfpMinPly": declared["RfpMinPly"][0]},
               rows, mined_rows, 10)

    return 0


if __name__ == "__main__":
    sys.exit(main())
