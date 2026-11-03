#!/usr/bin/env python3
"""S329: which commits to src/eval_tables.hpp moved a fitted constant.

Reads `src/eval_tables.hpp` at each commit by `git show` and compares, with the
commit before it in the list, the five piece-value defines, `piece_value`,
`phase_value`, `psqt_mg` and `psqt_eg`. With no arguments the list is `33aa3b4`
(S065's paste, where DEV_MANUAL.md's "The second fit's constants are the ones in
`eval_tables.hpp`" was written), every commit `git log 33aa3b4.. --
src/eval_tables.hpp` names, oldest first, and `1bb3080`, S329's base. Run from
a chesso checkout:

    python3 adocs/data/S329_eval_tables_consts.py [commit ...]

Its output on 2026-11-03 is adocs/data/S329_eval_tables_consts.txt.
"""
import re
import subprocess
import sys


def git(*args):
    return subprocess.run(("git",) + args, capture_output=True, text=True,
                          check=True).stdout


def constants(sha):
    src = git("show", f"{sha}:src/eval_tables.hpp")
    out = {"defines": re.findall(
        r"^#define (PAWN|KNIGHT|BISHOP|ROOK|QUEEN)\s+(-?\d+)", src, re.M)}
    for name in ("piece_value", "phase_value", "psqt_mg", "psqt_eg"):
        m = re.search(r"static\s+(?:constexpr|const)\s+int\s+" + name +
                      r"\b[^=]*=\s*\{(.*?)\};", src, re.S)
        if m is None:
            sys.exit(f"{sha}: no `{name}` array in src/eval_tables.hpp")
        body = re.sub(r"//[^\n]*", "", m.group(1))
        out[name] = re.findall(r"-?\b\d+\b|\b[A-Z]+\b", body)
    return out


def main():
    shas = sys.argv[1:]
    if not shas:
        log = git("log", "--reverse", "--format=%h", "33aa3b4..",
                  "--", "src/eval_tables.hpp").split()
        shas = ["33aa3b4"] + log + ["1bb3080"]
    prev = None
    for sha in shas:
        c = constants(sha)
        head = git("log", "-1", "--format=%h %ad %s", "--date=short", sha)
        print(head.strip())
        print("    defines: " +
              ", ".join(f"{name} {value}" for name, value in c["defines"]))
        if prev is None:
            print("    the reference: nothing to compare")
        else:
            moved = [k for k in c if c[k] != prev[k]]
            print("    moved: " + (", ".join(moved) if moved else "no constant"))
            for k in ("psqt_mg", "psqt_eg"):
                n = sum(1 for a, b in zip(c[k], prev[k]) if a != b)
                print(f"    {k}: {n} of {len(c[k])} entries differ")
        prev = c
    print("S329-EVAL-TABLES-DONE")


if __name__ == "__main__":
    main()
