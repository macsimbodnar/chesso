#!/usr/bin/env python3
"""Propose a symbol for every `path:line` citation in the pending step files.

S187, following adocs/data/S144_paths.py and adocs/data/S169_recite.py. Same
shape and same discipline: the evidence for a conversion is the mapping it was
made through, not the checker's verdict afterwards, because rewriting a step
file moves its own baseline (DEC-119).

    python3 adocs/data/S187_symbolise.py propose 124e557 > adocs/data/S187_symbols.tsv
    python3 adocs/data/S187_symbolise.py detail S132 124e557   # one file, readable
    python3 adocs/data/S187_symbolise.py apply                 # rewrite from the TSV

The revision argument is the commit the pending files are read at -- 124e557,
the checker change, is the commit this conversion started from. Without it
`propose` reads the working tree, and after the conversion the working tree
holds no line citation to map.

WHERE THE SYMBOL COMES FROM. The citation is read at the step file's baseline
-- the commit that last wrote it -- because that is the text its author was
looking at. The enclosing definition of the cited line is found by walking the
file at that commit, and then the symbol is required to still exist at HEAD.
A citation whose symbol is gone, or whose paragraph backticks a different
symbol of the same file, is a hand case and its sentence gets read.

METHODS, in the order they are tried:

    TITLE  the cited lines are inside a doctest TEST_CASE; the title is the
           symbol and it is written as a quoted phrase.
    SPAN   the range covers more than one definition. The first is proposed
           and the row is a hand case: `LMR_BASE` and `LMR_DIVISOR` want both
           names, and where the second goes is a question about the sentence.
    FILE   no enclosing definition -- a file-scope comment or a bare static.
           A hand case: the landing site is a quoted phrase out of the text.
    AUTO   exactly one enclosing definition, and the citing paragraph already
           backticks it. The conversion is a rewrite of what is already there.
    WALK   exactly one enclosing definition, not named in the paragraph.
           Mechanical, and the row carries the definition's own line so a
           reader can check it.
    HAND   the paragraph backticks a different symbol that the cited file
           carries, or the proposed symbol is gone at HEAD. These are the
           blind class: a citation that was wrong when it was written, which
           2026-09-04_plan_review-F07 found four of.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(REPO, "tools"))

import plan_prose_check as ppc  # noqa: E402

TSV = os.path.join(HERE, "S187_symbols.tsv")
WIDTH = 79

# A backticked identifier in the citing paragraph, used to agree with or
# contradict the walk-up. Same token grammar the checker recognises.
BACKTICKED = re.compile(r"`([A-Za-z_][A-Za-z0-9_:]*)(?:\s*\([^`)]*\))?`")


def baseline(rel):
    """The commit whose source this step file's citations were written against.

    Lifted out of the checker, which no longer needs it: no failure class
    consults a previous commit since S187. A file dirty in the working tree
    has no baseline and is read against the tree.
    """
    dirty = subprocess.run(["git", "-C", REPO, "status", "--porcelain", "--",
                            rel], capture_output=True, text=True).stdout.strip()
    if dirty:
        return None
    r = subprocess.run(["git", "-C", REPO, "log", "-1", "--format=%H", "--",
                        rel], capture_output=True, text=True)
    return r.stdout.strip() or None


# --------------------------------------------------------------- definitions

DEFINE = re.compile(r"^#\s*define\s+([A-Za-z_]\w*)")
XROW = re.compile(r"^\s+X\(\s*([A-Za-z_]\w*)")
TYPEDEF = re.compile(r"^(?:typedef\s+)?(?:struct|class|union|enum)"
                     r"(?:\s+class)?\s+([A-Za-z_]\w*)")
# The name is the last identifier before the `=`, or before the `(` of a
# definition or a declaration. Written as "everything up to the marker, then
# the identifier that ends it" rather than as a type grammar: a type grammar
# has to be told about `static constexpr int psqt_mg[6][64]` and about
# `static_assert(` starting the line, and it got both wrong.
ASSIGN = re.compile(r"^[^=;()]*?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*=[^=]")
FUNC = re.compile(r"^[^=;()]*?\b([A-Za-z_]\w*)\s*\(")
# A declaration -- a prototype in a header, or a file-scope object with no
# initialiser. `int capture_score(const board_t*, move_t);` in
# src/evaluation.hpp and `static std::mutex output_mutex;` in src/chesso.cpp
# are both landing sites a citation means, and both end in a semicolon.
DECL = re.compile(r"^[^=()]*?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*;")
SH_FUNC = re.compile(r"^([A-Za-z_]\w*)\s*\(\)\s*\{")
SH_ASSIGN = re.compile(r"^(?:export\s+|readonly\s+)?([A-Za-z_]\w*)=")
PY_DEF = re.compile(r"^(?:def|class)\s+(\w+)")
PY_ASSIGN = re.compile(r"^([A-Za-z_]\w*)\s*=")

_defs = {}


def _brace_end(lines, start):
    """Last line of a brace body opened at or just after `start`, 1-based."""
    depth, seen = 0, False
    for n in range(start, len(lines) + 1):
        line = lines[n - 1]
        depth += line.count("{") - line.count("}")
        if "{" in line:
            seen = True
        if seen and depth <= 0:
            return n
        if not seen and line.rstrip().endswith(";"):
            return n
    return len(lines)


def definitions(path, lines):
    """[(first line, last line, kind, symbol)] for one file, 1-based."""
    key = (path, len(lines), lines[0] if lines else "")
    if key in _defs:
        return _defs[key]
    out = []
    text = "\n".join(lines)
    if path.endswith((".cpp", ".hpp", ".h")):
        for m in ppc.TITLE.finditer(text):
            first = text.count("\n", 0, m.start()) + 1
            title = " ".join(m.group(1).replace('\\"', '"').split())
            out.append((first, _brace_end(lines, first), "title", title))
        for n, line in enumerate(lines, 1):
            if any(a <= n <= b for a, b, k, _s in out if k == "title"):
                continue
            m = DEFINE.match(line)
            if m:
                end = n
                while end < len(lines) and lines[end - 1].rstrip().endswith("\\"):
                    end += 1
                out.append((n, end, "define", m.group(1)))
                continue
            m = XROW.match(line)
            if m:
                out.append((n, n, "xmacro", m.group(1)))
                continue
            if not line or line[0].isspace() or line[0] in "}#/*":
                continue
            m = TYPEDEF.match(line)
            if m:
                out.append((n, _brace_end(lines, n), "type", m.group(1)))
                continue
            m = ASSIGN.match(line)
            if m:
                end = n
                while end < len(lines) and ";" not in lines[end - 1]:
                    end += 1
                out.append((n, end, "const", m.group(1)))
                continue
            m = FUNC.match(line)
            if m and not line.rstrip().endswith(";"):
                out.append((n, _brace_end(lines, n), "func", m.group(1)))
                continue
            m = FUNC.match(line) or DECL.match(line)
            if m:
                end = n
                while end < len(lines) and ";" not in lines[end - 1]:
                    end += 1
                out.append((n, end, "decl", m.group(1)))
    elif path.endswith(".sh"):
        for n, line in enumerate(lines, 1):
            m = SH_FUNC.match(line)
            if m:
                out.append((n, _brace_end(lines, n), "func", m.group(1)))
                continue
            m = SH_ASSIGN.match(line)
            if m:
                out.append((n, n, "const", m.group(1)))
    elif path.endswith(".py"):
        for n, line in enumerate(lines, 1):
            m = PY_DEF.match(line)
            if m:
                end = n
                while end < len(lines) and (not lines[end].strip()
                                            or lines[end][:1].isspace()):
                    end += 1
                out.append((n, end, "func", m.group(1)))
                continue
            m = PY_ASSIGN.match(line)
            if m:
                end = n
                while end < len(lines) and lines[end - 1].count("(") > \
                        lines[end - 1].count(")"):
                    end += 1
                out.append((n, end, "const", m.group(1)))
    out.sort()
    _defs[key] = out
    return out


MEMBER = re.compile(r"^\s+(?:static\s+|mutable\s+|const\s+)*"
                    r"[A-Za-z_][\w:<>,&*\s]*?\b([A-Za-z_]\w*)\s*"
                    r"(?:\[[^\]]*\])*\s*(?:=[^=]|;)")


def enclosing(path, lines, a, b):
    """Innermost definition at line `a` first, then every definition the range
    goes on to open.

    Innermost, not outermost: `tests/test_search.cpp` wraps its cases in a
    TEST_SUITE, and taking the first enclosing definition made every citation
    into that file resolve to the suite title. A struct is the other direction
    -- the enclosing definition is `search_state_t` and what the citation
    means is the member declared on the cited line, which is S024's
    `counter_moves`.
    """
    defs = definitions(path, lines)
    inner = sorted((d for d in defs if d[0] <= a <= d[1]),
                   key=lambda d: (d[0], -d[1]))
    out = list(inner[-1:])
    if out and out[0][2] == "type" and a == b:
        # One line inside a struct means the member declared on it; a range
        # covering the struct means the struct. S097 cites
        # `src/data_structures.hpp` lines 447 to 483 for "the per-ply arrays in
        # search_state_t" and line 447 happens to declare a member called
        # `type`, which is not what the sentence is about.
        m = MEMBER.match(lines[a - 1]) if a <= len(lines) else None
        if m:
            out = [(a, a, "member", m.group(1))]
    return out + [d for d in defs if a < d[0] <= b]


LEAD = re.compile(r"^(?:[/*#\-\s>]|\*\*)+")


def phrase_at(path, a, b, then):
    """A quotable phrase out of the cited lines as they stood at the baseline.

    For the file-scope class -- a comment, a blank line, a bare declaration --
    there is no enclosing definition to name, so the landing site is the text
    itself. Nine words or so, leading comment markers stripped, and required
    to occur exactly once in the file at HEAD: a phrase that occurs twice
    points at two places and is worth no more than the line number it
    replaces.

    Built from the baseline text and then looked for at HEAD, never the other
    way round. Nine of the fourteen document citations had already drifted --
    S136 cites `adocs/plan.md` line 359 for the sentence about how ids are
    allocated, and at HEAD that line is inside S199's paragraph -- so a phrase
    read at the cited line today would quote a sentence the step file never
    meant.
    """
    now = ppc._at(None, path)
    if now is None or not then:
        return ""
    # One line of the range, never a run of them. A comment block flattens to
    # text with `//` still inside it, and the checker matches a phrase against
    # the file's own characters with only its whitespace flattened -- so a
    # phrase that spans two comment lines has to carry the marker between them
    # to match, which reads as noise in the step file. Backticks, emphasis and
    # quotes are cut for the same reason: the longest run between them says
    # the same thing and quotes cleanly.
    best = ""
    for line in then:
        for run in re.split(r'[`*"]+', LEAD.sub("", line.strip())):
            words, taken = run.split(), []
            while words and (len(" ".join(taken)) < 45 or len(taken) < 6):
                taken.append(words.pop(0))
                if len(taken) >= 14:
                    break
            cand = " ".join(taken).strip(" ,;:.-")
            if len(cand) > len(best):
                best = cand
    whole = " ".join("\n".join(now).split())
    return best if len(best) >= 30 and whole.count(best) == 1 else ""


# ------------------------------------------------------------------ proposal

def window(para, first_line, line):
    """The citing line and the one before it, which is what a citation claims.

    The same window `anchor_titles` used for titles, and for the same reason:
    a symbol named beside a citation is what the citation is about, while one
    named three sentences earlier is what the code does. Widening this to the
    paragraph made 258 of 572 rows hand cases -- a paragraph that says
    `generate_quiets` beside a citation into `negamax` is not contradicting
    anything.
    """
    lines = para.split("\n")
    i = line - first_line
    return " ".join(lines[max(0, i - 1):i + 1])


def line_cites(text, tracked, byname):
    """Every line-form citation: (line, spelled, resolved, a, b, raw, para)."""
    out = []
    for first, para in ppc.paragraphs(ppc.unfenced(text)):
        for line, spelled, resolved, kind, what in ppc.refs_in(
                para, first, tracked, byname):
            if kind != "line":
                continue
            a, _, rest = what.partition(":")
            lo, _, hi = rest.partition("-")
            out.append((line, spelled, resolved, int(lo), int(hi or lo),
                        what, para, first))
    return out


def propose(at=None):
    """Every line-form citation and the symbol proposed for it.

    `at` is the revision the *step files* are read from, and it is not
    optional once the conversion has landed: a walk over the working tree
    after the rewrite finds no line citations at all and would silently write
    an empty mapping. Pass the commit the conversion started from.
    """
    tracked, byname = ppc._tracked()
    rows = []
    for path in ppc.pending_step_files(os.path.join(REPO, "adocs")):
        rel = os.path.relpath(path, REPO)
        rev = baseline(rel)
        if at:
            lines = ppc._at(at, rel)
            if lines is None:
                continue
            text = "\n".join(lines)
            rev = subprocess.run(
                ["git", "-C", REPO, "log", "-1", "--format=%H", at, "--", rel],
                capture_output=True, text=True).stdout.strip() or None
        else:
            with open(path, encoding="utf-8") as fh:
                text = fh.read()
        for line, spelled, resolved, a, b, raw, para, first_line in line_cites(
                text, tracked, byname):
            row = {"step": rel, "line": line, "cite": raw, "form": "",
                   "rev": (rev or "worktree")[:7], "then": "", "method": "HAND",
                   "symbol": "", "kind": "", "at": "", "why": ""}
            chosen = HAND_CHOICES.get((os.path.basename(rel)[:4], raw))
            if chosen:
                row["form"], row["method"] = chosen, "CHOSEN"
            if row["method"] == "CHOSEN":
                rows.append(row)
                continue
            if resolved is None:
                row["why"] = "path resolves to nothing"
                rows.append(row)
                continue
            then = ppc._at(rev, resolved) if rev else ppc._at(None, resolved)
            if then is None:
                then = ppc._at(None, resolved)
                row["why"] = "file absent at baseline, read at HEAD"
            if then is None or b > len(then):
                row["why"] = "cited range is past the end of the file"
                rows.append(row)
                continue
            row["then"] = " ".join("\n".join(then[a - 1:b]).split())[:110]
            touched = enclosing(resolved, then, a, b)
            named = set(BACKTICKED.findall(window(para, first_line, line)))
            if not touched:
                row["method"], row["why"] = "FILE", "no enclosing definition"
                row["symbol"] = phrase_at(resolved, a, b, then[a - 1:b])
                if row["symbol"]:
                    row["method"], row["kind"] = "PHRASE", "phrase"
                    row["at"] = row["symbol"]
                rows.append(row)
                continue
            first = touched[0]
            row["symbol"], row["kind"] = first[3], first[2]
            if first[2] == "title":
                row["method"] = "TITLE"
            elif len(touched) > 1:
                row["method"] = "SPAN"
                row["why"] = "also " + ", ".join(d[3] for d in touched[1:])
                names = list(dict.fromkeys(d[3] for d in touched))
                if len(names) == 2:
                    # A pair -- `LMR_BASE` and `LMR_DIVISOR`, an mg/eg couple
                    # -- gets both names with the path repeated, which is
                    # DEC-120 read literally. Three or more is a stretch and
                    # is decided by hand in HAND_CHOICES above.
                    row["form"] = (f"`{spelled}` `{names[0]}` and "
                                   f"`{spelled}` `{names[1]}`")
            elif first[3] in named:
                row["method"] = "AUTO"
            else:
                # Only a *definition* of the cited file contradicts the
                # walk-up. A paragraph naming `in_check` or `generate_quiets`
                # beside a citation into `negamax` is naming what the code
                # does, not where it is; requiring mere presence made 258 of
                # the 572 rows hand cases and none of them was one.
                defined = {d[3] for d in definitions(resolved, then)}
                other = [n for n in named if n != first[3] and n in defined]
                if other:
                    row["method"] = "HAND"
                    row["why"] = "paragraph names " + ", ".join(sorted(other))
                else:
                    row["method"] = "WALK"
            if row["method"] == "TITLE":
                ok = row["symbol"] in ppc.titles_of(resolved)
            else:
                ok = ppc.carries_symbol(row["symbol"], resolved)
            if not ok:
                row["method"] = "HAND"
                row["why"] = (row["why"] + "; " if row["why"] else "") \
                    + "symbol gone at HEAD"
            else:
                now = enclosing(resolved, ppc._at(None, resolved), a, b)
                row["at"] = now[0][3] if now else ""
            rows.append(row)
    return rows


# ------------------------------------------------------------------ by hand

# The rows the walk-up cannot decide, keyed by step id and citation as written.
# Each is a sentence that was read. The first six are the blind class -- a
# citation that was already wrong when it was written, which no mechanical rule
# can see because the line it names holds text the sentence is not about.
HAND_CHOICES = {
    # The countermove write. The cited line held `unmake_move(game);`; the
    # write is `state->counter_moves[...] =` further down, inside `negamax`.
    ("S024", "src/search.cpp:517"): "`src/search.cpp` `negamax`",
    # The cited line held a padding comment inside `search_state_t`; the array
    # the sentence names is the member.
    ("S024", "src/data_structures.hpp:436"):
        "`src/data_structures.hpp` `counter_moves`",
    # `see_value` is declared one line above the citation, and the cited line
    # is blank -- so there is no enclosing definition to walk up to.
    ("S091", "src/bitboard.cpp:1096"): "`src/bitboard.cpp` `see_value`",
    ("S109", "src/bitboard.cpp:1096"): "`src/bitboard.cpp` `see_value`",
    # Both citations were meant for the test-only counter and neither line
    # holds it: 60-64 is the thread and mutex block, 88-89 a typedef.
    ("S115", "src/chesso.cpp:60-64"):
        "`src/chesso.cpp` `last_aspiration_failures`",
    ("S115", "src/chesso.cpp:88-89"):
        "`src/chesso.cpp` `last_aspiration_failures`",
    # The scaler moved out of the budget function; 510-534 is inside
    # `compute_search_time_budget` at the baseline and the sentence names
    # `search_time_scale_percent()`, which is the function it became.
    ("S132", "src/chesso.cpp:510-534"):
        "`src/chesso.cpp` `search_time_scale_percent`",
    ("S132", "src/chesso.cpp:482"):
        "`src/chesso.cpp` `compute_search_time_budget`",
    # Ranges over a struct: the innermost definition at the first line is a
    # member, and the sentence is about the whole type.
    ("S097", "src/data_structures.hpp:388-420"):
        "`src/data_structures.hpp` `tt_entry_t`",
    ("S097", "src/data_structures.hpp:447-483"):
        "`src/data_structures.hpp` `search_state_t`",
    # Document citations. A line into plan.md is the shortest-lived reference
    # in the repository -- the Open list renumbers at every completion -- so
    # each becomes the sentence it was pointing at.
    ("S098", "adocs/plan.md:81"):
        '`adocs/plan.md` "the improving flag, was first in the pending order"',
    ("S125", "adocs/plan.md:374-375"):
        '`adocs/plan.md` "backward, phalanx, supported and weak unopposed '
        'pawns join the three"',
    ("S136", "adocs/plan.md:359"):
        '`adocs/plan.md` "taper mobility and king safety through one division '
        'instead of two"',
    ("S136", "adocs/plan.md:370"):
        '`adocs/plan.md` "unfreeze tempo, re-derive the truncation guard its '
        'zero weight holds"',
    ("S136", "adocs/plan_todo/S055_taper_stage_two_once.md:3"):
        '`adocs/plan_todo/S055_taper_stage_two_once.md` "re-pinned to the '
        'post-merge bound of 2 x 23/24 = 1.917"',
    ("S042", "adocs/plan.md:110"):
        '`adocs/plan.md` "under 1 % by its own file, below every instrument '
        'here"',
    ("S117", "adocs/specs.md:80-87"):
        '`adocs/specs.md` "A change is retained only against a measurement"',
    ("S119", "fastchess.sh:348"):
        '`fastchess.sh` "option.Hash=16 option.Threads=1"',
    # `search_state_t state = {}` moved out of `search_book_move`; all three
    # steps cite it for where the per-`go` state is constructed, and at each of
    # their baselines the line held the book move's `legal_moves` array.
    ("S132", "chesso.cpp:674"): "`src/chesso.cpp` `iterative_deepening_search`",
    # Same drift one line-set over: the per-iteration reset of `explored_nodes`
    # is in the deepening loop, and 726 had fallen into the book-move helper.
    ("S132", "chesso.cpp:726"): "`src/chesso.cpp` `iterative_deepening_search`",
    ("S159", "src/chesso.cpp:674"):
        "`src/chesso.cpp` `iterative_deepening_search`",
    # The node counter is a member of `search_state_t`, not of the table
    # declared just below it.
    ("S132", "src/data_structures.hpp:453"):
        "`src/data_structures.hpp` `explored_nodes`",
    # The whole comment is the landing site; its first clause names it.
    ("S112", "src/search_params.hpp:29-32"):
        '`src/search_params.hpp` "Not in the set, on purpose"',
    ("S112", "search_params.hpp:29-32"):
        '`src/search_params.hpp` "Not in the set, on purpose"',
    ("S159", "src/search_params.hpp:29"):
        '`src/search_params.hpp` "Not in the set, on purpose"',
    # A stretch of table, named by its ends rather than by all nine.
    ("S132", "src/search_params.hpp:270-320"):
        "`src/search_params.hpp` `TM_SOFT_PERCENT` to `src/search_params.hpp` "
        "`TM_SCALE_MIN_PERCENT`",
    ("S132", "chesso.cpp:71-93"):
        "`src/chesso.cpp` `last_aspiration_failures` to `src/chesso.cpp` "
        "`last_time_scale_percent`",
    ("S132", "uci.hpp:144-153"):
        "`src/uci.hpp` `uci_last_aspiration_failures` to `src/uci.hpp` "
        "`uci_last_time_scale_percent`",
    ("S132", "src/chesso.cpp:454-507"):
        "`src/chesso.cpp` `compute_search_time_budget`",
    ("S098", "src/search.cpp:96-130"):
        "`src/search.cpp` `build_lmr_table` and `src/search.cpp` "
        "`lmr_reduction`",
    ("S098", "src/search.cpp:113-122"):
        "`src/search.cpp` `search_params_rebuild_derived`",
    ("S109", "src/evaluation.cpp:33-37"):
        "`src/evaluation.cpp` `ORDER_TT_MOVE` to `src/evaluation.cpp` "
        "`ORDER_COUNTER`",
    ("S115", "src/chesso.cpp:60-64"):
        "`src/chesso.cpp` `last_aspiration_failures`",
    ("S117", "src/eval_tables.hpp:189-240"):
        "`src/eval_tables.hpp` `eval_add_piece` to `src/eval_tables.hpp` "
        "`eval_refresh`",
}


COLS = ("step", "line", "cite", "rev", "method", "kind", "symbol", "form",
        "at", "why", "then")


def write(rows, out):
    print("\t".join(COLS), file=out)
    for r in rows:
        print("\t".join(str(r[c]).replace("\t", " ") for c in COLS), file=out)


# ------------------------------------------------------------------- rewrite

def replacement(row):
    """The symbol form that replaces one `path:line` citation."""
    if row.get("form"):
        return row["form"]
    spelled = row["cite"].split(":")[0]
    if row["method"] in ("TITLE", "PHRASE"):
        return f'`{spelled}` "{row["symbol"]}"'
    return f'`{spelled}` `{row["symbol"]}`'


def rewrap(before, after):
    """`after` re-wrapped to the width `before` was written at.

    Only plain prose: a table row, a fence, a header field or a mixed-indent
    block is left exactly as the substitution left it. The flattened text is
    asserted equal on both sides, so a re-wrap can lose a word only by failing
    loudly.
    """
    lines = after.split("\n")
    if len(lines) < 2:
        return after
    if any(l.lstrip().startswith(("|", "```", ">")) or "|" in l for l in lines):
        return after
    if re.match(r"^[a-z_]+:", lines[0]):
        return after
    head = re.match(r"^(\s*(?:[-*]\s+|\d+\.\s+)?)", lines[0]).group(1)
    cont = re.match(r"^(\s*)", lines[1]).group(1)
    if any(re.match(r"^(\s*)", l).group(1) != cont for l in lines[1:]):
        return after
    words = " ".join(after.split()).split(" ")
    out, cur = [], head
    for w in words:
        candidate = cur + w if cur in (head, cont) else cur + " " + w
        if len(candidate) > WIDTH and cur not in (head, cont):
            out.append(cur)
            cur = cont + w
        else:
            cur = candidate
    out.append(cur)
    joined = "\n".join(out)
    assert " ".join(joined.split()) == " ".join(after.split())
    return joined


def apply_rows():
    with open(TSV, encoding="utf-8") as fh:
        head = fh.readline().rstrip("\n").split("\t")
        rows = [dict(zip(head, l.rstrip("\n").split("\t")))
                for l in fh if l.strip()]
    by_step = {}
    for r in rows:
        by_step.setdefault(r["step"], []).append(r)
    changed = 0
    for step, group in sorted(by_step.items()):
        path = os.path.join(REPO, step)
        with open(path, encoding="utf-8") as fh:
            text = fh.read()
        pending = {r["cite"]: replacement(r) for r in group
                   if r["symbol"] or r.get("form")}
        if not pending:
            continue
        blocks = re.split(r"(\n\s*\n)", text)
        for i, block in enumerate(blocks):
            if i % 2:
                continue
            fixed = block
            for cite, sub in sorted(pending.items(), key=lambda kv: -len(kv[0])):
                for form in (f"`{cite}`", cite):
                    if form in fixed:
                        fixed = fixed.replace(form, sub)
            if fixed != block:
                blocks[i] = rewrap(block, fixed)
                changed += 1
        with open(path, "w", encoding="utf-8") as fh:
            fh.write("".join(blocks))
    print(f"rewrote {len(rows)} citations over {len(by_step)} files, "
          f"{changed} paragraphs")


def main():
    what = sys.argv[1] if len(sys.argv) > 1 else "propose"
    if what == "propose":
        write(propose(sys.argv[2] if len(sys.argv) > 2 else None), sys.stdout)
    elif what == "detail":
        want = sys.argv[2]
        for r in propose(sys.argv[3] if len(sys.argv) > 3 else None):
            if want in r["step"]:
                print(f'{r["method"]:5} {r["cite"]:38} -> {r["symbol"]!r:34} '
                      f'{r["kind"]:7} {r["why"]}')
                print(f'      then: {r["then"][:100]}')
    elif what == "apply":
        apply_rows()
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
