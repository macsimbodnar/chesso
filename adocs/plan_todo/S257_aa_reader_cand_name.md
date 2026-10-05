id:         S257
goal:       the DEC-143 A/A band check reads a `fastchess.sh` PGN under the candidate name the run actually used, and refuses one where no side matches
accepts:    `adocs/data/S198_pairs.py <run>/games.pgn` reads a PGN whose candidate is `cand-<sha>` or `cand-<sha>+<hex>` (S256's names) with the same pair variance it reads for the same games named `candidate`, shown on a fixture by renaming the sides of an existing PGN; the band file `S219_aa_calibration.pgn` still reads 0.2905 under its own name; a PGN in which the requested or derived engine names neither side, or both, exits non-zero with a sentence instead of printing an inflated variance (observed red first on the unfixed reader); the derivation rule is the one `S024_pair_stats.py` and `S203_mine_cases.py` already use (`candidate` or the `cand-` prefix), or `S199_drift.py`'s (the side that is not the banner's `ref-`), chosen and stated
touches:    adocs/data/S198_pairs.py; adocs/data/S105_pairs.py only if the refusal belongs there and the step states why an edit beats an import; adocs/data/README.md; DEV_MANUAL.md
excludes:   any engine, harness or `fastchess.sh` change; re-reading any recorded A/A
decisions:  DEC-143, DEC-171, DEC-254
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-10-05, the coordinator, from the verification of 2026-10-04/05's work)

S256 renamed the candidate in every `fastchess.sh` run: `cand-<HEAD>` on a
clean tree, `cand-<HEAD>+<hex>` on a dirty one, where it used to be the bare
`candidate`. It updated the two readers it found (`S024_pair_stats.py`,
`S203_mine_cases.py`) and missed a third: `adocs/data/S198_pairs.py` calls
`S105_pairs.report(argv[1], engine='candidate')`. That is the command
DEV_MANUAL.md gives for reading the DEC-143 A/A owed after every harness change
(`ROUNDS=500 AA=1 ./fastchess.sh` then `S198_pairs.py <run>/games.pgn`). On a
post-S256 PGN no side is named `candidate`, so every game scores as Black's
points and the pair variance comes back inflated with nothing printed. The
header of `S198_pairs.py` describes exactly that failure. The next harness
change's A/A would be read wrong, and the next verdict's cost budgeted from it.

A tooling defect reaching no play and no reported score: a filler under
DEC-171, named in the pre-registration of any run taken while it is open. It
binds before the next A/A is read, whichever comes first.

`adocs/data/README.md` calls the directory append-only "in practice". S255 and
S256 edited the scripts there that run, as distinct from evidence of runs
taken, so editing `S198_pairs.py` follows their precedent. A new reader beside
it is the alternative, and the step chooses which and says why.
