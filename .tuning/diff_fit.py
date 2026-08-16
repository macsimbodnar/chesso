#!/usr/bin/env python3
"""Largest weight changes between what the engine ships at a ref and a fit.

Usage: diff_fit.py <emitted.hpp> [ref]
"""
import re
import subprocess
import sys

emitted_path = sys.argv[1]
ref = sys.argv[2] if len(sys.argv) > 2 else "HEAD"


def show(path):
    return subprocess.run(
        ["git", "show", "%s:%s" % (ref, path)],
        capture_output=True,
        text=True,
        cwd="/home/max/ws/chesso",
        check=True,
    ).stdout


def tables(text):
    out = {}
    for name in ("psqt_mg", "psqt_eg"):
        m = re.search(
            r"static constexpr int %s\[6\]\[64\] = \{(.*?)\n\};" % name, text, re.S
        )
        values = [int(v) for v in re.findall(r"-?\d+", re.sub(r"//[^\n]*", "", m.group(1)))]
        assert len(values) == 384, (name, len(values))
        out[name] = values
    return out


def flats(text):
    out = {}
    for name, body in re.findall(r"^const int (\w+)\[\w+\] = \{([^}]*)\};", text, re.M):
        out[name] = [int(v) for v in re.findall(r"-?\d+", re.sub(r"//[^\n]*", "", body))]
    for name, value in re.findall(r"^const int (\w+) = (-?\d+);", text, re.M):
        out[name] = [int(value)]
    return out


def defines(text):
    return dict(
        (n, int(v)) for n, v in re.findall(r"^#define (\w+)\s+(-?\d+)$", text, re.M)
    )


new_text = open(emitted_path).read()
old_tables_text = show("src/eval_tables.hpp")
old_eval_text = show("src/evaluation.cpp")

names = ["pawn", "knight", "bishop", "rook", "queen", "king"]
files = "abcdefgh"


def square_name(i):
    # index 0 is a8, index 63 is h1
    rank = 8 - i // 8
    return "%s%d" % (files[i % 8], rank)


old_t, new_t = tables(old_tables_text), tables(new_text)
old_d, new_d = defines(old_tables_text), defines(new_text)
old_f, new_f = flats(old_eval_text), flats(new_text)

print("=== material defines ===")
for k in ("PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN"):
    print("%-7s %6d -> %6d  (%+d)" % (k, old_d[k], new_d[k], new_d[k] - old_d[k]))

print("\n=== psqt: summary per table and piece ===")
for name in ("psqt_mg", "psqt_eg"):
    for t in range(6):
        o = old_t[name][t * 64 : t * 64 + 64]
        n = new_t[name][t * 64 : t * 64 + 64]
        deltas = [b - a for a, b in zip(o, n)]
        nz = [d for d in deltas if d != 0]
        mean = sum(deltas) / 64.0
        worst = max(range(64), key=lambda i: abs(deltas[i]))
        print(
            "%s %-7s mean %+7.1f  max |d| %4d at %-2s (%d -> %d)  moved %d/64"
            % (
                name,
                names[t],
                mean,
                abs(deltas[worst]),
                square_name(worst),
                o[worst],
                n[worst],
                len(nz),
            )
        )

print("\n=== psqt: 15 largest single-square moves ===")
rows = []
for name in ("psqt_mg", "psqt_eg"):
    for i in range(384):
        d = new_t[name][i] - old_t[name][i]
        rows.append((abs(d), name, names[i // 64], square_name(i % 64), old_t[name][i], new_t[name][i]))
rows.sort(reverse=True)
for a, name, piece, sq, o, n in rows[:15]:
    print("%-8s %-7s %-2s  %5d -> %5d  (%+d)" % (name, piece, sq, o, n, n - o))

print("\n=== the 54 outside eval_tables.hpp ===")
for name in sorted(new_f):
    o, n = old_f[name], new_f[name]
    print("%-20s %s" % (name, " ".join("%d->%d" % (a, b) for a, b in zip(o, n))))
