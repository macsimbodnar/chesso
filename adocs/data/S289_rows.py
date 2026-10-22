#!/usr/bin/env python3
"""S289. Per-row mate readings over a FEN list, through UCI, for the reach and
attribution tables in adocs/data/S289_reach.txt and S289_mined.txt.

    python3 adocs/data/S289_rows.py ENGINE SETFILE DEPTH OUT [Name=value ...]

SETFILE lines: fen<TAB>expected[<TAB>depth[<TAB>...]] -- expected is the signed
mate distance from the side to move (+n mates, -n is mated); a third column, when
present, is that row's `go depth`, else DEPTH. Twelve engine processes, rows dealt
round-robin, `ucinewgame` and `isready` before each row (S156's reading: the
answer is a property of the row, not of the order), Hash 16 unless given. OUT
gets one line a row: index, expected, the last info line's score kind and value,
the first depth reporting exactly `expected` (0 for none), bestmove, and every
iteration's score. The engine's own output only: nothing here asks a chess
question.
"""

import subprocess, sys
from concurrent.futures import ThreadPoolExecutor

eng, setfile, depth, out = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
opts = dict(o.split("=", 1) for o in sys.argv[5:])
opts.setdefault("Hash", "16")
rows = []
for line in open(setfile):
    if not line.strip() or line.startswith("#"):
        continue
    f = line.rstrip("\n").split("\t")
    rows.append((f[0], int(f[1]), int(f[2]) if len(f) > 2 and f[2] else depth))

JOBS = 12


def run(chunk):
    p = subprocess.Popen([eng], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                         text=True, bufsize=1)

    def send(s):
        p.stdin.write(s + "\n")
        p.stdin.flush()

    send("uci")
    for l in p.stdout:
        if l.strip() == "uciok":
            break
    for n, v in opts.items():
        send("setoption name %s value %s" % (n, v))
    res = []
    for idx, (fen, exp, rdepth) in chunk:
        send("ucinewgame")
        send("position fen " + fen)
        send("isready")
        for l in p.stdout:
            if l.strip() == "readyok":
                break
        send("go depth %d" % rdepth)
        per = []
        kind, val = None, 0
        for l in p.stdout:
            if l.startswith("info") and " score " in l:
                t = l.split()
                kind = t[t.index("score") + 1]
                val = int(t[t.index("score") + 2])
                d = int(t[t.index("depth") + 1])
                per.append((d, kind, val))
            if l.startswith("bestmove"):
                bm = l.split()[1]
                break
        first = 0
        for d, k, v in per:
            if k == "mate" and v == exp and first == 0:
                first = d
        s = " ".join("%d:%s%d" % (d, "m" if k == "mate" else "", v) for d, k, v in per)
        res.append((idx, exp, kind, val, first, bm, s))
    send("quit")
    p.wait()
    return res


chunks = [[] for _ in range(JOBS)]
for i, r in enumerate(rows):
    chunks[i % JOBS].append((i, r))
with ThreadPoolExecutor(JOBS) as ex:
    allres = sorted(sum(ex.map(run, chunks), []))
with open(out, "w") as h:
    for idx, exp, kind, val, first, bm, s in allres:
        h.write("%d\t%d\t%s\t%d\t%d\t%s\t%s\n" % (idx, exp, kind, val, first, bm, s))
exact = sum(1 for r in allres if r[2] == "mate" and r[3] == r[1])
print("%s: %d rows, %d exact at the last depth" % (out, len(allres), exact))
