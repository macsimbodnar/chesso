#!/usr/bin/env python3
"""The gate on tools/trim_console.py, the filter rating.sh writes its report
through. S241, DEC-235.

WHAT IS ASSERTED. The one block the tool exists to drop -- fastchess's
"PV continues after <rule>" warning and its three context lines, for both
rules it prints, threefold repetition and the fifty-move rule -- goes, whole,
counted per rule and engine; and nothing else does. The second half is the one
a filter on evidence has to prove: "Incomplete mating PV" prints the same
Info, Position and Moves lines, S238's pairs reading counts it per side, and a
filter that ate them would have silently changed a number in the ledger.

The tool is driven as a subprocess over stdin, the way the pipeline drives it,
so the test measures the command and not a function it happens to share.
"""

import os
import re
import subprocess
import sys
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOL = os.path.join(ROOT, "tools", "trim_console.py")

THREEFOLD = (b"Warning; PV continues after threefold repetition - move a8d8 from Leorik 2.4\n"
             b"Info; info depth 13 score cp 85 nodes 383964 pv g7g6 b5b6\n"
             b"Position; startpos\n"
             b"Moves; d2d4 g8f6 c2c4 d7d6\n")
FIFTY = (b"Warning; PV continues after fifty-move rule - move d6b5 from Blunder 8.5.5\n"
         b"Info; info depth 9 score cp 12 nodes 5000 pv d6b5\n"
         b"Position; startpos\n"
         b"Moves; e2e4 e7e5 g1f3\n")
MATING = (b"Warning; Incomplete mating PV - from cand-257c8fe\n"
          b"Info; info score mate 6 time 0 depth 1 nodes 63 pv b4d4\n"
          b"Position; fen rnbqkbnr/2p1pppp/p7/1p1p4/8/PP3N2/2PPPPPP/RNBQKB1R w KQkq - 0 4\n"
          b"Moves; d2d4 e7e6 e2e3 g8f6\n")
GAME = b"Started game 1 of 10 (chesso vs Leorik 2.4)\n"
DONE = b"Finished game 1 (chesso vs Leorik 2.4): 1-0 {White wins by adjudication}\n"
HEADING = b"=== fastchess console lines omitted from this report ===\n"


def run(data, *args):
    proc = subprocess.run([sys.executable, TOOL] + list(args), input=data,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return proc.returncode, proc.stdout, proc.stderr


class TrimConsole(unittest.TestCase):

    def test_threefold_block_goes_whole_and_nothing_else(self):
        status, out, err = run(GAME + THREEFOLD + DONE)
        self.assertEqual(status, 0, err)
        self.assertEqual(out, GAME + DONE)

    def test_fifty_move_block_goes_the_same_way(self):
        status, out, _ = run(GAME + FIFTY + DONE)
        self.assertEqual(status, 0)
        self.assertEqual(out, GAME + DONE)

    def test_mating_pv_warning_and_its_context_pass(self):
        status, out, _ = run(GAME + MATING + DONE)
        self.assertEqual(status, 0)
        self.assertEqual(out, GAME + MATING + DONE)

    def test_the_three_warnings_back_to_back(self):
        status, out, _ = run(THREEFOLD + MATING + FIFTY + THREEFOLD)
        self.assertEqual(status, 0)
        self.assertEqual(out, MATING)

    def test_context_lines_without_a_dropped_warning_pass(self):
        stray = b"Info; info depth 1\nPosition; startpos\nMoves; e2e4\n"
        status, out, _ = run(stray)
        self.assertEqual(status, 0)
        self.assertEqual(out, stray)

    def test_an_interleaved_line_ends_the_block(self):
        # A block cut by another game's line loses only its own lines: the
        # warning, the Info that followed it, and nothing after the intruder.
        cut = (b"Warning; PV continues after threefold repetition - move a8d8 from Leorik 2.4\n"
               b"Info; info depth 13\n" + DONE + b"Position; startpos\nMoves; d2d4\n")
        status, out, _ = run(cut)
        self.assertEqual(status, 0)
        self.assertEqual(out, DONE + b"Position; startpos\nMoves; d2d4\n")

    def test_summary_counts_per_rule_and_engine_sorted_by_count(self):
        status, out, _ = run(THREEFOLD + FIFTY + THREEFOLD + MATING, "--summary")
        self.assertEqual(status, 0)
        body, _, tail = out.partition(HEADING)
        self.assertEqual(body, MATING + b"\n")
        rows = [line for line in tail.split(b"\n") if line.startswith(b"  ") and not line.startswith(b"  total")]
        self.assertRegex(rows[0], rb"^  threefold repetition +Leorik 2\.4 +2$")
        self.assertRegex(rows[1], rb"^  fifty-move rule +Blunder 8\.5\.5 +1$")
        self.assertIn(b"  total: 3 blocks, 12 lines\n", tail)

    def test_summary_with_nothing_dropped_says_none(self):
        status, out, _ = run(GAME, "--summary")
        self.assertEqual(status, 0)
        self.assertTrue(out.startswith(GAME))
        self.assertIn(b"  none\n", out)
        self.assertIn(b"  total: 0 blocks, 0 lines\n", out)

    def test_a_warning_naming_no_engine_counts_as_unknown(self):
        nameless = b"Warning; PV continues after threefold repetition\nInfo; x\nPosition; y\nMoves; z\n"
        status, out, _ = run(nameless, "--summary")
        self.assertEqual(status, 0)
        self.assertRegex(out, rb"(?m)^  threefold repetition +unknown +1$")

    def test_a_warning_naming_no_rule_counts_as_unknown(self):
        bare = b"Warning; PV continues after  from Leorik 2.1\nInfo; x\nPosition; y\nMoves; z\n"
        status, out, _ = run(bare, "--summary")
        self.assertEqual(status, 0)
        self.assertRegex(out, rb"(?m)^  unknown +Leorik 2\.1 +1$")

    def test_bytes_pass_undecoded(self):
        raw = b"Started game 1 (chesso vs Caf\xe9)\n"
        status, out, _ = run(raw)
        self.assertEqual(status, 0)
        self.assertEqual(out, raw)

    def test_unknown_argument_is_refused(self):
        status, _, err = run(b"", "--verbose")
        self.assertEqual(status, 2)
        self.assertIn(b"unknown argument", err)


if __name__ == "__main__":
    unittest.main()
