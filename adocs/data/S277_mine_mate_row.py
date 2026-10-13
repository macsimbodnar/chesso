#!/usr/bin/env python3
"""S277. `adocs/data/S113_mine_mate_row.py`'s successor: the ProbCut mate row
of `tests/test_search.cpp` "pruning does not hide a forced mate", mined for
`B04_probcut_defender_gate_dropped` of tools/mutants/S113_probcut.py, with the
`--mode` DEC-238 gave S097's miner passed through to S097's `fires` and `pick`.

WHY. B04 drops a *guard*, and DEC-238 asks a guard row's firing witness of the
mutant's build: at the row's own depth the shipped ProbCut can leave the tree
as it is because the guard stops it. S113's script has no `--mode`, so S259's
re-mine took the guard-mode witness through a one-off,
`adocs/data/S259_b04_fires_guard.py`; on `338705e` the default witness moved
S259's row at 9, 10 and 12 and not at 11, the row's depth, and the guard-mode
one at 9 to 12. This file is that mode as a stage of the miner, so the next
re-mine reads its witness from this header.

WHAT IT IS. S113's miner and nothing else, imported rather than copied
(`adocs/data/` is append-only; S113 is not edited): importing it re-points
S097's driver from `SeMultiCut` to `ProbCut`, derives the swept range from
`ProbCutMinDepth`, and supplies the `fens` stage. The five stages and their
arguments are S113's; `fires` and `pick` gain `--mode`, with S097's two values
and S097's default:

  * `rule`, the default -- the mode every S113 derivation ran in, the
    shipped tune library's witness (`--lib` defaults to
    `build-tune/src/libchesso_engine.a`). What it prints and writes is what
    `S113_mine_mate_row.py` prints and writes.
  * `guard` (DEC-238) -- `fires --mode guard --lib <B04's tune library>`
    takes the witness on the mutant's build; `--lib` is then required and may
    not be the shipped tune library, and the witness file's first line names
    the mode. `pick --mode guard` reads only a guard-mode witness, and the
    default mode only a rule-mode one, so no derivation mixes the two.

The checks above are S097's own (`cmd_fires`, `cmd_pick`), and this file
hands them the mode and closes two gaps in guard mode. `fires` refuses the
shipped tune library by the file `--lib` resolves to, where S097 compares the
spelling with a relative path and an absolute one passes. `pick` refuses a
missing witness or one with no data rows, which S097's `pick` reads as no
firing condition at all. The default mode reaches neither. Their output still says "SeMultiCut" and "multicut" where it prints the
switch: read `ProbCut`, as S113's header says.

THE PICK RULE is S097's, unchanged (DEC-209 clause 4, DEC-238): the rule fires
on the position (in guard mode, on B04's build), the shipped build reports a
mate at some depth of the range and the mutant does not at that same depth;
the lowest such depth, the longest consecutive shipped profile first, then
the cheaper cell. A guard row derived with this file names its mode.

    P=~/.venv/chess/bin/python
    C=.tuning/coord
    # 1. the candidate list: the FEN column of adocs/data/S097_candidates.tsv
    $P adocs/data/S277_mine_mate_row.py fens --out $C/S277_candidates.fen
    # 2. the shipped sweep
    $P adocs/data/S277_mine_mate_row.py sweep --fens $C/S277_candidates.fen \\
        --lib build/src/libchesso_engine.a --out $C/S277_shipped.txt
    # 3. the same sweep on a library built with B04 applied. Build it out of
    #    tree: apply B04 to src/search.cpp, configure and build `build-b04`,
    #    and `build-b04-tune` with -DCHESSO_TUNE=ON for stage 5, then
    #    `git checkout -- src/search.cpp`. (S259 and S277 built both in a
    #    separate copy of the same commit instead, B04 applied by
    #    tools/mutation_check.py's own loader.)
    $P adocs/data/S277_mine_mate_row.py sweep --fens $C/S277_candidates.fen \\
        --lib build-b04/src/libchesso_engine.a --out $C/S277_open.txt
    # 4. the separator set
    $P adocs/data/S277_mine_mate_row.py separators \\
        --shipped $C/S277_shipped.txt --open $C/S277_open.txt \\
        --out $C/S277_separators.fen
    # 5, guard mode: the witness on B04's tune library
    $P adocs/data/S277_mine_mate_row.py fires --mode guard \\
        --fens $C/S277_separators.fen \\
        --lib build-b04-tune/src/libchesso_engine.a \\
        --out $C/S277_fires_guard.txt --fens-out $C/S277_fires_guard.fen
    # 6, guard mode
    $P adocs/data/S277_mine_mate_row.py pick --mode guard \\
        --shipped $C/S277_shipped.txt --open $C/S277_open.txt \\
        --fires $C/S277_fires_guard.txt

Stages 5 and 6 in the default mode are S113's own: `fires` with no `--mode`
and `--lib build-tune/src/libchesso_engine.a`, and `pick` with no `--mode`
over that witness.
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S113_mine_mate_row as S113  # noqa: E402

S097 = S113.S097
LO = S113.LO
HI = S113.HI


def cmd_fires(args):
    """S097's `fires`, refusing in guard mode the shipped tune library by the
    file `--lib` resolves to. S097 compares the spelling with a relative path,
    so an absolute or symlinked path to the same file passes there; the driver
    links `os.path.join(REPO, lib)`, and that is what is resolved here."""
    if args.mode == "guard" and args.lib is not None:
        shipped = os.path.realpath(os.path.join(S097.REPO,
                                                S097.SHIPPED_TUNE_LIB))
        given = os.path.realpath(os.path.join(S097.REPO, args.lib))

        if given == shipped:
            sys.stderr.write(f"guard mode takes the witness on the mutant's "
                             f"build, and {args.lib} is the shipped tune "
                             f"library ({shipped})\n")
            return 2

    return S097.cmd_fires(args)


def cmd_pick(args):
    """S097's `pick`, refused in guard mode without a witness behind it.
    S113's `pick` takes `--fires` as optional, and S097's reads a missing
    witness, or one with no data rows, as no firing condition at all; in guard
    mode that would be a guard-mode pick no witness stands behind, the one mix
    the mode exists to keep out. A data row is what S097's reader takes: a
    line not starting with `#` and holding a tab."""
    if args.mode == "guard":
        if not args.fires:
            sys.stderr.write("pick --mode guard reads a guard-mode witness: "
                             "name it with --fires\n")
            return 2

        with open(args.fires) as handle:
            rows = sum(1 for line in handle
                       if not line.startswith("#") and "\t" in line)

        if rows == 0:
            sys.stderr.write(f"pick --mode guard was handed a witness with no "
                             f"data rows: {args.fires}\n")
            return 2

    return S097.cmd_pick(args)


def main():
    parser = argparse.ArgumentParser(
        description=f"the swept range defaults to {LO}..{HI}, derived from "
                    f"ProbCutMinDepth")
    sub = parser.add_subparsers(dest="stage", required=True)

    zero = sub.add_parser("fens")
    zero.add_argument("--out", required=True)
    zero.set_defaults(run=S113.cmd_fens)

    two = sub.add_parser("fires")
    two.add_argument("--fens", required=True)
    two.add_argument("--lo", type=int, default=LO)
    two.add_argument("--hi", type=int, default=HI)
    # None and not S113's literal default: S097's `cmd_fires` fills in the
    # shipped tune library in the rule mode and refuses a missing one in
    # guard mode, which a filled-in default would hide.
    two.add_argument("--lib", default=None,
                     help=f"the tune library the witness drives; default "
                          f"{S097.SHIPPED_TUNE_LIB} in rule mode, required "
                          f"in guard mode (B04's)")
    two.add_argument("--mode", choices=S097.MODES, default="rule",
                     help="rule: the shipped build's witness, every S113 "
                          "derivation; guard: B04's (DEC-238)")
    two.add_argument("--out", required=True)
    two.add_argument("--fens-out", required=True)
    two.set_defaults(run=cmd_fires)

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
    four.add_argument("--mode", choices=S097.MODES, default="rule",
                      help="must match the mode the --fires file was "
                           "written in (DEC-238)")
    four.add_argument("--lo", type=int, default=LO)
    four.set_defaults(run=cmd_pick)

    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
