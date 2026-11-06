#!/usr/bin/env python3
"""S124's P0, head's conversion test (DEC-350 (4), the step file's pre-registration), and the
per-start record P1's comparison pairs by McNemar's test.

    PY=~/.venv/chess/bin/python
    $PY adocs/data/S124_p0.py select --pool POOL_ROOT --heldout VIEW_QS --out STARTS.tsv
                                     [--seed 20261106] [--per-class 200] [--skip 0]
    $PY adocs/data/S124_p0.py play --starts STARTS.tsv --engine CHESSO --out RECORDS.tsv
                                   [--nodes 100000] [--workers 12] [--limit N]
    $PY adocs/data/S124_p0.py read RECORDS.tsv
    $PY adocs/data/S124_p0.py mcnemar HEAD.tsv CANDIDATE.tsv

SELECT. The starts are training-side root positions of mop-up's domain: rows of S122's raw
root pool (`.tuning/s122/corpus/pool.tsv.root`, positions the games stood in; a qs row is a
leaf and no evidence the game reached it), of games outside the fixed held-out set, whose ids
are read from the qs view's game column alone. A row's class is its material, by piece
counts and nothing else: one side's king alone against the other side's king and men, the men
sorted Q R B N, the six classes KRvK, KQvK, KRBvK, KRNvK, KBNvK and KBBvK. One start a game
and class: the game's first root row in the class, by ply. Its halfmove clock is zeroed, and
it is kept only where the Syzygy 3-4-5 tables (~/syzygy/, python-chess) read WDL 2 for the
side with men there, which at a zeroed clock is a win under the fifty-move rule by the
tables' own definition (|dtz| <= 100); a position another kept start already holds (board,
side to move, castling, en passant) is dropped. The kept games are taken in the order of one
seeded shuffle per class (random.Random(seed), the games' ids sorted first), the first
`per-class` of them after the first `skip`: 200 each, and every KBBvK game's (per-class 0 is
all). `--skip 200` with the same seed is the next 200 of the same order, P1's fresh starts.
The win is the tables' verdict, never the agent's (CHESS rule); the class is a count.

PLAY. Self-play by CHESSO, two processes a start, one for the side with men and one for the
lone king, each `Hash 16`, `Threads 1`, a fresh game (`ucinewgame`) at each start, `go nodes
NODES` a move, the whole move list sent each time so the engine sees every repetition. Under
the laws and nothing else: the game ends at checkmate, stalemate, insufficient material, the
third occurrence of a position, or the fifty-move rule, and at the bound, 50 moves (100
plies) from the start; no adjudication, no resignation. Converted is mated within 50 moves:
checkmate of the lone king at ply 100 or before. One record a start:

    id class game ply fen wdl dtz converted plies end moves

`id` is `class:game`, the key P1's pairing reads; `plies` the plies played; `end` mate,
stalemate, material, threefold, fifty or bound; `moves` the UCI moves, space separated.

READ. Per class and in all: starts, converted, the rate, plies to mate over the converted
(mean, median, max), and the other ends by kind. MCNEMAR pairs two records files by id (the
same starts, P1's fresh ones played by head and by the candidate): b the starts head converts
and the candidate does not, c the reverse, z = (c - b) / sqrt(b + c) one-sided, and per class
the converted counts, for P1's rule (kept iff z > 1.645 and no class converts fewer).
"""

import argparse
import collections
import concurrent.futures
import hashlib
import os
import random
import statistics
import sys
import time

import chess
import chess.engine
import chess.syzygy

TB = os.path.expanduser("~/syzygy")
CLASSES = ("KRvK", "KQvK", "KRBvK", "KRNvK", "KBNvK", "KBBvK")
EVERY = "KBBvK"
BOUND = 100  # plies: 50 moves
FIELDS = ("id", "class", "game", "ply", "fen", "wdl", "dtz", "converted", "plies", "end",
          "moves")


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def class_of(placement):
    """(class, side with men: chess.WHITE or chess.BLACK) of a FEN placement, or None."""
    white = [c for c in placement if c in "PNBRQ"]
    black = [c.upper() for c in placement if c in "pnbrq"]
    if bool(white) == bool(black):
        return None
    men, side = (white, chess.WHITE) if white else (black, chess.BLACK)
    if "P" in men:
        return None
    name = "K" + "".join(sorted(men, key="QRBN".index)) + "vK"
    return (name, side) if name in CLASSES else None


# ------------------------------------------------------------------ select

def select(args):
    heldout = set()
    with open(args.heldout) as handle:
        for line in handle:
            heldout.add(line.split("\t", 6)[4])
    first = {}  # (class, game) -> (ply, fen, side)
    rows = 0
    with open(args.pool) as handle:
        for line in handle:
            rows += 1
            space = line.find(" ")
            placement = line[:space]
            if sum(c.isalpha() for c in placement) > 5:
                continue
            hit = class_of(placement)
            if hit is None:
                continue
            fields = line.rstrip("\n").split("\t")
            game = fields[4]
            if game in heldout:
                continue
            ply = int(fields[5])
            key = (hit[0], int(game))
            if key not in first or ply < first[key][0]:
                first[key] = (ply, fields[0], hit[1])

    by_class = collections.defaultdict(list)
    for (name, game) in sorted(first):
        by_class[name].append(game)

    out = open(args.out, "w")
    out.write("# S124 P0 starts: pool %s sha256 %s, held-out view %s sha256 %s, seed %d, "
              "per-class %d, skip %d%s\n" % (args.pool, sha256(args.pool), args.heldout,
                                             sha256(args.heldout), args.seed, args.per_class,
                                             args.skip,
                                             ", a smoke of %d" % args.smoke if args.smoke else ""))
    out.write("\t".join(FIELDS[:7]) + "\n")
    print("select: %d root rows read, %d held-out games left out" % (rows, len(heldout)))
    with chess.syzygy.open_tablebase(TB) as tb:
        for name in CLASSES:
            games = by_class[name]
            order = list(games)
            random.Random("%d %s" % (args.seed, name)).shuffle(order)
            want = 0 if name == EVERY else args.per_class
            seen = set()
            kept = []
            not_won = duplicates = 0
            for game in order:
                ply, fen, side = first[(name, game)]
                parts = fen.split()
                parts[4] = "0"
                zeroed = " ".join(parts)
                board = chess.Board(zeroed)
                wdl = tb.probe_wdl(board)
                view = wdl if board.turn == side else -wdl
                if view != 2:
                    not_won += 1
                    continue
                position = " ".join(parts[:4])
                if position in seen:
                    duplicates += 1
                    continue
                seen.add(position)
                kept.append((game, ply, zeroed, view, tb.probe_dtz(board)))
            if args.smoke:
                want = args.smoke
            taken = kept[args.skip:] if want == 0 else kept[args.skip:args.skip + want]
            for game, ply, zeroed, view, dtz in taken:
                out.write("%s:%d\t%s\t%d\t%d\t%s\t%d\t%d\n" % (name, game, name, game, ply,
                                                              zeroed, view, dtz))
            print("  %-6s training games with a root row in it %5d; not won at a zeroed clock "
                  "%4d; a position already kept %4d; won and distinct %5d; taken %4d" % (
                      name, len(games), not_won, duplicates, len(kept), len(taken)))
            if want and len(taken) < want:
                print("  %-6s SHORT: %d of the %d asked" % (name, len(taken), want))
    out.close()
    print("starts written to %s, sha256 %s" % (args.out, sha256(args.out)))
    print("S124-P0-SELECT-OK")
    return 0


# -------------------------------------------------------------------- play

def read_starts(path):
    starts = []
    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or line.startswith("id\t"):
                continue
            fields = line.rstrip("\n").split("\t")
            starts.append(dict(zip(FIELDS[:7], fields)))
    return starts


def play_one(start, engine_path, nodes):
    board = chess.Board(start["fen"])
    hit = class_of(start["fen"].split(" ", 1)[0])
    if hit is None or hit[0] != start["class"]:
        raise ValueError("start %s is not of its class" % start["id"])
    men = hit[1]
    engines = {}
    try:
        for colour in (chess.WHITE, chess.BLACK):
            engine = chess.engine.SimpleEngine.popen_uci(engine_path)
            engine.configure({"Hash": 16, "Threads": 1})
            engines[colour] = engine
        moves = []
        end = None
        while True:
            if board.is_checkmate():
                end = "mate"
                break
            if board.is_stalemate():
                end = "stalemate"
                break
            if board.is_insufficient_material():
                end = "material"
                break
            if board.is_repetition(3):
                end = "threefold"
                break
            if board.is_fifty_moves():
                end = "fifty"
                break
            if len(moves) >= BOUND:
                end = "bound"
                break
            result = engines[board.turn].play(board, chess.engine.Limit(nodes=nodes),
                                              game=start["id"])
            board.push(result.move)
            moves.append(result.move.uci())
        converted = int(end == "mate" and board.turn != men and len(moves) <= BOUND)
        return dict(start, converted=str(converted), plies=str(len(moves)), end=end,
                    moves=" ".join(moves))
    finally:
        for engine in engines.values():
            engine.quit()


def play(args):
    starts = read_starts(args.starts)
    if args.limit:
        starts = starts[:args.limit]
    engine_sha = sha256(args.engine)
    began = time.time()
    records = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(play_one, s, args.engine, args.nodes): s["id"] for s in starts}
        for future in concurrent.futures.as_completed(futures):
            records[futures[future]] = future.result()
    with open(args.out, "w") as out:
        out.write("# S124 P0 records: starts %s sha256 %s, engine %s sha256 %s, nodes %d, "
                  "bound %d plies, %d starts, %.0f s\n" % (
                      args.starts, sha256(args.starts), args.engine, engine_sha, args.nodes,
                      BOUND, len(starts), time.time() - began))
        out.write("\t".join(FIELDS) + "\n")
        for s in starts:
            r = records[s["id"]]
            out.write("\t".join(r[f] for f in FIELDS) + "\n")
    print("play: %d starts in %.0f s, records %s" % (len(starts), time.time() - began,
                                                      args.out))
    print("S124-P0-PLAY-OK")
    return 0


# -------------------------------------------------------------------- read

def read_records(path):
    out = []
    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or line.startswith("id\t"):
                continue
            out.append(dict(zip(FIELDS, line.rstrip("\n").split("\t"))))
    return out


def read(args):
    records = read_records(args.records)
    groups = collections.OrderedDict((c, []) for c in CLASSES)
    for r in records:
        groups[r["class"]].append(r)
    print("P0 %s: class, starts, converted (mated within %d plies), rate; plies to mate over "
          "the converted, mean / median / max; the other ends" % (args.records, BOUND))
    total = converted = 0
    for name, rows in list(groups.items()) + [("all", records)]:
        if not rows:
            continue
        won = [r for r in rows if r["converted"] == "1"]
        plies = [int(r["plies"]) for r in won]
        ends = collections.Counter(r["end"] for r in rows if r["converted"] != "1")
        print("  %-6s starts %4d converted %4d (%.3f)  plies %s  others %s" % (
            name, len(rows), len(won), len(won) / len(rows),
            "%.1f / %.0f / %d" % (statistics.mean(plies), statistics.median(plies),
                                  max(plies)) if plies else "-",
            ", ".join("%s %d" % kv for kv in sorted(ends.items())) or "none"))
        if name != "all":
            total += len(rows)
            converted += len(won)
    print("P0-CONVERTS-EVERY-START" if converted == total else
          "P0-FAILS-TO-CONVERT %d of %d" % (total - converted, total))
    return 0


def mcnemar(args):
    a = {r["id"]: r for r in read_records(args.head)}
    b = {r["id"]: r for r in read_records(args.candidate)}
    if set(a) != set(b):
        print("refused: the two records do not hold the same starts")
        return 1
    worse = sum(1 for i in a if a[i]["converted"] == "1" and b[i]["converted"] != "1")
    better = sum(1 for i in a if a[i]["converted"] != "1" and b[i]["converted"] == "1")
    z = (better - worse) / (better + worse) ** 0.5 if better + worse else 0.0
    print("mcnemar: %d starts; head alone converts %d, the candidate alone %d; z %+.3f "
          "one-sided" % (len(a), worse, better, z))
    fewer = []
    for name in CLASSES:
        ids = [i for i in a if a[i]["class"] == name]
        ha = sum(a[i]["converted"] == "1" for i in ids)
        hb = sum(b[i]["converted"] == "1" for i in ids)
        print("  %-6s starts %4d head %4d candidate %4d" % (name, len(ids), ha, hb))
        if hb < ha:
            fewer.append(name)
    print("P1 %s" % ("kept" if z > 1.645 and not fewer else "not kept") +
          (": converts fewer in " + ", ".join(fewer) if fewer else ""))
    return 0


def main(argv):
    ap = argparse.ArgumentParser(description="S124's P0 conversion test.")
    sub = ap.add_subparsers(dest="command", required=True)
    c = sub.add_parser("select")
    c.add_argument("--pool", required=True)
    c.add_argument("--heldout", required=True)
    c.add_argument("--out", required=True)
    c.add_argument("--seed", type=int, default=20261106)
    c.add_argument("--per-class", type=int, default=200)
    c.add_argument("--skip", type=int, default=0)
    c.add_argument("--smoke", type=int, default=0,
                   help="a smoke's starts: this many of every class, KBBvK's too")
    c = sub.add_parser("play")
    c.add_argument("--starts", required=True)
    c.add_argument("--engine", required=True)
    c.add_argument("--out", required=True)
    c.add_argument("--nodes", type=int, default=100000)
    c.add_argument("--workers", type=int, default=12)
    c.add_argument("--limit", type=int, default=0)
    c = sub.add_parser("read")
    c.add_argument("records")
    c = sub.add_parser("mcnemar")
    c.add_argument("head")
    c.add_argument("candidate")
    args = ap.parse_args(argv)
    return {"select": select, "play": play, "read": read, "mcnemar": mcnemar}[args.command](args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
