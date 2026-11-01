#!/usr/bin/env python3
"""S307: what the timer case reads, against a dead timer and a light board.

    python3 adocs/data/S307_matrix.py readings <tree>   # needs build/, build-tune/
    python3 adocs/data/S307_matrix.py matrix <worktree> # a linked worktree, clean

`readings` asks the Release binary for the board's first iteration and the tune
binary for it at each quiescence horizon, `MaxQsearchDepth` being an option only
there, with the case's other board beside it. It reads, and changes nothing.

`matrix` runs the one case under each of TB01, TB02 and a lighter `timer_fen`,
alone and with the dead timer, on the tree's test and on the test before S307.
Each row reverts with `git checkout --`, so the worktree must hold nothing else,
and each mutated row of the tree's own test is followed by the reverted case.
The lighter `timer_fen` is here and not in tools/mutants/ because
tools/mutation_check.py refuses a file outside src/.
"""

import re
import subprocess
import sys
import time

BASE = "36b5659"  # the test as it stood before S307
TEST = "tests/test_engine.cpp"
CASE = ("a stop inside the first iteration cuts it and the hard timer ends the "
        "search within its bound")
TIMER_FEN = "rn1qk1nr/qqqqqqqq/8/8/8/8/QQQQQQQQ/RN1QK1NR w - - 0 1"
FIRST_FEN = "q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1"


def go_depth_1(binary, fen, horizon=None):
    """(wall ms from `go` to `bestmove`, the engine's `time`, its `nodes`)."""
    proc = subprocess.Popen([binary], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, text=True, bufsize=1)
    if horizon is not None:
        proc.stdin.write(f"setoption name MaxQsearchDepth value {horizon}\n")
    proc.stdin.write(f"position fen {fen}\n")
    proc.stdin.flush()
    sent = time.perf_counter()
    proc.stdin.write("go depth 1\n")
    proc.stdin.flush()
    info = ""
    while True:
        line = proc.stdout.readline()
        if line.startswith("info"):
            info = line
        if line.startswith("bestmove"):
            wall = (time.perf_counter() - sent) * 1000
            break
    proc.stdin.write("quit\n")
    proc.stdin.flush()
    proc.wait()
    fields = info.split()
    return wall, fields[fields.index("time") + 1], fields[fields.index("nodes") + 1]


def readings(tree):
    print(f"load average {open('/proc/loadavg').read().strip()}")
    print("Release, `go depth 1` on the timer board, a fresh process each run")
    for run in range(5):
        wall, reported, nodes = go_depth_1(f"{tree}/build/src/chesso", TIMER_FEN)
        print(f"  run {run + 1}: bestmove {wall:7.1f} ms, time {reported}, nodes {nodes}")
    print("tune build, `go depth 1` at each MaxQsearchDepth, ms from go to bestmove")
    print("  horizon  timer board ms    nodes  eight-queens ms  nodes  over the bound?")
    for horizon in (8, 10, 11, 12, 13, 14, 16, 19):
        wall, _, nodes = go_depth_1(f"{tree}/build-tune/src/chesso", TIMER_FEN, horizon)
        first_wall, first_time, first_nodes = go_depth_1(
            f"{tree}/build-tune/src/chesso", FIRST_FEN, horizon)
        print(f"  {horizon:7d}  {wall:14.1f} {nodes:>8}  {first_time:>15} {first_nodes:>6}"
              f"  {'yes' if wall > 402 else 'no, a dead timer passes'}")
    print(f"load average {open('/proc/loadavg').read().strip()}")


def table_pairs(worktree):
    rows = {}

    def m(mid, file, klass, note, *pairs, expected="killed", origin=None):
        rows[mid.split("_")[0]] = (file, pairs)

    source = open(f"{worktree}/tools/mutants/S307_timer_board.py").read()
    exec(compile(source, "S307_timer_board.py", "exec"), {"m": m})
    return rows


def edit(worktree, rel, old, new):
    path = f"{worktree}/{rel}"
    text = open(path).read()
    if text.count(old) != 1:
        raise SystemExit(f"anchor occurs {text.count(old)} times in {rel}: {old}")
    open(path, "w").write(text.replace(old, new))


def git(worktree, *args):
    return subprocess.run(["git", "-C", worktree, *args], check=True, text=True,
                          stdout=subprocess.PIPE).stdout


def run_case(worktree):
    subprocess.run(["cmake", "--build", f"{worktree}/build", "-j12", "--target",
                    "test_engine"], check=True, stdout=subprocess.DEVNULL)
    proc = subprocess.run([f"{worktree}/build/tests/test_engine", f"-tc={CASE}", "-d"],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    seconds = re.search(r"^([0-9.]+) s: ", proc.stdout, re.M)
    what = []
    lines = proc.stdout.splitlines()
    for index, line in enumerate(lines):
        if "ERROR:" in line or "  values:" in line:
            what.append("    " + re.sub(r"^.*/tests/", "tests/", line.strip()))
        if "logged:" in line:
            what.extend("    " + text.strip()[:150] for text in lines[index + 1:index + 3])
    return proc.returncode, float(seconds.group(1)) if seconds else -1.0, what


def row(worktree, label, test_form, mutation, rows):
    git(worktree, "checkout", "--", ".")
    if test_form == "before S307":
        open(f"{worktree}/{TEST}", "w").write(git(worktree, "show", f"{BASE}:{TEST}"))
    for name in mutation:
        if name == "light":
            edit(worktree, TEST, f'"{TIMER_FEN}"', f'"{FIRST_FEN}"')
            continue
        rel, pairs = rows[name]
        for old, new in pairs:
            edit(worktree, rel, old, new)
    code, seconds, what = run_case(worktree)
    print(f"{test_form:12} {label:28} {'green' if code == 0 else 'RED  '} {seconds:7.3f} s")
    for line in what:
        print(line)
    git(worktree, "checkout", "--", ".")
    if mutation and test_form == "this tree":
        code, seconds, _ = run_case(worktree)
        print(f"{'':12} {'  reverted':28} {'green' if code == 0 else 'RED  '} {seconds:7.3f} s")
    sys.stdout.flush()


def matrix(worktree):
    if git(worktree, "status", "--porcelain").strip():
        raise SystemExit(f"{worktree} is not clean")
    print(f"worktree {worktree} at {git(worktree, 'rev-parse', '--short', 'HEAD').strip()}, "
          f"{BASE} for the test before S307")
    print(f"the light board: timer_fen becomes {FIRST_FEN}")
    rows = table_pairs(worktree)
    for form in ("this tree", "before S307"):
        for label, mutation in (("as shipped", ()),
                                ("TB01 dead timer", ("TB01",)),
                                ("TB02 horizon 12", ("TB02",)),
                                ("light board", ("light",)),
                                ("TB01 + TB02", ("TB01", "TB02")),
                                ("TB01 + light board", ("TB01", "light"))):
            row(worktree, label, form, mutation, rows)


if __name__ == "__main__":
    {"readings": readings, "matrix": matrix}[sys.argv[1]](sys.argv[2])
