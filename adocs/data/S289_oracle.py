#!/usr/bin/env python3
"""S289. The oracle behind the defender-root mate-in-two case: every row of
adocs/data/S165_defender_set.tsv whose side to move is proved mated in two,
re-proved and corroborated by tools, never by reading the board (CLAUDE.md).

    ~/.venv/chess/bin/python adocs/data/S289_oracle.py > adocs/data/S289_oracle.txt

Per row: python-chess's legality facts (is_valid, is_check, legal moves), the
exhaustive AND/OR enumeration S145 wrote (adocs/data/S145_mate_set.py, iterative
in the distance, so "mated in 2" also refutes "mated in 1"), and stockfish at
S165's node budget through python-chess -- the safe invocation of TOOLCHAIN.md,
which waits for the search to answer. Stockfish corroborates and never decides:
a frozen defending army is a class its network scores badly (S145).
"""

import os
import sys

import chess
import chess.engine

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S145_mate_set as constructed  # noqa: E402

TSV = os.path.join(HERE, "S165_defender_set.tsv")
STOCKFISH_NODES = 4000000  # S165's budget, and its reason (STOCKFISH_NODES there)


def proved(board):
    for k in range(1, 6):
        memo, budget = {}, [constructed.PROOF_NODE_CAP]
        if constructed._and_mate(board, 2 * k, memo, budget):
            return k
    return None


def main():
    rows = []
    for line in open(TSV):
        if line.startswith("#") or not line.strip():
            continue
        field = line.rstrip("\n").split("\t")
        if field[0] == "fen":
            continue
        if int(field[1]) == 2:
            rows.append(field)

    print("# S289 oracle: the %d rows of adocs/data/S165_defender_set.tsv proved "
          "mated in 2" % len(rows))
    print("# fen\tply\tvalid\tcheck\tlegal\tenumerated\tstockfish")
    agree = 0
    with chess.engine.SimpleEngine.popen_uci(constructed.stockfish_path()) as sf:
        for field in rows:
            board = chess.Board(field[0])
            k = proved(board)
            info = sf.analyse(board, chess.engine.Limit(nodes=STOCKFISH_NODES))
            score = info["score"].relative
            sf_mate = score.mate() if score.is_mate() else None
            agree += (k == 2 and sf_mate == -2)
            print("%s\t%s\t%s\t%s\t%d\t%s\t%s" % (
                field[0], field[2], board.is_valid(), board.is_check(),
                board.legal_moves.count(), k,
                ("Mate(%d)" % sf_mate) if sf_mate is not None else str(score)))
    print("# enumerated mated in 2 and stockfish Mate(-2): %d of %d" % (agree, len(rows)))


if __name__ == "__main__":
    main()
