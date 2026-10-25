#!/usr/bin/env python3
"""S135's speed cost: the timing workload and the reading of the pairs
adocs/data/S135_cost.sh takes. Standard library only.

    S135_cost.py workload ENGINE DEPTH
    S135_cost.py read DIR

WORKLOAD is S270's and S273's pos11: S020's 300 stratified positions out of
adocs/data/S018_raw.tsv -- four a phase at offsets 0, 1 and 2 -- searched to
DEPTH through one engine process, each to its bestmove. With PERF_COUNTERS and
PERF_OUT in the environment the engine, and only the engine, runs under
tools/perf_counters. Prints the position count and the node total, so a run
that searched another tree says so.

READ takes DIR/<comparison>/{bench16,pos11}/{base,cand}_<k>.cnt and
DIR/<comparison>/wall/pair_<k>.json for every comparison, A/A first, and
prints per comparison and instrument the geometric mean ratio cand/base with
its 95 % CI from the paired t, and DEC-266's reading against the A/A's floor:
the floor is the A/A's largest CI bound per workload and instrument; a cost
counts when its cycles CI on both workloads and its wall CI lie wholly beyond
the floor the same way and its instructions move the same way, and reads zero
otherwise.
"""

import collections
import glob
import json
import math
import os
import re
import statistics
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(
    __file__))))

T975 = {1: 12.706, 2: 4.303, 3: 3.182, 4: 2.776, 5: 2.571, 6: 2.447,
        7: 2.365, 8: 2.306, 9: 2.262, 10: 2.228, 11: 2.201, 12: 2.179,
        13: 2.160, 14: 2.145, 15: 2.131, 16: 2.120, 17: 2.110, 18: 2.101,
        19: 2.093, 20: 2.086, 21: 2.080, 22: 2.074, 23: 2.069, 24: 2.064}


def workload(engine, depth):
    by_phase = collections.defaultdict(list)
    with open(os.path.join(ROOT, "adocs", "data", "S018_raw.tsv")) as handle:
        head = handle.readline().rstrip("\n").split("\t")
        pa, fa = head.index("phase"), head.index("fen")
        for line in handle:
            x = line.rstrip("\n").split("\t")
            by_phase[int(x[pa])].append(x[fa])
    fens = []
    for offset in (0, 1, 2):
        for phase in sorted(by_phase):
            rows = by_phase[phase]
            if len(rows) < 4:
                continue
            step = len(rows) // 4
            fens += [rows[(i * step + offset) % len(rows)] for i in range(4)]
    command = [engine]
    if os.environ.get("PERF_COUNTERS"):
        command = [os.environ["PERF_COUNTERS"], "-o", os.environ["PERF_OUT"],
                   engine]
    engine_process = subprocess.Popen(command, stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE, text=True,
                                      bufsize=1)

    def send(text):
        engine_process.stdin.write(text + "\n")
        engine_process.stdin.flush()

    send("uci")
    while "uciok" not in engine_process.stdout.readline():
        pass
    total = 0
    for fen in fens:
        send("position fen " + fen)
        send(f"go depth {depth}")
        nodes = 0
        while True:
            line = engine_process.stdout.readline()
            if not line:
                sys.exit("engine died")
            if line.startswith("info") and " nodes " in line:
                nodes = int(line.split(" nodes ")[1].split()[0])
            if line.startswith("bestmove"):
                break
        total += nodes
    send("quit")
    engine_process.wait()
    print(len(fens), total)
    return 0


def ratio_ci(base, cand):
    logs = [math.log(c / b) for b, c in zip(base, cand)]
    n = len(logs)
    m = statistics.mean(logs)
    se = statistics.stdev(logs) / math.sqrt(n)
    t = T975.get(n - 1, 2.0)
    return math.exp(m) - 1, math.exp(m - t * se) - 1, math.exp(m + t * se) - 1


def counters(directory):
    """{instrument: (change, lo, hi)} over the pairs of one workload."""
    def one(path):
        words = open(path).read().split()
        kv = dict(zip(words[1::2], words[2::2]))
        if kv["scaled"] != "0":
            sys.exit(f"{path}: a counter was multiplexed")
        return {k: float(kv[k]) for k in ("instructions", "cycles")}
    ks = sorted(int(re.search(r"base_(\d+)\.cnt$", f).group(1))
                for f in glob.glob(os.path.join(directory, "base_*.cnt")))
    if len(ks) < 2:
        sys.exit(f"{directory}: fewer than two pairs")
    out = {}
    for name in ("instructions", "cycles"):
        base = [one(os.path.join(directory, f"base_{k}.cnt"))[name] for k in ks]
        cand = [one(os.path.join(directory, f"cand_{k}.cnt"))[name] for k in ks]
        out[name] = ratio_ci(base, cand)
    return out, len(ks)


def wall(directory):
    pairs = sorted(glob.glob(os.path.join(directory, "pair_*.json")),
                   key=lambda p: int(re.search(r"(\d+)\.json$", p).group(1)))
    base, cand = [], []
    for path in pairs:
        results = {r["command"]: r["mean"]
                   for r in json.load(open(path))["results"]}
        base.append(results["base"])
        cand.append(results["cand"])
    if len(base) < 2:
        sys.exit(f"{directory}: fewer than two pairs")
    return ratio_ci(base, cand), len(base)


def read(directory):
    order = sorted(d for d in os.listdir(directory)
                   if os.path.isdir(os.path.join(directory, d, "bench16")))
    if "aa" not in order:
        sys.exit("no A/A comparison: the floor cannot be set")
    order = ["aa"] + [d for d in order if d != "aa"]
    readings = {}
    for name in order:
        d = os.path.join(directory, name)
        b16, n1 = counters(os.path.join(d, "bench16"))
        p11, n2 = counters(os.path.join(d, "pos11"))
        w, n3 = wall(os.path.join(d, "wall"))
        readings[name] = {"bench16": b16, "pos11": p11, "wall": w}
        print(f"{name}: {n1} / {n2} counter pairs, {n3} wall pairs")
        for workload_name in ("bench16", "pos11"):
            for inst in ("instructions", "cycles"):
                c, lo, hi = readings[name][workload_name][inst]
                print(f"  {workload_name:<8}{inst:<13}{c * 100:+.3f} %  CI "
                      f"{lo * 100:+.3f} .. {hi * 100:+.3f} %")
        c, lo, hi = w
        print(f"  pos11   wall         {c * 100:+.3f} %  CI {lo * 100:+.3f} .. "
              f"{hi * 100:+.3f} %  (cand time over base, + is slower)")

    aa = readings["aa"]

    def floor(workload_name, inst):
        value = aa[workload_name] if inst == "wall" else aa[workload_name][inst]
        _, lo, hi = value
        return max(abs(lo), abs(hi))

    floors = {("bench16", "cycles"): floor("bench16", "cycles"),
              ("pos11", "cycles"): floor("pos11", "cycles"),
              ("wall", "wall"): floor("wall", "wall")}
    print("A/A floor: bench16 cycles %.3f %%, pos11 cycles %.3f %%, wall %.3f %%"
          % tuple(v * 100 for v in floors.values()))

    for name in order[1:]:
        r = readings[name]
        bounds = [r["bench16"]["cycles"], r["pos11"]["cycles"], r["wall"]]
        fl = list(floors.values())
        slower = all(lo > f for (_, lo, _), f in zip(bounds, fl))
        faster = all(hi < -f for (_, _, hi), f in zip(bounds, fl))
        instr = [r["bench16"]["instructions"][0], r["pos11"]["instructions"][0]]
        same_way = (slower and all(x > 0 for x in instr)) or \
                   (faster and all(x < 0 for x in instr))
        if (slower or faster) and same_way:
            cost = statistics.mean([b[0] for b in bounds])
            verdict = (f"COST {cost * 100:+.2f} % (cycles and wall beyond the "
                       "floor, instructions the same way)")
        elif slower or faster:
            verdict = ("beyond the floor but instructions do not agree: owes "
                       "DEC-266 (4)'s two perturbations before it is credited; "
                       "zero until then")
        else:
            verdict = "zero (inside the A/A's floor)"
        print(f"{name}: {verdict}")
    return 0


def main(argv):
    if len(argv) == 3 and argv[0] == "workload":
        return workload(argv[1], int(argv[2]))
    if len(argv) == 2 and argv[0] == "read":
        return read(argv[1])
    print(__doc__, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
