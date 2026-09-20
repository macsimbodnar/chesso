#!/usr/bin/env python3
"""Read the built tune binary's own `uci` reply back and compare every search
parameter against `src/search_params.hpp`'s X-macro table -- and the six axes
S231's lane fitted against the driver's rounded JSON as well.

    python3 adocs/data/S231_verify_fit.py [build-tune/src/chesso]

This is the cross-check DEC-142 asks of a golden that has a derivation: the
header is the derivation, the binary is what ships, and a disagreement between
them is a defect in either the edit or the plumbing. It prints one line per
disagreement and nothing when they agree; exit 1 on any disagreement.

It then reads `tests/test_search_params.cpp`'s golden table and compares it to
the binary's tune options row for row -- name, default, min and max, in order
-- which is the mechanical cross-check beside that golden's own stated
re-derivation (its site says `src/search_params.hpp` is the derivation and a
diff of the two is the re-derivation, and that rule is followed as well).
"""

import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
HPP = ROOT / 'src/search_params.hpp'
GOLD = ROOT / 'tests/test_search_params.cpp'
LOG = ROOT / 'adocs/data/S231_spsa.log'
FITTED = ['ContHistBonus', 'ContHistMalus', 'ContHistWeight',
          'ContHist2Bonus', 'ContHist2Malus', 'ContHist2Weight']

OPTION = re.compile(
    r'^option name (\S+) type spin default (-?\d+) min (-?\d+) max (-?\d+)$')
XROW = re.compile(
    r'X\(\s*[A-Z_0-9]+,\s*"([A-Za-z0-9]+)",\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\)')
GOLDEN = re.compile(
    r'\{"([A-Za-z0-9]+)",\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\},')


def from_binary(engine):
    out = subprocess.run([engine], input='uci\nquit\n', text=True,
                         capture_output=True, timeout=60).stdout
    rows = []
    for line in out.splitlines():
        m = OPTION.match(line.strip())
        if m:
            rows.append((m.group(1), int(m.group(2)),
                         int(m.group(3)), int(m.group(4))))
    return rows


def from_header():
    return [(m.group(1), int(m.group(2)), int(m.group(3)), int(m.group(4)))
            for m in XROW.finditer(HPP.read_text())]


def main():
    engine = sys.argv[1] if len(sys.argv) > 1 else str(ROOT / 'build-tune/src/chesso')
    binary = from_binary(engine)
    header = from_header()
    by_name = {r[0]: r for r in binary}

    bad = 0
    for name, default, lo, hi in header:
        if name not in by_name:
            print(f'{name}: in the header, not in the binary')
            bad += 1
            continue
        if by_name[name] != (name, default, lo, hi):
            print(f'{name}: header {default}/{lo}/{hi} '
                  f'binary {by_name[name][1]}/{by_name[name][2]}/{by_name[name][3]}')
            bad += 1
    print(f'{len(header)} header rows, {len(binary)} binary spin options, '
          f'{bad} disagreements')

    blocks = re.findall(r'^\{$.*?^\}$', LOG.read_text(), re.M | re.S)
    vec = json.loads(blocks[-1])
    for name in FITTED:
        got = by_name.get(name, (name, None))[1]
        ok = got == vec[name]
        print(f'  {name:16s} lane {vec[name]:4d}  binary {got:4d}  '
              f'{"ok" if ok else "MISMATCH"}')
        bad += 0 if ok else 1

    golden = [(m.group(1), int(m.group(2)), int(m.group(3)), int(m.group(4)))
              for m in GOLDEN.finditer(GOLD.read_text())]
    spin = [r for r in binary if r[0] not in ('Hash', 'Threads')]
    if len(golden) != len(spin):
        print(f'golden has {len(golden)} rows, the binary {len(spin)} '
              'tune options')
        bad += 1
    for i, (g, b) in enumerate(zip(golden, spin)):
        if g != b:
            print(f'row {i}: golden {g} binary {b}')
            bad += 1
    print(f'{len(golden)} golden rows compared to the binary row for row, '
          f'name, default, min and max, in order: '
          f'{"all equal" if not bad else f"{bad} disagreements"}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
