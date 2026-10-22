#!/usr/bin/env python3
"""S289. The ordinary-play census behind adocs/data/S289_census.txt: the root
answer of every FEN at `go depth N`, S210's census shape (adocs/data/
S210_f22_census.py's `go`, `ucinewgame` before each position, the default Hash)
dealt round-robin over twelve engine processes, each waiting for `bestmove`.

    ~/.venv/chess/bin/python adocs/data/S210_f22_census.py positions \
        adocs/data/S219_aa_calibration.pgn census.fens
    python3 adocs/data/S289_census.py <parent chesso> census.fens 10 parent.tsv
    python3 adocs/data/S289_census.py <fix chesso> census.fens 10 fix.tsv

OUT rows: fen, bestmove, the last info line's score, its nodes. The two tables
are compared row by row for the reading; they are scratch (an hour's work to
rebuild at most), the reading is what is kept. The engine's answers only.
"""
import subprocess, sys
from concurrent.futures import ThreadPoolExecutor
eng, fens, depth, out = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
opts = dict(o.split("=", 1) for o in sys.argv[5:])
rows = [l.strip() for l in open(fens) if l.strip() and not l.startswith("#")]
JOBS = 12
def run(chunk):
    p = subprocess.Popen([eng], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
    def send(s): p.stdin.write(s + "\n"); p.stdin.flush()
    send("uci")
    for l in p.stdout:
        if l.strip() == "uciok": break
    for n, v in opts.items(): send("setoption name %s value %s" % (n, v))
    res = []
    for i, fen in chunk:
        send("ucinewgame"); send("position fen " + fen); send("isready")
        for l in p.stdout:
            if l.strip() == "readyok": break
        send("go depth %d" % depth)
        score, nodes = "none", 0
        for l in p.stdout:
            if l.startswith("info") and " score " in l:
                t = l.split(); score = " ".join(t[t.index("score") + 1:t.index("score") + 3]); nodes = int(t[t.index("nodes") + 1])
            if l.startswith("bestmove"):
                res.append((i, fen, l.split()[1], score, nodes)); break
    send("quit"); p.wait()
    return res
chunks = [[] for _ in range(JOBS)]
for i, f in enumerate(rows): chunks[i % JOBS].append((i, f))
with ThreadPoolExecutor(JOBS) as ex:
    allres = sorted(sum(ex.map(run, chunks), []))
with open(out, "w") as h:
    for i, fen, bm, sc, n in allres: h.write("%s\t%s\t%s\t%d\n" % (fen, bm, sc, n))
print("CENSUS-DONE %s %d rows" % (out, len(allres)))
