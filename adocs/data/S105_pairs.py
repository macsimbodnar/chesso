#!/usr/bin/env python3
"""Pair-level statistics of an S105 calibration run.

    adocs/data/S105_pairs.py S105_calibration_before.pgn S105_calibration_after.pgn

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
"""

import collections
import re
import statistics
import sys

GAME = re.compile(r'\n(?=\[Event )')
ROUND = re.compile(r'\[Round "([^"]*)"\]')
WHITE = re.compile(r'\[White "([^"]*)"\]')
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


def main(argv):
    if len(argv) < 2:
        print(__doc__.strip().splitlines()[2].strip(), file=sys.stderr)
        return 2
    out = [report(p) for p in argv[1:]]
    if len(out) == 2:
        (v0, n0), (v1, n1) = out
        # sd of a sample variance, normal approximation. Enough to say whether
        # two variances differ at these counts; not a test.
        e0, e1 = v0 * (2 / (n0 - 1)) ** 0.5, v1 * (2 / (n1 - 1)) ** 0.5
        print(f'pair variance    {v0:.4f} +/- {e0:.4f}  vs  {v1:.4f} +/- {e1:.4f}')
        print(f'ratio            {v1 / v0:.3f}')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
