#!/usr/bin/env python3
"""S155. Census the constructed mate set, so its single motif is a measurement.

    python3 adocs/data/S155_motif_census.py
    python3 adocs/data/S155_motif_census.py --moves   # needs build/tests/debug_perft_app

Reads adocs/data/S145_mate_set.tsv and prints what varies across the 48
positions and what does not. The claim the test comment and specs.md carry --
one motif, one material signature per colour, one mating piece -- is this
script's output and not an assertion anybody has to trust.

The census itself reads the tracked TSV only. No engine, no oracle, no
python-chess: the construction and its two proofs are S145_mate_set.py's, and
re-proving the set is that script's `verify`.

`--moves` is the one part that asks a tool, because "no pawn here can promote"
is a claim about legal moves and not about a file's columns. It runs the
engine's own generator over every root and every guarded defender node at
depth 1 and counts what comes back. Measured 2026-09-01 over 152 positions:
1292 legal moves, **0 pawn moves and 0 promotions**. The pawns are mutually
blocked on non-adjacent files, so neither side has a pawn move to make at any
node the suite tests.
"""

import collections
import pathlib
import subprocess
import sys

TSV = pathlib.Path(__file__).with_name("S145_mate_set.tsv")


def rows():
    lines = [l.rstrip("\n") for l in TSV.read_text().splitlines()
             if not l.startswith("#")]
    header = lines[0].split("\t")
    return header, [dict(zip(header, l.split("\t"))) for l in lines[1:]]


def signature(fen):
    """Material signature: piece letters and counts, colour kept."""
    board = fen.split()[0]
    count = collections.Counter(c for c in board if c.isalpha())
    return " ".join(f"{p}{count[p]}" for p in sorted(count))


def pawn_files(fen):
    """Files carrying a pawn of either colour, as letters."""
    board = fen.split()[0]
    files = set()
    for rank_index, rank in enumerate(board.split("/")):
        file_index = 0
        for c in rank:
            if c.isdigit():
                file_index += int(c)
            else:
                if c in "Pp":
                    files.add("abcdefgh"[file_index])
                file_index += 1
    return "".join(sorted(files))


def mating_force(fen):
    """The side to move is the mating side; its non-king, non-pawn pieces."""
    board, side = fen.split()[0], fen.split()[1]
    keep = str.isupper if side == "w" else str.islower
    count = collections.Counter(c for c in board
                                if c.isalpha() and keep(c) and c.upper() not in "KP")
    return " ".join(f"{p}{count[p]}" for p in sorted(count)) or "(none)"


def legal_moves(fen, engine):
    """Every legal move the engine generates at this position, as strings."""
    out = subprocess.run([engine, "1", fen], capture_output=True, text=True,
                         check=True).stdout
    moves = []
    for line in out.splitlines():
        field = line.split()
        if len(field) == 2 and field[0][:1] in "abcdefgh":
            moves.append(field[0])
    return moves


def squares(fen):
    """FEN board as {square name: piece letter}."""
    out = {}
    for rank_index, rank in enumerate(fen.split()[0].split("/")):
        file_index = 0
        for c in rank:
            if c.isdigit():
                file_index += int(c)
            else:
                out["abcdefgh"[file_index] + str(8 - rank_index)] = c
                file_index += 1
    return out


def pawn_move_census(table, engine):
    """Ask the generator, over the roots and the guarded defender nodes."""
    fens = []
    for row in table:
        fens.append(row["fen"])
        fens += [f for f in row["defender_nodes"].split("|") if f]

    total = pawns = promotions = 0
    for fen in fens:
        board = squares(fen)
        for move in legal_moves(fen, engine):
            total += 1
            if len(move) == 5:
                promotions += 1
            if board.get(move[:2], "").upper() == "P":
                pawns += 1

    print(f"legal moves over {len(fens)} roots and defender nodes: {total}")
    print(f"  of which pawn moves: {pawns}")
    print(f"  of which promotions: {promotions}")


def tally(name, values):
    print(f"{name}:")
    for value, n in collections.Counter(values).most_common():
        print(f"  {n:3d}  {value}")


def main():
    header, table = rows()
    if "fen" not in header:
        print(f"{TSV}: no fen column, header is {header}", file=sys.stderr)
        return 1

    print(f"{len(table)} positions in {TSV.name}\n")
    tally("material signature", (signature(r["fen"]) for r in table))
    tally("mating force (side to move)", (mating_force(r["fen"]) for r in table))
    tally("pawn files", (pawn_files(r["fen"]) for r in table))
    tally("material lead of the mated side", (r["lead"] for r in table))
    tally("family", (r["family"] for r in table))
    tally("proved mate distance", (r["distance"] for r in table))

    if "--moves" in sys.argv[1:]:
        engine = pathlib.Path("build/tests/debug_perft_app")
        if not engine.exists():
            print(f"{engine}: not built, so the move census is skipped",
                  file=sys.stderr)
            return 1
        print()
        pawn_move_census(table, str(engine))

    return 0


if __name__ == "__main__":
    sys.exit(main())
