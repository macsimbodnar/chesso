#!/usr/bin/env python3
"""Time the search, not perft.

perft measures generate + make + unmake. A search also evaluates, orders moves
and probes the transposition table, so a change can move one number and not the
other. This drives the UCI binary to a fixed depth on fixed positions and
reports wall time and knps.

    tools/search_bench.py ./build/src/chesso 9

The node count is printed next to the time on purpose. A change meant to be a
pure speed-up must leave it identical; if the node count moved, the search
changed behaviour and the times are not comparable.

Depth 9 takes about ten seconds per binary. Interleave the runs when comparing
two builds, and check the machine is idle first - see TOOLCHAIN.md.
"""

import subprocess, sys, time

POSITIONS = [
    ("midgame", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"),
    ("kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"),
    ("tactical", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"),
]

def run(engine, depth):
    p = subprocess.Popen([engine], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
    def send(s):
        p.stdin.write(s + "\n"); p.stdin.flush()
    send("uci")
    while "uciok" not in p.stdout.readline(): pass
    total, results = 0.0, []
    for name, fen in POSITIONS:
        send("position fen " + fen)
        send(f"go depth {depth}")
        t0, nodes, last = time.perf_counter(), 0, ""
        while True:
            line = p.stdout.readline()
            if not line: break
            if line.startswith("info") and " nodes " in line:
                last = line
                nodes = int(line.split(" nodes ")[1].split()[0])
            if line.startswith("bestmove"):
                results.append((name, time.perf_counter() - t0, nodes, line.split()[1]))
                total += results[-1][1]
                break
    send("quit"); p.wait(timeout=10)
    return total, results

if __name__ == "__main__":
    engine, depth = sys.argv[1], int(sys.argv[2])
    total, rows = run(engine, depth)
    for name, secs, nodes, best in rows:
        print(f"  {name:10s} {secs:7.3f}s {nodes:12d} nodes  {nodes/secs/1000:8.0f} knps  best {best}")
    print(f"  {'total':10s} {total:7.3f}s")
