#!/usr/bin/env python3
"""Report time forfeits per engine and decide whether a rating run stands.

    tools/forfeit_report.py <games.pgn> [--max-pct 1.0]

Exits 0 if every engine's forfeit rate is at or below the threshold, 1 if any
engine is over it, and 2 if the PGN cannot be read or names no games.

WHY A RATE AND WHY PER ENGINE. S087 ran zero-forfeit: one forfeit voided a run.
S088 measured Stash v21.0 forfeiting 3 of its 668 games at concurrency 12 and 1
of 668 at concurrency 6, and the owner chose to tolerate a low rate rather than
spend a night per void (DEC-075, DEC-076).

The denominator is the engine's OWN games, not the run's. 3 forfeits in 3340 is
0.09 % overall but 0.45 % of Stash's 668, so a whole-run threshold of 0.5 %
would tolerate sixteen forfeits concentrated on one engine while printing a
comfortable number.

A forfeit is not always what it looks like. S088's single concurrency-6 forfeit
was a 25360 ms hang after 137 normal moves, against 149, 1118 and 1309 ms of
ordinary margin overrun at concurrency 12. The overruns are printed for that
reason: the rate alone cannot tell a thin Move Overhead from a broken search.

This does NOT cover crashes, disconnects, illegal moves or stalls. Those are a
broken instrument rather than a game result and still void a run at zero;
rating.sh checks them separately.
"""

import argparse
import collections
import re
import sys

RESULT = re.compile(r'\[Result "([^"]*)"\]')
WHITE = re.compile(r'\[White "([^"]*)"\]')
BLACK = re.compile(r'\[Black "([^"]*)"\]')
OVERRUN = re.compile(r"loses on time \((\d+)ms overrun\)")
FORFEIT = '[Termination "time forfeit"]'


def scan(text):
    """Return (played, forfeited, gained, overruns) counted over whole games."""
    played = collections.Counter()
    forfeited = collections.Counter()
    gained = collections.Counter()
    overruns = []
    for game in re.split(r"\n(?=\[Event )", text):
        w, b = WHITE.search(game), BLACK.search(game)
        if not (w and b):
            continue
        w, b = w.group(1), b.group(1)
        played[w] += 1
        played[b] += 1
        if FORFEIT not in game:
            continue
        result = RESULT.search(game)
        if not result:
            continue
        loser, winner = (w, b) if result.group(1) == "0-1" else (b, w)
        forfeited[loser] += 1
        gained[winner] += 1
        ms = OVERRUN.search(game)
        overruns.append((loser, winner, int(ms.group(1)) if ms else None))
    return played, forfeited, gained, overruns


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pgn")
    ap.add_argument("--max-pct", type=float, default=1.0)
    args = ap.parse_args()

    try:
        with open(args.pgn, encoding="utf-8", errors="replace") as f:
            played, forfeited, gained, overruns = scan(f.read())
    except OSError as exc:
        print(f"cannot read {args.pgn}: {exc}", file=sys.stderr)
        return 2

    if not played:
        print(f"{args.pgn} names no games", file=sys.stderr)
        return 2

    for loser, winner, ms in overruns:
        shown = f"{ms}ms" if ms is not None else "unknown"
        print(f"  forfeit: {loser} lost to {winner}, overrun {shown}")
    if overruns:
        print()

    over = []
    for name in sorted(played):
        n = played[name]
        f = forfeited[name]
        pct = 100.0 * f / n
        flag = ""
        if pct > args.max_pct:
            flag = f"  <-- OVER {args.max_pct} %, run is void"
            over.append(name)
        print(f"  {name:<16} {n:5d} games, {f} forfeits, {pct:.2f} %{flag}")

    if gained:
        print()
        for name in sorted(gained):
            print(f"  {name} gained {gained[name]} point(s) from forfeits")

    if over:
        print()
        print(f"FORFEIT-RATE-EXCEEDED: {', '.join(over)}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
