#!/usr/bin/env python3
"""S156. Where the mined set's floor comes from, and the red it has to produce.

    python3 adocs/data/S156_mined_floor_sweep.py
    python3 adocs/data/S156_mined_floor_sweep.py --depth 8 --ref 120497e

WHY THIS NEEDS A PATCHED TREE AND WILL NOT RUN WITHOUT ONE.

tests/test_mate_breadth.cpp asserts that the engine finds at least 143 of the
318 mined mates at the exact distance stockfish labelled them with. A floor is
worth what its separation is worth, so the number below it has to be a measured
reading and not an argument: the value the same set produces when the guard the
floor protects is weakened.

That value cannot be reached from a shipping binary any more. S142 narrowed
`RfpMinPly`'s minimum to 2 on S145's evidence, so `setoption name RfpMinPly
value 1` is out of range, is ignored, and leaves the engine at its default --
which reads as a perfect null result and is nothing of the kind. The first
sweep taken for S156 read exactly that and it was wrong.

So this script builds a throwaway git worktree, relaxes the bound there, and
measures in it. The tracked tree is never edited. Two readings come out:

  1. A count per `RfpMinPly` value, at a fixed depth, over the whole set. This
     is where the floor is placed - strictly between what ships and what the
     weakened guard gives.
  2. The gate itself, built with the weakened value as its compiled-in default
     and run. It must fail. A floor that cannot be made to go red is a floor
     that proves nothing, and this is the step that observes it.

The engine is driven over UCI with the standard library alone. python-chess is
this repository's tool for chess questions and is deliberately not a dependency
of anything the fast suite runs; nothing here asks a chess question, it only
reads back a score the engine printed. adocs/data/S145_mined_set.py is the
python-chess sibling that mines and labels the set, and that one does ask.

DEC-016. Nothing is copied: the positions are chesso's own games and the labels
came from stockfish run as a binary.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import time

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TSV = os.path.join(REPO, "adocs", "data", "S145_mined_set.tsv")

PARAMS = os.path.join("src", "search_params.hpp")

# The line as it ships, and the two rewrites of it this script needs. Matched
# whole so that a change to the default or to the bounds fails loudly here
# instead of silently measuring the wrong engine.
SHIPPING = 'X(RFP_MIN_PLY,       "RfpMinPly",       3,      2, 63)'
RELAXED = 'X(RFP_MIN_PLY,       "RfpMinPly",       3,      0, 63)'
WEAKENED = 'X(RFP_MIN_PLY,       "RfpMinPly",       1,      0, 63)'

SWEEP_VALUES = [3, 2, 1, 0]


def read_set(path=TSV):
    rows = []

    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or not line.strip():
                continue
            field = line.rstrip("\n").split("\t")
            if field[0] == "fen":
                continue
            rows.append((field[0], int(field[1])))

    return rows


def score(engine, rows, depth, options):
    """Exact, right-sign and wrong-sign counts over the whole set.

    One engine process for the set and `ucinewgame` before each position, so
    the count is a property of the set and not of the order it is read in. The
    score taken is the last one printed before `bestmove`, which is what a GUI
    reads.
    """
    process = subprocess.Popen([engine], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        process.stdin.write(line + "\n")
        process.stdin.flush()

    send("uci")
    while "uciok" not in process.stdout.readline():
        pass

    for name, value in options.items():
        send("setoption name %s value %s" % (name, value))

    found_any = found_exact = wrong_sign = 0

    for fen, distance in rows:
        send("ucinewgame")
        send("position fen " + fen)
        send("go depth %d" % depth)

        kind, value = None, 0
        while True:
            line = process.stdout.readline()
            if not line:
                break
            if line.startswith("info") and " score " in line:
                token = line.split()
                kind = token[token.index("score") + 1]
                value = int(token[token.index("score") + 2])
            if line.startswith("bestmove"):
                break

        if kind != "mate":
            continue
        if value < 0:
            wrong_sign += 1
            continue

        found_any += 1
        if value == distance:
            found_exact += 1

    send("quit")
    process.wait(timeout=10)

    return found_exact, found_any, wrong_sign


def run(command, cwd):
    result = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True)
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        raise SystemExit("failed in %s: %s" % (cwd, " ".join(command)))
    return result.stdout


def patch(tree, before, after):
    path = os.path.join(tree, PARAMS)
    with open(path) as handle:
        text = handle.read()

    if before not in text:
        raise SystemExit(
            "%s does not contain the line this script rewrites:\n  %s\n"
            "The parameter's default or its bounds have moved. Re-read "
            "src/search_params.hpp and update SHIPPING here before trusting "
            "any number below." % (path, before))

    with open(path, "w") as handle:
        handle.write(text.replace(before, after, 1))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ref", default="HEAD",
                        help="commit to measure; the worktree is checked out "
                             "detached at it")
    parser.add_argument("--depth", type=int, default=10)
    parser.add_argument("--floor", type=int, default=143,
                        help="the floor tests/test_mate_breadth.cpp asserts")
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--keep", action="store_true",
                        help="leave the worktree in place for inspection")
    args = parser.parse_args()

    rows = read_set()
    print("%d positions, depth %d, ref %s\n" % (len(rows), args.depth, args.ref))

    tree = tempfile.mkdtemp(prefix="S156_sweep_")
    shutil.rmtree(tree)

    run(["git", "worktree", "add", "--detach", tree, args.ref], REPO)

    try:
        # 1. The sweep. The bound is relaxed and the default is left alone, so
        #    every row below is the same binary answering a setoption.
        patch(tree, SHIPPING, RELAXED)

        run(["cmake", "-S", ".", "-B", "build-tune", "-DCMAKE_BUILD_TYPE=Release",
             "-DCHESSO_TUNE=ON"], tree)
        run(["cmake", "--build", "build-tune", "-j%d" % args.jobs,
             "--target", "chesso"], tree)

        engine = os.path.join(tree, "build-tune", "src", "chesso")

        print("  RfpMinPly   exact   right sign   wrong sign   wall")
        readings = {}

        for value in SWEEP_VALUES:
            start = time.time()
            exact, any_, wrong = score(engine, rows, args.depth,
                                       {"Hash": 16, "RfpMinPly": value})
            readings[value] = exact
            print("  %9d   %5d   %10d   %10d   %.1fs"
                  % (value, exact, any_, wrong, time.time() - start))

        ships = readings[SWEEP_VALUES[0]]
        weak = readings[SWEEP_VALUES[-1]]

        print("\n  floor %d, asserted by tests/test_mate_breadth.cpp" % args.floor)
        print("    shipping value %d, weakened-guard value %d, gap %d"
              % (ships, weak, ships - weak))

        if not weak < args.floor <= ships:
            print("    THE FLOOR NO LONGER SEPARATES. It has to sit above what "
                  "the weakened guard gives and at or below what ships.")

        # 2. The red. The bound alone cannot produce it: the gate is compiled
        #    against the default, not against a setoption, so the default is
        #    what has to move.
        patch(tree, RELAXED, WEAKENED)

        # tests/doctest and tests/json are submodules and a fresh worktree gets
        # them empty. The objects are already in this repository's store, so
        # this is a checkout and not a fetch.
        run(["git", "submodule", "update", "--init", "tests/doctest",
             "tests/json"], tree)

        run(["cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"], tree)
        run(["cmake", "--build", "build", "-j%d" % args.jobs,
             "--target", "test_mate_breadth"], tree)

        gate = subprocess.run([os.path.join(tree, "build", "tests",
                                            "test_mate_breadth")],
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              text=True)

        print("\n  the gate built with RfpMinPly defaulting to 1: %s"
              % ("RED, as it must be" if gate.returncode != 0
                 else "GREEN, WHICH IS THE FAILURE"))

        for line in gate.stdout.splitlines():
            if "mined set at depth" in line or "under the" in line:
                print("    " + line.strip())

        return 0 if gate.returncode != 0 else 1

    finally:
        if args.keep:
            print("\n  worktree kept at %s" % tree)
        else:
            run(["git", "worktree", "remove", "--force", tree], REPO)


if __name__ == "__main__":
    sys.exit(main())
