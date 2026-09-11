#!/usr/bin/env python3
"""Killer census driver, S216, 2026-09-11.

Replaces `adocs/data/S159_census_run.py` (2026-09-09), which stays beside this
file unedited -- `adocs/data/` is append-only, a changed reader lands as a new
file naming the old one (the convention `adocs/data/S198_pairs.py` states in its
own comment).

**Why it is replaced.** S208 refuses a placement with more than 16 pieces of a
colour at the load boundary, and DEC-177 made `KILLER_POS` legal by deleting its
h3 pawn. Row `promo-mess` of `S159_census_positions.txt` is the **pre-S208**
constant, 17 white pieces, and a post-S208 engine now refuses it. S159's reader
scrapes only lines that start `info` and contain ` nodes `, so the refusal --
`info string refused [position fen] <fen>, <reason>` -- carries no ` nodes `
field and is skipped; `set_position` has already done `game = previous`
(`src/chesso.cpp`), so the `go` that follows searches **the row above** and its
node count and best move are printed under the refused row's name. The census
then reports eleven rows over ten positions and says nothing about it. That is
the class DEC-142 exists to stop, so the instrument is what gets fixed, not the
transcript.

**What changed, and only this.** One synchronisation point per row: `position
fen` is followed by `isready` and read to `readyok`, so a refusal is seen
*before* `go` is written. A refused row is never searched, never scraped,
printed and written as `REFUSED`, and the script exits 1 once every row has been
attempted -- attempted, so one bad row does not hide the next one.

**Comparability with S159's census.** On an input where every row loads, the
command stream is S159's plus one `isready` per row; `command_isready` replies
`readyok` and touches no search state, so the node counts, best moves and the
`S159CENSUS` line are what S159's reader would print, and the stdout and file
formats are byte-identical. The `REFUSED_ROWS` trailer appears only when
something was refused, so a clean census file has S159's exact shape. A refused
row's node column holds `REFUSED` rather than a number, so a reader that parses
it raises instead of quietly averaging a duplicate.

    adocs/data/S216_census_run.py <engine> \
        adocs/data/S216_census_positions.txt <out prefix>

`S216_census_positions.txt` is the input from DEC-186 on: S159's eleven rows with
the refused one replaced by the legal `KILLER_POS` under the name
`promo-mess-s208`. S159's own positions file is still a valid argument -- it is
what the recorded census was taken on -- but a current build refuses its sixth
row, so a run over it is red by construction.

Prints the instrumented build's last S159CENSUS line, plus per-position nodes
and best move so the tree can be compared between two builds. Each `go` is
waited on until `bestmove` before the next command is written -- writing `quit`
behind an un-awaited `go` kills the search before it looks at a node, which is
TOOLCHAIN.md's "the one way to ask it that lies" and it silently reported 0
killer stores over the whole set when S159 first hit it.
"""
import subprocess, sys, threading

engine, positions, out = sys.argv[1], sys.argv[2], sys.argv[3]

REFUSAL = "info string refused [position fen]"

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
refused_rows = 0
for depth, name, fen in rows:
    send("position fen %s" % fen)
    # The refusal is asynchronous with respect to `position`, so the row needs a
    # reply of its own to be read against; without it the only evidence arrives
    # interleaved with the search of the board the refusal left standing.
    send("isready")
    refusal = ""
    for l in wait_for("readyok"):
        if l.startswith(REFUSAL):
            refusal = l.rstrip("\n")[len(REFUSAL):].strip()
    if refusal:
        refused_rows += 1
        per_pos.append((name, depth, None, "-"))
        print("  %-14s d%-3d %12s  REFUSED  %s" % (name, depth, "-", refusal))
        continue

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
        f.write("%s\t%d\t%s\t%s\n" % (name, depth,
                                      "REFUSED" if nodes is None else nodes,
                                      best))
    f.write("TOTAL\t\t%d\t\n" % total_nodes)
    if census:
        f.write(census[-1])
    if refused_rows:
        f.write("REFUSED_ROWS\t\t%d\t\n" % refused_rows)
print(census[-1].rstrip() if census else "  (no census line: uninstrumented build)")

if refused_rows:
    # The totals above are over the rows that loaded, so they are not the set
    # the file names: a census with a refused row is a failed run, not a short
    # one.
    print("  %d of %d rows refused: this census measures a different set than "
          "its input names" % (refused_rows, len(rows)))
    raise SystemExit(1)
