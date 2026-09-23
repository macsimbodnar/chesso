#!/usr/bin/env python3
"""S236: the distribution of the history sum where the fractional reduction
term reads it, so `LmrHistDiv` and `LmrHistClamp` rest on this engine's own
data and not on S098 verdict 1's fitted pair.

    ~/.venv/chess/bin/python adocs/data/S236_hist_census.py census
    ~/.venv/chess/bin/python adocs/data/S236_hist_census.py census --depth 12
    ~/.venv/chess/bin/python adocs/data/S236_hist_census.py census --keep

WHY IT EXISTS. S236 puts the late move reduction in fixed point and returns
S098 verdict 1's history term as a **fraction of a ply**. The two constants of
that term cannot be inherited from verdict 1: 699 and 3 were fitted against a
term whose smallest step was a whole ply, they measured zero at three scales,
and the term left the tree (DEC-213). A fraction needs its own scale, and the
scale is a statement about what the history tables **do** hold at the sites the
rule reads them -- which is what this measures. DEC-212's pattern, DEC-134 form
(b).

THE SEED RULES, stated before the numbers exist:

  LmrHistDiv    is the sum that buys one whole ply, seeded at **twice the p90
                of |sum|**, so the ninetieth percentile site moves the
                reduction by half a ply and the typical site by less. A
                fraction at the typical site is the whole hypothesis of the
                step; a divisor that made the median site a whole ply would be
                verdict 1 again under another name.
  LmrHistClamp  is the **p99's own contribution under that divisor**, so the
                clamp binds on the top hundredth of sites and not on the shape
                of the distribution. Capped at the declared range top of two
                plies; if the cap binds, this file says so and the value is a
                range bound rather than a fit.

WHAT IS COUNTED, AND WHERE. One site is one call of `lmr_adjusted_reduction`
in `negamax_at` -- one quiet move that reached the reduction with the node's
own eligibility satisfied: `ply > 0`, `depth >= 3`, past the third legal move,
neither side in check, the move neither a capture nor a promotion. Nothing else
is a site: a move the shallow-depth gate priced but the reduction never reached
is not one, and neither is a capture. For each site the instrumentation records
the signed `hist_sum` the rule would read and whether the raw table's tick
value at that (depth, move number) is non-zero.

THE TREE IT IS TAKEN ON IS THE TREE THE TERM IS ADDED TO. The throwaway copy
has `LmrHistClamp`'s default forced to 0, which is the term's off value, so
what is measured is the input distribution and not one a provisional term has
already moved. That is also leg 1's tree -- the accumulator with its rounding
and no history term -- so the census and the bisection talk about the same
engine. **The control binary is built from the same forced source**, so the
signature comparison below is like against like and says only what it is meant
to say: that the write-only counters do not move the tree.

One pass, and that bounds what a percentile from it means. A divisor taken from
this distribution changes the reduction, which changes the tree, which changes
the distribution. The fixed point is not iterated here and S127's lane
supersedes it: a fit searches the scale against games rather than against one
pass of a distribution.

SECOND PASS, WHAT THE SEED WOULD DO. Percentiles seed the constants; they do
not say how often the term actually moves a reduction, which for a fractional
term depends on where each site's accumulated sum sits against the rounding
boundary. So the same instrumented binary is driven a second time with
`CHESSO_HIST_TRY_DIV` and `CHESSO_HIST_TRY_CLAMP` set to the derived pair, and
it counts -- per site, on the tree without the term -- whether that pair would
have changed the whole-ply reduction and by how much. Exact rather than
estimated from a histogram, and still a counterfactual on one tree.

THE POSITIONS ARE THE BENCH'S OWN, which is what the step file asks for: the
eight of `bench_positions` in src/chesso.cpp, six of them reachable by the UCI
position shortcuts the engine already has and two read out of that file by an
anchored regex rather than copied here, so a bench position that moves moves
this census with it. `go depth 12` and not a single fixed-depth call: a game's
`go` is iterative deepening from a history table `iterative_deepening_search()`
zeroes at its top, so the sums a real move reads are the ones its own shallower
iterations built.

THE BINARY IS A THROWAWAY AND THE SHIPPED TREE IS NEVER TOUCHED. A detached
worktree is created under `.ref-builds/`, the **working tree's** `src/` is
copied into it -- so what is measured is the landing and not `HEAD` -- the
patches below are applied there, and the worktree is removed when the run ends.
The instrumentation is not in the shipping engine and never was: it writes one
file named by `CHESSO_HIST_CENSUS` at exit and prints nothing, so it is not on
the UCI surface `tests/test_uci_surface.cpp` guards. The patch is in this file
rather than in a diff beside it so the run is reproducible from the script
alone.

WHAT IT IS NOT. Not a strength measurement, and nothing here reads it as one
(DEC-019). It sizes two seeds; `adocs/data/S236_sprt.sh` decides the step.
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

OUT_TXT = os.path.join(REPO, "adocs", "data", "S236_hist_census.txt")
DEPTHS = (10, 12)

# The six bench positions the engine can be asked for by name, in
# `bench_positions`' own order, and the two it cannot.
SHORTCUTS = ["kiwipete", "killer", "blocked", "fine70", "mate2w", "mate2b"]
LITERAL_TAGS = ["midgame", "tactical"]

# The two literal FENs, read out of src/chesso.cpp by their own trailing tags
# rather than copied into this file: a bench position that moves has to move
# this census with it, and a copy would go stale silently.
LITERAL_RE = '"(?P<fen>[^"]+)",\\s+// %s\\b'

# The instrumentation. One anchor per pair, each occurring exactly once in the
# file it names, applied to the copy in the throwaway worktree and never to the
# tree this script runs from.
COUNTERS = r'''
// ---- S236 history census, throwaway instrumentation --------------------
// Not in the shipping engine: adocs/data/S236_hist_census.py patches it in to
// a detached worktree, builds there, and removes the worktree afterwards.
// One site is one call of lmr_adjusted_reduction below.
//
// Two counters in one: the histogram of the sum, which seeds the divisor, and
// -- when CHESSO_HIST_TRY_DIV and CHESSO_HIST_TRY_CLAMP are set -- what that
// candidate pair would do to this tree's whole-ply reductions, which the
// histogram alone cannot say for a fractional term.
namespace
{
constexpr int HIST_CENSUS_BOUND = 40000;

struct hist_census_t
{
  uint64_t sites = 0;
  uint64_t raw_nonzero = 0;
  uint64_t clipped = 0;
  int try_div = 0;
  int try_clamp = 0;
  uint64_t moved[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint64_t moved_up = 0;
  std::vector<uint64_t> all;
  std::vector<uint64_t> nonzero;

  hist_census_t()
      : all(2 * HIST_CENSUS_BOUND + 1, 0), nonzero(2 * HIST_CENSUS_BOUND + 1, 0)
  {
    const char* d = std::getenv("CHESSO_HIST_TRY_DIV");
    const char* c = std::getenv("CHESSO_HIST_TRY_CLAMP");

    if (d != nullptr) { try_div = std::atoi(d); }
    if (c != nullptr) { try_clamp = std::atoi(c); }
  }

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
      << "clipped\t" << hist_census.clipped << '\n'
      << "try_div\t" << hist_census.try_div << '\n'
      << "try_clamp\t" << hist_census.try_clamp << '\n'
      << "moved_up\t" << hist_census.moved_up << '\n';

  for (int i = 0; i < 8; ++i) {
    out << "moved\t" << i << '\t' << hist_census.moved[i] << '\n';
  }

  for (int i = 0; i <= 2 * HIST_CENSUS_BOUND; ++i) {
    if (hist_census.all[i] == 0 && hist_census.nonzero[i] == 0) { continue; }

    out << "bin\t" << (i - HIST_CENSUS_BOUND) << '\t' << hist_census.all[i]
        << '\t' << hist_census.nonzero[i] << '\n';
  }
}

inline void hist_census_site(int sum, int ticks, int node_adjustment)
{
  hist_census.sites++;

  if (ticks != 0) { hist_census.raw_nonzero++; }

  // What the candidate pair would do to this site's whole-ply reduction,
  // counted against the reduction this tree actually takes. The term is off in
  // this build -- the clamp is forced to 0 -- so `plain` is what the engine
  // under census used and `moved` is a counterfactual and not an observation.
  if (hist_census.try_div > 0 && hist_census.try_clamp > 0) {
    int term = (sum * LMR_SCALE) / hist_census.try_div;

    if (term > hist_census.try_clamp) { term = hist_census.try_clamp; }
    if (term < -hist_census.try_clamp) { term = -hist_census.try_clamp; }

    const int plain = lmr_plies_of(ticks + node_adjustment);
    const int moved = lmr_plies_of(ticks + node_adjustment - term);
    int delta = plain - moved;

    if (delta < 0) {
      hist_census.moved_up++;
      delta = -delta;
    }

    if (delta > 7) { delta = 7; }

    hist_census.moved[delta]++;
  }

  int v = sum;

  if (v > HIST_CENSUS_BOUND || v < -HIST_CENSUS_BOUND) {
    hist_census.clipped++;
    v = (v > 0) ? HIST_CENSUS_BOUND : -HIST_CENSUS_BOUND;
  }

  hist_census.all[v + HIST_CENSUS_BOUND]++;

  if (ticks != 0) { hist_census.nonzero[v + HIST_CENSUS_BOUND]++; }
}
}  // namespace
// ---- end S236 census ---------------------------------------------------

'''

PATCH = [
    # The counters, above the helper they count.
    ("// What this **move's own history** is worth against the table's guess,"
     " S236 and",
     COUNTERS
     + "// What this **move's own history** is worth against the table's"
       " guess, S236 and"),
    # The site itself: every call of the helper from the reduction.
    ("        reduction =\n"
     "            lmr_adjusted_reduction(depth,"
     " static_cast<int>(legal_moves_counter),\n"
     "                                   node_adjustment, hist_sum);",
     "        hist_census_site(\n"
     "            hist_sum,\n"
     "            lmr_reduction_ticks(depth,"
     " static_cast<int>(legal_moves_counter)),\n"
     "            node_adjustment);\n"
     "        reduction =\n"
     "            lmr_adjusted_reduction(depth,"
     " static_cast<int>(legal_moves_counter),\n"
     "                                   node_adjustment, hist_sum);"),
    # `<cstdlib>`, `<fstream>` and `<vector>` are not in scope in a release
    # build of this file -- the first two are inside its `#ifdef CHESSO_TUNE`
    # and the third is not included at all -- so the patch brings them.
    ("#include <limits>\n",
     "#include <limits>\n"
     "// S236 census, throwaway: see adocs/data/S236_hist_census.py.\n"
     "#include <cstdlib>\n"
     "#include <fstream>\n"
     "#include <vector>\n"),
]

# The term is forced off in the censused tree, which is what makes the
# distribution the term's input rather than its output. One row, one anchor.
CLAMP_OFF = (
    'X(LMR_HIST_CLAMP,    "LmrHistClamp",    ',
)


def sh(cmd, **kw):
    proc = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if proc.returncode != 0:
        sys.stderr.write(" ".join(cmd) + "\n" + proc.stdout[-4000:] +
                         proc.stderr[-4000:])
        sys.exit("command failed: " + " ".join(cmd))
    return proc.stdout


def bench_positions():
    """The eight the bench runs, as UCI position commands, in its own order."""
    text = open(os.path.join(REPO, "src", "chesso.cpp")).read()
    literals = {}
    for tag in LITERAL_TAGS:
        found = re.findall(LITERAL_RE % tag, text)
        if len(found) != 1:
            sys.exit("src/chesso.cpp does not carry exactly one FEN tagged "
                     "// %s (%d found); the bench list moved and this census "
                     "has to move with it" % (tag, len(found)))
        literals[tag] = found[0]

    return ([("midgame", "position fen " + literals["midgame"]),
             ("kiwipete", "position kiwipete"),
             ("tactical", "position fen " + literals["tactical"])] +
            [(name, "position " + name) for name in SHORTCUTS[1:]])


def force_clamp_off(work):
    """The censused tree is the tree the term is added to, not one a
    provisional term has already moved."""
    path = os.path.join(work, "src", "search_params.hpp")
    text = open(path).read()
    if text.count(CLAMP_OFF[0]) != 1:
        sys.exit("the LmrHistClamp row is not unique in src/search_params.hpp")
    row = re.search(re.escape(CLAMP_OFF[0]) + r"(-?\d+)", text)
    if row is None:
        sys.exit("the LmrHistClamp row has no default to force off")
    shipped = int(row.group(1))
    text = text.replace(CLAMP_OFF[0] + row.group(1),
                        CLAMP_OFF[0] + "0" + " " * (len(row.group(1)) - 1))
    open(path, "w").write(text)
    return shipped


def configure_and_build(work, name):
    build = os.path.join(work, name)
    sh(["cmake", "-S", work, "-B", build, "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
        "-DCHESSO_ARCH=native", "-DCHESSO_TUNE=OFF"])
    sh(["cmake", "--build", build, "--target", "chesso",
        "-j", str(os.cpu_count())])
    return os.path.join(build, "src", "chesso")


def build_pair(work):
    """A control binary and an instrumented one, from the same forced source.

    Two builds rather than one against the working tree's own: the census
    forces LmrHistClamp off in its copy, so the shipping build is a different
    tree by construction and comparing against it would prove nothing. The
    control is what says the counters are write-only.
    """
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

    shipped_clamp = force_clamp_off(work)
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

    return control, census, shipped_clamp


def bench_total(engine):
    out = subprocess.run([engine, "bench"], capture_output=True, text=True)
    for line in reversed(out.stdout.splitlines()):
        if line.endswith("nps"):
            return int(line.split()[0])
    sys.exit("no bench line from " + engine)


def signature(census, control):
    """The instrumented binary must search the same tree as the control.

    The counters are write-only -- nothing in the search reads one -- so the
    bench total has to be the control's exactly, and this compares them rather
    than asserting it in prose. A census taken on a binary that searches a
    different tree measures a different distribution, which is the one way this
    whole file could be quietly wrong.
    """
    total = bench_total(census)
    want = bench_total(control)
    if total != want:
        sys.exit("the instrumented binary searches a different tree: bench "
                 "%d against the control's %d. The counters are write-only, "
                 "so a difference means the patch is not what it claims and "
                 "the census would measure the wrong distribution."
                 % (total, want))
    return total


SCALE_RE = re.compile(r"inline constexpr int LMR_SCALE = 1 << LMR_SCALE_SHIFT")
SHIFT_RE = re.compile(r"inline constexpr int LMR_SCALE_SHIFT = (\d+)")


def accumulator_scale():
    """The engine's own scale, read from the source that defines it."""
    text = open(os.path.join(REPO, "src", "search.cpp")).read()
    found = SHIFT_RE.search(text)
    if found is None or SCALE_RE.search(text) is None:
        sys.exit("src/search.cpp does not define LMR_SCALE as a shift; the "
                 "seed rules below are stated in ticks and cannot be applied")
    return 1 << int(found.group(1))


def drive(engine, depth, census_path, extra_env=None):
    positions = bench_positions()
    env = dict(os.environ, CHESSO_HIST_CENSUS=census_path)
    env.update(extra_env or {})
    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1,
                            env=env)
    engine_io = s024.Engine.__new__(s024.Engine)
    engine_io.p = proc
    engine_io.uci_handshake()

    nodes = 0
    started = time.time()
    for name, command in positions:
        engine_io.ucinewgame()
        engine_io.isready()
        engine_io.send(command)
        engine_io.send("isready")
        lines = engine_io.read_until("readyok")
        refusals = [ln for ln in lines if "refused" in ln]
        if refusals:
            sys.exit("position refused: %r for %s" % (refusals, name))
        lines = engine_io.go_depth(depth)
        last = [l for l in lines if l.startswith("info") and " nodes " in l]
        if not last:
            sys.exit("no node count for " + name)
        nodes += int(last[-1].split(" nodes ")[1].split()[0])
    engine_io.quit()

    return len(positions), nodes, time.time() - started


def read_census(path):
    out = {"sites": 0, "raw_nonzero": 0, "clipped": 0, "try_div": 0,
           "try_clamp": 0, "moved_up": 0}
    bins, moved = {}, {}
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if parts[0] in out:
            out[parts[0]] = int(parts[1])
        elif parts[0] == "bin":
            bins[int(parts[1])] = (int(parts[2]), int(parts[3]))
        elif parts[0] == "moved":
            moved[int(parts[1])] = int(parts[2])
    out["bins"] = bins
    out["moved"] = moved
    return out


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


PCTS = (50, 75, 90, 95, 99)


def abs_counts(bins, column):
    out = {}
    for value, counts in bins.items():
        out[abs(value)] = out.get(abs(value), 0) + counts[column]
    return out


def seeds_from(abs_all, scale, clamp_top):
    """The two seed rules, applied. Stated in the docstring above and applied
    here so the arithmetic is in one place and not in prose.

    The uncapped clamp is returned beside the capped one: when the cap binds,
    the number the rule actually asked for is the reader's only way to see how
    far past the declared range it went, and a file that printed the bound
    alone would read as if the rule had produced it.
    """
    p = percentiles(abs_all, PCTS)
    divisor = 2 * p[90] if p[90] else None
    if not divisor:
        return None, None, None, p, False
    asked = (p[99] * scale) // divisor
    clamp = asked
    capped = clamp > clamp_top
    if capped:
        clamp = clamp_top
    return divisor, clamp, asked, p, capped


def summarise(depth, positions, nodes, wall, data, scale, clamp_top):
    bins = data["bins"]
    sites = data["sites"]
    signed_all = {v: c[0] for v, c in bins.items() if c[0]}
    abs_all = abs_counts(bins, 0)
    abs_nz = abs_counts(bins, 1)

    lines = []
    add = lines.append
    add("depth %d" % depth)
    add("  positions: %d   nodes: %d   wall: %.1f s" % (positions, nodes, wall))
    add("  sites: %d   of them with a non-zero raw table value: %d (%.2f %%)"
        % (sites, data["raw_nonzero"],
           100.0 * data["raw_nonzero"] / sites if sites else 0.0))
    add("  clipped at the histogram bound: %d" % data["clipped"])
    for name, counts in (("|sum|, all sites", abs_all),
                         ("|sum|, raw table non-zero", abs_nz),
                         ("signed sum, all sites", signed_all)):
        p = percentiles(counts, PCTS)
        add("  %-28s p50 %7s  p75 %7s  p90 %7s  p95 %7s  p99 %7s"
            % (name, p[50], p[75], p[90], p[95], p[99]))
    add("  zero sums: %d (%.2f %%)"
        % (abs_all.get(0, 0),
           100.0 * abs_all.get(0, 0) / sites if sites else 0.0))

    divisor, clamp, asked, p, capped = seeds_from(abs_all, scale, clamp_top)
    if divisor:
        add("  seed rules at this depth: LmrHistDiv = 2 * p90 = %d, "
            "LmrHistClamp = p99 * %d / LmrHistDiv = %d%s"
            % (divisor, scale, asked,
               "  ticks, which is %.1f plies -- CAPPED to the declared top %d"
               % (asked / float(scale), clamp_top) if capped else ""))
        add("  at that divisor the p50 site is worth %.3f of a ply and the "
            "p90 site %.3f"
            % (p[50] * scale / divisor / scale,
               p[90] * scale / divisor / scale))
    return "\n".join(lines), {"divisor": divisor, "clamp": clamp,
                              "abs_all": abs_all, "sites": sites}


def summarise_try(data, scale):
    """What the derived pair would do to the tree it was derived from."""
    sites = data["sites"]
    moved = data["moved"]
    total_moved = sum(c for d, c in moved.items() if d > 0)
    lines = ["what the seeded pair would do to this tree (a counterfactual "
             "on the tree without the term, not an observation of one with "
             "it)",
             "  LmrHistDiv %d   LmrHistClamp %d (%.3f of a ply)"
             % (data["try_div"], data["try_clamp"],
                data["try_clamp"] / float(scale))]
    lines.append("  sites whose whole-ply reduction moves: %d of %d (%.2f %%)"
                 % (total_moved, sites,
                    100.0 * total_moved / sites if sites else 0.0))
    for delta in sorted(moved):
        if moved[delta] == 0:
            continue
        lines.append("    by %d ply: %d (%.2f %%)"
                     % (delta, moved[delta],
                        100.0 * moved[delta] / sites if sites else 0.0))
    lines.append("  of the moves, the ones that reduce **more** (a negative "
                 "history sum): %d" % data["moved_up"])
    return "\n".join(lines)


def cmd_census(args):
    work = os.path.join(REPO, ".ref-builds", "s236census")
    scale = accumulator_scale()
    control, census, shipped_clamp = build_pair(work)
    total = signature(census, control)
    print("instrumented bench signature: %d, equal to the control's" % total)
    print("accumulator scale: %d ticks to the ply; the censused tree has "
          "LmrHistClamp forced to 0 (it ships at %d)" % (scale, shipped_clamp))

    depths = [args.depth] if args.depth else list(DEPTHS)
    clamp_top = args.clamp_top

    blocks, derived = [], {}
    for depth in depths:
        path = os.path.join(work, "census_d%d.tsv" % depth)
        positions, nodes, wall = drive(census, depth, path)
        data = read_census(path)
        block, summary = summarise(depth, positions, nodes, wall, data, scale,
                                   clamp_top)
        print(block)
        blocks.append(block)
        derived[depth] = summary

    # The seeds are depth 12's, which is the depth the step file names and the
    # one an 8+0.08 game lives nearest.
    seed_depth = 12 if 12 in derived else depths[-1]
    divisor = derived[seed_depth]["divisor"]
    clamp = derived[seed_depth]["clamp"]

    if divisor and clamp:
        path = os.path.join(work, "census_try.tsv")
        positions, nodes, wall = drive(
            census, seed_depth, path,
            {"CHESSO_HIST_TRY_DIV": str(divisor),
             "CHESSO_HIST_TRY_CLAMP": str(clamp)})
        block = summarise_try(read_census(path), scale)
        print(block)
        blocks.append(block)

    header = [
        "# S236: the history sum where the fractional reduction term reads it.",
        "# Regenerate with:",
        "#   ~/.venv/chess/bin/python adocs/data/S236_hist_census.py census",
        "# The script's docstring is the method; it holds the instrumentation",
        "# patch, so this file is reproducible from the repository alone.",
        "# The eight bench positions of src/chesso.cpp's bench_positions, one",
        "# persistent UCI process, ucinewgame between positions, go depth N.",
        "# A site is one call of lmr_adjusted_reduction in negamax_at.",
        "# Percentiles are nearest-rank, so every number is one the census saw.",
        "# Instrumented bench signature: %d, equal to a control binary built"
        % total,
        "# from the same source without the counters -- which is what says the",
        "# write-only counters do not move the tree the census is taken on.",
        "# The censused tree has LmrHistClamp forced to 0, the term's own off",
        "# value, so this is the distribution the term is added to and not one",
        "# a provisional term has already moved. It ships at %d."
        % shipped_clamp,
        "# Accumulator scale: %d ticks to the ply (LMR_SCALE, src/search.cpp)."
        % scale,
        "#",
        "# THE SEED RULES, stated in the script before these numbers existed:",
        "#   LmrHistDiv   = 2 * p90(|sum|), so the p90 site is half a ply",
        "#   LmrHistClamp = p99(|sum|) * scale / LmrHistDiv, capped at the",
        "#                  declared range top of %d ticks" % clamp_top,
        "#",
        "# ONE PASS, AND THAT IS A LIMIT ON WHAT A PERCENTILE FROM IT MEANS.",
        "# The distribution belongs to the tree the census ran on. A divisor",
        "# taken from it changes the reduction, which changes the tree, which",
        "# changes the distribution -- so the percentile is the 90th of the",
        "# tree that was measured and not of the tree that then ships. The",
        "# fixed point is not iterated here; S127's lane fits both constants",
        "# against games, which is what a second pass would not give.",
        "#",
        "# Not Elo and not a verdict (DEC-019): it sizes two seeds.",
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
    one.add_argument("--clamp-top", type=int, default=2048,
                     help="the declared range top of LmrHistClamp, in ticks; "
                          "the seed is capped at it and the cap is reported")
    one.set_defaults(run=cmd_census)
    args = parser.parse_args()
    return args.run(args)


if __name__ == "__main__":
    sys.exit(main())
