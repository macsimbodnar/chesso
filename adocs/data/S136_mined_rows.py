#!/usr/bin/env python3
"""S136. S145's mined set read one row at a time, for the churn behind a count.

    python3 adocs/data/S136_mined_rows.py ENGINE DEPTH [ROW,ROW,...]

`tests/test_mate_breadth.cpp` and `adocs/data/S156_mined_floor_sweep.py` give
one number, the exact mate distances over the 318 rows at depth 10. A net
change can hide rows swapping in and out (S154's churn argument), so this
prints one line a row: its index in the TSV's data rows (0-based, comment and
header lines skipped, as S156's `read_set` returns them), the labelled
distance, the score the engine printed last before `bestmove`, `exact` or
`-`, and the FEN. The protocol is S156's `score()`: one process, Hash 16,
`ucinewgame` before each position, `go depth DEPTH`. ROW limits the reading
to those indices.

Standard library only; the labels are the TSV's and nothing here asks a chess
question.
"""

import importlib.util
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)

    spec = importlib.util.spec_from_file_location(
        "s156", os.path.join(HERE, "S156_mined_floor_sweep.py"))
    s156 = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(s156)

    engine, depth = sys.argv[1], int(sys.argv[2])
    only = None
    if len(sys.argv) > 3:
        only = {int(x) for x in sys.argv[3].split(",")}

    process = subprocess.Popen([engine], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        process.stdin.write(line + "\n")
        process.stdin.flush()

    send("uci")
    while "uciok" not in process.stdout.readline():
        pass
    send("setoption name Hash value 16")

    for index, (fen, distance) in enumerate(s156.read_set()):
        if only is not None and index not in only:
            continue

        send("ucinewgame")
        send("position fen " + fen)
        send("go depth %d" % depth)

        kind, value = None, 0
        while True:
            line = process.stdout.readline()
            if not line:
                break
            if line.startswith("info") and " score " in line:
                token = line.split()
                kind = token[token.index("score") + 1]
                value = int(token[token.index("score") + 2])
            if line.startswith("bestmove"):
                break

        exact = kind == "mate" and value == distance
        print("%d\t%d\t%s %d\t%s\t%s"
              % (index, distance, kind, value, "exact" if exact else "-", fen))
        sys.stdout.flush()

    send("quit")
    process.wait(timeout=10)
    return 0


if __name__ == "__main__":
    sys.exit(main())
