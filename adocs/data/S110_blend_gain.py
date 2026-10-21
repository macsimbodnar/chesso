#!/usr/bin/env python3
"""S110: sum or average, measured -- which blend of the three correction
tables keeps the applied correction closest to the pawn table's own scale in
game-like play (the coordinator's question after DEC-300).

The update's target, `best_so_far - raw_eval`, does not depend on the blend,
so both blends are read off the same tables: the tune build's counter
(src/search.cpp `record_blend_gain`, on only when CHESSO_BLEND_GAIN names a
file, written at exit) records at every corrected-evaluation read the pawn
read p and the non-pawn reads w and b, before the mate band's hold, and this
script plays the games that fill the tables and reads the counter back.

THE GAMES. Twelve self-play games of the candidate's tune build at its
defaults (CorrHistNonPawnWeight 100, the tree as it stands), one engine
process per game, twelve in parallel. Each game is one `ucinewgame`, then per
ply `position fen <start> moves ...` and `go nodes 4000000`, up to 120 plies
or the game's end (python-chess's `is_game_over(claim_draw=True)`, which
ends a game at a threefold repetition or the fifty-move rule a harness would
adjudicate), with no `ucinewgame` between moves, so the tables accumulate as
in a match. The starts are the first twelve lines of the match book,
`books/noob_3moves.epd`, the file fastchess.sh plays; its lines are FENs
three moves in, so each game starts from its FEN and there are no book moves
to play first. Hash is the engine's default. The engine plays both sides; its
moves are what python-chess is given, and nothing here judges one.

WHAT IT PRINTS, over all reads of all games and again by game third (the
root's ply from the start FEN, 1-40, 41-80, 81-120), with m = (w + b) / 2:

    R_sum = mean|p + m| / mean|p|        the first design's applied scale
    R_avg = mean|(p + m) / 2| / mean|p|  DEC-300's, before its rounding
    mean|m| / mean|p|
    the share of reads with both non-pawn reads 0 (and both entries 0), and
    with the pawn read 0 (and the pawn entry 0)

R_avg is R_sum / 2 identically, since |(p + m) / 2| = |p + m| / 2: the
question is only whether R_sum lies above or below the square root of 2.

THE RULE, PRE-REGISTERED before the run (S110's step file, "2026-10-21: sum
or average, measured", written first): **the design whose R is closer to 1 in
|ln R| ships to the SPRT. A tie within 0.05 in |ln R| goes to the sum**, the
form the published practice and the family's records use. Read over all
games together; the thirds are reported and decide nothing.

    ~/.venv/chess/bin/python adocs/data/S110_blend_gain.py build-tune/src/chesso

About 5 to 15 minutes on the twelve cores. Each search is node-limited, so
the machine's load moves no count and no move.
"""
import concurrent.futures
import math
import os
import subprocess
import sys
import tempfile

import chess

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def book_path():
    """The match book: gitignored and fetched (books/fetch_book.sh), so a
    linked worktree has none of its own and reads the main checkout's, the
    file fastchess.sh plays."""
    here = os.path.join(ROOT, "books", "noob_3moves.epd")
    if os.path.isfile(here):
        return here
    common = subprocess.run(["git", "-C", ROOT, "rev-parse", "--path-format=absolute",
                             "--git-common-dir"], capture_output=True, text=True).stdout.strip()
    main = os.path.join(os.path.dirname(common), "books", "noob_3moves.epd")
    if common and os.path.isfile(main):
        return main
    sys.exit("no books/noob_3moves.epd here or in the main checkout")


BOOK = book_path()
GAMES = 12
PLIES = 120
NODES = 4000000
FIELDS = ("reads", "pawn2", "sum2", "pair2", "pair_read_zero",
          "pair_entry_zero", "pawn_read_zero", "pawn_entry_zero")


def starts():
    fens = []
    with open(BOOK) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            # A FEN with its clocks, or an EPD line's four fields.
            fields = line.split()
            if len(fields) >= 6 and fields[4].isdigit() and fields[5].isdigit():
                fens.append(" ".join(fields[:6]))
            else:
                fens.append(" ".join(fields[:4]) + " 0 1")
            if len(fens) == GAMES:
                break
    if len(fens) != GAMES:
        sys.exit(f"{BOOK} has fewer than {GAMES} lines")
    return fens


def play(engine, fen, index, tmp):
    path = os.path.join(tmp, f"game{index}.txt")
    env = dict(os.environ, CHESSO_BLEND_GAIN=path)
    p = subprocess.Popen([engine], stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE, text=True, bufsize=1, env=env)

    def send(s):
        p.stdin.write(s + "\n")
        p.stdin.flush()

    def wait_for(prefix):
        while True:
            line = p.stdout.readline()
            if not line:
                raise RuntimeError(f"game {index}: engine died")
            if line.startswith(prefix):
                return line

    send("uci")
    wait_for("uciok")
    send("ucinewgame")
    send("isready")
    wait_for("readyok")

    board = chess.Board(fen)
    moves = []
    while len(moves) < PLIES and not board.is_game_over(claim_draw=True):
        send(f"position fen {fen}" + ((" moves " + " ".join(moves)) if moves else ""))
        send(f"go nodes {NODES}")
        best = wait_for("bestmove").split()[1]
        move = chess.Move.from_uci(best)
        if move not in board.legal_moves:
            raise RuntimeError(f"game {index}: illegal bestmove {best} at {board.fen()}")
        board.push(move)
        moves.append(best)

    send("quit")
    p.wait(timeout=60)

    rows = {}
    with open(path) as f:
        for line in f:
            values = [int(v) for v in line.split()]
            rows[values[0]] = dict(zip(FIELDS, values[1:]))
    if sorted(rows) != [0, 1, 2]:
        raise RuntimeError(f"game {index}: the counter file has {sorted(rows)}")
    return index, len(moves), board.result(claim_draw=True), rows


def figures(t):
    if t["reads"] == 0 or t["pawn2"] == 0:
        return None
    r_sum = t["sum2"] / t["pawn2"]
    return {
        "reads": t["reads"],
        "R_sum": r_sum,
        "R_avg": r_sum / 2,
        "m_over_p": (t["pair2"] / 2) / t["pawn2"],
        "pair_cold": t["pair_read_zero"] / t["reads"],
        "pair_cold_entry": t["pair_entry_zero"] / t["reads"],
        "pawn_cold": t["pawn_read_zero"] / t["reads"],
        "pawn_cold_entry": t["pawn_entry_zero"] / t["reads"],
        "mean_p": t["pawn2"] / 2 / t["reads"],
    }


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    engine = os.path.abspath(sys.argv[1])
    fens = starts()

    with tempfile.TemporaryDirectory() as tmp:
        with concurrent.futures.ThreadPoolExecutor(max_workers=GAMES) as pool:
            results = list(pool.map(lambda a: play(engine, a[1], a[0], tmp),
                                    enumerate(fens)))

    totals = [dict.fromkeys(FIELDS, 0) for _ in range(3)]
    for index, plies, result, rows in results:
        print(f"game {index:2d}  {plies:3d} plies  {result:7s}  {fens[index]}")
        for third in range(3):
            for k in FIELDS:
                totals[third][k] += rows[third][k]
    whole = {k: sum(t[k] for t in totals) for k in FIELDS}

    print(f"\nS110 blend gain: {GAMES} games, go nodes {NODES}, up to {PLIES} plies, "
          f"engine {engine}")
    head = (f"{'':12}{'reads':>10}{'mean|p|':>9}{'R_sum':>8}{'R_avg':>8}{'|ln Rs|':>9}"
            f"{'|ln Ra|':>9}{'m/p':>7}{'pair 0':>8}{'(entry)':>8}{'pawn 0':>8}{'(entry)':>8}")
    print(head)
    for name, t in [("all", whole), ("plies 1-40", totals[0]),
                    ("plies 41-80", totals[1]), ("plies 81-120", totals[2])]:
        f = figures(t)
        if f is None:
            print(f"{name:12}{t['reads']:>10}  (no reads)")
            continue
        print(f"{name:12}{f['reads']:>10}{f['mean_p']:>9.2f}{f['R_sum']:>8.3f}{f['R_avg']:>8.3f}"
              f"{abs(math.log(f['R_sum'])):>9.3f}{abs(math.log(f['R_avg'])):>9.3f}"
              f"{f['m_over_p']:>7.3f}{f['pair_cold']:>8.3f}{f['pair_cold_entry']:>8.3f}"
              f"{f['pawn_cold']:>8.3f}{f['pawn_cold_entry']:>8.3f}")

    f = figures(whole)
    ls, la = abs(math.log(f["R_sum"])), abs(math.log(f["R_avg"]))
    if abs(ls - la) <= 0.05:
        verdict = "tie within 0.05 in |ln R|: the sum ships"
    elif ls < la:
        verdict = "the sum is closer to 1: the sum ships"
    else:
        verdict = "the average is closer to 1: the average ships"
    print(f"\nrule: |ln R_sum| {ls:.3f}, |ln R_avg| {la:.3f} -- {verdict}")


if __name__ == "__main__":
    main()
