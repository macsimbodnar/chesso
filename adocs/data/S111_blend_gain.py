#!/usr/bin/env python3
"""S111: what the continuation tables do to the applied correction, measured
on the same games -- DEC-302 (3), the coordinator's. **A record, not a rule:
the figure is written beside S111's SPRT verdict and decides nothing.** No
design ships or stops on it, and nothing is pre-registered against it; S127
owns the blend's weights and its total gain (DEC-300).

The question S110's measurement (adocs/data/S110_blend_gain.py) answered for
its pair, asked for S111's tables on the tree S110's revert left: at the
seeds the pawn read and each continuation read count a third each, so if the
continuation reads are small -- cold slots, or slow to learn -- the applied
correction shrinks against the pawn read alone, the parent's correction. This
reads by how much.

The tune build's counter (src/search.cpp `record_blend_gain`, on only when
CHESSO_BLEND_GAIN names a file, written at exit; S110's counter left with its
revert and this is its shape for S111's reads) records at every
corrected-evaluation read the sums of |p|, the pawn read; of |applied|, the
blend S111 applies; of |applied - p|; of |two| and |four|, the continuation
reads; and how many reads, and how many entries, of the pawn table and of
each continuation table were 0. All three are read off the same games at the
same nodes, so the pawn read is exactly what the parent would have applied
there on these tables.

THE GAMES, S110's exactly: twelve self-play games of the candidate's tune
build at its defaults (CorrHistCont2Weight and CorrHistCont4Weight 100, the
tree as it stands), one engine process per game, twelve in parallel, one
`ucinewgame`, then per ply `position fen <start> moves ...` and `go nodes
4000000`, up to 120 plies or the game's end (python-chess's
`is_game_over(claim_draw=True)`), from the first twelve lines of
`books/noob_3moves.epd`. The engine plays both sides; nothing here judges a
move.

WHAT IT PRINTS, over all reads and by game third (the root's ply, 1-40,
41-80, 81-120), each a mean over the reads:

    R = |applied| / |p|      S111's applied scale against the pawn read's
                             alone (S110 recorded its own as R_avg, 0.787)
    |applied - p| / |p|      how far the continuation reads move it
    |two| / |p|, |four| / |p|   each continuation read's size
    pawn 0, two 0, four 0    the share of reads that read 0, and (in
                             brackets) whose entry was 0: the cold slots

    ~/.venv/chess/bin/python adocs/data/S111_blend_gain.py build-tune/src/chesso

About 5 to 15 minutes on the twelve cores, as S110's (about 6). Each search is
node-limited, so the machine's load moves no count and no move.
"""
import concurrent.futures
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
# The counter's fields after the third, in its order.
FIELDS = ("reads", "pawn_abs", "applied_abs", "moved_abs", "two_abs",
          "four_abs", "pawn_read_zero", "pawn_entry_zero", "two_read_zero",
          "two_entry_zero", "four_read_zero", "four_entry_zero")


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
            if len(values) != 1 + len(FIELDS):
                raise RuntimeError(f"game {index}: {len(values)} fields, not "
                                   f"{1 + len(FIELDS)} -- not S111's counter")
            rows[values[0]] = dict(zip(FIELDS, values[1:]))
    if sorted(rows) != [0, 1, 2]:
        raise RuntimeError(f"game {index}: the counter file has {sorted(rows)}")
    return index, len(moves), board.result(claim_draw=True), rows


def figures(t):
    if t["reads"] == 0 or t["pawn_abs"] == 0:
        return None
    mean_p = t["pawn_abs"] / t["reads"]
    per = lambda k: t[k] / t["reads"]
    return {
        "reads": t["reads"],
        "mean_p": mean_p,
        "R": t["applied_abs"] / t["pawn_abs"],
        "moved_p": t["moved_abs"] / t["pawn_abs"],
        "two_p": t["two_abs"] / t["pawn_abs"],
        "four_p": t["four_abs"] / t["pawn_abs"],
        "pawn_cold": per("pawn_read_zero"),
        "pawn_cold_entry": per("pawn_entry_zero"),
        "two_cold": per("two_read_zero"),
        "two_cold_entry": per("two_entry_zero"),
        "four_cold": per("four_read_zero"),
        "four_cold_entry": per("four_entry_zero"),
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

    print(f"\nS111 blend gain: {GAMES} games, go nodes {NODES}, up to {PLIES} plies, "
          f"engine {engine}")
    print("A record beside S111's verdict; it decides nothing (DEC-302 (3)).")
    head = (f"{'':12}{'reads':>10}{'mean|p|':>9}{'R':>7}{'moved/p':>8}"
            f"{'two/p':>7}{'four/p':>7}{'pawn 0':>8}{'(entry)':>8}"
            f"{'two 0':>7}{'(entry)':>8}{'four 0':>7}{'(entry)':>8}")
    print(head)
    for name, t in [("all", whole), ("plies 1-40", totals[0]),
                    ("plies 41-80", totals[1]), ("plies 81-120", totals[2])]:
        f = figures(t)
        if f is None:
            print(f"{name:12}{t['reads']:>10}  (no reads)")
            continue
        print(f"{name:12}{f['reads']:>10}{f['mean_p']:>9.2f}{f['R']:>7.3f}"
              f"{f['moved_p']:>8.3f}{f['two_p']:>7.3f}{f['four_p']:>7.3f}"
              f"{f['pawn_cold']:>8.3f}{f['pawn_cold_entry']:>8.3f}"
              f"{f['two_cold']:>7.3f}{f['two_cold_entry']:>8.3f}"
              f"{f['four_cold']:>7.3f}{f['four_cold_entry']:>8.3f}")

if __name__ == "__main__":
    main()
