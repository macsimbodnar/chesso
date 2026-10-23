#!/usr/bin/env python3
"""S291. S145's 186 defender nodes as an S289_rows.py set file, each node's
mated distance proved by S145's exhaustive AND/OR enumeration
(adocs/data/S145_mate_set.py's _and_mate, iterative in the distance, so
"mated in k" also refutes every distance below k) -- the method
adocs/data/S289_reach.txt names for the same 186 and S289_oracle.py's
`proved()`. Tools only: no position is read off a board (CLAUDE.md).

    ~/.venv/chess/bin/python adocs/data/S291_defender186.py OUT.tsv

OUT lines: fen<TAB>-k<TAB>2k + 8 (S289_reach.txt's depth rule for a defender
node). A node the enumeration cannot prove within k <= 5 and S145's node cap is
an error, not a row.
"""

import os
import sys

import chess

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S145_mate_set as constructed  # noqa: E402


def proved(board):
    for k in range(1, 6):
        memo, budget = {}, [constructed.PROOF_NODE_CAP]
        if constructed._and_mate(board, 2 * k, memo, budget):
            return k
    return None


def main():
    nodes = []
    for row in constructed.read_tsv():
        nodes.extend(row["defender_nodes"])
    out = []
    for fen in nodes:
        k = proved(chess.Board(fen))
        if k is None:
            sys.exit("not proved: " + fen)
        out.append("%s\t%d\t%d\n" % (fen, -k, 2 * k + 8))
    with open(sys.argv[1], "w") as handle:
        handle.writelines(out)
    counts = {}
    for line in out:
        k = -int(line.split("\t")[1])
        counts[k] = counts.get(k, 0) + 1
    print("%d defender nodes, mated in: %s" % (
        len(out), " ".join("%d x%d" % (k, counts[k]) for k in sorted(counts))))


if __name__ == "__main__":
    main()
