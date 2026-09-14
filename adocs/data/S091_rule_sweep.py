#!/usr/bin/env python3
"""What S091's two rules each do to the three search_bench positions.

Characterisation, not a fit. A node count at a fixed depth is not Elo (DEC-019)
and nothing in this step's defaults was chosen from the numbers below; what
chose them is in `src/search_params.hpp` beside each parameter. This exists so
that "neither rule is inert and neither dominates" is a measurement.

Driven through the **tune build**, where the two parameters are settable, and
never through the release build the SPRT measures (S073). It waits for
`bestmove` before sending the next line: the race TOOLCHAIN.md's oracle section
describes is chesso's too.

    cmake --build build-tune -j12
    python3 adocs/data/S091_rule_sweep.py > adocs/data/S091_rule_sweep.txt
"""

import subprocess
import sys

ENGINE = "./build-tune/src/chesso"

# tools/search_bench.py's three, so the numbers sit beside the ones the step
# stamp and the pre-registration already quote.
POSITIONS = [
    ("midgame", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"),
    ("kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"),
    ("tactical", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"),
]

# The cap at 0 is the capture skip's exact off value and 8 is what ships; the
# extra ply is off at 0 and ships at 1.
CASES = [
    ("both off  ", [("SeeCaptureMaxLmrDepth", 0), ("SeeLmrExtra", 0)]),
    ("skip only ", [("SeeCaptureMaxLmrDepth", 8), ("SeeLmrExtra", 0)]),
    ("extra only", [("SeeCaptureMaxLmrDepth", 0), ("SeeLmrExtra", 1)]),
    ("both on   ", [("SeeCaptureMaxLmrDepth", 8), ("SeeLmrExtra", 1)]),
]


def run(opts, depth):
    proc = subprocess.Popen([ENGINE], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        proc.stdin.write(line + "\n")
        proc.stdin.flush()

    send("uci")
    while "uciok" not in proc.stdout.readline():
        pass

    for name, value in opts:
        send(f"setoption name {name} value {value}")

    out = []

    for _, fen in POSITIONS:
        send("ucinewgame")
        send(f"position fen {fen}")
        send(f"go depth {depth}")
        nodes, best = 0, ""
        while True:
            line = proc.stdout.readline()
            if line.startswith("bestmove"):
                best = line.split()[1]
                break
            if " nodes " in line:
                nodes = int(line.split(" nodes ")[1].split()[0])
        out.append((nodes, best))

    send("quit")
    proc.wait(timeout=30)
    return out


def main():
    print("# S091 per-rule node sweep, tune build, the three")
    print("# tools/search_bench.py positions. Regenerate with")
    print("#   python3 adocs/data/S091_rule_sweep.py")
    print("# The tune build is never SPRT'd (S073); a node count is not Elo")
    print("# (DEC-019) and no default here was chosen from these numbers.")

    for depth in (9, 12):
        print(f"\ndepth {depth}: midgame / kiwipete / tactical, nodes and best move")
        for label, opts in CASES:
            rows = run(opts, depth)
            print("  " + label + "  " +
                  "  ".join(f"{n:>9} {b}" for n, b in rows))


if __name__ == "__main__":
    sys.exit(main())
