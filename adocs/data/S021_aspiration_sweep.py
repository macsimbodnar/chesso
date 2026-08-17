#!/usr/bin/env python3
"""S021. What the aspiration window schedule costs in nodes, per setting.

Node counts at a fixed depth, which is what the search bench measures and what
S033's and S068's sweeps decided from. It is not Elo and it does not claim to
be: a setting that saves nodes is the candidate an SPRT is then spent on
(DEC-019, DEC-063).

Needs the tune build, where every search parameter is a UCI spin option, so one
binary answers for every setting and no rebuild sits between two numbers. S073
is what makes that possible; before it this was one cmake per row.

    cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
    cmake --build build-tune -j12
    adocs/data/S021_aspiration_sweep.py build-tune/src/chesso 11

Positions are 32 sampled from adocs/data/S018_raw.tsv, which is 13522 positions
chesso actually reached in 210 games against sgambetto, stratified by the
engine's own game_phase() so the middlegame does not answer for the endgame.
The three tools/search_bench.py positions are three positions.

AspirationMinDepth 64 is the off row: no iteration below depth 64 gets a
window, so it is this binary searching exactly what the shipping one searches
without the feature. Every other row is compared against it.

Columns: min_depth, delta, max_delta, total nodes, nodes relative to the off
row, and the number of positions where a window failed at least once.
"""
import subprocess, sys, collections

engine = sys.argv[1]
depth = int(sys.argv[2]) if len(sys.argv) > 2 else 11

# Which of each phase's positions the stratified pick starts from. A second run
# at a different offset is a second position set, which is how a ranking is
# checked for being a property of the setting rather than of the sample.
offset = int(sys.argv[3]) if len(sys.argv) > 3 else 0

raw = "adocs/data/S018_raw.tsv"
PER_PHASE = 4

# min_depth, delta, max_delta. The off row first so everything reads against it.
SETTINGS = [(64, 25, 400)]
for min_depth in (3, 4, 5, 6):
    for delta in (12, 25, 50, 100):
        SETTINGS.append((min_depth, delta, 400))
for max_delta in (100, 200, 800, 2000):
    SETTINGS.append((4, 25, max_delta))


def positions():
    by_phase = collections.defaultdict(list)
    with open(raw) as f:
        header = f.readline().rstrip("\n").split("\t")
        phase_at, fen_at = header.index("phase"), header.index("fen")
        for line in f:
            field = line.rstrip("\n").split("\t")
            by_phase[int(field[phase_at])].append(field[fen_at])

    picked = []
    for phase in sorted(by_phase):
        rows = by_phase[phase]
        if len(rows) < PER_PHASE:
            continue
        step = len(rows) // PER_PHASE
        picked += [rows[(i * step + offset) % len(rows)]
                   for i in range(PER_PHASE)]
    return picked


class Engine:
    def __init__(self, path):
        self.p = subprocess.Popen([path], stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE, text=True, bufsize=1)
        self.send("uci")
        while "uciok" not in self.p.stdout.readline():
            pass

    def send(self, s):
        self.p.stdin.write(s + "\n")
        self.p.stdin.flush()

    def go(self, fen, depth):
        """Returns (nodes, best_move) for one fixed depth search."""
        self.send("ucinewgame")
        self.send("position fen " + fen)
        self.send(f"go depth {depth}")
        nodes = 0
        while True:
            line = self.p.stdout.readline()
            if not line:
                sys.exit("engine died")
            if line.startswith("info score"):
                t = line.split()
                nodes = int(t[t.index("nodes") + 1])
            if line.startswith("bestmove"):
                return nodes, line.split()[1]


fens = positions()
print(f"# {len(fens)} positions, depth {depth}, engine {engine}",
      file=sys.stderr)

engine = Engine(engine)
baseline_nodes, baseline_moves = None, None
print("min_depth\tdelta\tmax_delta\tnodes\trel\tmoves_changed")

for min_depth, delta, max_delta in SETTINGS:
    engine.send(f"setoption name AspirationMinDepth value {min_depth}")
    engine.send(f"setoption name AspirationDelta value {delta}")
    engine.send(f"setoption name AspirationMaxDelta value {max_delta}")

    total, moves = 0, []
    for fen in fens:
        nodes, move = engine.go(fen, depth)
        total += nodes
        moves.append(move)

    if baseline_nodes is None:
        baseline_nodes, baseline_moves = total, moves
    changed = sum(1 for a, b in zip(moves, baseline_moves) if a != b)

    print(f"{min_depth}\t{delta}\t{max_delta}\t{total}"
          f"\t{total / baseline_nodes:.4f}\t{changed}", flush=True)

engine.send("quit")
