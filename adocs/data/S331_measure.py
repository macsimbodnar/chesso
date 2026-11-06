#!/usr/bin/env python3
"""S331: what datagen's two-fold rule, a draw at the first recurrence of a position, did to
the games and the labels of every corpus written from S207 (DEC-173) to S331. Stdlib only.

    python3 adocs/data/S331_measure.py read OUT [LOG]
    python3 adocs/data/S331_measure.py premise OLD_PREFIX NEW_PREFIX NEW_GAMES
    python3 adocs/data/S331_measure.py merge OFFSET PART1_DIR PART2_DIR PART2_LOG MERGED_ROOT OUT

READ reads one run of the step's datagen, `--leaf root,qs,pv --games-out games.tsv`, in OUT:
its log OUT/generate.log, or LOG, OUT/games.tsv and OUT/pool.tsv.<leaf> for every leaf the
log counts. It refuses, exit 1, unless the files hold what the log counts: one games line for
each game, the rows each `leaf <leaf>:` line counts, every row of every leaf carrying its
game's result, and the `repetition:` line's figures recomputed from the files. Each view is
streamed a row at a time and only the games and per-game counts are kept (S332): the first
form held every row of the three views and on S122's 150000-game pool ran out of memory. In
READ a field datagen writes as a number that is not one is refused on one line naming its
file and line (S335), as is a row whose game or result is not its games line's; MERGE and
PREMISE read a row's game as a number unchecked and can still end on a traceback there. A
pool merged after a stop (adocs/data/S122_corpus.sh merge, DEC-296) is read the same way
from the merge's log: the views' rows from its `merged <leaf>:` lines, the games from its
`merged games:` line, the settings from the two runs' logs it holds verbatim, indented, and
its one `repetition:` line, the two parts' combined. A merged pool whose `repetition
incomplete:` line says part 1's games file was cut short is refused: its check is partial.

The reading rests on one property of datagen (play_games(), tools/datagen.cpp): a game is a
function of its opening alone -- the table and the correction history are cleared before each
game and the rng is drawn only for the opening -- so under the old rule a game is the same game
up to its first recurrence and ends there, 0.5. The premise mode below checks it. So:

  - a game with no recurrence is the old rule's game, rows and label;
  - a game whose first recurrence comes after its opening is, under the old rule, the rows
    before that recurrence (ply below it), labelled 0.5; the rows from it on are new rows;
  - a game whose opening ended on a recurrence was, under the old rule, a discarded opening
    and another game in its place. It is outside both sides and counted apart.

The old corpus is then the first two kinds' old rows; a row whose label differs is one before
the first recurrence of a game that ended 1 or 0. Shares of games carry a binomial standard
error; shares of rows a standard error clustered by game (the ratio estimator's linearised
variance), because every row of a game carries one label. Phase is the row's own column, in
the coverage table's five bands and the census's cut at phase 6 (adocs/data/S124_census.txt's
signatures at phase <= 6). Direction is White's, the label's own perspective, and beside it
whether the new label goes the way of the row's own score.

PREMISE checks that property on two runs of one seed and one setting, OLD_PREFIX by the
datagen before S331 and NEW_PREFIX by the step's with --games-out NEW_GAMES: every leaf's rows
of each old game are the new game's rows before its first recurrence, field for field but the
label, and the old label is 0.5 where the game recurred and the new label where it did not.
It needs no opening ended on a recurrence, which would shift the old run's game numbers.

MERGE is S122_corpus.sh merge's, S332: the games file of a pool S082_pool_merge.py merged,
written as OUT.merging, and the merged log's `merged games:` line and combined `repetition:`
line, printed. Part 1, the run that was stopped, is PART1_DIR's games.tsv and pool.tsv.root;
part 2, the top-up, PART2_DIR's games.tsv and its log PART2_LOG; MERGED_ROOT is the merger's
root view and OFFSET what it added to part 2's game ids. Part 1's games lines are cut where
its rows were: kept up to the first line naming a game its root rows hold and the merged
root does not, so the game the merger dropped goes with every line after it, and a tail of
NUL bytes after the last newline, a power loss's, is taken and dropped. Refused unless every
game of the merged root has one games line, in the root's order, and each row the result of
its line; every kept part 1 id is under OFFSET; part 2's lines are the games its log books;
and part 2's lines and rows give its log's `repetition:` line. Part 1's figures are
recomputed from its kept lines and rows, its datagen having stopped before its closing
lines, and the combined line is theirs plus part 2's line. One exception, the coordinator's
ruling at S332's fast check: a power loss keeps in each file what the OS had flushed, so
part 1's games file may end before games the merger keeps. That is not refused when the
lines it has are the first kept games', in order: the games file holds what part 1's has,
and in place of the combined line a `repetition incomplete:` line counts the kept games
without one. The views are the merger's either way. A part 1 game with no row, before
the cut, is kept and counted apart, since the merger counts part 1's games by their rows.
OUT.merging holds part 1's kept lines byte for byte, then part 2's with the game id plus
OFFSET; it is read back, and removed on any refusal.
"""
import collections
import hashlib
import math
import os
import re
import sys

LEAVES = ("root", "qs", "pv")
PHASE_BANDS = ((0, 4), (5, 9), (10, 14), (15, 19), (20, 24))
CENSUS_CUT = 6

SETTINGS_TEXT = (r"datagen: (\d+) games, (\d+) nodes per move, (\d+) threads, "
                 r"seed (\d+), quiet-limit (-?\d+), leaf (\S+)$")
SETTINGS = re.compile("^" + SETTINGS_TEXT)
# A run's settings line in a merged pool's log, which holds each run's log indented four
# spaces (S122_corpus.sh merge).
PART_SETTINGS = re.compile("^    " + SETTINGS_TEXT)
WRITTEN = re.compile(r"^leaf (\w+): (\d+) rows from (\d+) games written to ")
MERGED = re.compile(r"^merged (\w+): (\d+) rows from (\d+) games written to ")
MERGED_GAMES = re.compile(
    r"^merged games: (\d+) games written to (\S+), sha256 [0-9a-f]{64}; part 1's (\d+) games "
    r"with a row before its cut and (\d+) without(?:, (\d+) of the first with no games line)?, "
    r"and part 2's (\d+) games played$")
# A merged pool whose part 1 games file a power loss cut short: no combined line (S332).
INCOMPLETE = re.compile(r"^repetition incomplete: \d+ of part 1's \d+ kept games have no games "
                        r"line")
REPETITION = re.compile(
    r"^repetition: (\d+) games reached a position that had occurred before, which drew a "
    r"game until S331; (\d+) of them ended other than drawn, (\d+) won by White and (\d+) "
    r"lost; (\d+) positions passed in them, a root row each, (\d+) before the first "
    r"recurrence; (\d+) openings ended on one, discarded until S331$")

# A game's counts in one list, S332: for each of KEYS its old rows, those whose label
# differs, those to win and those to loss; then its new rows and all its rows.
KEYS = ("all",) + tuple(f"band{i}" for i in range(len(PHASE_BANDS))) + ("low", "high")
LOW, HIGH = KEYS.index("low"), KEYS.index("high")
OLD, DIFFERS, TO_WIN, TO_LOSS = range(4)
FRESH = 4 * len(KEYS)
EVERY = FRESH + 1


class Refused(Exception):
    pass


# The fields datagen writes as numbers (tools/datagen.cpp), by position: a games line's,
# and those of a row that are read as numbers. A row's game and result are held to the
# games file instead (check_run), so a row's that is not a number is refused there.
GAMES_NUMBERS = ((0, "game", int), (1, "result", float), (2, "first ply", int),
                 (3, "first recurrence ply", int), (4, "last ply", int))
ROW_NUMBERS = ((2, "score", int), (3, "phase", int), (5, "ply", int), (6, "distance", int))


def not_a_number(path, number, numbers, fields):
    """The refusal of line NUMBER of PATH, which has a field of NUMBERS that is not a
    number: one line naming the file, the line and the field (S335; a ValueError's
    traceback before)."""
    for index, name, kind in numbers:
        try:
            kind(fields[index])
        except ValueError:
            return (f"{path}:{number}: field {index + 1}, the {name}, is '{fields[index]}', "
                    "not a number")
    return f"{path}:{number}: a field that is not a number"


def read_games(path):
    games = {}
    for number, line in enumerate(open(path, encoding="utf-8"), 1):
        fields = line.rstrip("\n").split("\t")
        if len(fields) != 5:
            raise Refused(f"{path}:{number}: {len(fields)} fields, not 5")
        game, result, first, recurred, end = fields
        if game in games:
            raise Refused(f"{path}:{number}: game {game} twice")
        try:
            int(game), float(result)
            games[game] = (result, int(first), int(recurred), int(end))
        except ValueError:
            raise Refused(not_a_number(path, number, GAMES_NUMBERS, fields)) from None
    return games


def stream_rows(path):
    """fen, result, score, phase, game, ply, distance; a row at a time, in file order."""
    with open(path, encoding="utf-8") as handle:
        for number, line in enumerate(handle, 1):
            fields = line.rstrip("\n").split("\t")
            if len(fields) != 7:
                raise Refused(f"{path}:{number}: {len(fields)} fields, not 7")
            fen, result, score, phase, game, ply, distance = fields
            try:
                row = fen, result, int(score), int(phase), game, int(ply), int(distance)
            except ValueError:
                raise Refused(not_a_number(path, number, ROW_NUMBERS, fields)) from None
            yield row


def read_rows(path):
    """Every row held, for the premise's two small runs."""
    return list(stream_rows(path))


def kind_of(game):
    _, first, recurred, _ = game
    if recurred < 0:
        return "plain"
    return "opening" if recurred == first else "recurred"


def figures(games, kinds, in_recurred, before):
    """The `repetition:` line's seven figures: GAMES and their KINDS, and the root rows of
    the recurred games, IN_RECURRED, of which BEFORE the first recurrence."""
    recurred = [g for g, k in kinds.items() if k == "recurred"]
    won = sum(1 for g in recurred if games[g][0] == "1.0")
    lost = sum(1 for g in recurred if games[g][0] == "0.0")
    openings = sum(1 for k in kinds.values() if k == "opening")
    return [len(recurred), won + lost, won, lost, in_recurred, before, openings]


def repetition_words(numbers):
    """datagen's `repetition:` line after its label (tools/datagen.cpp)."""
    recurred, other, won, lost, rows, before, openings = numbers
    return (f"{recurred} games reached a position that had occurred before, which drew a game "
            f"until S331; {other} of them ended other than drawn, {won} won by White and {lost} "
            f"lost; {rows} positions passed in them, a root row each, {before} before the first "
            f"recurrence; {openings} openings ended on one, discarded until S331")


def binomial(part, whole):
    if whole == 0:
        return "n/a"
    p = part / whole
    return f"{100 * p:.2f} % +/- {100 * math.sqrt(p * (1 - p) / whole):.2f}"


def clustered(parts, wholes):
    """sum(parts) / sum(wholes), each pair one game, with the clustered standard error."""
    n = len(wholes)
    total = sum(wholes)
    if total == 0 or n < 2:
        return "n/a"
    p = sum(parts) / total
    spread = sum((d - p * o) ** 2 for d, o in zip(parts, wholes))
    se = math.sqrt(n / (n - 1) * spread) / total
    return f"{100 * p:.3f} % +/- {100 * se:.3f}"


class Tally:
    """What the root rows give, counted per game as they stream past, never held."""

    def __init__(self, games):
        self.games = games
        self.kinds = {g: kind_of(v) for g, v in games.items()}
        self.per_game = {g: [0] * (EVERY + 1) for g in games}
        self.rows = 0
        self.in_recurred = 0
        self.before = 0
        self.toward = 0
        self.zero = 0

    def add(self, row, game):
        _, _, score, phase, g, ply, _ = row
        result, _, recurred, _ = game
        kind = self.kinds[g]
        counts = self.per_game[g]
        self.rows += 1
        counts[EVERY] += 1
        if kind == "recurred":
            self.in_recurred += 1
            self.before += ply < recurred
        if kind == "opening" or (kind == "recurred" and ply >= recurred):
            counts[FRESH] += 1
            return
        band = min(max(phase, 0) // 5, len(PHASE_BANDS) - 1)  # datagen's phase_band()
        differs = kind == "recurred" and result != "0.5"
        side = TO_WIN if result == "1.0" else TO_LOSS
        for key in (0, 1 + band, LOW if phase <= CENSUS_CUT else HIGH):
            counts[4 * key + OLD] += 1
            if differs:
                counts[4 * key + DIFFERS] += 1
                counts[4 * key + side] += 1
        if differs:
            self.toward += (score > 0 and result == "1.0") or (score < 0 and result == "0.0")
            self.zero += score == 0


def merged_counts(path, log):
    """A merged pool's log: the reading's description of the pool, {leaf: (rows, games)},
    the games its views play and the lines its games file holds."""
    parts = [m.groups() for m in map(PART_SETTINGS.match, log) if m]
    if len(parts) != 2:
        raise Refused(f"{path}: a merged pool's log with {len(parts)} runs' datagen settings "
                      "lines, not two")
    partial = [line for line in log if INCOMPLETE.match(line)]
    if partial:
        raise Refused(f"{path}: the merged pool's repetition: check is partial and is not "
                      f"reported as complete: {partial[0]}")
    one, two = parts
    if (one[1], one[2], one[4], one[5]) != (two[1], two[2], two[4], two[5]):
        raise Refused(f"{path}: its two runs' settings differ, {one} and {two}")
    written = {}
    for m in map(MERGED.match, log):
        if m:
            if m.group(1) in written:
                raise Refused(f"{path}: two 'merged {m.group(1)}:' lines")
            written[m.group(1)] = (int(m.group(2)), int(m.group(3)))
    played = {g for _, g in written.values()}
    if len(played) != 1:
        raise Refused(f"{path}: its merged views play {sorted(played)} games, not one count")
    booked = played.pop()
    lines = [m for m in map(MERGED_GAMES.match, log) if m]
    if len(lines) != 1:
        raise Refused(f"{path}: {len(lines)} 'merged games:' lines, not one; a pool merged "
                      "before S332 has no games file")
    expected, _, kept, without, unlined, topped = lines[0].groups()
    expected, kept, without, topped = int(expected), int(kept), int(without), int(topped)
    unlined = int(unlined or 0)
    if expected != kept - unlined + without + topped or kept + topped != booked:
        raise Refused(f"{path}: its games line counts {expected} games, {kept} - {unlined} + "
                      f"{without} + {topped}, against the {booked} its views play")
    _, nodes, threads, _, quiet, leaves = one
    run = (f"merged from two datagen runs (DEC-296), {booked} games, {nodes} nodes a move, "
           f"{threads} threads, seeds {one[3]} and {two[3]}, quiet-limit {quiet}, leaf {leaves}")
    return run, written, booked, expected


def check_run(out, log_path):
    log = open(log_path, encoding="utf-8", errors="replace").read().splitlines()
    merged = any(MERGED.match(line) for line in log)
    if merged:
        run, written, booked, expected = merged_counts(log_path, log)
    else:
        settings = [m for m in map(SETTINGS.match, log) if m]
        if len(settings) != 1:
            raise Refused(f"{log_path}: {len(settings)} datagen settings lines, not one")
        booked, nodes, threads, seed, quiet, leaves = settings[0].groups()
        run = (f"datagen {booked} games, {nodes} nodes a move, {threads} threads, seed {seed}, "
               f"quiet-limit {quiet}, leaf {leaves}")
        written = {m.group(1): (int(m.group(2)), int(m.group(3)))
                   for m in map(WRITTEN.match, log) if m}
        booked = expected = int(booked)
    if any(line.startswith("datagen: FAILED") for line in log):
        raise Refused(f"{log_path} carries datagen's FAILED line")
    repetition = [m for m in map(REPETITION.match, log) if m]
    if len(repetition) != 1:
        raise Refused(f"{log_path}: {len(repetition)} repetition: lines, not one")
    if "root" not in written:
        raise Refused(f"{log_path}: no 'leaf root:' line, so no root rows to read")
    games = read_games(f"{out}/games.tsv")
    if merged and len(games) != expected:
        raise Refused(f"{len(games)} games in games.tsv, the merge's games line {expected}")
    if len(games) != expected or written["root"][1] != booked:
        raise Refused(f"{len(games)} games in games.tsv, {written['root'][1]} in the log, "
                      f"{booked} booked")
    tally = Tally(games)
    for leaf, (count, _) in written.items():
        rows = wrong = 0
        first = ""
        for r in stream_rows(f"{out}/pool.tsv.{leaf}"):
            rows += 1
            game = games.get(r[4])
            if game is None or game[0] != r[1]:
                wrong += 1
                # The first named by its line, ROWS counting the file's lines (S335).
                if not first:
                    first = f"{out}/pool.tsv.{leaf}:{rows}: " + (
                        f"game '{r[4]}' has no games line" if game is None else
                        f"result '{r[1]}', its game {r[4]}'s games line {game[0]}")
            elif leaf == "root":
                tally.add(r, game)
        if rows != count:
            raise Refused(f"pool.tsv.{leaf} holds {rows} rows, the log {count}")
        if wrong:
            raise Refused(f"{wrong} rows of pool.tsv.{leaf} carry a result not their game's; "
                          f"the first, {first}")
    return run, [int(x) for x in repetition[0].groups()], games, tally


def read_command(out, log_path):
    run, line, games, tally = check_run(out, log_path)
    kinds = tally.kinds
    per_game = tally.per_game

    recurred = [g for g, k in kinds.items() if k == "recurred"]
    recomputed = figures(games, kinds, tally.in_recurred, tally.before)
    if recomputed != line:
        raise Refused(f"the repetition: line reads {line}, the files {recomputed}")
    _, _, won, lost, _, _, openings = recomputed

    old_games = [g for g, k in kinds.items() if k != "opening"]
    plain_draws = sum(1 for g in old_games if kinds[g] == "plain" and games[g][0] == "0.5")
    n = len(games)

    print(f"S331 reading of {out}: {run}")
    print(f"files     games.tsv {n} games; pool.tsv.root {tally.rows} rows; every row's result "
          f"its game's; the repetition: line equal to the files, {line}")
    print()
    print("GAMES. Recurred: a position on the board had occurred before, after the opening; "
          "the old rule drew the game there.")
    print(f"  games {n}; recurred {len(recurred)} ({binomial(len(recurred), n)} of all); "
          f"openings ended on a recurrence {openings}, outside both sides")
    print(f"  of the recurred, ended other than drawn {won + lost} "
          f"({binomial(won + lost, len(recurred))}): won by White {won} "
          f"({binomial(won, len(recurred))}), lost {lost} ({binomial(lost, len(recurred))})")
    print(f"  of all games, relabelled from a draw {binomial(won + lost, n)}")
    print(f"  draws, of the {len(old_games)} games both rules play: old rule "
          f"{plain_draws + len(recurred)} ({binomial(plain_draws + len(recurred), len(old_games))}), "
          f"new {plain_draws + len(recurred) - won - lost} "
          f"({binomial(plain_draws + len(recurred) - won - lost, len(old_games))})")
    print()
    print("ROOT ROWS. Old: the rows the old rule wrote, every row of a game with no recurrence and "
          "the rows before the first recurrence of a recurred game. Differs: an old row whose "
          "game ended 1 or 0, labelled 0.5 by the old rule. Errors clustered by game.")

    def share(key, slot):
        parts = [per_game[g][4 * key + slot] for g in old_games]
        wholes = [per_game[g][4 * key + OLD] for g in old_games]
        return sum(parts), sum(wholes), clustered(parts, wholes)

    labels = [(0, "all phases")]
    labels += [(1 + i, f"phase {lo} to {hi}") for i, (lo, hi) in enumerate(PHASE_BANDS)]
    labels += [(LOW, f"phase <= {CENSUS_CUT}"), (HIGH, f"phase > {CENSUS_CUT}")]
    for key, name in labels:
        d, o, s = share(key, DIFFERS)
        w, _, sw = share(key, TO_WIN)
        l, _, sl = share(key, TO_LOSS)
        print(f"  {name:15s} old {o:8d}  differs {d:7d} {s:>20s}   to win {w:7d} {sw:>20s}   "
              f"to loss {l:7d} {sl:>20s}")
    d = sum(per_game[g][DIFFERS] for g in old_games)
    toward = tally.toward
    zero = tally.zero
    print(f"  of the {d} that differ, the new label goes the way of the row's own score in "
          f"{toward}, against it in {d - toward - zero}, score 0 in {zero}")
    after = sum(per_game[g][FRESH] for g in recurred)
    from_openings = sum(counts[FRESH] for counts in per_game.values()) - after
    print(f"  new rows the old rule never wrote: {after} after a first recurrence and "
          f"{from_openings} from the openings, "
          f"{clustered([per_game[g][FRESH] for g in games], [per_game[g][EVERY] for g in games])} "
          f"of the {tally.rows} root rows")


def premise_command(old_prefix, new_prefix, new_games):
    games = read_games(new_games)
    openings = sum(1 for v in games.values() if kind_of(v) == "opening")
    if openings:
        raise Refused(f"{openings} openings ended on a recurrence: the old run's game numbers "
                      "are shifted and the two runs are not compared game by game")
    print(f"premise   {len(games)} games, {sum(1 for v in games.values() if kind_of(v) == 'recurred')} "
          f"of them recurred after the opening")
    for leaf in LEAVES:
        old = collections.defaultdict(list)
        new = collections.defaultdict(list)
        for r in read_rows(f"{old_prefix}.{leaf}"):
            old[r[4]].append(r)
        for r in read_rows(f"{new_prefix}.{leaf}"):
            result, _, recurred, _ = games[r[4]]
            if recurred < 0 or r[5] < recurred:
                new[r[4]].append(r)
        bad = []
        for g in sorted(set(old) | set(new), key=int):
            if g not in games:
                bad.append(f"game {g} is not in {new_games}")
                continue
            strip = lambda rs: [r[:1] + r[2:] for r in rs]
            if strip(old[g]) != strip(new[g]):
                bad.append(f"game {g}: {len(old[g])} old rows against {len(new[g])} new before "
                           "the recurrence, or a field apart")
                continue
            want = "0.5" if games[g][2] >= 0 else games[g][0]
            if any(r[1] != want for r in old[g]):
                bad.append(f"game {g}: old label not {want}")
        rows = sum(len(v) for v in old.values())
        if bad:
            raise Refused(f"leaf {leaf}: " + "; ".join(bad[:5]))
        print(f"premise   leaf {leaf}: {rows} old rows, each the new run's row before its game's "
              f"first recurrence but the label, which is 0.5 there and the game's elsewhere")
    print("premise   holds")


def games_lines(path, tail_allowed):
    """PATH's games lines in file order, (id, the line's bytes, its fields as read_games keeps
    them), and the NUL bytes after its last newline, which only TAIL_ALLOWED takes."""
    data = open(path, "rb").read()
    end = data.rfind(b"\n") + 1
    body, tail = data[:end], data[end:]
    if tail and not (tail_allowed and tail.count(0) == len(tail)):
        raise Refused(f"{path}: the bytes after its last newline are neither a line nor, where "
                      "a power loss is allowed for, a tail of NUL bytes")
    if 0 in body:
        raise Refused(f"{path}: a NUL byte before its tail")
    out = []
    for number, raw in enumerate(body.split(b"\n")[:-1], 1):
        fields = raw.decode("utf-8").split("\t")
        if len(fields) != 5:
            raise Refused(f"{path}:{number}: {len(fields)} fields, not 5")
        try:
            game, first, recurred, end_ply = int(fields[0]), *map(int, fields[2:])
        except ValueError:
            raise Refused(f"{path}:{number}: a game or ply field that is not a number") from None
        out.append((game, raw + b"\n", (fields[1], first, recurred, end_ply)))
    return out, len(tail)


def root_games(path):
    """The game ids of a view's rows, a run of rows each, in file order. A NUL tail is the
    merger's to have checked (S082_pool_merge.py part1)."""
    order = []
    with open(path, "rb") as handle:
        for line in handle:
            if not line.endswith(b"\n"):
                break
            game = int(line.split(b"\t", 5)[4])
            if not order or order[-1] != game:
                order.append(game)
    return order


def merge_command(offset_text, part1_dir, part2_dir, part2_log, merged_root, out):
    if not offset_text.isdigit() or int(offset_text) < 1:
        raise Refused(f"offset '{offset_text}' is not a positive number")
    offset = int(offset_text)
    p1_path, p2_path = f"{part1_dir}/games.tsv", f"{part2_dir}/games.tsv"
    p1_lines, p1_tail = games_lines(p1_path, tail_allowed=True)
    p2_lines, _ = games_lines(p2_path, tail_allowed=False)

    log = open(part2_log, encoding="utf-8", errors="replace").read().splitlines()
    settings = [m for m in map(SETTINGS.match, log) if m]
    repetition = [m for m in map(REPETITION.match, log) if m]
    if len(settings) != 1 or len(repetition) != 1:
        raise Refused(f"{part2_log}: {len(settings)} datagen settings lines and "
                      f"{len(repetition)} repetition: lines, not one each")
    p2_booked = int(settings[0].group(1))
    p2_line = [int(x) for x in repetition[0].groups()]
    if len(p2_lines) != p2_booked:
        raise Refused(f"part 2: {len(p2_lines)} games lines, its log books {p2_booked}")

    lines = {1: {}, 2: {}}
    for part, held, shift in ((1, p1_lines, 0), (2, p2_lines, offset)):
        for game, _, fields in held:
            if game + shift in lines[part]:
                raise Refused(f"part {part}: game {game} has two games lines")
            lines[part][game + shift] = fields

    # The merged root, streamed: each part's games in order, and its rows in recurred games
    # and before their first recurrence, the two figures of the line that are rows.
    order = {1: [], 2: []}
    rows = {1: [0, 0], 2: [0, 0]}
    for r in stream_rows(merged_root):
        game = int(r[4])
        part = 1 if game < offset else 2
        if not order[part] or order[part][-1] != game:
            order[part].append(game)
        fields = lines[part].get(game)
        if fields is None:
            if part == 2:
                raise Refused(f"merged root: game {game} has no games line in part 2's file")
            # Part 1's file cut short, as a power loss leaves it: held to that shape below.
            continue
        if fields[0] != r[1]:
            raise Refused(f"merged root: a row of game {game} carries {r[1]}, not its games "
                          f"line's {fields[0]}")
        if kind_of(fields) == "recurred":
            rows[part][0] += 1
            rows[part][1] += r[5] < fields[2]

    kept_root = order[1]
    p1_root = root_games(f"{part1_dir}/pool.tsv.root")
    if p1_root[:len(kept_root)] != kept_root:
        raise Refused("the merged root's part 1 games are not the first of part 1's root "
                      "games, in its order")
    dropped = set(p1_root[len(kept_root):])
    with_rows = set(p1_root)
    cut = next((i for i, (game, _, _) in enumerate(p1_lines) if game in dropped), len(p1_lines))
    kept = p1_lines[:cut]
    kept_with_row = [game for game, _, _ in kept if game in with_rows]
    # A power loss keeps in each file what the OS had flushed, so part 1's games file may
    # end before games the merger keeps: the lines it has are then the first kept games'.
    if kept_root[:len(kept_with_row)] != kept_with_row:
        first = next(a for a, b in zip(kept_root, kept_with_row) if a != b)
        raise Refused("part 1: its kept games lines are not in its kept root rows' order"
                      if first in lines[1] else
                      f"part 1: game {first} of its kept root rows has no games line and a later "
                      "kept game has one: not a games file cut short, as a power loss cuts it")
    unlined = kept_root[len(kept_with_row):]
    late = [g for g in unlined if g in lines[1]]
    if late:
        raise Refused(f"part 1: game {late[0]}'s games line comes after the cut: its kept games "
                      "lines are not in its kept root rows' order")
    high = [game for game, _, _ in kept if game >= offset]
    if high:
        raise Refused(f"part 1: game {high[0]}'s games line is kept and its id is at or above "
                      f"the offset, {offset}")
    without = len(kept) - len(kept_with_row)
    p2_order = set(order[2])
    if [game + offset for game, _, _ in p2_lines if game + offset in p2_order] != order[2]:
        raise Refused("part 2: its games lines are not in its root rows' order")

    def part_figures(held, part, shift):
        games = {game + shift: fields for game, _, fields in held}
        kinds = {g: kind_of(v) for g, v in games.items()}
        return figures(games, kinds, *rows[part])

    # Part 1's figures need every kept game's line: without one a game's kind is unknown.
    p1_figures = None if unlined else part_figures(kept, 1, 0)
    p2_figures = part_figures(p2_lines, 2, offset)
    if p2_figures != p2_line:
        raise Refused(f"part 2's files give {p2_figures}, its log's repetition: line {p2_line}")

    target = f"{out}.merging"
    sha = hashlib.sha256()
    want = [str(game) for game, _, _ in kept] + [str(game + offset) for game, _, _ in p2_lines]
    with open(target, "xb") as sink:
        try:
            for _, raw, _ in kept:
                sink.write(raw)
                sha.update(raw)
            for game, raw, _ in p2_lines:
                shifted = b"%d" % (game + offset) + raw[raw.index(b"\t"):]
                sink.write(shifted)
                sha.update(shifted)
            sink.close()
            if list(read_games(target)) != want:
                raise Refused(f"{target}, read back: not part 1's kept lines then part 2's "
                              "shifted")
            with open(target, "rb") as back:
                if hashlib.sha256(back.read()).hexdigest() != sha.hexdigest():
                    raise Refused(f"{target}, read back: not the bytes written")
        except BaseException:
            os.remove(target)
            raise

    where = (f"before game {p1_lines[cut][0]}'s, the first naming a game its root rows hold "
             "and the merge dropped" if cut < len(p1_lines) else
             "to its end, none naming a game the merge dropped")
    tail = f" and a tail of {p1_tail} NUL bytes" if p1_tail else ""
    short = (f"; {len(unlined)} of its {len(kept_root)} kept root games, the last in its order, "
             "have no line: the file ends before them, as a power loss leaves it" if unlined else "")
    print(f"part 1    games: {p1_path}, {len(p1_lines)} lines{tail}; the {len(kept)} lines "
          f"{where}, kept: its {len(kept_with_row)} games with a kept root row, in their order, "
          f"and {without} with none; {len(p1_lines) - len(kept)} dropped{short}")
    if p1_figures is None:
        print(f"part 1    repetition, not recomputed: {len(unlined)} of its kept games have no "
              "games line, so their kinds are not known")
    else:
        print(f"part 1    repetition, recomputed from its kept games lines and root rows, its "
              f"datagen stopped before its closing lines: {repetition_words(p1_figures)}")
    print(f"part 2    games: {p2_path}, {len(p2_lines)} lines, the games its log books, each id "
          f"plus {offset}")
    print(f"part 2    repetition, its log's line, which its games lines and root rows give: "
          f"{repetition_words(p2_line)}")
    unlined_words = f", {len(unlined)} of the first with no games line" if unlined else ""
    print(f"merged games: {len(want)} games written to {out}, sha256 {sha.hexdigest()}; "
          f"part 1's {len(kept_root)} games with a row before its cut and {without} without"
          f"{unlined_words}, and part 2's {len(p2_lines)} games played")
    if p1_figures is None:
        print(f"repetition incomplete: {len(unlined)} of part 1's {len(kept_root)} kept games have "
              "no games line, its file ending before them as a power loss leaves it; no combined "
              "repetition: line is given and the merged pool's check is partial, its views merged "
              "as the merger wrote them")
    else:
        combined = [a + b for a, b in zip(p1_figures, p2_line)]
        print(f"repetition: {repetition_words(combined)}")


def main(argv):
    try:
        if len(argv) in (3, 4) and argv[1] == "read":
            read_command(argv[2], argv[3] if len(argv) == 4 else f"{argv[2]}/generate.log")
        elif len(argv) == 5 and argv[1] == "premise":
            premise_command(argv[2], argv[3], argv[4])
        elif len(argv) == 8 and argv[1] == "merge":
            merge_command(*argv[2:])
        else:
            print(__doc__, file=sys.stderr)
            return 2
    except (Refused, OSError) as error:
        print(f"refused: {error}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
