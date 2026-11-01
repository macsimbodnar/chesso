#!/usr/bin/env python3
"""S101's screen: what each part frees and against what it is read, the identity
check and the ties from the audit, the reading of the fits, the bisection, the
paste checks and the clamp. adocs/data/S101_screen.sh runs every one of them; each
also runs alone. Standard library only, plus adocs/data/S135_screen.py,
adocs/data/S123_screen.py and adocs/data/S125_screen.py, imported so the fit logs
and headers are parsed the way the screens before it did.

    S101_screen.py plan PART --out OUT
    S101_screen.py audit-check REPORT
    S101_screen.py ties REPORT --out DIR
    S101_screen.py read PART --out OUT --tree TREE
    S101_screen.py start-check HEADER --tree TREE [--free GROUPS]
    S101_screen.py headroom HEADER --tree TREE
    S101_screen.py only-family TREE
    S101_screen.py landing-check --tree TREE [--commit SHA]
    S101_screen.py only-family-pair REF CAND --tree TREE
    S101_screen.py part NAME --out OUT
    S101_screen.py clamp AUDIT SPREAD

PART is P1 to P7; the header of S101_screen.sh is the rule's statement and this
file its implementation (DEC-333 (5)). PLAN prints what a part frees -- the earlier
survivors' groups and its own -- the fit it is read against, and the tie files its
fits read, from the readings of the parts before it under OUT/read, and refuses
when one of those is missing or unreadable. AUDIT-CHECK applies the identity rule to
a tools/feature_audit report. TIES writes one tie list a part from the same report's
support lines under the pre-registered threshold. READ applies the part's rule and
writes OUT/read/PART.txt, whose last line is PART-OUTCOME kept, dropped or
unreadable; P7's reading also names the candidate and the bisection. START-CHECK
compares a fit's header with TREE's HEAD sources, the free groups' arrays excepted.
HEADROOM is src/evaluation.cpp's assert on mobility's packed sum for a header's
weights, threat_family_half() included. ONLY-FAMILY refuses a paste that moved any
weight but the family's arrays. LANDING-CHECK refuses a TREE whose HEAD is not the
landing, S101's fourteen arrays all zero, because `screen` and `all` read every gain
against that evaluation; `part`, which fits over landed parts, does not run it.
ONLY-FAMILY-PAIR is ONLY-FAMILY between two commits; adocs/data/S101_sprt.sh plays a
tree, so its guard is ONLY-FAMILY. PART prints the free set of a bisection part, A, B or C, from the survivors
P7's reading names. CLAMP is DEC-326 (9)'s reading, as DEC-333 (6) carries it: the
share of rows the clamp binds on, against the bound, and the spreads S122 inherits.
"""

import argparse
import os
import re
import statistics
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S123_screen  # noqa: E402  (adocs/data/, by path)
import S125_screen  # noqa: E402
import S135_screen  # noqa: E402

from S135_screen import Refused, head_weights, read_fit, weights_of  # noqa: E402

PARTS = [f"P{i}" for i in range(1, 8)]
GROUP = {"P1": "threat_pawn", "P2": "threat_piece", "P3": "threat_minor",
         "P4": "hanging_piece", "P5": "hanging_pawn", "P6": "threat_king",
         "P7": "threat_push"}
ARRAYS = {p: [g] for p, g in GROUP.items()}
WIDTH = {"threat_pawn": 4, "threat_piece": 5, "threat_minor": 2,
         "hanging_piece": 4, "hanging_pawn": 1, "threat_king": 4,
         "threat_push": 4}
NAMES = {"P1": "a piece a pawn attacks",
         "P2": "a piece a lesser piece attacks, N>R N>Q B>R B>Q R>Q",
         "P3": "a minor a minor attacks, N>B B>N",
         "P4": "a piece attacked and undefended, N B R Q",
         "P5": "a pawn attacked and undefended",
         "P6": "a man only the king attacks, undefended, P N B R",
         "P7": "a piece a safe pawn push would attack, N B R Q"}
# DEC-333 (7): the H0 partition, survivors only, in order; a part with no survivor
# does not exist.
BISECTION = {"A": ["P1", "P2", "P3"], "B": ["P4", "P5", "P6"], "C": ["P7"]}
# S101's fourteen arrays, the ones the landing ships at zero and everything a
# candidate may move.
FAMILY = [a for p in PARTS for a in ARRAYS[p]]
MOBILITY_TABLES = ["knight", "bishop", "rook", "queen"]
HALF_LIMIT = 32767
SEEDS = S135_screen.SEEDS
PICK_Z = S135_screen.PICK_Z
FLOOR = S135_screen.FLOOR
SPREAD_UNREADABLE = S135_screen.SPREAD_UNREADABLE

# Pre-registered 2026-11-01, before any fit (the shell script's header).
TIE_GAMES = 1000           # a half of a cell is supported from this many
EXPECTED_RANK = 1074       # of 1121: S125's 1026 and the family's 48, before
PARAMS = 1121              # the mobility and family columns no row reaches
CLAMP_SHARE_BOUND = 1.0    # percent of the corpus's rows, DEC-326 (9), DEC-333 (6)
DEAD_PAWN_COLUMNS = 32
FREE_LIST = ",".join(GROUP[p] for p in PARTS)

FIT_EPOCHS = 5000
SAMPLES = ["full"] + [f"q{s}" for s in SEEDS]

SUPPORT = re.compile(r"^support (" + "|".join(FAMILY) + r")_(mg|eg)\[(\d+)\] "
                     r"rows (\d+) games (\d+)$")
RANK = re.compile(r"^identities: rank over the rationals exactly (\d+) of "
                  r"(\d+), nullity (\d+)")
ZERO = re.compile(r"^identities: (\d+) columns zero on every row:(.*)$")
SPAN_OK = ("every null direction is one Adam never moves, one the tuner "
           "holds, or a mobility level the run's frozen columns hold")
LEVEL_HELD = re.compile(r"^identities: mobility level, .*: held by the freeze "
                        r"under --free " + re.escape(FREE_LIST) + " ")
NO_MORE = "identities: no exact dependency beyond the ones listed above"
BY_CONSTRUCTION = re.compile(r"^identities: by construction, (.*): (holds on "
                             r"every row|FAILS)$")
EPOCH = re.compile(r"^epoch\s+(\d+)\s.*\*$")
PATIENCE = "no improvement in"
MOVABLE_ARRAY = re.compile(r"^(" + "|".join(FAMILY) + r")_(mg|eg)$")
EARLIER_FAMILY = re.compile(r"^(" + "|".join(S123_screen.FAMILY +
                                             S125_screen.FAMILY) + r")_(mg|eg)\[")
CLAMP = re.compile(r"the clamp binds on (\d+)")
TRACE_ROWS = re.compile(r"^trace: (\d+) rows")
SPREAD_ROW = re.compile(r"^\s+(p99|max)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)$")
THREAT_ROW = re.compile(r"^\s+threats (p99|max)\s+(-?\d+)$")


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


def fit_name(parts):
    """A fit is named by the parts it frees: `1`, `13`, `134`."""
    return "".join(p[1:] for p in parts)


def survivors_before(out, part):
    k = PARTS.index(part)
    return [p for p in PARTS[:k] if outcome_of(out, p) == "kept"]


def plan_of(part, out):
    """{free, groups, fit, pred, ties} for PART. A part frees the earlier
    survivors and its own group, and is read against the last survivor's fit; with
    no survivor before it, against its own start, which is then the predecessor's
    evaluation (DEC-329 (10), DEC-333 (5))."""
    kept = survivors_before(out, part)
    free = kept + [part]
    return {"free": free,
            "groups": ",".join(GROUP[p] for p in free),
            "fit": fit_name(free),
            "pred": fit_name(kept) if kept else None,
            "ties": " ".join(f"ties_{p}.txt" for p in free)}


def plan(args):
    p = plan_of(args.part, args.out)
    print(f"part {args.part}: {NAMES[args.part]}")
    print(f"groups {p['groups']}")
    print(f"fit {p['fit']}")
    print(f"ties {p['ties']}")
    if p["pred"] is not None:
        print(f"pred {p['pred']}")
    return 0


def support_of(report):
    """{(array, half): [(rows, games)] * width} from the support lines."""
    out = {}
    for line in open(report, encoding="utf-8", errors="replace"):
        m = SUPPORT.match(line.rstrip("\n"))
        if m:
            array, half, cell = m.group(1), m.group(2), int(m.group(3))
            out.setdefault((array, half), {})[cell] = (int(m.group(4)),
                                                       int(m.group(5)))
    for array in FAMILY:
        for half in ("mg", "eg"):
            row = out.get((array, half), {})
            if sorted(row) != list(range(WIDTH[array])):
                raise Refused(f"{report}: the support lines for {array} "
                              f"{half} are not cells 0 to {WIDTH[array] - 1}")
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
    family_zero = [z for z in zero
                   if re.match(r"(" + "|".join(FAMILY) + r")_(mg|eg)\[", z)
                   or EARLIER_FAMILY.match(z)]
    pawn_zero = [z for z in zero
                 if re.fullmatch(r"psqt_(mg|eg)\[pawn\]\[[a-h][18]\]", z)]
    other = [z for z in zero if z not in mobility_zero + family_zero +
             pawn_zero]
    if len(pawn_zero) != DEAD_PAWN_COLUMNS:
        problems.append(f"{len(pawn_zero)} dead pawn columns, not "
                        f"{DEAD_PAWN_COLUMNS}")
    if other:
        problems.append("zero columns outside the mobility table, the three "
                        "families and the dead pawn squares: " +
                        " ".join(other))
    want = EXPECTED_RANK - len(mobility_zero) - len(family_zero)
    rank = [RANK.match(line) for line in lines]
    rank = [m for m in rank if m]
    if not rank:
        problems.append("the rank over the rationals is not exact")
    else:
        got, of, _ = (int(x) for x in rank[0].groups())
        if of != PARAMS or got != want:
            problems.append(f"rank {got} of {of}, the rule wants {want} of "
                            f"{PARAMS} ({EXPECTED_RANK} less "
                            f"{len(mobility_zero)} mobility and "
                            f"{len(family_zero)} family zero columns)")
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
                        f"under --free {FREE_LIST}, not 8")
    print(f"audit-check {args.report}: {len(mobility_zero)} mobility and "
          f"{len(family_zero)} family columns zero on every row" +
          (": " + " ".join(mobility_zero + family_zero)
           if mobility_zero + family_zero else ""))
    print(f"audit-check: the rule's rank {want} of {PARAMS}, the 8 mobility "
          "levels and the 7 S134 directions by construction, nothing else")
    if problems:
        raise Refused("the identity rule fails: " + "; ".join(problems))
    print("audit-check: pass")
    return 0


def ties(args):
    """One tie list a part, DIR/ties_P1.txt to ties_P7.txt."""
    support = support_of(args.report)
    os.makedirs(args.out, exist_ok=True)
    print(f"ties: threshold {TIE_GAMES} games a half")
    total = 0
    for part in PARTS:
        lines = [f"# S101 ties, part {part} ({GROUP[part]}), from "
                 f"{args.report}",
                 f"# a half of a cell is supported from {TIE_GAMES} games; an "
                 "unsupported one shares the nearest supported cell of its "
                 "array and half, the lower cell on a tie (DEC-333 (5), "
                 "DEC-331 (6))"]
        for array in ARRAYS[part]:
            for half in ("mg", "eg"):
                row = support[(array, half)]
                supported = [c for c, (_, games) in enumerate(row)
                             if games >= TIE_GAMES]
                if not supported:
                    raise Refused(f"no cell of {array}_{half} reaches "
                                  f"{TIE_GAMES} games: nothing to tie it to, "
                                  "the coordinator's")
                for c, (rows, games) in enumerate(row):
                    if games >= TIE_GAMES:
                        continue
                    anchor = min(supported, key=lambda a: (abs(a - c), a))
                    lines.append(f"{array}_{half} {c} {anchor}  # rows {rows} "
                                 f"games {games}")
                    print(f"  tie {array}_{half}[{c}] -> [{anchor}]: rows "
                          f"{rows}, games {games}")
                    total += 1
        path = os.path.join(args.out, f"ties_{part}.txt")
        with open(path, "w") as handle:
            handle.write("\n".join(lines) + "\n")
    cells = 2 * sum(WIDTH[a] for a in FAMILY)
    print(f"ties: {total} of {cells} cell halves tied, written to "
          f"{args.out}/ties_P{{1..7}}.txt")
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

    say(f"S101 screen reading, {part}: {NAMES[part]}")
    say(f"free {p['groups']}, fit {p['fit']}"
        + (f", against {p['pred']}" if p["pred"] else ", against its start"))
    problems = []
    try:
        mine = {s: fit_of(args.out, p["fit"], s) for s in SAMPLES}
        pred = ({s: fit_of(args.out, p["pred"], s) for s in SAMPLES}
                if p["pred"] else None)
    except Refused as why:
        problems.append(str(why))
        mine = pred = None

    keep = False
    if mine is not None:
        for name, fits in ((p["fit"], mine), (p["pred"], pred)):
            if fits is None:
                continue
            for s in SAMPLES:
                f = fits[s]
                say(f"  {name:<9} {s:<5} start {f['start']:.6f} best "
                    f"{f['best']:.6f} at epoch {f['best_epoch']}, stopped on "
                    f"{f['stop']}")
        if pred is None:
            # No survivor before it: DEC-329 (10)'s reading, the run's own start,
            # the landing's evaluation on the fixed set, is the predecessor's, so
            # its paired d is this g.
            gains = {s: mine[s]["start"] - mine[s]["best"] for s in SAMPLES}
            rule = "d = start - best (DEC-329 (10), no survivor before it)"
        else:
            # DEC-326 (3): paired by sample, two fitted ends on one fixed
            # set, one K a sample.
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
        if not os.path.exists(header):
            # The tuner prints best: before it writes the header, so a log alone
            # does not show a fit exists: the shell header's "a fit missing".
            problems.append(f"{header} is missing, so its headroom is unread and "
                            "the fit is not there to paste")
        else:
            bounds = headroom_of(header, args.tree)
            say("headroom: mobility's packed sum " + ", ".join(
                f"{half} {b} of {HALF_LIMIT}" for half, b in bounds.items()))
            if max(bounds.values()) > HALF_LIMIT:
                problems.append("the full fit passes the packed sum's "
                                "static_assert: the pre-registered branch, "
                                "the coordinator's")
            say("tables, full fit (deciding nothing; monotonicity is not "
                "imposed): ")
            for name, values in sorted(weights_of(open(header).read()).items()):
                if MOVABLE_ARRAY.match(name):
                    say(f"  {name} {values}")

    if problems:
        say("unreadable: " + "; ".join(problems))
        say("PART-OUTCOME unreadable")
        status = 1
    else:
        say(f"{part} {'kept' if keep else 'dropped'}")
        if part == "P7":
            candidate_lines(args.out, say, last_kept=keep)
        say(f"PART-OUTCOME {'kept' if keep else 'dropped'}")
        status = 0
    with open(os.path.join(args.out, "read", f"{part}.txt"), "w") as handle:
        handle.write("\n".join(lines) + "\n")
    return status


def survivors_of(out, last_kept=None):
    """The kept parts among P1 to P7, P7's from `last_kept` when given."""
    kept = [p for p in PARTS[:-1] if outcome_of(out, p) == "kept"]
    seventh = (last_kept if last_kept is not None
               else outcome_of(out, "P7") == "kept")
    return kept + (["P7"] if seventh else [])


def bisection_parts(survivors):
    """DEC-333 (7)'s partition over the survivors, in order: each of A, B and C
    that holds a survivor; none when nothing survived."""
    out = []
    for name in ("A", "B", "C"):
        parts = [p for p in BISECTION[name] if p in survivors]
        if parts:
            out.append((name, parts))
    return out


def candidate_lines(out, say, last_kept):
    """The candidate and the bisection, once P7 is read."""
    survivors = survivors_of(out, last_kept)
    say("survivors " + (",".join(survivors) or "(none)"))
    if not survivors:
        say("candidate (none): no part lowers held-out loss past its margin; each "
            "is recorded with its figures, the family stays at zero and in no "
            "expression, and none plays a match")
        say("CANDIDATE none")
        return
    # The last survivor's full fit (DEC-321's form): it frees every survivor, each
    # part having been fitted with the earlier ones live.
    name = fit_name(survivors)
    say(f"candidate {name}: {os.path.join(out, 'fits', name, 'full.hpp')}")
    say("bisection on H0 or no verdict, in order: " + " | ".join(
        f"{n} = {','.join(parts)}" for n, parts in bisection_parts(survivors)))
    say(f"CANDIDATE {name} groups " + ",".join(GROUP[p] for p in survivors))


def spread_of(values):
    """pawn_cells_reach() in src/evaluation.cpp: 15 x (highest or 0 - lowest or
    0)."""
    return 15 * (max(max(values), 0) - min(min(values), 0))


def threat_piece_reach(w):
    """threat_piece_reach() there: a rook stands in cells 0 and 2, a queen in 1, 3
    and 4."""
    above = [max(v, 0) for v in w]
    below = [min(v, 0) for v in w]
    return 15 * (max(above[0] + above[2], above[1] + above[3] + above[4]) -
                 min(below[0] + below[2], below[1] + below[3] + below[4]))


def threat_half(weights, half):
    """threat_family_half() there, one half."""
    return (spread_of(weights[f"threat_pawn_{half}"]) +
            threat_piece_reach(weights[f"threat_piece_{half}"]) +
            spread_of(weights[f"threat_minor_{half}"]) +
            spread_of(weights[f"hanging_piece_{half}"]) +
            spread_of(weights[f"hanging_pawn_{half}"]) +
            spread_of(weights[f"threat_king_{half}"]) * 8 // 15 +
            spread_of(weights[f"threat_push_{half}"]))


def headroom_of(header, tree):
    """The left side of the static_assert on mobility's packed sum, per half: 32 x
    the largest mobility cell of either half, and that half's threats."""
    weights = head_weights(tree)
    weights.update(weights_of(open(header).read()))
    for a in FAMILY:
        for half in ("mg", "eg"):
            if len(weights.get(f"{a}_{half}", [])) != WIDTH[a]:
                raise Refused(f"{a}_{half} is not {WIDTH[a]} wide in "
                              f"{header} over {tree}'s HEAD")
    largest = max(abs(v) for t in MOBILITY_TABLES for half in ("mg", "eg")
                  for v in weights[f"mobility_{t}_{half}"])
    return {half: 32 * largest + threat_half(weights, half)
            for half in ("mg", "eg")}


def headroom(args):
    bounds = headroom_of(args.header, args.tree)
    for half, bound in bounds.items():
        print(f"mobility's packed sum, {half}: {bound} of {HALF_LIMIT}, headroom "
              f"{HALF_LIMIT - bound}")
    if max(bounds.values()) > HALF_LIMIT:
        raise Refused(f"{args.header} would fail the packed sum's static_assert; "
                      "the pre-registered branch applies, the weights are not "
                      "clipped")
    return 0


def free_arrays(groups):
    """The arrays the named groups free."""
    names = [g for g in (groups or "").split(",") if g]
    by_group = {GROUP[p]: ARRAYS[p] for p in PARTS}
    unknown = [g for g in names if g not in by_group]
    if unknown:
        raise Refused(f"unknown groups {unknown}")
    return {a for g in names for a in by_group[g]}


def start_check(args):
    emitted = weights_of(open(args.header).read())
    shipped = head_weights(args.tree)
    free = free_arrays(args.free)
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
        m = MOVABLE_ARRAY.match(name)
        if m and m.group(1) in free:
            continue
        if values != shipped[name]:
            bad.append(f"{name}: differs from HEAD's")
    missing = [n for n in shipped if MOVABLE_ARRAY.match(n) and n not in
               emitted]
    if missing:
        bad.append("family arrays missing: " + ", ".join(missing))
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short",
                           "HEAD"], check=True, capture_output=True,
                          text=True).stdout.strip()
    print(f"start-check {args.header}: {len(emitted)} definitions against "
          f"HEAD {head}, free {sorted(free) or '(none)'}")
    if bad:
        raise Refused("the tuner did not start from HEAD's constants: "
                      + "; ".join(bad))
    print("start-check: every held value is HEAD's")
    return 0


def only_family(args):
    before = head_weights(args.tree)
    after = {}
    for path in ("src/eval_tables.hpp", "src/evaluation.cpp"):
        after.update(weights_of(open(os.path.join(args.tree, path)).read()))
    moved = sorted(n for n in set(before) | set(after)
                   if before.get(n) != after.get(n))
    others = [n for n in moved if not MOVABLE_ARRAY.match(n)]
    print("moved: " + (", ".join(moved) or "(nothing)"))
    if others:
        raise Refused("the paste moved more than the family's arrays: " +
                      ", ".join(others))
    return 0


def landing_check(args):
    """HEAD's fourteen arrays all zero, or a named commit's: S101_sprt.sh's guard
    reads the commit the screen ran from, not what is checked out."""
    commit = args.commit or "HEAD"
    shipped = commit_weights(args.tree, commit)
    bad = []
    for a in FAMILY:
        for half in ("mg", "eg"):
            name = f"{a}_{half}"
            values = shipped.get(name)
            if values is None:
                bad.append(f"{name}: not in {commit}'s sources")
            elif len(values) != WIDTH[a]:
                bad.append(f"{name}: {len(values)} wide, not {WIDTH[a]}")
            elif any(values):
                bad.append(f"{name}: {values}, not zero")
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short", commit],
                          check=True, capture_output=True,
                          text=True).stdout.strip()
    if bad:
        raise Refused(f"{commit} {head} is not the landing, the family at zero: " +
                      "; ".join(bad) + "; the screen reads every gain against "
                      "that evaluation, so it runs only there")
    print(f"landing-check: {commit} {head}'s fourteen family arrays are all zero")
    return 0


def commit_weights(tree, commit):
    """Every define and int array of TREE's evaluation sources at COMMIT."""
    out = {}
    for path in ("src/eval_tables.hpp", "src/evaluation.cpp"):
        text = subprocess.run(["git", "-C", tree, "show", f"{commit}:{path}"],
                              check=True, capture_output=True,
                              text=True).stdout
        out.update(weights_of(text))
    return out


def only_family_pair(args):
    ref = commit_weights(args.tree, args.ref)
    cand = commit_weights(args.tree, args.cand)
    moved = sorted(n for n in set(ref) | set(cand) if ref.get(n) != cand.get(n))
    others = [n for n in moved if not MOVABLE_ARRAY.match(n)]
    missing = [f"{a}_{h}" for a in FAMILY for h in ("mg", "eg")
               if f"{a}_{h}" not in ref or f"{a}_{h}" not in cand]
    print("moved between REF and CAND: " + (", ".join(moved) or "(nothing)"))
    if missing:
        raise Refused("family arrays missing at REF or CAND: "
                      + ", ".join(missing))
    if others:
        raise Refused("CAND moves more than the family's arrays against REF: "
                      + ", ".join(others))
    print("only-family-pair: every weight outside the family is REF's")
    return 0


def part(args):
    """A bisection part's free set, fitted over the tree as it stands then: the
    landed parts are in its sources and held there. Each part frees its survivors'
    groups."""
    if args.name not in BISECTION:
        raise Refused(f"unknown part {args.name}; parts are A, B, C")
    survivors = survivors_of(args.out)
    parts = dict(bisection_parts(survivors)).get(args.name)
    if not parts:
        raise Refused(f"part {args.name} holds no survivor of the screen")
    print(f"part {args.name}: {', '.join(NAMES[p] for p in parts)}")
    print(f"groups {','.join(GROUP[p] for p in parts)}")
    print(f"fit part_{args.name}")
    print(f"ties {' '.join(f'ties_{p}.txt' for p in parts)}")
    return 0


def clamp(args):
    """DEC-326 (9)'s reading on a pasted tree's own tools. S122 inherits the
    spreads: mobility's own, S121's column, kept apart from the threats' by
    evaluate_expensive_terms() (DEC-333 (10))."""
    clamped = rows = None
    for line in open(args.audit, encoding="utf-8", errors="replace"):
        m = CLAMP.search(line)
        if m and line.startswith("trace:"):
            clamped = int(m.group(1))
            rows = int(TRACE_ROWS.match(line).group(1))
    if clamped is None:
        raise Refused(f"{args.audit} prints no trace line with the clamp")
    mobility = {}
    combined = {}
    threats = {}
    for line in open(args.spread, encoding="utf-8", errors="replace"):
        m = SPREAD_ROW.match(line)
        if m and m.group(1) not in mobility:
            mobility[m.group(1)] = int(m.group(2))
            combined[m.group(1)] = int(m.group(4))
        m = THREAT_ROW.match(line)
        if m:
            threats[m.group(1)] = int(m.group(2))
    if set(mobility) != {"p99", "max"} or set(threats) != {"p99", "max"}:
        raise Refused(f"{args.spread} prints no p99 and max rows")
    share = 100.0 * clamped / rows
    print(f"clamp: binds on {clamped} of {rows} rows, {share:.3f} % against the "
          f"bound {CLAMP_SHARE_BOUND:.3f} %")
    print(f"mobility spread: p99 {mobility['p99']}, max {mobility['max']}; threats "
          f"p99 {threats['p99']}, max {threats['max']}; combined p99 "
          f"{combined['p99']}, max {combined['max']} (S122 inherits these; deciding "
          "nothing here)")
    if share > CLAMP_SHARE_BOUND:
        print("STOP: the candidate's clamp share passes the pre-registered bound; "
              "phase 2 stops for the coordinator (DEC-326 (9), DEC-333 (6))")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S101's screen helpers.")
    commands = parser.add_subparsers(dest="command", required=True)

    c = commands.add_parser("plan")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)

    c = commands.add_parser("audit-check")
    c.add_argument("report")

    c = commands.add_parser("ties")
    c.add_argument("report")
    c.add_argument("--out", required=True)

    c = commands.add_parser("read")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)
    c.add_argument("--tree", required=True)

    c = commands.add_parser("start-check")
    c.add_argument("header")
    c.add_argument("--tree", required=True)
    c.add_argument("--free", default="")

    c = commands.add_parser("headroom")
    c.add_argument("header")
    c.add_argument("--tree", required=True)

    c = commands.add_parser("only-family")
    c.add_argument("tree")

    c = commands.add_parser("landing-check")
    c.add_argument("--tree", required=True)
    c.add_argument("--commit", default="")

    c = commands.add_parser("only-family-pair")
    c.add_argument("ref")
    c.add_argument("cand")
    c.add_argument("--tree", required=True)

    c = commands.add_parser("part")
    c.add_argument("name")
    c.add_argument("--out", required=True)

    c = commands.add_parser("clamp")
    c.add_argument("audit")
    c.add_argument("spread")

    args = parser.parse_args(argv)
    handlers = {"plan": plan, "audit-check": audit_check, "ties": ties,
                "read": read, "start-check": start_check,
                "headroom": headroom, "only-family": only_family,
                "landing-check": landing_check,
                "only-family-pair": only_family_pair, "part": part,
                "clamp": clamp}
    try:
        return handlers[args.command](args)
    except (Refused, OSError, subprocess.CalledProcessError) as why:
        print(f"S101_screen: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
