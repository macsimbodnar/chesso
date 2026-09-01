#!/usr/bin/env python3
"""S155. Census the constructed mate set, so its single motif is a measurement.

    python3 adocs/data/S155_motif_census.py

Reads adocs/data/S145_mate_set.tsv and prints what varies across the 48
positions and what does not. The claim the test comment and specs.md carry --
one motif, one material signature per colour, one mating piece -- is this
script's output and not an assertion anybody has to trust.

It reads the tracked TSV only. No engine, no oracle, no python-chess: the
construction and its two proofs are S145_mate_set.py's, and re-proving the set
is that script's `verify`.
"""

import collections
import pathlib
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
    return 0


if __name__ == "__main__":
    sys.exit(main())
