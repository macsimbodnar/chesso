#!/usr/bin/env python3
"""The gate on `tools/spsa_driver.py`'s reachability probe. S214.

WHY A STUB ENGINE. `check`'s probe answers one question -- does a `setoption`
this run will send reach the search -- and it answers it from node counts alone,
because an accepted option has no readback. What is under test is therefore what
the driver does with a sequence of node counts, and the sequences that matter
are ones no correct engine produces on demand: a count that does not move with
the option (a dead axis), and a count that moves on its own (a jittery one). A
stub that answers `uci` and `go` from a formula produces each in a line and
costs milliseconds; probing the real binary would test the engine instead and
could not reach either case.

The stub counts its own `go` commands into a file, so a case can make the count
a function of *which call this is* rather than of the option -- which is what
machine jitter looks like from the driver's side, and is the case the guard
under test exists to reject.

WHAT WENT WRONG BEFORE THIS FILE, all three found by the Tier-1 fast check over
S214's own diff:

1. The jitter guard was one-sided. A clock rung re-ran the low bound and
   required it to repeat; the high bound was measured exactly once. A dead axis
   whose clock count is bimodal then passes whenever the two low samples land on
   one mode and the single high sample lands on the other -- `p(1-p)` per axis,
   21 % at a 70/30 split, over the nine `Tm*` axes only the clock rung reaches.
2. The failure message stated a measurement that had not happened: a rung
   rejected for jitter fell out of the loop and was reported as "both searched
   N nodes ... and every probe before it agreed", neither of which was true.
3. An axis whose bounds round to the same UCI value was probed twice at that
   one value and then failed by name as unreachable, when the fault is the
   config.
"""

import contextlib
import io
import json
import os
import shutil
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import spsa_driver as sd  # noqa: E402

AXIS = "Axis"

# A UCI engine reduced to what the probe reads: the option listing, and one
# `info ... nodes N` line per `go`. NODES is the case's own expression, over
# `value` (the option as last set), `go` (the whole command, so a case can be
# deterministic under `go depth` and not under a clock, which is the real
# engine's behaviour) and `call` (how many `go` commands came before this one,
# across processes -- one probe is one process).
STUB = '''#!{python}
import os
import sys

OPTIONS = {options}
VALUES = {{name: spec[0] for name, spec in OPTIONS.items()}}
COUNTER = {counter!r}


def calls():
    """`go` commands so far, counted across processes. The driver spawns one
    engine per probe, so nothing in memory survives to the next sample."""
    seen = 0
    if os.path.exists(COUNTER):
        with open(COUNTER) as fh:
            seen = int(fh.read() or 0)
    with open(COUNTER, "w") as fh:
        fh.write(str(seen + 1))
    return seen


def nodes(go, call):
    value = VALUES[{axis!r}]
    return int({expr})


while True:
    line = sys.stdin.readline()
    if not line:
        break
    line = line.strip()
    if line == "uci":
        print("id name spsa-probe-stub")
        for name, (default, lo, hi) in OPTIONS.items():
            print("option name %s type spin default %d min %d max %d"
                  % (name, default, lo, hi))
        print("uciok", flush=True)
    elif line.startswith("setoption name "):
        name, _, value = line[len("setoption name "):].partition(" value ")
        VALUES[name.strip()] = int(value)
    elif line.startswith("go"):
        print("info depth 1 score cp 0 nodes %d pv e2e4" % nodes(line, calls()))
        print("bestmove e2e4", flush=True)
    elif line == "quit":
        break
'''

# The four node-count shapes, as expressions over `value`, `go` and `call`.
LIVE = "1000 + value"
DEAD = "4242"
# Deterministic under a fixed depth, bimodal under a clock, and never a
# function of the option: a dead axis on a machine whose clock probe lands on
# one of two values. Nothing here reads `value`.
JITTERY_DEAD = "4242 if 'depth' in go else (7000 if call % 2 == 0 else 9000)"
# The eight `Tm*` axes: invisible to every fixed-depth probe, live on a clock.
CLOCK_ONLY = "4242 if 'depth' in go else 1000 + value"


class ProbeCase(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp(prefix="chesso-spsa-probe.")
        self.addCleanup(shutil.rmtree, self.dir, ignore_errors=True)
        self.counter = os.path.join(self.dir, "go_calls")

    def engine(self, expr, lo=0, hi=1000, default=0, name=AXIS):
        path = os.path.join(self.dir, "engine.py")
        with open(path, "w") as fh:
            fh.write(STUB.format(
                python=sys.executable,
                options=repr({name: (default, lo, hi)}),
                counter=self.counter, axis=name, expr=expr))
        os.chmod(path, 0o755)
        return path

    def go_calls(self):
        """How many `go` commands the stub answered. 0 proves an axis was
        refused before any probe, which is what a config fault must do."""
        if not os.path.exists(self.counter):
            return 0
        with open(self.counter) as fh:
            return int(fh.read() or 0)

    def config(self, lo=0, hi=1000, start=500, c_end=50.0, name=AXIS):
        return sd.Config({"iterations": 10, "pairs_per_iter": 1, "params": [
            {"name": name, "start": start, "min": lo, "max": hi,
             "c_end": c_end}]})

    def check(self, engine, cfg):
        """`check`'s problems, with its own reporting captured rather than
        printed through the test runner's output."""
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            problems = sd.check(cfg, engine)
        return problems, out.getvalue()


class Separation(ProbeCase):
    def test_a_live_axis_separates_at_the_first_rung(self):
        """The cheap case, and the one 19 of the engine's 28 axes take: the
        node count is a function of the option at depth 9, so nothing below the
        first rung is ever run."""
        got = sd.probe_axis(self.engine(LIVE), self.config().params[0])
        self.assertTrue(got.separated, got.rungs)
        self.assertEqual(got.label, sd.PROBES[0][0])
        self.assertEqual((got.lo_nodes, got.hi_nodes), (1000, 2000))
        self.assertEqual(self.go_calls(), 2)

    def test_a_clock_only_axis_separates_on_the_clock(self):
        """The nine `Tm*` axes: nothing under `go depth` consults a clock, so
        every fixed-depth rung reports the same count and the clock rung is the
        only one that can see the parameter. The confirmation the jittery case
        below demands must not reject this one."""
        got = sd.probe_axis(self.engine(CLOCK_ONLY), self.config().params[0])
        self.assertTrue(got.separated, got.rungs)
        self.assertEqual(got.label, sd.PROBES[-1][0])
        self.assertEqual([r.verdict for r in got.rungs],
                         ["identical"] * (len(sd.PROBES) - 1) + ["separated"])

    def test_a_dead_axis_separates_nowhere(self):
        """The axis the check exists to catch: an option the binary accepts and
        no code reads. Every rung is tried and every rung agrees."""
        got = sd.probe_axis(self.engine(DEAD), self.config().params[0])
        self.assertFalse(got.separated)
        self.assertEqual([r.verdict for r in got.rungs],
                         ["identical"] * len(sd.PROBES))
        self.assertEqual(self.go_calls(), 2 * len(sd.PROBES))


class Jitter(ProbeCase):
    """Finding 1 of the fast check over S214.

    The guard re-ran the low bound only, so one unconfirmed high sample decided
    the verdict. A bimodal dead axis passed as reachable with probability
    `p(1-p)` -- 21 % at 70/30, and 25 % at the worst case -- per axis, over the
    nine axes only this rung reaches.
    """

    def test_a_bimodal_dead_axis_is_not_repeatable_never_reachable(self):
        got = sd.probe_axis(self.engine(JITTERY_DEAD), self.config().params[0])
        self.assertFalse(
            got.separated,
            "a node count that alternates on its own is the machine, not the "
            f"parameter: {got.rungs}")
        self.assertEqual(got.rungs[-1].verdict, "not repeatable")
        self.assertEqual([r.verdict for r in got.rungs[:-1]],
                         ["identical"] * (len(sd.PROBES) - 1))

    def test_both_bounds_are_confirmed_and_not_just_the_low_one(self):
        """The mechanism, stated separately from the verdict: whichever bound
        the jitter shows up at, the rung is rejected. A guard that confirms one
        bound passes this axis whenever the jitter lands on the other."""
        cfg = self.config()
        engine = self.engine(JITTERY_DEAD)
        # The alternation's phase is the number of `go` commands already made,
        # so priming it by one moves the disagreement from the low bound's
        # re-run to the high bound's.
        for prime in (0, 1):
            with open(self.counter, "w") as fh:
                fh.write(str(prime))
            got = sd.probe_axis(engine, cfg.params[0])
            self.assertFalse(got.separated, f"phase {prime}: {got.rungs}")
            self.assertEqual(got.rungs[-1].verdict, "not repeatable")


class FailureMessage(ProbeCase):
    """Finding 2 of the fast check over S214.

    A rung rejected for jitter fell out of the loop and `check` reported it as
    "both searched N nodes ... and every probe before it agreed". Neither clause
    was true, and the two failures need different answers from a reader: a dead
    axis is a wiring bug, an unrepeatable one is a machine that was busy.
    """

    def test_a_jittery_rung_is_not_reported_as_an_identical_one(self):
        problems, _ = self.check(self.engine(JITTERY_DEAD), self.config())
        self.assertEqual(len(problems), 1, problems)
        said = problems[0]
        self.assertIn(AXIS, said)
        self.assertIn("not repeatable", said)
        # The headline is what a reader acts on, and the old one asserted a
        # measurement that had not happened. The rungs that really did agree
        # may still say so below it.
        self.assertNotIn("both searched", said.splitlines()[0])
        self.assertNotIn("every probe before it agreed", said)
        # What actually happened, rung by rung: three that agreed, one that
        # separated once and did not repeat.
        for label, _fen, _go, _stable in sd.PROBES:
            self.assertIn(label, said)
        self.assertIn("7000", said)
        self.assertIn("9000", said)

    def test_a_dead_axis_is_reported_as_one(self):
        """The other half: when every rung really did search identically, the
        message says so and does not talk about repeatability."""
        problems, _ = self.check(self.engine(DEAD), self.config())
        self.assertEqual(len(problems), 1, problems)
        said = problems[0]
        self.assertIn("4242", said)
        self.assertNotIn("not repeatable", said)
        for label, _fen, _go, _stable in sd.PROBES:
            self.assertIn(label, said)

    def test_a_reachable_axis_is_reported_with_the_rung_that_reached_it(self):
        problems, printed = self.check(self.engine(LIVE), self.config())
        self.assertEqual(problems, [])
        self.assertIn("setoption reaches the search", printed)
        self.assertIn(sd.PROBES[0][0], printed)


class CollapsedBounds(ProbeCase):
    """Finding 4 of the fast check over S214.

    `uci_value` clamps and rounds, so bounds a hair apart are one integer by the
    time they are sent. Both probes then measure the same engine and the axis is
    failed by name as one the search never sees, when the fault is the config
    and no probe could have said otherwise.
    """

    def test_bounds_that_round_together_are_refused_before_any_probe(self):
        cfg = self.config(lo=1.2, hi=1.4, start=1.3, c_end=0.5)
        problems, _ = self.check(self.engine(LIVE, lo=1, hi=1, default=1), cfg)
        self.assertEqual(len(problems), 1, problems)
        self.assertIn(AXIS, problems[0])
        self.assertIn("collapse", problems[0])
        self.assertNotIn("random-walk", problems[0])
        self.assertEqual(self.go_calls(), 0,
                         "a collapsed axis must not be probed: both probes are "
                         "the same probe and prove nothing")

    def test_the_bounds_comparison_rounds_the_way_the_probe_does(self):
        """The adjacent inconsistency: the comparison truncated where the probe
        rounds, so `min 0.6` was held against the binary as 0 and sent as 1. The
        two must be one conversion or the message names a value nothing uses."""
        cfg = self.config(lo=0.6, hi=1.4, start=1.0, c_end=0.5)
        problems, _ = self.check(self.engine(LIVE, lo=1, hi=1, default=1), cfg)
        self.assertEqual(len(problems), 1, problems)
        self.assertIn("collapse", problems[0])
        self.assertNotIn("[0, 1]", problems[0])

    def test_a_mismatched_range_still_reports_the_binary_s(self):
        """The control: bounds that do not collapse and do not match the binary
        are still the pre-existing complaint, quoted in the values that are
        actually sent."""
        cfg = self.config(lo=10, hi=500)
        problems, _ = self.check(self.engine(LIVE, lo=0, hi=1000), cfg)
        self.assertEqual(len(problems), 1, problems)
        self.assertIn("[10, 500]", problems[0])
        self.assertIn("[0, 1000]", problems[0])
        self.assertEqual(self.go_calls(), 0)


class Cli(ProbeCase):
    """One end-to-end pass, so the exit status and DEC-061's marker are covered
    and not only the function's return value."""

    def test_a_dead_axis_fails_the_command_with_the_marker(self):
        import subprocess
        cfg_path = os.path.join(self.dir, "config.json")
        with open(cfg_path, "w") as fh:
            json.dump(self.config().raw, fh)
        done = subprocess.run(
            [sys.executable, os.path.join(ROOT, "tools", "spsa_driver.py"),
             "check", cfg_path, "--engine", self.engine(DEAD)],
            capture_output=True, text=True, timeout=120)
        self.assertEqual(done.returncode, 1, done.stdout)
        self.assertIn(f"PROBLEM: {AXIS}", done.stdout)
        self.assertTrue(done.stdout.strip().endswith("SPSA-FAILED"), done.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
