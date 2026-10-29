#!/usr/bin/env python3
"""S121's screen: what each part fits as and against, the identity check and
the ties from the audit, the reading of the fits, the bisection, and the paste
and clamp checks. adocs/data/S121_screen.sh runs every one of them; each also
runs alone. Standard library only, plus adocs/data/S135_screen.py, imported so
the fit logs and headers are parsed exactly the way S135's and S136's screens
parsed them.

    S121_screen.py plan PART --out OUT
    S121_screen.py audit-check REPORT
    S121_screen.py ties REPORT --out FILE
    S121_screen.py linear --out FILE
    S121_screen.py read PART --out OUT
    S121_screen.py start-check HEADER --tree TREE [--free]
    S121_screen.py headroom HEADER
    S121_screen.py only-mobility TREE
    S121_screen.py clamp AUDIT SPREAD
    S121_screen.py part NAME --landed MASK --curve table|linear

PART is P0, P1, P2a, P2b or P3; the header of S121_screen.sh is the rule's
statement and this file its implementation. PLAN prints what a part is fitted as -- its
form (the table, or the linear model when P0 was dropped), the parts mask its
variant is built with, the fits it compares against, the tie file its fits
read -- from the readings of the parts before it under OUT/read, and refuses
when one of those is missing or unreadable. AUDIT-CHECK applies the identity
rule to a tools/feature_audit report. TIES writes a variant's tie list from
the same report's support lines under the pre-registered threshold, LINEAR the
tie list that makes every row c times its count-1 cell. READ applies the
part's rule and writes OUT/read/PART.txt, whose last line is PART-OUTCOME kept,
dropped or unreadable; P3's reading also names the candidate and the
bisection order. P2b's variant always carries P2a's area: kept P2a makes it
P2b's predecessor, and a dropped P2a makes P2b the area with both own-pawn
exclusions against the last survivor, so low-rank pawns are never measured
over ~own, where excluding them is a no-op. START-CHECK compares a fit's header with TREE's HEAD sources,
the mobility tables excepted under --free. HEADROOM is the new overflow
assert's arithmetic for a header's tables. ONLY-MOBILITY refuses a paste that
moved any weight but the mobility tables. CLAMP reads the candidate's clamp
share and its mobility spread and prints a STOP line past the bound. PART
prints the fit a bisection part plays as.
"""

import argparse
import os
import re
import statistics
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S135_screen  # noqa: E402  (adocs/data/, by path)

from S135_screen import Refused, head_weights, read_fit, weights_of  # noqa: E402

PARTS = ["P0", "P1", "P2a", "P2b", "P3"]
# The parts mask bit each part adds to the variant (src/evaluation.hpp in
# adocs/data/S121_parts.patch); P0 is the table itself and adds none. P2b
# carries P2a's bit too (REQUIRES), so it is never read over ~own.
BIT = {"P0": 0, "P1": 1, "P2a": 2, "P2b": 8, "P3": 4}
REQUIRES = {"P2b": 2}
LATER = ["P1", "P2a", "P2b", "P3"]
LETTER = {"P0": "A", "P1": "B1", "P2a": "B2a", "P2b": "B2b", "P3": "C"}
NAMES = {"P0": "the curve, a table over attacks & ~own",
         "P1": "squares attacked by enemy pawns excluded",
         "P2a": "the area replaces ~own: own pieces count, own blocked pawns "
                "do not (the switch is inseparable from this exclusion)",
         "P2b": "own low-rank pawns excluded as well, always inside P2a's "
                "area",
         "P3": "a pinned piece counts only its pin line"}
PIECES = ["knight", "bishop", "rook", "queen"]
WIDTH = [9, 14, 15, 28]
CELLS = sum(WIDTH)
HALF_LIMIT = 32767
SEEDS = S135_screen.SEEDS
PICK_Z = S135_screen.PICK_Z
FLOOR = S135_screen.FLOOR
SPREAD_UNREADABLE = S135_screen.SPREAD_UNREADABLE

# Pre-registered 2026-10-28, before any fit (the shell script's header).
TIE_GAMES = 1000           # a half of a cell is supported from this many games
CLAMP_SHARE_BOUND = 1.0    # percent of the corpus's rows; past it, STOP
EXPECTED_RANK = 908        # of 955, before the mobility columns no row reaches
PARAMS = 955
DEAD_PAWN_COLUMNS = 32

FIT_EPOCHS = 5000
SAMPLES = ["full"] + [f"q{s}" for s in SEEDS]

SUPPORT = re.compile(r"^support mobility_(mg|eg)\[(\w+)\]\[(\d+)\] rows (\d+) "
                     r"games (\d+)$")
RANK = re.compile(r"^identities: rank over the rationals exactly (\d+) of "
                  r"(\d+), nullity (\d+)")
ZERO = re.compile(r"^identities: (\d+) columns zero on every row:(.*)$")
SPAN_OK = ("every null direction is one Adam never moves, one the tuner "
           "holds, or a mobility level the run's frozen columns hold")
LEVEL_HELD = re.compile(r"^identities: mobility level, .*: held by the freeze "
                        r"under --free mobility ")
NO_MORE = "identities: no exact dependency beyond the ones listed above"
BY_CONSTRUCTION = re.compile(r"^identities: by construction, (.*): (holds on "
                             r"every row|FAILS)$")
EPOCH = re.compile(r"^epoch\s+(\d+)\s.*\*$")
PATIENCE = "no improvement in"
MOBILITY_ARRAY = re.compile(r"^mobility_(knight|bishop|rook|queen)_(mg|eg)$")
CLAMP = re.compile(r"the clamp binds on (\d+)")
TRACE_ROWS = re.compile(r"^trace: (\d+) rows")
SPREAD_ROW = re.compile(r"^\s+(p99|max)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)$")


def outcome_of(out, part):
    """PART-OUTCOME of an earlier part's reading, or a refusal."""
    path = os.path.join(out, "read", f"{part}.txt")
    if not os.path.exists(path):
        raise Refused(f"{path} is missing: {part} has not been read")
    text = open(path).read()
    m = re.search(r"^PART-OUTCOME (kept|dropped|unreadable)$", text, re.M)
    if not m:
        raise Refused(f"{path} prints no PART-OUTCOME")
    if m.group(1) == "unreadable":
        raise Refused(f"{part} read unreadable; the coordinator's before "
                      "any later part")
    return m.group(1)


def fit_name(form, mask):
    return f"{form}_m{mask}"


def plan_of(part, out):
    """{form, mask, fit, pred, ties} for PART, from the readings before it."""
    k = PARTS.index(part)
    if part == "P0":
        return {"form": "table", "mask": 0, "fit": fit_name("table", 0),
                "pred": None, "ties": "ties_m0.txt"}
    curve = outcome_of(out, "P0") == "kept"
    form = "table" if curve else "linear"
    landed = 0
    for earlier in PARTS[1:k]:
        if outcome_of(out, earlier) == "kept":
            landed |= BIT[earlier] | REQUIRES.get(earlier, 0)
    # P2b with P2a dropped is the area with both exclusions against the last
    # survivor (DEC-326 (1)): its own bit and the area's.
    mask = landed | BIT[part] | REQUIRES.get(part, 0)
    return {"form": form, "mask": mask, "fit": fit_name(form, mask),
            "pred": fit_name(form, landed), "pred_mask": landed,
            "ties": f"ties_m{mask}.txt" if curve else "linear.txt",
            "pred_ties": f"ties_m{landed}.txt" if curve else "linear.txt"}


def plan(args):
    p = plan_of(args.part, args.out)
    print(f"part {args.part}: {NAMES[args.part]}")
    print(f"form {p['form']}")
    print(f"mask {p['mask']}")
    print(f"fit {p['fit']}")
    print(f"ties {p['ties']}")
    if p["pred"] is not None:
        print(f"pred {p['pred']}")
        print(f"pred_mask {p['pred_mask']}")
        print(f"pred_ties {p['pred_ties']}")
    return 0


def cell_name(half, piece, cell):
    return f"mobility_{piece}_{half}[{cell}]"


def support_of(report):
    """{(half, piece): [(rows, games)] * width} from the support lines."""
    out = {}
    for line in open(report, encoding="utf-8", errors="replace"):
        m = SUPPORT.match(line.rstrip("\n"))
        if m:
            half, piece, cell = m.group(1), m.group(2), int(m.group(3))
            out.setdefault((half, piece), {})[cell] = (int(m.group(4)),
                                                       int(m.group(5)))
    for half in ("mg", "eg"):
        for t, piece in enumerate(PIECES):
            row = out.get((half, piece), {})
            if sorted(row) != list(range(WIDTH[t])):
                raise Refused(f"{report}: the support lines for {piece} {half} "
                              f"are not cells 0 to {WIDTH[t] - 1}")
    return {key: [row[c] for c in sorted(row)] for key, row in out.items()}


def audit_check(args):
    text = open(args.report, encoding="utf-8", errors="replace").read()
    lines = text.splitlines()
    problems = []
    zero = None
    for line in lines:
        m = ZERO.match(line)
        if m:
            zero = m.group(2).split()
    if zero is None:
        raise Refused(f"{args.report} prints no zero-column line")
    mobility_zero = [z for z in zero if z.startswith("mobility_")]
    pawn_zero = [z for z in zero if re.fullmatch(r"psqt_(mg|eg)\[pawn\]\[[a-h][18]\]", z)]
    other = [z for z in zero if z not in mobility_zero and z not in pawn_zero]
    if len(pawn_zero) != DEAD_PAWN_COLUMNS:
        problems.append(f"{len(pawn_zero)} dead pawn columns, not "
                        f"{DEAD_PAWN_COLUMNS}")
    if other:
        problems.append("zero columns outside the mobility table and the dead "
                        "pawn squares: " + " ".join(other))
    want = EXPECTED_RANK - len(mobility_zero)
    rank = [RANK.match(line) for line in lines]
    rank = [m for m in rank if m]
    if not rank:
        problems.append("the rank over the rationals is not exact")
    else:
        got, of, _ = (int(x) for x in rank[0].groups())
        if of != PARAMS or got != want:
            problems.append(f"rank {got} of {of}, the rule wants {want} of "
                            f"{PARAMS} ({EXPECTED_RANK} less "
                            f"{len(mobility_zero)} mobility zero columns)")
    constructed = [BY_CONSTRUCTION.match(line) for line in lines]
    constructed = [m for m in constructed if m]
    levels = [m for m in constructed if "(S121)" in m.group(1)]
    if len(constructed) != 15 or len(levels) != 8:
        problems.append(f"{len(constructed)} dependencies by construction, "
                        f"{len(levels)} of them S121's: the rule wants 15 "
                        "and 8")
    failed = [m.group(1) for m in constructed if m.group(2) == "FAILS"]
    if failed:
        problems.append("a dependency by construction fails: " +
                        "; ".join(failed))
    if NO_MORE not in lines:
        problems.append("an exact dependency beyond the listed ones")
    if SPAN_OK not in text:
        problems.append("a null direction no gauge, zero column or frozen "
                        "mobility level accounts for")
    held = [line for line in lines if LEVEL_HELD.match(line)]
    if len(held) != 8:
        problems.append(f"{len(held)} mobility levels held by the freeze "
                        "under --free mobility, not 8")
    print(f"audit-check {args.report}: {len(mobility_zero)} mobility columns "
          f"zero on every row" + (": " + " ".join(mobility_zero)
                                  if mobility_zero else ""))
    print(f"audit-check: the rule's rank {want} of {PARAMS}, the 8 mobility "
          "levels and the 7 S134 directions by construction, nothing else")
    if problems:
        raise Refused("the identity rule fails: " + "; ".join(problems))
    print("audit-check: pass")
    return 0


def ties(args):
    support = support_of(args.report)
    lines = [f"# S121 ties from {args.report}",
             f"# a half of a cell is supported from {TIE_GAMES} games; an "
             "unsupported one shares the nearest supported cell of its piece "
             "and half, the lower count on a tie (DEC-326 (5))"]
    tied = 0
    print(f"ties: threshold {TIE_GAMES} games a half")
    for half in ("mg", "eg"):
        for t, piece in enumerate(PIECES):
            row = support[(half, piece)]
            supported = [c for c, (_, games) in enumerate(row)
                         if games >= TIE_GAMES]
            if not supported:
                raise Refused(f"no {piece} {half} cell reaches {TIE_GAMES} "
                              "games: the coordinator's")
            for c, (rows, games) in enumerate(row):
                if games >= TIE_GAMES:
                    continue
                anchor = min(supported, key=lambda a: (abs(a - c), a))
                lines.append(f"mobility_{piece}_{half} {c} {anchor}  "
                             f"# rows {rows} games {games}")
                print(f"  tie {cell_name(half, piece, c)} -> [{anchor}]: rows "
                      f"{rows}, games {games}")
                tied += 1
    with open(args.out, "w") as handle:
        handle.write("\n".join(lines) + "\n")
    print(f"ties: {tied} of {2 * CELLS} cell halves tied, written to {args.out}")
    return 0


def linear(args):
    lines = ["# S121's linear model: every cell c times its row's count-1 cell "
             "(DEC-326 (3), the curve dropped)"]
    for t, piece in enumerate(PIECES):
        for half in ("mg", "eg"):
            for c in range(WIDTH[t]):
                if c != 1:
                    lines.append(f"mobility_{piece}_{half} {c} 1 {c}")
    with open(args.out, "w") as handle:
        handle.write("\n".join(lines) + "\n")
    print(f"linear: {len(lines) - 1} cells tied to count 1, {args.out}")
    return 0


def fit_of(out, name, sample):
    """read_fit over OUT/fits/NAME/SAMPLE.log, and how the fit stopped."""
    log = os.path.join(out, "fits", name, f"{sample}.log")
    start, best, _k, _improved, _rows, _w = read_fit(log)
    best_epoch = 0
    patience = False
    for line in open(log, encoding="utf-8", errors="replace"):
        m = EPOCH.match(line)
        if m:
            best_epoch = int(m.group(1))
        if line.startswith(PATIENCE):
            patience = True
    return {"start": start, "best": best, "best_epoch": best_epoch,
            "stop": "patience" if patience else f"cap {FIT_EPOCHS}"}


def read(args):
    part = args.part
    p = plan_of(part, args.out)
    os.makedirs(os.path.join(args.out, "read"), exist_ok=True)
    lines = []

    def say(text=""):
        lines.append(text)
        print(text)

    say(f"S121 screen reading, {part}: {NAMES[part]}")
    say(f"form {p['form']}, mask {p['mask']}, fit {p['fit']}"
        + (f", against {p['pred']}" if p["pred"] else ", against its start"))
    problems = []
    try:
        mine = {s: fit_of(args.out, p["fit"], s) for s in SAMPLES}
        pred = ({s: fit_of(args.out, p["pred"], s) for s in SAMPLES}
                if p["pred"] else None)
    except Refused as why:
        problems.append(str(why))
        mine = pred = None

    if mine is not None:
        for name, fits in ((p["fit"], mine), (p["pred"], pred)):
            if fits is None:
                continue
            for s in SAMPLES:
                f = fits[s]
                say(f"  {name:<12} {s:<5} start {f['start']:.6f} best "
                    f"{f['best']:.6f} at epoch {f['best_epoch']}, stopped on "
                    f"{f['stop']}")
        if pred is None:
            # DEC-315: the gain against the run's own start, the shipped
            # constants on the fixed set.
            gains = {s: mine[s]["start"] - mine[s]["best"] for s in SAMPLES}
            rule = "g = start - best (DEC-315)"
        else:
            # DEC-326 (3): paired by sample, two fitted ends on one fixed set,
            # one K a sample.
            gains = {s: pred[s]["best"] - mine[s]["best"] for s in SAMPLES}
            rule = f"d = best({p['pred']}) - best({p['fit']}) (DEC-326 (3))"
        quarter = [gains[f"q{s}"] for s in SEEDS]
        s_dev = statistics.stdev(quarter)
        margin = max(PICK_Z * s_dev, FLOOR)
        if s_dev > SPREAD_UNREADABLE:
            problems.append(f"s {s_dev:.6f} above {SPREAD_UNREADABLE}")
        keep = gains["full"] > margin
        say(f"rule {rule}: full {gains['full']:+.6f}, s {s_dev:.6f}, m "
            f"{margin:.6f}; quarters " +
            " ".join(f"{g:+.6f}" for g in quarter))
        header = os.path.join(args.out, "fits", p["fit"], "full.hpp")
        if os.path.exists(header):
            room = headroom_of(header)
            say(f"headroom: 32 x {room} = {32 * room} of {HALF_LIMIT}")
            if 32 * room > HALF_LIMIT:
                problems.append("the full fit's tables pass the overflow "
                                "assert: the pre-registered branch, the "
                                "coordinator's")
            say("tables, full fit (deciding nothing): ")
            for name, values in sorted(weights_of(open(header).read()).items()):
                if MOBILITY_ARRAY.match(name):
                    say(f"  {name} {values}")

    if problems:
        say("unreadable: " + "; ".join(problems))
        say("PART-OUTCOME unreadable")
        status = 1
    else:
        say(f"{part} {'kept' if keep else 'dropped'}")
        if part == "P3":
            candidate_lines(args.out, say, last_kept=keep)
        say(f"PART-OUTCOME {'kept' if keep else 'dropped'}")
        status = 0
    with open(os.path.join(args.out, "read", f"{part}.txt"), "w") as handle:
        handle.write("\n".join(lines) + "\n")
    return status


def candidate_lines(out, say, last_kept):
    """The candidate and the bisection order, once P3 is read."""
    kept = {"P0": outcome_of(out, "P0") == "kept"}
    for part in ("P1", "P2a", "P2b"):
        kept[part] = outcome_of(out, part) == "kept"
    kept["P3"] = last_kept
    survivors = [p for p in PARTS if kept[p]]
    say("survivors " + (",".join(survivors) or "(none)"))
    if not survivors:
        say("candidate (none): no part lowers held-out loss past its margin; "
            "each is recorded with its figures and none plays a match")
        say("CANDIDATE none")
        return
    # The last survivor's full fit (DEC-321's form): its form and mask are the
    # survivors'.
    mask = 0
    for p in LATER:
        if kept[p]:
            mask |= BIT[p] | REQUIRES.get(p, 0)
    form = "table" if kept["P0"] else "linear"
    name = fit_name(form, mask)
    say(f"candidate {name}: {os.path.join(out, 'fits', name, 'full.hpp')}")
    order = [LETTER[p] for p in survivors]
    say("bisection on H0 or no verdict, in order: " + " | ".join(order))
    say(f"CANDIDATE {name} mask {mask}")


def headroom_of(header):
    """The largest |entry| over the header's mobility tables."""
    largest = 0
    found = 0
    for name, values in weights_of(open(header).read()).items():
        if MOBILITY_ARRAY.match(name):
            found += 1
            largest = max([largest] + [abs(v) for v in values])
    if found != 8:
        raise Refused(f"{header} emits {found} mobility tables, not 8")
    return largest


def headroom(args):
    room = headroom_of(args.header)
    print(f"mobility: 32 x {room} = {32 * room} of {HALF_LIMIT}")
    if 32 * room > HALF_LIMIT:
        raise Refused("the tables would fail the overflow static_assert; the "
                      "pre-registered branch applies, nothing is clipped")
    return 0


def start_check(args):
    emitted = weights_of(open(args.header).read())
    shipped = head_weights(args.tree)
    if not emitted:
        raise Refused(f"{args.header} emits nothing")
    bad = []
    for name, values in sorted(emitted.items()):
        if name not in shipped:
            bad.append(f"{name}: not in HEAD's sources")
            continue
        if len(values) != len(shipped[name]):
            bad.append(f"{name}: {len(values)} values, {len(shipped[name])} "
                       "at HEAD")
            continue
        if args.free and MOBILITY_ARRAY.match(name):
            continue
        if values != shipped[name]:
            bad.append(f"{name}: differs from HEAD's")
    missing = [n for n in shipped if MOBILITY_ARRAY.match(n) and n not in
               emitted]
    if missing:
        bad.append("mobility tables missing: " + ", ".join(missing))
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short",
                           "HEAD"], check=True, capture_output=True,
                          text=True).stdout.strip()
    print(f"start-check {args.header}: {len(emitted)} definitions against "
          f"HEAD {head}, mobility {'free' if args.free else 'held'}")
    if bad:
        raise Refused("the tuner did not start from HEAD's constants: "
                      + "; ".join(bad))
    print("start-check: every held value is HEAD's")
    return 0


def only_mobility(args):
    before = head_weights(args.tree)
    after = {}
    for path in ("src/eval_tables.hpp", "src/evaluation.cpp"):
        after.update(weights_of(open(os.path.join(args.tree, path)).read()))
    moved = sorted(n for n in set(before) | set(after)
                   if before.get(n) != after.get(n))
    others = [n for n in moved if not MOBILITY_ARRAY.match(n)]
    print("moved: " + (", ".join(moved) or "(nothing)"))
    if others:
        raise Refused("the paste moved more than the mobility tables: "
                      + ", ".join(others))
    return 0


def clamp(args):
    clamped = rows = None
    for line in open(args.audit, encoding="utf-8", errors="replace"):
        m = CLAMP.search(line)
        if m and line.startswith("trace:"):
            clamped = int(m.group(1))
            rows = int(TRACE_ROWS.match(line).group(1))
    if clamped is None:
        raise Refused(f"{args.audit} prints no trace line with the clamp")
    spread = {}
    for line in open(args.spread, encoding="utf-8", errors="replace"):
        m = SPREAD_ROW.match(line)
        if m and m.group(1) not in spread:
            spread[m.group(1)] = int(m.group(2))  # the mobility column
    if set(spread) != {"p99", "max"}:
        raise Refused(f"{args.spread} prints no p99 and max rows")
    share = 100.0 * clamped / rows
    print(f"clamp: binds on {clamped} of {rows} rows, {share:.3f} % against "
          f"the bound {CLAMP_SHARE_BOUND:.3f} %")
    print(f"mobility spread: p99 {spread['p99']}, max {spread['max']} "
          "(S122 inherits these; deciding nothing here)")
    if share > CLAMP_SHARE_BOUND:
        print("STOP: the candidate's clamp share passes the pre-registered "
              "bound; phase 2 stops for the coordinator (DEC-326 (9))")
    return 0


def part(args):
    """A bisection part's fit, from the tree as it stands then."""
    order = {letter: p for p, letter in LETTER.items()}
    if args.name not in order:
        raise Refused(f"unknown part {args.name}; parts are A, B1, B2a, B2b, "
                      "C")
    if not 0 <= args.landed <= 15 or (args.landed & 8 and
                                      not args.landed & 2):
        raise Refused("--landed is a parts mask, 0 to 15, bit 8 only with 2")
    if args.name == "A":
        print("part A: the table over the landed count, its own full fit")
        mask = args.landed
        form = "table"
    else:
        part_name = order[args.name]
        mask = args.landed | BIT[part_name] | REQUIRES.get(part_name, 0)
        form = args.curve
    print(f"form {form}")
    print(f"mask {mask}")
    print(f"fit {fit_name(form, mask)}")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S121's screen helpers.")
    commands = parser.add_subparsers(dest="command", required=True)

    c = commands.add_parser("plan")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)

    c = commands.add_parser("audit-check")
    c.add_argument("report")

    c = commands.add_parser("ties")
    c.add_argument("report")
    c.add_argument("--out", required=True)

    c = commands.add_parser("linear")
    c.add_argument("--out", required=True)

    c = commands.add_parser("read")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)

    c = commands.add_parser("start-check")
    c.add_argument("header")
    c.add_argument("--tree", required=True)
    c.add_argument("--free", action="store_true")

    c = commands.add_parser("headroom")
    c.add_argument("header")

    c = commands.add_parser("only-mobility")
    c.add_argument("tree")

    c = commands.add_parser("clamp")
    c.add_argument("audit")
    c.add_argument("spread")

    c = commands.add_parser("part")
    c.add_argument("name")
    c.add_argument("--landed", type=int, required=True)
    c.add_argument("--curve", choices=("table", "linear"), required=True)

    args = parser.parse_args(argv)
    handlers = {"plan": plan, "audit-check": audit_check, "ties": ties,
                "linear": linear, "read": read, "start-check": start_check,
                "headroom": headroom, "only-mobility": only_mobility,
                "clamp": clamp, "part": part}
    try:
        return handlers[args.command](args)
    except (Refused, OSError, subprocess.CalledProcessError) as why:
        print(f"S121_screen: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
