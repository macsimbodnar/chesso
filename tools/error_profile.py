#!/usr/bin/env python3
"""Rank one engine's own errors over many games, from a reference engine.

Takes the PGN of a match, runs every position through the S016 pipeline, and
reports where the centipawns actually went: bucketed by game phase, by error
size, and by the two crossed. This is the step that decides which evaluation
term is worth writing, because three borrowed Elo figures have now failed to
transfer here (DEC-019). Where this engine loses centipawns is measurable; where
other engines report gains is not evidence about this one.

    tools/error_profile.py match.pgn --player achesso \\
        --engine ~/.local/bin/stockfish --nodes 1000000 --workers 4 \\
        --raw-out raw.tsv

Scoring follows S016 exactly. Evaluations are side-to-move relative, so for ply
i the mover's position afterwards is worth -score[i+1] and the cost of the move
is score[i] + score[i+1]: positive means the mover gave that much away.

Book moves are excluded -- the engine did not choose them.

Re-bucketing an existing run costs nothing and needs no engine:

    tools/error_profile.py --from-raw raw.tsv
"""

import argparse
import os
import re
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor

MATE = 100000

# A mate score is not a centipawn quantity, and one mate at either end of a
# game would otherwise outweigh every real error in it. Scores are clamped
# before any arithmetic, so the worst a single move can be charged is twice
# this. Stated in the report, because it is a choice and not a fact.
CLAMP_CP = 1000

# GAME_PHASE_MAX in src/evaluation.hpp. 24 is a full board; 0 is kings and
# pawns. The engine emits this value itself, through pgn_to_positions, so a
# finding here names the same quantity the tapered evaluation tapers on.
GAME_PHASE_MAX = 24

# Lower bound, inclusive, of each band. Bands are contiguous and cover 0..24.
PHASE_BANDS = [
    (22, "opening"),
    (14, "early middlegame"),
    (7, "late middlegame"),
    (1, "endgame"),
    (0, "pawn endgame"),
]

COST_BANDS = [
    (0, 25, "noise <25"),
    (25, 50, "25-49"),
    (50, 100, "50-99"),
    (100, 200, "100-199"),
    (200, 400, "200-399"),
    (400, 1000, "400-999"),
    (1000, 1 << 30, ">=1000"),
]


def phase_band(phase):
    for lower, name in PHASE_BANDS:
        if phase >= lower:
            return name
    return PHASE_BANDS[-1][1]


def cost_band(cost):
    for lower, upper, name in COST_BANDS:
        if lower <= cost < upper:
            return name
    return COST_BANDS[-1][2]


#-############################  PGN PARSING  ##############################-#

HEADER = re.compile(r'\[(\w+)\s+"(.*)"\]')

# One pass over the movetext. Order matters: comments before anything that
# could appear inside one.
TOKEN = re.compile(r"""
      \{(?P<comment>[^}]*)\}
    | (?P<nag>\$\d+)
    | (?P<number>\d+\.(\.\.)?)
    | (?P<result>1-0|0-1|1/2-1/2|\*)
    | (?P<san>[A-Za-z][\w+#=\-]*)
""", re.VERBOSE)


def split_games(text):
    """Every game in a PGN file, as (headers, movetext)."""
    games = []
    headers, movetext = {}, []
    in_movetext = False

    for line in text.splitlines():
        stripped = line.strip()

        if stripped.startswith("["):
            if in_movetext and movetext:
                games.append((headers, " ".join(movetext)))
                headers, movetext = {}, []
                in_movetext = False

            match = HEADER.match(stripped)
            if match:
                headers[match.group(1)] = match.group(2)
            continue

        if stripped:
            in_movetext = True
            movetext.append(stripped)

    if movetext:
        games.append((headers, " ".join(movetext)))

    return games


def parse_moves(movetext):
    """(san, comment) per move, in order. Comment is "" when there is none."""
    moves = []

    for match in TOKEN.finditer(movetext):
        if match.group("san"):
            moves.append([match.group("san"), ""])
        elif match.group("comment") and moves:
            # fastchess appends the adjudication reason to the last comment,
            # after the eval, so a move can carry both.
            moves[-1][1] = match.group("comment")

    return [(san, comment) for san, comment in moves]


# fastchess writes "{-0.77/13 0.427s}" -- score, depth, time. The score is from
# the mover's point of view, which is the same convention as the reference
# engine's score for the position before the move.
ENGINE_EVAL = re.compile(r"^([+-]?\d+\.\d+|[+-]?M\d+|[+-]?\d+)/(\d+)")


def parse_engine_eval(comment):
    """The engine's own score in centipawns, or None."""
    match = ENGINE_EVAL.match(comment.strip())
    if not match:
        return None

    raw = match.group(1)

    if "M" in raw.upper():
        sign = -1 if raw.startswith("-") else 1
        return sign * CLAMP_CP

    return int(round(float(raw) * 100))


#-##############################  ENGINE  ################################-#

class Engine:
    def __init__(self, path):
        self.p = subprocess.Popen(
            [path], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            text=True, bufsize=1)
        self._send("uci")
        while "uciok" not in self.p.stdout.readline():
            pass

    def _send(self, line):
        self.p.stdin.write(line + "\n")
        self.p.stdin.flush()

    def new_game(self):
        """Clears the hash, so a game's numbers do not depend on which game the
        worker happened to analyse before it."""
        self._send("ucinewgame")
        self._send("isready")
        while "readyok" not in self.p.stdout.readline():
            pass

    def evaluate(self, fen, limit):
        """`limit` is a UCI go argument, "nodes 1000000" or "depth 20"."""
        self._send("position fen " + fen)
        self._send("go " + limit)

        # Only the last scoring line is ever used, so keep the string and parse
        # it once rather than splitting every line on the way past. A search
        # prints about 30 lines, so this is tidiness rather than a measured
        # speed-up; it was not the cause of anything.
        last, best = "", "-"
        while True:
            line = self.p.stdout.readline()
            if not line:
                break
            if line[0] == "i":
                if " score " in line and " pv " in line:
                    last = line
            elif line.startswith("bestmove"):
                best = line.split()[1]
                break

        if not last:
            return 0, best, False

        parts = last.split()
        kind = parts[parts.index("score") + 1]
        value = int(parts[parts.index("score") + 2])
        mate = kind == "mate"
        score = value if kind == "cp" else (
            MATE - abs(value) if value > 0 else -(MATE - abs(value)))

        return score, best, mate

    def close(self):
        self._send("quit")
        self.p.wait(timeout=10)


#-##############################  ANALYSIS  ##############################-#

def convert(converter, sans):
    """SAN list to the converter's ply/san/lan/fen/phase rows."""
    done = subprocess.run(
        [converter], input=" ".join(sans), capture_output=True, text=True)

    if done.returncode != 0:
        return None, done.stderr.strip()

    rows = []
    for line in done.stdout.splitlines():
        ply, san, lan, fen, phase = line.split("\t")
        rows.append((int(ply), san, lan, fen, int(phase)))

    return rows, None


def clamp(score):
    return max(-CLAMP_CP, min(CLAMP_CP, score))


def analyse_game(engine, game_index, headers, moves, rows, target, limit):
    """Every target-player move of one game, as raw output records."""
    white = headers.get("White", "")
    black = headers.get("Black", "")

    if target == white:
        target_parity = 0
    elif target == black:
        target_parity = 1
    else:
        return [], f"game {game_index}: {target} played neither side"

    # Book plies are not the engine's choices. The first position that has to
    # be scored is the one before the engine's first own move, and scoring the
    # move after it needs one more position.
    first = None
    for ply, (_, comment) in enumerate(moves):
        if comment.strip() == "book":
            continue
        if ply % 2 == target_parity:
            first = ply
            break

    if first is None:
        return [], f"game {game_index}: no non-book move by {target}"

    engine.new_game()

    scores, mates = {}, {}
    for index in range(first, len(rows)):
        score, _, mate = engine.evaluate(rows[index][3], limit)
        scores[index] = score
        mates[index] = mate

    records = []
    for ply in range(first, len(moves)):
        if ply % 2 != target_parity:
            continue
        if moves[ply][1].strip() == "book":
            continue
        if ply + 1 not in scores:
            continue

        cost = clamp(scores[ply]) + clamp(scores[ply + 1])
        own = parse_engine_eval(moves[ply][1])

        records.append({
            "game": game_index,
            "ply": ply,
            "phase": rows[ply][4],
            "cost": cost,
            "ref": clamp(scores[ply]),
            "own": "" if own is None else own,
            "mate": int(mates[ply] or mates[ply + 1]),
            "san": moves[ply][0],
            "fen": rows[ply][3],
        })

    return records, None


# One engine per worker process, opened once and reused for every game that
# process is handed. Module level because ProcessPoolExecutor's initializer has
# nowhere else to put it.
_ENGINE = None


def _open_engine(path):
    global _ENGINE
    _ENGINE = Engine(path)


def _worker(work):
    (index, headers, moves, rows), target, limit = work
    return analyse_game(_ENGINE, index, headers, moves, rows, target, limit)


RAW_FIELDS = ["game", "ply", "phase", "cost", "ref", "own", "mate", "san",
              "fen"]


def write_raw(path, records):
    with open(path, "w") as handle:
        handle.write("\t".join(RAW_FIELDS) + "\n")
        for record in records:
            handle.write("\t".join(str(record[f]) for f in RAW_FIELDS) + "\n")


def read_raw(path):
    records = []
    with open(path) as handle:
        header = handle.readline().rstrip("\n").split("\t")
        for line in handle:
            values = line.rstrip("\n").split("\t")
            record = dict(zip(header, values))
            for key in ("game", "ply", "phase", "cost", "ref", "mate"):
                record[key] = int(record[key])
            record["own"] = int(record["own"]) if record["own"] != "" else None
            records.append(record)

    return records


#-###############################  REPORT  ###############################-#

def report(records, meta, worst_count):
    if not records:
        print("no moves analysed")
        return

    total_cost = sum(max(0, r["cost"]) for r in records)
    games = len({r["game"] for r in records})

    print()
    for line in meta:
        print(line)

    print()
    print(f"moves by the profiled engine: {len(records)} over {games} games")
    print(f"total centipawns given away:  {total_cost}")
    print(f"scores clamped to +/-{CLAMP_CP} cp before costing; "
          f"{sum(r['mate'] for r in records)} moves touched a mate score")
    print()

    band_names = [name for _, name in PHASE_BANDS]
    cost_names = [name for _, _, name in COST_BANDS]

    #-- by phase ---------------------------------------------------------
    print("centipawns lost by game phase")
    print(f"{'phase':>17} {'moves':>7} {'cp lost':>10} {'cp/move':>8} "
          f"{'share':>7}")

    for name in band_names:
        rows = [r for r in records if phase_band(r["phase"]) == name]
        if not rows:
            continue
        lost = sum(max(0, r["cost"]) for r in rows)
        share = 100.0 * lost / total_cost if total_cost else 0.0
        print(f"{name:>17} {len(rows):>7} {lost:>10} "
              f"{lost / len(rows):>8.1f} {share:>6.1f}%")

    #-- by error size ----------------------------------------------------
    print()
    print("centipawns lost by error size")
    print(f"{'error':>17} {'moves':>7} {'cp lost':>10} {'cp/move':>8} "
          f"{'share':>7}")

    for name in cost_names:
        rows = [r for r in records if cost_band(max(0, r["cost"])) == name]
        if not rows:
            continue
        lost = sum(max(0, r["cost"]) for r in rows)
        share = 100.0 * lost / total_cost if total_cost else 0.0
        print(f"{name:>17} {len(rows):>7} {lost:>10} "
              f"{lost / len(rows):>8.1f} {share:>6.1f}%")

    #-- crossed ----------------------------------------------------------
    print()
    print("centipawns lost, phase against error size")
    print(f"{'phase':>17}" + "".join(f"{n:>12}" for n in cost_names))

    for name in band_names:
        rows = [r for r in records if phase_band(r["phase"]) == name]
        if not rows:
            continue
        cells = []
        for cost_name in cost_names:
            lost = sum(max(0, r["cost"]) for r in rows
                       if cost_band(max(0, r["cost"])) == cost_name)
            cells.append(f"{lost:>12}")
        print(f"{name:>17}" + "".join(cells))

    #-- evaluation bias --------------------------------------------------
    # The engine's own score against the reference's, for the same position.
    # Positive means the engine thought it was doing better than it was.
    biased = [r for r in records if r["own"] is not None and not r["mate"]]

    if biased:
        print()
        print("evaluation bias, engine score minus reference score, "
              "mate scores excluded")
        print(f"{'phase':>17} {'moves':>7} {'mean':>8} {'median':>8} "
              f"{'p90':>8}")

        for name in band_names:
            rows = [r for r in biased if phase_band(r["phase"]) == name]
            if not rows:
                continue
            deltas = sorted(r["own"] - r["ref"] for r in rows)
            mean = sum(deltas) / len(deltas)
            median = deltas[len(deltas) // 2]
            p90 = deltas[min(len(deltas) - 1, int(0.9 * len(deltas)))]
            print(f"{name:>17} {len(rows):>7} {mean:>8.1f} {median:>8} "
                  f"{p90:>8}")

    #-- worst moves ------------------------------------------------------
    print()
    print(f"the {worst_count} most expensive moves")
    print(f"{'game':>5} {'ply':>4} {'phase':>6} {'cost':>6} {'san':>7}  fen")

    for record in sorted(records, key=lambda r: -r["cost"])[:worst_count]:
        print(f"{record['game']:>5} {record['ply']:>4} {record['phase']:>6} "
              f"{record['cost']:>6} {record['san']:>7}  {record['fen']}")


#-################################  MAIN  ################################-#

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pgn", nargs="?")
    ap.add_argument("--player", default="achesso",
                    help="the White/Black tag value whose moves are profiled")
    ap.add_argument("--engine", default="stockfish")
    ap.add_argument("--nodes", type=int, default=1000000,
                    help="search limit per position, in nodes. Node limits "
                         "are used rather than a depth because fixed depth "
                         "has no bounded cost: over a sample of 12 positions "
                         "depth 18 ran a median of 0.71 s and a maximum of "
                         "925.90 s, while 1000000 nodes ran 1.63 s mean and "
                         "2.11 s worst, reaching a median depth of 21")
    ap.add_argument("--depth", type=int, default=0,
                    help="use a fixed depth instead of --nodes. Budget with "
                         "care, see --nodes")
    ap.add_argument("--workers", type=int, default=4)
    ap.add_argument("--converter", default="build/tools/pgn_to_positions")
    ap.add_argument("--max-games", type=int, default=0,
                    help="analyse at most this many games")
    ap.add_argument("--raw-out", default="")
    ap.add_argument("--from-raw", default="",
                    help="re-bucket an earlier run, no engine needed")
    ap.add_argument("--worst", type=int, default=15)
    args = ap.parse_args()

    # A depth, if one was asked for explicitly; nodes otherwise.
    search_limit = (f"depth {args.depth}" if args.depth
                    else f"nodes {args.nodes}")

    if args.from_raw:
        records = read_raw(args.from_raw)
        report(records, [f"re-bucketed from {args.from_raw}"], args.worst)
        return 0

    if not args.pgn:
        ap.error("a pgn is required unless --from-raw is given")

    if not os.access(args.converter, os.X_OK):
        print(f"no converter at {args.converter}", file=sys.stderr)
        return 1

    with open(args.pgn) as handle:
        games = split_games(handle.read())

    if args.max_games:
        games = games[:args.max_games]

    prepared, skipped = [], []
    for index, (headers, movetext) in enumerate(games):
        moves = parse_moves(movetext)
        if not moves:
            skipped.append(f"game {index}: no moves")
            continue

        rows, error = convert(args.converter, [san for san, _ in moves])
        if error:
            skipped.append(f"game {index}: {error}")
            continue

        prepared.append((index, headers, moves, rows))

    print(f"{len(prepared)} games parsed, {len(skipped)} skipped",
          file=sys.stderr)
    for line in skipped[:10]:
        print("  " + line, file=sys.stderr)

    all_records, errors = [], []
    done = 0

    # Processes rather than threads. Each worker owns a separate engine process
    # and the work is embarrassingly parallel, so there is nothing to share and
    # no reason to be in one interpreter.
    with ProcessPoolExecutor(
            max_workers=args.workers, initializer=_open_engine,
            initargs=(args.engine,)) as pool:
        work = [(item, args.player, search_limit) for item in prepared]

        for records, error in pool.map(_worker, work):
            done += 1
            print(f"\ranalysed {done}/{len(prepared)} games",
                  end="", file=sys.stderr)
            all_records.extend(records)
            if error:
                errors.append(error)

    print(file=sys.stderr)
    for line in errors[:10]:
        print("  " + line, file=sys.stderr)

    all_records.sort(key=lambda r: (r["game"], r["ply"]))

    if args.raw_out:
        write_raw(args.raw_out, all_records)
        print(f"raw per-move records written to {args.raw_out}",
              file=sys.stderr)

    meta = [
        f"pgn         {args.pgn}",
        f"profiled    {args.player}",
        f"reference   {args.engine} at {search_limit}",
        f"games       {len(prepared)} parsed, {len(skipped)} skipped, "
        f"{len(errors)} failed",
    ]

    report(all_records, meta, args.worst)

    return 0


if __name__ == "__main__":
    sys.exit(main())
