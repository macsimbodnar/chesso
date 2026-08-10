#!/usr/bin/env python3
"""Is a move that cost centipawns a search error or an evaluation error?

`error_profile.py` says how much chesso gave away and in which phase. It does
not say why. A move can cost 200 cp because the search did not see far enough,
or because it saw far enough and liked the wrong position. The two answers
point at completely different work, so the question is settled by asking the
engine again with much more search and re-costing what it says.

Take the expensive moves out of a raw profile, re-ask chesso at its in-game
budget and at a multiple of it, and cost both answers with the same reference
the profile used.

    tools/depth_vs_eval.py adocs/data/S018_raw.tsv --out probe.tsv

Depth removes most of the cost -> horizon errors, and the payoff is in search.
Depth removes little              -> chesso still likes the same wrong move much
                                     deeper, and the payoff is in what it
                                     believes a position is worth.

DEC-033 is the run this was written for: 160 positions, 4M against 64M nodes,
24.1 % of the error removed and 95 of 160 moves unchanged.

Both engine limits are node counts, never depths or times, for the reason
recorded in DEC-030: depth is unbudgetable and movetime is not reproducible.
"""

import argparse
import csv
import random
import sys
import time
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from error_profile import Engine, clamp  # noqa: E402


# One engine pair per worker process, opened once and reused. Module level
# because ProcessPoolExecutor's initializer has nowhere else to put it.
_ENGINE = None
_REFERENCE = None
_CONFIG = None


def _init(engine_path, reference_path, config):
    global _ENGINE, _REFERENCE, _CONFIG
    _ENGINE = Engine(engine_path)
    _REFERENCE = Engine(reference_path)
    _CONFIG = config


def best_move(engine, fen, nodes):
    engine.new_game()
    engine._send("position fen " + fen)
    engine._send(f"go nodes {nodes}")

    while True:
        line = engine.p.stdout.readline()
        if not line:
            return "-"
        if line.startswith("bestmove"):
            return line.split()[1]


def cost_of(fen, ref_before, move, limit):
    """The profiler's definition: clamp(before) + clamp(after).

    Both scores are side-to-move relative, so a move that hands the opponent an
    advantage makes the two agree in sign and the sum is what was given away.
    The position after the move is reached by handing the reference the move
    rather than a new FEN, so nothing here has to know how to make a move.
    """
    # Cleared per position so a score never depends on which position this
    # worker happened to be handed before it.
    _REFERENCE.new_game()

    score, _, mate = _REFERENCE.evaluate(f"{fen} moves {move}", limit)
    return clamp(ref_before) + clamp(score), mate


def probe(record):
    fen, ref_before = record["fen"], int(record["ref"])
    base_nodes, deep_nodes, limit = _CONFIG

    base = best_move(_ENGINE, fen, base_nodes)
    deep = best_move(_ENGINE, fen, deep_nodes)

    base_cost, base_mate = cost_of(fen, ref_before, base, limit)

    # The same move costs the same thing, and the reference search is the
    # expensive half of this program.
    if deep == base:
        deep_cost, deep_mate = base_cost, base_mate
    else:
        deep_cost, deep_mate = cost_of(fen, ref_before, deep, limit)

    return {
        "game": record["game"],
        "ply": record["ply"],
        "phase": record["phase"],
        "ref": ref_before,
        "played_cost": int(record["cost"]),
        "base_move": base,
        "deep_move": deep,
        "base_cost": base_cost,
        "deep_cost": deep_cost,
        "mate": int(base_mate or deep_mate),
    }


def report(results, base_nodes, deep_nodes):
    n = len(results)
    played = sum(max(0, r["played_cost"]) for r in results) / n
    base = sum(max(0, r["base_cost"]) for r in results) / n
    deep = sum(max(0, r["deep_cost"]) for r in results) / n

    # log2 of the ratio, without pulling in math for one line.
    ratio, doublings = deep_nodes // base_nodes, 0
    while ratio > 1:
        ratio //= 2
        doublings += 1

    changed = [r for r in results if r["base_move"] != r["deep_move"]]

    print()
    print(f"sampled {n} positions")
    print(f"cost as played in the game   {played:7.1f} cp/move")
    print(f"cost at {base_nodes / 1e6:>5.1f}M nodes        {base:7.1f} cp/move")
    print(f"cost at {deep_nodes / 1e6:>5.1f}M nodes        {deep:7.1f} cp/move")

    if base > 0 and doublings:
        print(f"removed by the deeper search {100 * (base - deep) / base:6.1f} %"
              f", {(base - deep) / doublings:.1f} cp per doubling")

    print(f"move unchanged at the deeper search {n - len(changed)}/{n}")

    if changed:
        was = sum(max(0, r["base_cost"]) for r in changed) / len(changed)
        now = sum(max(0, r["deep_cost"]) for r in changed) / len(changed)
        print(f"where the move changed, {len(changed)} of {n}: "
              f"{was:.1f} -> {now:.1f} cp")

    for cut in (50, 100, 200):
        shallow = sum(1 for r in results if max(0, r["base_cost"]) >= cut)
        deeper = sum(1 for r in results if max(0, r["deep_cost"]) >= cut)
        print(f"still >= {cut:>3} cp: {shallow:>4}/{n} shallow, "
              f"{deeper:>4}/{n} deep")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("raw", help="raw records from error_profile.py --raw-out")
    ap.add_argument("--engine", default="build/src/chesso")
    ap.add_argument("--reference", default="stockfish")
    ap.add_argument("--base-nodes", type=int, default=4000000,
                    help="what the engine gets per move in the games profiled")
    ap.add_argument("--deep-nodes", type=int, default=64000000)
    ap.add_argument("--reference-nodes", type=int, default=3000000,
                    help="must match the profile being sampled, DEC-030")
    ap.add_argument("--min-cost", type=int, default=100,
                    help="only sample moves that gave away at least this much")
    ap.add_argument("--decided", type=int, default=300,
                    help="skip positions already won or lost by this margin; "
                         "cost given away in them does not decide games")
    ap.add_argument("--sample", type=int, default=160)
    ap.add_argument("--seed", type=int, default=18)
    ap.add_argument("--workers", type=int, default=3)
    ap.add_argument("--out", default="")
    args = ap.parse_args()

    with open(args.raw) as handle:
        rows = list(csv.DictReader(handle, delimiter="\t"))

    pool = [r for r in rows
            if int(r["mate"]) == 0
            and int(r["cost"]) >= args.min_cost
            and abs(int(r["ref"])) < args.decided]

    print(f"{len(pool)} candidates of {len(rows)} moves "
          f"(cost >= {args.min_cost}, |ref| < {args.decided}, no mate)",
          file=sys.stderr)

    if not pool:
        sys.exit("nothing to probe")

    random.seed(args.seed)
    sample = random.sample(pool, min(args.sample, len(pool)))

    config = (args.base_nodes, args.deep_nodes, f"nodes {args.reference_nodes}")
    results, started = [], time.time()

    with ProcessPoolExecutor(max_workers=args.workers, initializer=_init,
                             initargs=(args.engine, args.reference,
                                       config)) as workers:
        for done, result in enumerate(workers.map(probe, sample), 1):
            results.append(result)
            if done % 10 == 0:
                print(f"{done}/{len(sample)}  {time.time() - started:.0f}s",
                      file=sys.stderr)

    if args.out:
        fields = list(results[0].keys())
        with open(args.out, "w") as handle:
            handle.write("\t".join(fields) + "\n")
            for r in results:
                handle.write("\t".join(str(r[f]) for f in fields) + "\n")
        print(f"per-position records in {args.out}", file=sys.stderr)

    report(results, args.base_nodes, args.deep_nodes)


if __name__ == "__main__":
    main()
