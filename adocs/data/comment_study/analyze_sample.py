#!/usr/bin/env python3
"""Question 2: byte shares per class, stratified estimator with a bootstrap.

Each block's bytes are split by the fractions in classified.tsv. A stratum's
share is the sum of class bytes over the sum of bytes in its sampled blocks,
weighted by the stratum's whole-population bytes. The 90 % interval comes from
resampling blocks within strata, 4000 times, fixed seed. It covers sampling
error only, not the classifier (one reader, no second rater).
"""
import json
import random
from collections import defaultdict

CLASSES = "CWRHPTX"
NAMES = dict(C="contract/invariant", W="non-derivable why/hazard", R="required by a project rule",
             H="history/narration", P="pointer only", T="restates the code", X="stale or wrong")
S = [json.loads(l) for l in open("sample.jsonl")]
F = {}
STALE = {}
for line in open("classified.tsv").read().splitlines()[1:]:
    parts = line.split("\t")
    bid, frac, stale = parts[0], parts[1], parts[2]
    d = {}
    for item in frac.split(","):
        d[item[0]] = float(item[1:])
    assert abs(sum(d.values()) - 1) < 0.011, (bid, d)
    F[bid] = d
    STALE[bid] = int(stale)
for b in S:
    b["frac"] = F[b["id"]]
    b["stale"] = STALE[b["id"]]


def estimate(blocks, strata_bytes):
    tot = {c: 0.0 for c in CLASSES}
    by = defaultdict(list)
    for b in blocks:
        by[(b["stratum_area"], b["band"])].append(b)
    weight = 0
    for k, bs in by.items():
        den = sum(x["comment_bytes"] for x in bs)
        w = strata_bytes[k]
        weight += w
        for x in bs:
            for c, fr in x["frac"].items():
                tot[c] += w * fr * x["comment_bytes"] / den
    return {c: tot[c] / weight for c in CLASSES}


def boot(blocks, strata_bytes, n=4000, seed=7):
    rng = random.Random(seed)
    by = defaultdict(list)
    for b in blocks:
        by[(b["stratum_area"], b["band"])].append(b)
    out = {c: [] for c in CLASSES}
    for _ in range(n):
        res = []
        for k, bs in by.items():
            res += [rng.choice(bs) for _ in bs]
        e = estimate(res, strata_bytes)
        for c in CLASSES:
            out[c].append(e[c])
    ci = {}
    for c in CLASSES:
        v = sorted(out[c])
        ci[c] = (v[int(0.05 * n)], v[int(0.95 * n)])
    return ci


strata_bytes = {}
for b in S:
    strata_bytes[(b["stratum_area"], b["band"])] = b["stratum_bytes"]

groups = {
    "all census": lambda b: True,
    "src": lambda b: b["stratum_area"] == "src",
    "tests": lambda b: b["stratum_area"] == "tests",
    "tools": lambda b: b["stratum_area"] == "tools",
    "root+cmake": lambda b: b["stratum_area"] == "root+cmake",
    "adocs_data": lambda b: b["stratum_area"] == "adocs_data",
    "src+tests+tools+root": lambda b: b["stratum_area"] != "adocs_data",
}
with open("sample_shares.tsv", "w") as f:
    f.write("group\tclass\tname\tshare\tlo90\thi90\n")
    for g, pred in groups.items():
        bs = [b for b in S if pred(b)]
        e = estimate(bs, strata_bytes)
        ci = boot(bs, strata_bytes)
        print(f"{g}: {len(bs)} blocks, " + "  ".join(
            f"{c} {100*e[c]:.1f} [{100*ci[c][0]:.1f},{100*ci[c][1]:.1f}]" for c in CLASSES))
        for c in CLASSES:
            f.write(f"{g}\t{c}\t{NAMES[c]}\t{e[c]:.4f}\t{ci[c][0]:.4f}\t{ci[c][1]:.4f}\n")

# stale: blocks with a false or dead claim, block-weighted within strata
for g, pred in groups.items():
    bs = [b for b in S if pred(b)]
    by = defaultdict(list)
    for b in bs:
        by[(b["stratum_area"], b["band"])].append(b)
    nb = 0
    ns = 0.0
    byt = 0
    bst = 0.0
    for k, v in by.items():
        pop_blocks = v[0]["stratum_blocks"]
        nb += pop_blocks
        ns += pop_blocks * sum(x["stale"] for x in v) / len(v)
        byt += v[0]["stratum_bytes"]
        den = sum(x["comment_bytes"] for x in v)
        bst += v[0]["stratum_bytes"] * sum(x["comment_bytes"] for x in v if x["stale"]) / den
    print(f"{g}: stale blocks in sample {sum(x['stale'] for x in bs)}/{len(bs)}; "
          f"population-weighted {100*ns/nb:.1f} % of blocks, {100*bst/byt:.1f} % of bytes sit in a block with a dead claim")

with open("sample_classified.jsonl", "w") as f:
    for b in S:
        f.write(json.dumps(b) + "\n")
