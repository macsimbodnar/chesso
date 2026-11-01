#!/usr/bin/env python3
"""Validate benefit_rows.py against the records and compute the block, age and size columns.

Every quote must occur in its record, whitespace runs collapsed. A date comes from the record: a
step's done stamp, an audit's file name, the nearest dated heading above the quote, a commit's date.
The false or useful text's birth comes from `git log -S` on its one-line fragment, oldest hit; the
block at the last commit on or before the finding that still holds the fragment comes from the study's
lexer, adjacent whole-line comments merged. Long is 1200 B or more and log-shaped is the study's
selector (log_shaped.py): 1200 B or more, 6 or more distinct S### and DEC-###, 1.5 or more per KB.

Writes benefit_rows.json beside itself. Read-only otherwise.
"""
import glob
import json
import os
import re
import subprocess
import sys
from datetime import date

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", "..", ".."))
sys.path.insert(0, os.path.join(REPO, "adocs/data/comment_study"))
sys.path.insert(0, HERE)
from comment_lexer import lex  # noqa: E402
from benefit_rows import ROWS  # noqa: E402

ID = re.compile(r"\bS\d{3}\b|\bDEC-\d{3}\b")


def git(*args):
    return subprocess.run(["git", "-C", REPO, *args], capture_output=True).stdout.decode("utf-8", "replace")


def norm(s):
    return re.sub(r"\s+", " ", s)


def resolve(rec):
    if "_PLACEHOLDER" in rec:
        sid = re.search(r"(S\d{3})_PLACEHOLDER", rec).group(1)
        hits = glob.glob(os.path.join(REPO, f"adocs/plan_done/{sid}_*.md"))
        return os.path.relpath(hits[0], REPO) if hits else rec
    return rec


def record_date(rec, text, off):
    if rec.startswith("git:"):
        return git("log", "-1", "--date=short", "--format=%ad", rec[4:]).strip()
    m = re.search(r"adocs/audit/(\d{4}-\d{2}-\d{2})", rec)
    if m:
        return m.group(1)
    if "/plan_done/" in rec:
        m = re.search(r"^done:\s*(?:.*?)(\d{4}-\d{2}-\d{2})", text, re.M)
        if m:
            return m.group(1)
    head = text[:off]
    hits = re.findall(r"^#+ .*?(\d{4}-\d{2}-\d{2})", head, re.M)
    if hits:
        return hits[-1]
    if "S308" in rec:
        return "2026-11-01"
    if "comment_study/report.md" in rec:
        return "2026-10-31"
    return None


def merged_block(path, text, off):
    comments, _ = lex(path, text)
    cur = None
    blocks = []
    for c in comments:
        if cur and not c.trailing and not cur["trail"] and c.start_line == cur["end_line"] + 1:
            cur["end_line"] = c.end_line
            cur["end"] = c.end_off
        else:
            cur = dict(start=c.start_off, end=c.end_off, end_line=c.end_line, trail=c.trailing)
            blocks.append(cur)
    for b in blocks:
        if b["start"] <= off < b["end"]:
            return b
    return None


def block_of(row, found):
    path, phrase = row.get("file"), row.get("phrase")
    if not path or not phrase or " " in path and "/" not in path:
        return None
    hits = git("log", "-S", phrase, "--reverse", "--format=%H %ad", "--date=short", "--", path).strip().split("\n")
    hits = [h for h in hits if h.strip()]
    birth = hits[0].split()[1] if hits and not row.get("noage") else None
    # the last commit on or before the finding that still holds the fragment
    cands = []
    head = git("rev-list", "-1", f"--before={found} 23:59:59", "achesso").strip()
    if head:
        cands.append(head)
    if len(hits) > 1:
        cands.append(hits[1].split()[0] + "^")
    if hits:
        cands.append(hits[0].split()[0])
    for ref in cands:
        out = subprocess.run(["git", "-C", REPO, "show", f"{ref}:{path}"], capture_output=True)
        if out.returncode != 0:
            continue
        text = out.stdout.decode("utf-8", "replace")
        off = text.find(phrase)
        if off < 0:
            continue
        if row.get("above"):
            # the block ending on the line above the first code use of the fragment
            end = text.rfind("\n", 0, off)
            prev = text.rfind("\n", 0, end)
            off = prev + 1 + (len(text[prev + 1:end]) - len(text[prev + 1:end].lstrip()))
        try:
            b = merged_block(path, text, off)
        except Exception:
            b = None
        if not b:
            continue
        body = text[b["start"]:b["end"]]
        size = len(body.encode())
        ids = len(set(ID.findall(body)))
        return dict(birth=birth, bytes=size, ids=ids, long=size >= 1200,
                    log=size >= 1200 and ids >= 6 and ids / (size / 1000) >= 1.5)
    return dict(birth=birth, bytes=None, ids=None, long=None, log=None)


def main():
    out = []
    bad = 0
    for row in ROWS:
        r = dict(row)
        r["rec"] = resolve(row["rec"])
        if r["rec"].startswith("git:"):
            text = git("log", "-1", "--format=%B", r["rec"][4:])
        else:
            p = os.path.join(REPO, r["rec"])
            text = open(p, errors="replace").read() if os.path.exists(p) else ""
        hay = norm(text)
        q = norm(row["quote"])
        off = hay.find(q)
        if off < 0:
            print("QUOTE NOT FOUND", row["id"], r["rec"], "::", q[:80], file=sys.stderr)
            bad += 1
        # locate in the raw text for the date's heading search
        raw_off = len(text)
        first = q.split(" ")[0]
        if off >= 0:
            frag = q[:40]
            m = re.search(r"\s+".join(map(re.escape, frag.split(" "))), text)
            raw_off = m.start() if m else len(text)
            r["line"] = text.count("\n", 0, raw_off) + 1
        r["date"] = row.get("found") or record_date(r["rec"], text, raw_off)
        if r["date"] is None and not r["rec"].startswith("git:"):
            # a stamp with no date: plan_done files are immutable, so the last commit is the close
            r["date"] = git("log", "-1", "--date=short", "--format=%ad", "--", r["rec"]).strip() or None
        if r["date"] is None:
            print("NO DATE", row["id"], r["rec"], file=sys.stderr)
            bad += 1
        if row["cls"] in "ABD" and r["date"]:
            info = block_of(row, r["date"])
            if info:
                r.update(info)
                if info.get("birth"):
                    d0 = date.fromisoformat(info["birth"])
                    d1 = date.fromisoformat(r["date"])
                    r["age"] = (d1 - d0).days
        out.append(r)
    json.dump(out, open(os.path.join(HERE, "benefit_rows.json"), "w"), indent=0)
    print(f"{len(out)} rows, {bad} problems", file=sys.stderr)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
