#!/usr/bin/env python3
"""S098 verdict 1: the distribution of the history sum at the sites the
reduction actually reads it, so `LmrHistDiv` rests on this engine's own data
and not on the arithmetic of the band's edge.

    ~/.venv/chess/bin/python adocs/data/S098_v1_hist_census.py census
    ~/.venv/chess/bin/python adocs/data/S098_v1_hist_census.py census --depth 12
    ~/.venv/chess/bin/python adocs/data/S098_v1_hist_census.py census --keep

WHY IT EXISTS. The first seed for `LmrHistDiv` was the saturated sum over the
clamp -- 17350 / 2 = 8675 -- which is a statement about what the history
tables *can* hold and not about what they *do* hold. The step's own ablation
then measured the term inert below depth 13 on the bench positions (+0.00 %
of the nodes at depths 9 to 11, -0.02 % at 12), because a sum only reaches
half its band after many cutoffs on one move. An `8+0.08` game lives at
depths 10 to 14, so an SPRT at that seed would price the seed and not the
technique. This census measures the sum where the rule reads it, and the
divisor is re-derived from a percentile of it.

WHAT IS COUNTED, AND WHERE. One site is one call of
`lmr_adjusted_reduction` in `negamax_at` -- that is, one quiet move that
reached the reduction with the node's own eligibility satisfied: `ply > 0`,
`depth >= 3`, past the third legal move, neither side in check, the move
neither a capture nor a promotion. Nothing else is a site: a move the gate
priced but the reduction never reached is not one, and neither is a capture.
For each site the instrumentation records the signed `hist_sum` the rule read
and whether the **raw** table reduction was non-zero at that (depth, move
number) -- the second so that "the term had a reduction to move" can be read
separately from "the term was consulted".

Stored as a histogram over the signed sum and not as a row per site: the
sites run to tens of millions and the only questions asked of them are
percentiles and a fraction. The histogram is exact to the unit, so the
percentiles are exact.

THE POSITIONS ARE S024'S AND THE DRIVER IS S024'S. `adocs/data/S024_census_positions.txt`
-- 400 positions, one per game, from `adocs/data/S219_aa_calibration.pgn`,
`random.Random(24)` shuffle, each game's own midpoint ply -- read through
that file's own `read_positions()`, and driven through its `Engine` class:
one persistent UCI process, `ucinewgame` between positions, `go depth N`
awaited to `bestmove`. TOOLCHAIN.md's oracle trap is general and that driver
is the form that does not fall into it. Reusing them rather than writing a
second sampler is the point: the two censuses are then over the same board
positions and can be read beside each other.

`go depth N` and not a single `search()` call, deliberately: a game's `go`
is iterative deepening from a history table that
`iterative_deepening_search()` zeroes at its top, so the sums a real move
reads are the ones its own shallower iterations built -- which is exactly
what a fixed-depth cold call would not show.

THE BINARY IS A THROWAWAY AND THE SHIPPED TREE IS NEVER TOUCHED. A detached
worktree is created under `.ref-builds/`, the **working tree's** `src/` is
copied into it -- so what is measured is the landing and not `HEAD` -- the
patch below is applied there, and the worktree is removed when the run ends.
The instrumentation is not in the shipping engine and never was: it writes
one file named by `CHESSO_HIST_CENSUS` at exit and prints nothing, so it is
not on the UCI surface `tests/test_uci_surface.cpp` guards. The patch is in
this file rather than in a diff beside it so the run is reproducible from the
script alone.

WHAT IT IS NOT. Not a strength measurement, and nothing here reads it as one
(DEC-019). It sizes a seed; `adocs/data/S098_v1_sprt.sh` decides the step.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import time

REPO = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(REPO, "adocs", "data"))

import S024_census_run as s024  # noqa: E402  (path set above)

OUT_TXT = os.path.join(REPO, "adocs", "data", "S098_v1_hist_census.txt")
DEPTHS = (10, 12)

# The instrumentation. One anchor per pair, each occurring exactly once in the
# file it names, applied to the copy in the throwaway worktree and never to the
# tree this script runs from.
COUNTERS = r'''
// ---- S098 verdict-1 history census, throwaway instrumentation ----------
// Not in the shipping engine: adocs/data/S098_v1_hist_census.py patches it in
// to a detached worktree, builds there, and removes the worktree afterwards.
// One site is one call of lmr_adjusted_reduction below.
namespace
{
constexpr int HIST_CENSUS_BOUND = 40000;

struct hist_census_t
{
  uint64_t sites = 0;
  uint64_t raw_nonzero = 0;
  uint64_t clipped = 0;
  std::vector<uint64_t> all;
  std::vector<uint64_t> nonzero;

  hist_census_t()
      : all(2 * HIST_CENSUS_BOUND + 1, 0), nonzero(2 * HIST_CENSUS_BOUND + 1, 0)
  {}

  ~hist_census_t();
};

hist_census_t hist_census;

hist_census_t::~hist_census_t()
{
  const char* path = std::getenv("CHESSO_HIST_CENSUS");

  if (path == nullptr) { return; }

  std::ofstream out(path);

  if (!out) { return; }

  out << "sites\t" << hist_census.sites << '\n'
      << "raw_nonzero\t" << hist_census.raw_nonzero << '\n'
      << "clipped\t" << hist_census.clipped << '\n';

  for (int i = 0; i <= 2 * HIST_CENSUS_BOUND; ++i) {
    if (hist_census.all[i] == 0 && hist_census.nonzero[i] == 0) { continue; }

    out << "bin\t" << (i - HIST_CENSUS_BOUND) << '\t' << hist_census.all[i]
        << '\t' << hist_census.nonzero[i] << '\n';
  }
}

inline void hist_census_site(int sum, bool raw_nonzero)
{
  hist_census.sites++;

  if (raw_nonzero) { hist_census.raw_nonzero++; }

  int v = sum;

  if (v > HIST_CENSUS_BOUND || v < -HIST_CENSUS_BOUND) {
    hist_census.clipped++;
    v = (v > 0) ? HIST_CENSUS_BOUND : -HIST_CENSUS_BOUND;
  }

  hist_census.all[v + HIST_CENSUS_BOUND]++;

  if (raw_nonzero) { hist_census.nonzero[v + HIST_CENSUS_BOUND]++; }
}
}  // namespace
// ---- end S098 census ---------------------------------------------------

'''

PATCH = [
    # The counters, above the helper they count.
    ("// What the raw table above is worth once this move's history is taken "
     "into\n// account, S098 verdict 1.",
     COUNTERS
     + "// What the raw table above is worth once this move's history is taken "
       "into\n// account, S098 verdict 1."),
    # The site itself: every call of the helper from the reduction.
    ("        reduction = lmr_adjusted_reduction(\n"
     "            depth, static_cast<int>(legal_moves_counter), hist_sum);",
     "        hist_census_site(\n"
     "            hist_sum,\n"
     "            lmr_reduction(depth, static_cast<int>(legal_moves_counter)) !=\n"
     "                0);\n"
     "        reduction = lmr_adjusted_reduction(\n"
     "            depth, static_cast<int>(legal_moves_counter), hist_sum);"),
    # `<cstdlib>`, `<fstream>` and `<vector>` are not in scope in a release
    # build of this file -- the first two are inside its `#ifdef CHESSO_TUNE`
    # and the third is not included at all -- so the patch brings them.
    ("#include <limits>\n",
     "#include <limits>\n"
     "// S098 census, throwaway: see adocs/data/S098_v1_hist_census.py.\n"
     "#include <cstdlib>\n"
     "#include <fstream>\n"
     "#include <vector>\n"),
]


def sh(cmd, **kw):
    proc = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if proc.returncode != 0:
        sys.stderr.write(" ".join(cmd) + "\n" + proc.stdout[-4000:] +
                         proc.stderr[-4000:])
        sys.exit("command failed: " + " ".join(cmd))
    return proc.stdout


def build_instrumented(work):
    """A detached worktree carrying the working tree's src/, patched."""
    if os.path.exists(work):
        subprocess.run(["git", "worktree", "remove", "--force", work],
                       cwd=REPO, capture_output=True, text=True)
        shutil.rmtree(work, ignore_errors=True)
    subprocess.run(["git", "worktree", "prune"], cwd=REPO, capture_output=True)
    sh(["git", "worktree", "add", "--detach", work, "HEAD"], cwd=REPO)

    # The landing, not HEAD: every source file as it stands in the tree this
    # script is run from.
    for name in sorted(os.listdir(os.path.join(REPO, "src"))):
        src = os.path.join(REPO, "src", name)
        if os.path.isfile(src):
            shutil.copy2(src, os.path.join(work, "src", name))

    path = os.path.join(work, "src", "search.cpp")
    text = open(path).read()
    for old, new in PATCH:
        if text.count(old) != 1:
            sys.exit("census patch anchor is not unique in src/search.cpp: "
                     + repr(old[:60]))
        text = text.replace(old, new)
    open(path, "w").write(text)

    sh(["cmake", "-S", work, "-B", os.path.join(work, "build"), "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
        "-DCHESSO_ARCH=native", "-DCHESSO_TUNE=OFF"])
    sh(["cmake", "--build", os.path.join(work, "build"), "--target", "chesso",
        "-j", str(os.cpu_count())])

    return os.path.join(work, "build", "src", "chesso")


def bench_total(engine):
    out = subprocess.run([engine, "bench"], capture_output=True, text=True)
    for line in reversed(out.stdout.splitlines()):
        if line.endswith("nps"):
            return int(line.split()[0])
    sys.exit("no bench line from " + engine)


def signature(engine, shipping):
    """The instrumented binary must search the same tree as the one that ships.

    The counters are write-only -- nothing in the search reads one -- so the
    bench total has to be the shipping build's exactly, and this compares them
    rather than asserting it in prose. A census taken on a binary that searches
    a different tree measures a different distribution, which is the one way
    this whole file could be quietly wrong.
    """
    total = bench_total(engine)
    if not os.path.exists(shipping):
        sys.exit("no shipping binary at %s to compare the signature against; "
                 "build it (cmake --build build -j12) or pass --shipping"
                 % shipping)
    want = bench_total(shipping)
    if total != want:
        sys.exit("the instrumented binary searches a different tree: bench "
                 "%d against the shipping build's %d (%s). The counters are "
                 "write-only, so a difference means the patch or the copied "
                 "src/ is not what ships and the census would measure the "
                 "wrong distribution." % (total, want, shipping))
    return total


DIVISOR = re.compile(r'X\(LMR_HIST_DIV,\s*"LmrHistDiv",\s*(-?\d+)')


def shipping_divisor():
    """The divisor the tree under census was built with, read from the source
    the worktree was copied from -- so the output file names the tree it was
    taken on rather than leaving it to be remembered."""
    text = open(os.path.join(REPO, "src", "search_params.hpp")).read()
    found = DIVISOR.search(text)
    return int(found.group(1)) if found else None


def drive(engine, depth, census_path):
    positions = s024.read_positions()
    env = dict(os.environ, CHESSO_HIST_CENSUS=census_path)
    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1,
                            env=env)
    engine_io = s024.Engine.__new__(s024.Engine)
    engine_io.p = proc
    engine_io.uci_handshake()

    nodes = 0
    started = time.time()
    for row in positions:
        engine_io.ucinewgame()
        engine_io.isready()
        engine_io.position_fen(row["fen"])
        lines = engine_io.go_depth(depth)
        last = [l for l in lines if l.startswith("info") and " nodes " in l]
        if not last:
            sys.exit("no node count for " + row["fen"])
        nodes += int(last[-1].split(" nodes ")[1].split()[0])
    engine_io.quit()

    return len(positions), nodes, time.time() - started


def read_census(path):
    sites = raw_nonzero = clipped = 0
    bins = {}
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if parts[0] == "sites":
            sites = int(parts[1])
        elif parts[0] == "raw_nonzero":
            raw_nonzero = int(parts[1])
        elif parts[0] == "clipped":
            clipped = int(parts[1])
        elif parts[0] == "bin":
            bins[int(parts[1])] = (int(parts[2]), int(parts[3]))
    return sites, raw_nonzero, clipped, bins


def percentiles(counts, wanted):
    """`counts` maps value -> count. The p-th percentile is the smallest value
    whose cumulative count reaches p per cent of the total -- the ordinary
    nearest-rank definition, so every number reported is a value the census
    actually saw."""
    total = sum(counts.values())
    if total == 0:
        return {p: None for p in wanted}
    out, seen, i = {}, 0, 0
    for value in sorted(counts):
        seen += counts[value]
        while i < len(wanted) and seen >= wanted[i] * total / 100.0:
            out[wanted[i]] = value
            i += 1
    while i < len(wanted):
        out[wanted[i]] = max(counts)
        i += 1
    return out


PCTS = (50, 75, 90, 99)


def summarise(depth, positions, nodes, wall, sites, raw_nonzero, clipped,
              bins):
    signed_all = {v: c[0] for v, c in bins.items() if c[0]}
    signed_nz = {v: c[1] for v, c in bins.items() if c[1]}
    abs_all, abs_nz = {}, {}
    for v, (a, n) in bins.items():
        abs_all[abs(v)] = abs_all.get(abs(v), 0) + a
        abs_nz[abs(v)] = abs_nz.get(abs(v), 0) + n

    lines = []
    add = lines.append
    add("depth %d" % depth)
    add("  positions: %d   nodes: %d   wall: %.1f s" % (positions, nodes, wall))
    add("  sites: %d   of them with a non-zero raw reduction: %d (%.2f %%)"
        % (sites, raw_nonzero,
           100.0 * raw_nonzero / sites if sites else 0.0))
    add("  clipped at the histogram bound: %d" % clipped)
    for name, counts in (("|sum|, all sites", abs_all),
                         ("|sum|, raw reduction non-zero", abs_nz),
                         ("signed sum, all sites", signed_all),
                         ("signed sum, raw non-zero", signed_nz)):
        p = percentiles(counts, PCTS)
        add("  %-30s p50 %7d  p75 %7d  p90 %7d  p99 %7d"
            % (name, p[50], p[75], p[90], p[99]))
    for threshold in (4337, 8675):
        share = sum(c for v, c in abs_all.items() if v >= threshold)
        add("  |sum| >= %-6d  %d of %d sites  (%.3f %%)"
            % (threshold, share, sites,
               100.0 * share / sites if sites else 0.0))
    add("  zero sums: %d (%.2f %%)"
        % (abs_all.get(0, 0), 100.0 * abs_all.get(0, 0) / sites if sites else 0))

    # What the rule would do at a divisor equal to this depth's own p75 of
    # |sum| -- the seed rule -- with the clamp at 2. The three shares are the
    # whole of what the term does to this tree, so the file carries them
    # rather than leaving them to be recomputed from the bins.
    div = percentiles(abs_all, (75,))[75]
    if div:
        one = sum(c for v, c in abs_all.items() if div <= v < 2 * div)
        two = sum(c for v, c in abs_all.items() if v >= 2 * div)
        none_ = sites - one - two
        add("  at divisor %d (this depth's p75) and clamp 2: 0 plies %.2f %%, "
            "1 ply %.2f %%, 2 plies %.2f %%"
            % (div, 100.0 * none_ / sites, 100.0 * one / sites,
               100.0 * two / sites))
    return "\n".join(lines), {"abs_all": abs_all, "abs_nz": abs_nz,
                              "sites": sites}


def cmd_census(args):
    work = os.path.join(REPO, ".ref-builds", "s098census")
    engine = build_instrumented(work)
    total = signature(engine, os.path.join(REPO, args.shipping))
    divisor = shipping_divisor()
    print("instrumented bench signature: %d, equal to the shipping build's"
          % total)

    blocks, data = [], {}
    for depth in ([args.depth] if args.depth else DEPTHS):
        path = os.path.join(work, "census_d%d.tsv" % depth)
        positions, nodes, wall = drive(engine, depth, path)
        sites, raw_nonzero, clipped, bins = read_census(path)
        block, summary = summarise(depth, positions, nodes, wall, sites,
                                   raw_nonzero, clipped, bins)
        print(block)
        blocks.append(block)
        data[depth] = summary

    header = [
        "# S098 verdict 1: the history sum where the reduction reads it.",
        "# Regenerate with:",
        "#   ~/.venv/chess/bin/python adocs/data/S098_v1_hist_census.py census",
        "# The script's docstring is the method; it holds the instrumentation",
        "# patch, so this file is reproducible from the repository alone.",
        "# 400 positions of adocs/data/S024_census_positions.txt, one",
        "# persistent UCI process, ucinewgame between positions, go depth N.",
        "# A site is one call of lmr_adjusted_reduction in negamax_at.",
        "# The raw-reduction column reads 100 % at both depths and that is not",
        "# a broken counter: the eligibility in front of the site is depth >= 3",
        "# past the third legal move, and the table returns at least 1",
        "# everywhere in that region, so every site the rule reaches already",
        "# had a reduction to move.",
        "# Percentiles are nearest-rank, so every number is one the census saw.",
        "# Instrumented bench signature: %d, compared against the shipping"
        % total,
        "# build's own bench total and equal to it, which is what says the",
        "# write-only counters do not move the tree the census is taken on.",
        "# LmrHistDiv on that tree: %s." % divisor,
        "#",
        "# ONE PASS, AND THAT IS A LIMIT ON WHAT A PERCENTILE FROM IT MEANS.",
        "# The distribution below belongs to the tree the census ran on. A",
        "# divisor taken from it changes the reduction, which changes the tree,",
        "# which changes the distribution -- so the percentile is the 75th of",
        "# the tree that was measured and not of the tree that then ships. A",
        "# fixed point would need the census and the re-seed iterated to",
        "# agreement, which this file does not do and DEC-212's SPSA lane",
        "# supersedes: the fit searches the scale against games rather than",
        "# against one pass of a distribution.",
        "",
    ]
    with open(OUT_TXT, "w") as fh:
        fh.write("\n".join(header) + "\n\n".join(blocks) + "\n")
    print("wrote " + OUT_TXT)

    if not args.keep:
        subprocess.run(["git", "worktree", "remove", "--force", work],
                       cwd=REPO, capture_output=True, text=True)
        subprocess.run(["git", "worktree", "prune"], cwd=REPO,
                       capture_output=True)
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(required=True)
    one = sub.add_parser("census")
    one.add_argument("--depth", type=int, default=None)
    one.add_argument("--keep", action="store_true",
                     help="leave the instrumented worktree in place")
    one.add_argument("--shipping", default="build/src/chesso",
                     help="the shipping build whose bench total the "
                          "instrumented one must equal")
    one.set_defaults(run=cmd_census)
    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
