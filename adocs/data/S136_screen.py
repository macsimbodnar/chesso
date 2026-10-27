#!/usr/bin/env python3
"""S136's screen: the reading of tempo's six fits, the truncation guard's
arithmetic at a fitted pair, its forecast over the corpus, the pick of the
four pinned rows, and the paste checks. adocs/data/S136_screen.sh runs every
one of them; each also runs alone. Standard library only, plus
adocs/data/S135_screen.py, imported so the fit logs and headers are parsed
exactly the way S135's screen parsed them and the rule's constants are its.

    S136_screen.py read DIR --tree TREE [--rows ROWS]...
    S136_screen.py start-check HEADER --tree TREE [--free]
    S136_screen.py only-tempo TREE
    S136_screen.py guard (--header HEADER | --pair MG EG) [--rows ROWS]...
    S136_screen.py forecast ROWS --pins PINS
    S136_screen.py pick SCAN (--header HEADER | --pair MG EG)
    S136_screen.py short SCAN (--header HEADER | --pair MG EG)

READ applies the rule S136_screen.sh pre-registers to the fits in DIR,
mechanically: tempo is a family of one (DEC-259), so S135's rule (DEC-315)
over one term. Exit 0 with SCREEN-OUTCOME candidate tempo or none; exit 1 on
SCREEN-OUTCOME unreadable. Printed beside, deciding nothing: the fitted pairs,
the guard's arithmetic at the full fit's pair and, with --rows, its forecast.

START-CHECK compares every value HEADER emits with TREE's HEAD sources and
refuses on any difference, tempo_mg and tempo_eg excepted under --free. A
tuner starts from the constants compiled into it, so a stale one would fit
tempo against other constants; S136_screen.sh runs this on a zero-epoch probe
of the frozen tuner and on every fit's header.

ONLY-TEMPO reads every weight a fit pastes from TREE's working copy and from
TREE's HEAD and refuses unless tempo_mg and tempo_eg are the only ones that
differ: the paste is an `--only tempo` fit.

GUARD prints tests/test_eval_trace.cpp's truncation arithmetic at a tempo
pair -- the four tapered divisions, whether tempo's can round, its largest
remainder, the bound, both non-vacuity thresholds -- and the four rows pinned
at S136's HEAD scored at that pair, with the assertions of "the pinned
positions reach the truncation bound" each would fail as the test stands.
This is the red the paste is expected to show first, before the guard is
re-targeted. With --rows, the forecast line for the pair as well, one per
rows file.

FORECAST reads adocs/data/S136_guard_forecast.cpp's rows for a set of
positions -- the corpus, or S082's three training views -- and prints, for
every tempo spread modulo 24 and both signs, what that set reaches at the
pair: its largest residual, the distinct positions at it and
past 69/24 by side, and whether adocs/data/S083_truncation_pick.py's rule (two
positions of each side at the maximum) could pick four. --pins is the same
program's reading of the four pinned rows, checked against PINS below: a
forecast is only valid at the weights PINS was read at.

PICK applies the four-division pick proposed in S136's step file to a
truncation_scan output read at the fitted weights (`--min 2.875`, over the
corpus or over S082's training views, the scans concatenated): the
positions past 69/24, the largest of them first, the rest spanning the phase
range with both sides to move. It prints the four, as C++ lines, and the
thresholds the re-targeted test would assert. DEC-322 (2): it picks where
S083's rule refuses on S082's training views.

SHORT prints REACHED when a scan's largest residual is the arithmetic reach
at the pair, and a STOP line when it falls short: DEC-322 (3) is unruled, so
phase 2 stops before the re-targeting and the owner rules.
"""

import argparse
import math
import os
import statistics
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import S135_screen  # noqa: E402  (adocs/data/, by path)

from S135_screen import (Refused, head_weights, read_fit,  # noqa: E402
                         weights_of)

TEMPO = ("tempo_mg", "tempo_eg")
SEEDS = S135_screen.SEEDS
PICK_Z = S135_screen.PICK_Z
FLOOR = S135_screen.FLOOR
SPREAD_UNREADABLE = S135_screen.SPREAD_UNREADABLE

PHASE_MAX = 24          # src/evaluation.hpp GAME_PHASE_MAX
THREE = 3 * 23          # three divisions at their largest remainder, in 24ths

# The four rows tests/test_eval_trace.cpp pins at S136's HEAD (80520d7, S135's
# fit), each with its phase, its side to move and its residual signed, smooth
# minus exact, White relative, in 24ths -- read by S136_guard_forecast with
# --min 0 on the four FENs. Every one sits at 69/24: the three divisions at
# their largest remainder, the same sign.
PINS = [
    ("2R5/8/3r3p/6p1/1k2B1P1/4K2P/5P2/8 b - - 16 52", 5, "b", 69),
    ("6k1/7p/p2b2p1/5n2/8/2Q5/PP3P1P/2KN3q w - - 1 33", 11, "w", -69),
    ("2q3k1/1b1R2p1/2p1r3/p2pNQ1p/Pb1P2pP/4P1P1/1P4K1/2R5 b - - 5 35", 17,
     "b", 69),
    ("rn5k/ppp3bp/3qbp2/3p2pB/8/1PN2Q1P/P1PP1PP1/R4RK1 w - - 4 16", 19, "w",
     69),
]

# What the test asserts as it stands, in 24ths where the residual lives on the
# lattice: each pin past 2.0 = 48/24, the worst past 2.8 (so at least 68/24).
PER_PIN_NOW = 2.0
WORST_NOW = 2.8
WORST_ROOM = 0.075      # 2.875 - 2.8: the worst pin's room under the maximum
# Past 69/24 on the lattice. 69/24 is a value rows sit exactly on, and ulp
# noise splits them either side of it (DEV_MANUAL.md, the 2.0 = 48/24
# column), so the per-pin threshold and the pick both take 1e-6 of room.
LATTICE_ROOM = 1.0e-6
PER_PIN_FOUR = THREE / PHASE_MAX + LATTICE_ROOM


def tempo_residual(mg, eg, phase, side):
    """Tempo's division, smooth minus exact, White relative, in 24ths: the
    engine adds trunc(N / 24) for the side to move, N = mg * phase + eg *
    (24 - phase), which is side * N / 24 truncated towards zero from White's
    side (tools/tuner_trace.hpp exact_taper on the side-signed weights)."""
    n = (1 if side == "w" else -1) * (mg * phase + eg * (PHASE_MAX - phase))
    r = abs(n) % PHASE_MAX
    return r if n >= 0 else -r


def arithmetic(mg, eg):
    """The guard's numbers at a tempo pair. tempo's numerator is congruent to
    (mg - eg) * phase modulo 24, so over the 25 phases its remainders are the
    multiples of g = gcd(mg - eg, 24): none but 0 when 24 divides mg - eg,
    and at most 24 - g otherwise."""
    spread = mg - eg
    g = math.gcd(abs(spread), PHASE_MAX)
    rounds = spread % PHASE_MAX != 0
    largest = PHASE_MAX - g if rounds else 0
    reach = THREE + largest
    out = {
        "spread": spread, "g": g, "rounds": rounds, "largest": largest,
        "divisions": 4 if rounds else 3, "reach": reach,
        # The bound test's literal today: 3 x 23/24 while both are zero,
        # 4 x 23/24 once either is not, whatever the spread.
        "literal_bound": THREE if (mg == 0 and eg == 0) else 4 * 23,
    }
    if rounds:
        out["per_pin"] = PER_PIN_FOUR
        out["worst"] = reach / PHASE_MAX - WORST_ROOM
    else:
        out["per_pin"] = PER_PIN_NOW
        out["worst"] = WORST_NOW
    return out


def pair_of(args):
    if args.header:
        emitted = weights_of(open(args.header).read())
        missing = [n for n in TEMPO if n not in emitted]
        if missing:
            raise Refused(f"{args.header} emits no {', '.join(missing)}")
        return emitted["tempo_mg"][0], emitted["tempo_eg"][0]
    return args.pair[0], args.pair[1]


def print_guard(mg, eg, out=sys.stdout):
    a = arithmetic(mg, eg)
    say = lambda s: print(s, file=out)  # noqa: E731
    say(f"guard at tempo_mg {mg}, tempo_eg {eg}: spread {a['spread']}, "
        f"gcd with 24 {a['g']}")
    if a["rounds"]:
        say(f"  tempo's division rounds: 4 divisions can, its largest "
            f"remainder {a['largest']}/24; the arithmetic bound "
            f"(69 + {a['largest']})/24 = {a['reach'] / 24:.6f}, the test's "
            f"literal {a['literal_bound']}/24 = "
            f"{a['literal_bound'] / 24:.6f}")
        say(f"  re-targeted: each pin past 69/24 + 1e-6; the worst past "
            f"{a['reach']}/24 - {WORST_ROOM} = {a['worst']:.6f}; the "
            f"precondition: tempo's division rounds")
    else:
        say("  tempo's division is exact at every phase (24 divides the "
            "spread): 3 divisions can round, bound 69/24 = 2.875, the "
            "thresholds stay 2.0 and 2.8; only the precondition's wording "
            "moves (from 'tempo is zero' to 'tempo's division is exact')")
    worst = 0
    reds = []
    if not (mg == 0 and eg == 0):
        reds.append("CHECK_MESSAGE(tempo_unfitted) -- certain at any "
                    "non-zero pair")
    for fen, phase, side, three in PINS:
        t = tempo_residual(mg, eg, phase, side)
        total = abs(three + t)
        worst = max(worst, total)
        state = "past 2.0" if total > 48 else "RED, not past 2.0"
        if total <= 48:
            reds.append(f"difference > 2.0 on the phase {phase} pin "
                        f"({total}/24)")
        say(f"  pin phase {phase:2d} {side}: {three:+d} {t:+d} -> "
            f"{total}/24 = {total / 24:.6f}, {state}")
    if worst < 68:
        reds.append(f"worst > 2.8 ({worst}/24)")
        # Mirrors read the same residual, so with every pin under 68/24 the
        # bound case's own worst clause rests on the other rows alone.
        reds.append("possibly the bound case's worst > 2.8 too: the pins "
                    "and their mirrors are all under 68/24, so it rests on "
                    "the shared list's other rows")
    say(f"  worst pin {worst}/24 = {worst / 24:.6f}")
    say("  expected red as the test stands: " +
        ("; ".join(reds) if reds else "(none)"))
    return a


def read_rows(path):
    rows = []
    for line in open(path, encoding="utf-8"):
        fields = line.rstrip("\n").split("\t")
        if len(fields) != 4:
            raise Refused(f"{path}: not a guard_forecast row: {line!r}")
        rows.append((int(fields[0]), int(fields[1]), fields[2], fields[3]))
    if not rows:
        raise Refused(f"{path} holds no rows")
    return rows


def forecast_of(rows, mg, eg):
    """What the corpus reaches at the pair: per distinct position its
    residual, the largest, and the positions at it and past 69/24."""
    best = {}
    for three, phase, side, fen in rows:
        total = abs(three + tempo_residual(mg, eg, phase, side))
        if total > best.get(fen, (-1,))[0]:
            best[fen] = (total, phase, side)
    top = max(v[0] for v in best.values())
    at_top = [v for v in best.values() if v[0] == top]
    past = [v for v in best.values() if v[0] > THREE]
    count = lambda vs, s: sum(1 for v in vs if v[2] == s)  # noqa: E731
    return {
        "top": top, "phases": sorted({v[1] for v in at_top}),
        "top_w": count(at_top, "w"), "top_b": count(at_top, "b"),
        "past_w": count(past, "w"), "past_b": count(past, "b"),
    }


def forecast_line(f, reach):
    s083 = "can pick" if f["top_w"] >= 2 and f["top_b"] >= 2 else "REFUSES"
    proposed = "can pick" if f["past_w"] >= 2 and f["past_b"] >= 2 \
        else "REFUSES"
    short = "" if f["top"] >= reach else f", SHORT of the reach {reach}/24"
    return (f"largest {f['top']}/24{short}; at it {f['top_w']} w "
            f"{f['top_b']} b, phases {f['phases']}; past 69/24 "
            f"{f['past_w']} w {f['past_b']} b; S083's pick {s083}, "
            f"S136's proposed pick {proposed}")


def forecast(args):
    pins = read_rows(args.pins)
    want = [(three, phase, side, fen) for fen, phase, side, three in PINS]
    if sorted(pins) != sorted(want):
        raise Refused(f"{args.pins} is not the four pins as PINS records "
                      "them: the weights have moved since S136's phase 1, "
                      "and PINS and this forecast are stale")
    print("pins: the four as PINS records them")
    rows = read_rows(args.rows)
    print(f"{len(rows)} rows past 46/24, {len({r[3] for r in rows})} "
          "distinct positions")
    for sign, name in ((1, "positive"), (-1, "negative")):
        pair = "(spread, 0)" if sign > 0 else "(-spread, 0)"
        print(f"tempo {name}, both weights of that sign: the pair {pair}; "
              "every such pair with the same mg - eg modulo 24 reads the "
              "same, a mixed-sign pair is read by `guard --pair`")
        for spread in range(PHASE_MAX):
            mg = sign * spread
            a = arithmetic(mg, 0)
            f = forecast_of(rows, mg, 0)
            print(f"  spread {spread:2d}  gcd {a['g']:2d}  reach "
                  f"{a['reach']}/24  " + forecast_line(f, a["reach"]))
    return 0


def print_forecasts(paths, mg, eg, reach):
    """One forecast line per rows file: the corpus's, and S082's training
    views' when the forecast stage read them."""
    for path in paths or []:
        print(f"  forecast, {os.path.basename(path)}: " + forecast_line(
            forecast_of(read_rows(path), mg, eg), reach))


def guard(args):
    mg, eg = pair_of(args)
    a = print_guard(mg, eg)
    print_forecasts(args.rows, mg, eg, a["reach"])
    return 0


def read_scan(path):
    """truncation_scan's stdout: `difference<TAB>phase NN<TAB>side<TAB>fen`,
    each position once at its first line, in the scan's order."""
    hits, seen = [], set()
    for line in open(path, encoding="utf-8"):
        fields = line.rstrip("\n").split("\t")
        if len(fields) != 4 or not fields[1].startswith("phase"):
            continue
        fen = fields[3]
        if fen in seen:
            continue
        seen.add(fen)
        hits.append((float(fields[0]), int(fields[1].split()[1]), fields[2],
                     fen, len(hits)))
    return hits


def pick(args):
    mg, eg = pair_of(args)
    a = arithmetic(mg, eg)
    if not a["rounds"]:
        raise Refused("tempo's division is exact at this pair: three "
                      "divisions round, the pins stand or are re-picked by "
                      "adocs/data/S083_truncation_pick.py, not by this rule")
    # Past 69/24 on the lattice, the per-pin threshold's own room: a row at
    # exactly 69/24 stays out whichever way its ulp falls.
    hits = [h for h in read_scan(args.scan) if h[0] > PER_PIN_FOUR]
    if not hits:
        raise Refused(f"{args.scan} holds no position past 69/24")
    top = max(h[0] for h in hits)
    anchor = next(h for h in hits if h[0] == top)
    lo = min(h[1] for h in hits)
    hi = max(h[1] for h in hits)
    targets = [lo, lo + (hi - lo) / 3.0, lo + 2.0 * (hi - lo) / 3.0, hi]
    sides = ["b", "w", "b", "w"]
    chosen = [None] * 4
    mine = [i for i in range(4) if sides[i] == anchor[2]]
    slot = min(mine, key=lambda i: (abs(anchor[1] - targets[i]), i))
    chosen[slot] = anchor
    for i in range(4):
        if chosen[i] is not None:
            continue
        free = [h for h in hits if h[2] == sides[i] and h not in chosen]
        if not free:
            raise Refused(f"no position past 69/24 with {sides[i]} to move "
                          f"for the target phase {targets[i]:.2f}")
        chosen[i] = min(free, key=lambda h: (abs(h[1] - targets[i]), -h[0],
                                             h[4]))
    print(f"{len(hits)} positions past 69/24, phases {lo}..{hi}, targets "
          + ", ".join(f"{t:.2f}" for t in targets) + f"; the largest "
          f"{top:.6f}, the arithmetic reach {a['reach']}/24 = "
          f"{a['reach'] / 24:.6f}" +
          ("" if round(top * PHASE_MAX) >= a["reach"] else
           " -- SHORT of it (see `short` for what that stops)"))
    for h in chosen:
        print(f"{h[0]:.6f}\tphase {h[1]:2d}\t{h[2]}\t{h[3]}")
    for h in chosen:
        print(f'    "{h[3]}",')
    worst = max(h[0] for h in chosen)
    least = min(h[0] for h in chosen)
    print(f"thresholds: each past 69/24 + 1e-6 (smallest {least:.6f}); the "
          f"worst {worst:.6f} past {a['worst']:.6f}: "
          + ("met" if worst > a["worst"] else "NOT met"))
    return 0


def short(args):
    """Whether the scan's largest residual reaches the arithmetic reach at
    the pair. Short of it, DEC-322 (3) is unruled: phase 2 stops before the
    re-targeting and the owner rules. Exit 0 either way; 1 on no row."""
    mg, eg = pair_of(args)
    a = arithmetic(mg, eg)
    hits = read_scan(args.scan)
    if not hits:
        raise Refused(f"{args.scan} holds no row")
    top = max(h[0] for h in hits)
    # The scan prints six decimals, so 92/24 reads 3.833333: compare on the
    # lattice the residuals live on, not on the printed value.
    if round(top * PHASE_MAX) >= a["reach"]:
        print(f"REACHED: the largest residual {top:.6f} is the arithmetic "
              f"reach {a['reach']}/24 at this pair")
    else:
        print(f"STOP: the largest residual {top:.6f} falls short of the "
              f"arithmetic reach {a['reach']}/24 = {a['reach'] / 24:.6f} at "
              "this pair; DEC-322 (3) is unruled, so phase 2 stops before "
              "the guard is re-targeted and the owner rules")
    return 0


def read(args):
    directory = args.dir
    print(f"S136 screen reading, {directory}")
    problems = []

    full = read_fit(os.path.join(directory, "full_tempo.log"))
    quarter = [read_fit(os.path.join(directory, f"quarter_s{s}_tempo.log"))
               for s in SEEDS]

    start, best, k, _improved, rows, _ = full
    print(f"baseline: held-out {start:.6f} at the shipped constants, K {k}, "
          f"{rows} rows")

    gain = start - best
    gains = [q[0] - q[1] for q in quarter]
    s = statistics.stdev(gains)
    m = max(PICK_Z * s, FLOOR)
    if s > SPREAD_UNREADABLE:
        problems.append(f"s {s:.6f} above {SPREAD_UNREADABLE}")
    keep = gain > m
    print(f"{'term':<16}{'gain':>11}{'s':>11}{'m':>11}  verdict   "
          "quarter gains, seeds 1 to 5")
    print(f"{'tempo':<16}{gain:>+11.6f}{s:>11.6f}{m:>11.6f}  "
          f"{'kept   ' if keep else 'dropped'}   "
          + " ".join(f"{g:+.6f}" for g in gains))

    pairs = {}
    for name in ["full"] + [f"quarter_s{s}" for s in SEEDS]:
        header = os.path.join(directory, f"{name}_tempo.hpp")
        if os.path.exists(header):
            w = weights_of(open(header).read())
            pairs[name] = (w["tempo_mg"][0], w["tempo_eg"][0])
    print("fitted pairs, mg / eg: " + ", ".join(
        f"{n} {p[0]} / {p[1]}" for n, p in pairs.items()))
    print("  beside, deciding nothing: S027's --only fit mg 10 / eg 0, "
          "S065's unfrozen fit mg 39 / eg 21; the label-side size near "
          "3.6 cp on S100's corpus and near -8.7 cp on this one (S135's "
          "audit), both confounded")
    if "full" in pairs:
        mg, eg = pairs["full"]
        a = print_guard(mg, eg)
        print_forecasts(args.rows, mg, eg, a["reach"])
    else:
        problems.append("the full fit wrote no header")

    if problems:
        print("outcome, unreadable: " + "; ".join(problems))
        print("SCREEN-OUTCOME unreadable")
        return 1
    if not keep:
        print("outcome: tempo does not lower held-out loss past its margin; "
              "recorded with its figures as zero, no match, the guard "
              "unchanged")
        print("SCREEN-OUTCOME none")
        return 0
    if pairs["full"] == (0, 0):
        print("outcome: kept, but the full fit rounds to the shipped 0 / 0, "
              "so there is nothing to paste and no match is owed (S076's "
              "rule 3); recorded as zero with its figures")
        print("SCREEN-OUTCOME none")
        return 0
    print("candidate: the full-corpus fit itself, full_tempo.hpp -- a family "
          "of one has no survivors to refit together, and the joint fit's "
          "command would be this one")
    print("SCREEN-OUTCOME candidate tempo")
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
        want = shipped[name]
        if len(values) != len(want):
            bad.append(f"{name}: {len(values)} values emitted, {len(want)} "
                       "at HEAD")
            continue
        if args.free and name in TEMPO:
            continue
        differ = [i for i, (x, y) in enumerate(zip(values, want)) if x != y]
        if differ:
            bad.append(f"{name}: {len(differ)} frozen values differ from "
                       f"HEAD's, the first at index {differ[0]}")
    for name in TEMPO:
        if name not in emitted:
            bad.append(f"{name}: not emitted")
    head = subprocess.run(["git", "-C", args.tree, "rev-parse", "--short",
                           "HEAD"], check=True, capture_output=True,
                          text=True).stdout.strip()
    print(f"start-check {args.header}: {len(emitted)} definitions against "
          f"HEAD {head}, free "
          f"{'tempo_mg, tempo_eg' if args.free else '(none)'}")
    if bad:
        raise Refused("the tuner did not start from HEAD's constants: "
                      + "; ".join(bad))
    print("start-check: every frozen value is HEAD's")
    return 0


def only_tempo(args):
    tree = args.tree
    before = head_weights(tree)
    after = {}
    for path in ("src/eval_tables.hpp", "src/evaluation.cpp"):
        after.update(weights_of(open(os.path.join(tree, path)).read()))
    moved = sorted(n for n in set(before) | set(after)
                   if before.get(n) != after.get(n))
    print("moved: " + (", ".join(
        f"{n} {before.get(n)} -> {after.get(n)}" for n in moved)
        or "(nothing)"))
    others = [n for n in moved if n not in TEMPO]
    if others:
        raise Refused("the paste moved more than the tempo weights: "
                      + ", ".join(others))
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S136's screen helpers.")
    commands = parser.add_subparsers(dest="command", required=True)

    rule = commands.add_parser("read")
    rule.add_argument("dir")
    rule.add_argument("--tree", required=True)
    rule.add_argument("--rows", action="append")

    start = commands.add_parser("start-check")
    start.add_argument("header")
    start.add_argument("--tree", required=True)
    start.add_argument("--free", action="store_true")

    paste = commands.add_parser("only-tempo")
    paste.add_argument("tree")

    for name in ("guard", "pick", "short"):
        sub = commands.add_parser(name)
        if name in ("pick", "short"):
            sub.add_argument("scan")
        which = sub.add_mutually_exclusive_group(required=True)
        which.add_argument("--header")
        which.add_argument("--pair", nargs=2, type=int, metavar=("MG", "EG"))
        if name == "guard":
            sub.add_argument("--rows", action="append")

    ahead = commands.add_parser("forecast")
    ahead.add_argument("rows")
    ahead.add_argument("--pins", required=True)

    args = parser.parse_args(argv)
    handlers = {"read": read, "start-check": start_check,
                "only-tempo": only_tempo, "guard": guard,
                "forecast": forecast, "pick": pick, "short": short}
    try:
        return handlers[args.command](args)
    except (Refused, OSError, subprocess.CalledProcessError) as why:
        print(f"S136_screen: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
