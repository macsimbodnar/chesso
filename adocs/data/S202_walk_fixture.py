#!/usr/bin/env python3
"""Re-derive the line tests/test_search.cpp's "the walk takes a certified child
over a bound entry's move" plants in the table. S202, DEC-251.

The root is S145's mate set row `rbrb4/p1p1p1k1/P1P1P3/6K1/8/3Q4/8/8 w`, key
d3h3, proved mate in 3 there. Printed here, by python-chess enumeration and not
by judgement: each black reply after d3h3, the mates in one after g7f8, and
after g7g8 the move stockfish (depth 25) plays, the replies to it and their
mates in one. The test needs g7f8 to be mated at once and g7g8 to go
g5g6 g8f8 h3h8.

    ~/.venv/chess/bin/python adocs/data/S202_walk_fixture.py
"""

import chess
import chess.engine

ROOT = "rbrb4/p1p1p1k1/P1P1P3/6K1/8/3Q4/8/8 w - - 0 1"


def mates_in_one(board):
    out = []
    for move in list(board.legal_moves):
        board.push(move)
        if board.is_checkmate():
            out.append(move.uci())
        board.pop()
    return out


def main():
    board = chess.Board(ROOT)
    board.push_uci("d3h3")
    print("after d3h3, black replies:", [m.uci() for m in board.legal_moves])

    board.push_uci("g7f8")
    print("after g7f8, mates in one:", mates_in_one(board))
    board.pop()

    board.push_uci("g7g8")
    engine = chess.engine.SimpleEngine.popen_uci("stockfish")
    try:
        info = engine.analyse(board, chess.engine.Limit(depth=25))
    finally:
        engine.quit()
    best = info["pv"][0]
    print("after g7g8, stockfish:", info["score"].pov(board.turn),
          " ".join(m.uci() for m in info["pv"]))

    board.push(best)
    for reply in list(board.legal_moves):
        board.push(reply)
        print(f"after g7g8 {best.uci()} {reply.uci()}, mates in one:",
              mates_in_one(board))
        board.pop()


if __name__ == "__main__":
    main()
