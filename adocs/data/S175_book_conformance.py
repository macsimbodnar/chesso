#!/usr/bin/env python3
"""Re-derive a Polyglot book from its PGN with python-chess and compare.

S175, closing 2026-09-03_adversarial-F01. `make_book` keys positions with the
engine's own get_key(), so a book it builds is self-consistent with the engine
whether or not the key is the format's -- which is exactly why the shipped book
looked right from inside the project and carried 7 non-conforming keys. This
script is the check from outside: python-chess's chess.polyglot.zobrist_hash()
is an independent implementation of the same specification, and a book whose
every (key, move, weight) it reproduces is a book any Polyglot reader can probe.

Method, matching make_book's defaults: every game's mainline is replayed to
--max-ply plies; each (key, Polyglot move word) is weighted 2 for a win, 1 for
a draw, 0 for a loss from the mover's point of view (1 when the result is
unknown), castling is written as the king taking its own rook, entries with
weight 0 or seen in fewer than --min-games games are dropped, weights are
clamped to 65535. The multiset is compared with the .bin read as big-endian
16-byte records.

Exit 0 only when missing, extra and weight_mismatch are all 0.

Usage:
  ~/.venv/chess/bin/python adocs/data/S175_book_conformance.py \
      books/8moves_v3.pgn src/openings.bin [--max-ply 16] [--min-games 1]

python-chess 1.11.2 in ~/.venv/chess (TOOLCHAIN.md); the system python has no
python-chess, which is why this is not a ctest.
"""

import argparse
import struct
import sys
from collections import Counter, defaultdict

import chess
import chess.pgn
import chess.polyglot

PROMOTION_CODE = {chess.KNIGHT: 1, chess.BISHOP: 2, chess.ROOK: 3, chess.QUEEN: 4}
CASTLING_ROOK = {chess.G1: chess.H1, chess.C1: chess.A1, chess.G8: chess.H8, chess.C8: chess.A8}


def polyglot_move(board, move):
    """The 16-bit Polyglot move word for `move` played on `board`."""
    to_square = move.to_square
    if board.is_castling(move):
        to_square = CASTLING_ROOK.get(to_square, to_square)
    promotion = PROMOTION_CODE.get(move.promotion, 0)
    return (chess.square_file(to_square)
            | (chess.square_rank(to_square) << 3)
            | (chess.square_file(move.from_square) << 6)
            | (chess.square_rank(move.from_square) << 9)
            | (promotion << 12))


def weight_for(result, mover_is_white):
    if result == "1/2-1/2":
        return 1
    if result == "1-0":
        return 2 if mover_is_white else 0
    if result == "0-1":
        return 0 if mover_is_white else 2
    return 1


def derive(pgn_path, max_ply, min_games):
    weights = Counter()
    games_seen = Counter()
    fen_of_key = {}
    games = 0
    plies = 0
    with open(pgn_path, encoding="utf-8", errors="replace") as handle:
        while True:
            game = chess.pgn.read_game(handle)
            if game is None:
                break
            games += 1
            result = game.headers.get("Result", "*")
            board = game.board()
            for ply, move in enumerate(game.mainline_moves()):
                if ply >= max_ply:
                    break
                key = chess.polyglot.zobrist_hash(board)
                entry = (key, polyglot_move(board, move))
                weights[entry] += weight_for(result, board.turn == chess.WHITE)
                games_seen[entry] += 1
                fen_of_key.setdefault(key, board.fen(en_passant="fen"))
                board.push(move)
                plies += 1
    derived = {}
    for entry, weight in weights.items():
        if games_seen[entry] < min_games or weight == 0:
            continue
        derived[entry] = min(weight, 65535)
    return derived, fen_of_key, games, plies


def read_book(bin_path):
    book = {}
    duplicates = 0
    with open(bin_path, "rb") as handle:
        data = handle.read()
    if len(data) % 16 != 0:
        sys.exit(f"{bin_path}: size {len(data)} is not a whole number of 16-byte entries")
    for offset in range(0, len(data), 16):
        key, move, weight, _learn = struct.unpack(">QHHI", data[offset:offset + 16])
        if (key, move) in book:
            duplicates += 1
        book[(key, move)] = weight
    return book, duplicates


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("pgn")
    parser.add_argument("bin")
    parser.add_argument("--max-ply", type=int, default=16)
    parser.add_argument("--min-games", type=int, default=1)
    parser.add_argument("--examples", type=int, default=10,
                        help="how many disagreeing entries to print per class")
    args = parser.parse_args()

    derived, fen_of_key, games, plies = derive(args.pgn, args.max_ply, args.min_games)
    book, duplicates = read_book(args.bin)
    if not derived or not book:
        sys.exit(f"nothing to compare: {len(derived)} entries derived from the PGN, "
                 f"{len(book)} in the book -- an empty run is not a passing one")

    missing = sorted(set(derived) - set(book))
    extra = sorted(set(book) - set(derived))
    mismatched = sorted(e for e in derived if e in book and derived[e] != book[e])

    print(f"games {games} plies {plies} derived_entries {len(derived)} "
          f"book_entries {len(book)} distinct_positions {len({k for k, _ in derived})}")
    print(f"missing {len(missing)} extra {len(extra)} weight_mismatch {len(mismatched)} "
          f"duplicate_book_entries {duplicates}")

    for label, entries in (("missing from book", missing), ("extra in book", extra),
                           ("weight mismatch", mismatched)):
        for key, move in entries[:args.examples]:
            fen = fen_of_key.get(key, "(position not in PGN-derived set)")
            detail = f"derived {derived.get((key, move))} book {book.get((key, move))}"
            print(f"  {label}: key {key:016x} move {move:04x} {detail}  {fen}")

    return 0 if not missing and not extra and not mismatched and duplicates == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
