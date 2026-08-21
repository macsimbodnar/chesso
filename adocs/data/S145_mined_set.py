#!/usr/bin/env python3
"""S145. The mined breadth set: mates chesso actually reached, and how many of
them it finds.

    ~/.venv/chess/bin/python adocs/data/S145_mined_set.py mine   --games 6000
    ~/.venv/chess/bin/python adocs/data/S145_mined_set.py score  --depth 10 --floor 143
    ~/.venv/chess/bin/python adocs/data/S145_mined_set.py score  --depth 10 \
        --option RfpMinPly=1 --engine build-tune/src/chesso

WHY THIS EXISTS BESIDE THE CONSTRUCTED SET, AND WHY IT IS SCORED DIFFERENTLY.

The constructed set in S145_mate_set.py tests one guard against one hazard: a
node that is a forced loss while its static score is high. It is small, every
position in it is a proof, and every position is asserted. This set is the
opposite instrument. It is wide, it is drawn from positions the engine met in
its own games, and no single position in it is asserted at all.

That is deliberate and it is the lesson S145's survey took from the field. Of
seventeen engines surveyed, two wrote exact mate-distance tests, watched their
own pruning break them, and switched the tests off rather than the pruning --
one marking them "requires no pruning", the other deleting the CI step as
flaky. Per-position pass/fail over mined positions is what produced that
outcome: a search is allowed to miss any particular deep mate, so a suite that
forbids it is a suite that will be disabled. A count with a floor is not: it
says "the engine finds at least this many of these, and a change that finds
fewer has cost something", which is a claim a search can actually keep.

ONE POSITION PER GAME, AND WHICH ONE. The final position of the game. These
games were adjudicated -- `-resign movecount=3 score=400` -- so almost none of
them ends in a played mate, and the last position is where the losing side is
most lost and therefore where a forced mate is most likely to exist unplayed.
Measured over 150 games: 10 carry a forced mate for the side to move, 6.7 %,
distances 2 to 10. Sampling a random ply instead yields far less for the same
stockfish time.

LABELLED BY STOCKFISH, NOT BY CHESSO. The label is the thing under test, so it
cannot come from the engine under test. Stockfish runs at a node limit rather
than a depth or a time limit: node-limited and single-threaded is the only one
of the three that gives the same answer on a re-run and on a loaded machine.

DEC-016. Stockfish is run as a binary and nothing is copied from it. The
positions are chesso's own games.
"""

import argparse
import os
import sys

import chess
import chess.engine
import chess.pgn


REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TSV = os.path.join(REPO, "adocs", "data", "S145_mined_set.tsv")
PGN = os.path.join(REPO, ".spsa", "S085", "games.pgn")

STOCKFISH = "/usr/games/stockfish"
STOCKFISH_OPTIONS = {"Threads": 1, "Hash": 16}
LABEL_NODES = 200_000

# A mate further away than this is not a mate-finding test, it is a tablebase
# question. Ten is where the measured distances stopped.
MAX_DISTANCE = 10

COLUMNS = ["fen", "distance", "game"]


def mine(pgn_path, limit, out):
    rows = []
    seen = set()
    games = 0

    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as engine:
        engine.configure(STOCKFISH_OPTIONS)

        with open(pgn_path) as handle:
            while games < limit:
                game = chess.pgn.read_game(handle)
                if game is None:
                    break

                moves = list(game.mainline_moves())
                if len(moves) < 4:
                    continue

                games += 1

                board = game.board()
                for move in moves:
                    board.push(move)

                if board.is_game_over():
                    continue

                score = engine.analyse(
                    board, chess.engine.Limit(nodes=LABEL_NODES))["score"].relative

                if not score.is_mate():
                    continue

                distance = score.mate()
                if distance is None or not 1 <= distance <= MAX_DISTANCE:
                    continue

                # The same opening book position can end the same way twice.
                fen = board.fen()
                if fen in seen:
                    continue
                seen.add(fen)

                rows.append({"fen": fen, "distance": distance, "game": games})

                if len(rows) % 25 == 0:
                    print("  %d games, %d positions" % (games, len(rows)), flush=True)

    rows.sort(key=lambda r: (r["distance"], r["fen"]))

    with open(out, "w") as handle:
        handle.write("# S145 mined breadth set: the final position of one game per row,\n")
        handle.write("# labelled by stockfish at %d nodes. Regenerate with:\n" % LABEL_NODES)
        handle.write("#   ~/.venv/chess/bin/python adocs/data/S145_mined_set.py mine --games N\n")
        handle.write("# Scored as a count with a floor, never per position:\n")
        handle.write("#   ~/.venv/chess/bin/python adocs/data/S145_mined_set.py score"
                     " --depth 10 --floor 143\n")
        handle.write("\t".join(COLUMNS) + "\n")

        for row in rows:
            handle.write("%s\t%d\t%d\n" % (row["fen"], row["distance"], row["game"]))

    print("\n%d games read, %d positions written to %s" % (games, len(rows), out))

    for distance in range(1, MAX_DISTANCE + 1):
        count = sum(1 for r in rows if r["distance"] == distance)
        if count:
            print("  mate in %2d: %d" % (distance, count))

    return 0


def read_tsv(path=TSV):
    rows = []

    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or not line.strip():
                continue
            field = line.rstrip("\n").split("\t")
            if field[0] == "fen":
                continue
            rows.append({"fen": field[0], "distance": int(field[1]),
                         "game": int(field[2])})

    return rows


def score(engine_path, depth, options, floor, path=TSV):
    """How many of the set the engine finds, and at what distance.

    Three counts, because they fail differently. `any` is a mate score of the
    right sign at all -- the engine knows it is winning by force. `exact` is the
    distance stockfish gave. `wrong_sign` is the one that is never acceptable at
    any count: a mate score for the side that is being mated.
    """
    rows = read_tsv(path)
    found_any = 0
    found_exact = 0
    wrong_sign = 0

    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        engine.configure(dict({"Hash": 16}, **options))

        for row in rows:
            board = chess.Board(row["fen"])
            result = engine.analyse(board, chess.engine.Limit(depth=depth))
            reported = result["score"].relative

            if not reported.is_mate():
                continue

            mate = reported.mate()
            if mate is None:
                continue

            if mate < 0:
                wrong_sign += 1
                print("  WRONG SIGN: engine says mate %d, stockfish says mate %d: %s"
                      % (mate, row["distance"], row["fen"]))
                continue

            found_any += 1
            if mate == row["distance"]:
                found_exact += 1

    print("%d positions at depth %d, options %s" % (len(rows), depth, options or "{}"))
    print("  mate found, right sign : %d" % found_any)
    print("  exact distance         : %d" % found_exact)
    print("  mate score, wrong sign : %d" % wrong_sign)

    # The floor, and where the number comes from. Measured 2026-08-21 on the
    # shipping build at depth 10: 147 found, 146 exact. The same run on the tune
    # build at `RfpMinPly` 1 and 0 -- which are the same engine, the root being
    # exempted by `!is_pv` and not by the parameter -- reads 140 and 139. So a
    # floor of 143 sits strictly between the shipping value and the value the
    # removed guard produces: it fails when the guard fails and not when the tree
    # shifts underneath it. A wrong sign is never acceptable at any count.
    if floor is None:
        return 0

    failed = found_exact < floor or wrong_sign > 0
    print("  floor %d: %s" % (floor, "FAILED" if failed else "met"))

    return 1 if failed else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["mine", "score"])
    parser.add_argument("--pgn", default=PGN)
    parser.add_argument("--out", default=TSV)
    parser.add_argument("--games", type=int, default=6000)
    parser.add_argument("--depth", type=int, default=10)
    parser.add_argument("--engine", default=os.path.join(REPO, "build", "src", "chesso"))
    parser.add_argument("--option", action="append", default=[],
                        help="NAME=VALUE, passed to the engine with setoption")
    parser.add_argument("--floor", type=int, default=None,
                        help="exit non-zero if fewer than this many exact, or "
                             "if any mate score has the wrong sign")
    args = parser.parse_args()

    if args.command == "mine":
        return mine(args.pgn, args.games, args.out)

    options = {}
    for pair in args.option:
        name, _, value = pair.partition("=")
        options[name] = int(value) if value.lstrip("-").isdigit() else value

    return score(args.engine, args.depth, options, args.floor, args.out)


if __name__ == "__main__":
    sys.exit(main())
