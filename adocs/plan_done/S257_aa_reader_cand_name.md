id:         S257
goal:       the DEC-143 A/A band check reads a `fastchess.sh` PGN under the candidate name the run actually used, and refuses one where no side matches
accepts:    `adocs/data/S198_pairs.py <run>/games.pgn` reads a PGN whose candidate is `cand-<sha>` or `cand-<sha>+<hex>` (S256's names) with the same pair variance it reads for the same games named `candidate`, shown on a fixture by renaming the sides of an existing PGN; the band file `S219_aa_calibration.pgn` still reads 0.2905 under its own name; a PGN in which the requested or derived engine names neither side, or both, exits non-zero with a sentence instead of printing an inflated variance (observed red first on the unfixed reader); the derivation rule is the one `S024_pair_stats.py` and `S203_mine_cases.py` already use (`candidate` or the `cand-` prefix), or `S199_drift.py`'s (the side that is not the banner's `ref-`), chosen and stated
touches:    adocs/data/S198_pairs.py; adocs/data/S105_pairs.py only if the refusal belongs there and the step states why an edit beats an import; adocs/data/README.md; DEV_MANUAL.md; tests/CMakeLists.txt (widened by the implementer: the reader's `--self-test` registered as `test_s198_pairs`)
excludes:   any engine, harness or `fastchess.sh` change; re-reading any recorded A/A
decisions:  DEC-143, DEC-171, DEC-254
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-05. `S198_pairs.py` derives the candidate by `S024_pair_stats.is_candidate` (imported) and `S105_pairs.report` refuses a name that is not a side of every pair. Red, unfixed: S219's band renamed `cand-5047070` or `cand-5047070+0123456789ab` read 0.3563 +/- 0.0226, z +2.26 OUTSIDE, exit 0, as did `chesso-x`/`chesso-y` and two `cand-` sides. Green: both renamed fixtures read 0.2905 +/- 0.0184, z +0.00, as the band does under `candidate`; neither and both exit 1 with a sentence. Every name-matching caller byte-identical by `cmp`. `--self-test` 13 cases as `test_s198_pairs` (fast), four mutants killed. Gate green both builds (42/42 each) and clang-format. DEV_MANUAL.md and README rows updated, MANUAL.md checked, no change. Found, not fixed: ten SPRT `_sprt_pairs.txt` readings scored as Black's points (S231: 0.3105 recorded, 0.3025 under its own name). touches widened to tests/CMakeLists.txt.

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

## As built (2026-10-05, the implementer)

**Observed red, on the unfixed reader.** Fixtures in the scratch area: the
1000 games of `adocs/data/S219_aa_calibration.pgn` with `[White|Black
"candidate"]` renamed `cand-5047070`, renamed `cand-5047070+0123456789ab`,
both sides renamed `chesso-x`/`chesso-y`, and both renamed `cand-aaaaaaa`/
`cand-bbbbbbb`. `python3 adocs/data/S198_pairs.py <fixture>` printed, for
**all four**, exit 0 and `pair variance 0.3563 +/- 0.0226 vs 0.2905 +/-
0.0184`, ratio 1.226, `z +2.26 (OUTSIDE the band)`, pair score mean 0.9850:
the band's own games, read under a name that is no side, falsely outside
their own band. The original-name band read 0.2905 +/- 0.0184, z +0.00.

**The rule: `S024_pair_stats.is_candidate`, imported, not copied.** The side
named `candidate` or starting `cand-`. Chosen over `S199_drift.py`'s "the side
that is not the banner's `ref-<sha>`" because that rule needs the run log's
banner and `S198_pairs.py` reads a PGN alone; and because it is the rule two
readers already share, so three now run one definition. `candidate_name()`
collects every White and Black name in the PGN; exactly one must match, and
none or more than one exits 1 with one sentence naming what the PGN holds.

**The refusal is in `S105_pairs.report` too, and why an edit beats the
import.** The silent mis-score lives in `report()`, and `S105_pairs.py`'s own
command line, which passes no name, has been run directly over
`fastchess.sh` PGNs. A check in `S198_pairs.py` alone leaves that path silent.
`report()` now exits 1 when the name it is given is not White in exactly one
game of every complete pair (the colours reverse within a pair). Every caller
that passes a side's name is byte-identical, checked with `cmp` against
captures taken before the edit: `S105_pairs.py` over S105's two PGNs (which
also matches `S105_calibration_pairs.txt` up to the path), `S198_pairs.py`
over `S219_aa_calibration.pgn` and over `S198_calibration.pgn`, and
`S199_drift.py --self-test`. The run half of `S198_pairs.py` over the band is
identical to `S219_aa_calibration_pairs.txt`'s; its second half differs only
because the band moved from S105 to S219 at S212 (DEC-190). The four committed
PGNs it reads have 0 pairs out of 500 each in which the name is White twice
or never. Behaviour change: `S105_pairs.py <fastchess.sh PGN>` now refuses
instead of printing.

**Found while doing it, not fixed here (evidence is never edited): ten
recorded SPRT readings carry Black's-points pair statistics.** In
`S095`, `S097_v1`, `S097_v2`, `S132`, `S132_confirm`, `S188`, `S231`, `S236`,
`S236_v2` and `S237` `_sprt_pairs.txt`, the `pair score 0.0` count equals the
`white won both` count (624 = 624, 580 = 580, ...). Under correct scoring
those are disjoint sets (the candidate losing both, against each side winning
its White game, a pair score of 1.0); under Black's points they are the same
set. All ten PGNs name `cand-<sha>`/`ref-<sha>`. Shown directly for S231,
whose PGN is still in `.tuning/sprt_s231_20260920_044538/`: under its own name
`cand-55891bb` it reads `[556, 1464, 2097, 1352, 566]`, mean 0.9924, variance
**0.3025**, which is fastchess's own printed `Ptnml(0-2)`; the recorded block
reads `[580, 1438, 2049, 1378, 590]`, mean 0.9967, variance **0.3105**. The
verdicts are fastchess's and unaffected; the variances are quoted in
`adocs/data/README.md`'s rows and in `status.md`. The other 24 readings with
both counts differ, so they were read under a side's name.

**Green.** `S198_pairs.py` over the two renamed fixtures: exit 0, `pair
variance 0.2905 +/- 0.0184 vs 0.2905 +/- 0.0184`, ratio 1.000, `z +0.00`,
pair score mean 0.9970, identical to the original-name band. Over the
`chesso-x`/`chesso-y` fixture: exit 1, `S198_pairs: <path> names chesso-x,
chesso-y, and 0 of them are `candidate` or `cand-...` where the band check
needs exactly one, so it reads nothing rather than a wrong variance`; over the
both-`cand-` fixture the same with 2. `S105_pairs.py` over the
`cand-5047070` fixture: exit 1, `S105_pairs: 'chesso-a' is White in 0 of the
2 games of round 5 ...`.

**Test.** `S198_pairs.py --self-test`, 13 cases, 0.2 s, stdlib only,
registered in `tests/CMakeLists.txt` as `test_s198_pairs` in the fast label
(test #37 in both builds). It drives `main()` over fabricated four-pair PGNs
whose variance 0.5000 and error bar 0.4082 are known by hand, under
`candidate`, `cand-1a2b3c4` and `cand-1a2b3c4+0123456789ab`; asserts both
refusals through `main()` and `report()`'s refusal of the literal
`candidate` on a `cand-` PGN; and reads the band under its own name and
renamed `cand-5047070+0123456789ab`, both at the golden 0.2905 +/- 0.0184
(S219_aa_calibration_pairs.txt, re-derived by this script). Four mutants run
against it, all killed: `main()` back to the literal `candidate` (6 fails);
that plus the original `S105_pairs.py`, i.e. the pre-S257 code (7 fails,
`0.1875 +/- 0.1531` printed); the both-sides refusal dropped (2); the
`report()` guard dropped (2). Wired into ctest rather than left like
`S199_drift.py`'s self-test because the A/A it guards is rare and is read long
after the rename that breaks it.

**Also corrected, in the file being edited:** `S198_pairs.py`'s header named
`S105_calibration_after.pgn` as the band, stale since S212 moved `BAND` to
`S219_aa_calibration.pgn`.

**Docs.** `adocs/data/README.md`'s `S198_pairs.py` and `S105_pairs.py` rows;
`DEV_MANUAL.md`'s CAND bullet (which said `S105_pairs.py` inflates the
variance silently) and a paragraph after the DEC-143 A/A commands stating the
rule and the refusal, every statement traced to `candidate_name()`,
`report()` and `S105_pairs.main`. `MANUAL.md` checked: it names neither
reader nor the candidate's PGN name, no change.
