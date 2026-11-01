#!/usr/bin/env python3
"""Comment-only commits on `achesso` since it left `bitboard`, by the study's lexer.

A commit counts when it changes at least one code file and every changed code file is identical
with comments stripped (whitespace runs collapsed, so a reflow does not count as code). Documents
may change beside it. Prints a TSV to stdout and totals to stderr. Read-only: git and the lexer.

Run from anywhere: python3 benefit_census.py > benefit_census.tsv
"""
import os
import re
import subprocess
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
sys.path.insert(0, os.path.join(REPO, "adocs/data/comment_study"))
from comment_lexer import strip_comments  # noqa: E402

BASE = "ca3602e"  # git merge-base achesso origin/bitboard
CODE = re.compile(r"\.(cpp|hpp|h|c|S|py|sh|cmake)$|(^|/)CMakeLists\.txt$")
EVIDENCE = re.compile(r"^(adocs/|tests/doctest|tests/json)")  # adocs/data scripts are records


def git(*args):
    return subprocess.run(["git", "-C", REPO, *args], capture_output=True).stdout.decode("utf-8", "replace")


def blob(ref, path):
    out = subprocess.run(["git", "-C", REPO, "show", f"{ref}:{path}"], capture_output=True)
    return out.stdout.decode("utf-8", "replace") if out.returncode == 0 else None


def stripped(path, text):
    try:
        body = strip_comments(path, text)
    except Exception:
        return None  # a file the lexer cannot read counts as code changed
    return re.sub(r"\s+", " ", body).strip()


def main():
    rows = []
    commits = git("rev-list", "--no-merges", "--reverse", f"{BASE}..achesso").split()
    for sha in commits:
        numstat = git("show", "--numstat", "--format=", "-M", sha).strip().split("\n")
        code = []
        for line in numstat:
            if not line.strip():
                continue
            added, removed, path = line.split("\t", 2)
            path = re.sub(r"\{.*? => (.*?)\}", r"\1", path).replace(" => ", "/")
            if CODE.search(path) and not EVIDENCE.search(path):
                code.append((path, added, removed))
        if not code:
            continue
        ok = True
        for path, _, _ in code:
            new = blob(sha, path)
            old = blob(sha + "^", path)
            if new is None or old is None:
                ok = False
                break
            a, b = stripped(path, old), stripped(path, new)
            if a is None or b is None or a != b:
                ok = False
                break
        if not ok:
            continue
        adds = sum(int(a) for _, a, _ in code if a != "-")
        rems = sum(int(r) for _, _, r in code if r != "-")
        head = git("log", "-1", "--date=short", "--format=%h\t%ad\t%s", sha).strip()
        src = sum(1 for p, _, _ in code if p.startswith("src/"))
        rows.append((head, adds, rems, len(code), src, ",".join(p for p, _, _ in code)[:120]))
    print("sha\tdate\tsubject\tadded\tremoved\tcode_files\tsrc_files\tfiles")
    for head, adds, rems, n, src, files in rows:
        print(f"{head}\t{adds}\t{rems}\t{n}\t{src}\t{files}")
    print(
        f"non-merge commits {len(commits)}; comment-only {len(rows)}; "
        f"lines added {sum(r[1] for r in rows)}, removed {sum(r[2] for r in rows)}",
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
