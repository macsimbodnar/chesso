#!/usr/bin/env python3
"""Measure the pilot: bytes and estimated tokens, code identity both ways."""
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).parent
sys.path.insert(0, str(HERE.parent))
from comment_lexer import lex, strip_comments

REPO = "/home/max/ws/chesso"
COMMIT = "5954c12"
PER_COMMENT_BYTE = 0.374  # measured, token_regression.json


def blob(p):
    return subprocess.run(["git", "-C", REPO, "show", f"{COMMIT}:{p}"], capture_output=True, check=True).stdout.decode()


def stats(path, text, start=None, end=None):
    lines = text.split("\n")
    cs, code = lex(path, text)
    cb = sum(len(text[c.start_off : c.end_off].encode()) for c in cs if c.kind != "shebang"
             and (start is None or start <= c.start_line <= end))
    cl = sum(c.end_line - c.start_line + 1 for c in cs if not c.trailing and (start is None or start <= c.start_line <= end))
    total = sum(len(l.encode()) + 1 for i, l in enumerate(lines, 1) if start is None or start <= i <= end)
    nlines = (end - start + 1) if start else len(lines)
    return dict(lines=nlines, bytes=total, comment_bytes=cb, comment_lines=cl)


def gpp(path_or_text, name):
    p = HERE / f".gpp_{name}.cpp"
    p.write_text(path_or_text)
    r = subprocess.run(["g++", "-fpreprocessed", "-dD", "-E", "-P", "-x", "c++", str(p)], capture_output=True, text=True)
    p.unlink()
    return r.stdout, r.returncode


def squeeze(s):
    return re.sub(r"\s+", "", s)


for name, path, out, section in (
    ("search.cpp section 1781-2089", "src/search.cpp", "search.cpp", (1781, 2089)),
    ("test_mate_carry.cpp whole", "tests/test_mate_carry.cpp", "test_mate_carry.cpp", None),
):
    before = blob(path)
    after = (HERE / out).read_text()
    if section:
        s0 = stats(path, before, *section)
        delta = len(before.split("\n")) - len(after.split("\n"))
        s1 = stats(path, after, section[0], section[1] - delta)
    else:
        s0 = stats(path, before)
        s1 = stats(path, after)
    print(f"== {name}")
    print("  before", s0)
    print("  after ", s1)
    b0, b1 = s0["bytes"], s1["bytes"]
    c0, c1 = s0["comment_bytes"], s1["comment_bytes"]
    print(f"  bytes {b0} -> {b1} ({100*(b1-b0)/b0:+.1f}%), comment bytes {c0} -> {c1} ({100*(c1-c0)/c0:+.1f}%)")
    print(f"  tokens chars/4: {b0//4} -> {b1//4}; comment tokens at the measured {PER_COMMENT_BYTE}/byte: {int(c0*PER_COMMENT_BYTE)} -> {int(c1*PER_COMMENT_BYTE)}")
    a = strip_comments(path, before)
    b = strip_comments(path, after)
    print("  lexer strip identical:", a == b, "whitespace-free identical:", squeeze(a) == squeeze(b))
    g0, r0 = gpp(before, "before")
    g1, r1 = gpp(after, "after")
    print("  g++ -fpreprocessed -dD -E -P rc", r0, r1, "identical:", g0 == g1, "bytes", len(g0), len(g1))
    if a != b:
        ad, bd = a.split("\n"), b.split("\n")
        for i, (x, y) in enumerate(zip(ad, bd)):
            if x != y:
                print("  first lexer diff at stripped line", i, repr(x[:80]), repr(y[:80]))
                break
