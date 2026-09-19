#!/usr/bin/env python3
"""Read the git log of an open-source repository as an SPRT ledger, 2026-09-19.

Some open-source engines commit their harness result block into the message of
every merged change -- an OpenBench-style block of `Elo | x +- y`, `SPRT |
<tc>s Threads=n Hash=nMB`, `LLR | l (a, b) [lo, hi]`, `Games | N: n W: w L: l
D: d` and `Penta | [...]` lines, one block per control. That log is a ledger of
passing tests and nothing else: failed tests live on the testing instance,
never in git. This script parses such a log and prints the aggregates the
2026-09-19 study (`2026-09-19_search_technique_study.md`) and its review
(`adocs/audit/2026-09-19_study_review.md`) quote, so every number there is
re-derived by a command and not re-read from a transcript.

    python3 adocs/data/2026-09-19_technique_ledger.py <repository> [out.json]

The argument is the path of a local checkout of the repository whose log is
being read. Pairing is positional per commit: the i-th Elo line goes with the
i-th SPRT, LLR and Games line of the same message, and a commit whose counts
disagree keeps the shorter pairing.
"""
import collections, json, re, statistics, subprocess, sys

ELO = re.compile(r'Elo\s*\|\s*(-?[\d.]+)\s*\+-\s*([\d.]+)')
TC = re.compile(r'SPRT\s*\|\s*([\d.]+\+[\d.]+)s\s*Threads=(\d+)')
LLR = re.compile(r'LLR\s*\|\s*(-?[\d.]+)\s*\(([^)]*)\)\s*\[(-?[\d.]+),\s*(-?[\d.]+)\]')
GAMES = re.compile(r'Games\s*\|\s*N:\s*(\d+)')


def parse(repository):
    log = subprocess.run(['git', '-C', repository, 'log', '--date=short',
                          '--format=%H%x01%ad%x01%s%x01%b%x02'],
                         capture_output=True, text=True, check=True).stdout
    recs = []
    for rec in log.split('\x02'):
        parts = rec.strip('\n').split('\x01')
        if len(parts) < 4:
            continue
        h, date, subject, body = parts[0], parts[1], parts[2], parts[3]
        elos, tcs, llrs, games = ELO.findall(body), TC.findall(body), LLR.findall(body), GAMES.findall(body)
        if not elos:
            continue
        runs = []
        for i in range(min(len(elos), len(games))):
            runs.append(dict(elo=float(elos[i][0]), err=float(elos[i][1]),
                             tc=tcs[i][0] if i < len(tcs) else None,
                             threads=int(tcs[i][1]) if i < len(tcs) else None,
                             lo=float(llrs[i][2]) if i < len(llrs) else None,
                             hi=float(llrs[i][3]) if i < len(llrs) else None,
                             games=int(games[i])))
        recs.append(dict(h=h[:7], date=date, subject=subject, runs=runs))
    return recs, log.count('\x02')


def report(recs, total_commits, since='2025-02'):
    r25 = [r for r in recs if r['date'] >= since]
    runs = [x for r in r25 for x in r['runs']]
    print(f'commits with a result block: {len(recs)} of {total_commits}; since {since}: {len(r25)}')
    print(f'games since {since}: {sum(x["games"] for x in runs)} over every run; '
          f'{sum(r["runs"][0]["games"] for r in r25)} first run per commit')
    print('controls:', collections.Counter(x['tc'] for x in runs).most_common(6))
    print('bounds:', collections.Counter((x['lo'], x['hi']) for x in runs).most_common(8))
    both = sum(1 for r in r25 if len({x['tc'] for x in r['runs'] if x['tc']}) >= 2)
    print(f'commits with two distinct controls: {both}')
    nr = [x for x in runs if x['hi'] is not None and x['hi'] <= 0.25 and x['lo'] < 0]
    print(f'non-regression runs: {len(nr)}, games {sum(x["games"] for x in nr)}, '
          f'mean {sum(x["games"] for x in nr) // max(1, len(nr))}')
    stc = [x for x in runs if x['tc'] == '8.0+0.08']
    print(f'\nSTC runs: {len(stc)}; games by reported Elo band')
    for lo, hi, name in [(20, 1e9, '>=20'), (10, 20, '10-20'), (5, 10, '5-10'),
                         (3, 5, '3-5'), (2, 3, '2-3'), (-1e9, 2, '<2')]:
        g = sorted(x['games'] for x in stc if lo <= x['elo'] < hi)
        if g:
            print(f'  {name:>6} runs={len(g):4d} median={int(statistics.median(g)):7d} '
                  f'p90={g[int(0.9 * (len(g) - 1))]:7d}')
    simp = sorted(x['games'] for x in stc if (x['lo'], x['hi']) == (-2.75, 0.25))
    if simp:
        print(f'STC runs at [-2.75, 0.25]: {len(simp)}, median {int(statistics.median(simp))}')
    print('\nmonth  commits  sum of first-run Elo')
    ms = collections.defaultdict(lambda: [0, 0.0])
    for r in r25:
        ms[r['date'][:7]][0] += 1
        ms[r['date'][:7]][1] += r['runs'][0]['elo']
    for m in sorted(ms):
        print(f'  {m} {ms[m][0]:4d} {ms[m][1]:8.1f}')
    spsa = [r for r in r25 if 'spsa' in r['subject'].lower()]
    print(f'\nSPSA-titled commits: {len(spsa)}; '
          f'{sum(1 for r in spsa if "smp" in r["subject"].lower())} SMP; '
          f'sum of best run per session {sum(max(x["elo"] for x in r["runs"]) for r in spsa):.1f}')
    rem = [r for r in recs if re.match(r'(revert|simplify|remove|drop)', r['subject'].lower())]
    print(f'removal-type commits with a block: {len(rem)}')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    recs, n = parse(sys.argv[1])
    report(recs, n)
    if len(sys.argv) > 2:
        json.dump(recs, open(sys.argv[2], 'w'), indent=0)
