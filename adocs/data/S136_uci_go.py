#!/usr/bin/env python3
"""S136 phase 2b: one `go depth` through UCI, every line it prints kept.

    python3 adocs/data/S136_uci_go.py ENGINE "FEN" DEPTH [Name=Value ...]

Hash 16 and `ucinewgame` as S136_mined_rows.py and S156's sweep use, any
Name=Value sent as setoption first; stdin stays open until `bestmove`, so the
search is never cut short by end of input (the trap TOOLCHAIN.md records for
Stockfish holds for chesso too). Prints the `info` lines and `bestmove`. The
environment passes through, so a build from S136_trace_patch.py logs its
nodes to $S295_DIAG for the root depth $S295_DEPTH (0 for every iteration).
"""

import subprocess
import sys


def main():
    if len(sys.argv) < 4:
        raise SystemExit(__doc__)
    engine, fen, depth = sys.argv[1], sys.argv[2], int(sys.argv[3])
    process = subprocess.Popen([engine], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        process.stdin.write(line + "\n")
        process.stdin.flush()

    send("uci")
    while "uciok" not in process.stdout.readline():
        pass
    send("setoption name Hash value 16")
    for opt in sys.argv[4:]:
        name, value = opt.split("=", 1)
        send("setoption name %s value %s" % (name, value))
    send("ucinewgame")
    send("position fen " + fen)
    send("go depth %d" % depth)
    while True:
        line = process.stdout.readline()
        if not line:
            break
        if line.startswith("info") or line.startswith("bestmove"):
            sys.stdout.write(line)
        if line.startswith("bestmove"):
            break
    send("quit")
    process.wait(timeout=30)
    return 0


if __name__ == "__main__":
    sys.exit(main())
