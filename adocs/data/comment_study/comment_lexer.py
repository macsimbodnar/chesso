#!/usr/bin/env python3
"""Comment lexers for the comment study. Stdlib only.

Each lex_* function takes the file's text and returns a list of Comment records,
in file order, plus the set of line numbers (1-based) that carry code. A line
holding only whitespace and comment is a comment line; a line with both code and
a comment counts as code and its comment is "trailing".

strip_comments(path, text) returns the text with every comment removed, used to
compare a compressed copy against the original.
"""
import ast
import io
import re
import tokenize
from dataclasses import dataclass


@dataclass
class Comment:
    start_line: int
    end_line: int
    start_off: int  # offset of the first comment char in the text
    end_off: int  # one past the last comment char
    kind: str  # line, block, docstring, shebang
    trailing: bool = False


def line_starts(text):
    starts = [0]
    i = text.find("\n")
    while i != -1:
        starts.append(i + 1)
        i = text.find("\n", i + 1)
    return starts


def _line_of(starts, off):
    # bisect by hand to keep it readable in one place
    lo, hi = 0, len(starts) - 1
    while lo < hi:
        mid = (lo + hi + 1) // 2
        if starts[mid] <= off:
            lo = mid
        else:
            hi = mid - 1
    return lo + 1


RAW_PREFIX = re.compile(r'(?:u8|u|U|L)?R"$')


def lex_cpp(text):
    """C and C++ including .S: comments, strings, char literals, raw strings,
    digit separators. Returns (comments, code_lines, if0_regions)."""
    comments = []
    code_lines = set()
    starts = line_starts(text)
    n = len(text)
    i = 0
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = i + 2
            while j < n:
                if text[j] == "\n":
                    # backslash-newline continues a line comment
                    if text[j - 1] == "\\":
                        j += 1
                        continue
                    break
                j += 1
            comments.append(
                Comment(_line_of(starts, i), _line_of(starts, max(i, j - 1)), i, j, "line")
            )
            i = j
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j == -1 else j + 2
            comments.append(
                Comment(_line_of(starts, i), _line_of(starts, j - 1), i, j, "block")
            )
            i = j
            continue
        if c == '"':
            # raw string: the opening quote is preceded by an R with a valid prefix
            k = i - 1
            is_raw = False
            if k >= 0 and text[k] == "R":
                head = text[max(0, k - 2) : i + 1]
                prev_ok = True
                m = RAW_PREFIX.search(head)
                if m:
                    before = max(0, k - 2) + m.start() - 1
                    if before >= 0 and (text[before].isalnum() or text[before] == "_"):
                        prev_ok = False
                    is_raw = prev_ok
            if is_raw:
                p = text.find("(", i + 1)
                delim = text[i + 1 : p]
                end = text.find(")" + delim + '"', p)
                end = n if end == -1 else end + len(delim) + 2
                for ln in range(_line_of(starts, i), _line_of(starts, end - 1) + 1):
                    code_lines.add(ln)
                i = end
                continue
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                elif text[j] == "\n":
                    break  # unterminated: a stray quote in preprocessor text or asm
                j += 1
            for ln in range(_line_of(starts, i), _line_of(starts, min(j, n - 1)) + 1):
                code_lines.add(ln)
            i = j + 1
            continue
        if c == "'":
            # digit separator: 1'000'000
            if i > 0 and text[i - 1].isalnum() and i + 1 < n and text[i + 1].isalnum():
                code_lines.add(_line_of(starts, i))
                i += 1
                continue
            j = i + 1
            while j < n and text[j] != "'":
                if text[j] == "\\":
                    j += 1
                elif text[j] == "\n":
                    break
                j += 1
            code_lines.add(_line_of(starts, i))
            i = j + 1
            continue
        if not c.isspace():
            code_lines.add(_line_of(starts, i))
        i += 1
    for cm in comments:
        cm.trailing = cm.start_line in code_lines
    return comments, code_lines


IF0 = re.compile(r"^\s*#\s*if\s+0\b", re.M)


def if0_regions(text):
    """(start_line, end_line) of each `#if 0` block, nested conditionals tracked."""
    out = []
    starts = line_starts(text)
    for m in IF0.finditer(text):
        depth = 1
        ln = _line_of(starts, m.start())
        cur = ln
        for raw in text[m.end() :].split("\n")[1:]:
            cur += 1
            s = raw.strip()
            if re.match(r"#\s*if", s):
                depth += 1
            elif re.match(r"#\s*endif", s):
                depth -= 1
                if depth == 0:
                    break
        out.append((ln, cur))
    return out


def lex_python(text):
    """tokenize for comments, ast for docstrings. Raises on unparsable source."""
    comments = []
    starts = line_starts(text)
    code_lines = set()
    doc_spans = []
    tree = ast.parse(text)
    for node in ast.walk(tree):
        if isinstance(node, (ast.Module, ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)):
            body = getattr(node, "body", None)
            if (
                body
                and isinstance(body[0], ast.Expr)
                and isinstance(body[0].value, ast.Constant)
                and isinstance(body[0].value.value, str)
            ):
                d = body[0].value
                doc_spans.append((d.lineno, d.end_lineno, d.col_offset, d.end_col_offset))
    doc_lines = set()
    for s, e, _, _ in doc_spans:
        doc_lines.update(range(s, e + 1))
    utf8_lines = text.split("\n")

    def off(line, col):
        # tokenize columns count characters, ast columns count utf-8 bytes
        return starts[line - 1] + col

    for tok in tokenize.generate_tokens(io.StringIO(text).readline):
        if tok.type == tokenize.COMMENT:
            kind = "shebang" if tok.start == (1, 0) and tok.string.startswith("#!") else "line"
            comments.append(
                Comment(
                    tok.start[0], tok.end[0], off(*tok.start), off(*tok.end), kind
                )
            )
        elif tok.type in (
            tokenize.NL,
            tokenize.NEWLINE,
            tokenize.INDENT,
            tokenize.DEDENT,
            tokenize.ENDMARKER,
        ):
            continue
        else:
            # a docstring token is not code; a string in an ordinary statement is
            if tok.type == tokenize.STRING and any(
                tok.start[0] == s and tok.end[0] == e for s, e, _, _ in doc_spans
            ):
                continue
            for ln in range(tok.start[0], tok.end[0] + 1):
                code_lines.add(ln)
    for s, e, sc, ec in doc_spans:
        # byte columns from ast mapped back to char columns on the line
        sline = utf8_lines[s - 1].encode("utf-8")[:sc].decode("utf-8", "replace")
        eline = utf8_lines[e - 1].encode("utf-8")[:ec].decode("utf-8", "replace")
        comments.append(Comment(s, e, off(s, len(sline)), off(e, len(eline)), "docstring"))
    comments.sort(key=lambda c: c.start_off)
    for cm in comments:
        cm.trailing = cm.start_line in code_lines
    return comments, code_lines


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
                    code_lines.add(_line_of(starts, i))
                    i = min(j + 1, n)
                    if (line.lstrip("\t") if strip_tabs else line) == delim:
                        break
            continue
        if c == "#" and word_start:
            j = text.find("\n", i)
            j = n if j == -1 else j
            kind = "shebang" if i == 0 and text.startswith("#!") else "line"
            comments.append(Comment(_line_of(starts, i), _line_of(starts, i), i, j, kind))
            i = j
            continue
        if c == "\\":
            code_lines.add(_line_of(starts, i))
            i += 2
            word_start = False
            continue
        if c == "'":
            j = text.find("'", i + 1)
            j = n - 1 if j == -1 else j
            for ln in range(_line_of(starts, i), _line_of(starts, j) + 1):
                code_lines.add(ln)
            i = j + 1
            word_start = False
            continue
        if c == "$" and i + 1 < n and text[i + 1] == "'":
            j = i + 2
            while j < n and text[j] != "'":
                if text[j] == "\\":
                    j += 1
                j += 1
            for ln in range(_line_of(starts, i), _line_of(starts, min(j, n - 1)) + 1):
                code_lines.add(ln)
            i = j + 1
            word_start = False
            continue
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                j += 1
            for ln in range(_line_of(starts, i), _line_of(starts, min(j, n - 1)) + 1):
                code_lines.add(ln)
            i = j + 1
            word_start = False
            continue
        if c == "<" and text.startswith("<<", i) and not text.startswith("<<<", i):
            m = re.compile(r"<<(-?)\s*(['\"]?)([A-Za-z_][A-Za-z0-9_]*)\2").match(text, i)
            if m:
                pending_heredocs.append((m.group(3), m.group(1) == "-"))
                code_lines.add(_line_of(starts, i))
                i = m.end()
                word_start = False
                continue
        if c in " \t;&|()":
            word_start = True
            if not c.isspace():
                code_lines.add(_line_of(starts, i))
            i += 1
            continue
        code_lines.add(_line_of(starts, i))
        word_start = False
        i += 1
    for cm in comments:
        cm.trailing = cm.start_line in code_lines
    return comments, code_lines


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
            m = re.compile(r"#\[(=*)\[").match(text, i)
            if m:
                end = text.find("]" + m.group(1) + "]", m.end())
                end = n if end == -1 else end + len(m.group(1)) + 2
                comments.append(
                    Comment(_line_of(starts, i), _line_of(starts, end - 1), i, end, "block")
                )
                i = end
                continue
            j = text.find("\n", i)
            j = n if j == -1 else j
            comments.append(Comment(_line_of(starts, i), _line_of(starts, i), i, j, "line"))
            i = j
            continue
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                j += 1
            for ln in range(_line_of(starts, i), _line_of(starts, min(j, n - 1)) + 1):
                code_lines.add(ln)
            i = j + 1
            continue
        m = re.compile(r"\[(=*)\[").match(text, i)
        if m:
            end = text.find("]" + m.group(1) + "]", m.end())
            end = n if end == -1 else end + len(m.group(1)) + 2
            for ln in range(_line_of(starts, i), _line_of(starts, end - 1) + 1):
                code_lines.add(ln)
            i = end
            continue
        if not c.isspace():
            code_lines.add(_line_of(starts, i))
        i += 1
    for cm in comments:
        cm.trailing = cm.start_line in code_lines
    return comments, code_lines


def lex_awk(text):
    """awk: `#` outside strings. Regex literals with a `#` are not handled; the
    tree holds one awk file and it is checked by hand."""
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
            comments.append(Comment(_line_of(starts, i), _line_of(starts, i), i, j, "line"))
            i = j
            continue
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                if text[j] == "\\":
                    j += 1
                j += 1
            code_lines.add(_line_of(starts, i))
            i = j + 1
            continue
        if not c.isspace():
            code_lines.add(_line_of(starts, i))
        i += 1
    for cm in comments:
        cm.trailing = cm.start_line in code_lines
    return comments, code_lines


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


def strip_comments(path, text):
    """The text with comment characters removed and whole comment lines dropped."""
    comments, _ = lex(path, text)
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
            only_space = nl != -1 and text[e:nl].strip() == ""
            if only_space:
                e = nl + 1
        else:
            while s > 0 and text[s - 1] in " \t":
                s -= 1
        out.append(text[pos:s])
        pos = e
    out.append(text[pos:])
    return "".join(out)
