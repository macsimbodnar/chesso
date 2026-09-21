#!/usr/bin/env python3
"""S097 verdict 2. Mine one mate position for `tests/test_search.cpp` "pruning
does not hide a forced mate", chosen so that the **multicut with its mate-range
guard dropped** loses the mate where the shipped, guarded build finds it.

Five stages, and this is the order they run in. Every one names its inputs,
because a default that points at a file an earlier stage has not written yet
is a recipe nobody can follow -- which is what the first version of this
header did.

    P=~/.venv/chess/bin/python
    C=.tuning/coord

    # 1. the candidate set, from S230's labelled mates
    $P adocs/data/S097_mine_mate_row.py candidates \
        --labelled $C/S230_labelled.txt \
        --out adocs/data/S097_candidates.tsv --fens $C/S097_candidates.fen

    # 2. the shipped sweep, over every candidate
    $P adocs/data/S097_mine_mate_row.py sweep \
        --fens $C/S097_candidates.fen \
        --lib build/src/libchesso_engine.a --out $C/S097_shipped.txt

    # 3. the same sweep against a library built with E21 applied. Build it
    #    out of tree so the shipped one is not disturbed and both sweeps can
    #    run at once: apply E21 to src/search.cpp by hand, configure and
    #    build `build-e21`, then `git checkout -- src/search.cpp`.
    $P adocs/data/S097_mine_mate_row.py sweep \
        --fens $C/S097_candidates.fen \
        --lib build-e21/src/libchesso_engine.a --out $C/S097_open.txt

    # 4. the separator set: the candidates the two sweeps disagree on. Derived
    #    and never written by hand -- it is what stage 5 is run over, and a
    #    firing witness over all 269 costs twice what a sweep does.
    $P adocs/data/S097_mine_mate_row.py separators \
        --shipped $C/S097_shipped.txt --open $C/S097_open.txt \
        --out $C/S097_separators.fen

    # 5. the firing witness, on the tune library, over the separators only
    $P adocs/data/S097_mine_mate_row.py fires \
        --fens $C/S097_separators.fen --out $C/S097_fires.txt \
        --fens-out $C/S097_fires.fen

    # 6. the pick, which applies the rule below to all three
    $P adocs/data/S097_mine_mate_row.py pick \
        --shipped $C/S097_shipped.txt --open $C/S097_open.txt \
        --fires $C/S097_fires.txt

WHY THIS EXISTS. S097's second verdict lets the verification search **return**:
where it fails high at or above the node's own beta, some move other than the
table's already clears a bar just under the entry's score at a reduced depth,
and the node ends on that score without being searched. That is pruning, and
pruning that hides a mate is this repository's recurring bug -- null move
pruning hid a mate in two by reducing to depth 0, late move reduction reduced
the mating move at the root, and both were caught by a mate case and by no
benchmark (CLAUDE.md). The step's accepts asks for a forced mate **inside the
multicut's pruned depth**, added beside the existing rows and **observed red
with the guard removed**. A row like that cannot be chosen by reading a board
(CLAUDE.md's CHESS rule, DEC-023); it is measured, and this file is the
measurement.

THE MUTATION THE RED IS OBSERVED UNDER. `E21_multicut_mate_band_gate_dropped`
of `tools/mutants/S097_singular_extension.py`: the two terms that keep the
returned value out of the mate band come off the multicut's condition in
`src/search.cpp` `negamax_at`, leaving

    } else if (SE_MULTICUT != 0 && vscore >= beta && !is_pv &&
               beta > -MATE_MIN) {

so a half-depth search under a window below the entry's score may hand a mate
distance back as the node's own value. `beta > -MATE_MIN` stays: that is S165's
guard on the node's window and a different rule. Applied by hand, observed,
reverted -- the S033 protocol -- and never committed.

WHERE THE POSITIONS COME FROM. This project's own games and nothing else
(DEC-016), through S230's pool: `adocs/data/S145_mined_set.tsv`,
`adocs/data/S145_mate_set.tsv` and the tail of every game of
`adocs/data/S219_aa_calibration.pgn`. No published mate collection and no other
engine's positions. Stockfish is run as a binary and labels them; nothing is
copied from it (DEC-016).

WHAT THIS FILE REUSES AND WHAT IS ITS OWN. It **imports** `S230_mine_r01_row`
rather than editing or copying it -- this directory is append-only, and S095
importing S230 is the precedent. Reused whole: the pool, the node-limited
shortlist, and the fresh-process oracle call. What is S097's own is everything
below, because the multicut needs two things S230's sweep does not measure.

  THE SWEPT RANGE IS NOT 3 TO 12. The block only looks at a node whose
  remaining depth is at least `SeMinDepth` and whose ply is above 0, and the
  root of a `search_fen()` drive is at ply 0, so the shallowest fixed depth at
  which any node of the tree can multicut at all is **`SeMinDepth` + 1**. A
  sweep from 3 would be eleven depths at which the rule cannot fire by
  construction. The defaults below are derived from the parameter and printed
  by `--help`, not written out, so a refit that moves `SeMinDepth` moves the
  sweep with it.

  THE RULE HAS TO BE SHOWN TO FIRE ON THE POSITION, and by the engine rather
  than by an argument (CHESS). `fires` is that witness and it needs no change
  to `src/`: it drives the **tune** library, where `SeMultiCut` is a variable,
  and runs each position at each depth twice -- once at 0 and once at 1. A
  position whose node count or mate reading moves between the two is a position
  the rule reaches; one where both are identical at every depth is one the
  block never returns at, and no red under E21 could come from this rule there.

  NODES AND MILLISECONDS PER CELL. A row of that table is read at one depth by
  a case the gate runs on every commit, so what the row costs is part of
  choosing it.

THE ENGINE MEASUREMENT IS `search_fen()` AND NOT THE UCI BINARY, for the reason
S230's header gives: the case that carries the row calls `search()` once at a
fixed depth from a cold table, `go depth N` is iterative deepening over a table
that carries between depths, and the two disagree on exactly this class. The
driver below is `search_fen()` of `tests/test_search.cpp` line for line.

THE PICK RULE, stated before the sweep runs (DEC-209 clause 4). A candidate is
taken when

  1. `fires` shows the multicut changing the tree at some depth of the swept
     range, so the rule is live on the position;
  2. the shipped build reports a mate at some depth of the range and the
     guard-dropped build does not report it at that same depth.

THE ORDER THE TWO WERE EVALUATED IN, 2026-09-21, and it is not the order above.
The two sweeps are the decisive measurement and `fires` costs twice what one of
them does, so on a three-hour machine box the mutant sweeps were run over the
whole candidate set first and `fires` over the survivors afterwards. **The
predicate is unchanged**: a row is taken only if it satisfies both, both are
evaluated on it, and which row wins does not depend on the order the two
filters are applied in. `pick --fires` is what enforces the first, and it
prints and skips a separating candidate the rule never reaches.

The row's depth is the **lowest** such depth and its `mate_in` is the distance
the shipped build reports there. What it must never be is a depth picked
because the row passes there. Where several candidates qualify, take the one
whose shipped profile is the longest run of consecutive depths -- a row that
holds over several depths is a row an ordinary ordering change will not
silently take away -- and break a remaining tie by the cheaper cell.
"""

import argparse
import csv
import os
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S230_mine_r01_row as S230  # noqa: E402

REPO = S230.REPO
SCRATCH = S230.SCRATCH

# The pool, the shortlist, the FEN lists and the two sweeps are scratch:
# minutes of machine to regenerate and megabytes of derived data in a directory
# that is evidence. `S097_candidates.tsv`, the survivors with the oracle's own
# line, is the committed artefact -- S230's split, and for its reason.
CANDIDATES = os.path.join(REPO, "adocs", "data", "S097_candidates.tsv")
CANDIDATE_FENS = os.path.join(SCRATCH, "S097_candidates.fen")
SEPARATORS = os.path.join(SCRATCH, "S097_separators.fen")
FIRES = os.path.join(SCRATCH, "S097_fires.txt")
FIRES_FENS = os.path.join(SCRATCH, "S097_fires.fen")


def se_min_depth():
    """`SeMinDepth` as the engine compiles it, read from the one list both
    builds are generated from. Written out nowhere in this file: a refit that
    moves the parameter moves this sweep's floor with it."""
    path = os.path.join(REPO, "src", "search_params.hpp")

    with open(path) as handle:
        for line in handle:
            if '"SeMinDepth"' in line and line.lstrip().startswith("X("):
                fields = [f.strip() for f in
                          line[line.index("(") + 1:line.rindex(")")].split(",")]
                return int(fields[2])

    raise SystemExit("SeMinDepth is not in src/search_params.hpp's X-macro")


# The shallowest fixed depth at which any node of a `search_fen()` tree can
# reach the block: the root is at ply 0 and the block wants `ply > 0` with a
# remaining depth of at least `SeMinDepth`, so the root has to be one deeper.
LO = se_min_depth() + 1
HI = LO + 3


def cmd_candidates(args):
    """Every labelled mate re-asked in a fresh process at a fixed depth, kept
    where the oracle still calls it a forced mate for the side to move at a
    distance the swept range can carry.

    The distance floor is 2 and it is the one chess-shaped filter here: a mate
    in one is answered at the root by any search and cannot be hidden inside a
    node the multicut returns for. Everything past that is left to the sweep,
    which is the only thing that can say whether the guard actually holds a
    mate up."""
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
        facts = S230.line_facts(fen, args.depth)

        if facts is None:
            continue

        if facts["mate"] < args.min_mate or facts["mate"] > args.max_mate:
            continue

        kept.append(facts)

        if index % 50 == 0:
            print(f"  ..{index}/{len(rows)} kept {len(kept)}", flush=True)

    write_candidates(kept, {}, args.out)

    with open(args.fens, "w") as handle:
        for row in kept:
            handle.write(row["fen"] + "\n")

    print(f"candidates {len(kept)} -> {args.out}, {args.fens}")
    return 0


def write_candidates(rows, profiles, path):
    with open(path, "w", newline="") as handle:
        handle.write(
            "# S097 verdict 2 candidates for the mate row that separates\n"
            "# E21_multicut_mate_band_gate_dropped.\n"
            "# Regenerate: adocs/data/S097_mine_mate_row.py candidates /\n"
            "# fires / sweep / pick.\n"
            "# distance, pv and nodes are stockfish through python-chess, one\n"
            "# fresh process per position; depths_found is the engine's own\n"
            "# search_fen() profile over the swept range.\n")
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["fen", "mate_in", "nodes", "pv_uci", "pv_san",
                         "depths_found"])

        for row in rows:
            writer.writerow([row["fen"], row["mate"], row["nodes"],
                             row["pv_uci"], row["pv_san"],
                             profiles.get(row["fen"], "")])


# `search_fen()` of tests/test_search.cpp, line for line, over a list of FENs
# and a depth range. Two things S230's driver does not print: the node count,
# which is what a firing witness reads, and the wall time of the cell, which is
# what the row costs the gate. In the tune build (-DCHESSO_TUNE) it also takes
# a `SeMultiCut` value and sets it before the sweep.
DRIVER = r"""
// Generated by adocs/data/S097_mine_mate_row.py.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "search.hpp"
#include "search_params.hpp"
#include "transposition_table.hpp"

namespace
{
game_t game;
transposition_table_t tt;
std::atomic_bool never_stop = false;
uint64_t last_nodes = 0;

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
  const search_t out = search(depth, &game, &state);
  last_nodes = state.explored_nodes;
  return out;
}
}  // namespace

int main(int argc, char** argv)
{
  if (argc != 5) { return 2; }
  const int lo = atoi(argv[2]);
  const int hi = atoi(argv[3]);
  const int multicut = atoi(argv[4]);
#ifdef CHESSO_TUNE
  if (!search_param_set("SeMultiCut", multicut)) {
    fprintf(stderr, "SeMultiCut %d refused\n", multicut);
    return 2;
  }
#else
  if (multicut != SE_MULTICUT) {
    fprintf(stderr, "this build compiles SeMultiCut %d, not %d\n",
            SE_MULTICUT, multicut);
    return 2;
  }
#endif
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
      const auto start = std::chrono::steady_clock::now();
      const search_t result = search_fen(fen, depth);
      const auto stop = std::chrono::steady_clock::now();
      const long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          stop - start).count();
      if (result.mate_found) { printf("m%d", result.mate_in); }
      else { printf("-"); }
      printf(":%llu:%ld ", (unsigned long long)last_nodes, ms);
      fflush(stdout);
    }
    printf("\t%s\n", fen.c_str());
    fflush(stdout);
  }
  return 0;
}
"""


def build_driver(lib, tune):
    """Compile the driver against the engine library **as it stands**. Run it
    once on a clean tree and once per applied mutant; the difference is the
    separation."""
    work = tempfile.mkdtemp(prefix="s097_")
    source = os.path.join(work, "driver.cpp")
    binary = os.path.join(work, "driver")

    with open(source, "w") as handle:
        handle.write(DRIVER)

    command = ["c++", "-I", os.path.join(REPO, "src"), "-O3", "-DNDEBUG",
               "-std=gnu++20", "-march=native"]

    if tune:
        command.append("-DCHESSO_TUNE")

    command += ["-o", binary, source, os.path.join(REPO, lib)]
    build = subprocess.run(command, capture_output=True, text=True)

    if build.returncode != 0:
        sys.stderr.write(build.stderr)
        return None

    return binary


def run_driver(binary, fens, lo, hi, multicut):
    run = subprocess.run([binary, fens, str(lo), str(hi), str(multicut)],
                         capture_output=True, text=True)
    sys.stderr.write(run.stderr)

    if run.returncode != 0:
        return None

    return run.stdout


def cmd_sweep(args):
    binary = build_driver(args.lib, args.tune)

    if binary is None:
        return 1

    out = run_driver(binary, args.fens, args.lo, args.hi, args.multicut)

    if out is None:
        return 1

    with open(args.out, "w") as handle:
        handle.write(out)

    print(out, end="")
    print(f"swept {args.lo}..{args.hi} at SeMultiCut {args.multicut} "
          f"-> {args.out}")
    return 0


def cmd_fires(args):
    """The firing witness, on the tune library so the switch is a variable.
    Each position is swept twice and a cell where the node count or the mate
    reading differs is the rule changing this position's tree."""
    binary = build_driver(args.lib, True)

    if binary is None:
        return 1

    start = time.time()
    off = run_driver(binary, args.fens, args.lo, args.hi, 0)
    on = run_driver(binary, args.fens, args.lo, args.hi, 1)

    if off is None or on is None:
        return 1

    off_rows = read_sweep(off, args.lo)
    on_rows = read_sweep(on, args.lo)
    kept = []

    with open(args.out, "w") as handle:
        handle.write(f"# S097 firing witness, SeMultiCut 0 against 1, depths "
                     f"{args.lo}..{args.hi}, tune library {args.lib}.\n")
        handle.write("# fen\tdepths_the_rule_changes\toff\ton\n")

        for fen, off_cells in off_rows.items():
            on_cells = on_rows.get(fen)

            if on_cells is None:
                continue

            # `mate:nodes` and never the milliseconds. The third field is wall
            # time and it differs between two runs of the **same** binary, so
            # comparing whole cells reports every position as changed -- which
            # a 20-position probe did, 20 of 20, with identical node counts in
            # every cell of every one of them.
            moved = [f"d{args.lo + i}" for i, cell in enumerate(off_cells)
                     if answer(cell) != answer(on_cells[i])]
            handle.write(f"{fen}\t{' '.join(moved) or 'none'}\t"
                         f"{' '.join(off_cells)}\t{' '.join(on_cells)}\n")

            if moved:
                kept.append(fen)

    with open(args.fens_out, "w") as handle:
        for fen in kept:
            handle.write(fen + "\n")

    print(f"the multicut changes the tree on {len(kept)} of "
          f"{len(off_rows)} positions in {time.time() - start:.0f} s "
          f"-> {args.out}, {args.fens_out}")
    return 0


def read_sweep(text, lo):
    """`{fen: ['m2:1234:7', '-:2345:9', ...]}` from a sweep's own output, in
    depth order. `lo` is the caller's because a sweep file does not record the
    range it was taken over -- every reader below has to be handed the same
    one the sweep ran at, which is why it is an argument and not a guess."""
    del lo
    out = {}

    for line in text.splitlines():
        if "\t" not in line:
            continue

        cells, fen = line.split("\t", 1)
        out[fen] = cells.split()

    return out


def read_sweep_file(path, lo):
    with open(path) as handle:
        return read_sweep(handle.read(), lo)


def answer(cell):
    """What a cell says about the search, with the wall time dropped: the mate
    reading and the node count, which are the two a rebuild cannot move on its
    own. Comparing whole cells compares milliseconds and finds a difference
    everywhere."""
    return cell.rsplit(":", 1)[0]


def mate_at(cell):
    """The distance a cell reports, or None where the build found no mate."""
    head = cell.split(":", 1)[0]
    return int(head[1:]) if head.startswith("m") else None


def cost_at(cell):
    nodes, ms = cell.split(":")[1:3]
    return int(nodes), int(ms)


def longest_run(depths):
    best = run = 0
    previous = None

    for depth in depths:
        run = run + 1 if previous is not None and depth == previous + 1 else 1
        best = max(best, run)
        previous = depth

    return best


def cmd_separators(args):
    """The candidates the two sweeps disagree on, in sweep order, as a FEN
    list `fires` can be run over.

    Both classes go in -- the mate lost and the distance moved -- because the
    witness is asked about a position and not about a reading, and `pick` is
    what applies the narrow rule afterwards. Deriving this set is what keeps
    the recipe followable: the witness costs twice what a sweep does, so it is
    not run over every candidate, and a hand-written list of survivors would
    be a step of the method that no command reproduces."""
    shipped = read_sweep_file(args.shipped, args.lo)
    opened = read_sweep_file(args.open, args.lo)
    kept = []

    for fen, cells in shipped.items():
        other = opened.get(fen)

        if other is None:
            continue

        if any(mate_at(cell) is not None
               and mate_at(other[i]) != mate_at(cell)
               for i, cell in enumerate(cells)):
            kept.append(fen)

    os.makedirs(os.path.dirname(args.out), exist_ok=True)

    with open(args.out, "w") as handle:
        for fen in kept:
            handle.write(fen + "\n")

    print(f"{len(kept)} of {len(shipped)} candidates separate the two sweeps "
          f"-> {args.out}")
    return 0


def cmd_pick(args):
    shipped = read_sweep_file(args.shipped, args.lo)
    opened = read_sweep_file(args.open, args.lo)
    fires = {}

    if args.fires:
        with open(args.fires) as handle:
            for line in handle:
                if line.startswith("#") or "\t" not in line:
                    continue

                fen, moved = line.rstrip("\n").split("\t")[:2]
                fires[fen] = moved

    rows = []

    for fen, cells in shipped.items():
        other = opened.get(fen)

        if other is None:
            continue

        found = [args.lo + i for i, cell in enumerate(cells)
                 if mate_at(cell) is not None]
        # The header's rule to the letter: the shipped build reports the mate
        # and the guard-dropped build **does not report it**. A cell where the
        # mutant reports a different *distance* is a red case too, and the
        # first implementation of this line counted it -- but that is not what
        # the rule pre-registered, so those rows are printed as their own
        # class and never taken. DEC-209 clause 4 is why the two are kept
        # apart instead of the wider reading being adopted once the data was
        # in: both readings were available before the sweep ran and the
        # narrow one is the one that was written down.
        separating = [args.lo + i for i, cell in enumerate(cells)
                      if mate_at(cell) is not None
                      and mate_at(other[i]) is None]
        distance_moved = [args.lo + i for i, cell in enumerate(cells)
                          if mate_at(cell) is not None
                          and mate_at(other[i]) is not None
                          and mate_at(other[i]) != mate_at(cell)]

        if not separating:
            if distance_moved:
                cell = cells[distance_moved[0] - args.lo]
                other_cell = other[distance_moved[0] - args.lo]
                print(f"  distance moved, not taken by the rule: "
                      f"d{distance_moved[0]} #{mate_at(cell)} -> "
                      f"#{mate_at(other_cell)}\t{fen}")
            continue

        if fires and fires.get(fen, "none") == "none":
            print(f"  skipped, the rule never fires here: {fen}")
            continue

        depth = separating[0]
        cell = cells[depth - args.lo]
        nodes, ms = cost_at(cell)
        rows.append({"fen": fen, "depth": depth, "mate_in": mate_at(cell),
                     "profile": " ".join(f"d{d}" for d in found),
                     "run": longest_run(found), "nodes": nodes, "ms": ms,
                     "open": " ".join(f"d{args.lo + i}"
                                      for i, c in enumerate(other)
                                      if mate_at(c) is not None) or "none",
                     "fires": fires.get(fen, "")})

    rows.sort(key=lambda row: (-row["run"], row["ms"], row["depth"]))

    for row in rows:
        print(f"  d{row['depth']}\t#{row['mate_in']}\t"
              f"run {row['run']}\t{row['nodes']} nodes\t{row['ms']} ms\t"
              f"shipped [{row['profile']}]\topen [{row['open']}]\t"
              f"fires [{row['fires']}]\t{row['fen']}")

    if not rows:
        print("no candidate separates the guard in this range")
        return 1

    taken = rows[0]
    print(f"\ntake: {taken['fen']}\n  depth {taken['depth']}, "
          f"mate in {taken['mate_in']}, shipped [{taken['profile']}], "
          f"guard dropped [{taken['open']}], {taken['nodes']} nodes, "
          f"{taken['ms']} ms")
    return 0


def main():
    parser = argparse.ArgumentParser(
        description=f"the swept range defaults to {LO}..{HI}, derived from "
                    f"SeMinDepth")
    sub = parser.add_subparsers(dest="stage", required=True)

    one = sub.add_parser("candidates")
    one.add_argument("--labelled", default=S230.LABELLED)
    one.add_argument("--depth", type=int, default=20)
    one.add_argument("--min-mate", type=int, default=2)
    one.add_argument("--max-mate", type=int, default=6)
    one.add_argument("--out", default=CANDIDATES)
    one.add_argument("--fens", default=CANDIDATE_FENS)
    one.set_defaults(run=cmd_candidates)

    two = sub.add_parser("fires")
    two.add_argument("--fens", default=SEPARATORS)
    two.add_argument("--lo", type=int, default=LO)
    two.add_argument("--hi", type=int, default=HI)
    two.add_argument("--lib", default="build-tune/src/libchesso_engine.a")
    two.add_argument("--out", default=FIRES)
    two.add_argument("--fens-out", default=FIRES_FENS)
    two.set_defaults(run=cmd_fires)

    three = sub.add_parser("sweep")
    three.add_argument("--fens", default=CANDIDATE_FENS)
    three.add_argument("--lo", type=int, default=LO)
    three.add_argument("--hi", type=int, default=HI)
    three.add_argument("--lib", default="build/src/libchesso_engine.a")
    three.add_argument("--tune", action="store_true")
    three.add_argument("--multicut", type=int, default=1)
    three.add_argument("--out", required=True)
    three.set_defaults(run=cmd_sweep)

    sep = sub.add_parser("separators")
    sep.add_argument("--shipped", required=True)
    sep.add_argument("--open", required=True)
    sep.add_argument("--lo", type=int, default=LO)
    sep.add_argument("--out", default=SEPARATORS)
    sep.set_defaults(run=cmd_separators)

    four = sub.add_parser("pick")
    four.add_argument("--shipped", required=True)
    four.add_argument("--open", required=True)
    four.add_argument("--fires", default=FIRES)
    four.add_argument("--lo", type=int, default=LO)
    four.set_defaults(run=cmd_pick)

    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
