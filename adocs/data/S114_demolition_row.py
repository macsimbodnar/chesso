#!/usr/bin/env python3
"""S114. Pick the demolition row: a node far ahead statically that is lost to a
forced mate the null move can only miss once its search has no ply left.

    ~/.venv/chess/bin/python adocs/data/S114_demolition_row.py pick

WHAT THE ROW HAS TO BE, and why each condition is asked of a tool (CLAUDE.md,
DEC-023). The case in tests/test_search.cpp "the static-score term does not
hide a forced mate" drives one node through `negamax_probed` with a beta far
under its static score, so the term S114 adds is at its cap there. The node
must then be one where a pass is exactly the wrong question:

  1. the side to move is not in check -- in check no null move is made at all
     and the row would prove nothing;
  2. every legal move of the side to move allows a mate in one -- the node is
     lost to a forced mate, proved here by enumeration and not read off the
     board;
  3. after a pass the opponent has a mate in one, and **every** such mate is a
     quiet move -- no capture, no promotion -- and the opponent has no capture
     or promotion at all. Quiescence searches captures and queen promotions
     only, so a null search that has fallen to quiescence cannot see this
     mate, and one that keeps a ply sees it at once. That is the difference
     the floor and the cap are about;
  4. stockfish at depth 20, in its own process through python-chess, agrees:
     the side to move is mated in one.

WHERE THE CANDIDATES COME FROM. `adocs/data/S165_defender_set.tsv`, the 104
defender nodes of S145's constructed mates, each proved by S165's AND/OR
enumeration; the rows with `mated_in` 1 are the candidates. The side to move
there holds a box of rooks and bishops behind a locked pawn wall, which is
material the evaluation counts and the position cannot use -- the "far ahead
statically" the step file asks for, built by S145 rather than here.

THE PICK RULE, fixed before the run: every candidate passing 1 to 4, ranked by
the material lead of the side to move in `piece_value`'s own scale
(src/eval_tables.hpp: 94, 327, 308, 487, 716), largest first, ties broken by
the FEN's sort order. The engine's own static score is asserted by the case
itself, where it is the engine's word.
"""

import argparse
import os
import sys

import chess
import chess.engine

HERE = os.path.dirname(os.path.abspath(__file__))
TSV = os.path.join(HERE, "S165_defender_set.tsv")
STOCKFISH = "/usr/games/stockfish"

# piece_value in src/eval_tables.hpp, the evaluation's own material scale.
VALUE = {chess.PAWN: 94, chess.KNIGHT: 327, chess.BISHOP: 308,
         chess.ROOK: 487, chess.QUEEN: 716, chess.KING: 0}


def candidates():
    rows = []
    with open(TSV) as f:
        for line in f:
            if not line.strip() or line.startswith("#"):
                continue
            field = line.rstrip("\n").split("\t")
            if field[0] == "fen":
                continue
            if int(field[1]) == 1:
                rows.append(field[0])
    return rows


def material_lead(board):
    total = 0
    for square, piece in board.piece_map().items():
        value = VALUE[piece.piece_type]
        total += value if piece.color == board.turn else -value
    return total


def mates_in_one(board):
    found = []
    for move in list(board.legal_moves):
        board.push(move)
        mate = board.is_checkmate()
        board.pop()
        if mate:
            found.append(move)
    return found


def check(fen):
    """The four conditions, each with the reason it failed."""
    board = chess.Board(fen)
    if not board.is_valid():
        return False, "not valid"
    if board.is_check():
        return False, "in check"
    moves = list(board.legal_moves)
    if not moves:
        return False, "no legal move"
    for move in moves:
        board.push(move)
        mated = bool(mates_in_one(board))
        board.pop()
        if not mated:
            return False, f"{board.san(move)} escapes the mate in one"
    board.push(chess.Move.null())
    threat = mates_in_one(board)
    noisy = [m for m in board.legal_moves
             if board.is_capture(m) or m.promotion is not None]
    quiet_threat = [m for m in threat
                    if not board.is_capture(m) and m.promotion is None]
    after_pass = {
        "threat": [board.san(m) for m in threat],
        "noisy": [board.san(m) for m in noisy],
    }
    board.pop()
    if not threat:
        return False, "no mate in one after a pass"
    if len(quiet_threat) != len(threat):
        return False, f"a noisy mate after a pass: {after_pass['threat']}"
    if noisy:
        return False, f"a capture or promotion after a pass: {after_pass['noisy']}"
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        info = engine.analyse(board, chess.engine.Limit(depth=20))
    score = info["score"].relative
    if score.mate() != -1:
        return False, f"stockfish reads {score}"
    return True, {
        "legal": len(moves),
        "lead": material_lead(board),
        "phase_pieces": sum(1 for p in board.piece_map().values()
                            if p.piece_type in (chess.KNIGHT, chess.BISHOP,
                                                chess.ROOK, chess.QUEEN)),
        "threat": after_pass["threat"],
        "stockfish": f"{score} depth {info['depth']} nodes {info['nodes']} "
                     f"pv {' '.join(m.uci() for m in info.get('pv', []))}",
    }


def pick(_args):
    passed = []
    rows = candidates()
    print(f"# {len(rows)} candidates, mated_in 1, from {os.path.relpath(TSV)}")
    for fen in rows:
        ok, detail = check(fen)
        if ok:
            passed.append((fen, detail))
            print(f"PASS\t{fen}\tlead {detail['lead']}\tlegal {detail['legal']}"
                  f"\tthreat {','.join(detail['threat'])}"
                  f"\tstockfish {detail['stockfish']}")
        else:
            print(f"FAIL\t{fen}\t{detail}")
    passed.sort(key=lambda row: (-row[1]["lead"], row[0]))
    print(f"# {len(passed)} of {len(rows)} pass")
    if passed:
        fen, detail = passed[0]
        print(f"PICK\t{fen}\tlead {detail['lead']}\t{detail['stockfish']}")
    return 0 if passed else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("pick", help="check every candidate and print the pick")
    args = parser.parse_args()
    return {"pick": pick}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
