#!/usr/bin/env python3
"""S199's drift reader: one finished drift match becomes one row of a trend.

    adocs/data/S199_drift.py <outdir> <runlog> [--verdicts IDS] [--conversions IDS]
    adocs/data/S199_drift.py --check
    adocs/data/S199_drift.py --self-test [DIR]

WHAT IT READS AND WHAT IT WRITES. `<outdir>` is the run's `OUT` directory, the
one `adocs/data/S199_drift.sh` stamps under `.tuning/`; `<runlog>` is the
detached run's own log, the file `nohup` captured. From the log it takes the
banner (both shas, the control, the hash, the book) and fastchess's **final**
results block; from `<outdir>/games.pgn` it takes the pentanomial and the pair
score variance. It appends one row to `adocs/data/S199_drift.tsv` and prints
what it appended. Stdlib only, and `S105_pairs` for the pair arithmetic --
imported rather than copied, because this directory is append-only (the
S198_pairs.py precedent).

THE ROW IS NEVER HALF FROM ONE RUN. Four refusals, each of them a class that
has happened here or nearly has:

  1. no `SPRT-RUN-DONE` in the log, or an `SPRT-RUN-INVALID` anywhere in it --
     an unfinished or voided run is not a reading (S212, DEC-061);
  2. a `bounds` line that is not `none -- fixed ... rounds` -- a drift point
     is a fixed-rounds estimate and an SPRT's early-stopped estimate is a
     different quantity, biased upward (DEC-063, DEC-143);
  3. the pentanomial computed from the PGN disagreeing with the one fastchess
     printed -- then the log and the PGN are not the same run, or the pairs
     were read under the wrong engine name;
  4. the PGN naming anything but two engines, or neither of them matching the
     banner's reference label.

THE ENGINE NAME IS DERIVED, NOT ASSUMED, AND THAT IS THE POINT OF (4).
`S105_pairs.report` takes the engine whose points a pair is scored in and
defaults to `chesso-a`; a name matching neither side is **not** an error --
every game then scores as Black's points, a pair one side won twice reads 0.0
instead of 2.0, and the variance comes back inflated with nothing printed to
say so. S151's and S198's headers both write that trap down. So the name is
read off the PGN: the side that is not `ref-<reference sha>` is the candidate,
and anything else is refused rather than guessed.

THE TREND, AND WHEN TWO ROWS MAY BE SUBTRACTED. `--check` re-reads the TSV and
prints each point with the step ids landed since the one before it. It
differences two consecutive rows only when they share a reference sha **and** a
regime: the book has already moved once under this project (DEC-189), a
control or hash change would move every number in the column, and a trend
across two regimes is not a trend. The reading rule itself -- what a point
inside, below or above the band means -- is in
`adocs/plan_current/S199_drift_match_against_pinned_reference.md` and is not
restated here, so the two cannot drift apart.

WHAT NO COLUMN OF THIS FILE IS. Not a verdict, and not a verdict on any one
change inside the interval it covers: a fixed match gives a sum, and per-patch
attribution is what it cannot give (S199 `excludes:`, DEC-108, DEC-143).
"""

import argparse
import os
import re
import statistics
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S105_pairs  # noqa: E402  -- the path above is what makes it importable

TSV = os.path.join(HERE, 'S199_drift.tsv')

# The pin, `adocs/data/S199_drift.sh`'s `REF=`. Not enforced -- a row records
# the reference it was taken against and `--check` refuses to difference two
# rows that disagree -- but a run against anything else says so out loud,
# because the step's `excludes:` is "changing the reference once chosen".
PIN = 'f548ff4'

COLUMNS = ('date', 'head', 'ref', 'regime', 'games', 'elo', 'elo_95', 'nelo',
           'nelo_95', 'ptnml', 'pair_var', 'verdicts_since', 'conversions_since')

# The two hand-filled columns start as this, so an unread row is visible as one.
UNFILLED = 'TODO'

BANNER_CAND = re.compile(r'^candidate\s+(\S+)\s+(\d{4}-\d{2}-\d{2})(\s+\+ uncommitted changes)?\s*$', re.M)
BANNER_REF = re.compile(r'^reference\s+(\S+)\s+(\d{4}-\d{2}-\d{2})\s*$', re.M)
BANNER_TCHASH = re.compile(r'^tc (\S+)\s+hash (\S+)\s+concurrency ', re.M)
BANNER_BOOK = re.compile(r'^book\s+(\S+)\s*$', re.M)
BANNER_BOUNDS = re.compile(r'^bounds\s+(.*\S)\s*$', re.M)

RESULTS = re.compile(r'^Results of (\S+) vs (\S+) \(', re.M)
ELO = re.compile(r'^Elo: (-?[\d.]+) \+/- ([\d.]+), nElo: (-?[\d.]+) \+/- ([\d.]+)', re.M)
GAMES = re.compile(r'^Games: (\d+),', re.M)
PTNML = re.compile(r'^Ptnml\(0-2\): \[ *(\d+), *(\d+), *(\d+), *(\d+), *(\d+) *\]', re.M)

PAIR_SCORES = (0.0, 0.5, 1.0, 1.5, 2.0)


class Refused(Exception):
    """A reason this run does not become a row. Printed, never swallowed."""


def read_log(path):
    """The banner and fastchess's final results block, or Refused."""
    with open(path) as handle:
        text = handle.read()

    if 'SPRT-RUN-INVALID' in text:
        raise Refused(f'{path} holds SPRT-RUN-INVALID: the run was voided '
                      '(a crash or a disconnect), so it is not a reading')
    if 'SPRT-RUN-DONE' not in text:
        raise Refused(f'{path} has no SPRT-RUN-DONE: the run did not finish, '
                      'or this is not the detached run log')

    out = {}
    for name, pattern in (('head', BANNER_CAND), ('ref', BANNER_REF)):
        match = pattern.search(text)
        if match is None:
            raise Refused(f'no {name} line in the banner of {path}')
        out[name] = match.group(1)
    out['dirty'] = BANNER_CAND.search(text).group(3) is not None

    tchash = BANNER_TCHASH.search(text)
    book = BANNER_BOOK.search(text)
    bounds = BANNER_BOUNDS.search(text)
    if tchash is None or book is None or bounds is None:
        raise Refused(f'the banner of {path} is missing its tc/hash, book or '
                      'bounds line')
    out['regime'] = f'{tchash.group(1)}/{tchash.group(2)}/{book.group(1)}'

    # "bounds none -- fixed 1000 rounds, a calibration or drift reading, NOT a
    # verdict" is what a fixed-rounds run prints; anything else is an SPRT.
    if not bounds.group(1).startswith('none -- fixed'):
        raise Refused(f'{path} ran with bounds "{bounds.group(1)}": an SPRT\'s '
                      'early-stopped estimate is biased upward (DEC-063) and is '
                      'not a drift point')

    # fastchess prints a results block periodically, so the run's own figure is
    # the LAST one. S042's 5741-game log holds 288 of them.
    blocks = list(RESULTS.finditer(text))
    if not blocks:
        raise Refused(f'no "Results of" block in {path}')
    tail = text[blocks[-1].start():]

    elo, games, ptnml = ELO.search(tail), GAMES.search(tail), PTNML.search(tail)
    if elo is None or games is None or ptnml is None:
        raise Refused(f'the final results block of {path} is missing its Elo, '
                      'Games or Ptnml line')

    out['elo'], out['elo_95'] = elo.group(1), elo.group(2)
    out['nelo'], out['nelo_95'] = elo.group(3), elo.group(4)
    out['games'] = int(games.group(1))
    out['ptnml'] = [int(n) for n in ptnml.groups()]
    return out


def engine_names(pgn_text):
    """Every distinct engine name in the PGN, in first-seen order."""
    names = []
    for pattern in (S105_pairs.WHITE, re.compile(r'\[Black "([^"]*)"\]')):
        for match in pattern.finditer(pgn_text):
            if match.group(1) not in names:
                names.append(match.group(1))
    return names


def read_pgn(path, ref_sha):
    """The candidate's name, the pentanomial and the pair variance."""
    with open(path) as handle:
        text = handle.read()

    names = engine_names(text)
    if len(names) != 2:
        raise Refused(f'{path} names {len(names)} engines ({", ".join(names) or "none"}); '
                      'a drift match has exactly two sides')
    ref_label = f'ref-{ref_sha}'
    if ref_label not in names:
        raise Refused(f'{path} names {names[0]} and {names[1]}, neither of them '
                      f'the banner\'s reference "{ref_label}" -- the log and the '
                      'PGN are not the same run')
    candidate = names[0] if names[1] == ref_label else names[1]

    scores = []
    by_round = {}
    for game in S105_pairs.GAME.split(text):
        result = S105_pairs.RESULT.search(game)
        if result is None:
            continue
        by_round.setdefault(S105_pairs.ROUND.search(game).group(1), []).append(
            (S105_pairs.WHITE.search(game).group(1), result.group(1)))

    for pair in by_round.values():
        # A round with one game is a game whose partner is missing; S105_pairs
        # drops it rather than counting half a pair, and so does this.
        if len(pair) != 2:
            continue
        scores.append(sum(S105_pairs.POINTS[r] if w == candidate
                          else 1.0 - S105_pairs.POINTS[r] for w, r in pair))

    if not scores:
        raise Refused(f'{path} holds no complete pair')

    penta = [sum(1 for s in scores if s == value) for value in PAIR_SCORES]
    return candidate, penta, statistics.pvariance(scores), len(scores)


def read_tsv(path):
    rows = []
    if not os.path.exists(path):
        return rows
    with open(path) as handle:
        for line in handle:
            line = line.rstrip('\n')
            if not line.strip() or line.startswith('#'):
                continue
            fields = line.split('\t')
            if fields[0] == COLUMNS[0]:
                if tuple(fields) != COLUMNS:
                    raise Refused(f'{path} has an unexpected header: {fields}')
                continue
            if len(fields) != len(COLUMNS):
                raise Refused(f'{path} has a row of {len(fields)} fields, '
                              f'expected {len(COLUMNS)}: {line}')
            rows.append(dict(zip(COLUMNS, fields)))
    return rows


def append(args):
    pgn = os.path.join(args.outdir, 'games.pgn')
    for path in (args.runlog, pgn):
        if not os.path.exists(path):
            raise Refused(f'no such file: {path}')

    log = read_log(args.runlog)
    candidate, penta, variance, pairs = read_pgn(pgn, log['ref'])

    if penta != log['ptnml']:
        raise Refused(
            f'the pentanomial from {pgn} read as "{candidate}" is {penta} and '
            f'fastchess printed {log["ptnml"]} -- the two do not describe the '
            'same run, so no row is written')

    if log['ref'] != PIN:
        print(f'WARNING: the reference is {log["ref"]}, not the pinned {PIN}. '
              'S199\'s excludes is "changing the reference once chosen"; this '
              'row will not be differenced against the pinned ones.')
    if log['dirty']:
        print('WARNING: the banner reported uncommitted changes, so the '
              f'candidate sha {log["head"]} does not fully name what played.')

    row = {
        'date': args.date,
        'head': log['head'],
        'ref': log['ref'],
        'regime': log['regime'],
        'games': str(log['games']),
        'elo': log['elo'],
        'elo_95': log['elo_95'],
        'nelo': log['nelo'],
        'nelo_95': log['nelo_95'],
        'ptnml': '[' + ','.join(str(n) for n in penta) + ']',
        'pair_var': f'{variance:.4f}',
        'verdicts_since': args.verdicts,
        'conversions_since': args.conversions,
    }

    rows = read_tsv(args.tsv)
    line = '\t'.join(row[name] for name in COLUMNS)
    if args.dry_run:
        print('dry run, nothing written:')
    else:
        if not os.path.exists(args.tsv):
            raise Refused(f'no {args.tsv} to append to; it carries the header '
                          'row and is created with the step, not by a run')
        with open(args.tsv, 'a') as handle:
            handle.write(line + '\n')

    print(f'=== drift point {len(rows) + 1}, {pairs} pairs read as "{candidate}"')
    for name in COLUMNS:
        print(f'  {name:<18} {row[name]}')
    print()
    print('The two TODO columns are filled by hand, from the plan: the step ids '
          'whose\nverdicts landed since the previous point, and the ids whose '
          'conversions did\n(F30, DEC-170). The reading rule is in the step '
          'file; this script reads nothing\ninto the number.')
    return 0


def check(args):
    rows = read_tsv(args.tsv)
    print(f'=== {args.tsv}: {len(rows)} point(s)')
    if not rows:
        print('No point yet. The first is taken after the S109 block lands '
              '(DEC-139).')
        return 0

    previous = None
    for row in rows:
        print()
        print(f'{row["date"]}  {row["head"]} vs {row["ref"]}  {row["regime"]}')
        print(f'  Elo  {float(row["elo"]):+8.2f} +/- {row["elo_95"]:<6}'
              f'  nElo {float(row["nelo"]):+8.2f} +/- {row["nelo_95"]:<6}'
              f'  Ptnml {row["ptnml"]}  v {row["pair_var"]}  {row["games"]} games')
        print(f'  verdicts since: {row["verdicts_since"]}'
              f'   conversions since: {row["conversions_since"]}')

        if previous is None:
            print('  first point: no previous point. It is read against zero '
                  'and against the\n  sum of the kept verdicts since the pin '
                  '-- the step file lists them.')
        elif previous['ref'] != row['ref'] or previous['regime'] != row['regime']:
            print('  NOT DIFFERENCED against the previous point: '
                  f'{"reference" if previous["ref"] != row["ref"] else "regime"} '
                  'changed, so the two are not on one footing. The trend '
                  'restarts here.')
        else:
            delta = float(row['elo']) - float(previous['elo'])
            combined = (float(row['elo_95']) ** 2
                        + float(previous['elo_95']) ** 2) ** 0.5
            n_delta = float(row['nelo']) - float(previous['nelo'])
            n_combined = (float(row['nelo_95']) ** 2
                          + float(previous['nelo_95']) ** 2) ** 0.5
            # The plain difference only. The step file's reading rule adds
            # the verdicts and conversions landed since the previous point
            # to the band before it says "inside", "above" or "below", and
            # this reader has no numbers for those -- the TSV carries them
            # as step ids -- so the rule is applied by hand from the row.
            plain = ('within' if abs(delta) < combined else
                     ('over' if delta > 0 else 'under'))
            print(f'  delta Elo  {delta:+.2f} on a combined +/- {combined:.2f}'
                  f'  -> {plain} the plain interval; add the verdicts and '
                  f'conversions landed since ({row.get("verdicts_since", "?")}; '
                  f'{row.get("conversions_since", "?")}) before reading it '
                  f'by the step file\'s rule')
            print(f'  delta nElo {n_delta:+.2f} on a combined +/- {n_combined:.2f}')
        previous = row

    print()
    print('A move inside the band is not "no progress" and a move above it is '
          'not a\nverdict on any one step: the band is this instrument\'s '
          'resolution, about\n+/- 11.6 logistic Elo a point at 1000 pairs, so '
          'two points differ by +/- 16 or\nmore before they differ at all. The '
          'reading rule is in the step file.')
    return 0


SELF_TEST_LOG = """candidate  abc1234  2026-09-13
reference  f548ff4  2026-08-20
config     arch native  tune off
tc 8+0.08  hash 16  concurrency 12 of 12 cores
book       noob_3moves.epd
seed       20260913120000
bounds     none -- fixed 4 rounds, a calibration or drift reading, NOT a verdict
out        /tmp/selftest

id name    candidate  Chesso abc1234 native
id name    ref-f548ff4  Chesso
           ^ no build stamp: every commit before S212 answers the bare
             literal, so what this binary holds cannot be checked here.

Started game 1 of 8 (candidate vs ref-f548ff4)
--------------------------------------------------
Results of candidate vs ref-f548ff4 (8+0.08, 1t, 16MB, noob_3moves.epd):
Elo: 999.00 +/- 99.00, nElo: 999.00 +/- 99.00
LOS: 50.00 %, DrawRatio: 0.00 %, PairsRatio: 1.00
Games: 2, Wins: 1, Losses: 1, Draws: 0, Points: 1.0 (50.00 %)
Ptnml(0-2): [9, 9, 9, 9, 9], WL/DD Ratio: 0.00
--------------------------------------------------
Finished game 8 (ref-f548ff4 vs candidate): 1-0 {White wins by adjudication}
--------------------------------------------------
Results of candidate vs ref-f548ff4 (8+0.08, 1t, 16MB, noob_3moves.epd):
Elo: 42.50 +/- 11.60, nElo: 55.25 +/- 15.20
LOS: 100.00 %, DrawRatio: 25.00 %, PairsRatio: 1.00
Games: 8, Wins: 3, Losses: 3, Draws: 2, Points: 4.0 (50.00 %)
Ptnml(0-2): [1, 0, 2, 0, 1], WL/DD Ratio: 3.00
--------------------------------------------------
Finished match
Total Time: 00:00:08 (hours:minutes:seconds)

=== 8 games in /tmp/selftest/games.pgn ===
      8 [Termination "normal"]
draws     2 of 8, 25.0 %
decisive  6 of 8, 75.0 %
forfeits  0 of 8, 0.00 %
SPRT-RUN-DONE full /tmp/selftest
"""

# Four fabricated pairs, scored from `candidate`: 2.0, 1.0, 1.0, 0.0. So the
# pentanomial is [1, 0, 2, 0, 1] and the population variance of the pair score
# is 0.5000 exactly -- both known by hand before the script is run, which is
# what makes this a test and not a demonstration.
SELF_TEST_PAIRS = (
    ('1', 'candidate', '1-0'), ('1', 'ref-f548ff4', '0-1'),
    ('2', 'candidate', '1/2-1/2'), ('2', 'ref-f548ff4', '1/2-1/2'),
    ('3', 'candidate', '1-0'), ('3', 'ref-f548ff4', '1-0'),
    ('4', 'candidate', '0-1'), ('4', 'ref-f548ff4', '1-0'),
)


def self_test_pgn():
    games = []
    for index, (round_id, white, result) in enumerate(SELF_TEST_PAIRS, start=1):
        black = 'ref-f548ff4' if white == 'candidate' else 'candidate'
        games.append('\n'.join((
            '[Event "chesso selftest"]',
            '[Site "?"]',
            '[Date "2026.09.13"]',
            f'[Round "{round_id}"]',
            f'[White "{white}"]',
            f'[Black "{black}"]',
            f'[Result "{result}"]',
            '[GameDuration "00:00:01"]',
            f'[PlyCount "{30 + index}"]',
            '[Termination "normal"]',
            '[TimeControl "8+0.08"]',
            '',
            f'1. e4 e5 {result}',
            '')))
    return '\n'.join(games)


def self_test(directory):
    """Parse a fabricated run whose every number is known by hand."""
    directory = directory or tempfile.mkdtemp(prefix='S199_selftest_')
    os.makedirs(directory, exist_ok=True)
    logpath = os.path.join(directory, 'run.log')
    pgnpath = os.path.join(directory, 'games.pgn')
    tsvpath = os.path.join(directory, 'S199_drift.tsv')
    with open(logpath, 'w') as handle:
        handle.write(SELF_TEST_LOG)
    with open(pgnpath, 'w') as handle:
        handle.write(self_test_pgn())
    with open(tsvpath, 'w') as handle:
        handle.write('\t'.join(COLUMNS) + '\n')

    print(f'=== self-test in {directory}')
    failures = []

    def expect(what, got, want):
        if got != want:
            failures.append(f'{what}: got {got!r}, expected {want!r}')
        print(f'  {"ok " if got == want else "FAIL"} {what:<22} {got!r}')

    log = read_log(logpath)
    expect('head', log['head'], 'abc1234')
    expect('ref', log['ref'], 'f548ff4')
    expect('regime', log['regime'], '8+0.08/16/noob_3moves.epd')
    expect('games', log['games'], 8)
    expect('elo', (log['elo'], log['elo_95']), ('42.50', '11.60'))
    expect('nelo', (log['nelo'], log['nelo_95']), ('55.25', '15.20'))
    # The decoy block above it holds [9,9,9,9,9] and Elo 999: the last block
    # wins, which is the property a 288-block run needs.
    expect('last block wins', log['ptnml'], [1, 0, 2, 0, 1])

    candidate, penta, variance, pairs = read_pgn(pgnpath, log['ref'])
    expect('engine derived', candidate, 'candidate')
    expect('pentanomial', penta, [1, 0, 2, 0, 1])
    expect('pair variance', round(variance, 6), 0.5)
    expect('pairs', pairs, 4)

    # S105_pairs' own report over the same file, which is the printed evidence
    # a run records; its variance must be this one.
    reported, reported_pairs = S105_pairs.report(pgnpath, engine=candidate)
    expect('S105_pairs agrees', (round(reported, 6), reported_pairs), (0.5, 4))

    # The four refusals, each one observed rather than assumed.
    for what, path, ref, edit in (
            ('an SPRT log', logpath, None,
             lambda t: t.replace('bounds     none -- fixed 4 rounds, a '
                                 'calibration or drift reading, NOT a verdict',
                                 'bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05')),
            ('a voided run', logpath, None,
             lambda t: t.replace('SPRT-RUN-DONE', 'SPRT-RUN-INVALID: 1 crash')),
            ('an unfinished run', logpath, None,
             lambda t: t.replace('SPRT-RUN-DONE full /tmp/selftest', '')),
            ('a wrong reference', pgnpath, 'deadbee', None)):
        scratch = os.path.join(directory, 'refusal')
        if edit is None:
            got = _refuses(read_pgn, path, ref)
        else:
            with open(path) as handle:
                text = handle.read()
            with open(scratch, 'w') as handle:
                handle.write(edit(text))
            got = _refuses(read_log, scratch)
        expect(f'refuses {what}', got, True)

    # A two-row TSV, so --check's differencing path runs on known numbers.
    with open(tsvpath, 'a') as handle:
        handle.write('\t'.join(('2026-09-13', 'abc1234', 'f548ff4',
                                '8+0.08/16/noob_3moves.epd', '2000', '60.00',
                                '11.60', '78.00', '15.20', '[1,0,2,0,1]',
                                '0.2900', '-', '-')) + '\n')
        handle.write('\t'.join(('2026-10-01', 'def5678', 'f548ff4',
                                '8+0.08/16/noob_3moves.epd', '2000', '95.00',
                                '11.60', '124.00', '15.20', '[1,0,2,0,1]',
                                '0.2900', 'S210, S213', 'S115')) + '\n')
    args = argparse.Namespace(tsv=tsvpath)
    print()
    check(args)
    print()

    if failures:
        print(f'SELF-TEST FAILED: {len(failures)}')
        for line in failures:
            print(f'  {line}')
        return 1
    print('SELF-TEST OK')
    return 0


def _refuses(function, *arguments):
    try:
        function(*arguments)
    except Refused:
        return True
    return False


def main(argv):
    parser = argparse.ArgumentParser(
        description=__doc__.strip().splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('outdir', nargs='?', help="the run's OUT directory")
    parser.add_argument('runlog', nargs='?', help="the detached run's log")
    parser.add_argument('--tsv', default=TSV)
    parser.add_argument('--date', default=None,
                        help='the row\'s date, default today')
    parser.add_argument('--verdicts', default=UNFILLED,
                        help='step ids whose verdicts landed since the previous point')
    parser.add_argument('--conversions', default=UNFILLED,
                        help='step ids whose conversions landed since it (F30)')
    parser.add_argument('--dry-run', action='store_true')
    parser.add_argument('--check', action='store_true',
                        help='re-read the TSV and print the trend')
    parser.add_argument('--self-test', nargs='?', const='', default=None,
                        metavar='DIR',
                        help='parse a fabricated run with known numbers')
    args = parser.parse_args(argv[1:])

    try:
        if args.self_test is not None:
            return self_test(args.self_test or None)
        if args.check:
            return check(args)
        if not args.outdir or not args.runlog:
            parser.error('give <outdir> and <runlog>, or --check, or --self-test')
        if args.date is None:
            import datetime
            args.date = datetime.date.today().isoformat()
        return append(args)
    except Refused as refusal:
        print(f'REFUSED: {refusal}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main(sys.argv))
