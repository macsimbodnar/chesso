#!/usr/bin/env python3
"""S292. The driver behind adocs/data/S292_cost.txt: positions from S289's
run's own games, searched at a fixed depth by the scratch census binary
(adocs/data/S292_census_patch.py) under each $S292_GUARD mode.

    # once: the sample, by python-chess from the run's PGN (seeded)
    ~/.venv/chess/bin/python adocs/data/S292_census.py sample \
        .tuning/sprt_s289_20261022_155029/games.pgn OUT_DIR
    # the engine's own replay of every sampled position, against python-chess
    python3 adocs/data/S292_census.py check ENGINE OUT_DIR/positions.tsv
    # one table per mode: independent positions, `ucinewgame` before each
    python3 adocs/data/S292_census.py run ENGINE MODE OUT_DIR/positions.tsv DEPTH OUT.tsv
    # one table per mode: whole games replayed, the candidate's moves only,
    # one process per game and no `ucinewgame` between its moves -- the table
    # carries from move to move as it did in the match; DEPTH 0 searches each
    # move to the node count the candidate spent on it in the run (`n=`)
    python3 adocs/data/S292_census.py replay ENGINE MODE OUT_DIR/games.tsv DEPTH OUT.tsv
    # the reading
    python3 adocs/data/S292_census.py compare BASE.tsv OTHER.tsv [...]

THE SAMPLE. Every game of the PGN is read with python-chess (its `[FEN]`
header and mainline; `build/tools/pgn_to_positions` replays from the default
position only, so it cannot start from a book FEN -- S024's and S210's
reason). The population is every position where the candidate, the side
playing S289's guard (`cand-cd50178`), was to move: each is a search the
guard's engine made in the run. `random.Random(SEED)` draws N_POSITIONS of
them without replacement for `positions.tsv`, then N_GAMES game indices for
`games.tsv`. Each row carries the book FEN and the UCI moves up to the
position, so the engine is given the game's history (`position fen ... moves
...`), as fastchess gave it, and the repetition rules see what they saw then.
`check` has the engine replay each row and compares its `fen` with
python-chess's: positions come from tools, never from a board in anyone's head.

THE SEARCHES. `go depth DEPTH` at the default Hash (16), twelve processes,
each waiting for `bestmove`; before it the census binary prints one `info
string S292 ...` line, which is stored whole with the root's best move, last
reported score, depth and nodes. The engine's answers only.
"""
import json, random, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

SEED = 292
N_POSITIONS = 3000
N_GAMES = 200
JOBS = 12
CAND = "cand-"


def sample(pgn_path, out_dir):
    import chess, chess.pgn
    pool, games = [], []
    with open(pgn_path) as h:
        gi = 0
        while True:
            g = chess.pgn.read_game(h)
            if g is None:
                break
            start = g.headers.get("FEN", chess.STARTING_FEN)
            cand_white = g.headers["White"].startswith(CAND)
            board = chess.Board(start)
            ucis, cand_plies, cand_nodes = [], [], []
            for ply, node in enumerate(g.mainline()):
                if board.turn == (chess.WHITE if cand_white else chess.BLACK):
                    cand_plies.append(ply)
                    m = re.search(r"\bn=(\d+)", node.comment)
                    cand_nodes.append(int(m.group(1)) if m else 0)
                    pool.append((gi, ply))
                mv = node.move
                ucis.append(mv.uci())
                board.push(mv)
            games.append((start, ucis, cand_plies, cand_nodes))
            gi += 1
    rng = random.Random(SEED)
    picked = sorted(rng.sample(pool, N_POSITIONS))
    gpicked = sorted(rng.sample(range(len(games)), N_GAMES))
    with open(out_dir + "/positions.tsv", "w") as h:
        h.write("# S292 sample: seed %d, %d of %d candidate-to-move positions in %d games\n"
                % (SEED, N_POSITIONS, len(pool), len(games)))
        for gi, ply in picked:
            start, ucis, _, _ = games[gi]
            b = chess.Board(start)
            for u in ucis[:ply]:
                b.push_uci(u)
            h.write("%d:%d\t%s\t%s\t%s\n" % (gi, ply, start, " ".join(ucis[:ply]), b.fen()))
    with open(out_dir + "/games.tsv", "w") as h:
        h.write("# S292 sample: seed %d, %d of %d games, the candidate's plies of each\n"
                % (SEED, N_GAMES, len(games)))
        for gi in gpicked:
            start, ucis, cplies, cnodes = games[gi]
            h.write("%d\t%s\t%s\t%s\t%s\n" % (gi, start, " ".join(ucis), ",".join(map(str, cplies)),
                                              ",".join(map(str, cnodes))))
    print("SAMPLE-DONE pool %d positions, %d games" % (len(pool), len(games)))


class Engine:
    def __init__(self, path, mode):
        import os
        env = dict(os.environ, S292_GUARD=str(mode))
        self.p = subprocess.Popen([path], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  text=True, bufsize=1, env=env)
        self.send("uci")
        self.until("uciok")

    def send(self, s):
        self.p.stdin.write(s + "\n")
        self.p.stdin.flush()

    def until(self, word):
        for l in self.p.stdout:
            if l.strip() == word or l.startswith(word):
                return l
        raise SystemExit("engine died waiting for " + word)

    def position(self, start, moves):
        self.send("position fen %s%s" % (start, (" moves " + moves) if moves else ""))
        self.send("isready")
        self.until("readyok")

    def go(self, limit):
        self.send("go " + limit)
        score, nodes, reached, s292 = "none", 0, 0, {}
        for l in self.p.stdout:
            if l.startswith("info string S292"):
                for kv in l.split()[3:]:
                    k, v = kv.split("=")
                    s292[k] = int(v)
            elif l.startswith("info") and " score " in l:
                t = l.split()
                score = " ".join(t[t.index("score") + 1:t.index("score") + 3])
                nodes = int(t[t.index("nodes") + 1])
                reached = int(t[t.index("depth") + 1])
            elif l.startswith("bestmove"):
                return l.split()[1], score, reached, nodes, s292
        raise SystemExit("engine died in go")

    def quit(self):
        self.send("quit")
        self.p.wait()


def read_rows(path):
    return [l.rstrip("\n").split("\t") for l in open(path) if l.strip() and not l.startswith("#")]


def write(out, res):
    with open(out, "w") as h:
        for tag, bm, sc, d, n, c in res:
            h.write("%s\t%s\t%s\t%d\t%d\t%s\n" % (tag, bm, sc, d, n, json.dumps(c, sort_keys=True)))


def run(eng, mode, positions, depth, out):
    rows = read_rows(positions)
    chunks = [rows[i::JOBS] for i in range(JOBS)]

    def work(chunk):
        e = Engine(eng, mode)
        res = []
        for tag, start, moves, _fen in [r + [""] * (4 - len(r)) for r in chunk]:
            e.send("ucinewgame")
            e.position(start, moves)
            res.append((tag,) + e.go("depth %d" % depth))
        e.quit()
        return res

    with ThreadPoolExecutor(JOBS) as ex:
        res = sorted(sum(ex.map(work, chunks), []), key=lambda r: tuple(map(int, r[0].split(":"))))
    write(out, res)
    print("RUN-DONE %s mode %s %d rows" % (out, mode, len(res)))


def replay(eng, mode, games, depth, out):
    rows = read_rows(games)

    def work(row):
        gi, start, ucis = row[0], row[1], row[2].split()
        cplies = [int(x) for x in row[3].split(",") if x]
        cnodes = [int(x) for x in row[4].split(",") if x]
        e = Engine(eng, mode)
        e.send("ucinewgame")
        res = []
        for ply, n in zip(cplies, cnodes):
            e.position(start, " ".join(ucis[:ply]))
            # depth 0 means the node count the candidate spent on this move in
            # the run, read off fastchess's `n=` comment
            limit = ("depth %d" % depth) if depth > 0 else ("nodes %d" % max(n, 1))
            res.append(("%s:%d" % (gi, ply),) + e.go(limit))
        e.quit()
        return res

    with ThreadPoolExecutor(JOBS) as ex:
        res = sorted(sum(ex.map(work, rows), []), key=lambda r: tuple(map(int, r[0].split(":"))))
    write(out, res)
    print("REPLAY-DONE %s mode %s %d rows" % (out, mode, len(res)))


def check(eng, positions):
    rows = read_rows(positions)
    e = Engine(eng, 0)
    bad = 0
    for tag, start, moves, fen in [r + [""] * (4 - len(r)) for r in rows]:
        e.position(start, moves)
        e.send("fen")
        e.send("isready")
        got = None
        for l in e.p.stdout:
            if l.strip() == "readyok":
                break
            if l.count("/") == 7:
                got = l.strip()
        if got is None or got.split()[:4] != fen.split()[:4]:
            bad += 1
            print("MISMATCH %s engine %s python-chess %s" % (tag, got, fen))
    e.quit()
    print("CHECK-DONE %d rows, %d mismatches (board, side, castling, en passant)" % (len(rows), bad))


def load(path):
    out = {}
    for l in open(path):
        tag, bm, sc, d, n, c = l.rstrip("\n").split("\t")
        out[tag] = (bm, sc, int(d), int(n), json.loads(c))
    return out


def compare(paths):
    """Each table against the first, row by row: the node total and its ratio
    with a 95 % interval from 2000 resamples of whole games (seed 1; a game's
    rows move together where the table carries), the rows whose node count,
    best move or score moved, the moved rows' own ratio and geometric mean,
    the mean depth reached, and the census counters summed."""
    import math
    base = load(paths[0])
    games = {}
    for k in base:
        games.setdefault(k.split(":")[0], []).append(k)
    glist = list(games.values())
    for p in paths:
        t = load(p)
        assert set(t) == set(base), p
        nodes = sum(v[3] for v in t.values())
        bnodes = sum(v[3] for v in base.values())
        tot = {}
        for v in t.values():
            for k, x in v[4].items():
                tot[k] = tot.get(k, 0) + x
        bm = sum(1 for k in t if t[k][0] != base[k][0])
        sc = sum(1 for k in t if t[k][1] != base[k][1])
        moved = [k for k in t if t[k][3] != base[k][3]]
        fired_rows = sum(1 for v in t.values() if any(k.startswith("band_") for k in v[4]))
        rng = random.Random(1)
        ratios = []
        for _ in range(2000):
            pick = [glist[rng.randrange(len(glist))] for _ in glist]
            a = sum(base[k][3] for ks in pick for k in ks)
            b = sum(t[k][3] for ks in pick for k in ks)
            ratios.append(b / a)
        ratios.sort()
        mv0 = sum(base[k][3] for k in moved)
        mv1 = sum(t[k][3] for k in moved)
        lr = [math.log(t[k][3] / base[k][3]) for k in moved if t[k][3] and base[k][3]]
        depth = sum(v[2] for v in t.values()) / len(t)
        print("== %s" % p)
        print("rows %d, games %d; nodes %d, x%.6f against %s, 95%% by game [%.4f, %.4f]"
              % (len(t), len(glist), nodes, nodes / bnodes, paths[0], ratios[50], ratios[1949]))
        print("rows whose node count moved %d, best move moved %d, score moved %d"
              % (len(moved), bm, sc))
        if moved:
            print("the moved rows: nodes x%.4f, geometric mean x%.4f, %d up and %d down"
                  % (mv1 / mv0, math.exp(sum(lr) / len(lr)), sum(1 for x in lr if x > 0),
                     sum(1 for x in lr if x < 0)))
        print("mean depth reached %.4f; rows with a node the guard's condition held at %d"
              % (depth, fired_rows))
        print("counters " + " ".join("%s=%d" % kv for kv in sorted(tot.items())))


if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "sample":
        sample(sys.argv[2], sys.argv[3])
    elif cmd == "check":
        check(sys.argv[2], sys.argv[3])
    elif cmd == "run":
        run(sys.argv[2], sys.argv[3], sys.argv[4], int(sys.argv[5]), sys.argv[6])
    elif cmd == "replay":
        replay(sys.argv[2], sys.argv[3], sys.argv[4], int(sys.argv[5]), sys.argv[6])
    elif cmd == "compare":
        compare(sys.argv[2:])
    else:
        raise SystemExit(__doc__)
