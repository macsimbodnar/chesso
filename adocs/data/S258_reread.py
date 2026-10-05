#!/usr/bin/env python3
"""S258: ten SPRT pair readings that were scored from Black's side, re-read.

    adocs/data/S258_reread.py > adocs/data/S258_reread_pairs.txt

WHY THIS EXISTS. `S105_pairs.report` scores a pair from the side it is named,
and its command line passed `chesso-a`, S105's calibration name, to every PGN
until S258 (other readings passed the literal `candidate`). A `fastchess.sh`
PGN names `cand-<sha>` and `ref-<sha>`, so neither side matched and every game
scored as Black's points, with exit 0. Ten recorded `*_sprt_pairs.txt` readings
were taken that way. They are evidence and are not edited; this re-reads each
run's PGN under its own candidate name and puts the recorded and the corrected
reading side by side.

FOUR CHECKS PER RUN, and a run that fails any of them exits 1:

- the side `S105_pairs.side_name` derives from the PGN is the `cand-<sha>` the
  recorded file's banner names;
- the corrected pentanomial equals the last `Ptnml(0-2)` fastchess printed in
  the run's committed log, `adocs/data/<run>_sprt.log` -- the one the SPRT used;
- the recorded pentanomial equals every game of the same PGN scored as
  Black's points, which is the diagnosis, shown rather than assumed (S237's
  recorded note blamed the PGN's game order instead);
- the PGN is still on disk. The ten live under `.tuning/` on the machine that
  ran them; on any other machine this exits 1 before printing a reading.

THEN THE SCAN that found the ten, re-run over every `adocs/data/*_pairs.txt`
but this script's own output: a block whose `pair score 0.0` count equals its
`white won both` count. Under a side's name those are disjoint sets (the side
losing both, against each colour winning its White game); under Black's points
they are one set. A block that matches the signature and is not one of the ten
is an eleventh, and exits 1. Beside it, each block's pentanomial against the
last fastchess `Ptnml(0-2)` in the same file, where the file carries one: equal,
mirrored (read from the reference's side, same variance, mean reflected), or
different; a block outside the ten that is not equal exits 1 as well.
"""

import collections
import contextlib
import glob
import io
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S105_pairs  # noqa: E402  -- the path above is what makes it importable

RUNS = ('S095', 'S097_v1', 'S097_v2', 'S132', 'S132_confirm', 'S188', 'S231',
        'S236', 'S236_v2', 'S237')
OWN_OUTPUT = 'S258_reread_pairs.txt'

OUT = re.compile(r'^out\s+(\S+)$', re.MULTILINE)
CAND = re.compile(r'^id name\s+(cand-\S+)', re.MULTILINE)
WHITE_BOTH = re.compile(r'^white won both\s+(\d+) of (\d+)', re.MULTILINE)
MEAN_VAR = re.compile(r'^pair score\s+mean ([\d.]+), variance ([\d.]+)',
                      re.MULTILINE)
BLOCK = re.compile(r'^=== (.*) ===$', re.MULTILINE)
# A recorded file quotes fastchess's lines as they are, or as `grep -n` printed
# them with the log's line number in front.
QUOTED_PTNML = re.compile(r'^(?:\d+:)?Ptnml\(0-2\): '
                          r'\[(\d+), (\d+), (\d+), (\d+), (\d+)\]', re.MULTILINE)


def black_points_pentanomial(path):
    """Every complete pair of `path` scored as Black's points: what
    `S105_pairs.report` printed when handed a name that is no side, before
    S257 made it refuse. Pairs are grouped by the Round tag, as there."""
    with open(path) as handle:
        text = handle.read()
    by_round = collections.defaultdict(list)
    for game in S105_pairs.GAME.split(text):
        result = S105_pairs.RESULT.search(game)
        if result is None:
            continue
        by_round[S105_pairs.ROUND.search(game).group(1)].append(result.group(1))
    counts = collections.Counter(
        sum(1.0 - S105_pairs.POINTS[r] for r in pair)
        for pair in by_round.values() if len(pair) == 2)
    return [counts[score / 2] for score in range(5)]


def recorded(run):
    path = os.path.join(HERE, f'{run}_sprt_pairs.txt')
    with open(path) as handle:
        text = handle.read()
    mean, variance = MEAN_VAR.search(text).groups()
    return {
        'out': OUT.search(text).group(1),
        'cand': CAND.search(text).group(1),
        'penta': S105_pairs.printed_pentanomial(text),
        'white_both': int(WHITE_BOTH.search(text).group(1)),
        'mean': mean,
        'variance': variance,
    }


def reread(run):
    """One run's block of the output, and whether every check held."""
    was = recorded(run)
    pgn = os.path.join(was['out'], 'games.pgn')
    log = os.path.join(HERE, f'{run}_sprt.log')
    if not os.path.exists(pgn):
        sys.exit(f'S258_reread: {pgn} is not on this machine; the ten PGNs are '
                 'under .tuning/ where they ran, or in the owner\'s archive')

    side = S105_pairs.side_name(pgn)
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        S105_pairs.report(pgn, engine=side)
    block = out.getvalue()
    now = S105_pairs.printed_pentanomial(block)
    mean, variance = MEAN_VAR.search(block).groups()
    fastchess = S105_pairs.fastchess_pentanomial(log)
    black = black_points_pentanomial(pgn)

    checks = {
        'side is the banner\'s candidate': side == was['cand'],
        'corrected = fastchess Ptnml': now == fastchess,
        'recorded = Black\'s points': was['penta'] == black,
    }
    lines = [f'##### {run}',
             f'PGN                {pgn}',
             f'log                adocs/data/{run}_sprt.log',
             f'side read          {side}  (banner: {was["cand"]})',
             f'recorded           {was["penta"]}  mean {was["mean"]}, '
             f'variance {was["variance"]}',
             f'Black\'s points     {black}',
             f'corrected          {now}  mean {mean}, variance {variance}',
             f'fastchess Ptnml    {fastchess}',
             *(f'check              {"ok  " if held else "FAIL"} {what}'
               for what, held in checks.items()),
             '',
             '--- S105_pairs.py over the PGN, under the side above',
             block]
    row = (run, was['variance'], variance, was['penta'], now, fastchess,
           all(checks.values()))
    return '\n'.join(lines), row


def scan():
    """The signature scan over every recorded *_pairs.txt but this output."""
    lines, elevenths, against_counts = [], [], collections.Counter()
    paths = sorted(p for p in glob.glob(os.path.join(HERE, '*_pairs.txt'))
                   if os.path.basename(p) != OWN_OUTPUT)
    blocks_total = 0
    for path in paths:
        with open(path) as handle:
            text = handle.read()
        name = os.path.basename(path)
        # A file that quotes no Ptnml is checked against its sibling log, and
        # only its first block: a later one reads another PGN (a band, or
        # S105's second regime) that the sibling log does not describe.
        sibling = os.path.join(HERE, name[:-len('_pairs.txt')] + '.log')
        heads = list(BLOCK.finditer(text))
        first = True
        for index, head in enumerate(heads):
            end = heads[index + 1].start() if index + 1 < len(heads) else len(text)
            body = text[head.start():end]
            penta = S105_pairs.printed_pentanomial(body)
            both = WHITE_BOTH.search(body)
            if len(penta) != 5 or both is None:
                continue
            blocks_total += 1
            quoted = QUOTED_PTNML.findall(text[:head.start()])
            if quoted:
                fast, where = [int(n) for n in quoted[-1]], 'quoted'
            elif first and os.path.exists(sibling):
                fast = S105_pairs.fastchess_pentanomial(sibling)
                where = os.path.basename(sibling)
            else:
                fast, where = None, ('later block' if not first else 'no log')
            first = False
            against = ('-' if fast is None
                       else 'equal' if penta == fast
                       else 'mirrored' if penta == fast[::-1]
                       else 'different')
            against_counts[against] += 1
            signature = penta[0] == int(both.group(1))
            run = (name[:-len('_sprt_pairs.txt')]
                   if name.endswith('_sprt_pairs.txt') else None)
            # Outside the ten, a block off fastchess's own Ptnml is a misread
            # the signature can miss (a reference-side reading mirrors it).
            misread = signature or against in ('different', 'mirrored')
            if misread and run not in RUNS:
                elevenths.append(name)
            lines.append(f'{name:<30} {os.path.basename(head.group(1)):<26} '
                         f'0.0 {penta[0]:<5} white-won-both {both.group(1):<5} '
                         f'{"SIGNATURE" if signature else "-":<9} '
                         f'Ptnml ({where}) {against}')
    summary = (f'{len(paths)} files, {blocks_total} pair blocks; '
               f'{sum("SIGNATURE" in line for line in lines)} carry the signature, '
               f'{len(elevenths)} outside the ten'
               + (f': {", ".join(elevenths)}' if elevenths else '')
               + '; against fastchess\'s Ptnml: '
               + ', '.join(f'{n} {what}' for what, n in sorted(against_counts.items())))
    return lines, summary, not elevenths


def main(argv):
    if len(argv) != 1:
        print(__doc__.strip().splitlines()[2].strip(), file=sys.stderr)
        return 2
    blocks, rows = [], []
    for run in RUNS:
        block, row = reread(run)
        blocks.append(block)
        rows.append(row)

    print('S258: ten SPRT pair readings re-read under each run\'s own candidate')
    print('name. Written by adocs/data/S258_reread.py; see its header.')
    print()
    print('=== summary: recorded (Black\'s points) beside corrected (the candidate\'s)')
    print(f'{"run":<14} {"recorded var":>12} {"corrected var":>13}  '
          f'{"corrected pentanomial":<32} fastchess Ptnml')
    for run, was, now, _, penta, fast, held in rows:
        print(f'{run:<14} {was:>12} {now:>13}  {str(penta):<32} '
              f'{"equal" if penta == fast else "MISMATCH " + str(fast)}'
              f'{"" if held else "  (a check FAILED)"}')
    print()
    for block in blocks:
        print(block)
    lines, summary, clean = scan()
    print('=== signature scan over adocs/data/*_pairs.txt, '
          f'{OWN_OUTPUT} excluded')
    print('\n'.join(lines))
    print(summary)
    return 0 if clean and all(row[-1] for row in rows) else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv))
