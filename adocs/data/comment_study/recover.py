#!/usr/bin/env python3
"""Question 4: for sampled history and why comments, is the information elsewhere?

Index: plan_done, plan_todo, decisions (by entry), audits, specs, git log messages
and adocs/data text files, by paragraph. For each sampled block with H or W >= 0.2,
outside adocs/data:

  facts      numbers of 4+ digits, decimals and backticked strings: exact hits
  sentences  a sentence is recovered when a single paragraph holds >= 60 % of its
             content words (6+ of them required); fuzzy, so a base rate is measured
             by asking the same question of a paragraph set that excludes docs
             whose vocabulary is the same subject (see control below).

Labels, per block, by bytes of sentences:
  by_id      in a document the block itself cites (S###, DEC-###)
  effort     in the index, but not in a document the block cites
  only       nowhere

Writes recover.tsv. `pointer_cost` is what following the cited ids costs in bytes:
the cited step files in full and the cited DEC entries in full.
"""
import json
import re
import subprocess
from collections import Counter, defaultdict
from pathlib import Path

COMMIT = "5954c12"
REPO = "/home/max/ws/chesso"


def git(*a):
    return subprocess.run(["git", "-C", REPO, *a], capture_output=True, check=True).stdout.decode("utf-8", "replace")


tree = git("ls-tree", "-r", "--name-only", COMMIT).split("\n")
STOP = set("the a an and or of to in on at by for from with is are was were be been it its this that these those which what as but not no so if then than there here their his her they we you i he she one two".split())
WORD = re.compile(r"[A-Za-z_][A-Za-z0-9_+-]{2,}|\d[\d.,]*\d")


def words(s):
    return {w.lower() for w in WORD.findall(s) if w.lower() not in STOP}


docs = {}  # doc id -> text
for p in tree:
    if re.match(r"adocs/(plan_done|plan_todo|plan_current|audit)/", p) and p.endswith(".md"):
        docs[p] = git("show", f"{COMMIT}:{p}")
    elif p.startswith("adocs/data/") and re.search(r"\.(txt|md|log)$", p):
        # the reading files; logs can be huge, and the first 200 kB is what a reader greps
        try:
            docs[p] = git("show", f"{COMMIT}:{p}")[:200000]
        except Exception:
            pass
for name in ("adocs/specs.md", "DEV_MANUAL.md", "MANUAL.md", "TOOLCHAIN.md"):
    docs[name] = git("show", f"{COMMIT}:{name}")
dec = git("show", f"{COMMIT}:adocs/decisions.md")
dec_entries = {}
for part in re.split(r"(?m)^(?=## DEC-\d+)", dec)[1:]:
    did = re.match(r"## (DEC-\d+)", part).group(1)
    dec_entries[did] = part
    docs[did] = part
log = git("log", "--format=%H%x00%B%x01", COMMIT)
for rec in log.split("\x01"):
    if "\x00" in rec:
        sha, body = rec.split("\x00", 1)
        docs["commit:" + sha.strip()[:10]] = body
step_doc = {}
for p in docs:
    m = re.match(r"adocs/plan_\w+/(S\d+)_", p)
    if m:
        step_doc[m.group(1)] = p

paras = []  # (doc, wordset)
postings = defaultdict(set)
for d, t in docs.items():
    for para in re.split(r"\n\s*\n", t):
        ws = words(para)
        if len(ws) < 4:
            continue
        pid = len(paras)
        paras.append((d, ws))
        for w in ws:
            postings[w].add(pid)
full_text = {d: t for d, t in docs.items()}
print("index:", len(docs), "documents,", len(paras), "paragraphs")

S = [json.loads(l) for l in open("sample_classified.jsonl")]
cache = {}


def blob(p):
    if p not in cache:
        cache[p] = git("show", f"{COMMIT}:{p}")
    return cache[p]


SID = re.compile(r"\bS\d{3}\b")
DID = re.compile(r"\bDEC-\d{3}\b")
FACT = re.compile(r"`([^`\n]{3,50})`|(?<![\w.])(\d{4,}(?:\.\d+)?)\b|(?<![\w.])(\d+\.\d+)\b")


def clean(t):
    t = re.sub(r"(?m)^\s*(//+|#+|\*+|/\*+|\"\"\"|\*/)", " ", t)
    t = t.replace("\\\n", "\n")
    return t


def sentences(t):
    t = re.sub(r"\s+", " ", clean(t))
    return [s.strip() for s in re.split(r"(?<=[.:;])\s+(?=[A-Z`*(])", t) if len(s.strip()) > 20]


def best(sent_words, allowed=None, exclude=None):
    """Best share of the sentence's words held by one paragraph."""
    cnt = Counter()
    for w in sent_words:
        for pid in postings.get(w, ()):
            if allowed is not None and paras[pid][0] not in allowed:
                continue
            if exclude is not None and paras[pid][0] in exclude:
                continue
            cnt[pid] += 1
    if not cnt:
        return 0.0, None
    pid, n = cnt.most_common(1)[0]
    return n / len(sent_words), paras[pid][0]


import sys as _s
TH = float(_s.argv[1]) if len(_s.argv) > 1 else 0.6
rows = []
for b in S:
    if b["stratum_area"] == "adocs_data":
        continue
    fr = b["frac"]
    if fr.get("H", 0) + fr.get("W", 0) < 0.2:
        continue
    text = blob(b["file"])[b["start_off"] : b["end_off"]]
    sids = sorted(set(SID.findall(text)))
    dids = sorted(set(DID.findall(text)))
    cited = {step_doc[s] for s in sids if s in step_doc} | set(dids)
    # commit messages that name a cited step count as cited: `git log --grep=S123` finds them
    cost = sum(len(full_text[d].encode()) for d in cited if d in full_text)
    facts = [m.group(1) or m.group(2) or m.group(3) for m in FACT.finditer(text)]
    f_id = f_any = 0
    for f in facts:
        hit_id = any(f in full_text.get(d, "") for d in cited)
        hit_any = hit_id or any(f in t for t in full_text.values())
        f_id += hit_id
        f_any += hit_any
    b_id = b_eff = b_only = 0
    for s in sentences(text):
        ws = words(s)
        n = len(s.encode())
        if len(ws) < 6:
            b_only += 0  # too short to test, left out of all three
            continue
        share_id, _ = best(ws, allowed=cited) if cited else (0.0, None)
        if share_id >= TH:
            b_id += n
            continue
        share_any, where = best(ws)
        if share_any >= TH:
            b_eff += n
        else:
            b_only += n
    rows.append(dict(id=b["id"], file=b["file"], line=b["start_line"], bytes=b["comment_bytes"], H=fr.get("H", 0), W=fr.get("W", 0),
                     sids=len(sids), dids=len(dids), cited_docs=len(cited), pointer_cost=cost,
                     facts=len(facts), f_id=f_id, f_any=f_any, b_id=b_id, b_eff=b_eff, b_only=b_only,
                     stratum_bytes=b["stratum_bytes"], area=b["stratum_area"], band=b["band"]))
cols = list(rows[0].keys())
with open("recover.tsv", "w") as f:
    f.write("\t".join(cols) + "\n")
    for r in rows:
        f.write("\t".join(str(r[c]) for c in cols) + "\n")
tb = sum(r["b_id"] + r["b_eff"] + r["b_only"] for r in rows)
print("blocks", len(rows))
print("sentence bytes: by id %.1f%%, effort %.1f%%, only %.1f%%" % tuple(100 * sum(r[k] for r in rows) / tb for k in ("b_id", "b_eff", "b_only")))
tf = sum(r["facts"] for r in rows)
print("facts", tf, "found by id %.1f%%, anywhere %.1f%%" % (100 * sum(r["f_id"] for r in rows) / tf, 100 * sum(r["f_any"] for r in rows) / tf))
# weighted by H and W separately: blocks mostly history vs mostly why
for lab, sel in (("H>=0.4", [r for r in rows if r["H"] >= 0.4]), ("W>=0.6", [r for r in rows if r["W"] >= 0.6])):
    t = sum(r["b_id"] + r["b_eff"] + r["b_only"] for r in sel) or 1
    ff = sum(r["facts"] for r in sel) or 1
    print(lab, "blocks", len(sel), "bytes id/effort/only %.0f/%.0f/%.0f" % tuple(100 * sum(r[k] for r in sel) / t for k in ("b_id", "b_eff", "b_only")),
          "facts id/any %.0f/%.0f" % (100 * sum(r["f_id"] for r in sel) / ff, 100 * sum(r["f_any"] for r in sel) / ff))
withc = [r for r in rows if r["cited_docs"]]
print("blocks citing a doc:", len(withc), "of", len(rows))
ratio = sorted(r["pointer_cost"] / max(1, r["bytes"]) for r in withc)
print("pointer cost / comment bytes: median %.1f p25 %.1f p75 %.1f max %.1f" % (ratio[len(ratio) // 2], ratio[len(ratio) // 4], ratio[3 * len(ratio) // 4], ratio[-1]))
print("mean pointer cost bytes %.0f, mean comment bytes %.0f" % (sum(r["pointer_cost"] for r in withc) / len(withc), sum(r["bytes"] for r in withc) / len(withc)))
# control: random sentences of the corpus's own prose from an unrelated domain would not fit here;
# instead shuffle: match each block's sentences against one fixed unrelated step's docs only
import random
rng = random.Random(3)
ctrl_docs = rng.sample(sorted(step_doc.values()), 12)
hit = tot = 0
for b in S:
    if b["stratum_area"] == "adocs_data":
        continue
    text = blob(b["file"])[b["start_off"] : b["end_off"]]
    for s in sentences(text):
        ws = words(s)
        if len(ws) < 6:
            continue
        tot += 1
        share, _ = best(ws, allowed=set(ctrl_docs))
        hit += share >= TH
print("control: sentences matching 12 random step files at the same threshold: %d of %d, %.1f%%" % (hit, tot, 100 * hit / max(1, tot)))
