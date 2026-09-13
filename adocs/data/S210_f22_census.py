#!/usr/bin/env python3
"""S210, F22. How often quiescence reaches a position no legal sequence can
mate from -- counted over real games, before the rule that scores it a draw was
written.

`2026-09-10_adversarial-F22`: `is_insufficient_material()` has a single call
site, in `negamax_at`, and it is guarded by `ply > 0`. Quiescence has none, so a
capture made inside quiescence that leaves KvK, KNvK or KBvK is handed to the
static evaluation, which scores the pieces left standing instead of the draw the
laws already decided. The fix is one test after `make_move`. The question this
file answers is the one DEC-143 asks before a match is booked and DEC-107
answers by census: **is the class the fix changes reached in ordinary play, and
if so how much of the tree is it.**

    cmake --build build-tune -j12                     # the counters live here
    ~/.venv/chess/bin/python adocs/data/S210_f22_census.py positions \
        adocs/data/S219_aa_calibration.pgn .tuning/coord/f22.fens
    ~/.venv/chess/bin/python adocs/data/S210_f22_census.py run \
        build-tune/src/chesso .tuning/coord/f22.fens .tuning/coord/f22_before
    # ... then the other tree, into .tuning/coord/f22_after, and:
    ~/.venv/chess/bin/python adocs/data/S210_f22_census.py compare \
        .tuning/coord/f22_before .tuning/coord/f22_after \
        adocs/data/S210_f22_changed.tsv

The sample list and the two per-position tables are **scratch and deliberately
not committed**: `positions` rebuilds the list byte for byte from a PGN that is
committed, in ten seconds, and `adocs/data/` is for output that costs hours to
regenerate. What is kept is the reading -- `adocs/data/S210_f22_census.txt` --
and the rows that moved, `adocs/data/S210_f22_changed.tsv`, which carry their
own FENs and need neither of the other three files to be read.

THE CORPUS. `adocs/data/S219_aa_calibration.pgn`, the DEC-143 fixed-rounds A/A
of 2026-09-12: 1000 games at 8+0.08 on `books/noob_3moves.epd`, chesso against
chesso, which is the regime every verdict here is taken in. Its 115021
positions -- every ply of every game, the book position and the final position
included -- are the population, and `positions` takes a systematic every-Nth
sample of it. Systematic and not random, and not filtered to endgames: one
search happens per ply of a real game, so a sample that keeps the ply
distribution keeps the weighting the question is about. An endgame-enriched
corpus would answer a different question and return a number that does not
transfer (DEC-019's failure mode from the other direction).

WHICH BINARY. The tune build, because the counters are compiled under
`CHESSO_TUNE` and nowhere else -- the release binary an SPRT measures carries
none of this. It is the same tree, and that equality is the licence to read a
tune-build census as a statement about the shipping engine, so `run` **checks
it** rather than assuming it: it takes `chesso bench` from the engine it was
given and from `build/src/chesso`, and says so on the first line. Not a
hardcoded number -- the total moves with every functional change by design
(DEC-142). It read 7111579 on both builds before this step's fix and 7105111 on
both after it.

WHAT IS COUNTED. Three totals over the whole run, written by the engine's exit
handler to the file `CHESSO_F22_CENSUS` names:

    qnodes  quiescence nodes entered
    qmoves  moves quiescence made that were legal -- one child node each
    dead    ... of which left a position is_insufficient_material() calls dead

`dead / qmoves` is the fraction of quiescence's move-making the rule touches.
A position is called dead by the engine's own function and never by this script
and never by a reading of the board: the CHESS rule holds here too.

AND THE HALF A COUNTER CANNOT SHOW. A fraction near zero still permits one
position whose answer moves, so `run` also records every position's `bestmove`,
score and node count at a fixed depth, and `compare` counts the rows that moved
between two runs. That is the same instrument S109's sweep used for "best moves
changed" and it is what turns "the rule rarely fires" into "the rule changed
what the engine plays, N times out of M".

DEPTH. Fixed depth 10. The corpus was played at 8+0.08 and its own comments
report depths of 10 to 14, so 10 is the shallow end of the tree the engine
really searches rather than a depth picked to be quick. Fixed depth and not
fixed time, because a timed run is not reproducible and the two halves of a
before/after comparison have to be the same tree.
"""
import os
import subprocess
import sys
import time

DEPTH = 10
EVERY = 10

# The release binary the tune build has to agree with, node for node. A path
# and never a number: the total is expected to move with every functional
# change, so a constant here would be a golden that goes stale by design.
RELEASE = "build/src/chesso"


def positions(pgn_path, out_path, every=EVERY):
    """Every ply of every game, then every `every`-th of those."""
    import chess.pgn

    kept, total, games = [], 0, 0
    with open(pgn_path) as handle:
        while True:
            game = chess.pgn.read_game(handle)
            if game is None:
                break
            games += 1
            board = game.board()
            if total % every == 0:
                kept.append(board.fen())
            total += 1
            for move in game.mainline_moves():
                board.push(move)
                if total % every == 0:
                    kept.append(board.fen())
                total += 1

    with open(out_path, "w") as handle:
        handle.write(f"# {pgn_path}: {games} games, {total} positions, "
                     f"every {every}th kept -> {len(kept)}\n")
        for fen in kept:
            handle.write(fen + "\n")
    print(f"{games} games, {total} positions, every {every}th -> {len(kept)}")


def read_fens(path):
    with open(path) as handle:
        return [line.strip() for line in handle
                if line.strip() and not line.startswith("#")]


class engine_t:
    """One process for the whole run, driven a command at a time.

    Never a bare pipe of every line at once: `quit` arriving while a search is
    running stops it, which is TOOLCHAIN.md's Stockfish trap and S109 met it
    here as well -- 300 positions answered in 0.4 s, every one at depth 1. Each
    `go` is waited out to its own `bestmove`.
    """

    def __init__(self, path, environment=None):
        self.proc = subprocess.Popen([path], stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, text=True,
                                     bufsize=1, env=environment)
        self.send("uci")
        self.wait_for("uciok")

    def send(self, line):
        self.proc.stdin.write(line + "\n")
        self.proc.stdin.flush()

    def wait_for(self, prefix):
        while True:
            line = self.proc.stdout.readline()
            if not line:
                sys.exit("the engine died")
            if line.startswith(prefix):
                return line.strip()

    def go(self, fen, depth):
        """The last info line's score and nodes, and the move played."""
        self.send("ucinewgame")
        self.send("position fen " + fen)
        self.send(f"go depth {depth}")
        score, nodes = "none", 0
        while True:
            line = self.proc.stdout.readline()
            if not line:
                sys.exit("the engine died")
            if line.startswith("info score"):
                field = line.split()
                nodes = int(field[field.index("nodes") + 1])
                score = " ".join(field[1:field.index("time")])
            if line.startswith("bestmove"):
                return line.split()[1], score, nodes

    def close(self):
        self.send("quit")
        return self.proc.wait()


def bench(path):
    out = subprocess.run([path, "bench"], capture_output=True, text=True)
    for line in out.stdout.splitlines():
        if line.endswith("nps") and "nodes" in line:
            return int(line.split()[0])
    sys.exit(f"{path} bench printed no total")


def run(engine, fens_path, out_prefix, depth=DEPTH):
    """Per-position answers to `<prefix>.tsv`, the counters to `<prefix>.census`.

    The bench total is checked first and printed with the rows. A census taken
    on a tree whose bench does not match the release build's is a census of a
    different engine, and that is the one thing about this method that has to
    hold rather than be assumed.
    """
    fens = read_fens(fens_path)
    total = bench(engine)
    shipping = bench(RELEASE) if os.path.exists(RELEASE) else None
    agrees = ("same tree as " + RELEASE if total == shipping else
              f"**DIFFERENT TREE from {RELEASE}, which benches {shipping}**"
              if shipping is not None else f"{RELEASE} not built, unchecked")
    print(f"# {engine}: bench {total}, {agrees}")
    if shipping is not None and total != shipping:
        # The docstring says the agreement is checked, not assumed: a census
        # on a tree that is not the shipping one measures the wrong engine.
        sys.exit(f"refused: {engine} benches {total} but {RELEASE} benches "
                 f"{shipping}; rebuild both from one tree before counting")
    print(f"# {len(fens)} positions, depth {depth}", flush=True)

    environment = dict(os.environ)
    environment["CHESSO_F22_CENSUS"] = os.path.abspath(out_prefix + ".census")

    proc = engine_t(engine, environment)
    started = time.perf_counter()
    with open(out_prefix + ".tsv", "w") as handle:
        handle.write(f"# {engine}, depth {depth}, bench {total}\n")
        handle.write("fen\tbestmove\tscore\tnodes\n")
        for index, fen in enumerate(fens):
            best, score, nodes = proc.go(fen, depth)
            handle.write(f"{fen}\t{best}\t{score}\t{nodes}\n")
            if (index + 1) % 250 == 0:
                elapsed = time.perf_counter() - started
                print(f"# {index + 1} of {len(fens)}, {elapsed:.0f}s",
                      flush=True)
    if proc.close() != 0:
        sys.exit("the engine exited non-zero")

    print(f"# {time.perf_counter() - started:.0f}s")
    report(out_prefix)


def read_census(path):
    counters = {}
    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or line.startswith("counter"):
                continue
            name, count = line.split()
            counters[name] = int(count)
    return counters


def read_answers(path):
    rows = []
    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or line.startswith("fen\t"):
                continue
            fen, best, score, nodes = line.rstrip("\n").split("\t")
            rows.append((fen, best, score, int(nodes)))
    return rows


def report(prefix):
    counters = read_census(prefix + ".census")
    rows = read_answers(prefix + ".tsv")
    qnodes, qmoves, dead = (counters["qnodes"], counters["qmoves"],
                            counters["dead"])
    nodes = sum(row[3] for row in rows)

    print()
    print(f"census {prefix}")
    print(f"  positions searched          {len(rows):>16}")
    print(f"  search nodes                {nodes:>16}")
    print(f"  quiescence nodes            {qnodes:>16}"
          f"   {100 * qnodes / max(nodes, 1):6.2f} % of nodes")
    print(f"  moves made in quiescence    {qmoves:>16}")
    print(f"  ... landing on a dead board {dead:>16}"
          f"   {100 * dead / max(qmoves, 1):9.5f} % of them")


def compare(before, after, changed_path=None):
    report(before)
    report(after)

    rows_before = read_answers(before + ".tsv")
    rows_after = read_answers(after + ".tsv")
    if len(rows_before) != len(rows_after):
        sys.exit("the two runs did not cover the same positions")

    moved, rescored, nodes_before, nodes_after = 0, 0, 0, 0
    changed = []
    for one, two in zip(rows_before, rows_after):
        if one[0] != two[0]:
            sys.exit("the two runs did not cover the same positions")
        nodes_before += one[3]
        nodes_after += two[3]
        if one[1] != two[1]:
            moved += 1
            changed.append((one[0], one[1], two[1], one[2], two[2]))
        if one[2] != two[2]:
            rescored += 1

    print()
    print(f"compare {before} -> {after}   over {len(rows_before)} positions")
    print(f"  best move moved   {moved:>8}   "
          f"{100 * moved / len(rows_before):.3f} %")
    print(f"  score moved       {rescored:>8}   "
          f"{100 * rescored / len(rows_before):.3f} %")
    print(f"  nodes             {nodes_before} -> {nodes_after}   "
          f"{nodes_after / max(nodes_before, 1):.6f}")
    for fen, one, two, score_one, score_two in changed[:10]:
        print(f"    {one} -> {two}   [{score_one}] -> [{score_two}]   {fen}")

    # Every position whose answer moved, written out where it can be kept. The
    # rows carry their own FENs, so the file stands on its own without the
    # sample list or either run's full table -- which is the reason those three
    # are scratch and this one is evidence.
    if changed_path is None:
        return

    with open(changed_path, "w") as handle:
        handle.write(f"# {moved} of {len(rows_before)} positions whose best "
                     f"move moved, {before} -> {after}\n")
        handle.write("fen\tbefore\tafter\tscore_before\tscore_after\n")
        for fen, one, two, score_one, score_two in changed:
            handle.write(f"{fen}\t{one}\t{two}\t{score_one}\t{score_two}\n")
    print(f"  wrote {changed_path}")


def main(argv):
    if len(argv) < 2:
        sys.exit(__doc__)
    if argv[1] == "positions":
        positions(argv[2], argv[3], int(argv[4]) if len(argv) > 4 else EVERY)
    elif argv[1] == "run":
        run(argv[2], argv[3], argv[4],
            int(argv[5]) if len(argv) > 5 else DEPTH)
    elif argv[1] == "report":
        report(argv[2])
    elif argv[1] == "compare":
        compare(argv[2], argv[3], argv[4] if len(argv) > 4 else None)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main(sys.argv)
