#!/usr/bin/env python3
"""S124's screen helpers: the identity rule over tools/feature_audit's report, the support
rule and F3's tie, what each part frees and against what it is read, the K iteration's
convergence, the tablebase guard's pairing, and the reading of a part. Standard library
only, plus adocs/data/S135_screen.py for the tuner log's lines, as S122_screen.py imports it.
adocs/data/S124_screen.sh runs every one; each also runs alone.

    S124_screen.py identity-check REPORT
    S124_screen.py ties REPORT --out DIR
    S124_screen.py plan PART --out OUT
    S124_screen.py converged K_BEFORE K_AFTER
    S124_screen.py guard-read PRED_GAMES PART_GAMES --label TEXT [--beside]
    S124_screen.py vector-edit BASE --cells LIST|all (--source VECTOR | --value N) --out FILE
    S124_screen.py read PART --out OUT --stamp SHA
    S124_screen.py landing-check HEADER

PART is F1, F2 or F3, DEC-350 (7)'s parts in that order; S124_screen.sh's header is the
rule's statement and this file its implementation. IDENTITY-CHECK is the identity rule as
the step file restated it on 2026-11-06 for a local rank. TIES applies the support rule
(DEC-326 (5)): OCB_PURE P<=2 under 1000 games stops before F1, OCB_PURE P>=5 under 1000 is
tied to P=3-4 at scale 1, any other cell under 1000 stops (amendment 9, 2026-11-06). PLAN prints a part's free groups
-- the earlier survivors' and its own -- its fit set's name, its predecessor's and its ties.
CONVERGED says whether a K refit has reached its fixed point, one unit of the fourth place.
GUARD-READ pairs two vectors' losses on the tablebase-labelled rows by game, both read at
one K (DEC-357), the refusal a rise past GUARD_Z of its errors; --beside prints the reading
at K_0, which decides nothing. VECTOR-EDIT writes the smoke's two guard probes. READ applies a part's rule and writes OUT/read/PART.txt,
whose last line is PART-OUTCOME kept, dropped or unreadable. LANDING-CHECK refuses a
zero-epoch header whose scale factors are not all F or whose mop-up is not zero.
"""

import argparse
import math
import os
import re
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S135_screen  # noqa: E402  (adocs/data/, by path)

from S135_screen import Refused, read_fit  # noqa: E402

PARTS = ["F1", "F2", "F3"]
GROUP = {"F1": "endgame_scale.f1", "F2": "endgame_scale.f2", "F3": "endgame_scale.f3"}
CELLS = ["P=0 d<=0", "P=0 d=1", "P=0 d>=2", "P=1 d<=0", "P=1 d=1", "P=1 d>=2", "P=2 d<=0",
         "P=2 d=1", "P=2 d>=2", "OCB_PURE P<=2", "OCB_PURE P=3-4", "OCB_PURE P>=5",
         "OCB_ROOK"]
PART_CELLS = {"F1": [0, 1, 9], "F2": [2, 3, 4, 5, 6, 7, 8], "F3": [10, 11, 12]}
OCB_LOW, OCB_MID, OCB_HIGH = 9, 10, 11

SAMPLES = ["full", "q1", "q2", "q3", "q4", "q5"]
PICK_Z = S135_screen.PICK_Z                  # 1.645
FLOOR = S135_screen.FLOOR                    # 2e-6
SPREAD_UNREADABLE = S135_screen.SPREAD_UNREADABLE
GUARD_Z = 1.645
TIE_GAMES = 1000
FIT_EPOCHS = int(os.environ.get("FIT_EPOCHS", "20000"))
K_STEP = 1e-4                                # the tuner's fourth place
CHECK_BOUND = 1e-6

# The identity rule, restated 2026-11-06 for the local rank (the step file).
PARAMS = 1190
EXPECTED_RANK = 1142
DEAD_PAWN_COLUMNS = 32
BY_CONSTRUCTION = {"S134": 7, "S121": 8, "S124": 1}

RANK = re.compile(r"^identities: rank over the rationals exactly (\d+) of (\d+), "
                  r"nullity (\d+), every null direction listed above$")
ZERO = re.compile(r"^identities: (\d+) columns zero on every row:(.*)$")
LOCAL = "identities: the curve's columns linearised at the weights read"
NO_MORE = "identities: no exact dependency beyond the ones listed above"
BUILT = re.compile(r"^identities: by construction, (.*): (holds on every row|FAILS)$")
SPAN = re.compile(r"^identities: .* span (\d+) of at most (\d+) null dimensions: (.*)$")
SPAN_OK = "every null direction is one Adam never moves"
DESIGN = re.compile(r"^identities: design check: .* worst \|difference\| (\S+?)[;\s]")
LINEAR = re.compile(r"^identities: linearisation check: (\d+) rows .* worst "
                    r"\|difference\| (\S+) in d\(score\)/d\(param\)")
TRACE = re.compile(r"^trace: \d+ rows, \d+ entries; (\d+) differ from evaluate\(\)")
SUPPORT = re.compile(r"^support endgame_scale\[(\d+)\] rows (\d+) games (\d+) "
                     r"candidate_rows (\d+)$")
MOPUP_SUPPORT = re.compile(r"^support mopup_(mg|eg)\[(\d)\] rows (\d+) games (\d+)$")
DECIDED = re.compile(r"^r(\d+) K (\S+)$")
ARRAY = re.compile(r"^(?:constexpr|const) int (\w+)\[\w*\] = \{([^}]*)\};", re.M)
ENGINE = re.compile(r"^// engine +(\S+),", re.M)
CAP = re.compile(r"^// run +(\d+) epochs max", re.M)
GUARD = re.compile(r"^GUARD (pass|refused) ")


def report_lines(path):
    return open(path, encoding="utf-8", errors="replace").read().splitlines()


# --------------------------------------------------------------- identity

def identity_check(args):
    lines = report_lines(args.report)
    problems = []

    trace = [m for m in (TRACE.match(x) for x in lines) if m]
    if not trace or int(trace[0].group(1)) != 0:
        problems.append("the trace's identity does not hold on every row")
    if not any(x.startswith(LOCAL) for x in lines):
        problems.append("the report is not in the local form")

    zero = [m for m in (ZERO.match(x) for x in lines) if m]
    if not zero:
        problems.append("no zero-column line")
    else:
        names = zero[0].group(2).split()
        dead = [z for z in names if re.fullmatch(r"psqt_(mg|eg)\[pawn\]\[[a-h][18]\]", z)]
        other = [z for z in names if z not in dead]
        if len(dead) != DEAD_PAWN_COLUMNS or other:
            problems.append(f"zero columns {len(dead)} dead pawn squares and {other or 'none'} "
                            f"other; the rule wants the {DEAD_PAWN_COLUMNS} and nothing else")

    built = [m for m in (BUILT.match(x) for x in lines) if m]
    counts = {"S134": 0, "S121": 0, "S124": 0, "S122": 0}
    for m in built:
        tag = re.search(r"\((S12[124])\)$", m.group(1))
        counts[tag.group(1) if tag else "S134"] += 1
        if m.group(2) == "FAILS":
            problems.append(f"a dependency by construction fails: {m.group(1)}")
    if counts["S122"]:
        problems.append(f"{counts['S122']} curve duplicates listed: the rule's local form has none")
    for tag, want in BY_CONSTRUCTION.items():
        if counts[tag] != want:
            problems.append(f"{counts[tag]} of {tag}'s dependencies by construction, not {want}")
    if NO_MORE not in lines:
        problems.append("an exact dependency beyond the 16 by construction")

    rank = [m for m in (RANK.match(x) for x in lines) if m]
    nullity = PARAMS - EXPECTED_RANK
    if not rank:
        problems.append("the rank over the rationals is not exact")
    else:
        got, of, null = (int(x) for x in rank[0].groups())
        if (got, of, null) != (EXPECTED_RANK, PARAMS, nullity):
            problems.append(f"rank {got} of {of}, nullity {null}; the rule wants "
                            f"{EXPECTED_RANK} of {PARAMS}, nullity {nullity}")

    span = [m for m in (SPAN.match(x) for x in lines) if m]
    if not span or int(span[0].group(1)) != nullity or int(span[0].group(2)) != nullity \
            or not span[0].group(3).startswith(SPAN_OK):
        problems.append(f"the span line is not {nullity} of {nullity} with every direction held")

    for name, pattern in (("design check", DESIGN), ("linearisation check", LINEAR)):
        m = [m for m in (pattern.match(x) for x in lines) if m]
        worst = float(m[0].groups()[-1]) if m else float("inf")
        if not m or not worst < CHECK_BOUND:
            problems.append(f"the {name} reads {worst}, not under {CHECK_BOUND}")
        else:
            print(f"identity-check: {name} worst {worst:.3g}, under {CHECK_BOUND}")

    support = {int(m.group(1)): (int(m.group(2)), int(m.group(3)))
               for m in (SUPPORT.match(x) for x in lines) if m}
    if sorted(support) != list(range(len(CELLS))):
        problems.append("the support lines are not the thirteen cells")
    empty = [CELLS[c] for c, (rows, _g) in support.items() if rows == 0]
    if empty:
        problems.append("cells with no row: " + ", ".join(empty))

    print(f"identity-check {args.report}: the rule's rank {EXPECTED_RANK} of {PARAMS}, "
          f"nullity {nullity}: the {DEAD_PAWN_COLUMNS} dead pawn columns, S134's 7, S121's 8 "
          "and the mop-up corner pair, no curve duplicate at the local point")
    if rank:
        print("identity-check: read " + rank[0].group(0)[len("identities: "):])
    if problems:
        raise Refused("the identity rule fails: " + "; ".join(problems))
    print("identity-check: pass")
    return 0


def ties(args):
    lines = report_lines(args.report)
    support = {int(m.group(1)): (int(m.group(2)), int(m.group(3)))
               for m in (SUPPORT.match(x) for x in lines) if m}
    if sorted(support) != list(range(len(CELLS))):
        raise Refused(f"{args.report}: the support lines are not the thirteen cells")
    os.makedirs(args.out, exist_ok=True)
    print(f"ties: a cell is supported from {TIE_GAMES} games (DEC-326 (5))")
    for c, (rows, games) in sorted(support.items()):
        print(f"  {CELLS[c]:<16} rows {rows:7d} games {games:6d}"
              + ("" if games >= TIE_GAMES else "  UNDER"))
    for m in (MOPUP_SUPPORT.match(x) for x in lines):
        if m:
            print(f"  mopup_{m.group(1)}[{m.group(2)}]   rows {int(m.group(3)):7d} "
                  f"games {int(m.group(4)):6d}  (a reading: mop-up is P0's and P1's)")
    lines_out = [f"# S124 ties, F3, from {args.report}",
                 f"# OCB_PURE P>=5 is tied to P=3-4 at scale 1 when under {TIE_GAMES} games "
                 "(DEC-350 (2), DEC-326 (5))"]
    if support[OCB_HIGH][1] < TIE_GAMES:
        lines_out.append(f"endgame_scale {OCB_HIGH} {OCB_MID}  # games {support[OCB_HIGH][1]}")
        print(f"ties: endgame_scale {OCB_HIGH} -> {OCB_MID}, OCB_PURE P>=5 on "
              f"{support[OCB_HIGH][1]} games")
    else:
        print(f"ties: none; OCB_PURE P>=5 on {support[OCB_HIGH][1]} games, so F3 fits it free")
    with open(os.path.join(args.out, "ties_f3.txt"), "w") as handle:
        handle.write("\n".join(lines_out) + "\n")
    if support[OCB_LOW][1] < TIE_GAMES:
        raise Refused(f"STOP: OCB_PURE P<=2 stands on {support[OCB_LOW][1]} games, under "
                      f"{TIE_GAMES}, with no supported neighbour in F1: the coordinator's "
                      "before F1 is fitted")
    other = [CELLS[c] for c, (_r, g) in support.items()
             if g < TIE_GAMES and c not in (OCB_LOW, OCB_HIGH)]
    if other:
        raise Refused("STOP: under the support rule and no tie pre-registered for it: "
                      + ", ".join(other) + "; the coordinator's")
    return 0


# ------------------------------------------------------------------ plans

def outcome_of(out, part):
    path = os.path.join(out, "read", f"{part}.txt")
    if not os.path.exists(path):
        return None
    last = [x for x in report_lines(path) if x.startswith("PART-OUTCOME ")]
    return last[-1].split()[1] if last else None


def fit_name(parts):
    return "_".join(p.lower() for p in parts)


def plan_of(part, out):
    before = PARTS[:PARTS.index(part)]
    survivors = []
    for p in before:
        outcome = outcome_of(out, p)
        if outcome not in ("kept", "dropped"):
            raise Refused(f"{p} has no decided reading ({outcome or 'none'}): read it first")
        if outcome == "kept":
            survivors.append(p)
    free = survivors + [part]
    ties_files = ["ties_f3.txt"] if "F3" in free else []
    return {"groups": ",".join(GROUP[p] for p in free), "name": fit_name(free),
            "pred": fit_name(survivors) if survivors else "landed",
            "survivors": survivors, "ties": " ".join(ties_files)}


def plan(args):
    p = plan_of(args.part, args.out)
    print(f"groups {p['groups']}")
    print(f"name {p['name']}")
    print(f"pred {p['pred']}")
    print(f"ties {p['ties']}")
    return 0


def converged(args):
    before, after = float(args.before), float(args.after)
    done = abs(after - before) <= K_STEP + 1e-12
    print("converged" if done else "moved")
    return 0


# ------------------------------------------------------------------ guard

def games_of(path):
    """tools/margin_census --games' `index game rows sse` lines, in file order."""
    out = []
    for line in open(path):
        if line.startswith("#") or not line.strip():
            continue
        index, game, rows, sse = line.split()
        out.append((int(index), int(game), int(rows), float(sse)))
    if [g[0] for g in out] != list(range(len(out))):
        raise Refused(f"{path}'s game indices are not 0 to {len(out) - 1}")
    return out


def guard_pair(pred_path, part_path):
    """The rise of the part's loss over its predecessor's on the guard's rows, and its
    error by game: the jackknife over games of a ratio of sums, S122_screen.py side-read's."""
    pred = games_of(pred_path)
    mine = games_of(part_path)
    if [(g[1], g[2]) for g in pred] != [(g[1], g[2]) for g in mine]:
        raise Refused("the two games files do not hold the same games, row for row")
    rows = [g[2] for g in pred]
    diff = [b[3] - a[3] for a, b in zip(pred, mine)]   # part less predecessor: a rise
    n_rows = sum(rows)
    rise = sum(diff) / n_rows
    total = sum(diff)
    leave = [(total - x) / (n_rows - r) for x, r in zip(diff, rows)]
    mean = sum(leave) / len(leave)
    se = math.sqrt((len(leave) - 1) / len(leave) * sum((v - mean) ** 2 for v in leave))
    return rise, se, len(pred), n_rows


def guard_read(args):
    """DEC-357: both vectors' games files read at one K. The deciding reading prints
    `GUARD pass|refused`; with --beside, the reading at K_0 for both, which decides
    nothing and so carries neither word."""
    rise, se, games, rows = guard_pair(args.pred, args.part)
    refused = rise > GUARD_Z * se and rise > 0.0
    text = (f"{args.label}: the tablebase-labelled loss moves {rise:+.7f} +/- {se:.7f} by "
            f"game ({games} games, {rows} rows)")
    if args.beside:
        print(f"beside, deciding nothing: {text}, past +{GUARD_Z} errors "
              f"{'yes' if refused else 'no'}")
    else:
        print(f"GUARD {'refused' if refused else 'pass'} {text}; refused past +{GUARD_Z} "
              "errors")
    return 0


def vector_edit(args):
    """A smoke's probe vectors (DEC-357's consequence): BASE with the scale factors,
    the vector's last thirteen values (tools/tuner_layout.hpp ENDGAME_SCALE_END ==
    PARAM_COUNT), taken from FROM at CELLS, or set to VALUE at CELLS."""
    lines = open(args.base).read().splitlines()
    values = [i for i, x in enumerate(lines) if x and not x.startswith("#")]
    if len(values) < len(CELLS):
        raise Refused(f"{args.base} holds fewer values than the thirteen factors")
    first = values[-len(CELLS)]
    cells = [int(c) for c in args.cells.split(",")] if args.cells != "all" \
        else list(range(len(CELLS)))
    if args.source:
        other = [x for x in open(args.source).read().splitlines()
                 if x and not x.startswith("#")]
        if len(other) != len(values):
            raise Refused(f"{args.source} and {args.base} differ in width")
    for c in cells:
        lines[first + c] = (other[len(other) - len(CELLS) + c] if args.source
                            else str(args.value))
    with open(args.out, "w") as handle:
        handle.write("\n".join(lines) + "\n")
    print(f"vector-edit: {args.out}, cells {cells} "
          + (f"from {args.source}" if args.source else f"at {args.value}"))
    return 0


# --------------------------------------------------------------- reading

def decided_round(directory):
    path = os.path.join(directory, "decided")
    if not os.path.exists(path):
        raise Refused(f"{directory} has no decided round")
    m = DECIDED.match(open(path).read().strip())
    if not m:
        raise Refused(f"{path} is not `rN K value`")
    return int(m.group(1)), float(m.group(2))


def fit_reading(directory, stamp):
    """The decided round's (start, best, K, header) and round 1's, the fit at K at the
    identity; each header's cap and engine checked."""
    r, k = decided_round(directory)
    out = {}
    for label, rr in (("decided", r), ("first", 1)):
        log = os.path.join(directory, f"r{rr}", "fit.log")
        start, best, _k, _improved, _rows, _w = read_fit(log)
        header = open(log[:-len(".log")] + ".hpp").read()
        cap = CAP.search(header)
        engine = ENGINE.search(header)
        if not cap or int(cap.group(1)) != FIT_EPOCHS:
            raise Refused(f"{log}'s header ran at {cap.group(1) if cap else 'no'} epochs, "
                          f"not {FIT_EPOCHS}")
        if not engine or engine.group(1) != stamp:
            raise Refused(f"{log}'s tuner was built at {engine.group(1) if engine else '?'}, "
                          f"not {stamp}")
        stopped = "patience" if "no improvement in" in open(log).read() else "cap"
        out[label] = {"start": start, "best": best, "round": rr, "stopped": stopped,
                      "header": header}
    out["k"] = k
    return out


def read(args):
    part = args.part
    p = plan_of(part, args.out)
    os.makedirs(os.path.join(args.out, "read"), exist_ok=True)
    lines = []

    def say(text=""):
        lines.append(text)
        print(text)

    say(f"S124 screen reading, {part}: cells " + ", ".join(CELLS[c] for c in PART_CELLS[part]))
    say(f"free {p['groups']}, fit {p['name']}, against {p['pred']}")
    problems = []
    gains, first_gains = {}, {}
    try:
        mine = {x: fit_reading(os.path.join(args.out, "fits", p["name"], x), args.stamp)
                for x in SAMPLES}
        pred = ({x: fit_reading(os.path.join(args.out, "fits", p["pred"], x), args.stamp)
                 for x in SAMPLES} if p["pred"] != "landed" else None)
    except Refused as why:
        mine = pred = None
        problems.append(str(why))

    if mine is not None:
        for x in SAMPLES:
            f = mine[x]
            # The landed tree at its own K, the identity's: round 1's start.
            base = f["first"]["start"] if pred is None else pred[x]["decided"]["best"]
            base0 = f["first"]["start"] if pred is None else pred[x]["first"]["best"]
            gains[x] = base - f["decided"]["best"]
            first_gains[x] = base0 - f["first"]["best"]
            say(f"  {x:<5} K {mine[x]['k']:.4f} after {f['decided']['round']} round(s): "
                f"best {f['decided']['best']:.6f} ({f['decided']['stopped']}) against "
                f"{base:.6f}, d {gains[x]:+.7f}; at K identity d {first_gains[x]:+.7f}")
        quarter = [gains[f"q{s}"] for s in range(1, 6)]
        s_dev = statistics.stdev(quarter)
        m = max(PICK_Z * s_dev, FLOOR)
        if s_dev > SPREAD_UNREADABLE:
            problems.append(f"s {s_dev:.6f} above {SPREAD_UNREADABLE}")
        quarter0 = [first_gains[f"q{s}"] for s in range(1, 6)]
        m0 = max(PICK_Z * statistics.stdev(quarter0), FLOOR)
        say(f"rule at K refit: full d {gains['full']:+.7f}, s {s_dev:.7f}, m {m:.7f}, kept iff "
            f"d > m; quarters " + " ".join(f"{g:+.7f}" for g in quarter))
        say(f"beside, at K identity (round 1): full d {first_gains['full']:+.7f}, m {m0:.7f}, "
            f"{'past' if first_gains['full'] > m0 else 'not past'} its m; deciding nothing")
        weights = dict(ARRAY.findall(mine["full"]["decided"]["header"]))
        if "endgame_scale" in weights:
            say("endgame_scale " + " ".join(v.strip() for v in
                                            weights["endgame_scale"].split(",")))

    guard_path = os.path.join(args.out, "fits", p["name"], "guard.txt")
    guard = None
    if os.path.exists(guard_path):
        g = [GUARD.match(x) for x in report_lines(guard_path)]
        g = [x for x in g if x]
        guard = g[-1].group(1) if g else None
        for x in report_lines(guard_path):
            say(x)
    if guard is None:
        problems.append(f"no guard reading at {guard_path}")

    if problems:
        say("unreadable: " + "; ".join(problems))
        say("PART-OUTCOME unreadable")
        status = 1
    else:
        keep = gains["full"] > m and guard == "pass"
        why = ("past its margin, the guard passes" if keep else
               "refused by the tablebase guard" if gains["full"] > m else
               "not past its margin")
        say(f"DECIDED fit {p['name']} pred {p['pred']} d {gains['full']:+.7f} K "
            f"{mine['full']['k']:.4f}")
        say(f"{part} {'kept' if keep else 'dropped'}: {why}")
        if part == PARTS[-1]:
            survivors = p["survivors"] + ([part] if keep else [])
            name = fit_name(survivors) if survivors else "none"
            say("survivors " + (",".join(survivors) or "(none)"))
            say(f"CANDIDATE {name}" + (f" dir {os.path.join(args.out, 'fits', name, 'full')}"
                                       if survivors else ""))
        say(f"PART-OUTCOME {'kept' if keep else 'dropped'}")
        status = 0
    with open(os.path.join(args.out, "read", f"{part}.txt"), "w") as handle:
        handle.write("\n".join(lines) + "\n")
    return status


def landing_check(args):
    weights = dict(ARRAY.findall(open(args.header).read()))
    problems = []
    scale = [int(v) for v in re.findall(r"-?\d+", weights.get("endgame_scale", ""))]
    if scale != [128] * len(CELLS):
        problems.append(f"endgame_scale is {scale or 'absent'}, not 128 x {len(CELLS)}")
    for name in ("mopup_mg", "mopup_eg"):
        values = [int(v) for v in re.findall(r"-?\d+", weights.get(name, ""))]
        if values != [0, 0, 0]:
            problems.append(f"{name} is {values or 'absent'}, not zero")
    if problems:
        raise Refused("not the landing at its identity: " + "; ".join(problems))
    print("landing-check: every scale factor at F and mop-up at zero")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S124's screen helpers.")
    commands = parser.add_subparsers(dest="command", required=True)
    c = commands.add_parser("identity-check")
    c.add_argument("report")
    c = commands.add_parser("ties")
    c.add_argument("report")
    c.add_argument("--out", required=True)
    c = commands.add_parser("plan")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)
    c = commands.add_parser("converged")
    c.add_argument("before")
    c.add_argument("after")
    c = commands.add_parser("guard-read")
    c.add_argument("pred")
    c.add_argument("part")
    c.add_argument("--label", required=True)
    c.add_argument("--beside", action="store_true")
    c = commands.add_parser("vector-edit")
    c.add_argument("base")
    c.add_argument("--cells", required=True)
    c.add_argument("--source", default="")
    c.add_argument("--value", type=int, default=0)
    c.add_argument("--out", required=True)
    c = commands.add_parser("read")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)
    c.add_argument("--stamp", required=True)
    c = commands.add_parser("landing-check")
    c.add_argument("header")
    args = parser.parse_args(argv)
    handler = {"identity-check": identity_check, "ties": ties, "plan": plan,
               "converged": converged, "guard-read": guard_read, "read": read,
               "landing-check": landing_check, "vector-edit": vector_edit}[args.command]
    try:
        return handler(args)
    except Refused as why:
        print(f"refused: {why}")
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
