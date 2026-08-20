#!/usr/bin/env python3
"""The gate on tools/spsa_driver.py. S084.

WHY A SYNTHETIC OBJECTIVE AND NOT GAMES. A driver tested by playing games cannot
tell a bug in itself from noise in the objective at any budget this machine can
afford: a sign error, a decayed-to-nothing step and a parameter that does not
matter all produce the same flat trajectory. A quadratic with a known optimum
costs a second, and every one of those three fails it loudly.

WHAT THE SYNTHETIC OBJECTIVE IS NOT. It is deliberately steeper than a real
search parameter -- 225 Elo between the start point and the optimum, where a
real parameter is worth a few. That is a property of the gate, not a claim about
the engine: the point is to resolve the iteration's correctness in a second of
CPU, and a realistic Elo scale would need fishtest's budget to resolve anything
at all. S085's budget is set from the real objective, and nothing here predicts
it.

THE CONSTANTS ARE MEASURED, NOT INHERITED (DEC-084). The convergence run below
uses r_end = 0.008 because 0.002 -- the seed value from OpenBench and the
fishtest era -- moves this objective 2 % of the way to its optimum in 20000
pairs and would pass no criterion worth writing:

    r_end 0.002  max axis error 293 of 300   final strength -21.8 Elo
    r_end 0.008  max axis error  28 of 300   final strength  -0.9 Elo
    r_end 0.050  max axis error 221 of 300   final strength  -6.2 Elo

The last row is fishtest RFC #535's operational complaint measured here: an
oversized end value does not decay away, because the end-value parametrisation
means the step size only shrinks by about 1.6x across the whole run.

The tolerances are the distribution's, not this seed's. Over seeds 1 to 10 the
worst axis error was 65.9 against the 75 the criterion allows, and the worst
final strength -3.96 Elo against the 5 allowed. A tolerance that only held for
one seed would break the day anything changes how many random numbers an
iteration draws.
"""

import json
import os
import subprocess
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import spsa_driver as sd  # noqa: E402

DRIVER = os.path.join(ROOT, "tools", "spsa_driver.py")

# One objective, used by every convergence case here. Five parameters on [0,
# 1000] starting at 500, with the optimum 300 off in alternating directions so a
# driver that moves every axis the same way cannot pass.
RANGE_HI = 1000.0
START = 500.0
OFFSETS = [300.0, -300.0, 300.0, -300.0, 300.0]
OPTIMUM = [START + o for o in OFFSETS]
INITIAL_ERROR = 300.0

SIM = {"optimum": OPTIMUM, "weights": [500.0] * 5, "sigmas": [1000.0] * 5,
       "draw_rate": 0.5}


def config(iterations=5000, pairs=4, r_end=0.008, c_end=50.0, seed=1,
           hi=RANGE_HI, count=5):
    return sd.Config({
        "iterations": iterations, "pairs_per_iter": pairs, "seed": seed,
        "r_end": r_end,
        "params": [{"name": f"P{i}", "start": START, "min": 0, "max": hi,
                    "c_end": c_end} for i in range(count)]})


def tail_mean(rundir, count, fraction=0.1):
    """Mean of each parameter column over the last `fraction` of iterations.

    A single final value is one draw from a random walk; SPSA's answer is the
    neighbourhood it settled in, which is what fishtest reads too."""
    with open(os.path.join(rundir, "trajectory.tsv")) as fh:
        rows = fh.read().splitlines()[1:]
    tail = rows[-max(1, int(len(rows) * fraction)):]
    cols = [[float(r.split("\t")[-count + i]) for r in tail] for i in range(count)]
    return [sum(c) / len(c) for c in cols]


class TestSchedule(unittest.TestCase):
    def test_end_values_are_reached_exactly(self):
        """The whole point of the end-value parametrisation: c_end and r_end are
        what the schedule holds at the last iteration, so they mean something
        without re-deriving Spall's constants per iteration count."""
        for iterations in (1, 2, 100, 5000):
            cfg = config(iterations=iterations)
            c_scale, r_k = sd.schedule(iterations - 1, cfg)
            self.assertAlmostEqual(c_scale, 1.0, places=12)
            self.assertAlmostEqual(r_k, cfg.r_end, places=12)

    def test_c_decays_and_r_grows(self):
        """c_k falls with gamma = 0.101 and r_k rises, because a_k decays faster
        than c_k squared. A driver whose step size collapsed early would look
        converged; this is the shape that keeps it moving."""
        cfg = config(iterations=5000)
        first_c, first_r = sd.schedule(0, cfg)
        last_c, last_r = sd.schedule(4999, cfg)
        self.assertGreater(first_c, last_c)
        self.assertLess(first_r, last_r)


class TestConfigRefusals(unittest.TestCase):
    def test_integer_c_end_below_half_is_refused(self):
        """round(x + 0.4) == round(x - 0.4) for most x, so the axis would be
        perturbed and never move. OpenBench enforces the same floor."""
        with self.assertRaises(sd.ConfigError) as ctx:
            config(c_end=0.4)
        self.assertIn("0.5", str(ctx.exception))
        config(c_end=0.5)  # the boundary itself is allowed

    def test_start_outside_bounds_is_refused(self):
        with self.assertRaises(sd.ConfigError):
            sd.Config({"iterations": 10, "pairs_per_iter": 1, "params": [
                {"name": "P", "start": 2000, "min": 0, "max": 1000, "c_end": 8}]})

    def test_inverted_bounds_are_refused(self):
        with self.assertRaises(sd.ConfigError):
            sd.Config({"iterations": 10, "pairs_per_iter": 1, "params": [
                {"name": "P", "start": 5, "min": 10, "max": 1, "c_end": 8}]})

    def test_duplicate_parameter_is_refused(self):
        with self.assertRaises(sd.ConfigError):
            sd.Config({"iterations": 10, "pairs_per_iter": 1, "params": [
                {"name": "P", "start": 5, "min": 0, "max": 10, "c_end": 8},
                {"name": "P", "start": 5, "min": 0, "max": 10, "c_end": 8}]})

    def test_narrow_range_warns(self):
        """Not fatal, but the early gradient is noise: theta+c and theta-c both
        clamp to the same bound, so the pair measures the same engine twice."""
        cfg = sd.Config({"iterations": 5000, "pairs_per_iter": 1, "params": [
            {"name": "P", "start": 500, "min": 460, "max": 540, "c_end": 50}]})
        self.assertTrue(any("narrower" in w for w in cfg.warnings()))
        self.assertEqual(config().warnings(), [])


class TestConvergence(unittest.TestCase):
    def test_converges_to_the_known_optimum(self):
        with tempfile.TemporaryDirectory() as d:
            cfg = config()
            sd.run(cfg, d, sim_spec=SIM)
            means = tail_mean(d, len(cfg.params))
            errors = [abs(m - o) for m, o in zip(means, OPTIMUM)]
            obj = sd.SimObjective(SIM, cfg.params, None)
            for i, err in enumerate(errors):
                self.assertLess(err, 0.25 * INITIAL_ERROR,
                                f"axis {i} ended {err:.1f} from the optimum, "
                                f"having started {INITIAL_ERROR:.0f} away")
            self.assertGreater(obj.strength(means), -5.0)
            self.assertAlmostEqual(obj.strength(OPTIMUM), 0.0, places=12)

    def test_flipped_sign_fails_the_same_criterion(self):
        """The reason the gate exists. A downhill run looks exactly like a run:
        same trajectory file, same schedules, same pair scores. Only the
        objective knows, so the same criterion is asserted to FAIL here -- a
        driver that passed both would be measuring nothing."""
        with tempfile.TemporaryDirectory() as d:
            cfg = config()
            sd.run(cfg, d, sim_spec=SIM, flip_sign=True)
            means = tail_mean(d, len(cfg.params))
            errors = [abs(m - o) for m, o in zip(means, OPTIMUM)]
            obj = sd.SimObjective(SIM, cfg.params, None)
            self.assertTrue(all(e > 0.25 * INITIAL_ERROR for e in errors),
                            f"a flipped update still converged: {errors}")
            self.assertLess(obj.strength(means), obj.strength([START] * 5),
                            "a flipped update ended no worse than it started")


class TestClamp(unittest.TestCase):
    def test_optimum_outside_a_bound_pins_at_the_bound(self):
        """An out-of-range setoption is refused rather than clamped by the
        engine. S137 makes the refusal audible over UCI, but only after the value
        was sent, so an unclamped driver still plays its games against a
        silently-default parameter. Every recorded value stays in range and the
        axis whose optimum is unreachable sits on its bound."""
        sim = dict(SIM)
        sim["optimum"] = [1400.0] + OPTIMUM[1:]
        with tempfile.TemporaryDirectory() as d:
            cfg = config(iterations=2000)
            theta = sd.run(cfg, d, sim_spec=sim)
            # The float theta and not only what was sent. A clamp applied at the
            # UCI boundary alone lets the internal value wind up thousands
            # outside its range while the trajectory prints a tidy bound, and
            # the axis then sits unresponsive for as many iterations as it took
            # to drift out.
            for i, p in enumerate(cfg.params):
                self.assertGreaterEqual(theta[i], p.lo)
                self.assertLessEqual(theta[i], p.hi)
            with open(os.path.join(d, "trajectory.tsv")) as fh:
                rows = fh.read().splitlines()[1:]
            count = len(cfg.params)
            for row in rows:
                for i, cell in enumerate(row.split("\t")[-count:]):
                    self.assertGreaterEqual(float(cell), cfg.params[i].lo)
                    self.assertLessEqual(float(cell), cfg.params[i].hi)
            self.assertGreater(tail_mean(d, count)[0], 0.95 * RANGE_HI)

    def test_perturbations_are_clamped_too(self):
        """theta can sit inside its range while theta+c does not. Both sides of
        the pair are what gets sent, so both are what has to be legal."""
        p = sd.Param({"name": "P", "start": 1000, "min": 0, "max": 1000,
                      "c_end": 50})
        self.assertEqual(p.uci_value(1000 + 136.0), 1000)
        self.assertEqual(p.uci_value(-4.0), 0)
        self.assertEqual(p.uci_value(74.6), 75)


class TestMatchParsing(unittest.TestCase):
    # Verbatim from `fastchess alpha 1.8.2 20260729-74deac2`, the installed
    # build, playing build-tune against itself at tc=1+0.01 on 2026-08-20. A
    # parser tested against a hand-written string tests the hand.
    OUTPUT = """Started game 1 of 4 (plus vs minus)
Finished game 2 (minus vs plus): 0-1 {Black mates}
--------------------------------------------------
Results of plus vs minus (1+0.01, 1t, 16MB, 8moves_v3.pgn):
Elo: -0.00 +/- 296.58, nElo: 0.00 +/- 340.48
LOS: 50.00 %, DrawRatio: 0.00 %, PairsRatio: 1.00
Games: 4, Wins: 1, Losses: 1, Draws: 2, Points: 2.0 (50.00 %)
Ptnml(0-2): [0, 1, 0, 1, 0], WL/DD Ratio: -nan
--------------------------------------------------
Finished match
"""

    def test_parses_wins_losses_and_pentanomial(self):
        res = sd.parse_match(self.OUTPUT)
        self.assertEqual((res.wins, res.losses, res.draws), (1, 1, 2))
        self.assertEqual(res.ptnml, [0, 1, 0, 1, 0])
        self.assertEqual(res.y, 0)
        res.check(pairs=2)

    def test_missing_games_are_refused(self):
        """A crashed game comes back as a smaller match, which shrinks the
        gradient rather than announcing itself."""
        with self.assertRaises(sd.RunError):
            sd.parse_match(self.OUTPUT).check(pairs=3)

    def test_no_result_block_is_refused(self):
        with self.assertRaises(sd.RunError):
            sd.parse_match("Started game 1 of 4\nsomething went wrong\n")

    def test_sign_is_the_first_engine(self):
        """theta+ is the first -engine, so Wins is theta+'s. Swapping these two
        is the same bug as flipping the update and is invisible in the same
        way."""
        text = self.OUTPUT.replace("Wins: 1, Losses: 1", "Wins: 3, Losses: 0")
        res = sd.parse_match(text)
        self.assertEqual(res.y, 3)


class TestPairScoreDistribution(unittest.TestCase):
    def test_pentanomial_accounts_for_every_pair(self):
        cfg = config()
        rng = __import__("random").Random(7)
        obj = sd.SimObjective(SIM, cfg.params, rng)
        res = obj.play([500] * 5, [500] * 5, 500, 0)
        self.assertEqual(sum(res.ptnml), 500)
        self.assertEqual(res.wins + res.losses + res.draws, 1000)

    def test_a_stronger_plus_scores_more(self):
        """Non-vacuous by construction: the simulator has to be able to tell the
        two sides apart before any convergence result means anything."""
        cfg = config()
        rng = __import__("random").Random(7)
        obj = sd.SimObjective(SIM, cfg.params, rng)
        at_optimum = obj.play([int(x) for x in OPTIMUM], [500] * 5, 2000, 0)
        self.assertGreater(at_optimum.y, 0)


class TestResume(unittest.TestCase):
    """A run of thousands of pairs cannot start over because it was interrupted.

    Both interruption points are covered. An iteration appends its trajectory
    row and then writes its checkpoint, so a kill in between leaves a row the
    checkpoint does not know about -- resume drops it, which is what makes the
    resumed file byte-identical rather than merely correct.
    """

    ITERATIONS = 200

    def _write(self, d, **kw):
        cfg = config(iterations=self.ITERATIONS, pairs=1, **kw)
        cfg_path = os.path.join(d, "config.json")
        sim_path = os.path.join(d, "sim.json")
        with open(cfg_path, "w") as fh:
            json.dump(cfg.raw, fh)
        with open(sim_path, "w") as fh:
            json.dump(SIM, fh)
        return cfg_path, sim_path

    def _run(self, args, expect=0):
        proc = subprocess.run([sys.executable, DRIVER] + args,
                              capture_output=True, text=True, timeout=300)
        self.assertEqual(proc.returncode, expect,
                         f"{args}\n{proc.stdout}\n{proc.stderr}")
        return proc

    def _reference(self, d):
        cfg_path, sim_path = self._write(d)
        out = os.path.join(d, "whole")
        proc = self._run(["run", cfg_path, "--out", out, "--sim", sim_path])
        self.assertTrue(proc.stdout.strip().endswith("SPSA-DONE"))
        with open(os.path.join(out, "trajectory.tsv"), "rb") as fh:
            return cfg_path, sim_path, fh.read()

    def _resumed(self, d, phase):
        cfg_path, sim_path, whole = self._reference(d)
        out = os.path.join(d, "killed_" + phase)
        # os._exit inside the driver, so the exit status is the kill's.
        self._run(["run", cfg_path, "--out", out, "--sim", sim_path,
                   "--crash-after", "77", "--crash-phase", phase], expect=9)
        with open(os.path.join(out, "checkpoint.json")) as fh:
            self.assertEqual(json.load(fh)["k"], 78 if phase == "checkpoint" else 77)
        self._run(["run", cfg_path, "--out", out, "--sim", sim_path, "--resume"])
        with open(os.path.join(out, "trajectory.tsv"), "rb") as fh:
            self.assertEqual(fh.read(), whole,
                             f"resume after a {phase} kill diverged")

    def test_resume_after_checkpoint_kill(self):
        with tempfile.TemporaryDirectory() as d:
            self._resumed(d, "checkpoint")

    def test_resume_after_trajectory_kill(self):
        with tempfile.TemporaryDirectory() as d:
            self._resumed(d, "trajectory")

    def test_resume_under_a_changed_config_is_refused(self):
        """Continuing a checkpoint under different bounds or a different
        schedule would splice two runs into one trajectory and the file would
        not say so."""
        with tempfile.TemporaryDirectory() as d:
            cfg_path, sim_path = self._write(d)
            out = os.path.join(d, "run")
            self._run(["run", cfg_path, "--out", out, "--sim", sim_path,
                       "--crash-after", "20"], expect=9)
            with open(cfg_path) as fh:
                raw = json.load(fh)
            raw["params"][0]["max"] = 900
            with open(cfg_path, "w") as fh:
                json.dump(raw, fh)
            proc = self._run(["run", cfg_path, "--out", out, "--sim", sim_path,
                              "--resume"], expect=1)
            self.assertIn("different config", proc.stdout)
            self.assertTrue(proc.stdout.strip().endswith("SPSA-FAILED"))

    def test_resume_under_a_changed_objective_is_refused(self):
        """The objective is as much the run as the bounds are, and a flipped sign
        halfway through is the one corruption whose trajectory looks normal."""
        with tempfile.TemporaryDirectory() as d:
            cfg_path, sim_path = self._write(d)
            for out, mutate in (("changed_sim", "sim"), ("changed_sign", "sign")):
                out = os.path.join(d, out)
                self._run(["run", cfg_path, "--out", out, "--sim", sim_path,
                           "--crash-after", "20"], expect=9)
                args = ["run", cfg_path, "--out", out, "--sim", sim_path,
                        "--resume"]
                if mutate == "sim":
                    other = os.path.join(d, "other_sim.json")
                    spec = dict(SIM)
                    spec["optimum"] = [600.0] * 5
                    with open(other, "w") as fh:
                        json.dump(spec, fh)
                    args[args.index(sim_path)] = other
                else:
                    args.append("--flip-sign")
                proc = self._run(args, expect=1)
                self.assertIn("different config", proc.stdout)
                self.assertTrue(proc.stdout.strip().endswith("SPSA-FAILED"))

    def test_an_unloadable_config_still_prints_the_marker(self):
        """A detached run whose config will not parse must end in a line the
        watcher exits on, not in a traceback it waits out to its ceiling."""
        with tempfile.TemporaryDirectory() as d:
            bad = os.path.join(d, "bad.json")
            with open(bad, "w") as fh:
                fh.write("{ not json")
            proc = self._run(["run", bad, "--out", os.path.join(d, "x")],
                             expect=1)
            self.assertTrue(proc.stdout.strip().endswith("SPSA-FAILED"))
            missing = os.path.join(d, "absent.json")
            proc = self._run(["run", missing, "--out", os.path.join(d, "y")],
                             expect=1)
            self.assertTrue(proc.stdout.strip().endswith("SPSA-FAILED"))
            with open(bad, "w") as fh:
                json.dump({"iterations": 5, "pairs_per_iter": 1, "params": [
                    {"name": "P", "start": 5, "min": 0, "max": 10,
                     "c_end": 0.1}]}, fh)
            proc = self._run(["run", bad, "--out", os.path.join(d, "z")],
                             expect=1)
            self.assertIn("0.5", proc.stdout)
            self.assertTrue(proc.stdout.strip().endswith("SPSA-FAILED"))

    def test_resume_without_a_checkpoint_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            cfg_path, sim_path = self._write(d)
            proc = self._run(["run", cfg_path, "--out", os.path.join(d, "nothing"),
                              "--sim", sim_path, "--resume"], expect=1)
            self.assertTrue(proc.stdout.strip().endswith("SPSA-FAILED"))

    def test_run_json_is_frozen_before_the_run(self):
        with tempfile.TemporaryDirectory() as d:
            cfg_path, sim_path = self._write(d)
            out = os.path.join(d, "run")
            self._run(["run", cfg_path, "--out", out, "--sim", sim_path])
            with open(os.path.join(out, "run.json")) as fh:
                frozen = json.load(fh)
            self.assertEqual(frozen["config"]["iterations"], self.ITERATIONS)
            self.assertEqual(frozen["sim"]["optimum"], OPTIMUM)
            self.assertFalse(frozen["flip_sign"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
