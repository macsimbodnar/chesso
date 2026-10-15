#!/usr/bin/env python3
"""The fixed validation set, and the corpora fitted against it. S082.

    tools/corpus_split.py split --seed 20261013 --validation-games 12000 \\
        POOL.root POOL.qs POOL.pv
    tools/corpus_split.py sample --in POOL.qs.train \\
        --validation POOL.qs.validation \\
        --rows 1000000 --per-game 8 --seed 1 --out corpus.tsv

Reads what tools/datagen writes since S082, `fen result score phase game ply
distance`, one file per leaf, every file of one run numbering the same game
the same way.

SPLIT holds out whole games, and chooses them before anything is sampled. The
game numbers of every file named are pooled, shuffled under `--seed`, and the
first `--validation-games` of them are the validation side. Each FILE becomes
FILE.validation, every row of those games, and FILE.train, every row of the
rest, in the order datagen wrote them. No game is on both sides by
construction, and the outputs are read back to show it. The validation set the
tuner is then handed with `--holdout` is one of the .validation files, kept
for every later fit and screen (DEC-259); the step that builds it records which
one, the seed, the game and row counts and the digest this prints.

SAMPLE makes one corpus out of a .train file. Games are taken in an order
shuffled under `--seed`; a row standing on a position any `--validation` file
holds is dropped -- the same position on both sides measures recall, not
generalisation, and the tuner refuses a corpus that has one; name every view
of the set a fit may be read on -- and then at
most `--per-game` rows of each game are kept, chosen under the same seed and
kept in their order, until exactly `--rows` are written. A pool too small for
that is refused rather than answered with a smaller corpus: the corpus size is
what a recipe comparison holds fixed.

A position is the FEN's first four fields, as tools/tuner_split.hpp
row_position() reads it -- the fields the engine's own key covers, never the
clocks.

The shuffles are Python's `random.Random(seed)`, the same on every machine
running the same Python. That is reproducible enough for a file whose digest is
recorded; it is the digest, not the seed, that names the set.

Standard library only. Both commands stream the pool and hold an index of
games, not their rows, so a pool of ten million rows costs its validation
positions and a game index in memory.
"""

import argparse
import hashlib
import os
import random
import sys


class Refused(Exception):
    """A run that would produce something other than what was asked."""


def row_fields(line, path, number):
    fields = line.rstrip(b"\n").split(b"\t")
    if len(fields) < 7:
        raise Refused(f"{path}:{number}: {len(fields)} fields, datagen's rows "
                      "have seven (fen result score phase game ply distance)")
    try:
        game = int(fields[4])
    except ValueError:
        raise Refused(f"{path}:{number}: game field {fields[4]!r} is not a "
                      "number") from None
    return fields, game


def position(fen_field, path, number):
    parts = fen_field.split()
    if len(parts) < 4:
        raise Refused(f"{path}:{number}: no position in FEN {fen_field!r}")
    return b" ".join(parts[:4])


def index_games(path):
    """[(game, first byte, end byte, rows)] in file order.

    A game's rows are contiguous in what datagen writes -- one lock_guard per
    game -- and every reader of the format leans on it, so a game number that
    comes back after another game's rows is refused, never merged.
    """
    games = []
    seen = set()
    offset = 0
    with open(path, "rb") as handle:
        for number, line in enumerate(handle, 1):
            start = offset
            offset += len(line)
            if not line.strip():
                continue
            _, game = row_fields(line, path, number)
            if games and games[-1][0] == game:
                first, _, rows = games[-1][1:]
                games[-1] = (game, first, offset, rows + 1)
                continue
            if game in seen:
                raise Refused(f"{path}:{number}: rows of game {game} are not "
                              "contiguous")
            seen.add(game)
            games.append((game, start, offset, 1))
    return games


def digest(path):
    sha = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            sha.update(block)
    return sha.hexdigest()


def refuse_existing(paths):
    for path in paths:
        if os.path.exists(path):
            raise Refused(f"{path} exists; a fixed set is never overwritten "
                          "in place")


def split(args):
    indexes = {path: index_games(path) for path in args.files}

    games = sorted({game for index in indexes.values()
                    for game, *_ in index})
    if args.validation_games <= 0 or args.validation_games >= len(games):
        raise Refused(f"--validation-games {args.validation_games} out of "
                      f"range: the pool holds {len(games)} games")

    order = list(games)
    random.Random(args.seed).shuffle(order)
    held = set(order[:args.validation_games])

    # What names the set before any row is written: the held-out game numbers,
    # sorted, one per line.
    held_text = "".join(f"{game}\n" for game in sorted(held)).encode()
    held_digest = hashlib.sha256(held_text).hexdigest()

    outputs = [(path, path + ".train", path + ".validation")
               for path in args.files]
    refuse_existing([out for _, train, valid in outputs
                     for out in (train, valid)])

    print(f"split: seed {args.seed}, {len(games)} games, "
          f"{len(held)} held out, game list sha256 {held_digest}")

    for path, train_path, valid_path in outputs:
        rows = {True: 0, False: 0}
        sides = {True: set(), False: set()}
        with open(path, "rb") as source, \
                open(train_path, "wb") as train, \
                open(valid_path, "wb") as valid:
            for number, line in enumerate(source, 1):
                if not line.strip():
                    continue
                _, game = row_fields(line, path, number)
                side = game in held
                (valid if side else train).write(line)
                rows[side] += 1
                sides[side].add(game)

        # Read back, not trusted: the property is the files', so it is
        # measured on them.
        back = {True: {g for g, *_ in index_games(valid_path)},
                False: {g for g, *_ in index_games(train_path)}}
        if back[True] & back[False]:
            raise Refused(f"{path}: {len(back[True] & back[False])} games on "
                          "both sides of the split")
        if back[True] != sides[True] or back[False] != sides[False]:
            raise Refused(f"{path}: the written files do not hold the games "
                          "that were routed to them")

        for side_path, side in ((valid_path, True), (train_path, False)):
            print(f"  {side_path}: {rows[side]} rows, {len(sides[side])} "
                  f"games, sha256 {digest(side_path)}")
    return 0


def validation_positions(paths):
    held = set()
    for path in paths:
        with open(path, "rb") as handle:
            for number, line in enumerate(handle, 1):
                if not line.strip():
                    continue
                fields, _ = row_fields(line, path, number)
                held.add(position(fields[0], path, number))
    return held


def sample(args):
    if args.rows <= 0:
        raise Refused("--rows must be positive")
    if args.per_game < 0:
        raise Refused("--per-game must be 0 (every row) or positive")
    refuse_existing([args.out])

    held = validation_positions(args.validation)
    games = index_games(args.input)
    named = ", ".join(args.validation)

    rng = random.Random(args.seed)
    order = list(range(len(games)))
    rng.shuffle(order)

    written = 0
    used = 0
    dropped = 0
    with open(args.input, "rb") as source, open(args.out, "wb") as out:
        for g in order:
            if written >= args.rows:
                break
            game, first, end, _ = games[g]
            source.seek(first)
            block = source.read(end - first).splitlines(keepends=True)

            kept = []
            for line in block:
                if not line.strip():
                    continue
                fields, number_game = row_fields(line, args.input, "?")
                if number_game != game:
                    raise Refused(f"{args.input}: the index of game {game} "
                                  "reads another game's row")
                if position(fields[0], args.input, "?") in held:
                    dropped += 1
                    continue
                kept.append(line)

            if args.per_game > 0 and len(kept) > args.per_game:
                chosen = sorted(rng.sample(range(len(kept)), args.per_game))
                kept = [kept[i] for i in chosen]

            take = kept[:args.rows - written]
            if not take:
                continue
            out.writelines(take)
            written += len(take)
            used += 1

    if written < args.rows:
        os.remove(args.out)
        raise Refused(f"{args.input} holds {written} rows at --per-game "
                      f"{args.per_game} once the validation set's positions "
                      f"are dropped; {args.rows} were asked for")

    print(f"sample: {written} rows from {used} games "
          f"({written / used:.2f} a game) of {len(games)}, "
          f"--per-game {args.per_game}, seed {args.seed}; {dropped} rows "
          f"dropped for a position {named} holds; "
          f"sha256 {digest(args.out)}  {args.out}")
    return 0


def main(argv):
    parser = argparse.ArgumentParser(
        description="The fixed validation set and the corpora fitted "
                    "against it. S082.")
    commands = parser.add_subparsers(dest="command", required=True)

    cut = commands.add_parser("split", help="hold out whole games")
    cut.add_argument("--seed", type=int, required=True)
    cut.add_argument("--validation-games", type=int, required=True)
    cut.add_argument("files", nargs="+")

    draw = commands.add_parser("sample", help="one corpus from a .train file")
    draw.add_argument("--in", dest="input", required=True)
    draw.add_argument("--validation", required=True, action="append",
                      help="a validation file whose positions the corpus "
                           "may not hold; repeat for several views of one "
                           "set")
    draw.add_argument("--rows", type=int, required=True)
    draw.add_argument("--per-game", type=int, default=0)
    draw.add_argument("--seed", type=int, required=True)
    draw.add_argument("--out", required=True)

    args = parser.parse_args(argv)
    try:
        return split(args) if args.command == "split" else sample(args)
    except Refused as why:
        print(f"corpus_split: refused: {why}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
