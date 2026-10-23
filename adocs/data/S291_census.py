#!/usr/bin/env python3
"""S291. The ordinary-play census behind adocs/data/S291_census.txt: S289's and
S292's census shape (adocs/data/S289_census.py: `ucinewgame` before each
position, the default Hash, `go depth N`, twelve processes dealt round-robin,
each waiting for `bestmove`), with one addition -- each engine process writes
its stderr to its own file, and the driver records, per position, how many
`S291 RFP-AGAINST-MATED` lines that search wrote (all of them, and those whose
entry came from a node that searched every legal move, `whole 1`). The engine
writes stderr unbuffered and before `bestmove`, so the count read when
`bestmove` arrives is that search's.

    ~/.venv/chess/bin/python adocs/data/S210_f22_census.py positions \\
        adocs/data/S219_aa_calibration.pgn census.fens
    S291_CENSUS=1 python3 adocs/data/S291_census.py <chesso> census.fens 10 off.tsv
    S291_CENSUS=1 S291_RFP_GUARD=1 python3 adocs/data/S291_census.py <chesso> census.fens 10 on.tsv
    python3 adocs/data/S291_census.py --compare off.tsv on.tsv

The engine is a scratch build of adocs/data/S291_trace.py (applied after
S289_trace.py to the shipped tree's src/search.cpp), whose $S291_CENSUS prints
the lines and whose $S291_RFP_GUARD is the scratch guard -- no reverse futility
where S292's null-move condition holds. **The guard is a scratch reading and is
never shipped**; neither is the census. With both unset the binary is the
tree's own (bench 6042933). OUT rows: index, fen, bestmove, last score, nodes,
firings, whole-search firings. The tables are scratch; the reading is kept.
The engine's answers only.
"""
import os
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

JOBS = 12


def count(path):
    with open(path) as handle:
        lines = [l for l in handle if "RFP-AGAINST-MATED" in l]
    return len(lines), sum(1 for l in lines if l.rstrip().endswith("whole 1"))


def census(eng, fens, depth, out):
    rows = [l.strip() for l in open(fens) if l.strip() and not l.startswith("#")]
    work = tempfile.mkdtemp(prefix="s291_census_")

    def run(job):
        chunk = [(i, f) for i, f in enumerate(rows) if i % JOBS == job]
        err_path = os.path.join(work, "err%d" % job)
        err = open(err_path, "w")
        p = subprocess.Popen([eng], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=err, text=True, bufsize=1)

        def send(s):
            p.stdin.write(s + "\n")
            p.stdin.flush()

        send("uci")
        for l in p.stdout:
            if l.strip() == "uciok":
                break
        res = []
        before = (0, 0)
        for i, fen in chunk:
            send("ucinewgame")
            send("position fen " + fen)
            send("isready")
            for l in p.stdout:
                if l.strip() == "readyok":
                    break
            send("go depth %d" % depth)
            score, nodes = "none", 0
            for l in p.stdout:
                if l.startswith("info") and " score " in l:
                    t = l.split()
                    score = " ".join(t[t.index("score") + 1:t.index("score") + 3])
                    nodes = int(t[t.index("nodes") + 1])
                if l.startswith("bestmove"):
                    bm = l.split()[1]
                    break
            now = count(err_path)
            res.append((i, fen, bm, score, nodes, now[0] - before[0],
                        now[1] - before[1]))
            before = now
        send("quit")
        p.wait()
        err.close()
        return res

    with ThreadPoolExecutor(JOBS) as ex:
        allres = sorted(sum(ex.map(run, range(JOBS)), []))
    with open(out, "w") as h:
        for r in allres:
            h.write("%d\t%s\t%s\t%s\t%d\t%d\t%d\n" % r)
    print("%s: %d positions, %d nodes, %d firings, %d whole-search" % (
        out, len(allres), sum(r[4] for r in allres), sum(r[5] for r in allres),
        sum(r[6] for r in allres)))


def load(path):
    rows = []
    for line in open(path):
        f = line.rstrip("\n").split("\t")
        rows.append((int(f[0]), f[1], f[2], f[3], int(f[4]), int(f[5]), int(f[6])))
    return rows


def compare(a_path, b_path):
    a, b = load(a_path), load(b_path)
    assert [r[1] for r in a] == [r[1] for r in b], "different position lists"
    n = len(a)
    nodes_a, nodes_b = sum(r[4] for r in a), sum(r[4] for r in b)
    fire, whole = sum(r[5] for r in a), sum(r[6] for r in a)
    print("positions %d" % n)
    print("  nodes                 %d -> %d, x%.6f" % (nodes_a, nodes_b, nodes_b / nodes_a))
    print("  firings (all)         %d, %.3f per million nodes" % (fire, 1e6 * fire / nodes_a))
    print("  firings (whole)       %d, %.3f per million nodes, in %d positions" % (
        whole, 1e6 * whole / nodes_a, sum(1 for r in a if r[6])))
    moved_bm = [i for i in range(n) if a[i][2] != b[i][2]]
    moved_sc = [i for i in range(n) if a[i][3] != b[i][3]]
    moved_nd = [i for i in range(n) if a[i][4] != b[i][4]]
    mate_a = [i for i in range(n) if a[i][3].startswith("mate") != b[i][3].startswith("mate")]
    print("  root best move moved  %d" % len(moved_bm))
    print("  root score moved      %d" % len(moved_sc))
    print("  node count moved      %d (%.3f %%)" % (len(moved_nd), 100.0 * len(moved_nd) / n))
    print("  a reported mate appears or disappears  %d" % len(mate_a))
    for i in sorted(set(moved_bm) | set(moved_sc) | set(mate_a)):
        print("    %5d  %s %-10s -> %s %-10s  whole firings %d  %s" % (
            i, a[i][2], a[i][3], b[i][2], b[i][3], a[i][6], a[i][1]))


if __name__ == "__main__":
    if sys.argv[1] == "--compare":
        compare(sys.argv[2], sys.argv[3])
    else:
        census(sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4])
