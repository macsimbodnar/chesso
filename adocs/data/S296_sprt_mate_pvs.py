#!/usr/bin/env python3
"""S296's SPRT: every `Incomplete mating PV` warning the run printed, read.

S296's subject is a reported mate shorter than the position's, so the seven
warnings its SPRT console carries (five the candidate's, two the reference's)
are checked here and not assumed residue. `adocs/data/S296_sprt_mate_pvs.txt`
is the reading, with every command that made it.

    P=~/.venv/chess/bin/python
    $P -I adocs/data/S296_sprt_mate_pvs.py warnings CONSOLE PGN
    $P -I adocs/data/S296_sprt_mate_pvs.py oracle CONSOLE PGN
    $P -I adocs/data/S296_sprt_mate_pvs.py cold CONSOLE PGN --engines NAME=BIN...
    $P -I adocs/data/S296_sprt_mate_pvs.py replay CONSOLE PGN --engines NAME=BIN...
    $P -I adocs/data/S296_sprt_mate_pvs.py rates LOG...

`warnings`: one row per warning -- the console line, the engine, the score,
depth, nodes and line it printed, the game it came from (the PGN game whose
FEN header and moves match the console's, numbered as fastchess numbers it:
2 * Round - 1 with the candidate White, 2 * Round with it Black, the order
that reproduces S293's and S135's recorded quarters), the searched position
put on a board by python-chess from the console's FEN and move list, the
plies the claimed distance needs against the plies printed, and whether the
printed line is legal from the position.

`oracle`: each distinct searched position at Stockfish 5000000 nodes through
python-chess, DEC-319 (3), `adocs/data/S296_oracle.py`'s form (`--nodes`,
`--threads` and `--hash` for a deeper reading where that one reads no mate,
labelled as deeper and not DEC-319's setting) -- the safe
form of TOOLCHAIN.md, reading until the search answers, `game=object()` so
python-chess sends `ucinewgame` first -- twice: from the bare FEN, as
S296_oracle.py asks, and with the game's moves, so a repetition the game
already holds is seen. The printed distance is read against the oracle's by
S296_compare.py's `classify`: `short` (a smaller distance than the oracle's,
same side), `ok`, `longer`, `sign` (the other side mates) or `nomate`.

`cold`: each warning's position on each named binary, one fresh process per
search, `Hash 16`, `ucinewgame`, the console's `position fen ... moves ...`
and `go depth N` at the warning's depth. Every mate line is checked for
length and for ending in checkmate at exactly its distance (python-chess).

`replay`: the game replayed through one process of the warning side's binary,
`Hash 16`, one `ucinewgame`: at each of that side's turns up to and including
the warning's, `go nodes n` with `n` the node count the PGN comment of the
move it played records -- the table a game gives that engine (stride 2,
DEC-151) at the budgets the game gave it. Every mate line of the final search
is printed and checked as `cold` checks it, and a short line at any earlier
turn is reported too.

`rates`: per log, `Incomplete mating PV` lines and distinct searches (engine,
FEN, move list) per side, and per 10000 games of the PGN count the log's
foot prints (`=== N games in ...`), or the last `Games:` line where it has
none.

Nothing here is a chess judgement of this project's: positions come from
python-chess, distances from Stockfish, lines from the engines.
"""

import argparse
import collections
import re
import subprocess
import sys

import chess
import chess.engine
import chess.pgn

STOCKFISH = "/usr/games/stockfish"
NODES = 5000000
HASH = 16
WARNING = "Warning; Incomplete mating PV - from "


def needed_plies(mate):
    """Plies a mate at this distance takes: the side delivering it moves last."""
    return 2 * mate - 1 if mate > 0 else -2 * mate


def parse_info(line):
    """(mate, depth, nodes, pv) of an info line with a mate score, else None."""
    t = line.split()
    if len(t) < 4 or t[0] != "info" or "score" not in t:
        return None
    i = t.index("score")
    if t[i + 1] != "mate":
        return None
    mate = int(t[i + 2])
    depth = int(t[t.index("depth") + 1]) if "depth" in t else 0
    nodes = int(t[t.index("nodes") + 1]) if "nodes" in t else 0
    pv = t[t.index("pv") + 1:] if "pv" in t else []
    return mate, depth, nodes, pv


def read_warnings(console):
    """Every warning: (line number, engine, info, fen, moves)."""
    out = []
    lines = open(console, encoding="utf-8", errors="replace", newline="").read().split("\n")
    for k, line in enumerate(lines):
        if not line.startswith(WARNING):
            continue
        engine = line[len(WARNING):].strip()
        info = lines[k + 1].split("Info; ", 1)[1].strip()
        pos = lines[k + 2].split("Position; ", 1)[1].strip()
        moves = lines[k + 3].split("Moves; ", 1)[1].split()
        fen = pos[len("fen "):] if pos.startswith("fen ") else chess.STARTING_FEN
        out.append((k + 1, engine, info, fen, moves))
    return out


def game_number(headers, cand):
    r = int(headers["Round"])
    return 2 * r - 1 if headers["White"].startswith(cand) else 2 * r


def find_games(pgn, wanted):
    """{(fen, moves tuple): (headers, mainline moves, node counts)}."""
    by_fen = collections.defaultdict(list)
    for fen, moves in wanted:
        by_fen[fen].append(tuple(moves))
    found = {}
    with open(pgn, encoding="utf-8", errors="replace") as handle:
        while True:
            offset = handle.tell()
            headers = chess.pgn.read_headers(handle)
            if headers is None:
                break
            fen = headers.get("FEN")
            if fen not in by_fen:
                continue
            here = handle.tell()
            handle.seek(offset)
            game = chess.pgn.read_game(handle)
            handle.seek(here)
            ucis, nodes = [], []
            for node in game.mainline():
                ucis.append(node.move.uci())
                m = re.search(r"n=(\d+)", node.comment or "")
                nodes.append(int(m.group(1)) if m else None)
            for moves in by_fen[fen]:
                if tuple(ucis[:len(moves)]) == moves and len(ucis) > len(moves):
                    found[(fen, moves)] = (dict(game.headers), ucis, nodes)
    return found


def line_check(board, mate, pv):
    """'legal', or where the line goes wrong; and 'reaches' when it mates."""
    b = board.copy()
    for i, uci in enumerate(pv):
        move = chess.Move.from_uci(uci)
        if move not in b.legal_moves:
            return "illegal at ply %d (%s)" % (i + 1, uci)
        b.push(move)
    need = needed_plies(mate)
    if len(pv) < need:
        return "short: %d of %d plies, legal" % (len(pv), need)
    if len(pv) == need and b.is_checkmate():
        return "complete: %d plies, checkmate" % len(pv)
    return "unreached: %d plies of %d, not checkmate there" % (len(pv), need)


def context(args):
    warns = read_warnings(args.console)
    games = find_games(args.pgn, [(w[3], w[4]) for w in warns])
    rows = []
    for ln, engine, info, fen, moves in warns:
        headers, ucis, nodes = games[(fen, tuple(moves))]
        board = chess.Board(fen)
        for uci in moves:
            board.push_uci(uci)
        side = "White" if board.turn == chess.WHITE else "Black"
        mate, depth, n, pv = parse_info(info)
        rows.append(dict(line=ln, engine=engine, info=info, fen=fen,
                         moves=moves, headers=headers, ucis=ucis,
                         nodes=nodes, board=board, side=side, mate=mate,
                         depth=depth, n=n, pv=pv,
                         game=game_number(headers, args.cand)))
    return rows


def warnings(args):
    for r in context(args):
        h = r["headers"]
        print("console line %d, %s: %s" % (r["line"], r["engine"], r["info"]))
        print("  game %d (Round %s), %s vs %s, %s {%s}, %d plies; the warning "
              "at ply %d, %s to move, the PGN's %s is %s" % (
                  r["game"], h["Round"], h["White"], h["Black"], h["Result"],
                  h.get("Termination"), len(r["ucis"]), len(r["moves"]),
                  r["side"], r["side"], h[r["side"]]))
        print("  searched position: %s" % r["board"].fen())
        print("  move played there: %s (n=%s); claimed mate %d, needs %d plies; "
              "line %s" % (r["ucis"][len(r["moves"])], r["nodes"][len(r["moves"])],
                           r["mate"], needed_plies(r["mate"]),
                           line_check(r["board"], r["mate"], r["pv"])))
    return 0


def classify(oracle_kind, oracle_value, mate):
    """S296_compare.py's classify, on one oracle reading."""
    if oracle_kind != "mate":
        return "nomate"
    if (mate > 0) != (oracle_value > 0):
        return "sign"
    if abs(mate) < abs(oracle_value):
        return "short"
    if abs(mate) > abs(oracle_value):
        return "longer"
    return "ok"


def oracle(args):
    rows = context(args)
    seen = {}
    with chess.engine.SimpleEngine.popen_uci(STOCKFISH) as sf:
        options = {}
        if args.threads:
            options["Threads"] = args.threads
        if args.hash:
            options["Hash"] = args.hash
        sf.configure(options)
        print("oracle: %s, go nodes %d, python-chess analyse, ucinewgame per "
              "call, %s" % (sf.id.get("name", "?"), args.nodes,
                            ", ".join("%s %s" % kv for kv in options.items())
                            or "default Hash and Threads"))
        for r in rows:
            key = (r["fen"], tuple(r["moves"]))
            if key not in seen:
                got = {}
                for mode, board in (("fen", chess.Board(r["board"].fen())),
                                    ("history", r["board"])):
                    info = sf.analyse(board, chess.engine.Limit(nodes=args.nodes),
                                      game=object())
                    s = info["score"].relative
                    kind, value = ("mate", s.mate()) if s.is_mate() else ("cp", s.score())
                    pv = " ".join(m.uci() for m in info.get("pv", [])[:6])
                    got[mode] = (kind, value, info.get("depth"), info.get("nodes"), pv)
                seen[key] = got
            got = seen[key]
            print("console line %d, %s, game %d, %s to move: engine mate %d at "
                  "depth %d" % (r["line"], r["engine"], r["game"], r["side"],
                                r["mate"], r["depth"]))
            for mode in ("fen", "history"):
                kind, value, depth, nodes, pv = got[mode]
                print("  oracle (%s): %s %s, depth %s, nodes %s, pv %s -> %s" % (
                    mode, kind, value, depth, nodes, pv,
                    classify(kind, value, r["mate"])))
    return 0


class Engine:
    def __init__(self, path):
        self.proc = subprocess.Popen([path], stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, text=True, bufsize=1)
        self.send("uci")
        self.read_until("uciok")
        self.send("setoption name Hash value %d" % HASH)
        self.send("setoption name Threads value 1")
        self.send("ucinewgame")
        self.send("isready")
        self.read_until("readyok")

    def send(self, line):
        self.proc.stdin.write(line + "\n")
        self.proc.stdin.flush()

    def read_until(self, token):
        # Reads to the answer; never writes `quit` to get one (TOOLCHAIN.md).
        out = []
        while True:
            line = self.proc.stdout.readline()
            if line == "":
                raise RuntimeError("engine closed its output")
            out.append(line.strip())
            if line.startswith(token):
                return out

    def search(self, fen, moves, spec):
        self.send("position fen %s%s" % (fen, " moves " + " ".join(moves) if moves else ""))
        self.send("go " + spec)
        return self.read_until("bestmove")

    def close(self):
        self.send("quit")
        self.proc.wait(timeout=10)


def engines_map(specs):
    out = {}
    for s in specs:
        name, path = s.split("=", 1)
        out[name] = path
    return out


def report_lines(board, lines, prefix="  "):
    short = 0
    for line in lines:
        p = parse_info(line)
        if p is None:
            continue
        mate, depth, nodes, pv = p
        verdict = line_check(board, mate, pv)
        short += not verdict.startswith("complete")
        print("%sdepth %2d nodes %8d mate %3d pv %2d plies: %s" % (
            prefix, depth, nodes, mate, len(pv), verdict))
    return short


def cold(args):
    rows = context(args)
    bins = engines_map(args.engines)
    for r in rows:
        print("console line %d, %s, game %d: printed mate %d at depth %d, "
              "%d plies" % (r["line"], r["engine"], r["game"], r["mate"],
                            r["depth"], len(r["pv"])))
        for name, path in bins.items():
            e = Engine(path)
            out = e.search(r["fen"], r["moves"], "depth %d" % r["depth"])
            e.close()
            print("  %s, cold, go depth %d:" % (name, r["depth"]))
            bad = report_lines(r["board"], out, "    ")
            last = [l for l in out if l.startswith("info") and " score " in l][-1:]
            print("    last: %s" % (last[0] if last else "(none)"))
            print("    %s; %s" % ("no mate line" if not any(
                parse_info(l) for l in out) else "%d incomplete" % bad, out[-1]))
    return 0


def replay(args):
    rows = context(args)
    bins = engines_map(args.engines)
    done = set()
    for r in rows:
        key = (r["fen"], tuple(r["moves"]), r["engine"])
        if key in done:
            continue
        done.add(key)
        path = bins[r["engine"]]
        target = len(r["moves"])
        e = Engine(path)
        print("game %d, %s (%s), replayed to ply %d at the PGN's node counts:" % (
            r["game"], r["engine"], path, target))
        board = chess.Board(r["fen"])
        earlier = 0
        for i, uci in enumerate(r["ucis"][:target + 1]):
            if i % 2 == target % 2:
                n = r["nodes"][i]
                if n is None:
                    print("  ply %d: no node count in the PGN, not searched" % i)
                else:
                    out = e.search(r["fen"], r["ucis"][:i], "nodes %d" % n)
                    if i == target:
                        print("  the warning's ply %d, go nodes %d:" % (i, n))
                        report_lines(board, out, "    ")
                        last = [l for l in out if l.startswith("info") and " score " in l][-1:]
                        print("    last: %s" % (last[0] if last else "(none)"))
                        print("    %s" % out[-1])
                    else:
                        for line in out:
                            p = parse_info(line)
                            if p and not line_check(board, p[0], p[3]).startswith("complete"):
                                earlier += 1
                                print("  ply %d, go nodes %d: %s" % (i, n, line))
                                print("    %s" % line_check(board, p[0], p[3]))
            board.push_uci(uci)
        print("  incomplete mate lines at the earlier turns: %d" % earlier)
        e.close()
    return 0


def rates(args):
    for path in args.logs:
        text = open(path, encoding="utf-8", errors="replace", newline="").read().split("\n")
        lines = collections.Counter()
        searches = collections.defaultdict(set)
        games = None
        for k, line in enumerate(text):
            if line.startswith(WARNING):
                side = line[len(WARNING):].strip().split("-")[0]
                lines[side] += 1
                searches[side].add((text[k + 2], text[k + 3]))
            m = re.match(r"=== (\d+) games in ", line)
            if m:
                games = int(m.group(1))
        if games is None:
            g = [l for l in text if l.startswith("Games: ")]
            games = int(g[-1].split()[1].rstrip(",")) if g else 0
        cells = []
        for side in ("cand", "ref"):
            cells.append("%s %d lines, %d searches, %.2f and %.2f per 10000" % (
                side, lines[side], len(searches[side]),
                1e4 * lines[side] / games, 1e4 * len(searches[side]) / games))
        print("%s: %d games; %s" % (path, games, "; ".join(cells)))
    return 0


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="stage", required=True)
    for stage in ("warnings", "oracle", "cold", "replay"):
        p = sub.add_parser(stage)
        p.add_argument("console")
        p.add_argument("pgn")
        p.add_argument("--cand", default="cand-")
        if stage == "oracle":
            p.add_argument("--nodes", type=int, default=NODES,
                           help="DEC-319 (3)'s 5000000 by default")
            p.add_argument("--threads", type=int,
                           help="Stockfish's Threads; its default (1) if unset")
            p.add_argument("--hash", type=int,
                           help="Stockfish's Hash in MB; its default if unset")
        if stage in ("cold", "replay"):
            p.add_argument("--engines", nargs="+", required=True,
                           help="NAME=BINARY, NAME the console's engine name")
    p = sub.add_parser("rates")
    p.add_argument("logs", nargs="+")
    args = ap.parse_args()
    return {"warnings": warnings, "oracle": oracle, "cold": cold,
            "replay": replay, "rates": rates}[args.stage](args)


if __name__ == "__main__":
    sys.exit(main())
