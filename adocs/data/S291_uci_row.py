#!/usr/bin/env python3
"""S291. The multicut mined row through UCI, the engine as played: `uci`,
Hash 16, `ucinewgame`, `position fen`, `go depth N`; prints every iteration's
score (m = mate) and the bestmove. The engine's output only.

    ~/.venv/chess/bin/python adocs/data/S291_uci_row.py ENGINE DEPTH [FEN]
"""
import subprocess
import sys

ROW = "6k1/5pp1/6np/1PP1p3/1Q2P1P1/3r1q1P/1B1R4/6K1 b - - 0 42"
eng, depth = sys.argv[1], int(sys.argv[2])
fen = sys.argv[3] if len(sys.argv) > 3 else ROW
p = subprocess.Popen([eng], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                     text=True, bufsize=1)


def send(s):
    p.stdin.write(s + "\n")
    p.stdin.flush()


send("uci")
for line in p.stdout:
    if line.strip() == "uciok":
        break
send("setoption name Hash value 16")
send("ucinewgame")
send("position fen " + fen)
send("isready")
for line in p.stdout:
    if line.strip() == "readyok":
        break
send("go depth %d" % depth)
out = []
for line in p.stdout:
    if line.startswith("info") and " score " in line:
        t = line.split()
        kind = t[t.index("score") + 1]
        out.append("%s:%s%s" % (t[t.index("depth") + 1],
                                "m" if kind == "mate" else "",
                                t[t.index("score") + 2]))
    if line.startswith("bestmove"):
        out.append(line.strip())
        break
send("quit")
p.wait()
print(" ".join(out))
