#!/usr/bin/env python3
"""Re-derive the cell tests/test_mate_pv.cpp's "an aborted iteration's line
longer than the mate beside it gives way to the completed line" searches, and
count the class it guards over two grids. S295, DEC-142, DEC-317.

THE CELL IS A GOLDEN. A position and a `go nodes` budget, on a cold table at
Hash 16, where the search ends inside an iteration that has a line of its
own, the last completed iteration reported a mate, the two lines start with
the same move, and the aborted line runs past the distance that mate needs
without a checkmate there -- so the walk leaves it as it is, S202's fallback
(which keys on a line still too short) does not run, and before S295 that
line was printed (adocs/data/S294_aborted_long_line.txt). S295's path gives
it way to the completed line. Read as a separation: the S295 build's last
line is the completed line verbatim and reaches its mate, and the build with
`MW03_long_aborted_line_kept` (tools/mutants/S202_mate_walk.py) applied --
S295's path taken out, the aborted path as it stood on 4fb915e -- prints a
last line longer than the mate needs that is not checkmate at that distance.

Stages, in this order; every input is named.

    P=~/.venv/chess/bin/python
    W=.tuning/s295

    # 1. a worktree with MW03 applied, and its engine. Never the main tree.
    git worktree add --detach $W/mw03 HEAD
    $P adocs/data/S295_long_line.py apply --worktree $W/mw03
    cmake -S $W/mw03 -B $W/mw03/build -DCMAKE_BUILD_TYPE=Release \\
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    cmake --build $W/mw03/build -j12 --target chesso

    # 2. both builds over the mined set (S294_fallback_cell.py's grid)
    $P adocs/data/S295_long_line.py sweep --engine build/src/chesso \\
        --out $W/fixed_mined.tsv
    $P adocs/data/S295_long_line.py sweep --engine $W/mw03/build/src/chesso \\
        --out $W/mw03_mined.tsv

    # 3. the pick, by the rule below
    $P adocs/data/S295_long_line.py pick \\
        --fixed $W/fixed_mined.tsv --mutant $W/mw03_mined.tsv

    # 4. remove the worktree (a plain remove refuses one with a submodule)
    git worktree remove --force $W/mw03

THE RULE, stated once. A cell separates when, on both builds, the last `info`
line repeats the depth of the line before it -- how
iterative_deepening_search() prints an aborted iteration, beside the last
completed iteration's score and depth -- that score is a mate, and the two
lines start with the same move; the fixed build's last line is the line
before it verbatim and is checkmate at exactly the distance the mate needs
(python-chess, a tool, not a judgement); and the mutant's last line is longer
than that distance and not checkmate at it. Per position, the longest run of
consecutive separating budgets on the grid; the position with the longest
run, the first in the file on a tie; the budget at that run's middle, the
lower one for an even run. The run's half-width is the margin. The rule is
S294_fallback_cell.py's with the mutant's class changed from short to long.

THE CENSUS, for the record and not a golden: every `info` line that carries
a mate score, over the mined grid above and over the S170 grid
(adocs/data/S203_case_sweep.sh's: the six cases of adocs/data/S170_cases.tsv,
strides 1 and 2, nine budgets, each from its own start ply, one process and
one `ucinewgame` per cell, Hash 16), classed as `full` (checkmate at exactly
the claimed distance), `short` (fewer plies than the distance, the residue
DEC-122 allows) or `wrong` (at least the distance and not checkmate there):

    $P adocs/data/S295_long_line.py s170 --engine build/src/chesso \\
        --out $W/fixed_s170.tsv
    $P adocs/data/S295_long_line.py census $W/fixed_mined.tsv $W/fixed_s170.tsv

THE PONDER READING, for the record: `moves` records every search's `bestmove`
and `ponder` over either grid (--grid mined or s170), and `ponder` compares
two such files cell for cell -- where the ponder token's presence or value, or
the move played, differs between two builds:

    $P adocs/data/S295_long_line.py moves --grid mined --engine build/src/chesso \\
        --out $W/fixed_moves_mined.tsv
    $P adocs/data/S295_long_line.py ponder $W/before.tsv $W/after.tsv

`sweep`, `s170` and `moves` are node-limited and single-threaded per engine, so they
give the same file at any --jobs and on a busy machine. The positions are
S145's mined set, one per game of this project's own play (DEC-016), and
S170's four games; nothing is chosen by reading a board (CLAUDE.md's CHESS
rule).
"""

import argparse
import collections
import concurrent.futures
import os
import subprocess
import sys

import chess

MUTANT_ID = "MW03_long_aborted_line_kept"
MUTANT_FILE = "tools/mutants/S202_mate_walk.py"
GRID = list(range(20000, 400001, 10000))
S170_BUDGETS = [100000, 300000, 500000, 1000000, 1200000, 1500000, 2000000,
                3000000, 4000000]
HASH_MB = 16

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
MINED = os.path.join(HERE, "S145_mined_set.tsv")
CASES = os.path.join(HERE, "S170_cases.tsv")

COLUMNS = ["set", "key", "nodes", "ply", "line", "last", "depth", "aborted",
           "same_first", "mate", "pv_len", "needed", "class", "pv"]


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


def read_cases(path):
    cases = []
    with open(path) as handle:
        for line in handle:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            f = line.split("\t")
            if f[0] == "name":
                continue
            cases.append((f[0], f[1], f[2].split(), int(f[4] or 0)))
    return cases


def plies_for(mate_in):
    return 2 * mate_in - 1 if mate_in > 0 else -2 * mate_in


def classify(board, pv, needed):
    """full, short or wrong, on a copy of `board` (python-chess)."""
    if len(pv) < needed:
        return "short"
    if len(pv) != needed:
        return "wrong"
    board = board.copy()
    for uci in pv:
        move = chess.Move.from_uci(uci)
        if move not in board.legal_moves:
            return "wrong"
        board.push(move)
    return "full" if board.is_checkmate() else "wrong"


def parse_info(line):
    """(depth, score kind, value, pv) of an `info score` line, else None."""
    token = line.split()
    if len(token) < 4 or token[0] != "info" or token[1] != "score":
        return None
    depth = int(token[token.index("depth") + 1]) if "depth" in token else 0
    pv = token[token.index("pv") + 1:] if "pv" in token else []
    return depth, token[2], int(token[3]), pv


class Engine:
    def __init__(self, path):
        self.proc = subprocess.Popen([path], stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, text=True,
                                     bufsize=1)
        self.send("uci")
        self.read_until("uciok")
        self.send(f"setoption name Hash value {HASH_MB}")

    def send(self, text):
        self.proc.stdin.write(text + "\n")
        self.proc.stdin.flush()

    def read_until(self, prefix):
        # Read to the terminator; never `quit` to reach it (TOOLCHAIN.md).
        out = []
        while True:
            line = self.proc.stdout.readline()
            if line == "":
                raise RuntimeError("engine closed its output")
            line = line.strip()
            out.append(line)
            if line.startswith(prefix):
                return out

    def close(self):
        self.send("quit")
        self.proc.wait(timeout=10)


def search_rows(engine, board, position, spec, tag):
    """One search: a row per mate line, in the order printed."""
    engine.send(position)
    engine.send("go " + spec)
    infos = [i for i in map(parse_info, engine.read_until("bestmove")) if i]
    rows = []
    for k, (depth, kind, value, pv) in enumerate(infos):
        if kind != "mate":
            continue
        prev = infos[k - 1] if k > 0 else None
        aborted = prev is not None and prev[0] == depth
        same_first = (aborted and bool(pv) and bool(prev[3])
                      and pv[0] == prev[3][0])
        needed = plies_for(value)
        rows.append(tag + [k, int(k == len(infos) - 1), depth, int(aborted),
                           int(same_first), value, len(pv), needed,
                           classify(board, pv, needed), " ".join(pv)])
    return rows


def sweep_one(engine_path, fen):
    engine = Engine(engine_path)
    board = chess.Board(fen)
    rows = []
    for nodes in GRID:
        engine.send("ucinewgame")
        rows += search_rows(engine, board, "position fen " + fen,
                            f"nodes {nodes}", ["mined", fen, nodes, 0])
    engine.close()
    return rows


def s170_one(engine_path, case, stride, nodes):
    name, fen, moves, start = case
    engine = Engine(engine_path)
    engine.send("ucinewgame")
    rows = []
    for ply in range(start, len(moves) + 1, stride):
        board = chess.Board(fen)
        for uci in moves[:ply]:
            board.push_uci(uci)
        position = f"position fen {fen}"
        if ply:
            position += " moves " + " ".join(moves[:ply])
        rows += search_rows(engine, board, position, f"nodes {nodes}",
                            ["s170", f"{name}/{stride}", nodes, ply])
    engine.close()
    return rows


MOVE_COLUMNS = ["set", "key", "nodes", "ply", "aborted_mate", "bestmove",
                "ponder"]


def bestmove_row(engine, position, spec, tag):
    """One search: whether it ended on an aborted iteration's line beside a
    mate, and its `bestmove` and `ponder` tokens ("-" for none)."""
    engine.send(position)
    engine.send("go " + spec)
    lines = engine.read_until("bestmove")
    infos = [i for i in map(parse_info, lines) if i]
    aborted_mate = (len(infos) >= 2 and infos[-1][0] == infos[-2][0]
                    and infos[-1][1] == "mate")
    token = lines[-1].split()
    ponder = token[3] if len(token) >= 4 and token[2] == "ponder" else "-"
    return tag + [int(aborted_mate), token[1], ponder]


def moves_mined_one(engine_path, fen):
    engine = Engine(engine_path)
    rows = []
    for nodes in GRID:
        engine.send("ucinewgame")
        rows.append(bestmove_row(engine, "position fen " + fen,
                                 f"nodes {nodes}", ["mined", fen, nodes, 0]))
    engine.close()
    return rows


def moves_s170_one(engine_path, case, stride, nodes):
    name, fen, moves, start = case
    engine = Engine(engine_path)
    engine.send("ucinewgame")
    rows = []
    for ply in range(start, len(moves) + 1, stride):
        position = f"position fen {fen}"
        if ply:
            position += " moves " + " ".join(moves[:ply])
        rows.append(bestmove_row(engine, position, f"nodes {nodes}",
                                 ["s170", f"{name}/{stride}", nodes, ply]))
    engine.close()
    return rows


def moves_stage(args):
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        if args.grid == "mined":
            results = list(pool.map(lambda f: moves_mined_one(args.engine, f),
                                    read_fens(MINED)))
        else:
            cells = [(c, s, n) for c in read_cases(CASES) for s in (1, 2)
                     for n in S170_BUDGETS]
            results = list(pool.map(
                lambda x: moves_s170_one(args.engine, *x), cells))
    with open(args.out, "w") as out:
        out.write(f"# S295_long_line.py moves --grid {args.grid}, engine "
                  f"{args.engine}, Hash {HASH_MB}\n")
        out.write("\t".join(MOVE_COLUMNS) + "\n")
        for rows in results:
            for row in rows:
                out.write("\t".join(str(x) for x in row) + "\n")
    return 0


def ponder_stage(args):
    before, after = read_rows(args.before), read_rows(args.after)
    if [(r["key"], r["nodes"], r["ply"]) for r in before] != \
            [(r["key"], r["nodes"], r["ply"]) for r in after]:
        print("the two files are not the same cells", file=sys.stderr)
        return 1
    count = collections.Counter()
    for b, a in zip(before, after):
        count["searches"] += 1
        count["aborted beside a mate"] += int(a["aborted_mate"])
        if b["bestmove"] != a["bestmove"]:
            count["bestmove differs"] += 1
        if (b["ponder"] == "-") != (a["ponder"] == "-"):
            count["ponder added" if b["ponder"] == "-" else
                  "ponder removed"] += 1
        elif b["ponder"] != a["ponder"]:
            count["ponder changed"] += 1
        else:
            continue
        print(f"  {a['key']}  nodes {a['nodes']}  ply {a['ply']}  aborted "
              f"beside a mate {a['aborted_mate']}  bestmove {b['bestmove']} "
              f"-> {a['bestmove']}  ponder {b['ponder']} -> {a['ponder']}")
    print(", ".join(f"{k} {v}" for k, v in count.items()))
    return 0


def write(path, header, results):
    with open(path, "w") as out:
        out.write("# " + header + "\n")
        out.write("\t".join(COLUMNS) + "\n")
        for rows in results:
            for row in rows:
                out.write("\t".join(str(x) for x in row) + "\n")


def sweep(args):
    fens = read_fens(MINED)
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        results = list(pool.map(lambda f: sweep_one(args.engine, f), fens))
    write(args.out, f"S295_long_line.py sweep, engine {args.engine}, "
          f"{os.path.relpath(MINED, ROOT)}, Hash {HASH_MB}, grid "
          f"{GRID[0]}..{GRID[-1]} by {GRID[1] - GRID[0]}", results)
    return 0


def s170(args):
    cells = [(c, s, n) for c in read_cases(CASES) for s in (1, 2)
             for n in S170_BUDGETS]
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        results = list(pool.map(lambda x: s170_one(args.engine, *x), cells))
    write(args.out, f"S295_long_line.py s170, engine {args.engine}, "
          f"{os.path.relpath(CASES, ROOT)}, Hash {HASH_MB}, strides 1 2, "
          f"budgets {' '.join(map(str, S170_BUDGETS))}", results)
    return 0


def read_rows(path):
    with open(path) as handle:
        lines = [l.rstrip("\n").split("\t") for l in handle
                 if not l.startswith("#")]
    head = lines[0]
    return [dict(zip(head, l)) for l in lines[1:]]


def last_lines(path):
    """(fen, nodes) -> (the last mate row, the row before it) of each cell."""
    cells = {}
    rows = read_rows(path)
    for i, row in enumerate(rows):
        if row["last"] != "1":
            continue
        prev = rows[i - 1] if i > 0 else None
        # The row before must be the cell's own `info` line just before it,
        # which is the completed iteration's when the last one is aborted.
        if prev is not None and not (
                prev["key"] == row["key"] and prev["nodes"] == row["nodes"]
                and prev["ply"] == row["ply"]
                and int(prev["line"]) == int(row["line"]) - 1):
            prev = None
        cells[(row["key"], int(row["nodes"]))] = (row, prev)
    return cells


def pick(args):
    fixed = last_lines(args.fixed)
    mutant = last_lines(args.mutant)
    order = read_fens(MINED)
    best = None
    separating = 0
    step = GRID[1] - GRID[0]
    for fen in order:
        run, longest, longest_end = 0, 0, None
        for nodes in GRID:
            f = fixed.get((fen, nodes))
            m = mutant.get((fen, nodes))
            ok = (f is not None and m is not None and f[1] is not None
                  and m[1] is not None
                  and f[0]["aborted"] == "1" and m[0]["aborted"] == "1"
                  and f[0]["same_first"] == "1" and m[0]["same_first"] == "1"
                  and f[0]["class"] == "full" and f[0]["pv"] == f[1]["pv"]
                  and m[0]["class"] == "wrong"
                  and int(m[0]["pv_len"]) > int(m[0]["needed"]))
            run = run + 1 if ok else 0
            if run > longest:
                longest, longest_end = run, nodes
        if longest == 0:
            continue
        separating += 1
        lo = longest_end - (longest - 1) * step
        print(f"separates  {longest:3d} budgets  {lo}..{longest_end}  {fen}")
        if best is None or longest > best[0]:
            best = (longest, lo, longest_end, fen)
    print(f"{separating} of {len(order)} positions separate")
    if best is None:
        print("no cell separates: the guard has no case on this tree")
        return 1
    longest, lo, hi, fen = best
    middle = lo + ((longest - 1) // 2) * step
    print(f"pick  {fen}  go nodes {middle}  (run {lo}..{hi}, {longest} "
          f"budgets, margin {middle - lo} nodes below and {hi - middle} "
          f"above)")
    return 0


def census(args):
    for path in args.files:
        rows = read_rows(path)
        total = collections.Counter(r["class"] for r in rows)
        aborted = collections.Counter(
            (r["class"], "same" if r["same_first"] == "1" else "other")
            for r in rows if r["aborted"] == "1")
        cells = len({(r["key"], r["nodes"]) for r in rows})
        print(f"{path}: {len(rows)} mate lines in {cells} cells reporting one;"
              f" full {total['full']}, short {total['short']}, wrong "
              f"{total['wrong']}")
        print(f"  of them from an aborted iteration: " + ", ".join(
            f"{c} {s} {aborted[(c, s)]}" for c in ("full", "short", "wrong")
            for s in ("same", "other")))
        for r in rows:
            if r["class"] == "wrong":
                print(f"  wrong  {r['key']}  nodes {r['nodes']}  ply "
                      f"{r['ply']}  depth {r['depth']}  aborted {r['aborted']}"
                      f"  same_first {r['same_first']}  mate {r['mate']}  pv "
                      f"{r['pv_len']}/{r['needed']}  {r['pv']}")
    return 0


def apply(args):
    """MW03's own pairs, read from the mutant file, applied to a worktree."""
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
    for name in ("sweep", "s170"):
        s = sub.add_parser(name)
        s.add_argument("--engine", required=True)
        s.add_argument("--out", required=True)
        s.add_argument("--jobs", type=int, default=os.cpu_count())
    p = sub.add_parser("pick")
    p.add_argument("--fixed", required=True)
    p.add_argument("--mutant", required=True)
    c = sub.add_parser("census")
    c.add_argument("files", nargs="+")
    mv = sub.add_parser("moves")
    mv.add_argument("--grid", choices=["mined", "s170"], required=True)
    mv.add_argument("--engine", required=True)
    mv.add_argument("--out", required=True)
    mv.add_argument("--jobs", type=int, default=os.cpu_count())
    pd = sub.add_parser("ponder")
    pd.add_argument("before")
    pd.add_argument("after")
    args = parser.parse_args()
    return {"apply": apply, "sweep": sweep, "s170": s170, "pick": pick,
            "census": census, "moves": moves_stage,
            "ponder": ponder_stage}[args.stage](args)


if __name__ == "__main__":
    sys.exit(main())
