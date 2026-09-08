#!/usr/bin/env python3
"""Plan hygiene: stale tense, citations, touches, and doc numbers vs the code.

Four checks over the plan and manual documents. `--prose` and `--citations` report and do
not rewrite -- which tense a sentence should take and which line a citation
meant are judgements, and the fix belongs in the same commit as the landing that
made it stale. Only `--touches` is in the ctest suite; the reason the other two
are not is in DEV_MANUAL.md and does not apply to it.

    tools/plan_prose_check.py             # all three checks
    tools/plan_prose_check.py --prose     # plan.md tense only
    tools/plan_prose_check.py --citations # pending step files' citations only
    tools/plan_prose_check.py --touches   # pending step files' touches only
    tools/plan_prose_check.py --params    # doc numbers against search_param_info()
    tools/plan_prose_check.py --prose adocs/plan.md          # explicit files
    tools/plan_prose_check.py --citations adocs/plan_todo/S091_*.md

Exits non-zero when any check flags anything.

**--params: a document sentence that states a different number for a search
parameter than the engine compiles.** 2026-08-21_adversarial-F02 found three:
`specs.md` gave the aspiration triple as 5 / 50 / 400 where the engine runs
2 / 21 / 437, and `MANUAL.md` said aspiration windows start at depth 5 eleven
lines below its own option table saying 2. S085 retuned them and the prose did
not follow. No existing check could see it: `--prose` and `--citations` compare
prose to prose, `tests/test_uci_surface.cpp` builds its expected option lines
*from* `search_param_info()` and only requires MANUAL.md to name each option,
and `tests/test_search_params.cpp` holds the code against itself.

The defaults come from `src/search_params.hpp`'s `X(symbol, name, default, min,
max)` list, which is the single source `search_param_info()` is generated from
in both builds -- `tests/test_search_params.cpp` is what keeps the two equal, so
reading the list is reading the function.

Three rules, and the third is the one with a maintenance cost:

  TABLE   MANUAL.md's option table, `| `Name` | default | min to max | ... |`.
          Compares all three numbers. test_uci_surface checks that the name is
          documented and never what the row says about it.
  NEAR    a sentence naming the parameter and then giving a number in one of a
          few tight forms -- `Name` is N, ships at N, = N, default N, N as
          shipped. Deliberately tight: "`LazyEvalMargin` at 0, 150 and 2000" is
          a sweep, not a claim about the default, and a looser rule flags it.
          The same three forms run against the C++ symbol as well, which is how
          a step file names a parameter -- `ASPIRATION_DELTA` and not
          `AspirationDelta`. There both backticks are mandatory; the comment
          above PARAM_NEAR_SYMBOL has the false positive that decided it.
  PHRASE  a sentence that never names the parameter at all. Two of F02's three
          were of this kind, so a name-adjacency scan alone would have missed
          them. These are keyed on the wording, in PARAM_PHRASES below, and
          that table is the maintenance cost of this check: rewrite the
          sentence and the rule stops matching. A rule that matches nothing
          anywhere is reported as STALE rather than passing silently, which is
          what stops the cost from being paid invisibly.

Coverage is what those three rules reach and nothing more. A number stated
about a parameter in prose that neither names it nor matches a phrase rule is
not checked and cannot be -- the check is a net with a stated mesh, not a
proof. Default file set: `adocs/specs.md`, `MANUAL.md`, `DEV_MANUAL.md`,
`adocs/plan.md`, and every file in `adocs/plan_todo/` and `adocs/plan_current/`
-- S184 added the pending step files, where the class also lived and where only
the symbol form can see it. `adocs/plan_done/` is excluded on purpose: it is
history and records what was true when it was written. S150, F02; S184, F05.

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
safety)` -- is `BARE`, and it fails the run. **The check never resolves one**:
it says the path is missing and stops, because in these files the inheritance
is not mechanically resolvable and guessing is worse than abstaining. Two
measured reasons, both from the first runs of this check. S095 wrote "returns
nullptr on a miss (transposition_table.cpp:96-103), and :449 already computes",
where :449 means src/search.cpp and the nearest named path is the other file.
And S098's Stockfish-reading section carried a page of continuations whose
subject was named paragraphs earlier, so :662 and :701-724 resolved onto a
288-line header. Between them they produced sixty impossible line numbers, and
a check that prints garbage gets ignored rather than acted on.

So the repair is a rule about writing rather than a cleverer resolver, and
`plan.md`'s "How this file works" states it: **a citation repeats its path**.
S144 converted the 383 that existed -- reading each citing sentence, then
relocating the text the citation was written against to prove the path -- and
this flag is what stops the 384th. Until S144 these were counted as
`loose, ungated` and reported, which is how they went stale in bulk with
nothing saying so: S138 found eight pending steps pointing an implementer at
the wrong mate-safety test, and the three the audit had missed were exactly the
three that wrote it bare.

**What a converted citation buys and what it does not.** BOUNDS catches a wrong
path whose line is past the end of that file, and ANCHOR catches one whose
quoted test title does not open there. A wrong path that is merely in range is
caught by neither, and DRIFT cannot help on the commit that writes it -- the
baseline is that commit. The evidence for a conversion is therefore the mapping
it was made through, `adocs/data/S144_pathings.tsv`, and not this check's
verdict. DEC-119, DEC-120.

Citations into `adocs/` and other `.md` files are counted and printed but do
not fail the run. Those documents are rewritten at every completion, by design,
so a line citation into one is stale by construction; the durable fix is to
cite a section or an id, and gating on them would make this check permanently
red and therefore ignored.

**--touches: a step whose goal names a symbol no file it may touch carries.**
`touches:` is the field that says where a change is allowed to land, so a step
whose goal is to change a symbol and whose `touches:` omits the file holding
that symbol cannot be completed without violating its own scope. S039 is the
measured case: its whole goal is to re-decide `LAZY_EVAL_MARGIN`, its `touches:`
named `src/evaluation.hpp`, and the constant moved to `src/search_params.hpp` at
S073 -- `src/evaluation.hpp` keeps only the comment saying what it means.
2026-08-20_plan_review-F06 found it by reading, S141 is the repair, and this is
the check that stops the class recurring.

    TOUCHES the goal names a symbol that lives in code somewhere in the
            repository, and no file the step may touch carries it in code.

**A mention inside a comment is not a landing site**, which is the whole of the
S039 case and the reason the file contents are comment-stripped before the
match. C and C++ comments and `#` comments in `.py` and `.sh` are stripped;
`.md` and everything else is matched whole, since a document has no code to be
outside of. A python docstring is a string and so counts as code -- it has never
mattered here, and pretending otherwise would need a parser.

**What counts as a symbol the goal names** -- deliberately narrow, because a
checker that fires on prose nouns is one that gets switched off:

  * an ALL_CAPS name with an underscore in it, `LAZY_EVAL_MARGIN`. The
    underscore is what keeps `SPRT`, `UCI` and `NNUE` out.
  * a call written with its parentheses, `search_param_info()`.
  * a `_t`-suffixed type, `move_t`.
  * a UCI option name, matched against the names `src/search_params.hpp`
    actually declares rather than against a CamelCase pattern, so `Stockfish`
    and `GitHub` cannot be read as symbols.

What it therefore does not catch, stated so nobody trusts it further than it
goes: a symbol named in prose without any of those markings (`tempo`,
`piece_placement_mg`, `evaluate` without parentheses); a landing site no symbol
points at, which is S120's defect -- its goal names nothing at all and the file
it has to clear the cache from, `src/chesso.cpp`, was found by reading its own
pitfalls; a step whose `touches:` names too much rather than too little; and any
claim about `accepts:`, which is S139's and S140's ground.

Two ungated classes print as notes rather than failing:

  * the symbol appears in code nowhere in the repository. Then it is prose, or
    it is a name the step is about to invent, and there is no file to demand.
  * `touches:` names a directory that already holds files of the symbol's own
    kind. A directory says "somewhere under here, possibly in a file that does
    not exist yet", which no check over the tree as it stands can decide --
    S150's tool lands in `tools/` and reads `search_param_info()` out of `src/`,
    and its accepts says in as many words that nothing in `src/` changes.
    Abstaining is the same call `--citations` makes on a bare `:line`: guessing
    is worse. The kind test narrows the abstention rather than removing it: a
    step naming `adocs/plan_todo/`, which holds `.md` and nothing else, still
    answers for a C++ constant. It is not airtight -- `adocs/data/` holds
    `.hpp`, because S075 dumped six fitted tables there, so naming that
    directory does buy silence for a C++ symbol. Name files, not directories.
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
    counts = {"code": 0, "doc": 0, "bare": 0}
    flags, notes = [], []

    for first, para in paragraphs(text):
        for line, spelled, resolved, a, b, kind in cites_in(
                para, first, tracked, byname):
            cite = (f"{spelled}:{a}" if spelled else f":{a}") \
                + (f"-{b}" if b != a else "")
            if spelled is None:
                counts["bare"] += 1
                flags.append((line, cite, "BARE",
                              "a citation carries its own path (DEC-120)"))
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
          f"({counts['bare']} bare and {counts['doc']} document, ungated)")
    for line, cite, kind, why in flags:
        print(f"  {kind:6} {rel}:{line}  {cite}  -- {why}")
    for line, cite, kind, why in notes:
        print(f"  note   {kind} {rel}:{line}  {cite}  -- {why}")
    return len(flags)


# ------------------------------------------------------------------ touches

# A step file's fields are `name:` at column zero and wrap onto indented
# continuation lines; this reads one field's whole value.
FIELD = re.compile(r"^([a-z_]+):[ \t]*(.*)$")

CONST_SYM = re.compile(r"\b[A-Z][A-Z0-9]*(?:_[A-Z0-9]+)+\b")
CALL_SYM = re.compile(r"\b([a-z_][a-z0-9_]*)\(\)")
TYPE_SYM = re.compile(r"\b[a-z][a-z0-9_]*_t\b")
UCI_DECL = re.compile(r"X\(\s*[A-Z][A-Z0-9_]*\s*,\s*\"([A-Za-z][A-Za-z0-9]*)\"")

# Where a symbol is looked for when deciding whether it exists in code at all.
CODE_ROOTS = ("src/", "tools/", "tests/", "scripts/", "bin/")
CODE_EXT = (".cpp", ".hpp", ".h", ".py", ".sh")

# This file is not part of the corpus it searches. Its own documentation names
# the symbols the check is about -- `LAZY_EVAL_MARGIN` and `search_param_info()`
# are both in the docstring above -- and a python docstring is a string rather
# than a comment, so leaving it in made the checker answer to its own prose:
# S150's `tools/` came back satisfied by this paragraph.
SELF = "tools/plan_prose_check.py"

_stripped = {}


def _strip_c(text):
    """C and C++ comments blanked, newlines and string literals kept.

    Newlines survive so a reported line number is the real one; string literals
    survive because `X(LAZY_EVAL_MARGIN, "LazyEvalMargin", ...)` is where a UCI
    option name is declared, not a place it is mentioned.
    """
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            out.append(c)
            i += 1
            while i < n:
                if text[i] == "\\":
                    out.append(text[i:i + 2])
                    i += 2
                    continue
                out.append(text[i])
                i += 1
                if text[i - 1] == c:
                    break
            continue
        if c == "/" and text[i:i + 2] == "//":
            while i < n and text[i] != "\n":
                i += 1
            continue
        if c == "/" and text[i:i + 2] == "/*":
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            out.append("\n" * text.count("\n", i, end))
            i = end
            continue
        out.append(c)
        i += 1
    return "".join(out)


def _strip_hash(text):
    out = []
    for line in text.split("\n"):
        cut = line.find("#")
        out.append(line if cut < 0 else line[:cut])
    return "\n".join(out)


def code_of(path):
    """A tracked file's text with its comments blanked, or None if unreadable."""
    if path not in _stripped:
        lines = _at(None, path)
        if lines is None:
            _stripped[path] = None
        else:
            text = "\n".join(lines)
            if path.endswith((".cpp", ".hpp", ".h")):
                text = _strip_c(text)
            elif path.endswith((".py", ".sh")):
                text = _strip_hash(text)
            _stripped[path] = text
    return _stripped[path]


def fields_of(text):
    """{field name: value} for one step file, continuation lines joined."""
    out, name = {}, None
    for raw in text.split("\n"):
        if raw.startswith("#"):
            break
        m = FIELD.match(raw)
        if m:
            name = m.group(1)
            out[name] = m.group(2).strip()
        elif name and raw.startswith((" ", "\t")):
            out[name] = (out[name] + " " + raw.strip()).strip()
        elif not raw.strip():
            name = None
    return out


def uci_names():
    """The UCI option names src/search_params.hpp declares."""
    text = code_of("src/search_params.hpp") or ""
    return set(UCI_DECL.findall(text))


def symbols_in(goal, options):
    """The code symbols a goal names, in the order they are written.

    A match carrying a file extension is a path and not a symbol -- S157's goal
    says "DEV_MANUAL.md says which scale the bounds are in", and DEV_MANUAL read
    as a constant sent the check looking for a definition of it.
    """
    def is_path(text, end):
        return re.match(r"\.[A-Za-z]", text[end:end + 2]) is not None

    found = []
    for regex in (CONST_SYM, CALL_SYM, TYPE_SYM):
        for m in regex.finditer(goal):
            if not is_path(goal, m.end()):
                found.append((m.start(), m.group(0)))
    for name in options:
        for m in re.finditer(r"\b" + re.escape(name) + r"\b", goal):
            if not is_path(goal, m.end()):
                found.append((m.start(), name))
    seen, out = set(), []
    for _pos, sym in sorted(found):
        if sym not in seen:
            seen.add(sym)
            out.append(sym)
    return out


def touches_paths(value, tracked, byname, dirs):
    """(files, directories) named by a touches field.

    Tokenised rather than pattern-matched, because the field is prose with paths
    in it and the prose contains slashes of its own -- `src/bitboard.cpp
    add_piece/remove_piece/move_piece` names one path and three functions. A
    token counts only when the repository actually has it.
    """
    files, directories = [], []
    for token in re.split(r"[\s,;()]+", value):
        token = token.strip().strip(".`'\"")
        if not token:
            continue
        if token in tracked:
            files.append(token)
        elif token.endswith("/") and token.rstrip("/") + "/" in dirs:
            directories.append(token.rstrip("/") + "/")
        elif "/" not in token and token in byname:
            files.append(byname[token])
    return files, directories


def _homes(sym, tracked):
    """`path:line` of the first code line carrying sym, one per file."""
    word = re.compile(r"\b" + re.escape(sym.rstrip("()")) + r"\b")
    out = []
    for path in sorted(tracked):
        if path == SELF or not path.endswith(CODE_EXT):
            continue
        if not (path.startswith(CODE_ROOTS) or "/" not in path):
            continue
        text = code_of(path)
        if not text:
            continue
        m = word.search(text)
        if m:
            out.append(f"{path}:{text.count(chr(10), 0, m.start()) + 1}")
    return out


def could_host(directories, homes, under):
    """The listed directories that might grow a file carrying this symbol.

    A directory says "somewhere under here, possibly in a file that does not
    exist yet", so it cannot be resolved against the tree as it stands -- but
    only for a symbol of a kind it already holds. `tools/` holds C++ and so
    might grow the definition of `search_param_info()`; `adocs/data/` holds
    logs and tables and will never hold a C++ constant, so listing it buys no
    silence. Without that second half a step could abstain from this check by
    naming the directory it writes its measurements into.
    """
    kinds = {os.path.splitext(h.split(":")[0])[1] for h in homes}
    return [d for d in directories
            if kinds & {os.path.splitext(p)[1] for p in under.get(d, [])}]


def carries(sym, path):
    text = None if path == SELF else code_of(path)
    if text is None:
        return False
    return re.search(r"\b" + re.escape(sym.rstrip("()")) + r"\b", text) is not None


def check_touches(path, tracked, byname, dirs, options, under):
    rel = os.path.relpath(os.path.abspath(path), REPO)
    with open(path, encoding="utf-8") as fh:
        fields = fields_of(fh.read())
    goal = fields.get("goal", "")
    syms = symbols_in(goal, options)
    if not syms:
        return 0, 0, 0

    files, directories = touches_paths(
        fields.get("touches", ""), tracked, byname, dirs)
    listed = list(files)
    for d in directories:
        listed += under.get(d, [])

    flags, notes = [], []
    for sym in syms:
        if any(carries(sym, p) for p in listed):
            continue
        homes = _homes(sym, tracked)
        where = "in code at " + ", ".join(homes[:3]) + (
            f" and {len(homes) - 3} more" if len(homes) > 3 else "")
        hosts = could_host(directories, homes, under)
        if not homes:
            notes.append((sym, "not in code anywhere -- read as prose"))
        elif hosts:
            notes.append((sym, where + "; touches names " + " ".join(hosts)
                          + ", which may grow a file for it"))
        else:
            flags.append((sym, where + "; touches names "
                          + (", ".join(files + directories) or "nothing")))

    print(f"{rel}: {len(syms)} symbol(s) in goal, {len(flags)} flagged"
          + (f", {len(notes)} noted" if notes else ""))
    for sym, why in flags:
        print(f"  TOUCHES {rel}  {sym}  -- {why}")
    for sym, why in notes:
        print(f"  note    {rel}  {sym}  -- {why}")
    return len(syms), len(flags), len(notes)


def touches(paths, adocs):
    tracked, byname = _tracked()
    if not tracked:
        print("touches: no tracked files, skipped (not a git checkout)")
        return 0
    dirs, under = set(), {}
    for p in tracked:
        parts = p.split("/")
        for i in range(1, len(parts)):
            d = "/".join(parts[:i]) + "/"
            dirs.add(d)
            under.setdefault(d, []).append(p)
    options = uci_names()
    files = paths or pending_step_files(adocs)
    total = flagged = noted = 0
    for f in files:
        s, fl, n = check_touches(f, tracked, byname, dirs, options, under)
        total, flagged, noted = total + s, flagged + fl, noted + n
    print(f"touches flagged: {flagged} over {len(files)} files "
          f"({total} symbols read, {noted} noted and ungated)")
    return flagged


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


# --- params: document numbers against the compiled search parameters ---------

PARAM_ROW = re.compile(
    r"^\|\s*`([A-Za-z][A-Za-z0-9]*)`\s*\|\s*(-?\d+)\s*\|"
    r"\s*(-?\d+)\s+to\s+(-?\d+)\s*\|")

PARAM_DECL = re.compile(
    r"X\(\s*([A-Z][A-Z0-9_]*)\s*,\s*\"([A-Za-z][A-Za-z0-9]*)\"\s*,"
    r"\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\)")

# A number close enough to the name to be a claim about the default. Tight on
# purpose: `{n}` at 0, 150 and 2000 is a sweep and must not be flagged.
PARAM_NEAR = (
    r"`?{n}`?\s*(?:is|ships at|ships|=|default)\s+\*{{0,2}}(-?\d+)",
    r"`?{n}`?[^.\n]{{0,30}}?\b(-?\d+)\s+as shipped",
    r"`?{n}`?[^.\n]{{0,20}}?\bdefault\s+(-?\d+)",
)

# The same three forms against the C++ symbol, which is how a step file names a
# parameter -- "`ASPIRATION_DELTA` is 50" and not the UCI name. **Both backticks
# are mandatory here and that is measured, not stylistic**: optional, the first
# form reads S114's formula line "`NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR` =
# 3 + depth/6" as a claim that the divisor is 3, because the regex takes the
# formula's own closing backtick as the symbol's. Mandatory, it does not, and no
# true hit anywhere in the file set is lost. S184.
PARAM_NEAR_SYMBOL = tuple(p.replace("`?{n}`?", "`{n}`") for p in PARAM_NEAR)

# The sentences that state a parameter's value without ever naming it. Keyed on
# the wording, which is the cost: rewrite the sentence and the rule goes STALE
# rather than silently passing. Each entry is (UCI name, regex capturing the
# number). F02's three cases are the first four rows.
PARAM_PHRASES = (
    ("AspirationMinDepth",
     r"search the root of each iteration from depth (\d+)"),
    ("AspirationDelta",
     r"from depth \d+ in a band \*\*(\d+) centipawns\*\*"),
    ("AspirationMaxDelta",
     r"doubling the failing side alone and going to the full window past (\d+)"),
    ("AspirationMinDepth",
     r"\*Aspiration windows\* are present[^.]*?from depth (\d+)"),
)

# Retired, and the retirement is the maintenance cost being paid in the open:
# ("MaxQsearchDepth", r"quiescence is capped at (\d+) plies") was F02's third
# case and was observed red against `eaad88b^:adocs/specs.md`, which is where
# that wording still lives. `eaad88b` rewrote the sentence to name the
# parameter, so NEAR covers it now -- "`MaxQsearchDepth` plies, 19 as shipped"
# -- and leaving the rule in would have printed STALE on every run until
# somebody deleted it to get green. S150.

# The four documents that describe the shipping engine. The pending step files
# are read too and are not listed here: they come from pending_step_files(), so
# a new step is covered the day it is written. `adocs/plan_done/` stays out --
# history records what was true when it was written. S184.
PARAM_DOCS = ("adocs/specs.md", "MANUAL.md", "DEV_MANUAL.md", "adocs/plan.md")


def _search_params(group):
    """{key: (default, min, max)} from src/search_params.hpp, keyed on group.

    Empty is a failure and never a pass: a list that stopped parsing would make
    every rule below vacuous, which is the trap this whole check exists against.
    """
    text = code_of("src/search_params.hpp") or ""
    return {m.group(group): (int(m.group(3)), int(m.group(4)), int(m.group(5)))
            for m in PARAM_DECL.finditer(text)}


def search_params():
    """{uci name: (default, min, max)} -- what the manuals and plan.md name."""
    return _search_params(2)


def search_symbols():
    """{C++ symbol: (default, min, max)} -- what a step file names."""
    return _search_params(1)


def _joined(text):
    """(text with newlines as spaces, line number of each character).

    The documents are hard-wrapped, so a sentence stating a number routinely
    spans two lines -- MANUAL.md's aspiration claim does. Matching on the joined
    text and mapping the offset back is what lets a rule be written as the
    sentence reads.
    """
    flat, lines, line = [], [], 1
    for ch in text:
        if ch == "\n":
            flat.append(" ")
            lines.append(line)
            line += 1
        else:
            flat.append(ch)
            lines.append(line)
    return "".join(flat), lines


def check_params(path, params, symbols=None):
    """Report every document number that disagrees with the compiled value."""
    full = os.path.join(REPO, path)
    try:
        with open(full, encoding="utf-8") as handle:
            text = handle.read()
    except OSError:
        return 0, set()

    bad = 0
    for number, line in enumerate(text.split("\n"), 1):
        row = PARAM_ROW.match(line)
        if row and row.group(1) in params:
            name = row.group(1)
            got = (int(row.group(2)), int(row.group(3)), int(row.group(4)))
            if got != params[name]:
                print("TABLE  {}:{}  {} documented {} default {} to {}, "
                      "code {} default {} to {}".format(
                          path, number, name, got[0], got[1], got[2],
                          *params[name]))
                bad += 1

    flat, lineof = _joined(text)

    for names, rules in ((params, PARAM_NEAR),
                         (symbols or {}, PARAM_NEAR_SYMBOL)):
        for name, (default, _lo, _hi) in names.items():
            for pattern in rules:
                for m in re.finditer(pattern.format(n=re.escape(name)), flat):
                    if int(m.group(1)) != default:
                        print("NEAR   {}:{}  {} stated as {}, code {}".format(
                            path, lineof[m.start()], name, m.group(1), default))
                        bad += 1

    fired = set()
    for index, (name, pattern) in enumerate(PARAM_PHRASES):
        if name not in params:
            continue
        for m in re.finditer(pattern, flat):
            fired.add(index)
            if int(m.group(1)) != params[name][0]:
                print("PHRASE {}:{}  {} stated as {}, code {}".format(
                    path, lineof[m.start()], name, m.group(1),
                    params[name][0]))
                bad += 1

    return bad, fired


def check_all_params(paths, adocs=None):
    params, symbols = search_params(), search_symbols()
    if not params:
        print("PARSE  src/search_params.hpp yielded no parameters -- the "
              "check would pass vacuously, so it fails instead")
        return 1

    default_set = list(PARAM_DOCS)
    if adocs:
        default_set += [os.path.relpath(p, REPO)
                        for p in pending_step_files(adocs)]

    bad, fired = 0, set()
    for path in paths or default_set:
        count, hit = check_params(path, params, symbols)
        bad += count
        fired |= hit

    for index, (name, pattern) in enumerate(PARAM_PHRASES):
        if index not in fired:
            print("STALE  PARAM_PHRASES[{}] for {} matched nothing: {}".format(
                index, name, pattern))
            bad += 1

    return bad


def main():
    adocs = os.path.join(REPO, "adocs")
    args = sys.argv[1:]
    mode = "all"
    if args and args[0] in ("--prose", "--citations", "--touches", "--params"):
        mode, args = args[0][2:], args[1:]
    if mode == "all" and args:
        # Refused rather than ignored: before the citation check existed a bare
        # path meant "check this plan file's prose", and silently dropping it
        # would report a green run over a file nobody looked at.
        print("usage: plan_prose_check.py "
              "[--prose|--citations|--touches|--params] [files...]",
              file=sys.stderr)
        print("  a file list needs the mode it belongs to", file=sys.stderr)
        return 2

    bad = 0
    if mode in ("all", "prose"):
        prose = args or [os.path.join(adocs, "plan.md")]
        bad += sum(check(p, adocs) for p in prose)
    if mode in ("all", "citations"):
        bad += citations(args if mode == "citations" else [], adocs)
    if mode in ("all", "touches"):
        bad += touches(args if mode == "touches" else [], adocs)
    if mode in ("all", "params"):
        bad += check_all_params(args if mode == "params" else [], adocs)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
