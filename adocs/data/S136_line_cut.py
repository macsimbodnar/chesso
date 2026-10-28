#!/usr/bin/env python3
"""S136 phase 2b: where a search's last aspiration pass leaves a line.

    python3 adocs/data/S136_line_cut.py LOG MOVE,MOVE,...

LOG is an $S295_DIAG file logged for one root depth (S295_DEPTH set), the
moves a line from the root -- the parent tree's mating PV. Takes the log's
last aspiration search and walks the line: at each ply, the nodes on the line
and the rule each returned by, compressed to `rule xN` with the depths seen,
and how the node one ply up handled the line's move (searched at what child
depth and reduction, or skipped by which prune rule; data_structures.hpp
`prune_rule_t`: 1 futility, 2 history, 3 SEE, 4 late move, 5 SEE capture).
It stops at the first ply no node of the search reaches. The rule names are
S295_trace_patch.py's (`null`, `rfp`, `tt`, `loop`, `qs`, `multicut`, ...).
Reads the engine's own log; nothing here judges a position.
"""

import collections
import sys


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    log, line = sys.argv[1], sys.argv[2].split(",")

    last = None
    for raw in open(log):
        if raw.startswith("S "):
            last = raw.split()[1]

    nodes, moves = [], collections.defaultdict(list)
    inside = False
    for raw in open(log):
        t = raw.split()
        if not t:
            continue
        if t[0] == "S":
            inside = t[1] == last
            continue
        if t[0] == "E":
            if inside:
                break
            continue
        if not inside:
            continue
        if t[0] == "N":
            arrow = t.index("->")
            nodes.append((t[9], int(t[1]), t[4], t[6], t[7], int(t[arrow + 1]),
                          t[arrow + 2]))
        elif t[0] == "Q":
            arrow = t.index("->")
            nodes.append((t[6], int(t[1]), "q", "-", "-", int(t[arrow + 1]),
                           "qs-node"))
        elif t[0] in ("M", "P"):
            moves[int(t[1])].append(t)

    print("search %s, line %s" % (last, ",".join(line)))
    for k in range(len(line) + 1):
        prefix = ",".join(line[:k]) if k else "-"
        here = [n for n in nodes if n[0] == prefix]
        if not here:
            print("  ply %d: no node reaches %s" % (k, prefix))
            break
        rules = collections.Counter(
            "%s %s %s" % (n[6], n[3], n[4]) for n in here)
        depths = sorted({n[2] for n in here}, key=lambda d: (len(d), d))
        values = sorted({n[5] for n in here})
        nxt = line[k] if k < len(line) else None
        handled = []
        if nxt:
            for n in here:
                for m in moves.get(n[1], []):
                    if m[2] != nxt:
                        continue
                    if m[0] == "P":
                        handled.append("skipped %s" % m[3])
                    else:
                        handled.append("searched %s %s %s %s"
                                       % (m[3], m[4], m[5], m[6]))
        print("  ply %d %s: %s; depths %s; values %s"
              % (k, prefix.split(",")[-1] if k else "root",
                 ", ".join("%s x%d" % kv for kv in sorted(rules.items())),
                 " ".join(depths), " ".join(map(str, values[:3] + values[-2:]
                                                if len(values) > 5 else values))))
        if handled:
            print("    %s: %s" % (nxt, "; ".join(handled[:6])
                                   + (" ..." if len(handled) > 6 else "")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
