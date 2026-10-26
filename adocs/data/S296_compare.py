#!/usr/bin/env python3
"""S296: the mate readings of the shipped tree and DEC-318's two forms side
by side, every reported mate distance checked against the oracle's
(adocs/data/S296_oracle.tsv, Stockfish at 5000000 nodes, S296_oracle.py).

    P=~/.venv/chess/bin/python
    $P adocs/data/S296_compare.py --oracle adocs/data/S296_oracle.tsv \\
        --sets DIR --rows DIR --grids DIR --forms ship a b

DIR/--sets holds S289's four set files (roots82, def104, def186, mined318:
fen<TAB>signed distance<TAB>depth, adocs/data/S291_reach.txt's rules);
--rows holds adocs/data/S289_rows.py's output per set and form,
<set>_<form>.txt; --grids holds adocs/data/S295_long_line.py's `sweep`
(S294's mined grid) and `s170` (the S170 grid) per form, mined_<form>.tsv and
s170_<form>.tsv. The first form is the reference the others are read against.

Per set: `exact` is S289_rows.py's own (the last iteration reports exactly the
set's distance), gained and lost against the reference by row. `short` counts
the rows whose last iteration reports a mate of the oracle's sign and a
smaller distance than the oracle's, and `short_it` every completed iteration
that does (S289_rows.py records one score per depth). Per grid, a search is
one `go nodes` cell (and, on the S170 grid, one ply of its game): `exact` its
last `info` line is a mate at the oracle's distance; `short` its last line's
distance is smaller than the oracle's, `short_lines` every mate line that is.
`sign` counts a mate of the other sign than the oracle's, `nomate` a mate where
the oracle reports none. Engine output against the oracle's; no
chess judgement here.
"""

import argparse
import os
import sys

import chess


HERE = os.path.dirname(os.path.abspath(__file__))
CASES = os.path.join(HERE, "S170_cases.tsv")
SETS = ["roots82", "def104", "def186", "mined318"]


def read_oracle(path):
    out = {}
    for line in open(path):
        if line.startswith("#") or line.startswith("fen\t"):
            continue
        f = line.rstrip("\n").split("\t")
        out[f[0]] = (f[1], int(f[2]))
    return out


def classify(oracle, fen, mate):
    """short, sign, nomate, longer or ok for one reported `mate` at fen."""
    kind, value = oracle[fen]
    if kind != "mate":
        return "nomate"
    if (mate > 0) != (value > 0):
        return "sign"
    if abs(mate) < abs(value):
        return "short"
    if abs(mate) > abs(value):
        return "longer"
    return "ok"


def read_set(path):
    rows = []
    for line in open(path):
        if line.strip() and not line.startswith("#"):
            f = line.rstrip("\n").split("\t")
            rows.append((f[0], int(f[1])))
    return rows


def read_rows(path):
    out = {}
    for line in open(path):
        f = line.rstrip("\n").split("\t")
        its = []
        for tok in f[6].split():
            d, s = tok.split(":")
            its.append((int(d), s))
        out[int(f[0])] = dict(exp=int(f[1]), kind=f[2], val=int(f[3]),
                              best=f[5], its=its)
    return out


def mate_of(score):
    return int(score[1:]) if score.startswith("m") else None


def sets_section(args, oracle):
    print("== Sets: S289's four, adocs/data/S289_rows.py, Hash 16, a fresh "
          "table per row")
    ref = args.forms[0]
    for name in SETS:
        rows = read_set(os.path.join(args.sets, name + ".set"))
        label_vs_oracle = sum(
            1 for fen, exp in rows
            if oracle[fen][0] != "mate" or oracle[fen][1] != exp)
        got = {f: read_rows(os.path.join(args.rows, "%s_%s.txt" % (name, f)))
               for f in args.forms}
        print("-- %s, %d rows (label and oracle disagree on %d)" %
              (name, len(rows), label_vs_oracle))
        for f in args.forms:
            r = got[f]
            exact = {i for i, x in r.items()
                     if x["kind"] == "mate" and x["val"] == x["exp"]}
            ref_exact = {i for i, x in got[ref].items()
                         if x["kind"] == "mate" and x["val"] == x["exp"]}
            short = short_it = sign = nomate = 0
            for i, x in r.items():
                fen = rows[i][0]
                if x["kind"] == "mate":
                    c = classify(oracle, fen, x["val"])
                    short += c == "short"
                    sign += c == "sign"
                    nomate += c == "nomate"
                for d, s in x["its"]:
                    m = mate_of(s)
                    if m is not None and classify(oracle, fen, m) == "short":
                        short_it += 1
            best_moved = sum(1 for i in r if r[i]["best"] != got[ref][i]["best"])
            score_moved = sum(1 for i in r
                              if (r[i]["kind"], r[i]["val"]) !=
                              (got[ref][i]["kind"], got[ref][i]["val"]))
            def oracle_exact(res):
                return {i for i, x in res.items() if x["kind"] == "mate" and
                        classify(oracle, rows[i][0], x["val"]) == "ok"}

            oex, ref_oex = oracle_exact(r), oracle_exact(got[ref])
            print("  %-5s exact %3d  gained %2d  lost %2d  | exact by the "
                  "oracle %3d  gained %2d  lost %2d  | short %d  short_it %d  "
                  "sign %d  nomate %d  scores moved %d  best moves moved %d" %
                  (f, len(exact), len(exact - ref_exact),
                   len(ref_exact - exact), len(oex), len(oex - ref_oex),
                   len(ref_oex - oex), short, short_it, sign, nomate,
                   score_moved, best_moved))
            if f != ref:
                for i in sorted((exact ^ ref_exact) | (oex ^ ref_oex)):
                    print("        index %d (label %+d, oracle %s %d): %s %s "
                          "-> %s %s%s%s" % (
                              i, rows[i][1], oracle[rows[i][0]][0],
                              oracle[rows[i][0]][1], got[ref][i]["kind"],
                              got[ref][i]["val"], r[i]["kind"], r[i]["val"],
                              "  by label " + ("gained" if i in exact else
                                               "lost")
                              if i in exact ^ ref_exact else "",
                              "  by oracle " + ("gained" if i in oex else
                                                "lost")
                              if i in oex ^ ref_oex else ""))


def read_cases():
    cases = {}
    for line in open(CASES):
        line = line.rstrip("\n")
        if not line or line.startswith("#"):
            continue
        f = line.split("\t")
        if f[0] != "name":
            cases[f[0]] = (f[1], f[2].split())
    return cases


def grid_searches(path, cases, fen_cache):
    """(key, nodes, ply) -> list of (fen, mate, last) per mate line."""
    out = {}
    head = None
    for line in open(path):
        if line.startswith("#"):
            continue
        f = line.rstrip("\n").split("\t")
        if head is None:
            head = f
            continue
        row = dict(zip(head, f))
        cell = (row["key"], int(row["nodes"]), int(row["ply"]))
        if row["set"] == "mined":
            fen = row["key"]
        else:
            k = (row["key"].split("/")[0], int(row["ply"]))
            if k not in fen_cache:
                base, moves = cases[k[0]]
                board = chess.Board(base)
                for uci in moves[:k[1]]:
                    board.push_uci(uci)
                fen_cache[k] = board.fen()
            fen = fen_cache[k]
        out.setdefault(cell, []).append(
            (fen, int(row["mate"]), row["last"] == "1"))
    return out


def grids_section(args, oracle):
    cases = read_cases()
    cache = {}
    ref = args.forms[0]
    for grid, title in (("mined", "S294's mined grid, adocs/data/"
                         "S295_long_line.py sweep (318 positions x 39 "
                         "budgets)"),
                        ("s170", "the S170 grid, S295_long_line.py s170 "
                         "(6 cases x 2 strides x 9 budgets)")):
        print("== %s" % title)
        got = {f: grid_searches(os.path.join(args.grids, "%s_%s.tsv" %
                                             (grid, f)), cases, cache)
               for f in args.forms}

        def exact_set(g):
            return {c for c, lines in g.items()
                    if any(last and classify(oracle, fen, m) == "ok"
                           for fen, m, last in lines)}

        ref_exact = exact_set(got[ref])
        for f in args.forms:
            g = got[f]
            ex = exact_set(g)
            lines = sum(len(v) for v in g.values())
            short_lines = sum(1 for v in g.values() for fen, m, last in v
                              if classify(oracle, fen, m) == "short")
            short = sum(1 for v in g.values() if any(
                last and classify(oracle, fen, m) == "short"
                for fen, m, last in v))
            sign = sum(1 for v in g.values() for fen, m, last in v
                       if classify(oracle, fen, m) == "sign")
            nomate = sum(1 for v in g.values() for fen, m, last in v
                         if classify(oracle, fen, m) == "nomate")
            print("  %-5s mate lines %5d  searches ending on a mate %5d  "
                  "exact %5d  gained %3d  lost %3d  short %3d  short_lines "
                  "%3d  sign %d  nomate %d" %
                  (f, lines, sum(1 for v in g.values() if any(
                      last for _, _, last in v)), len(ex), len(ex - ref_exact),
                   len(ref_exact - ex), short, short_lines, sign, nomate))
            if args.verbose and f != ref:
                for c in sorted(ref_exact - ex):
                    print("        lost", c)
                for c in sorted(ex - ref_exact):
                    print("        gained", c)
            if args.verbose:
                for c in sorted(g):
                    for fen, m, last in g[c]:
                        if classify(oracle, fen, m) == "short":
                            print("        %s short: %s mate %d (oracle %s %d)"
                                  " last %d" % (f, c, m, oracle[fen][0],
                                                oracle[fen][1], last))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--oracle", required=True)
    ap.add_argument("--sets", required=True)
    ap.add_argument("--rows", required=True)
    ap.add_argument("--grids", required=True)
    ap.add_argument("--forms", nargs="+", required=True)
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()
    oracle = read_oracle(args.oracle)
    sets_section(args, oracle)
    grids_section(args, oracle)
    return 0


if __name__ == "__main__":
    sys.exit(main())
