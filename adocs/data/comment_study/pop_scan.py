#!/usr/bin/env python3
"""Population-level, regex-only cross-check of Q2 over every comment block, plus
automatic stale probes: dead paths, file:line citations, ids that do not resolve,
forward-looking sentences about steps that are already done, doc paragraph numbers."""
import json
import re
import subprocess
from collections import Counter, defaultdict

COMMIT = "5954c12"
REPO = "/home/max/ws/chesso"


def git(*a):
    return subprocess.run(["git", "-C", REPO, *a], capture_output=True, check=True).stdout


tree = set(git("ls-tree", "-r", "--name-only", COMMIT).decode().splitlines())
steps = {}
for p in tree:
    m = re.match(r"adocs/plan_(done|todo|current)/(S\d+)_", p)
    if m:
        steps[m.group(2)] = m.group(1)
dec_text = git("show", f"{COMMIT}:adocs/decisions.md").decode()
decs = set(re.findall(r"^## (DEC-\d+)", dec_text, re.M))

SID = re.compile(r"\bS\d{3}\b")
DEC = re.compile(r"\bDEC-\d{3}\b")
INV = re.compile(r"\bINV-\d+\b")
AUD = re.compile(r"\b20\d\d-\d\d-\d\d[_ -][a-z_-]*-?F\d+\b")
DATE = re.compile(r"\b20\d\d-\d\d-\d\d\b")
NUM = re.compile(r"(?<![\w.])\d{3,}(?:\.\d+)?\b|\b\d+\.\d+\b")
MEAS = re.compile(r"\b(measured|measures|nElo|Elo|LLR|census|H0|H1|verdict|SPRT|ns\b|instructions|bench)\b", re.I)
PAST = re.compile(r"\b(until S\d+|before S\d+|since S\d+|used to|was the|were the|no longer|the old |at S\d+|first form|re-mined|re-derived|re-taken|moved to)\b", re.I)
GOLD = re.compile(r"\bGOLDEN\b|\bGolden\b|DEC-142")
SEED = re.compile(r"DEC-134|DEC-105|DEC-221|\bseed\b", re.I)
PATH = re.compile(r"(?<![\w./-])((?:src|tests|tools|adocs|cmake|books)/[A-Za-z0-9_./+-]+[A-Za-z0-9_])")
LINEREF = re.compile(r"(?<![\w./-])((?:src|tests|tools|adocs|cmake)/[A-Za-z0-9_./-]+\.[a-z]+|[a-z_]+\.(?:sh|py|cpp|hpp)):(\d+)(?:-(\d+))?")
PAR = re.compile(r"AGENTS\.md(?:'s)? (?:par\.\s?\d+|section \d+)")
FORWARD = re.compile(r"(S\d{3})(?:'s)?[^.\n]{0,40}\b(pending|owns|will|is planned|to come|when it lands|still to|not yet)\b|\b(pending|will|owned by)\b[^.\n]{0,25}(S\d{3})")

cache = {}


def blob(p):
    if p not in cache:
        cache[p] = git("show", f"{COMMIT}:{p}").decode()
    return cache[p]


agg = defaultdict(Counter)
dead_paths = Counter()
line_refs = []
dead_ids = Counter()
par_refs = []
forward_done = []
id_sizes = Counter()
for line in open("blocks.jsonl"):
    b = json.loads(line)
    t = blob(b["file"])[b["start_off"] : b["end_off"]]
    n = b["comment_bytes"]
    a = b["area"] if b["area"] not in ("cmake", "root") else "root+cmake"
    agg[a]["blocks"] += 1
    agg[a]["bytes"] += n
    sids = SID.findall(t)
    dids = DEC.findall(t)
    iids = INV.findall(t)
    aud = AUD.findall(t)
    idbytes = sum(map(len, sids)) + sum(map(len, dids)) + sum(map(len, iids)) + sum(map(len, aud))
    agg[a]["id_token_bytes"] += idbytes
    if sids or dids or iids or aud:
        agg[a]["blocks_with_id"] += 1
        agg[a]["bytes_in_blocks_with_id"] += n
    if sids:
        agg[a]["blocks_with_step"] += 1
    if dids:
        agg[a]["blocks_with_dec"] += 1
    if DATE.search(t):
        agg[a]["blocks_with_date"] += 1
        agg[a]["bytes_in_blocks_with_date"] += n
    if MEAS.search(t) and NUM.search(t):
        agg[a]["blocks_with_measure"] += 1
        agg[a]["bytes_in_blocks_with_measure"] += n
    if PAST.search(t):
        agg[a]["blocks_with_past"] += 1
        agg[a]["bytes_in_blocks_with_past"] += n
    if GOLD.search(t):
        agg[a]["blocks_with_golden"] += 1
        agg[a]["bytes_in_blocks_with_golden"] += n
    if SEED.search(t):
        agg[a]["blocks_with_seed"] += 1
    is_evidence = b["area"] == "adocs_data"
    for s in sids:
        if s not in steps:
            dead_ids[(a, s)] += 1
    for d in dids:
        if d not in decs:
            dead_ids[(a, d)] += 1
    for m in PATH.finditer(t):
        p = m.group(1).rstrip("/.")
        if p not in tree and not any(x.startswith(p + "/") for x in tree):
            if "<" in p or "*" in p or ".tuning" in p:
                continue
            dead_paths[(a, p)] += 1
            agg[a]["dead_paths"] += 1
    for m in LINEREF.finditer(t):
        line_refs.append((b["file"], b["start_line"], m.group(0)))
        agg[a]["line_citations"] += 1
    for m in PAR.finditer(t):
        par_refs.append((b["file"], b["start_line"], m.group(0)))
        agg[a]["agents_par"] += 1
    for m in FORWARD.finditer(t):
        s = m.group(1) or m.group(4)
        if steps.get(s) == "done":
            forward_done.append((a, b["file"], b["start_line"], m.group(0)[:110].replace("\n", " ")))
            agg[a]["forward_about_done"] += 1
with open("pop_scan.json", "w") as f:
    json.dump(
        dict(
            agg={k: dict(v) for k, v in agg.items()},
            dead_paths=[[k[0], k[1], v] for k, v in dead_paths.most_common()],
            line_refs=line_refs,
            dead_ids=[[k[0], k[1], v] for k, v in dead_ids.most_common()],
            par_refs=par_refs,
            forward_done=forward_done,
        ),
        f,
        indent=0,
    )
tot = Counter()
for a, c in agg.items():
    tot.update(c)
    print(a, dict(c))
print("TOTAL", dict(tot))
print("dead ids", len(dead_ids), list(dead_ids.most_common(12)))
print("dead paths", len(dead_paths), [(k, v) for k, v in dead_paths.most_common(14)])
print("line refs", len(line_refs), line_refs[:8])
print("par refs", len(par_refs), par_refs[:6])
print("forward about done", len(forward_done))
for x in forward_done[:14]:
    print("  ", x)
