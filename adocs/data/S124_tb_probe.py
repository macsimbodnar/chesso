#!/usr/bin/env python3
"""S124's research pass, 2026-11-03: the tablebase's word on the corpus's rows of five men
or fewer, under the fifty-move rule at each row's own halfmove clock, beside the case each
falls in and the result its game reached.

    ~/.venv/chess/bin/python adocs/data/S124_tb_probe.py [--k K] DUMP [DUMP ...] > out.txt

DUMP is adocs/data/S124_census.cpp's --dump: fen, result or '-', game, ply, case, strong
(0 White, 1 Black, -1 none, the side E1 favours), E1, signature, phase, the smooth score at
f = 1, the label cell, the proposal's cell. Every verdict is the Syzygy 3-4-5 tables'
through python-chess, never the agent's reading (CHESS rule).

**The laws, not the zeroed clock.** probe_wdl assumes the fifty-move counter is zero, and the
games ran under the rule, so a row's verdict here is the tables' at the row's own clock c:
probe_wdl, and where it reads 2 or -2, probe_dtz's n = |dtz|. python-chess documents that a
returned n can mean n + 1 plies (DTZ50'' rounding), so:

    WON / LOST   |wdl| = 2 and c + n + 1 <= 100: the zeroing move is forced in time either way
    SPOILED      |wdl| = 2 and c + n > 100: not in time either way, a draw under the rule
    EDGE         |wdl| = 2 and c + n = 100: the rounding decides; counted, read in no label
    DRAWN        wdl 0, or 1 and -1, which the tables already score drawn under the rule

turned to the strong side's view by sign only. With --k, each label cell's flat f is read
twice over the same rows, reading A of the census (s + (f - 1) E1 w_eg), once against the
game's result and once against the tables' verdict under the laws (WON 1, DRAWN and SPOILED
0.5, LOST 0, White relative), EDGE rows left out of both: in sample, a reading and never a
fit, on training rows only. A row with castling rights, or no table, is counted unprobed.
The result column is a training game's, written by the census only for games outside the
held-out list.
"""
import collections
import math
import sys

import chess
import chess.syzygy

TB = "/home/max/syzygy"

CASES = ["DEAD", "LONE", "OCB_PURE", "OCB_ROOK", "OCB_OTHER", "PAWNS", "NONE"]
DNAME = ["d<=-1", "d=0", "d=1", "d=2", "d>=3"]
GRID = [i / 32.0 for i in range(41)]
PROP_BASE = 1000
PROP = ["P=0 d<=0", "P=0 d=1", "P=0 d>=2", "P=1 d<=0", "P=1 d=1", "P=1 d>=2", "P=2 d<=0", "P=2 d=1",
        "P=2 d>=2", "OCB_PURE P<=2", "OCB_PURE P=3-4", "OCB_PURE P>=5", "OCB_ROOK", "MOPUP",
        "MOPUP of it KBN", "LONE other"]
VERDICTS = ["WON", "EDGE", "SPOILED", "DRAWN", "LOST", "none"]


def cell_name(cell):
    if cell >= PROP_BASE:
        return "prop " + PROP[cell - PROP_BASE]
    if cell < 7:
        return CASES[cell]
    if cell < 7 + 45:
        g = cell - 7
        return "PAWNS P=%d %s" % (g // 5, DNAME[g % 5])
    return "OCB_PURE P=%d" % (cell - 52)


def sigmoid(score, k):
    return 1.0 / (1.0 + math.exp(-k * score * math.log(10) / 400.0))


def law_verdict(white_wdl, dtz_n, clock):
    """White's verdict under the fifty-move rule at `clock`: 2, -2, 0, 'EDGE' or 'SPOILED'."""
    if abs(white_wdl) != 2:
        return 0
    if clock + dtz_n + 1 <= 100:
        return white_wdl
    if clock + dtz_n > 100:
        return "SPOILED"
    return "EDGE"


def view_name(verdict, strong):
    if strong < 0:
        return "none"
    if verdict in ("EDGE", "SPOILED"):
        return verdict
    v = verdict if strong == 0 else -verdict
    return {2: "WON", -2: "LOST", 0: "DRAWN"}[v]


def main(paths, k):
    cache = {}
    reading = collections.defaultdict(lambda: {"n": 0, "games": set(), "game": [0.0] * 41, "tb": [0.0] * 41,
                                               "lab_g": 0.0, "lab_t": 0.0, "edge": 0,
                                               "verdicts": collections.Counter()})
    by_case = collections.defaultdict(collections.Counter)
    by_sig = collections.defaultdict(collections.Counter)
    sig_games = collections.defaultdict(set)
    fail_games = collections.defaultdict(set)
    rows = 0
    unprobed = 0
    with chess.syzygy.open_tablebase(TB) as tb:
        for path in paths:
            with open(path) as f:
                for line in f:
                    fen, result, game, _ply, case, strong, e1, sig, phase, smooth, cell, prop = line.rstrip("\n").split("\t")
                    rows += 1
                    strong = int(strong)
                    fields = fen.split()
                    key = " ".join(fields[:4])
                    clock = int(fields[4])
                    if key not in cache:
                        try:
                            board = chess.Board(fen)
                            wdl = tb.probe_wdl(board)
                            dtz = tb.probe_dtz(board) if abs(wdl) == 2 else 0
                            white = wdl if board.turn == chess.WHITE else -wdl
                            cache[key] = (white, abs(dtz))
                        except (KeyError, ValueError, chess.syzygy.MissingTableError):
                            cache[key] = None
                    probe = cache[key]
                    if probe is None:
                        unprobed += 1
                        by_case[case]["unprobed"] += 1
                        continue
                    white, n = probe
                    verdict = law_verdict(white, n, clock)
                    raw_view = white if strong != 1 else -white
                    tag = view_name(verdict, strong)
                    by_case[case][tag] += 1
                    by_case[case]["rows"] += 1
                    if strong >= 0 and raw_view == 2:
                        by_case[case]["raw 2"] += 1
                    c = by_sig[(case, sig)]
                    c["rows"] += 1
                    c[tag] += 1
                    if strong >= 0 and raw_view == 2:
                        c["raw 2"] += 1
                    c["e1"] += abs(float(e1))
                    sig_games[(case, sig)].add(game)
                    if k is not None and result != "-":
                        cells = [int(cell)]
                        if int(prop) >= 0:
                            cells.append(PROP_BASE + int(prop))
                        if sig == "KBNvK":
                            cells.append(PROP_BASE + 14)
                        for one in cells:
                            reading[one]["verdicts"][tag] += 1
                        if verdict == "EDGE":
                            for one in cells:
                                reading[one]["edge"] += 1
                        else:
                            tb_label = 0.5 if verdict == "SPOILED" else {2: 1.0, -2: 0.0, 0: 0.5}[verdict]
                            egw = (24 - int(phase)) / 24.0
                            s1 = float(smooth)
                            r = float(result)
                            for one in cells:
                                rd = reading[one]
                                rd["n"] += 1
                                rd["games"].add(game)
                                rd["lab_g"] += r if strong != 1 else 1 - r
                                rd["lab_t"] += tb_label if strong != 1 else 1 - tb_label
                                for i, fv in enumerate(GRID):
                                    p = sigmoid(s1 + (fv - 1.0) * float(e1) * egw, k)
                                    rd["game"][i] += (r - p) ** 2
                                    rd["tb"][i] += (tb_label - p) ** 2
                    if result != "-" and strong >= 0:
                        r = float(result)
                        sr = r if strong == 0 else 1.0 - r
                        c["labelled"] += 1
                        c["result " + str(sr)] += 1
                        if tag == "WON" and sr == 0.5:
                            c["won, game drawn"] += 1
                            fail_games[(case, sig)].add(game)
                        if tag in ("DRAWN", "SPOILED") and sr != 0.5:
                            c["drawn, game decided"] += 1

    print("S124 tablebase probe under the fifty-move rule: %d rows, %d distinct positions, %d unprobed" % (
        rows, len(cache), unprobed))
    print("strong side's view at the row's clock: WON EDGE SPOILED DRAWN LOST, none = no strong side;"
          " raw 2 = probe_wdl's 2 at a zeroed clock")
    print("\nby case: rows probed, the verdicts, raw 2")
    for case in sorted(by_case):
        c = by_case[case]
        print("  %-10s %8d  %s  raw2 %7d  unprobed %d" % (
            case, c["rows"], " ".join("%s %7d" % (v, c[v]) for v in VERDICTS), c["raw 2"], c["unprobed"]))
    print("\nby case and signature, 60 largest: rows, games, WON EDGE SPOILED DRAWN LOST none, raw 2, mean |E1|;"
          " labelled rows, their strong-side results 1 / 0.5 / 0;"
          " WON with the game drawn (rows, games); DRAWN or SPOILED with the game decided")
    items = sorted(by_sig.items(), key=lambda kv: -kv[1]["rows"])
    for (case, sig), c in items[:60]:
        print("  %-9s %-22s %7d %6d  %s  raw2 %6d  |E1| %6.0f  lab %6d  %6d %6d %6d  won-drawn %5d %4d  drawn-decided %5d" % (
            case, sig, c["rows"], len(sig_games[(case, sig)]),
            " ".join("%6d" % c[v] for v in VERDICTS), c["raw 2"], c["e1"] / max(c["rows"], 1),
            c["labelled"], c["result 1.0"], c["result 0.5"], c["result 0.0"],
            c["won, game drawn"], len(fail_games[(case, sig)]), c["drawn, game decided"]))
    if k is not None:
        print("\nlabel cells over these rows at K %.4f, reading A, in sample, EDGE rows left out: rows, games, EDGE rows,"
              " the strong side's mean game label and law label, then the best flat f and its dMSE against f = 1"
              " (1e-6 of the cell's rows) under each label" % k)
        for cell in sorted(reading):
            rd = reading[cell]
            n = rd["n"]
            if n == 0:
                continue
            out = []
            for which in ("game", "tb"):
                e = rd[which]
                best = min(range(41), key=lambda i: e[i])
                out.append("%5.3f %10.1f" % (GRID[best], 1e6 * (e[best] - e[32]) / n))
            print("  %-20s %7d %6d  edge %4d  game %.3f law %.3f   f_game %s   f_law %s   %s" % (
                cell_name(cell), n, len(rd["games"]), rd["edge"], rd["lab_g"] / n, rd["lab_t"] / n, out[0], out[1],
                " ".join("%s %d" % (v, rd["verdicts"][v]) for v in VERDICTS[:5])))
    print("S124-TB-OK")


if __name__ == "__main__":
    args = sys.argv[1:]
    k = None
    if args[:1] == ["--k"]:
        k = float(args[1])
        args = args[2:]
    if not args:
        sys.exit("S124_tb_probe.py [--k K] DUMP [DUMP ...]")
    main(args, k)
