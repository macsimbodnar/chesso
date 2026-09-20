#!/usr/bin/env python3
"""S095. Mine one mate position for `tests/test_search.cpp` "pruning does not
hide a forced mate", chosen so that **an unguarded** no-table-move reduction
term loses the mate where the shipped, guarded build finds it.

    ~/.venv/chess/bin/python adocs/data/S095_mine_mate_row.py candidates \
        --labelled .tuning/coord/S230_labelled.txt
    # sweep both builds with S230's own driver, which this file does not copy:
    ~/.venv/chess/bin/python adocs/data/S230_mine_r01_row.py depths \
        --fens .tuning/coord/S095_candidates.fen \
        --out .tuning/coord/S095_shipped.txt --lo 3 --hi 12
    #   ... apply the unguarded mutation, rebuild, sweep again to S095_open.txt
    ~/.venv/chess/bin/python adocs/data/S095_mine_mate_row.py pick \
        --shipped .tuning/coord/S095_shipped.txt \
        --open .tuning/coord/S095_open.txt

WHY THIS EXISTS. S095 adds one ply of late move reduction at a node whose
transposition-table entry carries no move. More reduction on a late quiet is the
class of change that hides a mate -- null move pruning hid a mate in two by
reducing to depth 0, late move reduction reduced the mating move at the root,
and both were caught by a mate case and by no benchmark (CLAUDE.md). The step's
accepts asks for a position where the extra ply would hide a mate, **observed
red with the term unguarded**. A row like that cannot be chosen by reading a
board (CLAUDE.md's CHESS rule); it is measured, and this file is the
measurement.

THE MUTATION THE RED IS OBSERVED UNDER. The guard is one comparison at the one
call site in `src/search.cpp` `negamax`:

    const bool no_tt_move = tt_move == 0;      ->      = true;

which applies the term at every node whether or not the entry carries a move --
the "unguarded" form the accepts names. It orphans nothing, so the Release build
with `-Werror` still compiles it, which the obvious `if (true)` inside
`lmr_node_adjustment` does not: that leaves the parameter unused. A second,
stronger form is `adjustment += 3 * LMR_NO_TT_MOVE` in the same function, for a
position that one extra ply does not reach. Applied by hand, observed, reverted
-- the S033 protocol -- and never committed.

WHERE THE POSITIONS COME FROM. This project's own games and nothing else
(DEC-016), through S230's pool: `adocs/data/S145_mined_set.tsv`,
`adocs/data/S145_mate_set.tsv` and the tail of every game of
`adocs/data/S219_aa_calibration.pgn`. No published mate collection and no other
engine's positions. Stockfish is run as a binary and labels them; nothing is
copied from it.

WHAT THIS FILE ADDS TO S230 AND WHAT IT REUSES. It **imports**
`S230_mine_r01_row` rather than editing or copying it -- this directory is
append-only, and S148 importing S145's sweep is the precedent. Reused whole:
the pool, the node-limited shortlist, the fresh-process oracle call, and the
depth sweep with its throwaway driver, which is `search_fen()` of the search
suite line for line and not `go depth N` (the two disagree on exactly this
class; S230's header says why). What is S095's own is the filter -- the mating
line has to carry a move of the class this rule reduces -- and the pick rule
below.

THE FILTER. Late move reduction only ever reduces a move that is quiet, not a
promotion, not a check, at a node not in check, past the third legal move
(`may_reduce` in `src/search.cpp`). The oracle's line cannot say what move
number a move will have in this engine's ordering, so the filter is the part it
can answer: **the mating side plays a quiet, non-checking, non-promotion move on
the principal variation, at a node where it is not in check**. Everything else
is left to the sweep, which is the only thing that can say whether the extra ply
actually hides the mate.

THE PICK RULE, stated before the sweep runs (DEC-209 clause 4). A candidate is
taken when the shipped build reports the mate at some depth in the swept range
and the unguarded build does not report it at that same depth; the row's depth
is **the lowest such depth**, and the row's `mate_in` is the distance the
shipped build reports there. What it must never be is a depth picked because the
row passes there. Where several candidates qualify, take the one whose shipped
profile is the longest run of consecutive depths -- a row that holds over four
depths is a row an ordinary ordering change will not silently take away.
"""

import argparse
import csv
import os
import sys

import chess

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S230_mine_r01_row as S230  # noqa: E402

REPO = S230.REPO
SCRATCH = S230.SCRATCH

CANDIDATES = os.path.join(SCRATCH, "S095_candidates.tsv")
CANDIDATE_FENS = os.path.join(SCRATCH, "S095_candidates.fen")


def quiets_on_line(fen, pv_uci):
    """The moves of the principal variation that the mating side plays and that
    late move reduction is allowed to touch: quiet, no promotion, no check
    given, and not played out of check. python-chess replays the line; no
    position is tracked by hand (CLAUDE.md)."""
    board = chess.Board(fen)
    mover = board.turn
    out = []

    for text in pv_uci.split():
        move = chess.Move.from_uci(text)

        if board.turn == mover and not board.is_check():
            quiet = (not board.is_capture(move) and move.promotion is None
                     and not board.gives_check(move))

            if quiet:
                out.append(board.san(move))

        board.push(move)

    return out


def cmd_candidates(args):
    """Every labelled mate whose line carries a move of the reducible class."""
    rows = []

    with open(args.labelled) as handle:
        for text in handle:
            text = text.strip()

            if not text:
                continue

            mate, fen = text.split("\t", 1)
            rows.append((int(mate), fen))

    kept = []

    for index, (_, fen) in enumerate(rows):
        facts = S230.line_facts(fen, args.depth)

        if facts is None:
            continue

        quiets = quiets_on_line(fen, facts["pv_uci"])

        if not quiets:
            continue

        facts["quiets"] = " ".join(quiets)
        kept.append(facts)
        print(f"  {facts['mate']}\t{facts['quiets']}\t{fen}", flush=True)

        if index % 50 == 0:
            print(f"  ..{index}/{len(rows)} kept {len(kept)}", flush=True)

    os.makedirs(os.path.dirname(args.out), exist_ok=True)

    with open(args.out, "w", newline="") as handle:
        handle.write(
            "# S095 candidates for a mate row the unguarded no-table-move\n"
            "# reduction hides. Regenerate: S230 pool / label, then\n"
            "# adocs/data/S095_mine_mate_row.py candidates / pick.\n"
            "# distance, pv and nodes are stockfish through python-chess, one\n"
            "# fresh process per position at a fixed depth.\n")
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["fen", "mate_in", "nodes", "pv_uci", "pv_san",
                         "quiets_on_line"])

        for row in kept:
            writer.writerow([row["fen"], row["mate"], row["nodes"],
                             row["pv_uci"], row["pv_san"], row["quiets"]])

    with open(args.fens, "w") as handle:
        for row in kept:
            handle.write(row["fen"] + "\n")

    print(f"survivors {len(kept)} -> {args.out} and {args.fens}")
    return 0


def longest_run(found):
    """The longest run of consecutive depths in a `d7 d9 d10 d11` profile."""
    depths = sorted(int(cell[1:]) for cell in found.split() if cell != "none")
    best = run = 0
    previous = None

    for depth in depths:
        run = run + 1 if previous is not None and depth == previous + 1 else 1
        best = max(best, run)
        previous = depth

    return best


def cmd_pick(args):
    """The rule of this file's header, applied by the script and not by eye."""
    shipped = S230.read_sweep(args.shipped, args.lo)
    unguarded = S230.read_sweep(args.open, args.lo)
    taken = []

    for fen, found in shipped.items():
        if found == "none":
            continue

        lost = [cell for cell in found.split()
                if cell not in unguarded.get(fen, "none").split()]

        if not lost:
            continue

        first = sorted(int(cell[1:]) for cell in lost)[0]
        taken.append((longest_run(found), -first, first, fen, found,
                      unguarded.get(fen, "none")))

    taken.sort(reverse=True)

    for run, _, depth, fen, found, other in taken:
        print(f"depth {depth}\trun {run}\tshipped [{found}]\t"
              f"unguarded [{other}]\t{fen}")

    if not taken:
        print("no candidate separates the unguarded term at any swept depth")
        return 1

    run, _, depth, fen, found, other = taken[0]
    print(f"\ntake: {fen}\n  depth {depth}, shipped profile [{found}], "
          f"unguarded [{other}], longest consecutive run {run}")
    return 0


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="stage", required=True)

    one = sub.add_parser("candidates")
    one.add_argument("--labelled", default=S230.LABELLED)
    one.add_argument("--depth", type=int, default=20)
    one.add_argument("--out", default=CANDIDATES)
    one.add_argument("--fens", default=CANDIDATE_FENS)
    one.set_defaults(run=cmd_candidates)

    two = sub.add_parser("pick")
    two.add_argument("--shipped", required=True)
    two.add_argument("--open", required=True)
    two.add_argument("--lo", type=int, default=3)
    two.set_defaults(run=cmd_pick)

    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
