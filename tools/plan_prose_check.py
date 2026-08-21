#!/usr/bin/env python3
"""Plan hygiene: stale tense in plan.md's prose, stale citations in step files.

Two checks over the plan documents, neither in the ctest suite. They report,
they do not rewrite: which tense a sentence should take and which line a
citation meant are judgements, and the fix belongs in the same commit as the
landing that made it stale.

    tools/plan_prose_check.py             # both checks
    tools/plan_prose_check.py --prose     # plan.md tense only
    tools/plan_prose_check.py --citations # pending step files only
    tools/plan_prose_check.py --prose adocs/plan.md          # explicit files
    tools/plan_prose_check.py --citations adocs/plan_todo/S091_*.md

Exits non-zero when either check flags anything.

**--prose: a completed step described as pending.** plan.md is second in the
reading order and its prose is what a cold session reads before the ordered
list. The list is maintained by the workflow checker; the prose around it is
not, so it goes stale every time a step completes. That happened three times:
2026-08-13_plan_review-F05, then 2026-08-13_plan_review.2-F07 after S048 cleared
it, then again the moment S033 completed. S062 is the third repair.

Sentence-wise, not line-wise. The prose is hard-wrapped, so an id and the claim
about it routinely sit on different lines; a line-based version of this check
missed two of the five stale claims S062 found.

**--citations: a file:line citation that no longer holds what it is cited
for.** A step file's citations are written once and the source moves under them
at every landing. 2026-08-20_plan_review-F01 measured 70 of 147 stale, shifted
by up to 613 lines, and six of those had come to rest inside an unrelated test:
five pending steps that each add pruning or a reduction told their implementer
to extend the mate-safety gate at tests/test_search.cpp:1274, which by then was
inside "a mate bound is compared after the ply adjustment, not before". The
symptom of extending the wrong mate test is a strength regression, not a red
test, so the class is worth a check rather than a repair. S138 is the repair and
this is the check.

Three failures, each exact and none a judgement:

  BOUNDS  the cited path does not exist at HEAD, or the cited line is past the
          end of the file. Catches a citation that was never right.
  ANCHOR  a doctest title quoted beside the citation is not the test the cited
          lines open. This is the check that makes a citation survivable: a
          line number is a moving reference and a TEST_CASE title is a stable
          one, so a citation carrying both degrades to something still findable
          instead of to something silently wrong.
  DRIFT   the cited lines hold different text now than they held in the commit
          that last wrote the step file. Exact string comparison, so it reports
          the class F01 found and nothing else.

Citations resolve against the working tree, which is what an implementer
actually opens; DRIFT's "then" side comes from git. A step file with no commit
of its own has no baseline and is checked for BOUNDS and ANCHOR only.

The grammar is the one the step files already use, not a new one: a full
`path:line` or `path:line-line`, with the path either rooted or given as a bare
basename when that basename is unique in the repository.

A bare `:line` continuation -- `(src/evaluation.cpp:951 mobility, :953 king
safety)` -- is counted and skipped, because in these files it is not
mechanically resolvable and guessing is worse than abstaining. Two measured
reasons, both from the first runs of this check. S095 writes "returns nullptr
on a miss (transposition_table.cpp:96-103), and :449 already computes", where
:449 means src/search.cpp and the nearest named path is the other file. And
S098's Stockfish-reading section carries a page of continuations whose subject
was named paragraphs earlier, so :662 and :701-724 resolved onto a 288-line
header. Between them they produced sixty impossible line numbers, and a check
that prints garbage gets ignored rather than acted on. The repair pushes the
other way: a citation that has to be survivable is written as a full path with
a title beside it, which is exactly what brings it inside this check.

Citations into `adocs/` and other `.md` files are counted and printed but do
not fail the run. Those documents are rewritten at every completion, by design,
so a line citation into one is stale by construction; the durable fix is to
cite a section or an id, and gating on them would make this check permanently
red and therefore ignored.
"""
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PLAN_DIRS = ("plan_done", "plan_current", "plan_todo")

# Claims that only make sense about work that has not happened yet. Extend this
# when a new phrasing slips through, rather than loosening what it matches.
#
# "lead what is left" is here because it slipped through inside S062 itself: the
# sentence was written while S062 was current and went stale the moment the
# checker moved it to plan_done/, in the same minute. A sentence that states a
# live position in the queue goes stale by construction; the durable fix is to
# write the placement as history and let the list carry the order.
PENDING = re.compile(
    r"is next|are next|still pending|everything pending|"
    r"at the front of the pending|is at the front|front of the pending list|"
    r"pending order|leads the pending|lead what is left|leads what is left|"
    r"will be|is planned|comes next|is the next", re.I)

ENTRY = re.compile(r"^\d+\.\s+S\d{3}")
STEP_ID = re.compile(r"S\d{3}")


def state(sid, adocs):
    """Where a step id lives, or 'retired' when its file is gone."""
    for d in PLAN_DIRS:
        directory = os.path.join(adocs, d)
        if not os.path.isdir(directory):
            continue
        for name in os.listdir(directory):
            if name.startswith(sid + "_"):
                return d
    return "retired"


def prose_paragraphs(text):
    """Everything that is not an entry of the ordered list. plan.md says an id
    named in a sentence is prose and the list is the order; this is that split
    made mechanical."""
    kept = [l for l in text.split("\n") if not ENTRY.match(l)]
    return "\n".join(kept).split("\n\n")


def sentences(paragraph):
    flat = " ".join(paragraph.split())
    # Split on a full stop followed by a capital or a markdown marker, so that
    # "S044 to S052" and "10.3 cp" survive where a naive split on "." does not.
    return re.split(r"(?<=[.!?])\s+(?=[A-Z*`])", flat)


def check(path, adocs):
    text = open(path).read()
    flagged = []
    ids = set()

    for paragraph in prose_paragraphs(text):
        for sentence in sentences(paragraph):
            found = STEP_ID.findall(sentence)
            ids |= set(found)
            done = sorted({s for s in found if state(s, adocs) == "plan_done"})
            if done and PENDING.search(sentence):
                flagged.append((done, sentence))

    completed = sorted(s for s in ids if state(s, adocs) == "plan_done")
    print(f"{path}: {len(ids)} ids in prose, {len(completed)} of them completed")

    for done, sentence in flagged:
        print(f"  FLAG {' '.join(done)}")
        print(f"       {sentence[:160]}")

    print(f"  sentences flagged: {len(flagged)}")
    return len(flagged)


# ---------------------------------------------------------------- citations

CITED_EXT = ("cpp", "hpp", "h", "py", "sh", "json", "epd", "txt", "md")
_EXT = "|".join(CITED_EXT)

# A path token. Either rooted at a directory this repository has, or a bare
# basename -- the step files write both, and "chesso.cpp:698" is as much a
# citation as "src/chesso.cpp:698".
PATH = re.compile(
    r"(?<![\w/.-])((?:src|tools|tests|include|scripts|bin|adocs|books|"
    r"\.tuning)/[A-Za-z0-9_./-]+?\.(?:" + _EXT + r")"
    r"|[A-Za-z0-9_-]+\.(?:" + _EXT + r"))(?![\w/-])")

# A line or line range glued straight onto a path token. The two lookaheads
# stop `:145` from being read out of `:1450` and out of a decimal, and are two
# rather than one because `(?![\d\w.])` also rejected the full stop that ends
# `src/search_params.hpp:145-195.` -- which made the regex backtrack and read
# the range as a bare `:145`, silently citing one line instead of fifty.
_TAIL = r"(?![\d\w])(?!\.\d)"
DIRECT = re.compile(r":(\d{1,5})(?:\s*-\s*(\d{1,5}))?" + _TAIL)

# A continuation. Only after an opener, so "S085:315" and "19:65" are not
# citations and the ":1313" in "(tests/test_search.cpp:1274, :1313)" is.
CONT = re.compile(r"(?<=[\s(,/;]):(\d{1,5})(?:\s*-\s*(\d{1,5}))?" + _TAIL)

QUOTED = re.compile(r'"([^"]{4,120})"')
TITLE = re.compile(
    r"TEST_(?:CASE|CASE_FIXTURE|SUITE)\s*\(\s*(?:[A-Za-z_][\w]*\s*,\s*)?"
    r'"((?:[^"\\]|\\.)*)"', re.S)

_blob = {}
_titles = {}


def _tracked():
    out = subprocess.run(["git", "-C", REPO, "ls-files"],
                         capture_output=True, text=True).stdout.split()
    byname = {}
    for p in out:
        byname.setdefault(os.path.basename(p), []).append(p)
    return set(out), {b: v[0] for b, v in byname.items() if len(v) == 1}


def _at(rev, path):
    """Lines of path at rev, or in the working tree when rev is None."""
    key = (rev, path)
    if key not in _blob:
        if rev is None:
            full = os.path.join(REPO, path)
            try:
                with open(full, encoding="utf-8", errors="replace") as fh:
                    _blob[key] = fh.read().splitlines()
            except OSError:
                _blob[key] = None
        else:
            r = subprocess.run(["git", "-C", REPO, "show", f"{rev}:{path}"],
                               capture_output=True, text=True)
            _blob[key] = r.stdout.splitlines() if r.returncode == 0 else None
    return _blob[key]


def titles_of(path):
    """{doctest title: line the TEST_CASE token opens on} in the working tree."""
    if path not in _titles:
        found = {}
        lines = _at(None, path)
        if lines:
            text = "\n".join(lines)
            for m in TITLE.finditer(text):
                title = " ".join(m.group(1).replace('\\"', '"').split())
                found.setdefault(title, text.count("\n", 0, m.start()) + 1)
        _titles[path] = found
    return _titles[path]


def baseline(path):
    """The commit whose source a step file's citations were written against.

    The commit that last wrote the step file -- so a landing that shifts
    src/search.cpp invalidates every citation into it that nobody has looked at
    since. None when the step file is new or is modified in the working tree:
    a file being rewritten right now is being written against the tree as it
    stands, and has nothing to have drifted from yet.

    That does mean touching a step file relaxes its own drift check, and the
    same is true of committing one -- correcting a citation and moving the
    baseline are the same act. ANCHOR is the check that survives it, which is
    the argument for putting a title beside a line number rather than trusting
    the number.
    """
    dirty = subprocess.run(
        ["git", "-C", REPO, "status", "--porcelain", "--", path],
        capture_output=True, text=True).stdout.strip()
    if dirty:
        return None
    r = subprocess.run(
        ["git", "-C", REPO, "log", "-1", "--format=%H", "--", path],
        capture_output=True, text=True)
    return r.stdout.strip() or None


def paragraphs(text):
    """(first line number, text) per blank-line separated block."""
    out, line, buf, start = [], 1, [], 1
    for raw in text.split("\n"):
        if raw.strip():
            if not buf:
                start = line
            buf.append(raw)
        elif buf:
            out.append((start, "\n".join(buf)))
            buf = []
        line += 1
    if buf:
        out.append((start, "\n".join(buf)))
    return out


def cites_in(para, first_line, tracked, byname):
    """Every citation in one paragraph, with the path inherited across wraps."""
    def span(kind, m):
        a = int(m.group(1))
        return (kind, m.start(), m.end(), a,
                int(m.group(2)) if m.group(2) else a)

    marks = []
    for m in PATH.finditer(para):
        direct = DIRECT.match(para, m.end())
        if direct:
            marks.append(span("direct", direct) + (m.group(1),))
    taken = [(s, e) for _k, s, e, _a, _b, _p in marks]
    for m in CONT.finditer(para):
        if not any(s <= m.start() < e for s, e in taken):
            marks.append(span("cont", m) + (None,))
    marks.sort(key=lambda t: t[1])

    out = []
    for kind, start, _end, a, b, spelled in marks:
        line = first_line + para.count("\n", 0, start)
        if spelled is None:
            out.append((line, None, None, a, b, kind))
            continue
        resolved = spelled if spelled in tracked else byname.get(spelled)
        out.append((line, spelled, resolved, a, b, kind))
    return out


def anchor_titles(para, offset_line, first_line, path, cite=None):
    """Doctest titles of `path` that a citation on this line is claiming.

    Flattened before matching, because the prose is hard-wrapped and a title as
    long as "a side in check may not stand pat" straddles the wrap. The window
    is the citing line and the one before it, backward only and one line only: a
    title names the test a citation points at when it *introduces* it --
    `Extend "..." (tests/test_search.cpp:1887)` -- whereas text after a citation
    is as likely to be naming the test the line landed in by mistake, which is
    what S138's own evidence paragraph does.

    Paired positionally when `cite` is given, because a sentence may name two
    tests and cite two lines -- S112's and S131's accepts do, after S138 named
    both quiescence mate cases in them -- and demanding that every title in the
    window sit at every line cited in it fails both of them. Each citation
    answers to the title nearest to its left, and only falls back to the whole
    window when its own text cannot be located in the flattened line.
    """
    known = titles_of(path)
    if not known:
        return []
    lines = para.split("\n")
    i = offset_line - first_line
    window = " ".join(" ".join(lines[max(0, i - 1):i + 1]).split())
    found = [(m.group(1), m.end()) for m in QUOTED.finditer(window)
             if m.group(1) in known]
    if not found:
        return []
    if cite:
        nearest = []
        start = window.find(cite)
        while start != -1:
            before = [t for t, end in found if end <= start]
            if before:
                nearest.append(before[-1])
            start = window.find(cite, start + 1)
        if nearest:
            return nearest
    return [t for t, _end in found]


def gated(resolved):
    """Code citations gate the run; document citations are reported only."""
    return not (resolved.startswith("adocs/") or resolved.endswith(".md"))


def check_citations(path, tracked, byname):
    rel = os.path.relpath(os.path.abspath(path), REPO)
    with open(path, encoding="utf-8") as fh:
        text = fh.read()
    rev = baseline(rel)
    counts = {"code": 0, "doc": 0, "loose": 0}
    flags, notes = [], []

    for first, para in paragraphs(text):
        for line, spelled, resolved, a, b, kind in cites_in(
                para, first, tracked, byname):
            cite = f"{spelled}:{a}" + (f"-{b}" if b != a else "")
            if spelled is None:
                counts["loose"] += 1
                continue
            if resolved is None:
                counts["code"] += 1
                flags.append((line, cite, "BOUNDS",
                              "no such file in the repository"))
                continue
            now = _at(None, resolved)
            bucket = "code" if gated(resolved) else "doc"
            counts[bucket] += 1
            sink = flags if bucket == "code" else notes
            if now is None:
                sink.append((line, cite, "BOUNDS", "tracked but unreadable"))
                continue
            if a < 1 or a > b or b > len(now):
                sink.append((line, cite, "BOUNDS",
                             f"{resolved} has {len(now)} lines"))
                continue
            claimed = anchor_titles(para, line, first, resolved, cite)
            # One of the titles this citation claims has to be the test the
            # cited lines open. Any, not all: two titles reach a citation only
            # when the pairing above could not separate them.
            if claimed and not any(a <= titles_of(resolved)[t] <= b
                                   for t in claimed):
                worst = claimed[0]
                sink.append((line, cite, "ANCHOR",
                             f'"{worst}" opens at '
                             f"{resolved}:{titles_of(resolved)[worst]}"))
                continue
            if rev is None:
                continue
            then = _at(rev, resolved)
            if then is None or b > len(then):
                continue
            if "\n".join(then[a - 1:b]) != "\n".join(now[a - 1:b]):
                sink.append((line, cite, "DRIFT",
                             "held " + repr(then[a - 1][:60].strip())
                             + f" at {rev[:7]}"))

    tag = rev[:7] if rev else "none (new or edited)"
    print(f"{rel}: {counts['code']} code citations, baseline {tag}, "
          f"{len(flags)} flagged "
          f"({counts['doc']} document and {counts['loose']} loose, ungated)")
    for line, cite, kind, why in flags:
        print(f"  {kind:6} {rel}:{line}  {cite}  -- {why}")
    for line, cite, kind, why in notes:
        print(f"  note   {kind} {rel}:{line}  {cite}  -- {why}")
    return len(flags)


def pending_step_files(adocs):
    out = []
    for d in ("plan_todo", "plan_current"):
        directory = os.path.join(adocs, d)
        if not os.path.isdir(directory):
            continue
        out += [os.path.join(directory, n)
                for n in sorted(os.listdir(directory)) if n.endswith(".md")]
    return out


def citations(paths, adocs):
    tracked, byname = _tracked()
    files = paths or pending_step_files(adocs)
    flagged = sum(check_citations(p, tracked, byname) for p in files)
    print(f"citations flagged: {flagged} over {len(files)} files")
    return flagged


def main():
    adocs = os.path.join(REPO, "adocs")
    args = sys.argv[1:]
    mode = "both"
    if args and args[0] in ("--prose", "--citations"):
        mode, args = args[0][2:], args[1:]

    bad = 0
    if mode in ("both", "prose"):
        prose = args if mode == "prose" and args else [
            os.path.join(adocs, "plan.md")]
        bad += sum(check(p, adocs) for p in prose)
    if mode in ("both", "citations"):
        bad += citations(args if mode == "citations" else [], adocs)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
