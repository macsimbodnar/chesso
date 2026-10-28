#!/usr/bin/env python3
"""S136 phase 2b. The mined mates the tempo paste moves, read by tool.

    python3 adocs/data/S136_lost_mates.py depths ENGINE MAXDEPTH ROWS [--opt N=V ...]
    ~/.venv/chess/bin/python adocs/data/S136_lost_mates.py oracle ROWS [--nodes N]
        [--threads T --hash MB]

ROWS is a comma list of indices into S145's mined set as S156's `read_set`
returns them (0-based data rows), the indices `S136_mined_rows.py` prints.

`depths`: one `go depth MAXDEPTH` per row (one process, Hash 16, `ucinewgame`
before each position, any `--opt` sent as setoption first), and the score of
each iteration -- the last `info ... depth d` line printed for that depth, so
an aspiration search's re-searches leave only their final line. A fresh
`go depth d` runs exactly the first d iterations of this search, so iteration
d is what `S136_mined_rows.py ENGINE d` reads at depth d; the step file
records that check. Prints, per row, the label, the first iteration that
reports the exact mate, the iteration from which it is held to MAXDEPTH (`-`
if the last is not exact), and the per-iteration scores.

`oracle`: Stockfish through python-chess at `--nodes` (default 5000000,
DEC-319 (3)), TOOLCHAIN.md's safe form -- `analyse` reads until the search
answers, a fresh `game=object()` per call so python-chess sends `ucinewgame`.
The engine's answer only: kind (`mate`, signed from the side to move, or
`cp`), value, depth, nodes and the first moves of its line. Where it finds no
mate, DEC-320 reads the position again at 1000000000 nodes and labels it as
the deeper reading; `--nodes 1000000000 --threads 12 --hash 2048` is that
reading. Nothing here is a chess judgement of this project's.

DEC-016: the positions are chesso's own games, the labels S145's.
"""

import argparse
import importlib.util
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
STOCKFISH = "/usr/games/stockfish"


def mined_rows():
    spec = importlib.util.spec_from_file_location(
        "s156", os.path.join(HERE, "S156_mined_floor_sweep.py"))
    s156 = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(s156)
    return s156.read_set()


def wanted(text):
    return [int(x) for x in text.split(",")]


def depths(args):
    rows = mined_rows()
    process = subprocess.Popen([args.engine], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        process.stdin.write(line + "\n")
        process.stdin.flush()

    send("uci")
    while "uciok" not in process.stdout.readline():
        pass
    send("setoption name Hash value 16")
    for opt in args.opt:
        name, value = opt.split("=", 1)
        send("setoption name %s value %s" % (name, value))

    print("# %s, go depth %d, Hash 16, ucinewgame per row%s"
          % (args.engine, args.maxdepth,
             "".join(", setoption %s" % o for o in args.opt)))
    print("# row\tlabel\tfirst\theld\tper iteration (depth:score)")

    for index in wanted(args.rows):
        fen, distance = rows[index]
        send("ucinewgame")
        send("position fen " + fen)
        send("go depth %d" % args.maxdepth)

        last = {}
        while True:
            line = process.stdout.readline()
            if not line or line.startswith("bestmove"):
                break
            if line.startswith("info") and " score " in line \
                    and " depth " in line:
                t = line.split()
                depth = int(t[t.index("depth") + 1])
                kind = t[t.index("score") + 1]
                value = int(t[t.index("score") + 2])
                last[depth] = (kind, value)

        exact = {d for d, (k, v) in last.items() if k == "mate" and v == distance}
        first = min(exact) if exact else None
        held = None
        for d in sorted(last, reverse=True):
            if d not in exact:
                break
            held = d
        print("%d\tm%d\t%s\t%s\t%s"
              % (index, distance, first if first else "-",
                 held if held else "-",
                 " ".join("%d:%s%d" % (d, "m" if k == "mate" else "",
                                       v) for d, (k, v) in sorted(last.items()))))
        sys.stdout.flush()

    send("quit")
    process.wait(timeout=10)
    return 0


def oracle(args):
    import chess
    import chess.engine

    rows = mined_rows()
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        options = {}
        if args.threads:
            options["Threads"] = args.threads
        if args.hash:
            options["Hash"] = args.hash
        engine.configure(options)
        print("# oracle: %s, go nodes %d, python-chess analyse, ucinewgame per "
              "call, %s" % (engine.id.get("name", "?"), args.nodes,
                            ", ".join("%s %s" % kv for kv in options.items())
                            or "default Hash and Threads"))
        print("# row\tlabel\tkind\tvalue\tdepth\tnodes\tpv")

        for index in wanted(args.rows):
            fen, distance = rows[index]
            info = engine.analyse(chess.Board(fen),
                                  chess.engine.Limit(nodes=args.nodes),
                                  game=object())
            score = info["score"].relative
            if score.is_mate():
                kind, value = "mate", score.mate()
            else:
                kind, value = "cp", score.score()
            pv = " ".join(m.uci() for m in info.get("pv", [])[:12])
            print("%d\tm%d\t%s\t%s\t%s\t%s\t%s"
                  % (index, distance, kind, value, info.get("depth"),
                     info.get("nodes"), pv))
            sys.stdout.flush()
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="mode", required=True)
    d = sub.add_parser("depths")
    d.add_argument("engine")
    d.add_argument("maxdepth", type=int)
    d.add_argument("rows")
    d.add_argument("--opt", action="append", default=[])
    o = sub.add_parser("oracle")
    o.add_argument("rows")
    o.add_argument("--nodes", type=int, default=5000000)
    o.add_argument("--threads", type=int, default=0)
    o.add_argument("--hash", type=int, default=0)
    args = ap.parse_args()
    return depths(args) if args.mode == "depths" else oracle(args)


if __name__ == "__main__":
    sys.exit(main())
