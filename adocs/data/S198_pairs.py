#!/usr/bin/env python3
"""S198's workstation calibration, read against S105's.

    adocs/data/S198_pairs.py <run>/games.pgn
    adocs/data/S198_pairs.py --self-test

WHY THIS EXISTS AND WHY IT IS NOT AN EDIT. `S105_pairs.py` already does the
work -- the pentanomial, the pairs an opening decided, and the pair score
variance that sets what a verdict costs at fixed bounds. That file is imported
rather than copied, and what is added here is the comparison S198 owes: DEC-143
makes a fixed-rounds A/A follow every harness change, and a variance means
nothing until it is read against the last one taken.

TWO ARGUMENTS THAT ARE NOT DETAILS.

The candidate's name. `S105_pairs.report` scores a pair by which side is named
`engine`, and a name matching neither side used to be no error at all: every
game scored as Black's points and the variance came back wrong with nothing
printed. It was written here as the literal `candidate`, and S256 renamed the
candidate in every `fastchess.sh` run to `cand-<sha>` or `cand-<HEAD>+<hex>`,
so the next A/A would have read the band inflated: S219's own games renamed
`cand-5047070` read 0.3563 against their 0.2905, z +2.26, OUTSIDE (S257).

So the name is derived from the PGN, by the one rule `S024_pair_stats.py` and
`S203_mine_cases.py` already use and imported from the first of them: the side
named `candidate` (a working-tree run before S256) or starting `cand-`. Not
`S199_drift.py`'s rule, the side that is not the banner's `ref-<sha>`: that
needs the run log's banner, and this reads a PGN alone. Exactly one of the
PGN's names must match; none or more than one exits 1 with a sentence instead
of printing a variance. `S105_pairs.report` refuses a name that is not a side
of every pair as well, since S257, so its own command line is guarded too.

The band. `S219_aa_calibration.pgn` is the DEC-143 A/A on the current book,
1000 games at 8+0.08 on `noob_3moves.epd`, read under its own name
`candidate`, and it is a sanity band rather than a control: the engine moves
between A/As, and a stronger engine self-plays a different pentanomial. A
difference is attributed by shape and recorded -- it is never attributed to the
seed, which draws a different sample of the same book and cannot move a
distribution.

The error bar is the normal approximation `S105_pairs.main` uses,
`v * sqrt(2/(n-1))`, and materiality is the two-sided 95 % normal quantile on
the difference of two independent variances, `|z| > 1.96`.

`--self-test` runs the derivation and both refusals over fabricated PGNs whose
pair variance is known by hand, reads the band under its own name and renamed
`cand-<sha>+<hex>`, and is in the ctest fast label as `test_s198_pairs`.
"""

import contextlib
import io
import os
import re
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S024_pair_stats  # noqa: E402  -- the path above is what makes it importable
import S105_pairs  # noqa: E402

# 2026-09-13, S212: the band is the current book's. DEC-190 reset it to S219's
# A/A on noob_3moves.epd (0.2905 +/- 0.0184); S105's 0.2395 was the UHO book's
# regime and reads every run on this book as 'outside' by construction.
BAND = os.path.join(HERE, 'S219_aa_calibration.pgn')

BLACK = re.compile(r'\[Black "([^"]*)"\]')


def candidate_name(path):
    """The one engine name in the PGN that `S024_pair_stats.is_candidate`
    accepts, or exit 1 with a sentence naming what the PGN holds."""
    with open(path) as handle:
        text = handle.read()
    names = sorted({match.group(1)
                    for pattern in (S105_pairs.WHITE, BLACK)
                    for match in pattern.finditer(text)})
    matched = [name for name in names if S024_pair_stats.is_candidate(name)]
    if len(matched) != 1:
        sys.exit(f'S198_pairs: {path} names {", ".join(names) or "no engine"}, '
                 f'and {len(matched)} of them are `candidate` or `cand-...` '
                 'where the band check needs exactly one, so it reads nothing '
                 'rather than a wrong variance')
    return matched[0]


def main(argv):
    if len(argv) != 2:
        print('\n'.join(line.strip() for line in
                        __doc__.strip().splitlines()[2:4]), file=sys.stderr)
        return 2
    if argv[1] == '--self-test':
        return self_test()

    v_ws, n_ws = S105_pairs.report(argv[1], engine=candidate_name(argv[1]))
    v_ref, n_ref = S105_pairs.report(BAND, engine=candidate_name(BAND))

    e_ws = v_ws * (2 / (n_ws - 1)) ** 0.5
    e_ref = v_ref * (2 / (n_ref - 1)) ** 0.5
    z = (v_ws - v_ref) / (e_ws ** 2 + e_ref ** 2) ** 0.5

    print(f'pair variance    {v_ws:.4f} +/- {e_ws:.4f}  vs  '
          f'{v_ref:.4f} +/- {e_ref:.4f}  (this run vs S219 A/A, the current book, DEC-190)')
    print(f'ratio            {v_ws / v_ref:.3f}')
    print(f'z                {z:+.2f}  '
          f'({"inside" if abs(z) < 1.96 else "OUTSIDE"} the band, |z| < 1.96)')
    return 0


# Four fabricated pairs, scored from the candidate: 2.0, 1.0, 1.0, 0.0. The
# population variance of the pair score is 0.5000 exactly, known by hand before
# the script runs. Each entry is (round, the candidate's colour, result).
SELF_TEST_GAMES = (
    ('1', 'white', '1-0'), ('1', 'black', '0-1'),
    ('2', 'white', '1/2-1/2'), ('2', 'black', '1/2-1/2'),
    ('3', 'white', '1-0'), ('3', 'black', '1-0'),
    ('4', 'white', '0-1'), ('4', 'black', '1-0'),
)


def self_test_pgn(candidate, reference):
    games = []
    for index, (round_id, colour, result) in enumerate(SELF_TEST_GAMES, start=1):
        white, black = ((candidate, reference) if colour == 'white'
                        else (reference, candidate))
        games.append('\n'.join((
            '[Event "chesso selftest"]',
            f'[Round "{round_id}"]',
            f'[White "{white}"]',
            f'[Black "{black}"]',
            f'[Result "{result}"]',
            '[GameDuration "00:00:01"]',
            f'[PlyCount "{30 + index}"]',
            '',
            f'1. e4 e5 {result}',
            '')))
    return '\n'.join(games)


def self_test():
    failures = []

    def expect(what, got, want):
        if got != want:
            failures.append(f'{what}: got {got!r}, expected {want!r}')
        print(f'  {"ok " if got == want else "FAIL"} {what:<44} {got!r}')

    def quiet(function, *arguments, **keywords):
        with contextlib.redirect_stdout(io.StringIO()):
            return function(*arguments, **keywords)

    def refusal(function, *arguments, **keywords):
        """The exit message, or None when the call did not refuse."""
        try:
            quiet(function, *arguments, **keywords)
        except SystemExit as refused:
            return refused.code if isinstance(refused.code, str) else None
        return None

    def band_line(path):
        """main()'s `pair variance` line over `path`, or how it refused."""
        out = io.StringIO()
        try:
            with contextlib.redirect_stdout(out):
                code = main(['S198_pairs.py', path])
        except SystemExit as refused:
            return f'refused: {refused.code}'
        lines = [line for line in out.getvalue().splitlines()
                 if line.startswith('pair variance')]
        return lines[0] if code == 0 and len(lines) == 1 else f'exit {code}'

    print('=== S198_pairs self-test')
    with tempfile.TemporaryDirectory(prefix='S198_selftest_') as directory:
        def fixture(name, text):
            path = os.path.join(directory, name)
            with open(path, 'w') as handle:
                handle.write(text)
            return path

        # The three names fastchess.sh has given the candidate: the bare word
        # before S256, a commit or a clean tree, a dirty tree. Read through
        # main(), so the call the band check makes is the one under test; the
        # error bar 0.4082 is 0.5 * sqrt(2/3), four pairs, also by hand.
        for candidate in ('candidate', 'cand-1a2b3c4',
                          'cand-1a2b3c4+0123456789ab'):
            path = fixture('named.pgn', self_test_pgn(candidate, 'ref-1a2b3c4'))
            expect(f'derived {candidate}', candidate_name(path), candidate)
            expect(f'main as {candidate}', band_line(path).split('  (')[0],
                   'pair variance    0.5000 +/- 0.4082  vs  0.2905 +/- 0.0184')

        # The defect itself: the literal `candidate` this script passed until
        # S257, over a PGN fastchess.sh writes since S256. It printed a wrong
        # variance; S105_pairs.report now refuses it.
        path = fixture('post_s256.pgn',
                       self_test_pgn('cand-1a2b3c4', 'ref-1a2b3c4'))
        got = refusal(S105_pairs.report, path, engine='candidate')
        expect('report refuses `candidate` on cand-', got is not None, True)
        print(f'       {got}')

        for what, candidate, reference in (
                ('neither side', 'chesso-x', 'chesso-y'),
                ('both sides', 'cand-1a2b3c4', 'cand-5d6e7f8')):
            path = fixture('refused.pgn', self_test_pgn(candidate, reference))
            got = band_line(path)
            expect(f'main refuses {what}',
                   got.startswith('refused: S198_pairs: '), True)
            print(f'       {got}')

        # The band under its own name, and the same 1000 games renamed the way
        # a dirty-tree run since S256 is named: the accepts' fixture. Golden
        # numbers 0.2905 +/- 0.0184: S219's A/A as recorded in
        # S219_aa_calibration_pairs.txt (DEC-190), re-derived by this script
        # over S219_aa_calibration.pgn.
        expect('band derived', candidate_name(BAND), 'candidate')
        expect('main over the band', band_line(BAND).split('  (')[0],
               'pair variance    0.2905 +/- 0.0184  vs  0.2905 +/- 0.0184')
        with open(BAND) as handle:
            renamed = handle.read().replace('[White "candidate"]',
                                            '[White "cand-5047070+0123456789ab"]')
            renamed = renamed.replace('[Black "candidate"]',
                                      '[Black "cand-5047070+0123456789ab"]')
        path = fixture('band_renamed.pgn', renamed)
        expect('renamed band derived', candidate_name(path),
               'cand-5047070+0123456789ab')
        expect('main over the renamed band', band_line(path).split('  (')[0],
               'pair variance    0.2905 +/- 0.0184  vs  0.2905 +/- 0.0184')

    if failures:
        print(f'SELF-TEST FAILED: {len(failures)}')
        for line in failures:
            print(f'  {line}')
        return 1
    print('SELF-TEST OK')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
