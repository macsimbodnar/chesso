#!/usr/bin/env python3
"""Reads an S219 book comparison run and applies its pre-registered pick rule.

    adocs/data/S219_read.py <run directory>

WHY THIS EXISTS. `S219_book_compare.sh` plays one binary against itself at a
one-doubling time handicap on each candidate book and records what each match
cost in wall time. The question it was built to answer is not which book the
engine scores best on -- the handicap is the same everywhere, so the true
difference is the same everywhere -- but which book *reports* that known
difference most sharply per hour of machine. That is the selection metric
pre-registered in the script's header:

    M = nElo^2 * games_per_hour

and it is proportional to verdicts per hour, because an SPRT at fixed bounds
needs games in proportion to 1/nElo^2. This file computes it, with the error
that decides what counts as a tie, and prints the pick the rule produces so
that the rule and not the reader picks.

WHAT IS REUSED AND WHY IT IS NOT AN EDIT. `S105_pairs.py` already parses a
fastchess PGN into pairs -- and `adocs/data/` is append-only, so it is imported
rather than changed, as `S198_pairs.py` does. The pentanomial, the pairs an
opening decided and the pair score variance are its definitions. What is added
here is the per-book pooling, the Elo and normalized Elo of the handicap, the
throughput and the metric.

THE ENGINE NAME IS LOAD-BEARING. A pair is scored from one side, and a name
matching neither side is not an error: every game then scores as Black's
points, a pair White won twice reads 0.0 instead of 2.0, and the variance comes
back inflated with nothing printed to say so (S198 wrote that warning down).
So the names actually present in the PGN are checked against the side being
read, and a mismatch is fatal here.

NORMALIZED ELO IS THE PUBLISHED FISHTEST DEFINITION, NOT A LOCAL ONE. Michel
Van den Bergh, "Comments on normalized Elo",
https://www.cantate.be/Fishtest/normalized_elo_practical.pdf -- the document
https://github.com/official-stockfish/fishtest/wiki/Fishtest-mathematics links
as the definition of the scale fishtest states its bounds in. It defines the
normalized t-value as t_n = (mu - 1/2) / sigma_pg with mu the expected score
per game, and in the pentanomial case sigma_pg is the standard deviation of the
pair outcome distribution -- scored 0, 1/4, 2/4, 3/4, 1 -- multiplied by
sqrt(2); normalized Elo is t_n times C = 800/log(10) = 347.43.

Written on the pair score scale this file uses, 0 to 2, that is

    mu       = mean(pair score) / 2
    sigma_pg = sqrt(variance(pair score) / 2)
    nElo     = 800/log(10) * (mu - 1/2) / sigma_pg

and it is checked rather than trusted: every match's own stdout.log carries the
Elo, the nElo and the pentanomial fastchess computed from the same games, and
they are printed beside this file's numbers with a verdict on each line. A
disagreement beyond rounding means one of the two is wrong and the run is not
read until it is resolved.
"""

import collections
import math
import os
import re
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S105_pairs  # noqa: E402  -- the path above is what makes it importable

# The two sides `S219_book_compare.sh` names, and the side every score, Elo and
# pentanomial below is read from.
FULL, HALF = 'full', 'half'

# Which candidates are balanced, for the tie rule. The survey's classification
# (adocs/data/S219_book_survey.md): the two UHO books are openings filtered to
# an evaluation band and are unbalanced by construction.
BALANCED = {'popularpos', 'noob3'}
INCUMBENT = 'uho4852'

C_ELO_PER_T = 800 / math.log(10)
Z95 = statistics.NormalDist().inv_cdf(0.975)

TERMINATION = re.compile(r'\[Termination "([^"]*)"\]')
# The final block of a match's stdout.log. fastchess reprints it every rating
# interval, so the last one is the match and the earlier ones are partial.
FC_ELO = re.compile(
    r'^Elo: (\S+) \+/- (\S+), nElo: (\S+) \+/- (\S+)\s*$', re.M)
FC_PTNML = re.compile(r'^Ptnml\(0-2\): \[(\d+), (\d+), (\d+), (\d+), (\d+)\]', re.M)

PENTA_SCORES = (0.0, 0.5, 1.0, 1.5, 2.0)


def elo(mu):
    """Logistic Elo of an expected score per game."""
    if mu <= 0:
        return float('-inf')
    if mu >= 1:
        return float('inf')
    return -400 * math.log10(1 / mu - 1)


def read_match(pgn, engine=FULL):
    """One match's PGN, as pairs scored from `engine`'s side."""
    with open(pgn) as fh:
        text = fh.read()

    by_round = collections.defaultdict(list)
    names = set()
    games = draws = forfeits = 0

    for game in S105_pairs.GAME.split(text):
        result = S105_pairs.RESULT.search(game)
        if result is None:
            continue
        games += 1
        if result.group(1) == '1/2-1/2':
            draws += 1
        termination = TERMINATION.search(game)
        if termination is not None and termination.group(1) == 'time forfeit':
            forfeits += 1
        white = S105_pairs.WHITE.search(game).group(1)
        names.add(white)
        by_round[S105_pairs.ROUND.search(game).group(1)].append(
            (white, result.group(1)))

    if games and engine not in names:
        raise SystemExit(f'{pgn}: no game has {engine!r} as White -- the names '
                         f'are {sorted(names)}, and reading a pair from a name '
                         f'that is not there scores it from the other side')
    unexpected = names - {FULL, HALF}
    if unexpected:
        raise SystemExit(f'{pgn}: {sorted(unexpected)} played here as well as '
                         f'{FULL} and {HALF}, so this file holds more than the '
                         f'one match its round numbers are unique within')

    # A round is one opening played twice. An odd one is a game whose partner is
    # missing and it is dropped rather than counted as half a pair.
    scores, opening_decided = [], 0
    for pair in (v for v in by_round.values() if len(v) == 2):
        scores.append(sum(S105_pairs.POINTS[r] if w == engine
                          else 1.0 - S105_pairs.POINTS[r] for w, r in pair))
        if all(r == '1-0' for _, r in pair):
            opening_decided += 1

    return {'games': games, 'draws': draws, 'forfeits': forfeits,
            'scores': scores, 'opening_decided': opening_decided,
            'rounds': len(by_round)}


def fastchess_summary(stdout_log):
    """What fastchess itself printed for this match, for the cross-check."""
    if not os.path.exists(stdout_log):
        return None
    with open(stdout_log, errors='replace') as fh:
        text = fh.read()
    elos = FC_ELO.findall(text)
    ptnml = FC_PTNML.findall(text)
    if not elos:
        return None
    last = elos[-1]
    return {'elo': last[0], 'elo_err': last[1], 'nelo': last[2],
            'nelo_err': last[3],
            'ptnml': [int(x) for x in ptnml[-1]] if ptnml else None}


def measure(scores, games, wall_s):
    """Pair statistics, the two Elo scales, throughput and the metric."""
    n = len(scores)
    out = {'pairs': n, 'games': games, 'penta': collections.Counter(scores)}
    if n < 2:
        return out

    mean = statistics.fmean(scores)
    var = statistics.pvariance(scores)
    out['mean'] = mean
    out['variance'] = var
    # The sample variance's own error, the normal approximation S105_pairs.main
    # uses, so a variance printed here can be read against S105's band.
    out['variance_err'] = var * (2 / (n - 1)) ** 0.5

    mu = mean / 2                              # expected score per game
    se_mu = math.sqrt(var) / 2 / math.sqrt(n)  # and its standard error
    out['mu'] = mu
    out['elo'] = elo(mu)
    # fastchess converts the score interval's endpoints, which is why its Elo
    # error reads nan once mu + z*se reaches 1; matching it keeps the two
    # columns comparable instead of nearly comparable.
    out['elo_err'] = (elo(mu + Z95 * se_mu) - elo(mu - Z95 * se_mu)) / 2

    sigma_pg = math.sqrt(var / 2)
    if sigma_pg > 0:
        out['sigma_pg'] = sigma_pg
        out['nelo'] = C_ELO_PER_T * (mu - 0.5) / sigma_pg
        # Delta method on mu with sigma held fixed, which is fastchess's own
        # interval: it reduces to z * C / sqrt(2 * pairs) and so depends on the
        # pair count alone.
        out['nelo_se'] = C_ELO_PER_T * se_mu / sigma_pg
        out['nelo_err'] = Z95 * out['nelo_se']

    if wall_s > 0:
        out['gph'] = 3600 * games / wall_s
        if 'nelo' in out:
            out['metric'] = out['nelo'] ** 2 * out['gph']
            # d(n^2 G)/dn * se(n). The wall clock is measured rather than
            # estimated, so G carries no error term here.
            out['metric_se'] = 2 * abs(out['nelo']) * out['nelo_se'] * out['gph']
    return out


def show(label, group):
    games = group['games']
    draws = group['draws']
    m = measure(group['scores'], games, group['wall_s'])
    pairs = m['pairs']

    print(f'--- {label} ---')
    print(f'games            {games}')
    if games:
        print(f'draws            {draws}, {100 * draws / games:.1f} %')
        print(f'forfeits         {group["forfeits"]}, '
              f'{100 * group["forfeits"] / games:.2f} %')
    print(f'complete pairs   {pairs} of {group["rounds"]} rounds')
    if pairs:
        for score in PENTA_SCORES:
            n = m['penta'][score]
            print(f'  pair score {score:<4} {n:6d}  {100 * n / pairs:5.1f} %')
        print(f'opening decided  {group["opening_decided"]} of {pairs}, '
              f'{100 * group["opening_decided"] / pairs:.1f} %  (White won both)')
    if pairs < 2:
        print()
        return m

    print(f'pair score       mean {m["mean"]:.4f}, variance {m["variance"]:.4f} '
          f'+/- {m["variance_err"]:.4f}, sd {m["variance"] ** 0.5:.4f}')
    print(f'Elo  (full-half) {m["elo"]:+.2f} +/- {m["elo_err"]:.2f}')
    if 'nelo' in m:
        print(f'nElo (full-half) {m["nelo"]:+.2f} +/- {m["nelo_err"]:.2f}   '
              f'sigma_pg {m["sigma_pg"]:.4f}')
    print(f'wall             {group["wall_s"]} s')
    if 'gph' in m:
        print(f'games per hour   {m["gph"]:.1f}')
    if 'metric' in m:
        print(f'M = nElo^2 * G   {m["metric"]:.0f} +/- {m["metric_se"]:.0f} (1 sd)')

    printed = group.get('fastchess')
    if printed is not None:
        mine_penta = [m['penta'][s] for s in PENTA_SCORES]
        agree = (printed['ptnml'] is None or printed['ptnml'] == mine_penta)
        print(f'fastchess said   Elo {printed["elo"]} +/- {printed["elo_err"]}, '
              f'nElo {printed["nelo"]} +/- {printed["nelo_err"]}, '
              f'Ptnml {printed["ptnml"]}')
        print(f'this file says   Elo {m["elo"]:.2f} +/- {m["elo_err"]:.2f}, '
              f'nElo {m.get("nelo", float("nan")):.2f} +/- '
              f'{m.get("nelo_err", float("nan")):.2f}, Ptnml {mine_penta}   '
              f'[pentanomial {"agrees" if agree else "DISAGREES"}]')
    print()
    return m


def pick(books):
    """The pre-registered rule, applied to the pooled metric of each book.

    From the script's header: the pick is the largest M; two books whose M
    differ by less than one combined standard error are tied; a tie breaks
    toward the balanced book; and if nothing beats the incumbent by more than
    one standard error and the tied candidate is not balanced, the incumbent
    stays -- no harness change on noise.
    """
    scored = {b: m for b, m in books.items() if 'metric' in m}
    if not scored:
        return None, ['no book has a readable metric']

    best = max(scored, key=lambda b: scored[b]['metric'])
    reasons = [f'largest M: {best} at {scored[best]["metric"]:.0f}']

    tied = []
    for b, m in scored.items():
        if b == best:
            continue
        combined = math.hypot(scored[best]['metric_se'], m['metric_se'])
        if scored[best]['metric'] - m['metric'] < combined:
            tied.append(b)
            reasons.append(f'{b} ties it: M differs by '
                           f'{scored[best]["metric"] - m["metric"]:.0f}, '
                           f'inside one combined sd of {combined:.0f}')
    tie_set = [best] + tied

    balanced = [b for b in tie_set if b in BALANCED]
    if balanced:
        winner = max(balanced, key=lambda b: scored[b]['metric'])
        reasons.append(f'tie broken toward the balanced book: {winner}')
        return winner, reasons
    if INCUMBENT in tie_set:
        reasons.append(f'nothing balanced in the tie set and {INCUMBENT} is in '
                       f'it, so the incumbent stays: no harness change on noise')
        return INCUMBENT, reasons
    reasons.append(f'{best} stands: nothing balanced ties it and it beats '
                   f'{INCUMBENT} by more than one combined sd')
    return best, reasons


def main(argv):
    if len(argv) != 2:
        print(__doc__.strip().splitlines()[2].strip(), file=sys.stderr)
        return 2
    run = argv[1]

    results = os.path.join(run, 'results.tsv')
    if not os.path.exists(results):
        print(f'no {results}', file=sys.stderr)
        return 2

    header, rows = None, []
    print(f'=== {run} ===')
    with open(results) as fh:
        for line in fh:
            line = line.rstrip('\n')
            if line.startswith('#'):
                print(line)
                continue
            fields = line.split('\t')
            if header is None:
                header = fields
                continue
            rows.append(dict(zip(header, fields)))
    print()

    by_book = collections.defaultdict(list)
    for row in rows:
        by_book[row['book']].append(row)

    pooled_all, voided = {}, []
    for book, book_rows in by_book.items():
        kind = 'balanced' if book in BALANCED else 'unbalanced'
        pool = {'games': 0, 'draws': 0, 'forfeits': 0, 'rounds': 0,
                'opening_decided': 0, 'scores': [], 'wall_s': 0,
                'fastchess': None}
        used = 0

        for row in sorted(book_rows, key=lambda r: int(r['pass'])):
            pgn = row['pgn']
            if not os.path.exists(pgn):
                # The run directory may have been moved since the match ran.
                pgn = os.path.join(run, f'pass{row["pass"]}_{book}', 'games.pgn')
            group = read_match(pgn)
            group['wall_s'] = int(row['wall_s'])
            group['fastchess'] = fastchess_summary(
                os.path.join(os.path.dirname(pgn), 'stdout.log'))
            show(f'{book} ({kind})  pass {row["pass"]}  seed {row["seed"]}',
                 group)

            if row['voided'] != 'no':
                voided.append(f'{book} pass {row["pass"]}: {row["voided"]}')
                continue
            for key in ('games', 'draws', 'forfeits', 'rounds',
                        'opening_decided', 'wall_s'):
                pool[key] += group[key]
            pool['scores'] += group['scores']
            used += 1

        if len(book_rows) > 1:
            m = show(f'{book} ({kind})  POOLED over {used} of '
                     f'{len(book_rows)} passes', pool)
        else:
            # One pass is its own pooling, and reprinting it says nothing.
            m = measure(pool['scores'], pool['games'], pool['wall_s'])
        pooled_all[book] = (pool, m)

    for line in voided:
        print(f'VOIDED, left out of the pooled reading  {line}')
    if voided:
        print()

    print('=== the metric, pooled ===')
    print(f'{"book":<12}{"bal":<5}{"games":>7}{"draws%":>8}{"pairs":>7}'
          f'{"nElo":>9}{"+/-95%":>8}{"games/h":>9}{"M":>12}{"+/-1sd":>9}')
    for book, (pool, m) in sorted(pooled_all.items(),
                                  key=lambda kv: -kv[1][1].get('metric', -1)):
        bal = 'yes' if book in BALANCED else 'no'
        draws = 100 * pool['draws'] / pool['games'] if pool['games'] else 0.0
        if 'metric' not in m:
            print(f'{book:<12}{bal:<5}{pool["games"]:>7}{draws:>8.1f}'
                  f'{m["pairs"]:>7}{"too few pairs or no wall time":>38}')
            continue
        print(f'{book:<12}{bal:<5}{pool["games"]:>7}{draws:>8.1f}'
              f'{m["pairs"]:>7}{m["nelo"]:>9.2f}{m["nelo_err"]:>8.2f}'
              f'{m["gph"]:>9.1f}{m["metric"]:>12.0f}{m["metric_se"]:>9.0f}')
    print()

    winner, reasons = pick({b: m for b, (_, m) in pooled_all.items()})
    print('=== the pick, by the rule pre-registered in S219_book_compare.sh ===')
    for reason in reasons:
        print(f'  {reason}')
    print(f'pick             {winner}')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
