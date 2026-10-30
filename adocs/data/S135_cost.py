#!/usr/bin/env python3
"""S135's speed cost: the timing workload and the reading of the pairs
adocs/data/S135_cost.sh takes. Standard library only.

    S135_cost.py workload ENGINE DEPTH
    S135_cost.py code BINDIR
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

CODE and READ check the forced builds against their masks, since
2026-10-25: run 1's 0x78 build was the 0x7f build's code to the byte, its zero
lanes still multiplied, and 0x07 computed PL_BAD_BISHOP beside its three (the
step file's section of that date). CODE runs before any timing on
BINDIR/{zero,all,old3,new4}: the four .text sections pairwise distinct, and in
evaluate_cheap() each forced build more popcnt instructions than zero's, the
parts' extra counts summing to all's exactly -- the masks are disjoint and no
popcount is shared between a column of 0x07 and one of 0x78, so a part that
computes a column outside its mask counts it twice. READ refuses before any
verdict unless on both workloads every forced build's instructions CI lies
wholly above the A/A's instruction floor and all's lies above old3's and
new4's by more than that floor: each part costs something, and all more than
either part. That is coarse -- run 1's old3, the bad bishop computed beside its
three, passes it -- and a part overlapping another is CODE's to catch, not
READ's. READ prints how far the parts' sum is from all's and does not gate
it: shared loads and register pressure move a correct build's sum off all's
by an amount no measurement here has bounded.
"""

import collections
import glob
import hashlib
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
    while True:
        line = engine_process.stdout.readline()
        if not line:
            sys.exit("engine closed its output before uciok")
        if "uciok" in line:
            break
    total = 0
    for fen in fens:
        send("position fen " + fen)
        send(f"go depth {depth}")
        nodes = 0
        while True:
            line = engine_process.stdout.readline()
            if not line:
                sys.exit("engine closed its output before bestmove")
            if line.startswith("info") and " nodes " in line:
                nodes = int(line.split(" nodes ")[1].split()[0])
            if line.startswith("bestmove"):
                break
        total += nodes
    send("quit")
    engine_process.wait()
    print(len(fens), total)
    return 0


def code(bindir):
    """The static half of the masks' check, before any timing: see the
    module's docstring. Reads BINDIR/{zero,all,old3,new4} and nothing else."""
    names = ("zero",) + FORCED
    text, popcnt = {}, {}
    for name in names:
        path = os.path.join(bindir, name)
        if not os.access(path, os.X_OK):
            print(f"refused: no binary {path}")
            return 1
        section = subprocess.run(
            ["objcopy", "-O", "binary", "--only-section=.text", path,
             "/dev/stdout"], capture_output=True, check=True).stdout
        text[name] = hashlib.sha256(section).hexdigest()[:16]
        listing = subprocess.run(
            ["objdump", "-d", "--no-show-raw-insn", "-C", path],
            capture_output=True, text=True, check=True).stdout
        body = re.search(r"^[0-9a-f]+ <evaluate_cheap\(board_t const\*\)>:\n"
                         r"(.*?)\n\n", listing, re.S | re.M)
        if body is None:
            print(f"refused: {name} has no evaluate_cheap(board_t const*) "
                  "to read")
            return 1
        popcnt[name] = len(re.findall(r"\bpopcnt\b", body.group(1)))
        print(f"code {name:<5} .text {text[name]}  evaluate_cheap popcnt "
              f"{popcnt[name]}")
    ok = True
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            if text[a] == text[b]:
                print(f"refused: {a} and {b} have the same .text, so two "
                      "masks built the same code")
                ok = False
    extra = {name: popcnt[name] - popcnt["zero"] for name in FORCED}
    for name in FORCED:
        if extra[name] <= 0:
            print(f"refused: {name} has {extra[name]:+d} popcnt against zero's, "
                  "so its force computes nothing")
            ok = False
    if extra["old3"] + extra["new4"] != extra["all"]:
        print(f"refused: old3's {extra['old3']:+d} and new4's "
              f"{extra['new4']:+d} popcnt against zero's sum to "
              f"{extra['old3'] + extra['new4']:+d}, not all's "
              f"{extra['all']:+d}: a part computes a column outside its mask "
              "(more) or a force lost one (fewer)")
        ok = False
    if ok:
        print("code: four distinct .text sections; popcnt beyond zero's old3 "
              f"{extra['old3']:+d} + new4 {extra['new4']:+d} = all "
              f"{extra['all']:+d}")
    return 0 if ok else 1


def ratio_ci(base, cand):
    logs = [math.log(c / b) for b, c in zip(base, cand)]
    n = len(logs)
    m = statistics.mean(logs)
    se = statistics.stdev(logs) / math.sqrt(n)
    t = T975.get(n - 1, 2.0)
    return math.exp(m) - 1, math.exp(m - t * se) - 1, math.exp(m + t * se) - 1


FORCED = ("all", "old3", "new4")


def separates(readings, floor):
    """True when each forced build's instructions lie above zero's and all's
    above each part's, every gap wider than the A/A floor. It does not check
    that the parts add up to all -- an overlap between them passes here and is
    `code`'s to refuse -- so the sum is printed and not gated. The floor is the A/A's instruction CI bound, read here in billionths because
    the counts are near-deterministic and a percent with three decimals
    prints zero for it. Prints each check, and the reason when one fails."""
    missing = [name for name in FORCED if name not in readings]
    if missing:
        print("refused: no %s comparison, so the masks cannot be checked"
              % " and ".join(missing))
        return False
    ok = True
    for workload_name in ("bench16", "pos11"):
        fl = floor(workload_name, "instructions")
        ci = {name: readings[name][workload_name]["instructions"]
              for name in FORCED}
        print(f"masks {workload_name}: A/A instruction floor {fl * 1e9:.1f} "
              "ppb; " + ", ".join(f"{n} {ci[n][1] * 100:+.4f} .. "
                                  f"{ci[n][2] * 100:+.4f} %" for n in FORCED))
        for name in FORCED:
            if not ci[name][1] > fl:
                print(f"refused: {workload_name} {name}'s instructions CI "
                      f"starts at {ci[name][1] * 1e9:+.1f} ppb, not above the "
                      f"A/A floor {fl * 1e9:.1f} ppb: the forced build "
                      "computes nothing zero does not")
                ok = False
        for part in ("old3", "new4"):
            gap = ci["all"][1] - ci[part][2]
            if not gap > fl:
                print(f"refused: {workload_name} all's instructions exceed "
                      f"{part}'s by {gap * 1e9:+.1f} ppb at the nearest CI "
                      f"bounds, not more than the A/A floor {fl * 1e9:.1f} "
                      f"ppb: all forces columns {part} does not, so {part} "
                      "computed them too")
                ok = False
        parts = ci["old3"][0] + ci["new4"][0]
        print(f"masks {workload_name}: old3 + new4 {parts * 100:+.3f} % "
              f"against all {ci['all'][0] * 100:+.3f} %, the parts "
              f"{(parts - ci['all'][0]) * 100:+.3f} points from it "
              "(printed, not gated: shared loads and register pressure "
              "make it inexact)")
    if ok:
        print("masks: each part above zero and all above each part, beyond "
              "the floor; overlap between the parts is the code check's")
    return ok


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

    if not separates(readings, floor):
        return 1

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
    if len(argv) == 2 and argv[0] == "code":
        return code(argv[1])
    if len(argv) == 2 and argv[0] == "read":
        return read(argv[1])
    print(__doc__, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
