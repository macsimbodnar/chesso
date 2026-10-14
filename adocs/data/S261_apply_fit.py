#!/usr/bin/env python3
"""Write S261's fitted vector into the defaults, holding the axes item (4) holds.

    python3 adocs/data/S261_apply_fit.py <run dir> [--hold NAME[,NAME...]] [--dry-run]

`<run dir>` is the lane's `$OUT`, `.tuning/spsa_s261_<stamp>`. The step file's
pre-registration, item (4), decides which axes are held; this script applies
that decision and refuses to apply a vector the decision has not been made for.

WHERE THE VECTOR COMES FROM: `<run dir>/checkpoint.json`, rounded with the
driver's own `Param.uci_value` (clamp, then Python's `round`). The checkpoint is
the driver's state after its last iteration, rewritten atomically every
iteration, and it carries the two facts that make it this lane's finished run:
`k` equal to the config's 1250 iterations, and the config digest the driver
itself would compute for `tools/spsa_s261.json`. The trajectory's last row is
the same `uci_value` of the same theta, written one step earlier in the same
iteration, so it is read as a **cross-check** and any disagreement refuses: a
row without its checkpoint is the one state a kill leaves behind, and the
driver itself drops such a row on resume. The run log's JSON block, which
S231's script read, is the same numbers again but lives outside the run
directory and is not needed.

WHAT IT WRITES, and the same three places in a dry run, without writing:

  src/search_params.hpp       the default of each axis's X-macro row -- the one
                              list both builds are generated from (a constexpr
                              in the release build, the initial value of the
                              variable in the tune build), so one edit is both
                              builds' form. The min and max fields never move.
  tests/test_search_params.cpp  the default column of `golden_defaults`, the
                              DEC-142 pin whose derivation is the header above;
                              its min and max columns are asserted equal to the
                              config's and never written.
  MANUAL.md                   the default cell of the axis's option-table row,
                              which `tools/plan_prose_check.py --params` holds
                              against the compiled default.

Each axis's current value in all three must be its incumbent (the config's
`start`) or its fitted value -- any other value means the tree moved under
the lane, and the script refuses. That makes it re-runnable with different
`--hold` sets, which is what item (4)'s R4 halving needs.

Column alignment in the X-macro table is kept: the default field keeps its
width where the new number fits, right-aligned rows stay right-aligned, and
the continuation backslash stays in its column; a row that cannot be kept so
is refused by name.

THE RULE COLUMN, item (4)'s, computed and not chosen here:

  unmoved   fitted == incumbent, not at a bound
  R1        fitted == incumbent at a bound (LmpDepthCoeff seeded at its floor)
  R2        fitted at the axis's declared off value -- **must be held**; the
            script refuses to write until every R2 axis is in --hold
  R3        fitted at a bound that is not an off value; applied, and named in
            S261_sprt.sh's vector table as "at its range end"
  moved     fitted inside the range

and `held` beside it for every axis in --hold (R2 by rule, or R4's culprit).

It then names, without editing, what moves by hand in the same landing: every
other mention of a moved axis in MANUAL.md, DEV_MANUAL.md, adocs/specs.md and
src/search_params.hpp's comments, the narrative comment above
`golden_defaults`, and the search goldens DEC-142 says move with pruning and
reduction. Stdlib and tools/spsa_driver.py only.
"""

import argparse
import json
import os
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import spsa_driver  # noqa: E402  -- the driver's own rounding and digest

CONFIG = ROOT / "tools" / "spsa_s261.json"
HPP = ROOT / "src" / "search_params.hpp"
GOLD = ROOT / "tests" / "test_search_params.cpp"
MANUAL = ROOT / "MANUAL.md"
PROSE = [ROOT / "MANUAL.md", ROOT / "DEV_MANUAL.md", ROOT / "adocs" / "specs.md",
         ROOT / "src" / "search_params.hpp"]

# Item (4) R2: the declared off value of every axis that has one inside its
# range, from each parameter's comment in src/search_params.hpp.
OFF = {
    "LmrCutNode": 0, "LmrNotImproving": 0, "LmrTtCapture": 0, "LmrPv": 0,
    "LmrNoTtMove": 0,
    "LmpMaxLmrDepth": 0, "FutMaxLmrDepth": 0, "HistPruneMaxLmrDepth": 0,
    "SeeQuietMaxLmrDepth": 0, "SeeCaptureMaxLmrDepth": 0,
    "SeeLmrExtra": 0, "AspirationFailLowPull": 0,
    "LmrDeeperMinReduction": 126, "LmpBase": 27000, "FutBase": 48000,
}

# DEC-142 goldens in the fast suite whose sites say they move legitimately on
# a change to pruning, reduction, extensions or ordering, with the script each
# names. Named here for the landing; re-derived only on the trigger each site
# states, never re-read from a run.
SEARCH_GOLDENS = [
    ('tests/test_search.cpp', '"ordering keeps the tree small" band 65024 / 3251',
     'python3 adocs/data/S192_node_budget.py'),
    ('tests/test_search.cpp', 'mined mate row, depth 8, distance 2 (R01)',
     'adocs/data/S095_mine_mate_row.py, adocs/data/S230_mine_r01_row.py'),
    ('tests/test_search.cpp', 'multicut mate row, depth 14 (E21)',
     "adocs/data/S097_mine_mate_row.py's header"),
    ('tests/test_search.cpp', 'ProbCut mate row, depth 11, distance 6 (B04)',
     "adocs/data/S277_mine_mate_row.py's header"),
    ('tests/test_search.cpp', 'the table of mined depths and mutant labels',
     'adocs/data/S230_table_fens.txt and its script'),
    ('tests/test_engine.cpp', 'first_mate_depth 10 and 9',
     'its site in "engine: mate safety"'),
    ('tests/test_engine.cpp', 'mate-in-three floor 11',
     'python3 adocs/data/S154_floor_margin_sweep.py floor'),
    ('tests/test_mate_breadth.cpp', 'breadth floor 145',
     'python3 adocs/data/S156_mined_floor_sweep.py --floor 145'),
    ('tests/test_mate_carry.cpp', 'short-PV ceilings 6, 15, 5, 2, 11, 5',
     'adocs/data/S203_case_sweep.sh --ceilings'),
]

HPP_ROW = (r'^(\s*X\(\s*[A-Z_0-9]+,\s*"{name}",)( *)(-?\d+),( *)'
           r'(-?\d+), *(-?\d+)\)( *)(\\?)$')
GOLD_ROW = r'^(\s*\{{"{name}",)( +)(-?\d+),( +)(-?\d+),( +)(-?\d+)\}},\s*$'
MANUAL_ROW = (r'^(\|\s*`{name}`\s*\|\s*)(-?\d+)(\s*\|\s*)(-?\d+)( to )(-?\d+)'
              r'(\s*\|.*)$')


class Refused(Exception):
    pass


def load_vector(rundir, cfg, raw):
    frozen_path = os.path.join(rundir, "run.json")
    ck_path = os.path.join(rundir, "checkpoint.json")
    tj_path = os.path.join(rundir, "trajectory.tsv")
    for path in (frozen_path, ck_path, tj_path):
        if not os.path.exists(path):
            raise Refused(f"{path} does not exist; is {rundir} the lane's $OUT?")
    with open(frozen_path) as fh:
        frozen = json.load(fh)
    if frozen.get("config") != raw or frozen.get("sim") is not None \
            or frozen.get("flip_sign"):
        raise Refused(f"{frozen_path} is not a run of {CONFIG.relative_to(ROOT)} "
                      "as it stands: a config edited after launch, a simulator "
                      "run or a sign-flipped one")
    with open(ck_path) as fh:
        ck = json.load(fh)
    if ck.get("digest") != cfg.digest(None, False):
        raise Refused(f"{ck_path}'s digest is not this config's")
    if int(ck.get("k", -1)) != cfg.iterations:
        raise Refused(f"{ck_path} is at iteration {ck.get('k')} of "
                      f"{cfg.iterations}: the lane has not finished")
    theta = [float(x) for x in ck["theta"]]
    if len(theta) != len(cfg.params):
        raise Refused(f"{ck_path} has {len(theta)} axes, the config "
                      f"{len(cfg.params)}")
    fitted = {p.name: p.uci_value(t) for p, t in zip(cfg.params, theta)}

    with open(tj_path) as fh:
        lines = [ln.rstrip("\n").split("\t") for ln in fh if ln.strip()]
    header, last = lines[0], lines[-1]
    if int(last[0]) != cfg.iterations - 1:
        raise Refused(f"{tj_path}'s last row is iteration {last[0]}, not "
                      f"{cfg.iterations - 1}")
    row = dict(zip(header, last))
    bad = [n for n in fitted if int(row[n]) != fitted[n]]
    if bad:
        raise Refused(f"the trajectory's last row disagrees with the checkpoint "
                      f"on {bad}; the two are the same rounding of one theta")
    return fitted


def classify(p, fitted):
    inc = int(p.start)
    lo, hi = int(p.lo), int(p.hi)
    at_bound = fitted in (lo, hi)
    if fitted == inc:
        return "R1" if at_bound else "unmoved"
    if at_bound and OFF.get(p.name) == fitted:
        return "R2"
    if at_bound:
        return "R3"
    return "moved"


def rewrite_hpp_row(line, name, new):
    m = re.match(HPP_ROW.format(name=re.escape(name)), line)
    head, pre, old, post, lo, hi, tail, bs = m.groups()
    width = len(pre) + len(old) + 1 + len(post)
    if len(post) == 1 and len(pre) > 1:      # right-aligned default column
        new_pre, new_post = max(1, width - len(new) - 2), 1
    else:                                    # left-aligned or single-spaced
        new_pre, new_post = len(pre), max(1, width - len(pre) - len(new) - 1)
    lohi = line[m.start(5):m.end(6) + 1]     # "lo, hi)" as written
    body_old = line[:m.start(7)]
    body_new = head + " " * new_pre + new + "," + " " * new_post + lohi
    if bs:
        new_tail = len(tail) - (len(body_new) - len(body_old))
        if new_tail < 1:
            raise Refused(f"{name}: {old} -> {new} would push the continuation "
                          "backslash out of its column")
        return body_new + " " * new_tail + bs
    return body_new + tail


def apply_file(path, pattern, names, values, incumbents, fitted, check_range,
               rewrite):
    """Return the file's new text, or raise. `values` is what each axis gets."""
    lines = path.read_text().split("\n")
    found = {}
    for i, line in enumerate(lines):
        for name in names:
            m = re.match(pattern.format(name=re.escape(name)), line)
            if m:
                if name in found:
                    raise Refused(f"{path.relative_to(ROOT)}: two rows for {name}")
                found[name] = (i, m)
    missing = [n for n in names if n not in found]
    if missing:
        raise Refused(f"{path.relative_to(ROOT)} has no row for {missing}")
    changes = []
    for name in names:
        i, m = found[name]
        have = int(check_range(name, m))
        if have not in (incumbents[name], fitted[name]):
            raise Refused(f"{path.relative_to(ROOT)}: {name} is {have}, neither "
                          f"its incumbent {incumbents[name]} nor its fitted "
                          f"{fitted[name]} -- the tree moved under the lane")
        if have != values[name]:
            new_line = rewrite(lines[i], name, str(values[name]), m)
            changes.append((i + 1, lines[i], new_line))
            lines[i] = new_line
    return "\n".join(lines), changes


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("rundir")
    ap.add_argument("--hold", default="",
                    help="comma-separated axes held at their incumbent (R2, R4)")
    ap.add_argument("--dry-run", action="store_true",
                    help="print everything, write nothing")
    args = ap.parse_args()

    with open(CONFIG) as fh:
        raw = json.load(fh)
    cfg = spsa_driver.Config(raw)
    params = {p.name: p for p in cfg.params}
    names = [p.name for p in cfg.params]

    try:
        hold = [h.strip() for h in args.hold.split(",") if h.strip()]
        unknown = [h for h in hold if h not in params]
        if unknown:
            raise Refused(f"--hold names axes not in {CONFIG.relative_to(ROOT)}: "
                          f"{unknown}")
        fitted = load_vector(args.rundir, cfg, raw)
    except Refused as exc:
        print(f"REFUSED: {exc}")
        return 1

    incumbents = {n: int(params[n].start) for n in names}
    rules = {n: classify(params[n], fitted[n]) for n in names}
    applied = {n: (incumbents[n] if n in hold else fitted[n]) for n in names}

    print(f"{'axis':24s} {'incumbent':>9s} {'fitted':>7s} {'applied':>7s}  rule")
    for n in names:
        mark = " held" if n in hold else ""
        print(f"{n:24s} {incumbents[n]:9d} {fitted[n]:7d} {applied[n]:7d}  "
              f"{rules[n]}{mark}")
    moved = [n for n in names if applied[n] != incumbents[n]]
    print(f"{len(moved)} of {len(names)} axes move; held: {hold or 'none'}")

    unheld_r2 = [n for n in names if rules[n] == "R2" and n not in hold]
    if unheld_r2:
        print(f"REFUSED: R2 -- {unheld_r2} ended at the declared off value and "
              "the lane does not switch a rule off; add them to --hold")
        return 2
    r3 = [n for n in names if rules[n] == "R3" and n not in hold]
    if r3:
        print(f"R3, at a range end and applied -- name them in S261_sprt.sh's "
              f"vector table: {r3}")

    def hpp_range(name, m):
        lo, hi = int(m.group(5)), int(m.group(6))
        if (lo, hi) != (int(params[name].lo), int(params[name].hi)):
            raise Refused(f"src/search_params.hpp: {name}'s range [{lo}, {hi}] "
                          "is not the config's")
        return m.group(3)

    def gold_range(name, m):
        lo, hi = int(m.group(5)), int(m.group(7))
        if (lo, hi) != (int(params[name].lo), int(params[name].hi)):
            raise Refused(f"tests/test_search_params.cpp: {name}'s pinned range "
                          f"[{lo}, {hi}] is not the config's; ranges never move here")
        return m.group(3)

    def manual_range(name, m):
        lo, hi = int(m.group(4)), int(m.group(6))
        if (lo, hi) != (int(params[name].lo), int(params[name].hi)):
            raise Refused(f"MANUAL.md: {name}'s range {lo} to {hi} is not the "
                          "config's")
        return m.group(2)

    def gold_rewrite(line, name, new, m):
        width = len(m.group(2)) + len(m.group(3))
        return (line[:m.start(2)] + " " * max(1, width - len(new)) + new +
                line[m.end(3):])

    def manual_rewrite(line, name, new, m):
        return line[:m.start(2)] + new + line[m.end(2):]

    try:
        plans = [
            (HPP, HPP_ROW, hpp_range,
             lambda line, name, new, m: rewrite_hpp_row(line, name, new)),
            (GOLD, GOLD_ROW, gold_range, gold_rewrite),
            (MANUAL, MANUAL_ROW, manual_range, manual_rewrite),
        ]
        results = []
        for path, pattern, check_range, rewrite in plans:
            text, changes = apply_file(path, pattern, names, applied, incumbents,
                                       fitted, check_range, rewrite)
            results.append((path, text, changes))
    except Refused as exc:
        print(f"REFUSED: {exc}")
        return 1

    for path, text, changes in results:
        rel = path.relative_to(ROOT)
        print(f"\n{rel}: {len(changes)} row(s)"
              f"{' (dry run, not written)' if args.dry_run else ''}")
        for lineno, old, new in changes:
            print(f"  {lineno}: {old.strip()}")
            print(f"  {' ' * len(str(lineno))}  {new.strip()}")
        if changes and not args.dry_run:
            path.write_text(text)

    print("\nBY HAND IN THE SAME LANDING, named and not edited:")
    print("  tests/test_search_params.cpp: the narrative comment above "
          "golden_defaults gains S261's sentence (which axes moved, which held)")
    for path in PROSE:
        lines = path.read_text().split("\n")
        for n in moved:
            hits = [i + 1 for i, ln in enumerate(lines)
                    if re.search(r"\b" + re.escape(n) + r"\b", ln)
                    and not re.match(MANUAL_ROW.format(name=re.escape(n)), ln)
                    and not re.match(HPP_ROW.format(name=re.escape(n)), ln)]
            if hits:
                print(f"  {path.relative_to(ROOT)}: {n} at line(s) "
                      f"{', '.join(map(str, hits[:12]))}"
                      f"{' ...' if len(hits) > 12 else ''}")
    print("  then: python3 tools/plan_prose_check.py --params, which must print "
          "nothing")
    print("\nDEC-142 SEARCH GOLDENS that move legitimately on this change "
          "(re-derive on each site's own trigger, never by re-reading a run):")
    for path, what, how in SEARCH_GOLDENS:
        print(f"  {path}: {what} -- {how}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
