#!/usr/bin/env python3
"""Replay a game move by move through one engine process and report every
`info` line whose mate line is shorter than the distance it claims.

S170. The defect it exists to see cannot be reproduced by searching a position
on a cold table: the mate score is read back from an entry a search of an
earlier move in the same game wrote, and the entries carrying its line have
been overwritten since. One process, one `ucinewgame`, and the moves fed in
one at a time is the only shape that gives the table the contents a game gives
it -- which is why tests/test_mate_pv.cpp, where every case is its own
`ucinewgame`, cannot see it.

Reads cases from a TSV (`name`, `fen`, `moves`) or from --fen/--moves, and for
each replayed ply prints the short lines with the ply they came from.

Usage:
  S170_replay.py --engine build/src/chesso --cases adocs/data/S170_cases.tsv
  S170_replay.py --engine build/src/chesso --fen '<fen>' --moves 'e2e4 e7e5'
"""

import argparse
import subprocess
import sys


def plies_to_deliver(mate_in):
    """Plies a mate at this distance takes: the side delivering it moves last."""
    return 2 * mate_in - 1 if mate_in > 0 else -2 * mate_in


class Engine:
    def __init__(self, path, hash_mb):
        self.proc = subprocess.Popen(
            [path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
            bufsize=1,
        )
        self.send("uci")
        self.read_until("uciok")
        self.send(f"setoption name Hash value {hash_mb}")
        self.send("ucinewgame")

    def send(self, line):
        self.proc.stdin.write(line + "\n")
        self.proc.stdin.flush()

    def read_until(self, token):
        """Every line up to and including the one starting with `token`.

        Never write `quit` to reach a terminator: the engine answers `bestmove`
        when it is done and reading to it is what waits for the search. The
        same trap TOOLCHAIN.md records for stockfish applies to this engine.
        """
        out = []
        while True:
            line = self.proc.stdout.readline()
            if line == "":
                raise RuntimeError("engine closed its output")
            line = line.strip()
            out.append(line)
            if line.startswith(token):
                return out

    def go(self, spec):
        self.send("go " + spec)
        return self.read_until("bestmove")

    def close(self):
        self.send("quit")
        self.proc.wait(timeout=10)


def parse_info(line):
    """(mate_in, depth, pv) for an info line carrying a mate score, else None."""
    token = line.split()
    if len(token) < 4 or token[0] != "info" or token[1] != "score":
        return None
    if token[2] != "mate":
        return None

    mate_in = int(token[3])
    depth = 0
    pv = []

    i = 4
    while i < len(token):
        if token[i] == "depth" and i + 1 < len(token):
            depth = int(token[i + 1])
        if token[i] == "pv":
            pv = token[i + 1:]
            break
        i += 1

    return mate_in, depth, pv


def replay(engine, fen, moves, spec, name, verbose, start=0):
    """One game from ply `start` onward. Returns the short mate lines found.

    `start` skips the searches of the opening plies, not the plies themselves:
    the position is always the full move list, and what is dropped is the
    warming those early searches would have done to the table. A case that
    still reproduces from a late start is a cheaper reproduction, which is what
    puts one inside the fast gate.
    """
    short = []

    for i in range(start, len(moves) + 1):
        played = " ".join(moves[:i])
        position = f"position fen {fen}"
        if played:
            position += " moves " + played
        engine.send(position)

        for line in engine.go(spec):
            parsed = parse_info(line)
            if parsed is None:
                continue

            mate_in, depth, pv = parsed
            needed = plies_to_deliver(mate_in)

            if verbose:
                print(f"  ply {i:3d} depth {depth:2d} mate {mate_in:3d} "
                      f"pv {len(pv)}/{needed}")

            if len(pv) < needed:
                short.append((name, i, depth, mate_in, len(pv), needed,
                              " ".join(pv)))
                print(f"SHORT {name} ply {i} depth {depth} mate {mate_in} "
                      f"pv {len(pv)} plies, {needed} needed: {' '.join(pv)}")

    return short


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--engine", default="build/src/chesso")
    ap.add_argument("--cases", help="TSV: name, fen, moves")
    ap.add_argument("--fen")
    ap.add_argument("--moves", default="")
    ap.add_argument("--go",
                    help="override the case's own go arguments, e.g. 'nodes 200000'")
    ap.add_argument("--start-override", type=int, dest="start_override",
                    help="override the case's own start ply")
    ap.add_argument("--hash", type=int, default=16,
                    help="Hash in MB; 16 is what fastchess.sh sets")
    ap.add_argument("--only", help="run only the case with this name")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    cases = []
    if args.cases:
        with open(args.cases) as handle:
            for line in handle:
                line = line.rstrip("\n")
                if not line or line.startswith("#"):
                    continue
                field = line.split("\t")
                if field[0] == "name":
                    continue
                spec = field[3] if len(field) > 3 and field[3] else None
                start = int(field[4]) if len(field) > 4 and field[4] else 0
                cases.append((field[0], field[1], field[2].split(), spec,
                              start))
    elif args.fen:
        cases.append(("cli", args.fen, args.moves.split(), None, 0))
    else:
        ap.error("--cases or --fen is required")

    if args.only:
        cases = [c for c in cases if c[0] == args.only]

    total = 0
    for name, fen, moves, spec, start in cases:
        spec = args.go or spec or "movetime 150"
        start = args.start_override if args.start_override is not None else start

        engine = Engine(args.engine, args.hash)
        print(f"=== {name}: {len(moves)} plies from ply {start}, go {spec}")
        total += len(replay(engine, fen, moves, spec, name, args.verbose,
                            start))
        engine.close()

    print(f"TOTAL short mate lines: {total}")
    return 0 if total == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
