#!/usr/bin/env python3
"""Read both ends of tests/test_engine.cpp's `depth_1_floor_ms`, a GOLDEN (DEC-142).

S321. The case "a stop inside the first iteration cuts it and the hard timer
ends the search within its bound" asserts `reported_ms >= depth_1_floor_ms` on
an eight-queens board, and halves the floor into `prompt_stop_us`, the clock
its stop half reads a send against. So the floor sits between two measured
ends, DEC-116's form, and this script reads both on the machine it runs on:

  the upper end  the lowest `time` the board's depth-1 iteration reports, each
                 run a fresh `chesso` process sent `position fen` and then
                 `go depth 1`, `quit` only after `bestmove`. The case's rule:
                 the floor stands while that is at or above it, and one under
                 it takes a heavier board, not a smaller floor.
  the lower end  the stop half's send: `stop` sent and back after `go`, in
                 microseconds, as the case clocks it itself. The clock settles
                 the half's precondition only for a send under
                 `prompt_stop_us`. The case puts the figure in a message
                 context and doctest prints a passing assertion's contexts
                 under `--success`; the listener in tests/test_helpers.hpp
                 repeats them on stderr while a capture swallows stdout. So the
                 shipped test binary prints it, and no temporary build is
                 needed, which S242's reading of it had used.

The floor, the board and the halving are read out of tests/test_engine.cpp,
so the script judges what ships. Since S321 a send stands at the lower end
when its 99th percentile, by nearest rank, is under `prompt_stop_us`. The
quantile is S321's choice, made because S242 read 299 of 300 sends under it
beside a match. Both ends are printed with their margins, then every whole
floor both ends admit: the band.

**It does not produce 3.** For a floor, DEC-142's re-derivation is the
re-derivation of that band, with the chosen value and its margin stated:
DEC-116's form, by the coordinator's ruling at S321. 3 stays S210's choice
inside it, made under the 13.8 ms of its day for a four times faster machine.
Like adocs/data/S192_node_budget.py the script asserts nothing and changes
nothing: a floor outside the band is the next step's to act on.

    python3 adocs/data/S321_depth1_floor.py [--build build] [--runs 200] [--sends 300]

Run from anywhere; the paths are the repository's. Machine load lengthens
both readings, the `time`'s in the floor's favour, so an idle machine's
lowest `time` is the one that binds; the load averages are printed before and
after. Exits 0 with a reading. Every failure -- an input it cannot parse, a
case that did not pass, a process that timed out, died or broke its pipe, or
anything unforeseen -- prints its reason and then S321-FLOOR-REFUSED, and
exits 1, a process that timed out killed first.
"""
import argparse
import math
import os
import re
import subprocess
import sys
import threading
import traceback

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOURCE = os.path.join(REPO, "tests", "test_engine.cpp")
CASE = ("a stop inside the first iteration cuts it and the hard timer ends the "
        "search within its bound")

# The two fragments of the title as the source writes it, wrapped by
# clang-format over two adjacent literals.
TITLE = re.compile(r'"a stop inside the first iteration cuts it and the hard timer ends the "'
                   r'\s*"search within its bound"')
FLOOR = re.compile(r"const int depth_1_floor_ms = (\d+);")
BOARD = re.compile(r'const std::string fen =\s*"([^"]+)";')
HALF = re.compile(r"const int64_t prompt_stop_us = depth_1_floor_ms \* 1000 / 2;")
SEND = re.compile(r"attempt (\d+), stop sent and back (\d+) us after go")
TIME = re.compile(r" time (\d+)")
NODES = re.compile(r" nodes (\d+)")
QUANTILE = 0.99

# Deadlines, each far past what it bounds: a depth-1 process is milliseconds
# from start to exit and one run of the case about half a second.
DEPTH_1_LIMIT_S = 30
CASE_LIMIT_S = 300


def refuse(why):
    print("refused: " + why, flush=True)
    print("S321-FLOOR-REFUSED", flush=True)
    sys.exit(1)


def read_case():
    """The floor, the board and the halving, from the case's own text."""
    with open(SOURCE, encoding="utf-8") as handle:
        text = handle.read()
    title = TITLE.search(text)
    if not title:
        refuse("no case titled [" + CASE + "] in " + SOURCE)

    # Bounded at the next TEST_CASE, so a case that loses one of these lines
    # is refused rather than read off the next case.
    end = text.find("TEST_CASE", title.end())
    case = text[title.end():] if end < 0 else text[title.end():end]
    floor = FLOOR.search(case)
    board = BOARD.search(case)
    if not floor or not board or not HALF.search(case):
        refuse("the case no longer spells [const int depth_1_floor_ms = N;], its "
               "board as [const std::string fen = \"...\";] or prompt_stop_us as half "
               "the floor; read it by hand rather than trusting this script")
    return int(floor.group(1)), board.group(1)


def depth_1(engine, board):
    """One fresh process: the last info line's `time` and `nodes` at depth 1.

    One deadline covers the process from its start to its exit: a timer kills
    it DEPTH_1_LIMIT_S seconds in, which also ends a read that would otherwise
    block on an engine that never answers. It is reaped before any refusal.
    """
    try:
        process = subprocess.Popen([engine], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.DEVNULL, text=True)
    except OSError as error:
        refuse("cannot start %s: %s" % (engine, error))
    expired = threading.Event()

    def expire():
        expired.set()
        process.kill()

    timer = threading.Timer(DEPTH_1_LIMIT_S, expire)
    timer.start()
    time_ms, nodes, answered, broke = None, None, False, None
    try:
        process.stdin.write("position fen " + board + "\ngo depth 1\n")
        process.stdin.flush()
        for line in process.stdout:
            if line.startswith("info ") and TIME.search(line):
                time_ms = int(TIME.search(line).group(1))
                found = NODES.search(line)
                nodes = int(found.group(1)) if found else None
            if line.startswith("bestmove"):
                answered = True
                break

        # quit only now: sent with `go`, it ends the search before its first
        # node (TOOLCHAIN.md, the chess oracle's section).
        if answered:
            process.stdin.write("quit\n")
            process.stdin.flush()
    except BrokenPipeError:
        broke = "quit" if answered else "position and go"
    process.wait()
    timer.cancel()
    try:
        process.stdin.close()
    except BrokenPipeError:
        # Only what a dead engine left unflushed, and that is already a refusal
        # below: a live one read everything before its exit.
        pass
    process.stdout.close()

    if expired.is_set():
        refuse("a depth-1 process had not exited %d s after it started; killed"
               % DEPTH_1_LIMIT_S)
    if broke:
        refuse("a depth-1 process closed its input before %s was written, exit status %s"
               % (broke, process.returncode))
    if not answered:
        refuse("a depth-1 process exited before bestmove, exit status %s" % process.returncode)
    if process.returncode != 0:
        refuse("a depth-1 process exited with status %s after quit" % process.returncode)
    if time_ms is None:
        refuse("a depth-1 run printed no info line with a time")
    return time_ms, nodes


def sends(binary):
    """One run of the case: each attempt's send in microseconds, by attempt."""
    # run() kills the child itself when the deadline passes, then raises.
    try:
        run = subprocess.run([binary, "--test-case=" + CASE, "--success"], cwd=REPO,
                             capture_output=True, text=True, timeout=CASE_LIMIT_S)
    except subprocess.TimeoutExpired:
        refuse("a run of the case had not exited after %d s; killed" % CASE_LIMIT_S)
    except OSError as error:
        refuse("cannot start %s: %s" % (binary, error))
    if run.returncode != 0:
        refuse("the case did not pass, so its send is not a reading:\n"
               + run.stdout[-2000:] + run.stderr[-2000:])

    # Each attempt is logged by two passing assertions, or by one when the
    # clock left it unsettled; its figure is the same in both.
    seen = {}
    for attempt, us in SEND.findall(run.stdout + run.stderr):
        seen.setdefault(int(attempt), int(us))
    if not seen:
        refuse("no [stop sent and back N us after go] context under --success: the "
               "case no longer logs its send, and this script is stale, not the test")
    return [seen[attempt] for attempt in sorted(seen)]


def load():
    """The load averages: /proc/loadavg on Linux, sysctl on macOS (CLAUDE.md)."""
    if os.path.exists("/proc/loadavg"):
        with open("/proc/loadavg") as handle:
            return handle.read().strip()
    run = subprocess.run(["sysctl", "-n", "vm.loadavg"], capture_output=True, text=True)
    return run.stdout.strip() if run.returncode == 0 else "not read"


def nearest_rank(values, q):
    ordered = sorted(values)
    return ordered[max(0, math.ceil(q * len(ordered)) - 1)]


def histogram(values):
    counts = {}
    for value in values:
        counts[value] = counts.get(value, 0) + 1
    return ", ".join("%d x %d" % (counts[v], v) for v in sorted(counts))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build", default="build",
                        help="build directory, relative to the repository")
    parser.add_argument("--runs", type=int, default=200, help="depth-1 processes")
    parser.add_argument("--sends", type=int, default=300, help="runs of the case")
    args = parser.parse_args()

    build = os.path.join(REPO, args.build)
    engine = os.path.join(build, "src", "chesso")
    binary = os.path.join(build, "tests", "test_engine")
    for path in (engine, binary):
        if not os.access(path, os.X_OK):
            refuse("no executable at " + path + "; build it first")

    floor, board = read_case()
    prompt_stop_us = floor * 1000 // 2
    print("case          tests/test_engine.cpp \"%s\"" % CASE)
    print("floor         depth_1_floor_ms %d, prompt_stop_us %d us" % (floor, prompt_stop_us))
    print("board         %s" % board)
    print("binaries      %s, %s" % (engine, binary))
    print("load before   %s" % load())
    print()

    readings = [depth_1(engine, board) for _ in range(args.runs)]
    times = [time_ms for time_ms, _ in readings]
    nodes = sorted({count for _, count in readings if count is not None})
    lowest = min(times)
    print("upper end     depth-1 time over %d fresh processes, ms: %s" % (args.runs, histogram(times)))
    print("              nodes %s" % ", ".join(str(n) for n in nodes))
    print("              lowest %d against the floor %d: %s, margin %.2f times"
          % (lowest, floor, "at or above it" if lowest >= floor else "UNDER IT",
             lowest / floor))

    send = []
    for _ in range(args.sends):
        send.extend(sends(binary))
    under = sum(1 for us in send if us < prompt_stop_us)
    tail = nearest_rank(send, QUANTILE)
    print("lower end     send over %d runs of the case, %d attempts, us: median %d, p90 %d, "
          "p99 %d, max %d" % (args.sends, len(send), nearest_rank(send, 0.5),
                              nearest_rank(send, 0.9), tail, max(send)))
    print("              %d of %d under prompt_stop_us %d; p99 %s it, margin %.1f times"
          % (under, len(send), prompt_stop_us, "under" if tail < prompt_stop_us else "NOT UNDER",
             prompt_stop_us / tail if tail else math.inf))
    print("load after    %s" % load())
    print()

    # A whole floor F admits the upper end while lowest >= F and the lower end
    # while the p99 send is under F * 1000 / 2, the case's own halving.
    smallest = tail * 2 // 1000 + 1
    if smallest <= lowest:
        print("band          whole floors both ends admit: %d to %d; the floor %d %s"
              % (smallest, lowest, floor,
                 "is inside it" if smallest <= floor <= lowest else "is OUTSIDE it"))
    else:
        print("band          empty: the p99 send needs a floor of %d, the lowest time "
              "admits %d" % (smallest, lowest))
    stands = lowest >= floor and tail < prompt_stop_us
    print("S321-FLOOR-STANDS" if stands else "S321-FLOOR-OUTSIDE")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:  # refuse() exits by SystemExit, which passes
        traceback.print_exc()
        refuse("unforeseen %s: %s" % (type(error).__name__, error))
