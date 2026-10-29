#!/usr/bin/env python3
"""S121 phase 2, DEC-327: where case B's short mate lines are cut, read by tool.

    python3 adocs/data/S121_case_b.py replay ENGINE NODES STOP > OUT
    python3 adocs/data/S121_case_b.py trace TRACED NODES STOP SHORT_FROM LOG [KEYS] > OUT
    python3 adocs/data/S121_case_b.py read LOG REPLAY

`replay`: case B of adocs/data/S170_cases.tsv as adocs/data/S170_replay.py
replays it -- one process, `uci`, `setoption name Hash value 16`,
`ucinewgame`, then for each ply from the row's start (20) to STOP, stride 1,
`position fen <fen> moves <the first i moves>` and `go nodes NODES`, read to
`bestmove` -- and every `info` line printed, tagged with its ply. STOP cuts
the game short: a later ply cannot change an earlier search.

`trace`: the same commands through a build made by
adocs/data/S136_trace_patch.py (S295's patch at S296's loop return), with
S295_DEPTH=0 and S295_DIAG a FIFO this program reads. The patch logs every
node of every search; this keeps the `S` and `E` lines (one aspiration search
starting and ending), every node at ply 0 (the roots), every node and
quiescence node whose path is a prefix of one of the short lines' PVs in
SHORT_FROM (an S170_replay `--verbose` output, its `SHORT` lines), and, with
KEYS (comma-separated, the log's `h=` hex), every node of those board keys,
from any search -- which is what shows an entry carried from an earlier
move's search. LOG gets those lines,
each prefixed with the ply of the search it came from: the k-th `S ... depth
1 ` line opens the search of ply 20 + k - 1.

`read`: for each `SHORT` line of the replay, the last aspiration search of
its iteration and, along its PV, the deepest node of that search the PV
reaches, with the return it took (the patch's rule: `tt` with the entry it
read, `loop` with the stored type, ...); and, for a `tt` return, every node
in an earlier ply's search with the same board key, so an entry written by an
earlier move's search shows as such. The engine's own output only: nothing
here asks a chess question.

Standard library only.
"""

import os
import re
import subprocess
import sys
import tempfile
import threading

CASES = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                     "S170_cases.tsv")
START = 20


def case_b():
    for line in open(CASES):
        if line.startswith("B_mate6_shallow\t"):
            fields = line.rstrip("\n").split("\t")
            return fields[1], fields[2].split()
    sys.exit("no B_mate6_shallow row in " + CASES)


def drive(engine, nodes, stop, env=None):
    fen, moves = case_b()
    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1,
                            env=env)

    def send(text):
        proc.stdin.write(text + "\n")
        proc.stdin.flush()

    def until(token):
        out = []
        while True:
            line = proc.stdout.readline()
            if line == "":
                raise RuntimeError("engine closed its output")
            line = line.strip()
            out.append(line)
            if line.startswith(token):
                return out

    send("uci")
    until("uciok")
    send("setoption name Hash value 16")
    send("ucinewgame")
    lines = []
    for i in range(START, stop + 1):
        position = "position fen " + fen
        if moves[:i]:
            position += " moves " + " ".join(moves[:i])
        send(position)
        send("go nodes %d" % nodes)
        for line in until("bestmove"):
            lines.append("ply %d %s" % (i, line))
    send("quit")
    proc.wait(timeout=60)
    return lines


def short_pvs(path):
    out = []
    for line in open(path):
        m = re.match(r"SHORT \S+ ply (\d+) depth (\d+) mate (-?\d+) pv (\d+) "
                     r"plies, (\d+) needed: (.*)$", line.rstrip("\n"))
        if m:
            out.append(dict(ply=int(m[1]), depth=int(m[2]), mate=int(m[3]),
                            length=int(m[4]), needed=int(m[5]),
                            pv=m[6].split()))
    return out


def trace(args):
    traced, nodes, stop, short_from, log = args[:5]
    # Optional: board keys (the log's h= hex) whose every node is kept, in any
    # search, so the entry a cut read can be found where it was written.
    keys = set(args[5].split(",")) if len(args) > 5 else set()
    keep = set()
    for s in short_pvs(short_from):
        for j in range(1, len(s["pv"]) + 1):
            keep.add(",".join(s["pv"][:j]))

    work = tempfile.mkdtemp()
    fifo = os.path.join(work, "diag")
    os.mkfifo(fifo)
    kept = []

    def reader():
        search = START - 1
        with open(fifo) as handle:
            for line in handle:
                if line.startswith("S ") and " depth 1 " in line:
                    search += 1
                if line.startswith(("S ", "E ")):
                    kept.append("%d %s" % (search, line))
                    continue
                if not line.startswith(("N ", "Q ")):
                    continue
                words = line.split()
                key = line.rsplit("h=", 1)[-1].strip()
                if key in keys:
                    kept.append("%d %s" % (search, line))
                    continue
                # N id parent ply d<depth> [a,b] pv cut ex=<m> <path> -> ...
                # Q id parent ply q<qply> [a,b] <path> -> ...
                path = words[9] if words[0] == "N" else words[6]
                if (words[0] == "N" and words[3] == "0") or path in keep:
                    kept.append("%d %s" % (search, line))

    thread = threading.Thread(target=reader)
    thread.start()
    env = dict(os.environ, S295_DIAG=fifo, S295_DEPTH="0")
    lines = drive(traced, int(nodes), int(stop), env)
    thread.join()
    os.remove(fifo)
    os.rmdir(work)
    with open(log, "w") as handle:
        handle.writelines(kept)
    print("\n".join(lines))
    return 0


def plies_to_deliver(mate_in):
    return 2 * mate_in - 1 if mate_in > 0 else -2 * mate_in


def info_short_lines(replay):
    """The short mate lines of a `replay`/`trace` output, each with the
    iteration that printed it: a line repeating the depth of the line before
    it is the aborted next iteration's (iterative_deepening_search() prints
    an aborted iteration beside the last completed one's depth)."""
    out = []
    last = {}
    for line in open(replay):
        m = re.match(r"ply (\d+) info score (mate|cp) (-?\d+) .*?depth (\d+) .*?"
                     r"pv ?(.*)$", line.rstrip("\n"))
        if not m:
            continue
        ply, kind, value, depth = int(m[1]), m[2], int(m[3]), int(m[4])
        pv = m[5].split()
        iteration = depth + 1 if last.get(ply) == depth else depth
        last[ply] = depth
        if kind == "mate" and len(pv) < plies_to_deliver(value):
            out.append(dict(ply=ply, depth=depth, iteration=iteration,
                            mate=value, pv=pv,
                            needed=plies_to_deliver(value)))
    return out


def field(rest, n):
    words = rest.split()
    return words[n] if len(words) > n else ""


def path_of(rest):
    return field(rest, 9) if rest.startswith("N ") else field(rest, 6)


def rule_of(rest):
    """The return the node took, its score dropped: `tt ...`, `loop ...`."""
    if " -> " not in rest:
        return ""
    after = rest.split(" -> ", 1)[1].split(" ", 1)
    return after[1] if len(after) > 1 else ""


def read(args):
    log, replay = args
    entries = []
    for line in open(log):
        ply, rest = line.rstrip("\n").split(" ", 1)
        entries.append((int(ply), rest))

    for s in info_short_lines(replay):
        ply, iteration, pv = s["ply"], s["iteration"], s["pv"]
        print("SHORT ply %d depth %d%s mate %d, pv %d of %d plies: %s"
              % (ply, s["depth"],
                 " (the aborted iteration %d)" % iteration
                 if iteration != s["depth"] else "",
                 s["mate"], len(pv), s["needed"], " ".join(pv)))
        lo = None
        for k, (p, rest) in enumerate(entries):
            if p == ply and re.match(r"S \d+ depth %d " % iteration, rest):
                lo = k
        if lo is None:
            print("  no search at depth %d in ply %d's log" % (iteration, ply))
            continue
        hi = lo + 1
        while hi < len(entries) and entries[hi][0] == ply and \
                not entries[hi][1].startswith("S "):
            hi += 1
        ends = [rest for p, rest in entries[lo:hi] if rest.startswith("E ")]
        print("  its last aspiration search: %s; %s"
              % (entries[lo][1], ends[-1] if ends else "no E line"))
        deepest = None
        for j in range(len(pv), 0, -1):
            path = ",".join(pv[:j])
            hits = [rest for p, rest in entries[lo:hi]
                    if rest.startswith("N ") and path_of(rest) == path]
            if hits:
                deepest = (j, path, hits[-1])
                break
        if deepest is None:
            print("  no node of the PV logged in that search")
            continue
        j, path, node = deepest
        print("  the PV's deepest searched node, ply %d of %d: %s"
              % (j, len(pv), node[node.index(" d"):]))
        reads = [node]
        if rule_of(node).startswith("qs"):
            quiet = [rest for p, rest in entries[lo:hi]
                     if rest.startswith("Q ") and path_of(rest) == path]
            for q in quiet[-2:]:
                print("    its quiescence: " + q[q.index(" q"):])
            reads = quiet[-2:]
        for r in reads:
            if not rule_of(r).startswith("tt"):
                continue
            key = re.search(r"h=([0-9a-f]+)", r)[1]
            kind = re.search(r" t=(\d)", rule_of(r))
            index = next(k for k, (p, rest) in enumerate(entries) if rest == r)
            # A node stores at its return, its type in the loop rule; the
            # entry read is the latest store of that key and that type.
            # Its score is in the mate band, so the store's is too, on the
            # same side once the ply is taken out (the trace prints the
            # returned score, the entry's is normalised by ply).
            read_score = int(re.search(r" s=(-?\d+)", rule_of(r))[1])

            def same_band(rest):
                value = int(rest.split(" -> ", 1)[1].split()[0])
                return (abs(value) >= 48000 and abs(read_score) >= 48000 and
                        (value > 0) == (read_score > 0))

            stores = [(p, rest) for p, rest in entries[:index]
                      if "h=" + key in rest and rule_of(rest).startswith("loop")
                      and (kind is None or
                           " type=%s " % kind[1] in rule_of(rest) + " ")
                      and same_band(rest)]
            if stores:
                p, rest = stores[-1]
                which = [e for q, e in entries[:entries.index((p, rest))]
                         if q == p and e.startswith("S ")]
                print("    the entry it read, the last store of its key and type: ply %d's "
                      "search, %s: %s" % (p, which[-1] if which else "?",
                                          rest[rest.index(" d"):]))
            else:
                print("    no store of key %s logged before it" % key)
    return 0


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    stage, args = sys.argv[1], sys.argv[2:]
    if stage == "replay" and len(args) == 3:
        print("\n".join(drive(args[0], int(args[1]), int(args[2]))))
        return 0
    if stage == "trace" and len(args) in (5, 6):
        return trace(args)
    if stage == "read" and len(args) == 2:
        return read(args)
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
