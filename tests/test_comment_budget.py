#!/usr/bin/env python3
"""The gate on tools/comment_budget.py and adocs/data/S312_local_gate.sh.

The lexers are read directly, on fixtures with known answers. Every mode is driven
as a command over a throwaway git repository, so a red here is what a person would
see. A case asserting that something is not flagged first shows the thing is there.
"""

import importlib.util
import json
import os
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOL = os.path.join(ROOT, "tools", "comment_budget.py")
GATE = os.path.join(ROOT, "adocs", "data", "S312_local_gate.sh")
PAUSE_CHECK = os.path.join(ROOT, "adocs", "data", "S101_pause_check.sh")
PILOT = os.path.join(ROOT, "adocs", "data", "comment_study", "pilot")
PILOT_COMMIT = "5954c12"

DONE = "COMMENT-BUDGET-DONE"
FAILED = "COMMENT-BUDGET-FAILED"

spec = importlib.util.spec_from_file_location("comment_budget", TOOL)
cb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cb)


def clean_env(**extra):
    env = {k: v for k, v in os.environ.items() if not k.startswith("GIT_") and k != "CXX"}
    env.update(extra)
    return env


def read_text(path):
    with open(path, encoding="utf-8") as handle:
        return handle.read()


def pinned(path):
    """The file at the pilot's commit, or None in a clone without it: real text, and
    text no comment batch can rewrite under a fixture."""
    proc = subprocess.run(["git", "-C", ROOT, "show", f"{PILOT_COMMIT}:{path}"], capture_output=True, env=clean_env())
    return proc.stdout.decode("utf-8") if proc.returncode == 0 else None


def stripped(path, text):
    return cb.strip_comments(text, cb.editable_comments(path, text))


class LexerFixtures(unittest.TestCase):
    CPP = r"""#include <x> // one
int a = 1'000'000; /* two */ char c = '"'; // three
const char* s = "// not a comment /* neither */";
const char* r = R"x(// not
/* comment */ )" still )x"; // four
char d = '\''; char e = '/'; // five
/* six
   spans */
// seven \
   continues
int b = a /2/ 3; // eight
const char* u = u8R"(//no)"; char q = '\\'; // nine
"""

    def test_cpp(self):
        comments, _ = cb.lex_cpp(self.CPP)
        self.assertEqual(len(comments), 9, [self.CPP[c.start_off : c.end_off] for c in comments])
        self.assertEqual(
            [c.trailing for c in comments],
            [True, True, True, True, True, False, False, True, True],
        )

    def test_python(self):
        py = '''#!/usr/bin/env python3
"""Module doc."""
# one
x = "# no"  # two
def f():
    """Doc
    more."""
    y = """# not a doc
    # nor comment"""  # three
    return y
class C:
    "class doc"
s = 'é'  # four
'''
        comments, code = cb.lex_python(py)
        kinds = [c.kind for c in comments]
        self.assertEqual(kinds.count("line"), 4, kinds)
        self.assertEqual(kinds.count("docstring"), 3, kinds)
        self.assertEqual(kinds.count("shebang"), 1, kinds)
        self.assertEqual(py[comments[1].start_off : comments[1].end_off], '"""Module doc."""')
        self.assertIn(8, code)
        self.assertIn(9, code)
        self.assertNotIn(6, code)

    def test_shell(self):
        sh = r"""#!/bin/bash
# one
echo "# no" # two
echo '# no' ; # three
x=$# ; y=${#arr[@]} # four
cat <<HD
# heredoc data
HD
cat <<-'A' # five
	# data
	A
echo a#b # six
z=$'\'#' # seven
(( 16#ff )) # eight
"""
        comments, _ = cb.lex_shell(sh)
        real = [c for c in comments if c.kind == "line"]
        self.assertEqual(len(real), 8, [sh[c.start_off : c.end_off] for c in comments])

    def test_cmake(self):
        text = """# one
set(A "# no") # two
#[[ three
spans ]]
set(B [[ # no ]]) # four
"""
        comments, _ = cb.lex_cmake(text)
        self.assertEqual(len(comments), 4, [text[c.start_off : c.end_off] for c in comments])

    def test_strip(self):
        self.assertEqual(stripped("x.cpp", "int a; // z\n// w\nint b;\n"), "int a;\nint b;\n")


class ShellQuotesNest(unittest.TestCase):
    """A `"` inside `$(...)` or `${...}` closes nothing outside it."""

    RELAY = (
        'elif [[ "$(grep -c -x -- "yes $relay_snap # S272 smoke relay" '
        '"$relay_dir/relay_calls")" != 4 ]] \\\n'
    )

    def comments_of(self, text):
        comments, _ = cb.lex_shell(text)
        return [text[c.start_off : c.end_off] for c in comments]

    def test_the_relay_line_holds_no_comment(self):
        self.assertEqual(self.comments_of("if true; then :\n" + self.RELAY + "  || x; then :; fi\n"), [])

    def test_the_relay_line_in_the_file_it_comes_from(self):
        text = pinned("tests/test_fastchess_script.sh")
        if text is None:
            self.skipTest(f"{PILOT_COMMIT} is not in this clone")
        number = text[: text.index('relay_snap # S272 smoke relay"')].count("\n") + 1
        comments, code = cb.lex_shell(text)
        self.assertIn(number, code)
        self.assertEqual([c.start_line for c in comments if c.start_line == number], [])

    def test_nesting_forms(self):
        cases = [
            ('echo "${x:-"# no"}" # one\n', ["# one"]),
            ('echo "`echo "# no"`" # one\n', ["# one"]),
            ('echo "$(echo "$(echo "# no")") " # one\n', ["# one"]),
            ('echo "$(echo \'a"b # no\')" # one\n', ["# one"]),
            ('echo "$(( 16#ff ))" # one\n', ["# one"]),
            ('echo "$(echo a # it\'s ) not the end\n  b)" # one\n', ["# one"]),
            ('echo "plain \\" # no" # one\n', ["# one"]),
            ('x=$(echo "# no" # one\n)\n', ["# one"]),
        ]
        for text, want in cases:
            with self.subTest(text=text):
                self.assertEqual(self.comments_of(text), want)

    def test_every_live_shell_file_parses_the_same_stripped(self):
        paths = subprocess.run(
            ["git", "-C", ROOT, "ls-files", "*.sh"], capture_output=True, text=True, env=clean_env()
        ).stdout.split()
        live = [p for p in paths if cb.is_live(p)]
        self.assertGreater(len(live), 0)
        for path in live:
            with self.subTest(path=path):
                text = read_text(os.path.join(ROOT, path))
                status, printed = cb.shell_print(text)
                self.assertEqual(status, 0)
                self.assertEqual(cb.shell_print(stripped(path, text)), (0, printed))


class LogicalLines(unittest.TestCase):
    def test_backslash_only_line_inside_a_continuation_is_nothing(self):
        self.assertEqual(cb.logical_lines("a \\\n   \\\nb\n"), cb.logical_lines("a \\\nb\n"))

    def test_blank_lines_do_not_count(self):
        self.assertEqual(cb.logical_lines("a\n\n\nb\n"), cb.logical_lines("a\nb\n"))

    def test_a_blank_line_after_a_backslash_ends_the_macro(self):
        self.assertNotEqual(cb.logical_lines("a \\\n\nb\n"), cb.logical_lines("a \\\nb\n"))

    def test_whitespace_only_lines_read_as_blank(self):
        self.assertEqual(cb.logical_lines("a\n     \n\t\nb\n"), cb.logical_lines("a\nb\n"))

    def test_a_backslash_only_line_reads_the_same_at_any_column(self):
        narrow = "a \\\n" + " " * 38 + "\\\nb\n"
        wide = "a \\\n" + " " * 40 + "\\\nb\n"
        self.assertEqual(cb.logical_lines(narrow), cb.logical_lines(wide))

    def test_a_dropped_backslash_on_a_code_line_shows(self):
        self.assertNotEqual(cb.logical_lines("a \\\nb\n"), cb.logical_lines("a\nb\n"))


class FixtureRepo(unittest.TestCase):
    """A git repository in a temporary directory, the tool pointed at it with --repo."""

    def setUp(self):
        self.dir = tempfile.mkdtemp(prefix="comment_budget_test")
        self.addCleanup(shutil.rmtree, self.dir, ignore_errors=True)
        self.scratch = tempfile.mkdtemp(prefix="comment_budget_scratch")
        self.addCleanup(shutil.rmtree, self.scratch, ignore_errors=True)
        self.git("init", "-q", "--template=")
        self.git("config", "core.fsync", "none")

    def git(self, *args):
        proc = subprocess.run(
            ["git", "-c", "user.name=t", "-c", "user.email=t@t", "-c", "commit.gpgsign=false", "-C", self.dir]
            + list(args),
            capture_output=True,
            text=True,
            env=clean_env(),
        )
        self.assertEqual(proc.returncode, 0, proc.stderr)
        return proc.stdout

    def write(self, path, text):
        full = os.path.join(self.dir, path)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, "wb") as handle:
            handle.write(text.encode("utf-8", "surrogateescape"))

    def read(self, path):
        with open(os.path.join(self.dir, path), encoding="utf-8") as handle:
            return handle.read()

    def commit_all(self):
        self.git("add", "-A")
        self.git("commit", "-q", "--allow-empty", "-m", "fixture")

    def run_tool(self, *args, env=None):
        proc = subprocess.run(
            [sys.executable, TOOL, "--repo", self.dir, *args],
            capture_output=True,
            text=True,
            env=env or clean_env(),
        )
        return proc.returncode, proc.stdout, proc.stderr

    def assert_marked(self, out, marker):
        self.assertEqual(out.strip().split("\n")[-1].split()[0], marker, out)


CPP_FILE = """// clang-format off
int a = 1; // about a
// clang-format on
#if 0
int z = 3;
#endif
int f(); // NOLINT(foo)
"""
PY_FILE = '''#!/usr/bin/env python3
"""Module doc."""
# one
x = 1  # noqa: E402


def f():
    """Function doc."""
    return x
'''
SH_FILE = """#!/bin/bash
# one
# shellcheck disable=SC2086
echo a # two
"""
CMAKE_FILE = """# one
set(A 1) # two
"""
DOC_READER = '''"""Read at runtime."""
print(__doc__)
'''


class Identical(FixtureRepo):
    def setUp(self):
        super().setUp()
        self.write("src/a.cpp", CPP_FILE)
        self.write("tools/t.py", PY_FILE)
        self.write("tools/s.sh", SH_FILE)
        self.write("CMakeLists.txt", CMAKE_FILE)
        self.write("tools/doc_reader.py", DOC_READER)
        self.write("notes.md", "words\n")
        self.commit_all()

    def identical(self, *extra, env=None):
        return self.run_tool("--identical", "HEAD", *extra, env=env)

    def test_a_clean_tree_is_green_with_nothing_read(self):
        status, out, _ = self.identical()
        self.assertEqual(status, 0, out)
        self.assertIn("0 proved", out)
        self.assert_marked(out, DONE)

    def test_comment_edits_are_green_in_every_language(self):
        self.write("src/a.cpp", CPP_FILE.replace("about a", "about the a"))
        self.write("tools/t.py", PY_FILE.replace("# one", "# one, reworded").replace("Function doc.", "Other doc."))
        self.write("tools/s.sh", SH_FILE.replace("# two", "# reworded"))
        self.write("CMakeLists.txt", CMAKE_FILE.replace("# one", "# other"))
        status, out, _ = self.identical()
        self.assertEqual(status, 0, out)
        for path in ("src/a.cpp", "tools/t.py", "tools/s.sh", "CMakeLists.txt"):
            self.assertIn("SAME " + path, out)
        self.assertRegex(out, r"SAME src/a\.cpp .*(raw-tokens|g\+\+-text)")
        self.assertIn("SAME tools/t.py lexer,directives,ast,tokens", out)
        self.assertIn("SAME tools/s.sh lexer,directives,declare-f", out)
        self.assertIn("SAME CMakeLists.txt lexer,directives\n", out)

    def test_an_injected_code_change_is_red_in_every_language(self):
        cases = [
            ("src/a.cpp", CPP_FILE.replace("int a = 1;", "int a = 2;"), ["lexer"]),
            ("tools/t.py", PY_FILE.replace("x = 1", "x = 2"), ["lexer", "ast", "tokens"]),
            ("tools/s.sh", SH_FILE.replace("echo a", "echo b"), ["lexer", "declare-f"]),
            ("CMakeLists.txt", CMAKE_FILE.replace("set(A 1)", "set(A 2)"), ["lexer"]),
        ]
        for path, text, arms in cases:
            with self.subTest(path=path):
                original = self.read(path)
                self.write(path, text)
                status, out, _ = self.identical()
                self.assertEqual(status, 1, out)
                for arm in arms:
                    self.assertIn(f"DIFF {path} {arm}:", out)
                self.assert_marked(out, FAILED)
                self.write(path, original)

    def test_a_change_in_an_inactive_branch_is_red_in_both_cpp_arms(self):
        self.write("src/a.cpp", CPP_FILE.replace("int z = 3;", "int z = 4;"))
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF src/a.cpp lexer:", out)
        self.assertRegex(out, r"DIFF src/a\.cpp (raw-tokens|g\+\+-text):")

    def test_the_compiler_arm_alone_sees_an_inactive_branch_and_not_a_comment(self):
        compiler = cb.Compiler()
        base = compiler.view("a.cpp", CPP_FILE)
        self.assertNotEqual(base, compiler.view("a.cpp", CPP_FILE.replace("int z = 3;", "int z = 4;")))
        self.assertEqual(base, compiler.view("a.cpp", CPP_FILE.replace("about a", "about the a")))

    def test_a_dropped_directive_is_red_with_the_code_unchanged(self):
        cases = [
            ("src/a.cpp", CPP_FILE.replace("// clang-format off\n", "")),
            ("src/a.cpp", CPP_FILE.replace(" // NOLINT(foo)", "")),
            ("tools/t.py", PY_FILE.replace("  # noqa: E402", "")),
            ("tools/s.sh", SH_FILE.replace("# shellcheck disable=SC2086\n", "")),
        ]
        for path, text in cases:
            with self.subTest(path=path, text=text):
                original = self.read(path)
                self.write(path, text)
                status, out, _ = self.identical()
                self.assertEqual(status, 1, out)
                self.assertIn(f"DIFF {path} directives: lost", out)
                self.assertEqual([l for l in out.split("\n") if l.startswith("DIFF") and " directives:" not in l], [])
                self.write(path, original)

    def test_prose_that_names_a_directive_is_no_directive(self):
        prose = "// the clang-format off above is prose here\n"
        self.write("src/p.cpp", prose + "int p;\n")
        self.commit_all()
        self.write("src/p.cpp", "int p;\n")
        status, out, _ = self.identical()
        self.assertEqual(status, 0, out)

    def test_a_docstring_is_comment_unless_the_file_reads_doc(self):
        self.write("tools/t.py", PY_FILE.replace('    """Function doc."""\n', ""))
        status, out, _ = self.identical()
        self.assertEqual(status, 0, out)
        self.write("tools/doc_reader.py", DOC_READER.replace("Read at runtime.", "Read later."))
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        for arm in ("lexer", "ast", "tokens"):
            self.assertIn(f"DIFF tools/doc_reader.py {arm}:", out)

    def test_a_cpp_line_comment_ending_in_a_backslash_is_red_with_the_rest_green(self):
        self.write("src/a.cpp", CPP_FILE.replace("// about a", "// about a \\\n// and more"))
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF src/a.cpp wcomment:", out)
        self.assertEqual([l for l in out.split("\n") if l.startswith("DIFF") and " wcomment:" not in l], [])

    def test_the_relay_line_changing_its_count_is_red(self):
        text = pinned("tests/test_fastchess_script.sh")
        if text is None:
            self.skipTest(f"{PILOT_COMMIT} is not in this clone")
        needle = '"$relay_dir/relay_calls")" != 4 ]]'
        self.assertIn(needle, text)
        self.write("tests/test_fastchess_script.sh", text)
        self.commit_all()
        self.write("tests/test_fastchess_script.sh", text.replace(needle, needle.replace("4", "5"), 1))
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF tests/test_fastchess_script.sh lexer:", out)
        self.assertIn("DIFF tests/test_fastchess_script.sh declare-f:", out)

    def test_changes_no_lexer_can_prove_are_red_and_documents_are_not(self):
        self.write("notes.md", "other words\n")
        self.write("adocs/data/new_evidence.py", "x = 1\n")
        self.write("adocs/data/table.tsv", "a\n")
        status, out, _ = self.identical()
        self.assertEqual(status, 0, out)
        self.assertIn("NOT-CODE notes.md", out)
        self.assertIn("NOT-CODE adocs/data/new_evidence.py", out)
        self.assertIn("NOT-CODE adocs/data/table.tsv", out)
        self.write("tools/config.json", "{}\n")
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF tools/config.json no lexer", out)

    def test_an_added_a_deleted_and_a_chmod_file_are_red(self):
        self.write("src/new.cpp", "int n;\n")
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF src/new.cpp added", out)
        os.remove(os.path.join(self.dir, "src/new.cpp"))
        os.remove(os.path.join(self.dir, "tools/s.sh"))
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF tools/s.sh deleted", out)
        self.git("checkout", "--", "tools/s.sh")
        full = os.path.join(self.dir, "tools/t.py")
        os.chmod(full, os.stat(full).st_mode | stat.S_IXUSR)
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF tools/t.py mode:", out)

    def test_exempt_is_named_and_never_reaches_src(self):
        self.write("tools/new.py", "x = 1\n")
        status, out, _ = self.identical()
        self.assertEqual(status, 1, out)
        status, out, _ = self.identical("--exempt", "tools/new.py")
        self.assertEqual(status, 0, out)
        self.assertIn("EXEMPT tools/new.py", out)
        self.assertIn("1 exempt", out)
        status, out, err = self.identical("--exempt", "src/a.cpp")
        self.assertEqual(status, 2, out + err)
        self.assert_marked(out, FAILED)

    def test_the_g_plus_plus_arm_runs_the_command_it_names(self):
        stub = os.path.join(self.scratch, "fake_gpp")
        log = os.path.join(self.scratch, "fake_gpp.log")
        with open(stub, "w") as handle:
            handle.write(
                "#!/bin/sh\n"
                'if [ "$1" = --version ]; then\n'
                '  echo "g++ (GCC) 13.3.0"; echo "Free Software Foundation, Inc."; exit 0\n'
                "fi\n"
                f'echo "$@" >> "{log}"\n'
                'for last; do :; done\n'
                "sed -e 's#//.*##' \"$last\"\n"
            )
        os.chmod(stub, 0o755)
        self.write("src/a.cpp", CPP_FILE.replace("about a", "about the a"))
        env = clean_env(CXX=stub)
        status, out, _ = self.identical(env=env)
        self.assertEqual(status, 0, out)
        self.assertIn("SAME src/a.cpp lexer,directives,g++-text,wcomment", out)
        with open(log) as handle:
            self.assertIn("-fpreprocessed -dD -E -P -x c++ ", handle.read())
        self.write("src/a.cpp", CPP_FILE.replace("int z = 3;", "int z = 4;"))
        status, out, _ = self.identical(env=env)
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF src/a.cpp g++-text:", out)

    def test_the_g_plus_plus_arm_reads_a_backslash_line_as_space(self):
        stub = os.path.join(self.scratch, "fake_gpp_macro")
        with open(stub, "w") as handle:
            handle.write(
                "#!/bin/sh\n"
                'if [ "$1" = --version ]; then echo "g++ (GCC) 13.3.0"; exit 0; fi\n'
                "for last; do :; done\n"
                "sed -e 's#/[*].*[*]/##' -e 's#//.*##' \"$last\" | tr '\\n' ' '\n"
            )
        os.chmod(stub, 0o755)
        macro = "#define M 1 \\\n  /* block */ \\\n  + 2\n"
        self.write("src/m.hpp", macro)
        self.commit_all()
        self.write("src/m.hpp", "#define M 1 \\\n  + 2\n")
        env = clean_env(CXX=stub)
        status, out, _ = self.identical(env=env)
        self.assertEqual(status, 0, out)
        self.assertIn("SAME src/m.hpp lexer,directives,g++-text,wcomment", out)
        self.write("src/m.hpp", "#define M 1 \\\n  + 3\n")
        status, out, _ = self.identical(env=env)
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF src/m.hpp g++-text:", out)

    def test_nothing_to_compare_proves_nothing(self):
        empty = os.path.join(self.scratch, "empty")
        os.makedirs(empty)
        status, out, _ = self.identical("--against", empty)
        self.assertEqual(status, 2, out)
        self.assert_marked(out, FAILED)

    def test_a_compiler_that_exits_non_zero_is_no_agreement(self):
        stub = os.path.join(self.scratch, "fake_broken_clang")
        with open(stub, "w") as handle:
            handle.write(
                '#!/bin/sh\nif [ "$1" = --version ]; then echo "Apple clang version 99"; exit 0; fi\n'
                'echo "clang: error: unknown argument: -dump-raw-tokens" >&2\nexit 1\n'
            )
        os.chmod(stub, 0o755)
        self.write("src/a.cpp", CPP_FILE.replace("about a", "about the a"))
        status, out, err = self.identical(env=clean_env(CXX=stub))
        self.assertEqual(status, 2, out + err)
        self.assertNotIn("SAME src/a.cpp", out)
        self.assertIn("exited 1", out + err)
        self.assert_marked(out, FAILED)

    def test_a_dump_record_nobody_can_read_is_no_agreement(self):
        stub = os.path.join(self.scratch, "fake_odd_clang")
        with open(stub, "w") as handle:
            handle.write(
                '#!/bin/sh\nif [ "$1" = --version ]; then echo "clang version 99"; exit 0; fi\n'
                "printf 'some other format\\tLoc=<a:1:1>\\n' >&2\n"
            )
        os.chmod(stub, 0o755)
        self.write("src/a.cpp", CPP_FILE.replace("about a", "about the a"))
        status, out, err = self.identical(env=clean_env(CXX=stub))
        self.assertEqual(status, 2, out + err)
        self.assertNotIn("SAME src/a.cpp", out)

    def test_a_file_holding_only_comments_can_be_proved(self):
        self.write("src/only.hpp", "// one\n")
        self.commit_all()
        self.write("src/only.hpp", "// two\n")
        status, out, err = self.identical()
        self.assertEqual(status, 0, out + err)
        self.assertIn("SAME src/only.hpp", out)

    def test_a_line_comment_ending_in_backslash_then_space_takes_the_next_line(self):
        self.write("src/y.cpp", "// c\nint y;\n")
        self.commit_all()
        self.write("src/y.cpp", "// c \\ \nint y;\n")
        status, out, err = self.identical()
        self.assertEqual(status, 1, out + err)
        self.assertIn("DIFF src/y.cpp wcomment:", out)
        self.assertIn("DIFF src/y.cpp lexer:", out)

    def against(self, *files, stray=()):
        directory = os.path.join(self.scratch, "copies")
        os.makedirs(directory, exist_ok=True)
        for name, text in files:
            with open(os.path.join(directory, name), "w") as handle:
                handle.write(text)
        extra = [arg for name in stray for arg in ("--stray", name)]
        return self.identical("--against", directory, *extra)

    def test_a_copy_with_no_counterpart_is_red_unless_listed(self):
        files = (("a.cpp", CPP_FILE.replace("about a", "about the a")), ("b_new.cpp", "int CHANGED;\n"))
        status, out, _ = self.against(*files)
        self.assertEqual(status, 1, out)
        self.assertIn("NO-COUNTERPART b_new.cpp", out)
        self.assertIn("SAME src/a.cpp", out)
        self.assert_marked(out, FAILED)
        status, out, _ = self.against(*files, stray=("b_new.cpp",))
        self.assertEqual(status, 0, out)
        self.assertIn("STRAY b_new.cpp", out)

    def test_a_listed_stray_that_is_not_one_is_a_tool_error(self):
        status, out, _ = self.against(("a.cpp", CPP_FILE), stray=("a.cpp",))
        self.assertEqual(status, 2, out)
        status, out, _ = self.against(("a.cpp", CPP_FILE), stray=("nothing.cpp",))
        self.assertEqual(status, 2, out)

    def test_a_changed_copy_with_a_counterpart_and_no_lexer_is_not_skipped(self):
        self.write("tools/config.json", "{}\n")
        self.commit_all()
        status, out, _ = self.against(("config.json", '{"a": 1}\n'))
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF tools/config.json no lexer", out)

    def test_no_compiler_and_an_empty_dump_prove_nothing(self):
        self.write("src/a.cpp", CPP_FILE.replace("about a", "about the a"))
        status, out, _ = self.identical(env=clean_env(CXX="/nonexistent/c++"))
        self.assertEqual(status, 2, out)
        self.assert_marked(out, FAILED)
        stub = os.path.join(self.scratch, "fake_clang")
        with open(stub, "w") as handle:
            handle.write('#!/bin/sh\nif [ "$1" = --version ]; then echo clang version 99; fi\nexit 0\n')
        os.chmod(stub, 0o755)
        status, out, _ = self.identical(env=clean_env(CXX=stub))
        self.assertEqual(status, 2, out)
        self.assert_marked(out, FAILED)

    def test_every_exit_path_ends_in_a_marker(self):
        cases = [
            (2, FAILED, ("--identical", "no-such-ref")),
            (2, FAILED, ("--tiers", "no-such-ref")),
            (2, FAILED, ("--apply", "/nonexistent.json", "--out", self.dir)),
            (2, FAILED, ("--report", "HEAD", "--tree")),
            (2, FAILED, ()),
            (0, DONE, ("--help",)),
        ]
        for want, marker, args in cases:
            with self.subTest(args=args):
                status, out, err = self.run_tool(*args)
                self.assertEqual(status, want, out + err)
                self.assert_marked(out, marker)


GPP_MODEL = """#!/usr/bin/env python3
# A model of g++ -fpreprocessed -dD -E -P: a comment becomes one space, nothing else moves.
import sys
if sys.argv[1] == "--version":
    print("g++ (GCC) 13.3.0")
    sys.exit(0)
sys.dont_write_bytecode = True
sys.path.insert(0, {tools!r})
import comment_budget as cb
source = open(sys.argv[-1], encoding="utf-8", errors="surrogateescape").read()
comments, _ = cb.lex_cpp(source)
out, pos = [], 0
for c in comments:
    out.append(source[pos:c.start_off])
    out.append(" ")
    pos = c.end_off
out.append(source[pos:])
sys.stdout.write("".join(out))
"""


class GppArmOnAModel(FixtureRepo):
    """The g++ arm over a model of its output, since no g++ runs on the MacBook.

    Real g++ is read at the evening's gate. There the default compiler is g++, so
    `Apply.test_a_whole_block_leaves_the_real_macro_and_identity_stays_green` runs the
    arm for real: a red of it on the workstation means g++ prints the macro otherwise
    than this model does, and `_gpp_text` is what to read, not the edit."""

    def setUp(self):
        super().setUp()
        self.stub = os.path.join(self.scratch, "fake_gpp_model")
        with open(self.stub, "w") as handle:
            handle.write(GPP_MODEL.format(tools=os.path.dirname(TOOL)))
        os.chmod(self.stub, 0o755)
        self.env = clean_env(CXX=self.stub)

    def prove(self, old, new, path="src/x.cpp"):
        self.write(path, old)
        self.commit_all()
        self.write(path, new)
        return self.run_tool("--identical", "HEAD", env=self.env)

    def assert_gpp_red(self, old, new):
        status, out, err = self.prove(old, new)
        self.assertEqual(status, 1, out + err)
        self.assertIn("DIFF src/x.cpp g++-text:", out)

    def test_a_comment_that_separated_two_tokens_merging_them_is_red(self):
        self.assert_gpp_red("int /* c */x;\n", "intx;\n")

    def test_a_comment_with_no_space_either_side_merging_them_is_red(self):
        self.assert_gpp_red("int/**/x;\n", "intx;\n")

    def test_a_comment_inserted_inside_a_token_is_red(self):
        self.assert_gpp_red("int x;\n", "in/*c*/t x;\n")

    def test_a_continuation_pasting_two_tokens_is_red(self):
        self.assert_gpp_red("#define M foo \\\nbar\n", "#define M foo\\\nbar\n")

    def test_a_string_continuation_losing_its_spaces_is_red(self):
        self.assert_gpp_red('const char* s = "ab  \\\ncd";\n', 'const char* s = "ab\\\ncd";\n')

    def test_comment_edits_and_whole_comment_lines_stay_green(self):
        status, out, err = self.prove("// one\nint x; // two\n\n/* three */\nint y;\n", "int x; // other\nint y;\n")
        self.assertEqual(status, 0, out + err)
        self.assertIn("SAME src/x.cpp lexer,directives,g++-text,wcomment", out)

    def test_the_real_macro_stays_green_with_a_block_gone_and_red_on_a_real_change(self):
        path = "src/search_params.hpp"
        text = pinned(path)
        if text is None:
            self.skipTest(f"{PILOT_COMMIT} is not in this clone")
        comments = cb.editable_comments(path, text)
        format_off = cb.format_off_lines(text, comments)
        block = next(
            c
            for c in comments
            if c.kind == "block"
            and c.start_line in format_off
            and text[c.end_off : text.index("\n", c.end_off)].strip() == "\\"
        )
        lines = text.split("\n")
        whole = "\n".join(lines[: block.start_line - 1] + lines[block.end_line :])
        for name, new in (("by lines", whole), ("by range", text[: block.start_off] + text[block.end_off :])):
            with self.subTest(name=name):
                status, out, err = self.prove(text, new, path)
                self.assertEqual(status, 0, out + err)
                self.assertIn("g++-text", out)
        row = next(i for i, line in enumerate(lines) if line.lstrip().startswith("X(") and line.rstrip().endswith("\\"))
        moved = list(lines)
        moved[row] = re.sub(r"\d+", lambda m: str(int(m.group(0)) + 1), moved[row], count=1)
        status, out, err = self.prove(text, "\n".join(moved), path)
        self.assertEqual(status, 1, out + err)
        self.assertIn(f"DIFF {path} g++-text:", out)

    def test_an_empty_range_cannot_split_a_token_under_either_compiler(self):
        self.write("src/x.cpp", "int x;\n")
        self.commit_all()
        edits = os.path.join(self.scratch, "split.json")
        with open(edits, "w") as handle:
            json.dump(
                {"ref": "HEAD", "edits": [{"file": "src/x.cpp", "start": 2, "end": 2, "old": "", "text": "/*c*/"}]}, handle
            )
        for env in (self.env, clean_env()):
            status, out, err = self.run_tool("--apply", edits, "--tree", env=env)
            self.assertEqual(status, 1, out + err)
            self.assertIn("REFUSED", out)
            self.assertEqual(self.read("src/x.cpp"), "int x;\n")


class OnTheRealTree(unittest.TestCase):
    def run_real(self, *args):
        proc = subprocess.run([sys.executable, TOOL, *args], capture_output=True, text=True, env=clean_env())
        return proc.returncode, proc.stdout

    def test_report_and_tiers_run_and_report_never_fails(self):
        for args in (("--report", "HEAD"), ("--report", "HEAD", "--new-only"), ("--tiers", "HEAD")):
            with self.subTest(args=args):
                status, out = self.run_real(*args)
                self.assertEqual(status, 0, out)
                self.assertEqual(out.strip().split("\n")[-1].split()[0], DONE)
                self.assertNotIn("SKIPPED", out)

    def test_the_pilot_is_green(self):
        probe = subprocess.run(
            ["git", "-C", ROOT, "cat-file", "-e", PILOT_COMMIT + "^{commit}"], capture_output=True, env=clean_env()
        )
        if probe.returncode != 0:
            self.skipTest(f"{PILOT_COMMIT} is not in this clone")
        strays = ("apply_pilot.py", "lost.py", "measure_pilot.py")
        status, out = self.run_real("--identical", PILOT_COMMIT, "--against", PILOT)
        self.assertEqual(status, 1, out)
        for stray in strays:
            self.assertIn("NO-COUNTERPART " + stray, out)
        listed = [arg for stray in strays for arg in ("--stray", stray)]
        status, out = self.run_real("--identical", PILOT_COMMIT, "--against", PILOT, *listed)
        self.assertEqual(status, 0, out)
        self.assertIn("SAME src/search.cpp", out)
        self.assertIn("SAME tests/test_mate_carry.cpp", out)
        self.assertIn("2 proved", out)


MACRO = """// clang-format off
#define PARAMS(X)            \\
  /* first block comment     \\
     second line of it */    \\
  X(ONE, 1)                  \\
  /* another block */        \\
  X(TWO, 2)
// clang-format on
int after = 0;
"""
MACRO_NO_FORMAT_OFF = MACRO.replace("// clang-format off\n", "").replace("// clang-format on\n", "")


class ApplyFixture(FixtureRepo):
    def edit_list(self, edits, ref="HEAD"):
        path = os.path.join(self.scratch, "edits.json")
        with open(path, "w") as handle:
            json.dump({"ref": ref, "edits": edits}, handle)
        return path

    def with_old(self, edits, seeds=()):
        """Each edit with the `old` its writer would copy from its own view of the file.

        Cases about something else carry a correct one; `old` has its own class."""
        out = []
        for edit in edits:
            edit = dict(edit)
            if "old" not in edit:
                text = self.git("show", "HEAD:" + edit["file"])
                for seed in seeds:
                    same_size = seed["end"] - seed["start"] == len(seed.get("text", ""))
                    if seed["file"] == edit["file"] and same_size and seed["end"] <= len(text):
                        text = text[: seed["start"]] + seed["text"] + text[seed["end"] :]
                if "lines" in edit:
                    pieces = text.split("\n")
                    first, last = edit["lines"]
                    body = pieces[first - 1 : last]
                    edit["old"] = "\n".join(body) + ("\n" if last < len(pieces) else "")
                else:
                    edit["old"] = text[edit["start"] : edit["end"]]
            out.append(edit)
        return out

    def apply(self, edits, *where, seeds=()):
        return self.run_tool("--apply", self.edit_list(self.with_old(edits, seeds)), *(where or ("--tree",)))

    def base(self, path, text):
        self.write(path, text)
        self.commit_all()

    def assert_identical(self):
        status, out, err = self.run_tool("--identical", "HEAD")
        self.assertEqual(status, 0, out + err)

    def assert_refused(self, status, out, words):
        self.assertEqual(status, 1, out)
        self.assertIn("REFUSED", out)
        self.assertIn(words, out)
        self.assertIn("nothing written", out)
        self.assert_marked(out, FAILED)


class ApplyChecksWhatItCuts(ApplyFixture):
    TEXT = "// caf\u00e9 \u2192 sentence one. sentence two.\nint a;\n"

    def setUp(self):
        super().setUp()
        self.base("src/a.cpp", self.TEXT)

    def run_list(self, edits):
        return self.run_tool("--apply", self.edit_list(edits), "--tree")

    def test_an_edit_with_no_old_is_refused(self):
        start = self.TEXT.index("sentence two.")
        for edit in (
            {"file": "src/a.cpp", "start": start, "end": start + 13},
            {"file": "src/a.cpp", "lines": [1, 1]},
        ):
            with self.subTest(edit=edit):
                status, out, _ = self.run_list([edit])
                self.assert_refused(status, out, "no `old`")
                self.assertEqual(self.read("src/a.cpp"), self.TEXT)

    def test_byte_offsets_on_a_line_with_non_ascii_are_refused(self):
        raw = self.TEXT.encode("utf-8")
        start = raw.index(b"sentence two.")
        end = start + len(b"sentence two.")
        self.assertNotEqual(self.TEXT[start:end], "sentence two.", "the fixture's line holds no multi-byte character")
        status, out, _ = self.run_list([{"file": "src/a.cpp", "start": start, "end": end, "old": "sentence two."}])
        self.assert_refused(status, out, "does not match")
        self.assertEqual(self.read("src/a.cpp"), self.TEXT)

    def test_a_range_drifted_by_one_is_refused_though_all_comment(self):
        start = self.TEXT.index("sentence two.")
        status, out, _ = self.run_list([{"file": "src/a.cpp", "start": start + 1, "end": start + 14, "old": "sentence two."}])
        self.assert_refused(status, out, "does not match")
        self.assertEqual(self.read("src/a.cpp"), self.TEXT)

    def test_the_right_old_applies(self):
        start = self.TEXT.index(" sentence two.")
        status, out, _ = self.run_list([{"file": "src/a.cpp", "start": start, "end": start + 14, "old": " sentence two."}])
        self.assertEqual(status, 0, out)
        self.assertEqual(self.read("src/a.cpp"), self.TEXT.replace(" sentence two.", ""))

    def test_a_lines_edit_compares_the_whole_lines_with_their_newline(self):
        line = self.TEXT[: self.TEXT.index("\n") + 1]
        status, out, _ = self.run_list([{"file": "src/a.cpp", "lines": [1, 1], "old": line.rstrip("\n")}])
        self.assert_refused(status, out, "does not match")
        status, out, _ = self.run_list([{"file": "src/a.cpp", "lines": [1, 1], "old": line}])
        self.assertEqual(status, 0, out)
        self.assertEqual(self.read("src/a.cpp"), "int a;\n")

    def test_one_bad_old_writes_nothing_anywhere(self):
        self.base("src/b.cpp", "// two\nint b;\n")
        status, out, _ = self.run_list(
            [
                {"file": "src/b.cpp", "lines": [1, 1], "old": "// two\n"},
                {"file": "src/a.cpp", "lines": [1, 1], "old": "// not this\n"},
            ]
        )
        self.assert_refused(status, out, "does not match")
        self.assertEqual(self.read("src/b.cpp"), "// two\nint b;\n")


class Apply(ApplyFixture):
    def test_a_character_range_over_a_trailing_comment(self):
        text = "int a = 1;   // about a\nint b;\n"
        self.base("src/a.cpp", text)
        start = text.index("   //")
        end = text.index("\n")
        status, out, _ = self.apply([{"file": "src/a.cpp", "start": start, "end": end}])
        self.assertEqual(status, 0, out)
        self.assertEqual(self.read("src/a.cpp"), "int a = 1;\nint b;\n")
        self.assert_identical()

    def test_a_character_range_replacing_a_trailing_comment(self):
        text = "int a = 1;   // about a\nint b;\n"
        self.base("src/a.cpp", text)
        start = text.index("// about")
        end = text.index("\n")
        status, out, _ = self.apply([{"file": "src/a.cpp", "start": start, "end": end, "text": "// the a"}])
        self.assertEqual(status, 0, out)
        self.assertEqual(self.read("src/a.cpp"), "int a = 1;   // the a\nint b;\n")
        self.assert_identical()

    def test_a_trailing_comment_in_the_real_source(self):
        path = "src/bitboard.cpp"
        text = pinned(path)
        if text is None:
            self.skipTest(f"{PILOT_COMMIT} is not in this clone")
        comments, _ = cb.lex_cpp(text)
        trailing = [c for c in comments if c.trailing and text[c.start_off : c.end_off].startswith("//")]
        self.assertTrue(trailing, "no trailing // comment in the fixture, so this case tests nothing")
        comment = trailing[0]
        self.base(path, text)
        status, out, _ = self.apply([{"file": path, "start": comment.start_off, "end": comment.end_off}])
        self.assertEqual(status, 0, out)
        self.assert_identical()
        self.git("checkout", "--", path)
        # One character further left is the code the comment trails, once any space is passed.
        left = comment.start_off
        while text[left - 1].isspace():
            left -= 1
        status, out, _ = self.apply([{"file": path, "start": left - 1, "end": comment.end_off}])
        self.assert_refused(status, out, "touches a code token")
        self.assertEqual(self.read(path), text)

    def test_a_range_touching_code_is_refused(self):
        text = "int a = 1; // about a\n"
        self.base("src/a.cpp", text)
        status, out, _ = self.apply([{"file": "src/a.cpp", "start": 0, "end": 3}])
        self.assert_refused(status, out, "touches a code token at line 1")
        self.assertEqual(self.read("src/a.cpp"), text)

    def test_a_lines_edit_with_code_on_a_line_is_refused(self):
        text = "// one\nint a = 1; // about a\n"
        self.base("src/a.cpp", text)
        status, out, _ = self.apply([{"file": "src/a.cpp", "lines": [1, 2]}])
        self.assert_refused(status, out, "line 2 holds code")

    def test_overlap_is_refused_and_adjacent_is_not(self):
        text = "// one two three\nint a;\n"
        self.base("src/a.cpp", text)
        status, out, _ = self.apply(
            [{"file": "src/a.cpp", "start": 3, "end": 8}, {"file": "src/a.cpp", "start": 7, "end": 12}]
        )
        self.assert_refused(status, out, "overlap")
        status, out, _ = self.apply(
            [{"file": "src/a.cpp", "start": 3, "end": 7}, {"file": "src/a.cpp", "start": 7, "end": 11}]
        )
        self.assertEqual(status, 0, out)
        self.assertEqual(self.read("src/a.cpp"), "// three\nint a;\n")

    def test_new_code_through_the_replacement_text_is_refused(self):
        text = "// one\nint a;\n"
        self.base("src/a.cpp", text)
        status, out, _ = self.apply([{"file": "src/a.cpp", "start": 0, "end": 6, "text": "// one\nint x;"}])
        self.assert_refused(status, out, "red on lexer")
        status, out, _ = self.apply([{"file": "src/a.cpp", "lines": [1, 1], "text": "int x;"}])
        self.assert_refused(status, out, "red on lexer")
        self.assertEqual(self.read("src/a.cpp"), text)

    def test_a_dropped_directive_a_wcomment_and_a_read_docstring_are_refused(self):
        self.base("src/a.cpp", "// clang-format off\nint a;\n// clang-format on\n// about\nint b;\n")
        status, out, _ = self.apply([{"file": "src/a.cpp", "lines": [1, 1]}])
        self.assert_refused(status, out, "red on directives")
        status, out, _ = self.apply([{"file": "src/a.cpp", "lines": [4, 4], "text": "// about \\\n// more"}])
        self.assert_refused(status, out, "red on wcomment")
        self.base("tools/d.py", DOC_READER)
        status, out, _ = self.apply([{"file": "tools/d.py", "lines": [1, 1]}])
        self.assert_refused(status, out, "line 1 holds code")
        self.base("tools/s.sh", "#!/bin/bash\necho a\n")
        status, out, _ = self.apply([{"file": "tools/s.sh", "lines": [1, 1]}])
        self.assert_refused(status, out, "line 1 holds code")

    def test_one_refusal_writes_nothing_anywhere(self):
        self.base("src/a.cpp", "// one\nint a;\n")
        self.base("src/b.cpp", "// two\nint b;\n")
        status, out, _ = self.apply([{"file": "src/a.cpp", "lines": [1, 1]}, {"file": "src/b.cpp", "lines": [2, 2]}])
        self.assert_refused(status, out, "src/b.cpp")
        self.assertEqual(self.read("src/a.cpp"), "// one\nint a;\n")

    def test_the_tree_refuses_a_file_that_moved_and_out_does_not(self):
        self.base("src/a.cpp", "// one\nint a;\n")
        self.write("src/a.cpp", "// edited\nint a;\n")
        status, out, _ = self.apply([{"file": "src/a.cpp", "lines": [1, 1]}])
        self.assert_refused(status, out, "working tree differs")
        out_dir = os.path.join(self.scratch, "out")
        status, out, _ = self.apply([{"file": "src/a.cpp", "lines": [1, 1]}], "--out", out_dir)
        self.assertEqual(status, 0, out)
        with open(os.path.join(out_dir, "src", "a.cpp")) as handle:
            self.assertEqual(handle.read(), "int a;\n")
        self.assertEqual(self.read("src/a.cpp"), "// edited\nint a;\n")

    def test_a_malformed_edit_list_is_no_refusal_but_a_tool_error(self):
        self.base("src/a.cpp", "// one\nint a;\n")
        bad = os.path.join(self.scratch, "bad.json")
        for body in ("not json", '{"edits": []}', '{"ref": "HEAD", "edits": [{"lines": [1, 1]}]}'):
            with self.subTest(body=body):
                with open(bad, "w") as handle:
                    handle.write(body)
                status, out, _ = self.run_tool("--apply", bad, "--tree")
                self.assertEqual(status, 2, out)
                self.assert_marked(out, FAILED)

    def test_evidence_and_files_with_no_lexer_take_no_edit(self):
        self.base("adocs/data/x.py", "# one\nx = 1\n")
        self.base("notes.md", "words\n")
        status, out, _ = self.apply([{"file": "adocs/data/x.py", "lines": [1, 1]}])
        self.assert_refused(status, out, "take no comment edit")
        status, out, _ = self.apply([{"file": "notes.md", "lines": [1, 1]}])
        self.assert_refused(status, out, "no lexer")

    def test_a_macro_block_goes_whole_with_its_backslash_inside_format_off(self):
        self.base("src/m.hpp", MACRO)
        status, out, _ = self.apply([{"file": "src/m.hpp", "lines": [3, 4]}, {"file": "src/m.hpp", "lines": [6, 6]}])
        self.assertEqual(status, 0, out)
        self.assertNotIn("block", self.read("src/m.hpp"))
        status, out, err = self.run_tool("--identical", "HEAD")
        self.assertEqual(status, 0, out + err)
        self.assertRegex(out, r"SAME src/m\.hpp lexer,directives,(raw-tokens|g\+\+-text),wcomment")

    def test_a_character_range_may_not_take_the_backslash(self):
        self.base("src/m.hpp", MACRO)
        start = MACRO.index("/* another")
        end = MACRO.index("X(TWO")
        status, out, _ = self.apply([{"file": "src/m.hpp", "start": start, "end": end}])
        self.assert_refused(status, out, "touches a code token")

    def test_a_character_range_may_leave_the_backslash(self):
        self.base("src/m.hpp", MACRO)
        start = MACRO.index("/* another")
        end = MACRO.index("*/", start) + 2
        status, out, _ = self.apply([{"file": "src/m.hpp", "start": start, "end": end}])
        self.assertEqual(status, 0, out)
        self.assert_identical()

    def test_outside_format_off_a_backslash_remainder_is_code(self):
        self.base("src/m.hpp", MACRO_NO_FORMAT_OFF)
        status, out, _ = self.apply([{"file": "src/m.hpp", "lines": [2, 3]}])
        self.assert_refused(status, out, "outside a format-off macro")

    def test_an_edit_that_moves_the_macro_extent_is_refused(self):
        # The comment after the last continuation is what ends the macro: with it
        # gone `int after` would be the macro's.
        text = "// clang-format off\n#define Y 1 \\\n// ends it\nint after;\n// clang-format on\n"
        self.base("src/y.hpp", text)
        status, out, _ = self.apply([{"file": "src/y.hpp", "lines": [3, 3]}])
        self.assert_refused(status, out, "red on lexer")
        self.assertEqual(self.read("src/y.hpp"), text)

    def test_a_comment_inserted_after_a_continuation_is_refused(self):
        # The other direction: the new comment line would end the macro early.
        text = "#define Y 1 \\\n  + 2\nint after;\n"
        self.base("src/y.hpp", text)
        at = text.index("  + 2")
        status, out, _ = self.apply([{"file": "src/y.hpp", "start": at, "end": at, "text": "// ends it\n"}])
        self.assert_refused(status, out, "red on lexer")

    def real_macro(self):
        path = "src/search_params.hpp"
        text = pinned(path)
        if text is None:
            self.skipTest(f"{PILOT_COMMIT} is not in this clone")
        comments = cb.editable_comments(path, text)
        format_off = cb.format_off_lines(text, comments)
        blocks = [
            c
            for c in comments
            if c.kind == "block"
            and c.start_line in format_off
            and text[c.end_off : text.index("\n", c.end_off)].strip() == "\\"
        ]
        self.assertGreater(len(blocks), 1, "the fixture holds no macro block ending in a backslash")
        self.base(path, text)
        return path, text, blocks

    def test_a_whole_block_leaves_the_real_macro_and_identity_stays_green(self):
        path, text, blocks = self.real_macro()
        first, second = blocks[0], blocks[1]
        status, out, _ = self.apply(
            [
                {"file": path, "lines": [first.start_line, first.end_line]},
                {"file": path, "start": second.start_off, "end": second.end_off},
            ]
        )
        self.assertEqual(status, 0, out)
        status, out, err = self.run_tool("--identical", "HEAD")
        self.assertEqual(status, 0, out + err)
        self.assertRegex(out, r"SAME src/search_params\.hpp lexer,directives,(raw-tokens|g\+\+-text),wcomment")
        new = self.read(path)
        # The lines form takes the block's lines; the range takes all but the one its `\\` stays on.
        lines_gone = (first.end_line - first.start_line + 1) + (second.end_line - second.start_line)
        self.assertEqual(new.count("\n"), text.count("\n") - lines_gone)
        compiler = cb.Compiler()
        self.assertEqual(compiler.view("x.hpp", text), compiler.view("x.hpp", new))

    def test_a_real_change_in_the_real_macro_stays_red(self):
        path, text, blocks = self.real_macro()
        lines = text.split("\n")
        row = next(
            i for i, line in enumerate(lines) if line.lstrip().startswith("X(") and line.rstrip().endswith("\\")
        )
        comment_end = blocks[0].end_line - 1
        changed_row = re.sub(r"\d+", lambda m: str(int(m.group(0)) + 1), lines[row], count=1)
        self.assertNotEqual(changed_row, lines[row])
        cases = [
            ("a default moved", row, changed_row, ["lexer", "compiler"]),
            ("a row gone", row, None, ["lexer", "compiler"]),
            ("a backslash gone from a row", row, lines[row].rstrip()[:-1].rstrip(), ["lexer"]),
            ("a backslash gone after a block", comment_end, lines[comment_end].rstrip()[:-1].rstrip(), ["lexer"]),
        ]
        for name, at, replacement, arms in cases:
            with self.subTest(name=name):
                edited = list(lines)
                if replacement is None:
                    del edited[at]
                else:
                    edited[at] = replacement
                self.write(path, "\n".join(edited))
                status, out, _ = self.run_tool("--identical", "HEAD")
                self.assertEqual(status, 1, out)
                self.assertIn(f"DIFF {path} lexer:", out)
                if "compiler" in arms:
                    self.assertRegex(out, r"DIFF src/search_params\.hpp (raw-tokens|g\+\+-text):")


SEEDED = "tests/test_mate_carry.cpp"
SEED_TRUE = "A 5 -> 6"
SEED_FALSE = "A 7 -> 6"


class ApplySeeds(ApplyFixture):
    """A seed is a known-false claim in the writer's scratch copy, never in the tree."""

    def setUp(self):
        super().setUp()
        self.text = pinned(SEEDED)
        if self.text is None:
            self.skipTest(f"{PILOT_COMMIT} is not in this clone")
        self.assertEqual(self.text.count(SEED_TRUE), 1)
        self.base(SEEDED, self.text)
        self.at = self.text.index(SEED_TRUE)
        self.line = self.text.count("\n", 0, self.at) + 1
        self.seed = {"id": "W1", "file": SEEDED, "start": self.at, "end": self.at + len(SEED_TRUE), "text": SEED_FALSE}

    def seed_list(self, seeds, ref="HEAD"):
        path = os.path.join(self.scratch, "seeds.json")
        with open(path, "w") as handle:
            json.dump({"ref": ref, "seeds": seeds}, handle)
        return path

    def apply_seeded(self, edits, seeds):
        return self.apply(edits, "--tree", "--seeds", self.seed_list(seeds), seeds=seeds)

    def later_line(self):
        """A whole comment line well past the seed, to delete beside it."""
        needle = "// futility table guard."
        self.assertEqual(self.text.count(needle), 1)
        return self.text.count("\n", 0, self.text.index(needle)) + 1

    def test_without_seeds_the_edit_takes_the_true_sentence(self):
        # The failure --seeds exists for: a caught seed, its true text gone, exit 0.
        status, out, _ = self.apply([{"file": SEEDED, "lines": [self.line, self.line]}])
        self.assertEqual(status, 0, out)
        self.assertNotIn(SEED_TRUE, self.read(SEEDED))

    def test_a_caught_seed_leaves_its_true_sentence_and_the_rest_applies(self):
        later = self.later_line()
        status, out, err = self.apply_seeded(
            [{"file": SEEDED, "lines": [self.line, self.line]}, {"file": SEEDED, "lines": [later, later]}],
            [self.seed],
        )
        self.assertEqual(status, 0, out + err)
        self.assertIn(f"DROPPED {SEEDED} edit 0 seed W1", out)
        self.assertNotIn("edit 1 seed", out)
        new = self.read(SEEDED)
        self.assertIn(SEED_TRUE, new)
        self.assertNotIn(SEED_FALSE, new)
        self.assertNotIn("// futility table guard.", new)
        self.assertEqual(new.count("\n"), self.text.count("\n") - 1)
        self.assert_marked(out, DONE)
        self.assertIn("1 dropped on seeds", out)
        status, out, err = self.run_tool("--identical", "HEAD")
        self.assertEqual(status, 0, out + err)

    def test_a_sentence_range_overlapping_by_one_character_is_dropped(self):
        end_of_seed = self.at + len(SEED_TRUE)
        for start, end in ((self.at - 5, self.at + 1), (end_of_seed - 1, end_of_seed + 5), (self.at + 2, self.at + 3)):
            with self.subTest(start=start, end=end):
                status, out, _ = self.apply_seeded([{"file": SEEDED, "start": start, "end": end}], [self.seed])
                self.assertEqual(status, 0, out)
                self.assertIn(f"DROPPED {SEEDED} edit 0 seed W1", out)
                self.assertEqual(self.read(SEEDED), self.text)

    def test_a_range_touching_only_the_seeds_edge_is_kept(self):
        end_of_seed = self.at + len(SEED_TRUE)
        status, out, _ = self.apply_seeded(
            [
                {"file": SEEDED, "start": self.at - 2, "end": self.at},
                {"file": SEEDED, "start": end_of_seed, "end": end_of_seed + 2},
            ],
            [self.seed],
        )
        self.assertEqual(status, 0, out)
        self.assertNotIn("DROPPED", out)
        self.assertIn("SEED-UNTOUCHED W1", out)
        self.assertIn(SEED_TRUE, self.read(SEEDED))

    def test_one_edit_over_two_seeds_names_both(self):
        other = self.text.index("C 0 -> 5", self.at)
        seeds = [self.seed, {"id": "W2", "file": SEEDED, "start": other, "end": other + 8, "text": "C 0 -> 6"}]
        status, out, _ = self.apply_seeded([{"file": SEEDED, "lines": [self.line, self.line]}], seeds)
        self.assertEqual(status, 0, out)
        self.assertIn(f"DROPPED {SEEDED} edit 0 seed W1", out)
        self.assertIn(f"DROPPED {SEEDED} edit 0 seed W2", out)
        self.assertEqual(self.read(SEEDED), self.text)
        self.assertIn("apply 0 files", out)

    def test_an_untouched_seed_is_named_even_in_a_file_with_no_edit(self):
        self.base("src/other.cpp", "// about other\nint other;\n")
        status, out, _ = self.apply_seeded([{"file": "src/other.cpp", "lines": [1, 1]}], [self.seed])
        self.assertEqual(status, 0, out)
        self.assertIn("SEED-UNTOUCHED W1", out)
        self.assertEqual(self.read("src/other.cpp"), "int other;\n")
        self.assertEqual(self.read(SEEDED), self.text)

    def test_a_kept_edit_cannot_write_a_seeds_text_into_the_tree(self):
        needle = "// futility table guard."
        at = self.text.index(needle)
        edit = {"file": SEEDED, "start": at, "end": at + len(needle), "text": "// GOLDEN " + SEED_FALSE + " as read"}
        status, out, _ = self.apply_seeded([edit], [self.seed])
        self.assert_refused(status, out, "seed W1")
        self.assertEqual(self.read(SEEDED), self.text)

    def test_nor_can_one_in_another_file(self):
        self.base("src/other.cpp", "// about other\nint other;\n")
        edit = {"file": "src/other.cpp", "lines": [1, 1], "text": "// read as " + SEED_FALSE}
        status, out, _ = self.apply_seeded([edit], [self.seed])
        self.assert_refused(status, out, "seed W1")
        self.assertEqual(self.read("src/other.cpp"), "// about other\nint other;\n")

    def test_a_dropped_edit_holding_a_seeds_text_is_only_dropped(self):
        edit = {"file": SEEDED, "start": self.at, "end": self.at + 8, "text": SEED_FALSE}
        status, out, _ = self.apply_seeded([edit], [self.seed])
        self.assertEqual(status, 0, out)
        self.assertIn(f"DROPPED {SEEDED} edit 0 seed W1", out)
        self.assertEqual(self.read(SEEDED), self.text)

    def test_a_seed_holding_a_newline_is_refused(self):
        space = self.text.index(" ", self.at)
        newline = self.text.index("\n", self.at)
        cases = [
            ("a new line in the copy", dict(self.seed, start=space, end=space + 1, text="\n")),
            ("a line gone from the copy", dict(self.seed, start=newline, end=newline + 1, text=" ")),
        ]
        for name, seed in cases:
            with self.subTest(name=name):
                status, out, _ = self.run_tool(
                    "--apply", self.edit_list([]), "--tree", "--seeds", self.seed_list([seed])
                )
                self.assert_refused(status, out, "newline")

    def test_a_list_cut_from_the_seeded_copy_cannot_apply_without_the_seeds(self):
        # The caught seed's `old` reads the false text, so the true sentence is safe either way.
        at = self.text.index("// razoring moved")
        line_end = self.text.index("\n", at)
        seeded_line = self.text[at:line_end].replace(SEED_TRUE, SEED_FALSE) + "\n"
        edits = [{"file": SEEDED, "lines": [self.line, self.line], "old": seeded_line}]
        status, out, _ = self.run_tool("--apply", self.edit_list(edits), "--tree")
        self.assert_refused(status, out, "does not match")
        self.assertIn(SEED_TRUE, self.read(SEEDED))
        status, out, _ = self.run_tool("--apply", self.edit_list(edits), "--tree", "--seeds", self.seed_list([self.seed]))
        self.assertEqual(status, 0, out)
        self.assertIn(f"DROPPED {SEEDED} edit 0 seed W1", out)

    def test_an_edit_on_a_seed_whose_old_is_neither_view_is_refused(self):
        edits = [{"file": SEEDED, "lines": [self.line, self.line], "old": "// some other line\n"}]
        status, out, _ = self.run_tool("--apply", self.edit_list(edits), "--tree", "--seeds", self.seed_list([self.seed]))
        self.assert_refused(status, out, "does not match")

    def test_a_seed_that_changes_length_is_refused_and_nothing_is_written(self):
        longer = dict(self.seed, text="A 15 -> 6")
        later = self.later_line()
        status, out, _ = self.apply_seeded([{"file": SEEDED, "lines": [later, later]}], [longer])
        self.assert_refused(status, out, "seed W1")
        self.assertIn("byte length", out)
        self.assertEqual(self.read(SEEDED), self.text)

    def test_equal_bytes_with_fewer_characters_moves_offsets_and_is_refused(self):
        self.base("src/u.cpp", "// a claim of ab here\nint u;\n")
        start = "// a claim of ab here".index("ab")
        seed = {"id": "W9", "file": "src/u.cpp", "start": start, "end": start + 2, "text": "\u00e9"}
        self.assertEqual(len(seed["text"].encode("utf-8")), 2)
        status, out, _ = self.apply_seeded([{"file": "src/u.cpp", "lines": [1, 1]}], [seed])
        self.assert_refused(status, out, "seed W9")

    def test_a_bad_seed_is_refused(self):
        code_at = self.text.index("#include")
        base = dict(self.seed)
        cases = [
            ("over code", dict(base, start=code_at, end=code_at + 8, text="#inclube"), "code token"),
            ("past the end", dict(base, start=len(self.text) + 1, end=len(self.text) + 9), "not inside the file"),
            ("no substitution", dict(base, text=SEED_TRUE), "changes nothing"),
            ("no such file", dict(base, file="src/nothing.cpp"), "not in"),
        ]
        for name, seed, words in cases:
            with self.subTest(name=name):
                later = self.later_line()
                status, out, _ = self.apply_seeded([{"file": SEEDED, "lines": [later, later]}], [seed])
                self.assert_refused(status, out, words)
                self.assertEqual(self.read(SEEDED), self.text)

    def test_seeds_that_overlap_or_share_an_id_are_refused(self):
        twin = dict(self.seed, id="W2", start=self.at + 1, end=self.at + 3, text=" 7")
        status, out, _ = self.apply_seeded([], [self.seed, twin])
        self.assert_refused(status, out, "overlap")
        other = self.text.index("C 0 -> 5", self.at)
        same_id = dict(self.seed, start=other, end=other + 8, text="C 0 -> 6")
        status, out, _ = self.apply_seeded([], [self.seed, same_id])
        self.assert_refused(status, out, "W1 twice")

    def test_a_seed_list_that_is_not_one_or_belongs_elsewhere_is_a_tool_error(self):
        edits = [{"file": SEEDED, "lines": [self.line, self.line]}]
        bad = os.path.join(self.scratch, "bad_seeds.json")
        bodies = ("not json", '{"seeds": []}', '{"ref": "HEAD", "seeds": [{"id": "W1"}]}')
        for body in bodies:
            with self.subTest(body=body):
                with open(bad, "w") as handle:
                    handle.write(body)
                status, out, _ = self.apply(edits, "--tree", "--seeds", bad)
                self.assertEqual(status, 2, out)
                self.assert_marked(out, FAILED)
        self.base("src/later.cpp", "int later;\n")
        status, out, _ = self.apply(edits, "--tree", "--seeds", self.seed_list([self.seed], ref="HEAD~1"))
        self.assertEqual(status, 2, out)
        self.assertIn("different ref", out)
        self.assertEqual(self.read(SEEDED), self.text)

    def test_seeds_go_with_apply_alone(self):
        status, out, err = self.run_tool("--report", "HEAD", "--seeds", self.seed_list([self.seed]))
        self.assertEqual(status, 2, out + err)
        self.assert_marked(out, FAILED)


LOG_BLOCK = "\n".join(
    [
        f"// S10{i} did a thing to this search that was measured and then recorded, DEC-00{i} says why it stood."
        for i in range(1, 8)
    ]
    + ["// " + "padding of ordinary words about the search and what it reads " * 3] * 3
)


class Report(FixtureRepo):
    def setUp(self):
        super().setUp()
        for sid in ("S100", "S101", "S102", "S103", "S104", "S105", "S106", "S107"):
            self.write(f"adocs/plan_done/{sid}_x.md", "done\n")
        self.write("adocs/plan_todo/S200_x.md", "todo\n")
        self.write(
            "adocs/decisions.md",
            "".join(f"## DEC-00{i}  a decision\n" for i in range(1, 8)) + "## DEC-142  goldens\n",
        )
        self.write("adocs/specs.md", "INV-1 holds.\n")
        self.write("src/a.cpp", "int a;\n")
        self.write("src/real.cpp", "int real;\n")
        self.write("tools/count.py", "x = 1\n")
        self.commit_all()

    def report(self, *extra):
        status, out, err = self.run_tool("--report", "HEAD", *extra)
        self.assertEqual(status, 0, out + err)
        self.assert_marked(out, DONE)
        return out

    def flag(self, comment, kind, what=None):
        self.write("src/a.cpp", comment + "\nint a;\n")
        out = self.report()
        rows = [l for l in out.split("\n") if l.startswith(kind)]
        self.assertTrue(rows, f"no {kind} in:\n{out}")
        self.assertIn(" new ", rows[0])
        if what:
            self.assertIn(what, rows[0])

    def test_each_flag_on_a_new_comment_and_the_run_stays_green(self):
        self.flag("// see src/real.cpp:12 for it", "FILE-LINE", "src/real.cpp:12")
        self.flag("// see src/gone.cpp for it", "GONE-PATH", "src/gone.cpp")
        self.flag("// S100 owns the rest", "CLOSED-OWNS", "S100 owns")
        self.flag("// until S100\n// lands, this stays", "CLOSED-OWNS")
        self.flag("// from S100, S101 and DEC-001", "ID-CHAIN", "S100, S101 and DEC-001")
        self.flag("// S999 says so", "NO-SUCH-ID", "S999")
        self.flag("// DEC-999 says so", "NO-SUCH-ID", "DEC-999")
        self.flag("// INV-77 says so", "NO-SUCH-ID", "INV-77")
        self.flag("// GOLDEN (DEC-142): 5, the count.", "GOLDEN-FORM", "no command, no margin")
        self.flag("// previously this read the table", "NARRATION", "previously")

    def test_what_is_not_flagged(self):
        quiet = (
            "// see src/real.cpp for it, S100, and S200 owns the rest\n"
            "// GOLDEN (DEC-142): 5, the count. Re-derive with tools/count.py; margin 0.\n"
            "// NOT A GOLDEN (DEC-142): a window.\n"
        )
        self.write("src/a.cpp", quiet + "int a;\n")
        out = self.report()
        for kind in ("FILE-LINE", "GONE-PATH", "CLOSED-OWNS", "ID-CHAIN", "NO-SUCH-ID", "GOLDEN-FORM", "NARRATION"):
            self.assertEqual([l for l in out.split("\n") if l.startswith(kind + " ")], [], kind)

    def test_the_same_comment_in_the_ref_reads_old_and_new_only_hides_it(self):
        self.write("src/a.cpp", "// see src/real.cpp:12 for it\nint a;\n")
        self.commit_all()
        out = self.report()
        self.assertIn("FILE-LINE   src/a.cpp:1 old", out)
        self.assertNotIn("FILE-LINE   src/a.cpp:1 old", self.report("--new-only"))
        self.write("src/a.cpp", "// see src/real.cpp:12 for it\n// and src/real.cpp:13\nint a;\n")
        out = self.report("--new-only")
        self.assertIn("FILE-LINE   src/a.cpp:2 new", out)
        self.assertNotIn("src/a.cpp:1", out)

    def test_old_narration_is_only_counted(self):
        self.write("src/a.cpp", "// previously this read the table\nint a;\n")
        self.commit_all()
        out = self.report()
        self.assertEqual([l for l in out.split("\n") if l.startswith("NARRATION ")], [])
        self.assertRegex(out, r"NARRATION\s+1 total,\s+0 new")

    def test_a_log_shaped_block_listed_on_head_then_grown_then_new(self):
        block = cb.Block("x.cpp", 1, 1, 0, 0, "line", False, LOG_BLOCK, len(LOG_BLOCK))
        self.assertTrue(cb.is_log_shaped(block), "the fixture does not meet the selector")
        self.write("src/a.cpp", LOG_BLOCK + "\nint a;\n")
        self.commit_all()
        out = self.report()
        self.assertRegex(out, r"LOG-SHAPED  src/a\.cpp:1-10 same \d+ B \d+ ids")
        self.assertNotIn("LOG-SHAPED  src/a", self.report("--new-only"))

        self.write("src/a.cpp", LOG_BLOCK + "\n// and S107 added one more line to the log\nint a;\n")
        self.assertRegex(self.report("--new-only"), r"LOG-SHAPED  src/a\.cpp:1-11 grown")

        self.write("src/b.cpp", LOG_BLOCK.replace("thing", "other thing") + "\nint b;\n")
        self.assertRegex(self.report("--new-only"), r"LOG-SHAPED  src/b\.cpp:1-10 new")

    def test_no_row_is_about_length(self):
        long_block = "\n".join(
            f"// line {i} of a long specification with nothing but ordinary words in it" for i in range(60)
        )
        self.assertGreater(len(long_block), cb.T2_MIN_BYTES)
        self.write("src/a.cpp", long_block + "\nint a;\n")
        self.commit_all()
        status, out, _ = self.run_tool("--tiers", "HEAD")
        self.assertEqual(status, 0, out)
        self.assertRegex(out, r"T2\tsrc/a\.cpp\t1\t60\t")
        self.write("src/a.cpp", long_block + "\n// a new line about the same thing\nint a;\n")
        out = self.report()
        rows = [l for l in out.split("\n") if l.split(" ")[0].isupper() and "src/a.cpp" in l]
        self.assertEqual(rows, [])

    def test_vendored_and_evidence_files_are_not_live(self):
        self.write("tests/third_party/v.hpp", "// see src/gone.cpp\n")
        self.write("adocs/data/e.py", "# see src/gone.cpp\n")
        self.write("tests/live.cpp", "// see src/gone.cpp\n")
        out = self.report()
        rows = [l for l in out.split("\n") if l.startswith("GONE-PATH")]
        self.assertEqual(len(rows), 1, rows)
        self.assertIn("tests/live.cpp", rows[0])


class NewFacts(FixtureRepo):
    def test_new_ids_and_numbers_only(self):
        self.write("src/a.cpp", "// S100 read 42 at 4e34e61 over 1000 games\nint a;\n")
        self.commit_all()
        self.write("src/a.cpp", "// S100 read 42, S200 read 77 at 4e34e61; DEC-300 and abc1234\nint a = 9;\n")
        status, out, err = self.run_tool("--new-facts", "HEAD")
        self.assertEqual(status, 0, out + err)
        facts = sorted(l.split()[-1] for l in out.split("\n") if l.startswith("NEW-FACT"))
        self.assertEqual(facts, sorted(["S200", "77", "DEC-300", "abc1234"]))
        self.assertIn("NEW-FACT src/a.cpp:1 S200", out)
        self.assert_marked(out, DONE)

    def test_a_pure_deletion_has_no_new_fact(self):
        self.write("src/a.cpp", "// S100 read 42\n// and 77\nint a;\n")
        self.commit_all()
        self.write("src/a.cpp", "// S100 read 42\nint a;\n")
        status, out, _ = self.run_tool("--new-facts", "HEAD")
        self.assertEqual(status, 0, out)
        self.assertNotIn("NEW-FACT", out)


class Anchors(FixtureRepo):
    def mutants(self, source_text, anchor):
        self.write("src/x.cpp", source_text)
        self.write(
            "tools/mutants/x.py",
            f'm("X01", "src/x.cpp", "k", "note", ({anchor!r}, "changed"))\n',
        )

    def test_found_once_is_green_and_a_comment_overlap_is_named(self):
        self.mutants("// a comment\nint x = 1;\n", "int x = 1;")
        status, out, err = self.run_tool("--anchors")
        self.assertEqual(status, 0, out + err)
        self.assertIn("1 mutants, 1 anchors, 0 overlap comments, 0 bad", out)
        self.mutants("// a comment\nint x = 1;\n", "a comment")
        status, out, _ = self.run_tool("--anchors")
        self.assertEqual(status, 0, out)
        self.assertIn("ON-COMMENT X01 pair 0 src/x.cpp:1", out)

    def test_gone_or_doubled_is_red(self):
        for text in ("int y;\n", "int x = 1;\nint x = 1;\n"):
            with self.subTest(text=text):
                self.mutants(text, "int x = 1;")
                status, out, _ = self.run_tool("--anchors")
                self.assertEqual(status, 1, out)
                self.assertIn("ANCHOR X01 pair 0", out)
                self.assert_marked(out, FAILED)


class Tiers(FixtureRepo):
    def test_each_block_with_its_tier(self):
        big = "\n".join(f"// line {i} of ordinary words about what the code reads and why" for i in range(40))
        self.write("src/a.cpp", LOG_BLOCK + "\nint a;\n\n" + big + "\nint b;\n\n// short\nint c; // trailing\n")
        self.commit_all()
        status, out, err = self.run_tool("--tiers", "HEAD")
        self.assertEqual(status, 0, out + err)
        rows = [l.split("\t") for l in out.split("\n") if l.startswith("T") and "\t" in l and l[1].isdigit()]
        self.assertEqual([r[0] for r in rows], ["T1", "T2", "T3", "T3"])
        self.assertIn("T1 1 blocks", out)
        self.assertIn("T3 2 blocks", out)
        # The offsets are what --apply takes.
        text = self.read("src/a.cpp")
        start, end = int(rows[2][4]), int(rows[2][5])
        self.assertEqual(text[start:end], "// short")


FORMAT_SOURCE = """// a comment
struct alignas(64) tt_cluster_t
{
  tt_entry_t entries[TT_CLUSTER_ENTRIES];
};
int other;
"""
FORMAT_ERROR = "{path}:{line}:{col}: error: code should be clang-formatted [-Wclang-format-violations]\n{text}\n ^\n"


def format_output(*rows):
    return "".join(FORMAT_ERROR.format(path=p, line=l, col=1, text=t) for p, l, t in rows)


class Gate(FixtureRepo):
    def setUp(self):
        super().setUp()
        self.write("tools/comment_budget.py", read_text(TOOL))
        self.write("tools/mutation_check.py", read_text(os.path.join(ROOT, "tools", "mutation_check.py")))
        self.write("adocs/data/S312_local_gate.sh", read_text(GATE))
        self.write("adocs/data/S101_pause_check.sh", read_text(PAUSE_CHECK))
        os.chmod(os.path.join(self.dir, "adocs/data/S312_local_gate.sh"), 0o755)
        self.write("tools/mutants/x.py", 'm("X01", "src/a.cpp", "k", "note", ("int a = 1;", "int a = 2;"))\n')
        self.write("src/a.cpp", "// about a\nint a = 1;\n")
        self.notes = "".join(f"line {i} of a document\n" for i in range(40))
        self.write("DOC.md", self.notes)
        self.commit_all()
        # The paused diff: an edit to line 5 of the document, recorded and undone.
        self.write("DOC.md", self.notes.replace("line 5 of", "line five of"))
        self.write("adocs/data/S101_phase2_pause.diff", self.git("diff", "--", "DOC.md"))
        self.write("DOC.md", self.notes)
        self.commit_all()

    def gate(self, *args):
        proc = subprocess.run(
            [os.path.join(self.dir, "adocs/data/S312_local_gate.sh"), *args],
            capture_output=True,
            text=True,
            env=clean_env(),
        )
        return proc.returncode, proc.stdout + proc.stderr

    def assert_gate_ends(self, out, marker):
        self.assertEqual(out.strip().split("\n")[-1].split()[0].rstrip(":"), marker, out)

    def test_an_injected_code_change_is_red(self):
        status, out = self.gate("--only", "identity")
        self.assertEqual(status, 0, out)
        self.write("src/a.cpp", "// about a\nint a = 2;\n")
        status, out = self.gate("--only", "identity")
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF src/a.cpp lexer:", out)
        self.assert_gate_ends(out, "S312-GATE-FAILED")

    def test_a_comment_ending_in_a_backslash_is_red(self):
        self.write("src/a.cpp", "// about a \\\n// more\nint a = 1;\n")
        status, out = self.gate("--only", "identity")
        self.assertEqual(status, 1, out)
        self.assertIn("DIFF src/a.cpp wcomment:", out)

    def test_a_comment_edit_is_green_and_the_marker_says_partial(self):
        self.write("src/a.cpp", "// about the a\nint a = 1;\n")
        status, out = self.gate("--only", "identity,anchors")
        self.assertEqual(status, 0, out)
        self.assert_gate_ends(out, "S312-GATE-PARTIAL")

    def test_an_anchor_broken_by_a_comment_edit_is_red(self):
        self.write("tools/mutants/x.py", 'm("X01", "src/a.cpp", "k", "note", ("about a", "changed"))\n')
        self.commit_all()
        self.write("src/a.cpp", "// about the a\nint a = 1;\n")
        status, out = self.gate("--only", "anchors")
        self.assertEqual(status, 1, out)
        self.assertIn("ANCHOR X01 pair 0", out)

    def test_an_uncommitted_conflict_with_the_paused_diff_is_red(self):
        self.write("DOC.md", self.notes.replace("line 5 of", "line FIVE of"))
        # What the commit-less gate exists for: the check alone reads HEAD and is green here.
        alone = subprocess.run(
            ["bash", "adocs/data/S101_pause_check.sh"], cwd=self.dir, capture_output=True, text=True, env=clean_env()
        )
        self.assertEqual(alone.returncode, 0, alone.stdout + alone.stderr)
        status, out = self.gate("--only", "s101")
        self.assertEqual(status, 1, out)
        self.assertIn("S101-PAUSE-FAILED", out)
        self.assert_gate_ends(out, "S312-GATE-FAILED")

    def test_an_edit_clear_of_the_paused_diff_is_green_and_leaves_no_trace(self):
        self.write("DOC.md", self.notes.replace("line 35 of", "line thirty-five of"))
        self.write("adocs/data/brand_new.tsv", "a\n")
        head = self.git("rev-parse", "HEAD")
        status, out = self.gate("--only", "s101")
        self.assertEqual(status, 0, out)
        self.assertIn("S101-PAUSE-OK", out)
        self.assertEqual(self.git("rev-parse", "HEAD"), head)
        self.assertEqual(self.git("diff", "--cached", "--name-only"), "")
        self.assertEqual(len(self.git("worktree", "list").strip().split("\n")), 1)
        self.assertIn("?? adocs/data/brand_new.tsv", self.git("status", "--short"))

    def test_a_staged_change_the_tree_has_undone_is_red(self):
        self.write("src/a.cpp", "// about a\nint a = 2;\n")
        self.git("add", "src/a.cpp")
        self.write("src/a.cpp", "// about a\nint a = 1;\n")
        # What the check exists for: the proof reads the tree, and there it is green.
        status, out, err = self.run_tool("--identical", "HEAD")
        self.assertEqual(status, 0, out + err)
        self.assertIn("src/a.cpp", self.git("diff", "--cached", "--name-only"))
        status, out = self.gate("--only", "identity")
        self.assertEqual(status, 1, out)
        self.assertIn("S312-GATE-FAILED: the index", out)
        self.assert_gate_ends(out, "S312-GATE-FAILED")

    def test_a_batch_staged_in_part_is_red(self):
        self.write("src/a.cpp", "// about the a\nint a = 1;\n")
        self.git("add", "src/a.cpp")
        self.write("DOC.md", self.notes.replace("line 35 of", "line thirty-five of"))
        status, out = self.gate("--only", "identity")
        self.assertEqual(status, 1, out)
        self.assertIn("S312-GATE-FAILED: the index", out)

    def test_an_index_equal_to_the_tree_is_green(self):
        self.write("src/a.cpp", "// about the a\nint a = 1;\n")
        self.git("add", "src/a.cpp")
        status, out = self.gate("--only", "identity")
        self.assertEqual(status, 0, out)
        self.assertIn("SAME src/a.cpp", out)

    def test_an_untracked_file_beside_a_whole_index_is_not_what_a_commit_takes(self):
        self.write("src/a.cpp", "// about the a\nint a = 1;\n")
        self.git("add", "src/a.cpp")
        self.write("adocs/some_report.md", "words\n")
        status, out = self.gate("--only", "identity")
        self.assertEqual(status, 0, out)

    def stub_bin(self, **scripts):
        directory = os.path.join(self.scratch, "bin")
        os.makedirs(directory, exist_ok=True)
        for name, body in scripts.items():
            path = os.path.join(directory, name)
            with open(path, "w") as handle:
                handle.write("#!/bin/sh\n" + body + "\n")
            os.chmod(path, 0o755)
        return directory

    def stubbed_gate(self, *args, bin_dir=None, **extra):
        env = clean_env(**extra)
        if bin_dir:
            env["PATH"] = bin_dir + os.pathsep + env["PATH"]
        proc = subprocess.run(
            [os.path.join(self.dir, "adocs/data/S312_local_gate.sh"), *args],
            capture_output=True,
            text=True,
            env=env,
        )
        return proc.returncode, proc.stdout + proc.stderr

    def test_suite_runs_both_builds_without_the_held_test_and_a_red_in_either_stops_it(self):
        log = os.path.join(self.scratch, "calls.log")
        record = f'echo "$0 $*" >> "{log}"\n'
        bin_dir = self.stub_bin(cmake=record + "exit 0", ctest=record + "exit 0")
        status, out = self.stubbed_gate("--only", "suite", bin_dir=bin_dir)
        self.assertEqual(status, 0, out)
        calls = read_text(log)
        for words in ("--build build -j8", "--build build-tune -j8", "--test-dir build -L fast", "--test-dir build-tune -L fast"):
            self.assertIn(words, calls)
        self.assertEqual(calls.count("-E ^test_fastchess_script$"), 2)
        for failing in ("build", "build-tune"):
            with self.subTest(failing=failing):
                body = f'case "$*" in *"--test-dir {failing} "*) exit 8;; esac\nexit 0'
                status, out = self.stubbed_gate("--only", "suite", bin_dir=self.stub_bin(ctest=body))
                self.assertEqual(status, 1, out)
                self.assertIn("S312-GATE-FAILED: suite", out)
        status, out = self.stubbed_gate("--only", "suite", bin_dir=self.stub_bin(cmake="exit 2", ctest="exit 0"))
        self.assertEqual(status, 1, out)
        self.assertIn("S312-GATE-FAILED: suite", out)

    def engine(self, total):
        path = os.path.join(self.dir, "build", "src", "chesso")
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w") as handle:
            handle.write(f"#!/bin/sh\ncat > /dev/null\necho bestmove e2e4\necho '{total}'\n")
        os.chmod(path, 0o755)

    def test_bench_compares_the_total_and_the_environment_may_move_the_number(self):
        self.engine("5707449 nodes 123 nps")
        status, out = self.stubbed_gate("--only", "bench")
        self.assertEqual(status, 0, out)
        self.assertIn("bench: 5707449", out)
        self.engine("5707450 nodes 123 nps")
        status, out = self.stubbed_gate("--only", "bench")
        self.assertEqual(status, 1, out)
        self.assertIn("S312-GATE-FAILED: bench: 5707450, expected 5707449", out)
        status, out = self.stubbed_gate("--only", "bench", EXPECTED_BENCH="5707450")
        self.assertEqual(status, 0, out)
        self.engine("no total here")
        status, out = self.stubbed_gate("--only", "bench")
        self.assertEqual(status, 1, out)
        self.assertIn("S312-GATE-FAILED: bench: no", out)

    def format_script(self, body):
        self.write("clang-format.sh", "#!/bin/sh\n" + body + "\n")
        os.chmod(os.path.join(self.dir, "clang-format.sh"), 0o755)

    def test_format_stage_reads_the_script_and_a_violation_stops_it(self):
        self.format_script("exit 0")
        status, out = self.stubbed_gate("--only", "format")
        self.assertEqual(status, 0, out)
        self.format_script(
            "echo 'src/a.cpp:2:1: error: code should be clang-formatted [-Wclang-format-violations]'\nexit 1"
        )
        status, out = self.stubbed_gate("--only", "format")
        self.assertEqual(status, 1, out)
        self.assertIn("S312-GATE-FAILED: format", out)

    def test_citations_stage_stops_on_a_flag(self):
        self.write("tools/plan_prose_check.py", "print('citations flagged: 0 over 1 files')\n")
        status, out = self.stubbed_gate("--only", "citations")
        self.assertEqual(status, 0, out)
        self.write("tools/plan_prose_check.py", "import sys\nprint('citations flagged: 3 over 1 files')\nsys.exit(1)\n")
        status, out = self.stubbed_gate("--only", "citations")
        self.assertEqual(status, 1, out)
        self.assertIn("S312-GATE-FAILED: citations", out)

    def test_an_unknown_stage_and_argument_fail_with_the_marker(self):
        for args in (("--only", "nonsense"), ("--wat",)):
            status, out = self.gate(*args)
            self.assertEqual(status, 1, out)
            self.assert_gate_ends(out, "S312-GATE-FAILED")

    def format_gate(self, source, rows, status=1):
        os.makedirs(os.path.join(self.scratch, "src"), exist_ok=True)
        with open(os.path.join(self.scratch, "src", "data_structures.hpp"), "w") as handle:
            handle.write(source)
        output = os.path.join(self.scratch, "format.txt")
        with open(output, "w") as handle:
            handle.write(rows if isinstance(rows, str) else format_output(*rows))
        return self.gate("--read-format", output, str(status), self.scratch)

    def test_the_format_reader_takes_the_two_lines_by_content(self):
        name = "src/data_structures.hpp"
        status, out = self.format_gate(
            FORMAT_SOURCE, [(name, 3, "{"), (name, 4, "  tt_entry_t entries[TT_CLUSTER_ENTRIES];")]
        )
        self.assertEqual(status, 0, out)
        self.assertIn("2 flagged, tt_cluster_t's own", out)

    def test_the_format_reader_follows_the_lines_when_they_move(self):
        name = "src/data_structures.hpp"
        moved = FORMAT_SOURCE.replace("// a comment\n", "")
        status, out = self.format_gate(
            moved, [(name, 2, "{"), (name, 3, "  tt_entry_t entries[TT_CLUSTER_ENTRIES];")]
        )
        self.assertEqual(status, 0, out)

    def test_the_format_reader_refuses_any_other_line(self):
        name = "src/data_structures.hpp"
        cases = [
            [(name, 3, "{"), (name, 6, "int other;")],
            [("tests/test_mate_carry.cpp", 6, "// a long comment")],
            # both of its lines and one more
            [(name, 3, "{"), (name, 4, "  tt_entry_t entries[TT_CLUSTER_ENTRIES];"), (name, 1, "// a comment")],
        ]
        for rows in cases:
            with self.subTest(rows=rows):
                status, out = self.format_gate(FORMAT_SOURCE, rows)
                self.assertEqual(status, 1, out)
                self.assertIn("flagged beyond tt_cluster_t", out)
        status, out = self.format_gate(
            FORMAT_SOURCE.replace("struct alignas(64) tt_cluster_t", "struct something_else"),
            [(name, 3, "{")],
        )
        self.assertEqual(status, 1, out)

    def test_the_format_reader_takes_nothing_flagged_and_refuses_a_format_tool_that_failed(self):
        status, out = self.format_gate(FORMAT_SOURCE, "", status=0)
        self.assertEqual(status, 0, out)
        status, out = self.format_gate(FORMAT_SOURCE, "clang-format 23 not found.\n", status=1)
        self.assertEqual(status, 1, out)
        self.assertIn("flagged nothing", out)


if __name__ == "__main__":
    unittest.main()
