#!/usr/bin/env python3
"""S098 verdict 2: how often each of the four node-type conditions is true at
the sites late move reduction actually reads them, taken before any match.

    ~/.venv/chess/bin/python adocs/data/S098_v2_node_census.py census
    ~/.venv/chess/bin/python adocs/data/S098_v2_node_census.py census --depth 12
    ~/.venv/chess/bin/python adocs/data/S098_v2_node_census.py census --keep

WHY IT EXISTS, DEC-214. Verdict 1 cost four settings, two SPRTs and a night's
SPSA lane, and the expensive part was not the runs: its first seed was inert
and nothing said so until a census was taken (DEC-212). Verdict 2's four terms
are ply counts of 0 to 2 with no scale to get wrong, so no lane is owed -- but
the one question a seed can still get wrong here is whether the condition is
ever true at all. This answers that, for a build and a few minutes, before the
machine is asked for a verdict. **A condition true at under one per cent of
sites ships at 0 and is recorded as inert rather than measured.**

WHAT IS COUNTED, AND WHERE. One site is the reduction call site in
`negamax_at` -- the late-quiet reduction read, not every call of
`lmr_adjusted_reduction`, which the three `lmr_depth_of` gates also make --
one quiet move that reached the reduction with the node's
own eligibility satisfied: `ply > 0`, `depth >= 3`, past the third legal move,
neither side in check, the move neither a capture nor a promotion. Nothing
else is a site: a move the shallow-depth gate priced but the reduction never
reached is not one, and neither is a capture, which late move reduction
refuses. For each site the four conditions the terms read are recorded --
`cut_node`, `!improving`, a capturing transposition-table move, `is_pv` --
together with whether the raw table returned a non-zero reduction, so "the
term had a reduction to move" reads separately from "the term was consulted".

Kept as five counters and one 16-cell joint histogram over the four
conditions, not a row per site: the sites run to millions and the questions
asked of them are shares and co-occurrence.

**THE CENSUS IS TAKEN ON THE OFF TREE**, and that is the whole reason the
patch below also sets the four defaults to 0. The tree the shares describe is
then the tree the SPRT's reference searches -- the engine before verdict 2 --
which is the tree the question "would this term ever fire" is asked of. It has
an exact signature to check against, and the check is the second thing this
script buys: the instrumented binary's `bench` must equal **5685915**, the
total the commit before this landing prints, which says both that the
write-only counters do not move the tree and that the whole of verdict 2's
plumbing is inert at its off values in the Release build.

It inherits verdict 1's one-pass caveat in the mild form: the shares belong to
the off tree, and shipping the terms changes the tree and therefore the
shares. For a 0-to-2 ply count that matters far less than it did for a divisor
-- the reading taken from it is "does this condition fire at all", which no
re-shaping of the tree turns into "never".

THE POSITIONS ARE S024'S AND THE DRIVER IS S024'S, exactly as
`adocs/data/S098_v1_hist_census.py` used them: `adocs/data/S024_census_positions.txt`
-- 400 positions, one per game, from `adocs/data/S219_aa_calibration.pgn` --
read through that file's own `read_positions()` and driven through its
`Engine` class, one persistent UCI process, `ucinewgame` between positions,
`go depth N` awaited to `bestmove`. TOOLCHAIN.md's oracle trap is general and
that driver is the form that does not fall into it. Reusing them is the point:
the two censuses are over the same board positions and can be read beside each
other.

`go depth N` and not a single `search()` call, deliberately: a game's `go` is
iterative deepening, so the table entries a real move's reduction reads are
the ones its own shallower iterations wrote -- which is exactly what a cold
fixed-depth call would not show, and the transposition-table term depends on
it entirely.

THE BINARY IS A THROWAWAY AND THE SHIPPED TREE IS NEVER TOUCHED. A detached
worktree is created under `.ref-builds/`, the **working tree's** `src/` is
copied into it -- so what is measured is the landing and not `HEAD` -- the
patch below is applied there, and the worktree is removed when the run ends.
The instrumentation is not in the shipping engine and never was: it writes one
file named by `CHESSO_NODE_CENSUS` at exit and prints nothing, so it is not on
the UCI surface `tests/test_uci_surface.cpp` guards. The patch is in this file
rather than in a diff beside it so the run is reproducible from the script
alone.

WHAT IT IS NOT. Not a strength measurement, and nothing here reads it as one
(DEC-019). It sizes four seeds; `adocs/data/S098_v2_sprt.sh` decides the
verdict.
"""

import argparse
import os
import shutil
import subprocess
import sys
import time

REPO = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(REPO, "adocs", "data"))

import S024_census_run as s024  # noqa: E402  (path set above)

OUT_TXT = os.path.join(REPO, "adocs", "data", "S098_v2_node_census.txt")
DEPTHS = (10, 12)

# The bench total of the tree this census is taken on: the commit before
# verdict 2's landing, which is also this landing with its four constants at 0.
# Not a golden in tests/ and not read from a run -- it is the signature the
# whole method rests on and the script refuses to write a file without it.
OFF_TREE_BENCH = 5685915

# The instrumentation. One anchor per pair, each occurring exactly once in the
# file it names, applied to the copy in the throwaway worktree and never to the
# tree this script runs from.
COUNTERS = r'''
// ---- S098 verdict-2 node-type census, throwaway instrumentation -------
// Not in the shipping engine: adocs/data/S098_v2_node_census.py patches it in
// to a detached worktree, builds there, and removes the worktree afterwards.
// One site is one call of lmr_adjusted_reduction from the reduction below.
namespace
{
struct node_census_t
{
  uint64_t sites = 0;
  uint64_t cut_node = 0;
  uint64_t not_improving = 0;
  uint64_t tt_capture = 0;
  uint64_t is_pv = 0;
  uint64_t raw_nonzero = 0;
  uint64_t joint[16] = {};

  ~node_census_t();
};

node_census_t node_census;

node_census_t::~node_census_t()
{
  const char* path = std::getenv("CHESSO_NODE_CENSUS");

  if (path == nullptr) { return; }

  std::ofstream out(path);

  if (!out) { return; }

  out << "sites\t" << node_census.sites << '\n'
      << "cut_node\t" << node_census.cut_node << '\n'
      << "not_improving\t" << node_census.not_improving << '\n'
      << "tt_capture\t" << node_census.tt_capture << '\n'
      << "is_pv\t" << node_census.is_pv << '\n'
      << "raw_nonzero\t" << node_census.raw_nonzero << '\n';

  for (int i = 0; i < 16; ++i) {
    out << "joint\t" << i << '\t' << node_census.joint[i] << '\n';
  }
}

inline void node_census_site(bool cut_node,
                             bool improving,
                             bool tt_capture,
                             bool is_pv,
                             bool raw_nonzero)
{
  node_census.sites++;

  if (cut_node) { node_census.cut_node++; }
  if (!improving) { node_census.not_improving++; }
  if (tt_capture) { node_census.tt_capture++; }
  if (is_pv) { node_census.is_pv++; }
  if (raw_nonzero) { node_census.raw_nonzero++; }

  const int cell = (cut_node ? 1 : 0) | (improving ? 0 : 2) |
                   (tt_capture ? 4 : 0) | (is_pv ? 8 : 0);

  node_census.joint[cell]++;
}
}  // namespace
// ---- end S098 verdict-2 census ----------------------------------------

'''

PATCH = [
    # The counters, above the helper whose call sites they count.
    ("// What the table above is worth once the **node's own type** is taken "
     "into\n// account, S098 verdict 2.",
     COUNTERS
     + "// What the table above is worth once the **node's own type** is taken "
       "into\n// account, S098 verdict 2."),
    # The site itself: every call of the helper from the reduction.
    ("        reduction = lmr_adjusted_reduction(\n"
     "            depth, static_cast<int>(legal_moves_counter), "
     "node_adjustment);",
     "        node_census_site(\n"
     "            cut_node, improving, tt_move_is_capture, is_pv,\n"
     "            lmr_reduction(depth, static_cast<int>(legal_moves_counter)) "
     "!=\n"
     "                0);\n"
     "        reduction = lmr_adjusted_reduction(\n"
     "            depth, static_cast<int>(legal_moves_counter), "
     "node_adjustment);"),
    # `<cstdlib>`, `<fstream>` and `<vector>` are not in scope in a release
    # build of this file -- the first two are inside its `#ifdef CHESSO_TUNE`
    # -- so the patch brings them.
    ("#include <limits>\n",
     "#include <limits>\n"
     "// S098 census, throwaway: see adocs/data/S098_v2_node_census.py.\n"
     "#include <cstdlib>\n"
     "#include <fstream>\n"),
]

# The off tree. Four defaults to 0 in the worktree's copy, which is what makes
# the bench signature above checkable and the shares the reference tree's.
OFF_PATCH = [
    ('  X(LMR_CUTNODE,       "LmrCutNode",      1,      0, 2)',
     '  X(LMR_CUTNODE,       "LmrCutNode",      0,      0, 2)'),
    ('  X(LMR_NOT_IMPROVING, "LmrNotImproving", 1,      0, 2)',
     '  X(LMR_NOT_IMPROVING, "LmrNotImproving", 0,      0, 2)'),
    ('  X(LMR_TT_CAPTURE,    "LmrTtCapture",    1,      0, 2)',
     '  X(LMR_TT_CAPTURE,    "LmrTtCapture",    0,      0, 2)'),
    ('  X(LMR_PV,            "LmrPv",           1,      0, 2)',
     '  X(LMR_PV,            "LmrPv",           0,      0, 2)'),
]


def sh(cmd, **kw):
    proc = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if proc.returncode != 0:
        sys.stderr.write(" ".join(cmd) + "\n" + proc.stdout[-4000:] +
                         proc.stderr[-4000:])
        sys.exit("command failed: " + " ".join(cmd))
    return proc.stdout


def apply_patch(path, pairs):
    text = open(path).read()
    for old, new in pairs:
        if text.count(old) != 1:
            sys.exit("census patch anchor is not unique in %s: %r"
                     % (path, old[:70]))
        text = text.replace(old, new)
    open(path, "w").write(text)


def build_instrumented(work, off):
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

    apply_patch(os.path.join(work, "src", "search.cpp"), PATCH)

    if off:
        apply_patch(os.path.join(work, "src", "search_params.hpp"), OFF_PATCH)

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


def signature(engine, expect):
    """The instrumented binary must search the tree the census claims.

    The counters are write-only -- nothing in the search reads one -- and the
    four constants are patched to their off values, so the total has to be the
    off tree's exactly. A difference means either the patch moved the tree or
    the plumbing is not inert at its off values, and both would make every
    share below a share of some other distribution.
    """
    total = bench_total(engine)
    if total != expect:
        sys.exit("the instrumented binary searches a different tree: bench "
                 "%d against the expected %d. Either the counters moved the "
                 "tree or S098 verdict 2's plumbing is not inert at its off "
                 "values; the census is not taken until that is resolved."
                 % (total, expect))
    return total


def drive(engine, depth, census_path):
    positions = s024.read_positions()
    env = dict(os.environ, CHESSO_NODE_CENSUS=census_path)
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
    counts, joint = {}, {}
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if parts[0] == "joint":
            joint[int(parts[1])] = int(parts[2])
        else:
            counts[parts[0]] = int(parts[1])
    return counts, joint


CONDITIONS = (
    ("cut_node", "cut_node", "LmrCutNode"),
    ("not_improving", "!improving", "LmrNotImproving"),
    ("tt_capture", "a capturing TT move", "LmrTtCapture"),
    ("is_pv", "is_pv", "LmrPv"),
)


def summarise(depth, positions, nodes, wall, counts, joint):
    sites = counts["sites"]
    lines = []
    add = lines.append
    add("depth %d" % depth)
    add("  positions: %d   nodes: %d   wall: %.1f s" % (positions, nodes, wall))
    add("  sites: %d   of them with a non-zero raw reduction: %d (%.2f %%)"
        % (sites, counts["raw_nonzero"],
           100.0 * counts["raw_nonzero"] / sites if sites else 0.0))
    for key, label, option in CONDITIONS:
        share = 100.0 * counts[key] / sites if sites else 0.0
        add("  %-22s %12d of %d sites  %6.2f %%   %s%s"
            % (label, counts[key], sites, share, option,
               "   INERT (< 1 %, ships at 0)" if share < 1.0 else ""))

    # The four conditions co-occur, and the sum of the four terms is what a
    # site actually pays. The distribution of that sum at the midpoint seeds
    # -- every term 1, so the sum is the count of conditions true, with the PV
    # term subtracting -- is the whole of what verdict 2 does to this tree.
    add("  the joint distribution, plies at the midpoint seeds (PV subtracts):")
    by_plies = {}
    for cell, count in joint.items():
        plies = ((1 if cell & 1 else 0) + (1 if cell & 2 else 0) +
                 (1 if cell & 4 else 0) - (1 if cell & 8 else 0))
        by_plies[plies] = by_plies.get(plies, 0) + count
    for plies in sorted(by_plies):
        add("    %+d ply: %12d  %6.2f %%"
            % (plies, by_plies[plies],
               100.0 * by_plies[plies] / sites if sites else 0.0))
    return "\n".join(lines)


def cmd_census(args):
    work = os.path.join(REPO, ".ref-builds", "s098v2census")
    engine = build_instrumented(work, not args.on_tree)
    total = signature(engine, args.expect_bench)
    print("instrumented bench signature: %d, the off tree's" % total)

    blocks = []
    for depth in ([args.depth] if args.depth else DEPTHS):
        path = os.path.join(work, "census_d%d.tsv" % depth)
        positions, nodes, wall = drive(engine, depth, path)
        counts, joint = read_census(path)
        block = summarise(depth, positions, nodes, wall, counts, joint)
        print(block)
        blocks.append(block)

    header = [
        "# S098 verdict 2: the four node-type conditions where the reduction",
        "# reads them. DEC-214, taken before any match.",
        "# Regenerate with:",
        "#   ~/.venv/chess/bin/python adocs/data/S098_v2_node_census.py census",
        "# The script's docstring is the method; it holds the instrumentation",
        "# patch, so this file is reproducible from the repository alone.",
        "# 400 positions of adocs/data/S024_census_positions.txt, one",
        "# persistent UCI process, ucinewgame between positions, go depth N.",
        "# A site is the reduction call site in negamax_at, the late-quiet",
        "# reduction read -- not every call of lmr_adjusted_reduction, which",
        "# the three lmr_depth_of gates also make: a quiet past the third",
        "# legal move at depth 3 or more with neither side in check, which is",
        "# late move reduction's own eligibility.",
        "#",
        "# TAKEN ON THE OFF TREE. The four constants are patched to 0 in the",
        "# throwaway worktree, so the shares belong to the tree the SPRT's",
        "# reference searches. Instrumented bench signature: %d, equal to the"
        % total,
        "# total the commit before this landing prints, which says both that",
        "# the write-only counters do not move the tree and that verdict 2's",
        "# plumbing is inert at its off values in the Release build.",
        "#",
        "# A condition true at under one per cent of sites ships at 0 and is",
        "# recorded as inert rather than measured (DEC-214).",
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
    one.add_argument("--on-tree", action="store_true",
                     help="census the shipping defaults instead of the off "
                          "tree; --expect-bench must then name that tree's "
                          "own total")
    one.add_argument("--expect-bench", type=int, default=OFF_TREE_BENCH,
                     help="the bench total the instrumented binary must print")
    one.set_defaults(run=cmd_census)
    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
