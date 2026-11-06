#!/usr/bin/env python3
"""S122's screen: what each part frees and against what it is read, the identity
check and the ties from the audit, the margin's fixed point, P0, the reading of
the fits, the side readings, the bisection, the paste checks and the clamp.
adocs/data/S122_screen.sh runs every one of them; each also runs alone. Standard
library only, plus adocs/data/S135_screen.py and tools/corpus_split.py, imported
so the fit logs, headers and corpus rows are parsed the way the screens before it
parsed them.

    S122_screen.py plan PART --out OUT
    S122_screen.py audit-check REPORT
    S122_screen.py ties REPORT --out DIR
    S122_screen.py margin-read CENSUS --round R --margin M [--out FILE]
    S122_screen.py p0-read LOG184 LOGM
    S122_screen.py read PART --out OUT --tree TREE [--cap 80000]
    S122_screen.py start-check HEADER --tree TREE [--free GROUPS] [--allow-five]
    S122_screen.py headroom HEADER --tree TREE [--wide]
    S122_screen.py only-family TREE [--allow-five]
    S122_screen.py landing-check --tree TREE [--commit SHA]
    S122_screen.py zero-five TREE
    S122_screen.py part NAME --out OUT --ruled DEC-nnn --base DIR [--margin M]
    S122_screen.py ruled FIT --dec DEC-nnn --out OUT --base DIR
    S122_screen.py sample-prep SAMPLE --exclude FILE [--exclude FILE ...] --out FILE
    S122_screen.py side-read GAMES_PRED GAMES_PART --label TEXT --full-d D
    S122_screen.py clamp CENSUS --margin M

PART is P1 to P7; the header of S122_screen.sh is the rule's statement and this
file its implementation (DEC-344 (3), DEC-331's form). PLAN prints what a part frees
-- the earlier survivors' groups and its own -- the fit it is read against, the tree
it is fitted on (P6's and P7's are variants), and the tie files its fits read, and
refuses when an earlier reading is missing or unreadable. AUDIT-CHECK applies the
identity rule to a tools/feature_audit report. TIES writes one tie list a linear
part from the same report's support lines; an unsupported half of the curve stops.
MARGIN-READ is DEC-344 (4)'s step on one round's tools/margin_census output: the
fixed point, the next margin, or the stop. P0-READ is the margin alone at the
shipped constants. READ applies a part's rule and writes OUT/read/PART.txt, whose
last line is PART-OUTCOME kept, dropped, skipped, unreadable or escalate: a curve
part not kept on a pair a fit of which stopped on its cap owes DEC-345's re-run of
both sides at 80000 epochs, and `read --cap 80000` then decides on that pair. Every
fit set's directory names its cap, every header says the cap it ran at, and no pair
or reuse crosses caps. START-CHECK compares
a fit's header with TREE's HEAD sources, the free groups' columns excepted. HEADROOM
is src/evaluation.cpp's two asserts for a header's weights: king safety's packed sum
and the curve's x, P6's wider bound with --wide. ONLY-FAMILY refuses a paste that moved any weight but S122's.
LANDING-CHECK refuses a TREE whose HEAD is not the landing: the four linear arrays
at zero and the curve at its seed. ZERO-FIVE writes P7's variant, the shipped linear
five at zero, into TREE's sources. PART prints a bisection part's free set,
drawn since 2026-11-05 (DEC-353) from the survivors of the one decided fit the
ruling given names, and refuses without a ruling. RULED (DEC-353) accepts a fit
for `apply` only if a part's DECIDED line names it and it is the one decided fit
the decision given names, and prints that part's tree. Both read the decision's
`Decision:` field as committed at BASE's HEAD, and both refuse unless every P1
to P7 reading exists and is decided. `--decisions FILE` reads FILE in its place,
for a plant only; S122_screen.sh never passes it.
SAMPLE-PREP drops from the 12000-game sample every row on a position a training or
validation file holds, and refuses a sample that holds the corpus's own games or one
game twice. SIDE-READ pairs two vectors' per-game losses on one set and
applies DEC-344 (1)'s stop. CLAMP is DEC-333 (6)'s reading at the candidate's margin.
"""

import argparse
import collections
import math
import os
import re
import statistics
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "..", "tools"))

import S135_screen  # noqa: E402  (adocs/data/, by path)
import corpus_split  # noqa: E402  (tools/, by path)

from S135_screen import Refused, head_weights, read_fit, weights_of  # noqa: E402

PARTS = [f"P{i}" for i in range(1, 8)]
LINEAR = {"P1": "ks_checks", "P2": "ks_weak", "P3": "ks_defenders",
          "P4": "ks_storm"}
# The curve's input terms a linear part's survival lets P5 free (DEC-344 (3):
# "the counts of P1 to P3's survivors").
CURVE_TERM = {"P1": "ks_curve.checks", "P2": "ks_curve.weak",
              "P3": "ks_curve.defenders"}
CURVE_CORE = "ks_curve.core"
WIDTH = {"ks_checks": 4, "ks_weak": 1, "ks_defenders": 1, "ks_storm": 7,
         "ks_curve": 12}
LINEAR_ARRAYS = ["ks_checks", "ks_weak", "ks_defenders", "ks_storm"]
FAMILY = LINEAR_ARRAYS + ["ks_curve"]
# The curve's columns by term, ks_curve_input_t's order.
TERM_COLUMNS = {"ks_curve.core": range(0, 6), "ks_curve.checks": range(6, 10),
                "ks_curve.weak": range(10, 11),
                "ks_curve.defenders": range(11, 12)}
CURVE_SEED = [45] + [0] * 11
NAMES = {"P1": "safe checks on empty targets by N B R Q, linear",
         "P2": "weak zone squares, linear",
         "P3": "zone squares own knights and bishops attack, linear",
         "P4": "the storm's seven cells in S118's slot, linear",
         "P5": "the curve over the core and the survivors' counts",
         "P6": "P5's attacker and incidence inputs over the zone and its "
               "forward rank, a variant tree",
         "P7": "the shipped linear five at zero, the survivors refit, a "
               "variant tree"}
# DEC-344 (6): on H0 or no verdict, the margin alone first (when it moved), then
# these, survivors only, in order; a part with no survivor does not exist.
BISECTION = {"A": ["P1"], "B": ["P2", "P3", "P4"], "C": ["P5", "P6", "P7"]}

SEEDS = S135_screen.SEEDS
PICK_Z = S135_screen.PICK_Z
FLOOR = S135_screen.FLOOR
SPREAD_UNREADABLE = S135_screen.SPREAD_UNREADABLE

# Pre-registered 2026-11-02, before any fit (the shell script's header).
TIE_GAMES = 1000
EXPECTED_RANK = 1100        # of 1171, DEC-344 (3)
PARAMS = 1171
# S122 closed: the two pins stay its layout's, a tool re-derived at its own tree (DEC-347's form).
DEAD_PAWN_COLUMNS = 32
CURVE_CONSTANT_COLUMNS = ["ks_curve_mg[0]", "ks_curve_eg[0]"]
BY_CONSTRUCTION_COUNT = 37  # S134's 7, S121's 8, S122's 22
CURVE_DUPLICATES = 22
CLAMP_SHARE_BOUND = 1.0     # percent, DEC-333 (6), read at the candidate's M
ROUNDS = 3                  # the margin's fixed point, DEC-344 (4)
HALF_LIMIT = 32767
CURVE_X_LIMIT = 46340
SIDE_Z = 1.96               # DEC-344 (1)'s stop, two-sided 95 %
SHARED_GAMES = 0.01         # the sample's games allowed half on excluded rows
REPEATED_GAMES = 0.001      # the sample's games allowed to repeat another's rows,
                            # 12 of 12000; a shared worker seed repeats about 1000
# DEC-345: every fit's cap; a curve pair dropped on a fit that stopped on it is
# re-run at ESCALATE_EPOCHS, and a fit that stops there too stops the screen.
FIT_EPOCHS = int(os.environ.get("FIT_EPOCHS", "20000"))
ESCALATE_EPOCHS = int(os.environ.get("ESCALATE_EPOCHS", "80000"))
CURVE_PARTS = ("P5", "P6", "P7")
SAMPLES = ["full"] + [f"q{s}" for s in SEEDS]
LINEAR_FIVE = 5             # king_safety's attackers N B R Q and incidences

FREE_ALL = ",".join(FAMILY)
SUPPORT = re.compile(r"^support (" + "|".join(FAMILY) + r")_(mg|eg)\[(\d+)\] "
                     r"rows (\d+) games (\d+)$")
RANK = re.compile(r"^identities: rank over the rationals exactly (\d+) of "
                  r"(\d+), nullity (\d+)")
ZERO = re.compile(r"^identities: (\d+) columns zero on every row:(.*)$")
SPAN_OK = ("every null direction is one Adam never moves, one the tuner "
           "holds, a mobility level the run's frozen columns hold, or a "
           "curve duplicate the freeze or the curve's curvature holds")
SPAN_COUNTS = re.compile(r"and (\d+) curve duplicates \((\d+) held by the "
                         r"freeze, (\d+) null at the seed only\)")
SHARED_X = re.compile(r"^identities: the curve's columns at its shared x, mg "
                      r"(-?\d+) and eg (-?\d+) on every side")
NO_MORE = "identities: no exact dependency beyond the ones listed above"
BY_CONSTRUCTION = re.compile(r"^identities: by construction, (.*): (holds on "
                             r"every row|FAILS)$")
LEVEL_HELD = re.compile(r"^identities: mobility level, .*: held by the freeze ")
EPOCH = re.compile(r"^epoch\s+(\d+)\s.*\*$")
PATIENCE = "no improvement in"
MARGIN_LINE = re.compile(r"^// margin\s+(\d+),")
CAP_LINE = re.compile(r"^// run\s+(\d+) epochs max")
CENSUS_ROW = re.compile(r"^\s*(\d+)\s+(\d+)\s+([0-9.]+)\s+(\d+)\s+([0-9.]+)\s+"
                        r"([0-9.]+)\s+([0-9.]+)\s+([0-9.]+)\s+(pass|fail)$")
CENSUS_PICK = re.compile(r"^pick (\d+|none)")
CENSUS_X = re.compile(r"^\s*x > 0\s+kings ([0-9.]+) % mg, ([0-9.]+) % eg")
VALIDATION_START = re.compile(r"^start: train ([0-9.]+)\s+validation ([0-9.]+)")
# A reading's decided fit and the tree its part was fitted on, as `read` writes
# them, and a backticked token of a decision's entry: DEC-353's form, `ruled`.
DECIDED = re.compile(r"^DECIDED fit (\S+) pred \S+ d \S+ cap \d+$", re.M)
READ_FREE = re.compile(r"^free .*, tree ([^,\s]+), fit [^,\s]+, against ",
                       re.M)
BACKTICKED = re.compile(r"`([^`\n]+)`")


# ------------------------------------------------------------------- plan

def outcome_of(out, part):
    path = os.path.join(out, "read", f"{part}.txt")
    if not os.path.exists(path):
        raise Refused(f"{path} is missing: {part} has not been read")
    text = open(path).read()
    m = re.search(r"^PART-OUTCOME (kept|dropped|skipped|unreadable|escalate)$",
                  text, re.M)
    if not m:
        raise Refused(f"{path} prints no PART-OUTCOME")
    if m.group(1) == "unreadable":
        raise Refused(f"{part} read unreadable; the coordinator's before any "
                      "later part")
    if m.group(1) == "escalate":
        raise Refused(f"{part} is owed its {ESCALATE_EPOCHS}-epoch pair and is "
                      "not decided")
    return m.group(1)


def cap_of(out, part):
    """The cap PART was decided at, its reading's PART-CAP line."""
    text = open(os.path.join(out, "read", f"{part}.txt")).read()
    m = re.search(r"^PART-CAP (\d+)$", text, re.M)
    if not m:
        raise Refused(f"{part}'s reading names no PART-CAP")
    return int(m.group(1))


def fit_dir(out, name, cap):
    """Every fit set's directory names its cap, so two caps never share one."""
    return os.path.join(out, "fits", f"{name}_e{cap}")


def fit_name(parts):
    """A fit is named by the parts it frees or whose tree it is on: `1`,
    `13`, `135`, `1356`, `13567`."""
    return "".join(p[1:] for p in parts)


def survivors_before(out, part):
    k = PARTS.index(part)
    return [p for p in PARTS[:k] if outcome_of(out, p) == "kept"]


def groups_of(parts):
    """The --only list a set of kept parts frees: each linear survivor's group;
    with P5, the curve's core and the P1 to P3 survivors' curve terms. P6 and P7
    free nothing of their own: they are trees, not columns."""
    names = [LINEAR[p] for p in parts if p in LINEAR]
    if "P5" in parts:
        names.append(CURVE_CORE)
        names += [CURVE_TERM[p] for p in ("P1", "P2", "P3") if p in parts]
    return ",".join(names)


def tree_of(parts):
    """Which tree a fit runs on: the landing, P6's wide-zone variant, P7's
    zeroed linear five, or both."""
    return "+".join(t for t, p in (("p6", "P6"), ("p7", "P7"))
                    if p in parts) or "landing"


def ties_of(parts):
    return " ".join(f"ties_{p}.txt" for p in parts if p in LINEAR)


def plan_of(part, out):
    """{free, groups, fit, pred, ties, tree, cap, dirs} for PART, or {skip} when
    P6 or P7 has no P5 to run over (DEC-344 (3): "only if P5 survives"). A part
    pairs with its predecessor's decided fit, so it is fitted at that fit's cap,
    and at FIT_EPOCHS at least (DEC-345); `esc` is the pair at ESCALATE_EPOCHS,
    the predecessor refitted there with its own free set and tree."""
    kept = survivors_before(out, part)
    if part in ("P6", "P7") and "P5" not in kept:
        return {"skip": "P5 did not survive, so there is no curve to vary"}
    free = kept + [part]
    pred = fit_name(kept) if kept else None
    pred_cap = cap_of(out, kept[-1]) if kept else None
    cap = max(FIT_EPOCHS, pred_cap or 0)
    return {"free": free, "groups": groups_of(free), "fit": fit_name(free),
            "ties": ties_of(free), "tree": tree_of(free), "cap": cap,
            "dir": fit_dir(out, fit_name(free), cap),
            "pred": pred, "pred_cap": pred_cap,
            "pred_dir": fit_dir(out, pred, pred_cap) if pred else None,
            "pred_groups": groups_of(kept), "pred_ties": ties_of(kept),
            "pred_tree": tree_of(kept),
            "esc_dir": fit_dir(out, fit_name(free), ESCALATE_EPOCHS),
            "esc_pred_dir": (fit_dir(out, pred, ESCALATE_EPOCHS) if pred
                             else None)}


def plan(args):
    p = plan_of(args.part, args.out)
    print(f"part {args.part}: {NAMES[args.part]}")
    if "skip" in p:
        print(f"skip {p['skip']}")
        return 0
    print(f"groups {p['groups']}")
    print(f"fit {p['fit']}")
    print(f"ties {p['ties']}")
    print(f"tree {p['tree']}")
    print(f"cap {p['cap']}")
    print(f"dir {p['dir']}")
    print(f"escdir {p['esc_dir']}")
    if p["pred"] is not None:
        print(f"pred {p['pred']}")
        print(f"predcap {p['pred_cap']}")
        print(f"preddir {p['pred_dir']}")
        print(f"escpreddir {p['esc_pred_dir']}")
        print(f"predgroups {p['pred_groups']}")
        print(f"predties {p['pred_ties']}")
        print(f"predtree {p['pred_tree']}")
    return 0


# ------------------------------------------------------------- the audit

def support_of(report):
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
    pawn_zero = [z for z in zero
                 if re.fullmatch(r"psqt_(mg|eg)\[pawn\]\[[a-h][18]\]", z)]
    constant_zero = [z for z in zero if z in CURVE_CONSTANT_COLUMNS]
    other = [z for z in zero if z not in pawn_zero + constant_zero]
    if len(pawn_zero) != DEAD_PAWN_COLUMNS:
        problems.append(f"{len(pawn_zero)} dead pawn columns, not "
                        f"{DEAD_PAWN_COLUMNS}")
    if sorted(constant_zero) != sorted(CURVE_CONSTANT_COLUMNS):
        problems.append("the curve's constant columns are not both zero: "
                        "the landing is not at its seed")
    if other:
        problems.append("zero columns beyond the dead pawn squares and the "
                        "curve's constant, the coordinator's: " +
                        " ".join(other))
    if not any(SHARED_X.match(line) for line in lines):
        problems.append("the report did not take the curve's columns at a "
                        "shared x")
    rank = [m for m in (RANK.match(line) for line in lines) if m]
    if not rank:
        problems.append("the rank over the rationals is not exact")
    else:
        got, of, _ = (int(x) for x in rank[0].groups())
        if of != PARAMS or got != EXPECTED_RANK:
            problems.append(f"rank {got} of {of}, the rule wants "
                            f"{EXPECTED_RANK} of {PARAMS}")
    constructed = [m for m in (BY_CONSTRUCTION.match(line) for line in lines)
                   if m]
    levels = [m for m in constructed if "(S121)" in m.group(1)]
    curve = [m for m in constructed if "(S122)" in m.group(1)]
    if (len(constructed) != BY_CONSTRUCTION_COUNT or len(levels) != 8 or
            len(curve) != CURVE_DUPLICATES):
        problems.append(f"{len(constructed)} dependencies by construction, "
                        f"{len(levels)} S121's and {len(curve)} S122's: the "
                        f"rule wants {BY_CONSTRUCTION_COUNT}, 8 and "
                        f"{CURVE_DUPLICATES}")
    failed = [m.group(1) for m in constructed if m.group(2) == "FAILS"]
    if failed:
        problems.append("a dependency by construction fails: " +
                        "; ".join(failed))
    if NO_MORE not in lines:
        problems.append("an exact dependency beyond the listed ones")
    if SPAN_OK not in text:
        problems.append("a null direction nothing listed accounts for")
    held = [line for line in lines if LEVEL_HELD.match(line)]
    if len(held) != 8:
        problems.append(f"{len(held)} mobility levels held by the freeze, "
                        "not 8")
    span = [m for m in (SPAN_COUNTS.search(line) for line in lines) if m]
    if not span or int(span[0].group(1)) != CURVE_DUPLICATES:
        problems.append("the span line does not count the curve's "
                        f"{CURVE_DUPLICATES} duplicates")
    print(f"audit-check {args.report}: zero columns {len(pawn_zero)} dead "
          f"pawn and {len(constant_zero)} curve constant")
    print(f"audit-check: the rule's rank {EXPECTED_RANK} of {PARAMS}, "
          f"nullity {PARAMS - EXPECTED_RANK}: the 34 zero columns, S134's 7, "
          f"S121's 8 and the curve's {CURVE_DUPLICATES} duplicates at its seed")
    if span:
        print(f"audit-check: curve duplicates {span[0].group(1)}, "
              f"{span[0].group(2)} held by the freeze, {span[0].group(3)} null "
              "at the seed only")
    if problems:
        raise Refused("the identity rule fails: " + "; ".join(problems))
    print("audit-check: pass")
    return 0


def ties(args):
    """One tie list a linear part, DIR/ties_P1.txt to ties_P4.txt; the curve is
    checked and never tied."""
    support = support_of(args.report)
    os.makedirs(args.out, exist_ok=True)
    print(f"ties: threshold {TIE_GAMES} games a half")
    for half in ("mg", "eg"):
        for c, (rows, games) in enumerate(support[("ks_curve", half)]):
            if games < TIE_GAMES:
                raise Refused(f"ks_curve_{half}[{c}] stands on {games} games, "
                              f"under {TIE_GAMES}: the curve has no tie row, "
                              "the coordinator's")
    total = 0
    for part in ("P1", "P2", "P3", "P4"):
        array = LINEAR[part]
        lines = [f"# S122 ties, part {part} ({array}), from {args.report}",
                 f"# a half of a cell is supported from {TIE_GAMES} games; an "
                 "unsupported one shares the nearest supported cell of its "
                 "array and half, the lower cell on a tie (DEC-331 (6))"]
        for half in ("mg", "eg"):
            row = support[(array, half)]
            supported = [c for c, (_, games) in enumerate(row)
                         if games >= TIE_GAMES]
            if not supported:
                raise Refused(f"no cell of {array}_{half} reaches {TIE_GAMES} "
                              "games: nothing to tie it to, the coordinator's")
            for c, (rows, games) in enumerate(row):
                if games >= TIE_GAMES:
                    continue
                anchor = min(supported, key=lambda a: (abs(a - c), a))
                lines.append(f"{array}_{half} {c} {anchor}  # rows {rows} "
                             f"games {games}")
                print(f"  tie {array}_{half}[{c}] -> [{anchor}]: rows {rows}, "
                      f"games {games}")
                total += 1
        with open(os.path.join(args.out, f"ties_{part}.txt"), "w") as handle:
            handle.write("\n".join(lines) + "\n")
    cells = 2 * sum(WIDTH[a] for a in LINEAR_ARRAYS)
    print(f"ties: {total} of {cells} linear cell halves tied; every curve "
          f"input half on {TIE_GAMES} games or more")
    return 0


# ------------------------------------------------------------- the margin

def census_of(path):
    """{margin: (bound%, mass%, pass)}, the pick, and the x > 0 shares."""
    rows = {}
    pick = None
    x = None
    for line in open(path, encoding="utf-8", errors="replace"):
        m = CENSUS_ROW.match(line)
        if m:
            rows[int(m.group(1))] = (float(m.group(3)), float(m.group(6)),
                                     m.group(9) == "pass")
        m = CENSUS_PICK.match(line)
        if m:
            pick = None if m.group(1) == "none" else int(m.group(1))
        m = CENSUS_X.match(line)
        if m:
            x = (float(m.group(1)), float(m.group(2)))
    if not rows:
        raise Refused(f"{path} prints no census rows")
    return rows, pick, x


def margin_read(args):
    """DEC-344 (4), one round: the fit at margin M's census. M passes: the fixed
    point. Otherwise the pick is the next margin, unless the rounds are spent."""
    rows, pick, _ = census_of(args.census)
    if args.margin not in rows:
        raise Refused(f"{args.census} prints no row at {args.margin}; run it "
                      f"with --also {args.margin}")
    bound, mass, ok = rows[args.margin]
    print(f"round {args.round} at {args.margin}: rows bound {bound:.3f} %, "
          f"graded mass bound {mass:.2f} %, {'pass' if ok else 'fail'}")
    if ok and (args.round == 1 or pick == args.margin):
        verdict = f"MARGIN {args.margin}"
    elif ok:
        # It passes at its own margin, but a smaller multiple of 25 would too:
        # the fixed point is the pick, one more round.
        verdict = (f"NEXT {pick}" if args.round < ROUNDS
                   else f"MARGIN-STOP after {ROUNDS} rounds: {args.margin} "
                        f"passes, the pick {pick} differs")
    elif pick is None:
        verdict = "MARGIN-STOP: no multiple of 25 up to 2000 passes"
    elif args.round >= ROUNDS:
        verdict = (f"MARGIN-STOP after {ROUNDS} rounds: {args.margin} fails, "
                   f"the pick is {pick}")
    else:
        verdict = f"NEXT {pick}"
    print(verdict)
    if args.out:
        with open(args.out, "a") as handle:
            handle.write(f"round {args.round} margin {args.margin} bound "
                         f"{bound:.3f} mass {mass:.2f} pick {pick}: "
                         f"{verdict}\n")
    return 0


def p0_read(args):
    """The margin alone at the shipped constants: the fixed set's error at 184
    less at M, one K for both, no fit (DEC-344 (3), P0)."""
    def validation(path):
        for line in open(path, encoding="utf-8", errors="replace"):
            m = VALIDATION_START.match(line)
            if m:
                return float(m.group(2))
        raise Refused(f"{path} prints no start: line")
    at_184 = validation(args.log184)
    at_m = validation(args.logm)
    print(f"P0: validation {at_184:.6f} at the shipped margin, {at_m:.6f} at "
          f"the candidate's; d0 = {at_184 - at_m:+.6f} (a reading; every later "
          "d is paired against the fits' own starts at the candidate's margin)")
    return 0


# ------------------------------------------------------------ the reading

def fit_of(directory, sample, cap):
    """read_fit over DIR/SAMPLE.log, how it stopped, and the cap and margin its
    header says it ran at: a fit at another cap than CAP is refused, so no pair
    and no reuse crosses caps (S122's fast check, finding 6)."""
    log = os.path.join(directory, f"{sample}.log")
    start, best, _k, _improved, _rows, _w = read_fit(log)
    best_epoch = 0
    patience = False
    margin = ran = None
    for line in open(log, encoding="utf-8", errors="replace"):
        m = EPOCH.match(line)
        if m:
            best_epoch = int(m.group(1))
        if line.startswith(PATIENCE):
            patience = True
    header = log[:-len(".log")] + ".hpp"
    if not os.path.exists(header):
        raise Refused(f"{header} is missing")
    for line in open(header):
        m = MARGIN_LINE.match(line)
        if m:
            margin = int(m.group(1))
        m = CAP_LINE.match(line)
        if m:
            ran = int(m.group(1))
    if ran is None:
        raise Refused(f"{header} names no epoch cap")
    if ran != cap:
        raise Refused(f"{header} ran at {ran} epochs, not the pair's {cap}")
    return {"start": start, "best": best, "best_epoch": best_epoch,
            "stop": "patience" if patience else f"cap {ran}",
            "on_cap": not patience, "margin": margin}


def read(args):
    part = args.part
    p = plan_of(part, args.out)
    os.makedirs(os.path.join(args.out, "read"), exist_ok=True)
    lines = []

    def say(text=""):
        lines.append(text)
        print(text)

    say(f"S122 screen reading, {part}: {NAMES[part]}")
    if "skip" in p:
        say(f"{part} skipped: {p['skip']}")
        if part == "P7":
            # A skipped P7 is no survivor: its own reading is the one being
            # written, so it is not read back.
            candidate_lines(args.out, say, last=False, last_cap=None)
        say("PART-OUTCOME skipped")
        with open(os.path.join(args.out, "read", f"{part}.txt"), "w") as h:
            h.write("\n".join(lines) + "\n")
        return 0

    margin = margin_value(args.out)
    escalated = args.cap is not None
    if escalated and args.cap != ESCALATE_EPOCHS:
        raise Refused(f"--cap is {ESCALATE_EPOCHS} or absent")
    problems = []
    say(f"free {p['groups']}, tree {p['tree']}, fit {p['fit']}"
        + (f", against {p['pred']}" if p["pred"] else ", against its start"))

    def pair(cap, directory, pred_directory, label):
        """The pair at one cap: its lines said, its gains, whether a fit of
        either side stopped on the cap, or None when a fit is unreadable."""
        try:
            mine = {x: fit_of(directory, x, cap) for x in SAMPLES}
            pred = ({x: fit_of(pred_directory, x, cap) for x in SAMPLES}
                    if pred_directory else None)
        except Refused as why:
            problems.append(f"{label}: {why}")
            return None
        say(f"pair at {cap} epochs ({label}): {directory} against "
            f"{pred_directory or 'its start'}")
        for name, fits in ((p["fit"], mine), (p["pred"], pred)):
            if fits is None:
                continue
            for x in SAMPLES:
                f = fits[x]
                if f["margin"] is not None and f["margin"] != margin:
                    problems.append(f"{name}/{x} was fitted at margin "
                                    f"{f['margin']}, not the screen's {margin}")
                say(f"  {name:<9} {x:<5} start {f['start']:.6f} best "
                    f"{f['best']:.6f} at epoch {f['best_epoch']}, stopped on "
                    f"{f['stop']}")
        if pred is None:
            gains = {x: mine[x]["start"] - mine[x]["best"] for x in SAMPLES}
            rule = "d = start - best: P0 at the candidate's margin"
        else:
            gains = {x: pred[x]["best"] - mine[x]["best"] for x in SAMPLES}
            rule = f"d = best({p['pred']}) - best({p['fit']})"
        quarter = [gains[f"q{x}"] for x in SEEDS]
        s_dev = statistics.stdev(quarter)
        m = max(PICK_Z * s_dev, FLOOR)
        if s_dev > SPREAD_UNREADABLE:
            problems.append(f"{label}: s {s_dev:.6f} above {SPREAD_UNREADABLE}")
        # P7 retires the shipped five: kept unless it costs more than m.
        keep = (gains["full"] >= -m) if part == "P7" else (gains["full"] > m)
        on_cap = any(f["on_cap"] for fits in (mine, pred) if fits
                     for f in fits.values())
        say(f"rule {rule}: full {gains['full']:+.6f}, s {s_dev:.6f}, m "
            f"{m:.6f}, kept iff d {'>= -m' if part == 'P7' else '> m'}; "
            "quarters " + " ".join(f"{g:+.6f}" for g in quarter))
        return {"keep": keep, "d": gains["full"], "on_cap": on_cap,
                "dir": directory, "pred_dir": pred_directory}

    base = pair(p["cap"], p["dir"], p["pred_dir"], "the screen's")
    final = base
    final_cap = p["cap"]
    escalate = False
    if base is not None and part in CURVE_PARTS and not base["keep"] and \
            base["on_cap"]:
        if p["cap"] >= ESCALATE_EPOCHS:
            problems.append(f"STOP: a fit of the pair stopped on the "
                            f"{p['cap']}-epoch cap and the part is not kept: "
                            "the coordinator's (DEC-345)")
        elif not escalated:
            escalate = True
            say(f"ESCALATE {ESCALATE_EPOCHS}: {part} not kept and a fit of its "
                f"pair stopped on the {p['cap']}-epoch cap (DEC-345)")
        else:
            final = pair(ESCALATE_EPOCHS, p["esc_dir"], p["esc_pred_dir"],
                         "DEC-345's re-run")
            final_cap = ESCALATE_EPOCHS
            if final is not None and final["on_cap"]:
                problems.append(f"STOP: a fit of the {ESCALATE_EPOCHS}-epoch "
                                "pair stopped on its cap: the coordinator's "
                                "(DEC-345)")
    elif escalated:
        problems.append(f"--cap {ESCALATE_EPOCHS} asked of a pair that owes no "
                        "re-run")

    keep = bool(final and final["keep"])
    if final is not None and not escalate:
        header = os.path.join(final["dir"], "full.hpp")
        bounds = headroom_of(header, args.tree, wide="p6" in p["tree"])
        say("headroom: " + ", ".join(f"{k} {v}" for k, v in bounds.items()))
        if (max(bounds["safety mg"], bounds["safety eg"]) > HALF_LIMIT or
                max(bounds["curve mg"], bounds["curve eg"]) > CURVE_X_LIMIT):
            problems.append("the full fit passes an assert: the coordinator's "
                            "(DEC-084)")
        for name, values in sorted(weights_of(open(header).read()).items()):
            if re.match(r"^(" + "|".join(FAMILY) + r")_(mg|eg)$", name):
                say(f"  {name} {values}")
        if part in CURVE_PARTS:
            # A reading and no rule (DEC-345): every group fit starts at the
            # seed, where every king's x is the constant and positive.
            census = os.path.join(final["dir"], "census_end.txt")
            x = census_of(census)[2] if os.path.exists(census) else None
            if x is None:
                problems.append(f"{census} gives no x > 0 share")
            else:
                say(f"x > 0 at the fit's end, a reading: {x[0]:.2f} % mg, "
                    f"{x[1]:.2f} % eg of kings")

    if problems:
        say("unreadable: " + "; ".join(problems))
        say("PART-OUTCOME unreadable")
        status = 1
    elif escalate:
        say("PART-OUTCOME escalate")
        status = 0
    else:
        say(f"DECIDED fit {final['dir']} pred {final['pred_dir'] or 'start'} "
            f"d {final['d']:+.7f} cap {final_cap}")
        say(f"{part} {'kept' if keep else 'dropped'}")
        say(f"PART-CAP {final_cap}")
        if part == "P7":
            candidate_lines(args.out, say, last=keep, last_cap=final_cap)
        say(f"PART-OUTCOME {'kept' if keep else 'dropped'}")
        status = 0
    with open(os.path.join(args.out, "read", f"{part}.txt"), "w") as handle:
        handle.write("\n".join(lines) + "\n")
    return status


def margin_value(out):
    """M from OUT/margin/M.txt, which names the cap it was found at too; a margin
    found at another cap than FIT_EPOCHS is refused."""
    text = open(os.path.join(out, "margin", "M.txt")).read().split()
    if len(text) < 4 or text[0] != "MARGIN" or text[2] != "epochs":
        raise Refused(f"{out}/margin/M.txt is not `MARGIN M epochs E`")
    if int(text[3]) != FIT_EPOCHS:
        raise Refused(f"the margin was found at {text[3]} epochs, not "
                      f"FIT_EPOCHS {FIT_EPOCHS}")
    return int(text[1])


def survivors_of(out, last=None):
    kept = [p for p in PARTS[:-1] if outcome_of(out, p) == "kept"]
    seventh = (last if last is not None
               else outcome_of(out, "P7") == "kept")
    return kept + (["P7"] if seventh else [])


def bisection_parts(survivors):
    out = []
    for name in ("A", "B", "C"):
        parts = [p for p in BISECTION[name] if p in survivors]
        if parts:
            out.append((name, parts))
    return out


def candidate_lines(out, say, last, last_cap):
    survivors = survivors_of(out, last)
    margin = margin_value(out)
    say("survivors " + (",".join(survivors) or "(none)"))
    if not survivors:
        say("candidate (none): no part lowers held-out loss past its margin; "
            "each is recorded with its figures, the family stays at its seed, "
            "and the margin alone is DEC-344 (6)'s first part if it moved")
        say("CANDIDATE none")
        return
    name = fit_name(survivors)
    # The last survivor's decided fit, at the cap its pair was decided at.
    cap = last_cap if survivors[-1] == "P7" else cap_of(out, survivors[-1])
    directory = fit_dir(out, name, cap)
    say(f"candidate {name} at margin {margin}, {cap} epochs: "
        f"{os.path.join(directory, 'full.hpp')}")
    first = [] if margin == 184 else [f"M {margin} alone {{-5, 0}}"]
    say("bisection on H0 or no verdict, in order: " + " | ".join(
        first + [f"{n} = {','.join(parts)}" for n, parts in
                 bisection_parts(survivors)]))
    say(f"CANDIDATE {name} groups {groups_of(survivors)} tree "
        f"{tree_of(survivors)} margin {margin} dir {directory}")


# ----------------------------------------------------- headroom and checks

def headroom_of(header, tree, wide=False):
    """src/evaluation.cpp's two asserts for the header's weights: king safety's
    packed sum per half, and the curve's x per half. On P6's tree a king's zone
    and its forward rank are up to 12 squares, so the incidences reach 15 x 12
    = 180 and not 135 (S122_p6_wide_zone.patch moves the engine's bound the
    same way; S122's fast check, finding 8)."""
    weights = head_weights(tree)
    weights.update(weights_of(open(header).read()))
    for a in FAMILY:
        for half in ("mg", "eg"):
            if len(weights.get(f"{a}_{half}", [])) != WIDTH[a]:
                raise Refused(f"{a}_{half} is not {WIDTH[a]} wide")
    largest = max(abs(v) for half in ("mg", "eg")
                  for v in weights[f"king_safety_{half}"])

    def family_half(half):
        c = weights[f"ks_checks_{half}"]
        return (8 * abs(c[0]) + 13 * abs(c[1]) + 14 * abs(c[2]) +
                27 * abs(c[3]) + 9 * abs(weights[f"ks_weak_{half}"][0]) +
                9 * abs(weights[f"ks_defenders_{half}"][0]) +
                3 * max(abs(v) for v in weights[f"ks_storm_{half}"]))

    most = [1, 10, 10, 10, 9, 180 if wide else 135, 8, 13, 14, 27, 9, 9]
    out = {}
    packed = 2 * (16 * (1 + 9) + 4 * 8) * largest + 2 * max(
        family_half("mg"), family_half("eg"))
    out["safety mg"] = out["safety eg"] = packed
    for half in ("mg", "eg"):
        out[f"curve {half}"] = sum(m * abs(w) for m, w in
                                   zip(most, weights[f"ks_curve_{half}"]))
    return out


def headroom(args):
    bounds = headroom_of(args.header, args.tree, wide=args.wide)
    print(f"king safety's packed sum: {bounds['safety mg']} of {HALF_LIMIT}")
    print(f"the curve's x: mg {bounds['curve mg']}, eg {bounds['curve eg']} "
          f"of {CURVE_X_LIMIT}")
    if (bounds["safety mg"] > HALF_LIMIT or
            max(bounds["curve mg"], bounds["curve eg"]) > CURVE_X_LIMIT):
        raise Refused(f"{args.header} would fail an assert; the weights are "
                      "not clipped (DEC-084)")
    return 0


def free_columns(groups):
    """{array: set of free columns} for an --only list."""
    out = {}
    for name in [g for g in (groups or "").split(",") if g]:
        if name in LINEAR.values() or name == "ks_curve":
            out.setdefault(name, set()).update(range(WIDTH[name]))
        elif name in TERM_COLUMNS:
            out.setdefault("ks_curve", set()).update(TERM_COLUMNS[name])
        else:
            raise Refused(f"unknown group {name}")
    return out


def start_check(args):
    emitted = weights_of(open(args.header).read())
    shipped = head_weights(args.tree)
    free = free_columns(args.free)
    if not emitted:
        raise Refused(f"{args.header} emits nothing")
    bad = []
    for name, values in sorted(emitted.items()):
        if name not in shipped:
            bad.append(f"{name}: not in HEAD's sources")
            continue
        if len(values) != len(shipped[name]):
            bad.append(f"{name}: {len(values)} values, {len(shipped[name])} at "
                       "HEAD")
            continue
        array = re.sub(r"_(mg|eg)$", "", name)
        columns = free.get(array, set())
        for c, (a, b) in enumerate(zip(values, shipped[name])):
            # P7's variant starts from king safety's first five at zero.
            if (args.allow_five and array == "king_safety" and
                    c < LINEAR_FIVE):
                b = 0
            if c not in columns and a != b:
                bad.append(f"{name}[{c}]: {a}, HEAD's {b}")
    missing = [f"{a}_{h}" for a in FAMILY for h in ("mg", "eg")
               if f"{a}_{h}" not in emitted]
    if missing:
        bad.append("family arrays missing: " + ", ".join(missing))
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short",
                           "HEAD"], check=True, capture_output=True,
                          text=True).stdout.strip()
    print(f"start-check {args.header}: {len(emitted)} definitions against HEAD "
          f"{head}, free {sorted((a, sorted(c)) for a, c in free.items())}")
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
    allowed = re.compile(r"^(" + "|".join(FAMILY) + r")_(mg|eg)$")
    # P7's variant moves king safety's first five as well, to zero.
    five = [n for n in moved if n in ("king_safety_mg", "king_safety_eg")
            and args.allow_five and all(v == 0 for v in
                                        after[n][:LINEAR_FIVE])]
    others = [n for n in moved if not allowed.match(n) and n not in five]
    print("moved: " + (", ".join(moved) or "(nothing)"))
    if others:
        raise Refused("the paste moved more than S122's arrays: " +
                      ", ".join(others))
    return 0


def commit_weights(tree, commit):
    out = {}
    for path in ("src/eval_tables.hpp", "src/evaluation.cpp"):
        text = subprocess.run(["git", "-C", tree, "show", f"{commit}:{path}"],
                              check=True, capture_output=True,
                              text=True).stdout
        out.update(weights_of(text))
    return out


def landing_check(args):
    commit = args.commit or "HEAD"
    shipped = commit_weights(args.tree, commit)
    bad = []
    for a in FAMILY:
        for half in ("mg", "eg"):
            name = f"{a}_{half}"
            values = shipped.get(name)
            want = CURVE_SEED if a == "ks_curve" else [0] * WIDTH[a]
            if values is None:
                bad.append(f"{name}: not in {commit}'s sources")
            elif values != want:
                bad.append(f"{name}: {values}, not {want}")
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short",
                           commit], check=True, capture_output=True,
                          text=True).stdout.strip()
    if bad:
        raise Refused(f"{commit} {head} is not the landing: " + "; ".join(bad))
    print(f"landing-check: {commit} {head}'s linear arrays are zero and the "
          "curve is at its seed")
    return 0


def zero_five(args):
    """P7's variant: king_safety_mg and _eg's first five entries, the attackers
    N B R Q and the incidences, set to zero in TREE's src/evaluation.cpp."""
    path = os.path.join(args.tree, "src", "evaluation.cpp")
    text = open(path).read()
    for half in ("mg", "eg"):
        pattern = re.compile(r"(constexpr int king_safety_" + half +
                             r"\[KS_FEATURE_COUNT\] = \{)(.*?)(\};)", re.S)
        m = pattern.search(text)
        if not m:
            raise Refused(f"no king_safety_{half} in {path}")
        values = [int(v) for v in re.findall(r"-?\d+", m.group(2))]
        if len(values) != 9:
            raise Refused(f"king_safety_{half} is {len(values)} wide")
        values[:LINEAR_FIVE] = [0] * LINEAR_FIVE
        text = (text[:m.start()] + m.group(1) + ", ".join(map(str, values)) +
                m.group(3) + text[m.end():])
    with open(path, "w") as handle:
        handle.write(text)
    print(f"zero-five: king_safety's attackers and incidences at zero in {path}")
    return 0


def part(args):
    if args.name not in BISECTION:
        raise Refused(f"unknown part {args.name}; parts are A, B, C")
    # 2026-11-05, DEC-353: the screen's own survivor list reads P7 as kept, and
    # the ruling drops it, so a part is drawn only from the survivors of the
    # decided fit a ruling names.
    if not args.ruled:
        raise Refused("the screen's survivor list is overruled by DEC-353; a "
                      "bisection part's survivors are those of the decided "
                      "fit a ruling names: part NAME --ruled DEC")
    if not args.base:
        raise Refused("--ruled needs --base, the main checkout")
    survivors, token = ruled_survivors(args.out, args.ruled, args.base,
                                       args.decisions)
    print(f"ruled {args.ruled} names `{token}`: survivors "
          f"{','.join(survivors)}")
    parts = dict(bisection_parts(survivors)).get(args.name)
    if not parts:
        raise Refused(f"part {args.name} holds no survivor of the screen")
    groups = groups_of(parts)
    # The curve the screen kept reads the P1 to P3 survivors' counts, wherever
    # their own linear cells land: a part holding P5 frees those curve terms
    # too, the model the screen decided on (S122's fast check, finding 4).
    if "P5" in parts:
        groups = ",".join([g for g in groups.split(",") if g] +
                          [CURVE_TERM[q] for q in ("P1", "P2", "P3")
                           if q in survivors])
    print(f"part {args.name}: {', '.join(NAMES[p] for p in parts)}")
    print(f"groups {groups or '(none: a tree only)'}")
    print(f"tree {tree_of(parts)}")
    print(f"wide {'yes' if 'P6' in parts else 'no'}")
    # The margin names the fit: a refit at 184 on DEC-344 (6)'s word and one at
    # M never share a directory, a K or a reuse.
    margin = args.margin if args.margin is not None else margin_value(args.out)
    print(f"margin {margin}")
    print(f"fit part_{args.name}_m{margin}_e{FIT_EPOCHS}")
    print(f"ties {ties_of(parts)}")
    return 0


def decided_fits(out):
    """Every DECIDED line under OUT/read, in part order: the part, its
    reading's text, the line, and the fit's full.hpp by resolved path. Every
    P1 to P7 reading must exist and be decided first, as `outcome_of` has it:
    none missing, `unreadable` or `escalate`."""
    for p in PARTS:
        outcome_of(out, p)
    fits = []
    for p in PARTS:
        text = open(os.path.join(out, "read", f"{p}.txt")).read()
        for m in DECIDED.finditer(text):
            fits.append((p, text, m, os.path.realpath(
                os.path.join(m.group(1), "full.hpp"))))
    return fits


def decisions_text(base, plant):
    """adocs/decisions.md as committed at BASE's HEAD, so an uncommitted entry
    authorizes nothing; PLANT, a file, only for a plant (`--decisions`, which
    S122_screen.sh never passes)."""
    if plant:
        print(f"decisions: {plant}, a plant, not HEAD's")
        return open(plant, encoding="utf-8").read(), plant
    text = subprocess.run(["git", "-C", base, "show",
                           "HEAD:adocs/decisions.md"], check=True,
                          capture_output=True, text=True).stdout
    return text, f"HEAD:adocs/decisions.md in {base}"


def decision_names(dec, base, plant):
    """The files DEC's `Decision:` field names by a backticked path read from
    BASE, the main checkout: {resolved path: the path as written}. Context,
    Rejected and the other fields name nothing: they may cite a fit the
    decision turns down."""
    if not re.match(r"^DEC-\d+$", dec):
        raise Refused(f"{dec} is not a decision id, DEC-nnn")
    text, source = decisions_text(base, plant)
    heading = re.compile(rf"^## {re.escape(dec)}\b")
    seen = inside = on = False
    field = []
    for line in text.splitlines():
        if line.startswith("## "):
            inside = bool(heading.match(line))
            seen = seen or inside
            on = False
            continue
        if not inside:
            continue
        # A field starts at column 0; its continuation lines are indented.
        if line[:1] not in ("", " ", "\t"):
            on = line.startswith("Decision:")
        if on:
            field.append(line)
    if not seen:
        raise Refused(f"{source} holds no {dec}")
    if not field:
        raise Refused(f"{dec} in {source} has no Decision: field")
    names = {}
    for token in BACKTICKED.findall("\n".join(field)):
        names.setdefault(os.path.realpath(os.path.join(base, token)), token)
    return names


def ruled_fit(out, dec, base, plant):
    """The one decided fit DEC's Decision field names, as decided_fits gives
    it, and the path DEC writes it by. None, or more than one, is refused."""
    names = decision_names(dec, base, plant)
    named = [f for f in decided_fits(out) if f[3] in names]
    if not named:
        raise Refused(f"{dec} names no part's decided fit under {out}/read")
    if len(named) > 1:
        raise Refused(f"{dec} names {len(named)} decided fits; a ruling names "
                      "one")
    return named[0], names[named[0][3]]


def ruled_survivors(out, dec, base, plant):
    """DEC-353's survivors: the parts of the one decided fit DEC names, read
    from its fit set's directory, NAME_eCAP (`1245_e20000` is P1, P2, P4, P5)."""
    (_p, _text, _m, fit), token = ruled_fit(out, dec, base, plant)
    m = re.match(r"^([1-7]+)_e\d+$", os.path.basename(os.path.dirname(fit)))
    if not m:
        raise Refused(f"{fit} is not in a fit set's directory, NAME_eCAP")
    return [f"P{d}" for d in m.group(1)], token


def ruled(args):
    """DEC-353's form: the fit a decision names, in place of the screen's
    CANDIDATE line. FIT must be the full.hpp of a fit some part's DECIDED line
    names, and the one decided fit DEC's Decision field names, by a backticked
    path read from BASE, the main checkout. Files are compared by resolved
    path, so a link to the screen's records names the records themselves.
    Prints the tree that part's reading names; `apply` holds FIT's tree.txt to
    it."""
    fit = os.path.realpath(args.fit)
    if not os.path.isfile(fit):
        raise Refused(f"no fit {args.fit}")
    found = [f for f in decided_fits(args.out) if f[3] == fit]
    if not found:
        raise Refused(f"{args.fit} is no part's decided fit: no DECIDED line "
                      f"under {args.out}/read names it")
    (p, text, decided, ruled_one), token = ruled_fit(args.out, args.dec,
                                                     args.base, args.decisions)
    if ruled_one != fit:
        raise Refused(f"{args.dec} names {ruled_one}, not {fit}: a ruled paste "
                      "is the one fit its decision names")
    free = READ_FREE.search(text)
    if free is None:
        raise Refused(f"{p}'s reading names no tree")
    print(f"ruled {args.dec} names `{token}`")
    print(f"decided {p}: {decided.group(0)}")
    print(f"part {p}")
    print(f"name {os.path.basename(os.path.dirname(fit))}")
    print(f"tree {free.group(1)}")
    print(f"fit {fit}")
    return 0


# ---------------------------------------------------------- side readings

def sample_prep(args):
    """The 12000-game sample without a row on any position the excluded files
    hold: a reading on rows the fit trained on, or the fixed set holds, would
    read recall. A game is a run of one id at a rising ply, tools/margin_census's.
    Refused, the coordinator's (S122's fast check follow-up): a sample game more
    than half of whose rows sit on an excluded position, past SHARED_GAMES of the
    sample's games -- the corpus's own game, which datagen plays again for any
    pool whose workers' seeds [seed, seed + threads) meet the corpus's -- and
    sample games repeating another's rows past REPEATED_GAMES of them (DEC-345
    (5)): a worker seed two pools share repeats a worker's every game, while
    independent workers' openings transpose into the same game about 4.7e-9 a
    pair (the corpus's first 54645 games hold 7 such pairs), 0.34 expected in
    12000 games; those stay in, independent draws, and are counted."""
    held = corpus_split.validation_positions(args.exclude)
    kept = dropped = 0
    games = []          # [rows, rows held, positions]
    last = None
    temporary = args.out + ".tmp"
    with open(args.sample, "rb") as source, open(temporary, "wb") as sink:
        for number, line in enumerate(source, 1):
            if not line.strip():
                continue
            fields, game = corpus_split.row_fields(line, args.sample, number)
            ply = int(fields[5])
            if last is None or game != last[0] or ply <= last[1]:
                games.append([0, 0, []])
            last = (game, ply)
            here = corpus_split.position(fields[0], args.sample, number)
            games[-1][0] += 1
            games[-1][2].append(here)
            if here in held:
                games[-1][1] += 1
                dropped += 1
                continue
            sink.write(line)
            kept += 1
    shared = sum(1 for rows, inside, _ in games if 2 * inside > rows)
    repeats = collections.Counter(tuple(g[2]) for g in games)
    twice = sum(c - 1 for c in repeats.values() if c > 1)
    print(f"sample-prep: {len(games)} games, {kept} rows kept, {dropped} on an "
          f"excluded position; {shared} games more than half excluded, {twice} "
          "games another game repeats row for row")
    problems = []
    if shared > SHARED_GAMES * len(games):
        problems.append(f"{shared} of {len(games)} games are more than half on "
                        "an excluded position: the sample holds the corpus's "
                        "games, its seed range meets the corpus's")
    if twice > REPEATED_GAMES * len(games):
        problems.append(f"{twice} of {len(games)} games repeat another's rows "
                        f"row for row, past {REPEATED_GAMES:.1%}: more than "
                        "independent workers' transposed openings make, as "
                        "when a worker seed is two pools' (datagen seeds worker "
                        "w from seed + w)")
    if problems:
        os.remove(temporary)
        raise Refused("; ".join(problems) + "; the sample is the coordinator's")
    os.replace(temporary, args.out)
    return 0


def games_of(path):
    """[(index, game, rows, sse)] in file order: tools/margin_census --games'
    lines, a game a run of one id and a rising ply, so two pools joined in one
    file keep their games apart (S122's fast check, finding 1)."""
    out = []
    for line in open(path):
        if line.startswith("#") or not line.strip():
            continue
        fields = line.split()
        if len(fields) != 4:
            raise Refused(f"{path}: `{line.strip()}` is not `index game rows "
                          "sse`, tools/margin_census --games' line")
        index, game, rows, sse = fields
        out.append((int(index), int(game), int(rows), float(sse)))
    if [g[0] for g in out] != list(range(len(out))):
        raise Refused(f"{path}'s game indices are not 0 to {len(out) - 1}")
    return out


def side_read(args):
    """DEC-344 (1)'s reading of a part on the 12000-game sample: the paired
    difference of the two vectors' mean squared errors, its error by game (the
    jackknife over games of a ratio of sums), and the stop: the sample's d of
    the other sign from the screen's full d and beyond SIDE_Z errors."""
    pred = games_of(args.pred)
    mine = games_of(args.part)
    if [(g[1], g[2]) for g in pred] != [(g[1], g[2]) for g in mine]:
        raise Refused("the two files do not hold the same games in the same "
                      "order, row for row")
    games = [g[0] for g in pred]
    rows = [g[2] for g in pred]
    diff = [a[3] - b[3] for a, b in zip(pred, mine)]
    n_rows = sum(rows)
    d = sum(diff) / n_rows
    total_diff = sum(diff)
    leave = [(total_diff - x) / (n_rows - r) for x, r in zip(diff, rows)]
    mean = sum(leave) / len(leave)
    se = math.sqrt((len(leave) - 1) / len(leave) *
                   sum((v - mean) ** 2 for v in leave))
    stop = (d * args.full_d < 0) and abs(d) > SIDE_Z * se
    print(f"side {args.label}: sample d {d:+.7f} +/- {se:.7f} (by game, "
          f"{len(games)} games, {n_rows} rows), screen full d "
          f"{args.full_d:+.7f}: {'STOP' if stop else 'agrees within its error'}")
    if stop:
        print(f"STOP: {args.label} reads the other way on the sample, beyond "
              f"{SIDE_Z} errors: the coordinator's, before any SPRT "
              "(DEC-344 (1))")
    return 0


def clamp(args):
    """DEC-333 (6)'s 1.0 % at the candidate's margin, on the census of the
    candidate's vector, which is the pasted tree's arithmetic."""
    margin = int(args.margin)
    rows, _pick, _x = census_of(args.census)
    if margin not in rows:
        raise Refused(f"{args.census} has no row at {margin}")
    share = rows[margin][0]
    print(f"clamp: binds on {share:.3f} % of rows at margin {margin}, against "
          f"the bound {CLAMP_SHARE_BOUND:.3f} %")
    if share > CLAMP_SHARE_BOUND:
        print("STOP: the candidate's clamp share passes the bound; phase 2 "
              "stops for the coordinator (DEC-333 (6))")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S122's screen helpers.")
    commands = parser.add_subparsers(dest="command", required=True)

    c = commands.add_parser("plan")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)

    c = commands.add_parser("audit-check")
    c.add_argument("report")

    c = commands.add_parser("ties")
    c.add_argument("report")
    c.add_argument("--out", required=True)

    c = commands.add_parser("margin-read")
    c.add_argument("census")
    c.add_argument("--round", type=int, required=True)
    c.add_argument("--margin", type=int, required=True)
    c.add_argument("--out", default="")

    c = commands.add_parser("p0-read")
    c.add_argument("log184")
    c.add_argument("logm")

    c = commands.add_parser("read")
    c.add_argument("part", choices=PARTS)
    c.add_argument("--out", required=True)
    c.add_argument("--tree", required=True)
    c.add_argument("--cap", type=int, default=None)

    c = commands.add_parser("start-check")
    c.add_argument("header")
    c.add_argument("--tree", required=True)
    c.add_argument("--free", default="")
    c.add_argument("--allow-five", action="store_true")

    c = commands.add_parser("headroom")
    c.add_argument("header")
    c.add_argument("--tree", required=True)
    c.add_argument("--wide", action="store_true")

    c = commands.add_parser("only-family")
    c.add_argument("tree")
    c.add_argument("--allow-five", action="store_true")

    c = commands.add_parser("landing-check")
    c.add_argument("--tree", required=True)
    c.add_argument("--commit", default="")

    c = commands.add_parser("zero-five")
    c.add_argument("tree")

    c = commands.add_parser("part")
    c.add_argument("name")
    c.add_argument("--out", required=True)
    c.add_argument("--margin", type=int, default=None)
    c.add_argument("--ruled", default="")
    c.add_argument("--base", default="")
    c.add_argument("--decisions", default="",
                   help="a plant only: FILE in place of HEAD's decisions.md")

    c = commands.add_parser("ruled")
    c.add_argument("fit")
    c.add_argument("--dec", required=True)
    c.add_argument("--out", required=True)
    c.add_argument("--base", required=True)
    c.add_argument("--decisions", default="",
                   help="a plant only: FILE in place of HEAD's decisions.md")

    c = commands.add_parser("sample-prep")
    c.add_argument("sample")
    c.add_argument("--exclude", action="append", required=True)
    c.add_argument("--out", required=True)

    c = commands.add_parser("side-read")
    c.add_argument("pred")
    c.add_argument("part")
    c.add_argument("--label", required=True)
    c.add_argument("--full-d", type=float, required=True)

    c = commands.add_parser("clamp")
    c.add_argument("census")
    c.add_argument("--margin", required=True)

    args = parser.parse_args(argv)
    handlers = {"plan": plan, "audit-check": audit_check, "ties": ties,
                "margin-read": margin_read, "p0-read": p0_read, "read": read,
                "start-check": start_check, "headroom": headroom,
                "only-family": only_family, "landing-check": landing_check,
                "zero-five": zero_five, "part": part, "ruled": ruled,
                "sample-prep": sample_prep, "side-read": side_read,
                "clamp": clamp}
    try:
        return handlers[args.command](args)
    except (Refused, OSError, subprocess.CalledProcessError) as why:
        print(f"S122_screen: refused: {why}", file=sys.stderr)
        return 1
    except corpus_split.Refused as why:
        print(f"S122_screen: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
