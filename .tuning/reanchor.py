#!/usr/bin/env python3
"""Shift one piece type between its material value and its two psqt tables.

tools/tuner.cpp:26-30 documents the parameterisation as degenerate: adding a
constant to every square of psqt_mg[t] and psqt_eg[t] and subtracting it from
piece_value[t] is the same evaluation. DEC-059 spends that freedom to move
S065's queen residual off POSITIONAL_ROOM without touching the guard.

Applied to the tuner's emitted header, never to the source after the paste, so
verify_fit.py still compares the engine against one file.

Usage: reanchor.py <emitted.hpp> <out.hpp> <piece> <delta>
       delta is added to all 128 squares and subtracted from the define.
"""
import re
import sys

NAMES = ("pawn", "knight", "bishop", "rook", "queen", "king")
DEFINES = {
    "pawn": "PAWN",
    "knight": "KNIGHT",
    "bishop": "BISHOP",
    "rook": "ROOK",
    "queen": "QUEEN",
}


def render(values):
    """The emit format of tools/tuner.cpp:612-623, reproduced exactly."""
    out = []
    for rank in range(8):
        row = "   "
        for file in range(8):
            last = rank == 7 and file == 7
            row += " %4d%s" % (values[rank * 8 + file], "" if last else ",")
        out.append(row)
    return out


def shift_table(lines, table, piece, delta):
    """Rewrite one piece's 8 rows inside psqt_<table>, in place."""
    start = lines.index("static constexpr int psqt_%s[6][64] = {" % table)
    head = lines.index("  {  // %s" % piece, start)
    body = lines[head + 1 : head + 9]

    values = []
    for row in body:
        values.extend(int(n) for n in re.findall(r"-?\d+", row))
    if len(values) != 64:
        raise SystemExit("psqt_%s[%s]: read %d values" % (table, piece, len(values)))

    lines[head + 1 : head + 9] = render([v + delta for v in values])
    return values


def main():
    src, dst, piece, delta = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
    if piece not in DEFINES:
        raise SystemExit("piece must be one of %s" % ", ".join(DEFINES))

    lines = open(src).read().split("\n")

    name = DEFINES[piece]
    pattern = re.compile(r"^#define %s\s+(-?\d+)$" % name)
    hits = [i for i, line in enumerate(lines) if pattern.match(line)]
    if len(hits) != 1:
        raise SystemExit("#define %s: %d matches" % (name, len(hits)))
    before = int(pattern.match(lines[hits[0]]).group(1))
    after = before - delta
    lines[hits[0]] = "#define %s   %d" % (name, after)

    mg = shift_table(lines, "mg", piece, delta)
    eg = shift_table(lines, "eg", piece, delta)

    note = [
        "// Re-anchored after the fit, DEC-059: %+d on all 128 %s squares and" % (delta, piece),
        "// %d off #define %s, which tuner.cpp:26-30 documents as the same" % (delta, name),
        "// evaluation. Measured at one centipawn over the seven pinned anchors.",
        "",
    ]
    lines[0:0] = note

    open(dst, "w").write("\n".join(lines))

    print("#define %-7s %d -> %d" % (name, before, after))
    print("psqt_mg[%s] %d squares, mean %+.1f -> %+.1f"
          % (piece, len(mg), sum(mg) / 64.0, sum(mg) / 64.0 + delta))
    print("psqt_eg[%s] %d squares, mean %+.1f -> %+.1f"
          % (piece, len(eg), sum(eg) / 64.0, sum(eg) / 64.0 + delta))
    print("numbers changed: %d" % (1 + len(mg) + len(eg)))


main()
