#!/usr/bin/env python3
"""S132, census P3. What share of the root's own nodes chesso's best move takes.

The two constants of the node-fraction time manager are **(b) under DEC-105**:
a derivation over chesso's own tree, not a number from any engine. There is no
(a) to take -- the wiki's Time Management page names the ratio of the best
move's subtree to the whole tree as a consideration and states no method and
no number -- and every figure the open-source record carries for this
technique is another engine's tuned constant, which DEC-084 as amended by
DEC-105 refuses as a seed wherever it is republished.

So the pair is solved from two constraints of chesso's own, over the median
this script measures:

    factor_pct(f)  = (TmNodeBasePct - 100 f) * TmNodeScalePct / 100
    factor_pct(f_med) = 100                  spend today's allocation in
                                             expectation, so the SPRT measures
                                             a redistribution of the clock and
                                             not a longer one
    factor_pct(1)  = TmScaleMinPercent       the rule's hardest cut lands on
                                             the floor chesso already has

    TmNodeScalePct = 7000 / (100 - 100 f_med)
    TmNodeBasePct  = 100 + 3000 / TmNodeScalePct

THE PICK. The same 300 positions src/search_params.hpp's aspiration rows were
chosen over: 100 per offset from adocs/data/S018_raw.tsv, four per
game_phase() value that has at least four rows, at offsets 0, 1 and 2. It
imports adocs/data/S021_aspiration_sweep.py rather than copying its picker --
this directory is append-only and a second copy of a sampling rule is a second
sampling rule (the S148 and S198 precedent).

THE INSTRUMENT. build/tools/node_share_census, on the release build with the
counting half of this step in it, at `go depth 12` with a cold table per
position. The share it reports is the engine's own integer percentage, the
number the time manager reads.

    cmake --build build -j12 --target node_share_census
    adocs/data/S132_node_share_census.py build/tools/node_share_census \\
        --depth 12 --out adocs/data/S132_node_share_census.tsv

It prints the distribution -- the median, the quartiles, the deciles, the
share of positions at 100 % -- and the pair those constraints then produce.
Nodes at a fixed depth are not Elo and a distribution is not Elo either
(DEC-019): what this decides is where the sweep starts, and the SPRT is what
decides whether the rule is worth anything.
"""
import argparse
import os
import statistics
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PICK = os.path.join(HERE, "S021_aspiration_sweep.py")
CORPUS = "adocs/data/S018_raw.tsv"

OFFSETS = (0, 1, 2)


def s021_positions(offset):
    """S021's own picker at one offset, executed rather than copied.

    That file cannot be imported: it reads argv and runs a whole sweep at
    module level, engine subprocess and all. So its source is executed up to
    the line where the sweep itself begins, which leaves exactly the pick --
    the corpus path, PER_PHASE, and positions(). A copy of the rule here
    would be a second sampling rule, which is what the S148 and S198
    precedent in this directory is against.
    """
    with open(PICK) as handle:
        source = handle.read()

    marker = "\nfens = positions()"

    if marker not in source:
        raise SystemExit(
            PICK + " no longer begins its sweep at `fens = positions()`; "
            "this script cuts it there and must be re-pointed")

    namespace = {"__name__": "S021_pick"}
    saved = sys.argv
    sys.argv = [PICK, "unused-engine", "11", str(offset)]

    try:
        exec(compile(source[:source.index(marker)], PICK, "exec"), namespace)
    finally:
        sys.argv = saved

    return namespace["positions"]()

# chesso's own floor on a scaled soft limit, and the second constraint's right
# hand side. Read from the engine's parameter list rather than written out, so
# this script cannot disagree with src/search_params.hpp about it.
SEARCH_PARAMS = os.path.join(HERE, "..", "..", "src", "search_params.hpp")


def param_row(name):
    """(default, min, max) for one X-macro row of src/search_params.hpp."""
    quoted = '"%s"' % name

    with open(SEARCH_PARAMS) as handle:
        for line in handle:
            if quoted not in line:
                continue

            rest = line.split(quoted, 1)[1]
            rest = rest.split(")")[0].strip().lstrip(",")
            fields = [f.strip() for f in rest.split(",")]

            return int(fields[0]), int(fields[1]), int(fields[2])

    raise SystemExit(name + " is not in " + SEARCH_PARAMS)


def scale_min_percent():
    """TmScaleMinPercent's default, which is the second constraint's target."""
    return param_row("TmScaleMinPercent")[0]


def positions():
    """The 300, in the order the three offsets produce them."""
    if not os.path.exists(CORPUS):
        raise SystemExit(
            "cannot see " + CORPUS + " -- S021's picker reads it by a path "
            "relative to the repository root, so run this from there")

    picked = []

    for offset in OFFSETS:
        picked += s021_positions(offset)

    return picked


def seeds(f_med, floor_pct):
    """The two constants the two constraints give at this median."""
    if f_med >= 1.0:
        raise SystemExit(
            "every position spent the whole root on its best move, which "
            "leaves the first constraint with no solution -- look at the "
            "census before seeding anything")

    scale = round(7000.0 / (100.0 - 100.0 * f_med))

    if floor_pct != 30:
        # The 7000 and the 3000 are TmScaleMinPercent's 30 and the neutral
        # 100 written out: 100 * (100 - floor) and 100 * floor. Re-derived
        # here so a change to the floor cannot leave this script quoting
        # arithmetic that no longer follows from it.
        scale = round(100.0 * (100 - floor_pct) / (100.0 - 100.0 * f_med))

    base = round(100.0 + (100.0 * floor_pct) / scale)

    return int(base), int(scale)


def factor_pct(share_pct, base, scale):
    """The engine's own integer arithmetic, so the check below is its check."""
    return ((base - share_pct) * scale) // 100


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("instrument",
                        help="build/tools/node_share_census")
    parser.add_argument("--depth", type=int, default=12)
    parser.add_argument("--hash", type=int, default=16)
    parser.add_argument("--out", default=None,
                        help="where to write the per-position rows")
    args = parser.parse_args()

    fens = positions()
    print("# %d positions, depth %d, instrument %s"
          % (len(fens), args.depth, args.instrument), file=sys.stderr)

    run = subprocess.run(
        [args.instrument, "--depth", str(args.depth), "--hash", str(args.hash)],
        input="\n".join(fens) + "\n", text=True, capture_output=True)

    if run.returncode != 0:
        sys.stderr.write(run.stderr)
        raise SystemExit("the instrument exited %d" % run.returncode)

    rows = run.stdout.splitlines()
    header, rows = rows[0], rows[1:]

    if args.out:
        with open(args.out, "w") as handle:
            handle.write(header + "\n" + "\n".join(rows) + "\n")

    shares = []
    for row in rows:
        field = row.split("\t")
        shares.append(int(field[-1]))

    if len(shares) != len(fens):
        print("# %d rows for %d positions -- some were refused"
              % (len(shares), len(fens)), file=sys.stderr)

    shares.sort()
    quartiles = statistics.quantiles(shares, n=4, method="inclusive")
    deciles = statistics.quantiles(shares, n=10, method="inclusive")
    f_med = statistics.median(shares) / 100.0
    floor_pct = scale_min_percent()
    base, scale = seeds(f_med, floor_pct)

    print("positions      %d" % len(shares))
    print("share min      %d %%" % shares[0])
    print("share q1       %.1f %%" % quartiles[0])
    print("share median   %.1f %%" % quartiles[1])
    print("share q3       %.1f %%" % quartiles[2])
    print("share max      %d %%" % shares[-1])
    print("share mean     %.2f %%" % statistics.fmean(shares))
    print("deciles        %s" % " ".join("%.0f" % d for d in deciles))
    print("at 100 %%       %d position(s)" % sum(1 for s in shares if s == 100))
    print("at 0 %%         %d position(s)" % sum(1 for s in shares if s == 0))
    print()
    print("f_med          %.4f" % f_med)
    print("TmScaleMinPct  %d  (read from src/search_params.hpp)" % floor_pct)
    print("TmNodeScalePct %d" % scale)
    print("TmNodeBasePct  %d" % base)
    print()

    # THE RANGES ARE PART OF THE ANSWER. A high median needs a steep slope to
    # still reach the floor at a share of 100 %, and past a median of about
    # 0.77 the slope the second constraint asks for is outside the range this
    # step declared for it. That is a decision and not a clamp: either the
    # range's stated purpose is re-derived from this census, or the second
    # constraint is relaxed and the rule's hardest cut lands above the floor.
    # Said here, in the run's own output, so it cannot be discovered after the
    # seeds are already in the tree.
    for name, value in (("TmNodeBasePct", base), ("TmNodeScalePct", scale)):
        _, low, high = param_row(name)

        if value < low or value > high:
            print("!! %s %d is outside its declared range [%d, %d]"
                  % (name, value, low, high))
            print("!! the census, not the clamp, decides what happens next -- "
                  "see this step's file")

    print()

    # The two constraints, checked on the rounded integers the engine will
    # actually compile -- rounding moves both of them a point or two and the
    # step's stamp records where they landed rather than claiming they are
    # exact.
    median_pct = int(round(100 * f_med))
    print("factor at the median share  %d %%  (constraint: 100)"
          % factor_pct(median_pct, base, scale))
    print("factor at a share of 100 %%  %d %%  (constraint: %d)"
          % (factor_pct(100, base, scale), floor_pct))
    print("factor at a share of 0 %%    %d %%" % factor_pct(0, base, scale))


if __name__ == "__main__":
    main()
