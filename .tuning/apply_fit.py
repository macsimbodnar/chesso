#!/usr/bin/env python3
"""Apply a tuner-emitted header over src/eval_tables.hpp and src/evaluation.cpp.

Mechanical so that a paste cannot lose an array: every definition the tuner
writes is parsed out of the emitted file, matched against the definition in the
source by name, and the source is rewritten. Anything the tuner emitted and this
script could not place is a hard error, which is the failure DEV_MANUAL warns
about ("a fit that is half applied looks like a fit that did not work").

Usage: apply_fit.py <emitted.hpp> [--dry-run]
"""
import re
import sys

ROOT = "/home/max/ws/chesso/"
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
    arrays = {}
    for name, body in re.findall(
        r"^const int (\w+)\[\w+\] = \{([^}]*)\};", text, re.M
    ):
        arrays[name] = [int(v) for v in re.findall(r"-?\d+", body)]
    scalars = dict(
        (n, int(v)) for n, v in re.findall(r"^const int (\w+) = (-?\d+);", text, re.M)
    )
    return defines, tables, arrays, scalars


def replace_one(text, pattern, new, what):
    out, n = re.subn(pattern, new, text, count=1, flags=re.M | re.S)
    if n != 1:
        raise SystemExit("could not place %s (%d matches)" % (what, n))
    return out


def main():
    emitted = sys.argv[1]
    dry = "--dry-run" in sys.argv
    defines, tables, arrays, scalars = parse(emitted)

    old_defines, old_tables, old_arrays, old_scalars = parse(TABLES)
    _, _, eval_arrays, eval_scalars = parse(EVAL)

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
            r"^(const int %s\[\w+\] = \{)[^}]*(\};)" % name,
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
            r"^const int %s = -?\d+;$" % name,
            "const int %s = %d;" % (name, value),
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
