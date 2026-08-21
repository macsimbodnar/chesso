#!/usr/bin/env python3
"""S145. Measure the reverse futility floor and ceiling against the mate sets.

    ~/.venv/chess/bin/python adocs/data/S145_rfp_sweep.py floor
    ~/.venv/chess/bin/python adocs/data/S145_rfp_sweep.py ceiling
    ~/.venv/chess/bin/python adocs/data/S145_rfp_sweep.py both --mined

WHAT IS MEASURED, AND WHY EACH SWEEP HOLDS THE OTHER.

`RFP_MIN_PLY` and `RFP_MAX_DEPTH` are one guard read two ways, and S145 measured
that they substitute for each other: on the S033 position at `RfpMinPly` 1 the
mate appears at iteration 8 when `RfpMaxDepth` is 6 and never at all when it is
15. A sweep that moves both at once therefore cannot say which of them is
carrying the guard, and a sweep of one at the shipped value of the other is the
only reading that attributes anything. So the floor sweep holds the ceiling at
its shipping value and the ceiling sweep holds the floor at its shipping value,
and both are stated in the output rather than assumed by the reader.

THE NUMBERS PER SETTING, AND THEY ARE REPORTED PER MATE DISTANCE.

  exact     the final iteration reports the distance that was proved
  never     the proved distance is not reported at any iteration
  delay     iterations between the first depth that can contain the mate,
            2m - 1, and the first iteration that reports it
  short     a mate score CLOSER than the proved minimum, at any iteration.
            The proof says no shorter mate exists, so this is the one column
            that is a defect at any count rather than a strength reading
  sign      a mate score for the side that is being mated. Also a defect

`delay` is the reading a fixed-depth call cannot produce, and it is the one that
separates the two failure modes the old three-position gate conflated. A guard
that has been removed does not usually turn a mate into a non-mate at a generous
depth; it postpones it. A search called at exactly 2m - 1 sees the postponement
as a missing mate and a search called deep enough sees nothing at all, so only
the iteration the mate arrives at distinguishes "lost" from "late".

Per distance because the answer turned out to be almost entirely a function of
it. At the shipping defaults this engine reports every mate in two with no delay
at all, half the mates in three and late, and **none** of the mates in four or
five -- which is exactly the shape that made the old three-position gate look
green: all three of its cases were mates in two.

Run against the tune build, which is the only one with the options.
"""

import argparse
import os
import sys

import chess
import chess.engine

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S145_mate_set as constructed
import S145_mined_set as mined


REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TUNE_ENGINE = os.path.join(REPO, "build-tune", "src", "chesso")

FLOOR_VALUES = [0, 1, 2, 3, 4, 5]
CEILING_VALUES = [0, 3, 6, 10, 15, 63]

# Above the minimum, which is the point: at exactly 2m - 1 a postponed mate and
# a lost mate look the same. Eight, and the number is measured rather than
# chosen: one position in the set is delayed four iterations at the shipping
# defaults, so four would have been the boundary and a delay of five would have
# read as a lost mate. A whole search of these positions at depth 15 costs about
# 17 ms, so there is no reason to be tight.
EXTRA_DEPTH = 8


def read_defaults(engine_path):
    out = {}

    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        for name, option in engine.options.items():
            if name.startswith("Rfp"):
                out[name] = (option.default, option.min, option.max)

    return out


def admissible(values, bounds, axis):
    """The requested settings the binary will actually accept.

    S142 raised `RfpMinPly`'s declared minimum from 0 to 2 on this step's own
    evidence, so FLOOR_VALUES asks for two settings the engine now refuses.
    python-chess raises rather than sending them, which would end the sweep in a
    traceback halfway through a table -- and the values that remain are still the
    whole question, since the point of the floor is that below it the mate suite
    is red. So they are dropped and named, never silently skipped: a row missing
    from the table has to say why it is missing.
    """
    low, high = bounds[1], bounds[2]
    keep = [v for v in values if low <= v <= high]
    dropped = [v for v in values if v not in keep]

    if dropped:
        print("  %s: %s refused by the binary, declared range [%d, %d] since "
              "S142; rebuild with the bound relaxed to sweep them"
              % (axis, ", ".join(str(v) for v in dropped), low, high))

    return keep


def run_constructed(engine_path, options, rows):
    """One pass over the constructed set, bucketed by mate distance.

    A fresh engine process per setting, not `configure` on a live one:
    python-chess sends `setoption` for what it is given and leaves everything
    else as it was, so a loop that reconfigures one option at a time is
    measuring the union of every setting it has been through. That mistake
    produced a table where turning null move pruning off appeared to fix a
    missed mate, and it was reverse futility from the previous row.
    """
    per = {}

    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        engine.configure(dict({"Hash": 16}, **options))

        for row in rows:
            board = chess.Board(row["fen"])
            distance = row["distance"]
            minimum = 2 * distance - 1
            bucket = per.setdefault(distance, {"n": 0, "exact": 0, "delays": [],
                                               "short": 0, "sign": 0})
            bucket["n"] += 1

            first = None
            last = None

            with engine.analysis(
                    board,
                    chess.engine.Limit(depth=minimum + EXTRA_DEPTH)) as analysis:
                for info in analysis:
                    if "score" not in info or "depth" not in info:
                        continue

                    score = info["score"].relative
                    last = score

                    if not score.is_mate() or score.mate() is None:
                        continue

                    if score.mate() < 0:
                        bucket["sign"] += 1
                    elif score.mate() < distance:
                        bucket["short"] += 1
                    elif score.mate() == distance and first is None:
                        first = info["depth"]

            if last is not None and last.is_mate() and last.mate() == distance:
                bucket["exact"] += 1

            if first is not None:
                bucket["delays"].append(first - minimum)

    return per


def report(label, per):
    total = sum(b["n"] for b in per.values())
    exact = sum(b["exact"] for b in per.values())
    short = sum(b["short"] for b in per.values())
    sign = sum(b["sign"] for b in per.values())

    parts = []
    for distance in sorted(per):
        bucket = per[distance]
        delays = bucket["delays"]
        parts.append("m%d %d/%d d%s" % (distance, bucket["exact"], bucket["n"],
                                        max(delays) if delays else "-"))

    print("  %-14s exact %2d/%2d  %s  short %d  sign %d"
          % (label, exact, total, "  ".join(parts), short, sign))


def sweep(engine_path, axis, values, held, rows, mined_rows, mined_depth):
    print("\n%s swept, %s, %d constructed positions, depth 2m-1+%d"
          % (axis, ", ".join("%s held at %d" % kv for kv in held.items()),
             len(rows), EXTRA_DEPTH))

    for value in values:
        options = dict(held)
        options[axis] = value
        report("%s=%d" % (axis, value),
               run_constructed(engine_path, options, rows))


        if mined_rows:
            any_found = exact_found = 0

            with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
                engine.configure(dict({"Hash": 16}, **options))

                for row in mined_rows:
                    score = engine.analyse(
                        chess.Board(row["fen"]),
                        chess.engine.Limit(depth=mined_depth))["score"].relative
                    if score.is_mate() and (score.mate() or 0) > 0:
                        any_found += 1
                        if score.mate() == row["distance"]:
                            exact_found += 1

            print("  %-14s mined: %d of %d found, %d exact"
                  % ("", any_found, len(mined_rows), exact_found))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("which", choices=["floor", "ceiling", "both"])
    parser.add_argument("--engine", default=TUNE_ENGINE)
    parser.add_argument("--mined", action="store_true",
                        help="also count the mined breadth set at each setting")
    parser.add_argument("--mined-depth", type=int, default=10)
    args = parser.parse_args()

    declared = read_defaults(args.engine)
    print("shipping defaults read from the binary: %s"
          % {name: spec[0] for name, spec in declared.items()})
    print("declared ranges: %s"
          % {name: [spec[1], spec[2]] for name, spec in declared.items()})

    rows = constructed.read_tsv()
    mined_rows = mined.read_tsv() if args.mined else []

    if args.which in ("floor", "both"):
        sweep(args.engine, "RfpMinPly",
              admissible(FLOOR_VALUES, declared["RfpMinPly"], "RfpMinPly"),
              {"RfpMaxDepth": declared["RfpMaxDepth"][0]},
              rows, mined_rows, args.mined_depth)

    if args.which in ("ceiling", "both"):
        sweep(args.engine, "RfpMaxDepth",
              admissible(CEILING_VALUES, declared["RfpMaxDepth"], "RfpMaxDepth"),
              {"RfpMinPly": declared["RfpMinPly"][0]},
              rows, mined_rows, args.mined_depth)

    return 0


if __name__ == "__main__":
    sys.exit(main())
