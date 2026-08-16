#!/usr/bin/env python3
"""Check that every one of the 827 fitted numbers reached the engine's sources.

Re-extracts the weights from src/eval_tables.hpp and src/evaluation.cpp after the
paste and compares them, one by one, against the emitted header. A half-applied
fit looks like a fit that did not work, and a weight still holding the value the
engine shipped is indistinguishable from a term the fit had nothing to say about.

Usage: verify_fit.py <emitted.hpp>
"""
import re
import sys

ROOT = "/home/max/ws/chesso/"

WIDTHS = [
    ("PAWN", 1),
    ("KNIGHT", 1),
    ("BISHOP", 1),
    ("ROOK", 1),
    ("QUEEN", 1),
    ("psqt_mg", 384),
    ("psqt_eg", 384),
    ("mobility_mg", 4),
    ("mobility_eg", 4),
    ("king_safety_mg", 9),
    ("king_safety_eg", 9),
    ("passed_pawn_mg", 6),
    ("passed_pawn_eg", 6),
    ("pawn_structure_mg", 3),
    ("pawn_structure_eg", 3),
    ("piece_placement_mg", 4),
    ("piece_placement_eg", 4),
    ("tempo_mg", 1),
    ("tempo_eg", 1),
]


def extract(text):
    def strip(s):
        return re.sub(r"//[^\n]*", "", s)

    out = {}
    for name, value in re.findall(r"^#define (\w+)\s+(-?\d+)$", text, re.M):
        out[name] = [int(value)]
    for name in ("psqt_mg", "psqt_eg"):
        m = re.search(
            r"static constexpr int %s\[6\]\[64\] = \{(.*?)\n\};" % name, text, re.S
        )
        if m:
            out[name] = [int(v) for v in re.findall(r"-?\d+", strip(m.group(1)))]
    for name, body in re.findall(r"^const int (\w+)\[\w+\] = \{([^}]*)\};", text, re.M):
        out[name] = [int(v) for v in re.findall(r"-?\d+", strip(body))]
    for name, value in re.findall(r"^const int (\w+) = (-?\d+);", text, re.M):
        out[name] = [int(value)]
    return out


emitted = extract(open(sys.argv[1]).read())
shipped = extract(
    open(ROOT + "src/eval_tables.hpp").read() + open(ROOT + "src/evaluation.cpp").read()
)

total = 0
same = 0
bad = []
for name, width in WIDTHS:
    if name not in emitted:
        bad.append("%s: missing from the emitted header" % name)
        continue
    if name not in shipped:
        bad.append("%s: missing from the engine sources" % name)
        continue
    a, b = emitted[name], shipped[name]
    if len(a) != width or len(b) != width:
        bad.append("%s: width %d emitted, %d shipped, %d expected" % (name, len(a), len(b), width))
        continue
    total += width
    same += sum(1 for x, y in zip(a, b) if x == y)
    if a != b:
        bad.append(
            "%s: %d of %d values differ from the emitted header"
            % (name, sum(1 for x, y in zip(a, b) if x != y), width)
        )

print("%d parameters compared, %d identical to the emitted header" % (total, same))
for line in bad:
    print("  MISMATCH " + line)
sys.exit(1 if (bad or total != 827) else 0)
