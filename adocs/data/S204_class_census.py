#!/usr/bin/env python3
"""Split the mate lines `test_mate_carry` sees into the two classes the guard
used to merge, over the whole node-budget grid.

S204, DEC-162. The C++ fixture reported one failure list holding both, so the
budgets had to be chosen where the list was empty -- which is what pinned each
case to a cell. The two classes are not the same kind of thing:

  class i   pv shorter than the distance claimed. DEC-122 leaves it short and
            visible on a failed walk, so it is an expected residue and S202
            owns closing it. It moves with the tree.
  class ii  pv at least as long as the distance claimed, but the position after
            those plies is not checkmate. DEC-122's invariant: the walk extends
            only when it reaches checkmate at exactly the claimed distance, so
            this is a published lie and must be zero at every budget.

Measured 2026-09-09 at c982f9d over the five guarded cases at stride 1, nine
budgets each: **428 mate lines, 39 class i, 0 class ii** -- recorded as
`adocs/data/S204_class_census_head.txt`. The mate counts reproduce
`adocs/data/S204_sweep_head.txt` cell for cell, which is what says the driver
agrees with `adocs/data/S203_case_sweep.sh`.

Run from the repository root with a built engine and python-chess:

  ~/.venv/chess/bin/python adocs/data/S204_class_census.py build/src/chesso

Prints one row per cell: mates, class i, class ii, and every class ii line in
full, because one of those is a bug and not a number.
"""
import subprocess, sys, chess

BUDGETS = [100000, 300000, 500000, 1000000, 1200000, 1500000, 2000000, 3000000, 4000000]


def plies_for(m):
    return 2 * m - 1 if m > 0 else -2 * m


class Engine:
    def __init__(self, path, hash_mb=16):
        self.p = subprocess.Popen([path], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)
        self.send("uci"); self.read_until("uciok")
        self.send(f"setoption name Hash value {hash_mb}")
        self.send("ucinewgame")

    def send(self, s):
        self.p.stdin.write(s + "\n"); self.p.stdin.flush()

    def read_until(self, tok):
        out = []
        while True:
            line = self.p.stdout.readline()
            if line == "":
                raise RuntimeError("engine closed its output")
            line = line.strip(); out.append(line)
            if line.startswith(tok):
                return out

    def go(self, spec):
        self.send("go " + spec)
        return self.read_until("bestmove")

    def close(self):
        self.send("quit"); self.p.wait(timeout=10)


def parse(line):
    t = line.split()
    if len(t) < 4 or t[0] != "info" or t[1] != "score" or t[2] != "mate":
        return None
    mate_in, depth, pv, i = int(t[3]), 0, [], 4
    while i < len(t):
        if t[i] == "depth" and i + 1 < len(t):
            depth = int(t[i + 1])
        if t[i] == "pv":
            pv = t[i + 1:]; break
        i += 1
    return mate_in, depth, pv


def load_cases(path):
    out = []
    for line in open(path):
        if line.startswith("#"):
            continue
        f = line.rstrip("\n").split("\t")
        if len(f) < 7 or f[0] == "name":
            continue
        out.append((f[0], f[1], f[2].split(), int(f[4]), int(f[5]), f[6] == "yes"))
    return out


def run(engine_path, name, fen, moves, budget, start, stride):
    eng = Engine(engine_path)
    mates = short = wrong = 0
    detail = []
    for i in range(start, len(moves) + 1, stride):
        pos = "position fen " + fen
        if i:
            pos += " moves " + " ".join(moves[:i])
        eng.send(pos)
        for line in eng.go(f"nodes {budget}"):
            got = parse(line)
            if got is None:
                continue
            mate_in, depth, pv = got
            mates += 1
            needed = plies_for(mate_in)
            if len(pv) < needed:
                short += 1
                continue
            board = chess.Board(fen)
            for mv in moves[:i]:
                board.push_uci(mv)
            bad = None
            for k, mv in enumerate(pv[:needed]):
                try:
                    board.push_uci(mv)
                except ValueError:
                    bad = f"illegal {mv} at pv ply {k}"
                    break
            if bad is None and not board.is_checkmate():
                bad = "no checkmate after %d plies" % needed
            if bad:
                wrong += 1
                detail.append(f"    ply {i} depth {depth} mate {mate_in}: {bad}")
    eng.close()
    return mates, short, wrong, detail


def main():
    engine = sys.argv[1] if len(sys.argv) > 1 else "build/src/chesso"
    cases = load_cases("adocs/data/S170_cases.tsv")
    print(f"{'case':26s} {'stride':>6s} {'nodes':>9s} {'mates':>6s} {'shortI':>7s} {'wrongII':>8s}")
    tot = [0, 0, 0]
    for name, fen, moves, start, stride, guard in cases:
        if not guard:
            continue
        for n in BUDGETS:
            m, s, w, d = run(engine, name, fen, moves, n, start, stride)
            tot[0] += m; tot[1] += s; tot[2] += w
            print(f"{name:26s} {stride:6d} {n:9d} {m:6d} {s:7d} {w:8d}", flush=True)
            for line in d:
                print(line, flush=True)
    print(f"TOTAL mates {tot[0]}  class i (short) {tot[1]}  class ii (full but not mate) {tot[2]}")


main()
