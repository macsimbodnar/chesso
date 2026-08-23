#!/usr/bin/env python3
"""S108 reach: how often the change alters the tree, and under what pressure.

The candidate stores the static evaluation at every non-check main-search node
and stops overwriting a stored one with TT_EVAL_NONE. Quiescence then stands
pat on an exact score where it would have used a lazy bound, so the tree moves
-- but only where a main-search entry reaches the stand-pat site without
answering the node outright, which is thin.

    adocs/data/S108_node_reach.py REF_ENGINE CAND_ENGINE DEPTH POSITIONS [HASH]

Positions come from the book the SPRT regime plays, books/UHO_Lichess_4852_v1.epd
(DEC-088, S105). HASH matters more than the depth does and defaults to the 16 MB
that regime uses: replacement pressure is what makes the preserved evaluation
worth anything, so a run at the engine's default hash understates the reach by
about an order of magnitude. Measured 2026-08-23 against bbbd9f4:

    depth 11, 60 positions,  default hash    0 of 60 differ, 63680446 nodes both
    depth 12, 200 positions, Hash 16         3 of 200 differ, 0 best moves
                                             347297369 -> 346772409, -0.15 %
                                             -4.0 %, -24.5 % and -2 nodes

So a bench-sized sample at the default hash reads behaviour-neutral and is not:
INV-6 is unavailable here and the run in S108_sprt.sh is what decides the step.
"""
import subprocess, sys


def run(engine, fens, depth, hash_mb):
    p = subprocess.Popen([engine], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                         text=True, bufsize=1)

    def send(s):
        p.stdin.write(s + "\n")
        p.stdin.flush()

    send("uci")
    while "uciok" not in p.stdout.readline():
        pass
    send(f"setoption name Hash value {hash_mb}")
    out = []
    for fen in fens:
        send("position fen " + fen)
        send(f"go depth {depth}")
        nodes = 0
        while True:
            line = p.stdout.readline()
            if not line:
                break
            if line.startswith("info") and " nodes " in line:
                nodes = int(line.split(" nodes ")[1].split()[0])
            if line.startswith("bestmove"):
                out.append((nodes, line.split()[1]))
                break
        # A fresh table per position, so a difference is that position's and
        # not one carried in from the previous one.
        send("ucinewgame")
    send("quit")
    p.wait(timeout=60)
    return out


if __name__ == "__main__":
    ref, cand, depth, count = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
    hash_mb = int(sys.argv[5]) if len(sys.argv) > 5 else 16

    fens = []
    with open("books/UHO_Lichess_4852_v1.epd") as f:
        for line in f:
            line = line.strip()
            if line:
                fens.append(line.split(";")[0])
            if len(fens) >= count:
                break

    a, b = run(ref, fens, depth, hash_mb), run(cand, fens, depth, hash_mb)

    nodes_differ = best_differ = 0
    for i, ((na, ba), (nb, bb)) in enumerate(zip(a, b)):
        if na != nb:
            nodes_differ += 1
        if ba != bb:
            best_differ += 1
        if na != nb or ba != bb:
            pct = 100.0 * (nb - na) / na if na else 0.0
            print(f"  {i:4d} {na:10d} {ba:6s} -> {nb:10d} {bb:6s}  {pct:+.2f} %")

    total_a = sum(n for n, _ in a)
    total_b = sum(n for n, _ in b)
    print(f"{len(fens)} positions, depth {depth}, Hash {hash_mb}: "
          f"{nodes_differ} node-count differences, {best_differ} best-move differences")
    print(f"total nodes {total_a} -> {total_b}, "
          f"{100.0 * (total_b - total_a) / total_a:+.2f} %")
