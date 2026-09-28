#!/usr/bin/env python3
"""S113. The ProbCut margin seed, derived by the paper's own method over
chesso's own positions (DEC-134 form (b)).

Jiang and Buro, "First Experimental Results of ProbCut Applied to Chess", ACG
10, 2003 (https://skatgame.net/mburo/ps/chessmpc.pdf): a shallow search result
v' predicts the deep result v through `v = a*v' + b + e`, e normal with
deviation sigma; the cut is taken when the shallow search clears a bar
`t * sigma` above beta, with t = 1.0 (the paper's Figure 2, `#define T 1.0`,
form (a)). This script measures a, b and sigma for chesso at the depth pair
the step seeds -- d' = PROBCUT_MIN_DEPTH - PROBCUT_DEPTH_OFFSET, d =
PROBCUT_MIN_DEPTH, 4 and 8 -- and prints round(t * sigma), the margin seed.

Positions: the 300-position stratified pick of adocs/data/S021_aspiration_sweep.py
-- 4 per game_phase() value from adocs/data/S018_raw.tsv, at the three offsets
0, 1 and 2. The pick is re-implemented here rather than imported because that
script runs its sweep at import.

Each search is a fresh `ucinewgame` then `go depth N`; v and v' are the
`score cp` of the last `info` line carrying a score. A position is dropped
when either score is a mate score or when |v'| > 3 * PAWN (PAWN is 94 in
src/eval_tables.hpp), the paper's own two exclusions ("We only used v' data
points in the range [-300, 300]", its scale, read here as three of chesso's
pawns).

    ~/.venv/chess/bin/python adocs/data/S113_probcut_fit.py ENGINE [SHALLOW DEEP [OFFSETS]]

ENGINE is any build: at depth 8 the root searches its children at depth 7, so
no node of either search reaches a ProbCut block gated at depth 8 below the
root, and the parent and the candidate give the same numbers. The run records
which binary it used on its first line.
"""
import collections
import statistics
import subprocess
import sys

engine_path = sys.argv[1]
SHALLOW = int(sys.argv[2]) if len(sys.argv) > 2 else 4
DEEP = int(sys.argv[3]) if len(sys.argv) > 3 else 8
# How many offsets of the stratified pick, 100 positions each. Three is the
# step file's 300; the first run left 180 survivors, under the step's floor of
# 200, and the step's own remedy is to widen the set -- six offsets, 600.
N_OFFSETS = int(sys.argv[4]) if len(sys.argv) > 4 else 3

RAW = "adocs/data/S018_raw.tsv"
PER_PHASE = 4
OFFSETS = tuple(range(N_OFFSETS))
PAWN = 94
T = 1.0


def positions(offset):
    by_phase = collections.defaultdict(list)
    with open(RAW) as f:
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
        self.send("setoption name Hash value 16")
        self.send("isready")
        while "readyok" not in self.p.stdout.readline():
            pass

    def send(self, s):
        self.p.stdin.write(s + "\n")
        self.p.stdin.flush()

    def score(self, fen, depth):
        """(kind, value) of the last scored info line: kind 'cp' or 'mate'."""
        self.send("ucinewgame")
        self.send("position fen " + fen)
        self.send(f"go depth {depth}")
        last = None
        while True:
            line = self.p.stdout.readline()
            if not line:
                sys.exit("engine died")
            t = line.split()
            if t and t[0] == "info" and "score" in t:
                i = t.index("score")
                last = (t[i + 1], int(t[i + 2]))
            if line.startswith("bestmove"):
                if last is None:
                    sys.exit("no score for " + fen)
                return last


fens = []
for offset in OFFSETS:
    fens += positions(offset)

# A phase with fewer rows than the offsets wraps and picks a row twice; a
# position counted twice would weigh twice in the fit.
fens = list(dict.fromkeys(fens))

print(f"# engine {engine_path}, pair ({SHALLOW}, {DEEP}), {len(fens)} "
      f"positions, offsets {OFFSETS}", flush=True)
print("fen\tv_shallow\tv_deep\tkept", flush=True)

engine = Engine(engine_path)
xs, ys = [], []
dropped_mate, dropped_range = 0, 0

for fen in fens:
    ks, vs = engine.score(fen, SHALLOW)
    kd, vd = engine.score(fen, DEEP)
    if ks != "cp" or kd != "cp":
        dropped_mate += 1
        kept = "mate"
    elif abs(vs) > 3 * PAWN:
        dropped_range += 1
        kept = "range"
    else:
        xs.append(vs)
        ys.append(vd)
        kept = "yes"
    print(f"{fen}\t{ks} {vs}\t{kd} {vd}\t{kept}", flush=True)

engine.send("quit")

n = len(xs)
mx, my = statistics.fmean(xs), statistics.fmean(ys)
sxx = sum((x - mx) ** 2 for x in xs)
sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
a = sxy / sxx
b = my - a * mx
residuals = [y - (a * x + b) for x, y in zip(xs, ys)]
# The deviation of e in the paper's model: residuals of the fitted line, with
# the two fitted parameters taken off the degrees of freedom.
sigma = (sum(r * r for r in residuals) / (n - 2)) ** 0.5
r = sxy / (sxx * sum((y - my) ** 2 for y in ys)) ** 0.5

print(f"# surviving {n}, dropped mate {dropped_mate}, dropped |v'| > "
      f"{3 * PAWN}: {dropped_range}")
print(f"# a {a:.4f}  b {b:.2f}  sigma {sigma:.2f}  r {r:.4f}")
print(f"# margin seed round(t * sigma) at t = {T}: {round(T * sigma)}")
if n < 200:
    print("# FEWER THAN 200 SURVIVING POSITIONS: a finding, not a seed")
print("FIT-DONE")
