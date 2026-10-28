#!/usr/bin/env python3
"""S136 phase 2b: how one search of an S136_trace_patch.py log meets a line.

    python3 adocs/data/S136_trace_line.py LOG SEARCH MOVE,MOVE,... [--kids]

LOG is an $S295_DIAG file (S295_trace_patch.py's docstring has its format),
SEARCH the aspiration search's number from its `S <n>` line, and the moves a
line from the root -- the parent tree's mating PV, as `info ... pv` printed
it. For each prefix of the line it prints every node of that search whose
path is exactly the prefix (a node is logged at its return, so a path can
appear more than once: a re-search, a verification, a null move's child),
with the node's window, depth, flags, value and the rule it returned by, and
under each node the moves it searched (M: order, child depth, reduction,
score) and skipped (P: the prune rule's number), the next move of the line
marked `<-`. `--kids` also prints the N/Q line of every child of those nodes.
Reads the engine's own log; nothing here judges a position.
"""

import sys


def main():
    if len(sys.argv) < 4:
        raise SystemExit(__doc__)
    log, search, line = sys.argv[1], int(sys.argv[2]), sys.argv[3].split(",")
    kids = "--kids" in sys.argv[4:]

    nodes, moves, children = {}, {}, {}
    inside = False
    for raw in open(log):
        t = raw.split()
        if not t:
            continue
        if t[0] == "S":
            inside = int(t[1]) == search
            continue
        if t[0] == "E":
            if inside:
                break
            continue
        if not inside:
            continue
        if t[0] in ("N", "Q"):
            node_id, parent = int(t[1]), int(t[2])
            path = t[9] if t[0] == "N" else t[6]
            nodes[node_id] = (path, raw.rstrip("\n"))
            children.setdefault(parent, []).append(node_id)
        elif t[0] in ("M", "P"):
            moves.setdefault(int(t[1]), []).append(raw.rstrip("\n"))

    for k in range(len(line) + 1):
        prefix = ",".join(line[:k]) if k else "-"
        nxt = line[k] if k < len(line) else None
        hits = [i for i, (p, _) in nodes.items() if p == prefix]
        print("== ply %d, path %s, next %s: %d node(s)"
              % (k, prefix, nxt or "-", len(hits)))
        for i in sorted(hits):
            print("  " + nodes[i][1])
            for m in moves.get(i, []):
                mark = "  <-" if nxt and m.split()[2] == nxt else ""
                print("      " + m + mark)
            if kids:
                for c in children.get(i, []):
                    print("      child " + nodes[c][1])
    return 0


if __name__ == "__main__":
    sys.exit(main())
