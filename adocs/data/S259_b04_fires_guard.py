#!/usr/bin/env python3
"""S259. The ProbCut mate row's firing witness in guard mode (DEC-238), taken
for the record beside the default-mode witness `S113_mine_mate_row.py fires`
takes. Not a stage of that script and not the pick's input: S113's pick ran in
its header's default mode, and this file exists so the guard-mode reading the
row's GOLDEN block quotes can be re-run.

WHY. B04 drops a guard, and DEC-238 asks a guard row's witness of the
mutant's build: at the row's own depth the shipped rule can be still because
the guard stops it. On `338705e` the default witness moves the tree on
`4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8 w - - 3 73` at depths 9, 10 and 12 and not at
11, the depth the row is read at; this one, on a tune library built with B04
applied, at 9 to 12.

HOW. S113's script has no `--mode`, and neither script is edited here
(`adocs/data/` is append-only). Importing `S113_mine_mate_row` re-points
S097's driver from `SeMultiCut` to `ProbCut` and derives the depth range from
`ProbCutMinDepth`; S097's own `cmd_fires` is then called with `mode="guard"`,
exactly as `S097_mine_mate_row.py fires --mode guard` would call it. Its output
lines say "SeMultiCut" and "the mutant's multicut" where they mean `ProbCut`,
as S113's header says of every line its stages print.

    P=~/.venv/chess/bin/python
    C=.tuning/coord/S259/remine
    $P adocs/data/S259_b04_fires_guard.py --fens $C/b04_separators.fen \\
        --lib $C/libs/b04_tune.a \\
        --out $C/b04_fires_guard.txt --fens-out $C/b04_fires_guard.fen

`--lib` is a tune library (`-DCHESSO_TUNE=ON`) built with
`B04_probcut_defender_gate_dropped` of tools/mutants/S113_probcut.py applied.
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S113_mine_mate_row as S113  # noqa: E402

S097 = S113.S097


def main():
    parser = argparse.ArgumentParser(
        description=f"S113's fires in guard mode (DEC-238), depths "
                    f"{S113.LO}..{S113.HI} derived from ProbCutMinDepth")
    parser.add_argument("--fens", required=True)
    parser.add_argument("--lib", required=True,
                        help="the tune library built with B04 applied")
    parser.add_argument("--out", required=True)
    parser.add_argument("--fens-out", required=True)
    parser.add_argument("--lo", type=int, default=S113.LO)
    parser.add_argument("--hi", type=int, default=S113.HI)
    args = parser.parse_args()
    args.mode = "guard"
    return S097.cmd_fires(args)


if __name__ == "__main__":
    sys.exit(main())
