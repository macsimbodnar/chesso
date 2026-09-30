#!/usr/bin/env python3
"""S114. What the null move reduction costs in nodes, per base and divisor, with
the static-score term alongside.

Node counts at a fixed depth over the 300 positions S021's sweep defined -- the
S021/S068 instrument. It is not Elo and it does not claim to be: a setting that
saves nodes is the candidate an SPRT is then spent on, and the SPRT decides
(DEC-019, DEC-063). One binary answers for every setting, the tune build, where
every search parameter is a UCI spin option (S073):

    cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
    cmake --build build-tune -j4
    adocs/data/S114_null_move_sweep.py build-tune/src/chesso 11 > sweep.tsv

THE POSITIONS are `adocs/data/S021_aspiration_sweep.py`'s, picked the same way:
4 per `game_phase()` value of `adocs/data/S018_raw.tsv` over the 25 values that
have at least 4 rows, 100 positions per offset, and offsets 0, 1 and 2 make the
300. The pick is restated here rather than imported because that script runs
its sweep at import.

THE GRID is the step file's section 4: `NullMoveBase` 1 to 4 and
`NullMoveDivisor` 3 to 8, every pair, with `NullMoveEvalMargin` 94 and
`NullMoveEvalCap` 8 -- the term at the seeds it ships at, since published
practice lands the family together and the base and divisor are re-decided in
its presence -- and `NullMoveEvalGate` 0, where S114's first verdict ships it
(DEC-243). Two reference rows come first and everything reads against the
first:

  off     cap 0, base 3, divisor 6 -- the tree before S114, node for node (the
          off value, proved on `bench` and `tools/search_bench.py` separately)
  seeds   cap 8, base 3, divisor 6 -- the candidate at its seeds

A row is `sample`, the five settings, total nodes, nodes relative to the off
row, and how many of the sample's best moves differ from the off row's.
`--gate 1` sweeps the same grid with the gate on, which is the second verdict's
tree; the off row keeps the gate at 0 either way.
"""

import argparse
import collections
import subprocess
import sys

RAW = "adocs/data/S018_raw.tsv"
PER_PHASE = 4
OFFSETS = (0, 1, 2)


def positions(offset):
    by_phase = collections.defaultdict(list)
    with open(RAW) as f:
        header = f.readline().rstrip("\n").split("\t")
        phase_at, fen_at = header.index("phase"), header.index("fen")
        for line in f:
            field = line.rstrip("\n").split("\t")
            by_phase[int(field[phase_at])].append(field[fen_at])

    picked = []
    for phase in sorted(by_phase):
        rows = by_phase[phase]
        if len(rows) < PER_PHASE:
            continue
        step = len(rows) // PER_PHASE
        picked += [rows[(i * step + offset) % len(rows)]
                   for i in range(PER_PHASE)]
    return picked


class Engine:
    def __init__(self, path):
        self.p = subprocess.Popen([path], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)
        self.send("uci")
        while "uciok" not in self.p.stdout.readline():
            pass

    def send(self, s):
        self.p.stdin.write(s + "\n")
        self.p.stdin.flush()

    def set(self, name, value):
        self.send(f"setoption name {name} value {value}")

    def go(self, fen, depth):
        """(nodes, best move) of one fixed-depth search, read until bestmove."""
        self.send("ucinewgame")
        self.send("position fen " + fen)
        self.send(f"go depth {depth}")
        nodes = 0
        while True:
            line = self.p.stdout.readline()
            if not line:
                sys.exit("engine died")
            if line.startswith("info") and " nodes " in line:
                t = line.split()
                nodes = int(t[t.index("nodes") + 1])
            if line.startswith("bestmove"):
                return nodes, line.split()[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("engine")
    parser.add_argument("depth", type=int)
    parser.add_argument("--gate", type=int, default=0, choices=(0, 1))
    args = parser.parse_args()

    # gate, cap, base, divisor. The two reference rows first.
    settings = [("off", 0, 0, 3, 6), ("seeds", args.gate, 8, 3, 6)]
    for base in (1, 2, 3, 4):
        for divisor in (3, 4, 5, 6, 7, 8):
            settings.append(("grid", args.gate, 8, base, divisor))

    engine = Engine(args.engine)
    engine.set("NullMoveEvalMargin", 94)
    print(f"# depth {args.depth}, engine {args.engine}, margin 94, "
          f"{len(OFFSETS)} samples of {len(positions(0))}", flush=True)
    print("sample\trow\tgate\tcap\tbase\tdivisor\tnodes\trel\tmoves_changed",
          flush=True)

    for offset in OFFSETS:
        fens = positions(offset)
        off_nodes, off_moves = None, None
        for row, gate, cap, base, divisor in settings:
            engine.set("NullMoveEvalGate", gate)
            engine.set("NullMoveEvalCap", cap)
            engine.set("NullMoveBase", base)
            engine.set("NullMoveDivisor", divisor)
            total, moves = 0, []
            for fen in fens:
                nodes, move = engine.go(fen, args.depth)
                total += nodes
                moves.append(move)
            if off_nodes is None:
                off_nodes, off_moves = total, moves
            changed = sum(1 for a, b in zip(moves, off_moves) if a != b)
            print(f"{offset}\t{row}\t{gate}\t{cap}\t{base}\t{divisor}\t{total}"
                  f"\t{total / off_nodes:.4f}\t{changed}", flush=True)

    engine.send("quit")
    print("SWEEP-DONE", flush=True)


if __name__ == "__main__":
    main()
