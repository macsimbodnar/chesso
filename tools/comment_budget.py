#!/usr/bin/env python3
"""The comment rule's instruments: what breaks it, and that an edit moved no code.

    tools/comment_budget.py --report <ref> [--new-only]
    tools/comment_budget.py --identical <ref> [--against DIR [--stray NAME]...] [--exempt PATH]...
    tools/comment_budget.py --apply EDITS (--tree | --out DIR) [--seeds SEEDS]
    tools/comment_budget.py --new-facts <ref> [--against DIR]
    tools/comment_budget.py --anchors
    tools/comment_budget.py --tiers <ref>

`<ref>` is any commit, and the other side is the working tree unless `--against`
names a directory. `--repo DIR` reads another checkout. Stdlib only.

Exit status: 0 the mode ran and, where it proves something, proved it; 1 a proof
is red or an edit list was refused; 2 the tool could not run, which proves
nothing. The last line is COMMENT-BUDGET-DONE or COMMENT-BUDGET-FAILED on every
exit. `--report`, `--new-facts` and `--tiers` report and never fail on a finding:
no comment check has a measured precision, so none of them is a gate (DEC-336).

--report flags, over live files: log-shaped blocks new or grown against `<ref>`,
three or more ids in a row, `file:line` citations, paths gone from the tree, a
forward claim on a closed step, a GOLDEN without name, command and margin, an id
no plan directory, decision or spec holds. Narration is a lexical list and noisy,
so an old hit is only counted. Every other row says `new` when its line is not in
`<ref>`'s comments, `old` otherwise. No flag looks at length, DEC-336 (1).

--identical proves each changed file by every arm its language has. A changed
file outside `adocs/` with no lexer is red, since it cannot be proved; `.md`,
anything else under `adocs/` without a lexer, and any new file under `adocs/` are
named and not code. A code file added, deleted or changing mode is red.

    lexer       stripped text, for every language; whitespace-only and `\\`-only
                lines read as blank. A blank line after a `\\` still ends the
                continuation, so a macro's extent moving is red. Blind to directives.
    raw-tokens  C++, `clang++ -x c++ -fsyntax-only -Xclang -dump-raw-tokens`, or
    g++-text    `g++ -fpreprocessed -dD -E -P`, by which compiler `$CXX` or `c++`
                says it is. Both see inactive `#if` branches. The first is a cc1
                flag and no stable interface, which is why the lexer stands beside it.
    ast, tokens Python. A file naming `__doc__` keeps its docstrings as code.
    declare-f   shell, each script in a function printed by bash.
    directives  the multiset of `clang-format`, `noqa`, `NOLINT`, `shellcheck` and
                like comments. A moved directive reads unchanged.
    wcomment    a `//` comment ending in `\\`, which g++ -Wcomment stops the build on
                under -Werror and clang does not.

No arm sees a false sentence.

--against compares a directory of copies. A copy with no counterpart at `<ref>` is
red unless `--stray NAME` lists it as expected.

--apply takes `{"ref": <commit>, "edits": [...]}`. An edit is `{"file", "start",
"end", "old", "text"}`, a half-open range of characters against that ref's blob, as
--tiers prints them, or `{"file", "lines": [first, last], "old", "text"}`, whole lines,
1-based. `old` is required and must equal the range exactly, a line's newline
included: it is what catches a drifted range, which no arm can see. `text` defaults
to nothing. A range touching a code token is refused; a lines edit may take a line
whose remainder is a lone `\\` inside a format-off macro, and only that. Overlap is
refused, and so is any result an identity arm reads red. One refusal writes nothing.
`--tree` writes the checkout and refuses a file that differs from the ref; `--out DIR`
writes DIR/<path>.

--seeds takes `{"ref": <commit>, "seeds": [{"id", "file", "start", "end", "text"}]}`:
known-false claims in a writer's scratch copy and never in the tree. A seed keeps its
length, in bytes and in characters, and holds no newline, so the list's offsets and
lines hold against the unseeded ref; one that does not is refused. `old` reads the
writer's copy, seeds in it. An edit overlapping a seed is dropped and listed, DROPPED
<file> edit <n> seed <id>: applied, it would take the true sentence the seed stands on.
A kept edit whose `text` holds a seed's text is refused, in any file. A seed no edit
overlaps is listed too, SEED-UNTOUCHED <id>. Both lists go to the batch's register.

--new-facts lists ids, shas and numbers in new comment text absent from the
file's old comment text. --anchors finds every `tools/mutants/` anchor exactly
once in the working tree. --tiers lists each block with its tier: T1 log-shaped,
T2 any other block of 1200 B or more, T3 the rest.
"""

import argparse
import ast
import bisect
import collections
import io
import json
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import tokenize
import traceback
from dataclasses import dataclass

sys.dont_write_bytecode = True

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = os.path.dirname(os.path.abspath(__file__))

DONE_MARKER = "COMMENT-BUDGET-DONE"
FAILED_MARKER = "COMMENT-BUDGET-FAILED"

EXIT_DONE = 0
EXIT_RED = 1
EXIT_TOOL = 2

VENDORED = ("tests/third_party/", "tests/doctest")
LIVE_ROOTS = ("src/", "tests/", "tools/", "cmake/")
NOT_CLEANED = ("adocs/",) + VENDORED

# The study's selector, read against hand classes: 76 blocks at 55d7e75.
LOG_MIN_BYTES = 1200
LOG_MIN_IDS = 6
LOG_MIN_IDS_PER_KB = 1.5
T2_MIN_BYTES = 1200


class ToolError(Exception):
    """The tool could not run. Never a verdict on the tree."""


# --------------------------------------------------------------------------
# Lexers. Each returns comments in file order and the set of 1-based lines that
# carry code. A line with code and a comment counts as code; its comment is
# trailing.


@dataclass
class Comment:
    start_line: int
    end_line: int
    start_off: int
    end_off: int
    kind: str  # line, block, docstring, shebang
    trailing: bool = False


def line_starts(text):
    starts = [0]
    i = text.find("\n")
    while i != -1:
        starts.append(i + 1)
        i = text.find("\n", i + 1)
    return starts


def line_of(starts, off):
    return bisect.bisect_right(starts, off)


def mark_trailing(comments, code_lines):
    for cm in comments:
        cm.trailing = cm.start_line in code_lines
    return comments, code_lines


RAW_PREFIX = re.compile(r'(?:u8|u|U|L)?R"$')
CPP_NEXT = re.compile(r"""//|/\*|"|'|[^\s/"']+|/""")


def lex_cpp(text):
    """C and C++: comments, strings, char literals, raw strings, digit separators."""
    comments = []
    code_lines = set()
    starts = line_starts(text)
    n = len(text)
    i = 0
    while i < n:
        m = CPP_NEXT.search(text, i)
        if not m:
            break
        i = m.start()
        tok = m.group(0)
        if tok == "//":
            j = i + 2
            while j < n:
                j = text.find("\n", j)
                if j == -1:
                    j = n
                    break
                # a backslash-newline continues a line comment, spaces between them too:
                # both compilers splice it, with a warning
                k = j - 1
                while k > i and text[k] in " \t":
                    k -= 1
                if text[k] == "\\":
                    j += 1
                    continue
                break
            comments.append(Comment(line_of(starts, i), line_of(starts, max(i, j - 1)), i, j, "line"))
            i = j
            continue
        if tok == "/*":
            j = text.find("*/", i + 2)
            j = n if j == -1 else j + 2
            comments.append(Comment(line_of(starts, i), line_of(starts, j - 1), i, j, "block"))
            i = j
            continue
        if tok == '"':
            k = i - 1
            is_raw = False
            if k >= 0 and text[k] == "R":
                head = text[max(0, k - 2) : i + 1]
                raw = RAW_PREFIX.search(head)
                if raw:
                    before = max(0, k - 2) + raw.start() - 1
                    is_raw = not (before >= 0 and (text[before].isalnum() or text[before] == "_"))
            if is_raw:
                p = text.find("(", i + 1)
                delim = text[i + 1 : p]
                end = text.find(")" + delim + '"', p)
                end = n if end == -1 else end + len(delim) + 2
                code_lines.update(range(line_of(starts, i), line_of(starts, end - 1) + 1))
                i = end
                continue
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                elif text[j] == "\n":
                    break  # unterminated: a stray quote in preprocessor text or asm
                j += 1
            code_lines.update(range(line_of(starts, i), line_of(starts, min(j, n - 1)) + 1))
            i = j + 1
            continue
        if tok == "'":
            # digit separator: 1'000'000
            if i > 0 and text[i - 1].isalnum() and i + 1 < n and text[i + 1].isalnum():
                code_lines.add(line_of(starts, i))
                i += 1
                continue
            j = i + 1
            while j < n and text[j] != "'":
                if text[j] == "\\":
                    j += 1
                elif text[j] == "\n":
                    break
                j += 1
            code_lines.add(line_of(starts, i))
            i = j + 1
            continue
        code_lines.add(line_of(starts, i))
        i = m.end()
    return mark_trailing(comments, code_lines)


def docstring_spans(text):
    """(start line, end line, start col, end col) of each docstring. Columns are utf-8 bytes."""
    spans = []
    for node in ast.walk(ast.parse(text)):
        if isinstance(node, (ast.Module, ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)):
            body = node.body
            if (
                body
                and isinstance(body[0], ast.Expr)
                and isinstance(body[0].value, ast.Constant)
                and isinstance(body[0].value.value, str)
            ):
                d = body[0].value
                spans.append((d.lineno, d.end_lineno, d.col_offset, d.end_col_offset))
    return spans


def lex_python(text):
    """tokenize for comments, ast for docstrings. Raises on unparsable source."""
    comments = []
    starts = line_starts(text)
    code_lines = set()
    spans = docstring_spans(text)
    lines = text.split("\n")

    def off(line, col):
        return starts[line - 1] + col

    for tok in tokenize.generate_tokens(io.StringIO(text).readline):
        if tok.type == tokenize.COMMENT:
            kind = "shebang" if tok.start == (1, 0) and tok.string.startswith("#!") else "line"
            comments.append(Comment(tok.start[0], tok.end[0], off(*tok.start), off(*tok.end), kind))
        elif tok.type in (tokenize.NL, tokenize.NEWLINE, tokenize.INDENT, tokenize.DEDENT, tokenize.ENDMARKER):
            continue
        else:
            if tok.type == tokenize.STRING and any(
                tok.start[0] == s and tok.end[0] == e for s, e, _, _ in spans
            ):
                continue
            code_lines.update(range(tok.start[0], tok.end[0] + 1))
    for s, e, sc, ec in spans:
        # ast's byte columns mapped back to characters on the line
        sline = lines[s - 1].encode("utf-8", "surrogateescape")[:sc].decode("utf-8", "replace")
        eline = lines[e - 1].encode("utf-8", "surrogateescape")[:ec].decode("utf-8", "replace")
        comments.append(Comment(s, e, off(s, len(sline)), off(e, len(eline)), "docstring"))
    comments.sort(key=lambda c: c.start_off)
    return mark_trailing(comments, code_lines)


def skip_backtick(text, i):
    """Index of the backtick closing the one at `i`."""
    n = len(text)
    j = i + 1
    while j < n and text[j] != "`":
        if text[j] == "\\":
            j += 1
        j += 1
    return min(j, n)


def skip_param(text, i):
    """Index of the `}` closing a `${` opened just before `i`. Quotes nest in it."""
    n = len(text)
    j = i
    while j < n:
        c = text[j]
        if c == "\\":
            j += 2
        elif c == "}":
            return j
        elif c == '"':
            j = skip_dquote(text, j) + 1
        elif text.startswith("$(", j):
            j = skip_subst(text, j + 2) + 1
        elif text.startswith("${", j):
            j = skip_param(text, j + 2) + 1
        elif c == "`":
            j = skip_backtick(text, j) + 1
        else:
            j += 1
    return n


def skip_subst(text, i):
    """Index of the `)` closing a `$(` opened just before `i`. Everything in it is
    a fresh shell: quotes nest, and a `#` at a word start runs to the line's end."""
    n = len(text)
    j = i
    depth = 1
    word_start = True
    while j < n:
        c = text[j]
        if c == "\\":
            j += 2
            word_start = False
        elif c == "'":
            found = text.find("'", j + 1)
            j = n if found == -1 else found + 1
            word_start = False
        elif c == '"':
            j = skip_dquote(text, j) + 1
            word_start = False
        elif text.startswith("$(", j):
            j = skip_subst(text, j + 2) + 1
            word_start = False
        elif text.startswith("${", j):
            j = skip_param(text, j + 2) + 1
            word_start = False
        elif c == "`":
            j = skip_backtick(text, j) + 1
            word_start = False
        elif c == "#" and word_start:
            found = text.find("\n", j)
            j = n if found == -1 else found
        elif c == "(":
            depth += 1
            j += 1
            word_start = True
        elif c == ")":
            depth -= 1
            if depth == 0:
                return j
            j += 1
            word_start = True
        else:
            word_start = c in " \t\n;&|"
            j += 1
    return n


def skip_dquote(text, i):
    """Index of the `"` closing the one at `i`. `$(...)` and `${...}` hold quotes of
    their own, so the first `"` met inside them closes nothing here."""
    n = len(text)
    j = i + 1
    while j < n:
        c = text[j]
        if c == "\\":
            j += 2
        elif c == '"':
            return j
        elif text.startswith("$(", j):
            j = skip_subst(text, j + 2) + 1
        elif text.startswith("${", j):
            j = skip_param(text, j + 2) + 1
        elif c == "`":
            j = skip_backtick(text, j) + 1
        else:
            j += 1
    return n


HEREDOC = re.compile(r"<<(-?)\s*(['\"]?)([A-Za-z_][A-Za-z0-9_]*)\2")


def lex_shell(text):
    """Bash: quotes, $'...', heredocs, `$#` and `${#x}`. Comments begin at a word start."""
    comments = []
    code_lines = set()
    starts = line_starts(text)
    n = len(text)
    i = 0
    pending_heredocs = []  # delimiters waiting for the next newline
    word_start = True
    while i < n:
        c = text[i]
        if c == "\n":
            i += 1
            word_start = True
            while pending_heredocs:
                delim, strip_tabs = pending_heredocs.pop(0)
                # the body is code: data the script emits, not commentary
                while i < n:
                    j = text.find("\n", i)
                    j = n if j == -1 else j
                    line = text[i:j]
                    code_lines.add(line_of(starts, i))
                    i = min(j + 1, n)
                    if (line.lstrip("\t") if strip_tabs else line) == delim:
                        break
            continue
        if c == "#" and word_start:
            j = text.find("\n", i)
            j = n if j == -1 else j
            kind = "shebang" if i == 0 and text.startswith("#!") else "line"
            comments.append(Comment(line_of(starts, i), line_of(starts, i), i, j, kind))
            i = j
            continue
        if c == "\\":
            code_lines.add(line_of(starts, i))
            i += 2
            word_start = False
            continue
        if c == "'":
            j = text.find("'", i + 1)
            j = n - 1 if j == -1 else j
            code_lines.update(range(line_of(starts, i), line_of(starts, j) + 1))
            i = j + 1
            word_start = False
            continue
        if c == "$" and i + 1 < n and text[i + 1] == "'":
            j = i + 2
            while j < n and text[j] != "'":
                if text[j] == "\\":
                    j += 1
                j += 1
            code_lines.update(range(line_of(starts, i), line_of(starts, min(j, n - 1)) + 1))
            i = j + 1
            word_start = False
            continue
        if c == '"':
            j = skip_dquote(text, i)
            code_lines.update(range(line_of(starts, i), line_of(starts, min(j, n - 1)) + 1))
            i = j + 1
            word_start = False
            continue
        if c == "<" and text.startswith("<<", i) and not text.startswith("<<<", i):
            m = HEREDOC.match(text, i)
            if m:
                pending_heredocs.append((m.group(3), m.group(1) == "-"))
                code_lines.add(line_of(starts, i))
                i = m.end()
                word_start = False
                continue
        if c in " \t;&|()":
            word_start = True
            if not c.isspace():
                code_lines.add(line_of(starts, i))
            i += 1
            continue
        code_lines.add(line_of(starts, i))
        word_start = False
        i += 1
    return mark_trailing(comments, code_lines)


CMAKE_BRACKET_COMMENT = re.compile(r"#\[(=*)\[")
CMAKE_BRACKET_ARG = re.compile(r"\[(=*)\[")


def lex_cmake(text):
    """CMake: `#`, bracket comments `#[[ ]]`, quoted args, bracket args."""
    comments = []
    code_lines = set()
    starts = line_starts(text)
    n = len(text)
    i = 0
    while i < n:
        c = text[i]
        if c == "#":
            m = CMAKE_BRACKET_COMMENT.match(text, i)
            if m:
                end = text.find("]" + m.group(1) + "]", m.end())
                end = n if end == -1 else end + len(m.group(1)) + 2
                comments.append(Comment(line_of(starts, i), line_of(starts, end - 1), i, end, "block"))
                i = end
                continue
            j = text.find("\n", i)
            j = n if j == -1 else j
            comments.append(Comment(line_of(starts, i), line_of(starts, i), i, j, "line"))
            i = j
            continue
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                j += 1
            code_lines.update(range(line_of(starts, i), line_of(starts, min(j, n - 1)) + 1))
            i = j + 1
            continue
        m = CMAKE_BRACKET_ARG.match(text, i)
        if m:
            end = text.find("]" + m.group(1) + "]", m.end())
            end = n if end == -1 else end + len(m.group(1)) + 2
            code_lines.update(range(line_of(starts, i), line_of(starts, end - 1) + 1))
            i = end
            continue
        if not c.isspace():
            code_lines.add(line_of(starts, i))
        i += 1
    return mark_trailing(comments, code_lines)


def lex_awk(text):
    """awk: `#` outside strings. A regex literal holding `#` is not handled."""
    comments = []
    code_lines = set()
    starts = line_starts(text)
    n = len(text)
    i = 0
    while i < n:
        c = text[i]
        if c == "#":
            j = text.find("\n", i)
            j = n if j == -1 else j
            comments.append(Comment(line_of(starts, i), line_of(starts, i), i, j, "line"))
            i = j
            continue
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                j += 1
            code_lines.add(line_of(starts, i))
            i = j + 1
            continue
        if not c.isspace():
            code_lines.add(line_of(starts, i))
        i += 1
    return mark_trailing(comments, code_lines)


def language_of(path):
    base = path.rsplit("/", 1)[-1]
    if base == "CMakeLists.txt" or base.endswith(".cmake"):
        return "cmake"
    ext = base.rsplit(".", 1)[-1] if "." in base else ""
    return {
        "cpp": "cpp",
        "hpp": "cpp",
        "h": "cpp",
        "cc": "cpp",
        "S": "cpp",
        "py": "python",
        "sh": "shell",
        "awk": "awk",
    }.get(ext)


LEXERS = {
    "cpp": lex_cpp,
    "python": lex_python,
    "shell": lex_shell,
    "cmake": lex_cmake,
    "awk": lex_awk,
}


def lex(path, text):
    return LEXERS[language_of(path)](text)


def strip_comments(text, comments, keep_lines=False):
    """The text with comment characters removed and whole comment lines dropped.

    With `keep_lines` a whole comment line leaves a blank one: a comment on the line
    after a `\\` is what ends that continuation, and dropping the line would hide
    the macro growing when the comment goes.
    """
    out = []
    pos = 0
    starts = line_starts(text)
    for cm in comments:
        if cm.kind == "shebang":
            continue
        s = cm.start_off
        e = cm.end_off
        if not cm.trailing:
            # whole-line comment: take the indentation and the newline with it
            s = starts[cm.start_line - 1]
            nl = text.find("\n", e)
            if nl != -1 and text[e:nl].strip() == "":
                e = nl if keep_lines else nl + 1
        else:
            while s > 0 and text[s - 1] in " \t":
                s -= 1
        out.append(text[pos:s])
        pos = e
    out.append(text[pos:])
    return "".join(out)


# --------------------------------------------------------------------------
# What counts as code.


def reads_doc(text):
    """The file names `__doc__` in code, so a docstring there changes behaviour."""
    try:
        for tok in tokenize.generate_tokens(io.StringIO(text).readline):
            if tok.type == tokenize.NAME and tok.string == "__doc__":
                return True
    except (tokenize.TokenError, IndentationError) as exc:
        raise SyntaxError(str(exc))
    return False


def editable_comments(path, text):
    """The comments an edit may touch: a shebang and a `__doc__` docstring are code."""
    comments, _ = lex(path, text)
    keep_doc = language_of(path) == "python" and reads_doc(text)
    return [c for c in comments if c.kind != "shebang" and not (keep_doc and c.kind == "docstring")]


def logical_lines(stripped):
    """Physical lines joined on a trailing `\\`, blank lines dropped.

    A blank line ends a continuation, so one taken away or added after a `\\` moves
    the extent of a macro and shows here as a different logical line. A `\\`-only
    line inside a continuation adds nothing.
    """
    out = []
    current = None
    for raw in stripped.split("\n"):
        line = raw.rstrip()
        continued = line.endswith("\\")
        body = line[:-1].rstrip() if continued else line
        if current is None:
            if not body and not continued:
                continue
            current = []
        if body:
            current.append(body)
        if not continued:
            out.append(tuple(current))
            current = None
    if current is not None:
        out.append(tuple(current))
    return out


def first_difference(old, new):
    for index, (a, b) in enumerate(zip(old, new)):
        if a != b:
            # One macro is one logical line of a thousand: say which of its lines moved.
            if isinstance(a, tuple) and isinstance(b, tuple):
                return first_difference(a, b)
            return f"entry {index + 1}: old {str(a)[:120]!r} new {str(b)[:120]!r}"
    if len(old) != len(new):
        longer, shorter = (old, new) if len(old) > len(new) else (new, old)
        side = "old" if longer is old else "new"
        return f"{side} has {len(longer) - len(shorter)} more, next {str(longer[len(shorter)])[:120]!r}"
    return "no entry differs"


COMMENT_LEADER = r"(?:^(?://+|/\*+)\s*|(?:^|\s)#+\s*)"
DIRECTIVE = re.compile(
    COMMENT_LEADER
    + r"(clang-format\s+(?:off|on)\b"
    r"|NOLINT(?:NEXTLINE|BEGIN|END)?(?:\([^)\n]*\))?"
    r"|noqa\b(?::\s*[\w, ]*\w)?"
    r"|shellcheck\s+\w[\w-]*=\S+"
    r"|type:\s*ignore(?:\[[^\]\n]*\])?"
    r"|pylint:\s*\S+"
    r"|pragma:\s*no\s+\w+"
    r"|fmt:\s*(?:off|on)\b"
    r"|yapf:\s*\w+"
    r"|flake8:\s*\S+"
    r"|mypy:\s*\S+"
    r"|nosec\b"
    r"|cppcheck-suppress\s+\S+"
    r"|IWYU\s+pragma:\s*\S+"
    r"|LCOV_EXCL_\w+)",
    re.M,
)


def directive_multiset(text, comments):
    found = collections.Counter()
    for cm in comments:
        if cm.kind == "docstring":
            continue
        for m in DIRECTIVE.finditer(text[cm.start_off : cm.end_off]):
            found[" ".join(m.group(1).split())] += 1
    return found


WCOMMENT = re.compile(r"\\[ \t]*(?:\n|$)")


def wcomment_lines(text, comments):
    """Lines of `//` comments holding a backslash at a line's end."""
    hits = []
    for cm in comments:
        body = text[cm.start_off : cm.end_off]
        if cm.kind == "line" and body.startswith("//") and WCOMMENT.search(body):
            hits.append(cm.start_line)
    return hits


class Compiler:
    """The C++ arm, resolved once, and only when a C++ file needs it."""

    def __init__(self):
        self.family = None
        self.path = None

    def resolve(self):
        if self.path:
            return
        wanted = os.environ.get("CXX")
        candidates = [wanted] if wanted else ["c++", "clang++", "g++"]
        for name in candidates:
            path = shutil.which(name)
            if not path:
                continue
            proc = subprocess.run([path, "--version"], capture_output=True)
            banner = (proc.stdout + proc.stderr).decode("utf-8", "replace").lower()
            if "clang" in banner:
                self.family, self.path = "clang", path
                return
            if "free software foundation" in banner or "(gcc)" in banner or "g++" in banner:
                self.family, self.path = "g++", path
                return
        raise ToolError("no C++ compiler that identifies as clang or g++, tried " + ", ".join(candidates))

    @property
    def arm(self):
        self.resolve()
        return "raw-tokens" if self.family == "clang" else "g++-text"

    def view(self, path, text):
        """What the compiler's own lexer reads in `text`, comments and spacing gone."""
        self.resolve()
        ext = os.path.splitext(path)[1] or ".cpp"
        with tempfile.TemporaryDirectory() as tmp:
            source = os.path.join(tmp, "under_test" + ext)
            with open(source, "wb") as out:
                out.write(text.encode("utf-8", "surrogateescape"))
            if self.family == "clang":
                return self._raw_tokens(source, text)
            return self._gpp_text(source)

    def _raw_tokens(self, source, text):
        proc = subprocess.run(
            [self.path, "-x", "c++", "-fsyntax-only", "-Xclang", "-dump-raw-tokens", source],
            capture_output=True,
        )
        dump = proc.stderr.decode("utf-8", "replace")
        # This is a cc1 flag. Each of these three would compare equal to itself on
        # both sides, and read as a proof.
        if proc.returncode != 0:
            raise ToolError(f"{self.path} -dump-raw-tokens exited {proc.returncode}: {dump[:200]!r}")
        tokens = []
        records = 0
        for record in re.split(r"\tLoc=<[^>]*>\n", dump):
            record = record.strip("\n")
            if not record:
                continue
            records += 1
            m = re.match(r"^(\S+) '(.*)'\t?( \[.*\])?$", record, re.S)
            if not m:
                raise ToolError(f"{self.path} -dump-raw-tokens printed an unreadable record: {record[:120]!r}")
            kind, spelling = m.group(1), m.group(2)
            if kind == "comment" or (kind == "unknown" and not spelling.strip()):
                continue
            tokens.append((kind, spelling))
        if not records and text.strip():
            raise ToolError(f"{self.path} -dump-raw-tokens printed nothing: {dump[:200]!r}")
        return tokens

    def _gpp_text(self, source):
        proc = subprocess.run(
            [self.path, "-fpreprocessed", "-dD", "-E", "-P", "-x", "c++", source],
            capture_output=True,
        )
        if proc.returncode != 0:
            raise ToolError(f"{self.path} -fpreprocessed: {proc.stderr.decode('utf-8', 'replace')[:200]}")
        # -fpreprocessed does not splice, so a `\\`-only line taken from a macro would
        # read as a change: splice here, as the compiler proper does. Whitespace folds to
        # one space and is not removed, or `int x` and `intx` compare equal. A `\\` left
        # before space is a continuation a re-spacing printer has moved.
        printed = proc.stdout.decode("utf-8", "surrogateescape")
        spliced = re.sub(r"\\[ \t]*\n", "", printed)
        return [re.sub(r"\s+", " ", re.sub(r"\\\s+", " ", spliced)).strip()]


def python_views(text):
    """(ast dump, tokens), docstrings out unless the file reads `__doc__`."""
    keep_doc = reads_doc(text)
    tree = ast.parse(text)
    doc_starts = set()
    if not keep_doc:
        for node in ast.walk(tree):
            if isinstance(node, (ast.Module, ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)):
                body = node.body
                if (
                    body
                    and isinstance(body[0], ast.Expr)
                    and isinstance(body[0].value, ast.Constant)
                    and isinstance(body[0].value.value, str)
                ):
                    doc_starts.add((body[0].value.lineno, body[0].value.end_lineno))
                    node.body = body[1:]
    tokens = []
    skip_newline = False
    for tok in tokenize.generate_tokens(io.StringIO(text).readline):
        if tok.type in (tokenize.COMMENT, tokenize.NL):
            continue
        if skip_newline:
            skip_newline = False
            if tok.type == tokenize.NEWLINE:
                continue
        if tok.type == tokenize.STRING and (tok.start[0], tok.end[0]) in doc_starts:
            skip_newline = True
            continue
        tokens.append((tok.type, tok.string))
    return ast.dump(tree), tokens


def shell_print(text):
    """What bash's own parser makes of the script, printed back. (status, output)."""
    wrapped = "__comment_budget_identity__() {\n" + text + "\n}\ndeclare -f __comment_budget_identity__\n"
    env = {k: v for k, v in os.environ.items() if k != "BASH_ENV"}
    with tempfile.TemporaryDirectory() as tmp:
        source = os.path.join(tmp, "wrapped.sh")
        with open(source, "wb") as out:
            out.write(wrapped.encode("utf-8", "surrogateescape"))
        try:
            proc = subprocess.run(["bash", source], capture_output=True, env=env)
        except FileNotFoundError:
            raise ToolError("no bash on PATH for the shell arm")
    return proc.returncode, proc.stdout


def identity_failures(path, old, new, compiler):
    """[(arm, detail)] for one changed file, and the arms read. Empty proves it."""
    lang = language_of(path)
    arms = ["lexer", "directives"]
    failures = []

    try:
        old_comments = editable_comments(path, old)
    except (SyntaxError, ValueError) as exc:
        raise ToolError(f"{path}: the old side does not parse: {exc}")
    try:
        new_comments = editable_comments(path, new)
    except (SyntaxError, ValueError) as exc:
        return [("lexer", f"the new side does not parse: {exc}")], arms

    old_lines = logical_lines(strip_comments(old, old_comments, keep_lines=True))
    new_lines = logical_lines(strip_comments(new, new_comments, keep_lines=True))
    if old_lines != new_lines:
        failures.append(("lexer", first_difference(old_lines, new_lines)))

    old_dirs = directive_multiset(old, old_comments)
    new_dirs = directive_multiset(new, new_comments)
    if old_dirs != new_dirs:
        lost = sorted((old_dirs - new_dirs).elements())
        gained = sorted((new_dirs - old_dirs).elements())
        failures.append(("directives", f"lost {lost} gained {gained}"))

    if lang == "cpp":
        arm = compiler.arm
        arms += [arm, "wcomment"]
        old_view = compiler.view(path, old)
        new_view = compiler.view(path, new)
        if old_view != new_view:
            failures.append((arm, first_difference(old_view, new_view)))
        hits = wcomment_lines(new, new_comments)
        if hits:
            failures.append(("wcomment", f"a // comment ends in a backslash at line {hits[0]}"))
    elif lang == "python":
        arms += ["ast", "tokens"]
        old_ast, old_tokens = python_views(old)
        new_ast, new_tokens = python_views(new)
        if old_ast != new_ast:
            failures.append(("ast", "the syntax trees differ"))
        if old_tokens != new_tokens:
            failures.append(("tokens", first_difference(old_tokens, new_tokens)))
    elif lang == "shell":
        arms.append("declare-f")
        old_status, old_print = shell_print(old)
        if old_status != 0:
            raise ToolError(f"{path}: bash cannot parse the old side")
        new_status, new_print = shell_print(new)
        if new_status != 0:
            failures.append(("declare-f", "bash cannot parse the new side"))
        elif old_print != new_print:
            failures.append(
                (
                    "declare-f",
                    first_difference(
                        old_print.decode("utf-8", "replace").split("\n"),
                        new_print.decode("utf-8", "replace").split("\n"),
                    ),
                )
            )
    return failures, arms


# --------------------------------------------------------------------------
# The repository.


def git(repo, *args):
    proc = subprocess.run(["git", "-C", repo, *args], capture_output=True)
    if proc.returncode != 0:
        raise ToolError("git " + " ".join(args) + ": " + proc.stderr.decode("utf-8", "replace").strip())
    return proc.stdout


def resolve_ref(repo, ref):
    return git(repo, "rev-parse", "--verify", "--quiet", ref + "^{commit}").decode().strip()


def decode(raw):
    return raw.decode("utf-8", "surrogateescape")


def encode(text):
    return text.encode("utf-8", "surrogateescape")


def blob_text(repo, commit, path):
    return decode(git(repo, "cat-file", "blob", f"{commit}:{path}"))


def blobs_at(repo, commit, paths):
    """{path: text} for many paths through one process, where a census reads hundreds."""
    request = "".join(f"{commit}:{path}\n" for path in paths)
    proc = subprocess.run(
        ["git", "-C", repo, "cat-file", "--batch"], input=encode(request), capture_output=True
    )
    if proc.returncode != 0:
        raise ToolError("git cat-file --batch: " + proc.stderr.decode("utf-8", "replace").strip())
    out = proc.stdout
    texts = {}
    pos = 0
    for path in paths:
        end = out.index(b"\n", pos)
        header = out[pos:end].split()
        if len(header) != 3:
            raise ToolError(f"git cat-file --batch: {path} at {commit[:7]}: {out[pos:end].decode()}")
        size = int(header[2])
        texts[path] = decode(out[end + 1 : end + 1 + size])
        pos = end + 1 + size + 1
    return texts


def open_bytes(path):
    with open(path, "rb") as handle:
        return handle.read()


def read_file(path):
    return decode(open_bytes(path))


def tracked_at(repo, commit):
    raw = git(repo, "ls-tree", "-r", "-z", "--name-only", commit)
    return [p for p in decode(raw).split("\0") if p]


def tree_files(repo):
    raw = git(repo, "ls-files", "--cached", "--others", "--exclude-standard", "-z")
    return sorted({p for p in decode(raw).split("\0") if p and os.path.isfile(os.path.join(repo, p))})


def is_live(path):
    if path.startswith(VENDORED) or language_of(path) is None:
        return False
    return path.startswith(LIVE_ROOTS) or "/" not in path


def is_not_code(path):
    return language_of(path) is None and (path.endswith(".md") or path.startswith("adocs/"))


def changed_in_tree(repo, commit):
    """[(status, path, old mode, new mode)] of the working tree against the commit."""
    raw = git(repo, "diff", "--raw", "--no-renames", "--abbrev=40", "-z", "--ignore-submodules=dirty", commit)
    fields = decode(raw).split("\0")
    out = []
    for meta, path in zip(fields[0::2], fields[1::2]):
        parts = meta.split()
        out.append((parts[4], path, parts[0].lstrip(":"), parts[1]))
    others = git(repo, "ls-files", "--others", "--exclude-standard", "-z")
    for path in decode(others).split("\0"):
        if path:
            out.append(("A", path, "000000", "100644"))
    return sorted(out, key=lambda row: row[1])


def against_pairs(repo, commit, directory):
    """Files under the directory mapped to their paths at the commit.

    By relative path, then by a basename unique among tracked files outside
    `adocs/`, since a flat directory of scratch copies keeps only the basename.
    """
    tracked = set(tracked_at(repo, commit))
    bases = collections.defaultdict(list)
    for path in tracked:
        if not path.startswith("adocs/"):
            bases[os.path.basename(path)].append(path)
    pairs, strays = [], []
    for root, _dirs, files in os.walk(directory):
        for name in sorted(files):
            full = os.path.join(root, name)
            rel = os.path.relpath(full, directory).replace(os.sep, "/")
            if rel in tracked:
                pairs.append((rel, full))
            elif len(bases.get(name, [])) == 1:
                pairs.append((bases[name][0], full))
            else:
                strays.append(rel)
    return sorted(pairs), sorted(strays)


# --------------------------------------------------------------------------
# Blocks and tiers.


@dataclass
class Block:
    path: str
    start_line: int
    end_line: int
    start_off: int
    end_off: int
    kind: str
    trailing: bool
    text: str
    size: int  # comment bytes, indentation between merged lines not counted


ID_RE = re.compile(r"\bS\d{3}\b|\bDEC-\d{3}\b")


def blocks_of(path, text):
    """Adjacent whole-line comments merge; a trailing comment, a block comment and a
    docstring stand alone."""
    comments, _ = lex(path, text)
    blocks = []
    mergeable = False
    for cm in comments:
        if cm.kind == "shebang":
            continue
        size = len(encode(text[cm.start_off : cm.end_off]))
        whole_line_comment = not cm.trailing and cm.kind == "line"
        if whole_line_comment and blocks and mergeable and cm.start_line == blocks[-1].end_line + 1:
            last = blocks[-1]
            last.end_line = cm.end_line
            last.end_off = cm.end_off
            last.size += size
            last.text = text[last.start_off : last.end_off]
            continue
        blocks.append(
            Block(
                path,
                cm.start_line,
                cm.end_line,
                cm.start_off,
                cm.end_off,
                cm.kind,
                cm.trailing,
                text[cm.start_off : cm.end_off],
                size,
            )
        )
        mergeable = whole_line_comment
    return blocks


def distinct_ids(block):
    return len(set(ID_RE.findall(block.text)))


def is_log_shaped(block):
    ids = distinct_ids(block)
    return (
        block.size >= LOG_MIN_BYTES
        and ids >= LOG_MIN_IDS
        and ids / (block.size / 1000) >= LOG_MIN_IDS_PER_KB
    )


def tier_of(block):
    if is_log_shaped(block):
        return "T1"
    return "T2" if block.size >= T2_MIN_BYTES else "T3"


def live_blocks(path, text, skipped):
    try:
        return blocks_of(path, text)
    except (SyntaxError, ValueError, tokenize.TokenError) as exc:
        skipped.append(f"{path} (does not lex: {exc})")
        return []


def run_tiers(repo, ref):
    commit = resolve_ref(repo, ref)
    print("tier\tfile\tstart_line\tend_line\tstart_off\tend_off\tbytes\tids")
    totals = collections.Counter()
    sizes = collections.Counter()
    skipped = []
    paths = [p for p in tracked_at(repo, commit) if is_live(p)]
    texts = blobs_at(repo, commit, paths)
    for path in paths:
        for b in live_blocks(path, texts[path], skipped):
            tier = tier_of(b)
            totals[tier] += 1
            sizes[tier] += b.size
            print(
                f"{tier}\t{path}\t{b.start_line}\t{b.end_line}\t{b.start_off}\t{b.end_off}"
                f"\t{b.size}\t{distinct_ids(b)}"
            )
    for line in skipped:
        print("SKIPPED", line)
    for tier in ("T1", "T2", "T3"):
        print(f"{tier} {totals[tier]} blocks {sizes[tier]} B")
    total = sum(totals.values())
    return EXIT_DONE, f"tiers {len(paths)} files {total} blocks {sum(sizes.values())} B at {commit[:7]}"


# --------------------------------------------------------------------------
# --report


FILE_LINE = re.compile(r"\b([\w./-]+\.(?:cpp|hpp|py|sh|md|txt|cmake)):(\d+)(?:-\d+)?\b")
FORWARD = re.compile(
    r"\b(S\d{3})\s+(?:owns|owes|will|is pending|is open|takes this|moves this|holds this|has yet|remains|still owes)\b"
    r"|\b(?:until|once|after|when|before|waits? (?:for|on))\s+(S\d{3})\s+(?:lands?|closes?|is done|ships?)\b"
    r"|\b(?:an? H[01] (?:on|for)|on an H[01] (?:on|for)|pending)\s+(S\d{3})\b"
)
STEP_ID = re.compile(r"\bS\d{3}\b")
DEC_ID = re.compile(r"\bDEC-\d{3}\b")
INV_ID = re.compile(r"\bINV-\d+\b")
TREE_PATH = re.compile(r"(?<![\w/.-])((?:src|tests|tools|cmake|adocs)/[\w./-]+\.\w+)")
ANY_ID = r"(?:S\d{3}|DEC-\d{3}|INV-\d+)"
ID_CHAIN = re.compile(
    r"\b" + ANY_ID + r"(?:(?:\s*(?:,|;|/|->|\+|\band\b)\s*|\s+)" + ANY_ID + r"){2,}\b"
)
GOLDEN_MARK = re.compile(r"(?<!NOT A )\bGOLDEN\b(?=\s*[(:,]|\s*$)|\(GOLDEN\b")
GOLDEN_NAME = re.compile(r"\bGOLDEN\b\s*(?:\([^)]*\))?\s*(?:(?:since|until)\b[^:]*)?[:,.)]?\s*[\w`\"'-]")
GOLDEN_COMMAND = re.compile(
    r"\b(?:python3?|bash|ctest|cmake|stockfish)\b"
    r"|(?<![\w/.-])(?:\.{0,2}/)?(?:adocs|tools|tests|build[\w-]*|src)/[\w./-]+\.(?:py|sh)\b"
    r"|(?<![\w/.-])build[\w-]*/[\w./-]+"
    r"|~/[\w./-]+"
    r"|\bchesso bench\b",
)
GOLDEN_MARGIN = re.compile(r"\bmargin\b", re.I)
NARRATION = re.compile(
    r"\b(?:previously|formerly|originally|used to|no longer|now uses|at the time"
    r"|(?:was|were) (?:changed|added|removed|moved|renamed|replaced|rewritten|fixed)"
    r"|this (?:step|commit|change|patch)"
    r"|the (?:old|previous) (?:code|version|implementation)"
    r"|we (?:added|changed|removed|moved|fixed)"
    r"|(?:until|before|after|since) S\d{3}"
    r"|S\d{3} (?:added|moved|removed|changed|fixed|rewrote|replaced|introduced))\b"
)
PIECE_LEADER = re.compile(r"^\s*(?://+!?|/\*+|\*+(?!/)|#+!?|\"\"\"|''')?\s?")
PIECE_TAIL = re.compile(r"\s*(?:\*+/|\"\"\"|''')?\s*\\?\s*$")


def comment_pieces(block):
    """(line, text) of a block with its comment leaders and continuation marks gone."""
    out = []
    for offset, raw in enumerate(block.text.split("\n")):
        piece = PIECE_TAIL.sub("", PIECE_LEADER.sub("", raw, count=1), count=1)
        if piece:
            out.append((block.start_line + offset, piece))
    return out


class FlatBlock:
    """A block's text on one line, since an id and its verb wrap across lines."""

    def __init__(self, block):
        self.text = ""
        self.starts = []
        self.lines = []
        self.pieces = []
        for line, piece in comment_pieces(block):
            self.starts.append(len(self.text))
            self.lines.append(line)
            self.pieces.append(piece)
            self.text += piece + " "

    def line_at(self, off):
        return self.lines[bisect.bisect_right(self.starts, off) - 1]

    def pieces_between(self, start, end):
        first = bisect.bisect_right(self.starts, start) - 1
        last = bisect.bisect_right(self.starts, max(start, end - 1)) - 1
        return self.pieces[first : last + 1]


def known_ids(repo):
    steps, done = set(), set()
    for directory in ("plan_done", "plan_todo", "plan_current"):
        full = os.path.join(repo, "adocs", directory)
        if not os.path.isdir(full):
            continue
        for name in os.listdir(full):
            if re.match(r"S\d{3}", name):
                steps.add(name[:4])
                if directory == "plan_done":
                    done.add(name[:4])
    decisions, invariants = set(), set()
    decisions_md = os.path.join(repo, "adocs", "decisions.md")
    if os.path.isfile(decisions_md):
        decisions = set(re.findall(r"^## (DEC-\d+)", read_file(decisions_md), re.M))
    specs_md = os.path.join(repo, "adocs", "specs.md")
    if os.path.isfile(specs_md):
        invariants = set(INV_ID.findall(read_file(specs_md)))
    return steps, done, decisions, invariants


def exists_in_tree(path, tree):
    i = bisect.bisect_left(tree, path)
    return i < len(tree) and tree[i].startswith(path)


def log_state(block, old_blocks, old_piece_owner):
    """new, grown, same, or changed: the block against the old block it shares most lines with."""
    hits = collections.Counter()
    for _line, piece in comment_pieces(block):
        for owner in old_piece_owner.get(piece, ()):
            hits[owner] += 1
    if not hits:
        return "new"
    old = old_blocks[hits.most_common(1)[0][0]]
    if old.text == block.text:
        return "same"
    return "grown" if block.size > old.size else "changed"


def block_findings(block, known, tree):
    """(kind, start offset in the flat text, end offset, what) over one block."""
    steps, done, decisions, invariants = known
    flat = FlatBlock(block)
    text = flat.text
    found = []
    for m in ID_CHAIN.finditer(text):
        found.append(("ID-CHAIN", m.start(), m.end(), m.group(0)))
    for m in FILE_LINE.finditer(text):
        found.append(("FILE-LINE", m.start(), m.end(), m.group(0)))
    for m in TREE_PATH.finditer(text):
        path = m.group(1).rstrip(".,;:")
        if not exists_in_tree(path, tree):
            found.append(("GONE-PATH", m.start(), m.end(), path))
    for m in FORWARD.finditer(text):
        sid = m.group(1) or m.group(2) or m.group(3)
        if sid in done:
            found.append(("CLOSED-OWNS", m.start(), m.end(), m.group(0)))
    for m in STEP_ID.finditer(text):
        if m.group(0) not in steps:
            found.append(("NO-SUCH-ID", m.start(), m.end(), m.group(0)))
    for m in DEC_ID.finditer(text):
        if m.group(0) not in decisions:
            found.append(("NO-SUCH-ID", m.start(), m.end(), m.group(0)))
    for m in INV_ID.finditer(text):
        if m.group(0) not in invariants:
            found.append(("NO-SUCH-ID", m.start(), m.end(), m.group(0)))
    mark = GOLDEN_MARK.search(text)
    if mark:
        missing = []
        if not GOLDEN_NAME.search(text, mark.start()):
            missing.append("name")
        if not GOLDEN_COMMAND.search(text):
            missing.append("command")
        if not GOLDEN_MARGIN.search(text):
            missing.append("margin")
        if missing:
            found.append(("GOLDEN-FORM", mark.start(), mark.end(), "no " + ", no ".join(missing)))
    for m in NARRATION.finditer(text):
        found.append(("NARRATION", m.start(), m.end(), m.group(0)))
    return flat, found


def run_report(repo, ref, new_only):
    commit = resolve_ref(repo, ref)
    tracked = set(tracked_at(repo, commit))
    tree = tree_files(repo)
    known = known_ids(repo)
    counts = collections.Counter()
    new_counts = collections.Counter()
    skipped = []
    log_total = log_bytes = 0
    rows = []
    live = [p for p in tree if is_live(p)]
    old_texts = blobs_at(repo, commit, [p for p in live if p in tracked])
    for path in live:
        text = read_file(os.path.join(repo, path))
        old_text = old_texts.get(path, "")
        blocks = live_blocks(path, text, skipped)
        old_blocks = live_blocks(path, old_text, skipped) if old_text else []
        old_owner = collections.defaultdict(set)
        for index, old in enumerate(old_blocks):
            for _line, piece in comment_pieces(old):
                old_owner[piece].add(index)
        for block in blocks:
            if is_log_shaped(block):
                state = log_state(block, old_blocks, old_owner)
                log_total += 1
                log_bytes += block.size
                counts["LOG-SHAPED"] += 1
                if state in ("new", "grown"):
                    new_counts["LOG-SHAPED"] += 1
                if not new_only or state in ("new", "grown"):
                    rows.append(
                        f"LOG-SHAPED  {path}:{block.start_line}-{block.end_line} {state} "
                        f"{block.size} B {distinct_ids(block)} ids"
                    )
            flat, found = block_findings(block, known, tree)
            for kind, start, end, what in found:
                is_new = any(p not in old_owner for p in flat.pieces_between(start, end))
                counts[kind] += 1
                if is_new:
                    new_counts[kind] += 1
                # An old narration hit is only counted: the list is lexical, and
                # thousands of rows would bury every other flag.
                if (new_only or kind == "NARRATION") and not is_new:
                    continue
                state = "new" if is_new else "old"
                rows.append(f"{kind:11} {path}:{flat.line_at(start)} {state} {what[:120]}")
    for row in rows:
        print(row)
    for line in skipped:
        print("SKIPPED", line)
    print(f"against {commit[:7]}, working tree")
    for kind in (
        "LOG-SHAPED",
        "ID-CHAIN",
        "FILE-LINE",
        "GONE-PATH",
        "CLOSED-OWNS",
        "GOLDEN-FORM",
        "NO-SUCH-ID",
        "NARRATION",
    ):
        print(f"  {kind:11} {counts[kind]:5} total, {new_counts[kind]:4} new or grown")
    print(f"  log-shaped bytes {log_bytes}")
    flagged = sum(new_counts.values())
    return EXIT_DONE, f"report {flagged} new or grown, {log_total} log-shaped; report only, never a gate"


# --------------------------------------------------------------------------
# --identical


def run_identical(repo, ref, against, exempt, listed_strays=()):
    commit = resolve_ref(repo, ref)
    compiler = Compiler()
    tracked = set(tracked_at(repo, commit))
    proved = not_code = red_files = 0
    exempted = 0

    def prove(path, old, new):
        nonlocal proved, red_files
        failures, arms = identity_failures(path, old, new, compiler)
        if failures:
            red_files += 1
            for arm, detail in failures:
                print(f"DIFF {path} {arm}: {detail}")
        else:
            proved += 1
            print(f"SAME {path} {','.join(arms)}")

    if against:
        pairs, strays = against_pairs(repo, commit, against)
        wrong = sorted(set(listed_strays) - set(strays))
        if wrong:
            raise ToolError(f"--stray names what is no stray under {against}: {', '.join(wrong)}")
        if not pairs:
            raise ToolError(f"nothing under {against} has a counterpart at {commit[:7]}")
        for rel in strays:
            if rel in listed_strays:
                print(f"STRAY {rel}, listed")
            else:
                red_files += 1
                print(f"NO-COUNTERPART {rel}: at {commit[:7]} there is nothing to prove it against")
        for path, full in pairs:
            if language_of(path):
                prove(path, blob_text(repo, commit, path), read_file(full))
            elif is_not_code(path):
                not_code += 1
                print(f"NOT-CODE {path}")
            elif git(repo, "cat-file", "blob", f"{commit}:{path}") != open_bytes(full):
                red_files += 1
                print(f"DIFF {path} no lexer: a changed copy cannot be proved code-identical")
    else:
        for status, path, old_mode, new_mode in changed_in_tree(repo, commit):
            if path in exempt:
                exempted += 1
                print(f"EXEMPT {path}")
                continue
            if is_not_code(path):
                not_code += 1
                print(f"NOT-CODE {path} {status}")
                continue
            if status == "A" and path.startswith("adocs/"):
                not_code += 1
                print(f"NOT-CODE {path} added evidence")
                continue
            if language_of(path) is None:
                red_files += 1
                print(f"DIFF {path} no lexer: {status}, and a change here cannot be proved code-identical")
                continue
            if status == "D":
                red_files += 1
                print(f"DIFF {path} deleted")
                continue
            if status == "A" or path not in tracked:
                red_files += 1
                print(f"DIFF {path} added")
                continue
            if old_mode != new_mode:
                red_files += 1
                print(f"DIFF {path} mode: {old_mode} to {new_mode}")
                continue
            prove(path, blob_text(repo, commit, path), read_file(os.path.join(repo, path)))
    summary = f"{proved} proved, {red_files} red, {not_code} not code, {exempted} exempt, against {commit[:7]}"
    if red_files:
        return EXIT_RED, "identical " + summary
    return EXIT_DONE, "identical " + summary


# --------------------------------------------------------------------------
# --apply


EDIT_KEYS_RANGE = {"file", "start", "end", "old", "text"}
EDIT_KEYS_LINES = {"file", "lines", "old", "text"}


def format_off_lines(text, comments):
    """Lines between a `clang-format off` and its `on`."""
    lines = set()
    off_at = None
    for cm in comments:
        for m in DIRECTIVE.finditer(text[cm.start_off : cm.end_off]):
            word = m.group(1)
            if word.startswith("clang-format") and word.endswith("off"):
                off_at = cm.end_line if off_at is None else off_at
            elif word.startswith("clang-format") and word.endswith("on") and off_at is not None:
                lines.update(range(off_at + 1, cm.start_line))
                off_at = None
    if off_at is not None:
        lines.update(range(off_at + 1, text.count("\n") + 2))
    return lines


def comment_mask(text, comments):
    mask = bytearray(len(text))
    for cm in comments:
        mask[cm.start_off : cm.end_off] = b"\1" * (cm.end_off - cm.start_off)
    return mask


def code_in_range(text, mask, start, end):
    """First non-comment, non-space offset in the range, or None."""
    for off in range(start, end):
        if not mask[off] and not text[off].isspace():
            return off
    return None


def old_refusal(edit, view, start, end):
    """Why the range does not hold the text the writer says it cut, or None."""
    if view[start:end] == edit["old"]:
        return None
    return f"range {start}..{end} does not match its `old`: it holds {view[start:end][:60]!r}"


def resolve_edit(edit, text, view, mask, starts, format_off):
    """(start, end, replacement) or a refusal string. `view` is what the writer read."""
    keys = set(edit)
    replacement = edit.get("text", "")
    if not isinstance(replacement, str):
        return "`text` is not a string"
    if not isinstance(edit.get("old"), str):
        return "no `old`, so nothing says this range is the text the writer cut"
    if "lines" in edit:
        if keys - EDIT_KEYS_LINES:
            return f"unknown keys {sorted(keys - EDIT_KEYS_LINES)}"
        span = edit["lines"]
        if (
            not isinstance(span, list)
            or len(span) != 2
            or not all(isinstance(v, int) for v in span)
            or not 1 <= span[0] <= span[1] <= len(starts)
        ):
            return f"lines {span} are not inside the file"
        first, last = span
        start = starts[first - 1]
        end = starts[last] if last < len(starts) else len(text)
        mismatch = old_refusal(edit, view, start, end)
        if mismatch:
            return mismatch
        for line in range(first, last + 1):
            begin = starts[line - 1]
            finish = starts[line] if line < len(starts) else len(text)
            remainder = "".join(
                text[off] for off in range(begin, finish) if not mask[off]
            ).strip()
            if remainder == "":
                continue
            if remainder == "\\" and line in format_off:
                continue
            where = "outside a format-off macro" if remainder == "\\" else "code"
            return f"line {line} holds {where}: {remainder[:80]!r}"
        if replacement and not replacement.endswith("\n"):
            replacement += "\n"
        return start, end, replacement
    if keys - EDIT_KEYS_RANGE or "start" not in edit or "end" not in edit:
        return f"an edit needs start and end, or lines; has {sorted(keys)}"
    start, end = edit["start"], edit["end"]
    if not isinstance(start, int) or not isinstance(end, int) or not 0 <= start <= end <= len(text):
        return f"range {start}..{end} is not inside the file"
    mismatch = old_refusal(edit, view, start, end)
    if mismatch:
        return mismatch
    off = code_in_range(text, mask, start, end)
    if off is not None:
        line = line_of(starts, off)
        return f"range {start}..{end} touches a code token at line {line}: {text[off:off + 40]!r}"
    return start, end, replacement


def load_edits(path):
    try:
        with open(path, encoding="utf-8") as handle:
            doc = json.load(handle)
    except (OSError, ValueError) as exc:
        raise ToolError(f"cannot read the edit list {path}: {exc}")
    if not isinstance(doc, dict) or set(doc) != {"ref", "edits"} or not isinstance(doc["edits"], list):
        raise ToolError('an edit list is {"ref": <commit>, "edits": [...]}')
    for index, edit in enumerate(doc["edits"]):
        if not isinstance(edit, dict) or not isinstance(edit.get("file"), str):
            raise ToolError(f"edit {index} names no file")
    return doc["ref"], doc["edits"]


def load_seeds(repo, path, commit):
    try:
        with open(path, encoding="utf-8") as handle:
            doc = json.load(handle)
    except (OSError, ValueError) as exc:
        raise ToolError(f"cannot read the seed list {path}: {exc}")
    if not isinstance(doc, dict) or set(doc) != {"ref", "seeds"} or not isinstance(doc["seeds"], list):
        raise ToolError('a seed list is {"ref": <commit>, "seeds": [...]}')
    if resolve_ref(repo, doc["ref"]) != commit:
        raise ToolError(f"the seed list names a different ref from the edit list, {doc['ref']} and {commit[:7]}")
    for index, seed in enumerate(doc["seeds"]):
        if not isinstance(seed, dict) or set(seed) != {"id", "file", "start", "end", "text"}:
            raise ToolError(f"seed {index} needs id, file, start, end and text")
    return doc["seeds"]


def seed_refusal(seed, text, mask):
    """Why a seed cannot stand in a writer's copy, or None."""
    start, end, new = seed["start"], seed["end"], seed["text"]
    if not isinstance(start, int) or not isinstance(end, int) or not isinstance(new, str):
        return "start and end are numbers, text is a string"
    if not 0 <= start <= end <= len(text):
        return f"range {start}..{end} is not inside the file"
    old = text[start:end]
    if len(encode(new)) != len(encode(old)):
        return f"changes byte length, {len(encode(old))} to {len(encode(new))}, so every later offset moves"
    if len(new) != len(old):
        return f"changes character count, {len(old)} to {len(new)}, so every later offset moves"
    if new == old:
        return "changes nothing"
    if "\n" in new or "\n" in old:
        return "holds a newline, so a writer's lines form would read another line"
    off = code_in_range(text, mask, start, end)
    if off is not None:
        return f"touches a code token at offset {off}: {text[off:off + 40]!r}"
    return None


def overlaps(a_start, a_end, b_start, b_end):
    if a_start == a_end:
        return b_start < a_start < b_end
    return a_start < b_end and b_start < a_end


def run_apply(repo, edits_path, to_tree, out_dir, seeds_path=None):
    ref, edits = load_edits(edits_path)
    commit = resolve_ref(repo, ref)
    seeds = load_seeds(repo, seeds_path, commit) if seeds_path else []
    tracked = set(tracked_at(repo, commit))
    by_file = collections.defaultdict(list)
    for index, edit in enumerate(edits):
        by_file[edit["file"]].append((index, edit))
    compiler = Compiler()
    refusals = []
    results = {}

    seeds_by_file = collections.defaultdict(list)
    seen_ids = set()
    for seed in seeds:
        path, sid = seed["file"], seed["id"]
        if sid in seen_ids:
            refusals.append(f"seed {sid}: the list names {sid} twice")
        seen_ids.add(sid)
        if path not in tracked:
            refusals.append(f"seed {sid}: {path} not in {commit[:7]}")
        elif language_of(path) is None:
            refusals.append(f"seed {sid}: {path} has no lexer")
        else:
            old = blob_text(repo, commit, path)
            why = seed_refusal(seed, old, comment_mask(old, editable_comments(path, old)))
            if why:
                refusals.append(f"seed {sid}: {path}: {why}")
            else:
                seeds_by_file[path].append(seed)
    for path, group in seeds_by_file.items():
        ordered = sorted(group, key=lambda s: s["start"])
        for one, two in zip(ordered, ordered[1:]):
            if one["end"] > two["start"]:
                refusals.append(f"seed {one['id']} and seed {two['id']}: {path}: they overlap")

    dropped = []
    touched_seeds = set()
    kept_edits = 0
    for path, group in sorted(by_file.items()):
        if path not in tracked:
            refusals.append(f"{path}: not in {commit[:7]}")
            continue
        if language_of(path) is None:
            refusals.append(f"{path}: no lexer for this file")
            continue
        if path.startswith(NOT_CLEANED):
            refusals.append(f"{path}: `adocs/` and vendored code take no comment edit")
            continue
        old = blob_text(repo, commit, path)
        comments = editable_comments(path, old)
        mask = comment_mask(old, comments)
        starts = line_starts(old)
        format_off = format_off_lines(old, comments)
        view = old
        for seed in seeds_by_file.get(path, []):
            view = view[: seed["start"]] + seed["text"] + view[seed["end"] :]
        ranges = []
        bad = False
        for index, edit in group:
            resolved = resolve_edit(edit, old, view, mask, starts, format_off)
            if isinstance(resolved, str):
                refusals.append(f"{path}: edit {index}: {resolved}")
                bad = True
            else:
                ranges.append((resolved[0], resolved[1], resolved[2], index))
        if bad:
            continue
        ranges.sort(key=lambda r: (r[0], r[1]))
        for one, two in zip(ranges, ranges[1:]):
            if one[1] > two[0] or one[0] == two[0]:
                refusals.append(f"{path}: edit {one[3]} and edit {two[3]} overlap")
                bad = True
        if bad:
            continue
        kept = []
        for edit_range in ranges:
            hits = [
                s
                for s in seeds_by_file.get(path, [])
                if overlaps(edit_range[0], edit_range[1], s["start"], s["end"])
            ]
            if hits:
                dropped.extend((path, edit_range[3], s["id"]) for s in hits)
                touched_seeds.update(s["id"] for s in hits)
                continue
            kept.append(edit_range)
            for seed in seeds:
                if seed["text"] in edit_range[2]:
                    refusals.append(f"{path}: edit {edit_range[3]}: its text holds seed {seed['id']}'s text")
                    bad = True
        if bad or not kept:
            continue
        new = old
        for start, end, replacement, _index in reversed(kept):
            new = new[:start] + replacement + new[end:]
        failures, _arms = identity_failures(path, old, new, compiler)
        if failures:
            for arm, detail in failures:
                refusals.append(f"{path}: the result is red on {arm}: {detail}")
            continue
        if to_tree:
            full = os.path.join(repo, path)
            if not os.path.isfile(full) or read_file(full) != old:
                refusals.append(f"{path}: the working tree differs from {commit[:7]}")
                continue
        results[path] = (old, new, len(kept))
        kept_edits += len(kept)
    if refusals:
        for line in refusals:
            print("REFUSED", line)
        return EXIT_RED, f"apply {len(refusals)} refused, nothing written"
    for path, (old, new, count) in sorted(results.items()):
        target = os.path.join(repo, path) if to_tree else os.path.join(out_dir, path)
        os.makedirs(os.path.dirname(target) or ".", exist_ok=True)
        with open(target, "wb") as handle:
            handle.write(encode(new))
        print(f"APPLIED {path} {count} edits, {len(encode(old))} B to {len(encode(new))} B")
    for path, index, sid in dropped:
        print(f"DROPPED {path} edit {index} seed {sid}")
    untouched = [s["id"] for s in seeds if s["id"] not in touched_seeds]
    for sid in untouched:
        print(f"SEED-UNTOUCHED {sid}")
    note = f"apply {len(results)} files, {kept_edits} edits"
    if seeds:
        note += f", {len({(p, i) for p, i, _s in dropped})} dropped on seeds, {len(untouched)} seeds untouched"
    return EXIT_DONE, note


# --------------------------------------------------------------------------
# --new-facts


FACT = re.compile(
    r"\bS\d{3}\b|\bDEC-\d+\b|\bINV-\d+\b"
    r"|\b(?=[0-9a-f]*[a-f])(?=[0-9a-f]*\d)[0-9a-f]{7,40}\b"
    r"|\b0x[0-9a-fA-F]+\b"
    r"|\b\d+(?:[.,']\d+)*\b"
)


def facts_in(text, comments):
    for cm in comments:
        body = text[cm.start_off : cm.end_off]
        for m in FACT.finditer(body):
            yield m.group(0), cm.start_line + body.count("\n", 0, m.start())


def run_new_facts(repo, ref, against):
    commit = resolve_ref(repo, ref)
    tracked = set(tracked_at(repo, commit))
    if against:
        pairs, _strays = against_pairs(repo, commit, against)
        pairs = [(p, f) for p, f in pairs if language_of(p)]
    else:
        pairs = [
            (path, os.path.join(repo, path))
            for status, path, _old, _new in changed_in_tree(repo, commit)
            if status != "D" and language_of(path) and is_live(path)
        ]
    total = 0
    for path, full in pairs:
        new = read_file(full)
        old = blob_text(repo, commit, path) if path in tracked else ""
        old_facts = {fact for fact, _line in facts_in(old, lex(path, old)[0])} if old else set()
        for fact, line in facts_in(new, lex(path, new)[0]):
            if fact not in old_facts:
                total += 1
                print(f"NEW-FACT {path}:{line} {fact}")
    return EXIT_DONE, f"new-facts {total} over {len(pairs)} files against {commit[:7]}; a reader decides"


# --------------------------------------------------------------------------
# --anchors


def run_anchors(repo):
    sys.path.insert(0, TOOLS)
    import mutation_check

    directory = os.path.join(repo, "tools", "mutants")
    try:
        mutants = mutation_check.load_mutants([directory])
    except mutation_check.Refused as exc:
        raise ToolError(str(exc))
    anchors = on_comments = 0
    bad = []
    spans = {}
    for mut in mutants:
        path = mut["file"]
        full = os.path.join(repo, path)
        if not os.path.isfile(full):
            bad.append(f"ANCHOR {mut['id']}: no such file {path}")
            continue
        text = read_file(full)
        if path not in spans:
            spans[path] = sorted((c.start_off, c.end_off) for c in lex(path, text)[0])
        for index, (old, _new) in enumerate(mut["pairs"]):
            anchors += 1
            count = text.count(old)
            if count != 1:
                bad.append(f"ANCHOR {mut['id']} pair {index}: occurs {count} times in {path}, needs exactly 1")
                continue
            at = text.index(old)
            if any(s < at + len(old) and at < e for s, e in spans[path]):
                on_comments += 1
                print(f"ON-COMMENT {mut['id']} pair {index} {path}:{text.count(chr(10), 0, at) + 1}")
    for line in bad:
        print(line)
    summary = f"anchors {len(mutants)} mutants, {anchors} anchors, {on_comments} overlap comments, {len(bad)} bad"
    return (EXIT_RED if bad else EXIT_DONE), summary


# --------------------------------------------------------------------------


def build_parser():
    parser = argparse.ArgumentParser(
        prog="comment_budget.py",
        description="The comment rule's instruments. tools/comment_budget.py's header has each mode's contract.",
    )
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--report", metavar="REF", help="what breaks the rule, against REF; report only")
    mode.add_argument("--identical", metavar="REF", help="prove the code of each changed file identical to REF")
    mode.add_argument("--apply", metavar="EDITS", help="apply an edit list, refusing any code token")
    mode.add_argument("--new-facts", metavar="REF", help="ids and numbers in new comment text absent from the old")
    mode.add_argument("--anchors", action="store_true", help="find every tools/mutants/ anchor exactly once")
    mode.add_argument("--tiers", metavar="REF", help="each block at REF with its tier")
    parser.add_argument("--repo", default=REPO, help="the checkout to read; default the one holding this tool")
    parser.add_argument("--against", metavar="DIR", help="compare DIR's files instead of the working tree")
    parser.add_argument("--stray", metavar="NAME", action="append", default=[],
                        help="with --identical --against, a copy expected to have no counterpart")
    parser.add_argument("--exempt", metavar="PATH", action="append", default=[],
                        help="with --identical, a path that is not comment-only, named in the output; never under src/")
    parser.add_argument("--new-only", action="store_true", help="with --report, only what is new or grown")
    parser.add_argument("--tree", action="store_true", help="with --apply, write the checkout")
    parser.add_argument("--out", metavar="DIR", help="with --apply, write DIR/<path>")
    parser.add_argument("--seeds", metavar="SEEDS", help="with --apply, drop and list any edit overlapping a seed")
    return parser


def dispatch(args, parser):
    repo = os.path.abspath(args.repo)
    if args.against and not (args.identical or args.new_facts):
        parser.error("--against goes with --identical or --new-facts")
    if args.stray and not (args.identical and args.against):
        parser.error("--stray goes with --identical --against")
    if args.exempt and not args.identical:
        parser.error("--exempt goes with --identical")
    if args.exempt and args.against:
        parser.error("--exempt reads the working tree, --against does not")
    for path in args.exempt:
        if path.startswith("src/"):
            parser.error(f"--exempt {path}: a commit touching src/ proves itself by its own rules")
    if args.new_only and not args.report:
        parser.error("--new-only goes with --report")
    if (args.tree or args.out or args.seeds) and not args.apply:
        parser.error("--tree, --out and --seeds go with --apply")
    if args.apply and bool(args.tree) == bool(args.out):
        parser.error("--apply writes one of --tree or --out DIR")
    if args.report:
        return run_report(repo, args.report, args.new_only)
    if args.identical:
        return run_identical(repo, args.identical, args.against, set(args.exempt), set(args.stray))
    if args.apply:
        return run_apply(repo, args.apply, args.tree, args.out, args.seeds)
    if args.new_facts:
        return run_new_facts(repo, args.new_facts, args.against)
    if args.anchors:
        return run_anchors(repo)
    return run_tiers(repo, args.tiers)


def main():
    # `--tiers | head` is a normal use and the reader going away is no failure.
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)
    parser = build_parser()
    try:
        status, note = dispatch(parser.parse_args(), parser)
    except SystemExit as exc:
        status = EXIT_DONE if exc.code in (0, None) else EXIT_TOOL
        note = "help" if status == EXIT_DONE else "usage"
    except ToolError as exc:
        print(exc, file=sys.stderr)
        status, note = EXIT_TOOL, str(exc).split("\n")[0][:200]
    except Exception as exc:  # the marker owes a watcher its line; the trace is kept
        traceback.print_exc()
        status, note = EXIT_TOOL, f"{type(exc).__name__}: {exc}"[:200]
    print(f"{DONE_MARKER} {note}" if status == EXIT_DONE else f"{FAILED_MARKER} {note}", flush=True)
    return status


if __name__ == "__main__":
    sys.exit(main())
