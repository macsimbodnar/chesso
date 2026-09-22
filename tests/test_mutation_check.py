#!/usr/bin/env python3
"""The gate on tools/mutation_check.py. S196.

WHY A SANDBOX AND NOT THE ENGINE. A full pass of the real tool is forty builds
and forty runs of the fast suite, better than an hour of machine, and it is in
no gate. What can break in the driver is not the engine's behaviour: an anchor
counted wrong, a revert that leaves a mutant behind, a verdict assigned to the
wrong branch, a marker missing from an exit path. Every one of those resolves in
a throwaway git repository holding a four-line `src/x.cpp`, with `cmake`, `ctest`
and the engine as stubs first on PATH -- the technique tests/test_fastchess_script.sh
uses, for the same reason: no real build, no real games, and the real .ref-builds/
and build/ are never read or written.

WHY IT IS REGISTERED IN THE FAST SUITE. DEC-118's argument, applied to a tool
rather than a build: a thing nobody runs for weeks is found broken by whoever
next needs it, and the moment somebody next needs this one is the moment they
are adding a pruning rule and want to know their new test bites (DEC-141
clause 2). It costs a second.

THE STUBS' CONTRACT. `ctest` knows one label, `fast`, and answers any other the
way the real one answers a label no test carries -- "No tests were found!!!",
no summary line, exit 0; `empty_summary` is that emptiness in its other shape,
a summary line counting to zero. Otherwise it reads the worktree's src/x.cpp
and goes red when it holds the marker MUTANT_SEEN, times out on TIMEOUT_ONLY,
does both at once on TIMEOUT_PLUS_FAIL, and refuses to run at all on
CTEST_BROKEN; the engine's bench total moves when it holds BENCH_MOVES and it
prints no bench line at all on BENCH_BROKEN; `cmake` fails with an `error:` on
WILL_NOT_COMPILE, which is the -Werror class the two `(void)` mutants exist
for. So the four squares of the verdict table -- suite red, suite green
with a moved signature, suite green with a still signature declared equivalent,
suite green with a still signature not declared -- are each reachable by one
mutant, and that is what the cases below drive.
"""

import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOL = os.path.join(ROOT, "tools", "mutation_check.py")

# The sandbox source. `int twice` occurs in two lines, which is the ambiguous
# anchor the tool has to refuse; the two guards are the sites the mutants edit.
SOURCE = """// sandbox source for tests/test_mutation_check.py
int guard_one = 1;
int guard_two = 2;
int twice = 0;
int twice_again = 0;
"""

# The rest of the fixture the tool now holds to its own sha -- the suite that
# judges a mutant and the documents a run's evidence is written against, both
# of which S097 verdict 2 edited in place while the header said nothing -- and
# the .gitignore that makes the real fixture clean whole, since the tool checks
# the whole tree and the build directory it writes into lives inside it.
FIXTURE_FILES = {
    "tests/test_x.cpp": "// the suite the sandbox pretends to run\n",
    "adocs/specs.md": "the document a run's evidence is written against\n",
    "CMakeLists.txt": "# outside the paths an allowlist would have named\n",
    ".gitignore": "build/\n",
}

CTEST_RED = r"""#!/bin/sh
# One label exists here. Any other gets what the real ctest gives a label no
# test carries: a project line, "No tests were found!!!", no summary line at
# all, and a zero exit -- which is the green an empty suite fakes.
label=
prev=
for arg in "$@"; do
  if [ "$prev" = "-L" ]; then label="$arg"; fi
  prev="$arg"
done
if [ "$label" = "empty_summary" ]; then
  # The other shape of an empty run: a summary line that counts to zero, which
  # a parser reading only the failure count reads as a clean green.
  echo "Test project /sandbox"
  echo "100% tests passed, 0 tests failed out of 0"
  exit 0
fi
if [ "$label" != "fast" ]; then
  echo "Test project /sandbox"
  echo "No tests were found!!!"
  exit 0
fi
if grep -q TIMEOUT_PLUS_FAIL "$MUT_SRC"; then
  cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/2 Test #1: test_x ...........................***Failed    0.01 sec
===============================================================================
/sandbox/tests/test_x.cpp:10:
TEST CASE:  a fabricated case the stub reports

/sandbox/tests/test_x.cpp:12: ERROR: CHECK( guard_one_holds() ) is NOT correct!
  values: CHECK( false )

    Start 2: test_y
2/2 Test #2: test_y ...........................***Timeout  60.01 sec

0% tests passed, 2 tests failed out of 2

The following tests FAILED:
	  1 - test_x (Failed)
	  2 - test_y (Timeout)
FAIL: a line ctest prints after the last test block, belonging to no binary
Errors while running CTest
EOF
  exit 8
fi
if grep -q TIMEOUT_ONLY "$MUT_SRC"; then
  cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/1 Test #1: test_x ...........................***Timeout  60.01 sec

0% tests passed, 1 tests failed out of 1

The following tests FAILED:
	  1 - test_x (Timeout)
Errors while running CTest
EOF
  exit 8
fi
if grep -q CTEST_BROKEN "$MUT_SRC"; then
  echo "No tests were found!!!"
  exit 1
fi
if grep -q MUTANT_SEEN "$MUT_SRC"; then
  cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/1 Test #1: test_x ...........................***Failed    0.01 sec
===============================================================================
/sandbox/tests/test_x.cpp:10:
TEST CASE:  a fabricated case the stub reports

/sandbox/tests/test_x.cpp:12: ERROR: CHECK( guard_one_holds() ) is NOT correct!
  values: CHECK( false )
100% tests passed, 0 tests failed out of 0

0% tests passed, 1 tests failed out of 1

The following tests FAILED:
	  1 - test_x (Failed)
Errors while running CTest
EOF
  exit 8
fi
cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/1 Test #1: test_x ...........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1
EOF
exit 0
"""

ENGINE = r"""#!/bin/sh
while read -r line; do
  case "$line" in
    help) echo bench; echo go; echo quit ;;
    bench)
      if grep -q BENCH_BROKEN "$MUT_SRC"; then echo "info string no bench here"; continue; fi
      if grep -q BENCH_MOVES "$MUT_SRC"; then total=222222; else total=111111; fi
      i=0
      while [ "$i" -lt 8 ]; do echo "bestmove e2e4"; i=$((i + 1)); done
      echo "$total nodes 1000 nps"
      ;;
    quit) exit 0 ;;
  esac
done
"""

CMAKE = r"""#!/bin/sh
if grep -q SLOW_BUILD "$MUT_SRC"; then
  sleep 30
  exit 0
fi
if grep -q WILL_NOT_COMPILE "$MUT_SRC"; then
  echo "/sandbox/src/x.cpp:2:5: error: 'guard_one' declared void"
  exit 2
fi
echo 'stub cmake: nothing to build'
exit 0
"""


def write_exec(path, text):
    with open(path, "w") as handle:
        handle.write(text)
    os.chmod(path, 0o755)


class Sandbox:
    """A git repository, a linked worktree of it, and the three stubs."""

    def __init__(self):
        self.root = tempfile.mkdtemp(prefix="chesso-mutation-check.")
        self.repo = os.path.join(self.root, "repo")
        self.worktree = os.path.join(self.root, "wt")
        self.stubs = os.path.join(self.root, "stub")
        os.makedirs(os.path.join(self.repo, "src"))
        os.makedirs(self.stubs)

        with open(os.path.join(self.repo, "src", "x.cpp"), "w") as handle:
            handle.write(SOURCE)
        for rel, text in FIXTURE_FILES.items():
            path = os.path.join(self.repo, rel)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w") as handle:
                handle.write(text)
        self.git(self.repo, "init", "--quiet")
        self.git(self.repo, "add", "src/x.cpp", *FIXTURE_FILES)
        self.git(self.repo, "-c", "user.email=t@example.invalid",
                 "-c", "user.name=test", "commit", "--quiet", "-m", "sandbox")
        self.git(self.repo, "worktree", "add", "--detach", "--quiet",
                 self.worktree, "HEAD")

        engine_dir = os.path.join(self.worktree, "build", "src")
        os.makedirs(engine_dir)
        write_exec(os.path.join(engine_dir, "chesso"), ENGINE)
        write_exec(os.path.join(self.stubs, "cmake"), CMAKE)
        write_exec(os.path.join(self.stubs, "ctest"), CTEST_RED)

        self.src = os.path.join(self.worktree, "src", "x.cpp")

    @staticmethod
    def git(cwd, *args):
        subprocess.run(["git", "-C", cwd, *args], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def mutant_file(self, body):
        path = os.path.join(self.root, "mutants.py")
        with open(path, "w") as handle:
            handle.write(body)
        return path

    def spawn(self, mutants, target=None, extra=()):
        """Popen rather than run: a case that signals the tool needs it alive."""
        env = dict(os.environ)
        env["PATH"] = self.stubs + os.pathsep + env["PATH"]
        env["MUT_SRC"] = self.src
        return subprocess.Popen(
            [sys.executable, TOOL, mutants, target or self.worktree, *extra],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, env=env)

    def run(self, mutants, target=None, extra=()):
        env = dict(os.environ)
        env["PATH"] = self.stubs + os.pathsep + env["PATH"]
        env["MUT_SRC"] = self.src
        proc = subprocess.run(
            [sys.executable, TOOL, mutants, target or self.worktree, *extra],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
            timeout=120, env=env)
        return proc

    def worktree_dirty(self):
        """Over the whole fixture, not src/ alone: what the tool now guards."""
        out = subprocess.run(["git", "-C", self.worktree, "status",
                              "--porcelain", "--", "src", "tests", "adocs",
                              "tools/mutants"],
                             stdout=subprocess.PIPE, text=True, check=True)
        return out.stdout.strip()

    def dirty_the_fixture(self, *rels):
        """Edit committed files in place, leaving src/ clean -- S097 verdict
        2's second run, which the header called by a sha whose tests differed."""
        for rel in rels:
            with open(os.path.join(self.worktree, rel), "a") as handle:
                handle.write("an edit the fixture's sha does not carry\n")

    def close(self):
        shutil.rmtree(self.root, ignore_errors=True)


KILLED = """
m("K01_killed", "src/x.cpp", "sandbox", "the suite sees this one",
  ("int guard_one = 1;", "int guard_one = 1;  // MUTANT_SEEN"))
"""

EQUIVALENT = """
m("E01_equivalent", "src/x.cpp", "sandbox", "declared equivalent by a person",
  ("int guard_two = 2;", "int guard_two = 2;  // quiet"),
  expected="equivalent")
"""

BENCH_SURVIVOR = """
m("S01_bench_moved", "src/x.cpp", "sandbox", "the tree moves and no test sees it",
  ("int guard_two = 2;", "int guard_two = 2;  // BENCH_MOVES"))
"""

BLIND_SURVIVOR = """
m("S02_bench_blind", "src/x.cpp", "sandbox", "still signature, nothing red",
  ("int guard_two = 2;", "int guard_two = 2;  // quiet"))
"""

TIMEOUT_ONLY = """
m("T01_timeout_only", "src/x.cpp", "sandbox", "a ceiling hit and nothing else",
  ("int guard_two = 2;", "int guard_two = 2;  // TIMEOUT_ONLY"))
"""

TIMEOUT_PLUS_FAIL = """
m("T02_timeout_plus_fail", "src/x.cpp", "sandbox", "a ceiling hit beside a real red",
  ("int guard_two = 2;", "int guard_two = 2;  // TIMEOUT_PLUS_FAIL"))
"""

BENCH_BROKEN = """
m("B01_bench_broken", "src/x.cpp", "sandbox", "the engine stops printing a bench line",
  ("int guard_one = 1;", "int guard_one = 1;  // MUTANT_SEEN BENCH_BROKEN"))
"""

CTEST_BROKEN = """
m("C01_ctest_broken", "src/x.cpp", "sandbox", "ctest exits non-zero having run nothing",
  ("int guard_two = 2;", "int guard_two = 2;  // CTEST_BROKEN"))
"""

STILLBORN = """
m("D01_stillborn", "src/x.cpp", "sandbox", "the mutant does not compile",
  ("int guard_one = 1;", "int guard_one = 1;  // WILL_NOT_COMPILE"))
"""

PAIR_BREAKS_LATER_PAIR = """
m("P01_two_pairs", "src/x.cpp", "sandbox", "pair 0 makes pair 1's anchor ambiguous",
  ("int guard_one = 1;", "int guard_one = 1; int guard_two = 2;"),
  ("int guard_two = 2;", "int guard_two = 3;"))
"""

SLOW_BUILD = """
m("W01_slow_build", "src/x.cpp", "sandbox", "a build long enough to signal during",
  ("int guard_one = 1;", "int guard_one = 1;  // SLOW_BUILD"))
"""

AMBIGUOUS = """
m("A01_ambiguous", "src/x.cpp", "sandbox", "an anchor that occurs twice",
  ("int twice", "int thrice"))
"""

MISSING = """
m("A02_missing", "src/x.cpp", "sandbox", "an anchor that occurs nowhere",
  ("int guard_three = 3;", "int guard_three = 4;"))
"""

OUTSIDE_SRC = """
m("O01_outside_src", "tools/gate.sh", "sandbox", "a file this tool will not touch",
  ("marked=0", "marked=1"))
"""

SELF_REPLACING = """
m("R01_self_replacing", "src/x.cpp", "sandbox", "a pair that changes nothing",
  ("int guard_one = 1;", "int guard_one = 1;"))
"""

BAD_EXPECTED = """
m("X01_bad_expected", "src/x.cpp", "sandbox", "a verdict nobody can hold it to",
  ("int guard_one = 1;", "int guard_one = 0;"), expected="probably")
"""

DUPLICATE = KILLED + KILLED


def last_line(text):
    lines = [line for line in text.splitlines() if line.strip()]
    return lines[-1] if lines else ""


class MutationCheckTest(unittest.TestCase):

    def setUp(self):
        self.box = Sandbox()
        self.addCleanup(self.box.close)

    # -- refusals -----------------------------------------------------------

    def test_ambiguous_anchor_refused_before_any_write(self):
        proc = self.box.run(self.box.mutant_file(AMBIGUOUS))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("occurs 2 times", proc.stdout)
        self.assertEqual(self.box.worktree_dirty(), "")
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_missing_anchor_refused_before_any_write(self):
        proc = self.box.run(self.box.mutant_file(MISSING))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("occurs 0 times", proc.stdout)
        self.assertEqual(self.box.worktree_dirty(), "")
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_duplicate_id_refused(self):
        proc = self.box.run(self.box.mutant_file(DUPLICATE))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("duplicate id", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_file_outside_src_refused(self):
        """`git checkout --` reverts whatever the mutant named, so the tool
        writes only where it is certain reverting is safe."""
        proc = self.box.run(self.box.mutant_file(OUTSIDE_SRC))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("is outside src/", proc.stdout)
        self.assertEqual(self.box.worktree_dirty(), "")
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_pair_that_replaces_its_anchor_with_itself_refused(self):
        """It would build, run and come back green, and the row would read as a
        suite that missed a bug rather than a bug that was never applied."""
        proc = self.box.run(self.box.mutant_file(SELF_REPLACING))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("replaces its anchor with itself", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_expected_outside_the_two_verdicts_refused(self):
        proc = self.box.run(self.box.mutant_file(BAD_EXPECTED))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("not 'killed' or 'equivalent'", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_main_tree_refused(self):
        """`git checkout --` in the tree somebody works in reverts their work."""
        proc = self.box.run(self.box.mutant_file(KILLED), target=self.box.repo)
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("is the main tree", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_unknown_only_id_refused(self):
        proc = self.box.run(self.box.mutant_file(KILLED), extra=["--only", "M99"])
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("no such mutant: M99", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_a_fixture_dirty_outside_src_refused(self):
        """S097 verdict 2's second run: src/ clean, tests/ and adocs/ edited in
        place. The header named a commit whose tests did not hold the mined row
        the run's kill depended on, and nothing in the run said so."""
        self.box.dirty_the_fixture("tests/test_x.cpp", "adocs/specs.md")
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("the fixture is dirty", proc.stdout)
        self.assertIn("tests/test_x.cpp", proc.stdout)
        self.assertIn("adocs/specs.md", proc.stdout)
        # No row was scored against a tree nobody can name.
        self.assertNotIn("K01_killed", proc.stdout)
        self.assertNotIn("mutation score", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_the_header_names_the_fixture_sha_and_what_dirties_it(self):
        """The header is the whole record of what a run measured, so it is
        printed before the guards and carries the paths when the tree is not
        the sha beside them. The file dirtied here is the root CMakeLists,
        which the allowlist this guard started as would not have looked at."""
        clean = self.box.run(self.box.mutant_file(KILLED))
        self.assertEqual(clean.returncode, 0, clean.stdout)
        self.assertRegex(clean.stdout, r"worktree \S+ at [0-9a-f]{7,} clean")
        self.box.dirty_the_fixture("CMakeLists.txt")
        dirty = self.box.run(self.box.mutant_file(KILLED))
        self.assertNotEqual(dirty.returncode, 0)
        self.assertRegex(
            dirty.stdout, r"worktree \S+ at [0-9a-f]{7,} dirty: CMakeLists\.txt")
        self.assertIn("the fixture is dirty", dirty.stdout)

    def test_the_header_names_where_the_mutant_list_was_read_from(self):
        """The list is the one thing a run reads that the fixture's sha does
        not describe: load_mutants resolves it against this process's cwd, so
        the documented invocation reads the main tree's copy while the
        worktree's own is never opened."""
        outside = self.box.run(self.box.mutant_file(KILLED))
        self.assertRegex(outside.stdout,
                         r"list\s+\S+/mutants\.py\s+outside any git tree")

        listed = os.path.join(self.box.repo, "tools", "mutants", "listed.py")
        os.makedirs(os.path.dirname(listed))
        with open(listed, "w") as handle:
            handle.write(KILLED)
        untracked = self.box.run(listed)
        self.assertRegex(untracked.stdout,
                         r"list\s+\S+/listed\.py\s+dirty: tools/mutants/listed\.py")

        self.box.git(self.box.repo, "add", "tools/mutants/listed.py")
        self.box.git(self.box.repo, "-c", "user.email=t@example.invalid",
                     "-c", "user.name=test", "commit", "--quiet", "-m", "list")
        tracked = self.box.run(listed)
        self.assertRegex(tracked.stdout, r"list\s+\S+/listed\.py\s+clean")

    # -- verdicts -----------------------------------------------------------

    def test_killed_row_carries_the_failing_assertion(self):
        """The failing test's own output holds a summary-shaped line of its
        own -- this suite is exactly such a binary, and --output-on-failure
        prints it before ctest's real summary. The row reads 1/1 because the
        count comes from the last summary in the log and not the first."""
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        self.assertRegex(proc.stdout, r"K01_killed\s+yes\s+1/1\s+\S+\s+killed")
        self.assertIn("[test_x] a fabricated case the stub reports", proc.stdout)
        self.assertIn("CHECK( guard_one_holds() )", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-DONE")
        self.assertEqual(self.box.worktree_dirty(), "")

    def test_declared_equivalent_reads_equivalent(self):
        proc = self.box.run(self.box.mutant_file(EQUIVALENT))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        self.assertRegex(proc.stdout,
                         r"E01_equivalent\s+yes\s+0/1\s+same\s+equivalent")
        self.assertIn("mutation score 0 of 0", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-DONE")

    def test_moved_signature_with_a_green_suite_survives(self):
        """A moved bench is a changed tree, so no declaration could rescue it."""
        proc = self.box.run(self.box.mutant_file(BENCH_SURVIVOR))
        self.assertNotEqual(proc.returncode, 0)
        self.assertRegex(proc.stdout,
                         r"S01_bench_moved\s+yes\s+0/1\s+moved\s+survived")
        self.assertIn("not what the list expects", proc.stdout)
        self.assertEqual(last_line(proc.stdout),
                         "MUTATION-RUN-FAILED: a verdict differs "
                         "from what the mutant list expects")

    def test_still_signature_undeclared_survives(self):
        """The M19 class: the bench is blind and nothing was declared, so the
        verdict is survived and not equivalent."""
        proc = self.box.run(self.box.mutant_file(BLIND_SURVIVOR))
        self.assertNotEqual(proc.returncode, 0)
        self.assertRegex(proc.stdout,
                         r"S02_bench_blind\s+yes\s+0/1\s+same\s+survived")
        self.assertEqual(last_line(proc.stdout),
                         "MUTATION-RUN-FAILED: a verdict differs "
                         "from what the mutant list expects")

    def test_a_ceiling_hit_alone_is_unmeasured(self):
        """A busy machine turns a passing test into a Timeout row, and that row
        is the whole evidence: nothing asserted, so nothing was detected."""
        proc = self.box.run(self.box.mutant_file(TIMEOUT_ONLY))
        self.assertNotEqual(proc.returncode, 0)
        self.assertRegex(proc.stdout,
                         r"T01_timeout_only\s+yes\s+1/1\s+same\s+unmeasured")
        self.assertEqual(last_line(proc.stdout),
                         "MUTATION-RUN-FAILED: a verdict differs "
                         "from what the mutant list expects")

    def test_a_ceiling_hit_beside_a_real_red_is_killed(self):
        """M22 is this row on the real tree: it hangs test_uci_surface, which
        passes in 9.26 s unmutated, while four other binaries fail on
        assertions. Reading that as unmeasured hides a kill, and a quiet machine
        does not recover it -- the hang belongs to the mutant."""
        proc = self.box.run(self.box.mutant_file(TIMEOUT_PLUS_FAIL))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        self.assertRegex(proc.stdout,
                         r"T02_timeout_plus_fail\s+yes\s+2/2\s+\S+\s+killed")
        self.assertIn("CHECK( guard_one_holds() )", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-DONE")

    def test_a_bench_that_will_not_read_still_reports_the_kill(self):
        """An engine too broken to print a bench line is a strong signal, not a
        missing one. The suite asserted; that it did is the verdict."""
        proc = self.box.run(self.box.mutant_file(BENCH_BROKEN))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        self.assertRegex(proc.stdout,
                         r"B01_bench_broken\s+yes\s+1/1\s+n/a\s+killed")
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-DONE")

    def test_ctest_that_failed_to_run_is_not_a_kill(self):
        """A non-zero exit with no failing row parsed is ctest failing, not a
        test failing -- the naive-parser trap the ceiling rule was written
        against, in its other form."""
        proc = self.box.run(self.box.mutant_file(CTEST_BROKEN))
        self.assertNotEqual(proc.returncode, 0)
        self.assertRegex(proc.stdout,
                         r"C01_ctest_broken\s+yes\s+\S+\s+\S+\s+unmeasured")
        self.assertEqual(last_line(proc.stdout),
                         "MUTATION-RUN-FAILED: a verdict differs "
                         "from what the mutant list expects")

    def test_a_mutant_that_does_not_compile_is_stillborn(self):
        """The -Werror class: a mutant that orphans a local does not build in
        Release, which is a fact about the mutant and not about the suite. It is
        scored out, and the run says so rather than counting it a kill."""
        proc = self.box.run(self.box.mutant_file(STILLBORN))
        self.assertNotEqual(proc.returncode, 0)
        self.assertRegex(proc.stdout, r"D01_stillborn\s+no\s+-\s+-\s+stillborn")
        self.assertIn("does not compile", proc.stdout)
        self.assertIn("declared void", proc.stdout)
        self.assertEqual(last_line(proc.stdout),
                         "MUTATION-RUN-FAILED: a verdict differs "
                         "from what the mutant list expects")

    def test_only_takes_the_short_id(self):
        """`--only M26` is what a step stamp quotes, not the full name."""
        proc = self.box.run(self.box.mutant_file(KILLED + BENCH_SURVIVOR),
                            extra=("--only", "K01"))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        self.assertIn("mutants  1 of 2", proc.stdout)
        self.assertIn("K01_killed", proc.stdout)
        self.assertNotIn("S01_bench_moved", proc.stdout)

    # -- the tree it borrows ------------------------------------------------

    def test_a_pair_that_breaks_a_later_pair_leaves_nothing_behind(self):
        """validate() checks each anchor against the pristine file, so a mutant
        whose pair 0 makes pair 1 ambiguous passes it and fails while applying,
        with pair 0 already written. The revert has to cover that."""
        proc = self.box.run(self.box.mutant_file(PAIR_BREAKS_LATER_PAIR))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("stopped being unique", proc.stdout)
        self.assertEqual(self.box.worktree_dirty(), "")
        self.assertIn("MUTATION-RUN-FAILED", last_line(proc.stdout))

    def test_sigterm_reverts_the_mutant_before_exiting(self):
        """A detached run is stopped with `kill`, and Python's default SIGTERM
        exits without unwinding: the mutant would stay in the worktree and every
        later run would refuse at the clean-src check."""
        proc = self.box.spawn(self.box.mutant_file(SLOW_BUILD))
        self.addCleanup(proc.kill)
        deadline = time.time() + 30
        while time.time() < deadline:
            with open(self.box.src) as handle:
                if "SLOW_BUILD" in handle.read():
                    break
            time.sleep(0.05)
        else:
            self.fail("the mutant was never applied")
        proc.terminate()
        output = proc.communicate(timeout=30)[0]
        self.assertNotEqual(proc.returncode, 0)
        self.assertEqual(self.box.worktree_dirty(), "")
        self.assertIn("MUTATION-RUN-FAILED", output)

    def test_the_summary_after_the_last_test_is_not_its_assertion(self):
        """ctest's trailing block follows the last test with no `Start` line to
        close it, so a `FAIL:` or `ERROR:` line there is read as the last
        binary's own -- and a binary with no output of its own, a ceiling hit
        here, has nothing to shadow it."""
        proc = self.box.run(self.box.mutant_file(TIMEOUT_PLUS_FAIL))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        self.assertIn("[test_y] -  |  (no assertion text in the log)",
                      proc.stdout)
        self.assertNotIn("belonging to no binary", proc.stdout)

    def test_worktree_is_clean_after_every_mutant(self):
        both = KILLED + BENCH_SURVIVOR
        proc = self.box.run(self.box.mutant_file(both))
        self.assertIn("K01_killed", proc.stdout)
        self.assertIn("S01_bench_moved", proc.stdout)
        self.assertEqual(self.box.worktree_dirty(), "")
        with open(self.box.src) as handle:
            self.assertEqual(handle.read(), SOURCE)

    def test_red_baseline_stops_the_run(self):
        """A suite already red cannot say whether it saw the mutant, so every
        row after it would be uninterpretable."""
        with open(self.box.src, "a") as handle:
            handle.write("// MUTANT_SEEN planted before the run\n")
        self.box.git(self.box.worktree, "-c", "user.email=t@example.invalid",
                     "-c", "user.name=test", "commit", "--quiet", "-a",
                     "-m", "a red tree")
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("the unmutated worktree is red", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_a_label_that_selects_no_test_stops_the_run(self):
        """S097 verdict 2's first launch passed `--label S097_v2`, a label no
        test carries. ctest selected nothing, exited 0 and printed no summary;
        the run read `baseline green, ? tests` and scored twenty mutants as
        survivors over an empty suite in 154 s."""
        proc = self.box.run(self.box.mutant_file(KILLED),
                            extra=("--label", "S097_v2"))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("the baseline ran no tests", proc.stdout)
        self.assertIn("ctest printed no summary line", proc.stdout)
        self.assertIn("'S097_v2'", proc.stdout)
        self.assertNotIn("baseline green", proc.stdout)
        self.assertNotIn("K01_killed", proc.stdout)
        self.assertNotIn("mutation score", proc.stdout)
        self.assertEqual(self.box.worktree_dirty(), "")
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_a_baseline_whose_ctest_never_ran_is_not_blamed_on_the_label(self):
        """A missing count has two causes and they are different faults: a
        label that selects nothing, and a ctest that would not run. The second
        gets its exit code, not a sentence about the label."""
        with open(self.box.src, "a") as handle:
            handle.write("// CTEST_BROKEN planted before the run\n")
        self.box.git(self.box.worktree, "-c", "user.email=t@example.invalid",
                     "-c", "user.name=test", "commit", "--quiet", "-a",
                     "-m", "a tree whose ctest will not start")
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("the baseline printed no test count", proc.stdout)
        self.assertIn("ctest exited 1", proc.stdout)
        self.assertNotIn("the baseline ran no tests", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_a_baseline_summary_counting_to_zero_stops_the_run(self):
        """The same emptiness in the shape that does print a summary: zero
        failed out of zero is a green a failure count alone cannot tell from a
        real one, and the count is what the tool reads."""
        proc = self.box.run(self.box.mutant_file(KILLED),
                            extra=("--label", "empty_summary"))
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("the baseline ran no tests", proc.stdout)
        self.assertIn("ctest ran 0 tests", proc.stdout)
        self.assertIn("'empty_summary'", proc.stdout)
        self.assertNotIn("K01_killed", proc.stdout)
        self.assertNotIn("mutation score", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_results_tsv_is_written_under_the_build_directory(self):
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        tsv = os.path.join(self.box.worktree, "build", "mutation", "results.tsv")
        self.assertTrue(os.path.isfile(tsv))
        with open(tsv) as handle:
            body = handle.read()
        self.assertIn("BASELINE", body)
        self.assertRegex(body, r"K01_killed\t.*\tkilled\t")


if __name__ == "__main__":
    unittest.main(verbosity=2)
