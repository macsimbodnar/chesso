#!/usr/bin/env python3
"""S162. How often does a real game reach the halfmove clock this step changed?

    ~/.venv/chess/bin/python adocs/data/S162_clock_census.py <games.pgn>

S162 makes checkmate outrank the 100-halfmove draw, and its accepts asked for a
`--nonreg` verdict as insurance. DEC-107 replaced that with this: an SPRT can
only report the effect of a path the games actually take, so count the path
instead of booking a night on it. Two numbers, and they point opposite ways.

  peak clock >= 80    the games where the *branch* runs. This engine's depth
                      means a root at 80 sees clock 100 inside its tree, so
                      these are the games that pay for the added is_check() and
                      move generation
  checkmate at >= 100 the games where the *behaviour* differs. This is the only
                      count the SPRT could have measured

Reported per ten-halfmove bucket so the shape is visible and not just the tail.
Run it over any run's PGN, not only S162's -- the shape is a property of the
control and the adjudication settings, both of which move.
"""

import sys
from collections import Counter

import chess.pgn


def main(path):
    buckets = Counter()
    games = peak_max = mated_at_100 = mated_at_90 = 0

    with open(path) as handle:
        while True:
            game = chess.pgn.read_game(handle)

            if game is None:
                break

            games += 1
            board = game.board()
            peak = 0

            for move in game.mainline_moves():
                board.push(move)
                peak = max(peak, board.halfmove_clock)

                # Terminal positions only: a checkmate mid-game does not exist,
                # so this is the last position of the game or nothing.
                if board.is_checkmate():
                    if board.halfmove_clock >= 100:
                        mated_at_100 += 1
                        print("  fired: %s" % board.fen())
                    elif board.halfmove_clock >= 90:
                        mated_at_90 += 1

            peak_max = max(peak_max, peak)
            buckets[min(peak // 10 * 10, 100)] += 1

    print("games                     %6d" % games)
    print("highest halfmove clock    %6d" % peak_max)
    print("peak >= 80, branch runs   %6d  (%.1f %%)"
          % (sum(v for k, v in buckets.items() if k >= 80),
             100.0 * sum(v for k, v in buckets.items() if k >= 80) / max(games, 1)))
    print("checkmate at clock >= 100 %6d   <-- the only count an SPRT could see"
          % mated_at_100)
    print("checkmate at clock 90-99  %6d" % mated_at_90)
    print()

    for low in sorted(buckets):
        print("  peak clock %3d-%3d : %6d games" % (low, low + 9, buckets[low]))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
