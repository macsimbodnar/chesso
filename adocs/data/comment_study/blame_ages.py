#!/usr/bin/env python3
"""Question 3: age of comment bytes and code bytes by `git blame -w`, one file at
a time. Writes blame_lines.jsonl.gz-free compact tables:
  blame_by_week.tsv  area, iso week, comment bytes, code bytes
  blame_blocks.jsonl per sampled block: oldest, newest, byte-weighted median date,
                     distinct commits
Blame date is the last commit that touched the line, so a rewrite looks young.
"""
import json
import subprocess
import sys
import time
from collections import Counter, defaultdict
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from comment_lexer import lex, line_starts

COMMIT = "5954c12"
REPO = "/home/max/ws/chesso"


def git(*a):
    return subprocess.run(["git", "-C", REPO, *a], capture_output=True, check=True).stdout


def blame(path):
    out = git("blame", "-w", "--line-porcelain", COMMIT, "--", path).decode("utf-8", "replace")
    res = []  # per line (sha, unix time)
    sha = None
    t = None
    for line in out.split("\n"):
        if line.startswith("\t"):
            res.append((sha, t))
        elif line.startswith("author-time "):
            t = int(line.split()[1])
        elif len(line) >= 40 and all(c in "0123456789abcdef" for c in line[:40]) and " " in line:
            sha = line[:40]
    return res


rows = [l.split("\t") for l in open("files.tsv").read().splitlines()]
cols = rows[0]
files = [dict(zip(cols, r)) for r in rows[1:]]
sample = [json.loads(l) for l in open("sample.jsonl")]
by_file = defaultdict(list)
for b in sample:
    by_file[b["file"]].append(b)

wk = defaultdict(lambda: [0, 0])
per_commit_comment = Counter()
blocks_out = []
start = time.time()
for f in files:
    path = f["file"]
    area = f["area"] if f["area"] not in ("cmake", "root") else "root+cmake"
    text = git("show", f"{COMMIT}:{path}").decode("utf-8")
    comments, code_lines = lex(path, text)
    bl = blame(path)
    starts = line_starts(text)
    lines = text.split("\n")
    line_comment = Counter()
    for c in comments:
        if c.kind == "shebang":
            continue
        for ln in range(c.start_line, c.end_line + 1):
            s = max(c.start_off, starts[ln - 1])
            e = min(c.end_off, starts[ln] if ln < len(starts) else len(text))
            line_comment[ln] += len(text[s:e].encode())
    for i, (sha, t) in enumerate(bl, 1):
        d = datetime.fromtimestamp(t, tz=timezone.utc)
        y, w, _ = d.isocalendar()
        key = (area, f"{y}-W{w:02d}")
        cb = line_comment.get(i, 0)
        total = len(lines[i - 1].encode()) + 1 if i - 1 < len(lines) else 0
        wk[key][0] += cb
        wk[key][1] += max(0, total - cb)
        if cb:
            per_commit_comment[sha] += cb
    for b in by_file.get(path, []):
        rec = []
        for ln in range(b["start_line"], b["end_line"] + 1):
            if ln - 1 < len(bl):
                rec.append((bl[ln - 1][1], bl[ln - 1][0], line_comment.get(ln, 0)))
        rec.sort()
        total = sum(r[2] for r in rec)
        acc = 0
        med = rec[0][0]
        for r in rec:
            acc += r[2]
            if acc * 2 >= total:
                med = r[0]
                break
        blocks_out.append(dict(id=b["id"], oldest=rec[0][0], newest=rec[-1][0], median=med,
                               commits=len({r[1] for r in rec})))
with open("blame_by_week.tsv", "w") as out:
    out.write("area\tweek\tcomment_bytes\tcode_bytes\n")
    for (a, w), v in sorted(wk.items()):
        out.write(f"{a}\t{w}\t{v[0]}\t{v[1]}\n")
with open("blame_blocks.jsonl", "w") as out:
    for r in blocks_out:
        out.write(json.dumps(r) + "\n")
with open("blame_commits.json", "w") as out:
    json.dump(per_commit_comment.most_common(40), out)
print("BLAME-DONE", round(time.time() - start, 1), "s")
