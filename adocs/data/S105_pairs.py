#!/usr/bin/env python3
"""Pair-level statistics of an S105 calibration run.

    adocs/data/S105_pairs.py S105_calibration_before.pgn S105_calibration_after.pgn
    adocs/data/S105_pairs.py <run>/games.pgn       a fastchess.sh run, since S258
    adocs/data/S105_pairs.py --self-test

WHY THIS EXISTS. The draw rate is the number the book class is specified in --
Pohl's floor is about 45 % draws -- but it is a proxy. What the floor is really
about is pairs that carry no signal: with `-repeat` every opening is played
twice with the colours reversed, and a pair where each side won its white game
scores 1:1 and says nothing about which engine is stronger. Below the floor,
the claim goes, an unbalanced book buys decisive games that are decided by the
opening rather than by the engine.

So this counts the pairs directly, and the pentanomial they fall into, instead
of arguing from the draw rate. The load-bearing number is the variance of the
pair score: that is what sets how many games a verdict costs at fixed Elo
bounds, and it is the number that says whether the book bought anything.

The runs it reads are A/A -- one binary played against itself -- so the true
difference is zero by construction and the pair distribution is the book's and
the control's, with no strength effect mixed into it.

WHOSE POINTS THE COMMAND LINE SCORES. `report()` scores a pair from the side
it is named, and the command line passes the name `side_name()` takes from the
PGN: the one side `S024_pair_stats.is_candidate` accepts (`candidate`, or the
`cand-<sha>` and `cand-<HEAD>+<hex>` `fastchess.sh` names since S256), and
`chesso-a`, S105's own calibration, only where no side matches and the PGN
names `chesso-a`. Anything else exits 1 with a sentence. Until S258 it passed
`chesso-a` to every PGN, which scored a `fastchess.sh` run from Black's side
with exit 0 and then, since S257, refused it. Ten SPRT readings were scored
from Black's side, by this command line or by a caller passing a name no side
carried (`S258_reread_pairs.txt` re-reads them). `--self-test` runs the
derivation, the refusals and S105's own two PGNs, and checks a renamed band's
pentanomial against the one fastchess printed for it.
"""

import collections
import contextlib
import io
import os
import re
import statistics
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S024_pair_stats  # noqa: E402  -- the path above is what makes it importable

GAME = re.compile(r'\n(?=\[Event )')
ROUND = re.compile(r'\[Round "([^"]*)"\]')
WHITE = re.compile(r'\[White "([^"]*)"\]')
BLACK = re.compile(r'\[Black "([^"]*)"\]')
RESULT = re.compile(r'\[Result "([^"]*)"\]')
PLIES = re.compile(r'\[PlyCount "(\d+)"\]')
DURATION = re.compile(r'\[GameDuration "(\d+):(\d+):(\d+)"\]')

POINTS = {'1-0': 1.0, '0-1': 0.0, '1/2-1/2': 0.5}


def report(path, engine='chesso-a'):
    text = open(path).read()
    by_round = collections.defaultdict(list)
    plies, seconds = [], []

    for game in GAME.split(text):
        result = RESULT.search(game)
        if result is None:
            continue
        by_round[ROUND.search(game).group(1)].append(
            (WHITE.search(game).group(1), result.group(1)))
        plies.append(int(PLIES.search(game).group(1)))
        h, m, s = DURATION.search(game).groups()
        seconds.append(int(h) * 3600 + int(m) * 60 + int(s))

    games = len(plies)
    draws = sum(1 for v in by_round.values() for _, r in v if r == '1/2-1/2')

    # A round is one opening played twice. An odd one is a game whose partner
    # is missing and it is dropped rather than counted as half a pair.
    pairs = [v for v in by_round.values() if len(v) == 2]

    # The colours reverse within a pair, so a side's name is White in exactly
    # one of its two games. A name that is not a side scores every game as
    # Black's points and the variance comes out wrong with nothing printed;
    # ten SPRT readings here, S231's among them, were scored that way over
    # `cand-<sha>` PGNs before this refused it. S257.
    for round_id, pair in by_round.items():
        if len(pair) == 2 and sum(w == engine for w, _ in pair) != 1:
            sys.exit(f'S105_pairs: {engine!r} is White in '
                     f'{sum(w == engine for w, _ in pair)} of the 2 games of '
                     f'round {round_id} in {path} (sides '
                     f'{" and ".join(sorted(w for w, _ in pair))}), so it is '
                     'not a side of this PGN; pass report() the name the run '
                     'gave the side it scores')

    scores, opening_decided = [], 0
    for pair in pairs:
        scores.append(sum(POINTS[r] if w == engine else 1.0 - POINTS[r]
                          for w, r in pair))
        if all(r == '1-0' for _, r in pair):
            opening_decided += 1

    penta = collections.Counter(scores)
    variance = statistics.pvariance(scores)

    print(f'=== {path} ===')
    print(f'games            {games}')
    print(f'draws            {draws}, {100 * draws / games:.1f} %')
    print(f'decisive         {games - draws}, {100 * (games - draws) / games:.1f} %')
    print(f'plies per game   mean {statistics.fmean(plies):.1f}, '
          f'median {statistics.median(plies):.0f}')
    print(f'seconds per game mean {statistics.fmean(seconds):.1f}, '
          f'median {statistics.median(seconds):.0f}')
    print(f'seconds per ply  {sum(seconds) / sum(plies):.4f}')
    print(f'complete pairs   {len(pairs)}')
    for score in (0.0, 0.5, 1.0, 1.5, 2.0):
        n = penta[score]
        print(f'  pair score {score:<4} {n:5d}  {100 * n / len(pairs):5.1f} %')
    print(f'white won both   {opening_decided} of {len(pairs)}, '
          f'{100 * opening_decided / len(pairs):.1f} %  '
          f'(the pair Pohl\'s floor is about)')
    print(f'pair score       mean {statistics.fmean(scores):.4f}, '
          f'variance {variance:.4f}, sd {variance ** 0.5:.4f}')
    print()
    return variance, len(pairs)


def side_name(path):
    """The side the command line scores `path` from, or exit 1 with a sentence.

    `report()`'s default `chesso-a` is S105's own calibration and no side of a
    `fastchess.sh` PGN, so taking it unasked scored ten SPRT readings as
    Black's points (S258). `chesso-a` is the fallback only where the PGN names
    it and no side is the candidate."""
    with open(path) as handle:
        text = handle.read()
    names = sorted({match.group(1)
                    for pattern in (WHITE, BLACK)
                    for match in pattern.finditer(text)})
    matched = [name for name in names if S024_pair_stats.is_candidate(name)]
    if len(matched) == 1:
        return matched[0]
    if not matched and 'chesso-a' in names:
        return 'chesso-a'
    sys.exit(f'S105_pairs: {path} names {", ".join(names) or "no engine"}, '
             f'and {len(matched)} of them are `candidate` or `cand-...` where '
             'the command line scores exactly one such side, or `chesso-a` '
             'when none is and the PGN names it, so it reads nothing rather '
             'than a wrong variance; call report() with the side\'s name')


def main(argv):
    if len(argv) < 2:
        print('\n'.join(line.strip() for line in
                        __doc__.strip().splitlines()[2:5]), file=sys.stderr)
        return 2
    if argv[1:] == ['--self-test']:
        return self_test()
    out = [report(p, engine=side_name(p)) for p in argv[1:]]
    if len(out) == 2:
        (v0, n0), (v1, n1) = out
        # sd of a sample variance, normal approximation. Enough to say whether
        # two variances differ at these counts; not a test.
        e0, e1 = v0 * (2 / (n0 - 1)) ** 0.5, v1 * (2 / (n1 - 1)) ** 0.5
        print(f'pair variance    {v0:.4f} +/- {e0:.4f}  vs  {v1:.4f} +/- {e1:.4f}')
        print(f'ratio            {v1 / v0:.3f}')
    return 0


PTNML = re.compile(r'^Ptnml\(0-2\): \[(\d+), (\d+), (\d+), (\d+), (\d+)\]',
                   re.MULTILINE)
PAIR_SCORE = re.compile(r'^  pair score (\S+) +(\d+) ', re.MULTILINE)


def printed_pentanomial(text):
    """The five `pair score` counts `report()` printed, in Ptnml(0-2) order:
    the scored side's 0, 0.5, 1, 1.5 and 2 points a pair."""
    return [int(count) for _, count in PAIR_SCORE.findall(text)]


def fastchess_pentanomial(log_path):
    """The last `Ptnml(0-2)` fastchess printed in a run log: the final one."""
    with open(log_path) as handle:
        found = PTNML.findall(handle.read())
    return [int(count) for count in found[-1]] if found else None


def self_test():
    # Imported here, not at the top: S198_pairs imports this module, and its
    # four-pair fabricated PGN (pair variance 0.5000 and error bar 0.4082 for
    # four pairs, both by hand) is reused rather than copied.
    import S198_pairs

    failures = []

    def expect(what, got, want):
        if got != want:
            failures.append(f'{what}: got {got!r}, expected {want!r}')
        print(f'  {"ok " if got == want else "FAIL"} {what:<44} {got!r}')

    def run_main(*paths):
        """main()'s stdout over `paths`, or how it refused."""
        out = io.StringIO()
        try:
            with contextlib.redirect_stdout(out):
                code = main(['S105_pairs.py', *paths])
        except SystemExit as refused:
            return f'refused: {refused.code}'
        return out.getvalue() if code == 0 else f'exit {code}'

    def derive(path):
        """side_name(path), or how it refused."""
        try:
            return side_name(path)
        except SystemExit as refused:
            return f'refused: {refused.code}'

    def variance_line(text):
        lines = [line for line in text.splitlines()
                 if line.startswith('pair score       mean')]
        return lines[0].split('variance ')[1] if len(lines) == 1 else text

    print('=== S105_pairs self-test')
    with tempfile.TemporaryDirectory(prefix='S105_selftest_') as directory:
        def fixture(name, text):
            path = os.path.join(directory, name)
            with open(path, 'w') as handle:
                handle.write(text)
            return path

        # Every name the command line has to read: the three fastchess.sh has
        # given the candidate, and S105's own `chesso-a`, the one fallback.
        for candidate, reference in (('candidate', 'ref-1a2b3c4'),
                                     ('cand-1a2b3c4', 'ref-1a2b3c4'),
                                     ('cand-1a2b3c4+0123456789ab', 'ref-1a2b3c4'),
                                     ('chesso-a', 'chesso-b')):
            path = fixture('named.pgn',
                           S198_pairs.self_test_pgn(candidate, reference))
            expect(f'derived {candidate}', derive(path), candidate)
            expect(f'main as {candidate}', variance_line(run_main(path)),
                   '0.5000, sd 0.7071')

        for what, candidate, reference in (
                ('neither side', 'chesso-x', 'chesso-y'),
                ('both sides', 'cand-1a2b3c4', 'cand-5d6e7f8'),
                ('chesso-b alone', 'chesso-b', 'ref-1a2b3c4')):
            path = fixture('refused.pgn',
                           S198_pairs.self_test_pgn(candidate, reference))
            got = run_main(path)
            # side_name's sentence, naming every side, and not report()'s
            # guard firing later on a name it was handed.
            expect(f'main refuses {what}',
                   got.startswith(f'refused: S105_pairs: {path} names '), True)
            print(f'       {got}')

        # S105's own two PGNs through the command line, under `chesso-a`.
        # Golden 0.2343 +/- 0.0148 vs 0.2395 +/- 0.0152: S105's calibration as
        # recorded in S105_calibration_pairs.txt, re-derived by this script as
        # `S105_pairs.py S105_calibration_before.pgn S105_calibration_after.pgn`.
        got = run_main(os.path.join(HERE, 'S105_calibration_before.pgn'),
                       os.path.join(HERE, 'S105_calibration_after.pgn'))
        lines = [line for line in got.splitlines()
                 if line.startswith('pair variance')]
        expect('main over S105\'s two PGNs', lines,
               ['pair variance    0.2343 +/- 0.0148  vs  0.2395 +/- 0.0152'])

        # A `cand-` PGN reads the pentanomial fastchess printed for the same
        # games: S219's band renamed the way a run is named since S256,
        # against the last Ptnml(0-2) in its own log, read live and not typed.
        want = fastchess_pentanomial(os.path.join(HERE,
                                                  'S219_aa_calibration.log'))
        with open(os.path.join(HERE, 'S219_aa_calibration.pgn')) as handle:
            renamed = handle.read().replace('[White "candidate"]',
                                            '[White "cand-5047070"]')
            renamed = renamed.replace('[Black "candidate"]',
                                      '[Black "cand-5047070"]')
        path = fixture('band_renamed.pgn', renamed)
        expect('renamed band derived', derive(path), 'cand-5047070')
        expect('renamed band = fastchess Ptnml',
               printed_pentanomial(run_main(path)), want)

    if failures:
        print(f'SELF-TEST FAILED: {len(failures)}')
        for line in failures:
            print(f'  {line}')
        return 1
    print('SELF-TEST OK')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
