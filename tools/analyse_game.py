#!/usr/bin/env python3
"""Score every move of a game with a reference engine.

Reads the tab separated output of the SAN converter - ply, SAN, long algebraic,
FEN before the move - and asks the reference engine what each position is worth.
A move's cost is how much the evaluation fell because of it.

    tools/analyse_game.py positions.tsv --engine ~/.local/bin/stockfish --depth 18

Evaluations come back relative to the side to move, so for ply i the mover's
position afterwards is worth -eval[i+1]. The cost of the move is therefore
eval[i] + eval[i+1]: positive means the mover gave that much away.

Never judge a game by reading it. This is the tool for it.
"""

import argparse
import subprocess
import sys

MATE = 100000


class Engine:
    def __init__(self, path):
        self.p = subprocess.Popen(
            [path], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            text=True, bufsize=1)
        self._send("uci")
        while "uciok" not in self.p.stdout.readline():
            pass

    def _send(self, line):
        self.p.stdin.write(line + "\n")
        self.p.stdin.flush()

    def evaluate(self, fen, depth):
        """Score in centipawns from the side to move, plus the best move."""
        self._send("position fen " + fen)
        self._send(f"go depth {depth}")

        score, best = 0, "-"
        while True:
            line = self.p.stdout.readline()
            if not line:
                break
            if line.startswith("info") and " score " in line and " pv " in line:
                parts = line.split()
                kind = parts[parts.index("score") + 1]
                value = int(parts[parts.index("score") + 2])
                score = value if kind == "cp" else (
                    MATE - abs(value) if value > 0 else -(MATE - abs(value)))
            if line.startswith("bestmove"):
                best = line.split()[1]
                break
        return score, best

    def close(self):
        self._send("quit")
        self.p.wait(timeout=10)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("positions")
    ap.add_argument("--engine", default="stockfish")
    ap.add_argument("--depth", type=int, default=18)
    ap.add_argument("--threshold", type=int, default=100,
                    help="only report moves that cost at least this many cp")
    args = ap.parse_args()

    rows = []
    with open(args.positions) as handle:
        for line in handle:
            ply, san, lan, fen = line.rstrip("\n").split("\t")
            rows.append((int(ply), san, lan, fen))

    engine = Engine(args.engine)
    scores, bests = [], []

    for i, (ply, san, lan, fen) in enumerate(rows):
        score, best = engine.evaluate(fen, args.depth)
        scores.append(score)
        bests.append(best)
        print(f"\rscored {i + 1}/{len(rows)}", end="", file=sys.stderr)

    engine.close()
    print(file=sys.stderr)

    print(f"{'move':>6} {'played':>7} {'best':>7} {'before':>8} {'after':>8} {'cost':>7}")

    for i in range(len(rows) - 1):
        ply, san, lan, _ = rows[i]
        cost = scores[i] + scores[i + 1]

        if cost < args.threshold:
            continue

        number = ply // 2 + 1
        side = "." if ply % 2 == 0 else "..."
        print(f"{number:>4}{side:<2} {san:>7} {bests[i]:>7} "
              f"{scores[i]:>8} {-scores[i + 1]:>8} {cost:>7}")


if __name__ == "__main__":
    main()
