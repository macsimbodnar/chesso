#!/usr/bin/env python3
"""S331: what datagen's two-fold rule, a draw at the first recurrence of a position, did to
the games and the labels of every corpus written from S207 (DEC-173) to S331. Stdlib only.

    python3 adocs/data/S331_measure.py read OUT
    python3 adocs/data/S331_measure.py premise OLD_PREFIX NEW_PREFIX NEW_GAMES

READ reads one run of the step's datagen, `--leaf root,qs,pv --games-out games.tsv`, in OUT:
its log OUT/generate.log, OUT/games.tsv and OUT/pool.tsv.root. It refuses, exit 1, unless the
files hold what the log counts: one games line for each game, the root rows the `leaf root:`
line counts, every row of every leaf carrying its game's result, and the `repetition:` line's
figures recomputed from the files.

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
"""
import collections
import math
import re
import sys

LEAVES = ("root", "qs", "pv")
PHASE_BANDS = ((0, 4), (5, 9), (10, 14), (15, 19), (20, 24))
CENSUS_CUT = 6

SETTINGS = re.compile(r"^datagen: (\d+) games, (\d+) nodes per move, (\d+) threads, "
                      r"seed (\d+), quiet-limit (-?\d+), leaf (\S+)$")
WRITTEN = re.compile(r"^leaf (\w+): (\d+) rows from (\d+) games written to ")
REPETITION = re.compile(
    r"^repetition: (\d+) games reached a position that had occurred before, which drew a "
    r"game until S331; (\d+) of them ended other than drawn, (\d+) won by White and (\d+) "
    r"lost; (\d+) positions passed in them, a root row each, (\d+) before the first "
    r"recurrence; (\d+) openings ended on one, discarded until S331$")


class Refused(Exception):
    pass


def read_games(path):
    games = {}
    for number, line in enumerate(open(path, encoding="utf-8"), 1):
        fields = line.rstrip("\n").split("\t")
        if len(fields) != 5:
            raise Refused(f"{path}:{number}: {len(fields)} fields, not 5")
        game, result, first, recurred, end = fields
        if game in games:
            raise Refused(f"{path}:{number}: game {game} twice")
        games[game] = (result, int(first), int(recurred), int(end))
    return games


def read_rows(path):
    """fen, result, score, phase, game, ply, distance; in file order."""
    rows = []
    for number, line in enumerate(open(path, encoding="utf-8"), 1):
        fields = line.rstrip("\n").split("\t")
        if len(fields) != 7:
            raise Refused(f"{path}:{number}: {len(fields)} fields, not 7")
        fen, result, score, phase, game, ply, distance = fields
        rows.append((fen, result, int(score), int(phase), game, int(ply), int(distance)))
    return rows


def kind_of(game):
    _, first, recurred, _ = game
    if recurred < 0:
        return "plain"
    return "opening" if recurred == first else "recurred"


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


def check_run(out):
    log = open(f"{out}/generate.log", encoding="utf-8", errors="replace").read().splitlines()
    settings = [m for m in map(SETTINGS.match, log) if m]
    if len(settings) != 1:
        raise Refused(f"{out}/generate.log: {len(settings)} datagen settings lines, not one")
    booked, nodes, threads, seed, quiet, leaves = settings[0].groups()
    if any(line.startswith("datagen: FAILED") for line in log):
        raise Refused(f"{out}/generate.log carries datagen's FAILED line")
    written = {m.group(1): (int(m.group(2)), int(m.group(3))) for m in map(WRITTEN.match, log) if m}
    repetition = [m for m in map(REPETITION.match, log) if m]
    if len(repetition) != 1:
        raise Refused(f"{out}/generate.log: {len(repetition)} repetition: lines, not one")
    if "root" not in written:
        raise Refused(f"{out}/generate.log: no 'leaf root:' line, so no root rows to read")
    games = read_games(f"{out}/games.tsv")
    if len(games) != int(booked) or written["root"][1] != int(booked):
        raise Refused(f"{len(games)} games in games.tsv, {written['root'][1]} in the log, "
                      f"{booked} booked")
    rows = {}
    for leaf, (count, _) in written.items():
        rows[leaf] = read_rows(f"{out}/pool.tsv.{leaf}")
        if len(rows[leaf]) != count:
            raise Refused(f"pool.tsv.{leaf} holds {len(rows[leaf])} rows, the log {count}")
        wrong = sum(1 for r in rows[leaf] if r[4] not in games or games[r[4]][0] != r[1])
        if wrong:
            raise Refused(f"{wrong} rows of pool.tsv.{leaf} carry a result not their game's")
    return settings[0].groups(), [int(x) for x in repetition[0].groups()], games, rows


def read_command(out):
    (booked, nodes, threads, seed, quiet, leaves), line, games, rows = check_run(out)
    root = rows["root"]

    kinds = {g: kind_of(v) for g, v in games.items()}
    recurred = [g for g, k in kinds.items() if k == "recurred"]
    won = sum(1 for g in recurred if games[g][0] == "1.0")
    lost = sum(1 for g in recurred if games[g][0] == "0.0")
    openings = sum(1 for k in kinds.values() if k == "opening")

    in_recurred = [r for r in root if kinds[r[4]] == "recurred"]
    before = [r for r in in_recurred if r[5] < games[r[4]][2]]
    recomputed = [len(recurred), won + lost, won, lost, len(in_recurred), len(before), openings]
    if recomputed != line:
        raise Refused(f"the repetition: line reads {line}, the files {recomputed}")

    # Per game: its old rows, and of them those whose label differs, by band and direction.
    old = collections.defaultdict(lambda: collections.Counter())
    for r in root:
        game = games[r[4]]
        kind = kinds[r[4]]
        if kind == "opening" or (kind == "recurred" and r[5] >= game[2]):
            continue
        band = min(max(r[3], 0) // 5, len(PHASE_BANDS) - 1)  # datagen's phase_band()
        keys = ["all", f"band{band}", "low" if r[3] <= CENSUS_CUT else "high"]
        differs = kind == "recurred" and game[0] != "0.5"
        for key in keys:
            old[r[4]][key] += 1
            if differs:
                old[r[4]][key + ":d"] += 1
                old[r[4]][key + (":w" if game[0] == "1.0" else ":l")] += 1
        if differs:
            toward = (r[2] > 0 and game[0] == "1.0") or (r[2] < 0 and game[0] == "0.0")
            old[r[4]]["toward"] += toward
            old[r[4]]["zero"] += r[2] == 0

    old_games = [g for g, k in kinds.items() if k != "opening"]
    plain_draws = sum(1 for g in old_games if kinds[g] == "plain" and games[g][0] == "0.5")
    n = len(games)

    print(f"S331 reading of {out}: datagen {booked} games, {nodes} nodes a move, {threads} "
          f"threads, seed {seed}, quiet-limit {quiet}, leaf {leaves}")
    print(f"files     games.tsv {n} games; pool.tsv.root {len(root)} rows; every row's result "
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

    def share(key, suffix):
        parts = [old[g][key + suffix] for g in old_games]
        wholes = [old[g][key] for g in old_games]
        return sum(parts), sum(wholes), clustered(parts, wholes)

    labels = [("all", "all phases")]
    labels += [(f"band{i}", f"phase {lo} to {hi}") for i, (lo, hi) in enumerate(PHASE_BANDS)]
    labels += [("low", f"phase <= {CENSUS_CUT}"), ("high", f"phase > {CENSUS_CUT}")]
    for key, name in labels:
        d, o, s = share(key, ":d")
        w, _, sw = share(key, ":w")
        l, _, sl = share(key, ":l")
        print(f"  {name:15s} old {o:8d}  differs {d:7d} {s:>20s}   to win {w:7d} {sw:>20s}   "
              f"to loss {l:7d} {sl:>20s}")
    d = sum(old[g]["all:d"] for g in old_games)
    toward = sum(old[g]["toward"] for g in old_games)
    zero = sum(old[g]["zero"] for g in old_games)
    print(f"  of the {d} that differ, the new label goes the way of the row's own score in "
          f"{toward}, against it in {d - toward - zero}, score 0 in {zero}")
    fresh = collections.Counter()
    every = collections.Counter()
    for r in root:
        every[r[4]] += 1
        kind = kinds[r[4]]
        fresh[r[4]] += kind == "opening" or (kind == "recurred" and r[5] >= games[r[4]][2])
    after = sum(fresh[g] for g in recurred)
    from_openings = sum(fresh.values()) - after
    print(f"  new rows the old rule never wrote: {after} after a first recurrence and "
          f"{from_openings} from the openings, "
          f"{clustered([fresh[g] for g in games], [every[g] for g in games])} of the "
          f"{len(root)} root rows")


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


def main(argv):
    try:
        if len(argv) == 3 and argv[1] == "read":
            read_command(argv[2])
        elif len(argv) == 5 and argv[1] == "premise":
            premise_command(argv[2], argv[3], argv[4])
        else:
            print(__doc__, file=sys.stderr)
            return 2
    except (Refused, OSError) as error:
        print(f"refused: {error}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
