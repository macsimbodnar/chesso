#!/usr/bin/env python3
"""SPSA over the tune build's search parameters. S084.

    tools/spsa_driver.py check <config.json> [--engine build-tune/src/chesso]
    tools/spsa_driver.py run   <config.json> --out .spsa/run1 [--resume]
    tools/spsa_driver.py run   <config.json> --out .spsa/sim1 --sim <sim.json>

Simultaneous Perturbation Stochastic Approximation, Spall 1992, in the form
fishtest and OpenBench use: two objective evaluations per iteration whatever the
dimensionality, so forty parameters cost what one costs. `adocs/plan_todo/` S084
carries the references and the reasoning; this file is the implementation.

Choosing which parameters to tune, their bounds and the budget is S085's, and
the returned vector is a hypothesis until an SPRT of the SHIPPING build says
otherwise -- the tune build is not the release binary and no strength number is
taken on it (DEV_MANUAL "The tune build").

WHAT THE ITERATION IS. Per iteration k: draw Delta with each component +/-1
(Rademacher, and nothing else -- the estimator divides by Delta_i and a uniform
or normal draw has no finite inverse moment), play theta+c*Delta against
theta-c*Delta as one paired mini-match, and step theta along
(wins - losses) * Delta. Pairing is the variance reduction that matters: both
sides get the same opening in both colours, so the opening's own bias cancels
instead of being averaged away over thousands of games.

THE SIGN. theta moves ALONG the gradient estimate, because a game result is a
thing to maximise. Spall's update subtracts because he minimises a loss. A flip
here produces a run that looks exactly like a run and walks downhill, which is
the single easiest way to spend a night and learn nothing -- so `--flip-sign`
exists as a test hook and `tests/test_spsa_driver.py` asserts the flipped run
fails the same criterion the honest one passes.

WHY END VALUES AND NOT SPALL'S CONSTANTS. Spall's a and c are unitless and have
to be re-derived whenever the iteration count changes; fishtest and OpenBench
parametrise by what the schedule reaches at the LAST iteration instead, which is
the quantity with a meaning: c_end is the smallest change in a parameter that
could plausibly matter, and r_end sets the final step. This file takes the same
two and recovers Spall's from them:

    c_i(k) = c_end_i * ((K+1)/(k+1))**gamma
    r(k)   = r_end * ((K+1+A)/(k+1+A))**alpha / ((K+1)/(k+1))**(2*gamma)
    theta_i += r(k) * c_i(k) * (wins - losses) * Delta_i

r(k) has no per-parameter part: c_end_i cancels between a_i(k) and c_i(k)**2,
which is why it is one column in the trajectory rather than one per parameter.
The update carries Spall's factor of two folded into r_end, as both frameworks'
do -- the seeded r_end = 0.002 was calibrated against this form, and splitting
the two apart would leave the constant meaning something else.

(wins - losses) is a SUM over the iteration's pairs, so doubling pairs_per_iter
doubles the step. The two are chosen together and frozen together in the config.

WHAT THE ENGINE WILL NOT TELL YOU. `setoption` with a value outside a
parameter's range is REFUSED, not clamped (src/search_params.hpp:231-239), and
in the tune build the refusal is invisible: it goes to LOG_W, which compiles to
`if (false) std::clog` under NDEBUG (src/log.hpp:35), and `uci` re-prints the
compiled default rather than the live value. So a driver that fails to clamp
plays its games against a silently-default parameter and converges confidently
to nonsense. Every value is clamped here -- theta, and theta+c and theta-c
separately -- and `check` compares the config's bounds against the binary's own
`uci` listing before a single game, because that comparison is the only
guard the engine side offers.

Integers are floats internally and rounded only at the UCI boundary. An integer
parameter with c_end < 0.5 is refused at config load: round(x+c) == round(x-c)
below that, so the perturbation vanishes and the axis never moves.
"""

import argparse
import hashlib
import json
import math
import os
import random
import re
import shutil
import subprocess
import sys

# Spall 1998 section 2, and OpenBench's defaults, which are the same figures:
# the lowest exponents the convergence proof allows, and they beat the
# asymptotically optimal 1.0 and 1/6 at any sample size a chess run can afford.
# Seeds per DEC-084 -- verified on the synthetic objective, not on authority.
ALPHA = 0.602
GAMMA = 0.101
A_RATIO = 0.1  # Spall: A at 10 % or less of the planned iteration count
R_END = 0.002

# fastchess prints one result block per match. Both lines are parsed: the first
# is the gradient, the second is the pentanomial the pairing exists to produce.
GAMES_RE = re.compile(r"^Games: (\d+), Wins: (\d+), Losses: (\d+), Draws: (\d+)")
PTNML_RE = re.compile(r"^Ptnml\(0-2\): \[(\d+), (\d+), (\d+), (\d+), (\d+)\]")
OPTION_RE = re.compile(
    r"^option name (\S+) type spin default (-?\d+) min (-?\d+) max (-?\d+)")


class ConfigError(Exception):
    """A run that cannot start. Raised before any game is played."""


class RunError(Exception):
    """A run that has to stop. Ends in SPSA-FAILED, never in a quiet return."""


class Param:
    def __init__(self, spec):
        for key in ("name", "start", "min", "max", "c_end"):
            if key not in spec:
                raise ConfigError(f"parameter {spec.get('name', spec)} has no '{key}'")
        self.name = str(spec["name"])
        self.start = float(spec["start"])
        self.lo = float(spec["min"])
        self.hi = float(spec["max"])
        self.c_end = float(spec["c_end"])

        if self.lo >= self.hi:
            raise ConfigError(f"{self.name}: min {self.lo} is not below max {self.hi}")
        if not self.lo <= self.start <= self.hi:
            raise ConfigError(
                f"{self.name}: start {self.start} outside [{self.lo}, {self.hi}]")
        # Rounding eats anything smaller, so the axis would be perturbed and
        # never moved. OpenBench enforces the same floor for integer parameters.
        if self.c_end < 0.5:
            raise ConfigError(
                f"{self.name}: c_end {self.c_end} is below 0.5, so round(x+c) == "
                "round(x-c) and the parameter can never move")

    def uci_value(self, x):
        """Clamp then round: what is actually sent, and never out of range."""
        return int(round(min(self.hi, max(self.lo, x))))


class Config:
    def __init__(self, raw):
        self.raw = raw
        self.iterations = int(raw.get("iterations", 0))
        self.pairs_per_iter = int(raw.get("pairs_per_iter", 0))
        self.alpha = float(raw.get("alpha", ALPHA))
        self.gamma = float(raw.get("gamma", GAMMA))
        self.a_ratio = float(raw.get("a_ratio", A_RATIO))
        self.r_end = float(raw.get("r_end", R_END))
        self.seed = int(raw.get("seed", 1))
        self.match = raw.get("match", {})
        self.params = [Param(p) for p in raw.get("params", [])]

        if self.iterations < 1:
            raise ConfigError("iterations must be at least 1")
        if self.pairs_per_iter < 1:
            raise ConfigError("pairs_per_iter must be at least 1")
        if not self.params:
            raise ConfigError("no parameters to tune")
        if not 0.0 < self.alpha <= 1.0:
            raise ConfigError(f"alpha {self.alpha} outside (0, 1]")
        if not 0.0 < self.gamma <= 1.0:
            raise ConfigError(f"gamma {self.gamma} outside (0, 1]")
        if self.r_end <= 0.0:
            raise ConfigError(f"r_end {self.r_end} is not positive")
        if self.a_ratio < 0.0:
            raise ConfigError(f"a_ratio {self.a_ratio} is negative")

        names = [p.name for p in self.params]
        if len(set(names)) != len(names):
            raise ConfigError("a parameter name appears twice")

    def digest(self, sim_spec=None, flip_sign=False):
        """What a resume is allowed to continue.

        Everything that decides what the next iteration does: the config, the
        objective, and whether the update is negated -- not the run directory.
        Resuming under any of them changed would splice two different runs into
        one trajectory and the file would not say so. The objective counts
        because a simulator spec is as much the run as its bounds are, and the
        sign because a half-flipped trajectory is the one result that looks like
        a normal one."""
        return hashlib.sha256(json.dumps(
            [self.raw, sim_spec, bool(flip_sign)],
            sort_keys=True).encode()).hexdigest()[:16]

    def warnings(self):
        out = []
        for p in self.params:
            c0 = p.c_end * schedule(0, self)[0]
            if p.hi - p.lo < 2.0 * c0:
                out.append(
                    f"{p.name}: range {p.hi - p.lo:g} is narrower than the first "
                    f"iteration's 2c = {2 * c0:.1f}, so theta+c and theta-c both "
                    "clamp to the bounds and the early gradient is noise")
        return out


def schedule(k, cfg):
    """(c multiplier, r) at iteration k, 0-based. Both reach their end value at
    the last iteration by construction: c_scale = 1 and r = r_end at k = K."""
    last = cfg.iterations - 1
    a_offset = cfg.a_ratio * cfg.iterations
    c_scale = ((last + 1.0) / (k + 1.0)) ** cfg.gamma
    a_scale = ((last + 1.0 + a_offset) / (k + 1.0 + a_offset)) ** cfg.alpha
    return c_scale, cfg.r_end * a_scale / (c_scale ** 2)


class Result:
    """One iteration's mini-match, from theta+'s side."""

    def __init__(self, wins, losses, draws, ptnml):
        self.wins, self.losses, self.draws = wins, losses, draws
        self.ptnml = list(ptnml)

    @property
    def y(self):
        return self.wins - self.losses

    def check(self, pairs):
        played = self.wins + self.losses + self.draws
        if played != 2 * pairs:
            raise RunError(
                f"{played} games came back for {pairs} pairs, expected {2 * pairs}; "
                "a missing game shrinks the gradient silently")


class SimObjective:
    """A noisy quadratic with a known optimum, in Elo.

    WHY THE GATE IS THIS AND NOT A GAME. A driver tested by playing games cannot
    tell a bug in itself from noise in the objective, at any budget this machine
    can afford: a sign error and a flat parameter produce the same trajectory. A
    known optimum costs milliseconds, isolates the iteration from the engine
    entirely, and fails loudly on the sign, on the decay schedules and on the
    clamp. The games come in S085, where they are the point.

    strength(theta) = -sum_i w_i * ((theta_i - opt_i) / sigma_i)**2, so the
    optimum scores 0 and everything else is negative Elo. A pair is two games
    sampled at d = strength(plus) - strength(minus), with a fixed draw rate:
    the pair score's spread lands where a real 8+0.08 pair's does, which is the
    regime the schedules have to survive.
    """

    def __init__(self, spec, params, rng):
        self.opt = [float(x) for x in spec["optimum"]]
        self.weights = [float(x) for x in spec["weights"]]
        self.sigmas = [float(x) for x in spec["sigmas"]]
        self.draw_rate = float(spec.get("draw_rate", 0.5))
        self.rng = rng
        if not len(self.opt) == len(self.weights) == len(self.sigmas) == len(params):
            raise ConfigError("simulator spec does not match the parameter count")
        if not 0.0 <= self.draw_rate < 1.0:
            raise ConfigError(f"draw_rate {self.draw_rate} outside [0, 1)")

    def strength(self, theta):
        return -sum(w * ((t - o) / s) ** 2
                    for t, o, w, s in zip(theta, self.opt, self.weights, self.sigmas))

    def play(self, plus, minus, pairs, start_round):
        del start_round  # no openings to walk through
        d = self.strength(plus) - self.strength(minus)
        p_win = (1.0 - self.draw_rate) / (1.0 + 10.0 ** (-d / 400.0))
        p_loss = (1.0 - self.draw_rate) / (1.0 + 10.0 ** (d / 400.0))
        wins = losses = draws = 0
        ptnml = [0, 0, 0, 0, 0]
        for _ in range(pairs):
            pair = 0.0
            for _ in range(2):
                u = self.rng.random()
                if u < p_win:
                    wins += 1
                    pair += 1.0
                elif u < p_win + p_loss:
                    losses += 1
                else:
                    draws += 1
                    pair += 0.5
            ptnml[int(round(pair * 2))] += 1
        return Result(wins, losses, draws, ptnml)


class MatchObjective:
    """One short fastchess match per iteration: theta+ against theta-.

    The installed fastchess (alpha 1.8.2) has no SPSA mode of its own, so the
    match is built here. `fastchess.sh` is deliberately untouched: that is the
    SPRT harness and this is not an SPRT.

    The engine binary is snapshotted into the run directory first. fastchess
    spawns it once per game, so pointing a multi-hour run at `build-tune/` means
    a rebuild part way through swaps the engine under the run and the trajectory
    becomes a mixture of two versions -- which has already happened once to the
    SPRT harness (fastchess.sh:141-147).
    """

    def __init__(self, match, params, rundir):
        self.params = params
        self.rundir = rundir
        self.tc = str(match.get("tc", "8+0.08"))
        self.hash_mb = int(match.get("hash", 16))
        self.threads = int(match.get("threads", 1))
        self.concurrency = int(match.get("concurrency", os.cpu_count() or 1))
        self.book = str(match.get("book", ""))
        self.book_format = str(match.get("book_format", "epd"))
        self.book_size = int(match.get("book_size", 0))
        self.extra = list(match.get("extra", []))
        self.timeout_s = int(match.get("timeout_s", 0))

        engine = str(match.get("engine", ""))
        if not engine or not os.access(engine, os.X_OK):
            raise ConfigError(f"match.engine '{engine}' is not an executable")
        if not self.book or not os.access(self.book, os.R_OK):
            raise ConfigError(f"match.book '{self.book}' is not readable")
        if self.book_size < 1:
            raise ConfigError("match.book_size must be the opening count, for wrapping")
        if shutil.which("fastchess") is None:
            raise ConfigError("no fastchess on PATH")

        self.engine = os.path.join(rundir, "engine.snapshot")
        if not os.path.exists(self.engine):
            shutil.copy2(engine, self.engine)
            os.chmod(self.engine, 0o755)
        self.pgn = os.path.join(rundir, "games.pgn")
        self.log = os.path.join(rundir, "fastchess.log")

    def _timeout(self, pairs):
        if self.timeout_s:
            return self.timeout_s
        base, _, inc = self.tc.partition("+")
        try:
            budget = 2.0 * (float(base) + 80.0 * float(inc or 0.0))
        except ValueError:
            raise ConfigError(f"match.tc '{self.tc}' is not base+inc")
        waves = math.ceil(2.0 * pairs / max(1, self.concurrency))
        return int(120 + 4.0 * waves * budget)

    def command(self, plus, minus, pairs, start_round):
        cmd = ["fastchess"]
        for name, values in (("plus", plus), ("minus", minus)):
            cmd += ["-engine", f"cmd={self.engine}", f"name={name}"]
            cmd += [f"option.{p.name}={v}" for p, v in zip(self.params, values)]
        cmd += ["-openings", f"file={self.book}", f"format={self.book_format}",
                "order=sequential", f"start={start_round}"]
        cmd += ["-each", f"tc={self.tc}", f"option.Hash={self.hash_mb}",
                f"option.Threads={self.threads}"]
        cmd += ["-games", "2", "-rounds", str(pairs), "-repeat",
                "-concurrency", str(self.concurrency), "-recover"]
        cmd += self.extra
        cmd += ["-pgnout", f"file={self.pgn}", "-log", f"file={self.log}"]
        return cmd

    def play(self, plus, minus, pairs, start_round):
        # 1-based round index, wrapped: a long run walks off the end of the book
        # and replaying from the top beats fastchess deciding for us.
        start = 1 + (start_round % self.book_size)
        cmd = self.command(plus, minus, pairs, start)
        try:
            proc = subprocess.run(cmd, capture_output=True, text=True,
                                  timeout=self._timeout(pairs))
        except subprocess.TimeoutExpired:
            raise RunError(f"fastchess did not finish inside "
                           f"{self._timeout(pairs)} s: {' '.join(cmd)}")
        if proc.returncode != 0:
            raise RunError(f"fastchess exited {proc.returncode}:\n{proc.stderr[-2000:]}")
        return parse_match(proc.stdout)


def parse_match(text):
    """The LAST result block in fastchess's output, from the first engine's side."""
    games = ptnml = None
    for line in text.splitlines():
        m = GAMES_RE.match(line.strip())
        if m:
            games = (int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4)))
        m = PTNML_RE.match(line.strip())
        if m:
            ptnml = [int(m.group(i)) for i in range(1, 6)]
    if games is None:
        raise RunError("no 'Games:' line in fastchess output")
    _, wins, losses, draws = games
    return Result(wins, losses, draws, ptnml or [0, 0, 0, 0, 0])


def read_engine_options(engine):
    """The binary's own `uci` listing: name -> (default, min, max)."""
    proc = subprocess.run([engine], input="uci\nquit\n", capture_output=True,
                          text=True, timeout=30)
    out = {}
    for line in proc.stdout.splitlines():
        m = OPTION_RE.match(line.strip())
        if m:
            out[m.group(1)] = (int(m.group(2)), int(m.group(3)), int(m.group(4)))
    if not out:
        raise ConfigError(f"{engine} printed no spin options; is it the tune build?")
    return out


def probe_nodes(engine, name, value, fen, depth):
    """Node count at fixed depth with one option set. The only channel the tune
    build offers for proving a `setoption` reached the search: the refusal is
    compiled out of a Release build (src/log.hpp:35) and `uci` re-prints the
    compiled default rather than the live value, so nothing else can be
    observed. DEV_MANUAL's own RfpMargin figures are this measurement."""
    # Read to `bestmove` rather than piping the script in and closing stdin: the
    # engine takes EOF for `quit` and abandons the search part way, which
    # reported 230 nodes at depth 9 against the 164123 DEV_MANUAL quotes and
    # read as "setoption does not work" (2026-08-20). tools/search_bench.py
    # holds the pipe open for the same reason.
    proc = subprocess.Popen([engine], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1)
    script = ["uci"]
    if value is not None:
        script.append(f"setoption name {name} value {value}")
    script += [f"position fen {fen}", f"go depth {depth}"]
    try:
        proc.stdin.write("\n".join(script) + "\n")
        proc.stdin.flush()
        nodes = None
        while True:
            line = proc.stdout.readline()
            if not line:
                break
            if line.startswith("info") and " nodes " in line:
                nodes = int(line.split(" nodes ")[1].split()[0])
            if line.startswith("bestmove"):
                break
        proc.stdin.write("quit\n")
        proc.stdin.flush()
    finally:
        try:
            proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            proc.kill()
    if nodes is None:
        raise ConfigError(f"{engine} reported no node count at depth {depth}")
    return nodes


# tools/search_bench.py's midgame position, which DEV_MANUAL's tune-build
# paragraph already quotes node counts on.
PROBE_FEN = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
PROBE_DEPTH = 9


def check(cfg, engine):
    """Everything that can be wrong before a game is played. Returns problems."""
    problems = []
    if engine:
        options = read_engine_options(engine)
        for p in cfg.params:
            if p.name not in options:
                problems.append(
                    f"{p.name} is not an option of {engine}; the engine ignores an "
                    "unknown name in silence and the axis would never move")
                continue
            _, lo, hi = options[p.name]
            if (lo, hi) != (int(p.lo), int(p.hi)):
                problems.append(
                    f"{p.name}: config bounds [{int(p.lo)}, {int(p.hi)}] against the "
                    f"binary's [{lo}, {hi}]; a value outside the binary's range is "
                    "refused, not clamped, and the refusal is invisible")

        # Does setoption reach the search in this binary at all?
        low = probe_nodes(engine, "RfpMargin", 75, PROBE_FEN, PROBE_DEPTH)
        high = probe_nodes(engine, "RfpMargin", 2000, PROBE_FEN, PROBE_DEPTH)
        if low == high:
            problems.append(
                f"RfpMargin 75 and 2000 both searched {low} nodes, so setoption is "
                "not reaching the search; is this the tune build?")
        else:
            print(f"setoption reaches the search: RfpMargin 75 -> {low} nodes, "
                  f"2000 -> {high}")
    return problems


def atomic_write(path, text, durable=False):
    """Rewritten every iteration, so a kill mid-write must not eat the run.

    The rename is the guarantee and it is unconditional: a reader sees the old
    file or the new one, never half of either, so a killed process cannot leave
    a checkpoint that will not parse. `durable` adds the fsync on top, which
    buys the one further case of the machine losing power mid-iteration -- worth
    13 ms when the iteration was a mini-match that took minutes, and not worth
    it when the iteration was a simulated one that takes microseconds and is
    reproducible from the seed. It is what keeps the synthetic gate inside the
    fast suite: 20000 iterations of two fsyncs is 26 s of waiting for a disk."""
    tmp = path + ".tmp"
    with open(tmp, "w") as fh:
        fh.write(text)
        fh.flush()
        if durable:
            os.fsync(fh.fileno())
    os.replace(tmp, path)


def trajectory_header(cfg):
    return "\t".join(["k", "pairs", "c_scale", "r_k", "y", "wins", "losses",
                      "draws", "ptnml"] + [p.name for p in cfg.params]) + "\n"


def truncate_trajectory(path, k0, cfg):
    """Keep rows for iterations before k0 and drop anything after.

    An iteration appends its row and then writes its checkpoint, so a kill
    between the two leaves one row the checkpoint does not know about. Dropping
    it is what makes a resumed trajectory byte-identical to an uninterrupted
    one, which is the only version of the file worth comparing."""
    if not os.path.exists(path):
        return
    with open(path) as fh:
        lines = fh.readlines()
    kept = [lines[0]] if lines else []
    for line in lines[1:]:
        try:
            if int(line.split("\t", 1)[0]) < k0:
                kept.append(line)
        except ValueError:
            continue
    if not kept:
        kept = [trajectory_header(cfg)]
    atomic_write(path, "".join(kept))


def run(cfg, rundir, sim_spec=None, resume=False, flip_sign=False,
        crash_after=None, crash_phase="checkpoint", durable=None):
    os.makedirs(rundir, exist_ok=True)
    ck_path = os.path.join(rundir, "checkpoint.json")
    tj_path = os.path.join(rundir, "trajectory.tsv")

    # The frozen config, written before the run and never rewritten: what the
    # numbers in the trajectory are numbers about.
    digest = cfg.digest(sim_spec, flip_sign)
    frozen = {"config": cfg.raw, "sim": sim_spec, "flip_sign": flip_sign,
              "digest": digest}
    if not resume:
        atomic_write(os.path.join(rundir, "run.json"),
                     json.dumps(frozen, indent=2, sort_keys=True) + "\n")

    rng = random.Random(cfg.seed)
    theta = [p.start for p in cfg.params]
    k0, pairs_done = 0, 0

    if resume:
        if not os.path.exists(ck_path):
            raise RunError(f"no checkpoint at {ck_path} to resume from")
        with open(ck_path) as fh:
            ck = json.load(fh)
        if ck.get("digest") != digest:
            raise RunError(
                "checkpoint was written under a different config, objective or "
                "update sign; resuming would splice two runs into one trajectory")
        k0 = int(ck["k"])
        theta = [float(x) for x in ck["theta"]]
        pairs_done = int(ck["pairs"])
        state = ck["rng"]
        rng.setstate((state[0], tuple(state[1]), state[2]))
        truncate_trajectory(tj_path, k0, cfg)
        print(f"resumed at iteration {k0} of {cfg.iterations}, {pairs_done} pairs in")

    if not os.path.exists(tj_path):
        atomic_write(tj_path, trajectory_header(cfg))

    objective = (SimObjective(sim_spec, cfg.params, rng) if sim_spec is not None
                 else MatchObjective(cfg.match, cfg.params, rundir))
    sign = -1.0 if flip_sign else 1.0
    if durable is None:
        durable = sim_spec is None

    for k in range(k0, cfg.iterations):
        c_scale, r_k = schedule(k, cfg)
        delta = [rng.choice((-1.0, 1.0)) for _ in cfg.params]
        plus, minus = [], []
        for i, p in enumerate(cfg.params):
            c = p.c_end * c_scale
            plus.append(p.uci_value(theta[i] + c * delta[i]))
            minus.append(p.uci_value(theta[i] - c * delta[i]))

        res = objective.play(plus, minus, cfg.pairs_per_iter, pairs_done)
        res.check(cfg.pairs_per_iter)
        pairs_done += cfg.pairs_per_iter

        for i, p in enumerate(cfg.params):
            step = r_k * p.c_end * c_scale * res.y * delta[i]
            theta[i] = min(p.hi, max(p.lo, theta[i] + sign * step))

        row = [k, pairs_done, f"{c_scale:.6f}", f"{r_k:.8g}", res.y,
               res.wins, res.losses, res.draws,
               "/".join(str(n) for n in res.ptnml)]
        row += [p.uci_value(theta[i]) for i, p in enumerate(cfg.params)]
        with open(tj_path, "a") as fh:
            fh.write("\t".join(str(c) for c in row) + "\n")
            fh.flush()
            if durable:
                os.fsync(fh.fileno())

        if crash_after == k and crash_phase == "trajectory":
            os._exit(9)  # test hook: killed between the row and the checkpoint

        state = rng.getstate()
        atomic_write(ck_path, json.dumps({
            "k": k + 1, "theta": theta, "pairs": pairs_done,
            "rng": [state[0], list(state[1]), state[2]],
            "digest": digest}) + "\n", durable=durable)

        if crash_after == k and crash_phase == "checkpoint":
            os._exit(9)  # test hook: killed just after the checkpoint landed

    return theta


def final_vector(cfg, theta):
    return {p.name: p.uci_value(theta[i]) for i, p in enumerate(cfg.params)}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = ap.add_subparsers(dest="cmd", required=True)

    c = sub.add_parser("check", help="validate a config against the engine")
    c.add_argument("config")
    c.add_argument("--engine", default=None,
                   help="tune build to compare bounds against and probe")

    r = sub.add_parser("run", help="run the iteration")
    r.add_argument("config")
    r.add_argument("--out", required=True, help="run directory, gitignored")
    r.add_argument("--resume", action="store_true")
    r.add_argument("--sim", default=None,
                   help="synthetic objective spec; no games are played")
    r.add_argument("--flip-sign", action="store_true",
                   help="test hook: negate the update, so the gate can prove the "
                        "sign matters")
    r.add_argument("--crash-after", type=int, default=None,
                   help="test hook: _exit after this iteration")
    r.add_argument("--crash-phase", choices=("trajectory", "checkpoint"),
                   default="checkpoint", help="test hook: where to _exit")

    args = ap.parse_args(argv)
    # Inside the marker's reach: a config that will not load is the most likely
    # way a detached run fails, and a traceback is not a line any watcher exits
    # on -- it would sit until its ceiling (DEC-061).
    try:
        with open(args.config) as fh:
            cfg = Config(json.load(fh))
    except (OSError, ValueError, ConfigError) as exc:
        print(f"ERROR: {args.config}: {exc}")
        print("SPSA-FAILED")
        return 1

    if args.cmd == "check":
        problems = check(cfg, args.engine)
        for w in cfg.warnings():
            print(f"WARNING: {w}")
        for p in problems:
            print(f"PROBLEM: {p}")
        if problems:
            print("SPSA-FAILED")
            return 1
        pairs = cfg.iterations * cfg.pairs_per_iter
        print(f"{len(cfg.params)} parameters, {cfg.iterations} iterations x "
              f"{cfg.pairs_per_iter} pairs = {pairs} pairs, {2 * pairs} games")
        print("SPSA-DONE")
        return 0

    sim_spec = None
    if args.sim:
        with open(args.sim) as fh:
            sim_spec = json.load(fh)

    for w in cfg.warnings():
        print(f"WARNING: {w}")
    try:
        theta = run(cfg, args.out, sim_spec=sim_spec, resume=args.resume,
                    flip_sign=args.flip_sign, crash_after=args.crash_after,
                    crash_phase=args.crash_phase)
    except (ConfigError, RunError) as exc:
        print(f"ERROR: {exc}")
        print("SPSA-FAILED")  # DEC-061: the last line a watcher exits on
        return 1
    print(json.dumps(final_vector(cfg, theta), indent=2, sort_keys=True))
    print("SPSA-DONE")
    return 0


if __name__ == "__main__":
    sys.exit(main())
