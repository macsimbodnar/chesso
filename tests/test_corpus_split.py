#!/usr/bin/env python3
"""The gate on tools/corpus_split.py, the fixed validation set's builder. S082.

The set it builds is read by every later fit and screen (DEC-259), so the two
properties that make a held-out error mean generalisation are held here, each
on a pool built so that it could fail:

  1. no game is on both sides, and the same games are held out of every leaf's
     file -- the pool's games number the same game the same way in each
  2. nothing is lost: a file's two sides are its rows, each side in file order
  3. the same seed gives the same bytes, another seed another set, and a set
     is never overwritten in place
  4. a sampled corpus holds no position the validation set holds, in any view
     named -- the pool is built with positions repeated across games and
     across views, and the precondition that such a row sits in the training
     side is asserted before sampling
  5. a sampled corpus has exactly the rows asked, at most --per-game a game, a
     game's rows in their order; a pool too small is refused, never answered
     with fewer rows, and leaves no file
  6. a pool whose game rows are not contiguous is refused

Synthetic rows: the builder reads the game column and the FEN's first four
fields and nothing else, so no chess is involved. Stdlib only, under a second.
"""

import contextlib
import io
import os
import re
import shutil
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import corpus_split  # noqa: E402  (after the path is set, which is the point)

KINDS = ("root", "qs", "pv")
GAMES = 40
HELD = 8


def placement(game, row, kind):
    # Every game's first row stands on one shared position, as the early
    # positions of self-play games do, at clocks that differ from game to game;
    # every other row is its game's own. So each training game holds exactly
    # one row the position rule must drop. The pv file's second row is the next
    # game's root second row instead, a position only that view holds, which
    # is what a second --validation has to keep out.
    if row == 0:
        return "shared"
    if kind == "pv" and row == 1:
        return f"g{(game + 1) % GAMES}r1"
    return f"g{game}r{row}"


def pool_rows(kind):
    """{game: [row bytes]} for one leaf's file, games 0..GAMES-1."""
    out = {}
    for game in range(GAMES):
        rows = []
        for row in range(5 + game % 11):
            ply = 8 + row
            distance = 0 if kind == "root" else row % 3
            fen = (f"{placement(game, row, kind)} "
                   f"{'w' if row % 2 == 0 else 'b'} - - "
                   f"{(game + row) % 7} {ply // 2 + 1}")
            rows.append(f"{fen}\t0.5\t{row}\t12\t{game}\t{ply}\t{distance}\n"
                        .encode())
        out[game] = rows
    return out


def run(*argv):
    out = io.StringIO()
    err = io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        code = corpus_split.main(list(argv))
    return code, out.getvalue() + err.getvalue()


def games_of(path):
    with open(path, "rb") as handle:
        return [int(line.split(b"\t")[4]) for line in handle if line.strip()]


def position_of(line):
    return b" ".join(line.split(b"\t")[0].split()[:4])


class CorpusSplitTest(unittest.TestCase):

    def setUp(self):
        self.dir = tempfile.mkdtemp(prefix="chesso_test_corpus_split.")
        self.rows = {kind: pool_rows(kind) for kind in KINDS}
        self.pool = {}
        for kind in KINDS:
            path = os.path.join(self.dir, f"pool.tsv.{kind}")
            with open(path, "wb") as handle:
                for game in range(GAMES):
                    handle.writelines(self.rows[kind][game])
            self.pool[kind] = path

    def tearDown(self):
        shutil.rmtree(self.dir, ignore_errors=True)

    def split(self, seed=7, where=None):
        if where is not None:
            for kind in KINDS:
                shutil.copy(self.pool[kind], where)
            files = [os.path.join(where, os.path.basename(self.pool[k]))
                     for k in KINDS]
        else:
            files = [self.pool[k] for k in KINDS]
        code, text = run("split", "--seed", str(seed),
                         "--validation-games", str(HELD), *files)
        self.assertEqual(code, 0, text)
        return files

    def test_no_game_on_both_sides_and_the_same_games_held_in_every_file(self):
        files = self.split()
        held = None
        for path in files:
            train = set(games_of(path + ".train"))
            valid = set(games_of(path + ".validation"))
            self.assertEqual(train & valid, set())
            self.assertEqual(len(valid), HELD)
            self.assertEqual(train | valid, set(range(GAMES)))
            held = valid if held is None else held
            self.assertEqual(valid, held)

    def test_nothing_is_lost_and_each_side_keeps_file_order(self):
        files = self.split()
        for kind, path in zip(KINDS, files):
            held = set(games_of(path + ".validation"))
            expect = {True: [], False: []}
            for game in range(GAMES):
                expect[game in held].extend(self.rows[kind][game])
            with open(path + ".validation", "rb") as handle:
                self.assertEqual(handle.readlines(), expect[True])
            with open(path + ".train", "rb") as handle:
                self.assertEqual(handle.readlines(), expect[False])

    def test_seeded_reproducible_and_never_overwritten(self):
        first = self.split(seed=7)
        again_dir = os.path.join(self.dir, "again")
        other_dir = os.path.join(self.dir, "other")
        os.mkdir(again_dir)
        os.mkdir(other_dir)
        again = self.split(seed=7, where=again_dir)
        other = self.split(seed=8, where=other_dir)

        for a, b in zip(first, again):
            for side in (".train", ".validation"):
                self.assertEqual(corpus_split.digest(a + side),
                                 corpus_split.digest(b + side))

        self.assertNotEqual(set(games_of(first[1] + ".validation")),
                            set(games_of(other[1] + ".validation")))

        code, text = run("split", "--seed", "7", "--validation-games",
                         str(HELD), *first)
        self.assertEqual(code, 1, text)
        self.assertIn("exists", text)

    def test_a_sample_holds_no_validation_position(self):
        qs = self.split()[1]
        with open(qs + ".validation", "rb") as handle:
            held = {position_of(line) for line in handle if line.strip()}
        with open(qs + ".train", "rb") as handle:
            train = [line for line in handle if line.strip()]

        # The precondition: the training side does hold such a row, so a
        # sample that kept them would be caught.
        self.assertTrue(any(position_of(line) in held for line in train))

        out = os.path.join(self.dir, "sample.tsv")
        code, text = run("sample", "--in", qs + ".train", "--validation",
                         qs + ".validation", "--rows", "60", "--per-game", "3",
                         "--seed", "1", "--out", out)
        self.assertEqual(code, 0, text)
        dropped = re.search(r"; (\d+) rows dropped", text)
        self.assertIsNotNone(dropped, text)
        self.assertGreater(int(dropped.group(1)), 0)

        with open(out, "rb") as handle:
            written = [line for line in handle if line.strip()]
        self.assertFalse(any(position_of(line) in held for line in written))

    def test_a_sample_has_the_rows_asked_at_most_per_game_in_order(self):
        qs = self.split()[1]
        out = os.path.join(self.dir, "sample.tsv")
        code, text = run("sample", "--in", qs + ".train", "--validation",
                         qs + ".validation", "--rows", "60", "--per-game", "3",
                         "--seed", "1", "--out", out)
        self.assertEqual(code, 0, text)

        with open(out, "rb") as handle:
            written = [line for line in handle if line.strip()]
        self.assertEqual(len(written), 60)

        by_game = {}
        for line in written:
            by_game.setdefault(int(line.split(b"\t")[4]), []).append(line)
        for game, lines in by_game.items():
            self.assertLessEqual(len(lines), 3)
            plies = [int(line.split(b"\t")[5]) for line in lines]
            self.assertEqual(plies, sorted(plies))
            # A game's rows are contiguous in the corpus, as in the pool.
            first = written.index(lines[0])
            self.assertEqual(written[first:first + len(lines)], lines)

        again = os.path.join(self.dir, "again.tsv")
        other = os.path.join(self.dir, "other.tsv")
        run("sample", "--in", qs + ".train", "--validation", qs + ".validation",
            "--rows", "60", "--per-game", "3", "--seed", "1", "--out", again)
        run("sample", "--in", qs + ".train", "--validation", qs + ".validation",
            "--rows", "60", "--per-game", "3", "--seed", "2", "--out", other)
        self.assertEqual(corpus_split.digest(out), corpus_split.digest(again))
        self.assertNotEqual(corpus_split.digest(out), corpus_split.digest(other))

    def test_every_named_view_of_the_set_is_kept_out(self):
        root, qs, pv = self.split()

        def held_in(path):
            with open(path, "rb") as handle:
                return {position_of(line) for line in handle if line.strip()}

        qs_held = held_in(qs + ".validation")
        pv_held = held_in(pv + ".validation")
        both = qs_held | pv_held

        with open(root + ".train", "rb") as handle:
            train = [line for line in handle if line.strip()]

        # The precondition: a training row stands on a position only the pv
        # view holds, so a sample read against the qs view alone keeps it.
        self.assertTrue(any(position_of(line) in pv_held - qs_held
                            for line in train))

        # Every row the two views leave, so the sample has to visit them all.
        available = sum(1 for line in train if position_of(line) not in both)

        out = os.path.join(self.dir, "both_views.tsv")
        code, text = run("sample", "--in", root + ".train",
                         "--validation", qs + ".validation",
                         "--validation", pv + ".validation",
                         "--rows", str(available), "--seed", "1",
                         "--out", out)
        self.assertEqual(code, 0, text)

        with open(out, "rb") as handle:
            written = [line for line in handle if line.strip()]
        self.assertEqual(len(written), available)
        self.assertFalse(any(position_of(line) in both for line in written))

    def test_a_pool_too_small_is_refused_and_leaves_nothing(self):
        qs = self.split()[1]
        with open(qs + ".train", "rb") as handle:
            available = sum(1 for line in handle if line.strip())

        out = os.path.join(self.dir, "too_many.tsv")
        code, text = run("sample", "--in", qs + ".train", "--validation",
                         qs + ".validation", "--rows", str(available + 1),
                         "--seed", "1", "--out", out)
        self.assertEqual(code, 1, text)
        self.assertFalse(os.path.exists(out))

    def test_rows_of_a_game_out_of_order_are_refused(self):
        path = os.path.join(self.dir, "scrambled.tsv")
        with open(path, "wb") as handle:
            handle.writelines(self.rows["qs"][0][:2])
            handle.writelines(self.rows["qs"][1])
            handle.writelines(self.rows["qs"][0][2:])
            for game in range(2, GAMES):
                handle.writelines(self.rows["qs"][game])
        code, text = run("split", "--seed", "7", "--validation-games",
                         str(HELD), path)
        self.assertEqual(code, 1, text)
        self.assertIn("not contiguous", text)


if __name__ == "__main__":
    unittest.main()
