#!/usr/bin/env python3
"""Re-derive the book entries the S194 goldens pin, from outside the project.

S194, closing 2026-09-04_test_review-F06. The two fast cases over the UCI book
path assert against numbers that describe the shipped book: how many moves it
offers for the start position, what they weigh in total, which one is heaviest
and by how much, and that the S175 repaired position carries exactly one entry.
Those are goldens (DEC-142) and this is the script that re-derives them, so a
rebuilt `src/openings.bin` moves the test rather than the test carrying a stale
copy of a book it no longer describes.

Read with python-chess's own Polyglot reader and its own zobrist_hash(), which
is an independent implementation of the format -- the same property that makes
`adocs/data/S175_book_conformance.py` a check from outside and not a restatement
of what make_book already believes.

Usage:
  ~/.venv/chess/bin/python adocs/data/S194_book_start_key.py [src/openings.bin]

python-chess 1.11.2 in ~/.venv/chess (TOOLCHAIN.md); the system python has no
python-chess, which is why this is not a ctest.
"""

import sys

import chess
import chess.polyglot

# The position S175 repaired, which the test drives through `position fen`: an
# edge-file en-passant square, where the 2026-09-03 book carried a key that
# wrapped round the board edge and the engine probed it happily.
S175_FEN = "rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8"


def report(reader, board, label):
    entries = list(reader.find_all(board))
    total = sum(entry.weight for entry in entries)

    print(f"{label}")
    print(f"  key      {chess.polyglot.zobrist_hash(board):016x}")
    print(f"  entries  {len(entries)}")
    print(f"  weight   {total}")

    for entry in sorted(entries, key=lambda e: (-e.weight, e.move.uci())):
        print(f"    {entry.move.uci():<6} {entry.weight}")


def main(argv):
    path = argv[1] if len(argv) > 1 else "src/openings.bin"

    with chess.polyglot.open_reader(path) as reader:
        report(reader, chess.Board(), "start position")
        print()
        report(reader, chess.Board(S175_FEN), "S175 position")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
