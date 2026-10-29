#!/usr/bin/env python3
"""S121's SPRT: the two readings S296_sprt_mate_pvs.py does not make.

`adocs/data/S121_sprt_mate_pvs.txt` is the reading, with every command that
made it. The warnings themselves, the oracle, the cold and warm replays and
the per-10000 rates are `adocs/data/S296_sprt_mate_pvs.py`'s, run unchanged.

    P=~/.venv/chess/bin/python
    $P -I adocs/data/S121_sprt_mate_pvs.py games LOG...
    $P -I adocs/data/S121_sprt_mate_pvs.py quarters PGN [--cand cand-]

`games`: per log, the games that printed an `Incomplete mating PV` warning,
per side -- a game named by its warning's start FEN and the first 16 moves
of its move list, which every warning of one game shares and no two games of
a run do (each game opens from a different book line) -- and the side-games
the log's foot counts (`=== N games in ...`, or the last `Games:` line).
The three runs' warnings come in runs of consecutive searches of one game,
so a game is the independent unit, not a line or a search.

`quarters`: the inspection's quarters, S296's and S136's method, which no
earlier step kept as a script. The PGN's games in fastchess's game-number
order (2 * Round - 1 with the candidate White, 2 * Round with it Black), cut
at i * N // 4; per quarter the candidate's mean score s, its 95 % per-game
interval s +/- 1.96 sd / sqrt(n), and both as Elo by -400 log10(1/s - 1);
the spread of the four about the whole run's mean score as chi-square on 3
degrees of freedom, each quarter's (s - S) over its sd / sqrt(n). Checked:
it reproduces S135's +5.6, -2.5, +13.8, +6.6 (chi-square 6.56) and S296's
-0.7, -3.3, +4.4, +3.3 (2.73) from their PGNs.

Nothing here is a chess judgement: it counts games and scores.
"""

import argparse
import math
import re
import sys

import chess.pgn

WARNING = "Warning; Incomplete mating PV - from "
PREFIX_MOVES = 16


def elo(s):
    return -400.0 * math.log10(1.0 / s - 1.0)


def games(args):
    for path in args.logs:
        text = open(path, encoding="utf-8", errors="replace",
                    newline="").read().split("\n")
        found = {"cand": set(), "ref": set()}
        n = None
        for k, line in enumerate(text):
            if line.startswith(WARNING):
                side = line[len(WARNING):].strip().split("-")[0]
                fen = text[k + 2]
                moves = text[k + 3].split()[1:1 + PREFIX_MOVES]
                found[side].add((fen, tuple(moves)))
            m = re.match(r"=== (\d+) games in ", line)
            if m:
                n = int(m.group(1))
        if n is None:
            g = [l for l in text if l.startswith("Games: ")]
            n = int(g[-1].split()[1].rstrip(",")) if g else 0
        print("%s: %d games; cand %d games with a warning, ref %d" % (
            path, n, len(found["cand"]), len(found["ref"])))
    return 0


def quarters(args):
    rows = []
    with open(args.pgn, encoding="utf-8", errors="replace") as f:
        while True:
            h = chess.pgn.read_headers(f)
            if h is None:
                break
            res = h.get("Result")
            if res not in ("1-0", "0-1", "1/2-1/2"):
                continue
            white_cand = h["White"].startswith(args.cand)
            r = int(h["Round"])
            num = 2 * r - 1 if white_cand else 2 * r
            white = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": 0.5}[res]
            rows.append((num, white if white_cand else 1.0 - white))
    rows.sort()
    n = len(rows)
    scores = [s for _, s in rows]
    mean = sum(scores) / n
    chi = 0.0
    for i in range(4):
        q = scores[i * n // 4:(i + 1) * n // 4]
        m = len(q)
        s = sum(q) / m
        sd = math.sqrt(sum((x - s) ** 2 for x in q) / m)
        lo, hi = s - 1.96 * sd / math.sqrt(m), s + 1.96 * sd / math.sqrt(m)
        e, half = elo(s), (elo(hi) - elo(lo)) / 2.0
        chi += ((s - mean) / (sd / math.sqrt(m))) ** 2
        print("quarter %d: games %d to %d of %d (%d), Elo %+.1f +/- %.1f" % (
            i + 1, i * n // 4 + 1, (i + 1) * n // 4, n, m, e, half))
    # chi-square on 3 degrees of freedom: P(X > x) in closed form
    p = math.erfc(math.sqrt(chi / 2)) + math.sqrt(2 * chi / math.pi) * math.exp(-chi / 2)
    print("run Elo %+.2f over %d games; chi-square %.2f on 3 df, p %.2f" % (
        elo(mean), n, chi, p))
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("games")
    p.add_argument("logs", nargs="+")
    p = sub.add_parser("quarters")
    p.add_argument("pgn")
    p.add_argument("--cand", default="cand-")
    args = ap.parse_args()
    return {"games": games, "quarters": quarters}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
