#!/usr/bin/env python3
r"""Write the session tests/test_bare_fen_relay.py replays through the relay.

    ~/.venv/chess/bin/python adocs/data/S272_relay_session.py RECORDED_LOG \
        > tests/assets/bare_fen_relay_session.txt

RECORDED_LOG is what fastchess sent one engine, taken through a `tee` wrapper
(S272, 2026-10-11: fastchess alpha 1.8.1, two games at 1+0.01 from
`noob_3moves.epd`, Hash 16): `uci`, the two `setoption` lines, `ucinewgame`,
the `isready` pings, `position fen <book> moves ...` before every `go`, and
`stop` and `quit` at the end. It is kept whole up to its final `stop`.

Then two games from tracked PGNs, a `position` line at every ply so the board
right after each move is sent: S018_match.pgn game 155, from the start
position (`position startpos moves ...`), which castles kingside on both sides,
captures en passant and promotes; and S198_calibration.pgn game 593, from its
book FEN, which castles queenside and under-promotes. The games were found by
python-chess over those files, not chosen by reading them, and this script
refuses either if it no longer carries what it was picked for. The session
ends with `go infinite`, `stop` and `quit`.

Lines starting with `#` are this header in the output; the test drops them.
"""

import os
import sys

import chess
import chess.pgn

HERE = os.path.dirname(os.path.abspath(__file__))
GO = "go wtime 1000 btime 1000 winc 10 binc 10"


def nth_game(path, number):
    with open(path) as handle:
        for _ in range(number - 1):
            chess.pgn.skip_game(handle)
        return chess.pgn.read_game(handle)


def features(game):
    board, found = game.board(), set()
    for move in game.mainline_moves():
        if board.is_kingside_castling(move):
            found.add("kingside castling")
        if board.is_queenside_castling(move):
            found.add("queenside castling")
        if board.is_en_passant(move):
            found.add("en passant")
        if move.promotion:
            found.add("promotion" if move.promotion == chess.QUEEN else "under-promotion")
        board.push(move)
    return found


def game_lines(game, startpos):
    moves = [move.uci() for move in game.mainline_moves()]
    base = "position startpos" if startpos else "position fen " + game.headers["FEN"]
    lines = ["ucinewgame", "isready"]
    for ply in range(len(moves) + 1):
        lines.append(base + ("" if ply == 0 else " moves " + " ".join(moves[:ply])))
        lines += ["isready", GO]
    return lines


def main(argv):
    if len(argv) != 2:
        print(__doc__.split("\n\n")[1], file=sys.stderr)
        return 2
    recorded = open(argv[1]).read().splitlines()
    cut = len(recorded) - 1 - recorded[::-1].index("stop")

    picks = [("S018_match.pgn", 155, True, {"kingside castling", "en passant", "promotion"}),
             ("S198_calibration.pgn", 593, False, {"queenside castling", "under-promotion"})]
    out = [line for line in __doc__.strip().splitlines()]
    out = ["# " + line if line else "#" for line in out]
    out += recorded[:cut]
    for name, number, startpos, wanted in picks:
        game = nth_game(os.path.join(HERE, name), number)
        if "FEN" in game.headers and startpos:
            sys.exit(f"{name} game {number} has a FEN header; it was picked for startpos")
        missing = wanted - features(game)
        if missing:
            sys.exit(f"{name} game {number} no longer carries {sorted(missing)}")
        out += game_lines(game, startpos)
    out += ["position startpos moves e2e4", "isready", "go infinite", "stop", "quit"]
    print("\n".join(out))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
