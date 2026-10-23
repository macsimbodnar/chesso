#!/usr/bin/env python3
"""S291. Reads S289_trace.py's stderr on stdin and keeps, for each visit of a
traced node N at ply P, its entry line, its own move and pruned lines (indent
2P + 2) and its exit line; with --all, every traced node's entry and exit as
well. S289's trace prints every visit of a traced key at any ply, so the same
key reached by transposition elsewhere in the tree would otherwise interleave.

    S289_TRACE=... <driver> 2>&1 | python3 adocs/data/S291_trace_filter.py N P
"""
import re
import sys

node, ply = int(sys.argv[1]), int(sys.argv[2])
show_all = "--all" in sys.argv
depth = 0
inside = []
for line in sys.stdin:
    m = re.match(r"^(\s*)([<>]) N(\d+) it\d+ ply(\d+) ", line)
    if m:
        n, p = int(m.group(3)), int(m.group(4))
        if m.group(2) == ">":
            inside.append((n, p))
            if (n, p) == (node, ply) or show_all:
                print(line, end="")
        else:
            if inside:
                inside.pop()
            if (n, p) == (node, ply) or show_all:
                print(line, end="")
        continue
    if inside and inside[-1] == (node, ply):
        if re.match(r"^\s{%d}(move|pruned|rfp:) " % (2 * ply + 2), line):
            print(line, end="")
    elif line.startswith("ITER") or line.startswith("d"):
        print(line, end="")
