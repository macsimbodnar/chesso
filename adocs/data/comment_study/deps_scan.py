#!/usr/bin/env python3
"""Question 6 and 7 measurements, read-only over git objects at the fixed commit:
(a) comment-only share of recent src/ and tests/ diffs, exact by lexing both images;
(b) mutant anchors that overlap a comment, or that only stay unique because of one;
(c) pending step file citations whose phrase lives only in a comment;
(d) line-form citations in the documents;
(e) tests and tools that read source text.
"""
import json
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from comment_lexer import language_of, lex, line_starts, strip_comments

COMMIT = "5954c12"
REPO = "/home/max/ws/chesso"


def git(*a, check=True):
    r = subprocess.run(["git", "-C", REPO, *a], capture_output=True)
    if check and r.returncode:
        raise RuntimeError(r.stderr.decode())
    return r.stdout


cache = {}


def blob(rev, path):
    k = (rev, path)
    if k not in cache:
        r = subprocess.run(["git", "-C", REPO, "show", f"{rev}:{path}"], capture_output=True)
        cache[k] = r.stdout.decode("utf-8", "replace") if r.returncode == 0 else None
    return cache[k]


def comment_only_lines(path, text):
    """1-based line numbers that hold only comment."""
    try:
        comments, code = lex(path, text)
    except Exception:
        return None
    out = set()
    for c in comments:
        for ln in range(c.start_line, c.end_line + 1):
            if ln not in code:
                out.add(ln)
    return out


HUNK = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")


def diff_share(area_paths, n_commits):
    shas = git("log", f"-n{n_commits}", "--format=%H", COMMIT, "--", *area_paths).decode().split()
    totals = Counter()
    per_commit = []
    by_area = Counter()
    for sha in shas:
        files = git("show", "--format=", "--name-only", sha, "--", *area_paths).decode().split()
        ca = cr = xa = xr = 0
        for f in files:
            if not language_of(f):
                continue
            post = blob(sha, f)
            pre = blob(sha + "^", f)
            post_c = comment_only_lines(f, post) if post is not None else set()
            pre_c = comment_only_lines(f, pre) if pre is not None else set()
            if post_c is None or pre_c is None:
                continue
            d = git("show", "--format=", "-U0", "--no-color", sha, "--", f).decode("utf-8", "replace")
            old_ln = new_ln = 0
            for line in d.split("\n"):
                m = HUNK.match(line)
                if m:
                    old_ln = int(m.group(1))
                    new_ln = int(m.group(3))
                    continue
                if line.startswith(("diff ", "index ", "--- ", "+++ ")):
                    continue
                if line.startswith("+"):
                    if new_ln in post_c:
                        ca += 1
                    elif line[1:].strip():
                        xa += 1
                    new_ln += 1
                elif line.startswith("-"):
                    if old_ln in pre_c:
                        cr += 1
                    elif line[1:].strip():
                        xr += 1
                    old_ln += 1
        totals["comment_added"] += ca
        totals["comment_removed"] += cr
        totals["code_added"] += xa
        totals["code_removed"] += xr
        per_commit.append((sha[:7], ca, cr, xa, xr))
    return totals, per_commit


res = {}
for name, paths in (("src", ["src"]), ("tests", ["tests"]), ("tools", ["tools"])):
    t, pc = diff_share(paths, 150)
    only_comment = [p for p in pc if p[3] == 0 and p[4] == 0 and (p[1] or p[2])]
    res[name] = dict(totals=t, commits=len(pc), comment_only_commits=len(only_comment),
                     churn_share=(t["comment_added"] + t["comment_removed"]) / max(1, sum(t.values())))
    print(name, dict(t), "commits", len(pc), "comment-only commits", len(only_comment),
          f"comment share of churn {100*res[name]['churn_share']:.1f}%")

# Mutants: run each file with a stub `m`, as mutation_check.py execs them.
anchors = []


def stub_factory(srcfile):
    def m(mid, file, *rest, **kw):
        for r in rest:
            if isinstance(r, tuple) and len(r) >= 2:
                anchors.append((srcfile, mid, file, r[0], r[1]))
    return m


paths = git("ls-tree", "-r", "--name-only", COMMIT, "tools/mutants").decode().split()
for p in paths:
    if not p.endswith(".py"):
        continue
    src = blob(COMMIT, p)
    env = {"m": stub_factory(p), "__name__": "mutants"}
    try:
        exec(compile(src, p, "exec"), env)
    except Exception as e:
        print("cannot exec", p, type(e).__name__, str(e)[:80])
print("anchors", len(anchors), "mutant files", len(paths))
stats = Counter()
files_hit = Counter()
examples = []
comment_spans = {}
for srcfile, mid, file, old, new in anchors:
    text = blob(COMMIT, file)
    if text is None:
        stats["target_missing"] += 1
        continue
    n = text.count(old)
    if n == 0:
        stats["not_found_at_head"] += 1
        continue
    if n > 1:
        stats["ambiguous_at_head"] += 1
    stats["found"] += 1
    if file not in comment_spans:
        cs, _ = lex(file, text)
        comment_spans[file] = [(c.start_off, c.end_off) for c in cs]
    i = text.find(old)
    j = i + len(old)
    overlap = any(a < j and b > i for a, b in comment_spans[file])
    if overlap:
        stats["anchor_overlaps_comment"] += 1
        files_hit[file] += 1
        examples.append((srcfile, mid, file))
    if "//" in old or "/*" in old:
        stats["anchor_text_has_comment_marker"] += 1
print("anchor stats", dict(stats), "by target", files_hit.most_common(), "examples", examples[:12])
res["anchors"] = dict(total=len(anchors), stats=stats, files=files_hit, examples=examples)

# Citations in pending step files: use the checker's own notions, approximated.
# A phrase is the double-quoted text after a backticked path.
PATH = re.compile(r"`((?:src|tests|tools|cmake)/[A-Za-z0-9_./-]+)`\s*(?:'s\s*)?(?:the\s+)?\"([^\"\n]{6,120})\"")
SYM = re.compile(r"`((?:src|tests|tools|cmake)/[A-Za-z0-9_./-]+)`\s*(?:'s\s*)?`([^`\n]{2,80})`")
LINEF = re.compile(r"`?((?:src|tests|tools|cmake)/[A-Za-z0-9_./-]+\.[a-z]+):(\d+)")
pending = [p for p in git("ls-tree", "-r", "--name-only", COMMIT, "adocs/plan_todo", "adocs/plan_current").decode().split()]
cit = Counter()
only_comment = []
def flatten(s):
    return re.sub(r"\s+", " ", s)
for p in pending:
    t = blob(COMMIT, p)
    for m in PATH.finditer(t):
        f, phrase = m.group(1), m.group(2)
        body = blob(COMMIT, f)
        cit["phrase_citations"] += 1
        if body is None:
            cit["phrase_file_missing"] += 1
            continue
        fp = flatten(phrase)
        if fp in flatten(body):
            if language_of(f) and fp not in flatten(strip_comments(f, body)):
                cit["phrase_only_in_comment"] += 1
                only_comment.append((p, f, phrase[:60]))
        else:
            cit["phrase_not_literal"] += 1
    for m in SYM.finditer(t):
        cit["symbol_citations"] += 1
    for m in LINEF.finditer(t):
        cit["line_citations"] += 1
print("pending citations", dict(cit))
for x in only_comment[:15]:
    print("   ", x)
res["citations"] = dict(counts=cit, only_comment=only_comment)

# Line and phrase citations in the documents
docs = ["adocs/specs.md", "adocs/plan.md", "adocs/status.md", "adocs/decisions.md", "DEV_MANUAL.md", "MANUAL.md",
        "TOOLCHAIN.md", "CLAUDE.md", "AGENTS.md", "adocs/testing_strategy.md", "adocs/eval_tuning_strategy.md"]
LF2 = re.compile(r"(?:src|tests|tools)/[A-Za-z0-9_./-]+\.[a-z]+:\d+")
for d in docs:
    t = blob(COMMIT, d)
    if t is None:
        continue
    n = len(LF2.findall(t))
    c = len(re.findall(r"\bcomment", t, re.I))
    print(f"doc {d}: line citations {n}, word 'comment' {c}")
done = [p for p in git("ls-tree", "-r", "--name-only", COMMIT, "adocs/plan_done").decode().split()]
ln = cmt = 0
for p in done:
    t = blob(COMMIT, p)
    ln += len(LF2.findall(t))
    cmt += len(re.findall(r"\bcomment", t, re.I))
print(f"plan_done {len(done)} files: line citations {ln}, word 'comment' {cmt}")
json.dump(res, open("deps_scan.json", "w"), default=lambda o: dict(o) if isinstance(o, Counter) else str(o), indent=0)
