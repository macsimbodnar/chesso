#!/usr/bin/env python3
"""The gate on tools/ledger.py, the SPRT ledger. S233, DEC-220.

TWO THINGS CAN BREAK IN IT and neither shows up anywhere else. The first is
the arithmetic over the seed: `adocs/plan.md`'s cost section is now that
script's output, so a wall time parsed wrong or a class counted twice moves
the mean, the median or a class mean silently, and every projection built on
them with it. The second is the parser: DEC-220's
block is hand-written into a commit message once per verdict, `git log` is the
only place that verdict then lives, and a parser that skips a line it does not
recognise loses a run instead of refusing to print.

So: the four figures and the row count over the seed file are goldens, and
every refusal the parser owes is a case. No build, no engine, no git call --
the figures run with `--no-git` so they stay put as commits land, and the
parser cases feed `parse_block` synthetic messages directly. Under a second.
"""

import os
import subprocess
import sys
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(ROOT, "tools", "ledger.py")
SEED = os.path.join(ROOT, "adocs", "data", "ledger_seed.tsv")

sys.path.insert(0, os.path.join(ROOT, "tools"))
import ledger  # noqa: E402  (after the path is set, which is the point)


# A whole block in DEC-220's shape, one line per key, so a case can drop, bend
# or repeat exactly one of them. Numbers are S098 v3 leg 1's, which is the run
# `adocs/data/S098_v3_leg1_sprt.log` holds and the shape the real thing takes.
BLOCK = [
    ("SPRT", "SPRT | cand 8d60551 vs ref efdbc9b, 8+0.08, Hash=16, "
             "noob_3moves.epd, {0, 5} nElo"),
    ("Elo", "Elo | 5.75 +/- 4.37, nElo 7.44 +/- 5.65"),
    ("LLR", "LLR | 2.97 (-2.94, 2.94) -> H1"),
    ("Games", "Games | N: 14510 W: 4559 L: 4319 D: 5632, "
              "Ptnml [603, 1695, 2522, 1729, 706]"),
    ("Wall", "Wall | 6 h 49 m, 2129.6 games/h, forfeits 0"),
    ("Log", "Log | adocs/data/S098_v3_leg1_sprt.log"),
]

SUBJECT = "Record S098 v3 leg 1's H1 for the deeper re-search path"


def body(skip=None, bend=None, repeat=None):
    lines = ["the prose of the commit, which the parser ignores", ""]
    for name, line in BLOCK:
        if name == skip:
            continue
        lines.append(bend if name == bend_name(bend) else line)
        if name == repeat:
            lines.append(line)
    lines.append("Co-Authored-By: nobody <nobody@example.invalid>")
    return "\n".join(lines)


def bend_name(bend):
    return bend.split(" |", 1)[0] if bend else None


class LedgerFigures(unittest.TestCase):
    """The seed's arithmetic, and plan.md's cost section with it."""

    # GOLDEN, all five. Re-derive with
    #     python3 tools/ledger.py --no-git --figures
    # and, for the row count,
    #     python3 tools/ledger.py --no-git --table | tail -n +3 | wc -l
    # `adocs/data/ledger_seed.tsv` is written once and never rewritten
    # (DEC-220), so these move only if the script's arithmetic moves -- which
    # is the whole reason they are here. DEC-142.
    ROWS = 20
    MEAN = "4 h 27 m"
    MEDIAN = "4 h 18 m"
    FAST_MEAN = "1 h 58 m"
    SLOW_MEAN = "6 h 56 m"
    # The totals the same paragraph carries, from the same run.
    TOTALS = "199259 games in 89.20 hours, 2233.9 an hour"

    @classmethod
    def setUpClass(cls):
        cls.figures = subprocess.run(
            [sys.executable, SCRIPT, "--no-git", "--figures"],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            universal_newlines=True, check=True).stdout.strip()
        cls.table = subprocess.run(
            [sys.executable, SCRIPT, "--no-git", "--table"],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            universal_newlines=True, check=True).stdout.rstrip("\n").split("\n")

    def test_the_seed_holds_exactly_twenty_rows(self):
        self.assertEqual(len(ledger.read_seed(SEED)), self.ROWS)
        self.assertEqual(len(self.table) - 2, self.ROWS)

    def test_the_mean_and_median_are_the_recorded_goldens(self):
        self.assertIn("mean %s" % self.MEAN, self.figures)
        self.assertIn("median %s" % self.MEDIAN, self.figures)

    def test_the_two_class_means_are_the_recorded_goldens(self):
        self.assertIn("mean **%s**" % self.FAST_MEAN, self.figures)
        self.assertIn("mean **%s**" % self.SLOW_MEAN, self.figures)

    def test_the_totals_are_the_recorded_golden(self):
        self.assertIn(self.TOTALS, self.figures)

    def test_every_seed_row_is_in_one_of_the_two_classes(self):
        rows = ledger.read_seed(SEED)
        fast = [row for row in rows if row["class"] == "fast"]
        slow = [row for row in rows if row["class"] == "slow"]
        self.assertEqual(len(fast) + len(slow), self.ROWS)
        self.assertEqual((len(fast), len(slow)), (10, 10))

    def test_the_table_is_markdown_with_the_ledger_column_order(self):
        self.assertEqual(
            self.table[0],
            "| run | what it measured | wall | games | bounds | verdict |")
        for row in self.table[2:]:
            self.assertEqual(row.count(" | "), 5, row)


class ClassRule(unittest.TestCase):
    """fast when the nElo interval misses the bounds pair, slow when it meets it."""

    def test_far_outside_is_fast(self):
        self.assertEqual(ledger.classify(56.76, 18.44, 0.0, 5.0), "fast")
        self.assertEqual(ledger.classify(-13.40, 10.16, 0.0, 5.0), "fast")

    def test_inside_is_slow(self):
        self.assertEqual(ledger.classify(1.48, 5.26, 0.0, 5.0), "slow")

    def test_an_interval_reaching_a_bound_is_slow(self):
        # The rule's whole point: "on a bound" is a statement about the truth,
        # so an estimate whose interval touches the pair is not outside it.
        self.assertEqual(ledger.classify(7.44, 5.65, 0.0, 5.0), "slow")
        self.assertEqual(ledger.classify(6.00, 1.00, 0.0, 5.0), "slow")
        self.assertEqual(ledger.classify(6.01, 1.00, 0.0, 5.0), "fast")


class BlockParser(unittest.TestCase):
    """The six lines in, one ledger row out -- or a loud refusal."""

    def test_a_whole_block_parses_to_its_fields(self):
        row = ledger.parse_block("abc1234", SUBJECT, body())
        self.assertEqual(row["run"], "S098 v3 leg 1")
        self.assertEqual(row["what"], "the deeper re-search path")
        self.assertEqual(row["wall"], "6 h 49 m")
        self.assertEqual(row["seconds"], 6 * 3600 + 49 * 60)
        self.assertEqual(row["games"], "14510")
        self.assertEqual(row["bounds"], "{0, 5}")
        self.assertEqual(row["verdict"], "H1, +5.75 +/- 4.37")
        self.assertEqual(row["nelo"], "7.44 +/- 5.65")
        self.assertEqual(row["class"], "slow")
        self.assertEqual(row["source"], "adocs/data/S098_v3_leg1_sprt.log")

    def test_no_verdict_prints_as_no_verdict(self):
        row = ledger.parse_block("abc1234", SUBJECT,
                                 body(bend="LLR | 0.41 (-2.94, 2.94) -> none"))
        self.assertEqual(row["verdict"], "**no verdict**")

    def test_a_missing_line_is_named(self):
        for name in ("Elo", "LLR", "Games", "Wall", "Log"):
            with self.assertRaises(ledger.LedgerError) as caught:
                ledger.parse_block("abc1234", SUBJECT, body(skip=name))
            self.assertIn("`%s |`" % name, str(caught.exception))

    def test_a_malformed_line_is_named(self):
        bad = [
            "SPRT | cand 8d60551 vs ref efdbc9b, 8+0.08, Hash=16, "
            "noob_3moves.epd, {0, 5}",              # no `nElo`
            "Elo | 5.75 +- 4.37, nElo 7.44 +/- 5.65",   # `+-`, not `+/-`
            "LLR | 2.97 (-2.94, 2.94) -> YES",          # not one of the three
            "Games | N: 14510 W: 4559 L: 4319 D: 5632, Ptnml [603, 1695]",
            "Wall | 6 h 49 m, 2129.6 games/h",          # no forfeit count
            "Log | /tmp/somewhere_else.log",            # not under adocs/data
        ]
        for line in bad:
            with self.assertRaises(ledger.LedgerError) as caught:
                ledger.parse_block("abc1234", SUBJECT, body(bend=line))
            self.assertIn("`%s |`" % bend_name(line), str(caught.exception))

    def test_a_repeated_line_is_refused(self):
        with self.assertRaises(ledger.LedgerError) as caught:
            ledger.parse_block("abc1234", SUBJECT, body(repeat="Games"))
        self.assertIn("two `Games |` lines", str(caught.exception))

    def test_a_subject_with_no_step_id_is_refused(self):
        with self.assertRaises(ledger.LedgerError) as caught:
            ledger.parse_block("abc1234", "Record the verdict", body())
        self.assertIn("no S<nnn> run id", str(caught.exception))


class SubjectRule(unittest.TestCase):
    """How the run id and the description come out of the commit subject."""

    CASES = [
        ("Record S231's H1 for the null child's two-ply key",
         "S231", "the null child's two-ply key"),
        ("Record S231's verdict: the null child's two-ply key",
         "S231", "the null child's two-ply key"),
        ("Record S098 v3 leg 1's H0 for the re-search depth rule",
         "S098 v3 leg 1", "the re-search depth rule"),
        ("Record S210 F22's verdict, quiescence and dead positions",
         "S210 F22", "quiescence and dead positions"),
        ("Record S024 v1's no verdict", "S024 v1", "Record S024 v1's no verdict"),
    ]

    def test_each_subject_reads_out_as_expected(self):
        for subject, run, what in self.CASES:
            self.assertEqual(ledger.split_run_and_what(subject), (run, what),
                             subject)


class WallParsing(unittest.TestCase):
    def test_every_shape_the_seed_carries(self):
        self.assertEqual(ledger.wall_seconds("2 h 44 m"), 9840)
        self.assertEqual(ledger.wall_seconds("1 h 37 m 52 s"), 5872)
        self.assertEqual(ledger.wall_seconds("37 m 51 s"), 2271)
        self.assertEqual(ledger.wall_seconds("13 h 16 m 30 s"), 47790)

    def test_a_wall_time_that_parses_to_nothing_is_refused(self):
        with self.assertRaises(ledger.LedgerError):
            ledger.wall_seconds("a while")

    def test_the_minute_is_truncated_not_rounded(self):
        # plan.md's mean and median were written that way by hand and the
        # script reproduces them; 4 h 27 m 36 s is the seed's own mean.
        self.assertEqual(ledger.hm(4 * 3600 + 27 * 60 + 36), "4 h 27 m")
        self.assertEqual(ledger.hm(3900), "1 h 05 m")


if __name__ == "__main__":
    unittest.main(verbosity=2)
