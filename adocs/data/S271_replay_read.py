"""Read S271_replay.py's output (files r_<engine>_<mode>_d<depth>_h<hash>.json,
engine h = the commit under test, k = the same with S271_keep_table.diff): nodes to a fixed depth, per send mode, against `base`.

Conversion to Elo (named a conversion, not a verdict): extra nodes to the same
depth are read as the same fraction of thinking time lost, priced at S219's
measured +174.85 Elo per doubling of time (4+0.04 -> 8+0.08, self-play).
"""
import json
import math
import statistics
import sys
from pathlib import Path

ELO_PER_DOUBLING = 174.85
d = Path(sys.argv[1])

for depth in (12, 14):
    for hs in (16, 128):
        runs = {}
        for tag in ("h_base", "h_bare", "h_irrev", "k_bare"):
            f = d / f"r_{tag}_d{depth}_h{hs}.json"
            if f.exists():
                runs[tag] = json.loads(f.read_text())
        if "h_base" not in runs:
            continue
        base = runs["h_base"]
        print(f"depth {depth}, Hash {hs} MB: {len(base)} positions, "
              f"base total {sum(r['nodes'] for r in base)} nodes")
        for tag, rec in runs.items():
            if tag == "h_base":
                continue
            assert [(r["game"], r["ply"]) for r in rec] == \
                   [(r["game"], r["ply"]) for r in base]
            tot = sum(r["nodes"] for r in rec) / sum(r["nodes"] for r in base)
            per = [r["nodes"] / b["nodes"] for r, b in zip(rec, base) if b["nodes"]]
            moved = sum(r["best"] != b["best"] for r, b in zip(rec, base))
            elo = ELO_PER_DOUBLING * math.log2(tot)
            print(f"  {tag:8s} total x{tot:.3f}  median x{statistics.median(per):.3f}"
                  f"  best move differs {moved}/{len(rec)}"
                  f"  ~{elo:.0f} Elo by conversion")
        print()
