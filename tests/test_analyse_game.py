#!/usr/bin/env python3
"""The gate on tools/analyse_game.py. S214, 2026-09-10_adversarial-F28.

WHY A STUB ENGINE AND NOT STOCKFISH. What is under test is what the tool does
with an engine's answers, and the three answers that matter -- none, a bound
only, a bound after a real score -- are ones no correct engine produces on
demand. A `/bin/sh` stub produces each of them in one line, costs
milliseconds, and needs nothing installed; driving a real engine would test
the engine instead and could not reach the cases at all.

WHAT THE CASES ARE. The tool exists because agent chess judgement is banned
(`CLAUDE.md`, DEC-023), so every number it prints is taken on trust. Before
S214 it initialised `score, best = 0, "-"` and returned them when no `info`
line carrying both a score and a pv arrived, which turns an engine that
answers `bestmove` alone into a game where every position is a dead draw --
and the cost table that comes out of that looks exactly like a real one. It
also read `lowerbound`/`upperbound` lines as values, which they are not: the
search proved "at least beta" and nothing more.
"""

import os
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOL = os.path.join(ROOT, "tools", "analyse_game.py")

# Two positions, so main() has a pair to print a cost row from. The FENs are
# the start position and the same after 1.e4; nothing here depends on their
# content beyond being distinguishable in an error message.
START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
AFTER_E4 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"

POSITIONS = "\t".join(["0", "e4", "e2e4", START, "24"]) + "\n" + \
            "\t".join(["1", "e5", "e7e5", AFTER_E4, "24"]) + "\n"


def stub(go_reply):
    """A UCI engine that answers the handshake and then `go_reply`."""
    return (
        "#!/bin/sh\n"
        "while read -r line; do\n"
        "  case \"$line\" in\n"
        "    uci) echo 'id name stub'; echo uciok ;;\n"
        "    isready) echo readyok ;;\n"
        "    go*)\n"
        + go_reply +
        "      ;;\n"
        "    quit) exit 0 ;;\n"
        "  esac\n"
        "done\n")


# No `info` line at all: the case F28 names.
SILENT = stub("      echo 'bestmove e2e4'\n")

# Bound lines only. Both carry a score and a pv and are still not values.
BOUNDS_ONLY = stub(
    "      echo 'info depth 5 score cp 300 lowerbound nodes 100 pv e2e4'\n"
    "      echo 'info depth 6 score cp -400 upperbound nodes 200 pv e2e4'\n"
    "      echo 'bestmove e2e4'\n")

# A real score, then a bound the tool must not overwrite it with. 12 is what
# the table below has to show; 999 is what a run that reads bounds shows.
SCORE_THEN_BOUND = stub(
    "      echo 'info depth 5 score cp 12 nodes 100 pv e2e4 e7e5'\n"
    "      echo 'info depth 6 score cp 999 lowerbound nodes 200 pv e2e4'\n"
    "      echo 'bestmove e2e4'\n")


class AnalyseGame(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp(prefix="chesso-analyse-game.")
        # One per case, four per fast-suite run, and the suite runs several
        # times a day: without this the executable stubs pile up in /tmp
        # forever. `ignore_errors` because a cleanup that fails must not turn a
        # green test red -- the leak is the thing being fixed, not a new way to
        # fail. tests/test_mutation_check.py does the same.
        self.addCleanup(shutil.rmtree, self.dir, ignore_errors=True)
        self.positions = os.path.join(self.dir, "positions.tsv")
        with open(self.positions, "w") as handle:
            handle.write(POSITIONS)

    def run_tool(self, engine_text, threshold=0):
        engine = os.path.join(self.dir, "engine.sh")
        with open(engine, "w") as handle:
            handle.write(engine_text)
        os.chmod(engine, 0o755)
        return subprocess.run(
            [sys.executable, TOOL, self.positions, "--engine", engine,
             "--depth", "1", "--threshold", str(threshold)],
            capture_output=True, text=True, timeout=60)

    def test_no_info_line_is_refused_and_names_the_position(self):
        """F28: `bestmove` alone must not read as 0.00."""
        done = self.run_tool(SILENT)
        self.assertNotEqual(done.returncode, 0, done.stdout)
        self.assertIn(START, done.stderr)
        self.assertNotIn("cost", done.stdout)

    def test_bound_lines_alone_are_not_a_score(self):
        """A half-open window is not an evaluation, so this is the case above
        with the engine talking."""
        done = self.run_tool(BOUNDS_ONLY)
        self.assertNotEqual(done.returncode, 0, done.stdout)
        self.assertIn(START, done.stderr)
        self.assertNotIn("300", done.stdout)

    def test_a_bound_does_not_overwrite_the_score_before_it(self):
        """The last info line wins, unless it is a bound. Both positions score
        12, so the row is 12 / -12 / 24; reading the bound would print 999."""
        done = self.run_tool(SCORE_THEN_BOUND)
        self.assertEqual(done.returncode, 0, done.stderr)
        rows = [r for r in done.stdout.splitlines() if r.strip().startswith("1.")]
        self.assertEqual(len(rows), 1, done.stdout)
        self.assertEqual(rows[0].split()[-3:], ["12", "-12", "24"])
        self.assertNotIn("999", done.stdout)

    def test_a_scoring_run_still_works(self):
        """The control: the same stub without the bound line prints the table
        and exits 0, so the three refusals above are not a tool that refuses
        everything."""
        good = stub("      echo 'info depth 5 score cp 12 nodes 100 pv e2e4 e7e5'\n"
                    "      echo 'bestmove e2e4'\n")
        done = self.run_tool(good)
        self.assertEqual(done.returncode, 0, done.stderr)
        self.assertIn("cost", done.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
