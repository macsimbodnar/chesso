#!/usr/bin/env python3
"""Question 1: per-file and per-area comment census at a fixed commit.

Reads blobs through `git show`, so the working tree does not matter. Writes
files.tsv (one row per file) and blocks.jsonl (one record per comment block).
A block is a run of adjacent whole-line comments, or a single trailing comment,
or a block comment, or a docstring.
"""
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from comment_lexer import language_of, lex, if0_regions, line_starts

COMMIT = "5954c12"
REPO = "/home/max/ws/chesso"
OUT = Path(__file__).parent
ROOT_SCRIPTS = {"build_release.sh", "clang-format.sh", "fastchess.sh", "rating.sh", "test_uci.sh"}
# vendored, not this project's comments; named in the report as excluded
EXCLUDE_PREFIX = ("tests/third_party/", "tests/doctest")


def git(*args):
    return subprocess.run(
        ["git", "-C", REPO, *args], check=True, capture_output=True
    ).stdout


def area_of(path):
    if path.startswith("src/"):
        return "src"
    if path.startswith("tests/"):
        return "tests"
    if path.startswith("tools/"):
        return "tools"
    if path.startswith("adocs/data/"):
        return "adocs_data"
    if path.startswith("cmake/"):
        return "cmake"
    if path in ROOT_SCRIPTS or path == "CMakeLists.txt":
        return "root"
    return None


def blockify(path, text, comments):
    """Merge adjacent whole-line comments into blocks."""
    starts = line_starts(text)
    blocks = []
    cur = None
    for cm in comments:
        if cm.kind == "shebang":
            continue
        body = text[cm.start_off : cm.end_off]
        whole_line = not cm.trailing
        if (
            whole_line
            and cm.kind == "line"
            and cur is not None
            and cur["mergeable"]
            and cm.start_line == cur["end_line"] + 1
        ):
            cur["end_line"] = cm.end_line
            cur["end_off"] = cm.end_off
            cur["comment_bytes"] += len(body.encode())
            cur["n_comments"] += 1
            continue
        cur = {
            "file": path,
            "start_line": cm.start_line,
            "end_line": cm.end_line,
            "start_off": cm.start_off,
            "end_off": cm.end_off,
            "kind": cm.kind,
            "trailing": cm.trailing,
            "comment_bytes": len(body.encode()),
            "n_comments": 1,
            "mergeable": whole_line and cm.kind == "line",
        }
        blocks.append(cur)
    for b in blocks:
        del b["mergeable"]
        # the removable span: comment chars plus indentation and newline when whole-line
        if not b["trailing"]:
            s = starts[b["start_line"] - 1]
            nl = text.find("\n", b["end_off"])
            e = nl + 1 if nl != -1 else len(text)
            b["removable_bytes"] = len(text[s:e].encode())
        else:
            b["removable_bytes"] = b["comment_bytes"]
    return blocks


def main():
    paths = git("ls-tree", "-r", "--name-only", COMMIT).decode().splitlines()
    rows = []
    all_blocks = []
    skipped = []
    with open(OUT / "blocks.jsonl", "w") as bf:
        for path in paths:
            area = area_of(path)
            if area is None or path.startswith(EXCLUDE_PREFIX):
                continue
            lang = language_of(path)
            if lang is None:
                skipped.append(path)
                continue
            raw = git("show", f"{COMMIT}:{path}")
            try:
                text = raw.decode("utf-8")
            except UnicodeDecodeError:
                skipped.append(path + " (not utf-8)")
                continue
            try:
                comments, code_lines = lex(path, text)
            except Exception as exc:  # recorded per file in the report, never hidden
                skipped.append(f"{path} (lex error: {exc})")
                continue
            lines = text.split("\n")
            n_lines = len(lines) - (1 if text.endswith("\n") else 0)
            comment_lines = set()
            doc_lines = set()
            for cm in comments:
                if cm.kind == "shebang":
                    code_lines.add(cm.start_line)
                    continue
                for ln in range(cm.start_line, cm.end_line + 1):
                    if ln not in code_lines:
                        comment_lines.add(ln)
                        if cm.kind == "docstring":
                            doc_lines.add(ln)
            blank = sum(
                1
                for i, ln in enumerate(lines[:n_lines], 1)
                if not ln.strip() and i not in comment_lines and i not in code_lines
            )
            blocks = blockify(path, text, comments)
            cb = sum(b["comment_bytes"] for b in blocks)
            rb = sum(b["removable_bytes"] for b in blocks)
            trailing_bytes = sum(b["comment_bytes"] for b in blocks if b["trailing"])
            doc_bytes = sum(b["comment_bytes"] for b in blocks if b["kind"] == "docstring")
            rows.append(
                dict(
                    file=path,
                    area=area,
                    lang=lang,
                    bytes=len(raw),
                    lines=n_lines,
                    code_lines=len(code_lines),
                    comment_lines=len(comment_lines),
                    blank_lines=blank,
                    trailing_comments=sum(1 for b in blocks if b["trailing"]),
                    blocks=len(blocks),
                    comment_bytes=cb,
                    removable_bytes=rb,
                    trailing_bytes=trailing_bytes,
                    docstring_bytes=doc_bytes,
                    if0=";".join(f"{a}-{b}" for a, b in if0_regions(text)) if lang == "cpp" else "",
                )
            )
            for b in blocks:
                b["area"] = area
                b["lang"] = lang
                bf.write(json.dumps(b) + "\n")
                all_blocks.append(b)
    cols = list(rows[0].keys())
    with open(OUT / "files.tsv", "w") as f:
        f.write("\t".join(cols) + "\n")
        for r in rows:
            f.write("\t".join(str(r[c]) for c in cols) + "\n")
    with open(OUT / "skipped.txt", "w") as f:
        f.write("\n".join(skipped) + "\n")
    print(f"files {len(rows)} blocks {len(all_blocks)} skipped {len(skipped)}")


if __name__ == "__main__":
    main()
