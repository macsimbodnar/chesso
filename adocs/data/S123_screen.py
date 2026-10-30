#!/usr/bin/env python3
"""S123's screen: what each part frees and against what it is read, the
identity check and the ties from the audit, the reading of the fits, the
bisection, and the paste checks. adocs/data/S123_screen.sh runs every one of
them; each also runs alone. Standard library only, plus
adocs/data/S135_screen.py, imported so the fit logs and headers are parsed
exactly the way S135's, S136's and S121's screens parsed them.

    S123_screen.py plan PART --out OUT
    S123_screen.py audit-check REPORT
    S123_screen.py ties REPORT --out DIR
    S123_screen.py read PART --out OUT --tree TREE
    S123_screen.py start-check HEADER --tree TREE [--free GROUPS]
    S123_screen.py headroom HEADER --tree TREE
    S123_screen.py only-family TREE
    S123_screen.py landing-check --tree TREE
    S123_screen.py only-family-pair REF CAND --tree TREE
    S123_screen.py part NAME

PART is A, B or C; the header of S123_screen.sh is the rule's statement and
this file its implementation. PLAN prints what a part frees -- its own group
and the earlier survivors' -- the fit it is read against, and the tie files
its fits read, from the readings of the parts before it under OUT/read, and
refuses when one of those is missing or unreadable. AUDIT-CHECK applies the
identity rule to a tools/feature_audit report. TIES writes one tie list a
group from the same report's support lines under the pre-registered
threshold. READ applies the part's rule and writes OUT/read/PART.txt, whose
last line is PART-OUTCOME kept, dropped or unreadable; C's reading also names
the candidate and the bisection order. START-CHECK compares a fit's header
with TREE's HEAD sources, the free groups' arrays excepted. HEADROOM is
src/evaluation.cpp's stage-one overflow arithmetic for a header's weights.
ONLY-FAMILY refuses a paste that moved any weight but the family's arrays.
LANDING-CHECK refuses a TREE whose HEAD's ten family arrays are not all zero:
`screen` and `all` read every gain against the shipped evaluation with the
family at zero, so a HEAD past the landing (a candidate committed) would read
them against another baseline in silence; `part`, which fits over landed
parts, does not run it. ONLY-FAMILY-PAIR is ONLY-FAMILY between two commits,
every non-family weight of REF's sources equal to CAND's, for
adocs/data/S123_sprt.sh's guard. PART prints the free set a bisection part
is fitted with.
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

PARTS = ["A", "B", "C"]
GROUP = {"A": "passed_advance", "B": "passed_kings", "C": "candidate_passers"}
ARRAYS = {"A": ["passed_free", "passed_safe"],
          "B": ["passed_king_own", "passed_king_enemy"],
          "C": ["candidate_passer"]}
WIDTH = {"passed_free": 6, "passed_safe": 6, "passed_king_own": 6,
         "passed_king_enemy": 6, "candidate_passer": 5}
NAMES = {"A": "can-advance and safe-advance increments per own rank 2 to 7",
         "B": "a slope per own rank 2 to 7 on each king's distance to the "
              "push square",
         "C": "candidate passers per own rank 2 to 6"}
FAMILY = [a for p in PARTS for a in ARRAYS[p]]
HALF_LIMIT = 32767
SEEDS = S135_screen.SEEDS
PICK_Z = S135_screen.PICK_Z
FLOOR = S135_screen.FLOOR
SPREAD_UNREADABLE = S135_screen.SPREAD_UNREADABLE

# Pre-registered 2026-10-30, before any fit (the shell script's header).
TIE_GAMES = 1000           # a half of a rank cell is supported from this many
EXPECTED_RANK = 966        # of 1013: S121's 908 and the family's 58, before
PARAMS = 1013              # the mobility and family columns no row reaches
DEAD_PAWN_COLUMNS = 32

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
                        r"under --free passed_advance,passed_kings,"
                        r"candidate_passers ")
NO_MORE = "identities: no exact dependency beyond the ones listed above"
BY_CONSTRUCTION = re.compile(r"^identities: by construction, (.*): (holds on "
                             r"every row|FAILS)$")
EPOCH = re.compile(r"^epoch\s+(\d+)\s.*\*$")
PATIENCE = "no improvement in"
FAMILY_ARRAY = re.compile(r"^(" + "|".join(FAMILY) + r")_(mg|eg)$")


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


def plan_of(part, out):
    """{free, fit, pred, ties} for PART, from the readings before it. A fit is
    named by the parts it frees in order, `A`, `AB`, `AC`, `ABC`, `B`, `BC`,
    `C`; the predecessor is the last survivor's fit, or none -- the shipped
    start -- when no part before it survived."""
    k = PARTS.index(part)
    kept = [p for p in PARTS[:k] if outcome_of(out, p) == "kept"]
    free = kept + [part]
    return {"free": free,
            "groups": ",".join(GROUP[p] for p in free),
            "fit": "".join(free),
            "pred": "".join(kept) if kept else None,
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
                              f"{half} are not ranks 0 to {WIDTH[array] - 1}")
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
                   if re.match(r"(" + "|".join(FAMILY) + r")_(mg|eg)\[", z)]
    pawn_zero = [z for z in zero
                 if re.fullmatch(r"psqt_(mg|eg)\[pawn\]\[[a-h][18]\]", z)]
    other = [z for z in zero if z not in mobility_zero + family_zero +
             pawn_zero]
    if len(pawn_zero) != DEAD_PAWN_COLUMNS:
        problems.append(f"{len(pawn_zero)} dead pawn columns, not "
                        f"{DEAD_PAWN_COLUMNS}")
    if other:
        problems.append("zero columns outside the mobility table, the family "
                        "and the dead pawn squares: " + " ".join(other))
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
                        "under --free passed_advance,passed_kings,"
                        "candidate_passers, not 8")
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
    """One tie list a group, DIR/ties_A.txt, ties_B.txt, ties_C.txt."""
    support = support_of(args.report)
    os.makedirs(args.out, exist_ok=True)
    print(f"ties: threshold {TIE_GAMES} games a half")
    total = 0
    for part in PARTS:
        lines = [f"# S123 ties, part {part} ({GROUP[part]}), from "
                 f"{args.report}",
                 f"# a half of a rank cell is supported from {TIE_GAMES} "
                 "games; an unsupported one shares the nearest supported rank "
                 "of its array and half, the lower rank on a tie (DEC-329 "
                 "(4), DEC-326 (5))"]
        for array in ARRAYS[part]:
            for half in ("mg", "eg"):
                row = support[(array, half)]
                supported = [r for r, (_, games) in enumerate(row)
                             if games >= TIE_GAMES]
                if not supported:
                    raise Refused(f"no rank of {array}_{half} reaches "
                                  f"{TIE_GAMES} games: the coordinator's")
                for r, (rows, games) in enumerate(row):
                    if games >= TIE_GAMES:
                        continue
                    anchor = min(supported, key=lambda a: (abs(a - r), a))
                    lines.append(f"{array}_{half} {r} {anchor}  # rows {rows} "
                                 f"games {games}")
                    print(f"  tie {array}_{half}[{r}] -> [{anchor}]: rows "
                          f"{rows}, games {games}")
                    total += 1
        path = os.path.join(args.out, f"ties_{part}.txt")
        with open(path, "w") as handle:
            handle.write("\n".join(lines) + "\n")
    cells = 2 * sum(WIDTH.values())
    print(f"ties: {total} of {cells} rank-cell halves tied, written to "
          f"{args.out}/ties_{{A,B,C}}.txt")
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

    say(f"S123 screen reading, {part}: {NAMES[part]}")
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
                say(f"  {name:<4} {s:<5} start {f['start']:.6f} best "
                    f"{f['best']:.6f} at epoch {f['best_epoch']}, stopped on "
                    f"{f['stop']}")
        if pred is None:
            # DEC-315: the gain against the run's own start, the shipped
            # constants on the fixed set. A part with no survivor before it
            # reads the same way: its predecessor is the start.
            gains = {s: mine[s]["start"] - mine[s]["best"] for s in SAMPLES}
            rule = "g = start - best (DEC-315)"
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
        if os.path.exists(header):
            bounds = headroom_of(header, args.tree)
            say("headroom: stage-one bound " + ", ".join(
                f"{half} {b} of {HALF_LIMIT}" for half, b in bounds.items()))
            if max(bounds.values()) > HALF_LIMIT:
                problems.append("the full fit passes stage_one_bound's "
                                "static_assert: the pre-registered branch, "
                                "the coordinator's")
            say("tables, full fit (deciding nothing; monotonicity is not "
                "imposed): ")
            for name, values in sorted(weights_of(open(header).read()).items()):
                if FAMILY_ARRAY.match(name):
                    say(f"  {name} {values}")

    if problems:
        say("unreadable: " + "; ".join(problems))
        say("PART-OUTCOME unreadable")
        status = 1
    else:
        say(f"{part} {'kept' if keep else 'dropped'}")
        if part == "C":
            candidate_lines(args.out, say, last_kept=keep)
        say(f"PART-OUTCOME {'kept' if keep else 'dropped'}")
        status = 0
    with open(os.path.join(args.out, "read", f"{part}.txt"), "w") as handle:
        handle.write("\n".join(lines) + "\n")
    return status


def candidate_lines(out, say, last_kept):
    """The candidate and the bisection order, once C is read."""
    kept = {p: outcome_of(out, p) == "kept" for p in ("A", "B")}
    kept["C"] = last_kept
    survivors = [p for p in PARTS if kept[p]]
    say("survivors " + (",".join(survivors) or "(none)"))
    if not survivors:
        say("candidate (none): no part lowers held-out loss past its margin; "
            "each is recorded with its figures and none plays a match")
        say("CANDIDATE none")
        return
    # The last survivor's full fit (DEC-321's form): it frees every survivor,
    # each part having been fitted with the earlier survivors live.
    name = "".join(survivors)
    say(f"candidate {name}: {os.path.join(out, 'fits', name, 'full.hpp')}")
    say("bisection on H0 or no verdict, in order: " + " | ".join(survivors))
    say(f"CANDIDATE {name} groups "
        + ",".join(GROUP[p] for p in survivors))


def half_bound(weights, half, reach):
    """src/evaluation.cpp stage_one_half() with passer_family_half(), for one
    half: S135_screen.py's arithmetic and S123's family term."""
    bound = S135_screen.half_bound(weights, half, reach)
    largest = {a: max(abs(w) for w in weights[f"{a}_{half}"]) for a in FAMILY}
    bound += 2 * 15 * (largest["passed_free"] + largest["passed_safe"])
    bound += 2 * 15 * 7 * (largest["passed_king_own"] +
                           largest["passed_king_enemy"])
    bound += 2 * 15 * largest["candidate_passer"]
    return bound


def headroom_of(header, tree):
    weights = head_weights(tree)
    weights.update(weights_of(open(header).read()))
    for a in FAMILY:
        for half in ("mg", "eg"):
            if len(weights.get(f"{a}_{half}", [])) != WIDTH[a]:
                raise Refused(f"{a}_{half} is not {WIDTH[a]} wide in "
                              f"{header} over {tree}'s HEAD")
    reach = S135_screen.reach_of(tree)
    return {half: half_bound(weights, half, reach) for half in ("mg", "eg")}


def headroom(args):
    bounds = headroom_of(args.header, args.tree)
    for half, bound in bounds.items():
        print(f"stage-one bound, {half}: {bound} of {HALF_LIMIT}, headroom "
              f"{HALF_LIMIT - bound}")
    if max(bounds.values()) > HALF_LIMIT:
        raise Refused(f"{args.header} would fail stage_one_bound's "
                      "static_assert; the pre-registered branch applies, the "
                      "weights are not clipped")
    return 0


def free_arrays(groups):
    """The family arrays the named groups free."""
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
        m = FAMILY_ARRAY.match(name)
        if m and m.group(1) in free:
            continue
        if values != shipped[name]:
            bad.append(f"{name}: differs from HEAD's")
    missing = [n for n in shipped if FAMILY_ARRAY.match(n) and n not in
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
    others = [n for n in moved if not FAMILY_ARRAY.match(n)]
    print("moved: " + (", ".join(moved) or "(nothing)"))
    if others:
        raise Refused("the paste moved more than the family's arrays: "
                      + ", ".join(others))
    return 0


def landing_check(args):
    shipped = head_weights(args.tree)
    bad = []
    for a in FAMILY:
        for half in ("mg", "eg"):
            name = f"{a}_{half}"
            values = shipped.get(name)
            if values is None:
                bad.append(f"{name}: not in HEAD's sources")
            elif len(values) != WIDTH[a]:
                bad.append(f"{name}: {len(values)} wide, not {WIDTH[a]}")
            elif any(values):
                bad.append(f"{name}: {values}, not zero")
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short",
                           "HEAD"], check=True, capture_output=True,
                          text=True).stdout.strip()
    if bad:
        raise Refused(f"HEAD {head} is not the landing, the family at zero: "
                      + "; ".join(bad) + "; the screen reads every gain "
                      "against the shipped evaluation with the family at "
                      "zero, so it runs only there")
    print(f"landing-check: HEAD {head}'s ten family arrays are all zero")
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
    others = [n for n in moved if not FAMILY_ARRAY.match(n)]
    missing = [f"{a}_{h}" for a in FAMILY for h in ("mg", "eg")
               if f"{a}_{h}" not in ref or f"{a}_{h}" not in cand]
    print("moved between REF and CAND: " + (", ".join(moved) or "(nothing)"))
    if missing:
        raise Refused("family arrays missing at REF or CAND: "
                      + ", ".join(missing))
    if others:
        raise Refused("CAND moves more than the family's arrays against REF: "
                      + ", ".join(others))
    print("only-family-pair: every non-family weight is REF's")
    return 0


def part(args):
    """A bisection part's free set, fitted over the tree as it stands then:
    the landed parts are in its sources and held there."""
    if args.name not in PARTS:
        raise Refused(f"unknown part {args.name}; parts are A, B, C")
    print(f"part {args.name}: {NAMES[args.name]}")
    print(f"groups {GROUP[args.name]}")
    print(f"fit part_{args.name}")
    print(f"ties ties_{args.name}.txt")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S123's screen helpers.")
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

    c = commands.add_parser("only-family-pair")
    c.add_argument("ref")
    c.add_argument("cand")
    c.add_argument("--tree", required=True)

    c = commands.add_parser("part")
    c.add_argument("name")

    args = parser.parse_args(argv)
    handlers = {"plan": plan, "audit-check": audit_check, "ties": ties,
                "read": read, "start-check": start_check,
                "headroom": headroom, "only-family": only_family,
                "landing-check": landing_check,
                "only-family-pair": only_family_pair, "part": part}
    try:
        return handlers[args.command](args)
    except (Refused, OSError, subprocess.CalledProcessError) as why:
        print(f"S123_screen: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
