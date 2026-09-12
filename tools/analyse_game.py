#!/usr/bin/env python3
"""Score every move of a game with a reference engine.

Reads the tab separated output of the SAN converter - ply, SAN, long algebraic,
FEN before the move - and asks the reference engine what each position is worth.
A move's cost is how much the evaluation fell because of it.

    tools/analyse_game.py positions.tsv --engine ~/.local/bin/stockfish --depth 18

Evaluations come back relative to the side to move, so for ply i the mover's
position afterwards is worth -eval[i+1]. The cost of the move is therefore
eval[i] + eval[i+1]: positive means the mover gave that much away.

It refuses rather than defaulting: a position the engine answered without ever
stating a score exits non-zero and names the position, and a `lowerbound` or
`upperbound` line is a window edge and not a value (S214, F28).

Never judge a game by reading it. This is the tool for it.
"""

import argparse
import subprocess
import sys

MATE = 100000


class EngineError(Exception):
    """The engine did not answer what was asked. Never a score.

    2026-09-10_adversarial-F28. Every path out of `evaluate()` that is not a
    score the engine stated raises this instead of returning a default, and the
    caller exits non-zero naming the position. The version before S214
    initialised `score, best = 0, "-"` and returned them, so an engine that
    answered `bestmove` and nothing else produced 0.00 -- a dead draw -- for
    every position of the game, and the table of costs that came out of it
    looked exactly like a table of costs. This is the tool `CLAUDE.md` mandates
    *because* agent chess judgement is banned, so a silent 0.00 is the DEC-023
    failure arriving through the instrument that exists to prevent it.
    """


class Engine:
    def __init__(self, path):
        self.path = path
        self.p = subprocess.Popen(
            [path], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            text=True, bufsize=1)
        self._send("uci")
        while True:
            line = self.p.stdout.readline()
            # A dead engine's stdout reads as "" forever, so the loop this
            # replaced spun on EOF instead of reporting it.
            if not line:
                raise EngineError(
                    f"{path} closed its output before answering `uciok`")
            if "uciok" in line:
                break

    def _send(self, line):
        self.p.stdin.write(line + "\n")
        self.p.stdin.flush()

    def evaluate(self, fen, depth):
        """Score in centipawns from the side to move, plus the best move.

        An `info` line carrying both a score and a `pv` is the only thing a
        score is read from, and a `lowerbound`/`upperbound` line is not one of
        those: the search proved "at most alpha" or "at least beta" and nothing
        more, so the number beside it is a window edge and not a value. Raises
        when no such line arrived before `bestmove`.
        """
        self._send("position fen " + fen)
        self._send(f"go depth {depth}")

        score, best = None, None
        while True:
            line = self.p.stdout.readline()
            if not line:
                raise EngineError(
                    f"{self.path} closed its output before `bestmove` on {fen}")
            if line.startswith("info") and " score " in line and " pv " in line:
                parts = line.split()
                at = parts.index("score")
                kind = parts[at + 1]
                # UCI puts the qualifier directly after the value: `score cp 23
                # lowerbound`. Anything else there is the next field.
                bound = parts[at + 3] if len(parts) > at + 3 else ""
                if kind not in ("cp", "mate"):
                    continue
                if bound in ("lowerbound", "upperbound"):
                    continue
                value = int(parts[at + 2])
                score = value if kind == "cp" else (
                    MATE - abs(value) if value > 0 else -(MATE - abs(value)))
            if line.startswith("bestmove"):
                parts = line.split()
                if len(parts) < 2:
                    raise EngineError(
                        f"{self.path} answered a bare `bestmove` on {fen}")
                best = parts[1]
                break

        if score is None:
            raise EngineError(
                f"{self.path} answered `bestmove {best}` on {fen} without one "
                "`info` line carrying both a score and a pv; there is no "
                "evaluation of this position to report")
        return score, best

    def close(self):
        self._send("quit")
        self.p.wait(timeout=10)

    def kill(self):
        """The error path. The engine may be mid-search and not reading its
        stdin, so `quit` is not a line it would see."""
        self.p.kill()
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
            # Only the first four fields are used here. The converter also
            # emits a phase column, which error_profile.py reads and this does
            # not.
            ply, san, lan, fen = line.rstrip("\n").split("\t")[:4]
            rows.append((int(ply), san, lan, fen))

    engine = Engine(args.engine)
    scores, bests = [], []

    for i, (ply, san, lan, fen) in enumerate(rows):
        try:
            score, best = engine.evaluate(fen, args.depth)
        except EngineError as exc:
            engine.kill()
            print(f"\nERROR: ply {ply}, after {san}: {exc}", file=sys.stderr)
            return 1
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

    return 0


if __name__ == "__main__":
    sys.exit(main())
