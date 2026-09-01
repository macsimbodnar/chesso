#!/usr/bin/env python3
"""S154. What the constructed set's mate counts tolerate, measured per position.

    python3 adocs/data/S154_floor_margin_sweep.py here
    python3 adocs/data/S154_floor_margin_sweep.py hash
    python3 adocs/data/S154_floor_margin_sweep.py refs
    python3 adocs/data/S154_floor_margin_sweep.py floor

WHAT THIS ANSWERS THAT S145's SWEEP DOES NOT.

`tests/test_engine.cpp` asserts `exact_by_distance[3] >= MATE_IN_THREE_FLOOR`
and the comment beside it claims the placement makes the floor fail "when the
guard fails and not when the tree shifts underneath it". S145 measured the first
half of that sentence -- the count at several `RfpMinPly` settings -- and
asserted the second half without a number. This script measures the second half:
how far the count moves when nothing about the guard changes at all.

Three probes, and they are not interchangeable:

  hash   the transposition table is resized and nothing else. No rule changes,
         no parameter moves; only which nodes get a table hit, which reshapes
         the tree. This is the noise floor -- movement here is movement with no
         cause at all, and a floor inside it proves nothing.
  refs   the same set through the binary built at each commit that touched
         `src/` since the floor was placed. This is the realised drift: what
         unrelated steps actually cost the count, which is the quantity the
         audit finding asks for.
  floor  the `RfpMinPly` table re-taken at the current tree, with the declared
         minimum relaxed in a throwaway worktree so 1 and 0 are reachable.
         S142 made 2 the minimum on S145's own evidence, so a shipping binary
         refuses anything below it and reads as a perfect null -- the mistake
         S156 made and recorded.

COUNTS ARE NOT ENOUGH, SO THE IDENTITY IS CARRIED.

A net count of 9 can hold still while four positions swap in and out, and a
floor sized off the net count would then be sized off a coincidence. Every row
prints a mask -- one character per position in TSV order, `#` exact and `.` not
-- and the churn column is the Hamming distance from the row above it. That is
the number the floor has to survive, and it is always at least the change in the
count and often more.

THE ASSERTION IS STRONGER THAN THE COUNT, FOR MATE IN TWO.

The gate does not merely require a mate in two to be found. It requires
`first_exact == 2m - 1`: reported at the very first iteration deep enough to
hold it. So `m2` prints two numbers, `exact` and `on time`, and the second is
what goes red. S145's log and step file characterise the `RfpMinPly` 1 failure
by the first -- "13 of 16" -- and the assertion fails 7 of 16 there, not 3.

The engine is driven over UCI with the standard library alone, deliberately:
nothing here asks a chess question, it only reads back a score the engine
printed, and the set's labels were proved once by S145's two oracles and are
read from the TSV. `ucinewgame` before every position and a fresh process per
setting, so a row is a property of the setting and not of the order it was
measured in.

DEC-016. Nothing is copied: the positions are this project's own construction
and the distances were proved by exhaustive enumeration, not taken from anyone.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import time

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TSV = os.path.join(REPO, "adocs", "data", "S145_mate_set.tsv")
SHIPPING_ENGINE = os.path.join(REPO, "build", "src", "chesso")
TUNE_ENGINE = os.path.join(REPO, "build-tune", "src", "chesso")

PARAMS = os.path.join("src", "search_params.hpp")
CMAKE = "CMakeLists.txt"

# Matched whole so that a move in the default or the bounds stops this script
# instead of letting it measure the wrong engine. S156 uses the same two lines.
SHIPPING_BOUND = 'X(RFP_MIN_PLY,       "RfpMinPly",       3,      2, 63)'
RELAXED_BOUND = 'X(RFP_MIN_PLY,       "RfpMinPly",       3,      0, 63)'
WEAKENED_BOUND = 'X(RFP_MIN_PLY,       "RfpMinPly",       1,      0, 63)'

# The mate-in-two clause aborts the case, so with the guard weakened nothing
# downstream of it ever runs and the floor's own red would never be seen. The
# `red` mode demotes those three to CHECK in the throwaway worktree, which
# records the failure and carries on to the assertion that is the subject.
# REQUIRE is what ships; this is an observation harness and nothing else.
M2_REQUIRE = """        REQUIRE_MESSAGE(last.is_mate, title);
        REQUIRE_MESSAGE(last.value == 2, title);
        REQUIRE_MESSAGE(first_exact == minimum,"""
M2_CHECK = """        CHECK_MESSAGE(last.is_mate, title);
        CHECK_MESSAGE(last.value == 2, title);
        CHECK_MESSAGE(first_exact == minimum,"""

# And the same for the count that follows the loop: 13 of 16 fails it, which is
# fatal, and the floor is the very next line.
M2_TOTAL_REQUIRE = "REQUIRE(exact_by_distance[2] == total_by_distance[2]);"
M2_TOTAL_CHECK = "CHECK(exact_by_distance[2] == total_by_distance[2]);"

# Pre-S167 commits do not compile under Apple clang's -Werror -- S167 deleted
# the dead constant that broke it, on 2026-08-27, and every ref older than that
# is in the sweep. Dropping -Werror changes no codegen; it is patched in the
# throwaway worktree and never in the tracked tree.
WERROR = "add_compile_options(-Wall -Wextra -Werror)"
NO_WERROR = "add_compile_options(-Wall -Wextra)"

# Above 2m - 1, and this is `MATE_DEPTH_SLACK` in tests/test_engine.cpp. The
# gate searches to exactly this depth, so a reading taken at any other one is
# not a reading of the gate.
EXTRA_DEPTH = 8

HASH_VALUES = [1, 2, 4, 8, 16, 32, 64, 128, 256]

# Every commit that touched src/ since 14748c9 placed the floor, oldest first
# and complete: `git log --reverse 14748c9..HEAD -- src/`. Nothing is selected
# out, because a sweep that picks its own commits is picking its own answer.
# The ones expected to be inert at a fixed depth are marked and are the
# controls -- a comment, a citation, a dead constant, a format pin.
REFS = [
    ("14748c9", "S145  the floor is placed here, at 8 of 16"),
    ("ac4c588", "S142  RfpMinPly bounded at 2, history at its band"),
    ("bfc0463", "S142  a citation is repointed  [control]"),
    ("229a309", "S149  the duplicated killer slots are kept  [control]"),
    ("40f5b56", "S093  quiet history malus, gravity, butterfly"),
    ("8eb24e9", "S130  stand pat takes the table score"),
    ("ecd735e", "S161  load_FEN clears unsupportable fields"),
    ("ea9ba2c", "S162  checkmate outranks the hundredth halfmove"),
    ("ba0194a", "S163  a stale hard-limit timer is disarmed"),
    ("aa8c077", "S165  null move guarded at both mate edges"),
    ("3b39e5a", "S108  the static evaluation is hoisted"),
    ("b4d688b", "S108  the static evaluation is stored per node"),
    ("672e35a", "S167  a dead constant is deleted  [control]"),
    ("e424032", "S024  one-ply continuation history, later reverted"),
    ("cf5758d", "DEC-110  the clang-format pin moves  [control]"),
    ("a433433", "S024  the continuation history is reverted"),
    ("HEAD", "the tree as it ships today"),
]


def read_set(path=TSV):
    """The constructed set, in file order, which is the order the masks use."""
    rows = []

    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or not line.strip():
                continue
            field = line.rstrip("\n").split("\t")
            if field[0] == "fen":
                continue
            rows.append({"fen": field[0], "distance": int(field[1]),
                         "family": field[2]})

    return rows


class engine_t(object):
    """One engine process, spoken to over UCI and nothing more."""

    def __init__(self, path, options):
        self.process = subprocess.Popen([path], stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE, text=True,
                                        bufsize=1)
        self.send("uci")
        while "uciok" not in self.process.stdout.readline():
            pass

        for name, value in options.items():
            self.send("setoption name %s value %s" % (name, value))

        # The engine answers a refused setoption since S137, and this is the
        # only thing standing between a sweep and the failure S156 recorded:
        # `RfpMinPly` 1 is outside the declared range, is refused, and leaves
        # the default in place, so the row reads as a perfect null instead of
        # as a weakened guard. Reading the answer turns that into a stop.
        self.send("isready")
        refused = []

        while True:
            line = self.process.stdout.readline()

            if "refused" in line:
                refused.append(line.strip())
            if line.startswith("readyok"):
                break

        if refused:
            self.close()
            raise SystemExit(
                "the engine refused a setting this sweep depends on:\n  %s\n"
                "It stayed at its default, so the row would have measured the "
                "shipping engine and called it something else. Relax the bound "
                "in a worktree -- the `floor` mode does -- and measure there."
                % "\n  ".join(refused))

    def send(self, line):
        self.process.stdin.write(line + "\n")
        self.process.stdin.flush()

    def iterations(self, fen, depth):
        """Every finished iteration's score, as (depth, is_mate, value).

        The same three fields tests/test_engine.cpp's `deepen_scores` keeps,
        parsed from the same line, so `first_exact` here means what it means
        there. The final node count comes back beside them and is not
        decoration: a row that reads no movement has to show that the probe
        moved the tree at all, and the node total is that proof.
        """
        self.send("ucinewgame")
        self.send("position fen " + fen)
        self.send("go depth %d" % depth)

        out = []
        nodes = 0

        while True:
            line = self.process.stdout.readline()
            if not line or line.startswith("bestmove"):
                break
            if not line.startswith("info ") or " score " not in line:
                continue

            token = line.split()
            at = token.index("score")
            nodes = int(token[token.index("nodes") + 1])
            out.append((int(token[token.index("depth") + 1]),
                        token[at + 1] == "mate", int(token[at + 2])))

        return out, nodes

    def close(self):
        self.send("quit")
        self.process.wait(timeout=10)


def measure(path, options, rows, slack=None):
    """One pass over the whole set, bucketed by mate distance.

    `exact` is the gate's `exact_by_distance`: the last iteration reports the
    proved distance. `on_time` is the extra clause the mate-in-two assertion
    carries, `first_exact == 2m - 1`. `short` and `sign` are defects at any
    count and are counted per position, not per iteration, so the column reads
    as "how many positions were wrong" rather than "how often".
    """
    per = {}
    total_nodes = 0
    engine = engine_t(path, dict({"Hash": 16}, **options))

    try:
        for row in rows:
            distance = row["distance"]
            minimum = 2 * distance - 1
            bucket = per.setdefault(distance,
                                    {"n": 0, "exact": 0, "on_time": 0,
                                     "delays": [], "short": 0, "sign": 0,
                                     "mask": ""})
            bucket["n"] += 1

            first = None
            last = None
            short = sign = False

            iterations, nodes = engine.iterations(
                row["fen"], minimum + (EXTRA_DEPTH if slack is None else slack))
            total_nodes += nodes

            for depth, is_mate, value in iterations:
                last = (is_mate, value)

                if not is_mate:
                    continue
                if value < 0:
                    sign = True
                elif value < distance:
                    short = True
                elif value == distance and first is None:
                    first = depth

            exact = last is not None and last[0] and last[1] == distance

            bucket["exact"] += 1 if exact else 0
            bucket["mask"] += "#" if exact else "."
            bucket["short"] += 1 if short else 0
            bucket["sign"] += 1 if sign else 0

            if first is not None:
                bucket["delays"].append(first - minimum)
                if first == minimum:
                    bucket["on_time"] += 1
    finally:
        engine.close()

    per["nodes"] = total_nodes

    return per


def churn(before, after):
    """Positions that changed verdict between two masks, in either direction."""
    if before is None or len(before) != len(after):
        return None

    return sum(1 for a, b in zip(before, after) if a != b)


def report(label, per, previous, note=""):
    """One row, and the mate-in-three mask is printed because it is the subject.

    The other distances get their counts. Mate in three is the one the floor
    fences, so its identity is on the line and a reader can see which position
    moved rather than only that the total did.
    """
    parts = []

    for distance in sorted(d for d in per if isinstance(d, int)):
        bucket = per[distance]
        delays = bucket["delays"]
        cell = "m%d %2d/%2d" % (distance, bucket["exact"], bucket["n"])

        if distance == 2:
            cell += " on-time %2d" % bucket["on_time"]

        cell += " d%s" % (max(delays) if delays else "-")
        parts.append(cell)

    mask = per[3]["mask"] if 3 in per else ""
    moved = churn(previous, mask)

    buckets = [per[d] for d in per if isinstance(d, int)]

    print("  %-22s %s  short %d  sign %d"
          % (label, "  ".join(parts),
             sum(b["short"] for b in buckets),
             sum(b["sign"] for b in buckets)))
    print("  %-22s m3 %s  churn %s  %d nodes%s"
          % ("", mask, "-" if moved is None else str(moved), per["nodes"],
             ("   " + note) if note else ""))

    return mask


def worktree(ref, patches, targets, verify=(), overlay=(), submodules=(),
             build=True, tune=False, jobs=8):
    """A detached worktree at `ref`, patched, built, and its binary's path.

    A patch marked optional may already be in place -- the `RfpMinPly` bound
    was 0 before S142 narrowed it, so a ref older than that needs no rewrite --
    and `verify` is what makes that safe: the lines the tree must contain once
    the patching is done, checked whether they were written here or were
    already there. Without it an optional patch would silently measure the
    wrong engine at exactly the refs where the rewrite matters.

    The caller owns the cleanup: the return value is (tree, engine path) and
    `git worktree remove --force` is what ends it.
    """
    tree = tempfile.mkdtemp(prefix="S154_sweep_")
    shutil.rmtree(tree)
    run(["git", "worktree", "add", "--detach", tree, ref], REPO)

    for relative in overlay:
        shutil.copyfile(os.path.join(REPO, relative),
                        os.path.join(tree, relative))

    # tests/doctest and tests/json are submodules and a fresh worktree gets
    # them empty. They are copied out of this checkout rather than initialised:
    # `git submodule update --init` in a linked worktree clones from the
    # recorded URL, which is git@github.com: here, so it needs an ssh key and
    # fails without one. Both are header-only trees the build only reads.
    for module in submodules:
        source = os.path.join(REPO, module)

        if not os.path.exists(os.path.join(source, ".git")):
            run(["git", "worktree", "remove", "--force", tree], REPO)
            raise SystemExit(
                "%s is not checked out here, so there is nothing to copy into "
                "the worktree. Run `git submodule update --init` first."
                % source)

        target = os.path.join(tree, module)
        shutil.rmtree(target, ignore_errors=True)
        shutil.copytree(source, target)

    for relative, before, after, required in patches:
        path = os.path.join(tree, relative)
        with open(path) as handle:
            text = handle.read()

        if before not in text:
            if required:
                run(["git", "worktree", "remove", "--force", tree], REPO)
                raise SystemExit(
                    "%s at %s does not contain the line this script rewrites:\n"
                    "  %s\nRe-read the file and update this script before "
                    "trusting any number below." % (relative, ref, before))
            continue

        with open(path, "w") as handle:
            handle.write(text.replace(before, after, 1))

    for relative, expected in verify:
        with open(os.path.join(tree, relative)) as handle:
            if expected in handle.read():
                continue

        run(["git", "worktree", "remove", "--force", tree], REPO)
        raise SystemExit(
            "%s at %s does not contain the line every reading below depends "
            "on:\n  %s\nThe patching left the wrong engine behind."
            % (relative, ref, expected))

    if not build:
        return tree, None

    directory = "build-tune" if tune else "build"
    configure = ["cmake", "-S", ".", "-B", directory,
                 "-DCMAKE_BUILD_TYPE=Release"]

    if tune:
        configure.append("-DCHESSO_TUNE=ON")

    run(configure, tree)
    run(["cmake", "--build", directory, "-j%d" % jobs] + targets, tree)

    return tree, os.path.join(tree, directory, "src", "chesso")


def run(command, cwd):
    result = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True)
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        raise SystemExit("failed in %s: %s" % (cwd, " ".join(command)))
    return result.stdout


def head(rows, what):
    print("%s\n%d positions, depth 2m-1+%d, per distance %s"
          % (what, len(rows), EXTRA_DEPTH,
             ", ".join("m%d %d" % (d, sum(1 for r in rows if r["distance"] == d))
                       for d in sorted({r["distance"] for r in rows}))))
    print()


def sweep_here(args, rows):
    head(rows, "The set through %s, at its shipping defaults."
         % os.path.relpath(args.engine, REPO))
    report("shipping", measure(args.engine, {}, rows, args.slack), None)
    return 0


def sweep_slack(args, rows):
    """What the gate's depth window costs and what it buys.

    `MATE_DEPTH_SLACK` decides how many iterations above 2m - 1 the gate
    searches, and a position whose mate arrives after the last one reads as
    lost. The window is the other half of the fence and it is sized the same
    way the floor is: by the largest delay measured under it, with room left
    over. Both the shipping guard and the removed one are swept, because the
    delay that matters is the worst either produces.
    """
    head(rows, "MATE_DEPTH_SLACK swept, at the shipping guard and at the "
               "weakened one.\nThe wall column is the whole 48-position pass, "
               "which is what the gate pays.")

    tree, engine = worktree(
        args.ref,
        [(PARAMS, SHIPPING_BOUND, RELAXED_BOUND, False),
         (CMAKE, WERROR, NO_WERROR, True)],
        ["--target", "chesso"],
        verify=[(PARAMS, RELAXED_BOUND)],
        tune=True,
        jobs=args.jobs)

    try:
        for label, options in (("shipping", {}),
                               ("RfpMinPly=1", {"RfpMinPly": 1})):
            for slack in args.slacks:
                started = time.time()
                per = measure(engine, options, rows, slack)
                report("%s slack=%d" % (label, slack), per, None,
                       "%.1fs" % (time.time() - started))
    finally:
        run(["git", "worktree", "remove", "--force", tree], REPO)

    return 0


def sweep_hash(args, rows):
    head(rows, "Hash swept and nothing else. No rule changes and no parameter "
               "moves;\nonly which nodes get a table hit. This is the noise "
               "floor under every other\nreading in this file.")

    previous = None
    counts = []

    for value in HASH_VALUES:
        per = measure(args.engine, {"Hash": value}, rows)
        previous = report("Hash=%d" % value, per, previous)
        counts.append(per[3]["exact"])

    print("\n  mate in three over %d table sizes: min %d, max %d, spread %d"
          % (len(counts), min(counts), max(counts), max(counts) - min(counts)))

    return 0


def sweep_refs(args, rows):
    head(rows, "The same set through the binary built at each commit that "
               "touched src/\nsince the floor was placed. -Werror is dropped "
               "in the worktree: pre-S167\ncommits do not compile under Apple "
               "clang, and dropping it changes no codegen.")

    previous = None
    counts = []

    for ref, note in REFS:
        tree, engine = worktree(
            ref,
            [(CMAKE, WERROR, NO_WERROR, True)],
            ["--target", "chesso"],
            jobs=args.jobs)

        try:
            sha = run(["git", "rev-parse", "--short", "HEAD"], tree).strip()
            per = measure(engine, {}, rows)
            previous = report(sha, per, previous, note)
            counts.append(per[3]["exact"])
        finally:
            run(["git", "worktree", "remove", "--force", tree], REPO)

    print("\n  mate in three over %d commits: min %d, max %d, spread %d"
          % (len(counts), min(counts), max(counts), max(counts) - min(counts)))

    return 0


def sweep_floor(args, rows):
    head(rows, "RfpMinPly swept at %s, RfpMaxDepth held at its shipping "
               "value.\nThe declared minimum is relaxed to 0 in a throwaway "
               "worktree: S142 made it 2,\nso a shipping binary refuses 1 and "
               "0 and answers with its default instead." % args.ref)

    tree, engine = worktree(
        args.ref,
        [(PARAMS, SHIPPING_BOUND, RELAXED_BOUND, False),
         (CMAKE, WERROR, NO_WERROR, True)],
        ["--target", "chesso"],
        verify=[(PARAMS, RELAXED_BOUND)],
        tune=True,
        jobs=args.jobs)

    try:
        previous = None

        for value in [5, 4, 3, 2, 1, 0]:
            previous = report("RfpMinPly=%d" % value,
                              measure(engine, {"RfpMinPly": value}, rows),
                              previous)
    finally:
        run(["git", "worktree", "remove", "--force", tree], REPO)

    return 0


def sweep_red(args, rows):
    """The gate rebuilt with the guard weakened, and it has to fail.

    A floor that cannot be made to go red is a floor that proves nothing, and
    the reason S154 exists is that 7 had quietly become such a floor. So the
    new one is observed failing rather than argued to be sound. Two things are
    weakened at once and only one of them is the subject: the default is moved
    to 1 so the compiled-in gate sees it -- a setoption cannot reach a test
    binary -- and the declared minimum has to come down with it or the default
    is out of its own range.

    The test file comes from the working tree, not from the ref, so the floor
    being observed is the one about to be committed and not the one that
    shipped. Everything else is HEAD.
    """
    del rows

    tree, engine = worktree(
        args.ref,
        [(PARAMS, SHIPPING_BOUND, WEAKENED_BOUND, True),
         (CMAKE, WERROR, NO_WERROR, True),
         ("tests/test_engine.cpp", M2_REQUIRE, M2_CHECK, True),
         ("tests/test_engine.cpp", M2_TOTAL_REQUIRE, M2_TOTAL_CHECK, True)],
        [],
        verify=[(PARAMS, WEAKENED_BOUND)],
        overlay=["tests/test_engine.cpp"],
        submodules=["tests/doctest", "tests/json"],
        build=False,
        jobs=args.jobs)

    try:
        run(["cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"],
            tree)
        run(["cmake", "--build", "build", "-j%d" % args.jobs,
             "--target", "test_engine"], tree)

        gate = subprocess.run([os.path.join(tree, "build", "tests",
                                            "test_engine"),
                               "--test-case=*mate in two*"],
                              stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, text=True)

        print("  the gate built with RfpMinPly defaulting to 1: %s"
              % ("RED, as it must be" if gate.returncode != 0
                 else "GREEN, WHICH IS THE FAILURE"))

        # Only the two counts at the end of the case, not the seven per-position
        # lines the mate-in-two clause logs: those are the other assertion's
        # red and they are recorded in the log's prose, not repeated here.
        for line in gate.stdout.splitlines():
            if "exact_by_distance" in line or "assertions:" in line \
                    or ("values:" in line and "==" not in line) \
                    or ">=" in line:
                print("    " + line.strip())

        return 0 if gate.returncode != 0 else 1
    finally:
        run(["git", "worktree", "remove", "--force", tree], REPO)


MODES = {"here": sweep_here, "hash": sweep_hash, "refs": sweep_refs,
         "floor": sweep_floor, "slack": sweep_slack, "red": sweep_red}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=sorted(MODES))
    parser.add_argument("--engine", default=SHIPPING_ENGINE,
                        help="binary for the modes that do not build one")
    parser.add_argument("--ref", default="HEAD",
                        help="commit the floor mode measures; the worktree is "
                             "checked out detached at it")
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--slack", type=int, default=None,
                        help="override MATE_DEPTH_SLACK for the `here` mode")
    parser.add_argument("--slacks", default="8,10,12,14",
                        type=lambda v: [int(x) for x in v.split(",")],
                        help="the window sizes the `slack` mode sweeps")
    args = parser.parse_args()

    started = time.time()
    status = MODES[args.mode](args, read_set())
    print("\n  %s, %.1fs" % (args.mode, time.time() - started))

    return status


if __name__ == "__main__":
    sys.exit(main())
