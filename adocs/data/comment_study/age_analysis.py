#!/usr/bin/env python3
"""Question 3: comment bytes by age, class and staleness against age in the sample,
and how far a blame date sits from a comment's first appearance (pickaxe on 24 blocks)."""
import csv
import json
import random
import subprocess
from collections import Counter, defaultdict
from datetime import datetime, timezone

COMMIT = "5954c12"
REPO = "/home/max/ws/chesso"
HEAD_DATE = datetime(2026, 10, 31, tzinfo=timezone.utc)

rows = list(csv.DictReader(open("blame_by_week.tsv"), delimiter="\t"))


def bucket(week):
    y, w = week.split("-W")
    y, w = int(y), int(w)
    if y < 2026 or w < 31:
        return "pre-achesso (master, bitboard)"
    if w <= 35:
        return "2026-08 (W31-35)"
    if w <= 39:
        return "2026-09 (W36-39)"
    return "2026-10 (W40-44)"


order = ["pre-achesso (master, bitboard)", "2026-08 (W31-35)", "2026-09 (W36-39)", "2026-10 (W40-44)"]
tab = defaultdict(Counter)
for r in rows:
    b = bucket(r["week"])
    tab[r["area"]][b] += int(r["comment_bytes"])
    tab["ALL"][b] += int(r["comment_bytes"])
    tab["ALL code"][b] += int(r["code_bytes"])
    tab[r["area"] + " code"][b] += int(r["code_bytes"])
print("share of bytes by age of last touching commit")
for a in ["src", "tests", "tools", "root+cmake", "adocs_data", "ALL", "src code", "tests code", "ALL code"]:
    t = sum(tab[a].values())
    print(f"{a:12s} {t:9d} " + "  ".join(f"{100*tab[a][k]/t:5.1f}%" for k in order))
print("columns:", order)
wk_all = defaultdict(int)
for r in rows:
    wk_all[r["week"]] += int(r["comment_bytes"])
tot = sum(wk_all.values())
young = sum(v for k, v in wk_all.items() if k >= "2026-W40")
print(f"bytes under a month old (W40+): {100*young/tot:.1f}%")

S = [json.loads(l) for l in open("sample_classified.jsonl")]
B = {json.loads(l)["id"]: json.loads(l) for l in open("blame_blocks.jsonl")}
for b in S:
    x = B[b["id"]]
    b["median_t"] = x["median"]
    b["oldest_t"] = x["oldest"]
    b["newest_t"] = x["newest"]
    b["commits"] = x["commits"]
    b["age_days"] = (HEAD_DATE.timestamp() - x["median"]) / 86400

# restrict the class-by-age reading to what a cleanup could touch
live = [b for b in S if b["stratum_area"] != "adocs_data"]
live.sort(key=lambda b: b["age_days"])
n = len(live)
terciles = [live[: n // 3], live[n // 3 : 2 * n // 3], live[2 * n // 3 :]]
print("\nclass by age tercile, sampled blocks outside adocs/data, byte-weighted within sample (no stratum weights)")
print("tercile  blocks  age_days(median)  C W R H P T X  stale_blocks  mean_commits_per_block  mean_block_bytes")
for i, t in enumerate(terciles):
    bytes_ = sum(b["comment_bytes"] for b in t)
    cl = Counter()
    for b in t:
        for c, f in b["frac"].items():
            cl[c] += f * b["comment_bytes"]
    ages = sorted(b["age_days"] for b in t)
    stale = sum(b["stale"] for b in t)
    print(i + 1, len(t), round(ages[len(ages) // 2], 1), " ".join(f"{c}={100*cl[c]/bytes_:.0f}" for c in "CWRHPTX"), stale,
          round(sum(b["commits"] for b in t) / len(t), 1), round(bytes_ / len(t)))


# Rank correlation by hand: age against H share, stale, and T share
def ranks(v):
    o = sorted(range(len(v)), key=lambda i: v[i])
    r = [0] * len(v)
    i = 0
    while i < len(o):
        j = i
        while j + 1 < len(o) and v[o[j + 1]] == v[o[i]]:
            j += 1
        for k in range(i, j + 1):
            r[o[k]] = (i + j) / 2
        i = j + 1
    return r


def spearman(a, b):
    ra, rb = ranks(a), ranks(b)
    m = (len(a) - 1) / 2
    num = sum((x - m) * (y - m) for x, y in zip(ra, rb))
    da = sum((x - m) ** 2 for x in ra) ** 0.5
    db = sum((y - m) ** 2 for y in rb) ** 0.5
    return num / (da * db) if da and db else 0


def perm_p(a, b, n=4000, seed=5):
    rng = random.Random(seed)
    obs = abs(spearman(a, b))
    c = list(b)
    hits = 0
    for _ in range(n):
        rng.shuffle(c)
        hits += abs(spearman(a, c)) >= obs
    return (hits + 1) / (n + 1)


ages = [b["age_days"] for b in live]
for name, f in (
    ("H share", lambda b: b["frac"].get("H", 0)),
    ("W share", lambda b: b["frac"].get("W", 0)),
    ("C share", lambda b: b["frac"].get("C", 0)),
    ("T share", lambda b: b["frac"].get("T", 0)),
    ("stale flag", lambda b: b["stale"]),
    ("block bytes", lambda b: b["comment_bytes"]),
):
    v = [f(b) for b in live]
    print(f"spearman(age, {name}) = {spearman(ages, v):+.3f}  permutation p = {perm_p(ages, v):.3f}")
print("multi-commit blocks:", sum(1 for b in live if b["commits"] > 1), "of", len(live),
      "; those with a stale claim:", sum(b["stale"] for b in live if b["commits"] > 1), "; single-commit with a stale claim:", sum(b["stale"] for b in live if b["commits"] == 1))
big = [b for b in live if b["comment_bytes"] >= 1200]
print("blocks >=1200B:", len(big), "stale", sum(b["stale"] for b in big), "| smaller:", len(live) - len(big), "stale", sum(b["stale"] for b in live) - sum(b["stale"] for b in big))

# Blame date against the first commit that holds the block's opening words.
rng = random.Random(11)
pick = rng.sample([b for b in live if b["comment_bytes"] >= 100], 24)
cache = {}
gaps = []
for b in pick:
    t = cache.setdefault(b["file"], subprocess.run(["git", "-C", REPO, "show", f"{COMMIT}:{b['file']}"], capture_output=True).stdout.decode())
    text = t[b["start_off"] : b["end_off"]]
    first = text.split("\n")[0].strip().lstrip("/#* ")[:45].strip()
    if len(first) < 15:
        continue
    r = subprocess.run(["git", "-C", REPO, "log", COMMIT, "--format=%at", "--reverse", "-S", first, "--", b["file"]], capture_output=True, text=True)
    ts = r.stdout.split()
    if not ts:
        continue
    origin = int(ts[0])
    gaps.append((round((b["median_t"] - origin) / 86400, 1), b["id"], b["commits"]))
gaps.sort()
print("\nblame median minus first appearance of the opening words, days, 24 blocks:", [g[0] for g in gaps])
print("median gap", gaps[len(gaps) // 2][0], "; gap > 7 days:", sum(1 for g in gaps if g[0] > 7), "of", len(gaps))
