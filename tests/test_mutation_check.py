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
CTEST_BROKEN; it lists a test that never ran -- skipped on SKIP_ONE, disabled on
DISABLE_ONE, its executable missing on NOT_RUN -- and SKIP_ONE beside
MUTANT_SEEN is a real red with a skip next to it (S339); the engine's bench total moves when it holds BENCH_MOVES and it
prints no bench line at all on BENCH_BROKEN; `cmake` fails with an `error:` on
WILL_NOT_COMPILE, which is the -Werror class the two `(void)` mutants exist
for. So the four squares of the verdict table -- suite red, suite green
with a moved signature, suite green with a still signature declared equivalent,
suite green with a still signature not declared -- are each reachable by one
mutant, and that is what the cases below drive. Every call the stub takes is
logged with its arguments, so a case can read what the tool asked ctest for.

AND THE REAL LIST, S338. MutantAnchorsTest is the one class here that reads
tools/mutants/ and src/ rather than a sandbox: every anchor of every mutant has
to match exactly once in the tree this file sits in. Each class runs under its
own ctest entry, named in it, because that class must stay out of the suite a
mutation run judges by and this one must not.
"""

import importlib.util
import os
import re
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
echo "$*" >> "$CTEST_ARGS_LOG"
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
# S339: the rows ctest lists for a test that never ran, in the shapes ctest
# 3.28.3 printed for them on a scratch project on 2026-11-12. A test exiting
# with its SKIP_RETURN_CODE and one marked DISABLED go under "did not run" and
# leave the exit at 0; one whose executable is missing goes under FAILED and
# makes it 8. SKIP_ONE beside MUTANT_SEEN is a real failure with a skip next to
# it, the two blocks in the order ctest prints them.
if grep -q SKIP_ONE "$MUT_SRC" && grep -q MUTANT_SEEN "$MUT_SRC"; then
  cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/2 Test #1: test_x ...........................***Failed    0.01 sec
===============================================================================
/sandbox/tests/test_x.cpp:10:
TEST CASE:  a fabricated case the stub reports

/sandbox/tests/test_x.cpp:12: ERROR: CHECK( guard_one_holds() ) is NOT correct!
  values: CHECK( false )

    Start 2: test_skips
2/2 Test #2: test_skips .......................***Skipped   0.00 sec

50% tests passed, 1 tests failed out of 2

The following tests did not run:
	  2 - test_skips (Skipped)

The following tests FAILED:
	  1 - test_x (Failed)
Errors while running CTest
EOF
  exit 8
fi
if grep -q SKIP_ONE "$MUT_SRC"; then
  cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/2 Test #1: test_x ...........................   Passed    0.01 sec
    Start 2: test_skips
2/2 Test #2: test_skips .......................***Skipped   0.00 sec

100% tests passed, 0 tests failed out of 2

The following tests did not run:
	  2 - test_skips (Skipped)
EOF
  exit 0
fi
if grep -q DISABLE_ONE "$MUT_SRC"; then
  cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/2 Test #1: test_x ...........................   Passed    0.01 sec
    Start 2: test_off
2/2 Test #2: test_off .........................***Not Run (Disabled)   0.00 sec

100% tests passed, 0 tests failed out of 1

The following tests did not run:
	  2 - test_off (Disabled)
EOF
  exit 0
fi
if grep -q NOT_RUN "$MUT_SRC"; then
  cat <<'EOF'
Test project /sandbox
    Start 1: test_x
1/2 Test #1: test_x ...........................   Passed    0.01 sec
    Start 2: test_gone
Could not find executable /sandbox/build/tests/test_gone
Unable to find executable: /sandbox/build/tests/test_gone
2/2 Test #2: test_gone ........................***Not Run   0.00 sec

50% tests passed, 1 tests failed out of 2

The following tests FAILED:
	  2 - test_gone (Not Run)
Errors while running CTest
EOF
  exit 8
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
        # Every ctest call the tool makes, one line of arguments each.
        self.ctest_args = os.path.join(self.root, "ctest_args")

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
        env["CTEST_ARGS_LOG"] = self.ctest_args
        return subprocess.Popen(
            [sys.executable, TOOL, mutants, target or self.worktree, *extra],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, env=env)

    def run(self, mutants, target=None, extra=()):
        env = dict(os.environ)
        env["PATH"] = self.stubs + os.pathsep + env["PATH"]
        env["MUT_SRC"] = self.src
        env["CTEST_ARGS_LOG"] = self.ctest_args
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

    def commit_marker(self, marker, message):
        """A stub marker committed into the worktree's own source, so the
        unmutated tree -- the baseline, and every mutant's run -- carries it."""
        with open(self.src, "a") as handle:
            handle.write(f"// {marker} planted before the run\n")
        self.git(self.worktree, "-c", "user.email=t@example.invalid",
                 "-c", "user.name=test", "commit", "--quiet", "-a",
                 "-m", message)

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

NOT_RUN = """
m("N01_not_run", "src/x.cpp", "sandbox", "ctest cannot find a test's executable",
  ("int guard_two = 2;", "int guard_two = 2;  // NOT_RUN"))
"""

# S339: a pair that is not an (old, new) pair of strings, as the text that
# stands for it in an m() call. Before S339 the first three died on a
# ValueError at validate's unpacking, the fourth on a TypeError at its count,
# and the fifth passed validate and died while being applied.
MALFORMED_PAIRS = {
    "a bare string": '"int guard_one = 1;"',
    "one string": '("int guard_one = 1;",)',
    "three strings": '("int guard_one = 1;", "int guard_one = 0;", "x")',
    "a number for the anchor": '(1, "int guard_one = 0;")',
    "a number for the replacement": '("int guard_one = 1;", 0)',
}
MALFORMED = """
m("Q01_malformed_pair", "src/x.cpp", "sandbox", "a pair that is not a pair",
  {pair})
"""

# The form of the two evidence copies under adocs/data/: the file brings its
# own `m` and appends to `M`, and the loader reads that list. {pairs} is the
# field the entry gives, or nothing.
OWN_M = """
M = []


def m(mid, file, klass, note, *pairs):
    M.append(dict(id=mid, file=file, klass=klass, note=note{pairs}))


m("O02_own_m", "src/x.cpp", "sandbox", "an entry from the file's own m",
  ("int guard_one = 1;", "int guard_one = 1;  // MUTANT_SEEN"))
"""


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

    def test_a_pair_that_is_not_two_strings_refused_by_name(self):
        """S338's plant p14: a bare string where a pair goes died on a
        ValueError at validate's unpacking, a traceback naming neither the
        mutant nor its file. Each shape is refused before the baseline, by the
        line every refusal of validate's starts with."""
        for shape, pair in MALFORMED_PAIRS.items():
            with self.subTest(shape=shape):
                path = self.box.mutant_file(MALFORMED.replace("{pair}", pair))
                proc = self.box.run(path)
                self.assertNotEqual(proc.returncode, 0)
                self.assertIn(f"Q01_malformed_pair in {path}: pair 0 is not an "
                              "(old, new) pair of strings", proc.stdout)
                self.assertNotIn("Traceback", proc.stdout)
                self.assertNotIn("baseline green", proc.stdout)
                self.assertEqual(self.box.worktree_dirty(), "")
                self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-FAILED")

    def test_an_own_m_entry_without_pairs_refused_by_name(self):
        """S338's plant p15: a file that brings its own `m` -- the two evidence
        copies under adocs/data/ do -- and an entry that gives no pairs died
        on a KeyError in normalise, naming no file. The same file giving its
        pairs loads and runs, so what is refused is the missing field and not
        the form."""
        given = self.box.run(self.box.mutant_file(
            OWN_M.replace("{pairs}", ", pairs=list(pairs)")))
        self.assertEqual(given.returncode, 0, given.stdout)
        self.assertRegex(given.stdout, r"O02_own_m\s+yes\s+1/1\s+\S+\s+killed")

        path = self.box.mutant_file(OWN_M.replace("{pairs}", ""))
        proc = self.box.run(path)
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn(f"O02_own_m in {path}: an entry from the file's own `m` "
                      "gives no pairs", proc.stdout)
        self.assertNotIn("Traceback", proc.stdout)
        self.assertNotIn("baseline green", proc.stdout)
        self.assertEqual(self.box.worktree_dirty(), "")
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

    def test_the_progress_line_reads_the_bench_as_the_table_does(self):
        """A run stopped before its table leaves only these lines, and they
        called every unread bench `same`: S118's first run has PT07 as `bench
        same` on its line and `n/a` in its table. `same` is a claim about the
        tree, so it takes a signature that was read. S305. The two rows that
        did read one are here so a line printing `n/a` always would not pass."""
        proc = self.box.run(self.box.mutant_file(
            BENCH_BROKEN + STILLBORN + BENCH_SURVIVOR + BLIND_SURVIVOR))
        for mutant, bench in (("B01_bench_broken", "n/a"),
                              ("D01_stillborn", "-"),
                              ("S01_bench_moved", "moved"),
                              ("S02_bench_blind", "same")):
            with self.subTest(mutant=mutant):
                self.assertRegex(
                    proc.stdout,
                    rf"(?m)^  {mutant}\s+\S+\s+fast \S+\s+bench {bench}  ")
                self.assertRegex(proc.stdout,
                                 rf"(?m)^{mutant}\s+\S+\s+\S+\s+{bench}\s")

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

    def test_a_test_that_did_not_run_is_no_kill_and_the_run_names_it(self):
        """S338 found it on a two-test scratch project: ctest lists a test that
        exited with its SKIP_RETURN_CODE under "The following tests did not
        run:", in the `N - name (how)` shape of a failure, and the tool read
        the row as an assertion, so an unmutated tree scored killed.
        test_bare_fen_relay skips that way on a machine where no python imports
        python-chess. A DISABLED test is listed the same way. The mutant is the
        quiet one, so its suite reads what the unmutated tree's reads."""
        for marker, name, how, fast in (("SKIP_ONE", "test_skips", "Skipped", "0/2"),
                                        ("DISABLE_ONE", "test_off", "Disabled", "0/1")):
            with self.subTest(how=how):
                box = Sandbox()
                self.addCleanup(box.close)
                box.commit_marker(marker, f"a tree with a test that is {how}")
                proc = box.run(box.mutant_file(EQUIVALENT))
                self.assertRegex(
                    proc.stdout,
                    rf"E01_equivalent\s+yes\s+{fast}\s+same\s+equivalent")
                self.assertIn("mutation score 0 of 0", proc.stdout)
                # And the precondition that would make it a kill: ctest listed
                # the row, and the run says which test did not run.
                self.assertRegex(proc.stdout,
                                 rf"(?m)^baseline did not run: {name} \({how}\)")
                self.assertEqual(proc.returncode, 0, proc.stdout)
                self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-DONE")

    def test_a_real_failure_beside_a_skipped_test_is_still_killed(self):
        """The skip takes nothing away from a kill and adds nothing to it: the
        row reads killed by the binary that asserted, and the test that did not
        run is neither in what caught it nor in the failing tests the TSV
        records."""
        self.box.commit_marker("SKIP_ONE", "a tree with a skipped test")
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        self.assertRegex(proc.stdout, r"K01_killed\s+yes\s+1/2\s+\S+\s+killed")
        self.assertIn("[test_x] a fabricated case the stub reports", proc.stdout)
        self.assertNotIn("[test_skips]", proc.stdout)
        self.assertEqual(last_line(proc.stdout), "MUTATION-RUN-DONE")
        tsv = os.path.join(self.box.worktree, "build", "mutation", "results.tsv")
        with open(tsv) as handle:
            row = [line.split("\t") for line in handle
                   if line.startswith("K01_killed\t")]
        self.assertEqual(len(row), 1)
        self.assertEqual(row[0][10], "test_x(Failed)")

    def test_a_test_whose_executable_is_missing_is_no_kill(self):
        """ctest lists it under FAILED as `(Not Run)` and exits non-zero, and
        it ran nothing: with no assertion beside it that is ctest failing to
        run a test, which is unmeasured and not a detection."""
        proc = self.box.run(self.box.mutant_file(NOT_RUN))
        self.assertNotEqual(proc.returncode, 0)
        self.assertRegex(proc.stdout, r"N01_not_run\s+yes\s+1/2\s+\S+\s+unmeasured")
        self.assertNotIn("[test_gone]", proc.stdout)
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

    def test_the_suite_that_judges_a_mutant_leaves_out_the_list_readers(self):
        """S338: MutantAnchorsTest below reads this list against src/, so with
        a mutant applied it finds that mutant's anchor gone and goes red -- under
        261 of the 264 mutants of the day it landed, each of which would then
        score killed by a test that never ran the engine. The baseline is
        excluded too, so its count is the count of the suite that judges."""
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        with open(self.box.ctest_args) as handle:
            calls = handle.read().splitlines()
        # The baseline and the one mutant: without both, nothing below ran.
        self.assertEqual(len(calls), 2, calls)
        for call in calls:
            self.assertIn("-L fast -LE mutant_list", call)

    def test_the_anchors_entry_runs_under_fast_and_its_own_label(self):
        """S338's fast check: an anchors entry that loses `fast` stops running,
        and MutantAnchorsTest's own guard stops with it, so the gate reads green
        while nothing holds the anchors. This class runs under `fast` whatever
        happens to that entry, so the check on its labels lives here."""
        with open(CMAKE_LISTS) as handle:
            entries = file_entries(handle.read())
        anchors = [entry for entry in entries
                   if entry["after"] == [ANCHORS_CLASS]]
        self.assertTrue(anchors, f"no live entry runs {ANCHORS_CLASS}")
        for entry in anchors:
            self.assertEqual(sorted(entry["labels"]), ["fast", "mutant_list"],
                             entry["name"])

    def test_results_tsv_is_written_under_the_build_directory(self):
        proc = self.box.run(self.box.mutant_file(KILLED))
        self.assertEqual(proc.returncode, 0, proc.stdout)
        tsv = os.path.join(self.box.worktree, "build", "mutation", "results.tsv")
        self.assertTrue(os.path.isfile(tsv))
        with open(tsv) as handle:
            body = handle.read()
        self.assertIn("BASELINE", body)
        self.assertRegex(body, r"K01_killed\t.*\tkilled\t")


MUTANTS = os.path.join(ROOT, "tools", "mutants")
CMAKE_LISTS = os.path.join(ROOT, "tests", "CMakeLists.txt")
ANCHORS_CLASS = "MutantAnchorsTest"

# Read as cmake reads it, because S338's fast check beat a regex over the raw
# text four ways: a commented-out call read as live, and three entries running
# this file that the regex never saw -- a `-v`, a ${Python3_EXECUTABLE}, a
# Class.method.
CMAKE_BRACKET_COMMENT_RE = re.compile(r"#\[(=*)\[")
CMAKE_CALL_RE = re.compile(r"\b(add_test|set_tests_properties|set_property)\s*\(",
                           re.I)
CMAKE_TOKEN_RE = re.compile(r'"((?:\\.|[^"\\])*)"|([^\s()"]+)')
ADD_TEST_KEYWORDS = ("CONFIGURATIONS", "WORKING_DIRECTORY", "COMMAND_EXPAND_LISTS")


def cmake_live(text):
    """The text with every comment blanked, line and bracket, outside strings."""
    out = []
    i, quoted = 0, False
    while i < len(text):
        char = text[i]
        if quoted:
            if char == "\\":
                out.append(text[i:i + 2])
                i += 2
                continue
            quoted = char != '"'
        elif char == '"':
            quoted = True
        elif char == "#":
            bracket = CMAKE_BRACKET_COMMENT_RE.match(text, i)
            if bracket:
                close = "]" + bracket.group(1) + "]"
                end = text.find(close, bracket.end())
                end = len(text) if end < 0 else end + len(close)
            else:
                end = text.find("\n", i)
                end = len(text) if end < 0 else end
            out.append(" ")
            i = end
            continue
        out.append(char)
        i += 1
    return "".join(out)


def cmake_calls(text):
    """(command, its arguments) for every live call that registers a test or
    sets a property on one, arguments unquoted."""
    live = cmake_live(text)
    calls = []
    for match in CMAKE_CALL_RE.finditer(live):
        i, depth, quoted = match.end(), 1, False
        while i < len(live) and depth:
            char = live[i]
            if quoted:
                if char == "\\":
                    i += 1
                quoted = char != '"'
            elif char == '"':
                quoted = True
            elif char in "()":
                depth += 1 if char == "(" else -1
            i += 1
        body = live[match.end():i - 1]
        args = [token.group(1) if token.group(1) is not None else token.group(2)
                for token in CMAKE_TOKEN_RE.finditer(body)]
        calls.append((match.group(1).lower(), args))
    return calls


def file_entries(text):
    """Every live add_test whose command mentions this file: its name, the
    arguments after the file (None when no argument is the file itself), its
    LABELS as the last set_tests_properties setting them leaves them, and
    whether a set_property touches its labels."""
    calls = cmake_calls(text)
    labels, by_set_property = {}, set()
    for command, args in calls:
        if command == "set_tests_properties" and "PROPERTIES" in args:
            split = args.index("PROPERTIES")
            pairs = args[split + 1:]
            for key, value in zip(pairs[::2], pairs[1::2]):
                if key == "LABELS":
                    for name in args[:split]:
                        labels[name] = value.split(";")
        elif command == "set_property" and "LABELS" in args:
            by_set_property.update(args)
    entries = []
    for command, args in calls:
        if command != "add_test" or not any("test_mutation_check" in arg
                                            for arg in args):
            continue
        if args[0] == "NAME":
            name = args[1]
            run = args[args.index("COMMAND") + 1:] if "COMMAND" in args else []
            for keyword in ADD_TEST_KEYWORDS:
                if keyword in run:
                    run = run[:run.index(keyword)]
        else:
            name, run = args[0], args[1:]
        at = [index for index, arg in enumerate(run)
              if arg.endswith("test_mutation_check.py")]
        entries.append(dict(name=name,
                            after=run[at[0] + 1:] if at else None,
                            labels=labels.get(name, []),
                            set_property=name in by_set_property))
    return entries


def test_classes():
    return sorted(name for name, value in globals().items()
                  if isinstance(value, type)
                  and issubclass(value, unittest.TestCase))


def registration_problems(text, classes, list_label):
    """What keeps MutantAnchorsTest out of the suite a mutation run judges by,
    and every class of this file running at all. Empty when both hold."""
    entries = file_entries(text)
    if not entries:
        return ["no live add_test runs test_mutation_check.py"]
    problems = []
    named = set()
    for entry in entries:
        name, after = entry["name"], entry["after"]
        if after is None or len(after) != 1 or after[0] not in classes:
            problems.append(f"{name} runs the file with arguments {after}, "
                            f"not one class of {classes}")
            continue
        named.add(after[0])
        if entry["set_property"]:
            problems.append(f"{name} has labels from set_property, which this "
                            "guard does not read; use set_tests_properties")
        if after[0] == ANCHORS_CLASS and list_label not in entry["labels"]:
            problems.append(f"{name} runs {ANCHORS_CLASS} with labels "
                            f"{entry['labels']}, without {list_label!r}, so a "
                            "mutation run would judge by it")
    for missing in sorted(set(classes) - named):
        problems.append(f"{missing} has no entry of its own, so nothing runs it")
    return problems


# The registration the guard has to pass, a comment that is no registration
# included, and the plants it has to refuse: (old, new, what the red says).
REGISTRATION = """\
# add_test(NAME retired COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py)
add_test(NAME test_mutation_check
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py MutationCheckTest)
set_tests_properties(test_mutation_check PROPERTIES LABELS "fast" TIMEOUT 60)

add_test(NAME test_mutant_anchors
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py MutantAnchorsTest)
set_tests_properties(test_mutant_anchors PROPERTIES LABELS "fast;mutant_list" TIMEOUT 60)
"""
ANCHORS_PROPERTIES = ('set_tests_properties(test_mutant_anchors PROPERTIES '
                      'LABELS "fast;mutant_list" TIMEOUT 60)')
REGISTRATION_PLANTS = {
    "the labels commented out, a live fast-only line below": (
        ANCHORS_PROPERTIES,
        "# " + ANCHORS_PROPERTIES + "\nset_tests_properties(test_mutant_anchors "
        'PROPERTIES LABELS "fast" TIMEOUT 60)',
        "without 'mutant_list'"),
    "a later live call overriding the labels": (
        ANCHORS_PROPERTIES,
        ANCHORS_PROPERTIES + "\nset_tests_properties(test_mutant_anchors "
        'PROPERTIES LABELS "fast")',
        "without 'mutant_list'"),
    "an entry naming no class": (
        "test_mutation_check.py MutationCheckTest)",
        "test_mutation_check.py)",
        "test_mutation_check runs the file with arguments []"),
    "an entry passing -v": (
        ANCHORS_PROPERTIES,
        ANCHORS_PROPERTIES + "\nadd_test(NAME x COMMAND python3 "
        "${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py -v)\n"
        'set_tests_properties(x PROPERTIES LABELS "fast" TIMEOUT 60)',
        "runs the file with arguments ['-v']"),
    "an entry through ${Python3_EXECUTABLE} under fast alone": (
        ANCHORS_PROPERTIES,
        ANCHORS_PROPERTIES + "\nadd_test(NAME x COMMAND ${Python3_EXECUTABLE} "
        "${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py MutantAnchorsTest)\n"
        'set_tests_properties(x PROPERTIES LABELS "fast" TIMEOUT 60)',
        "x runs MutantAnchorsTest with labels ['fast'], without 'mutant_list'"),
    "an entry naming Class.method": (
        ANCHORS_PROPERTIES,
        ANCHORS_PROPERTIES + "\nadd_test(NAME x COMMAND python3 "
        "${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py "
        "MutantAnchorsTest.test_every_mutant_validates_against_this_tree)\n"
        'set_tests_properties(x PROPERTIES LABELS "fast" TIMEOUT 60)',
        "x runs the file with arguments "
        "['MutantAnchorsTest.test_every_mutant_validates_against_this_tree']"),
    "labels from set_property": (
        ANCHORS_PROPERTIES,
        ANCHORS_PROPERTIES + "\nset_property(TEST test_mutant_anchors "
        "PROPERTY LABELS fast)",
        "labels from set_property"),
    "a class with no entry": (
        "add_test(NAME test_mutation_check\n         COMMAND python3 "
        "${CMAKE_CURRENT_SOURCE_DIR}/test_mutation_check.py MutationCheckTest)",
        "",
        "MutationCheckTest has no entry of its own"),
}


def import_tool():
    """The tool as a module, so its loader and validator are the ones called.

    Imported rather than run: main() refuses the main tree and wants a clean
    commit and a green baseline before it validates anything, and none of those
    bears on whether an anchor still matches the tree it sits in.
    """
    sys.dont_write_bytecode = True  # no tools/__pycache__ left by a gate run
    spec = importlib.util.spec_from_file_location("mutation_check", TOOL)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def validate_list(tool, mutants_dir, tree):
    """(the mutants read, validate's refusal or None) -- main()'s two calls
    before it writes a byte. A file that does not load raises out of here with
    its own path and line in the traceback, which is the red for that kind."""
    mutants = tool.load_mutants([mutants_dir])
    try:
        tool.validate(mutants, tree)
    except tool.Refused as refused:
        return mutants, str(refused)
    return mutants, None


class MutantAnchorsTest(unittest.TestCase):
    """S338: the real list in tools/mutants/ against the src/ beside it.

    The class above gates the driver on a sandbox and never opens the real
    list, so on 2026-11-11 S102 gave `outside_pawn_terms` a sixth argument, the
    anchor of PT08 in tools/mutants/S118_pawn_cache.py matched no line, the
    validator would have refused every mutation run over the directory, and
    both gates were green. This class turns that red in the commit that does it.

    It builds nothing and applies nothing. A mutant that validates and is no
    longer killed is still the mutation run's to find.
    """

    @classmethod
    def setUpClass(cls):
        cls.tool = import_tool()

    def test_every_mutant_validates_against_this_tree(self):
        mutants, refusal = validate_list(self.tool, MUTANTS, ROOT)
        # The precondition: an empty list validates. Every mutant file in the
        # directory has to have given the loader at least one mutant.
        files = sorted(os.path.join(MUTANTS, name)
                       for name in os.listdir(MUTANTS) if name.endswith(".py"))
        self.assertTrue(files, f"no mutant file in {MUTANTS}")
        read = sorted({mut["source"] for mut in mutants})
        self.assertEqual(read, files, "the loader's files are not the "
                         f"directory's: none read from "
                         f"{sorted(set(files) - set(read))}, read from outside "
                         f"it {sorted(set(read) - set(files))}")
        if refusal:
            self.fail(f"{refusal}\n\nThe fix is a re-anchor: give the pair "
                      "named the text this tree now holds at that site, in the "
                      "mutant file named, and keep what the mutant breaks. A "
                      "mutant is never deleted to clear this (S338).")

    def test_a_red_names_the_mutant_its_file_the_anchor_and_the_count(self):
        """What turns a red into a re-anchor rather than a search: the id, the
        mutant file, the source file, the count and the anchor's own text."""
        tree = tempfile.mkdtemp(prefix="chesso-mutant-anchors.")
        self.addCleanup(shutil.rmtree, tree, True)
        os.makedirs(os.path.join(tree, "src"))
        with open(os.path.join(tree, "src", "x.cpp"), "w") as handle:
            handle.write(SOURCE)
        plants = os.path.join(tree, "mutants", "plants.py")
        os.makedirs(os.path.dirname(plants))
        with open(plants, "w") as handle:
            handle.write(AMBIGUOUS + MISSING)

        mutants, refusal = validate_list(self.tool, os.path.dirname(plants), tree)
        self.assertEqual(len(mutants), 2)
        self.assertIsNotNone(refusal)
        for mid, count, anchor in (("A01_ambiguous", 2, "int twice"),
                                   ("A02_missing", 0, "int guard_three = 3;")):
            with self.subTest(mutant=mid):
                self.assertIn(f"{mid} in {plants}: pair 0 anchor occurs "
                              f"{count} times in src/x.cpp, needs exactly 1\n"
                              f"    anchor {anchor!r}", refusal)

    def test_a_mutation_run_leaves_this_class_out(self):
        """A mutant applied removes its own anchor, so this class is red inside
        every mutation run. What keeps it out of the suite a run judges by is
        read here from the live calls in tests/CMakeLists.txt: every entry that
        runs this file names exactly one class and nothing else, every class
        has one, and every entry running this class carries the label the tool
        excludes. That its entry also carries `fast` is MutationCheckTest's to
        check, since this class stops running when that label goes."""
        with open(CMAKE_LISTS) as handle:
            cmake = handle.read()
        problems = registration_problems(cmake, test_classes(),
                                         self.tool.LIST_LABEL)
        if problems:
            self.fail(f"{CMAKE_LISTS}:\n  " + "\n  ".join(problems))

    def test_the_registration_guard_reads_what_cmake_reads(self):
        """S338's fast check beat the first form of the guard above four ways,
        each putting this class back into a mutation run's suite under `fast`
        while the guard stayed green. Each is planted here into a registration
        the guard passes, with the two other ways a label can change and a
        class left without an entry."""
        classes = test_classes()
        label = self.tool.LIST_LABEL
        with self.subTest(plant="none"):
            self.assertEqual(registration_problems(REGISTRATION, classes, label),
                             [])
        for plant, (old, new, red) in REGISTRATION_PLANTS.items():
            with self.subTest(plant=plant):
                self.assertEqual(REGISTRATION.count(old), 1)
                problems = registration_problems(
                    REGISTRATION.replace(old, new), classes, label)
                self.assertTrue(any(red in line for line in problems), problems)


if __name__ == "__main__":
    unittest.main(verbosity=2)
