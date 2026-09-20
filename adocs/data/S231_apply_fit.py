#!/usr/bin/env python3
"""Write S231's fitted vector into src/search_params.hpp's six X-macro rows.

    python3 adocs/data/S231_apply_fit.py [--check]

The values are read from the driver's own rounded JSON block at the end of
`adocs/data/S231_spsa.log` and never retyped: `spsa_driver.py`'s
`final_vector` rounds at the UCI boundary, which is where the run itself sent
every value, so the integers below are the ones 60000 games were played with
and not a hand-rounded theta. S222 landed its eleven the same way.

Each row is rewritten by UCI name, in place, with every column width preserved
-- the file's X-macro table is column aligned and its continuation backslashes
sit in a fixed column, so a substitution that changed a field's width would
move them and `clang-format` would see a different table. All six fitted
values are two digits, as the six they replace were, so no width moves.

`--check` reports what would change and writes nothing (exit 1 if any row is
not already at its fitted value).
"""

import json
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
LOG = ROOT / 'adocs/data/S231_spsa.log'
HPP = ROOT / 'src/search_params.hpp'

# The six axes this lane fitted, in the config's own order.
AXES = ['ContHistBonus', 'ContHistMalus', 'ContHistWeight',
        'ContHist2Bonus', 'ContHist2Malus', 'ContHist2Weight']


def fitted_vector() -> dict:
    """The driver's rounded JSON block, the last one in the run log."""
    text = LOG.read_text()
    blocks = re.findall(r'^\{$.*?^\}$', text, re.M | re.S)
    if not blocks:
        sys.exit(f'no JSON vector in {LOG}')
    vec = json.loads(blocks[-1])
    missing = [a for a in AXES if a not in vec]
    if missing:
        sys.exit(f'{LOG} names no value for {missing}')
    extra = [k for k in vec if k not in AXES]
    if extra:
        sys.exit(f'{LOG} carries axes this script does not know: {extra}')
    return vec


def main() -> int:
    check = '--check' in sys.argv[1:]
    vec = fitted_vector()
    text = HPP.read_text()
    stale = []
    for name in AXES:
        want = vec[name]
        # X(SYMBOL, "UciName", default, min, max) -- only the default moves.
        pat = re.compile(
            r'(X\(\s*[A-Z_0-9]+,\s*"' + re.escape(name) + r'",\s*)(\d+)(,)')
        m = pat.search(text)
        if not m:
            sys.exit(f'{HPP} has no X-macro row for {name}')
        have = int(m.group(2))
        if have == want:
            print(f'{name:16s} {have:5d}  already fitted')
            continue
        stale.append(name)
        if len(str(want)) != len(m.group(2)):
            sys.exit(f'{name}: {have} -> {want} changes the column width; '
                     'the table is aligned and this script will not move it')
        print(f'{name:16s} {have:5d} -> {want}')
        text = text[:m.start(2)] + str(want) + text[m.end(2):]
    if check:
        return 1 if stale else 0
    if stale:
        HPP.write_text(text)
    print(f'{len(stale)} of {len(AXES)} rows written to {HPP.relative_to(ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
