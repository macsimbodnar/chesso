#!/usr/bin/env python3
"""S291. The FENs along a move line, from python-chess applying the moves (the
tool, never a mental board), '|'-joined for S289_trace.py's $S289_TRACE.

    ~/.venv/chess/bin/python adocs/data/S291_fens.py "ROOT FEN" m1 m2 ...

Prints the root's FEN, then the FEN after each move in turn. Exits 1 on an
illegal move.
"""
import sys

import chess

board = chess.Board(sys.argv[1])
fens = [board.fen()]
for uci in sys.argv[2:]:
    move = chess.Move.from_uci(uci)
    if move not in board.legal_moves:
        sys.exit("illegal: %s in %s" % (uci, board.fen()))
    board.push(move)
    fens.append(board.fen())
print("|".join(fens))
