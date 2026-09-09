#!/usr/bin/env python3
"""S159 killer census driver.

One engine process, Hash 64, every position in S159_census_positions.txt
searched to its own depth. Each `go` is waited on until `bestmove` before the
next command is written -- writing `quit` behind an un-awaited `go` kills the search before
it looks at a node, which is TOOLCHAIN.md's "the one way to ask it that lies"
and it silently reported 0 killer stores over the whole set here.

    adocs/data/S159_census_run.py <engine> \
        adocs/data/S159_census_positions.txt <out prefix>

Prints the instrumented build's last S159CENSUS line, plus per-position nodes
and best move so the tree can be compared between two builds.
"""
import subprocess, sys, threading

engine, positions, out = sys.argv[1], sys.argv[2], sys.argv[3]

rows = []
for line in open(positions):
    line = line.rstrip("\n")
    if not line or line.startswith("#"):
        continue
    depth, name, fen = line.split("\t")
    rows.append((int(depth), name, fen))

p = subprocess.Popen([engine], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                     stderr=subprocess.PIPE, text=True, bufsize=1)

err_lines = []
def drain_err():
    for l in p.stderr:
        err_lines.append(l)
t = threading.Thread(target=drain_err, daemon=True); t.start()

def send(s):
    p.stdin.write(s + "\n"); p.stdin.flush()

def wait_for(token):
    while True:
        l = p.stdout.readline()
        if not l:
            raise SystemExit("engine died waiting for %s" % token)
        yield l
        if l.startswith(token):
            return

send("uci")
for _ in wait_for("uciok"): pass
send("setoption name Hash value 64")
send("ucinewgame")
send("isready")
for _ in wait_for("readyok"): pass

total_nodes = 0
per_pos = []
for depth, name, fen in rows:
    send("position fen %s" % fen)
    send("go depth %d" % depth)
    nodes, best = 0, "-"
    for l in wait_for("bestmove"):
        if l.startswith("info") and " nodes " in l:
            f = l.split()
            nodes = int(f[f.index("nodes") + 1])
        if l.startswith("bestmove"):
            best = l.split()[1]
    total_nodes += nodes
    per_pos.append((name, depth, nodes, best))
    print("  %-14s d%-3d %12d nodes  best %s" % (name, depth, nodes, best))

send("quit")
p.wait(timeout=30)
t.join(timeout=5)

print("  %-14s %5s %12d nodes" % ("TOTAL", "", total_nodes))
open(out + ".stderr", "w").writelines(err_lines)
census = [l for l in err_lines if "S159CENSUS" in l]
with open(out + ".txt", "w") as f:
    for name, depth, nodes, best in per_pos:
        f.write("%s\t%d\t%d\t%s\n" % (name, depth, nodes, best))
    f.write("TOTAL\t\t%d\t\n" % total_nodes)
    if census:
        f.write(census[-1])
print(census[-1].rstrip() if census else "  (no census line: uninstrumented build)")
