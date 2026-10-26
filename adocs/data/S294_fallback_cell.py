#!/usr/bin/env python3
"""Re-derive the cell tests/test_mate_pv.cpp's "an aborted iteration's line
printed beside a completed mate reaches it" searches. S294, DEC-142.

THE CELL IS A GOLDEN. A position and a `go nodes` budget, on a cold table at
the default Hash, where the search ends inside an iteration that has a line
of its own, the last completed iteration reported a mate, the aborted line
starts with the completed line's first move -- and the walk cannot complete
the aborted line against the completed score, so the line printed is the
completed one (S202's aborted-line fallback, DEC-251). The last clause is the
one a test cannot see from outside the engine, and no `src/` change was
wanted to expose it, so it is read here, as a separation: the shipped build
prints a line that reaches its mate, and the build with
`MW02_aborted_line_no_fallback` (tools/mutants/S202_mate_walk.py) applied
prints the aborted line short in the same cell.

Four stages, in this order; every input is named.

    P=~/.venv/chess/bin/python
    W=.tuning/s294

    # 1. a worktree with MW02 applied, and its engine. Never the main tree.
    git worktree add --detach $W/mw02 HEAD
    $P adocs/data/S294_fallback_cell.py apply --worktree $W/mw02
    cmake -S $W/mw02 -B $W/mw02/build -DCMAKE_BUILD_TYPE=Release \\
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    cmake --build $W/mw02/build -j12 --target chesso

    # 2. the shipped sweep and the mutant's, over the mined set
    $P adocs/data/S294_fallback_cell.py sweep --engine build/src/chesso \\
        --fens adocs/data/S145_mined_set.tsv --out $W/shipped.tsv
    $P adocs/data/S294_fallback_cell.py sweep \\
        --engine $W/mw02/build/src/chesso \\
        --fens adocs/data/S145_mined_set.tsv --out $W/mw02.tsv

    # 3. the pick, by the rule below
    $P adocs/data/S294_fallback_cell.py pick \\
        --shipped $W/shipped.tsv --mutant $W/mw02.tsv

    # 4. remove the worktree (a plain remove refuses one with a submodule)
    git worktree remove --force $W/mw02

THE RULE, stated once. A cell separates when, on both builds, the last `info`
line repeats the depth of the line before it -- which is how
iterative_deepening_search() prints an aborted iteration, beside the last
completed iteration's score and depth -- that score is a mate, the two lines
start with the same move, the shipped build's last line is at least as long
as the mate needs and ends in checkmate (python-chess, a tool, not a
judgement), and the mutant's last line does not: shorter than the mate
needs, or not checkmate at that distance -- the sweep's `short`, which is any
such line that is not `full`. Per position, the longest run of
consecutive separating budgets on the grid; the position with the longest
run, the first in the file on a tie; the budget at that run's middle, the
lower one for an even run. The run's half-width is the margin: a change that
moves the abort point by less keeps the cell.

The grid is 20000 to 400000 nodes by 10000, Hash 16, one `ucinewgame` per
cell. The positions are S145's mined set, one per game of this project's own
play (DEC-016); nothing is chosen by reading a board (CLAUDE.md's CHESS rule).

`sweep` is node-limited and single-threaded, so it gives the same file at any
--jobs and on a busy machine.
"""

import argparse
import concurrent.futures
import os
import subprocess
import sys

import chess

MUTANT_ID = "MW02_aborted_line_no_fallback"
MUTANT_FILE = "tools/mutants/S202_mate_walk.py"
GRID = list(range(20000, 400001, 10000))
HASH_MB = 16

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))


def read_fens(path):
    fens = []
    with open(path) as handle:
        for line in handle:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            fen = line.split("\t")[0]
            if fen != "fen":
                fens.append(fen)
    return fens


def plies_for(mate_in):
    return 2 * mate_in - 1 if mate_in > 0 else -2 * mate_in


def ends_in_mate(fen, pv, count):
    board = chess.Board(fen)
    for uci in pv[:count]:
        move = chess.Move.from_uci(uci)
        if move not in board.legal_moves:
            return False
        board.push(move)
    return board.is_checkmate()


def parse_info(line):
    """(depth, score kind, value, pv) of an `info score` line, else None."""
    token = line.split()
    if len(token) < 4 or token[0] != "info" or token[1] != "score":
        return None
    depth = int(token[token.index("depth") + 1]) if "depth" in token else 0
    pv = token[token.index("pv") + 1:] if "pv" in token else []
    return depth, token[2], int(token[3]), pv


def sweep_one(engine, fen):
    """Every grid cell of one position: one engine, a cold table per cell."""
    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(text):
        proc.stdin.write(text + "\n")
        proc.stdin.flush()

    def read_until(prefix):
        out = []
        while True:
            line = proc.stdout.readline()
            if line == "":
                raise RuntimeError("engine closed its output on " + fen)
            line = line.strip()
            out.append(line)
            if line.startswith(prefix):
                return out

    send("uci")
    read_until("uciok")
    send(f"setoption name Hash value {HASH_MB}")
    rows = []
    for nodes in GRID:
        send("ucinewgame")
        send("position fen " + fen)
        send(f"go nodes {nodes}")
        infos = [i for i in map(parse_info, read_until("bestmove")) if i]
        if len(infos) < 2:
            rows.append((fen, nodes, "none", 0, 0, ""))
            continue
        (depth, kind, value, pv), prev = infos[-1], infos[-2]
        aborted = depth == prev[0]
        same_first = bool(pv) and bool(prev[3]) and pv[0] == prev[3][0]
        if not (aborted and kind == "mate" and same_first):
            rows.append((fen, nodes, "none", 0, 0, ""))
            continue
        needed = plies_for(value)
        mated = len(pv) >= needed and ends_in_mate(fen, pv, needed)
        rows.append((fen, nodes, "full" if mated else "short", len(pv),
                     needed, " ".join(pv)))
    send("quit")
    proc.wait(timeout=10)
    return rows


def sweep(args):
    fens = read_fens(args.fens)
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        results = list(pool.map(lambda f: sweep_one(args.engine, f), fens))
    with open(args.out, "w") as out:
        out.write(f"# S294_fallback_cell.py sweep, engine {args.engine}, "
                  f"fens {args.fens}, Hash {HASH_MB}, grid {GRID[0]}.."
                  f"{GRID[-1]} by {GRID[1] - GRID[0]}\n")
        out.write("fen\tnodes\taborted_mate_line\tpv_len\tneeded\tpv\n")
        for rows in results:
            for row in rows:
                out.write("\t".join(str(x) for x in row) + "\n")
    return 0


def read_sweep(path):
    cells = {}
    order = []
    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or line.startswith("fen\t"):
                continue
            fen, nodes, kind, *_ = line.rstrip("\n").split("\t")
            if fen not in cells:
                cells[fen] = {}
                order.append(fen)
            cells[fen][int(nodes)] = kind
    return order, cells


def pick(args):
    order, shipped = read_sweep(args.shipped)
    _, mutant = read_sweep(args.mutant)
    best = None
    separating = 0
    for fen in order:
        run, longest, longest_end = 0, 0, None
        for nodes in GRID:
            ok = (shipped[fen].get(nodes) == "full"
                  and mutant.get(fen, {}).get(nodes) == "short")
            run = run + 1 if ok else 0
            if run > longest:
                longest, longest_end = run, nodes
        if longest == 0:
            continue
        separating += 1
        step = GRID[1] - GRID[0]
        lo = longest_end - (longest - 1) * step
        print(f"separates  {longest:3d} budgets  {lo}..{longest_end}  {fen}")
        if best is None or longest > best[0]:
            best = (longest, lo, longest_end, fen)
    print(f"{separating} of {len(order)} positions separate")
    if best is None:
        print("no cell separates: the guard has no case on this tree")
        return 1
    longest, lo, hi, fen = best
    step = GRID[1] - GRID[0]
    middle = lo + ((longest - 1) // 2) * step
    print(f"pick  {fen}  go nodes {middle}  (run {lo}..{hi}, {longest} "
          f"budgets, margin {((longest - 1) // 2) * step} nodes below and "
          f"{hi - middle} above)")
    return 0


def apply(args):
    """MW02's own pairs, read from the mutant file, applied to a worktree."""
    sys.path.insert(0, os.path.join(ROOT, "tools"))
    import mutation_check  # noqa: E402

    worktree = os.path.abspath(args.worktree)
    mutation_check.require_linked_worktree(worktree)
    chosen = [m for m in mutation_check.load_mutants(
        [os.path.join(ROOT, MUTANT_FILE)]) if m["id"] == MUTANT_ID]
    if len(chosen) != 1:
        print(f"{MUTANT_ID} not found once in {MUTANT_FILE}", file=sys.stderr)
        return 1
    mutation_check.validate(chosen, worktree)
    mutation_check.apply_mutant(worktree, chosen[0])
    print(f"applied {MUTANT_ID} to {worktree}/{chosen[0]['file']}")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = parser.add_subparsers(dest="stage", required=True)
    a = sub.add_parser("apply")
    a.add_argument("--worktree", required=True)
    s = sub.add_parser("sweep")
    s.add_argument("--engine", required=True)
    s.add_argument("--fens", required=True)
    s.add_argument("--out", required=True)
    s.add_argument("--jobs", type=int, default=os.cpu_count())
    p = sub.add_parser("pick")
    p.add_argument("--shipped", required=True)
    p.add_argument("--mutant", required=True)
    args = parser.parse_args()
    return {"apply": apply, "sweep": sweep, "pick": pick}[args.stage](args)


if __name__ == "__main__":
    sys.exit(main())
