#!/usr/bin/env python3
"""S024 verdict-1 H0 census: is the one-ply continuation history table
actually touched in ordinary play, before H0 is believed as a verdict on the
technique -- the step file's own pre-registration, "## Measurement, verdict
1, pre-registered 2026-09-12" in
`adocs/plan_current/S024_continuation_history.md`.

    ~/.venv/chess/bin/python adocs/data/S024_census_run.py sample
    ~/.venv/chess/bin/python adocs/data/S024_census_run.py run ENGINE

S222 RE-RAN THIS DRIVER ON ITS FITTED BUILD (adocs/data/S222_census.txt) AND
S231 RE-RUNS IT AGAIN WITH THE TWO-PLY TABLE COUNTED. The instrumentation
itself is throwaway -- five counters at S024, **eight since S231** -- patched
into a Release build in a detached worktree and removed with it, so this file
carries the counter contract and not the patch. `parse_go_output` accepts a
five-counter line and an eight-counter one and refuses anything else, the
three new counters being appended: a run of S024's own patch still reads the
way it always did, and the two-ply shares are reported only when the binary
counted them rather than printed as three zeroes that would read as an inert
table. The eight, in order, are listed at CENSUS_FIELDS_ONE_PLY below.

This script writes CENSUS_TSV, which is S024's own committed artefact, so a
re-run is taken from a worktree copy of the script and never from the main
tree -- what S222 did and what S231 does.

WHERE THE POSITIONS COME FROM. No fixed 400-position corpus file exists in
this repository. `src/search.cpp`'s null-move mate-band comment ("over 400
corpus positions at depth 10 the band is reached on 0 of 301620...") and
S103's reverse-futility hit-rate note (300 positions) both cite ad hoc,
uncommitted instrumented passes -- checked by grepping `adocs/data/` and
every `S165`/`S103`/`S108` file: none holds a checked-in 400-line FEN list.
So `sample()` below builds one, from `adocs/data/S219_aa_calibration.pgn`
(S219's book-comparison self-play, 1000 games, the newest corpus of real
chesso games on disk, srand-seeded and already committed).

WHY NOT `build/tools/pgn_to_positions`, per the task's first-choice method.
That tool hardcodes `DEFAULT_POSITION` as its replay start
(`tools/pgn_to_positions.cpp`: `load_FEN(DEFAULT_POSITION, &game)`), and every
one of this PGN's 1000 games carries its own `[SetUp "1"]`/`[FEN ...]`
book-opening header -- confirmed by grep, 1000 of 1000 `[SetUp "1"]` lines
for 1000 `[Event`. Replaying such a game's move list from the default
position desyncs at the first move that is illegal (or means something else)
on the wrong board, which is exactly the wall S042 already hit over this
same file (`adocs/plan_done/S042_en_passant_only_when_capturable.md`, "FEN
conformance" section): "these are self-play games from an opening book, not
from the default position, so `build/tools/pgn_to_positions` cannot replay
them, it hardcodes DEFAULT_POSITION" -- S042 drove the engine directly over
UCI instead (`position fen <book FEN> moves <uci...>` then `fen`), because
its need was to test the engine's own FEN writer. This script's need is
narrower -- valid FENs of positions from real games, not a test of the
engine's writer -- so it reads each game's `[FEN]` header and mainline moves
with `python-chess`, already this codebase's oracle everywhere a position is
checked against or built from outside the engine itself (S042's own X-FEN
comparison, S165's mate-set construction, S165's own defender sweep above
this file). No move is tracked by hand (CLAUDE.md, DEC-023) -- python-chess
replays every game and the sampled FEN is exactly the board after N of its
own validated pushes.

SAMPLING RULE, DETERMINISTIC. `random.Random(SEED)` (SEED=24, this step's own
id, so the seed is traceable) shuffles the 1000 game indices, then visits
them in that order and takes, from each, the position at the game's own
midpoint ply (`len(mainline_moves) // 2`): one candidate position per game,
so 400 distinct games rather than several plies clustered in a few, and a
per-game midpoint rather than a fixed ply so a 31-ply game and a 634-ply game
are each read from their own middle rather than a fixed ply that might be
past one of them. A game shorter than MIN_PLIES is skipped (none of these
1000 are: shortest is 31 plies, checked directly) and the shuffle moves to
its next index. The first N_POSITIONS games that yield a position are kept,
in shuffle order, so the seed alone reproduces the set.

DRIVING THE ENGINE, AND THE TRAP THIS AVOIDS. TOOLCHAIN.md's chess-oracle
warning is written about Stockfish but the trap is general: `printf
'...\ngo depth 10\n' | build/src/chesso` answers depth 1 at 49 nodes, not
depth 10 -- confirmed directly against both this census's own instrumented
binary and an unmodified `build/src/chesso`, same input, same wrong answer.
Closing stdin (what a one-shot `printf | engine` pipe does the instant its
input is written) reads to the engine the same way an explicit `quit` would:
it races `go` and kills the search before it finishes. The fix is the one
`S108_node_reach.py` and `S165_nmp_defender_sweep.py` already use: one
persistent `subprocess.Popen` per run, stdin held open across every command,
`isready`/`readyok` before each `go` and every `bestmove` read in full before
the next command is sent, `quit` only after the last `bestmove` -- never
written behind an un-awaited `go`.
"""
import argparse
import os
import random
import statistics
import subprocess
import sys
import time

import chess.pgn

HERE = os.path.dirname(os.path.abspath(__file__))
PGN = os.path.join(HERE, "S219_aa_calibration.pgn")
POSITIONS_TXT = os.path.join(HERE, "S024_census_positions.txt")
CENSUS_TSV = os.path.join(HERE, "S024_census.tsv")

SEED = 24  # S024's own id -- traceable, not a tuned or hand-picked value.
N_POSITIONS = 400
MIN_PLIES = 10  # guards against a near-empty game's "midpoint" being the
                # book FEN itself; none of these 1000 games are this short.
DEPTH = 10
HASH_MB = 16  # the S105 regime's value; stated explicitly rather than left
              # to whatever the binary's compiled default happens to be.


def sample():
    """Read S219_aa_calibration.pgn, pick 400 positions, write POSITIONS_TXT."""
    games = []
    with open(PGN) as fh:
        while True:
            game = chess.pgn.read_game(fh)
            if game is None:
                break
            games.append(game)

    if len(games) < N_POSITIONS:
        sys.exit("only %d games in %s, need %d" % (len(games), PGN, N_POSITIONS))

    order = list(range(len(games)))
    random.Random(SEED).shuffle(order)

    rows = []
    skipped = []
    for game_index in order:
        if len(rows) >= N_POSITIONS:
            break

        game = games[game_index]
        moves = list(game.mainline_moves())

        if len(moves) < MIN_PLIES:
            skipped.append(game_index)
            continue

        board = game.board()  # respects [SetUp]/[FEN]
        mid = len(moves) // 2

        for i, move in enumerate(moves):
            if i == mid:
                break
            board.push(move)

        rows.append({"game_index": game_index, "ply": mid, "fen": board.fen()})

    if len(rows) < N_POSITIONS:
        sys.exit("only %d of %d positions sampled (%d games skipped under "
                 "%d plies)" % (len(rows), N_POSITIONS, len(skipped), MIN_PLIES))

    with open(POSITIONS_TXT, "w") as out:
        out.write("# S024 verdict-1 H0 census positions. Regenerate with:\n")
        out.write("#   ~/.venv/chess/bin/python "
                 "adocs/data/S024_census_run.py sample\n")
        out.write("# %d positions from adocs/data/S219_aa_calibration.pgn "
                 "(1000 self-play games), one per game, at that game's own\n"
                 "# midpoint ply (len(moves)//2). Deterministic:\n"
                 "# random.Random(%d).shuffle(range(1000)), first %d games\n"
                 "# with >= %d plies, in shuffle order. Columns: game_index "
                 "(0-based, PGN order) TAB ply TAB fen.\n"
                 % (len(rows), SEED, N_POSITIONS, MIN_PLIES))
        for r in rows:
            out.write("%d\t%d\t%s\n" % (r["game_index"], r["ply"], r["fen"]))

    print("%d positions written to %s (%d games skipped under %d plies)"
         % (len(rows), POSITIONS_TXT, len(skipped), MIN_PLIES))
    return 0


def read_positions():
    rows = []
    with open(POSITIONS_TXT) as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            game_index, ply, fen = line.rstrip("\n").split("\t")
            rows.append({"game_index": int(game_index), "ply": int(ply), "fen": fen})
    return rows


class Engine:
    """One persistent UCI process. Every command is followed by a wait for
    the reply that answers it -- isready/readyok, position/isready/readyok,
    go/bestmove -- so quit is never written behind an un-awaited go."""

    def __init__(self, path):
        self.p = subprocess.Popen([path], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(self, s):
        self.p.stdin.write(s + "\n")
        self.p.stdin.flush()

    def read_until(self, prefix):
        lines = []
        while True:
            line = self.p.stdout.readline()
            if not line:
                sys.exit("engine closed stdout waiting for a line starting "
                         "%r; last lines: %r" % (prefix, lines[-5:]))
            line = line.rstrip("\n")
            lines.append(line)
            if line.startswith(prefix):
                return lines

    def isready(self):
        self.send("isready")
        self.read_until("readyok")

    def uci_handshake(self):
        self.send("uci")
        self.read_until("uciok")
        self.send("setoption name Hash value %d" % HASH_MB)
        self.isready()

    def position_fen(self, fen):
        self.send("position fen " + fen)
        # A refused position (S216 precedent: fail loudly, never silently
        # measure the wrong board) prints on the UCI channel before the next
        # readyok; isready/readyok is also the synchronisation go needs.
        self.send("isready")
        lines = self.read_until("readyok")
        refusals = [ln for ln in lines if "refused" in ln]
        if refusals:
            sys.exit("position fen refused: %r for %r" % (refusals, fen))

    def go_depth(self, depth):
        self.send("go depth %d" % depth)
        lines = self.read_until("bestmove")
        return lines

    def ucinewgame(self):
        self.send("ucinewgame")

    def quit(self):
        self.send("quit")
        self.p.wait(timeout=30)


# How many counters the census line carries, and what each shape means. S024
# instrumented five; S231 added the two-ply table and instruments eight, the
# three new ones **appended** so that a five-counter line stays exactly what it
# always was and S024's and S222's runs remain reproducible from their own
# instrumentation:
#
#   0 writes_total        history_on_quiet_cutoff() calls
#   1 writes_with_prev    those with prev_move != 0
#   2 reads_total         quiet score_move() evaluations
#   3 reads_with_prev     those where the one-ply term is consulted
#   4 reads_nonzero       those where the one-ply entry is non-zero
#   5 writes_with_prev2   writes with prev_move2 != 0            (S231)
#   6 reads_with_prev2    reads where the two-ply term is consulted   (S231)
#   7 reads_nonzero2      those where the two-ply entry is non-zero   (S231)
#
# Any other count is a wrong or half-applied instrumentation patch and the run
# refuses rather than reporting a share it has misread.
CENSUS_FIELDS_ONE_PLY = 5
CENSUS_FIELDS_TWO_PLY = 8

TWO_PLY_KEYS = ("writes_with_prev2", "reads_with_prev2", "reads_nonzero2")


def parse_go_output(lines):
    """The last regular info line's node count, and the census line's counters.

    Depth 10 is requested; the last info line before bestmove is the one
    search_bench.py and the engine's own protocol treat as the search's total
    (chesso.cpp: "[nodes] is the whole search's count")."""
    nodes = None
    census = None
    for line in lines:
        if line.startswith("info string cont_hist_census"):
            census = [int(x) for x in line.split()[3:]]
        elif line.startswith("info") and " nodes " in line:
            nodes = int(line.split(" nodes ")[1].split()[0])

    if nodes is None:
        sys.exit("no info line with a node count in: %r" % (lines,))
    if census is None:
        sys.exit("no cont_hist_census line in: %r -- wrong binary? "
                 "(needs the S024 census instrumentation)" % (lines,))
    if len(census) not in (CENSUS_FIELDS_ONE_PLY, CENSUS_FIELDS_TWO_PLY):
        sys.exit("cont_hist_census carries %d counters, expected %d (S024's "
                 "one-ply instrumentation) or %d (S231's, with the two-ply "
                 "table counted): %r"
                 % (len(census), CENSUS_FIELDS_ONE_PLY,
                    CENSUS_FIELDS_TWO_PLY, census))

    return nodes, census


def run(engine_path):
    positions = read_positions()
    engine = Engine(engine_path)
    engine.uci_handshake()

    rows = []
    t_start = time.time()

    for i, pos in enumerate(positions):
        engine.position_fen(pos["fen"])

        t0 = time.time()
        lines = engine.go_depth(DEPTH)
        elapsed = time.time() - t0

        nodes, census = parse_go_output(lines)
        row = {
            "idx": i + 1, "game_index": pos["game_index"], "ply": pos["ply"],
            "fen": pos["fen"], "nodes": nodes, "elapsed_s": elapsed,
            "writes_total": census[0], "writes_with_prev": census[1],
            "reads_total": census[2], "reads_with_prev": census[3],
            "reads_nonzero": census[4],
        }

        # S231. Absent from a five-counter line and 0 there, so a run of
        # S024's own instrumentation reports the two-ply shares as zero rather
        # than failing; a run of S231's reports them for real.
        for key, value in zip(TWO_PLY_KEYS, census[CENSUS_FIELDS_ONE_PLY:]):
            row[key] = value
        for key in TWO_PLY_KEYS:
            row.setdefault(key, 0)

        rows.append(row)

        engine.ucinewgame()

        if (i + 1) % 50 == 0:
            print("  %d / %d positions" % (i + 1, len(positions)),
                 file=sys.stderr)

    wall = time.time() - t_start
    engine.quit()

    totals = {
        key: sum(r[key] for r in rows)
        for key in ("nodes", "writes_total", "writes_with_prev", "reads_total",
                    "reads_with_prev", "reads_nonzero") + TWO_PLY_KEYS
    }

    with open(CENSUS_TSV, "w") as out:
        out.write("# S024 verdict-1 H0 census. Regenerate with:\n")
        out.write("#   ~/.venv/chess/bin/python adocs/data/S024_census_run.py "
                 "run <engine>\n")
        out.write("# One row per adocs/data/S024_census_positions.txt entry, "
                 "depth %d, Hash %d MB, ucinewgame between positions.\n"
                 % (DEPTH, HASH_MB))
        out.write("# writes = calls of history_on_quiet_cutoff(); "
                 "writes_with_prev = those with prev_move != 0; "
                 "writes_with_prev2 = those with prev_move2 != 0 (S231).\n")
        out.write("# reads = quiet-move score_move() evaluations; "
                 "reads_with_prev = those where the one-ply continuation term "
                 "is consulted; reads_nonzero = those where it is non-zero; "
                 "reads_with_prev2 and reads_nonzero2 are the same two for the "
                 "two-ply table (S231), and are 0 when the binary carries "
                 "S024's five-counter instrumentation.\n")
        columns = ["idx", "game_index", "ply", "fen", "nodes", "elapsed_s",
                   "writes_total", "writes_with_prev", "reads_total",
                   "reads_with_prev", "reads_nonzero"] + list(TWO_PLY_KEYS)
        out.write("\t".join(columns) + "\n")
        for r in rows:
            out.write("\t".join(str(r[k]) for k in columns) + "\n")
        out.write("# totals\t\t\t\t%d\t%.3f\t%d\t%d\t%d\t%d\t%d\t%d\t%d"
                 "\t%d\n" % (
            totals["nodes"], wall, totals["writes_total"],
            totals["writes_with_prev"], totals["reads_total"],
            totals["reads_with_prev"], totals["reads_nonzero"],
            totals["writes_with_prev2"], totals["reads_with_prev2"],
            totals["reads_nonzero2"]))

    shares = [r["reads_nonzero"] / r["reads_with_prev"]
             if r["reads_with_prev"] else 0.0 for r in rows]
    shares.sort()

    print("\npositions: %d" % len(rows))
    print("total nodes: %d" % totals["nodes"])
    print("wall time: %.1f s" % wall)
    print("writes_total: %d  writes_with_prev: %d  share: %.4f%%"
         % (totals["writes_total"], totals["writes_with_prev"],
            100.0 * totals["writes_with_prev"] / totals["writes_total"]
            if totals["writes_total"] else 0.0))
    print("reads_total: %d  reads_with_prev: %d  share: %.4f%%"
         % (totals["reads_total"], totals["reads_with_prev"],
            100.0 * totals["reads_with_prev"] / totals["reads_total"]
            if totals["reads_total"] else 0.0))
    print("reads_nonzero: %d  of reads_with_prev: %d  share: %.4f%%"
         % (totals["reads_nonzero"], totals["reads_with_prev"],
            100.0 * totals["reads_nonzero"] / totals["reads_with_prev"]
            if totals["reads_with_prev"] else 0.0))
    print("per-position reads_nonzero/reads_with_prev share: "
         "min %.4f%%  median %.4f%%  max %.4f%%"
         % (100.0 * shares[0], 100.0 * statistics.median(shares),
            100.0 * shares[-1]))

    # S231. The same three shares for the two-ply table, printed only when the
    # binary counted it -- a five-counter run would otherwise report three
    # zeroes that read as an inert table rather than as an uninstrumented one.
    if totals["reads_with_prev2"] or totals["writes_with_prev2"]:
        print("writes_with_prev2: %d  of writes_total: %d  share: %.4f%%"
             % (totals["writes_with_prev2"], totals["writes_total"],
                100.0 * totals["writes_with_prev2"] / totals["writes_total"]
                if totals["writes_total"] else 0.0))
        print("reads_with_prev2: %d  of reads_total: %d  share: %.4f%%"
             % (totals["reads_with_prev2"], totals["reads_total"],
                100.0 * totals["reads_with_prev2"] / totals["reads_total"]
                if totals["reads_total"] else 0.0))
        print("reads_nonzero2: %d  of reads_with_prev2: %d  share: %.4f%%"
             % (totals["reads_nonzero2"], totals["reads_with_prev2"],
                100.0 * totals["reads_nonzero2"] / totals["reads_with_prev2"]
                if totals["reads_with_prev2"] else 0.0))

        shares2 = sorted(r["reads_nonzero2"] / r["reads_with_prev2"]
                        if r["reads_with_prev2"] else 0.0 for r in rows)
        print("per-position reads_nonzero2/reads_with_prev2 share: "
             "min %.4f%%  median %.4f%%  max %.4f%%"
             % (100.0 * shares2[0], 100.0 * statistics.median(shares2),
                100.0 * shares2[-1]))
    else:
        print("two-ply table: not counted by this binary (five-counter "
             "instrumentation, S024's own)")

    print("\nS024-CENSUS-DONE")
    return 0


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="command", required=True)
    sub.add_parser("sample")
    run_parser = sub.add_parser("run")
    run_parser.add_argument("engine")
    args = ap.parse_args()

    if args.command == "sample":
        return sample()

    return run(args.engine)


if __name__ == "__main__":
    sys.exit(main())
