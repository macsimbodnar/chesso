#!/usr/bin/env python3
"""Measure what chesso's fast suite detects, by breaking the engine on purpose.

S196. Mutation testing, as the literature defines it: change the program in one
small way -- "each changed version is called a mutant" -- and a suite that fails
on it has detected it, which "is called killing the mutant"; the mutation score
is killed over total (https://en.wikipedia.org/wiki/Mutation_testing). Just et
al., FSE 2014, is why the score is worth having: it correlates with real-fault
detection independently of coverage (`adocs/testing_strategy.md` section 3.1).

TWO DIFFERENCES FROM THE PUBLISHED TOOLS, both decided here.

Mutants are hand-written, one per plausible engine bug -- a guard dropped, a
sign flipped, an off-by-one -- not operator-generated. Mull and dextool were
refused under the DEPS rule (`adocs/testing_strategy.md` section 5).

And the suite gets a second oracle, the bench signature. OpenBench requires a
`bench` that reports a final node count from an engine that produces the same
result every time; Stockfish defines a functional change as one that leads to a
different search tree. So a mutant whose bench signature moves has changed the
tree, and a green suite on it is a proved gap -- it cannot be equivalent. The
converse does not hold: 12 of the 33 mutants of the 2026-09-04 review left the
depth-9 counts and best moves unchanged -- M02-M04, M14, M19, M20, M24-M26, M28,
M32, M33 -- and the suite caught ten of those anyway
(`adocs/audit/2026-09-04_test_review.md`, "The bench signature is blind to a
third of them"); of the other two, M26 is the declared equivalent and M19 was
the survivor S193 was written to kill. A still bench never argues
equivalence. Equivalence is declared in the mutant file and never inferred.

The signature read here is the total this tree's `bench` prints together with
its eight `bestmove` replies. A different best move is a different tree by the
same argument as a different total, and counting it makes `equivalent` harder to
claim, which is the direction that costs nothing.

USAGE

    python3 tools/mutation_check.py tools/mutants .ref-builds/mut
    python3 tools/mutation_check.py tools/mutants .ref-builds/mut --only M26

The worktree is a linked git worktree, never the main tree: the tool reverts
each mutant with `git checkout --`, which in a tree holding your own work would
revert that too. Create one as fastchess.sh does:

    git worktree add --detach .ref-builds/mut HEAD
    git -C .ref-builds/mut submodule update --init tests/doctest tests/json
    cmake -S .ref-builds/mut -B .ref-builds/mut/build \
          -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

The submodule line is not in fastchess.sh's recipe because that script builds
src/chesso and nothing else. This one builds the tests, and tests/doctest and
tests/json are submodules a fresh worktree does not carry: without them cmake
fails on doctest.h and the run refuses at "the unmutated worktree does not
build".

Release only. The root CMakeLists adds -Wall -Wextra -Werror everywhere and
turns the unused-* warnings off in Debug alone, so a mutant that deletes the
last use of a local does not compile in Release -- which is a fact about the
mutant that a Debug build would hide. The two mutants in that class carry a
`(void)` of the symbol they orphan.

A full pass holds the machine for the better part of an hour and is in no gate.
It prints MUTATION-RUN-DONE or MUTATION-RUN-FAILED as its last action on every
exit path, success and failure both (WATCHERS rule), so it can be detached.
"""

import argparse
import os
import re
import subprocess
import sys
import time

BUILD_TIMEOUT_S = 600
CTEST_TIMEOUT_S = 3600
BENCH_TIMEOUT_S = 900

DONE_MARKER = "MUTATION-RUN-DONE"
FAILED_MARKER = "MUTATION-RUN-FAILED"

# Doctest's own reporter strings, both in tests/doctest/doctest/doctest.h: the
# case header is "TEST CASE:  " with two spaces, and a failed assertion ends
# "is NOT correct!". Confirmed against a real log before this regex was fixed.
CASE_RE = re.compile(r"^TEST CASE:\s\s(.*)$")
ASSERT_RE = re.compile(r"ERROR:\s+(.*?)\s+is NOT correct!")
OTHER_ERROR_RE = re.compile(r"ERROR:\s+(.*)$")
PLAIN_FAIL_RE = re.compile(r"^\s*FAIL:\s*(.*)$")

CTEST_SUMMARY_RE = re.compile(r"(\d+)% tests passed, (\d+) tests failed out of (\d+)")
CTEST_FAILED_RE = re.compile(r"^\s*\d+ - (\S+) \((Failed|Timeout|[^)]*)\)\s*$", re.M)
CTEST_START_RE = re.compile(r"^\s*Start\s+\d+:\s+(\S+)\s*$")

BENCH_TOTAL_RE = re.compile(r"^(\d+) nodes (\d+) nps$", re.M)
BENCH_BESTMOVE_RE = re.compile(r"^bestmove (\S+)$", re.M)
# tools/search_bench.py's per-position line, the fallback oracle's shape.
SEARCH_BENCH_RE = re.compile(
    r"^\s+(\w+)\s+[\d.]+s\s+(\d+) nodes\s+\d+ knps\s+best (\S+)", re.M)


class Refused(Exception):
    """A precondition the run will not proceed without."""


# --------------------------------------------------------------------------
# Mutant files


def load_mutants(paths):
    """Load every mutant file, in sorted order, into one validated list.

    A mutant file is only `m(...)` calls and path constants: `m` is pre-bound
    here, so the files carry no driver of their own. A file that defines its own
    `m` and appends to `M` -- the two evidence copies under adocs/data/ do, and
    they are never edited -- is read through that list instead.
    """
    files = []
    for path in paths:
        if os.path.isdir(path):
            files.extend(sorted(
                os.path.join(path, name) for name in os.listdir(path)
                if name.endswith(".py")))
        else:
            files.append(path)
    if not files:
        raise Refused("no mutant file found in " + " ".join(paths))

    mutants = []
    for path in files:
        collected = []

        def m(mid, file, klass, note, *pairs, expected="killed", origin=None):
            collected.append(dict(id=mid, file=file, klass=klass, note=note,
                                  pairs=list(pairs), expected=expected,
                                  origin=origin, source=path))

        namespace = {"m": m, "M": []}
        with open(path) as handle:
            source = handle.read()
        exec(compile(source, path, "exec"), namespace)  # noqa: S102

        entries = collected
        if not entries:
            entries = [normalise(entry, path) for entry in namespace.get("M", [])]
        if not entries:
            raise Refused("no mutant defined in " + path)
        mutants.extend(entries)
    return mutants


def normalise(entry, path):
    """An entry from a file that brought its own `m`, given this tool's fields."""
    out = dict(entry)
    out.setdefault("expected", "killed")
    out.setdefault("origin", None)
    out["source"] = path
    out["pairs"] = list(out["pairs"])
    return out


def validate(mutants, worktree):
    """Refuse the whole run before a single byte is written.

    Every anchor must occur exactly once in the file it names, at the worktree's
    own commit: an ambiguous anchor would mutate a site nobody chose and a
    missing one would measure nothing. Ids are never reused, so a duplicate is a
    mistake in the list rather than a second reading of the same bug.
    """
    problems = []
    seen = {}
    for mut in mutants:
        mid = mut["id"]
        if mid in seen:
            problems.append(f"{mid}: duplicate id, also in {seen[mid]}")
            continue
        seen[mid] = mut["source"]

        if mut["expected"] not in ("killed", "equivalent"):
            problems.append(f"{mid}: expected={mut['expected']!r}, "
                            "not 'killed' or 'equivalent'")

        rel = mut["file"]
        if not rel.startswith("src/") or ".." in rel.split("/"):
            problems.append(f"{mid}: file {rel} is outside src/")
            continue

        path = os.path.join(worktree, rel)
        if not os.path.isfile(path):
            problems.append(f"{mid}: no such file {rel} in the worktree")
            continue
        with open(path) as handle:
            text = handle.read()

        if not mut["pairs"]:
            problems.append(f"{mid}: no (old, new) pair")
        for index, (old, new) in enumerate(mut["pairs"]):
            if old == new:
                problems.append(f"{mid}: pair {index} replaces its anchor "
                                "with itself")
                continue
            count = text.count(old)
            if count != 1:
                problems.append(f"{mid}: pair {index} anchor occurs {count} "
                                f"times in {rel}, needs exactly 1")
    if problems:
        raise Refused("mutant list rejected:\n  " + "\n  ".join(problems))


# --------------------------------------------------------------------------
# The worktree


def git(worktree, *args, check=True):
    proc = subprocess.run(["git", "-C", worktree, *args],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          text=True)
    if check and proc.returncode != 0:
        raise Refused(f"git {' '.join(args)} failed in {worktree}: "
                      f"{proc.stdout.strip()}")
    return proc.stdout.strip()


def require_linked_worktree(worktree):
    """The main tree is refused: `git checkout --` there reverts your own work.

    A linked worktree's git dir is .git/worktrees/<name> while its common dir is
    .git; in the main tree the two are equal.
    """
    if not os.path.isdir(worktree):
        raise Refused(f"no worktree at {worktree}")
    git_dir = git(worktree, "rev-parse", "--absolute-git-dir")
    # `--git-common-dir` answers relative to the worktree, not to this process:
    # resolving it against the cwd made the guard pass only when the tool
    # happened to be run from the repository root.
    common = git(worktree, "rev-parse", "--git-common-dir")
    if not os.path.isabs(common):
        common = os.path.join(worktree, common)
    if os.path.realpath(git_dir) == os.path.realpath(common):
        raise Refused(f"{worktree} is the main tree, not a linked worktree; "
                      "git worktree add --detach .ref-builds/mut HEAD")


def require_clean_src(worktree, when):
    dirty = git(worktree, "status", "--porcelain", "--", "src")
    if dirty:
        raise Refused(f"src/ is dirty in {worktree} {when}:\n{dirty}")


def apply_mutant(worktree, mut):
    for old, new in mut["pairs"]:
        path = os.path.join(worktree, mut["file"])
        with open(path) as handle:
            text = handle.read()
        # Validated as unique already; re-counted because the pairs of one
        # mutant are applied in sequence and an earlier one can move a later
        # anchor.
        if text.count(old) != 1:
            raise Refused(f"{mut['id']}: anchor stopped being unique while "
                          "applying the mutant")
        with open(path, "w") as handle:
            handle.write(text.replace(old, new))


def revert_mutant(worktree, mut):
    git(worktree, "checkout", "--", mut["file"])


# --------------------------------------------------------------------------
# The three measurements


def run(cmd, log_path=None, timeout=None, stdin_text=None):
    """Return (returncode, output). Never a pipeline: DEV_MANUAL's Test section
    records that a pipe hides ctest's status, so the status read here is
    subprocess's own."""
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, text=True,
                              input=stdin_text, timeout=timeout)
        code, output = proc.returncode, proc.stdout
    except subprocess.TimeoutExpired as expired:
        output = expired.output or ""
        if isinstance(output, bytes):
            output = output.decode("utf-8", "replace")
        output += f"\n[mutation_check] timed out after {timeout}s\n"
        code = None
    if log_path:
        with open(log_path, "w") as handle:
            handle.write(output)
    return code, output


def build(build_dir, jobs, log_path):
    code, output = run(["cmake", "--build", build_dir, f"-j{jobs}"],
                       log_path=log_path, timeout=BUILD_TIMEOUT_S)
    first_error = ""
    for line in output.splitlines():
        if "error:" in line:
            first_error = line.strip()
            break
    return code, first_error


def bench(engine, worktree, oracle):
    """The tree's node signature: the bench total and the eight best moves."""
    if oracle == "engine":
        code, output = run([engine], stdin_text="bench\nquit\n",
                           timeout=BENCH_TIMEOUT_S)
        if code is None:
            return None, "timeout"
        total = BENCH_TOTAL_RE.search(output)
        best = BENCH_BESTMOVE_RE.findall(output)
        if not total or not best:
            return None, "no bench line"
        return (total.group(1), tuple(best)), f"{total.group(1)} nodes"

    script = os.path.join(worktree, "tools", "search_bench.py")
    code, output = run(["python3", script, engine, "9"], timeout=BENCH_TIMEOUT_S)
    if code is None:
        return None, "timeout"
    rows = SEARCH_BENCH_RE.findall(output)
    if not rows:
        return None, "no search_bench rows"
    signature = tuple(sorted((name, nodes, best) for name, nodes, best in rows))
    total = sum(int(nodes) for _, nodes, _ in rows)
    return signature, f"{total} nodes"


def ctest(build_dir, label, log_path):
    code, output = run(["ctest", "--test-dir", build_dir, "-L", label,
                        "--output-on-failure"],
                       log_path=log_path, timeout=CTEST_TIMEOUT_S)
    summary = CTEST_SUMMARY_RE.search(output)
    failed = summary.group(2) if summary else "?"
    total = summary.group(3) if summary else "?"
    rows = CTEST_FAILED_RE.findall(output)
    # `asserted` is the detection: a row that failed for any reason other than
    # its ceiling. `inconclusive` is the run saying nothing -- this tool's own
    # ceiling on the whole of ctest (code None), or a non-zero exit with no
    # failing row parsed, which is ctest failing to run rather than a test
    # failing. A `(Timeout)` row alone lands here; beside an `asserted` one it
    # does not, and DEC-165 is why.
    asserted = any(how != "Timeout" for _, how in rows)
    inconclusive = code is None or (code != 0 and not asserted)
    return dict(code=code, failed=failed, total=total,
                tests=[(name, how) for name, how in rows],
                asserted=asserted, inconclusive=inconclusive,
                kills=extract_kills(output, [name for name, _ in rows]))


def extract_kills(output, failing_tests):
    """What actually caught the mutant: binary, case title, first assertion.

    ctest --output-on-failure prints a failing test's own output after its
    `***Failed` line and before the next `Start` line, so the log is split on
    those. A count alone would not do: the 2026-09-04 review found ten mutants
    caught by a single case each, and DEC-142 makes a kill by a golden alone a
    weak one, so every failing binary is listed and the reading is left to a
    person.
    """
    if not failing_tests:
        return []
    wanted = set(failing_tests)
    chunks = {}
    current = None
    for line in output.splitlines():
        start = CTEST_START_RE.match(line)
        if start:
            current = start.group(1)
            chunks.setdefault(current, [])
            continue
        if current is not None:
            chunks[current].append(line)

    kills = []
    for name in failing_tests:
        lines = chunks.get(name, [])
        case = None
        cases = []
        for line in lines:
            case_match = CASE_RE.match(line.strip())
            if case_match:
                case = case_match.group(1).strip()
                cases.append([case, []])
                continue
            assertion = ASSERT_RE.search(line)
            if not assertion:
                other = OTHER_ERROR_RE.search(line)
                assertion = other if other else PLAIN_FAIL_RE.match(line)
            if assertion:
                text = assertion.group(1).strip()
                if not cases:
                    cases.append([None, []])
                cases[-1][1].append(text)
        for title, found in cases:
            if not found:
                continue
            extra = f"  (+{len(found) - 1} more)" if len(found) > 1 else ""
            kills.append((name, title or "-", found[0], extra))
        if not any(found for _, found in cases):
            kills.append((name, "-", "(no assertion text in the log)", ""))
    if wanted and not kills:
        kills = [(name, "-", "(no output captured)", "") for name in failing_tests]
    return kills


# --------------------------------------------------------------------------
# Verdicts


def verdict_of(row):
    """Suite red is a kill whatever the bench did, and whatever else in the run
    was inconclusive: an assertion that fired is a detection even if another
    test hit its ceiling or the engine would not print a bench line. Suite green splits on the
    second oracle: a moved signature is a proved gap, and a still one leaves the
    mutant file's own declaration as the only thing that can call it
    equivalent."""
    if row["built"] is False:
        return "stillborn"
    if row["suite_red"]:
        return "killed"
    if row["unmeasured"]:
        return "unmeasured"
    if row["bench_moved"]:
        return "survived"
    if row["expected"] == "equivalent":
        return "equivalent"
    return "survived"


def bench_column(row):
    """`same` is a claim about the tree and is only made when the signature was
    read. A mutant that leaves the engine unable to print one gets `n/a`, which
    is what the second oracle actually said."""
    if not row["built"]:
        return "-"
    if not row["bench_read"]:
        return "n/a"
    return "moved" if row["bench_moved"] else "same"


def print_table(rows, oracle, out=sys.stdout):
    print(file=out)
    print(f"kill table   second oracle: {oracle} bench signature "
          "(total nodes and the eight best moves)", file=out)
    print(file=out)
    header = f"{'id':<34} {'built':<6} {'fast':<9} {'bench':<7} " \
             f"{'verdict':<11} {'expected':<10} {'s':>6}"
    print(header, file=out)
    print("-" * len(header), file=out)
    for row in rows:
        built = {True: "yes", False: "no", None: "n/a"}[row["built"]]
        fast = f"{row['failed']}/{row['total']}" if row["built"] else "-"
        moved = bench_column(row)
        print(f"{row['id']:<34} {built:<6} {fast:<9} {moved:<7} "
              f"{row['verdict']:<11} {row['expected']:<10} "
              f"{row['seconds']:>6.1f}", file=out)

    print(file=out)
    print("what caught each mutant", file=out)
    for row in rows:
        if not row["kills"]:
            continue
        print(f"\n== {row['id']}: {row['note']}", file=out)
        print(f"   ctest: {row['failed']} of {row['total']} failed", file=out)
        for binary, title, assertion, extra in row["kills"]:
            print(f"   [{binary}] {title}  |  {assertion}{extra}", file=out)


def write_tsv(path, rows, baseline_signature, oracle):
    with open(path, "w") as handle:
        handle.write("id\tclass\torigin\tbuilt\tfast_failed\tfast_total\t"
                     "bench\tverdict\texpected\tseconds\tfailing_tests\tnote\n")
        handle.write(f"BASELINE\t-\t-\tyes\t0\t-\t{baseline_signature}\t-\t-\t"
                     f"-\t-\tunmutated worktree, {oracle} oracle\n")
        for row in rows:
            built = {True: "yes", False: "no", None: "n/a"}[row["built"]]
            tests = " ".join(f"{name}({how})" for name, how in row["tests"])
            moved = bench_column(row)
            handle.write("\t".join([
                row["id"], row["klass"], row["origin"] or "-", built,
                str(row["failed"]), str(row["total"]), moved, row["verdict"],
                row["expected"], f"{row['seconds']:.1f}", tests, row["note"],
            ]) + "\n")


# --------------------------------------------------------------------------


def selected(mutants, only):
    """`--only M26` selects the mutant whose id begins with that field: ids read
    `M26_repetition_skips_first`, and the short form is what a step stamp
    quotes."""
    if not only:
        return list(mutants)
    wanted = set(only)
    chosen = [mut for mut in mutants
              if mut["id"] in wanted or mut["id"].split("_")[0] in wanted]
    found = {mut["id"] for mut in chosen} | {mut["id"].split("_")[0]
                                             for mut in chosen}
    missing = sorted(wanted - found)
    if missing:
        raise Refused("no such mutant: " + ", ".join(missing))
    return chosen


def detect_oracle(engine):
    """`help` prints the dispatch table's own names (src/chesso.cpp
    `uci_command_names`). An engine without `bench` predates S189 and is
    measured through tools/search_bench.py instead."""
    code, output = run([engine], stdin_text="help\nquit\n", timeout=120)
    if code is None:
        raise Refused("the engine did not answer `help`")
    for line in output.splitlines():
        if line.strip() == "bench":
            return "engine"
    return "search_bench"


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="apply hand-written engine bugs one at a time and record "
                    "what the fast suite catches")
    parser.add_argument("mutants",
                        help="a mutant .py file, or a directory of them")
    parser.add_argument("worktree", help="a linked git worktree, not the main tree")
    parser.add_argument("--only", action="append", nargs="+", default=None,
                        metavar="ID", help="run these mutants only")
    parser.add_argument("--build-dir", default=None)
    parser.add_argument("--label", default="fast")
    parser.add_argument("--jobs", type=int, default=os.cpu_count())
    parser.add_argument("--log-dir", default=None)
    args = parser.parse_args(argv)

    worktree = os.path.abspath(args.worktree)
    build_dir = os.path.abspath(args.build_dir or os.path.join(worktree, "build"))
    log_dir = os.path.abspath(args.log_dir or
                              os.path.join(worktree, "build", "mutation"))
    only = [item for group in (args.only or []) for item in group]

    require_linked_worktree(worktree)
    require_clean_src(worktree, "before the run")
    mutants = load_mutants([args.mutants])
    validate(mutants, worktree)
    chosen = selected(mutants, only)

    os.makedirs(log_dir, exist_ok=True)
    engine = os.path.join(build_dir, "src", "chesso")

    started = time.time()
    print(f"worktree {worktree} at {git(worktree, 'rev-parse', '--short', 'HEAD')}")
    print(f"build    {build_dir}   jobs {args.jobs}   label {args.label}")
    print(f"logs     {log_dir}")
    # Flushed: stdout to a file is block-buffered, and a detached run whose log
    # is empty for the two minutes the baseline takes reads as one that failed
    # to start.
    print(f"mutants  {len(chosen)} of {len(mutants)}", flush=True)

    code, first_error = build(build_dir, args.jobs,
                              os.path.join(log_dir, "baseline_build.log"))
    if code != 0:
        raise Refused(f"the unmutated worktree does not build: {first_error}")
    oracle = detect_oracle(engine)
    baseline_signature, baseline_display = bench(engine, worktree, oracle)
    if baseline_signature is None:
        raise Refused(f"the unmutated worktree has no bench signature: "
                      f"{baseline_display}")
    baseline = ctest(build_dir, args.label,
                     os.path.join(log_dir, "baseline_ctest.log"))
    if baseline["code"] != 0:
        # Every later row would be uninterpretable: a test already red cannot
        # say whether it saw the mutant.
        raise Refused("the unmutated worktree is red: "
                      f"{baseline['failed']} of {baseline['total']} failed "
                      f"({', '.join(name for name, _ in baseline['tests'])})")
    print(f"baseline green, {baseline['total']} tests, "
          f"bench {baseline_display} via {oracle}", flush=True)

    rows = []
    for mut in chosen:
        mutant_started = time.time()
        apply_mutant(worktree, mut)
        try:
            code, first_error = build(
                build_dir, args.jobs, os.path.join(log_dir, mut["id"] + "_build.log"))
            row = dict(id=mut["id"], klass=mut["klass"], note=mut["note"],
                       origin=mut["origin"], expected=mut["expected"],
                       built=None, failed="-", total="-", tests=[], kills=[],
                       bench_moved=False, bench_read=False, suite_red=False,
                       unmeasured=False, seconds=0.0)
            if code is None:
                row["built"] = None
                row["unmeasured"] = True
                row["note"] += "  [build timed out]"
            elif code != 0:
                row["built"] = False
                row["note"] += f"  [does not compile: {first_error}]"
            else:
                row["built"] = True
                signature, display = bench(engine, worktree, oracle)
                if signature is None:
                    row["unmeasured"] = True
                    row["note"] += f"  [no bench signature: {display}]"
                else:
                    row["bench_read"] = True
                    row["bench_moved"] = signature != baseline_signature
                result = ctest(build_dir, args.label,
                               os.path.join(log_dir, mut["id"] + "_ctest.log"))
                row["failed"] = result["failed"]
                row["total"] = result["total"]
                row["tests"] = result["tests"]
                row["kills"] = result["kills"]
                row["suite_red"] = result["asserted"]
                if result["inconclusive"]:
                    # A ceiling hit on a busy machine is not a kill: re-run the
                    # row with --only when the machine is quiet. Only when the
                    # ceiling is the whole evidence, though. M22 and M31 hang
                    # test_uci_surface at 60 s -- 9.26 s unmutated -- while two
                    # and four other binaries fail on assertions in the same
                    # run, and calling that unmeasured hides a kill no re-run
                    # can recover: the hang is the mutant's, so a quiet machine
                    # reproduces it exactly. DEC-165.
                    row["unmeasured"] = True
        finally:
            revert_mutant(worktree, mut)
            require_clean_src(worktree, f"after reverting {mut['id']}")
        row["seconds"] = time.time() - mutant_started
        row["verdict"] = verdict_of(row)
        rows.append(row)
        print(f"  {row['id']:<34} {row['verdict']:<11} "
              f"fast {row['failed']}/{row['total']}  "
              f"bench {'moved' if row['bench_moved'] else 'same'}  "
              f"{row['seconds']:.1f}s", flush=True)

    # The build directory still holds the last mutant's object files; leave the
    # worktree's binary matching its own source.
    build(build_dir, args.jobs, os.path.join(log_dir, "final_build.log"))

    print_table(rows, oracle)
    write_tsv(os.path.join(log_dir, "results.tsv"), rows, baseline_display, oracle)

    counts = {}
    for row in rows:
        counts[row["verdict"]] = counts.get(row["verdict"], 0) + 1
    scored = (len(rows) - counts.get("equivalent", 0) - counts.get("stillborn", 0)
              - counts.get("unmeasured", 0))
    killed = counts.get("killed", 0)
    print(f"\nmutation score {killed} of {scored}"
          + (f" ({100.0 * killed / scored:.0f}%)" if scored else "")
          + "   " + ", ".join(f"{k} {v}" for k, v in sorted(counts.items())))
    print(f"wall {time.time() - started:.0f}s   results "
          f"{os.path.join(log_dir, 'results.tsv')}")

    off = [row for row in rows if row["verdict"] != row["expected"]]
    if off:
        print("\nnot what the list expects:")
        for row in off:
            print(f"  {row['id']}: {row['verdict']}, expected {row['expected']}"
                  f" -- {row['note']}")
    return 1 if off else 0


if __name__ == "__main__":
    try:
        status = main()
    except Refused as refused:
        print(f"refused: {refused}", file=sys.stderr)
        print(FAILED_MARKER, flush=True)
        sys.exit(1)
    except BaseException as error:  # marker on every exit path, WATCHERS rule
        import traceback
        traceback.print_exc()
        print(f"{FAILED_MARKER}: {type(error).__name__}: {error}", flush=True)
        sys.exit(1)
    print(DONE_MARKER if status == 0 else f"{FAILED_MARKER}: a verdict differs "
          "from what the mutant list expects", flush=True)
    sys.exit(status)
