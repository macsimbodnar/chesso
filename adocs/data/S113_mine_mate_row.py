#!/usr/bin/env python3
"""S113. Mine one mate position for `tests/test_search.cpp` "pruning does not
hide a forced mate", chosen so that **ProbCut with its defender mate-band
guard dropped** (`B04_probcut_defender_gate_dropped` of
tools/mutants/S113_probcut.py) loses the mate where the shipped, guarded build
finds it. The accepts' own clause: "a mate inside the pruned depth is in the
fast suite and observed red with the guard removed".

S097's miner, re-pointed, and nothing else. Every stage and every rule is
`adocs/data/S097_mine_mate_row.py`'s, imported rather than copied (this
directory is append-only and S097 importing S230 is the precedent), with three
differences, all of them the rule's and not the method's:

  THE CANDIDATES are S097's own committed set, `adocs/data/S097_candidates.tsv`:
  269 labelled mates in 2 to 6 from this project's own games, each re-asked of
  stockfish at depth 20 in a fresh process by S097. Nothing is re-labelled
  here. `fens` writes the FEN column out as the list every stage reads.

  THE SWITCH the driver sets is `ProbCut`, not `SeMultiCut`. S097's driver is
  a module-level string and is re-pointed below by a replace of the two
  names, so its output lines still say "SeMultiCut" where they print the
  value: read them as `ProbCut`.

  THE SWEPT RANGE is `ProbCutMinDepth + 1` to `+ 4`: the block wants `ply > 0`
  at a remaining depth of at least `ProbCutMinDepth`, and a `search_fen()`
  root is at ply 0, so the shallowest fixed depth any node of the tree can
  reach it at is one deeper. Derived from the parameter, not written out.

THE PICK RULE is S097's, stated before the sweep runs (DEC-209 clause 4): the
rule fires on the position (ProbCut 0 against 1 moves a cell of the tune
build's sweep), the shipped build reports a mate at some depth of the range
and the mutant does not at that same depth; the lowest such depth, the longest
consecutive shipped profile first, then the cheaper cell.

    P=~/.venv/chess/bin/python
    C=.tuning/coord
    $P adocs/data/S113_mine_mate_row.py fens --out $C/S113_candidates.fen
    $P adocs/data/S113_mine_mate_row.py sweep --fens $C/S113_candidates.fen \\
        --lib build/src/libchesso_engine.a --out $C/S113_shipped.txt
    # apply B04 by hand to src/search.cpp, build `build-b04`, revert
    $P adocs/data/S113_mine_mate_row.py sweep --fens $C/S113_candidates.fen \\
        --lib build-b04/src/libchesso_engine.a --out $C/S113_open.txt
    $P adocs/data/S113_mine_mate_row.py separators \\
        --shipped $C/S113_shipped.txt --open $C/S113_open.txt \\
        --out $C/S113_separators.fen
    $P adocs/data/S113_mine_mate_row.py fires --fens $C/S113_separators.fen \\
        --lib build-tune/src/libchesso_engine.a \\
        --out $C/S113_fires.txt --fens-out $C/S113_fires.fen
    $P adocs/data/S113_mine_mate_row.py pick \\
        --shipped $C/S113_shipped.txt --open $C/S113_open.txt \\
        --fires $C/S113_fires.txt
"""

import argparse
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S097_mine_mate_row as S097  # noqa: E402

REPO = S097.REPO
CANDIDATES = os.path.join(REPO, "adocs", "data", "S097_candidates.tsv")

S097.DRIVER = (S097.DRIVER.replace('"SeMultiCut"', '"ProbCut"')
               .replace("SE_MULTICUT", "PROBCUT")
               .replace("S097_mine_mate_row.py", "S113_mine_mate_row.py"))


def probcut_min_depth():
    path = os.path.join(REPO, "src", "search_params.hpp")

    with open(path) as handle:
        for line in handle:
            if '"ProbCutMinDepth"' in line and line.lstrip().startswith("X("):
                fields = [f.strip() for f in
                          line[line.index("(") + 1:line.rindex(")")].split(",")]
                return int(fields[2])

    raise SystemExit("ProbCutMinDepth is not in src/search_params.hpp")


LO = probcut_min_depth() + 1
HI = LO + 3


def cmd_fens(args):
    with open(CANDIDATES) as handle:
        rows = [line for line in handle if not line.startswith("#")]

    reader = csv.DictReader(rows, delimiter="\t")
    fens = [row["fen"] for row in reader]

    with open(args.out, "w") as handle:
        for fen in fens:
            handle.write(fen + "\n")

    print(f"{len(fens)} candidates -> {args.out}")
    return 0


def main():
    parser = argparse.ArgumentParser(
        description=f"the swept range defaults to {LO}..{HI}, derived from "
                    f"ProbCutMinDepth")
    sub = parser.add_subparsers(dest="stage", required=True)

    zero = sub.add_parser("fens")
    zero.add_argument("--out", required=True)
    zero.set_defaults(run=cmd_fens)

    two = sub.add_parser("fires")
    two.add_argument("--fens", required=True)
    two.add_argument("--lo", type=int, default=LO)
    two.add_argument("--hi", type=int, default=HI)
    two.add_argument("--lib", default="build-tune/src/libchesso_engine.a")
    two.add_argument("--out", required=True)
    two.add_argument("--fens-out", required=True)
    two.set_defaults(run=S097.cmd_fires)

    three = sub.add_parser("sweep")
    three.add_argument("--fens", required=True)
    three.add_argument("--lo", type=int, default=LO)
    three.add_argument("--hi", type=int, default=HI)
    three.add_argument("--lib", default="build/src/libchesso_engine.a")
    three.add_argument("--tune", action="store_true")
    # S097's driver reads this field as the switch value it sets or checks.
    three.add_argument("--probcut", dest="multicut", type=int, default=1)
    three.add_argument("--out", required=True)
    three.set_defaults(run=S097.cmd_sweep)

    sep = sub.add_parser("separators")
    sep.add_argument("--shipped", required=True)
    sep.add_argument("--open", required=True)
    sep.add_argument("--lo", type=int, default=LO)
    sep.add_argument("--out", required=True)
    sep.set_defaults(run=S097.cmd_separators)

    four = sub.add_parser("pick")
    four.add_argument("--shipped", required=True)
    four.add_argument("--open", required=True)
    four.add_argument("--fires", default="")
    four.add_argument("--lo", type=int, default=LO)
    four.set_defaults(run=S097.cmd_pick)

    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
