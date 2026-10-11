#!/usr/bin/env python3
"""Send every position to a UCI engine as a bare FEN.

    ~/.venv/chess/bin/python -I tools/bare_fen_relay.py ENGINE [ENGINE_ARGS...]

Sits between a UCI client (fastchess) and the engine it launches. Each
`position startpos ...` or `position fen <fen> ...` line, with or without
`moves`, is replaced by `position fen <the board it reaches>`, with no `moves`.
Every other line goes to the engine byte for byte, and everything the engine
prints comes back byte for byte, both as soon as it arrives. The relay exits
with the engine's status when the engine exits, and closing its input closes
the engine's.

WHY IT EXISTS. S271 (DEC-264) stopped a new base FEN from clearing the
transposition table. fastchess holds the base fixed for a game, so harness
play did not change and S271 landed on node counts alone. A client that sends
the board each move -- some GUIs, python-chess on a board with no move stack --
is the one regime where it changes play, and this relay puts fastchess in that
regime so S272's SPRT can price it. Both sides of such a match go through it.

THE BOARD IS PYTHON-CHESS'S, never a hand-written one: the base is
`chess.Board(<fen>)` (or the start position), each move goes through
`push_uci`, which refuses an illegal one, and the line sent is `Board.fen()`.
So castling rights, the en-passant square, the halfmove clock and the
fullmove number are the ones python-chess computes from the moves. That
default writes the en-passant square only when a legal capture exists, which
is the form `adocs/data/S271_replay.py`'s `bare` mode sent, the replay whose
conversion S272 tests. Chesso's FEN loader keeps an en-passant square only
when a pawn of the side to move can take it (S042), so the two agree except
where that capture is pseudo-legal but illegal.

WHAT A BARE FEN CANNOT CARRY is the game's history: the engine no longer sees
the positions before the current one, so it cannot see a repetition of them.
That is the residue DEC-264 accepts. The fifty-move count is in the FEN.

A LINE IT CANNOT READ PASSES UNCHANGED, with a note: a `position` with neither
`startpos` nor `fen` (chesso's own `position kiwipete`), an invalid FEN or an
illegal move. The relay never decides what a bad position means; the engine's
own refusal rules still apply to it. The note goes to stderr, and is appended
to the file BARE_FEN_RELAY_NOTES names when that is set: fastchess discards an
engine's stderr, so `fastchess.sh` sets it and voids a run whose file is not
empty, since some position then reached an engine as base plus moves.

COST. A game's `position` lines share their base and each extends the last
one's moves, so the board is kept and only the new moves are pushed: two per
line in a game, whatever its length. The added latency, relay against direct,
is measured by `adocs/data/S272_relay_latency.py`; its reading is
`adocs/data/S272_relay_latency.txt`.

The test is `tests/test_bare_fen_relay.py`, in the fast suite.
"""

import os
import signal
import subprocess
import sys
import threading

import chess


class Rewriter:
    """Turns a `position` line into a bare FEN, keeping the last board."""

    def __init__(self):
        self.base = None
        self.moves = []
        self.board = None

    def fen_for(self, line):
        """The FEN of the board a `position` line reaches.

        None when the line is not a `position startpos|fen` line; ValueError
        (python-chess's own) when its FEN or a move cannot be played.
        """
        tokens = line.split()
        if len(tokens) < 2 or tokens[0] != "position":
            return None
        if tokens[1] == "startpos":
            base = chess.STARTING_FEN
            rest = tokens[2:]
        elif tokens[1] == "fen":
            end = tokens.index("moves") if "moves" in tokens else len(tokens)
            base = " ".join(tokens[2:end])
            rest = tokens[end:]
        else:
            return None
        if rest and rest[0] != "moves":
            return None
        moves = rest[1:]

        known = len(self.moves)
        if (self.board is not None and base == self.base
                and moves[:known] == self.moves):
            board, todo = self.board, moves[known:]
        else:
            board, todo = chess.Board(base), moves
        # The kept board is pushed in place, so a move that fails half way
        # leaves it matching no line; it is dropped before the error leaves.
        self.board = None
        for move in todo:
            board.push_uci(move)
        self.base, self.moves, self.board = base, moves, board
        return board.fen()


def note(text):
    print(text, file=sys.stderr, flush=True)
    path = os.environ.get("BARE_FEN_RELAY_NOTES")
    if path:
        with open(path, "a") as notes:
            notes.write(text + "\n")


def write_all(fd, data):
    view = memoryview(data)
    while view:
        view = view[os.write(fd, view):]


def main(argv):
    if len(argv) < 2:
        print(__doc__.split("\n\n")[1], file=sys.stderr)
        return 2
    try:
        engine = subprocess.Popen(argv[1:], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, bufsize=0)
    except OSError as error:
        print(f"bare_fen_relay: cannot start {argv[1]}: {error}", file=sys.stderr)
        return 127

    def stop(signum, _frame):
        engine.kill()
        os._exit(128 + signum)

    for signum in (signal.SIGTERM, signal.SIGINT, signal.SIGHUP):
        signal.signal(signum, stop)

    # The engine's output is copied as raw chunks, not lines: nothing in it is
    # rewritten, so nothing waits for a newline. Its end is the relay's end,
    # whatever the input side is doing.
    def pump():
        source = engine.stdout.fileno()
        while True:
            try:
                data = os.read(source, 65536)
                if not data:
                    break
                write_all(1, data)
            except OSError:
                break
        status = engine.wait()
        os._exit(status if status >= 0 else 128 - status)

    threading.Thread(target=pump, daemon=True).start()

    rewriter = Rewriter()
    sink = engine.stdin.fileno()
    while True:
        raw = sys.stdin.buffer.readline()
        if not raw:
            break
        out = raw
        if raw.lstrip().startswith(b"position"):
            text = raw.decode("ascii", "replace")
            try:
                fen = rewriter.fen_for(text)
                reason = "not position startpos|fen [moves ...]"
            except ValueError as error:
                fen, reason = None, str(error)
            if fen is not None:
                ending = raw[len(raw.rstrip(b"\r\n")):]
                out = b"position fen " + fen.encode("ascii") + ending
            else:
                note(f"bare_fen_relay: passed unchanged ({reason}): {text.strip()}")
        try:
            write_all(sink, out)
        except OSError:
            break

    # Input closed: the engine sees its own input close, and the pump ends the
    # relay when the engine has gone.
    try:
        engine.stdin.close()
    except OSError:
        pass
    threading.Event().wait()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
