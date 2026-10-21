#!/usr/bin/env python3
"""S082's pool, cut by a power loss and completed by a top-up (DEC-296).

    python3 adocs/data/S082_pool_merge.py part1 PART1_PREFIX PART1_LOG
    python3 adocs/data/S082_pool_merge.py check PREFIX LOG
    python3 adocs/data/S082_pool_merge.py merge --games 150000 \\
        --seed 20261013 --topup-seed 2026102000 --nodes 100000 \\
        PART1_PREFIX PART1_LOG PART2_PREFIX PART2_LOG OUT_PREFIX

A prefix names the three files `datagen --leaf root,qs,pv` writes, PREFIX.root,
PREFIX.qs and PREFIX.pv, each row `fen result score phase game ply distance`.
`adocs/data/S082_recipe.sh merge` runs the third form; the first two are
read-only.

PART1 is the pool `generate` was writing when the power failed: three files
ending on a newline and a tail of NUL bytes, and a log without datagen's
closing lines. Its cut is computed from the files. Each file's games are
indexed in file order; qs's and pv's must all be root's games, in root's
order; the last game each file reaches may be cut short, so the earliest of
those last games in root's order is dropped with everything after it in every
file, and the games before it are kept -- a prefix of each file, copied byte
for byte. Refused, rather than cut, when anything else is damaged: a NUL byte
before the tail, anything but NUL bytes after the last newline, a row without
seven fields or with a field that is not a number where datagen writes one, a
game whose rows are not contiguous, a qs or pv game root does not hold or
holds in another order, a kept game with more rows in a leaf than in root.
`part1` prints the cut and what the kept rows still give, and writes nothing.

CHECK reads a whole pool against its datagen log: the same structural checks,
no tail allowed, and every figure datagen printed that the rows determine --
rows, how many differ from the root and the mean distance, the positions that
passed (root's rows), the refusals' total per leaf, the coverage tables -- must
be what the rows give. That is the check of the recomputation itself, and the
merge runs it on part 2.

MERGE checks both logs (one datagen binary by sha256, the same settings but
games and seed, the seeds given, per-worker seed ranges apart, part 2 exited 0
and part 1 never closed), cuts part 1, checks part 2 as CHECK does, refuses
unless part 1's kept games and part 2's booked games make --games, and writes
OUT_PREFIX.<leaf>.merging: part 1's kept bytes unchanged, then part 2's rows
with one change, the game field plus a fixed offset -- the smallest power of
ten above every game id in part 1's files, so every id is unique. The written
files are read back, not trusted: the same structural checks, every id once,
and rows, games and every recomputed figure equal to part 1's kept plus part
2's. Any refusal removes the .merging files this run wrote. The recipe script
renames them once its log is written. Exit 0, or 1 with the reason printed.

Datagen's end-of-run figures that the rows do not determine -- the filter's
considered positions and its skips by clause, refusals by kind, the pv line's
share of the distance, the walks' and the games' nodes, the elapsed time --
exist for part 2 only. They decide nothing (DEC-275).

Standard library only.
"""

import argparse
import hashlib
import os
import re
import sys

LEAVES = ("root", "qs", "pv")

# tools/datagen.cpp's coverage table: five phase bands of five, five |score|
# bands below 50, 150, 300, 600 and the rest; phase is the written position's,
# score the sampled position's, both in the row.
PHASE_BANDS = 5
SCORE_EDGES = (50, 150, 300, 600)
BANDS = PHASE_BANDS * (len(SCORE_EDGES) + 1)

DATAGEN = re.compile(r"^datagen: (\d+) games, (\d+) nodes per move, "
                     r"(\d+) threads, seed (\d+), quiet-limit (-?\d+), "
                     r"leaf (\S+)$")
BINARY = re.compile(r"^datagen\s+sha256 ([0-9a-f]{64})\s")
EXIT = re.compile(r"^exit\s+(\d+)$")
FAILED = re.compile(r"^datagen: FAILED")
FILTER = re.compile(r"^filter: \d+ considered, (\d+) passed;")
WRITTEN = re.compile(r"^leaf (\w+): (\d+) rows from (\d+) games written to "
                     r"(\S+), [\d.]+ a game; the leaf differs from the root "
                     r"on (\d+) \([\d.]+ %\), ([\d.]+) plies on average")
REFUSED = re.compile(r"^leaf (\w+): not written of (\d+) passed: "
                     r"terminal (\d+) \([^)]*\) truncated (\d+) \([^)]*\) "
                     r"aborted (\d+) \([^)]*\) uncertified (\d+)")
COVERAGE = re.compile(r"^coverage (\w+), rows by")
COVERAGE_ROW = re.compile(r"^\s+\d+ to\s+\d+((?:\s+\d+){5})$")
CLOSING = re.compile(r"^(filter: |leaf \w+: |walk: |elapsed )")


class Refused(Exception):
    """A pool this script will not cut, check or merge as it stands."""


class Totals:
    """What a set of rows gives: rows, rows not at the root, plies past it,
    and datagen's coverage table."""

    def __init__(self):
        self.rows = 0
        self.differs = 0
        self.distance = 0
        self.coverage = [0] * BANDS

    def minus(self, other):
        out = Totals()
        out.rows = self.rows - other.rows
        out.differs = self.differs - other.differs
        out.distance = self.distance - other.distance
        out.coverage = [a - b for a, b in zip(self.coverage, other.coverage)]
        return out

    def plus(self, other):
        out = Totals()
        out.rows = self.rows + other.rows
        out.differs = self.differs + other.differs
        out.distance = self.distance + other.distance
        out.coverage = [a + b for a, b in zip(self.coverage, other.coverage)]
        return out

    def same(self, other):
        return (self.rows, self.differs, self.distance, self.coverage) == \
               (other.rows, other.differs, other.distance, other.coverage)

    def mean_distance(self):
        return self.distance / self.rows if self.rows else 0.0


class Scan:
    """One file read once: its games in file order, [id, first byte, end
    byte, rows], what its rows give, where its rows end and its tail."""

    def __init__(self, path):
        self.path = path
        self.games = []
        self.totals = Totals()
        self.end = 0
        self.tail = 0
        self.max_id = -1


def scan(path, tail_allowed, start=0, stop=None):
    """Reads PATH's rows from byte START to STOP (the file's end by default)."""
    out = Scan(path)
    totals = out.totals
    coverage = totals.coverage
    games = out.games
    seen = set()
    offset = start
    rows = differs = distance = 0
    width = len(SCORE_EDGES) + 1
    top = PHASE_BANDS - 1
    # Unrolled below: this loop runs once a row, thirty million times.
    edge0, edge1, edge2, edge3 = SCORE_EDGES

    def where(number, first):
        return f"{path}:{number}" if start == 0 else f"{path}, byte {first}"

    with open(path, "rb") as handle:
        handle.seek(start)
        for number, line in enumerate(handle, 1):
            if stop is not None and offset >= stop:
                break
            first = offset
            offset += len(line)

            if line[-1:] != b"\n":
                # The bytes after the file's last newline: a power loss's NUL
                # tail is allowed where asked for, and nothing else is.
                if tail_allowed and line.count(0) == len(line):
                    out.tail = len(line)
                    offset = first
                    break
                raise Refused(f"{where(number, first)}: the file does not "
                              "end on a newline and what follows its last "
                              "one is not a tail of NUL bytes")
            if 0 in line:
                raise Refused(f"{where(number, first)}: a NUL byte before "
                              "the file's tail")

            fields = line.split(b"\t")
            if len(fields) != 7:
                raise Refused(f"{where(number, first)}: {len(fields)} fields, "
                              "datagen's rows have seven (fen result score "
                              "phase game ply distance)")
            try:
                score = int(fields[2])
                phase = int(fields[3])
                game = int(fields[4])
                plies = int(fields[6])
            except ValueError:
                raise Refused(f"{where(number, first)}: a score, phase, game "
                              "or distance field that is not a number") \
                    from None
            if game < 0 or plies < 0:
                raise Refused(f"{where(number, first)}: a negative game or "
                              "distance field")

            if games and games[-1][0] == game:
                games[-1][2] = offset
                games[-1][3] += 1
            else:
                if game in seen:
                    raise Refused(f"{where(number, first)}: the rows of game "
                                  f"{game} are not contiguous: it comes back "
                                  "after another game's rows")
                seen.add(game)
                games.append([game, first, offset, 1])

            rows += 1
            if plies:
                differs += 1
                distance += plies

            magnitude = score if score >= 0 else -score
            if magnitude < edge0:
                band = 0
            elif magnitude < edge1:
                band = 1
            elif magnitude < edge2:
                band = 2
            elif magnitude < edge3:
                band = 3
            else:
                band = 4
            phase_band = phase // 5 if phase > 0 else 0
            if phase_band > top:
                phase_band = top
            coverage[phase_band * width + band] += 1

    if stop is not None and offset != stop:
        raise Refused(f"{path}: rows do not end at byte {stop}")
    totals.rows = rows
    totals.differs = differs
    totals.distance = distance
    out.max_id = max((game for game, *_ in games), default=-1)
    out.end = offset
    return out


def check_order(scans, what):
    """qs's and pv's games are all root's, in root's order. Returns root's
    {game: position}."""
    root_pos = {}
    for position, (game, *_) in enumerate(scans["root"].games):
        root_pos[game] = position
    for leaf in LEAVES[1:]:
        last = -1
        for game, *_ in scans[leaf].games:
            position = root_pos.get(game)
            if position is None:
                raise Refused(f"{what} {leaf}: game {game} is not in root's "
                              "file")
            if position <= last:
                raise Refused(f"{what} {leaf}: game {game} is listed in "
                              "another order than root lists it")
            last = position
    return root_pos


def check_rows_per_game(games_by_leaf, what):
    """A leaf row is a root row the walk resolved, so no game has more."""
    root_rows = {game: rows for game, _, _, rows in games_by_leaf["root"]}
    for leaf in LEAVES[1:]:
        for game, _, _, rows in games_by_leaf[leaf]:
            if rows > root_rows[game]:
                raise Refused(f"{what} {leaf}: game {game} has {rows} rows, "
                              f"root {root_rows[game]}")


def nonempty(scans, what):
    for leaf in LEAVES:
        if not scans[leaf].games:
            raise Refused(f"{what} {leaf}: no rows")


class Cut:
    """Part 1 after its cut: per leaf, the kept games, the byte they end at,
    what they give, and what was dropped."""

    def __init__(self):
        self.games = {}
        self.end = {}
        self.totals = {}
        self.dropped_games = {}
        self.dropped_rows = {}
        self.scans = {}
        self.dropped_game = None
        self.cut_rows = {}
        self.max_id = -1


def cut_part1(prefix):
    scans = {leaf: scan(f"{prefix}.{leaf}", tail_allowed=True)
             for leaf in LEAVES}
    nonempty(scans, "part 1")
    root_pos = check_order(scans, "part 1")

    # The last game each file reaches may be cut short; the earliest of them
    # in root's order is the first game not kept.
    cut = min(root_pos[scans[leaf].games[-1][0]] for leaf in LEAVES)
    if cut == 0:
        raise Refused("part 1: no game before the first file's last one")

    out = Cut()
    out.scans = scans
    out.dropped_game = scans["root"].games[cut][0]
    out.max_id = max(scans[leaf].max_id for leaf in LEAVES)

    for leaf in LEAVES:
        games = scans[leaf].games
        kept = 0
        while root_pos[games[kept][0]] < cut:
            kept += 1
        end = games[kept][1]
        dropped = scan(scans[leaf].path, False, end, scans[leaf].end)

        out.games[leaf] = games[:kept]
        out.end[leaf] = end
        out.totals[leaf] = scans[leaf].totals.minus(dropped.totals)
        out.dropped_games[leaf] = len(games) - kept
        out.dropped_rows[leaf] = dropped.totals.rows
        out.cut_rows[leaf] = next(
            (rows for game, _, _, rows in games[kept:]
             if game == out.dropped_game), 0)

    check_rows_per_game(out.games, "part 1")
    return out


def read_whole(paths, what):
    scans = {leaf: scan(paths[leaf], tail_allowed=False) for leaf in LEAVES}
    nonempty(scans, what)
    check_order(scans, what)
    check_rows_per_game({leaf: scans[leaf].games for leaf in LEAVES}, what)
    return scans


def lines_of(path):
    with open(path, encoding="utf-8", errors="replace") as handle:
        return handle.read().splitlines()


def datagen_settings(lines, what):
    found = [DATAGEN.match(line) for line in lines]
    found = [match for match in found if match]
    if len(found) != 1:
        raise Refused(f"{what}'s log has {len(found)} 'datagen:' settings "
                      "lines, not one")
    games, nodes, threads, seed, quiet, leaf = found[0].groups()
    return {"games": int(games), "nodes": int(nodes), "threads": int(threads),
            "seed": int(seed), "quiet": int(quiet), "leaf": leaf}


def binary_sha(lines, what):
    found = {match.group(1) for match in map(BINARY.match, lines) if match}
    if len(found) != 1:
        raise Refused(f"{what}'s log names {len(found)} datagen sha256, "
                      "not one")
    return found.pop()


def datagen_figures(lines, what):
    """The figures datagen printed that the rows determine."""
    passed = None
    written = {}
    refused = {}
    coverage = {}
    for index, line in enumerate(lines):
        match = FILTER.match(line)
        if match:
            passed = int(match.group(1))
        match = WRITTEN.match(line)
        if match:
            leaf, rows, games, path, differs, mean = match.groups()
            written[leaf] = (int(rows), int(games), path, int(differs), mean)
        match = REFUSED.match(line)
        if match:
            refused[match.group(1)] = tuple(int(match.group(i))
                                            for i in range(2, 7))
        match = COVERAGE.match(line)
        if match:
            table = []
            for row in lines[index + 2:index + 2 + PHASE_BANDS]:
                cells = COVERAGE_ROW.match(row)
                if not cells:
                    raise Refused(f"{what}'s log: coverage {match.group(1)} "
                                  "is not five rows of five counts")
                table += [int(cell) for cell in cells.group(1).split()]
            coverage[match.group(1)] = table
    if passed is None:
        raise Refused(f"{what}'s log has no 'filter:' line")
    for leaf in LEAVES:
        if leaf not in written or leaf not in coverage:
            raise Refused(f"{what}'s log has no 'leaf {leaf}:' or coverage "
                          "line")
        if leaf != "root" and leaf not in refused:
            raise Refused(f"{what}'s log has no 'leaf {leaf}: not written' "
                          "line")
    return passed, written, refused, coverage


def check_against_log(scans, lines, what):
    """Every figure the rows determine equals the one datagen printed."""
    passed, written, refused, coverage = datagen_figures(lines, what)
    wrong = []
    root_rows = scans["root"].totals.rows
    if passed != root_rows:
        wrong.append(f"{passed} passed, root {root_rows} rows")
    for leaf in LEAVES:
        totals = scans[leaf].totals
        rows, _, _, differs, mean = written[leaf]
        if rows != totals.rows:
            wrong.append(f"{leaf} {rows} rows, the file {totals.rows}")
        if differs != totals.differs:
            wrong.append(f"{leaf} differs on {differs}, the rows "
                         f"{totals.differs}")
        if mean != f"{totals.mean_distance():.3f}":
            wrong.append(f"{leaf} {mean} plies on average, the rows "
                         f"{totals.mean_distance():.3f}")
        if coverage[leaf] != totals.coverage:
            wrong.append(f"{leaf} coverage differs from the rows'")
        if leaf != "root":
            if refused[leaf][0] != passed:
                wrong.append(f"{leaf} refusals of {refused[leaf][0]} passed")
            if sum(refused[leaf][1:]) != root_rows - totals.rows:
                wrong.append(f"{leaf} refused {sum(refused[leaf][1:])}, root "
                             f"rows less {leaf} rows "
                             f"{root_rows - totals.rows}")
    if wrong:
        raise Refused(f"{what}: the rows do not give what datagen printed: "
                      + "; ".join(wrong))
    return written, refused


def share(part, whole):
    return 100.0 * part / whole if whole else 0.0


def zero_row_games(games, threads):
    """Games that wrote no row, seen as absent ids inside one worker's run:
    worker w plays ids w, w + threads, ... and writes each before the next, so
    an id missing below a worker's last kept id wrote nothing. A lower bound:
    a game after a worker's last kept row cannot be seen."""
    by_worker = {}
    for game, *_ in games:
        by_worker.setdefault(game % threads, []).append(game)
    missing = 0
    for worker, ids in by_worker.items():
        missing += (max(ids) - worker) // threads + 1 - len(ids)
    return missing


def print_part1(part, threads):
    for leaf in LEAVES:
        scanned = part.scans[leaf]
        print(f"part 1    {scanned.path}: {scanned.totals.rows} rows, "
              f"{len(scanned.games)} games, a tail of {scanned.tail} NUL "
              f"bytes after its last newline, last game "
              f"{scanned.games[-1][0]} ({scanned.games[-1][3]} rows)")
    print("part 1    checked: no NUL before a tail, nothing but NUL after the "
          "last newline, seven fields a row, each game's rows contiguous, "
          "qs's and pv's games all root's and in root's order, no kept game "
          "with more rows in a leaf than in root")
    kept_games = len(part.games["root"])
    print(f"part 1    cut: the {kept_games} games before game "
          f"{part.dropped_game} in root's order are kept; game "
          f"{part.dropped_game} ("
          + ", ".join(f"{leaf} {part.cut_rows[leaf]} rows" for leaf in LEAVES)
          + ") is dropped with everything after it ("
          + ", ".join(f"{leaf} {part.dropped_games[leaf]} game(s), "
                      f"{part.dropped_rows[leaf]} rows" for leaf in LEAVES)
          + ") and the NUL tails")
    print("part 1    kept: "
          + ", ".join(f"{leaf} {part.totals[leaf].rows} rows in "
                      f"{len(part.games[leaf])} games, bytes 0 to "
                      f"{part.end[leaf]}" for leaf in LEAVES))
    print(f"part 1    at least {zero_row_games(part.games['root'], threads)} "
          f"of its games wrote no row (ids absent inside one of {threads} "
          "workers' runs); a game played is visible only by its rows, so "
          "its games are counted as games with a row")


def print_figures(part1, part2=None, written=None, refused=None):
    """Part 1's figures recomputed from its kept rows, beside part 2's as its
    datagen printed them (its rows give the same, checked)."""
    print("figures   per leaf; part 1 recomputed from its kept rows, a game "
          "over its games with a row; part 2 datagen's own, a game over its "
          "games played; refused = root rows less the leaf's, of the "
          "positions that passed (root's rows)")
    print("  leaf part      rows    games  a game     differs  "
          "plies  refused")
    p1_games = len(part1.games["root"])
    p1_passed = part1.totals["root"].rows
    for leaf in LEAVES:
        totals = part1.totals[leaf]
        lost = p1_passed - totals.rows
        kinds = "" if leaf == "root" else ", kind unknown"
        print(f"  {leaf:<4} 1    {totals.rows:9d} {p1_games:8d} "
              f"{totals.rows / p1_games:7.2f} "
              f"{share(totals.differs, totals.rows):9.2f} % "
              f"{totals.mean_distance():6.3f}  {lost} "
              f"({share(lost, p1_passed):.4f} %){kinds}")
        if part2 is None:
            continue
        rows, games, _, differs, mean = written[leaf]
        p2_passed = part2["root"].totals.rows
        if leaf == "root":
            kinds = "0"
        else:
            terminal, truncated, aborted, uncertified = refused[leaf][1:]
            lost = terminal + truncated + aborted + uncertified
            kinds = (f"{lost} ({share(lost, p2_passed):.4f} %): terminal "
                     f"{terminal}, truncated {truncated}, aborted {aborted}, "
                     f"uncertified {uncertified}")
        print(f"  {leaf:<4} 2    {rows:9d} {games:8d} {rows / games:7.2f} "
              f"{share(differs, rows):9.2f} % {mean:>6}  {kinds}")


def print_coverage(part):
    width = len(SCORE_EDGES) + 1
    for leaf in LEAVES:
        print(f"part 1    coverage {leaf}, recomputed from its kept rows: "
              "the written position's phase by the sampled position's "
              "|score| <50 <150 <300 <600 rest")
        table = part.totals[leaf].coverage
        for band in range(PHASE_BANDS):
            cells = " ".join(f"{count:9d}"
                             for count in table[band * width:
                                                (band + 1) * width])
            print(f"    phase {band * 5:2d}+  {cells}")


LOST = ("part 1    lost with datagen's closing lines, and not in the rows: "
        "the filter's considered positions and its skips by clause, "
        "refusals by kind, the pv line's share of the distance, the walks' "
        "and the games' nodes, the elapsed time, the games played; they "
        "decide nothing (DEC-275)")


def read_part1_log(path):
    lines = lines_of(path)
    closing = [line for line in lines if CLOSING.match(line)]
    if closing:
        raise Refused(f"part 1's log {path} has datagen's closing lines -- "
                      f"'{closing[0]}' -- so its run finished and there is "
                      "nothing to cut")
    return lines


def part1_command(args):
    lines = read_part1_log(args.log)
    settings = datagen_settings(lines, "part 1")
    part = cut_part1(args.prefix)
    print_part1(part, settings["threads"])
    print_figures(part)
    print_coverage(part)
    print(LOST)
    print(f"offset    a part 2 would take the offset "
          f"{offset_above(part.max_id)}: the smallest power of ten above "
          f"part 1's largest game id, {part.max_id}")
    return 0


def check_command(args):
    lines = lines_of(args.log)
    scans = read_whole({leaf: f"{args.prefix}.{leaf}" for leaf in LEAVES},
                       args.prefix)
    check_against_log(scans, lines, args.prefix)
    for leaf in LEAVES:
        totals = scans[leaf].totals
        print(f"check     {leaf}: {totals.rows} rows, "
              f"{len(scans[leaf].games)} games; differs {totals.differs}, "
              f"{totals.mean_distance():.3f} plies on average, coverage -- "
              "each equal to the log's")
    print(f"check     {args.prefix}: the rows give every figure {args.log} "
          "prints that rows determine")
    return 0


def offset_above(largest):
    return 10 ** len(str(largest))


def write_merged(args, part1, part2_scans, offset, created):
    """OUT.<leaf>.merging: part 1's kept bytes, then part 2's rows with the
    game field shifted. Returns {leaf: sha256}; CREATED names each file this
    run made, for the cleanup a refusal does."""
    digests = {}
    for leaf in LEAVES:
        target = f"{args.out}.{leaf}.merging"
        sha = hashlib.sha256()
        with open(target, "xb") as sink:
            created.append(target)
            with open(f"{args.part1}.{leaf}", "rb") as source:
                remaining = part1.end[leaf]
                while remaining:
                    block = source.read(min(1 << 20, remaining))
                    if not block:
                        raise Refused(f"{args.part1}.{leaf}: shorter than "
                                      "its own index")
                    sink.write(block)
                    sha.update(block)
                    remaining -= len(block)
            with open(part2_scans[leaf].path, "rb") as source:
                batch = []
                for line in source:
                    fields = line.split(b"\t")
                    fields[4] = b"%d" % (int(fields[4]) + offset)
                    batch.append(b"\t".join(fields))
                    if len(batch) == 65536:
                        block = b"".join(batch)
                        sink.write(block)
                        sha.update(block)
                        batch = []
                block = b"".join(batch)
                sink.write(block)
                sha.update(block)
        digests[leaf] = sha.hexdigest()
    return digests


def read_back(args, part1, part2_scans, offset):
    merged = read_whole({leaf: f"{args.out}.{leaf}.merging"
                         for leaf in LEAVES}, "merged")
    wrong = []
    for leaf in LEAVES:
        expected = part1.totals[leaf].plus(part2_scans[leaf].totals)
        if not merged[leaf].totals.same(expected):
            wrong.append(f"{leaf}'s rows do not give part 1's kept plus "
                         "part 2's")
        ids = [game for game, *_ in merged[leaf].games]
        expected_ids = [game for game, *_ in part1.games[leaf]] + \
                       [game + offset for game, *_ in part2_scans[leaf].games]
        if ids != expected_ids:
            wrong.append(f"{leaf}'s games are not part 1's kept then part "
                         "2's shifted")
        if len(set(ids)) != len(ids):
            wrong.append(f"{leaf} holds a game id twice")
    if wrong:
        raise Refused("merged files, read back: " + "; ".join(wrong))
    return merged


def merge_command(args):
    lines1 = read_part1_log(args.part1_log)
    lines2 = lines_of(args.part2_log)
    one = datagen_settings(lines1, "part 1")
    two = datagen_settings(lines2, "part 2")

    wrong = []
    if one["seed"] != args.seed:
        wrong.append(f"part 1's seed is {one['seed']}, not {args.seed}")
    if two["seed"] != args.topup_seed:
        wrong.append(f"part 2's seed is {two['seed']}, not "
                     f"{args.topup_seed}")
    for key in ("nodes", "threads", "quiet", "leaf"):
        if one[key] != two[key]:
            wrong.append(f"{key} {one[key]} in part 1, {two[key]} in part 2")
    if one["nodes"] != args.nodes:
        wrong.append(f"{one['nodes']} nodes a move, not {args.nodes}")
    if one["leaf"] != "root,qs,pv":
        wrong.append(f"leaf {one['leaf']}, not root,qs,pv")
    # Worker i seeds its generator with seed + i.
    if abs(one["seed"] - two["seed"]) < max(one["threads"], two["threads"]):
        wrong.append("the two runs' per-worker seeds overlap")
    sha1 = binary_sha(lines1, "part 1")
    sha2 = binary_sha(lines2, "part 2")
    if sha1 != sha2:
        wrong.append(f"datagen sha256 {sha1[:12]} in part 1, {sha2[:12]} in "
                     "part 2")
    exits = [int(match.group(1)) for match in map(EXIT.match, lines2)
             if match]
    if exits != [0]:
        wrong.append(f"part 2's log has exit lines {exits}, not one 'exit 0'")
    if any(FAILED.match(line) for line in lines2):
        wrong.append("part 2's log has a 'datagen: FAILED' line")
    if wrong:
        raise Refused("the logs: " + "; ".join(wrong))

    print(f"runs      one datagen, sha256 {sha1}; part 1 seed "
          f"{one['seed']} (workers {one['seed']} to "
          f"{one['seed'] + one['threads'] - 1}), part 2 seed {two['seed']} "
          f"(workers {two['seed']} to {two['seed'] + two['threads'] - 1}); "
          f"{one['nodes']} nodes a move, {one['threads']} threads, "
          f"quiet-limit {one['quiet']}, leaf {one['leaf']}, in both")

    part1 = cut_part1(args.part1)
    print_part1(part1, one["threads"])

    part2 = read_whole({leaf: f"{args.part2}.{leaf}" for leaf in LEAVES},
                       "part 2")
    written, refused = check_against_log(part2, lines2, "part 2")
    for leaf in LEAVES:
        if written[leaf][1] != two["games"]:
            raise Refused(f"part 2's leaf {leaf} line plays "
                          f"{written[leaf][1]} games of {two['games']} "
                          "booked")
    part2_with_rows = len(part2["root"].games)
    print(f"part 2    {args.part2}.*: {two['games']} games played, "
          f"{part2_with_rows} with a row, "
          f"{two['games'] - part2_with_rows} without; its rows give every "
          "figure its log prints that rows determine -- rows, differs, "
          "mean distance, passed, refusals' total, coverage")

    kept = len(part1.games["root"])
    if kept + two["games"] != args.games:
        raise Refused(f"part 1 keeps {kept} games and part 2 played "
                      f"{two['games']}: {kept + two['games']}, not "
                      f"{args.games}")

    print_figures(part1, part2, written, refused)
    print_coverage(part1)
    print(LOST)

    offset = offset_above(part1.max_id)
    print(f"offset    part 2's game ids plus {offset}, the smallest power of "
          f"ten above every id in part 1's files (largest {part1.max_id}): "
          f"part 2's {min(g[0] for g in part2['root'].games)} to "
          f"{part2['root'].max_id} become "
          f"{min(g[0] for g in part2['root'].games) + offset} to "
          f"{part2['root'].max_id + offset}; no other byte of a part 2 row "
          "changes, and part 1's kept bytes are copied unchanged")

    targets = [f"{args.out}.{leaf}" for leaf in LEAVES]
    for target in targets:
        if os.path.exists(target) or os.path.exists(target + ".merging"):
            raise Refused(f"{target} or its .merging exists; a pool is never "
                          "overwritten")

    created = []
    try:
        digests = write_merged(args, part1, part2, offset, created)
        merged = read_back(args, part1, part2, offset)
    except BaseException:
        for path in created:
            os.remove(path)
        raise

    print("read back the merged files: no NUL, seven fields a row, each "
          "game's rows contiguous, every id once, qs's and pv's games all "
          "root's and in root's order; rows, games and every recomputed "
          "figure equal part 1's kept plus part 2's")
    for leaf in LEAVES:
        print(f"merged {leaf}: {merged[leaf].totals.rows} rows from "
              f"{args.games} games written to {args.out}.{leaf}, sha256 "
              f"{digests[leaf]}; part 1's {kept} games with a row before "
              f"its cut and part 2's {two['games']} games played, "
              f"{len(merged[leaf].games)} games with a {leaf} row")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(
        description="S082's pool cut by a power loss, and its top-up "
                    "(DEC-296).")
    commands = parser.add_subparsers(dest="command", required=True)

    one = commands.add_parser("part1", help="part 1's cut, read only")
    one.add_argument("prefix")
    one.add_argument("log")

    whole = commands.add_parser("check", help="a whole pool against its log")
    whole.add_argument("prefix")
    whole.add_argument("log")

    join = commands.add_parser("merge", help="part 1's kept games and part 2")
    join.add_argument("--games", type=int, required=True)
    join.add_argument("--seed", type=int, required=True)
    join.add_argument("--topup-seed", type=int, required=True)
    join.add_argument("--nodes", type=int, required=True)
    join.add_argument("part1")
    join.add_argument("part1_log")
    join.add_argument("part2")
    join.add_argument("part2_log")
    join.add_argument("out")

    args = parser.parse_args(argv)
    run = {"part1": part1_command, "check": check_command,
           "merge": merge_command}[args.command]
    try:
        return run(args)
    except Refused as why:
        print(f"refused: {why}")
        return 1
    except OSError as why:
        print(f"refused: {why}")
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
