#!/usr/bin/env python3
"""S083's corpus: the chunk check, the merge, the capacity, the filter totals
and the reading. adocs/data/S083_corpus.sh runs every one of them; each also
runs alone. Standard library only, plus tools/corpus_split.py, imported so the
capacity counts rows exactly the way `sample` keeps them.

    S083_corpus.py chunk-check FILE LOG GAMES [--abort-share P --rate-floor R]
    S083_corpus.py merge --out FILE [--base FILE --base-sha256 HEX] FILE:K ...
    S083_corpus.py capacity --in TRAIN --validation V [...] --per-game D [...]
    S083_corpus.py filters LOG [LOG ...]
    S083_corpus.py read DIR --stage N --last-stage M --mode s082|none

CHUNK-CHECK reads one datagen run of `--leaf qs` against its own log: the log
carries `exit      0`, no `datagen: FAILED` line, and a `leaf qs:` line that
plays GAMES games and names FILE; FILE is a regular file ending on a newline,
holds no NUL byte, holds the rows that line counts, every row seven fields with
numbers where datagen writes numbers, each game's rows contiguous and every
game id below GAMES (worker i of T plays ids i, i + T, ...). Prints the rows,
the games with a row and the sha256.

MERGE writes one training file: the base (S082's `pool.tsv.qs.train`, its
sha256 checked before a byte is read) copied byte for byte, then every chunk
FILE:K with its game ids moved up by ID_BASE + K * ID_STRIDE and no other byte
changed -- datagen numbers every run's games from 0, as DEC-296's top-up found.
Written under a `.merging` name, read back (no NUL, seven fields, contiguous
games, every id once, the rows and games the parts add up to), then renamed.
Refuses an existing output.

CAPACITY is how many rows `corpus_split.py sample --per-game D` can write from
TRAIN once every row on a position any validation view holds is dropped: the
sum over games of min(D, rows kept), D = 0 for every row. Sampling exactly that
many rows takes every game the pool has with a row left, which is how the
ladder's top and the second reading's corpora use every game.

FILTERS adds up datagen's own closing lines over the chunk logs -- the filter's
clauses, the qs leaf's rows, refusals by kind, how often the leaf differs from
the root, the walks' nodes, the coverage matrix and each chunk's elapsed time --
for the step file, which is where the accepts want them.

READ applies the rule adocs/data/S083_corpus.sh pre-registers to one stage's
fit logs, mechanically; the script's header is the rule's statement and this is
its implementation. Exit 0 with an outcome, `continue` or `stop`; exit 1 on
outcome (c), unreadable. Under MODE=none stage 0 is informational: reported,
readable or not, and always `continue`.

CHUNK-CHECK with --abort-share and --rate-floor also applies the
pre-registered abort rule to the run's own log and exits 2 when it fires.
"""

import argparse
import hashlib
import math
import os
import re
import statistics
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(
    __file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import corpus_split  # noqa: E402  (tools/, by path)

ID_BASE = 2000000    # above every id S082's pool holds, 1027251 the largest
ID_STRIDE = 100000   # above every id one 12000-game chunk writes
PICK_Z = 1.645       # one-sided 95 % point of the standard normal
SPREAD_UNREADABLE = 1.0e-03

LEAF_QS = re.compile(
    r"^leaf qs: (\d+) rows from (\d+) games written to (.+?), [0-9.]+ a game; "
    r"the leaf differs from the root on (\d+) \([^)]*\), ([0-9.]+) plies")
REFUSED = re.compile(
    r"^leaf qs: not written of (\d+) passed: terminal (\d+) \([^)]*\) "
    r"truncated (\d+) \([^)]*\) aborted (\d+) \([^)]*\) uncertified (\d+)")
FILTER = re.compile(
    r"^filter: (\d+) considered, (\d+) passed; skipped in-check (\d+), "
    r"mate (\d+), score past (\d+) (\d+); no completed iteration (\d+)")
WALK = re.compile(r"^walk: (\d+) nodes, [0-9.]+ % of the (\d+) ")
COVER = re.compile(r"^\s+(\d+) to\s+(\d+)((?:\s+\d+){5})\s*$")
ELAPSED = re.compile(r"^elapsed\s+(\d+) s$")
EXIT = re.compile(r"^exit\s+(-?\d+)$")
BEST = re.compile(r"^best: train (\S+)\s+validation (\S+)\s+\(start (\S+) / "
                  r"(\S+?)\)")
K_LINE = re.compile(r"^fitted K = (\S+)")
EPOCH = re.compile(r"^epoch\s+(\d+)\s+train \S+\s+validation \S+\s+\*")
NOT_IMPROVED = "held-out error did not improve"
CORPUS = re.compile(r"(\d+) positions, (\d+) games, ")


class Refused(Exception):
    """Something other than what was asked for would be produced or read."""


class AbortRule(Exception):
    """The pre-registered abort rule fires: the run stops, exit 2."""


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def scan_rows(path, games_below=None):
    """(rows, [game ids in file order]) after the structural checks every
    reader of datagen's format leans on."""
    if not os.path.isfile(path):
        raise Refused(f"{path} is missing or not a regular file")
    if os.path.getsize(path) == 0:
        raise Refused(f"{path} is empty")
    with open(path, "rb") as handle:
        handle.seek(-1, os.SEEK_END)
        if handle.read(1) != b"\n":
            raise Refused(f"{path} does not end on a newline: its last row is "
                          "cut")
    rows = 0
    order = []
    seen = set()
    with open(path, "rb") as handle:
        for number, line in enumerate(handle, 1):
            if b"\0" in line:
                raise Refused(f"{path}:{number}: a NUL byte")
            fields = line.rstrip(b"\n").split(b"\t")
            if len(fields) != 7:
                raise Refused(f"{path}:{number}: {len(fields)} fields, "
                              "datagen's rows have seven")
            try:
                float(fields[1])
                for field in fields[2:]:
                    int(field)
            except ValueError:
                raise Refused(f"{path}:{number}: a field that is not a number "
                              "where datagen writes one") from None
            game = int(fields[4])
            if games_below is not None and not 0 <= game < games_below:
                raise Refused(f"{path}:{number}: game {game} outside 0 .. "
                              f"{games_below - 1}")
            if not order or order[-1] != game:
                if game in seen:
                    raise Refused(f"{path}:{number}: the rows of game {game} "
                                  "are not contiguous")
                seen.add(game)
                order.append(game)
            rows += 1
    return rows, order


def chunk_check(args):
    text = open(args.log, encoding="utf-8", errors="replace").read()
    lines = text.splitlines()
    if any(line.startswith("datagen: FAILED") for line in lines):
        raise Refused(f"{args.log} carries datagen's FAILED line")
    exits = [int(m.group(1)) for m in map(EXIT.match, lines) if m]
    if exits != [0]:
        raise Refused(f"{args.log}: exit lines {exits}, not one 'exit 0'")
    written = [m for m in map(LEAF_QS.match, lines) if m]
    if len(written) != 1:
        raise Refused(f"{args.log}: {len(written)} 'leaf qs:' lines, not one")
    rows, played, named = (int(written[0].group(1)), int(written[0].group(2)),
                           written[0].group(3))
    if played != args.games:
        raise Refused(f"{args.log}: {played} games played, {args.games} "
                      "booked")
    if os.path.abspath(named) != os.path.abspath(args.file):
        raise Refused(f"{args.log} names {named}, not {args.file}")
    counted, order = scan_rows(args.file, games_below=args.games)
    if counted != rows:
        raise Refused(f"{args.file} holds {counted} rows, its log {rows}")

    # The abort rule, from this run's own log: truncated, aborted and
    # uncertified refusals against a share of what passed the filter, terminal
    # ones reported and never gated; and the games an hour against a floor.
    verdicts = []
    if args.abort_share is not None:
        refused = [m for m in map(REFUSED.match, lines) if m]
        if len(refused) != 1:
            raise Refused(f"{args.log}: {len(refused)} qs refusal lines, not "
                          "one; the abort rule cannot be read")
        passed = int(refused[0].group(1))
        terminal = int(refused[0].group(2))
        broken = sum(int(refused[0].group(i)) for i in (3, 4, 5))
        verdicts.append(f"abort rule: {broken} truncated, aborted or "
                        f"uncertified of {passed} passed, terminal {terminal} "
                        "(not gated)")
        if broken * 100 > passed * args.abort_share:
            raise AbortRule(f"{args.log}: {broken} of {passed} passed refused "
                            f"as truncated, aborted or uncertified, past "
                            f"{args.abort_share} %")
    if args.rate_floor is not None:
        elapsed = [int(m.group(1)) for m in map(ELAPSED.match, lines) if m]
        if len(elapsed) != 1:
            raise Refused(f"{args.log}: {len(elapsed)} elapsed lines, not one; "
                          "the rate cannot be read")
        rate = played * 3600 // max(elapsed[0], 1)
        verdicts.append(f"rate: {rate} games an hour, floor {args.rate_floor}")
        if rate < args.rate_floor:
            raise AbortRule(f"{args.log}: {rate} games an hour, under the "
                            f"{args.rate_floor} the abort rule allows")
    for line in verdicts:
        print(line)
    print(f"chunk {args.file}: {rows} rows, {len(order)} games with a row of "
          f"{played} played, sha256 {sha256(args.file)}")
    return 0


def merge(args):
    out = args.out
    if os.path.exists(out):
        raise Refused(f"{out} exists; a training pool is never overwritten")
    if bool(args.base) != bool(args.base_sha256):
        raise Refused("--base and --base-sha256 go together")
    parts = []
    for spec in args.chunks:
        path, _, index = spec.rpartition(":")
        if not path or not index.isdigit():
            raise Refused(f"'{spec}' is not FILE:K")
        parts.append((path, int(index)))
    indices = [k for _, k in parts]
    if len(set(indices)) != len(indices):
        raise Refused("a chunk index named twice")

    expected_rows = 0
    expected_games = 0
    report = []
    if args.base:
        digest = sha256(args.base)
        if digest != args.base_sha256:
            raise Refused(f"{args.base} has sha256 {digest}, not the "
                          f"{args.base_sha256} its record names")
        rows, order = scan_rows(args.base)
        if order and max(order) >= ID_BASE:
            raise Refused(f"{args.base} holds a game id at or above {ID_BASE}")
        expected_rows += rows
        expected_games += len(order)
        report.append(f"base {args.base}: {rows} rows, {len(order)} games, "
                      f"sha256 {digest}, copied byte for byte")

    temporary = out + ".merging"
    if os.path.exists(temporary):
        os.remove(temporary)  # this stage's own file from a run that died
    try:
        with open(temporary, "wb") as target:
            if args.base:
                with open(args.base, "rb") as source:
                    for block in iter(lambda: source.read(1 << 20), b""):
                        target.write(block)
            for path, k in parts:
                rows, order = scan_rows(path, games_below=ID_STRIDE)
                offset = ID_BASE + k * ID_STRIDE
                with open(path, "rb") as source:
                    for line in source:
                        fields = line.rstrip(b"\n").split(b"\t")
                        fields[4] = str(int(fields[4]) + offset).encode()
                        target.write(b"\t".join(fields) + b"\n")
                expected_rows += rows
                expected_games += len(order)
                report.append(f"chunk {path}: {rows} rows, {len(order)} games, "
                              f"ids + {offset}, sha256 {sha256(path)}")
            target.flush()
            os.fsync(target.fileno())

        rows, order = scan_rows(temporary)
        if rows != expected_rows or len(order) != expected_games:
            raise Refused(f"read back {rows} rows in {len(order)} games, the "
                          f"parts add up to {expected_rows} in "
                          f"{expected_games}")
        digest = sha256(temporary)
        os.rename(temporary, out)
    except BaseException:
        if os.path.exists(temporary):
            os.remove(temporary)
        raise
    for line in report:
        print(line)
    print(f"read back: no NUL, seven fields a row, each game's rows "
          f"contiguous, every id once")
    print(f"merged {out}: {rows} rows, {len(order)} games, sha256 {digest}")
    return 0


def capacity(args):
    held = corpus_split.validation_positions(args.validation)
    kept = {}
    order = []
    dropped = 0
    with open(args.input, "rb") as handle:
        for number, line in enumerate(handle, 1):
            if not line.strip():
                continue
            fields, game = corpus_split.row_fields(line, args.input, number)
            if not order or order[-1] != game:
                if game in kept:
                    raise Refused(f"{args.input}:{number}: the rows of game "
                                  f"{game} are not contiguous")
                kept[game] = 0
                order.append(game)
            if corpus_split.position(fields[0], args.input, number) in held:
                dropped += 1
                continue
            kept[game] += 1
    with_rows = sum(1 for game in order if kept[game] > 0)
    for per_game in args.per_game:
        if per_game < 0:
            raise Refused("--per-game must be 0 or positive")
        rows = sum(min(per_game, n) if per_game > 0 else n
                   for n in kept.values())
        print(f"capacity: per-game {per_game}, {rows} rows, {with_rows} games "
              f"with a row kept, of {len(order)} games; {dropped} rows dropped "
              f"for a validation position")
    return 0


def filters(args):
    totals = {"games": 0, "rows": 0, "differs": 0, "distance": 0.0,
              "considered": 0, "passed": 0, "in_check": 0, "mate": 0,
              "score": 0, "incomplete": 0, "terminal": 0, "truncated": 0,
              "aborted": 0, "uncertified": 0, "walk": 0, "searched": 0,
              "elapsed": 0}
    cover = [[0] * 5 for _ in range(5)]
    for log in args.logs:
        lines = open(log, encoding="utf-8", errors="replace").read().splitlines()
        found = {"leaf": False, "refused": False, "filter": False}
        band = 0
        for line in lines:
            m = LEAF_QS.match(line)
            if m:
                rows = int(m.group(1))
                totals["rows"] += rows
                totals["games"] += int(m.group(2))
                totals["differs"] += int(m.group(4))
                totals["distance"] += float(m.group(5)) * rows
                found["leaf"] = True
            m = REFUSED.match(line)
            if m:
                for key, group in (("terminal", 2), ("truncated", 3),
                                   ("aborted", 4), ("uncertified", 5)):
                    totals[key] += int(m.group(group))
                found["refused"] = True
            m = FILTER.match(line)
            if m:
                for key, group in (("considered", 1), ("passed", 2),
                                   ("in_check", 3), ("mate", 4), ("score", 6),
                                   ("incomplete", 7)):
                    totals[key] += int(m.group(group))
                found["filter"] = True
            m = WALK.match(line)
            if m:
                totals["walk"] += int(m.group(1))
                totals["searched"] += int(m.group(2))
            m = COVER.match(line)
            if m and band < 5:
                for column, value in enumerate(m.group(3).split()):
                    cover[band][column] += int(value)
                band += 1
            m = ELAPSED.match(line)
            if m:
                totals["elapsed"] += int(m.group(1))
        missing = [key for key, seen in found.items() if not seen]
        if missing:
            raise Refused(f"{log} lacks datagen's {', '.join(missing)} line")

    def share(part, whole):
        return 100.0 * part / whole if whole else 0.0

    t = totals
    print(f"chunks {len(args.logs)}: {t['games']} games, {t['rows']} qs rows, "
          f"{t['rows'] / max(t['games'], 1):.2f} a game")
    print(f"filter: {t['considered']} considered, {t['passed']} passed; "
          f"skipped in-check {t['in_check']}, mate {t['mate']}, score past "
          f"the quiet limit {t['score']}; no completed iteration "
          f"{t['incomplete']}")
    print(f"leaf qs: differs from the root on {t['differs']} "
          f"({share(t['differs'], t['rows']):.2f} %), "
          f"{t['distance'] / max(t['rows'], 1):.3f} plies on average")
    print(f"leaf qs: not written of {t['passed']} passed: terminal "
          f"{t['terminal']} ({share(t['terminal'], t['passed']):.4f} %) "
          f"truncated {t['truncated']} aborted {t['aborted']} uncertified "
          f"{t['uncertified']}")
    print(f"walk: {t['walk']} nodes, {share(t['walk'], t['searched']):.3f} % "
          f"of the {t['searched']} the games' searches spent")
    hours = t["elapsed"] / 3600.0
    print(f"wall: {t['elapsed']} s over the chunks, {hours:.2f} h, "
          f"{t['games'] / hours if hours else 0:.0f} games an hour")
    print("coverage qs, rows by the written position's phase and the sampled "
          "position's |score| <50 <150 <300 <600 rest")
    for band, row in enumerate(cover):
        print(f"  phase {band * 5:2d}+ " + " ".join(f"{v:10d}" for v in row))
    return 0


def read_fit(path):
    """(L, start L, K, best epoch, improved, rows, games) from a tuner log."""
    if not os.path.exists(path):
        raise Refused(f"missing {path}")
    best = k = None
    epoch = 0
    improved = True
    rows = games = None
    with open(path, encoding="utf-8", errors="replace") as handle:
        for line in handle:
            m = BEST.match(line)
            if m:
                best = (float(m.group(2)), float(m.group(4)))
            m = K_LINE.match(line)
            if m:
                k = float(m.group(1))
            m = EPOCH.match(line)
            if m:
                epoch = int(m.group(1))
            if NOT_IMPROVED in line:
                improved = False
            m = CORPUS.match(line)
            if m and rows is None:
                rows, games = int(m.group(1)), int(m.group(2))
    if best is None:
        raise Refused(f"no best: line in {path}")
    return best[0], best[1], k, epoch, improved, rows, games


def read(args):
    """The rule, mechanically. Under MODE=none stage 0 -- the old engine's
    pool -- is informational: its figures are reported, readable or not, and
    the outcome is `continue` to stage 1 whatever they say (DEC-294, DEC-304
    (pending, the coordinator's)). Under MODE=s082 stage 0 decides as every
    other stage does."""
    informational = args.mode == "none" and args.stage == 0
    directory = args.dir
    problems = []
    print(f"S083 reading, stage {args.stage}, mode {args.mode}, "
          f"{directory}")
    if informational:
        print("stage 0 under MODE=none is informational: the old engine's "
              "pool, reported and never decisive; stage 1 follows whatever "
              "it reads")

    def informational_end():
        if problems:
            print("would be unreadable, reported only: " + "; ".join(problems))
        print("outcome: stage 1 is generated from the new engine's games "
              "(MODE=none)")
        print("READ-OUTCOME continue")
        return 0

    plan_path = os.path.join(directory, "ladder.txt")
    ladder = []
    seeds_at = None
    densities = []
    if os.path.exists(plan_path):
        for line in open(plan_path):
            word = line.split()
            if not word:
                continue
            if word[0] == "rung":
                ladder.append(int(word[1]))
            elif word[0] == "seeds":
                seeds_at = (int(word[1]), [int(s) for s in word[2:]])
            elif word[0] == "density":
                densities.append((int(word[1]), int(word[2])))
    if len(ladder) < 2 or seeds_at is None:
        why = (f"{plan_path} is missing, or names {len(ladder)} rungs and "
               f"{'no' if seeds_at is None else 'its'} seed line; the rule "
               "needs two rungs and five seeds")
        if informational:
            problems.append(why)
            return informational_end()
        raise Refused(why)

    print(f"{'fit':<22}{'rows':>10}{'games':>9}  {'L (qs view)':>11}  "
          f"{'start':>9}  {'K':>6}  {'best at':>7}  improved")

    def show(label, path):
        try:
            fit = read_fit(path)
        except Refused as why:
            if not informational:
                raise
            problems.append(str(why))
            print(f"{label:<22}  missing or unreadable")
            return None
        L, start, k, epoch, improved, rows, games = fit
        print(f"{label:<22}{rows or 0:>10}{games or 0:>9}  {L:>11.6f}  "
              f"{start:>9.6f}  {k if k is not None else float('nan'):>6.4f}  "
              f"{epoch:>7}  {'yes' if improved else 'NO'}")
        return fit

    fits = {}
    for rows in ladder:
        fits[(rows, 1)] = show(f"rung {rows} s1",
                               os.path.join(directory, f"fit_{rows}_s1.log"))
    smallest, seeds = seeds_at
    for seed in seeds:
        if seed != 1:
            fits[(smallest, seed)] = show(
                f"rung {smallest} s{seed}",
                os.path.join(directory, f"fit_{smallest}_s{seed}.log"))
    density_fits = {}
    for per_game, _rows in densities:
        density_fits[per_game] = show(
            f"every game, {per_game or 'all'}/game",
            os.path.join(directory, f"fit_dens{per_game}.log"))

    spread = [fits[(smallest, seed)][0] for seed in seeds
              if fits.get((smallest, seed)) is not None]
    if len(spread) < 2:
        problems.append(f"{len(spread)} of the seeds at {smallest} rows read; "
                        "no spread")
        return informational_end()  # only reachable when informational
    s = statistics.stdev(spread)
    m = PICK_Z * math.sqrt(2.0) * s
    print(f"spread at {smallest} rows over {len(spread)} seeds: mean "
          f"{statistics.mean(spread):.6f}, s = {s:.6f}; "
          f"m = {PICK_Z} * sqrt(2) * s = {m:.6f}")

    L = {rows: fits[(rows, 1)][0] for rows in ladder
         if fits[(rows, 1)] is not None}
    top, half = ladder[0], ladder[1]
    print("per doubling, seed 1, nested samples:")
    for bigger, smaller in zip(ladder, ladder[1:]):
        if bigger in L and smaller in L:
            gain = L[smaller] - L[bigger]
            print(f"  {smaller:>10} -> {bigger:>10}: L {L[smaller]:.6f} -> "
                  f"{L[bigger]:.6f}, gain {gain:+.6f}, "
                  f"{'past m' if gain > m else 'inside m'}")

    for per_game, fit in sorted(density_fits.items()):
        if per_game == 46 or fit is None or top not in L:
            continue
        gap = fit[0] - L[top]
        note = ("FINDING: beats 46 by more than m at the same games"
                if gap < -m else "within m of 46 or worse; decides nothing")
        print(f"second reading, every game: {per_game or 'all'} a game, L "
              f"{fit[0]:.6f}, {gap:+.6f} against 46's {L[top]:.6f} -- {note}")

    if s > SPREAD_UNREADABLE:
        problems.append(f"s {s:.6f} above {SPREAD_UNREADABLE}")
    if top in L:
        for rows in ladder[1:]:
            if rows in L and L[rows] < L[top] - m:
                problems.append(f"rung {rows} fits better than the top by "
                                "more than m: more rows hurt")
        if not fits[(top, 1)][4]:
            print("note: the top's fit did not improve on the shipped "
                  "constants; if `apply` finds nothing moved, no SPRT is owed "
                  "(S076's rule 3)")
    d_top = L[half] - L[top] if (top in L and half in L) else None
    if d_top is not None:
        print(f"top doubling: d = L({half}) - L({top}) = {d_top:+.6f} "
              f"against m {m:.6f}")

    if informational:
        return informational_end()
    if problems:
        print("outcome (c), unreadable: " + "; ".join(problems))
        print("READ-OUTCOME unreadable")
        return 1
    candidate = os.path.join(directory, f"fit_{top}_s1.hpp")
    if d_top > m and args.stage < args.last_stage:
        print(f"outcome: still descending at the top -- the last doubling "
              f"gained {d_top:.6f} > m {m:.6f}; stage {args.stage + 1} is "
              "generated")
        print("READ-OUTCOME continue")
        return 0
    reason = (f"the last doubling gained {d_top:+.6f}, inside m {m:.6f}"
              if d_top <= m else
              f"still descending ({d_top:+.6f} > m {m:.6f}) and the budget's "
              "last stage is read: more rows still helped where the budget "
              "ended")
    print(f"outcome: stop -- {reason}. The candidate is the top: every game "
          f"of this stage's pool at 46 rows a game, {top} rows, "
          f"{fits[(top, 1)][6]} games")
    print(f"candidate {candidate}")
    print("READ-OUTCOME stop")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(description="S083's corpus helpers.")
    commands = parser.add_subparsers(dest="command", required=True)

    one = commands.add_parser("chunk-check")
    one.add_argument("file")
    one.add_argument("log")
    one.add_argument("games", type=int)
    one.add_argument("--abort-share", type=int)
    one.add_argument("--rate-floor", type=int)

    join = commands.add_parser("merge")
    join.add_argument("--out", required=True)
    join.add_argument("--base")
    join.add_argument("--base-sha256")
    join.add_argument("chunks", nargs="*")

    fill = commands.add_parser("capacity")
    fill.add_argument("--in", dest="input", required=True)
    fill.add_argument("--validation", required=True, action="append")
    fill.add_argument("--per-game", type=int, required=True, action="append")

    tally = commands.add_parser("filters")
    tally.add_argument("logs", nargs="+")

    rule = commands.add_parser("read")
    rule.add_argument("dir")
    rule.add_argument("--stage", type=int, required=True)
    rule.add_argument("--last-stage", type=int, required=True)
    rule.add_argument("--mode", choices=("s082", "none"), required=True)

    args = parser.parse_args(argv)
    handlers = {"chunk-check": chunk_check, "merge": merge,
                "capacity": capacity, "filters": filters, "read": read}
    try:
        return handlers[args.command](args)
    except AbortRule as why:
        print(f"S083_corpus: ABORT RULE: {why}", file=sys.stderr)
        return 2
    except (Refused, corpus_split.Refused) as why:
        print(f"S083_corpus: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
