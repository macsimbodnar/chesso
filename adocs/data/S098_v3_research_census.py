#!/usr/bin/env python3
"""S098 verdict 3: what the reduced fail-high re-search sites actually look
like, taken before any match.

    ~/.venv/chess/bin/python adocs/data/S098_v3_research_census.py census
    ~/.venv/chess/bin/python adocs/data/S098_v3_research_census.py census --depth 12
    ~/.venv/chess/bin/python adocs/data/S098_v3_research_census.py census --keep

WHY IT EXISTS, DEC-214 and DEC-212. Verdict 1 cost four settings, two SPRTs and
a night's SPSA lane to learn that its first seed was inert, and nothing said so
until a census was taken. Verdict 2's four terms were ply counts with no scale,
so a count of how often each condition fired was enough. Verdict 3's two
margins **are** scales -- they are compared against search scores in chesso's
material scale -- so this census asks the question DEC-212 exists for: at the
seed, does each path fire on a share of sites worth measuring?

THE SEED RULE, STATED BEFORE THE RUN. The (c) seed 47, the midpoint of the
0-to-PAWN range the step's section 4 declares, stands for a path that fires at
**one per cent of re-search sites or more**. A path under one per cent is
re-seeded by DEC-105 derivation **(b)** from this census -- the margin at the
quantile that makes it fire at about a tenth of sites, stated as such. A path
that cannot be made to fire at one per cent anywhere inside 0 to PAWN ships at
its off value and is recorded as inert rather than measured.

WHAT IS COUNTED, AND WHERE. One site is one execution of the reduced fail-high
re-search in `negamax_at`: the block guarded by
`!state->aborted && reduction > 0 && score > alpha`, which is the zero-window
repeat a *reduced* late move that beat alpha is owed, and the only place
`lmr_research_depth` is consulted. Nothing else is a site -- not the
first-move full-window search, not the full-window re-search below it, not a
late move that was never reduced. For each site four numbers are recorded:

    r        the reduction actually taken, after every clamp
    score    what the reduced zero-window search returned
    alpha    the node's alpha at that moment, which a PV node raises as it goes
    best     `best_so_far` **before** this move updates it, the fail-soft base
             the deeper margin is measured from (the 65e2150 re-basing)

and kept as histograms rather than a row per site, because the sites run to
millions and every question asked of them is a share or a quantile. Four
histograms of one-unit bins over -8192 to 8192 with named overflow buckets --
`score - best` and `score - alpha`, each over all sites and again over the
sites with `r >= 2` -- plus a histogram of `r` itself. The pair conditioned on
`r >= 2` is what makes each **condition's** share readable at any margin
without re-running.

**A condition's share is not a path's share, and only the second answers
DEC-214.** The shallower path is tested first, so a site where both conditions
hold belongs to it; the deeper path fires only on sites the shallower one left.
The histograms are one-dimensional and cannot see that overlap, so the three
path outcomes are counted in the engine at the shipped seeds -- read out of
src/search_params.hpp and substituted into the instrumentation below, so they
cannot drift from what the engine evaluates -- and printed beside the two
conditions. Re-seeding a margin therefore needs another run, which is the price
of counting the thing the decision is actually about.

Quantiles are exact inside the range because the bins are one unit wide; a
quantile that falls in an overflow bucket is printed as such and never
interpolated.

**THE CENSUS IS TAKEN ON THE OFF TREE**, which is why the patch below also
writes the two off values into the worktree's copy of src/search_params.hpp:
`LmrShallowerMargin` 0 and `LmrDeeperMinReduction` at its range top. The tree
the shares describe is then the tree the SPRT's reference searches -- the
engine before verdict 3 -- which is the tree the question "would this path ever
fire" is asked of. It has an exact signature to check against, and that check
is the second thing this script buys: the instrumented binary's `bench` must
equal **5469072**, the total the commit before this landing prints, which says
both that the write-only counters do not move the tree and that the whole of
verdict 3 is inert at its off values in the Release build.

It inherits verdict 1's one-pass caveat and states it rather than hiding it:
the distribution belongs to the off tree, and shipping the rule changes the
tree and therefore the distribution. What survives that caveat is the reading
this census is taken for -- whether a path fires at all, and by roughly how
much the two margins differ in reach -- and not a fixed point.

THE POSITIONS ARE S024'S AND THE DRIVER IS S024'S, exactly as
`adocs/data/S098_v2_node_census.py` used them: `adocs/data/S024_census_positions.txt`
-- 400 positions, one per game, from `adocs/data/S219_aa_calibration.pgn` --
read through that file's own `read_positions()` and driven through its `Engine`
class, one persistent UCI process, `ucinewgame` between positions, `go depth N`
awaited to `bestmove`. TOOLCHAIN.md's oracle trap is general and that driver is
the form that does not fall into it. Reusing them is the point: the three
censuses are over the same board positions and can be read beside each other.

`go depth N` and not a single `search()` call, deliberately: a game's `go` is
iterative deepening, so the window a re-search site sees is the one the
aspiration loop and the table produced, which a cold fixed-depth call would
not show.

THE BINARY IS A THROWAWAY AND THE SHIPPED TREE IS NEVER TOUCHED. A detached
worktree is created under `.ref-builds/`, the **working tree's** `src/` is
copied into it -- so what is measured is the landing and not `HEAD` -- the
patch below is applied there, and the worktree is removed when the run ends.
The instrumentation is not in the shipping engine and never was: it writes one
file named by `CHESSO_RESEARCH_CENSUS` at exit and prints nothing, so it is not
on the UCI surface `tests/test_uci_surface.cpp` guards. The patch is in this
file rather than in a diff beside it so the run is reproducible from the script
alone.

WHAT IT IS NOT. Not a strength measurement, and nothing here reads it as one
(DEC-019). It sizes two seeds; `adocs/data/S098_v3_sprt.sh` decides the verdict.
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

OUT_TXT = os.path.join(REPO, "adocs", "data", "S098_v3_research_census.txt")
DEPTHS = (10, 12)

# The bench total of the tree this census is taken on: the commit before
# verdict 3's landing, which is also this landing with its two off values in
# place. Not a golden in tests/ and not read from a run -- it is the signature
# the whole method rests on and the script refuses to write a file without it.
OFF_TREE_BENCH = 5469072

# The histogram's half-width. One-unit bins, so a quantile inside the range is
# exact; outside it the bucket is named and never interpolated.
SPAN = 8192

# The seed under test and the top of its declared range, both from the step's
# section 4. PAWN is 94 in src/eval_tables.hpp.
SEED = 47
PAWN = 94


def shipped_seeds():
    """The three defaults as src/search_params.hpp holds them, read and not
    copied: the instrumentation counts the rule as the engine evaluates it, and
    a seed hard-coded here would go stale the first time S127 refits one."""
    import re

    text = open(os.path.join(REPO, "src", "search_params.hpp")).read()
    out = {}
    for symbol in ("LMR_DEEPER_MARGIN", "LMR_SHALLOWER_MARGIN",
                   "LMR_DEEPER_MIN_REDUCTION"):
        found = re.search(r"X\(%s,\s*\"[A-Za-z]+\",\s*(-?\d+)," % symbol, text)
        if found is None:
            sys.exit("cannot read %s from src/search_params.hpp" % symbol)
        out[symbol] = int(found.group(1))
    return out

# DEC-214's threshold, as a percentage of sites.
INERT_BELOW = 1.0

# The instrumentation. One anchor per pair, each occurring exactly once in the
# file it names, applied to the copy in the throwaway worktree and never to the
# tree this script runs from.
COUNTERS = r'''
// ---- S098 verdict-3 re-search census, throwaway instrumentation ------
// Not in the shipping engine: adocs/data/S098_v3_research_census.py patches it
// in to a detached worktree, builds there, and removes the worktree afterwards.
// One site is one execution of the reduced fail-high re-search block.
namespace
{
constexpr int RESEARCH_CENSUS_SPAN = 8192;
constexpr int RESEARCH_CENSUS_BINS = 2 * RESEARCH_CENSUS_SPAN + 1;

// The shipped seeds, substituted by the script from src/search_params.hpp so
// they cannot drift from the values the engine ships. The worktree's own
// constants are patched to their off values, which is why they cannot be read
// from there.
constexpr int RESEARCH_CENSUS_DEEPER = @@DEEPER@@;
constexpr int RESEARCH_CENSUS_SHALLOWER = @@SHALLOWER@@;
constexpr int RESEARCH_CENSUS_MIN_REDUCTION = @@MIN_REDUCTION@@;

struct research_census_t
{
  uint64_t sites = 0;
  uint64_t guarded = 0;  // sites with r >= 2

  // The three outcomes of the rule at the shipped seeds, **after precedence**:
  // the shallower path is tested first, so a site where both conditions hold
  // belongs to it and not to the deeper one. A share of a condition is not a
  // share of a path and the two are kept apart here.
  uint64_t path_shallower = 0;
  uint64_t path_deeper = 0;
  uint64_t path_unchanged = 0;

  // The conditions on their own, before precedence, which is what the margins
  // themselves are seeded against.
  uint64_t cond_shallower = 0;
  uint64_t cond_deeper = 0;
  uint64_t cond_both = 0;

  uint64_t reduction[128] = {};

  // [0] over-best all sites, [1] over-best where r >= 2,
  // [2] over-alpha all sites, [3] over-alpha where r >= 2.
  uint64_t hist[4][RESEARCH_CENSUS_BINS] = {};
  uint64_t under[4] = {};
  uint64_t over[4] = {};

  ~research_census_t();
};

research_census_t research_census;

inline void research_census_bin(int which, int value)
{
  if (value < -RESEARCH_CENSUS_SPAN) {
    research_census.under[which]++;
  } else if (value > RESEARCH_CENSUS_SPAN) {
    research_census.over[which]++;
  } else {
    research_census.hist[which][value + RESEARCH_CENSUS_SPAN]++;
  }
}

research_census_t::~research_census_t()
{
  const char* path = std::getenv("CHESSO_RESEARCH_CENSUS");

  if (path == nullptr) { return; }

  std::ofstream out(path);

  if (!out) { return; }

  out << "sites\t" << research_census.sites << '\n'
      << "guarded\t" << research_census.guarded << '\n'
      << "path_shallower\t" << research_census.path_shallower << '\n'
      << "path_deeper\t" << research_census.path_deeper << '\n'
      << "path_unchanged\t" << research_census.path_unchanged << '\n'
      << "cond_shallower\t" << research_census.cond_shallower << '\n'
      << "cond_deeper\t" << research_census.cond_deeper << '\n'
      << "cond_both\t" << research_census.cond_both << '\n'
      << "span\t" << RESEARCH_CENSUS_SPAN << '\n';

  for (int r = 0; r < 128; ++r) {
    if (research_census.reduction[r] != 0) {
      out << "reduction\t" << r << '\t' << research_census.reduction[r] << '\n';
    }
  }

  for (int which = 0; which < 4; ++which) {
    out << "under\t" << which << '\t' << research_census.under[which] << '\n';
    out << "over\t" << which << '\t' << research_census.over[which] << '\n';

    for (int i = 0; i < RESEARCH_CENSUS_BINS; ++i) {
      if (research_census.hist[which][i] != 0) {
        out << "bin\t" << which << '\t' << (i - RESEARCH_CENSUS_SPAN) << '\t'
            << research_census.hist[which][i] << '\n';
      }
    }
  }
}

inline void research_census_site(int reduction, int score, int alpha, int best)
{
  research_census.sites++;

  if (reduction >= 0 && reduction < 128) {
    research_census.reduction[reduction]++;
  }

  const int over_best = score - best;
  const int over_alpha = score - alpha;

  research_census_bin(0, over_best);
  research_census_bin(2, over_alpha);

  if (reduction >= 2) {
    research_census.guarded++;
    research_census_bin(1, over_best);
    research_census_bin(3, over_alpha);
  }

  // The rule as `lmr_research_depth` actually evaluates it, at the seeds the
  // step ships. The literals come from src/search_params.hpp through this
  // script, which reads them and refuses to run if it cannot.
  const bool shallower =
      reduction >= 2 && score < alpha + RESEARCH_CENSUS_SHALLOWER;
  const bool deeper = reduction >= RESEARCH_CENSUS_MIN_REDUCTION &&
                      score > best + RESEARCH_CENSUS_DEEPER;

  if (shallower) { research_census.cond_shallower++; }
  if (deeper) { research_census.cond_deeper++; }
  if (shallower && deeper) { research_census.cond_both++; }

  if (shallower) {
    research_census.path_shallower++;
  } else if (deeper) {
    research_census.path_deeper++;
  } else {
    research_census.path_unchanged++;
  }
}
}  // namespace
// ---- end S098 verdict-3 census ---------------------------------------

'''

PATCH = [
    # The counters, above the rule whose site they count.
    ("// The depth the zero-window re-search of a reduced move runs at, S098\n"
     "// verdict 3.",
     COUNTERS
     + "// The depth the zero-window re-search of a reduced move runs at, S098"
       "\n// verdict 3."),
    # The site itself: the one execution of the reduced fail-high re-search.
    ("        assert(best_so_far > MIN);\n",
     "        assert(best_so_far > MIN);\n"
     "        research_census_site(reduction, score, alpha, best_so_far);\n"),
    # `<cstdlib>` and `<fstream>` are not in scope in a release build of this
    # file -- both are inside its `#ifdef CHESSO_TUNE` -- so the patch brings
    # them.
    ("#include <limits>\n",
     "#include <limits>\n"
     "// S098 census, throwaway: see adocs/data/S098_v3_research_census.py.\n"
     "#include <cstdlib>\n"
     "#include <fstream>\n"),
]

# The off tree. The two off values in the worktree's copy, which is what makes
# the bench signature above checkable and the distribution the reference
# tree's. `LmrDeeperMargin` is deliberately **not** patched: its range top is
# not an off value, which is one of the things this census measures.
OFF_PATCH = [
    ('  X(LMR_SHALLOWER_MARGIN,     "LmrShallowerMargin",     47, 0, 94)',
     '  X(LMR_SHALLOWER_MARGIN,     "LmrShallowerMargin",      0, 0, 94)'),
    ('  X(LMR_DEEPER_MIN_REDUCTION, "LmrDeeperMinReduction",   2, 1, 126)',
     '  X(LMR_DEEPER_MIN_REDUCTION, "LmrDeeperMinReduction", 126, 1, 126)'),
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

    seeds = shipped_seeds()
    counters = (COUNTERS
                .replace("@@DEEPER@@", str(seeds["LMR_DEEPER_MARGIN"]))
                .replace("@@SHALLOWER@@", str(seeds["LMR_SHALLOWER_MARGIN"]))
                .replace("@@MIN_REDUCTION@@",
                         str(seeds["LMR_DEEPER_MIN_REDUCTION"])))
    patch = [(old, new.replace(COUNTERS, counters)) for old, new in PATCH]

    apply_patch(os.path.join(work, "src", "search.cpp"), patch)

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
    two off values are patched in, so the total has to be the off tree's
    exactly. A difference means either the patch moved the tree or verdict 3 is
    not inert at its off values, and both would make every share below a share
    of some other distribution.
    """
    total = bench_total(engine)
    if total != expect:
        sys.exit("the instrumented binary searches a different tree: bench "
                 "%d against the expected %d. Either the counters moved the "
                 "tree or S098 verdict 3 is not inert at its off values; the "
                 "census is not taken until that is resolved." % (total, expect))
    return total


def drive(engine, depth, census_path):
    positions = s024.read_positions()
    env = dict(os.environ, CHESSO_RESEARCH_CENSUS=census_path)
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


class histogram_t:
    """One-unit bins with named overflow buckets, so a quantile inside the
    range is exact and one outside it is reported and never interpolated."""

    def __init__(self):
        self.bins = {}
        self.under = 0
        self.over = 0

    @property
    def total(self):
        return sum(self.bins.values()) + self.under + self.over

    def quantile(self, q):
        """The smallest value v with P(X <= v) >= q, or a bucket name."""
        total = self.total
        if total == 0:
            return "no sites"
        want = q * total
        seen = self.under
        if seen >= want:
            return "< -%d" % SPAN
        for value in sorted(self.bins):
            seen += self.bins[value]
            if seen >= want:
                return value
        return "> %d" % SPAN

    def count_above(self, margin):
        """P(X > margin) * total, with the overflow bucket counted in."""
        return self.over + sum(c for v, c in self.bins.items() if v > margin)

    def count_below(self, margin):
        """P(X < margin) * total, with the underflow bucket counted in."""
        return self.under + sum(c for v, c in self.bins.items() if v < margin)


def read_census(path):
    counts = {}
    reduction = {}
    hists = [histogram_t() for _ in range(4)]
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if parts[0] == "bin":
            hists[int(parts[1])].bins[int(parts[2])] = int(parts[3])
        elif parts[0] == "under":
            hists[int(parts[1])].under = int(parts[2])
        elif parts[0] == "over":
            hists[int(parts[1])].over = int(parts[2])
        elif parts[0] == "reduction":
            reduction[int(parts[1])] = int(parts[2])
        else:
            counts[parts[0]] = int(parts[1])
    return counts, reduction, hists


QUANTILES = (0.10, 0.25, 0.50, 0.75, 0.90)


def fmt_quantiles(hist):
    return "  ".join("p%02d %s" % (int(q * 100), hist.quantile(q))
                     for q in QUANTILES)


def summarise(depth, positions, nodes, wall, counts, reduction, hists):
    sites = counts["sites"]
    guarded = counts["guarded"]
    over_best_all, over_best_guarded, over_alpha_all, over_alpha_guarded = hists

    lines = []
    add = lines.append
    add("depth %d" % depth)
    add("  positions: %d   nodes: %d   wall: %.1f s" % (positions, nodes, wall))
    add("  re-search sites: %d" % sites)
    add("  with r >= 2:     %d  (%.2f %%)"
        % (guarded, 100.0 * guarded / sites if sites else 0.0))
    add("  the reduction taken, share of sites:")
    for r in sorted(reduction):
        add("    r = %-3d %12d  %6.2f %%"
            % (r, reduction[r], 100.0 * reduction[r] / sites if sites else 0.0))

    add("  score - best   (all sites)    %s" % fmt_quantiles(over_best_all))
    add("  score - best   (r >= 2)       %s" % fmt_quantiles(over_best_guarded))
    add("  score - alpha  (all sites)    %s" % fmt_quantiles(over_alpha_all))
    add("  score - alpha  (r >= 2)       %s"
        % fmt_quantiles(over_alpha_guarded))

    deeper_top = over_best_guarded.count_above(PAWN)

    def pct(count):
        return 100.0 * count / sites if sites else 0.0

    # **THE PATHS, AFTER PRECEDENCE**, which is what DEC-214's inert test is
    # about: the shallower path is tested first, so a site where both
    # conditions hold belongs to it. Counted in the engine, not derived from
    # the one-dimensional histograms below, which cannot see the overlap.
    add("  the three outcomes of the rule at the shipped seeds, after")
    add("  precedence -- this is the firing share of each PATH:")
    for key, label in (("path_shallower", "shallower"),
                       ("path_deeper", "deeper"),
                       ("path_unchanged", "unchanged")):
        share = pct(counts[key])
        add("    %-10s %12d of %d sites  %6.2f %%%s"
            % (label, counts[key], sites, share,
               "   INERT (< %.0f %%)" % INERT_BELOW
               if label != "unchanged" and share < INERT_BELOW else ""))

    # The conditions on their own, which is what each margin is seeded
    # against, and the overlap that separates them from the paths above.
    add("  the two conditions before precedence, and their overlap:")
    add("    shallower  r >= 2 and score < alpha + seed   %12d  %6.2f %%"
        % (counts["cond_shallower"], pct(counts["cond_shallower"])))
    add("    deeper     r >= min and score > best + seed  %12d  %6.2f %%"
        % (counts["cond_deeper"], pct(counts["cond_deeper"])))
    add("    both hold                                    %12d  %6.2f %%"
        % (counts["cond_both"], pct(counts["cond_both"])))
    add("  the deeper condition at the range top, which section 4 expected to")
    add("  be an off value:")
    add("    deeper     r >= 2 and score > best + %-3d %12d  %6.2f %%"
        % (PAWN, deeper_top, pct(deeper_top)))

    # What a (b) re-seed would be, whether or not it is needed: the margin at
    # which each path fires at about a tenth of sites.
    add("  the margin that makes each CONDITION hold at about a tenth of")
    add("  sites, which is the DEC-105 (b) re-seed if the (c) seed is inert.")
    add("  A condition and not a path: a re-seed moves the overlap too, so the")
    add("  path share at a new margin needs another run of this script.")
    for name, hist, above in (("deeper", over_best_guarded, True),
                              ("shallower", over_alpha_guarded, False)):
        want = 0.10 * sites
        pick = None
        for m in range(0, PAWN + 1):
            count = (hist.count_above(m) if above else hist.count_below(m))
            if above and count <= want:
                pick = m
                break
            if not above and count >= want:
                pick = m
                break
        if pick is None:
            add("    %-10s no margin inside 0 to %d reaches a tenth of sites"
                % (name, PAWN))
        else:
            count = (hist.count_above(pick) if above
                     else hist.count_below(pick))
            add("    %-10s margin %3d fires at %.2f %% of sites"
                % (name, pick, pct(count)))

    return "\n".join(lines)


def cmd_census(args):
    work = os.path.join(REPO, ".ref-builds", "s098v3census")
    engine = build_instrumented(work, not args.on_tree)
    total = signature(engine, args.expect_bench)
    print("instrumented bench signature: %d, the off tree's" % total)

    blocks = []
    for depth in ([args.depth] if args.depth else DEPTHS):
        path = os.path.join(work, "census_d%d.tsv" % depth)
        positions, nodes, wall = drive(engine, depth, path)
        counts, reduction, hists = read_census(path)
        block = summarise(depth, positions, nodes, wall, counts, reduction,
                          hists)
        print(block)
        blocks.append(block)

    header = [
        "# S098 verdict 3: the reduced fail-high re-search sites and what the",
        "# two margins would fire at. DEC-214, taken before any match.",
        "# Regenerate with:",
        "#   ~/.venv/chess/bin/python adocs/data/S098_v3_research_census.py census",
        "# The script's docstring is the method; it holds the instrumentation",
        "# patch, so this file is reproducible from the repository alone.",
        "# 400 positions of adocs/data/S024_census_positions.txt, one",
        "# persistent UCI process, ucinewgame between positions, go depth N.",
        "# A site is one execution of the reduced fail-high re-search block in",
        "# negamax_at -- `!state->aborted && reduction > 0 && score > alpha`,",
        "# the zero-window repeat a reduced late move that beat alpha is owed",
        "# and the only place lmr_research_depth is consulted. `best` is",
        "# best_so_far before the move updates it.",
        "#",
        "# TAKEN ON THE OFF TREE. LmrShallowerMargin 0 and",
        "# LmrDeeperMinReduction at its range top are patched into the",
        "# throwaway worktree, so the distribution belongs to the tree the",
        "# SPRT's reference searches. Instrumented bench signature: %d, equal"
        % total,
        "# to the total the commit before this landing prints, which says both",
        "# that the write-only counters do not move the tree and that verdict",
        "# 3 is inert at its off values in the Release build.",
        "#",
        "# THE SEED RULE, written before the run: the (c) seed 47 stands for a",
        "# path firing at one per cent of sites or more; a path under one per",
        "# cent is re-seeded by DEC-105 (b) at the quantile that makes it fire",
        "# at about a tenth of sites; a path that cannot reach one per cent",
        "# anywhere inside 0 to PAWN ships at its off value as inert.",
        "#",
        "# A CONDITION'S SHARE IS NOT A PATH'S SHARE. The shallower path is",
        "# tested first, so a site where both conditions hold belongs to it and",
        "# the deeper path fires only on what is left. DEC-214's inert test is",
        "# about the path, so the three outcomes after precedence are counted",
        "# in the engine at the shipped seeds and the conditions are printed",
        "# beside them, labelled as conditions. The histograms are",
        "# one-dimensional and cannot see the overlap, so a re-seed needs",
        "# another run of this script.",
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
