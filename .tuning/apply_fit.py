#!/usr/bin/env python3
"""Apply a tuner-emitted header over src/eval_tables.hpp and src/evaluation.cpp.

Mechanical so that a paste cannot lose an array: every definition the tuner
writes is parsed out of the emitted file, matched against the definition in the
source by name, and the source is rewritten. Anything the tuner emitted and this
script could not place is a hard error, which is the failure DEV_MANUAL warns
about ("a fit that is half applied looks like a fit that did not work").

Usage: apply_fit.py <emitted.hpp> [--dry-run] [--root TREE | --root=TREE]

--root names the tree whose sources are rewritten, the main checkout by
default. S083 lands its fit in a worktree of its own, as every landing since
S110 does, so the default alone would write the wrong tree.
"""
import re
import sys


def arguments(flags):
    """(emitted, root, flags seen): the one positional, `--root TREE` or
    `--root=TREE`, and the bare flags this script knows. Anything else is
    refused, so a misspelt option cannot fall back to the main checkout."""
    emitted = None
    root = "/home/max/ws/chesso/"
    seen = set()
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        arg = args[i]
        if arg == "--root":
            if i + 1 >= len(args) or not args[i + 1]:
                raise SystemExit("--root needs a directory")
            root = args[i + 1]
            i += 1
        elif arg.startswith("--root="):
            root = arg[len("--root="):]
            if not root:
                raise SystemExit("--root needs a directory")
        elif arg in flags:
            seen.add(arg)
        elif arg.startswith("-") or emitted is not None:
            raise SystemExit("unrecognised argument %s" % arg)
        else:
            emitted = arg
        i += 1
    if emitted is None:
        raise SystemExit("usage: %s <emitted.hpp> [--root TREE]" % sys.argv[0])
    return emitted, root.rstrip("/") + "/", seen


EMITTED, ROOT, FLAGS = arguments({"--dry-run"})
TABLES = ROOT + "src/eval_tables.hpp"
EVAL = ROOT + "src/evaluation.cpp"


def parse(path):
    text = open(path).read()
    defines = dict(re.findall(r"^#define (\w+)\s+(-?\d+)$", text, re.M))
    tables = {}
    for name in ("psqt_mg", "psqt_eg"):
        m = re.search(
            r"static constexpr int %s\[6\]\[64\] = \{\n(.*?)\n\};\n" % name,
            text,
            re.S,
        )
        if m is not None:
            tables[name] = m.group(1)
    # Either keyword: the weights have been constexpr since S117 and the tuner
    # emits them so, and a header written before that says const.
    arrays = {}
    for name, body in re.findall(
        r"^(?:constexpr|const) int (\w+)\[\w+\] = \{([^}]*)\};", text, re.M
    ):
        arrays[name] = [int(v) for v in re.findall(r"-?\d+", body)]
    scalars = dict(
        (n, int(v))
        for n, v in re.findall(
            r"^(?:constexpr|const) int (\w+) = (-?\d+);", text, re.M
        )
    )
    return defines, tables, arrays, scalars


def declared(path):
    # Every int definition at the start of a line, whatever its qualifiers or
    # shape. A name in here that parse() did not return is a definition this
    # script would silently skip -- what a keyword change did, S117.
    text = open(path).read()
    return set(re.findall(r"^(?:\w+ )*int (\w+) ?(?:\[|=)", text, re.M))


def replace_one(text, pattern, new, what):
    out, n = re.subn(pattern, new, text, count=1, flags=re.M | re.S)
    if n != 1:
        raise SystemExit("could not place %s (%d matches)" % (what, n))
    return out


def main():
    emitted = EMITTED
    dry = "--dry-run" in FLAGS
    defines, tables, arrays, scalars = parse(emitted)

    unparsed = declared(emitted) - set(tables) - set(arrays) - set(scalars)
    if unparsed:
        raise SystemExit(
            "emitted definitions this script cannot parse: %s"
            % ", ".join(sorted(unparsed))
        )
    if not (defines or tables or arrays or scalars):
        raise SystemExit("nothing to place in %s" % emitted)

    old_defines, old_tables, old_arrays, old_scalars = parse(TABLES)
    _, _, eval_arrays, eval_scalars = parse(EVAL)

    # Checked before either file is written, so a name the source no longer
    # spells the way this script reads it stops the run with nothing applied.
    missing = [n for n in arrays if n not in eval_arrays] + [
        n for n in scalars if n not in eval_scalars
    ]
    if missing:
        raise SystemExit(
            "emitted %s not found in %s" % (", ".join(missing), EVAL)
        )

    applied = []

    text = open(TABLES).read()
    for name, value in defines.items():
        text = replace_one(
            text,
            r"^#define %s\s+-?\d+$" % name,
            "#define %-6s %s" % (name, value),
            "#define " + name,
        )
        applied.append("%s = %s (was %s)" % (name, value, old_defines[name]))
    for name, body in tables.items():
        text = replace_one(
            text,
            r"(static constexpr int %s\[6\]\[64\] = \{\n).*?(\n\};\n)" % name,
            lambda m, body=body: m.group(1) + body + m.group(2),
            name,
        )
        applied.append(name)
    if not dry:
        open(TABLES, "w").write(text)

    text = open(EVAL).read()
    for name, values in arrays.items():
        if name not in eval_arrays:
            raise SystemExit("emitted array %s is in neither source file" % name)
        text = replace_one(
            text,
            r"^((?:constexpr|const) int %s\[\w+\] = \{)[^}]*(\};)" % name,
            lambda m, values=values: m.group(1)
            + ", ".join(str(v) for v in values)
            + m.group(2),
            name,
        )
        applied.append(
            "%s = %s (was %s)" % (name, values, eval_arrays[name])
        )
    for name, value in scalars.items():
        if name not in eval_scalars:
            raise SystemExit("emitted scalar %s is in neither source file" % name)
        text = replace_one(
            text,
            r"^((?:constexpr|const) int %s = )-?\d+;$" % name,
            lambda m, value=value: m.group(1) + "%d;" % value,
            name,
        )
        applied.append("%s = %d (was %d)" % (name, value, eval_scalars[name]))
    if not dry:
        open(EVAL, "w").write(text)

    print("\n".join(applied))
    print(
        "\n%d defines, %d tables, %d arrays, %d scalars"
        % (len(defines), len(tables), len(arrays), len(scalars))
    )


main()
