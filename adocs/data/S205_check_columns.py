#!/usr/bin/env python3
"""Re-derives S205's two numbers: the census of asset layers carrying a value
for each of the four check columns, and the convention those values are
written in.

The census is what S205 was opened on. The convention is what it found: the
four columns are not four independent counts, and reading them as such put
`test_perft` red on one layer out of twenty-nine.

Run with the repository's python-chess interpreter, from anywhere:

    ~/.venv/chess/bin/python adocs/data/S205_check_columns.py
    ~/.venv/chess/bin/python adocs/data/S205_check_columns.py --max-nodes 5000000

The cross-check needs python-chess; the census does not.
"""

import argparse
import glob
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ASSET_DIR = os.path.join(ROOT, "tests", "assets", "perft_json")

# The two files `test_perft.cpp`'s `test_files` actually loads. debug_perft.json
# sits beside them in the directory and is commented out there, which is why a
# census over the glob and a census over what a run reads are different numbers.
LOADED = ["talkchess_perft.json", "perft.json"]

# `#define MAXIMUM_DEPTH 20` in tests/test_perft.cpp.
MAXIMUM_DEPTH = 20

COLUMNS = ["checks", "discovery_checks", "double_checks", "checkmates"]


def layers(files, only_what_a_run_reads):
    """Every depth layer of `files`, or only the ones a run compares."""
    for name in files:
        path = os.path.join(ASSET_DIR, name)
        for case in json.load(open(path)):
            if only_what_a_run_reads and not case["enable"]:
                continue
            limit = min(case["depth_limit"], MAXIMUM_DEPTH)
            for layer in case["depth_layers"]:
                if only_what_a_run_reads and layer["depth"] > limit:
                    continue
                yield case["start_fen"], layer


def census():
    rows = []
    for label, files, run_only in (
        ("glob, every layer", sorted(os.path.basename(p) for p in
                                     glob.glob(os.path.join(ASSET_DIR, "*.json"))), False),
        ("files test_perft loads, every layer", LOADED, False),
        ("layers a run compares", LOADED, True),
    ):
        counts = dict.fromkeys(COLUMNS, 0)
        total = 0
        for _, layer in layers(files, run_only):
            total += 1
            for column in COLUMNS:
                if layer.get(column) is not None:
                    counts[column] += 1
        rows.append((label, total, counts))

    width = max(len(label) for label, _, _ in rows)
    print(f"{'scope':<{width}} {'layers':>7} " +
          " ".join(f"{c:>17}" for c in COLUMNS))
    for label, total, counts in rows:
        print(f"{label:<{width}} {total:>7} " +
              " ".join(f"{counts[c]:>17}" for c in COLUMNS))
    return rows


def classify(board, move, mate):
    """The four columns for one leaf, in the convention the assets use.

    `checks` is every check leaf, mates included. `checkmates` is every mate.
    The two classification columns describe the non-mating checks only and are
    exclusive of each other: a double check that is also discovered counts once,
    as double. A discovered check is delivered by a piece this move did not
    move -- which for a castle means neither the king's square nor the rook's.
    """
    import chess

    checkers = board.checkers()
    if not checkers:
        return dict.fromkeys(COLUMNS, 0)

    out = {"checks": 1, "checkmates": 1 if mate else 0,
           "double_checks": 0, "discovery_checks": 0}
    if mate:
        return out

    if chess.popcount(int(checkers)) > 1:
        out["double_checks"] = 1
        return out

    moved_to = chess.BB_SQUARES[move.to_square]
    if board.is_castling(move):
        # The rook lands on the square the king stepped over.
        moved_to |= chess.BB_SQUARES[(move.from_square + move.to_square) // 2]
    if int(checkers) & ~moved_to:
        out["discovery_checks"] = 1
    return out


def walk(board, depth, totals):
    import chess

    for move in list(board.legal_moves):
        board.push(move)
        if depth == 1:
            totals["nodes"] += 1
            if board.is_check():
                mate = board.is_checkmate()
                for column, hit in classify(board, move, mate).items():
                    totals[column] += hit
        else:
            walk(board, depth - 1, totals)
        board.pop()


def cross_check(max_nodes):
    """Recount every affordable asset layer with python-chess and compare.

    An independent implementation of the board, the legality and the check
    detection, so agreement is evidence about the convention rather than about
    one generator agreeing with itself.
    """
    try:
        import chess  # noqa: F401
    except ImportError:
        print("\npython-chess not importable; census only. "
              "Use ~/.venv/chess/bin/python.", file=sys.stderr)
        return 1

    import chess

    print(f"\ncross-check against python-chess, layers under {max_nodes} nodes")
    checked = skipped = 0
    bad = []
    for fen, layer in layers(LOADED, True):
        depth = layer["depth"]
        if not any(layer.get(c) is not None for c in COLUMNS):
            continue
        if layer["nodes"] > max_nodes:
            skipped += 1
            continue

        totals = dict.fromkeys(COLUMNS, 0)
        totals["nodes"] = 0
        board = chess.Board(fen)
        if depth == 0:
            totals["nodes"] = 1
        else:
            walk(board, depth, totals)

        if totals["nodes"] != layer["nodes"]:
            bad.append((fen, depth, "nodes", layer["nodes"], totals["nodes"]))
        for column in COLUMNS:
            expected = layer.get(column)
            if expected is not None and expected != totals[column]:
                bad.append((fen, depth, column, expected, totals[column]))
        checked += 1

    print(f"  {checked} layers recounted, {skipped} over the node budget")
    if bad:
        for fen, depth, column, expected, got in bad:
            print(f"  MISMATCH {fen[:28]} d{depth} {column}: "
                  f"asset {expected}, python-chess {got}")
        return 1
    print("  every column agrees on every layer recounted")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--max-nodes", type=int, default=250_000,
                        help="skip cross-checking layers larger than this "
                             "(default 250000, about 4 s; 5000000 reaches "
                             "Kiwipete depth 4 and takes about 2 minutes)")
    parser.add_argument("--census-only", action="store_true")
    args = parser.parse_args()

    census()
    if args.census_only:
        return 0
    return cross_check(args.max_nodes)


if __name__ == "__main__":
    sys.exit(main())
