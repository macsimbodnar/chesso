#!/usr/bin/env python3
"""S296: the oracle's mate distance for every position S296 compares a
reported mate against, cached in adocs/data/S296_oracle.tsv. Stockfish at
5000000 nodes through python-chess, TOOLCHAIN.md's safe form (the call reads
until the search answers; `quit` only on close); one process per worker, a
fresh `game=object()` per call so python-chess sends `ucinewgame` before each.

Not `go depth 20`, S295's trace's setting and this script's first run:
on these positions depth 20 ends after a few thousand nodes and reports the
first mate it found, not the shortest -- S145 mined line 96 read `#+10` at
depth 20 and `#+4` at depth 30, at 200000 nodes and at 2000000 (the label's
own 4, and the engine's). Against the proved distances of S145's and S165's
sets (S145's enumeration) depth 20 disagreed on 50 of 372 rows (a longer mate
on 23, no mate on 27) and the 5000000-node run on none, and read against
depth 20, 17 of the shipped engine's last-iteration mates over S289's four
sets were "short" where the node-limited oracle reads none. The node budget
is what lets the search converge; outside the proved sets it is an oracle,
not a proof.

    P=~/.venv/chess/bin/python
    $P adocs/data/S296_oracle.py positions --sets SET... --grids GRID... \\
        --out FENS
    $P adocs/data/S296_oracle.py run FENS adocs/data/S296_oracle.tsv

`positions`: every FEN of the given S289_rows.py set files (fen<TAB>...), and
every position at which a given S295_long_line.py sweep or s170 file reports
a mate line (the mined grid's key is its FEN; an S170 cell's position is its
case's FEN with the first `ply` moves of the case's line applied by
python-chess). Deduplicated, in first-seen order.

`run`: one row per FEN, `fen<TAB>kind<TAB>value<TAB>depth<TAB>pv0`, kind
`mate` (signed, from the side to move: +n mates, -n is mated) or `cp`. The
engine's answer only; nothing here is a chess judgement of this project's.
"""

import argparse
import concurrent.futures
import os
import sys

import chess
import chess.engine

HERE = os.path.dirname(os.path.abspath(__file__))
CASES = os.path.join(HERE, "S170_cases.tsv")
STOCKFISH = "/usr/games/stockfish"
NODES = 5000000


def read_cases():
    cases = {}
    for line in open(CASES):
        line = line.rstrip("\n")
        if not line or line.startswith("#"):
            continue
        f = line.split("\t")
        if f[0] == "name":
            continue
        cases[f[0]] = (f[1], f[2].split())
    return cases


def grid_fens(path, cases):
    """The position of every mate line in an S295_long_line.py file."""
    out = []
    head = None
    for line in open(path):
        if line.startswith("#"):
            continue
        f = line.rstrip("\n").split("\t")
        if head is None:
            head = f
            continue
        row = dict(zip(head, f))
        if row["set"] == "mined":
            out.append(row["key"])
        else:
            fen, moves = cases[row["key"].split("/")[0]]
            board = chess.Board(fen)
            for uci in moves[:int(row["ply"])]:
                board.push_uci(uci)
            out.append(board.fen())
    return out


def positions(args):
    cases = read_cases()
    seen, order = set(), []
    for path in args.sets:
        for line in open(path):
            if line.strip() and not line.startswith("#"):
                fen = line.split("\t")[0]
                if fen not in seen:
                    seen.add(fen)
                    order.append(fen)
    for path in args.grids:
        for fen in grid_fens(path, cases):
            if fen not in seen:
                seen.add(fen)
                order.append(fen)
    with open(args.out, "w") as h:
        for fen in order:
            h.write(fen + "\n")
    print("%d positions" % len(order))
    return 0


def analyse(chunk):
    rows = []
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        for fen in chunk:
            info = engine.analyse(chess.Board(fen),
                                  chess.engine.Limit(nodes=NODES),
                                  game=object())
            score = info["score"].relative
            if score.is_mate():
                kind, value = "mate", score.mate()
            else:
                kind, value = "cp", score.score()
            pv0 = info["pv"][0].uci() if info.get("pv") else "-"
            rows.append((fen, kind, value, info.get("depth", 0), pv0))
    return rows


def run(args):
    fens = [l.strip() for l in open(args.fens) if l.strip()]
    jobs = args.jobs
    chunks = [fens[i::jobs] for i in range(jobs)]
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        got = {r[0]: r for rows in pool.map(analyse, chunks) for r in rows}
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        name = engine.id.get("name", "?")
    with open(args.out, "w") as h:
        h.write("# S296 oracle: %s, go nodes %d, python-chess analyse, "
                "ucinewgame per position, default Hash and Threads.\n" %
                (name, NODES))
        h.write("# Made by: ~/.venv/chess/bin/python adocs/data/S296_oracle.py "
                "run <fens> adocs/data/S296_oracle.tsv (the fens by its "
                "`positions` stage, the command in the step file).\n")
        h.write("fen\tkind\tvalue\tdepth\tpv0\n")
        for fen in fens:
            h.write("%s\t%s\t%s\t%s\t%s\n" % got[fen])
    print("%d rows -> %s" % (len(fens), args.out))
    return 0


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="stage", required=True)
    p = sub.add_parser("positions")
    p.add_argument("--sets", nargs="*", default=[])
    p.add_argument("--grids", nargs="*", default=[])
    p.add_argument("--out", required=True)
    r = sub.add_parser("run")
    r.add_argument("fens")
    r.add_argument("out")
    r.add_argument("--jobs", type=int, default=os.cpu_count())
    args = ap.parse_args()
    return positions(args) if args.stage == "positions" else run(args)


if __name__ == "__main__":
    sys.exit(main())
