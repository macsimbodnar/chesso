#!/usr/bin/env python3
"""S165. Measure the mate sets from the defender's side, guard against no guard.

    ~/.venv/chess/bin/python adocs/data/S165_nmp_defender_sweep.py generate
    ~/.venv/chess/bin/python adocs/data/S165_nmp_defender_sweep.py sweep ENGINE [ENGINE ...]

WHY FROM THE DEFENDER'S SIDE. Reverse futility guards both edges of the mate
band -- `beta < MATE_MIN && beta > -MATE_MIN`, src/search.cpp -- and null move
pruning guards only the positive one, so where `beta <= -MATE_MIN` any null-move
result clears beta and the node fails high on a reduced search's word. Beta is a
mate bound against the side to move at a defender node inside a mate proof.
S145's sweeps varied reverse futility and assert from the attacker's side, so
they cannot see this. 2026-08-22_adversarial-F04.

WHERE THE POSITIONS COME FROM, AND WHY THEIR DISTANCES ARE PROVED AND NOT
COMPUTED. `adocs/data/S145_mate_set.tsv`'s `defender_nodes` column already holds
them: the mating line's positions at plies 1, 3, 5, 7, built for exactly the
nodes a guard has to search. What cannot be taken from that file is how far each
one is from mate. `representative_line()` walks the proof taking the attacker's
first quiet mating move and **the defender's first legal reply**, not the reply
that holds out longest, so a defender node's distance is not `distance` minus
its index -- that arithmetic is wrong on 3 of the 104 nodes, where the first
legal reply shortens the mate. Each distance is therefore proved here, by the
same exhaustive AND/OR enumeration S145 proves its roots with, iterative in the
distance so every shorter one is refuted, and corroborated by stockfish in its
own process. A mate in k against the side to move is 2k plies: k defender moves
and k attacker moves. That is not the attacker-side `2k - 1`, and getting it
wrong reads every node as one mate further away than it is.

THE NUMBERS PER SETTING, per distance, in S145_rfp_sweep.py's shape:

  exact     the final iteration reports being mated in exactly k
  delay     iterations between 2k, the first depth that can hold it, and the
            first iteration that reports it
  short     a mate against the side to move CLOSER than the proved k. The
            enumeration refuted every shorter distance, so this is a defect
  sign      a mate score FOR the side being mated. The false mate this guard is
            about, and a defect at any count

Run against the tune build, which is the build S145's sweeps use, with the
reverse futility parameters held at their shipping defaults so the sweep moves
one thing.
"""

import argparse
import os
import subprocess
import sys

import chess
import chess.engine

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S145_mate_set as constructed

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
TSV = os.path.join(HERE, "S165_defender_set.tsv")

STOCKFISH = "/usr/games/stockfish"

# S145's corroboration budget for this position class, and its warning with it:
# a node-limited stockfish is reproducible only inside one identical call
# sequence, and a frozen defending army is a class its network scores badly
# wrong. It corroborates here; the enumeration decides.
STOCKFISH_NODES = 4000000

# S145's number and for its reason: at exactly the minimum depth a postponed
# mate and a lost one look the same.
EXTRA_DEPTH = 8

# Held, so the sweep moves one thing. The shipping values, asserted against the
# binary rather than trusted.
HELD = {"RfpMinPly": 3, "RfpMaxDepth": 15}

MAX_DISTANCE = 7


def proved_distance(fen):
    """Smallest k with the side to move mated in k, by exhaustive enumeration.

    `_and_mate(board, plies)` is "the side to move is mated within `plies`
    whatever it plays", so k costs 2k plies from a defender node. Iterative in
    k, so returning k is also a refutation of every distance below it.
    """
    board = chess.Board(fen)

    for k in range(1, MAX_DISTANCE + 1):
        memo, budget = {}, [constructed.PROOF_NODE_CAP]

        try:
            if constructed._and_mate(board, 2 * k, memo, budget):
                return k
        except constructed.BudgetExceeded:
            return None

    return None


def stockfish_distance(fen):
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        info = engine.analyse(chess.Board(fen),
                              chess.engine.Limit(nodes=STOCKFISH_NODES))
        score = info["score"].relative

        if score.is_mate() and score.mate() is not None:
            return score.mate()

    return None


def generate():
    rows = constructed.read_tsv()
    out = []
    arithmetic_wrong = 0
    disagreed = []

    for row in rows:
        for i, fen in enumerate(row["defender_nodes"]):
            k = proved_distance(fen)

            if k is None:
                sys.exit("no proved distance within %d for %s" % (MAX_DISTANCE, fen))

            # What the arithmetic would have said, kept only to count how often
            # it is wrong. It is not used for anything.
            if k != row["distance"] - (i + 1):
                arithmetic_wrong += 1

            sf = stockfish_distance(fen)
            if sf != -k:
                disagreed.append((fen, k, sf))

            out.append({"fen": fen, "mated_in": k, "ply": 2 * i + 1,
                        "root_fen": row["fen"], "root_distance": row["distance"],
                        "family": row["family"]})

    with open(TSV, "w") as handle:
        handle.write("# S165 defender-side mate set. Regenerate with:\n")
        handle.write("#   ~/.venv/chess/bin/python "
                     "adocs/data/S165_nmp_defender_sweep.py generate\n")
        handle.write("# One row per defender node of adocs/data/S145_mate_set.tsv.\n")
        handle.write("# mated_in is the side to move's proved distance from being\n")
        handle.write("# mated, by exhaustive AND/OR enumeration, iterative in the\n")
        handle.write("# distance so every shorter one is refuted. It is NOT\n")
        handle.write("# root_distance - (ply + 1) / 2: that arithmetic is wrong on\n")
        handle.write("# %d of these %d rows, because representative_line() takes the\n"
                     % (arithmetic_wrong, len(out)))
        handle.write("# defender's FIRST legal reply and not its longest.\n")
        handle.write("\t".join(["fen", "mated_in", "ply", "root_distance",
                                "family", "root_fen"]) + "\n")

        for r in out:
            handle.write("\t".join([r["fen"], str(r["mated_in"]), str(r["ply"]),
                                    str(r["root_distance"]), r["family"],
                                    r["root_fen"]]) + "\n")

    print("%d defender nodes written to %s" % (len(out), TSV))
    print("the arithmetic root_distance - (ply + 1) / 2 is wrong on %d of them"
          % arithmetic_wrong)

    if disagreed:
        print("stockfish at %d nodes disagreed on %d:" % (STOCKFISH_NODES,
                                                          len(disagreed)))
        for fen, k, sf in disagreed:
            print("  enumeration -%d, stockfish %s: %s" % (k, sf, fen))
    else:
        print("stockfish at %d nodes agrees on all %d"
              % (STOCKFISH_NODES, len(out)))

    return 0


def read_tsv():
    rows = []

    with open(TSV) as handle:
        for line in handle:
            if line.startswith("#") or not line.strip():
                continue
            field = line.rstrip("\n").split("\t")
            if field[0] == "fen":
                continue
            rows.append({"fen": field[0], "mated_in": int(field[1]),
                         "ply": int(field[2])})

    return rows


def check_held(engine_path):
    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        for name, value in HELD.items():
            option = engine.options.get(name)

            if option is None:
                sys.exit("%s: no option %s -- this is not the tune build"
                         % (engine_path, name))

            if option.default != value:
                sys.exit("%s: %s defaults to %s, this sweep holds it at %d; "
                         "the held values are stale"
                         % (engine_path, name, option.default, value))


def sweep_one(engine_path, rows):
    per = {}

    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        engine.configure(dict({"Hash": 16}, **HELD))

        for row in rows:
            k = row["mated_in"]
            minimum = 2 * k
            bucket = per.setdefault(k, {"n": 0, "exact": 0, "delays": [],
                                        "short": 0, "sign": 0})
            bucket["n"] += 1

            first = None
            last = None

            with engine.analysis(
                    chess.Board(row["fen"]),
                    chess.engine.Limit(depth=minimum + EXTRA_DEPTH)) as analysis:
                for info in analysis:
                    if "score" not in info or "depth" not in info:
                        continue

                    # Relative, so the side to move is the side being mated: a
                    # correct report is negative and a positive mate score is
                    # the false mate this guard is about.
                    score = info["score"].relative
                    last = score

                    if not score.is_mate() or score.mate() is None:
                        continue

                    mate = score.mate()

                    if mate > 0:
                        bucket["sign"] += 1
                    elif -mate < k:
                        bucket["short"] += 1
                    elif -mate == k and first is None:
                        first = info["depth"]

            if last is not None and last.is_mate() and last.mate() == -k:
                bucket["exact"] += 1

            if first is not None:
                bucket["delays"].append(first - minimum)

    return per


def report(label, per):
    total = sum(b["n"] for b in per.values())
    exact = sum(b["exact"] for b in per.values())
    short = sum(b["short"] for b in per.values())
    sign = sum(b["sign"] for b in per.values())

    parts = []
    for k in sorted(per):
        bucket = per[k]
        delays = bucket["delays"]
        parts.append("m%d %d/%d d%s" % (k, bucket["exact"], bucket["n"],
                                        max(delays) if delays else "-"))

    print("  %-26s exact %3d/%3d  %s  short %d  sign %d"
          % (label, exact, total, "  ".join(parts), short, sign))


def sweep(engines):
    rows = read_tsv()

    counts = {}
    for row in rows:
        counts[row["mated_in"]] = counts.get(row["mated_in"], 0) + 1

    print("%d defender nodes, depth 2k+%d, %s"
          % (len(rows), EXTRA_DEPTH,
             ", ".join("%s held at %d" % kv for kv in HELD.items())))
    print("  per proved distance: %s\n"
          % "  ".join("m%d %d" % (k, counts[k]) for k in sorted(counts)))

    for engine_path in engines:
        check_held(engine_path)
        report(engine_path, sweep_one(engine_path, rows))

    print("\nS165-SWEEP-DONE")
    return 0


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="command", required=True)
    sub.add_parser("generate")
    run = sub.add_parser("sweep")
    run.add_argument("engines", nargs="+")
    args = ap.parse_args()

    if args.command == "generate":
        return generate()

    return sweep(args.engines)


if __name__ == "__main__":
    sys.exit(main())
