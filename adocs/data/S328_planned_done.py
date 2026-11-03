#!/usr/bin/env python3
"""List the audit findings whose `Status:` reads `planned` and whose steps are all done.

S328. A finding under adocs/audit/ is moved to `planned` when a step is written
to close it, and to `closed` when it is closed; not every line followed its
step. This script reads, on the tree it is pointed at:

  the findings  every `### <report>-Fnn` heading in adocs/audit/*.md, and every
                `**Fnn ` paragraph, the short form Part E of
                2026-09-10_adversarial takes, under that report's prefix; the
                finding's status is the first line starting `Status:` after
                its start and before the next finding. A fenced block is
                skipped, and so is the template heading `### <report>-F01` of
                a report's "Finding format" section, which still bounds the
                finding before it.
  the steps     the `closes:` line of every step file in adocs/plan_todo/,
                plan_current/ and plan_done/, each `<report>-Fnn` id on it.

Then it prints the census of the status words, each finding with no `Status:`
line and the steps that name it, and each `planned` finding in one of three
classes: every step that names it in `closes:` is in plan_done/;
some step that names it is not; no step names it. The first class is the one
S328 reads against its steps' `done:` stamps. Each `open` finding is printed
with the steps that name it, and a `closes:` id that names no finding too.

With `--against-rev REV` it also proves what DEC-346 admits and nothing more:
besides changed `Status:` lines, only a `Status:` line and an empty line
inserted together under a finding that had none, in `2026-09-10_adversarial.md`
alone. Per report, every finding whose `Status:` line is new since REV must be
in that report, must have had none at REV, and must take S134's placement: the
line on its own paragraph after the finding's text, an exactly empty line
before it and after it, and after that the next finding, a `---` rule or a
heading. A second `Status:` line under any finding is refused, and so is a
change in the count of `Status:` lines of any other report. With those pairs
taken out, the tree's report must be REV's line for
line, except where REV's line is a finding's `Status:` line and the tree's is a
`Status:` line too. "Empty" means exactly "\n", never whitespace. Every refusal
is printed with its reason, then a diff of the two with each `Status:` line's
text masked, and the run exits 1.

`--dotless-ids` reads a `closes:` id with no dot allowed in its prefix. That is
not the right grammar, since `2026-08-13_plan_review.2-F06` has one; it is here
because it reproduces the 60 of 72 S328's step file states for `1bb3080`, where
the right one reads 63: the three `plan_review.2` ids lose their prefix and
name no finding.

It reads and prints; it edits nothing.

    python3 adocs/data/S328_planned_done.py [--rev REV | --tree DIR] [--against-rev REV] [--dotless-ids]

The tree read is the working tree of the repository this script sits in, with
`--rev` that commit's `adocs/` through `git archive`, or with `--tree` a
directory holding an `adocs/` of its own, which is how a planted copy is read.
"""

import argparse
import contextlib
import difflib
import pathlib
import re
import subprocess
import sys
import tempfile

HEADING = re.compile(r"^### (\S+-F\d+)\b")
SHORT = re.compile(r"^\*\*(F\d+) ")
STATUS = re.compile(r"^Status:")
FINDING_ID = re.compile(r"[\w.\-]+-F\d+")
# DEC-346: the one report whose findings may take an inserted `Status:` line.
INSERTS_ADMITTED = {"2026-09-10_adversarial.md"}
DOTLESS_ID = re.compile(r"[\w\-]+-F\d+")
PLAN_DIRS = ("plan_todo", "plan_current", "plan_done")
REPO = pathlib.Path(__file__).resolve().parents[2]


@contextlib.contextmanager
def tree_at(rev):
    """The tree read: the working tree, or REV's `adocs/` extracted to a temporary directory."""
    if rev is None:
        yield REPO
        return
    with tempfile.TemporaryDirectory() as scratch:
        archive = subprocess.run(
            ["git", "-C", str(REPO), "archive", rev, "adocs/audit", *("adocs/" + d for d in PLAN_DIRS)],
            check=True, capture_output=True).stdout
        subprocess.run(["tar", "-x", "-C", scratch], input=archive, check=True)
        yield pathlib.Path(scratch)


def status_word(line):
    text = line[len("Status:"):].strip()
    word = re.match(r"[a-z]+", text)
    return word.group(0) if word else text


def finding_blocks(lines, stem):
    """Each finding of one report's lines: [id, start index, status index or None, next start index]."""
    prefix = None
    for line in lines:
        match = HEADING.match(line)
        if match and not match.group(1).startswith("<"):
            prefix = match.group(1).rsplit("-F", 1)[0]
            break
    if prefix is None:
        prefix = stem
    blocks = []
    current = None
    fenced = False
    for index, line in enumerate(lines):
        if line.startswith("```") or line.startswith("~~~"):
            fenced = not fenced
            continue
        if fenced:
            continue
        heading = HEADING.match(line)
        short = SHORT.match(line)
        if heading or short:
            if current is not None:
                current[3] = index
                blocks.append(current)
            if heading and heading.group(1).startswith("<"):
                current = None
            elif heading:
                current = [heading.group(1), index, None, None]
            else:
                current = [prefix + "-" + short.group(1), index, None, None]
            continue
        if current is not None and current[2] is None and STATUS.match(line):
            current[2] = index
    if current is not None:
        current[3] = len(lines)
        blocks.append(current)
    return blocks


def read_findings(report):
    """Each finding of one report: (id, start line, status line number, status text)."""
    lines = report.read_text().splitlines()
    return [[fid, start + 1, None if status is None else status + 1, None if status is None else lines[status]]
            for fid, start, status, _ in finding_blocks(lines, report.stem)]


def read_closes(root, grammar):
    """Finding id -> [(step id, plan directory)] from every step's `closes:` line."""
    closers = {}
    for directory in PLAN_DIRS:
        for step in sorted((root / "adocs" / directory).glob("S*.md")):
            for line in step.read_text().splitlines():
                if line.startswith("closes:"):
                    value = re.sub(r"<!--.*?-->", "", line[len("closes:"):])
                    for finding in grammar.findall(value):
                        closers.setdefault(finding, []).append(
                            (step.name.split("_", 1)[0], directory))
                    break
    return closers


def census(root, label, grammar):
    reports = sorted((root / "adocs" / "audit").glob("*.md"))
    findings = []
    for report in reports:
        for finding in read_findings(report):
            findings.append((report.name,) + tuple(finding))
    closers = read_closes(root, grammar)

    # Every `Status:` line outside a fence, attributed or not, so the census
    # can be held against a plain grep of the reports.
    all_status = {}
    for report in reports:
        fenced = False
        for line in report.read_text().splitlines():
            if line.startswith("```") or line.startswith("~~~"):
                fenced = not fenced
            elif not fenced and STATUS.match(line):
                word = status_word(line)
                all_status[word] = all_status.get(word, 0) + 1

    words = {}
    for finding in findings:
        if finding[4] is not None:
            word = status_word(finding[4])
            words[word] = words.get(word, 0) + 1
    no_status = [f for f in findings if f[4] is None]

    print(f"tree: {label}; closes: ids read as {grammar.pattern}")
    print(f"reports: {len(reports)}, findings: {len(findings)}, "
          f"with a Status: line: {len(findings) - len(no_status)}, without: {len(no_status)}")
    print("Status: lines outside fences, by first word: "
          + ", ".join(f"{w} {n}" for w, n in sorted(all_status.items())))
    print("findings' Status: lines, by first word:      "
          + ", ".join(f"{w} {n}" for w, n in sorted(words.items())))

    planned = [f for f in findings if f[4] is not None and status_word(f[4]) == "planned"]
    all_done, some_open, none = [], [], []
    for finding in planned:
        steps = closers.get(finding[1], [])
        if not steps:
            none.append((finding, steps))
        elif all(directory == "plan_done" for _, directory in steps):
            all_done.append((finding, steps))
        else:
            some_open.append((finding, steps))

    def show(title, rows):
        print()
        print(f"{title}: {len(rows)}")
        for (report, fid, start, number, line), steps in rows:
            named = ", ".join(f"{s} {d}" for s, d in steps) or "no step"
            print(f"  {fid}  {report}:{number}  [{named}]")
            print(f"    {line}")

    print()
    print(f"without a Status: line, with the steps naming it in closes:: {len(no_status)}")
    for report, fid, start, number, line in no_status:
        named = ", ".join(f"{s} {d}" for s, d in closers.get(fid, [])) or "no step"
        print(f"  {fid}  {report}:{start}  [{named}]")

    print()
    print(f"planned: {len(planned)}; every closing step in plan_done/: {len(all_done)} of {len(planned)}")
    show("planned, every step naming it in closes: is in plan_done/", all_done)
    show("planned, a step naming it is not in plan_done/", some_open)
    show("planned, no step names it in closes:", none)
    opened = [f for f in findings if f[4] is not None and status_word(f[4]) == "open"]
    show("open, with the steps naming it in closes:", [(f, closers.get(f[1], [])) for f in opened])

    known = {f[1] for f in findings}
    dangling = sorted((fid, steps) for fid, steps in closers.items() if fid not in known)
    print()
    print(f"closes: ids that name no finding: {len(dangling)}")
    for fid, steps in dangling:
        print(f"  {fid}  [{', '.join(f'{s} {d}' for s, d in steps)}]")


def refusals(name, old, new):
    """Why the tree's report is more than REV's with DEC-346's edits, and the counts: (reasons, changed, inserted)."""
    stem = name[:-len(".md")]
    reasons = []
    old_blocks = {b[0]: b for b in finding_blocks(old, stem)}
    new_blocks = finding_blocks(new, stem)
    taken = set()
    for fid, start, status, after in new_blocks:
        base = old_blocks.get(fid)
        if status is None or (base is not None and base[2] is not None):
            continue
        where = f"{fid}, line {status + 1}"
        if name not in INSERTS_ADMITTED:
            reasons.append(f"{where}: a Status: line inserted outside {sorted(INSERTS_ADMITTED)}")
        if base is None:
            reasons.append(f"{where}: a finding REV does not have")
        if not (status > 0 and new[status - 1] == "\n" and status + 1 < len(new) and new[status + 1] == "\n"):
            reasons.append(f"{where}: not its own paragraph between two exactly empty lines")
        elif not (status + 2 == after or status + 2 >= len(new) or new[status + 2].startswith(("---", "#"))):
            reasons.append(f"{where}: not after the finding's text, more of the finding follows it")
        taken.update((status, status + 1))
    fenced, inside = set(), False
    for index, line in enumerate(new):
        if line.startswith("```") or line.startswith("~~~"):
            inside = not inside
        elif inside:
            fenced.add(index)
    first = {b[2] for b in new_blocks if b[2] is not None}
    for index, line in enumerate(new):
        if STATUS.match(line) and index not in first and index not in fenced:
            owner = next((b[0] for b in new_blocks if b[1] <= index < b[3]), "no finding")
            reasons.append(f"line {index + 1}: a second Status: line, under {owner}")
    new_ids = {b[0] for b in new_blocks}
    for fid, base in old_blocks.items():
        if fid not in new_ids:
            reasons.append(f"{fid}: a finding REV has and the tree does not")
    if name not in INSERTS_ADMITTED:
        before = sum(1 for line in old if STATUS.match(line))
        after = sum(1 for line in new if STATUS.match(line))
        if before != after:
            reasons.append(f"{after - before:+d} Status: lines, outside {sorted(INSERTS_ADMITTED)}")
    reduced = [line for index, line in enumerate(new) if index not in taken]
    status_at_base = {b[2] for b in old_blocks.values() if b[2] is not None}
    changed = 0
    if len(reduced) != len(old):
        reasons.append(f"{len(reduced) - len(old):+d} lines besides the inserted pairs")
        left, right = masked(old), masked(reduced)
        matcher = difflib.SequenceMatcher(None, left, right, autojunk=False)
        tag, i1, i2, j1, j2 = next(op for op in matcher.get_opcodes() if op[0] != "equal")
        reasons.append(f"the first, Status: lines masked, at REV line {i1 + 1}: {tag} "
                       f"{[line.rstrip(chr(10))[:40] for line in left[i1:i2]]} -> "
                       f"{[line.rstrip(chr(10))[:40] for line in right[j1:j2]]}")
        return reasons, None, len(taken) // 2, reduced
    for index, (was, now) in enumerate(zip(old, reduced)):
        if was == now:
            continue
        if index in status_at_base and STATUS.match(now):
            changed += 1
            continue
        kind = "a finding's Status: line" if index in status_at_base else "a line that is not a finding's Status: line"
        column = next((c for c, (a, b) in enumerate(zip(was, now)) if a != b), min(len(was), len(now)))
        window = slice(max(0, column - 25), column + 25)
        reasons.append(f"REV line {index + 1}, {kind}, differs at column {column + 1}: "
                       f"{was[window].rstrip(chr(10))!r} -> {now[window].rstrip(chr(10))!r}")
    return reasons, changed, len(taken) // 2, reduced


def masked(lines):
    return ["Status: <masked>\n" if STATUS.match(line) else line for line in lines]


def against(root, label, old_root, old_label):
    """Exit status of the proof that the tree's reports differ from OLD_ROOT's only as DEC-346 admits."""
    print()
    print(f"the reports of {label} against {old_label}'s: changed Status: lines, and Status: lines inserted "
          f"with an empty line under a finding that had none, in {', '.join(sorted(INSERTS_ADMITTED))} alone:")
    held = True
    new_reports = sorted(p.name for p in (root / "adocs" / "audit").glob("*.md"))
    old_reports = sorted(p.name for p in (old_root / "adocs" / "audit").glob("*.md"))
    if new_reports != old_reports:
        print(f"  REFUSED: the report lists differ: {sorted(set(new_reports) ^ set(old_reports))}")
        held = False
    totals = [0, 0]
    for name in sorted(set(new_reports) & set(old_reports)):
        old = (old_root / "adocs" / "audit" / name).read_text().splitlines(keepends=True)
        new = (root / "adocs" / "audit" / name).read_text().splitlines(keepends=True)
        reasons, changed, inserted, reduced = refusals(name, old, new)
        rest = list(difflib.unified_diff(masked(old), masked(reduced),
                                         fromfile="old/" + name, tofile="new/" + name))
        verdict = "holds" if not reasons and not rest else "REFUSED"
        shown = "-" if changed is None else changed
        print(f"  {name}: lines {len(old)} -> {len(new)}, Status: lines changed {shown}, inserted {inserted}, "
              f"refusals {len(reasons)}, masked diff of the rest: {len(rest)} lines, {verdict}")
        for reason in reasons[:5]:
            print(f"    REFUSED: {reason}")
        if len(reasons) > 5:
            print(f"    ... and {len(reasons) - 5} more")
        sys.stdout.writelines("    " + line for line in rest[:20])
        totals[0] += changed or 0
        totals[1] += inserted
        if verdict != "holds":
            held = False
    print(f"  all reports: Status: lines changed {totals[0]}, inserted {totals[1]}")
    print("S328-REPORTS-STATUS-ONLY" if held else "S328-REPORTS-DIFFER")
    return 0 if held else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    source = parser.add_mutually_exclusive_group()
    source.add_argument("--rev", help="read this commit's adocs/ instead of the working tree")
    source.add_argument("--tree", type=pathlib.Path, help="read DIR/adocs/ instead of the working tree")
    parser.add_argument("--against-rev", metavar="REV",
                        help="prove the reports differ from REV's only as DEC-346 admits")
    parser.add_argument("--dotless-ids", action="store_true",
                        help="read closes: ids without the dot, the reading that reproduces S328's 60")
    args = parser.parse_args()
    label = args.rev if args.rev else f"the tree under {args.tree.name}" if args.tree else "the working tree"
    with (contextlib.nullcontext(args.tree) if args.tree else tree_at(args.rev)) as root:
        census(root, label, DOTLESS_ID if args.dotless_ids else FINDING_ID)
        if args.against_rev is None:
            return 0
        with tree_at(args.against_rev) as old_root:
            return against(root, label, old_root, args.against_rev)


if __name__ == "__main__":
    sys.exit(main())
