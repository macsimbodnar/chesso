#!/usr/bin/env python3
"""S296. S292's ordinary-play census (adocs/data/S289_census.py's shape:
`ucinewgame` before each position, the default Hash, `go depth N`, twelve
processes, each waiting for `bestmove`) with the throwaway builds' firing
counters kept per position: adocs/data/S296_forms_patch.py's builds print
`info string S296 ret=.. bindn=.. bindm=..` before `bestmove` when
$S296_COUNT is set, and this driver sets it and records the line.

    python3 adocs/data/S296_census.py run <chesso> census.fens 10 OUT.tsv
    python3 adocs/data/S296_census.py read REF.tsv OTHER.tsv...

`run` rows: fen, bestmove, the last info line's score, its nodes, the
counters. `read` compares each table with the first: nodes, root best moves
and scores moved, the counters summed and the searches they touched, and
every moved root best move with both scores and whether the form's rule fired
in that search. The engine's answers only.
"""
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

JOBS = 12


def run(eng, fens, depth, out):
    rows = [l.strip() for l in open(fens) if l.strip() and not l.startswith("#")]
    env = dict(os.environ, S296_COUNT="1")

    def worker(chunk):
        p = subprocess.Popen([eng], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             text=True, bufsize=1, env=env)

        def send(s):
            p.stdin.write(s + "\n")
            p.stdin.flush()

        send("uci")
        for l in p.stdout:
            if l.strip() == "uciok":
                break
        res = []
        for i, fen in chunk:
            send("ucinewgame")
            send("position fen " + fen)
            send("isready")
            for l in p.stdout:
                if l.strip() == "readyok":
                    break
            send("go depth %d" % depth)
            score, nodes, counters = "none", 0, "-"
            for l in p.stdout:
                if l.startswith("info string S296 "):
                    counters = l.split(None, 3)[3].strip()
                elif l.startswith("info") and " score " in l:
                    t = l.split()
                    score = " ".join(t[t.index("score") + 1:t.index("score") + 3])
                    nodes = int(t[t.index("nodes") + 1])
                if l.startswith("bestmove"):
                    res.append((i, fen, l.split()[1], score, nodes, counters))
                    break
        send("quit")
        p.wait()
        return res

    chunks = [[] for _ in range(JOBS)]
    for i, fen in enumerate(rows):
        chunks[i % JOBS].append((i, fen))
    with ThreadPoolExecutor(JOBS) as ex:
        allres = sorted(sum(ex.map(worker, chunks), []))
    with open(out, "w") as h:
        h.write("# S296_census.py run %s %s %d\n" % (eng, fens, depth))
        for r in allres:
            h.write("%s\t%s\t%s\t%d\t%s\n" % r[1:])
    print("S296-CENSUS %s %d rows" % (out, len(allres)))


def read_table(path):
    return [l.rstrip("\n").split("\t") for l in open(path) if not l.startswith("#")]


def counters(field):
    return {k: int(v) for k, v in (kv.split("=") for kv in field.split())} \
        if field != "-" else {}


def read(ref_path, others):
    ref = read_table(ref_path)
    base = sum(int(x[3]) for x in ref)
    for path in [ref_path] + others:
        r = read_table(path)
        assert [x[0] for x in r] == [x[0] for x in ref], path
        nodes = sum(int(x[3]) for x in r)
        tot, touched = {}, {}
        for x in r:
            for k, v in counters(x[4]).items():
                tot[k] = tot.get(k, 0) + v
                touched[k] = touched.get(k, 0) + (v > 0)
        moved = [i for i, (x, y) in enumerate(zip(r, ref)) if x[1] != y[1]]
        print("%s: %d positions, nodes %d (x%.6f), root best moves moved %d, "
              "scores moved %d, node counts moved %d" % (
                  path, len(r), nodes, nodes / base, len(moved),
                  sum(1 for x, y in zip(r, ref) if x[2] != y[2]),
                  sum(1 for x, y in zip(r, ref) if x[3] != y[3])))
        print("  counters summed %s; searches touched %s" % (
            " ".join("%s=%d" % kv for kv in sorted(tot.items())),
            " ".join("%s=%d" % kv for kv in sorted(touched.items()))))
        if path == ref_path:
            continue
        untouched = [i for i, (x, y) in enumerate(zip(r, ref))
                     if (x[1], x[2], x[3]) != (y[1], y[2], y[3])
                     and counters(x[4]).get("ret", 0) == 0
                     and counters(x[4]).get("bindm", 0) == 0]
        print("  searches moved where neither counter fired in this build: %d"
              % len(untouched))
        deltas = []
        for i in moved:
            a, b = ref[i][2].split(), r[i][2].split()
            if a[0] == b[0] == "cp":
                deltas.append(abs(int(a[1]) - int(b[1])))
        deltas.sort()
        if deltas:
            q = lambda f: deltas[min(len(deltas) - 1, int(f * len(deltas)))]
            print("  moved best moves, |score change| in cp over %d: median "
                  "%d, p90 %d, max %d; mate scores among them %d" % (
                      len(deltas), q(0.5), q(0.9), deltas[-1],
                      len(moved) - len(deltas)))
        for i in moved:
            print("    %d\t%s\t%s %s -> %s %s\t%s" % (
                i, ref[i][0], ref[i][1], ref[i][2], r[i][1], r[i][2], r[i][4]))


if __name__ == "__main__":
    if sys.argv[1] == "run":
        run(sys.argv[2], sys.argv[3], int(sys.argv[4]), sys.argv[5])
    else:
        read(sys.argv[2], sys.argv[3:])
