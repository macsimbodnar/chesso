#!/usr/bin/env python3
"""Check that every one of the 955 fitted numbers reached the engine's sources.

Re-extracts the weights from src/eval_tables.hpp and src/evaluation.cpp after the
paste and compares them, one by one, against the emitted header. A half-applied
fit looks like a fit that did not work, and a weight still holding the value the
engine shipped is indistinguishable from a term the fit had nothing to say about.

Usage: verify_fit.py <emitted.hpp> [--root TREE | --root=TREE]

823 since S134 folded the sixth passer bucket and rook on the seventh into the
tables (DEC-090): passed_pawn is five wide a phase and piece_placement three.
This list said six and four, and 827, until S083 -- the first fit pasted since
S076 -- found it; it would have refused a correct paste. 831 since S135, whose
four placement columns make piece_placement seven wide: a fit emitted by a
tuner built before S135 is three wide there and is refused here, which is
right, because it cannot place the four. --root names the tree read, the main
checkout by default.

955 since S121, whose mobility is eight one-dimensional tables, one per piece
type and half, 9, 14, 15 and 28 wide (src/evaluation.hpp, MOBILITY_WIDTH),
where it was mobility_mg[4] and mobility_eg[4]: a fit emitted by a tuner built
before S121 names arrays the engine no longer has and is refused, which is
right. .tuning/verify_fit.py is tracked -- .gitignore ignores `.tuning/*` and
then keeps `!.tuning/*.py` -- although S121's phase 1 wrote that it was
gitignored and landed this layout as adocs/data/S121_verify_fit.py alone; the
two are one file, copied over .tuning/verify_fit.py by S121's phase 2 commit.
.tuning/apply_fit.py needs no change, because its array pattern already takes
any one-dimensional `constexpr int name[N] = {...};`.
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


EMITTED, ROOT, _ = arguments(set())
TOTAL = 955

WIDTHS = [
    ("PAWN", 1),
    ("KNIGHT", 1),
    ("BISHOP", 1),
    ("ROOK", 1),
    ("QUEEN", 1),
    ("psqt_mg", 384),
    ("psqt_eg", 384),
    ("mobility_knight_mg", 9),
    ("mobility_knight_eg", 9),
    ("mobility_bishop_mg", 14),
    ("mobility_bishop_eg", 14),
    ("mobility_rook_mg", 15),
    ("mobility_rook_eg", 15),
    ("mobility_queen_mg", 28),
    ("mobility_queen_eg", 28),
    ("king_safety_mg", 9),
    ("king_safety_eg", 9),
    ("passed_pawn_mg", 5),
    ("passed_pawn_eg", 5),
    ("pawn_structure_mg", 3),
    ("pawn_structure_eg", 3),
    ("piece_placement_mg", 7),
    ("piece_placement_eg", 7),
    ("tempo_mg", 1),
    ("tempo_eg", 1),
]


def extract(text):
    def strip(s):
        return re.sub(r"//[^\n]*", "", s)

    out = {}
    for name, value in re.findall(r"^#define (\w+)\s+(-?\d+)$", text, re.M):
        out[name] = [int(value)]
    for name in ("psqt_mg", "psqt_eg"):
        m = re.search(
            r"static constexpr int %s\[6\]\[64\] = \{(.*?)\n\};" % name, text, re.S
        )
        if m:
            out[name] = [int(v) for v in re.findall(r"-?\d+", strip(m.group(1)))]
    for name, body in re.findall(r"^(?:constexpr|const) int (\w+)\[\w+\] = \{([^}]*)\};", text, re.M):
        out[name] = [int(v) for v in re.findall(r"-?\d+", strip(body))]
    for name, value in re.findall(r"^(?:constexpr|const) int (\w+) = (-?\d+);", text, re.M):
        out[name] = [int(value)]
    return out


emitted = extract(open(EMITTED).read())
shipped = extract(
    open(ROOT + "src/eval_tables.hpp").read() + open(ROOT + "src/evaluation.cpp").read()
)

total = 0
same = 0
bad = []
for name, width in WIDTHS:
    if name not in emitted:
        bad.append("%s: missing from the emitted header" % name)
        continue
    if name not in shipped:
        bad.append("%s: missing from the engine sources" % name)
        continue
    a, b = emitted[name], shipped[name]
    if len(a) != width or len(b) != width:
        bad.append("%s: width %d emitted, %d shipped, %d expected" % (name, len(a), len(b), width))
        continue
    total += width
    same += sum(1 for x, y in zip(a, b) if x == y)
    if a != b:
        bad.append(
            "%s: %d of %d values differ from the emitted header"
            % (name, sum(1 for x, y in zip(a, b) if x != y), width)
        )

print("%d parameters compared, %d identical to the emitted header" % (total, same))
for line in bad:
    print("  MISMATCH " + line)
sys.exit(1 if (bad or total != TOTAL) else 0)
