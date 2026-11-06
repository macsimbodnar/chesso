#!/usr/bin/env python3
"""S124's P1: mop-up's SPSA on conversion, option (b), a driver that rebuilds per point
(DEC-350 (4), the step file's "Mop-up's SPSA instrument" and "P1's instrument, option (b)").

    PY=~/.venv/chess/bin/python
    $PY adocs/data/S124_p1.py all   --out OUT --train-starts P0_STARTS   # setup, run, test
    $PY adocs/data/S124_p1.py setup --out OUT --train-starts P0_STARTS
    $PY adocs/data/S124_p1.py run   --out OUT [--resume]
    $PY adocs/data/S124_p1.py test  --out OUT
    $PY adocs/data/S124_p1.py smoke --out SMOKE_OUT --train-starts P0_STARTS [--plant NAME]
    $PY adocs/data/S124_p1.py rule-check --out DIR      # DEC-358's kept rule, synthetic

Run from the checkout whose HEAD is the landed tree, with build/ built at that HEAD.
Nothing under src/ is ever written in the tree: every point's constants go into a scratch
copy of HEAD's sources under OUT/scratch (`git archive HEAD`), rebuilt there with ccache.
Each subcommand ends S124-P1-<SUB>-DONE or S124-P1-<SUB>-FAILED; `all` ends S124-P1-DONE
or S124-P1-FAILED, its last stdout line whatever happened, so it runs detached under a
four-exit watcher (WATCHERS). OUT defaults to .tuning/s124/p1 (DEC-235).

==========================================================================================
PRE-REGISTRATION -- written 2026-11-06 with P1's instrument, before any SPSA iteration was
played, from the step file's draft ("Pre-registered: the screen, a draft", its P1 line)
and the coordinator's choice of option (b). Each departure from the draft is a dated
amendment in the step file ("P1's instrument, option (b)"). Nothing below changes once
`run` has played its first iteration; a change is a dated section in the step file and a
decision.
==========================================================================================

THE PARAMETERS. Mop-up's three linear pairs (src/evaluation.cpp, `mopup_mg`, `mopup_eg`;
features CENTRE, KINGS, CORNER, src/evaluation.hpp). The corner pair is one direction,
2 mg + 22 eg, KBN against a king alone standing at phase 2 always: **the corner's
middlegame half is frozen** at its landed 0 -- the draft's "tied or frozen" read as the
freeze, the form `tools/feature_audit` holds the same direction by -- and its endgame half
carries the direction. Five free:

    name        slot          start  box           c_end
    centre_mg   mopup_mg[0]   0      [-256, 256]   32
    kings_mg    mopup_mg[1]   0      [-256, 256]   32
    centre_eg   mopup_eg[0]   0      [-64, 192]    4
    kings_eg    mopup_eg[1]   0      [-64, 192]    4
    corner_eg   mopup_eg[2]   0      [-64, 192]    4

THE START: the landed constants, every one 0 (DEC-350 (3)), read from the tree; setup
refuses a tree whose mop-up is not all zero, this registration being written for that
start. No engine's or page's number seeds anything (DEC-105).

THE SCALING OF c AND a. A middlegame half reaches the domain's score at phase / 24, an
endgame half at (24 - phase) / 24; over the training starts the phase is 2 to 4 (KRvK,
KBNvK, KBBvK 2; KRBvK, KRNvK 3; KQvK 4), mean 2.71, so a middlegame weight moves the score
about 1/8 as much as an endgame weight: c_end_mg = 8 c_end_eg, so each perturbation moves
the domain's score by about the same. c_end_eg = 4, in the engine's integer units per
count, four rounding steps at the last iteration. `tools/spsa_driver.py`'s schedule and
update, imported from it: c_i(k) = c_end_i ((K)/(k+1))^gamma, r(k) = r_end ((K+A)/(k+1+A))
^alpha / (K/(k+1))^(2 gamma), theta_i += r(k) c_i(k) y delta_i; in Spall's form
a_i(k) = 2 r(k) c_i(k)^2, so a scales with c^2 and a unit of score sensitivity moves every
parameter by the same score. Spall's constants at these figures: c = 6.635 (eg) and 53.08
(mg), the c_end times K^gamma = 1.6588; a = 2 r_end c_end^2 (K+A)^alpha = 8.649 (eg) and
553.6 (mg), (K+A)^alpha = 21.62; A = 15; alpha 0.602, gamma 0.101 (spsa_driver's ALPHA and
GAMMA). r_end = 0.0125, for y in plies a start (below): the first step on an endgame half
is 0.1228 y, the last 0.05 y. Chosen on the smoke's synthetic objective (SIM_* below) at
the registered schedule and batch: the first calibration, 0.025, sized from a guessed
first gradient, diverged there at ten times the guessed steepness, so the rule became
"never diverge from a quarter to ten times it, at a per-start noise of 50 or 150 plies",
and the grid was read for 0.025 and its half. Of the seed's excess, 0.025 left 11 to 26 %
at a quarter, 0 to 4 % at one and four times, and 609 to 1054 % at ten; 0.0125 left 28 to
30 %, 2 to 3 %, 0 % and 1 %. S085's reading is the same asymmetry: a step too small costs
a little, one too large costs the run (DEV_MANUAL "Sizing a real run"). The synthetic
steepness is a guess and the grid calibrates nothing beyond it.

THE INTEGERS. theta is a float; a point is clamped to its box and rounded (Python's
round(), half to even: `spsa_driver.Param.uci_value`), the frozen slot at its landed value,
then held under the stage-one bound: stage_one_with_mopup = stage_one_bound + max over the
halves of 6 |centre| + 12 |kings| + 7 |corner| must stay <= 32767 (the `static_assert`
over `mopup_half()`), so ROOM = 32767 - stage_one_bound, read from the scratch copy at
setup (27929 at `8cc9a38`, ROOM 4838) and proved there by the assert itself: a vector
reaching exactly ROOM compiles, one reaching ROOM + 1 fires it. A half reaching past ROOM
is scaled toward zero, each weight w to trunc(w ROOM / reach), so it never fails a build;
the trajectory flags it. The boxes reach 18 x 256 = 4608 and 25 x 192 = 4800, inside
4838, so at this tree the clamp is a guard and binds nowhere.

THE STARTS. Training: P0's 1126 starts (`S124_p0.py select`, seed 20261106, skip 0, 200 of
KRvK, KQvK, KRBvK, KRNvK, KBNvK and all 126 KBBvK), every one. Each iteration plays a
stratified batch: 40 of each of the five classes and 21 of KBBvK, 221 starts, taken in
order from one seeded shuffle a class and cycle (random.Random("20261106 CLASS CYCLE"),
the ids sorted first), so every start is played once every 5 iterations (KBBvK every 6)
and the batch's classes weigh 40:40:40:40:40:21. theta+ and theta- play the **same** 221
starts, paired by start.

THE GAMES. Self-play by each point's own build, `S124_p0.play_one`: two processes a game,
Hash 16, Threads 1, `go nodes 100000` a move, under the laws, no adjudication, the bound
100 plies (50 moves) from the start; 12 concurrent games, so 442 games an iteration.

THE OBJECTIVE, minimised. A start scores its plies to mate when converted (mated within
the bound), and 200 plies, twice the bound, for any failure: the fifty-move rule, a
repetition, stalemate, insufficient material or the bound. y = mean over the batch of
score(theta-) - score(theta+), plies a start, positive when theta+ converts sooner.
**Amended from the draft's "the bound for a failure":** P1 is kept on conversions, and at
the bound a failure scores what a mate on the last ply scores, so turning one failure
into a mate at ply 80 gains 20 plies, about what one start's mate length moves between
theta+ and theta- (20.2 on average over the smoke's 25 pairs both builds converted, at its
larger perturbations); at 200 it gains 120, so the objective ranks conversion first and
speed second, the order the kept rule reads.

THE ITERATIONS AND THE STOPPING RULE. K = 150 iterations, fixed: no early stop, no reading
of the trajectory decides anything. The candidate is theta after the 150th update, as
integers by the rule above. A run that fails resumes from its checkpoint (`run
--resume`), the same draws and batches, never restarted under other constants; one that
fails inside iteration 0, before any checkpoint, resumes at 0 (the fast check's finding,
2026-11-06). Seed 20261106 for the Rademacher draws.

THE TEST, ON FRESH STARTS, disjoint from training: `S124_p0.py select` with the training
starts' own pool, held-out view and seed, --per-class 200 --skip 200, the next 200 of the
same order: KRvK, KQvK, KRBvK, KRNvK 200 each and KBNvK 184 (384 won and distinct, 200
taken by P0), **KBBvK none** (126, all P0's). Disjointness is checked by id and by
position before a game. Head (a scratch build at the landed constants) and the candidate
each play them at the games' settings, and KBBvK's 126 training starts too, paired by
start. **Amended 2026-11-06, DEC-358, before iteration 0** (the draft's "McNemar's
one-sided z > 1.645 and no class converts fewer" read raw per-class counts, which refuse
on chaos in every class the candidate does not touch, and its uncorrected z keeps b = 0,
c = 3): **kept iff the exact one-sided McNemar test over every fresh start -- the binomial
on the discordant pairs at 1/2, the candidate converting more -- reads p < 0.05, and no
class reads the exact one-sided test the other way, the candidate converting fewer, at
p < 0.05.** KBBvK, in sample on its training starts, is held to the same per-class
refusal; it can refuse and never helps keep. `S124_p0.py mcnemar`'s reading, the rule
replaced, is printed beside and decides nothing. `rule-check` holds the rule on
synthetic paired records: b = 0, c = 3 is not kept (p 0.125); a KBNvK gain of 120 with one
KRBvK start lost by chaos is kept, which the old rule refused; eight KRvK starts lost
(p 0.0039), or six KBBvK training starts (p 0.0156), refuse.

TERMINAL MARKERS. `all`: S124-P1-DONE or S124-P1-FAILED. Each subcommand: S124-P1-SETUP-,
-RUN-, -TEST-, -SMOKE-, -RULE-CHECK- DONE or FAILED; `test` prints S124-P1-KEPT or
S124-P1-NOT-KEPT before its marker.

MACHINE HOURS, from the measurements in the step file's section. A point's build, one
evaluation.cpp and the link under ccache: 1.9 s alone at load 5; an iteration's two in
parallel 2.3 to 3.2 s on a cache miss and 0.4 s on a hit, under S124's screen. A ply:
0.0420 core-seconds at 12 games on the idle machine (P0: 1126 starts, 59111 plies, 207 s),
0.0361 and 0.0471 in the two smokes at 4 games under the screen. At head's mix an
iteration's 442 games are 23104 plies, so 81 s of games at P0's figure and 91 s at the
smokes' higher one, and an iteration 84 to 94 s: K = 150 in 3.5 to 3.9 h. The test about
9 min: the fresh select 71 s (the 1.4 GB pool), 2 x 984 fresh starts and 2 x 126 KBBvK.
3.7 to 4.1 h in all, at DEC-155's four-hour line; where it runs is the coordinator's call.
Conversions shorten the games as the run goes, so head's mix is the high figure. At worst,
every game to the bound (4.2 core-seconds a start), about 6.7 h. A watcher's ceiling: 9 h,
32400 s, twice the high estimate.
"""

import argparse
import collections
import concurrent.futures
import fractions
import hashlib
import importlib.util
import json
import math
import os
import random
import re
import shutil
import subprocess
import sys
import time

import chess

# ----------------------------------------------------------- the pre-registered numbers

SEED = 20261106
ITERATIONS = 150
A_RATIO = 0.1  # A = 15
R_END = 0.0125
NODES = 100000
WORKERS = 12
PENALTY_FACTOR = 2  # a failure scores PENALTY_FACTOR times the bound, in plies
BATCH = collections.OrderedDict([("KBBvK", 21), ("KBNvK", 40), ("KRvK", 40), ("KRBvK", 40),
                                 ("KRNvK", 40), ("KQvK", 40)])  # longest games first
# (name, array, feature index, low, high, c_end); the corner's middlegame half is frozen.
PARAMS = (("centre_mg", "mopup_mg", 0, -256, 256, 32),
          ("kings_mg", "mopup_mg", 1, -256, 256, 32),
          ("centre_eg", "mopup_eg", 0, -64, 192, 4),
          ("kings_eg", "mopup_eg", 1, -64, 192, 4),
          ("corner_eg", "mopup_eg", 2, -64, 192, 4))
FROZEN = (("mopup_mg", 2),)
ARRAYS = ("mopup_mg", "mopup_eg")
REACH = (6, 12, 7)  # mopup_half()'s reach, a count's largest value
HALF_MAX = 32767
FRESH_SKIP = 200
P_KEPT = 0.05  # DEC-358's exact one-sided tests
# The smoke's overrides and nothing else's.
SMOKE = {"iterations": 3, "batch": 2, "train_limit": 4, "workers": 4, "fresh": 2}

# The eval check's vector: distinct values in every free slot, so a slot written to the
# wrong array or feature moves some row's evaluation by far more than the truncation's 2.
CHECK_VECTOR = {"mopup_mg": [11, 5, 0], "mopup_eg": [7, 13, 19]}
# Rows outside mop-up's domain, whose evaluation no mop-up weight may move: a pawn, knights
# alone, both sides with men.
OUTSIDE = ("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", "4k3/8/8/8/8/8/8/2N1KN2 w - - 0 1",
           "4k3/8/8/3r4/8/8/8/R3K3 b - - 0 1",
           "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

HERE = os.path.dirname(os.path.abspath(__file__))
SCRATCHES = ("head", "plus", "minus", "final")
LINE_RE = r"^constexpr int %s\[MOPUP_FEATURE_COUNT\] = \{(-?\d+), (-?\d+), (-?\d+)\};$"
ASSERT_TEXT = "stage one's packed sum with mop-up can overflow a 16-bit half"

# Test hooks: the smoke's planted breaks. `swap` maps each array's values into the other
# array, `order` writes each array's features in reverse; both read back through the same
# map, so the textual check agrees and only the engine's evaluation can see them.
# `room` reads ROOM one too large, `noclamp` skips the stage-one clamp, `sign` negates the
# update, `raw` puts back the per-class raw count comparison DEC-358 replaced.
PLANT = None


class Refused(Exception):
    """A stage that cannot go on; its message is printed above the FAILED marker."""


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def sha256_text(text):
    return hashlib.sha256(text.encode()).hexdigest()


def atomic_write(path, text):
    tmp = path + ".tmp"
    with open(tmp, "w") as handle:
        handle.write(text)
        handle.flush()
        os.fsync(handle.fileno())
    os.replace(tmp, path)


def say(*parts):
    print(*parts, flush=True)


def git(root, *args):
    return subprocess.run(["git", "-C", root] + list(args), check=True, capture_output=True,
                          text=True).stdout


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


# ------------------------------------------------------------------ the vector's arrays

def slot_map():
    arrays = {"mopup_mg": "mopup_mg", "mopup_eg": "mopup_eg"}
    order = (0, 1, 2)
    if PLANT == "swap":
        arrays = {"mopup_mg": "mopup_eg", "mopup_eg": "mopup_mg"}
    if PLANT == "order":
        order = (2, 1, 0)
    return arrays, order


def eval_cpp(scratch):
    return os.path.join(scratch, "src", "evaluation.cpp")


def read_vector(scratch):
    with open(eval_cpp(scratch)) as handle:
        text = handle.read()
    arrays, order = slot_map()
    vector = {}
    for name in ARRAYS:
        found = re.findall(LINE_RE % arrays[name], text, re.M)
        if len(found) != 1:
            raise Refused("%s: %d lines define %s, not one" % (eval_cpp(scratch), len(found),
                                                                 arrays[name]))
        written = [int(x) for x in found[0]]
        vector[name] = [written[order[i]] for i in range(3)]
    return vector


def write_vector(scratch, vector):
    """Writes the six constants and reads them back; rewrites the file only when a value
    moves, so a point equal to the last one costs no build."""
    path = eval_cpp(scratch)
    with open(path) as handle:
        text = handle.read()
    arrays, order = slot_map()
    new = text
    for name in ARRAYS:
        values = [0, 0, 0]
        for i in range(3):
            values[order[i]] = vector[name][i]
        line = "constexpr int %s[MOPUP_FEATURE_COUNT] = {%d, %d, %d};" % (
            (arrays[name],) + tuple(values))
        new, count = re.subn(LINE_RE % arrays[name], line, new, flags=re.M)
        if count != 1:
            raise Refused("%s: %d lines define %s, not one" % (path, count, arrays[name]))
    if new != text:
        atomic_write(path, new)
    if read_vector(scratch) != vector:
        raise Refused("%s does not read back the vector written" % path)


def vector_text(vector):
    return "%s|%s" % (",".join(map(str, vector["mopup_mg"])),
                      ",".join(map(str, vector["mopup_eg"])))


def reach(weights):
    return sum(r * abs(w) for r, w in zip(REACH, weights))


def clamp_reach(vector, room):
    """Each half held under ROOM: a half past it scaled toward zero, w to trunc(w ROOM /
    reach), in integers, so its reach lands at ROOM or under. Returns (vector, clamped)."""
    out, clamped = {}, False
    for name in ARRAYS:
        weights = list(vector[name])
        total = reach(weights)
        if total > room and PLANT != "noclamp":
            weights = [(1 if w >= 0 else -1) * (abs(w) * room // total) for w in weights]
            clamped = True
        out[name] = weights
    return out, clamped


def exact_reach_vector(target):
    """An endgame half reaching exactly `target` (6a + 7c, c = target mod 6), the middlegame
    half zero: the assert's own arithmetic read at one point."""
    corner = target % 6
    centre = (target - 7 * corner) // 6
    if centre < 0:
        raise Refused("no vector reaches exactly %d" % target)
    return {"mopup_mg": [0, 0, 0], "mopup_eg": [centre, 0, corner]}


# ----------------------------------------------------------------- scratch and builds

def build_env(scratch):
    # No repository above the copy, so the build stamps `unknown` and never reads the
    # enclosing checkout's state into `id name`.
    return dict(os.environ, GIT_CEILING_DIRECTORIES=os.path.dirname(scratch))


def make_scratch(source, sha, scratch, log):
    if os.path.exists(scratch):
        shutil.rmtree(scratch)
    os.makedirs(scratch)
    archive = subprocess.run(["git", "-C", source, "archive", "--format=tar", sha,
                              "CMakeLists.txt", "cmake", "src", "tests", "tools"],
                             check=True, capture_output=True).stdout
    subprocess.run(["tar", "-x", "-C", scratch], input=archive, check=True)
    command = ["cmake", "-S", scratch, "-B", os.path.join(scratch, "build"),
               "-DCMAKE_BUILD_TYPE=Release"]
    if shutil.which("ccache"):
        command.append("-DCMAKE_CXX_COMPILER_LAUNCHER=ccache")
    with open(log, "a") as handle:
        if subprocess.run(command, stdout=handle, stderr=subprocess.STDOUT,
                          env=build_env(scratch)).returncode != 0:
            raise Refused("cmake did not configure %s; %s" % (scratch, log))


def start_build(scratch, jobs, log):
    handle = open(log, "a")
    process = subprocess.Popen(["cmake", "--build", os.path.join(scratch, "build"), "--target",
                                "chesso", "-j", str(jobs)], stdout=handle,
                               stderr=subprocess.STDOUT, env=build_env(scratch))
    return process, handle


def build(scratches, jobs, log_dir):
    """Builds every scratch in parallel; returns the wall seconds."""
    began = time.time()
    running = []
    for scratch in scratches:
        log = os.path.join(log_dir, "build_%s.log" % os.path.basename(scratch))
        running.append((scratch, log) + start_build(scratch, jobs, log))
    failed = []
    for scratch, log, process, handle in running:
        if process.wait() != 0:
            failed.append(log)
        handle.close()
    if failed:
        raise Refused("a build failed: %s" % ", ".join(failed))
    return time.time() - began


def engine_of(scratch):
    return os.path.join(scratch, "build", "src", "chesso")


def bench(engine):
    result = subprocess.run([engine, "bench"], capture_output=True, text=True)
    last = (result.stdout.strip().splitlines() or [""])[-1]
    match = re.match(r"^(\d+) nodes ", last)
    if result.returncode != 0 or not match:
        raise Refused("%s bench printed no node count: %s" % (engine, last))
    return int(match.group(1))


def syntax_check(scratch):
    """g++ -fsyntax-only over the scratch's evaluation.cpp: (holds, the assert fired)."""
    result = subprocess.run(["g++", "-std=gnu++20", "-march=native", "-fsyntax-only",
                             "-I", os.path.join(scratch, "src"),
                             "-I", os.path.join(scratch, "build", "generated"),
                             eval_cpp(scratch)], capture_output=True, text=True)
    return result.returncode == 0, ASSERT_TEXT in result.stderr


def stage_one_bound(scratch, tmp_dir):
    """stage_one_bound as the compiler evaluates it, named in a diagnostic; the caller
    holds the figure to the assert itself."""
    probe = os.path.join(tmp_dir, "bound_probe.cpp")
    with open(probe, "w") as handle:
        handle.write('#include "%s"\ntemplate <int N> struct s124_p1_value;\n'
                     's124_p1_value<stage_one_bound> s124_p1_probe;\n' % eval_cpp(scratch))
    result = subprocess.run(["g++", "-std=gnu++20", "-march=native", "-fsyntax-only",
                             "-I", os.path.join(scratch, "src"),
                             "-I", os.path.join(scratch, "build", "generated"), probe],
                            capture_output=True, text=True)
    found = re.findall(r"s124_p1_value<(-?\d+)>", result.stderr)
    if not found:
        raise Refused("stage_one_bound not read: %s" % result.stderr[-400:])
    return int(found[0])


# ------------------------------------------------------------------- mop-up's counts

def engine_square(square):
    # src/'s index: 0 is a8, file a to h, rank 8 down to 1.
    return (7 - chess.square_rank(square)) * 8 + chess.square_file(square)


def mopup_expect(fen):
    """(in the domain, White-relative sign of the side with men, counts, phase) computed
    here from the definitions in src/evaluation.hpp, not from the engine."""
    board = chess.Board(fen)
    men = {}
    for colour in (chess.WHITE, chess.BLACK):
        men[colour] = board.occupied_co[colour] & ~board.kings
    phase = 0
    for colour in (chess.WHITE, chess.BLACK):
        phase += (len(board.pieces(chess.KNIGHT, colour)) + len(board.pieces(chess.BISHOP, colour))
                  + 2 * len(board.pieces(chess.ROOK, colour)) +
                  4 * len(board.pieces(chess.QUEEN, colour)))
    phase = min(phase, 24)
    if bool(men[chess.WHITE]) == bool(men[chess.BLACK]):
        return False, 0, None, phase
    side = chess.WHITE if men[chess.WHITE] else chess.BLACK
    if board.pieces(chess.PAWN, side):
        return False, 0, None, phase
    knights = len(board.pieces(chess.KNIGHT, side))
    bishops = board.pieces(chess.BISHOP, side)
    units = (knights + len(bishops) + 2 * len(board.pieces(chess.ROOK, side)) +
             4 * len(board.pieces(chess.QUEEN, side)))
    if units < 2 or units == knights:
        return False, 0, None, phase
    lone = engine_square(board.king(not side))
    own = engine_square(board.king(side))

    def centre(sq):
        f, r = sq & 7, sq >> 3
        return (3 - f if f < 4 else f - 4) + (3 - r if r < 4 else r - 4)

    def manhattan(a, b):
        return abs((a >> 3) - (b >> 3)) + abs((a & 7) - (b & 7))

    def chebyshev(a, b):
        return max(abs((a >> 3) - (b >> 3)), abs((a & 7) - (b & 7)))

    counts = [centre(lone), 14 - manhattan(lone, own), 0]
    if knights == 1 and units == 2 and len(bishops) == 1:
        b = engine_square(next(iter(bishops)))
        on_one = ((b >> 3) + (b & 7)) % 2 == 0  # a8 and h1's colour
        corners = (0, 63) if on_one else (7, 56)  # a8 h1, or h8 a1
        counts[2] = 7 - min(chebyshev(lone, c) for c in corners)
    return True, (1 if side == chess.WHITE else -1), counts, phase


def eval_check(st, scratch, fens, log_dir):
    """The written constants are the ones the build evaluates with: evaluate() through the
    engine's library at CHECK_VECTOR less evaluate() at head, against this file's own
    counts times the vector, tapered; within 2 in the domain (two truncations), 0 outside."""
    write_vector(scratch, CHECK_VECTOR)
    build([scratch], 6, log_dir)
    probe_src = os.path.join(st["out"], "bin", "S124_engine_eval.cpp")
    rows = os.path.join(log_dir, "eval_check_fens.txt")
    with open(rows, "w") as handle:
        handle.write("".join(f + "\n" for f in fens))
    evals = {}
    for name, where in (("head", st["scratch"]["head"]), ("check", scratch)):
        binary = os.path.join(log_dir, "engine_eval_%s" % name)
        result = subprocess.run(["g++", "-std=c++20", "-O2", "-march=native", "-DNDEBUG",
                                 "-I", os.path.join(where, "src"), probe_src,
                                 os.path.join(where, "build", "src", "libchesso_engine.a"),
                                 "-lpthread", "-o", binary], capture_output=True, text=True)
        if result.returncode != 0:
            raise Refused("S124_engine_eval did not build against %s: %s" %
                          (where, result.stderr[-400:]))
        out = subprocess.run([binary, rows], capture_output=True, text=True)
        if out.returncode != 0:
            raise Refused("engine_eval failed: %s" % out.stderr[-400:])
        evals[name] = [int(x) for x in out.stdout.split()]
        if len(evals[name]) != len(fens):
            raise Refused("engine_eval read %d rows of %d" % (len(evals[name]), len(fens)))
    worst, bad, inside, slots = 0.0, [], 0, [0, 0, 0, 0, 0, 0]
    for i, fen in enumerate(fens):
        domain, men, counts, phase = mopup_expect(fen)
        stm = 1 if chess.Board(fen).turn == chess.WHITE else -1
        diff = evals["check"][i] - evals["head"][i]
        if not domain:
            if diff != 0:
                bad.append("%s outside the domain moved %d" % (fen, diff))
            continue
        inside += 1
        numerator = sum(c * (CHECK_VECTOR["mopup_mg"][f] * phase +
                             CHECK_VECTOR["mopup_eg"][f] * (24 - phase))
                        for f, c in enumerate(counts))
        expected = stm * men * numerator / 24.0
        worst = max(worst, abs(diff - expected))
        if abs(diff - expected) >= 2:
            bad.append("%s moved %d, expected %.2f" % (fen, diff, expected))
        for f, c in enumerate(counts):
            if c:
                slots[f] += 1
                slots[3 + f] += 1
    say("eval check: %d rows in the domain, %d outside; |evaluate() moved - expected| worst "
        "%.3f in the domain (bound 2), every outside row %s; rows with each count non-zero "
        "(centre, kings, corner): %s" % (inside, len(fens) - inside, worst,
                                         "0" if not any("outside" in b for b in bad) else
                                         "MOVED", slots[:3]))
    for line in bad[:10]:
        say("  " + line)
    if bad:
        raise Refused("the build does not evaluate the constants written: %d rows" % len(bad))
    if inside == 0 or not all(slots[:3]):
        raise Refused("the eval check's rows leave a count at zero everywhere")
    write_vector(scratch, st["landed"])


# ----------------------------------------------------------------------------- setup

def out_dir(args):
    return os.path.abspath(args.out)


def load_setup(out):
    path = os.path.join(out, "setup.json")
    if not os.path.exists(path):
        raise Refused("no %s; run setup first" % path)
    with open(path) as handle:
        return json.load(handle)


def train_header(path):
    with open(path) as handle:
        first = handle.readline()
    match = re.match(r"^# S124 P0 starts: pool (\S+) sha256 ([0-9a-f]+), held-out view (\S+) "
                     r"sha256 ([0-9a-f]+), seed (\d+), per-class (\d+), skip (\d+)(.*)$", first)
    if not match:
        raise Refused("%s is not S124_p0.py select's output" % path)
    return {"pool": match.group(1), "pool_sha": match.group(2), "heldout": match.group(3),
            "heldout_sha": match.group(4), "seed": int(match.group(5)),
            "per_class": int(match.group(6)), "skip": int(match.group(7)),
            "rest": match.group(8).strip()}


def setup(args):
    out = out_dir(args)
    source = git(args.source, "rev-parse", "--show-toplevel").strip()
    sha = git(source, "rev-parse", "HEAD").strip()
    smoke = bool(getattr(args, "smoke_mode", False))
    if os.path.exists(os.path.join(out, "setup.json")):
        st = load_setup(out)
        if st["sha"] != sha or st["smoke"] != smoke:
            raise Refused("%s was set up at %s (smoke %s), not HEAD %s; take a fresh OUT" %
                          (out, st["sha"][:7], st["smoke"], sha[:7]))
        say("setup: already done at %s" % sha[:7])
        return st
    os.makedirs(os.path.join(out, "bin"), exist_ok=True)
    os.makedirs(os.path.join(out, "logs"), exist_ok=True)
    logs = os.path.join(out, "logs")

    # The script and its helpers at HEAD: P1 runs what the tree commits, as the screen does.
    script_sha = sha256(os.path.abspath(__file__))
    try:
        committed = sha256_text(git(source, "show", "HEAD:adocs/data/S124_p1.py"))
    except subprocess.CalledProcessError:
        committed = None
    if committed != script_sha and not smoke:
        raise Refused("adocs/data/S124_p1.py differs from HEAD's; commit it or run the smoke")
    helpers = {}
    for name, path in (("S124_p0.py", "adocs/data/S124_p0.py"),
                       ("spsa_driver.py", "tools/spsa_driver.py"),
                       ("S124_engine_eval.cpp", "adocs/data/S124_engine_eval.cpp")):
        copy = os.path.join(out, "bin", name)
        with open(copy, "w") as handle:
            handle.write(git(source, "show", "HEAD:" + path))
        helpers[name] = sha256(copy)

    header = train_header(args.train_starts)
    if header["skip"] != 0 or header["rest"]:
        raise Refused("%s is not P0's full selection (skip %d %s)" % (
            args.train_starts, header["skip"], header["rest"]))
    # P0's whole selection, frozen here: the test holds the fresh starts disjoint from it,
    # so the run does not depend on the screen's directory outliving it.
    p0_copy = os.path.join(out, "bin", "p0_starts.tsv")
    shutil.copy2(args.train_starts, p0_copy)
    p0 = load_module("S124_p0", os.path.join(out, "bin", "S124_p0.py"))
    starts = p0.read_starts(args.train_starts)
    by_class = collections.OrderedDict((c, []) for c in BATCH)
    for s in starts:
        by_class[s["class"]].append(s)
    limit = SMOKE["train_limit"] if smoke else 0
    batch = {c: (SMOKE["batch"] if smoke else m) for c, m in BATCH.items()}
    train = []
    for name, rows in by_class.items():
        rows = sorted(rows, key=lambda s: s["id"])
        if limit:
            rows = rows[:limit]
        if not rows or len(rows) % batch[name]:
            raise Refused("class %s: %d training starts, not a multiple of its batch %d" % (
                name, len(rows), batch[name]))
        train.extend(rows)
    train_path = os.path.join(out, "train.tsv")
    with open(train_path, "w") as handle:
        handle.write("# S124 P1 training starts: %s sha256 %s%s\n" % (
            args.train_starts, sha256(args.train_starts),
            ", the smoke's first %d a class" % limit if limit else ""))
        handle.write("\t".join(p0.FIELDS[:7]) + "\n")
        for s in train:
            handle.write("\t".join(s[f] for f in p0.FIELDS[:7]) + "\n")

    scratch = {name: os.path.join(out, "scratch", name) for name in SCRATCHES}
    for name in SCRATCHES:
        make_scratch(source, sha, scratch[name], os.path.join(logs, "configure.log"))
    landed = read_vector(scratch["head"])
    if landed != {"mopup_mg": [0, 0, 0], "mopup_eg": [0, 0, 0]}:
        raise Refused("the landed mop-up is %s, not the zeros this registration starts from" %
                      vector_text(landed))
    seconds = sum(build([scratch[name]], 6, logs) for name in SCRATCHES)
    tree_engine = os.path.join(source, "build", "src", "chesso")
    if not os.path.exists(tree_engine):
        raise Refused("no %s; build build/ at HEAD" % tree_engine)
    head_bench, tree_bench = bench(engine_of(scratch["head"])), bench(tree_engine)
    say("setup: %s at %s, four scratch copies built in %.0f s; bench %d head's copy, %d "
        "build/" % (source, sha[:7], seconds, head_bench, tree_bench))
    if head_bench != tree_bench:
        raise Refused("the head copy benches %d and build/ %d: not one engine" % (
            head_bench, tree_bench))

    st = {"out": out, "source": source, "sha": sha, "smoke": smoke, "scratch": scratch,
          "landed": landed, "bench": head_bench,
          "train": train_path, "train_sha": sha256(train_path),
          "train_starts": p0_copy, "train_starts_from": os.path.abspath(args.train_starts),
          "train_starts_sha": sha256(p0_copy), "train_header": header,
          "helpers": helpers, "script_sha": script_sha, "script_committed": committed,
          "iterations": SMOKE["iterations"] if smoke else ITERATIONS, "batch": batch,
          "workers": SMOKE["workers"] if smoke else WORKERS, "nodes": NODES,
          "plant": PLANT}
    # Every training start and the outside rows, through the engine's own library.
    eval_check(st, scratch["final"], [s["fen"] for s in train] + list(OUTSIDE), logs)
    # ROOM, read by the compiler and held to the assert: ROOM compiles, ROOM + 1 fires.
    bound = stage_one_bound(scratch["final"], logs)
    room = HALF_MAX - bound + (1 if PLANT == "room" else 0)
    held = {}
    for target in (room, room + 1):
        write_vector(scratch["final"], exact_reach_vector(target))
        held[target] = syntax_check(scratch["final"])
    write_vector(scratch["final"], landed)
    say("room: stage_one_bound %d, ROOM %d; a half reaching %d compiles %s, %d fires the "
        "assert %s" % (bound, room, room, held[room][0], room + 1, held[room + 1][1]))
    if not held[room][0] or held[room + 1][0] or not held[room + 1][1]:
        raise Refused("ROOM %d is not the assert's: %s" % (room, held))

    st["room"], st["stage_one_bound"] = room, bound
    head_copy = os.path.join(out, "bin", "head.chesso")
    shutil.copy2(engine_of(scratch["head"]), head_copy)
    st["head_engine"], st["head_sha"] = head_copy, sha256(head_copy)
    atomic_write(os.path.join(out, "setup.json"), json.dumps(st, indent=2, sort_keys=True) +
                 "\n")
    say("setup written to %s" % os.path.join(out, "setup.json"))
    return st


# ------------------------------------------------------------------------------- run

def config_of(st, sd):
    raw = {"iterations": st["iterations"], "pairs_per_iter": sum(st["batch"].values()),
           "alpha": sd.ALPHA, "gamma": sd.GAMMA, "a_ratio": A_RATIO, "r_end": R_END,
           "seed": SEED,
           "params": [{"name": n, "start": st["landed"][a][i], "min": lo, "max": hi,
                       "c_end": c} for n, a, i, lo, hi, c in PARAMS]}
    return sd.Config(raw)


def to_vector(st, cfg, floats):
    vector = {name: list(st["landed"][name]) for name in ARRAYS}
    for (name, array, index, _, _, _), param, x in zip(PARAMS, cfg.params, floats):
        vector[array][index] = param.uci_value(x)
    for array, index in FROZEN:
        vector[array][index] = st["landed"][array][index]
    return clamp_reach(vector, st["room"])


def batch_of(st, train, k):
    """Iteration k's starts: per class, the k-th run of `batch` in the stream of seeded
    shuffles of the class's ids, one shuffle a cycle."""
    by_class = collections.OrderedDict((c, []) for c in st["batch"])
    for s in train:
        by_class[s["class"]].append(s)
    out = []
    for name, rows in by_class.items():
        m = st["batch"][name]
        per = len(rows) // m
        cycle, offset = divmod(k, per)
        order = sorted(rows, key=lambda s: s["id"])
        random.Random("%d %s %d" % (SEED, name, cycle)).shuffle(order)
        out.extend(order[offset * m:(offset + 1) * m])
    return out


def score(record, penalty):
    return int(record["plies"]) if record["converted"] == "1" else penalty


def play_pairs(p0, jobs, workers, nodes):
    """jobs: (key, start, engine); returns {key: record}."""
    results = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futures = {pool.submit(p0.play_one, start, engine, nodes): key
                   for key, start, engine in jobs}
        for future in concurrent.futures.as_completed(futures):
            results[futures[future]] = future.result()
    return results


def run_digest(st, cfg):
    return sha256_text(json.dumps([cfg.raw, st["sha"], st["train_sha"], st["room"],
                                   st["landed"], st["helpers"], st["script_sha"],
                                   st["batch"], st["nodes"], st["plant"], PENALTY_FACTOR],
                                  sort_keys=True))[:16]


TRAJECTORY = ("k", "c_scale", "r_k", "y", "conv_plus", "conv_minus", "score_plus",
              "score_minus", "clamped", "plus", "minus", "theta", "build_s", "games_s",
              "plus_sha", "minus_sha")


def real_objective(st, p0, penalty):
    """Each point written into its scratch copy and built, then the batch played by both
    builds, paired by start."""
    scratch, logs = st["scratch"], os.path.join(st["out"], "logs")

    def play(k, plus, minus, batch):
        write_vector(scratch["plus"], plus)
        write_vector(scratch["minus"], minus)
        build_s = build([scratch["plus"], scratch["minus"]], 2, logs)
        engines = {"plus": engine_of(scratch["plus"]), "minus": engine_of(scratch["minus"])}
        shas = {side: sha256(path) for side, path in engines.items()}
        jobs = [((side, s["id"]), s, engines[side]) for s in batch for side in ("plus", "minus")]
        t0 = time.time()
        records = play_pairs(p0, jobs, st["workers"], st["nodes"])
        games_s = time.time() - t0
        sp = [score(records[("plus", s["id"])], penalty) for s in batch]
        sm = [score(records[("minus", s["id"])], penalty) for s in batch]
        conv = {side: sum(records[(side, s["id"])]["converted"] == "1" for s in batch)
                for side in ("plus", "minus")}
        lines = ["# S124 P1 iteration %d: plus %s sha256 %s, minus %s sha256 %s, nodes %d, "
                 "penalty %d\n" % (k, vector_text(plus), shas["plus"], vector_text(minus),
                                   shas["minus"], st["nodes"], penalty),
                 "id\tclass\tplus_converted\tplus_plies\tplus_end\tminus_converted\t"
                 "minus_plies\tminus_end\n"]
        for s in batch:
            a, b = records[("plus", s["id"])], records[("minus", s["id"])]
            lines.append("\t".join([s["id"], s["class"], a["converted"], a["plies"], a["end"],
                                    b["converted"], b["plies"], b["end"]]) + "\n")
        return sp, sm, conv, "".join(lines), build_s, games_s, shas
    return play


# The smoke's synthetic objective, in the real one's units: plies a start, a quadratic
# whose excess over its floor at the seed is SIM_GAIN per parameter, each parameter's
# optimum SIM_OPT, and a per-start paired difference of SIM_SD plies. Its steepness is a
# guess written here (the corner's gradient at the seed about -0.75 plies a unit, the
# draft's KBNvK failures), not a measurement: it shows the update's sign and the
# schedule's reach, and calibrates nothing beyond that guess.
SIM_OPT = (64, -32, 24, 16, 40)
SIM_GAIN = (2.0, 2.0, 6.0, 4.0, 15.0)
SIM_FLOOR = 60.0
SIM_SD = 50.0


def sim_loss(vector):
    values = [vector[array][index] for _, array, index, _, _, _ in PARAMS]
    return SIM_FLOOR + sum(g * ((v - o) / float(o)) ** 2
                           for v, o, g in zip(values, SIM_OPT, SIM_GAIN))


def sim_objective():
    def play(k, plus, minus, batch):
        noise = random.Random("%d sim %d" % (SEED, k))
        fp, fm = sim_loss(plus), sim_loss(minus)
        sp = [fp + noise.gauss(0.0, SIM_SD / 2 ** 0.5) for _ in batch]
        sm = [fm + noise.gauss(0.0, SIM_SD / 2 ** 0.5) for _ in batch]
        return sp, sm, {"plus": 0, "minus": 0}, None, 0.0, 0.0, {"plus": "sim", "minus": "sim"}
    return play


def run(args, sim=False, flip=False, name="run"):
    """The SPSA loop. `sim` plays the synthetic objective at the registered iterations and
    batch, no build and no game; `flip` negates the update, the smoke's test hook."""
    out = out_dir(args)
    st = load_setup(out)
    if sha256(os.path.abspath(__file__)) != st["script_sha"]:
        raise Refused("S124_p1.py changed since setup")
    sd = load_module("spsa_driver", os.path.join(out, "bin", "spsa_driver.py"))
    p0 = load_module("S124_p0", os.path.join(out, "bin", "S124_p0.py"))
    for helper, digest in st["helpers"].items():
        if sha256(os.path.join(out, "bin", helper)) != digest:
            raise Refused("%s is not the copy frozen at setup" % helper)
    if sim:
        st = dict(st, iterations=ITERATIONS, batch=dict(BATCH))
    cfg = config_of(st, sd)
    for warning in cfg.warnings():
        say("WARNING: " + warning)
    penalty = PENALTY_FACTOR * p0.BOUND
    if sim:
        train = [{"id": "%s:%d" % (c, i), "class": c} for c, m in BATCH.items()
                 for i in range(m)]
        play = sim_objective()
    else:
        if sha256(st["train"]) != st["train_sha"]:
            raise Refused("%s is not the training set written at setup" % st["train"])
        train = p0.read_starts(st["train"])
        play = real_objective(st, p0, penalty)
    rundir = os.path.join(out, name)
    iters = os.path.join(rundir, "iters")
    os.makedirs(iters, exist_ok=True)
    ck_path = os.path.join(rundir, "checkpoint.json")
    tj_path = os.path.join(rundir, "trajectory.tsv")
    final_path = os.path.join(out if name == "run" else rundir, "final.txt")
    digest = run_digest(st, cfg) + ("-sim" if sim else "") + ("-flip" if flip else "")
    rng = random.Random(SEED)
    theta = [float(p.start) for p in cfg.params]
    k0 = 0
    if os.path.exists(ck_path):
        if not args.resume:
            raise Refused("a run is in %s; --resume continues it" % rundir)
        with open(ck_path) as handle:
            ck = json.load(handle)
        if ck["digest"] != digest:
            raise Refused("the checkpoint was written under another config or tree")
        k0, theta = int(ck["k"]), [float(x) for x in ck["theta"]]
        state = ck["rng"]
        rng.setstate((state[0], tuple(state[1]), state[2]))
        with open(tj_path) as handle:
            kept = [line for line in handle
                    if line.startswith("k\t") or int(line.split("\t", 1)[0]) < k0]
        atomic_write(tj_path, "".join(kept))
        say("resumed at iteration %d of %d" % (k0, cfg.iterations))
    elif os.path.exists(os.path.join(rundir, "run.json")):
        # A run that failed inside iteration 0 left its frozen config and no checkpoint:
        # resumed, it starts again at 0 with the same draw and batch.
        if not args.resume:
            raise Refused("a run is in %s with no checkpoint; --resume restarts it at 0" % rundir)
        with open(os.path.join(rundir, "run.json")) as handle:
            if json.load(handle)["digest"] != digest:
                raise Refused("the run in %s was written under another config or tree" % rundir)
        atomic_write(tj_path, "\t".join(TRAJECTORY) + "\n")
        say("resumed at iteration 0 of %d, no checkpoint written yet" % cfg.iterations)
    elif os.path.exists(tj_path):
        raise Refused("%s exists without a run.json; take a fresh OUT" % tj_path)
    else:
        atomic_write(os.path.join(rundir, "run.json"), json.dumps(
            {"config": cfg.raw, "digest": digest, "penalty": penalty,
             "params": PARAMS, "frozen": FROZEN}, indent=2, sort_keys=True) + "\n")
        atomic_write(tj_path, "\t".join(TRAJECTORY) + "\n")
    sign = -1.0 if flip != (PLANT == "sign") else 1.0
    began = time.time()
    for k in range(k0, cfg.iterations):
        if args.stop_after is not None and k >= args.stop_after:
            say("stopped before iteration %d (--stop-after)" % k)
            return st
        c_scale, r_k = sd.schedule(k, cfg)
        delta = [rng.choice((-1.0, 1.0)) for _ in cfg.params]
        plus_f = [t + p.c_end * c_scale * d for t, p, d in zip(theta, cfg.params, delta)]
        minus_f = [t - p.c_end * c_scale * d for t, p, d in zip(theta, cfg.params, delta)]
        plus, plus_clamped = to_vector(st, cfg, plus_f)
        minus, minus_clamped = to_vector(st, cfg, minus_f)
        batch = batch_of(st, train, k)
        sp, sm, conv, lines, build_s, games_s, shas = play(k, plus, minus, batch)
        if getattr(args, "fail_at", None) == k:
            raise Refused("the smoke's planted failure inside iteration %d" % k)
        # y > 0 when theta+ mates sooner: theta moves toward theta+, Spall's descent on the
        # plies, spsa_driver's form of the step.
        y = sum(m - p for p, m in zip(sp, sm)) / len(batch)
        for i, p in enumerate(cfg.params):
            step = r_k * p.c_end * c_scale * y * delta[i]
            theta[i] = min(p.hi, max(p.lo, theta[i] + sign * step))
        if lines is not None:
            atomic_write(os.path.join(iters, "k%03d.tsv" % k), lines)
        row = [k, "%.6f" % c_scale, "%.8g" % r_k, "%.4f" % y, conv["plus"], conv["minus"],
               "%.3f" % (sum(sp) / len(sp)), "%.3f" % (sum(sm) / len(sm)),
               "%d%d" % (plus_clamped, minus_clamped), vector_text(plus), vector_text(minus),
               ",".join("%.4f" % t for t in theta), "%.1f" % build_s, "%.1f" % games_s,
               shas["plus"][:12], shas["minus"][:12]]
        with open(tj_path, "a") as handle:
            handle.write("\t".join(str(c) for c in row) + "\n")
        state = rng.getstate()
        atomic_write(ck_path, json.dumps({"k": k + 1, "theta": theta, "digest": digest,
                                          "rng": [state[0], list(state[1]), state[2]]}) + "\n")
        if not sim:
            say("iteration %d: y %+.2f plies, converted %d / %d of %d, build %.1f s, games "
                "%.0f s, theta %s" % (k, y, conv["plus"], conv["minus"], len(batch), build_s,
                                      games_s, ",".join("%.2f" % t for t in theta)))
    final, clamped = to_vector(st, cfg, theta)
    text = ("# S124 P1's candidate: theta after %d iterations, clamped and rounded%s%s\n"
            "theta %s\nvector %s\n"
            "constexpr int mopup_mg[MOPUP_FEATURE_COUNT] = {%d, %d, %d};\n"
            "constexpr int mopup_eg[MOPUP_FEATURE_COUNT] = {%d, %d, %d};\n" % (
                cfg.iterations, ", held under ROOM" if clamped else "",
                ", the synthetic objective's" if sim else "",
                ",".join("%.6f" % t for t in theta), vector_text(final),
                *final["mopup_mg"], *final["mopup_eg"]))
    atomic_write(final_path, text)
    say("%s: %d iterations in %.0f s this session; the candidate %s" % (
        name, cfg.iterations - k0, time.time() - began, vector_text(final)))
    return final


# ------------------------------------------------------------------------------ test

def read_final(out):
    path = os.path.join(out, "final.txt")
    if not os.path.exists(path):
        raise Refused("no %s; the run has not finished" % path)
    with open(path) as handle:
        text = handle.read()
    match = re.search(r"^vector (-?\d+),(-?\d+),(-?\d+)\|(-?\d+),(-?\d+),(-?\d+)$", text, re.M)
    values = [int(x) for x in match.groups()]
    return {"mopup_mg": values[:3], "mopup_eg": values[3:]}


def p0_command(out, *args):
    command = [sys.executable, os.path.join(out, "bin", "S124_p0.py")] + list(args)
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        raise Refused("S124_p0.py %s failed: %s" % (args[0], (result.stdout +
                                                               result.stderr)[-600:]))
    return result.stdout


def position_of(fen):
    return " ".join(fen.split()[:4])


def test(args):
    out = out_dir(args)
    st = load_setup(out)
    p0 = load_module("S124_p0", os.path.join(out, "bin", "S124_p0.py"))
    tdir = os.path.join(out, "test")
    os.makedirs(tdir, exist_ok=True)
    logs = os.path.join(out, "logs")
    final = read_final(out)
    write_vector(st["scratch"]["final"], final)
    build([st["scratch"]["final"]], 6, logs)
    cand = os.path.join(out, "bin", "final.chesso")
    shutil.copy2(engine_of(st["scratch"]["final"]), cand)
    if sha256(st["head_engine"]) != st["head_sha"]:
        raise Refused("the head engine is not the copy frozen at setup")
    say("test: head %s, the candidate %s (%s), bench %d" % (
        vector_text(st["landed"]), vector_text(final), sha256(cand)[:12], bench(cand)))

    # The fresh starts: the training selection's pool, view and seed, the next 200.
    h = st["train_header"]
    fresh = os.path.join(tdir, "fresh.tsv")
    if not os.path.exists(fresh):
        extra = ["--smoke", str(SMOKE["fresh"])] if st["smoke"] else []
        t0 = time.time()
        said = p0_command(out, "select", "--pool", h["pool"], "--heldout", h["heldout"],
                          "--out", fresh + ".tmp", "--seed", str(h["seed"]),
                          "--per-class", str(h["per_class"]), "--skip", str(FRESH_SKIP),
                          *extra)
        with open(os.path.join(tdir, "select.txt"), "w") as handle:
            handle.write(said + "selected in %.0f s\n" % (time.time() - t0))
        fh = train_header(fresh + ".tmp")
        if (fh["pool_sha"], fh["heldout_sha"]) != (h["pool_sha"], h["heldout_sha"]):
            raise Refused("the pool or held-out view moved since P0 selected its starts")
        os.replace(fresh + ".tmp", fresh)
    with open(os.path.join(tdir, "select.txt")) as handle:
        say(handle.read().rstrip())
    fresh_starts = p0.read_starts(fresh)
    if sha256(st["train_starts"]) != st["train_starts_sha"]:
        raise Refused("%s is not the copy of P0's starts frozen at setup" % st["train_starts"])
    every_train = p0.read_starts(st["train_starts"])
    overlap_ids = {s["id"] for s in fresh_starts} & {s["id"] for s in every_train}
    overlap_pos = ({position_of(s["fen"]) for s in fresh_starts} &
                   {position_of(s["fen"]) for s in every_train})
    if overlap_ids or overlap_pos:
        raise Refused("fresh and training starts meet: %d ids, %d positions" % (
            len(overlap_ids), len(overlap_pos)))
    by_class = collections.Counter(s["class"] for s in fresh_starts)
    say("fresh: %d starts, %s; disjoint from P0's %d by id and by position" % (
        len(fresh_starts), ", ".join("%s %d" % (c, by_class[c]) for c in p0.CLASSES),
        len(every_train)))

    # KBBvK has no fresh start: its training starts, in sample, can only refuse.
    kbb = os.path.join(tdir, "kbb_train.tsv")
    with open(kbb, "w") as handle:
        handle.write("# S124 P1 KBBvK's training starts, in sample: %s\n" % st["train"])
        handle.write("\t".join(p0.FIELDS[:7]) + "\n")
        for s in p0.read_starts(st["train"]):
            if s["class"] == "KBBvK":
                handle.write("\t".join(s[f] for f in p0.FIELDS[:7]) + "\n")
    plays = (("head_fresh", fresh, st["head_engine"]), ("cand_fresh", fresh, cand),
             ("head_kbb", kbb, st["head_engine"]), ("cand_kbb", kbb, cand))
    for name, starts, engine in plays:
        target = os.path.join(tdir, name + ".tsv")
        if not os.path.exists(target):
            t0 = time.time()
            p0_command(out, "play", "--starts", starts, "--engine", engine, "--out",
                       target + ".tmp", "--nodes", str(st["nodes"]), "--workers",
                       str(st["workers"]))
            os.replace(target + ".tmp", target)
            say("played %s in %.0f s" % (name, time.time() - t0))
        say(p0_command(out, "read", target).rstrip())
    kept, lines = exact_reading(p0.read_records(os.path.join(tdir, "head_fresh.tsv")),
                                p0.read_records(os.path.join(tdir, "cand_fresh.tsv")),
                                p0.read_records(os.path.join(tdir, "head_kbb.tsv")),
                                p0.read_records(os.path.join(tdir, "cand_kbb.tsv")), p0.CLASSES)
    for line in lines:
        say(line)
    said = p0_command(out, "mcnemar", os.path.join(tdir, "head_fresh.tsv"),
                      os.path.join(tdir, "cand_fresh.tsv"))
    say("beside, deciding nothing: the rule DEC-358 replaced, S124_p0.py mcnemar")
    say("\n".join("  " + line for line in said.rstrip().splitlines()))
    say("S124-P1-KEPT" if kept else "S124-P1-NOT-KEPT")
    return kept


# ------------------------------------------------------------------- the kept rule

def binomial_tail(n, k):
    """P(X >= k), X ~ Binomial(n, 1/2), exactly."""
    if k <= 0:
        return 1.0
    return float(fractions.Fraction(sum(math.comb(n, i) for i in range(k, n + 1)), 2 ** n))


def discordant(head, cand):
    """Per class (head alone converts, the candidate alone converts, starts, head's, the
    candidate's), the two records paired by id."""
    a = {r["id"]: r for r in head}
    b = {r["id"]: r for r in cand}
    if set(a) != set(b):
        raise Refused("the two records do not hold the same starts")
    out = collections.OrderedDict()
    for i in sorted(a):
        row = out.setdefault(a[i]["class"], [0, 0, 0, 0, 0])
        ha, ca = a[i]["converted"] == "1", b[i]["converted"] == "1"
        row[0] += ha and not ca
        row[1] += ca and not ha
        row[2] += 1
        row[3] += ha
        row[4] += ca
    return out


def exact_reading(fresh_head, fresh_cand, kbb_head, kbb_cand, classes):
    """DEC-358's rule: kept iff the exact one-sided McNemar test over every fresh start, the
    candidate converting more, reads p < P_KEPT, and no class -- KBBvK's training starts,
    in sample, among them -- reads the exact one-sided test the other way, the candidate
    converting fewer, at p < P_KEPT. Returns (kept, the lines that say why)."""
    fresh = discordant(fresh_head, fresh_cand)
    head_alone = sum(r[0] for r in fresh.values())
    cand_alone = sum(r[1] for r in fresh.values())
    p_more = binomial_tail(head_alone + cand_alone, cand_alone)
    lines = ["exact McNemar (DEC-358): %d fresh starts; head alone converts %d, the candidate "
             "alone %d; one-sided p %.4g that it converts more (kept below %g)" % (
                 sum(r[2] for r in fresh.values()), head_alone, cand_alone, p_more, P_KEPT),
             "  class  set       starts  head  cand  head-alone  cand-alone  p(fewer)"]
    refused = []
    rows = [(name, "fresh", fresh[name]) for name in classes if name in fresh]
    kbb = discordant(kbb_head, kbb_cand)
    rows += [(name, "in-sample", kbb[name]) for name in kbb]
    for name, where, (b, c, n, h, k) in rows:
        p_fewer = binomial_tail(b + c, b)
        # The `raw` plant puts back the raw count comparison DEC-358 replaced.
        refuse = (k < h) if PLANT == "raw" else (p_fewer < P_KEPT)
        lines.append("  %-6s %-9s %6d %5d %5d %11d %11d  %.4g%s" % (
            name, where, n, h, k, b, c, p_fewer, "  REFUSED" if refuse else ""))
        if refuse:
            refused.append("%s (%s)" % (name, where))
    kept = p_more < P_KEPT and not refused
    lines.append("P1 %s%s" % ("kept" if kept else "not kept",
                              (": converts fewer in " + ", ".join(refused)) if refused else
                              ("" if kept else ": p %.4g is not under %g" % (p_more, P_KEPT))))
    return kept, lines


def synthetic_records(path, rows):
    """S124_p0.py's record format, for the rule's check: rows of (id, class, converted)."""
    with open(path, "w") as handle:
        handle.write("# S124 P1 rule-check: synthetic records, no game played\n")
        handle.write("id\tclass\tgame\tply\tfen\twdl\tdtz\tconverted\tplies\tend\tmoves\n")
        for ident, name, converted in rows:
            handle.write("\t".join([ident, name, ident.split(":")[1], "0",
                                    "8/8/8/8/8/8/8/K6k w - - 0 1", "2", "0", str(converted),
                                    "40" if converted else "100",
                                    "mate" if converted else "fifty", ""]) + "\n")


def rule_check(out, p0_path):
    """DEC-358's rule on synthetic paired records, and the rule it replaced beside: b = 0,
    c = 3 is not kept; a KBNvK gain with one KRBvK start lost by chaos is kept, which the
    old rule refused; a class losing significantly, fresh or KBBvK in sample, refuses."""
    p0 = load_module("S124_p0", p0_path)
    os.makedirs(out, exist_ok=True)
    sizes = (("KRvK", 200, 158), ("KQvK", 200, 200), ("KRBvK", 200, 183), ("KRNvK", 200, 190),
             ("KBNvK", 184, 21))

    def head_rows():
        return [["%s:%d" % (name, i), name, int(i < conv)] for name, n, conv in sizes
                for i in range(n)]

    def flip(rows, name, frm, count):
        done = 0
        for row in rows:
            if row[1] == name and row[2] == frm and done < count:
                row[2] = 1 - frm
                done += 1
        if done != count:
            raise Refused("rule-check: %s has %d starts at %d, not %d" % (name, done, frm, count))
        return rows

    kbb_head = [["KBBvK:%d" % i, "KBBvK", int(i < 102)] for i in range(126)]
    cases = []
    cases.append(("b0c3", flip(head_rows(), "KBNvK", 0, 3), kbb_head, False, None))
    gain = flip(flip(head_rows(), "KBNvK", 0, 120), "KRBvK", 1, 1)
    cases.append(("kbn-gain-chaos", gain, kbb_head, True, None))
    loss = flip([list(r) for r in gain], "KRvK", 1, 8)
    cases.append(("class-loss", loss, kbb_head, False, "KRvK (fresh)"))
    kbb_loss = flip([list(r) for r in kbb_head], "KBBvK", 1, 6)
    cases.append(("kbb-loss", [list(r) for r in gain], kbb_loss, False, "KBBvK (in-sample)"))
    wrong = []
    for name, cand, kbb_cand, want, refusal in cases:
        paths = {}
        for side, rows in (("head_fresh", head_rows()), ("cand_fresh", cand),
                           ("head_kbb", kbb_head), ("cand_kbb", kbb_cand)):
            paths[side] = os.path.join(out, "%s_%s.tsv" % (name, side))
            synthetic_records(paths[side], rows)
        kept, lines = exact_reading(*(p0.read_records(paths[side]) for side in
                                      ("head_fresh", "cand_fresh", "head_kbb", "cand_kbb")),
                                    p0.CLASSES)
        old = subprocess.run([sys.executable, p0_path, "mcnemar", paths["head_fresh"],
                              paths["cand_fresh"]], capture_output=True, text=True).stdout
        old_verdict = re.search(r"^P1 .*$", old, re.M).group(0)
        say("rule-check %s: %s; the replaced rule, %s" % (name, lines[-1], old_verdict))
        if kept != want or (refusal and refusal not in lines[-1]):
            wrong.append(name)
    if wrong:
        raise Refused("DEC-358's rule misreads %s" % ", ".join(wrong))


# ----------------------------------------------------------------------------- smoke

def smoke(args):
    out = out_dir(args)
    flag = os.path.join(out, "smoke.flag")
    if os.path.exists(out):
        if not os.path.exists(flag):
            raise Refused("%s exists and is no smoke's; take another --out" % out)
        shutil.rmtree(out)
    os.makedirs(out)
    open(flag, "w").close()
    args.smoke_mode = True
    began = time.time()
    st = setup(args)
    logs = os.path.join(out, "logs")
    p0 = load_module("S124_p0", os.path.join(out, "bin", "S124_p0.py"))

    # The clamp: a vector far past ROOM in both halves comes back under it and compiles.
    far = {"mopup_mg": [400, 400, 0], "mopup_eg": [800, 800, 800]}
    clamped, flagged = clamp_reach(far, st["room"])
    write_vector(st["scratch"]["final"], clamped)
    holds, fired = syntax_check(st["scratch"]["final"])
    write_vector(st["scratch"]["final"], st["landed"])
    say("clamp: %s clamped to %s, reaches %d and %d of ROOM %d; compiles %s" % (
        vector_text(far), vector_text(clamped), reach(clamped["mopup_mg"]),
        reach(clamped["mopup_eg"]), st["room"], holds))
    if not holds or fired or not flagged:
        raise Refused("a clamped point fails the stage-one assert")

    # At the landed zeros the writer is the identity: the plus copy benches head's.
    write_vector(st["scratch"]["plus"], CHECK_VECTOR)
    build([st["scratch"]["plus"]], 6, logs)
    moved = bench(engine_of(st["scratch"]["plus"]))
    write_vector(st["scratch"]["plus"], st["landed"])
    seconds = build([st["scratch"]["plus"]], 6, logs)
    zero = bench(engine_of(st["scratch"]["plus"]))
    say("identity: at %s the plus copy benches %d, at the landed %s %d, head %d (rebuilt in "
        "%.1f s)" % (vector_text(CHECK_VECTOR), moved, vector_text(st["landed"]), zero,
                     st["bench"], seconds))
    if zero != st["bench"] or moved == st["bench"]:
        raise Refused("the writer at the landed zeros is not head, or the check vector is")

    # The update's sign and the schedule's reach: the registered schedule and batch on the
    # synthetic objective must remove half its excess at the seed, and the same run with
    # the update negated must remove none.
    args.resume, args.stop_after = False, None
    excess = sim_loss(st["landed"]) - SIM_FLOOR
    honest = sim_loss(run(args, sim=True, name="sim")) - SIM_FLOOR
    flipped = sim_loss(run(args, sim=True, flip=True, name="sim_flip")) - SIM_FLOOR
    say("synthetic: excess %.2f plies at the seed, %.2f after the registered run, %.2f with "
        "the update negated" % (excess, honest, flipped))
    if honest > excess / 2 or flipped < excess:
        raise Refused("the update does not descend the synthetic objective")

    # DEC-358's kept rule on synthetic paired records, the replaced rule beside.
    rule_check(os.path.join(out, "rule_check"), os.path.join(out, "bin", "S124_p0.py"))

    # The iteration: a planted failure inside iteration 0, before any checkpoint, resumed
    # at 0; then stopped after two and resumed for the third.
    args.resume, args.stop_after, args.fail_at = False, None, 0
    try:
        run(args)
        raise Refused("the planted failure inside iteration 0 did not fail")
    except Refused as exc:
        if "planted failure" not in str(exc):
            raise
        say("failed as planted: %s; checkpoint %s" % (exc, os.path.exists(
            os.path.join(out, "run", "checkpoint.json"))))
    args.resume, args.stop_after, args.fail_at = True, 2, None
    run(args)
    args.resume, args.stop_after = True, None
    run(args)
    with open(os.path.join(out, "run", "trajectory.tsv")) as handle:
        rows = [line.rstrip("\n").split("\t") for line in handle][1:]
    if [int(r[0]) for r in rows] != list(range(st["iterations"])):
        raise Refused("the trajectory holds iterations %s" % [r[0] for r in rows])
    builds = [float(r[TRAJECTORY.index("build_s")]) for r in rows]
    games = [float(r[TRAJECTORY.index("games_s")]) for r in rows]
    starts = sum(st["batch"].values())
    per_start = sum(games) * st["workers"] / (2 * starts * len(rows))
    plies = []
    for k in range(len(rows)):
        with open(os.path.join(out, "run", "iters", "k%03d.tsv" % k)) as handle:
            for line in handle:
                if line.startswith("#") or line.startswith("id\t"):
                    continue
                f = line.rstrip("\n").split("\t")
                plies += [int(f[3]), int(f[6])]
    say("timing: two builds in parallel %.1f s an iteration (%s); %d games an iteration at "
        "%d workers, %.2f core-seconds a start, %.1f plies a game, %.4f core-seconds a ply" % (
            sum(builds) / len(builds), ", ".join("%.1f" % b for b in builds), 2 * starts,
            st["workers"], per_start, sum(plies) / len(plies),
            sum(games) * st["workers"] / sum(plies)))

    # Determinism, a reading: head plays the first starts twice.
    train = p0.read_starts(st["train"])[:6]
    jobs = [((n, s["id"]), s, st["head_engine"]) for s in train for n in (0, 1)]
    twice = play_pairs(p0, jobs, st["workers"], st["nodes"])
    same = sum(twice[(0, s["id"])]["moves"] == twice[(1, s["id"])]["moves"] for s in train)
    say("determinism: head played %d starts twice, %d move lists identical" % (len(train),
                                                                               same))
    test(args)
    say("smoke: %.0f s" % (time.time() - began))
    return True


# ------------------------------------------------------------------------------ main

def main(argv):
    global PLANT
    ap = argparse.ArgumentParser(description="S124's P1, mop-up's SPSA on conversion.")
    sub = ap.add_subparsers(dest="command", required=True)
    default_out = os.path.join(os.path.dirname(os.path.dirname(HERE)), ".tuning", "s124", "p1")
    for name in ("all", "setup", "run", "test", "smoke", "rule-check"):
        c = sub.add_parser(name)
        c.add_argument("--out", default=default_out if name not in ("smoke", "rule-check")
                       else None, required=name in ("smoke", "rule-check"))
        if name in ("all", "setup", "smoke"):
            c.add_argument("--train-starts", required=True,
                           help="P0's starts.tsv, S124_p0.py select's output at skip 0")
            c.add_argument("--source", default=os.path.dirname(os.path.dirname(HERE)),
                           help="the checkout whose HEAD is built (default: this one)")
        if name == "run":
            c.add_argument("--resume", action="store_true")
        if name == "smoke":
            c.add_argument("--plant", choices=("swap", "order", "room", "noclamp", "sign",
                                                "raw"), default=None)
    args = ap.parse_args(argv)
    args.stop_after = None
    args.resume = getattr(args, "resume", False)
    marker = {"all": "S124-P1", "setup": "S124-P1-SETUP", "run": "S124-P1-RUN",
              "test": "S124-P1-TEST", "smoke": "S124-P1-SMOKE",
              "rule-check": "S124-P1-RULE-CHECK"}[args.command]
    PLANT = getattr(args, "plant", None)
    try:
        if args.command == "setup":
            setup(args)
        elif args.command == "run":
            run(args)
        elif args.command == "test":
            test(args)
        elif args.command == "smoke":
            smoke(args)
        elif args.command == "rule-check":
            rule_check(out_dir(args), os.path.join(HERE, "S124_p0.py"))
        else:
            setup(args)
            args.resume = os.path.exists(os.path.join(out_dir(args), "run", "run.json"))
            run(args)
            test(args)
    except Exception as exc:  # every failure ends on the marker a watcher exits on
        say("ERROR: %s: %s" % (type(exc).__name__, exc))
        say(marker + "-FAILED")
        return 1
    say(marker + "-DONE")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
