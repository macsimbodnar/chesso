#!/usr/bin/env python3
"""Read a fastchess log for its mating-PV warnings, per engine and per class.

S203. `adocs/data/S170_cases.tsv` is six eviction reproductions mined from a
match's `Incomplete mating PV` warnings, and it is valid only for the Zobrist
key set it was mined under (DEC-154). This is the reader that turns a fresh
match into candidate material.

    adocs/data/S203_mine_cases.py <outdir-or-log> [--class incomplete]

fastchess writes three warning classes, all of them found by
`strings /usr/local/bin/fastchess`:

    Warning; Incomplete mating PV - from {engine}
    Warning; Too long mating PV - from {engine}
    Warning; Mating PV does not end with checkmate - from {engine}

each followed by an `Info;` line carrying the offending `info score mate ...`
string, then a `Position; <fen>` line and a `Moves; <moves>` line. The `Info;`
line is why this scans forward for the two it wants instead of reading the next
two lines: assuming they were adjacent found 0 warnings in a log holding 3, and
reported them as malformed rather than silently. Only the
first class is what S147 and S171 counted and what the case set is built from;
the other two are reported because a nonzero count in either is a finding about
the search and not case material -- a mate line that is *too long* or that does
not end in checkmate is a wrong line, where a short one is the class DEC-122
leaves visible and S202 owns.

The engine name is fastchess's, so `candidate` against `ref-<sha>` is the
control S171 read: the same 1500 openings under two key sets.

Prints a summary, then one block per candidate warning ready to become a TSV
row -- the `name`, `go`, `start` and `stride` columns are the sweep's job
(`adocs/data/S170_replay.py`), not this script's.
"""
import argparse
import collections
import os
import re
import sys

WARNING = re.compile(r"^.*Warning;\s+(.+?)\s+-\s+from\s+(\S+)\s*$")
POSITION = re.compile(r"^.*Position;\s+(?:fen\s+)?(.*?)\s*$")
MOVES = re.compile(r"^.*Moves;\s+(.*?)\s*$")
INFO = re.compile(r"^.*Info;\s+(.*?)\s*$")

# How far past a warning its Position and Moves lines may sit. They follow one
# Info; line today; the bound stops a malformed block from swallowing the next
# warning's rows, which would attach one game's moves to another's root.
LOOKAHEAD = 4

CASE_CLASS = "Incomplete mating PV"


def parse(path):
    """Every warning, in order, as (klass, engine, fen, moves, info).

    A warning's Position and Moves lines follow it within LOOKAHEAD lines, with
    an Info; line carrying the offending `info score mate ...` string among
    them. A warning missing either is counted as malformed rather than skipped:
    a truncated log or a moved format is a thing to know about, not to average
    over.
    """
    warnings = []
    malformed = 0

    with open(path, encoding="utf-8", errors="replace") as handle:
        lines = handle.read().splitlines()

    i = 0
    while i < len(lines):
        hit = WARNING.match(lines[i])
        if not hit:
            i += 1
            continue

        klass, engine = hit.group(1), hit.group(2)
        fen = moves = info = None
        j = i + 1

        while j < len(lines) and j <= i + LOOKAHEAD:
            if WARNING.match(lines[j]):
                break
            if fen is None:
                position = POSITION.match(lines[j])
                if position:
                    fen = position.group(1)
                    j += 1
                    continue
            if moves is None:
                move_line = MOVES.match(lines[j])
                if move_line:
                    moves = move_line.group(1)
                    j += 1
                    continue
            if info is None:
                info_line = INFO.match(lines[j])
                if info_line:
                    info = info_line.group(1)
            j += 1

        if fen is None or moves is None:
            malformed += 1
            i += 1
            continue

        warnings.append((klass, engine, fen, moves, info))
        i = j

    return warnings, malformed


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("target", help="a run's output directory, or the log itself")
    parser.add_argument("--engine", default="candidate",
                        help="whose warnings to print as case material")
    args = parser.parse_args()

    path = args.target
    if os.path.isdir(path):
        path = os.path.join(path, "fastchess.log")
    if not os.path.exists(path):
        sys.exit("no log at %s" % path)

    warnings, malformed = parse(path)

    print("log %s" % path)
    print("warnings %d, malformed %d" % (len(warnings), malformed))
    if malformed:
        print("  MALFORMED WARNINGS PRESENT -- the log is truncated or the "
              "format moved; read it before trusting the counts")

    counts = collections.Counter((klass, engine) for klass, engine, _, _, _ in warnings)
    if not counts:
        print("\nno mating-PV warnings in this run.")
        return

    print("\n%-42s %-20s %s" % ("class", "engine", "lines"))
    for (klass, engine), n in sorted(counts.items()):
        print("%-42s %-20s %d" % (klass, engine, n))

    for klass in sorted({k for k, _, _, _, _ in warnings}):
        if klass != CASE_CLASS:
            print("\nNOTE: %d line(s) of class %r. That is not case material: a "
                  "mate line that is too long or does not end in checkmate is a "
                  "wrong line, not a short one." %
                  (sum(n for (k, _), n in counts.items() if k == klass), klass))

    # Case material: distinct (fen, moves) pairs, since one game warns on
    # several searches and every warning repeats the game up to its own ply.
    # The longest move list for a given root is the one to keep -- it reaches
    # the last search that warned, which is what S170's rows record.
    by_root = {}
    for klass, engine, fen, moves, info in warnings:
        if klass != CASE_CLASS or engine != args.engine:
            continue
        if fen not in by_root or len(moves.split()) > len(by_root[fen][0].split()):
            by_root[fen] = (moves, info)

    print("\n%d distinct root position(s) from %s, the case material:"
          % (len(by_root), args.engine))
    for n, (fen, (moves, info)) in enumerate(sorted(by_root.items()), start=1):
        print("\n--- root %d ---" % n)
        print("fen\t%s" % fen)
        print("moves\t%s" % moves)
        print("plies\t%d" % len(moves.split()))
        print("info\t%s" % (info or "(none captured)"))


if __name__ == "__main__":
    main()
