#!/usr/bin/env python3
"""Pentanomial statistics over a fastchess PGN, by round range.

fastchess prints one result for the whole match. This reads the games back and
computes the same numbers over any subset of rounds, which is what a match
interrupted part way through needs: whether the blocks either side of the
interruption are the same experiment is a question about subsets, and the
running total cannot answer it.

    adocs/data/S024_pair_stats.py GAMES.PGN            whole file
    adocs/data/S024_pair_stats.py GAMES.PGN 1 1132     a round range
    adocs/data/S024_pair_stats.py GAMES.PGN --drop 1133,1134

VALIDATED AGAINST FASTCHESS ITSELF, and that check is the reason to trust any
number this prints. Over rounds 1 to 1138 of S024's first run it reads
`ptnml [115, 256, 421, 237, 109]`, score 0.4932, `Elo -4.73 +/- 11.16`, against
the `[115, 256, 421, 237, 109]`, 49.32 %, `Elo -4.73 +/- 11.15` fastchess
printed for the same 2276 games. The last digit is rounding.

Pairs, not games: a round is two games on one opening with the colours
reversed, so the pair is the independent unit and its score is the pentanomial
one. A round with fewer than two finished games is skipped rather than counted
as half a pair.

Elo is the logistic conversion of the pair score and its interval comes from
the variance across pairs, which is what makes it a pentanomial figure rather
than a trinomial one. Written for S024's aborted first run; nothing in it is
specific to that match.
"""
import collections
import math
import re
import sys

CANDIDATE = "candidate"


def read_rounds(path):
    rounds = collections.defaultdict(list)
    current = {}

    for line in open(path):
        match = re.match(r'\[(\w+) "(.*)"\]', line.strip())

        if not match:
            continue

        key, value = match.group(1), match.group(2)
        current[key] = value

        if key == "Result" and "Round" in current and "White" in current:
            rounds[int(current["Round"])].append(
                (current["White"], current["Black"], value))
            current = {"Round": current["Round"]}

    return rounds


def candidate_score(white, black, result):
    if result == "1-0":
        return 1.0 if white == CANDIDATE else 0.0
    if result == "0-1":
        return 1.0 if black == CANDIDATE else 0.0
    if result == "1/2-1/2":
        return 0.5
    return None


def elo(score):
    score = min(max(score, 1e-9), 1 - 1e-9)
    return -400 * math.log10(1 / score - 1)


def report(rounds, label, keep):
    ptnml = [0] * 5
    pairs = []

    for number in sorted(rounds):
        if not keep(number):
            continue

        games = rounds[number]

        if len(games) != 2:
            continue

        scores = [candidate_score(*game) for game in games]

        if any(s is None for s in scores):
            continue

        ptnml[int(round(sum(scores) * 2))] += 1
        pairs.append(sum(scores) / 2.0)

    if len(pairs) < 2:
        print("%-24s no pairs" % label)
        return

    n = len(pairs)
    mean = sum(pairs) / n
    variance = sum((p - mean) ** 2 for p in pairs) / (n - 1)
    sigma = math.sqrt(variance / n)
    half = (elo(mean + 1.959964 * sigma) - elo(mean - 1.959964 * sigma)) / 2

    print("%-24s pairs %5d  games %5d  ptnml %-28s score %.4f  Elo %+.2f +/- %.2f"
          % (label, n, n * 2, str(ptnml), mean, elo(mean), half))


def main():
    if len(sys.argv) < 2:
        print(__doc__.strip())
        return 2

    rounds = read_rounds(sys.argv[1])
    rest = sys.argv[2:]

    if rest and rest[0] == "--drop":
        dropped = set(int(x) for x in rest[1].split(","))
        report(rounds, "all pairs", lambda r: True)
        report(rounds, "dropping %s" % rest[1], lambda r: r not in dropped)
    elif len(rest) == 2:
        lo, hi = int(rest[0]), int(rest[1])
        report(rounds, "rounds %d-%d" % (lo, hi), lambda r: lo <= r <= hi)
    else:
        report(rounds, "all pairs", lambda r: True)

    return 0


if __name__ == "__main__":
    sys.exit(main())
