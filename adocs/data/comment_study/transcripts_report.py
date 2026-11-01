#!/usr/bin/env python3
"""Tables from transcript_rows.jsonl. Prints and writes transcript_tables.txt."""
import json
import re
import statistics as st
import sys
from collections import Counter, defaultdict

rows = [json.loads(l) for l in open("transcript_rows.jsonl")]
excl = [r for r in rows if r["excluded"]]
rows = [r for r in rows if not r["excluded"]]
out = []


def P(*a):
    s = " ".join(str(x) for x in a)
    out.append(s)


def fam(cat):
    cat = cat.split(":", 1)[-1] if cat.startswith(("Read:", "Grep:", "Glob:")) else cat
    cat = cat.replace("Bash:read/", "").replace("Bash:", "")
    return cat


main = [r for r in rows if r["kind"] == "main"]
sub = [r for r in rows if r["kind"] != "main"]
P("excluded this study's own transcript(s):", len(excl))
P("transcripts:", len(rows), "main", len(main), "subagent", len(sub))
P("span:", min(r["first"] for r in rows if r["first"]), max(r["last"] for r in rows if r["last"]))
P("requests:", sum(r["requests"] for r in rows), "main", sum(r["requests"] for r in main), "sub", sum(r["requests"] for r in sub))
tot = Counter()
for r in rows:
    tot.update(r["totals"])
P("totals:", {k: int(v) for k, v in tot.items()})
for name, group in (("main", main), ("subagent", sub)):
    t = Counter()
    for r in group:
        t.update(r["totals"])
    P(name, {k: int(v) for k, v in t.items()})
models = Counter()
for r in rows:
    for m, n in r["models"].items():
        models[m] += n
P("models by assistant record:", models.most_common(8))

# Category table, whole corpus
raw = Counter()
w = Counter()
rawc = Counter()
wc = Counter()
item = Counter()
itemc = Counter()
n_res = Counter()
for r in rows:
    for k, v in r["raw_by"].items():
        raw[fam(k)] += v
    for k, v in r["w_by"].items():
        w[fam(k)] += v
    for k, v in r["raw_comment"].items():
        rawc[fam(k)] += v
    for k, v in r["w_comment"].items():
        wc[fam(k)] += v
    for k, v in r["item_tokens"].items():
        item[fam(k)] += v
    for k, v in r["item_comment_tokens"].items():
        itemc[fam(k)] += v
    for k, v in r["n_results"].items():
        n_res[fam(k)] += v
raw_total = tot["raw_context"]
w_total = tot["weighted_total"]
base_raw = sum(r["base_tokens"] * r["requests"] for r in rows)
P("")
P("Context load by category (tokens=bytes/4; raw is token x requests it sat in; weighted is cache-priced)")
P(f"{'category':26s}{'results':>8s}{'tok_in':>11s}{'cmt_tok':>10s}{'cmt%':>6s}{'raw':>14s}{'raw%':>7s}{'weighted':>12s}{'wtd%':>7s}")
for k, v in sorted(raw.items(), key=lambda kv: -w[kv[0]]):
    if w[k] < 0.0005 * w_total:
        continue
    pc = 100 * itemc[k] / item[k] if item[k] else 0
    P(f"{k:26s}{n_res[k]:8d}{int(item[k]):11d}{int(itemc[k]):10d}{pc:6.0f}{int(v):14d}{100*v/raw_total:7.2f}{int(w[k]):12d}{100*w[k]/w_total:7.2f}")
attr_raw = sum(raw.values())
attr_w = sum(w.values())
P(f"attributed raw {100*attr_raw/raw_total:.1f}% of context processed; weighted items {100*attr_w/w_total:.1f}% of weighted total")
P(f"output at 5x: {100*tot['weighted_output']/w_total:.1f}% of weighted total; base context (first request size x requests) raw {100*base_raw/raw_total:.1f}%")

comment_raw = sum(rawc.values())
comment_w = sum(wc.values())
P(f"COMMENTS IN CONTEXT: raw {int(comment_raw)} = {100*comment_raw/raw_total:.2f}% of context processed; weighted {int(comment_w)} = {100*comment_w/w_total:.2f}% of weighted total")
for k in sorted(wc, key=lambda x: -wc[x])[:12]:
    P(f"   {k:24s} raw {100*rawc[k]/raw_total:6.2f}%  weighted {100*wc[k]/w_total:6.2f}%  tokens in {int(itemc[k])}")

# authored comment bytes
auth = Counter()
for r in rows:
    auth.update(r["authored"])
ab = sum(v for k, v in auth.items() if k.endswith(":bytes"))
cb = sum(v for k, v in auth.items() if k.endswith(":comment_bytes"))
code_ab = sum(v for k, v in auth.items() if k.endswith(":bytes") and k.split(":")[0] in ("src", "tests", "tools", "root_scripts", "adocs/data_script"))
P(f"authored through Edit/Write: {int(ab)} bytes, of which code files {int(code_ab)}, comment lines in those {int(cb)} ({100*cb/max(1,code_ab):.0f}%)")
P("authored by area:", {k: int(v) for k, v in auth.most_common(16)})
author_w = cb / 4 * 5
P(f"authored comment tokens {int(cb/4)}; at output price 5x = {int(author_w)} units = {100*author_w/w_total:.2f}% of weighted total (retention in context not added; counted as output only)")

# Per subagent distributions
def q(v, p):
    v = sorted(v)
    return v[min(len(v) - 1, int(p * len(v)))] if v else 0


P("")
P("Per subagent: requests, raw context, weighted, comment shares")
big = [r for r in sub if r["requests"] >= 3]
for label, sel in (
    ("all", big),
    ("implement", [r for r in big if r["desc_kind"] == "implement"]),
    ("close", [r for r in big if r["desc_kind"] == "close"]),
    ("review", [r for r in big if r["desc_kind"] == "review"]),
    ("explore", [r for r in big if r["desc_kind"] == "explore"]),
):
    if not sel:
        continue
    reqs = [r["requests"] for r in sel]
    rawc_ = [sum(r["raw_comment"].values()) for r in sel]
    rawt = [r["totals"]["raw_context"] for r in sel]
    wt = [r["totals"]["weighted_total"] for r in sel]
    wc_ = [sum(r["w_comment"].values()) for r in sel]
    outp = [r["totals"]["output"] for r in sel]
    sh = [100 * a / b for a, b in zip(wc_, wt) if b]
    shr = [100 * a / b for a, b in zip(rawc_, rawt) if b]
    agg = 100 * sum(wc_) / sum(wt)
    aggr = 100 * sum(rawc_) / sum(rawt)
    P(f"{label:10s} n={len(sel):3d} req med {q(reqs,.5)} p90 {q(reqs,.9)} | raw med {int(q(rawt,.5))} p90 {int(q(rawt,.9))} | weighted med {int(q(wt,.5))} p90 {int(q(wt,.9))} | output med {int(q(outp,.5))} | cmt wtd% med {q(sh,.5):.1f} p90 {q(sh,.9):.1f} agg {agg:.1f} | cmt raw% med {q(shr,.5):.1f} p90 {q(shr,.9):.1f} agg {aggr:.1f}")

# Per step aggregation: all subagents naming a step id in their description
by_step = defaultdict(list)
for r in big:
    ids = r["desc_steps"] or r["brief_steps"][:1]
    if ids:
        by_step[ids[0]].append(r)
P("")
P(f"steps with at least one subagent attributed: {len(by_step)}")
rows_step = []
for s, rs in by_step.items():
    wt = sum(r["totals"]["weighted_total"] for r in rs)
    rt = sum(r["totals"]["raw_context"] for r in rs)
    wc_ = sum(sum(r["w_comment"].values()) for r in rs)
    rc = sum(sum(r["raw_comment"].values()) for r in rs)
    req = sum(r["requests"] for r in rs)
    code_tokens = sum(sum(r["item_comment_tokens"].values()) for r in rs)
    rows_step.append((s, len(rs), req, rt, wt, rc, wc_, code_tokens))
rows_step.sort(key=lambda x: -x[4])
med = lambda i: q([x[i] for x in rows_step], 0.5)
P(f"per step medians: subagents {med(1)} requests {med(2)} raw {int(med(3))} weighted {int(med(4))} comment raw {int(med(5))} comment weighted {int(med(6))} comment tokens read {int(med(7))}")
shs = [100 * x[6] / x[4] for x in rows_step if x[4]]
shr = [100 * x[5] / x[3] for x in rows_step if x[3]]
P(f"per step comment share of weighted: med {q(shs,.5):.1f} p25 {q(shs,.25):.1f} p75 {q(shs,.75):.1f} p90 {q(shs,.9):.1f}; of raw: med {q(shr,.5):.1f} p25 {q(shr,.25):.1f} p75 {q(shr,.75):.1f} p90 {q(shr,.9):.1f}")
P("top 12 steps by weighted:")
for x in rows_step[:12]:
    P(f"  {x[0]} agents {x[1]} requests {x[2]} raw {int(x[3])} weighted {int(x[4])} cmt_raw {int(x[5])} ({100*x[5]/x[3]:.1f}%) cmt_w {int(x[6])} ({100*x[6]/x[4]:.1f}%) cmt_tok_read {int(x[7])}")

# Main sessions
P("")
P("Main sessions")
for r in sorted(main, key=lambda r: -r["totals"].get("weighted_total", 0))[:10]:
    t = r["totals"]
    if not t:
        continue
    P(f"  {r['file'][:8]} {str(r['first'])[:10]}..{str(r['last'])[:10]} req {r['requests']} compact {r['compactions']} base {r['base_tokens']} raw {int(t['raw_context'])} wtd {int(t['weighted_total'])} out {t['output']} cw {t['cache_write']} cr {t['cache_read']} cmt_w {100*sum(r['w_comment'].values())/t['weighted_total']:.2f}%")
P("subagent base context, first request size: median", q([r["base_tokens"] for r in big], 0.5), "p90", q([r["base_tokens"] for r in big], 0.9))
P("brief bytes: median", q([r["brief_bytes"] for r in big], 0.5), "p90", q([r["brief_bytes"] for r in big], 0.9))
c1h = tot["cache_write_1h"]
c5m = tot["cache_write_5m"]
P(f"cache writes: 5m {c5m} 1h {c1h}; read/write ratio {tot['cache_read']/max(1,tot['cache_write']):.1f}; uncached input {tot['input']}")
txt = "\n".join(out)
open("transcript_tables.txt", "w").write(txt + "\n")
print(txt)
