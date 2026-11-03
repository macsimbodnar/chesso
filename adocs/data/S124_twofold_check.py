#!/usr/bin/env python3
"""S124's research pass, 2026-11-03: do drawn games that ended in a tablebase-won lone-king
ending stop on a two-fold repetition datagen adjudicates and the search does not call a draw?

    ~/.venv/chess/bin/python adocs/data/S124_twofold_check.py LIST POOL_ROOT ENGINE > out.txt

LIST is adocs/data/S124_conversion.py's: drawn training games whose last root row stands in
KRvK, KQvK or KBNvK and is won there under the fifty-move rule (Syzygy, clock + |dtz| + 1 <=
100), `game sig fen` with that row's FEN. POOL_ROOT is the raw root pool the games' root rows
are read from. From each game's last root row, ENGINE searches 100000 nodes a move, one
process for the whole run, and plays two plies; the line is reported as reaching a position
the game's root rows already hold (a two-fold, which tools/datagen.cpp's terminal_result()
scores 0.5 through is_position_repeated(), while src/search.cpp's classify_repetition() reads
a pre-root single occurrence as no draw, DEC-173), or not.

**An indication, not a bound either way.** This search is not datagen's: its table did not
carry across the game, it is given no history, and its transposition table carries across
these positions instead. It can repeat where datagen's search did not, and miss a repetition
datagen's search made; positions in check are never recorded, so a repetition through one is
invisible here. No position is judged.
"""
import collections
import sys

import chess
import chess.engine


def key(board):
    return " ".join(board.fen().split()[:4])


def main(list_path, root_pool, engine_path):
    games = {}
    for line in open(list_path):
        game, sig, fen = line.rstrip("\n").split("\t")
        games[game] = (sig, fen)
    keys = collections.defaultdict(set)
    for line in open(root_pool):
        f = line.split("\t", 6)
        if f[4] in games:
            keys[f[4]].add(" ".join(f[0].split()[:4]))
    out = collections.Counter()
    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        for game, (sig, fen) in sorted(games.items()):
            board = chess.Board(fen)
            verdict = "no repeat in two plies"
            for _ in range(2):
                if board.is_game_over():
                    verdict = "game over: " + board.result()
                    break
                result = engine.play(board, chess.engine.Limit(nodes=100000))
                board.push(result.move)
                if key(board) in keys[game]:
                    verdict = "two-fold of a recorded root position"
                    break
            out[(sig, verdict)] += 1
    for sig in ("KRvK", "KQvK", "KBNvK"):
        total = sum(v for (s, _), v in out.items() if s == sig)
        print("  %-6s drawn games ending WON in it %d" % (sig, total))
        for (s, verdict), v in sorted(out.items()):
            if s == sig:
                print("    %-40s %d" % (verdict, v))
    print("S124-TWOFOLD-OK")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit("S124_twofold_check.py LIST POOL_ROOT ENGINE")
    main(sys.argv[1], sys.argv[2], sys.argv[3])
