#!/usr/bin/env python3
"""Re-derive the census floors that tests/test_invariants.cpp asserts (DEC-142).

The test walks the JSON corpus at CORPUS_DEPTH and the five positions of
tests/test_engine.cpp's hash oracle at their own depths, and asserts floors on
the number of make_move calls and on how many of them were castlings, en
passants, promotions and capture promotions. Those floors are goldens: they are
re-derived by this script whenever the corpus, either depth, or the generator
moves, and never re-read from the test's own run.

This counts the same tree with python-chess, an implementation of the rules the
engine does not share a line of code with, so a generator that loses a class of
move shows up here as a disagreement rather than as a quietly lower floor.

    ~/.venv/chess/bin/python adocs/data/S190_walk_census.py

Prints the five counts and the floors to write into the test (half of each,
rounded down to a round number). Minutes, no engine build needed.
"""

import json
import os
import sys

import chess

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ASSETS = os.path.join(REPO, "tests", "assets", "test_jsons")

# tests/test_helpers.hpp json_test_files, same order.
JSON_FILES = [
    "castling.json",
    "checkmates.json",
    "famous.json",
    "pawns.json",
    "promotions.json",
    "stalemates.json",
    "standard.json",
    "taxing.json",
]

# tests/test_invariants.cpp CORPUS_DEPTH.
CORPUS_DEPTH = 2

# The five positions and depths of tests/test_engine.cpp's hash oracle, copied
# into tests/test_invariants.cpp.
FIVE = [
    ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4),
    ("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3),
    ("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4),
    ("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 3),
    ("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3),
]

counts = dict(makes=0, castlings=0, en_passants=0, promotions=0,
              capture_promotions=0)


def all_test_fens():
    """tests/test_helpers.hpp all_test_fens(): every start and every expected
    position from the eight suites, sorted and deduplicated."""
    result = []

    for name in JSON_FILES:
        with open(os.path.join(ASSETS, name)) as handle:
            cases = json.load(handle)

        for case in cases["testCases"]:
            result.append(case["start"]["fen"])
            for expected in case["expected"]:
                result.append(expected["fen"])

    return sorted(set(result))


def walk(board, depth):
    if depth == 0:
        return

    for move in board.legal_moves:
        counts["makes"] += 1
        if board.is_castling(move):
            counts["castlings"] += 1
        if board.is_en_passant(move):
            counts["en_passants"] += 1
        if move.promotion is not None:
            counts["promotions"] += 1
            if board.is_capture(move):
                counts["capture_promotions"] += 1

        board.push(move)
        walk(board, depth - 1)
        board.pop()


def main():
    fens = all_test_fens()
    print(f"corpus: {len(fens)} FENs at depth {CORPUS_DEPTH}", file=sys.stderr)

    for i, fen in enumerate(fens):
        walk(chess.Board(fen), CORPUS_DEPTH)
        if (i + 1) % 500 == 0:
            print(f"  {i + 1}/{len(fens)}  {counts['makes']} makes",
                  file=sys.stderr)

    for fen, depth in FIVE:
        walk(chess.Board(fen), depth)

    print("census")
    for key in ("makes", "castlings", "en_passants", "promotions",
                "capture_promotions"):
        print(f"  {key:<20} {counts[key]}")

    print("floors (half, rounded down to one significant figure)")
    for key in ("makes", "castlings", "en_passants", "promotions",
                "capture_promotions"):
        half = counts[key] // 2
        magnitude = 10 ** (len(str(half)) - 1)
        print(f"  {key:<20} {half // magnitude * magnitude}")


if __name__ == "__main__":
    main()
