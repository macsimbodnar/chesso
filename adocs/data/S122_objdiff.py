#!/usr/bin/env python3
"""S122's object-code half of INV-6: two builds of src/evaluation.cpp.o, the
parent's and the landing's, compared function by function once the padding is
taken out. adocs/data/S101_objdiff.py's form, its normalisation S122's.
Standard library and binutils' objdump only.

    S122_objdiff.py PARENT.o LANDING.o

Normalised away, and nothing else: the `cs`/`ds`/`ss`/`es` prefixes and the
alignment nops the padding adds, each instruction's own address, the addresses
and offsets of branch and call targets, and runs of whitespace. And one name:
S122 added `king_danger_t*` to evaluate_mobility_and_king_safety<collect>(), which
every symbol naming it spells; it is dropped from the landing's names so the two
instantiations are compared as one function.

Exit 0 when every function the search reaches is identical: every function but
the collecting instantiations -- evaluate_mobility_and_king_safety<true>,
threat_terms<true> -- and the accessors that call them, king_danger_counts()
among them, which the search never calls (src/evaluation.cpp says so over each).
"""

import re
import subprocess
import sys

COLLECTING = re.compile(r"^(int evaluate_mobility_and_king_safety<true>|"
                        r"void threat_terms<true>|king_safety_features|"
                        r"evaluate_expensive_terms|threat_family_counts|"
                        r"king_danger_counts)\(")


def strip_added(text):
    return text.replace(", king_danger_t*", "")


def functions(obj):
    text = subprocess.run(["objdump", "-d", "--no-show-raw-insn", "-C", obj],
                          capture_output=True, text=True, check=True).stdout
    name = None
    out = {}
    for line in text.splitlines():
        m = re.match(r"^[0-9a-f]+ <(.*)>:$", line)
        if m:
            name = strip_added(m.group(1))
            out[name] = []
            continue
        m = re.match(r"^\s+[0-9a-f]+:\s+(.*)$", line)
        if not m or name is None:
            continue
        ins = re.sub(r"\s+#.*$", "", m.group(1).strip())
        ins = re.sub(r"^((cs|ds|ss|es) )+", "", ins)
        if re.match(r"^(nop|xchg\s+%ax,%ax|data16)", ins):
            continue
        ins = strip_added(ins)
        ins = re.sub(r"\b[0-9a-f]+ <", "<", ins)
        ins = re.sub(r"\+0x[0-9a-f]+>", ">", ins)
        ins = re.sub(r"\s+", " ", ins)
        out[name].append(ins)
    return out


def main(argv):
    if len(argv) != 2:
        print(__doc__.strip().splitlines()[5].strip())
        return 2
    parent, landing = functions(argv[0]), functions(argv[1])
    bad = []
    for name in sorted(set(parent) | set(landing)):
        a, b = parent.get(name), landing.get(name)
        if a is None:
            verdict = "new"
        elif b is None:
            verdict = "gone"
        else:
            verdict = "identical" if a == b else "differs"
        print("  %-10s %5s %5s  %s" % (verdict, "-" if a is None else len(a),
                                        "-" if b is None else len(b), name))
        if verdict != "identical" and not COLLECTING.match(name):
            bad.append(name)
    if bad:
        print("NOT INV-6: a function the search reaches differs: " +
              "; ".join(bad))
        return 1
    print("every function the search reaches is identical modulo padding")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
