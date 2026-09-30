#!/usr/bin/env python3
"""S115. What the aspiration loop's fail-low pull costs in nodes, and what the
widening ratio costs with it on.

Node counts at a fixed depth over the 300 positions S021's sweep defined -- the
S021/S068 instrument, re-run here because the accepts asks for it and because
S021 recorded that one sample chooses the wrong setting. It is not Elo and it
does not claim to be: the SPRT decides whether anything is worth having, and
this decides how many SPRTs there are and which ratio ships (DEC-019, DEC-063).
One binary answers for every row, the tune build, where every search parameter
is a UCI spin option (S073):

    cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
    cmake --build build-tune -j4
    adocs/data/S115_aspiration_sweep.py build-tune/src/chesso 11 > sweep.tsv
    adocs/data/S115_aspiration_sweep.py build-tune/src/chesso 11 --rows first

TWO ROW SETS, both run on 2026-09-30, and the reason there are two. `--rows
first` is the sweep the step first ran, on the tree with **both** of the rules
it built -- the pull and the root fail-high reduction, whose switch
`AspirationFailHighReduce` exists only with adocs/data/S115_reduction_as_built.diff
applied (`git apply` from the repository root, onto the tree before S115). That
run decided the verdict count and was then overtaken: the reduction was refused
on the mate guards (DEC-245) and its code left. Its reading is
adocs/data/S115_aspiration_sweep_d11.tsv. `--rows pull`, the default, is the
tree that ships, the pull alone, with the ratio swept under it; its reading is
adocs/data/S115_aspiration_sweep_pull_d11.tsv. That set was run twice: on the
step's first tree, `cd50c7a`, S114 verdict 1's candidate, kept as
adocs/data/S115_aspiration_sweep_pull_d11_cd50c7a.tsv, and again after that
verdict's H0 on the tree its removal leaves, which is the file named above and
the one the shipped comment and the pre-registration quote.

THE POSITIONS are adocs/data/S021_aspiration_sweep.py's own pick, executed
rather than copied (the S132 precedent: a second copy of a sampling rule is a
second sampling rule) at **S021's three offsets, 0, 37 and 71** -- the offsets
its TSV and adocs/data/README.md record, 4 per game_phase() value of
adocs/data/S018_raw.tsv over the 25 values with at least 4 rows, 100 positions
each. Never the three tools/search_bench.py positions.

THE OFF ROW is the shipped triple, **re-read from src/search_params.hpp** before
the run rather than written here -- AspirationMinDepth, AspirationDelta and
AspirationMaxDelta at their defaults, S085's 2 / 21 / 437 as this is written --
with AspirationFailLowPull 0 and AspirationFailHighReduce 0, the two off values,
and AspirationWidenPct 200: the tree before S115, node for node (proved on
`bench` and tools/search_bench.py separately, DEC-215). Taken at S021's own
5 / 50 / 400 it would measure S085's vector and this step's rules together.

THE ROWS, in order, every one at the shipped triple. `--rows pull`:

  off       pull 0, widen 200             the parent's tree
  pull      pull 2, widen 200             the pull at its seed, today's ratio
  pull150   pull 2, widen 150
  pull300   pull 2, widen 300

`--rows first`, where `reduce` is AspirationFailHighReduce:

  off       pull 0, reduce 0, widen 200   the parent's tree
  pull      pull 2, reduce 0, widen 200   the midpoint pull alone, at its seed
  reduce    pull 0, reduce 1, widen 200   the root fail-high reduction alone
  both      pull 2, reduce 1, widen 200   the two together, today's ratio
  both150   pull 2, reduce 1, widen 150
  both300   pull 2, reduce 1, widen 300

A row is `sample`, the row's name, the six settings, total nodes, nodes
relative to that sample's off row, and how many of the sample's best moves
differ from the off row's. The pooled block after the three samples sums each
row's nodes and changed moves over all 300.

THE READINGS, stated before the run (the brief's ruling 3 and DEC-244):

  inert apart   (`--rows first` only) a part is inert apart when its own
                row -- `pull` or `reduce` -- moves the pooled node count by
                less than 1 % of the off row's and changes the best move on
                at most 3 of the 300 positions. Both parts inert apart is
                DEC-082's condition for one SPRT; otherwise there are two, the
                reduction first.
  the ratio     AspirationWidenPct leaves 200 only for a ratio whose row --
                `pull150` or `pull300`, `both150` or `both300` -- costs fewer
                nodes than its set's 200 row (`pull`, `both`) on every one of
                the three samples, and then for the one of those with the
                fewest pooled; a lead that does not hold on every sample is
                inside the instrument's own disagreement and re-decides
                nothing (DEC-244). Pooled nodes are the ranking, as in S021.

The script prints both readings under the pooled block, mechanically, and ends
with SWEEP-DONE.
"""

import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PICK = os.path.join(HERE, "S021_aspiration_sweep.py")
SEARCH_PARAMS = os.path.join(HERE, "..", "..", "src", "search_params.hpp")
CORPUS = "adocs/data/S018_raw.tsv"

OFFSETS = (0, 37, 71)

# name, AspirationFailLowPull, AspirationFailHighReduce (None: not sent, the
# shipping tree has no such option), AspirationWidenPct.
ROW_SETS = {
    "pull": [
        ("off", 0, None, 200),
        ("pull", 2, None, 200),
        ("pull150", 2, None, 150),
        ("pull300", 2, None, 300),
    ],
    "first": [
        ("off", 0, 0, 200),
        ("pull", 2, 0, 200),
        ("reduce", 0, 1, 200),
        ("both", 2, 1, 200),
        ("both150", 2, 1, 150),
        ("both300", 2, 1, 300),
    ],
}

# Each set's ratio rows and the 200 row they are read against.
RATIO_ROWS = {"pull": ("pull", ("pull150", "pull300")),
              "first": ("both", ("both150", "both300"))}

INERT_NODES = 0.01
INERT_MOVES = 3


def s021_positions(offset):
    """S021's own picker at one offset, executed rather than copied.

    That file cannot be imported: it reads argv and runs a whole sweep at
    module level. Its source is executed up to the line where the sweep
    begins, which leaves exactly the pick -- the corpus path, PER_PHASE and
    positions(). adocs/data/S132_node_share_census.py does the same.
    """
    with open(PICK) as handle:
        source = handle.read()

    marker = "\nfens = positions()"

    if marker not in source:
        raise SystemExit(
            PICK + " no longer begins its sweep at `fens = positions()`; "
            "this script cuts it there and must be re-pointed")

    namespace = {"__name__": "S021_pick"}
    saved = sys.argv
    sys.argv = [PICK, "unused-engine", "11", str(offset)]

    try:
        exec(compile(source[:source.index(marker)], PICK, "exec"), namespace)
    finally:
        sys.argv = saved

    return namespace["positions"]()


def param_default(name):
    """The default of one X-macro row of src/search_params.hpp."""
    quoted = '"%s"' % name

    with open(SEARCH_PARAMS) as handle:
        for line in handle:
            if quoted not in line:
                continue

            rest = line.split(quoted, 1)[1]
            rest = rest.split(")")[0].strip().lstrip(",")
            return int(rest.split(",")[0].strip())

    raise SystemExit(name + " is not in " + SEARCH_PARAMS)


class Engine:
    def __init__(self, path):
        self.p = subprocess.Popen([path], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)
        self.send("uci")
        while "uciok" not in self.p.stdout.readline():
            pass

    def send(self, s):
        self.p.stdin.write(s + "\n")
        self.p.stdin.flush()

    def set(self, name, value):
        self.send(f"setoption name {name} value {value}")

    def go(self, fen, depth):
        """(nodes, best move) of one fixed-depth search, read until bestmove."""
        self.send("ucinewgame")
        self.send("position fen " + fen)
        self.send(f"go depth {depth}")
        nodes = 0
        while True:
            line = self.p.stdout.readline()
            if not line:
                sys.exit("engine died")
            if line.startswith("info") and " nodes " in line:
                t = line.split()
                nodes = int(t[t.index("nodes") + 1])
            if line.startswith("bestmove"):
                return nodes, line.split()[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("engine")
    parser.add_argument("depth", type=int)
    parser.add_argument("--rows", choices=sorted(ROW_SETS), default="pull")
    args = parser.parse_args()
    ROWS = ROW_SETS[args.rows]

    if not os.path.exists(CORPUS):
        raise SystemExit(
            "cannot see " + CORPUS + " -- S021's picker reads it by a path "
            "relative to the repository root, so run this from there")

    triple = {name: param_default(name)
              for name in ("AspirationMinDepth", "AspirationDelta",
                           "AspirationMaxDelta")}

    engine = Engine(args.engine)
    for name, value in triple.items():
        engine.set(name, value)

    print(f"# rows {args.rows}, depth {args.depth}, engine {args.engine}, "
          f"offsets {OFFSETS}, "
          f"triple from src/search_params.hpp: "
          + ", ".join(f"{k} {v}" for k, v in triple.items()), flush=True)
    print("sample\trow\tpull\treduce\twiden\tmin_depth\tdelta\tmax_delta"
          "\tnodes\trel\tmoves_changed", flush=True)

    # per row: nodes per sample, changed moves per sample
    nodes = {row[0]: [] for row in ROWS}
    changed = {row[0]: [] for row in ROWS}
    positions = 0

    for offset in OFFSETS:
        fens = s021_positions(offset)
        positions += len(fens)
        off_nodes, off_moves = None, None

        for name, pull, reduce, widen in ROWS:
            engine.set("AspirationFailLowPull", pull)
            if reduce is not None:
                engine.set("AspirationFailHighReduce", reduce)
            engine.set("AspirationWidenPct", widen)

            total, moves = 0, []
            for fen in fens:
                n, move = engine.go(fen, args.depth)
                total += n
                moves.append(move)

            if off_nodes is None:
                off_nodes, off_moves = total, moves

            diff = sum(1 for a, b in zip(moves, off_moves) if a != b)
            nodes[name].append(total)
            changed[name].append(diff)

            print(f"{offset}\t{name}\t{pull}\t{'-' if reduce is None else reduce}\t{widen}"
                  f"\t{triple['AspirationMinDepth']}"
                  f"\t{triple['AspirationDelta']}"
                  f"\t{triple['AspirationMaxDelta']}"
                  f"\t{total}\t{total / off_nodes:.4f}\t{diff}", flush=True)

    engine.send("quit")

    off_pooled = sum(nodes["off"])
    print(f"# pooled over {positions} positions", flush=True)
    print("pooled\trow\tnodes\trel\tmoves_changed\tper_sample_rel", flush=True)
    for name, _, _, _ in ROWS:
        pooled = sum(nodes[name])
        per_sample = " ".join(f"{n / o:.4f}"
                              for n, o in zip(nodes[name], nodes["off"]))
        print(f"pooled\t{name}\t{pooled}\t{pooled / off_pooled:.4f}"
              f"\t{sum(changed[name])}\t{per_sample}", flush=True)

    # The readings, mechanically, exactly as the docstring states them.
    rel = sum(nodes["pull"]) / off_pooled
    print(f"# reach: the pull at its seed moves the pooled count by "
          f"{100 * (rel - 1.0):+.2f} % and changes {sum(changed['pull'])} of "
          f"{positions} best moves", flush=True)

    if args.rows == "first":
        inert = {}
        for part in ("pull", "reduce"):
            rel = sum(nodes[part]) / off_pooled
            moves = sum(changed[part])
            inert[part] = abs(rel - 1.0) < INERT_NODES and moves <= INERT_MOVES
            print(f"# reading: {part} moves the pooled count by "
                  f"{100 * (rel - 1.0):+.2f} % and changes {moves} of "
                  f"{positions} best moves -> "
                  f"{'inert apart' if inert[part] else 'NOT inert apart'}",
                  flush=True)
        print("# reading: "
              + ("both parts inert apart: DEC-082's condition holds, one SPRT"
                 if all(inert.values()) else
                 "not both inert apart: two SPRTs, the reduction first"),
              flush=True)

    base, candidates = RATIO_ROWS[args.rows]
    leaders = []
    for name in candidates:
        if all(n < b for n, b in zip(nodes[name], nodes[base])):
            leaders.append((sum(nodes[name]), name))
    if leaders:
        best = min(leaders)[1]
        print(f"# reading: {best} costs fewer nodes than {base} (200) on every "
              f"sample and has the fewest pooled of those that do: the ratio "
              f"moves to {dict((r[0], r[3]) for r in ROWS)[best]}", flush=True)
    else:
        print("# reading: no ratio leads 200 on every sample: "
              "AspirationWidenPct stays 200", flush=True)

    print("SWEEP-DONE", flush=True)


if __name__ == "__main__":
    main()
