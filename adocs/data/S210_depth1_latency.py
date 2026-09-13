#!/usr/bin/env python3
"""Depth-1 wall time over the S018 corpus, for 2026-09-10_adversarial-F21.

F21's exposure argument is that the first iteration is short enough that
nothing in ordinary play ever noticed it could not be stopped: the reviewer
measured median 0.56 ms, p99 1.04 ms and max 1.34 ms over 400 positions from
`adocs/data/S018_raw.tsv`. S210 makes the first iteration stoppable, so the
same number is what says whether that changed anything a match can see -- an
abort that fires inside a 0.5 ms iteration fires in no game at this time
control.

Measured the way a GUI sees it: one engine process, `position fen` then
`go depth 1`, wall clock from the write to the `bestmove` line, so the
pipe and the parse are inside the number. `ucinewgame` between positions
so each one is searched cold and the order of the file does not matter.

    ~/.venv/chess/bin/python adocs/data/S210_depth1_latency.py ./build/src/chesso

Prints the count, median, p95, p99 and max in milliseconds, and the whole
sorted list is written to the path given by --out when one is passed.
"""

import argparse
import statistics
import subprocess
import sys
import time
from pathlib import Path

CORPUS = Path(__file__).with_name("S018_raw.tsv")


def positions(limit):
    seen = []

    with CORPUS.open() as handle:
        header = handle.readline().rstrip("\n").split("\t")
        column = header.index("fen")

        for line in handle:
            fields = line.rstrip("\n").split("\t")

            if len(fields) <= column:
                continue

            fen = fields[column]

            if fen and fen not in seen:
                seen.append(fen)

            if len(seen) >= limit:
                break

    return seen


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("engine")
    parser.add_argument("--positions", type=int, default=400)
    parser.add_argument("--out")
    args = parser.parse_args()

    fens = positions(args.positions)

    if len(fens) < args.positions:
        print(f"only {len(fens)} distinct FENs in {CORPUS}", file=sys.stderr)

    engine = subprocess.Popen(
        [args.engine],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        bufsize=1,
    )

    engine.stdin.write("uci\n")
    while "uciok" not in engine.stdout.readline():
        pass

    times_ms = []

    for fen in fens:
        engine.stdin.write(f"ucinewgame\nposition fen {fen}\n")
        started = time.perf_counter()
        engine.stdin.write("go depth 1\n")

        while not engine.stdout.readline().startswith("bestmove"):
            pass

        times_ms.append((time.perf_counter() - started) * 1000.0)

    engine.stdin.write("quit\n")
    engine.wait()

    times_ms.sort()

    def percentile(fraction):
        index = min(len(times_ms) - 1, int(round(fraction * (len(times_ms) - 1))))
        return times_ms[index]

    print(f"positions {len(times_ms)}")
    print(f"median    {statistics.median(times_ms):.2f} ms")
    print(f"p95       {percentile(0.95):.2f} ms")
    print(f"p99       {percentile(0.99):.2f} ms")
    print(f"max       {times_ms[-1]:.2f} ms")

    if args.out:
        Path(args.out).write_text("\n".join(f"{ms:.3f}" for ms in times_ms) + "\n")


if __name__ == "__main__":
    main()
