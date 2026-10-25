#!/usr/bin/env python3
"""S083, DEC-313 (3). The first iteration at which the ply-2 case's root
reports the mate against itself, the golden tests/test_engine.cpp's "reverse
futility at ply 2 does not hide a mate from the side to move" asserts.

    python3 adocs/data/S083s_rfp_ply2.py build/src/chesso

Until S083 that clause was 4, the oracle's Mate(-2) at twice its distance, and
not a golden. At S083's constants the engine reports `mate -2` first at
iteration 6 and holds it to 8; the owner set the clause to what the engine
reads (DEC-313 (3)), which makes it a number read off the engine, so DEC-142
asks for the script that reads it: this one.

The root and the depth are the case's. The engine is driven over UCI by
adocs/data/S154_floor_margin_sweep.py's `engine_t`, whose `iterations()`
keeps the three fields tests/test_engine.cpp's `deepen_scores()` keeps and
parses them from the same line, in a fresh process at the defaults -- so
`first_exact` here is the case's `first_exact`. The engine's own report,
read back; nothing here asks a chess question (CLAUDE.md).
"""

import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))

ROOT = "rbrb2R1/p1p1p2p/P1P1P2P/8/8/6K1/8/7k b - - 3 2"
DEPTH = 8
DISTANCE = -2


def load_sweep():
    spec = importlib.util.spec_from_file_location(
        "S154_floor_margin_sweep",
        os.path.join(HERE, "S154_floor_margin_sweep.py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: S083s_rfp_ply2.py ENGINE")

    engine = load_sweep().engine_t(sys.argv[1], {})
    iterations, nodes = engine.iterations(ROOT, DEPTH)
    engine.close()

    first_exact = 0
    for depth, is_mate, value in iterations:
        print("  depth %2d  %s %d" % (depth, "mate" if is_mate else "cp", value))
        if is_mate and value == DISTANCE and first_exact == 0:
            first_exact = depth

    last = iterations[-1] if iterations else None
    held = last is not None and last[1] and last[2] == DISTANCE

    print("iterations %d, nodes %d" % (len(iterations), nodes))
    print("first_exact %d" % first_exact)
    print("mate %d at the last iteration: %s" % (DISTANCE, "yes" if held else "no"))

    return 0 if len(iterations) == DEPTH and first_exact and held else 1


if __name__ == "__main__":
    sys.exit(main())
