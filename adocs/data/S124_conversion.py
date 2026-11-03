#!/usr/bin/env python3
"""S124's research pass, 2026-11-03: how the training games that reached a lone-king ending on
the board ended, read from the games' root rows and the tablebase under the fifty-move rule.

    ~/.venv/chess/bin/python adocs/data/S124_conversion.py POOL_ROOT HELDOUT LIST > out.txt

POOL_ROOT is S122's raw root pool (`.tuning/s122/corpus/pool.tsv.root`): datagen's root rows,
positions the game itself stood in, recorded where the side to move is not in check, no mate
was found and the score is inside the quiet limit. A qs row is a quiescence leaf and not a
position the game reached, so games are classified here by their root rows alone. HELDOUT is
the held-out games' ids, one a line; their results are not read. LIST is written: one line a
drawn training game whose last root row stands in KRvK, KQvK or KBNvK and is WON there under
the rule, `game sig fen`, for adocs/data/S124_twofold_check.py.

Per signature of mop-up's domain (a lone king against a pawnless side; the signature is the
side with men's pieces): the training games with a root row in it, how they ended from that
side's view, the drawn ones with a root row WON under the rule (Syzygy probe_wdl 2 for that
side and clock + |probe_dtz| + 1 <= 100, so DTZ50'' rounding cannot spoil it), and the drawn
ones whose last root row stands in the signature, with that row's clock. Counts and the
tables' verdicts only; no position is judged by the agent (CHESS rule).
"""
import collections
import sys

import chess
import chess.syzygy

TB = "/home/max/syzygy"
SIGS = ("KRvK", "KQvK", "KBNvK", "KRBvK", "KRNvK", "KBBvK")


def lone_sig(placement):
    """The mop-up signature of a FEN placement, 'KRvK' and so on, and the side with men
    (0 White, 1 Black); None for anything else."""
    white = [c for c in placement if c in "PNBRQ"]
    black = [c for c in placement if c in "pnbrq"]
    if (white and black) or (not white and not black) or len(white) + len(black) > 3:
        return None
    men, side = (white, 0) if white else ([c.upper() for c in black], 1)
    if "P" in men:
        return None
    sig = "K" + "".join(sorted(men, key="QRBN".index)) + "vK"
    return (sig, side) if sig in SIGS else None


def main(pool, heldout_path, list_path):
    heldout = {line.strip() for line in open(heldout_path) if line.strip()}
    last = {}                                   # game -> (ply, fen), training games only
    result = {}
    reached = collections.defaultdict(dict)     # sig -> game -> [side, won_rows, rows]
    cache = {}
    with chess.syzygy.open_tablebase(TB) as tb:
        for line in open(pool):
            f = line.split("\t", 7)
            game = f[4]
            if game in heldout:
                continue
            ply = int(f[5])
            fen = f[0]
            result[game] = f[1]
            if game not in last or ply > last[game][0]:
                last[game] = (ply, fen)
            placement = fen.split(" ", 1)[0]
            if sum(c.isalpha() for c in placement) > 5:
                continue
            hit = lone_sig(placement)
            if hit is None:
                continue
            sig, side = hit
            parts = fen.split()
            key = " ".join(parts[:4])
            if key not in cache:
                board = chess.Board(fen)
                wdl = tb.probe_wdl(board)
                dtz = tb.probe_dtz(board) if abs(wdl) == 2 else 0
                cache[key] = (wdl if board.turn == chess.WHITE else -wdl, abs(dtz))
            white, n = cache[key]
            view = white if side == 0 else -white
            won = view == 2 and int(parts[4]) + n + 1 <= 100
            slot = reached[sig].setdefault(game, [side, 0, 0])
            slot[1] += 1 if won else 0
            slot[2] += 1

    out = open(list_path, "w")
    print("signature: training games with a root row in it; their results from the side with men; drawn games"
          " with a root row WON under the rule; drawn games whose last root row stands in it, and of those the ones"
          " WON there, by that row's clock")
    for sig in SIGS:
        games = reached[sig]
        res = collections.Counter()
        drawn_won = 0
        ended = 0
        ended_won = 0
        clocks = collections.Counter()
        other = collections.Counter()
        for game, (side, won_rows, _rows) in games.items():
            r = float(result[game])
            sr = r if side == 0 else 1.0 - r
            res[sr] += 1
            if sr != 0.5:
                continue
            if won_rows > 0:
                drawn_won += 1
            ply, fen = last[game]
            hit = lone_sig(fen.split(" ", 1)[0])
            if hit is None or hit[0] != sig:
                continue
            ended += 1
            parts = fen.split()
            white, n = cache[" ".join(parts[:4])]
            view = white if side == 0 else -white
            clock = int(parts[4])
            if view == 2 and clock + n + 1 <= 100:
                ended_won += 1
                clocks["<40" if clock < 40 else ("40-89" if clock < 90 else ">=90")] += 1
                out.write("%s\t%s\t%s\n" % (game, sig, fen))
            else:
                lone_to_move = (parts[1] == "w") != (side == 0)
                verdict = {2: "won past the rule", 0: "drawn", -2: "lost"}.get(view, "wdl %d" % view)
                other["%s, %s to move, clock %s" % (verdict, "lone king" if lone_to_move else "side with men",
                                                    "0" if clock == 0 else ">0")] += 1
        print("  %-6s games %5d  won %5d  drawn %5d  lost %4d   drawn with a WON root row %5d   drawn ending in it %5d,"
              " WON at that last row %5d (clock <40 %d, 40-89 %d, >=90 %d)" % (
                  sig, len(games), res[1.0], res[0.5], res[0.0], drawn_won, ended, ended_won,
                  clocks["<40"], clocks["40-89"], clocks[">=90"]))
        for what, count in sorted(other.items()):
            print("         drawn ending in it, not WON at that last row: %-45s %d" % (what, count))
    out.close()
    print("training games read %d; distinct positions probed %d" % (len(last), len(cache)))
    print("S124-CONVERSION-OK")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit("S124_conversion.py POOL_ROOT HELDOUT LIST")
    main(sys.argv[1], sys.argv[2], sys.argv[3])
