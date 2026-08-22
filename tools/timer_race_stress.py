#!/usr/bin/env python3
"""Stress the hard-limit timer against the session that follows it.

`stop_search_after_ms()` arms a detached thread that raises the global stop
flag when its sleep ends, unless the search session has moved on since. A move
normally ends at its soft limit, so the hard timer is still sleeping when the
next `go` arrives: stale timers are routine, one per move of every game. If the
timer's session check and its stop store are not one decision, a timer preempted
between them raises the flag after the next `go` has cleared it, and that search
dies at its first poll - depth 1, an instant reply for no reason.

This is the regression net for that. Each iteration:

    position fen <midgame>
    go movetime 1     arms a 1 ms timer, leaves it stale within a millisecond
    go depth <d>      carries no timer of its own, so it must reach depth d

A `go depth` search that reports less than depth d was stopped by something,
and the only candidate is the previous iteration's timer. The two commands land
about a millisecond apart, which is where that timer expires, so the coincidence
the race needs happens every iteration.

    taskset -c 0 tools/timer_race_stress.py ./build/src/chesso --seconds 600

`taskset -c 0` is the point: affinity is inherited by the engine, so the timer
thread and the UCI thread share one core and a preemption between the check and
the store becomes plausible instead of merely possible.

Prints TIMER-RACE-CLEAN, TIMER-RACE-HIT or TIMER-RACE-FAILED as its last line,
and exits 0, 1 or 2. All three are terminal, so a watcher armed on the
alternation exits on a crash instead of waiting out its ceiling (AGENTS.md par.12).
S163, closing 2026-08-22_adversarial-F05.
"""

import argparse
import subprocess
import sys
import time

# Out of book and out of the endgame, so `go depth` searches a real tree and
# `go movetime 1` has something to abandon.
DEFAULT_FEN = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"


class engine_t:
    def __init__(self, path):
        self.p = subprocess.Popen(
            [path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,
        )
        self.send("uci")
        while "uciok" not in self.readline():
            pass

    def send(self, line):
        self.p.stdin.write(line + "\n")
        self.p.stdin.flush()

    def readline(self):
        line = self.p.stdout.readline()
        if not line:
            raise RuntimeError("engine closed stdout")
        return line

    # Returns the deepest completed depth the search reported. One info line per
    # completed iteration since S037, and the depth field is that iteration's.
    def go(self, command):
        self.send(command)
        deepest = 0
        while True:
            line = self.readline()
            if line.startswith("info") and " depth " in line:
                deepest = max(deepest, int(line.split(" depth ")[1].split()[0]))
            if line.startswith("bestmove"):
                return deepest

    def quit(self):
        self.send("quit")
        try:
            self.p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.p.kill()


def main():
    try:
        return stress()
    except Exception as err:
        # A dead engine or a protocol surprise must reach the watcher as a
        # terminal line, not as an absence of one.
        print(f"{type(err).__name__}: {err}", flush=True)
        print("TIMER-RACE-FAILED")
        return 2


def stress():
    ap = argparse.ArgumentParser()
    ap.add_argument("engine")
    ap.add_argument("--seconds", type=int, default=60)
    ap.add_argument("--depth", type=int, default=6)
    ap.add_argument("--movetime", type=int, default=1)
    ap.add_argument("--fen", default=DEFAULT_FEN)
    args = ap.parse_args()

    eng = engine_t(args.engine)
    deadline = time.monotonic() + args.seconds
    next_report = time.monotonic() + 10.0

    started = time.monotonic()
    iterations, hits = 0, []

    while time.monotonic() < deadline:
        eng.send("position fen " + args.fen)
        eng.go(f"go movetime {args.movetime}")
        reached = eng.go(f"go depth {args.depth}")
        iterations += 1

        if reached < args.depth:
            hits.append((iterations, reached))
            print(
                f"HIT at iteration {iterations}: [go depth {args.depth}] "
                f"reported depth {reached}",
                flush=True,
            )

        now = time.monotonic()
        if now >= next_report:
            elapsed = now - started
            print(
                f"{iterations} iterations, {iterations / elapsed:.0f}/s, "
                f"{len(hits)} hits, {elapsed:.0f}s",
                flush=True,
            )
            next_report = now + 10.0

    eng.quit()

    elapsed = time.monotonic() - started
    print(
        f"{iterations} iterations over {elapsed:.1f}s "
        f"({iterations / elapsed:.0f}/s), {len(hits)} hits"
    )
    print("TIMER-RACE-HIT" if hits else "TIMER-RACE-CLEAN")
    return 1 if hits else 0


if __name__ == "__main__":
    sys.exit(main())
