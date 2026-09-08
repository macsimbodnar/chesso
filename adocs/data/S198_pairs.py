#!/usr/bin/env python3
"""S198's workstation calibration, read against S105's.

    adocs/data/S198_pairs.py <run>/games.pgn

WHY THIS EXISTS AND WHY IT IS NOT AN EDIT. `S105_pairs.py` already does the
work -- the pentanomial, the pairs an opening decided, and the pair score
variance that sets what a verdict costs at fixed bounds. This directory is
append-only, so that file is imported rather than changed, and what is added
here is the comparison S198 owes: DEC-143 makes a fixed-rounds A/A follow every
harness change, and a variance means nothing until it is read against the last
one taken.

TWO ARGUMENTS THAT ARE NOT DETAILS.

`engine='candidate'`. `S105_pairs.report` defaults to `chesso-a`, which is what
`S105_calibration.sh` named its sides; `fastchess.sh` names them `candidate`
and `ref-<sha>`. A name matching neither side is not an error -- every game
then scores as Black's points, a pair White won twice reads 0.0 instead of 1.0,
and the variance comes back inflated with nothing printed to say so. So each
PGN is read under its own run's name.

The band. `S105_calibration_after.pgn` is the same regime on the same machine,
1000 games at 8+0.08 on the UHO book, and it is a sanity band rather than a
control: the engine has moved since 2026-08-20 (S107, S108, S149, S165 and
more) and a stronger engine self-plays a different pentanomial. A difference is
attributed by shape and recorded -- it is never attributed to the seed, which
draws a different sample of the same book and cannot move a distribution.

The error bar is the normal approximation `S105_pairs.main` uses,
`v * sqrt(2/(n-1))`, and materiality is the two-sided 95 % normal quantile on
the difference of two independent variances, `|z| > 1.96`.
"""

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import S105_pairs  # noqa: E402  -- the path above is what makes it importable

BAND = os.path.join(HERE, 'S105_calibration_after.pgn')


def main(argv):
    if len(argv) != 2:
        print(__doc__.strip().splitlines()[2].strip(), file=sys.stderr)
        return 2

    v_ws, n_ws = S105_pairs.report(argv[1], engine='candidate')
    v_ref, n_ref = S105_pairs.report(BAND, engine='chesso-a')

    e_ws = v_ws * (2 / (n_ws - 1)) ** 0.5
    e_ref = v_ref * (2 / (n_ref - 1)) ** 0.5
    z = (v_ws - v_ref) / (e_ws ** 2 + e_ref ** 2) ** 0.5

    print(f'pair variance    {v_ws:.4f} +/- {e_ws:.4f}  vs  '
          f'{v_ref:.4f} +/- {e_ref:.4f}  (this run vs S105 after)')
    print(f'ratio            {v_ws / v_ref:.3f}')
    print(f'z                {z:+.2f}  '
          f'({"inside" if abs(z) < 1.96 else "OUTSIDE"} the band, |z| < 1.96)')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
