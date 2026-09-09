#!/usr/bin/env python3
"""The gate on `tools/plan_prose_check.py --citations`. S187.

WHY PLANTED CASES AND NOT THE REAL SET. The check runs over
`adocs/plan_todo/` and `adocs/plan_current/`, and those files are edited by
every step. A test that asserted something about them would go red on a plan
edit and say nothing about the checker; worse, the day the checker stopped
recognising a citation altogether the real set would go green and nobody
would know. So each case here writes one step file into a temporary directory,
runs the mode over that file alone -- `citations()` accepts an explicit file
list for exactly this -- and asserts the exit code and the flag word.

A planted file has no commit and lives outside the repository. That is fine
and it is deliberate: no failure class consults a previous commit any more
(the retired DRIFT did, which is why this mode used to cost 6.3 s and could
not be in the suite). Resolution is against the working tree, so the symbols
the passing cases name are real ones in this checkout -- `negamax` in
`src/search.cpp`, "pruning does not hide a forced mate" in
`tests/test_search.cpp`. If one of those is renamed, this test goes red and
the rename is the fix, which is the same coupling the real set has.

WHAT IS NOT ASSERTED. Relevance. `src/search.cpp` `state` passes because
`state` occurs in the file; the check is existence, by DEC-135's mandate, and
the mapping a conversion is made through is where relevance is proved.
"""

import os
import subprocess
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CHECKER = os.path.join(ROOT, "tools", "plan_prose_check.py")


class CitationChecks(unittest.TestCase):
    def run_on(self, body):
        """Exit code and stdout of --citations over one planted step file."""
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "S999_planted.md")
            with open(path, "w", encoding="utf-8") as fh:
                fh.write(body)
            r = subprocess.run([sys.executable, CHECKER, "--citations", path],
                               capture_output=True, text=True, cwd=ROOT)
        return r.returncode, r.stdout

    def assert_flag(self, body, word):
        code, out = self.run_on(body)
        self.assertEqual(code, 1, out)
        self.assertIn(word, out)

    def assert_clean(self, body):
        code, out = self.run_on(body)
        self.assertEqual(code, 0, out)
        self.assertNotIn("LINE", out)
        self.assertNotIn("MISSING", out)
        self.assertNotIn("BARE", out)

    # --- the retired form ---------------------------------------------------

    def test_line_citation_fails(self):
        self.assert_flag("A line citation `src/chesso.cpp:12` is the retired "
                         "form.\n", "LINE")

    def test_line_range_fails(self):
        self.assert_flag("A range `src/search.cpp:100-120` is one too.\n",
                         "LINE")

    def test_line_into_a_document_fails(self):
        """Gated like a code one: a citation into plan.md rots fastest."""
        self.assert_flag("The plan says so at `adocs/plan.md:359`.\n", "LINE")

    def test_line_after_a_slash_fails(self):
        """`PATH` refuses a path preceded by a slash, so that it cannot match
        the tail of a longer one -- and S020 and S117 both write
        `path:line/path:line` for a pair of sites, which hid four citations
        from the walk. A LINE flag resolves nothing, so the retired form is
        looked for in the file's text as well."""
        self.assert_flag("Reached from make via `src/bitboard.cpp` `add_piece`"
                         "/src/bitboard.cpp:704 and directly.\n", "LINE")

    def test_bare_continuation_still_fails(self):
        self.assert_flag("Mobility at `src/evaluation.cpp` `evaluate_cheap`, "
                         "and :953 for king safety.\n", "BARE")

    # --- the symbol form ----------------------------------------------------

    def test_absent_symbol_fails(self):
        self.assert_flag("The guard lives in `src/search.cpp` "
                         "`no_such_symbol_anywhere`.\n", "MISSING")

    def test_absent_title_fails(self):
        self.assert_flag('Pinned by `tests/test_search.cpp` "no such title at '
                         'all".\n', "MISSING")

    def test_absent_file_fails(self):
        self.assert_flag("Written in `src/no_such_file.cpp` `negamax`.\n",
                         "MISSING")

    def test_symbol_only_in_a_comment_fails(self):
        """`code_of` blanks comments, so a comment is not a landing site.

        The precondition is established rather than assumed: `GAME_PHASE_MAX`
        occurs in `src/search_params.hpp` exactly once and inside the "Not in
        the set, on purpose" comment, so the symbol form must fail there while
        the same words as a quoted phrase pass on the raw text -- which is how
        a citation into a comment is written.
        """
        with open(os.path.join(ROOT, "src", "search_params.hpp"),
                  encoding="utf-8") as fh:
            lines = fh.read().splitlines()
        holding = [n for n in lines if "GAME_PHASE_MAX" in n]
        self.assertEqual(len(holding), 1, holding)
        self.assertTrue(holding[0].lstrip().startswith("//"), holding[0])
        self.assert_flag("Excluded by `src/search_params.hpp` "
                         "`GAME_PHASE_MAX`.\n", "MISSING")
        self.assert_clean('Excluded by `src/search_params.hpp` "GAME_PHASE_MAX '
                          'are definitions and not settings".\n')

    def test_symbol_passes(self):
        self.assert_clean("The move loop is `src/search.cpp` `negamax`.\n")

    def test_parenthesised_tail_is_dropped_whole(self):
        """rstrip('()') would leave `CHESSO_SEARCH_PARAMS(X` and find nothing."""
        self.assert_clean("Generated from `src/search_params.hpp` "
                          "`CHESSO_SEARCH_PARAMS(X)`.\n")

    def test_title_passes(self):
        self.assert_clean('Extended by `tests/test_search.cpp` "pruning does '
                          'not hide a forced mate".\n')

    def test_title_across_a_wrap_passes(self):
        """The prose is hard-wrapped; both sides are flattened before matching."""
        self.assert_clean('Extended by `tests/test_search.cpp` "pruning does\n'
                          'not hide a forced mate".\n')

    def test_symbol_on_the_next_line_passes(self):
        self.assert_clean("The move loop is `src/search.cpp`\n`negamax`.\n")

    def test_phrase_from_a_comment_passes_case_folded(self):
        self.assert_clean('It says `tests/test_uci_surface.cpp` "the release '
                          'build declares exactly the five golden lines".\n')

    def test_path_then_prose_is_not_a_citation(self):
        """88 places write a backticked path and then an ordinary word."""
        self.assert_clean("`src/search.cpp` is where the search lives, and "
                          "`tools/gate.sh` runs the gate.\n")

    def test_absent_phrase_in_a_document_is_a_note(self):
        """The document bucket does not gate: those files are rewritten at
        every completion, so gating a phrase in one makes red the normal
        state. The owner's answer of 2026-09-09, recorded in S187."""
        code, out = self.run_on('The plan says `adocs/plan.md` "no such '
                                'sentence lives here".\n')
        self.assertEqual(code, 0, out)
        self.assertIn("note", out)
        self.assertIn("MISSING", out)

    def test_fenced_block_is_not_scanned(self):
        """A fence holds a command, and the quote closing a string literal in
        one reads as the opening of a phrase."""
        self.assert_clean('Run it:\n\n```bash\n'
                          'python3 -c \'open("src/no_such_file.cpp:3")\'\n'
                          '```\n\nand read the output.\n')


if __name__ == "__main__":
    unittest.main(verbosity=2)
