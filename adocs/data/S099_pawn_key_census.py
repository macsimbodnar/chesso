#!/usr/bin/env python3
"""S099's census of the pawn correction table's reach (DEC-270).

Section 4 of the step file sizes the table per side to move "at the next
power of two above the 95th percentile" of the distinct pawn keys a search
visits per position, over the 300 positions of the S021 stratified pick at
`go depth 12`. This drives the tune build's census (src/search.cpp
`search_pawn_key_census_flush`, on only when CHESSO_PAWN_KEY_CENSUS names a
file): one `ucinewgame` before each position, so each line of the census
file is one cold search, then one more at the end to flush the last.

The positions are S020's timing workload's selection from
adocs/data/S018_raw.tsv -- four per game phase at offsets 0, 1 and 2 of a
quarter-stride walk, 300 in all -- the same 300 the timings use.

A count per side, because the table is indexed by side to move and then by
key: a side's slots fill with that side's keys alone. The size is read off
the larger side's count, position by position.

Node counts, not timings: the machine's load changes nothing it prints.

    python3 adocs/data/S099_pawn_key_census.py build-tune/src/chesso [depth]
"""
import collections
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def positions():
    by_phase = collections.defaultdict(list)
    with open(os.path.join(ROOT, "adocs", "data", "S018_raw.tsv")) as f:
        header = f.readline().rstrip("\n").split("\t")
        pa, fa = header.index("phase"), header.index("fen")
        for line in f:
            x = line.rstrip("\n").split("\t")
            by_phase[int(x[pa])].append(x[fa])
    fens = []
    for off in (0, 1, 2):
        for ph in sorted(by_phase):
            rows = by_phase[ph]
            if len(rows) < 4:
                continue
            step = len(rows) // 4
            fens += [rows[(i * step + off) % len(rows)] for i in range(4)]
    return fens


def percentile(sorted_values, p):
    """Nearest rank, the plain definition: the smallest value with at least
    p percent of the values at or below it."""
    k = max(1, -(-len(sorted_values) * p // 100))
    return sorted_values[k - 1]


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    engine = sys.argv[1]
    depth = int(sys.argv[2]) if len(sys.argv) > 2 else 12
    fens = positions()

    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "census.txt")
        env = dict(os.environ, CHESSO_PAWN_KEY_CENSUS=path)
        p = subprocess.Popen([engine], stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, text=True, bufsize=1,
                             env=env)

        def send(s):
            p.stdin.write(s + "\n")
            p.stdin.flush()

        def wait_for(prefix):
            while True:
                line = p.stdout.readline()
                if not line:
                    sys.exit("engine died")
                if line.startswith(prefix):
                    return line

        send("uci")
        wait_for("uciok")
        nodes_total = 0
        for fen in fens:
            send("ucinewgame")
            send("isready")
            wait_for("readyok")
            send("position fen " + fen)
            send(f"go depth {depth}")
            nodes = 0
            while True:
                line = p.stdout.readline()
                if not line:
                    sys.exit("engine died")
                if line.startswith("info") and " nodes " in line:
                    nodes = int(line.split(" nodes ")[1].split()[0])
                if line.startswith("bestmove"):
                    break
            nodes_total += nodes
        send("ucinewgame")
        send("isready")
        wait_for("readyok")
        send("quit")
        p.wait()

        with open(path) as f:
            rows = [tuple(int(v) for v in line.split()) for line in f if line.strip()]

    if len(rows) != len(fens):
        sys.exit(f"census has {len(rows)} lines for {len(fens)} positions")

    white = sorted(r[0] for r in rows)
    black = sorted(r[1] for r in rows)
    larger = sorted(max(r) for r in rows)

    print(f"S099 pawn key census: {len(fens)} positions, go depth {depth}, "
          f"{nodes_total} nodes, engine {engine}")
    print(f"{'per position':<22}{'p50':>8}{'p90':>8}{'p95':>8}{'p99':>8}{'max':>8}")
    for name, values in (("white to move", white), ("black to move", black),
                         ("larger side", larger)):
        print(f"{name:<22}" + "".join(f"{percentile(values, q):>8}" for q in (50, 90, 95, 99))
              + f"{values[-1]:>8}")
    p95 = percentile(larger, 95)
    size = 1
    while size <= p95:
        size *= 2
    print(f"size: the next power of two above the larger side's p95 ({p95}) is {size} slots a side")


if __name__ == "__main__":
    main()
