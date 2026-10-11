#!/usr/bin/env python3
"""What tools/bare_fen_relay.py adds to a move, relay against direct.

    ~/.venv/chess/bin/python adocs/data/S272_relay_latency.py ENGINE [GAMES [LOAD]]

    py=~/.venv/chess/bin/python; s=adocs/data/S272_relay_latency.py
    { $py $s build/src/chesso 40 0; echo; $py $s build/src/chesso 40 12; } \
        > adocs/data/S272_relay_latency.txt

Two copies of ENGINE run side by side, one direct and one through the relay,
and both are sent the same fastchess-shaped turns, alternating per turn so
machine drift lands on both alike. A turn is what fastchess sends before every
move (the session tests/assets/bare_fen_relay_session.txt recorded):
`position fen <book> moves ...`, `isready`, wait for `readyok`, then
`go nodes 1`, wait for `bestmove`. `nodes 1` keeps the search itself near
nothing, so what is left is the pipe and the relay.

Two spans per turn, because fastchess clocks only the second. It waits for
`readyok` before it sends `go` and starts the engine's clock there: an engine
held 0.3 s on every `position` line played 79 moves of a 2+0.02 game without a
forfeit (S272, 2026-10-11, `.tuning/coord/S272/rec/`). So the rewrite, which
happens on the `position` line, is paid before the clock starts, and the relay
costs the clock only the two hops `go` and `bestmove` take through it.

  handshake   `position` written to `readyok` read: the rewrite, off the clock
  clocked     `go` written to `bestmove` read: what the engine's clock sees

The positions are every ply of the first GAMES games (default 40) of
S198_calibration.pgn, from their book FENs, one `ucinewgame` per game.
LOAD (default 0) busy engines run `go infinite` beside the two for the whole
reading: 12 is a match's load on this machine, one search per game at
concurrency 12. Nanosecond wall time; medians and the 99th percentile, per
mode and as the relay's excess. The figure depends on the machine and its load; it is a
reading, not a constant.
"""

import os
import statistics
import subprocess
import sys
import time

import chess.pgn

HERE = os.path.dirname(os.path.abspath(__file__))
RELAY = os.path.join(HERE, "..", "..", "tools", "bare_fen_relay.py")


class Side:
    def __init__(self, command):
        self.proc = subprocess.Popen(command, stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, bufsize=0)
        self.out = self.proc.stdout
        self.buffer = b""

    def send(self, text):
        self.proc.stdin.write(text.encode())

    def wait_for(self, prefix):
        while True:
            newline = self.buffer.find(b"\n")
            if newline >= 0:
                line, self.buffer = self.buffer[:newline], self.buffer[newline + 1:]
                if line.startswith(prefix):
                    return
                continue
            chunk = os.read(self.out.fileno(), 65536)
            if not chunk:
                raise RuntimeError(f"engine closed while waiting for {prefix!r}")
            self.buffer += chunk

    def turn(self, position):
        start = time.perf_counter_ns()
        self.send(position + "\nisready\n")
        self.wait_for(b"readyok")
        ready = time.perf_counter_ns()
        self.send("go nodes 1\n")
        self.wait_for(b"bestmove")
        done = time.perf_counter_ns()
        return ready - start, done - ready


def games(count):
    with open(os.path.join(HERE, "S198_calibration.pgn")) as handle:
        for _ in range(count):
            game = chess.pgn.read_game(handle)
            yield game.headers["FEN"], [move.uci() for move in game.mainline_moves()]


def loadavg():
    try:
        return open("/proc/loadavg").read().split()[0]
    except OSError:
        return "unread"


def spread(values_ns):
    ordered = sorted(values_ns)
    p99 = ordered[min(len(ordered) - 1, int(0.99 * len(ordered)))]
    return statistics.median(ordered) / 1e3, p99 / 1e3


def main(argv):
    if len(argv) not in (2, 3, 4):
        print(__doc__.split("\n\n")[1], file=sys.stderr)
        return 2
    engine = argv[1]
    count = int(argv[2]) if len(argv) >= 3 else 40
    load = int(argv[3]) if len(argv) == 4 else 0
    load_before = loadavg()
    busy = [subprocess.Popen([engine], stdin=subprocess.PIPE, stdout=subprocess.DEVNULL)
            for _ in range(load)]
    for proc in busy:
        proc.stdin.write(b"position startpos\ngo infinite\n")
        proc.stdin.flush()
    sides = {"direct": Side([engine]),
             "relay": Side([sys.executable, "-I", RELAY, engine])}
    spans = {name: ([], []) for name in sides}
    for side in sides.values():
        side.send("uci\n")
        side.wait_for(b"uciok")
        side.send("setoption name Hash value 16\nisready\n")
        side.wait_for(b"readyok")

    turns = 0
    for base, moves in games(count):
        for side in sides.values():
            side.send("ucinewgame\nisready\n")
            side.wait_for(b"readyok")
        for ply in range(len(moves)):
            position = f"position fen {base}" + (f" moves {' '.join(moves[:ply])}" if ply else "")
            order = ("direct", "relay") if ply % 2 == 0 else ("relay", "direct")
            for name in order:
                handshake, clocked = sides[name].turn(position)
                spans[name][0].append(handshake)
                spans[name][1].append(clocked)
            turns += 1
    for side in sides.values():
        side.send("quit\n")
        side.proc.wait(timeout=10)
    for proc in busy:
        proc.kill()
        proc.wait()

    print(f"# S272 relay latency: {turns} turns, every ply of {count} games of "
          f"S198_calibration.pgn, `go nodes 1`, Hash 16, {load} busy engines beside")
    print(f"# engine {engine}, relay on {sys.executable} -I, as fastchess.sh launches it")
    # The one-minute average lags by about a minute, so it is printed at both
    # ends: an "idle" reading taken while it is still falling from a loaded
    # one is not idle, and says so here.
    print(f"# one-minute load average {load_before} at the start, {loadavg()} at the end")
    print()
    print(f"{'span':10} {'mode':7} {'median us':>10} {'p99 us':>10}")
    for index, span in enumerate(("handshake", "clocked")):
        for name in sides:
            median, p99 = spread(spans[name][index])
            print(f"{span:10} {name:7} {median:10.1f} {p99:10.1f}")
        excess = [r - d for r, d in zip(spans["relay"][index], spans["direct"][index])]
        median, p99 = spread(excess)
        print(f"{span:10} {'excess':7} {median:10.1f} {p99:10.1f}   (relay minus direct, per turn)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
