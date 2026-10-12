#!/usr/bin/env python3
"""S275. How far a fit moved along each exact null direction of the evaluation.

tools/feature_audit's identity report finds the 823-parameter design rank 784:
32 pawn columns no row can touch (ranks 1 and 8), five material-against-table
directions -- piece_value[t] + c with every psqt_mg[t] and psqt_eg[t] square a
pawn can stand on - c -- and one direction per king table, + c on all 64 squares
(one king a side, so each king table's signed occupancy sums to zero). No
evaluation can see a move along any of them, so a fit's position along them is
wherever its optimiser left it.

Reads two weight sets in the tuner's emitted format -- the shipped
src/eval_tables.hpp is in the same format -- and prints, per direction, the
level shift: the mean change over the squares the direction moves. That is the
coordinate tools/tuner_groups.hpp's hold_gauges() keeps at its start since S275,
so for a fit taken before S275 it is exactly how far the pinned fit's vector
sits from it along that direction; material carries the rest. The 32 zero
columns' largest change is printed too, each table's shape change -- the root
mean square of the per-square change about its mean -- for scale, and the
largest single square's distance from the level shift: under 1 means the two
tables differ by that shift and integer rounding, nothing else. Between two
emitted headers the term weights after the tables are compared as well.

Usage: S275_drift.py <before.hpp> <after.hpp>
       S275_drift.py <after.hpp>      before is the shipped src/eval_tables.hpp
"""
import math
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
NAMES = ("pawn", "knight", "bishop", "rook", "queen", "king")
DEFINES = ("PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN")


def read_weights(path):
    """The five piece defines and both 6x64 tables, index 0 = a8."""
    with open(path) as handle:
        lines = handle.read().splitlines()

    material = {}
    for line in lines:
        match = re.match(r"#define\s+(PAWN|KNIGHT|BISHOP|ROOK|QUEEN)\s+(-?\d+)", line)
        if match:
            material[match.group(1)] = int(match.group(2))
    if len(material) != 5:
        raise SystemExit("%s: read %d piece defines, not 5" % (path, len(material)))

    tables = {}
    for table in ("mg", "eg"):
        start = lines.index("static constexpr int psqt_%s[6][64] = {" % table)
        values = []
        for line in lines[start + 1 :]:
            if line.startswith("};"):
                break
            values.extend(int(n) for n in re.findall(r"-?\d+", line.split("//")[0]))
        if len(values) != 384:
            raise SystemExit("%s: psqt_%s holds %d values, not 384" % (path, table, len(values)))
        tables[table] = values

    return [material[d] for d in DEFINES], tables


def term_weights(path):
    """Every integer after the tables in an emitted header, comments stripped;
    None for a file that is not one (the shipped eval_tables.hpp)."""
    with open(path) as handle:
        text = handle.read()
    marker = "These eight live in src/evaluation.cpp"
    if marker not in text:
        return None
    values = []
    for line in text[text.index(marker) :].splitlines()[1:]:
        code = line.split("//")[0]
        code = re.sub(r"\[[A-Z_0-9]*\]", "", code)
        values.extend(int(n) for n in re.findall(r"-?\d+", code))
    return values


def back_rank(square):
    """A pawn cannot stand here: rank 8 is 0..7, rank 1 is 56..63."""
    return square < 8 or square >= 56


def main():
    if len(sys.argv) == 2:
        before_path = os.path.join(REPO, "src", "eval_tables.hpp")
        after_path = sys.argv[1]
    elif len(sys.argv) == 3:
        before_path, after_path = sys.argv[1], sys.argv[2]
    else:
        raise SystemExit(__doc__)

    material0, tables0 = read_weights(before_path)
    material1, tables1 = read_weights(after_path)

    def delta(table, piece, square):
        index = piece * 64 + square
        return tables1[table][index] - tables0[table][index]

    print("before %s" % before_path)
    print("after  %s" % after_path)
    print(
        "%-26s %8s %8s %12s %10s %10s"
        % ("direction", "columns", "material", "level shift", "shape rms", "worst off")
    )

    for piece in range(6):
        if piece == 0:
            squares = [s for s in range(64) if not back_rank(s)]
        else:
            squares = list(range(64))

        if piece < 5:
            changes = [delta(t, piece, s) for t in ("mg", "eg") for s in squares]
            mean = sum(changes) / len(changes)
            rms = math.sqrt(sum((c - mean) ** 2 for c in changes) / len(changes))
            worst = max(abs(c - mean) for c in changes)
            print(
                "%-26s %8d %+8d %+12.3f %10.3f %10.3f"
                % (
                    "material/table %s" % NAMES[piece],
                    len(changes),
                    material1[piece] - material0[piece],
                    mean,
                    rms,
                    worst,
                )
            )
        else:
            for table in ("mg", "eg"):
                changes = [delta(table, piece, s) for s in squares]
                mean = sum(changes) / len(changes)
                rms = math.sqrt(sum((c - mean) ** 2 for c in changes) / len(changes))
                worst = max(abs(c - mean) for c in changes)
                print(
                    "%-26s %8d %8s %+12.3f %10.3f %10.3f"
                    % ("psqt_%s[king] sum" % table, len(changes), "-", mean, rms, worst)
                )

    zero = [abs(delta(t, 0, s)) for t in ("mg", "eg") for s in range(64) if back_rank(s)]
    print("%-26s %8d %8s %12s %10s %10s   largest |change| %d" % ("pawn ranks 1 and 8", len(zero), "-", "-", "-", "-", max(zero)))

    terms0 = term_weights(before_path)
    terms1 = term_weights(after_path)
    if terms0 is not None and terms1 is not None:
        if len(terms0) != len(terms1):
            raise SystemExit("term weights: %d against %d values" % (len(terms0), len(terms1)))
        differ = sum(1 for a, b in zip(terms0, terms1) if a != b)
        print("%-26s %8d   %d differ" % ("term weights after tables", len(terms0), differ))


if __name__ == "__main__":
    main()
