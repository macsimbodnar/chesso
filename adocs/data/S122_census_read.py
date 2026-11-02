#!/usr/bin/env python3
"""S122's research pass, 2026-11-02: assembles adocs/data/S122_census.txt from the raw
outputs adocs/data/S122_census.sh leaves under OUT. Arithmetic over those files and no
more: no engine, no label beyond what the census printed.

  S122_census_read.py OUT > adocs/data/S122_census.txt
"""
import math
import re
import sys
from pathlib import Path

OUT = Path(sys.argv[1])


def text(name):
    return (OUT / name).read_text()


def rows_of(name):
    out = {}
    for line in text(name).splitlines():
        parts = line.split()
        if len(parts) >= 5:
            out[parts[0]] = (int(parts[1]), int(parts[2]), int(parts[3]), int(parts[4]))
    return out


print("""# S122's research pass, 2026-11-02, read at ba1cad6, bench 5482277, with the fast check's scans
# that DEC-344 rests on. Written by adocs/data/S122_census.sh read. Counts and instructions only: no
# fit, no match, no timing. The census (S122_census.cpp, S122_census_core.hpp) reads the FEN and game
# columns; the result column is read for S083's training corpus and head's 12000-game sample, each
# at its stated K, and S082's fixed set keeps its labels unread. No weight below is a proposal, and
# "attacking" means only the count thresholds the census prints.
""")

print("== head's spread and clamp, tools/eval_spread over S083's corpus")
print(text("spread_head_s083.txt"))
print("== head's spread and clamp, tools/eval_spread over S082's qs set")
print(text("spread_head_s082qs.txt"))
print("== head's truncation_scan over S083's corpus, --min 2.8")
print(text("trunc_head_s083.txt"))

for name, title in (
    ("c_s083.txt", "S083's corpus, 6226944 rows: the census, with the label sections"),
    ("c_s082qs.txt", "S082's fixed set, qs view"),
    ("c_s082root.txt", "S082's fixed set, root view"),
    ("c_s082pv.txt", "S082's fixed set, pv view"),
    ("c_pool_qs_train.txt", "S082's pool, qs training side: every row S083 sampled from"),
    ("c_pool_root_train.txt", "S082's pool, root training side"),
    ("c_pool_pv_train.txt", "S082's pool, pv training side"),
):
    print(f"== {title}")
    print(text(name))

for name, title in (
    ("scan_s083.txt", "the scans, S083's corpus at K 0.7119 (S101's zero-epoch K, DEC-344's reading)"),
    ("scan_sample.txt", "the scans, head's 12000-game sample inside the filter, at its own K 0.7407"),
    ("scan_s083_ownk.txt", "the scans, quick: S083's corpus at the K head's evaluate asks of it, 0.7147"),
    ("scan_sample_k7119.txt", "the scans, quick: the sample at S083's K 0.7119, the reading DEC-344 replaced"),
):
    print(f"== {title}")
    print((OUT / "scan" / name).read_text())

print("== what each tool's row-by-row check catches, S122_identity_plants.patch")
print(text("plants.txt"))

print("== instructions on bench and bench 16, the sink rows, perf_counters' instructions")
cost = rows_of("cost_rows.txt")
base = cost["v0"]
print(f"{'row':10s} {'nodes':>10s} {'instructions':>14s} {'%':>7s} {'nodes16':>10s} {'instr16':>14s} {'%':>7s}")
for name, r in cost.items():
    print(f"{name:10s} {r[0]:10d} {r[1]:14d} {100 * (r[1] / base[1] - 1):+7.2f} {r[2]:10d} {r[3]:14d} {100 * (r[3] / base[3] - 1):+7.2f}")
print("""# v0 the tree at ba1cad6's arithmetic, v64 the finalizer alone over a constant on each side,
# v1 the accumulator over today's attacker and incidence counts, v128 the same over the
# zone plus its forward rank, v2 safe checks, v4 weak squares, v8 defenders, v16 storm on a
# slot miss, v256 defenders by squares, each with the finalizer; v3, v5, v9 pairs; v31
# together; v63 with the checks gated on a king that has an attacker. twice* run stage two
# twice, one call's price. v64 folds to constants at this weight and reads layout only.
""")

print("== the search census, S122_CENSUS over `chesso bench`, every call of each site")
census = text("search_census.txt")
print(census)
calls = {}
for line in census.splitlines():
    m = re.match(r"site (.+?)\s+calls (\d+)", line)
    if m:
        calls[m.group(1)] = int(m.group(2))
past = calls.get("lazy, past the shortcut", 0)
every = calls.get("lazy, every call", 0)
exact = calls.get("search evaluate()", 0)
print(f"calls {exact} evaluate + {every} lazy; stage two on {exact + past}")
if every:
    print(f"the shortcut skips {100 * (every - past) / every:.1f} % of lazy calls, {100 * (every - past) / (every + exact):.1f} % of all")
two = cost.get("twice0")
if two and exact + past:
    print(f"one stage-two call, head: {(two[1] - base[1]) / (exact + past):.0f} instructions")
two = cost.get("twice31")
if two and exact + past:
    print(f"one stage-two call, all parts: {(two[1] - cost['v31'][1]) / (exact + past):.0f} instructions")
print()

print("== margin rows, real builds: the clamp and the shortcut both at the margin, nodes moving")
margin = rows_of("margin_rows.txt")
print(f"{'row':10s} {'nodes':>10s} {'instr':>14s} {'instr/node':>10s} {'%':>7s}   {'nodes16':>10s} {'instr16':>14s} {'instr/node':>10s} {'%':>7s}")
per = base[1] / base[0]
per16 = base[3] / base[2]
print(f"{'m184':10s} {base[0]:10d} {base[1]:14d} {per:10.1f} {0.0:+7.2f}   {base[2]:10d} {base[3]:14d} {per16:10.1f} {0.0:+7.2f}")
a184 = cost["v31"]
for name, r in margin.items():
    if name.startswith("a"):
        continue
    print(f"{name:10s} {r[0]:10d} {r[1]:14d} {r[1] / r[0]:10.1f} {100 * (r[1] / r[0] / per - 1):+7.2f}   {r[2]:10d} {r[3]:14d} {r[3] / r[2]:10.1f} {100 * (r[3] / r[2] / per16 - 1):+7.2f}")
print("# all five parts' sink live; % is against the all-parts row at 184, per node:")
per_a = a184[1] / a184[0]
per16_a = a184[3] / a184[2]
print(f"{'a184':10s} {a184[0]:10d} {a184[1]:14d} {per_a:10.1f} {0.0:+7.2f}   {a184[2]:10d} {a184[3]:14d} {per16_a:10.1f} {0.0:+7.2f}")
for name, r in margin.items():
    if not name.startswith("a"):
        continue
    print(f"{name:10s} {r[0]:10d} {r[1]:14d} {r[1] / r[0]:10.1f} {100 * (r[1] / r[0] / per_a - 1):+7.2f}   {r[2]:10d} {r[3]:14d} {r[3] / r[2]:10.1f} {100 * (r[3] / r[2] / per16_a - 1):+7.2f}")
print("# m32000 and a32000 are the shortcut and the clamp both out of reach; the m rows' % is against m184.")
print("# A real margin moves the tree, so these rows are the tree's chaos and the price together. The")
print("# first-order price below holds the tree still, and the two disagree in sign at 250 (first order")
print("# +2.53 %, real -2.2 and -6.5 % a node): first-order prices carry no pick (DEC-344 (4)).\n")
m = re.search(r"lazy calls (\d+), skipped at margin:(.*)", census)
if m and exact + past:
    lazy = int(m.group(1))
    skips = {int(a): float(b) for a, b in re.findall(r"(\d+):([\d.]+)%", m.group(2))}
    price0 = (cost["twice0"][1] - base[1]) / (exact + past)
    price1 = (cost["twice31"][1] - cost["v31"][1]) / (exact + past)
    print(f"{'margin':>7s} {'skipped':>8s} {'extra stage-two calls':>22s} {'head instr %':>13s} {'all parts instr %':>18s}")
    for margin_value, share in skips.items():
        extra = (skips[184] - share) / 100.0 * lazy
        print(f"{margin_value:7d} {share:7.2f}% {extra:22.0f} {100 * extra * price0 / base[1]:+13.2f} {100 * extra * price1 / cost['v31'][1]:+18.2f}")
    print(f"# every lazy call past the shortcut: +{100 * skips[184] / 100.0 * lazy * price0 / base[1]:.2f} % at head's price, +{100 * skips[184] / 100.0 * lazy * price1 / cost['v31'][1]:.2f} % at all parts'.")
print()

print("== forced-weight probes: tools/eval_spread and tools/truncation_scan, a build per row, the")
print("#  flat unit weight w at T = (w u)^2 / 4096 in the middlegame half alone, inside the clamp.")
print("#  A probe of reach; no weight is a proposal. The tools check themselves on every row.")
print(text("probe_rows.txt"))
for path in sorted(OUT.glob("p*m*.spread")):
    print(f"-- {path.stem}, eval_spread")
    print(path.read_text())
    trunc = path.with_suffix(".trunc")
    print(f"-- {path.stem}, truncation_scan --min 3.3")
    print(trunc.read_text())

sample = OUT / "smoke"
if (sample / "run.log").exists():
    print("== the sample: head's engine at 100000 nodes, --quiet-limit 30000, 3000 games then 9000")
    print((sample / "run.log").read_text())
    print((sample / "b" / "run.log").read_text())
    for name, title in (
        ("c_qs_inside.txt", "qs rows whose sampled score is under 1000, what today's filter keeps"),
        ("c_qs_past.txt", "qs rows whose sampled score is 1000 or past, what today's filter drops"),
    ):
        if (sample / name).exists():
            print(f"-- {title}")
            print((sample / name).read_text())
