#!/usr/bin/env python3
"""How deep the search gets on a fixed number of nodes. S097.

THE INSTRUMENT AND WHAT IT IS FOR. A singular extension trades nodes for
depth on purpose: the move it calls singular is searched a ply deeper, so the
same node budget buys fewer iterations. That is the technique working, up to
the point where it stops being one -- an extension that fires too often spends
the budget on one line and the search reaches a shallower depth everywhere
else, which is the published hazard for this class of rule and the one a node
count cannot see. `chesso bench` gets *smaller* under an over-firing extension
if the tree is reordered, and `tools/search_bench.py` measures a fixed depth,
so neither of them can tell a good trade from a bad one.

This asks the other question: **at a fixed node budget, what depth does the
search reach, and what does it play?** Run it on the tree before a change and
on the tree after it. A depth that falls on every position with no rating to
show for it is the explosion signature; a depth that holds while the move
changes is the rule doing what it is for.

It is an instrument and not a verdict. Depth at a fixed budget is not Elo and
nothing here reads it as Elo (DEC-019): what decides the step is
`adocs/data/S097_v1_sprt.sh` and `adocs/data/S097_v2_sprt.sh`.

    adocs/data/S097_fixed_node_depth.py ./build/src/chesso
    adocs/data/S097_fixed_node_depth.py ./build/src/chesso --nodes 1000000
    adocs/data/S097_fixed_node_depth.py ./build/src/chesso --hash 16

The three positions are `tools/search_bench.py`'s, unchanged and in its order,
so a reading here sits beside the node counts recorded for the same step.

Output is one TSV row per position -- position, depth reached, nodes actually
searched, best move, score as the engine reports it -- and a final `total` row.
The engine's own last `info` line is what is read, and the depth on it is
`last_complete_depth` (`src/chesso.cpp`): the last iteration that **finished**,
never the one the budget stopped inside, whose score and window mean nothing.
That is exactly the number this instrument is about. There is no `seldepth`
column because this engine prints no such field (`MANUAL.md`, "What a search
prints"), and a column that can only ever read zero is worse than no column.

**The `quit` trap, TOOLCHAIN.md's, and why it is not sprung here.** A one-shot
`printf 'go ...\nquit\n' | engine` delivers `quit` while the search is being
started and kills it before it looks at a node -- `go depth 6` from the start
position answers `depth 1` and 49 nodes that way. This reads lines until
`bestmove` arrives and sends `quit` only afterwards.

The machine has to be idle for the timing half of any comparison, but not for
this one: a node budget is enforced exactly (`check_limits` in src/search.cpp
compares the count, it does not sample a clock), so the depth reached is a
property of the tree and not of the load. TOOLCHAIN.md's load check still
applies to anything read beside it.
"""

import argparse
import subprocess
import sys

# tools/search_bench.py's three, in its order.
POSITIONS = [
    ("midgame",
     "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"),
    ("kiwipete",
     "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"),
    ("tactical",
     "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"),
]


def field(line, name, default=None):
    """The token after `name` on a UCI info line, or `default`."""
    parts = line.split()

    for i, token in enumerate(parts):
        if token == name and i + 1 < len(parts):
            return parts[i + 1]

    return default


def run_one(engine, fen, nodes, hash_mb, options):
    """Drive one position to a node budget and read the last info line."""
    process = subprocess.Popen([engine], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(text):
        process.stdin.write(text + "\n")
        process.stdin.flush()

    send("uci")

    while "uciok" not in process.stdout.readline():
        pass

    send(f"setoption name Hash value {hash_mb}")

    for name, value in options:
        send(f"setoption name {name} value {value}")

    send("ucinewgame")
    send("isready")

    while "readyok" not in process.stdout.readline():
        pass

    send(f"position fen {fen}")
    send(f"go nodes {nodes}")

    last = ""
    best = ""

    while True:
        line = process.stdout.readline()

        if not line:
            break

        if line.startswith("info ") and " depth " in line:
            last = line.strip()

        if line.startswith("bestmove"):
            best = line.split()[1]
            break

    send("quit")
    process.wait(timeout=10)

    return {
        "depth": int(field(last, "depth", 0)),
        "nodes": int(field(last, "nodes", 0)),
        "score": " ".join(last.split("score")[1].split()[:2])
                 if "score" in last else "",
        "best": best,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("engine")
    parser.add_argument("--nodes", type=int, default=1000000,
                        help="the budget per position (default 1000000)")
    parser.add_argument("--hash", type=int, default=16,
                        help="Hash in MB; the SPRT regime's 16 by default")
    parser.add_argument("--option", action="append", default=[],
                        metavar="NAME=VALUE",
                        help="a setoption to send first; repeatable. Only the "
                             "tune build has any")
    args = parser.parse_args()

    options = []

    for pair in args.option:
        if "=" not in pair:
            print(f"--option wants NAME=VALUE, got {pair}", file=sys.stderr)
            return 2

        name, value = pair.split("=", 1)
        options.append((name, value))

    print(f"# {args.engine}, go nodes {args.nodes}, Hash {args.hash}"
          + (f", options {args.option}" if args.option else ""))
    print("position\tdepth\tnodes\tbest\tscore")

    total_nodes = 0
    total_depth = 0

    for name, fen in POSITIONS:
        row = run_one(args.engine, fen, args.nodes, args.hash, options)
        total_nodes += row["nodes"]
        total_depth += row["depth"]

        print(f"{name}\t{row['depth']}\t{row['nodes']}"
              f"\t{row['best']}\t{row['score']}")

    print(f"total\t{total_depth}\t{total_nodes}\t\t")

    return 0


if __name__ == "__main__":
    sys.exit(main())
