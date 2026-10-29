#!/usr/bin/env python3
"""S121, DEC-326 (6): the constructive fallback for the coverage rows.

adocs/data/S121_coverage_rows.cpp closes every mobility cell from
all_test_fens() and then from S083's corpus. A cell neither reaches -- a queen
on 22 to 27 squares, a rook on 14, a bishop on 13, a knight on none, at a phase
that weights both halves and clear of the lazy clamp -- is closed from here: a
deterministic search over legal positions with the count the cell names. It is
a tool and not a hand pick. Written 2026-10-28 in S121's phase 1a, before it
ran on any target; phase 1b runs it only for the cells the coverage tool
prints UNCLOSED, and only if it fails too does phase 1b stop for an owner
ruling.

    ~/.venv/chess/bin/python adocs/data/S121_construct_rows.py \\
        --cells queen:22-27,rook:14,bishop:13 [--parts MASK] [--per-cell N] \\
        [--tries N] [--seed S] > constructed.tsv

THE SEARCH, per cell (piece, count), in the order given: a seeded generator
(random.Random(f"{seed} {piece} {count}"), so each cell's search is its own
and reproducible) draws, up to --tries times, a position of a White piece of
that type on a square, both kings, and 0 to 6 White pawns on ranks 2 to 7 --
nothing else, so the phase is the piece's own (1, 2 or 4: between 1 and 23,
both halves weighted) and stage two stays small. A draw is kept when
python-chess says the board `is_valid()` -- kings apart, the side not to move
not in check, no pawn on a back rank -- White is to move and not in check, and
the piece's count under the parts mask is the cell's. The first --per-cell
kept positions (4 by default) are printed, `fen<TAB>piece<TAB>count`, and a
cell with none after --tries draws is printed on stderr as UNCONSTRUCTED and
the exit is 1.

THE COUNT, from src/evaluation.hpp's definitions and with python-chess's
attack sets, not the engine's: the squares the piece attacks that are in the
area -- every square but an own piece's (mask 0); with bit 2 (P2a) every
square but an own blocked pawn's, a pawn whose push square holds any piece;
with bit 8 (P2b) not an own pawn on its own second or third rank either; with
bit 1 (P1) not a square a Black pawn attacks; with bit 4 (P3), for a piece
pinned to its own king, only the squares on the pin line (python-chess
`pin()`). The engine's own trace re-checks every row the coverage tool takes
from this file: it applies the same eligibility -- phase, clamp, both halves
-- with evaluate() as it is built, so a count this file got wrong closes no
cell and is never written into the fixture.
"""

import argparse
import random
import sys

import chess

PIECES = {"knight": chess.KNIGHT, "bishop": chess.BISHOP, "rook": chess.ROOK,
          "queen": chess.QUEEN}
WIDTH = {"knight": 9, "bishop": 14, "rook": 15, "queen": 28}


def count_of(board, square, parts):
    """The mobility count of the White piece on `square` under `parts`."""
    attacks = int(board.attacks(square))
    own = int(board.occupied_co[chess.WHITE])
    pawns = int(board.pieces(chess.PAWN, chess.WHITE))
    if parts & 2:
        excluded = 0
        for p in chess.SquareSet(pawns):
            push = p + 8
            if push < 64 and board.piece_at(push) is not None:
                excluded |= chess.BB_SQUARES[p]
            if parts & 8 and chess.square_rank(p) in (1, 2):
                excluded |= chess.BB_SQUARES[p]
        area = ~excluded & chess.BB_ALL
    else:
        area = ~own & chess.BB_ALL
    if parts & 1:
        for p in board.pieces(chess.PAWN, chess.BLACK):
            area &= ~int(board.attacks(p))
    if parts & 4 and board.is_pinned(chess.WHITE, square):
        attacks &= int(board.pin(chess.WHITE, square))
    return bin(attacks & area).count("1")


def draw(rng, piece_type):
    board = chess.Board(None)
    square = rng.randrange(64)
    board.set_piece_at(square, chess.Piece(piece_type, chess.WHITE))
    free = [s for s in range(64) if s != square]
    rng.shuffle(free)
    board.set_piece_at(free.pop(), chess.Piece(chess.KING, chess.WHITE))
    board.set_piece_at(free.pop(), chess.Piece(chess.KING, chess.BLACK))
    for _ in range(rng.randrange(7)):
        s = free.pop()
        if 1 <= chess.square_rank(s) <= 6:
            board.set_piece_at(s, chess.Piece(chess.PAWN, chess.WHITE))
    board.turn = chess.WHITE
    return board, square


def cells_of(text):
    out = []
    for item in text.split(","):
        name, _, counts = item.partition(":")
        if name not in PIECES or not counts:
            sys.exit(f"--cells: '{item}' is not piece:count or piece:lo-hi")
        lo, _, hi = counts.partition("-")
        lo, hi = int(lo), int(hi or lo)
        if not 0 <= lo <= hi < WIDTH[name]:
            sys.exit(f"--cells: {item} is outside the {name}'s row")
        out += [(name, c) for c in range(lo, hi + 1)]
    return out


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--cells", required=True)
    parser.add_argument("--parts", type=int, default=0)
    parser.add_argument("--per-cell", type=int, default=4)
    parser.add_argument("--tries", type=int, default=200000)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args(argv)
    if not (0 <= args.parts <= 15 and (args.parts & 8 == 0 or args.parts & 2)):
        sys.exit("--parts is a mask, 0 to 15, bit 8 only beside bit 2")
    missing = []
    for name, count in cells_of(args.cells):
        rng = random.Random(f"{args.seed} {name} {count}")
        kept = 0
        for _ in range(args.tries):
            board, square = draw(rng, PIECES[name])
            if not board.is_valid() or board.is_check():
                continue
            if count_of(board, square, args.parts) != count:
                continue
            print(f"{board.fen()}\t{name}\t{count}")
            kept += 1
            if kept == args.per_cell:
                break
        if kept == 0:
            missing.append(f"{name} {count}")
            print(f"UNCONSTRUCTED {name} {count}: none in {args.tries} draws",
                  file=sys.stderr)
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
