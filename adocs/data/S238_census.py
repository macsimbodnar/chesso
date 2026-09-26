#!/usr/bin/env python3
"""S238: the cutoff count where the rule reads it -- how many of a node's
children had failed high in their own move loops when the node reduced a late
quiet -- so the rule's threshold rests on this engine's own data (DEC-134 form
(b)).

    ~/.venv/chess/bin/python adocs/data/S238_census.py census
    ~/.venv/chess/bin/python adocs/data/S238_census.py census --depth 12
    ~/.venv/chess/bin/python adocs/data/S238_census.py census --keep

WHY IT EXISTS. S238 raises the reduction of a node's later quiet moves when
the count of its children that failed high so far is over a threshold. The
threshold has no publication behind it that is not another engine's shipped
number (DEC-105, DEC-134), so it is sized from what the count looks like on
this engine where the rule reads it. DEC-212's pattern, S237's shape.

THE SEED RULES, stated before the numbers exist:

  CutoffCountThreshold  the **p75 of the count** over the sites below. The
                        rule fires on a count strictly over the threshold, so
                        at most the upper quarter of the sites -- the nodes
                        whose children have failed high the most for the
                        point in the move list they are at.
  CutoffCountReduction  not seeded here. A count says when, never how much,
                        so it is (c), the midpoint of its declared range,
                        stated at its site in src/search_params.hpp.

Percentiles are nearest-rank -- the smallest value whose cumulative count
reaches p per cent -- so the seed is a value the census saw.

WHAT IS COUNTED, AND WHERE. One site is one late quiet whose reduction
`negamax_at` computes through `lmr_adjusted_reduction`: past the third legal
move, depth at least 3, not the root, neither side in check, not a capture
and not a promotion -- exactly the moves the rule's term is added to. For
each site the counters record the slot the rule reads,
`state->cutoff_counts[ply + 1]`, and the move number, `legal_moves_counter`,
jointly: the count can never exceed the number of child searches before the
move, so its size is partly the move number's, and the file reports how often
the count equals `move number - 1` -- every earlier child failed high.

THE TREE IT IS TAKEN ON IS THE TREE THE RULE IS ADDED TO. The throwaway copy
has `CutoffCountReduction` forced to 0, the off value, so nothing the census
counts has been moved by a provisional adjustment; at 0 the engine is the
parent commit's to the node. **The control binary is built from the same
forced source**, so the signature comparison says only that the write-only
counters do not move the tree.

One pass, and that bounds what a percentile from it means: a threshold taken
from this distribution changes reductions, which changes the tree, which
changes the distribution. The fixed point is not iterated; S127's lane
supersedes it.

WHAT THE SEED WOULD DO. From the same histogram, and without a second run,
the file reports the share of sites the rule would fire on at the derived
threshold -- exact over the sites, a counterfactual on the tree without the
rule.

THE POSITIONS ARE THE BENCH'S OWN: the eight of `bench_positions` in
src/chesso.cpp, read the way `adocs/data/S236_hist_census.py` reads them (that
file's `bench_positions` is imported). `go depth N` from one persistent UCI
process with `ucinewgame` between positions, so each search is iterative
deepening as a game's is.

THE BINARY IS A THROWAWAY AND THE SHIPPED TREE IS NEVER TOUCHED. A detached
worktree under `.ref-builds/`, the **working tree's** `src/` copied into it,
the patch below applied there, the worktree removed at the end. The counters
write one file named by `CHESSO_CUTOFF_CENSUS` at exit and print nothing, so
nothing reaches the UCI surface. Builds run at `-j4`, and the whole script is
meant to be run under `nice -n 19` when the machine is shared.

WHAT IT IS NOT. Not a strength measurement (DEC-019). It sizes one seed;
`adocs/data/S238_sprt.sh` decides the step.
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

OUT_TXT = os.path.join(REPO, "adocs", "data", "S238_census.txt")
DEPTHS = (10, 12)
SEED_DEPTH = 12

C_BOUND = 255
M_BOUND = 255

COUNTERS = r'''
// ---- S238 cutoff-count census, throwaway instrumentation ----------------
// Not in the shipping engine: adocs/data/S238_census.py patches it in to a
// detached worktree, builds there, and removes the worktree afterwards.
namespace
{
constexpr int CC_C_BOUND = %(C)d;
constexpr int CC_M_BOUND = %(M)d;

struct cutoff_census_t
{
  uint64_t sites = 0;
  uint64_t clipped = 0;
  // [count][move number]
  std::vector<uint64_t> bins;

  cutoff_census_t() : bins((CC_C_BOUND + 1) * (CC_M_BOUND + 1), 0) {}
  ~cutoff_census_t();
};

cutoff_census_t cutoff_census;

cutoff_census_t::~cutoff_census_t()
{
  const char* path = std::getenv("CHESSO_CUTOFF_CENSUS");

  if (path == nullptr) { return; }

  std::ofstream out(path);

  if (!out) { return; }

  out << "sites\t" << sites << '\n' << "clipped\t" << clipped << '\n';

  for (int c = 0; c <= CC_C_BOUND; ++c) {
    for (int m = 0; m <= CC_M_BOUND; ++m) {
      const uint64_t n =
          bins[static_cast<size_t>(c) * (CC_M_BOUND + 1) +
               static_cast<size_t>(m)];

      if (n == 0) { continue; }

      out << "bin\t" << c << '\t' << m << '\t' << n << '\n';
    }
  }
}

inline void cutoff_census_site(int count, int move_number)
{
  cutoff_census.sites++;

  int c = count;
  int m = move_number;

  if (c > CC_C_BOUND || m > CC_M_BOUND || c < 0) { cutoff_census.clipped++; }

  if (c > CC_C_BOUND) { c = CC_C_BOUND; }
  if (c < 0) { c = 0; }
  if (m > CC_M_BOUND) { m = CC_M_BOUND; }

  cutoff_census.bins[static_cast<size_t>(c) * (CC_M_BOUND + 1) +
                     static_cast<size_t>(m)]++;
}
}  // namespace
// ---- end S238 census ---------------------------------------------------

''' % {"C": C_BOUND, "M": M_BOUND}

PATCH = [
    # The counters, above the function whose sites they count.
    ("template <bool PROBING>\nstatic int negamax_at(int alpha0,",
     COUNTERS + "template <bool PROBING>\nstatic int negamax_at(int alpha0,"),
    # The site: the late quiet whose reduction the rule's term joins.
    ("      if (!is_capture && !MOVE_PROMOTED(moves[i])) {\n"
     "        const int cutoff_ticks =\n",
     "      if (!is_capture && !MOVE_PROMOTED(moves[i])) {\n"
     "        cutoff_census_site(state->cutoff_counts[ply + 1],\n"
     "                           static_cast<int>(legal_moves_counter));\n"
     "        const int cutoff_ticks =\n"),
    ("#include <limits>\n",
     "#include <limits>\n"
     "// S238 census, throwaway: see adocs/data/S238_census.py.\n"
     "#include <cstdlib>\n"
     "#include <fstream>\n"
     "#include <vector>\n"),
]

# The adjustment is the rule's switch; forced to its off value so the
# censused tree is the tree the rule is added to.
FORCE_OFF = [
    ('X(CUTOFF_COUNT_REDUCTION, "CutoffCountReduction", ', 0),
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
        # Right-aligned in the row's own width, so the column stays put.
        new = anchor + str(off).rjust(len(old) - len(anchor))
        text = text.replace(old, new)
    open(path, "w").write(text)
    return shipped


def configure_and_build(work, name):
    build = os.path.join(work, name)
    s236.sh(["cmake", "-S", work, "-B", build, "-G", "Ninja",
             "-DCMAKE_BUILD_TYPE=Release",
             "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
             "-DCHESSO_ARCH=native", "-DCHESSO_TUNE=OFF"])
    s236.sh(["cmake", "--build", build, "--target", "chesso", "-j", "4"])
    return os.path.join(build, "src", "chesso")


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
    control = configure_and_build(work, "build-control")

    path = os.path.join(work, "src", "search.cpp")
    text = open(path).read()
    for old, new in PATCH:
        if text.count(old) != 1:
            sys.exit("census patch anchor is not unique in src/search.cpp: "
                     + repr(old[:60]))
        text = text.replace(old, new)
    open(path, "w").write(text)

    census = configure_and_build(work, "build-census")
    return control, census, shipped


def drive(engine, depth, census_path):
    env = dict(os.environ, CHESSO_CUTOFF_CENSUS=census_path)
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
    out = {"sites": 0, "clipped": 0, "bins": {}}
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if parts[0] in ("sites", "clipped"):
            out[parts[0]] = int(parts[1])
        elif parts[0] == "bin":
            out["bins"][(int(parts[1]), int(parts[2]))] = int(parts[3])
    return out


PCTS = (25, 50, 75, 90, 95, 99)


def marginal(bins, f):
    out = {}
    for key, c in bins.items():
        v = f(key)
        out[v] = out.get(v, 0) + c
    return out


def summarise(depth, positions, nodes, wall, data):
    bins = data["bins"]
    sites = data["sites"]
    count = marginal(bins, lambda k: k[0])
    moven = marginal(bins, lambda k: k[1])
    pc = s236.percentiles(count, PCTS)
    pm = s236.percentiles(moven, PCTS)

    lines = []
    add = lines.append
    add("depth %d" % depth)
    add("  positions: %d   nodes: %d   wall: %.1f s" % (positions, nodes, wall))
    add("  sites (late quiets reduced through lmr_adjusted_reduction): %d "
        "(%.2f %% of nodes)" % (sites, 100.0 * sites / max(1, nodes)))
    add("  clipped at the histogram bounds (count %d, move %d): %d"
        % (C_BOUND, M_BOUND, data["clipped"]))
    fmt = "  %-22s" + "  p%d %5s" * len(PCTS)
    for name, p in (("cutoff count", pc), ("move number", pm)):
        add(fmt % ((name,) + tuple(x for q in PCTS for x in (q, p[q]))))
    add("  count histogram: " + "  ".join(
        "%d:%.2f%%" % (c, 100.0 * count[c] / sites) for c in sorted(count)
        if 100.0 * count[c] / sites >= 0.01))
    every = sum(c for (n, m), c in bins.items() if n == m - 1)
    over = sum(c for (n, m), c in bins.items() if n > m - 1)
    add("  sites where every earlier child failed high (count == move - 1): "
        "%d (%.2f %%); count above move - 1 (re-searches counted): %d "
        "(%.2f %%)" % (every, 100.0 * every / sites, over,
                       100.0 * over / sites))

    threshold = pc[75]
    fire = sum(c for (n, m), c in bins.items() if n > threshold)
    add("  seed rule at this depth: CutoffCountThreshold = p75(count) = %d"
        % threshold)
    add("  at that threshold the rule fires on %d sites (%.2f %%)"
        % (fire, 100.0 * fire / sites))
    return "\n".join(lines), threshold


def cmd_census(args):
    work = os.path.join(REPO, ".ref-builds", "s238census")
    control, census, shipped = build_pair(work)
    total = s236.signature(census, control)
    print("instrumented bench signature: %d, equal to the control's" % total)

    depths = [args.depth] if args.depth else list(DEPTHS)
    blocks, derived = [], {}
    for depth in depths:
        path = os.path.join(work, "census_d%d.tsv" % depth)
        positions, nodes, wall = drive(census, depth, path)
        block, threshold = summarise(depth, positions, nodes, wall,
                                     read_census(path))
        print(block)
        blocks.append(block)
        derived[depth] = threshold

    seed_depth = SEED_DEPTH if SEED_DEPTH in derived else depths[-1]
    blocks.append("seed (depth %d): CutoffCountThreshold %d; "
                  "CutoffCountReduction is (c), not from this census"
                  % (seed_depth, derived[seed_depth]))
    print(blocks[-1])

    header = [
        "# S238: the cutoff count where the rule reads it.",
        "# Regenerate with:",
        "#   ~/.venv/chess/bin/python adocs/data/S238_census.py census",
        "# The script's docstring is the method and holds the patch.",
        "# The eight bench positions, one persistent UCI process, ucinewgame",
        "# between positions, go depth N.",
        "# A site is one late quiet whose reduction negamax_at computes;",
        "# the count is state->cutoff_counts[ply + 1] at that moment.",
        "# Percentiles are nearest-rank, so every number is one the census saw.",
        "# Instrumented bench signature: %d, equal to a control binary built"
        % total,
        "# from the same forced source without the counters.",
        "# The censused tree has CutoffCountReduction forced to its off value",
        "# 0; the source it was copied from held %s."
        % ", ".join("%s %d" % kv for kv in sorted(shipped.items())),
        "#",
        "# THE SEED RULE, stated in the script before these numbers existed:",
        "#   CutoffCountThreshold = p75(count); fires on count > threshold",
        "#   CutoffCountReduction: (c), the range midpoint, not from here",
        "#",
        "# One pass: the fixed point is not iterated; S127 fits against games.",
        "# Not Elo and not a verdict (DEC-019): it sizes one seed.",
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
    print("CENSUS-DONE")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(required=True)
    one = sub.add_parser("census")
    one.add_argument("--depth", type=int, default=None)
    one.add_argument("--keep", action="store_true",
                     help="leave the instrumented worktree in place")
    one.set_defaults(run=cmd_census)
    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
