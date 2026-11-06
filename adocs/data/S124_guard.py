#!/usr/bin/env python3
"""S124's phase 2a: the tablebase guard's rows, and mop-up's domain by piece counts.

    ~/.venv/chess/bin/python adocs/data/S124_guard.py CORPUS --out GUARD.tsv > out.txt

CORPUS is S122's regenerated prepared training corpus (`.tuning/s122/prepared/corpus.tsv`),
read in place: `fen result score phase game ply x`, the result White relative. It is the
training side only -- the held-out games are its three views, which this never reads.

**The guard's rows** (DEC-350, the draft's "tablebase guard"): every row of five men or fewer,
labelled by the Syzygy 3-4-5 tables through python-chess (~/syzygy/) **at the row's own
halfmove clock** under the fifty-move rule, S124_tb_probe.py's law_verdict(): probe_wdl, and
where it reads 2 or -2, probe_dtz's n = |dtz|, python-chess documenting that n can mean n + 1
plies (DTZ50'' rounding):

    WON / LOST   |wdl| = 2 and clock + n + 1 <= 100: label 1 / 0 for the side to move's
    SPOILED      |wdl| = 2 and clock + n > 100: a draw under the rule, 0.5
    EDGE         |wdl| = 2 and clock + n = 100: the rounding decides; left out
    DRAWN        wdl 0, or 1 and -1, which the tables score drawn under the rule: 0.5

turned White relative. GUARD.tsv is CORPUS's own row with its result column replaced by that
label, in CORPUS's order, so a game stays a run of rows and tools/margin_census reads it as a
corpus (--games pairs two vectors' losses on it by game). A row the tables cannot probe -- one
with castling rights, or a table missing -- is counted and left out with the EDGE rows.

**Mop-up's domain by counts** (src/evaluation.hpp, S124's definitions): one side's king alone
against a side with no pawn whose men are worth two phase units or more (knight and bishop 1,
rook 2, queen 4), knights alone excepted. Rows and games, by the side with men's material, and
of it king, bishop and knight alone, the corner count's rows. A cross-check of the engine's own
count (adocs/data/S124_cell_check.cpp's `mop-up's domain` line), not a second definition.

No position is judged by the agent: counts, and the tables' verdicts (CHESS rule).
"""
import argparse
import collections
import hashlib
import sys

import chess
import chess.syzygy

TB = "/home/max/syzygy"
PHASE = {"N": 1, "B": 1, "R": 2, "Q": 4}
VERDICTS = ("WON", "LOST", "DRAWN", "SPOILED", "EDGE", "unprobed")


def law_verdict(white_wdl, dtz_n, clock):
    """White's verdict under the fifty-move rule at `clock` (S124_tb_probe.py's)."""
    if abs(white_wdl) != 2:
        return 0
    if clock + dtz_n + 1 <= 100:
        return white_wdl
    if clock + dtz_n > 100:
        return "SPOILED"
    return "EDGE"


def domain_of(placement):
    """The mop-up signature, e.g. 'KRvK', when the placement is in mop-up's domain."""
    white = [c for c in placement if c in "PNBRQ"]
    black = [c.upper() for c in placement if c in "pnbrq"]
    if white and black or not white and not black:
        return None
    men = white or black
    if "P" in men:
        return None
    if sum(PHASE[m] for m in men) < 2 or all(m == "N" for m in men):
        return None
    return "K" + "".join(sorted(men, key="QRBN".index)) + "vK"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("corpus")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    digest = hashlib.sha256()
    rows = 0
    games_all = set()
    small_rows = 0
    small_games = set()
    verdicts = collections.Counter()
    verdict_games = collections.defaultdict(set)
    written = 0
    written_games = set()
    domain_rows = collections.Counter()
    domain_games = collections.defaultdict(set)
    cache = {}

    with open(args.corpus, "rb") as raw, open(args.out, "w") as out, \
            chess.syzygy.open_tablebase(TB) as tb:
        for line_bytes in raw:
            digest.update(line_bytes)
            line = line_bytes.decode().rstrip("\n")
            fields = line.split("\t")
            fen = fields[0]
            game = fields[4]
            rows += 1
            games_all.add(game)
            placement = fen.split(" ", 1)[0]

            sig = domain_of(placement)
            if sig is not None:
                domain_rows[sig] += 1
                domain_games[sig].add(game)

            if sum(c.isalpha() for c in placement) > 5:
                continue
            small_rows += 1
            small_games.add(game)

            parts = fen.split()
            key = " ".join(parts[:4])
            if key not in cache:
                try:
                    board = chess.Board(fen)
                    wdl = tb.probe_wdl(board)
                    dtz = tb.probe_dtz(board) if abs(wdl) == 2 else 0
                    cache[key] = (wdl if board.turn == chess.WHITE else -wdl, abs(dtz))
                except (KeyError, ValueError, chess.syzygy.MissingTableError):
                    cache[key] = None
            probe = cache[key]
            if probe is None:
                verdicts["unprobed"] += 1
                verdict_games["unprobed"].add(game)
                continue
            white, n = probe
            v = law_verdict(white, n, int(parts[4]))
            if v == "EDGE":
                verdicts["EDGE"] += 1
                verdict_games["EDGE"].add(game)
                continue
            if v == "SPOILED":
                name, label = "SPOILED", "0.5"
            elif v == 2:
                name, label = "WON", "1.0"     # White wins under the rule
            elif v == -2:
                name, label = "LOST", "0.0"
            else:
                name, label = "DRAWN", "0.5"
            verdicts[name] += 1
            verdict_games[name].add(game)
            fields[1] = label
            out.write("\t".join(fields) + "\n")
            written += 1
            written_games.add(game)

    print("S124 guard, phase 2a: %s, %d rows, %d games, sha256 %s" % (
        args.corpus, rows, len(games_all), digest.hexdigest()))
    print("rows of five men or fewer: %d in %d games; %d distinct positions probed" % (
        small_rows, len(small_games), len(cache)))
    print("verdicts White relative at each row's own clock (WON and LOST are White's): " +
          ", ".join("%s %d rows %d games" % (v, verdicts[v], len(verdict_games[v]))
                    for v in VERDICTS))
    print("guard rows written: %d in %d games (EDGE and unprobed left out) -> %s" % (
        written, len(written_games), args.out))
    total_rows = sum(domain_rows.values())
    total_games = set().union(*domain_games.values()) if domain_games else set()
    print("mop-up's domain by counts: %d rows (%.3f %%) in %d games" % (
        total_rows, 100.0 * total_rows / max(rows, 1), len(total_games)))
    for sig, n in sorted(domain_rows.items(), key=lambda kv: -kv[1]):
        print("  %-10s rows %7d games %5d" % (sig, n, len(domain_games[sig])))
    print("S124-GUARD-OK")


if __name__ == "__main__":
    sys.exit(main())
