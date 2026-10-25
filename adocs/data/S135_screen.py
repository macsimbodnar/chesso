#!/usr/bin/env python3
"""S135's screen: which corpus, the reading of the term fits, the bisection's
parts, and the paste check. adocs/data/S135_screen.sh runs every one of them;
each also runs alone. Standard library only, plus tools/corpus_split.py,
imported so a corpus re-drawn here is drawn exactly the way S083 drew it.

    S135_screen.py locate --source s083|s082 --s083-out DIR --s082-out DIR
                          --out FILE
    S135_screen.py read DIR --tree TREE
    S135_screen.py part NAME --survivors TERM[,TERM...]
    S135_screen.py start-check HEADER --tree TREE [--free TERM[,TERM...]]
    S135_screen.py headroom HEADER --tree TREE
    S135_screen.py only-placement TREE [--free TERM[,TERM...]]

LOCATE finds the corpus the screen fits on and writes FILE: `source`,
`corpus`, `sha256`, `rows`. --source is the coordinator's statement of S083's
verdict (the header of S135_screen.sh): s083 is the candidate S083's reading
stopped at -- the last stage whose reading.txt printed READ-OUTCOME stop, its
`candidate` fit, and the corpus that fit was drawn on, cand_<top>_s1.tsv
beside it -- and s082 is S083's stage 0 top rung, every game of S082's
training side at 46 rows a game. Since S083's H1 (DEC-313) S135_screen.sh
passes s083 alone; S083 stopped at stage 0, where the two name the same file.
Either way the sha256 is the one the stage's samples.log recorded when
`corpus_split.py sample` wrote the file, and the file is hashed against it. A
corpus S083 has since removed is drawn again from the pool its curve.log
names, under the same seed, rows and density, with the fixed set's three
views under --s082-out excluded as S083 excluded them, into FILE's directory,
and must come out at that sha256.

READ applies the rule S135_screen.sh pre-registers to the fits in DIR,
mechanically; the script's header is the rule's statement and this is its
implementation. Exit 0 with SCREEN-OUTCOME candidate or none; exit 1 on
SCREEN-OUTCOME unreadable.

PART prints one bisection part's terms among the survivors and the --freeze
list that fits it alone, by the partition the header writes down.

START-CHECK compares every value HEADER emits with TREE's HEAD sources and
refuses on any difference outside the placement columns of the --free terms.
A tuner starts from the constants compiled into it, so a tuner built before
HEAD -- before S083's paste, or before a bisection part landed -- would fit
from other constants and emit a landed part's columns at their old values;
S135_screen.sh runs this on a zero-epoch probe of each frozen tuner before
it fits, and on every fit's header after.

HEADROOM computes src/evaluation.cpp's stage_one_bound for the weights HEADER
emits -- the same arithmetic, per half: the tables' king spread plus fifteen
times the other tables' spread, thirty passers, fifteen per structure feature
and each placement weight times its reach, read from TREE's
piece_placement_reach -- and refuses when a half passes 32767, which is the
static_assert the paste's build would fail on.

ONLY-PLACEMENT reads every weight array and define a fit pastes from TREE's
working copy and from TREE's HEAD and refuses unless piece_placement_mg and
piece_placement_eg are the only ones that differ, and, with --free, unless
every placement column outside the free terms is unchanged too: the candidate
and a part are `--only piece_placement` fits with the other terms frozen, so
anything else moving is a paste of the wrong file or of a stale tuner's.
"""

import argparse
import hashlib
import os
import re
import statistics
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(
    __file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import corpus_split  # noqa: E402  (tools/, by path)

# The five terms, in tools/tuner_groups.hpp PLACEMENT_TERMS order, and the
# name each takes after `piece_placement.`.
TERMS = ["bishop_pair", "rook_open", "rook_half_open", "bad_bishop",
         "trapped"]
# Each term's columns in the group, tools/tuner_groups.hpp PLACEMENT_TERMS.
COLUMNS = {"bishop_pair": [0], "rook_open": [1], "rook_half_open": [2],
           "bad_bishop": [3, 4], "trapped": [5, 6]}
PLACEMENT_WIDTH = 7
HALF_LIMIT = 32767
SEEDS = [1, 2, 3, 4, 5]
PICK_Z = 1.645          # one-sided 95 % point of the standard normal
FLOOR = 2.0e-06         # two units of the tuner's sixth printed decimal
SPREAD_UNREADABLE = 1.0e-03
PER_GAME = 46

# The bisection, written before any fit: each part's terms, its two halves.
PARTS = {
    "family": (TERMS, ("A", "B")),
    "A": (["bishop_pair", "rook_open", "rook_half_open"], ("A1", "A2")),
    "B": (["bad_bishop", "trapped"], ("B1", "B2")),
    "A1": (["bishop_pair"], ()),
    "A2": (["rook_open", "rook_half_open"], ("A2a", "A2b")),
    "A2a": (["rook_open"], ()),
    "A2b": (["rook_half_open"], ()),
    "B1": (["bad_bishop"], ()),
    "B2": (["trapped"], ()),
}

# Section 4 of the step file: a sanity band stated before the fit and never a
# target. Printed beside the term, deciding nothing.
BANDS = {
    "bishop_pair": "Kaufman's half a pawn, 47 on chesso's scale (DEC-105 "
                   "form (a))",
    "rook_open": "0 to 47 by declared purpose, midpoint 24 (DEC-134 form "
                 "(c))",
    "rook_half_open": "half of rook_open's, by the same declaration",
}

BEST = re.compile(r"^best: train (\S+)\s+validation (\S+)\s+\(start (\S+) / "
                  r"(\S+?)\)")
START = re.compile(r"^start: train (\S+)\s+validation (\S+)")
K_LINE = re.compile(r"^fitted K = (\S+)")
CORPUS = re.compile(r"^(\d+) positions, (\d+) games, ")
NOT_IMPROVED = "held-out error did not improve"
ARRAY = re.compile(r"^(?:constexpr|const) int (piece_placement_(?:mg|eg))"
                   r"\[\w+\] = \{([^}]*)\};", re.M)
SAMPLED = re.compile(r"sha256 ([0-9a-f]{64})  (\S+)$")


class Refused(Exception):
    """Something other than what was asked for would be produced or read."""


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def count_rows(path):
    rows = 0
    with open(path, "rb") as handle:
        for line in handle:
            if line.strip():
                rows += 1
    return rows


def recorded_sha(stage_dir, corpus):
    """The sha256 samples.log recorded beside `corpus` when it was drawn."""
    log = os.path.join(stage_dir, "samples.log")
    if not os.path.exists(log):
        raise Refused(f"{log} is missing: no record of {corpus}'s sha256")
    found = None
    for line in open(log, encoding="utf-8", errors="replace"):
        m = SAMPLED.search(line.rstrip("\n"))
        if m and os.path.abspath(m.group(2)) == os.path.abspath(corpus):
            found = m.group(1)
    if found is None:
        raise Refused(f"{log} records no sample written to {corpus}")
    return found


def stage_pool(stage_dir):
    """(pool path, pool sha256) from a stage's curve.log."""
    log = os.path.join(stage_dir, "curve.log")
    pool = None
    for line in open(log, encoding="utf-8", errors="replace"):
        m = re.match(r"^pool\s+(\S+), sha256 ([0-9a-f]{64})$", line.strip())
        if m:
            pool = (m.group(1), m.group(2))
    if pool is None:
        raise Refused(f"{log} names no pool")
    return pool


def views(s082):
    return [os.path.join(s082, f"pool.tsv.{v}.validation")
            for v in ("qs", "root", "pv")]


def locate(args):
    s083 = args.s083_out
    curve = os.path.join(s083, "curve")
    if args.source == "s083":
        stages = sorted((int(d[1:]) for d in os.listdir(curve)
                         if re.fullmatch(r"s\d+", d)), reverse=True)
        chosen = None
        for n in stages:
            reading = os.path.join(curve, f"s{n}", "reading.txt")
            if not os.path.exists(reading):
                continue
            text = open(reading, encoding="utf-8", errors="replace").read()
            if re.search(r"^READ-OUTCOME stop$", text, re.M):
                m = re.search(r"^candidate (\S+)$", text, re.M)
                if not m:
                    raise Refused(f"{reading} stops and names no candidate")
                chosen = (n, m.group(1))
                break
        if chosen is None:
            raise Refused(f"no stage under {curve} printed READ-OUTCOME stop; "
                          "S083 has not chosen its candidate")
        stage, fit = chosen
        m = re.fullmatch(r"fit_(\d+)_s1\.hpp", os.path.basename(fit))
        if not m:
            raise Refused(f"S083's candidate {fit} is not a top-rung seed-1 "
                          "fit")
        top = int(m.group(1))
    else:
        stage = 0
        ladder = os.path.join(curve, "s0", "ladder.txt")
        rungs = [int(line.split()[1]) for line in open(ladder)
                 if line.startswith("rung ")]
        if not rungs:
            raise Refused(f"{ladder} names no rung")
        top = rungs[0]
        fit = None

    stage_dir = os.path.join(curve, f"s{stage}")
    corpus = os.path.join(stage_dir, f"cand_{top}_s1.tsv")
    want = recorded_sha(stage_dir, corpus)
    if not os.path.exists(corpus):
        # S083 may remove a read stage's samples; draw it again from its
        # pool under the same seed, rows and density, which reproduces it.
        pool, pool_sha = stage_pool(stage_dir)
        if sha256(pool) != pool_sha:
            raise Refused(f"{pool} is not the pool {stage_dir} recorded")
        corpus = os.path.join(os.path.dirname(os.path.abspath(args.out)),
                              f"cand_{top}_s1.tsv")
        if os.path.exists(corpus):
            os.remove(corpus)
        status = corpus_split.main(
            ["sample", "--in", pool] +
            [x for v in views(args.s082_out) for x in ("--validation", v)] +
            ["--rows", str(top), "--per-game", str(PER_GAME), "--seed", "1",
             "--out", corpus])
        if status != 0:
            raise Refused(f"re-drawing {corpus} from {pool} failed")
    got = sha256(corpus)
    if got != want:
        raise Refused(f"{corpus} has sha256 {got}, not the {want} "
                      f"{stage_dir}/samples.log recorded")
    rows = count_rows(corpus)
    if rows != top:
        raise Refused(f"{corpus} holds {rows} rows, its name says {top}")
    with open(args.out, "w") as out:
        out.write(f"source {args.source}\n")
        out.write(f"stage {stage}\n")
        out.write(f"corpus {corpus}\n")
        out.write(f"sha256 {got}\n")
        out.write(f"rows {rows}\n")
        if fit:
            out.write(f"s083_fit {fit}\n")
    print(open(args.out).read(), end="")
    return 0


def read_fit(path):
    """(start L, best L, K, improved, rows, placement) from a tuner log and
    the header beside it."""
    if not os.path.exists(path):
        raise Refused(f"missing {path}")
    best = k = None
    start = None
    improved = True
    rows = None
    for line in open(path, encoding="utf-8", errors="replace"):
        m = BEST.match(line)
        if m:
            best = (m.group(2), m.group(4))
        m = START.match(line)
        if m:
            start = m.group(2)
        m = K_LINE.match(line)
        if m:
            k = m.group(1)
        m = CORPUS.match(line)
        if m and rows is None:
            rows = int(m.group(1))
        if NOT_IMPROVED in line:
            improved = False
    if best is None:
        raise Refused(f"no best: line in {path}")
    if start is None or start != best[1]:
        raise Refused(f"{path}: the start: line and the best: line disagree "
                      "about the start")
    placement = {}
    header = path[:-len(".log")] + ".hpp"
    if os.path.exists(header):
        for name, body in ARRAY.findall(open(header).read()):
            placement[name] = [int(v) for v in re.findall(r"-?\d+", body)]
    return float(start), float(best[0]), k, improved, rows, placement


def read(args):
    directory = args.dir
    print(f"S135 screen reading, {directory}")
    problems = []

    full = {}
    for term in TERMS:
        full[term] = read_fit(os.path.join(directory, f"full_{term}.log"))

    # One baseline for all five: the same corpus, the same fixed set, the
    # same starting constants and so the same K and the same start figure.
    starts = {f"{full[t][0]:.6f}" for t in TERMS}
    ks = {full[t][2] for t in TERMS}
    if len(starts) != 1 or len(ks) != 1:
        problems.append(f"the five full fits do not share a baseline: start "
                        f"{sorted(starts)}, K {sorted(ks)}")
    baseline = full[TERMS[0]][0]
    print(f"baseline: held-out {baseline:.6f} at the shipped constants, K "
          f"{full[TERMS[0]][2]}, {full[TERMS[0]][4]} rows")

    quarter = {}
    for seed in SEEDS:
        for term in TERMS:
            quarter[(term, seed)] = read_fit(
                os.path.join(directory, f"quarter_s{seed}_{term}.log"))

    print(f"{'term':<16}{'gain':>11}{'s':>11}{'m':>11}  verdict   "
          "quarter gains, seeds 1 to 5")
    survivors = []
    for term in TERMS:
        start, best, _k, _improved, _rows, _w = full[term]
        # best is the minimum over reports on the fixed set that also scores
        # the term, so the gain is >= 0 and selected on that set.
        gain = start - best
        gains = [quarter[(term, s)][0] - quarter[(term, s)][1]
                 for s in SEEDS]
        # Training-resampling spread only: the six fits share the fixed set,
        # so its sampling noise and the best-report pick are not in s (the
        # shell script's THE MARGIN says why that is accepted).
        s = statistics.stdev(gains)
        m = max(PICK_Z * s, FLOOR)
        if s > SPREAD_UNREADABLE:
            problems.append(f"{term}: s {s:.6f} above {SPREAD_UNREADABLE}")
        keep = gain > m
        if keep:
            survivors.append(term)
        print(f"{term:<16}{gain:>+11.6f}{s:>11.6f}{m:>11.6f}  "
              f"{'kept   ' if keep else 'dropped'}   "
              + " ".join(f"{g:+.6f}" for g in gains))
    for term in TERMS:
        header = os.path.join(directory, f"full_{term}.hpp")
        if args.tree and os.path.exists(header):
            bounds = headroom_of(header, args.tree)
            print(f"  {term}: stage-one bound mg {bounds['mg']}, eg "
                  f"{bounds['eg']} of {HALF_LIMIT}")
            if max(bounds.values()) > HALF_LIMIT:
                problems.append(f"{term}'s fit alone passes the stage-one "
                                "bound")
    for term in TERMS:
        weights = full[term][5]
        if weights:
            print(f"  {term}: fitted alone, mg {weights.get('piece_placement_mg')}"
                  f" eg {weights.get('piece_placement_eg')}")
        if term in BANDS:
            print(f"    sanity band, never a target: {BANDS[term]}")

    if problems:
        print("outcome, unreadable: " + "; ".join(problems))
        print("SCREEN-OUTCOME unreadable")
        return 1
    if not survivors:
        print("outcome: no term lowers held-out loss past its margin; each "
              "is recorded with its figures and none plays a match")
        print("SCREEN-OUTCOME none")
        return 0
    dropped = [t for t in TERMS if t not in survivors]
    print("survivors " + ",".join(survivors))
    print("freeze " + (",".join(f"piece_placement.{t}" for t in dropped)
                       or "(nothing)"))
    print("bisection on H0 or no verdict, in order: " + bisection(survivors))
    print("SCREEN-OUTCOME candidate " + ",".join(survivors))
    return 0


def part_terms(name, survivors):
    return [t for t in PARTS[name][0] if t in survivors]


def children(name, survivors):
    """The parts a failed part splits into among the survivors: empty halves
    dropped, and a half holding all of the part's survivors skipped down to
    its own halves, since measuring it would repeat the part's SPRT."""
    mine = part_terms(name, survivors)
    out = []
    for child in PARTS[name][1]:
        theirs = part_terms(child, survivors)
        if not theirs:
            continue
        if theirs == mine:
            out.extend(children(child, survivors))
        else:
            out.append(child)
    return out


def bisection(survivors):
    def walk(name):
        kids = children(name, survivors)
        if not kids:
            return name
        return f"{name} -> [" + ", ".join(walk(k) for k in kids) + "]"
    return walk("family")


def part(args):
    survivors = [t for t in args.survivors.split(",") if t]
    unknown = [t for t in survivors if t not in TERMS]
    if unknown or args.name not in PARTS:
        raise Refused(f"unknown part {args.name} or terms {unknown}")
    terms = part_terms(args.name, survivors)
    if not terms:
        raise Refused(f"part {args.name} holds none of the survivors "
                      f"{survivors}")
    frozen = [t for t in TERMS if t not in terms]
    print("terms " + ",".join(terms))
    print("freeze " + ",".join(f"piece_placement.{t}" for t in frozen))
    print("halves " + (" ".join(children(args.name, survivors)) or "(a leaf)"))
    return 0


def weights_of(text):
    """Every define and int array a fit pastes, by name."""
    out = {}
    for name, value in re.findall(r"^#define (\w+)\s+(-?\d+)$", text, re.M):
        out[name] = [int(value)]
    for name, body in re.findall(
            r"^(?:static )?(?:constexpr|const) int (\w+)\[[^=]*\] = "
            r"\{(.*?)\};", text, re.M | re.S):
        out[name] = [int(v) for v in re.findall(
            r"-?\d+", re.sub(r"//[^\n]*", "", body))]
    for name, value in re.findall(
            r"^(?:constexpr|const) int (\w+) = (-?\d+);", text, re.M):
        out[name] = [int(value)]
    return out


def head_weights(tree):
    """Every define and int array of TREE's HEAD evaluation sources."""
    out = {}
    for path in ("src/eval_tables.hpp", "src/evaluation.cpp"):
        head = subprocess.run(["git", "-C", tree, "show", f"HEAD:{path}"],
                              check=True, capture_output=True,
                              text=True).stdout
        out.update(weights_of(head))
    return out


def free_columns(free):
    """The placement columns the named terms free."""
    names = [t for t in (free or "").split(",") if t]
    unknown = [t for t in names if t not in TERMS]
    if unknown:
        raise Refused(f"unknown terms {unknown}")
    return sorted(c for t in names for c in COLUMNS[t])


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
        want = shipped[name]
        if len(values) != len(want):
            bad.append(f"{name}: {len(values)} values emitted, {len(want)} "
                       "at HEAD")
            continue
        differ = [i for i, (x, y) in enumerate(zip(values, want)) if x != y
                  and not (name.startswith("piece_placement_") and i in free)]
        if differ:
            bad.append(f"{name}: {len(differ)} frozen values differ from "
                       f"HEAD's, the first at index {differ[0]}")
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short",
                           "HEAD"], check=True, capture_output=True,
                          text=True).stdout.strip()
    print(f"start-check {args.header}: {len(emitted)} definitions against "
          f"HEAD {head}, free placement columns {free or '(none)'}")
    if bad:
        raise Refused("the tuner did not start from HEAD's constants: "
                      + "; ".join(bad))
    print("start-check: every frozen value is HEAD's")
    return 0


def reach_of(tree):
    text = open(os.path.join(tree, "src", "evaluation.cpp")).read()
    m = re.search(r"piece_placement_reach\[\w+\] = \{([^}]*)\};", text)
    if not m:
        raise Refused(f"{tree}/src/evaluation.cpp has no piece_placement_reach")
    reach = [int(v) for v in re.findall(r"-?\d+", m.group(1))]
    if len(reach) != PLACEMENT_WIDTH:
        raise Refused(f"piece_placement_reach is {len(reach)} wide")
    return reach


def half_bound(weights, half, reach):
    """src/evaluation.cpp stage_one_half(), for one half."""
    table = weights[f"psqt_{half}"]
    if len(table) != 384:
        raise Refused(f"psqt_{half} holds {len(table)} values")
    kings = table[5 * 64:]
    pieces = table[:5 * 64]
    bound = (max(max(kings), 0) - min(min(kings), 0)) + \
        15 * (max(max(pieces), 0) - min(min(pieces), 0))
    bound += 2 * 15 * max(abs(w) for w in weights[f"passed_pawn_{half}"])
    structure = weights[f"pawn_structure_{half}"]
    bound += len(structure) * 15 * max(abs(w) for w in structure)
    placement = weights[f"piece_placement_{half}"]
    if len(placement) != PLACEMENT_WIDTH:
        raise Refused(f"piece_placement_{half} is {len(placement)} wide")
    bound += sum(r * abs(w) for r, w in zip(reach, placement))
    return bound


def headroom_of(header, tree):
    weights = head_weights(tree)
    weights.update(weights_of(open(header).read()))
    reach = reach_of(tree)
    return {half: half_bound(weights, half, reach) for half in ("mg", "eg")}


def headroom(args):
    bounds = headroom_of(args.header, args.tree)
    for half, bound in bounds.items():
        print(f"stage-one bound, {half}: {bound} of {HALF_LIMIT}, headroom "
              f"{HALF_LIMIT - bound}")
    if max(bounds.values()) > HALF_LIMIT:
        raise Refused(f"{args.header} would fail stage_one_bound's "
                      "static_assert; the step file's pre-registered branch "
                      "applies, the weights are not clipped")
    return 0


def only_placement(args):
    tree = args.tree
    before = head_weights(tree)
    after = {}
    for path in ("src/eval_tables.hpp", "src/evaluation.cpp"):
        after.update(weights_of(open(os.path.join(tree, path)).read()))
    moved = sorted(n for n in set(before) | set(after)
                   if before.get(n) != after.get(n))
    others = [n for n in moved
              if n not in ("piece_placement_mg", "piece_placement_eg")]
    print("moved: " + (", ".join(moved) or "(nothing)"))
    if others:
        raise Refused("the paste moved more than the placement weights: "
                      + ", ".join(others))
    if args.free is not None:
        free = free_columns(args.free)
        for name in ("piece_placement_mg", "piece_placement_eg"):
            held = [i for i in range(PLACEMENT_WIDTH) if i not in free and
                    before[name][i] != after[name][i]]
            if held:
                raise Refused(f"{name}: columns {held} belong to no free term "
                              "and moved")
        print(f"placement columns outside {free} unchanged")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S135's screen helpers.")
    commands = parser.add_subparsers(dest="command", required=True)

    find = commands.add_parser("locate")
    find.add_argument("--source", choices=("s083", "s082"), required=True)
    find.add_argument("--s083-out", required=True)
    find.add_argument("--s082-out", required=True)
    find.add_argument("--out", required=True)

    rule = commands.add_parser("read")
    rule.add_argument("dir")
    rule.add_argument("--tree")

    start = commands.add_parser("start-check")
    start.add_argument("header")
    start.add_argument("--tree", required=True)
    start.add_argument("--free")

    room = commands.add_parser("headroom")
    room.add_argument("header")
    room.add_argument("--tree", required=True)

    cut = commands.add_parser("part")
    cut.add_argument("name")
    cut.add_argument("--survivors", required=True)

    paste = commands.add_parser("only-placement")
    paste.add_argument("tree")
    paste.add_argument("--free")

    args = parser.parse_args(argv)
    handlers = {"locate": locate, "read": read, "part": part,
                "start-check": start_check, "headroom": headroom,
                "only-placement": only_placement}
    try:
        return handlers[args.command](args)
    except (Refused, corpus_split.Refused, OSError,
            subprocess.CalledProcessError) as why:
        print(f"S135_screen: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
