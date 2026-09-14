#!/usr/bin/env python3
"""S230. Mine one mate position for the capture table of the search suite's
case "pruning does not hide a forced mate", chosen so that the S091 mutant
`R01_extra_reduction_gives_check` loses the mate where the shipped build finds
it.

    ~/.venv/chess/bin/python adocs/data/S230_mine_r01_row.py pool
    ~/.venv/chess/bin/python adocs/data/S230_mine_r01_row.py label  --nodes 50000
    ~/.venv/chess/bin/python adocs/data/S230_mine_r01_row.py line
    ~/.venv/chess/bin/python adocs/data/S230_mine_r01_row.py depths --lo 3 --hi 12
    ~/.venv/chess/bin/python adocs/data/S230_mine_r01_row.py join   \
        --shipped .tuning/coord/S230_shipped.txt

WHY THIS EXISTS. S222's continuation table reordered every quiet move, the
depth at which one row of that table separated its mutants moved with it, and
the table's own rule -- "the depth is where the shipped build reports the mate
and the mutant does not" -- removed the row rather than re-pick its depth
(DEC-209 clause 4). R01's kill inside the mate table went with it. A row that
restores it cannot be chosen, only measured, and this file is that measurement
end to end: pool, oracle label, oracle line, and the engine's own depth
profile.

WHAT R01 IS, AND WHAT THE ROW HAS TO CONTAIN. `tools/mutants/S091_capture_see.py`
deletes `!capture_gives_check` from `may_reduce` in `src/search.cpp`, so a
capture that gives check takes the extra reduction ply meant for a capture the
exchange evaluation writes off. A position separates it when its forced mating
line runs through a capture that both gives check and loses material, deep
enough that one reduced ply hides the mate and shallow enough that the shipped
build reports it at a depth in the swept range.

WHERE THE POSITIONS COME FROM. This project's own games and nothing else
(DEC-016): `adocs/data/S145_mined_set.tsv` and `adocs/data/S145_mate_set.tsv`,
the two committed mate sets, plus the tail of every game of
`adocs/data/S219_aa_calibration.pgn`, the 1000-game A/A corpus. No published
mate collection and no other engine's positions. Stockfish is run as a binary
and labels them; nothing is copied from it.

THE ORACLE IS ASKED THE ONLY TWO WAYS THAT DO NOT LIE. `python-chess` through
`chess.engine.SimpleEngine`, never a printf pipe -- TOOLCHAIN.md's oracle
section is what that rule is written from. `label` reuses one engine process
over the whole pool and is a shortlist only: a node-limited stockfish is
reproducible inside one identical call sequence and not across two, so no
number it prints reaches the test. `line` re-asks each survivor at a fixed
depth in a **fresh process per position**, and those are the facts that end up
in the row's comment.

THE ENGINE MEASUREMENT IS `search_fen()` AND NOT THE UCI BINARY. The case that
carries the row calls `search()` once at a fixed depth from a cold 4 MB table.
`go depth N` is iterative deepening over a table that carries between depths,
and the two disagree on exactly this class -- the table's own comment records
that the shipped engine answers its rows in centipawns over UCI at the depths
they are read at. So `depths` compiles a throwaway driver against the engine
library, line for line the test's `search_fen()`, and sweeps it. Re-derives
the `depth` field of every row of `capture_mates` in tests/test_search.cpp
(DEC-142); re-derives the mutant labels too, by being run again after a mutant
has been applied to the working tree and the library rebuilt:

    # shipped
    cmake --build build -j12
    ... depths --fens F --out .tuning/coord/S230_shipped.txt
    # under R01, applied by hand from tools/mutants/S091_capture_see.py
    cmake --build build -j12
    ... depths --fens F --out .tuning/coord/S230_R01.txt
    git checkout -- src/search.cpp && cmake --build build -j12

A row is separated at depth d when the shipped sweep reports the mate at d and
the mutant sweep does not.
"""

import argparse
import csv
import os
import subprocess
import sys
import tempfile

import chess
import chess.engine
import chess.pgn

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
STOCKFISH = "/usr/games/stockfish"

MINED_SET = os.path.join(REPO, "adocs", "data", "S145_mined_set.tsv")
MATE_SET = os.path.join(REPO, "adocs", "data", "S145_mate_set.tsv")
AA_PGN = os.path.join(REPO, "adocs", "data", "S219_aa_calibration.pgn")

# The pool and its labels are scratch: 40000 FENs and their shortlist labels
# are minutes of machine to regenerate and would be megabytes of derived data
# in a directory that is evidence. `S230_candidates.tsv`, the survivors with
# the oracle's own line, is the committed artefact.
SCRATCH = os.path.join(REPO, ".tuning", "coord")
POOL = os.path.join(SCRATCH, "S230_pool.txt")
LABELLED = os.path.join(SCRATCH, "S230_labelled.txt")
CANDIDATES = os.path.join(REPO, "adocs", "data", "S230_candidates.tsv")

# A capture the exchange evaluation is likely to write off. A shortlist filter
# and nothing more: the engine's own see_ge() is what the reduction rule reads,
# and the row's comment names the move it found.
VALUE = {chess.PAWN: 100, chess.KNIGHT: 300, chess.BISHOP: 300,
         chess.ROOK: 500, chess.QUEEN: 900, chess.KING: 10000}

# How many plies at the end of each A/A game enter the pool. The tail is where
# a forced mate lives: S145 measured 6.7 % of final positions carrying one and
# far less at a random ply.
TAIL_PLIES = 40


def fens_from_sets():
    """Every position of the two committed mate sets."""
    out = []
    for path in (MINED_SET, MATE_SET):
        with open(path) as handle:
            for line in handle:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                field = line.split("\t")
                if field[0] == "fen":
                    continue
                out.append(field[0])
    return out


def fens_from_pgn(path, tail):
    """The last `tail` positions of every game. python-chess replays the game;
    no position is tracked by hand (CLAUDE.md)."""
    out = []
    with open(path) as handle:
        while True:
            game = chess.pgn.read_game(handle)
            if game is None:
                break
            board = game.board()
            seen = []
            for move in game.mainline_moves():
                seen.append(board.fen())
                board.push(move)
            seen.append(board.fen())
            out.extend(seen[-tail:])
    return out


def cmd_pool(args):
    seen = set()
    order = []
    for fen in fens_from_sets() + fens_from_pgn(AA_PGN, args.tail):
        board = chess.Board(fen)
        if board.is_game_over():
            continue
        if fen in seen:
            continue
        seen.add(fen)
        order.append(fen)
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    with open(args.out, "w") as handle:
        for fen in order:
            handle.write(fen + "\n")
    print(f"pool {len(order)} positions -> {args.out}")


def cmd_label(args):
    """One engine process over the whole pool at a node limit. A shortlist:
    nothing printed here reaches a test."""
    fens = [line.strip() for line in open(args.pool) if line.strip()]
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    kept = 0
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        engine.configure({"Threads": 1, "Hash": 64})
        with open(args.out, "w") as handle:
            for index, fen in enumerate(fens):
                if index % 2000 == 0:
                    print(f"  {index}/{len(fens)} kept {kept}", flush=True)
                info = engine.analyse(chess.Board(fen),
                                      chess.engine.Limit(nodes=args.nodes))
                score = info["score"].relative
                if not score.is_mate():
                    continue
                mate = score.mate()
                if mate is None or mate < args.min_mate or mate > args.max_mate:
                    continue
                handle.write(f"{mate}\t{fen}\n")
                handle.flush()
                kept += 1
    print(f"labelled {kept} mates in [{args.min_mate},{args.max_mate}] -> {args.out}")


def losing_capture(board, move):
    """Crude sign of an exchange the mover comes out behind on: it takes less
    than it is worth, or the square it lands on is defended after the capture.
    A filter only -- see_ge() in src/search.cpp decides, and the row names the
    move it found."""
    if not board.is_capture(move):
        return False
    attacker = board.piece_at(move.from_square)
    if board.is_en_passant(move):
        taken = VALUE[chess.PAWN]
    else:
        taken = VALUE[board.piece_at(move.to_square).piece_type]
    gain = taken
    if move.promotion:
        gain += VALUE[move.promotion] - VALUE[chess.PAWN]
    board.push(move)
    defended = bool(board.attackers(board.turn, move.to_square))
    board.pop()
    return defended or VALUE[attacker.piece_type] > gain


def line_facts(fen, depth):
    """A fresh stockfish per position, `analyse` at a fixed depth. Returns the
    oracle's mate distance, its principal variation in SAN and UCI, and every
    move on it that the mating side plays as a capture giving check."""
    board = chess.Board(fen)
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        engine.configure({"Threads": 1, "Hash": 64})
        info = engine.analyse(board, chess.engine.Limit(depth=depth))
    score = info["score"].relative
    if not score.is_mate() or score.mate() is None or score.mate() <= 0:
        return None
    mover = board.turn
    pv = info.get("pv", [])
    walk = board.copy()
    uci = []
    san = []
    hits = []
    for move in pv:
        is_hit = (walk.turn == mover and walk.is_capture(move)
                  and walk.gives_check(move) and losing_capture(walk, move))
        text = walk.san(move)
        if is_hit:
            hits.append(text)
        uci.append(move.uci())
        san.append(text)
        walk.push(move)
    return {
        "fen": fen,
        "mate": score.mate(),
        "nodes": info.get("nodes", 0),
        "depth": info.get("depth", 0),
        "pv_uci": " ".join(uci),
        "pv_san": " ".join(san),
        "captures": " ".join(hits),
    }


def cmd_line(args):
    rows = []
    with open(args.labelled) as handle:
        for text in handle:
            text = text.strip()
            if not text:
                continue
            mate, fen = text.split("\t", 1)
            rows.append((int(mate), fen))
    kept = []
    for index, (_, fen) in enumerate(rows):
        facts = line_facts(fen, args.depth)
        if facts is None or not facts["captures"]:
            continue
        kept.append(facts)
        print(f"  {facts['mate']}\t{facts['captures']}\t{fen}", flush=True)
        if index % 50 == 0:
            print(f"  ..{index}/{len(rows)} kept {len(kept)}", flush=True)
    write_candidates(kept, {}, args.out)
    print(f"survivors {len(kept)} -> {args.out}")


def write_candidates(rows, depths, path):
    with open(path, "w", newline="") as handle:
        handle.write(
            "# S230 candidates for the capture mate row that separates R01.\n"
            "# Regenerate: adocs/data/S230_mine_r01_row.py pool / label / line / join.\n"
            "# distance, pv and nodes are stockfish through python-chess, one\n"
            "# fresh process per position; depths_found is the engine's own\n"
            "# search_fen() profile over the swept range.\n")
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["fen", "mate_in", "nodes", "pv_uci", "pv_san",
                         "captures_on_line", "depths_found"])
        for row in rows:
            writer.writerow([row["fen"], row["mate"], row["nodes"],
                             row["pv_uci"], row["pv_san"], row["captures"],
                             depths.get(row["fen"], "")])


DRIVER = r"""
// Generated by adocs/data/S230_mine_r01_row.py. search_fen() of
// tests/test_search.cpp, line for line, over a list of FENs and a depth range.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "search.hpp"
#include "transposition_table.hpp"

namespace
{
game_t game;
transposition_table_t tt;
std::atomic_bool never_stop = false;

search_t search_fen(const std::string& fen, int depth)
{
  if (!load_FEN(fen, &game)) {
    fprintf(stderr, "bad FEN: %s\n", fen.c_str());
    exit(2);
  }
  never_stop = false;
  tt_reset(&tt);
  tt_new_search(&tt);
  search_state_t state = {};
  state.tt = &tt;
  state.stop = &never_stop;
  state.node_limit = NODE_BUDGET_UNLIMITED;
  return search(depth, &game, &state);
}
}  // namespace

int main(int argc, char** argv)
{
  if (argc != 4) { return 2; }
  const int lo = atoi(argv[2]);
  const int hi = atoi(argv[3]);
  std::vector<std::string> fens;
  std::ifstream handle(argv[1]);
  std::string line;
  while (std::getline(handle, line)) {
    if (!line.empty() && line[0] != '#') { fens.push_back(line); }
  }
  initialize_game_const_data(&game);
  tt_resize(&tt, 4);
  tt_reset(&tt);
  for (const std::string& fen : fens) {
    for (int depth = lo; depth <= hi; ++depth) {
      const search_t result = search_fen(fen, depth);
      if (result.mate_found) { printf("%d ", result.mate_in); }
      else { printf("- "); }
      fflush(stdout);
    }
    printf("\t%s\n", fen.c_str());
    fflush(stdout);
  }
  return 0;
}
"""


def cmd_depths(args):
    """Compile the driver against the engine library **as it stands** and sweep.
    Run it once on a clean tree and once per applied mutant; the difference is
    the separation."""
    work = tempfile.mkdtemp(prefix="s230_")
    source = os.path.join(work, "driver.cpp")
    binary = os.path.join(work, "driver")
    with open(source, "w") as handle:
        handle.write(DRIVER)
    build = subprocess.run(
        ["c++", "-I", os.path.join(REPO, "src"), "-O3", "-DNDEBUG",
         "-std=gnu++20", "-march=native", "-o", binary, source,
         os.path.join(REPO, args.lib)],
        capture_output=True, text=True)
    if build.returncode != 0:
        sys.stderr.write(build.stderr)
        return 1
    run = subprocess.run([binary, args.fens, str(args.lo), str(args.hi)],
                         capture_output=True, text=True)
    sys.stderr.write(run.stderr)
    if run.returncode != 0:
        return run.returncode
    with open(args.out, "w") as handle:
        handle.write(run.stdout)
    print(run.stdout, end="")
    print(f"swept {args.lo}..{args.hi} -> {args.out}")
    return 0


def read_sweep(path, lo):
    """`d7 d9 d10` for the depths a sweep file reports a mate at."""
    out = {}
    with open(path) as handle:
        for line in handle:
            if "\t" not in line:
                continue
            cells, fen = line.rstrip("\n").split("\t", 1)
            found = [f"d{lo + i}" for i, cell in enumerate(cells.split())
                     if cell != "-"]
            out[fen] = " ".join(found) if found else "none"
    return out


def cmd_join(args):
    rows = []
    with open(args.candidates) as handle:
        reader = csv.reader((l for l in handle if not l.startswith("#")),
                            delimiter="\t")
        header = next(reader)
        for row in reader:
            item = dict(zip(header, row))
            rows.append({"fen": item["fen"], "mate": int(item["mate_in"]),
                         "nodes": int(item["nodes"]), "pv_uci": item["pv_uci"],
                         "pv_san": item["pv_san"],
                         "captures": item["captures_on_line"]})
    shipped = read_sweep(args.shipped, args.lo)
    write_candidates(rows, shipped, args.candidates)
    print(f"joined {len(shipped)} sweeps -> {args.candidates}")


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="stage", required=True)

    one = sub.add_parser("pool")
    one.add_argument("--tail", type=int, default=TAIL_PLIES)
    one.add_argument("--out", default=POOL)
    one.set_defaults(run=cmd_pool)

    two = sub.add_parser("label")
    two.add_argument("--pool", default=POOL)
    two.add_argument("--nodes", type=int, default=50000)
    two.add_argument("--min-mate", type=int, default=2)
    two.add_argument("--max-mate", type=int, default=6)
    two.add_argument("--out", default=LABELLED)
    two.set_defaults(run=cmd_label)

    three = sub.add_parser("line")
    three.add_argument("--labelled", default=LABELLED)
    three.add_argument("--depth", type=int, default=20)
    three.add_argument("--out", default=CANDIDATES)
    three.set_defaults(run=cmd_line)

    four = sub.add_parser("depths")
    four.add_argument("--fens", required=True)
    four.add_argument("--lo", type=int, default=3)
    four.add_argument("--hi", type=int, default=12)
    four.add_argument("--lib", default="build/src/libchesso_engine.a")
    four.add_argument("--out", required=True)
    four.set_defaults(run=cmd_depths)

    five = sub.add_parser("join")
    five.add_argument("--candidates", default=CANDIDATES)
    five.add_argument("--shipped", required=True)
    five.add_argument("--lo", type=int, default=3)
    five.set_defaults(run=cmd_join)

    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
