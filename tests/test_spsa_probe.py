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

S303 and S304 added what an engine does to its own output: closes it, cuts a
line, prints a count that is no number. `tools/depth_vs_eval.py` has no test file
of its own and the same device reaches it, so its `bestmove` cases are here too.
"""

import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import depth_vs_eval as dve  # noqa: E402
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

# What no formula reaches: an engine doing something to its own output. Answers
# `uci` with the axis and runs a case's own shell at `go`, with `$go` counting
# them. A stub that stays alive does it by `exec sleep`, bounded, so a case that
# fails cannot leave one behind for long. `read` and `exec 1>&-` are the same
# device S303's stand-ins used.
SH_STUB = r"""#!/bin/sh
echo $$ > {pidfile}
echo $$ >> {pidfile}.all
trap 'echo exit-time work >> {log}' EXIT
go=0
while read -r line; do
    case "$line" in
        uci) printf 'option name {axis} type spin default 0 min 0 max 1000\nuciok\n' ;;
        isready) printf 'readyok\n' ;;
        position*) echo "$line" >> {log} ;;
        go*)
            go=$((go + 1))
            {at_go}
            ;;
        quit)
            echo quit >> {log}
            exit 0
            ;;
    esac
done
"""

# Comfortably under the 30 s `probe_nodes` waits on an engine it did not kill, so
# a case that finishes inside this bound shows the kill and not the wait. A kill
# runs no shell trap, so `exit-time work` in a stub's log is its own exit.
KILLED_WITHIN_S = 15
STAYS_ALIVE = "exec sleep 45"


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

    def sh_engine(self, at_go, name="sh_engine"):
        """The shell stub, with its pid and every `position` it took in files."""
        path = os.path.join(self.dir, name)
        with open(path, "w") as fh:
            fh.write(SH_STUB.format(pidfile=path + ".pid", log=path + ".log",
                                    axis=AXIS, at_go=at_go))
        os.chmod(path, 0o755)
        return path

    def script(self, name, body):
        """A stub with no loop at all, its pid in the same file."""
        path = os.path.join(self.dir, name)
        with open(path, "w") as fh:
            fh.write(f"#!/bin/sh\necho $$ > {path}.pid\n{body}")
        os.chmod(path, 0o755)
        return path

    def gone(self, engine):
        """Whether the process a shell stub recorded has ended. `ps` and not
        `kill -0`, which a zombie answers, and a moment's grace: a killed process
        is reaped by whoever inherits it, on that process's schedule."""
        with open(engine + ".pid") as fh:
            pid = fh.read().strip()
        for _ in range(100):
            state = subprocess.run(["ps", "-o", "stat=", "-p", pid],
                                   capture_output=True, text=True).stdout.strip()
            if not state or state.startswith("Z"):
                return True
            time.sleep(0.05)
        return False

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


class MisbehavingOutput(ProbeCase):
    """`probe_nodes` on an engine that does not finish its answer. S303, S304.

    A partial count read as a measurement: an engine that died at its first
    `info` line passed the check that exists to prove an axis reaches the search.
    """

    PARTIAL = "printf 'info depth 1 nodes 1500\\n'; "

    def probe(self, engine):
        return sd.probe_nodes(engine, AXIS, 500, sd.PROBE_FEN, "go depth 9")

    def refused(self, at_go, *words):
        engine = self.sh_engine(at_go)
        started = time.monotonic()
        with self.assertRaises(sd.ConfigError) as ctx:
            self.probe(engine)
        for word in words:
            self.assertIn(word, str(ctx.exception))
        self.assertIn("go depth 9", str(ctx.exception))
        self.assertLess(time.monotonic() - started, KILLED_WITHIN_S)
        self.assertTrue(self.gone(engine), f"still alive after: {at_go}")

    def test_closing_without_bestmove_names_the_wait(self):
        """Exits, closes and reads on, closes and never reads again."""
        for end in ("exit 0", "exec 1>&-", f"exec 1>&-; {STAYS_ALIVE}"):
            with self.subTest(end=end):
                self.refused(self.PARTIAL + end, "before bestmove")

    def test_a_line_cut_before_its_number_is_not_a_stale_count(self):
        self.refused("printf 'info depth 1 nodes 900\\ninfo depth 2 nodes '; exec 1>&-",
                     "before bestmove")

    def test_a_count_that_is_no_number_is_refused_and_the_engine_killed(self):
        """int() takes `-5` and `1_000`, so it is digits and not int() that decides,
        and the engine stays alive after the line so only a kill ends it in time."""
        for count in ("abc", "-5", "1_000", "12x4"):
            with self.subTest(count=count):
                self.refused(f"printf 'info depth 1 nodes {count}\\n'; {STAYS_ALIVE}",
                             "no number", count)

    def test_bestmove_cut_before_its_word_ends_is_no_bestmove(self):
        self.refused(self.PARTIAL + "printf 'bestmov'; exec 1>&-", "before bestmove")

    def test_bestmove_cut_after_its_word_keeps_the_whole_count(self):
        """The one cut this lets through, on purpose: the count comes from a whole
        `info` line and the probe never reads the move."""
        for cut in ("bestmove", "bestmove e3", "bestmove e2e4 pon"):
            with self.subTest(cut=cut):
                engine = self.sh_engine(self.PARTIAL + f"printf '{cut}'; exec 1>&-")
                self.assertEqual(self.probe(engine), 1500)


class BestMoveLine(unittest.TestCase):
    """`depth_vs_eval.py`'s reading of the line, without a process. S304."""

    def test_a_whole_move_with_its_newline_is_read(self):
        for line, move in (("bestmove e2e4\n", "e2e4"),
                           ("bestmove e7e8q\n", "e7e8q"),
                           ("bestmove e2e4 ponder e7e5\n", "e2e4"),
                           ("bestmove e2e4 pon\n", "e2e4"),
                           ("bestmove  e2e4 \n", "e2e4")):
            with self.subTest(line=line):
                self.assertEqual(dve.BESTMOVE_RE.fullmatch(line).group(1), move)

    def test_anything_else_holds_no_move(self):
        """Without its newline a line was cut by end of file, however whole it
        reads: `e7e8` is what is left of `e7e8q`."""
        for line in ("bestmove", "bestmove\n", "bestmove e3", "bestmove e3\n",
                     "bestmove e2e4", "bestmove e7e8", "bestmove e2e4 pon",
                     "bestmove e7e8qq\n", "bestmove e2e9\n", "bestmove (none)\n",
                     "bestmove 0000\n", "bestmovee2e4\n"):
            with self.subTest(line=line):
                self.assertIsNone(dve.BESTMOVE_RE.fullmatch(line))


class DepthVsEvalCli(ProbeCase):
    """The tool end to end on one position: its first `go` answers whole, so the
    case sits at the deep search, where S301 traced the worst of it. S303, S304."""

    FIRST = 'if [ "$go" = 1 ]; then printf \'bestmove e3d4\\n\'; continue; fi; '
    # Opaque here: no stub parses a position or a move, it only logs them.
    FEN = "rn1qk2r/pp2ppbp/8/2pp1p2/3P4/1P2PQ1P/P1P2PP1/RN2KB1R w KQkq c6 0 9"

    def setUp(self):
        super().setUp()
        self.raw = os.path.join(self.dir, "raw.tsv")
        with open(self.raw, "w") as fh:
            fh.write("game\tply\tphase\tcost\tref\town\tmate\tsan\tfen\n")
            fh.write(f"0\t16\t20\t150\t-5\t-8\t0\tQxf5\t{self.FEN}\n")
        self.out = os.path.join(self.dir, "out.tsv")
        self.reference = self.sh_engine(
            "printf 'info depth 1 score cp 46 nodes 1 pv e2e4\\nbestmove e2e4\\n'",
            name="reference")

    def run_tool(self, engine, reference=None, workers=1, raw=None):
        # stderr to a file and not a pipe: an engine left behind inherits it, so
        # `capture_output` would wait out that engine's life before returning and
        # the case looking for it would find it gone. It did, on the old tool.
        with tempfile.TemporaryFile("w+") as err:
            done = subprocess.run(
                [sys.executable, os.path.join(ROOT, "tools", "depth_vs_eval.py"),
                 raw or self.raw, "--engine", engine,
                 "--reference", reference or self.reference,
                 "--base-nodes", "1000", "--deep-nodes", "2000",
                 "--reference-nodes", "500", "--workers", str(workers),
                 "--out", self.out],
                stdout=subprocess.PIPE, stderr=err, text=True, timeout=60)
            err.seek(0)
            done.stderr = err.read()
        return done

    def handed(self):
        """Every `position` the reference took: a refused move never reaches it."""
        return [line for line in self.log(self.reference)
                if line.startswith("position")]

    def log(self, stub):
        if not os.path.exists(stub + ".log"):
            return []
        with open(stub + ".log") as fh:
            return fh.read().splitlines()

    def starts(self, stub):
        """How many processes of a stub were started, over a whole run."""
        with open(stub + ".pid.all") as fh:
            return len(fh.read().splitlines())

    def stops(self, engine, said):
        done = self.run_tool(engine)
        self.assertEqual(done.returncode, 1, done.stdout + done.stderr)
        self.assertIn(said, done.stderr)
        self.assertFalse(os.path.exists(self.out), "a row was written")
        self.assertEqual(self.handed(), [])

    def test_a_bestmove_with_no_whole_move_stops_the_run(self):
        """`bestmove e3` used to exit 0 with `deep_move` `e3`, costed by a reference
        that answers, and bare `bestmove` died in an IndexError with no message."""
        for line in ("'bestmove e3'", "'bestmove'", "'bestmove e7e8'",
                     "'bestmove e3\\n'", "'bestmove (none)\\n'"):
            with self.subTest(line=line):
                engine = self.sh_engine(self.FIRST + f"printf {line}; exit 0")
                self.stops(engine, "waiting for bestmove")

    def test_closing_without_bestmove_stops_the_run(self):
        self.stops(self.sh_engine(self.FIRST + "exit 0"),
                   "closed its output before bestmove")

    def test_a_whole_move_is_a_row(self):
        """The control: the first `go`'s move and a promotion with a ponder word."""
        engine = self.sh_engine(self.FIRST + "printf 'bestmove e7e8q ponder e2e4\\n'")
        done = self.run_tool(engine)
        self.assertEqual(done.returncode, 0, done.stderr)
        with open(self.out) as fh:
            head, row = fh.read().splitlines()
        row = dict(zip(head.split("\t"), row.split("\t")))
        self.assertEqual((row["base_move"], row["deep_move"]), ("e3d4", "e7e8q"))
        self.assertEqual([p.split()[-1] for p in self.handed()], ["e3d4", "e7e8q"])
        self.assertTrue(self.gone(engine))

    def never_reads(self, name):
        """Closes its output and never looks at stdin, from its first instruction."""
        return self.script(name, f"exec 1>&-\n{STAYS_ALIVE}\n")

    def test_a_healthy_engine_takes_quit_and_ends_by_itself(self):
        """The ruling on S304's fast check: a kill here lost the tune build's census,
        which a destructor writes. Both sides, on a run that succeeds and on one that
        fails beside them."""
        engine = self.sh_engine("printf 'bestmove e3d4\\n'", name="healthy")
        self.assertEqual(self.run_tool(engine).returncode, 0)
        for stub in (engine, self.reference):
            self.assertEqual(self.log(stub)[-2:], ["quit", "exit-time work"])

        cut = self.sh_engine(self.FIRST + "printf 'bestmove e3\\n'", name="cut")
        self.assertEqual(self.run_tool(cut).returncode, 1)
        self.assertEqual(self.log(cut)[-2:], ["quit", "exit-time work"])

    def test_an_engine_that_stops_reading_does_not_outlive_the_tool(self):
        """Closing a worker's end of the pipe ends only an engine that reads it. Costs
        `QUIT_WAIT_S`, once, for the one that keeps its stdin. The reference beside it
        takes `quit`, so its kill would show in its log."""
        engine = self.sh_engine(self.FIRST + f"exec 1>&-; {STAYS_ALIVE}")
        self.assertEqual(self.run_tool(engine).returncode, 1)
        self.assertTrue(self.gone(engine))
        self.assertTrue(self.gone(self.reference))
        self.assertEqual(self.log(self.reference)[-2:], ["quit", "exit-time work"])

    def test_an_engine_that_closed_its_stdin_is_killed_without_the_wait(self):
        """`quit` has nowhere to go, so the write is what fails."""
        engine = self.sh_engine(self.FIRST + f"exec 0<&- 1>&-; {STAYS_ALIVE}")
        started = time.monotonic()
        done = self.run_tool(engine)
        self.assertEqual(done.returncode, 1)
        self.assertLess(time.monotonic() - started, dve.QUIT_WAIT_S)
        self.assertNotIn("BrokenPipeError", done.stderr)
        self.assertTrue(self.gone(engine))

    def test_a_worker_with_no_task_opens_no_engine(self):
        """Above one worker, and the engines open in the task. Back in the pool's
        initializer this starts three of each: S304's stated departure, the count, and
        its reason, below."""
        engine = self.sh_engine("printf 'bestmove e3d4\\n'", name="healthy")
        self.assertEqual(self.run_tool(engine, workers=3).returncode, 0)
        self.assertEqual((self.starts(engine), self.starts(self.reference)), (1, 1))

    def test_an_engine_dead_at_birth_fails_its_task_and_not_the_pool(self):
        """An initializer that raises breaks the pool, and the pool's SIGTERM ends the
        other workers past their Finalize. In a task the wait is what the run says."""
        engine = self.never_reads("never_reads")
        done = self.run_tool(engine, workers=3)
        self.assertEqual(done.returncode, 1)
        self.assertIn("before uciok", done.stderr)
        self.assertNotIn("BrokenProcessPool", done.stderr)
        self.assertTrue(self.gone(engine))

    def test_rows_do_not_depend_on_the_worker_count(self):
        """`workers.map` keeps the sample's order and the stubs answer alike, so this
        is the order and the first-task opening together. It says nothing about an
        engine with memory: the pending comparison's `--workers 3` section does."""
        raw = os.path.join(self.dir, "raw6.tsv")
        with open(raw, "w") as fh:
            fh.write("game\tply\tphase\tcost\tref\town\tmate\tsan\tfen\n")
            for game in range(6):
                fh.write(f"{game}\t16\t20\t150\t-5\t-8\t0\tQxf5\t{self.FEN}\n")
        engine = self.sh_engine("printf 'bestmove e3d4\\n'", name="healthy")
        tables = []
        for workers in (1, 3):
            self.assertEqual(self.run_tool(engine, workers=workers, raw=raw).returncode, 0)
            with open(self.out) as fh:
                tables.append(fh.read())
        self.assertEqual(tables[0], tables[1])
        self.assertEqual(len(tables[0].splitlines()), 7)

    def test_an_engine_that_never_read_does_not_outlive_the_tool(self):
        """Nothing holds a process whose constructor raised, on either side."""
        engine = self.never_reads("never_reads")
        done = self.run_tool(engine)
        self.assertEqual(done.returncode, 1)
        self.assertIn("before uciok", done.stderr)
        self.assertTrue(self.gone(engine))

        healthy = self.sh_engine("printf 'bestmove e3d4\\n'", name="healthy")
        reference = self.never_reads("never_reads_reference")
        done = self.run_tool(healthy, reference)
        self.assertEqual(done.returncode, 1)
        self.assertTrue(self.gone(reference))
        self.assertTrue(self.gone(healthy))


class Cli(ProbeCase):
    """One end-to-end pass, so the exit status and DEC-061's marker are covered
    and not only the function's return value."""

    def test_a_config_of_any_wrong_shape_ends_check_in_the_marker(self):
        """S304's fast check: the load took `OSError`, `ValueError` and `ConfigError`,
        so these died in a traceback with no marker. The last is the one that loads
        and falls over later, in `warnings()`."""
        one = '[{"name": "A", "start": 5, "min": 0, "max": 10, "c_end": 1}]'
        for text in ("[]", "5", '{"iterations": null, "pairs_per_iter": 1, "params": %s}' % one,
                     '{"iterations": 5, "pairs_per_iter": 1, "params": '
                     '[{"name": "A", "start": null, "min": 0, "max": 10, "c_end": 1}]}',
                     '{"iterations": 5, "pairs_per_iter": 1, "params": 5}',
                     '{"iterations": 5, "pairs_per_iter": 1, "params": [5]}',
                     '{"iterations": 1e400, "pairs_per_iter": 1, "params": %s}' % one,
                     '{"iterations": 1%s, "pairs_per_iter": 1, "params": %s}' % ("0" * 400, one)):
            with self.subTest(config=text[:60]):
                path = os.path.join(self.dir, "bad.json")
                with open(path, "w") as fh:
                    fh.write(text)
                done = subprocess.run(
                    [sys.executable, os.path.join(ROOT, "tools", "spsa_driver.py"),
                     "check", path], capture_output=True, text=True, timeout=60)
                self.assertEqual(done.returncode, 1, done.stdout)
                lines = done.stdout.splitlines()
                self.assertEqual(lines[-1], "SPSA-FAILED", done.stdout + done.stderr)
                self.assertTrue(lines[-2].startswith("ERROR: "), done.stdout)
                self.assertNotIn("Traceback", done.stderr)

    def run_check(self, engine):
        cfg_path = os.path.join(self.dir, "config.json")
        with open(cfg_path, "w") as fh:
            json.dump(self.config().raw, fh)
        return subprocess.run(
            [sys.executable, os.path.join(ROOT, "tools", "spsa_driver.py"),
             "check", cfg_path, "--engine", engine],
            capture_output=True, text=True, timeout=120)

    def test_a_dead_axis_fails_the_command_with_the_marker(self):
        done = self.run_check(self.engine(DEAD))
        self.assertEqual(done.returncode, 1, done.stdout)
        self.assertIn(f"PROBLEM: {AXIS}", done.stdout)
        self.assertTrue(done.stdout.strip().endswith("SPSA-FAILED"), done.stdout)

    def test_a_live_axis_passes_the_command_with_the_marker(self):
        done = self.run_check(self.engine(LIVE))
        self.assertEqual(done.returncode, 0, done.stdout)
        self.assertTrue(done.stdout.strip().endswith("SPSA-DONE"), done.stdout)

    def test_a_raise_inside_check_ends_in_the_marker_and_not_a_traceback(self):
        """A traceback is not a line a watcher exits on, DEC-061. Every way out of
        `check()` that is not a list of problems, S304: its own ConfigErrors, and
        the OSError of an engine that is not there."""
        closes = self.sh_engine("printf 'info depth 1 nodes 1500\\n'; exec 1>&-")
        malformed = self.sh_engine("printf 'info depth 1 nodes abc\\n'; exec 1>&-",
                                   name="malformed")
        lists_nothing = self.script("lists_nothing", "exit 0\n")
        for engine, said in ((closes, "before bestmove"), (malformed, "no number"),
                             (lists_nothing, "no spin options"),
                             (os.path.join(self.dir, "absent"), "No such file")):
            with self.subTest(said=said):
                done = self.run_check(engine)
                self.assertEqual(done.returncode, 1, done.stdout)
                lines = done.stdout.splitlines()
                self.assertEqual(lines[-1], "SPSA-FAILED", done.stdout)
                self.assertTrue(lines[-2].startswith("ERROR: "), done.stdout)
                self.assertIn(said, lines[-2])
                self.assertNotIn("Traceback", done.stderr)


if __name__ == "__main__":
    unittest.main(verbosity=2)
