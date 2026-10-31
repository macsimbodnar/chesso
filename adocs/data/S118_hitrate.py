#!/usr/bin/env python3
"""S118's hit rate over a real search: the accepts' "measured and not assumed".

    ~/.venv/chess/bin/python adocs/data/S118_hitrate.py ENGINE --pgn GAMES.pgn

ENGINE is a tune build (cmake -DCHESSO_TUNE=ON). Only that build counts: its
src/evaluation.cpp `pawn_table_census_t` appends one line to the file
CHESSO_PAWN_TABLE_CENSUS names, at the process's end, and a binary that writes
none is refused here rather than read as a table that was never probed. It runs
one process per workload, in turn, each with an empty table per `go` as the
search has, and the machine owes it nothing: it counts, it does not time, and a
tune build's tree is the release build's (INV-6, adocs/data/S118_inv6.txt).

The workloads:

    bench         `chesso bench`, the eight positions, depth 14
    bench16       `chesso bench 16`, the one the timing reads
    pos11         S020's 300 positions at depth 11, through one process, the
                  other one the timing reads (adocs/data/S135_cost.py workload)
    replay        the first --games of GAMES.pgn replayed move by move, each
                  move's recorded n= as `go nodes`, `ucinewgame` per game, Hash 16
                  as the harness runs it (S120's replay form)

Printed per workload: probes, the share of them that hit, and the shelter slot's
probes and share, one count per king where the build has the slot. The size is
the build's PAWN_TABLE_BITS. Another size takes a tune build with that one line
changed, adocs/data/S118_cost.sh's edit. Expected time, one process: about 10 min
at 20 games, nearly all of it the replay, ceiling 30 min. Ends on S118-HITRATE-DONE
or S118-HITRATE-FAILED, its last line either way.
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def read_census(path):
    """{field: count}, summed over the file's lines, or None when it has none."""
    if not os.path.exists(path):
        return None
    total = {}
    for line in open(path):
        words = line.split()
        for key, value in zip(words[0::2], words[1::2]):
            total[key] = total.get(key, 0) + int(value)
    return total or None


def run_plain(engine, args, env):
    subprocess.run([engine] + args, env=env, check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def run_positions(engine, env):
    subprocess.run([sys.executable,
                    os.path.join(ROOT, "adocs", "data", "S135_cost.py"),
                    "workload", engine, "11"],
                   env=env, check=True, stdout=subprocess.DEVNULL)


def run_replay(engine, pgn_path, games, env):
    import chess.pgn  # the venv's: only the replay needs it

    process = subprocess.Popen([engine], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.DEVNULL, text=True, bufsize=1,
                               env=env)

    def send(text):
        process.stdin.write(text + "\n")
        process.stdin.flush()

    def wait(prefix):
        while True:
            line = process.stdout.readline()
            if not line:
                sys.exit("the engine closed its output")
            if line.startswith(prefix):
                return

    send("uci")
    wait("uciok")
    send("setoption name Hash value 16")
    send("isready")
    wait("readyok")

    searches = 0
    with open(pgn_path) as handle:
        for _ in range(games):
            game = chess.pgn.read_game(handle)
            if game is None:
                break
            start = game.board().fen()
            moves = []
            send("ucinewgame")
            send("isready")
            wait("readyok")
            for node in game.mainline():
                found = re.search(r"n=(\d+)", node.comment or "")
                if found and int(found.group(1)) > 0:
                    tail = " moves " + " ".join(moves) if moves else ""
                    send("position fen " + start + tail)
                    send("go nodes " + found.group(1))
                    wait("bestmove")
                    searches += 1
                moves.append(node.move.uci())
    send("quit")
    process.wait()
    if searches == 0:
        sys.exit(f"{pgn_path}: no move carries a recorded n=")


def share(hit, miss):
    return 100.0 * hit / (hit + miss) if hit + miss else 0.0


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("engine")
    parser.add_argument("--pgn", required=True)
    parser.add_argument("--games", type=int, default=20)
    args = parser.parse_args(argv)

    engine = os.path.abspath(args.engine)
    workloads = [
        ("bench", lambda env: run_plain(engine, ["bench"], env)),
        ("bench16", lambda env: run_plain(engine, ["bench", "16"], env)),
        ("pos11", lambda env: run_positions(engine, env)),
        ("replay", lambda env: run_replay(engine, args.pgn, args.games, env)),
    ]

    print("workload   probes        hit %      slot probes   slot hit %")
    with tempfile.TemporaryDirectory() as scratch:
        for name, run in workloads:
            path = os.path.join(scratch, name)
            run(dict(os.environ, CHESSO_PAWN_TABLE_CENSUS=path))
            counts = read_census(path)
            if counts is None:
                sys.exit(f"{name}: no census, so {engine} is not a tune build")
            hits, misses = counts["hits"], counts["misses"]
            slot_hits = counts.get("slot_hits", 0)
            slot_misses = counts.get("slot_misses", 0)
            print(f"{name:<10} {hits + misses:<13} {share(hits, misses):<10.3f} "
                  f"{slot_hits + slot_misses:<13} "
                  f"{share(slot_hits, slot_misses):.3f}", flush=True)
    return 0


if __name__ == "__main__":
    try:
        status = main(sys.argv[1:])
    except (OSError, subprocess.CalledProcessError) as error:
        print(error)
        status = 1
    except SystemExit as stop:
        if isinstance(stop.code, str):
            print(stop.code)
            status = 1
        else:
            status = stop.code or 0
    print("S118-HITRATE-DONE" if status == 0 else "S118-HITRATE-FAILED")
    sys.exit(status)
