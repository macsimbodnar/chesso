#!/usr/bin/env python3
"""S237: the reductions a reduced move arrives with, and how the static
evaluation moves across it, where the hindsight rule reads them -- so the
rule's thresholds rest on this engine's own data (DEC-134 form (b)).

    ~/.venv/chess/bin/python adocs/data/S237_census.py census
    ~/.venv/chess/bin/python adocs/data/S237_census.py census --depth 12
    ~/.venv/chess/bin/python adocs/data/S237_census.py census --keep

WHY IT EXISTS. S237 lets a node correct the depth its parent gave it, by one
ply either way, on two inputs: the reduction the parent took off the move that
reached it, and the static evaluation's move across that move from the mover's
side. Its four constants say what counts as a heavy reduction, a light one, a
worsening and an improvement. None has a publication behind it that is not
another engine's shipped number (DEC-105, DEC-134), so the two the step file
names are sized from what the rule's own inputs look like on this engine.
DEC-212's pattern, S236's shape.

THE SEED RULES, stated before the numbers exist:

  HindsightHeavyReduction  the **p75 of the applied reductions** over the
                           sites below: a move counts as heavily reduced at or
                           above the upper quartile of what the engine takes
                           off a reduced move.
  HindsightWorseMargin     the **p50 of |mover delta|** over the same sites,
  HindsightBetterMargin    both of them: a move's evaluation counts as having
                           moved when it moved further than the median reduced
                           move's did.
  HindsightLightReduction  not seeded here. The step file names no percentile
                           for it, so it is (c), the midpoint of its declared
                           range, stated at its site in src/search_params.hpp.

Percentiles are nearest-rank -- the smallest value whose cumulative count
reaches p per cent -- so every seed is a value the census saw.

WHAT IS COUNTED, AND WHERE. One site is one node at which the rule computes
its delta in `negamax_at`: reached through a reduced first search
(`parent_reduction > 0`), not the root, not in check, the parent's slot not
the sentinel, neither static score in the mate band. That is exactly the set
of nodes on which the two conditions are evaluated, so the distributions are
the rule's inputs and nothing wider. Arrivals through a reduced search that
never got that far -- a table cutoff, a draw, a leaf, a node in check -- are
counted apart and reported, and are not sites. For each site the counters
record the parent's reduction, the mover's delta `-static_eval -
static_evals[ply - 1]`, and whether the node's depth is at least 2, which the
give-up branch also requires.

THE TREE IT IS TAKEN ON IS THE TREE THE RULE IS ADDED TO. The throwaway copy
has `HindsightHeavyReduction` forced to 126 and `HindsightLightReduction` to
0, the two off values, so nothing the census counts has been moved by a
provisional correction; at those values the engine is the parent commit's to
the node. **The control binary is built from the same forced source**, so the
signature comparison says only that the write-only counters do not move the
tree.

One pass, and that bounds what a percentile from it means: a threshold taken
from this distribution changes depths, which changes the tree, which changes
the distribution. The fixed point is not iterated; S127's lane supersedes it.

WHAT THE SEEDS WOULD DO. From the same joint histogram, and without a second
run, the file reports the share of sites each branch would fire on at the
derived seeds -- exact over the sites, and a counterfactual on the tree
without the rule, not an observation of one with it.

THE POSITIONS ARE THE BENCH'S OWN: the eight of `bench_positions` in
src/chesso.cpp, read the way `adocs/data/S236_hist_census.py` reads them (that
file's `bench_positions` is imported, so a bench position that moves moves
this census with it). `go depth N` from one persistent UCI process with
`ucinewgame` between positions, so each search is iterative deepening as a
game's is.

THE BINARY IS A THROWAWAY AND THE SHIPPED TREE IS NEVER TOUCHED. A detached
worktree under `.ref-builds/`, the **working tree's** `src/` copied into it,
the patch below applied there, the worktree removed at the end. The counters
write one file named by `CHESSO_HINDSIGHT_CENSUS` at exit and print nothing,
so nothing reaches the UCI surface.

WHAT IT IS NOT. Not a strength measurement (DEC-019). It sizes three seeds;
`adocs/data/S237_sprt.sh` decides the step.
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
import S236_hist_census as s236  # noqa: E402

OUT_TXT = os.path.join(REPO, "adocs", "data", "S237_census.txt")
DEPTHS = (10, 12)
SEED_DEPTH = 12

R_BOUND = 63
D_BOUND = 4000

COUNTERS = r'''
// ---- S237 hindsight census, throwaway instrumentation ------------------
// Not in the shipping engine: adocs/data/S237_census.py patches it in to a
// detached worktree, builds there, and removes the worktree afterwards.
namespace
{
constexpr int HS_R_BOUND = %(R)d;
constexpr int HS_D_BOUND = %(D)d;
constexpr int HS_D_SPAN = 2 * HS_D_BOUND + 1;

struct hindsight_census_t
{
  uint64_t arrivals = 0;
  uint64_t sites = 0;
  uint64_t clipped = 0;
  // [depth >= 2][reduction][delta + bound]
  std::vector<uint64_t> bins;

  hindsight_census_t() : bins(2 * (HS_R_BOUND + 1) * HS_D_SPAN, 0) {}
  ~hindsight_census_t();
};

hindsight_census_t hindsight_census;

hindsight_census_t::~hindsight_census_t()
{
  const char* path = std::getenv("CHESSO_HINDSIGHT_CENSUS");

  if (path == nullptr) { return; }

  std::ofstream out(path);

  if (!out) { return; }

  out << "arrivals\t" << arrivals << '\n'
      << "sites\t" << sites << '\n'
      << "clipped\t" << clipped << '\n';

  for (int deep = 0; deep < 2; ++deep) {
    for (int r = 0; r <= HS_R_BOUND; ++r) {
      for (int d = 0; d < HS_D_SPAN; ++d) {
        const uint64_t c =
            bins[(static_cast<size_t>(deep) * (HS_R_BOUND + 1) + r) *
                     HS_D_SPAN + d];

        if (c == 0) { continue; }

        out << "bin\t" << deep << '\t' << r << '\t' << (d - HS_D_BOUND)
            << '\t' << c << '\n';
      }
    }
  }
}

inline void hindsight_census_arrival() { hindsight_census.arrivals++; }

inline void hindsight_census_site(int reduction, int delta, int depth)
{
  hindsight_census.sites++;

  int r = reduction;
  int d = delta;

  if (r > HS_R_BOUND || d > HS_D_BOUND || d < -HS_D_BOUND) {
    hindsight_census.clipped++;
  }

  if (r > HS_R_BOUND) { r = HS_R_BOUND; }
  if (d > HS_D_BOUND) { d = HS_D_BOUND; }
  if (d < -HS_D_BOUND) { d = -HS_D_BOUND; }

  const size_t deep = (depth >= 2) ? 1 : 0;

  hindsight_census.bins[(deep * (HS_R_BOUND + 1) + static_cast<size_t>(r)) *
                            HS_D_SPAN +
                        static_cast<size_t>(d + HS_D_BOUND)]++;
}
}  // namespace
// ---- end S237 census ---------------------------------------------------

''' % {"R": R_BOUND, "D": D_BOUND}

PATCH = [
    # The counters, above the function whose sites they count.
    ("template <bool PROBING>\nstatic int negamax_at(int alpha0,",
     COUNTERS + "template <bool PROBING>\nstatic int negamax_at(int alpha0,"),
    # Every arrival through a reduced first search that reached the rule.
    ("  int hindsight = 0;\n",
     "  int hindsight = 0;\n"
     "  if (parent_reduction > 0) { hindsight_census_arrival(); }\n"),
    # The site: the delta the two conditions read, where they read it.
    ("      const int mover_delta = -static_eval - parent_eval;\n",
     "      const int mover_delta = -static_eval - parent_eval;\n"
     "      hindsight_census_site(parent_reduction, mover_delta, depth);\n"),
    ("#include <limits>\n",
     "#include <limits>\n"
     "// S237 census, throwaway: see adocs/data/S237_census.py.\n"
     "#include <cstdlib>\n"
     "#include <fstream>\n"
     "#include <vector>\n"),
]

# The two reduction thresholds are each branch's switch; both forced to their
# off values so the censused tree is the tree the rule is added to.
FORCE_OFF = [
    ('X(HINDSIGHT_HEAVY_REDUCTION, "HindsightHeavyReduction", ', 126),
    ('X(HINDSIGHT_LIGHT_REDUCTION, "HindsightLightReduction", ', 0),
]


def force_off(work):
    path = os.path.join(work, "src", "search_params.hpp")
    text = open(path).read()
    shipped = {}
    for anchor, off in FORCE_OFF:
        if text.count(anchor) != 1:
            sys.exit("row not unique in src/search_params.hpp: " + anchor)
        row = re.search(re.escape(anchor) + r"(\s*)(-?\d+)", text)
        if row is None:
            sys.exit("no default after " + anchor)
        shipped[anchor.split('"')[1]] = int(row.group(2))
        old = anchor + row.group(1) + row.group(2)
        new = (anchor + row.group(1) + str(off)).ljust(len(old))
        text = text.replace(old, new)
    open(path, "w").write(text)
    return shipped


def build_pair(work):
    if os.path.exists(work):
        subprocess.run(["git", "worktree", "remove", "--force", work],
                       cwd=REPO, capture_output=True, text=True)
        shutil.rmtree(work, ignore_errors=True)
    subprocess.run(["git", "worktree", "prune"], cwd=REPO, capture_output=True)
    s236.sh(["git", "worktree", "add", "--detach", work, "HEAD"], cwd=REPO)

    for name in sorted(os.listdir(os.path.join(REPO, "src"))):
        src = os.path.join(REPO, "src", name)
        if os.path.isfile(src):
            shutil.copy2(src, os.path.join(work, "src", name))

    shipped = force_off(work)
    control = s236.configure_and_build(work, "build-control")

    path = os.path.join(work, "src", "search.cpp")
    text = open(path).read()
    for old, new in PATCH:
        if text.count(old) != 1:
            sys.exit("census patch anchor is not unique in src/search.cpp: "
                     + repr(old[:60]))
        text = text.replace(old, new)
    open(path, "w").write(text)

    census = s236.configure_and_build(work, "build-census")
    return control, census, shipped


def drive(engine, depth, census_path):
    env = dict(os.environ, CHESSO_HINDSIGHT_CENSUS=census_path)
    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1,
                            env=env)
    io = s024.Engine.__new__(s024.Engine)
    io.p = proc
    io.uci_handshake()

    positions = s236.bench_positions()
    nodes = 0
    started = time.time()
    for name, command in positions:
        io.ucinewgame()
        io.isready()
        io.send(command)
        io.send("isready")
        lines = io.read_until("readyok")
        if [ln for ln in lines if "refused" in ln]:
            sys.exit("position refused for " + name)
        lines = io.go_depth(depth)
        last = [l for l in lines if l.startswith("info") and " nodes " in l]
        if not last:
            sys.exit("no node count for " + name)
        nodes += int(last[-1].split(" nodes ")[1].split()[0])
    io.quit()
    return len(positions), nodes, time.time() - started


def read_census(path):
    out = {"arrivals": 0, "sites": 0, "clipped": 0, "bins": {}}
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if parts[0] in ("arrivals", "sites", "clipped"):
            out[parts[0]] = int(parts[1])
        elif parts[0] == "bin":
            key = (int(parts[1]), int(parts[2]), int(parts[3]))
            out["bins"][key] = int(parts[4])
    return out


PCTS = (25, 50, 75, 90, 95, 99)


def marginal(bins, f):
    out = {}
    for key, c in bins.items():
        v = f(key)
        out[v] = out.get(v, 0) + c
    return out


def firing(bins, heavy, light, worse, better):
    """The two conditions of src/search.cpp's rule, over the histogram."""
    back = up = 0
    for (deep, r, d), c in bins.items():
        if r >= heavy and d < -worse:
            back += c
        elif 1 <= r <= light and d > better and deep:
            up += c
    return back, up


def summarise(depth, positions, nodes, wall, data, light_seed):
    bins = data["bins"]
    sites = data["sites"]
    red = marginal(bins, lambda k: k[1])
    absd = marginal(bins, lambda k: abs(k[2]))
    signed = marginal(bins, lambda k: k[2])
    pr = s236.percentiles(red, PCTS)
    pd = s236.percentiles(absd, PCTS)
    ps = s236.percentiles(signed, PCTS)

    lines = []
    add = lines.append
    add("depth %d" % depth)
    add("  positions: %d   nodes: %d   wall: %.1f s" % (positions, nodes, wall))
    add("  arrivals through a reduced first search that reached the rule: %d"
        % data["arrivals"])
    add("  sites (the rule's guards passed): %d (%.2f %% of arrivals, %.2f %% "
        "of nodes)" % (sites, 100.0 * sites / max(1, data["arrivals"]),
                       100.0 * sites / max(1, nodes)))
    add("  clipped at the histogram bounds (r %d, |delta| %d): %d"
        % (R_BOUND, D_BOUND, data["clipped"]))
    fmt = "  %-26s" + "  p%d %6s" * len(PCTS)
    for name, p in (("applied reduction", pr), ("|mover delta|", pd),
                    ("signed mover delta", ps)):
        add(fmt % ((name,) + tuple(x for q in PCTS for x in (q, p[q]))))
    add("  reduction histogram: " + "  ".join(
        "%d:%.2f%%" % (r, 100.0 * red[r] / sites) for r in sorted(red)))
    deep = sum(c for k, c in bins.items() if k[0])
    add("  sites at depth >= 2: %d (%.2f %%)" % (deep, 100.0 * deep / sites))

    heavy, margin = pr[75], pd[50]
    add("  seed rules at this depth: HindsightHeavyReduction = p75 = %d, "
        "HindsightWorseMargin = HindsightBetterMargin = p50(|delta|) = %d"
        % (heavy, margin))
    back, up = firing(bins, heavy, light_seed, margin, margin)
    add("  at those seeds with HindsightLightReduction %d: give back on %d "
        "sites (%.2f %%), give up on %d (%.2f %%), neither on %.2f %%"
        % (light_seed, back, 100.0 * back / sites, up, 100.0 * up / sites,
           100.0 - 100.0 * (back + up) / sites))
    return "\n".join(lines), {"heavy": heavy, "margin": margin}


def cmd_census(args):
    work = os.path.join(REPO, ".ref-builds", "s237census")
    control, census, shipped = build_pair(work)
    total = s236.signature(census, control)
    print("instrumented bench signature: %d, equal to the control's" % total)

    depths = [args.depth] if args.depth else list(DEPTHS)
    blocks, derived = [], {}
    for depth in depths:
        path = os.path.join(work, "census_d%d.tsv" % depth)
        positions, nodes, wall = drive(census, depth, path)
        block, summary = summarise(depth, positions, nodes, wall,
                                   read_census(path), args.light)
        print(block)
        blocks.append(block)
        derived[depth] = summary

    seed_depth = SEED_DEPTH if SEED_DEPTH in derived else depths[-1]
    seeds = derived[seed_depth]
    blocks.append("seeds (depth %d): HindsightHeavyReduction %d, "
                  "HindsightWorseMargin %d, HindsightBetterMargin %d; "
                  "HindsightLightReduction %d is (c), not from this census"
                  % (seed_depth, seeds["heavy"], seeds["margin"],
                     seeds["margin"], args.light))
    print(blocks[-1])

    header = [
        "# S237: the rule's inputs where hindsight reductions read them.",
        "# Regenerate with:",
        "#   ~/.venv/chess/bin/python adocs/data/S237_census.py census",
        "# The script's docstring is the method and holds the patch.",
        "# The eight bench positions, one persistent UCI process, ucinewgame",
        "# between positions, go depth N.",
        "# A site is one node where negamax_at computes the mover's delta.",
        "# Percentiles are nearest-rank, so every number is one the census saw.",
        "# Instrumented bench signature: %d, equal to a control binary built"
        % total,
        "# from the same forced source without the counters.",
        "# The censused tree has the two reduction thresholds forced to their",
        "# off values (126, 0); the source it was copied from held %s."
        % ", ".join("%s %d" % kv for kv in sorted(shipped.items())),
        "#",
        "# THE SEED RULES, stated in the script before these numbers existed:",
        "#   HindsightHeavyReduction = p75(applied reduction)",
        "#   HindsightWorseMargin = HindsightBetterMargin = p50(|mover delta|)",
        "#   HindsightLightReduction: (c), the range midpoint, not from here",
        "#",
        "# One pass: the fixed point is not iterated; S127 fits against games.",
        "# Not Elo and not a verdict (DEC-019): it sizes three seeds.",
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
    one.add_argument("--light", type=int, default=1,
                     help="HindsightLightReduction for the firing counts; "
                          "its seed is (c), the midpoint of 0 to 2")
    one.set_defaults(run=cmd_census)
    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
