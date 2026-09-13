id:         S199
goal:       a fixed-rounds match against a pinned early-S105 reference is played at each block boundary and read as a trend, so the sum of the kept verdicts is measured and not assumed
accepts:    the reference commit is chosen and recorded here -- the first commit after S105 landed the harness regime -- and built by `fastchess.sh`'s worktree machinery; `adocs/data/S199_drift.sh` plays 1000 pairs at the `fastchess.sh` regime, fixed rounds, `-repeat`, detached with a terminal marker, and `adocs/data/S199_drift.py` appends Elo, nElo, the pentanomial and the 95 % interval to `adocs/data/S199_drift.tsv`; the reading rule is written in this file before the first run: a point inside the previous point's interval plus the verdicts landed since is "as expected", a point below it names those verdicts as the suspects for S183's discount, and no point is a verdict on any one of them; the first point is taken after the S109 block lands, on the workstation; `adocs/plan.md`'s Elo paragraph cites the file as its measured check; `DEV_MANUAL.md` "Play games" documents the instrument beside `rating.sh` and says what it is not
touches:    adocs/data/, adocs/plan.md, DEV_MANUAL.md
excludes:   the gauntlet and S152 (DEC-108 stands); any per-patch attribution, which a fixed match cannot give; changing the reference once chosen
decisions:  DEC-139, DEC-108
closes:     2026-09-10_adversarial-F30
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator for the instrument (DEC-185, DEC-199); the matches are the coordinator's; started 2026-09-13 09:05 beside S109's SPRT, scripts and documents only
done:       2026-09-13 14:00. **The S109 block boundary produced its two numbers, both as expected.** **Drift point 1** (`adocs/data/S199_drift.sh`, 09:04 to 09:59): the engine at `eff4b9b` against the pinned `f548ff4` -- the commit right after the S105 regime landed, chosen by the accepts' rule with three git checks and never to change -- 1000 pairs at `8+0.08`, `Hash=16`, `noob_3moves.epd`, seed `20260913090435`: **`Elo 98.82 +/- 13.18`, `nElo 120.53 +/- 15.23`, `Ptnml [49, 127, 285, 299, 240]`**, 2000 games in 54 m 47 s (2190 an hour), 0 forfeits, pair variance 0.3188, appended to `adocs/data/S199_drift.tsv` by `S199_drift.py` with the verdicts landed since the pin (S085, S093 v1, S107, S108, S130, S165, S207, S042, S109) and no conversions. Read under the rule written above before the run: the kept verdicts' logistic point estimates sum to +106.88 and the point's interval [+85.6, +112.0] contains it -- **as expected**: the kept verdicts are in the engine, DEC-063's upward bias on the sum is small here, and no verdict on any one of them follows. **The longer-control reading of the block** (`adocs/data/S109_ltc.sh`, DEC-202, 10:01 to 13:46): `600f448` against `50e3661` at `32+0.32`, `Hash=64`, 1000 pairs, seed `20260913100109`: **`Elo 30.65 +/- 12.11`, `nElo 38.75 +/- 15.23`, `Ptnml [80, 187, 333, 277, 123]`**, 2000 games in 3 h 44 m 52 s (534 an hour against S151's 545), 0 forfeits, 0 `Incomplete mating PV` from either side, pair variance 0.3113. Read against the `8+0.08` SPRT's `46.90 +/- 15.43`: a difference of 16.25 on a combined error of 19.62, z 0.83 -- agreement within noise; the interval excludes zero by a wide margin, so the block's gain transfers to four times the control, compressed as the published record says (DEC-202's reading, an estimate and never a verdict; nothing in the engine changes). Abort rules never approached. The instrument (scripts, reader with its self-test, TSV, the reading rule with F30's conversions term) is the "Instrument landed" section; `DEV_MANUAL.md` has "The drift instrument, and what it is not" and `plan.md`'s Elo paragraph cites the TSV. Evidence `adocs/data/S199_drift_point1.log`, `S109_ltc.log`, `S109_ltc_pairs.txt`. The next point is at the next block boundary. Instrument by an Opus 5 subagent; runs, readings and stamp by the coordinator (DEC-185, DEC-199).

## Why this exists

R14 of `adocs/testing_strategy.md`. A one-stage `{0,5}` SPRT passes a true
zero one run in twenty, and "the SPRT Elo estimates are only unbiased if one
takes all patches into account, both passed and non-passed ones" (fishtest
FAQ); with about 45 pending verdicts the sum of the kept point estimates
drifts up by construction, which is the mechanism S183 is discounting for.
fishtest's answer is the regression test -- a fixed match against a pinned
reference, repeated, read as a trend. Here that is 1000 pairs, about 52
minutes at the MacBook's 2337 games/h and less on the workstation, with about
a +/- 10 Elo interval at S105's pair variance: enough to see whether the kept
verdicts are in the engine, and not the gauntlet DEC-108 deferred.

## Cost

A script and a reading rule, an hour; about 52 minutes of machine per point,
one point per block boundary.

## Amended 2026-09-11, DEC-170: the reading rule gains a term for timing conversions (F30)

`2026-09-10_adversarial-F30`: the reading rule above prices a point as "inside
the previous point's interval plus the verdicts landed since", and block 2 is
eight speed steps that land no verdict at all -- discharged by an interleaved
timing converted at DEC-083's 1.43 or 2.10 Elo per percent of nps, named as a
conversion. A drift point taken after block 2 would read high against a rule
with no slot for them. The rule therefore counts, beside the verdicts, the
conversions landed since, at their published rate and marked as conversions,
so the expected band has a term for speed; the point then says whether the
conversions were worth what the rate claims, which is a measurement this
project has never had. DEC-172 also places the S151-form longer-control
reading -- one fixed 1000-pair match at `32+0.32` -- beside each drift point,
so a boundary produces two numbers: drift at the regime, and transfer to four
times the control.

## Instrument landed 2026-09-13 08:55

Scripts, a reader and the reading rule. **No game has been played for this
step**: S109's SPRT held the machine for all but the last minutes of the work,
so nothing here was built, run or timed, and the only things executed were
`bash -n`, `git`, `tools/plan_prose_check.py` and the reader's own self-test on
a fabricated input. The first point is the coordinator's, after the S109 block
lands (DEC-139) -- which it did at 08:49 the same morning -- and it is what
closes this step, so `done:` is deliberately empty.

### The pinned reference: `f548ff4`, and it is pinned for good

    f548ff4  2026-08-20  Refuse a reused OUT, and date the 2559 rating to its hash (S105)

The `accepts:` names "the first commit after S105 landed the harness regime".
The regime landed in `21c1949` (2026-08-20, "Move the SPRT harness to the
surveyed engines' regime"), and `f548ff4` is its child. Three checks, run at
HEAD `690f7db`:

    git rev-parse --short f548ff4^     -> 21c1949
    git diff 21c1949 f548ff4 -- src/   -> empty
    git log --oneline --reverse 21c1949~1..HEAD | head -3
      21c1949  Move the SPRT harness to the surveyed engines' regime (S105)
      f548ff4  Refuse a reused OUT, and date the 2559 rating to its hash (S105)
      ec4d1dd  Sweep the transposition bound signs and mate round trip (S106)

The two readings of the rule agree on one sha, which is why it is this one:
`f548ff4` is literally the first commit after the landing, **and** its `src/`
is the engine as it stood at the landing -- it changes `fastchess.sh` and
`DEV_MANUAL.md` and nothing else, and `ec4d1dd` (S106) is the first commit
after the regime that touches `src/` at all. Changing it later is what the
`excludes:` forbids.

**That it still builds here was checked by reading, not by building**, the
machine being busy. `git diff --stat f548ff4 3488506 -- src/` is 5 files and
166 insertions, and `3488506` (2026-08-20) is S151's reference, built fresh by
`build_ref` and played for 3 h 40 m under gcc 13.3 on 2026-09-12; the only
include the range adds is `<charconv>`, and it is added in the *newer* commit.
`git show f548ff4:src/CMakeLists.txt` has `add_executable(chesso ...)`, the
target `build_ref` builds; `git show f548ff4:CMakeLists.txt` already includes
`cmake/arch.cmake`, so S104 (`883c255`) predates the pin and the reference is
built `CHESSO_ARCH=native` with its hardware popcount exactly as the candidate
is -- **S104's speed is inside the pin and therefore on both sides of every
drift point**. S167's dead `MAX` constant fails `-Werror` under Apple clang
only and gcc never warned on it. The pin answers the bare `id name Chesso`
(`git show f548ff4:src/chesso.cpp`), so it plays with `fastchess.sh` printing
that the identity check did not happen -- DEC-204 (b), decided for exactly this
class of run -- and it advertises `Hash` and `Threads`, which is what the
harness sets.

### The two scripts

| | `adocs/data/S199_drift.sh` | `adocs/data/S109_ltc.sh` |
|---|---|---|
| what | drift at the regime | transfer to four times the control |
| candidate | the working tree at the coordinator's HEAD | `600f448` (S109's landing commit) |
| reference | `f548ff4` (pinned) | `50e3661` (the pre-block commit, its parent) |
| regime | `fastchess.sh`'s own defaults: `8+0.08`, `Hash=16`, `noob_3moves.epd` | `TC=32+0.32`, `HASH=64`, same book |
| rounds | 1000 pairs, 2000 games, no `-sprt` | 1000 pairs, 2000 games, no `-sprt` |
| estimate | **56 min** at the measured 2133 games/h (S212's A/A) | **3.7 h** at the measured 545 games/h (S151) |
| ceiling | 6750 s | 27360 s |
| resolution | +/- 15.2 nElo, about +/- 11.6 logistic Elo | the same |
| abort | forfeits over 1.0 % a side; a crash voids (`SPRT-RUN-INVALID`, S212) | the same |

Both are launched detached with a terminal marker on every exit path, in
`S151_ltc.sh`'s shape: `set -uo pipefail`, the `cd` guard, the `[[ -x ]]` check,
`shopt -s execfail` and the explicit `SPRT-RUN-FAILED` after the `exec`.
`bash -n` clean, both.

**`S199_drift.sh` sets no `TC` and no `HASH`**, deliberately: a drift point is
taken at the regime verdicts are taken at, so the trend and the ledger stay on
one footing. The regime is read off each run's banner into the TSV, and the
reader refuses to difference two rows whose regime or reference disagree --
the book has already moved once (DEC-189), and a trend across two regimes is
not a trend. The candidate is the working tree, so **the coordinator rebuilds
`build/` before launching**: a binary stamped `<sha>-dirty` is refused once
HEAD moves, which is S212's identity check doing its job.

**`S109_ltc.sh` takes `600f448` and not the tests-only follow-up `1952c56`**,
because `git diff 600f448 1952c56 -- src/` is empty: the follow-up plays the
identical engine and adds nothing to the comparison, exactly as S151 chose
`21b4a21` over its own completing commit. `git diff 600f448 HEAD -- src/` is
also empty, so this run's candidate is the same engine the `8+0.08` SPRT is
playing, and `S109_sprt.sh` uses `REF=50e3661` too -- the two runs measure one
pair at two controls. Its header carries `# S109 SPRT at 8+0.08: <filled by the
coordinator at launch>` for the figure the reading is taken against. All ten
constants the block adds (`LmpBase`, `LmpDepthCoeff`, `LmpMaxLmrDepth`,
`FutBase`, `FutSlope`, `FutMaxLmrDepth`, `HistPruneCoeff`,
`HistPruneMaxLmrDepth`, `SeeQuietCoeff`, `SeeQuietMaxLmrDepth`) are DEC-202's
bound class, so the block is inside the rule's scope without a judgement call.
One difference from S151's run is pre-registered: both commits **post-date**
S147, S170 and S171, so an `Incomplete mating PV` count is no longer expected
from either side -- a non-zero count is worth a sentence and a look, never an
abort of a run already played.

### The reader, and what it refuses

`adocs/data/S199_drift.py <outdir> <runlog>` appends one row to
`adocs/data/S199_drift.tsv`: date, candidate sha, reference sha, regime, games,
Elo and its 95 % half-width, nElo and its half-width, the pentanomial, the pair
variance, and the two hand-filled columns -- the verdicts landed since the
previous point and the conversions landed since it (F30). `--check` re-reads
the file and prints the trend. Stdlib only; `S105_pairs` is imported and not
copied, this directory being append-only (the `S198_pairs.py` precedent).

It refuses rather than writes a row that is half from one run: a log with no
`SPRT-RUN-DONE` or any `SPRT-RUN-INVALID`; a `bounds` line that is not
`none -- fixed ... rounds`, because an SPRT's early-stopped estimate is a
different and upward-biased quantity (DEC-063); a pentanomial from the PGN that
disagrees with the one fastchess printed; and a PGN that does not name exactly
two engines, one of them the banner's `ref-<sha>`. The candidate's name is
**derived** from the PGN rather than assumed, which is the S151/S198 trap
written down as code: `S105_pairs.report` defaults to `chesso-a`, and a name
matching neither side silently inflates the variance.

**Self-test, run 2026-09-13** (`--self-test .tuning/coord/S199_selftest`, single
core, instant): a fabricated 8-game PGN of four pairs scoring 2.0, 1.0, 1.0 and
0.0 and a fabricated log carrying **two** results blocks. Sixteen assertions,
all green -- banner shas, regime, games, `Elo 42.50 +/- 11.60`, `nElo 55.25 +/-
15.20`, the pentanomial `[1, 0, 2, 0, 1]` taken from the **last** block and not
the decoy `[9, 9, 9, 9, 9]` before it, the engine name derived as `candidate`,
the pair variance 0.5000 exactly, `S105_pairs.report` agreeing to the digit,
and all four refusals observed. The `--check` differencing path is exercised on
a two-row fixture in the same run. `SELF-TEST OK`.

### The reading rule, written before the first run

**A point is a sum, never an attribution.** A fixed match against a pinned
reference measures everything between the two commits at once; no point is a
verdict on any one change inside it, and nothing in the engine changes on any
outcome (`excludes:`, DEC-108, DEC-143).

Point *k* is read against a band, and the band is built before the point is
looked at:

    expected_k   = elo_(k-1)
                 + sum of the logistic Elo point estimates of the verdicts
                   landed since point k-1
                 + sum of the conversions landed since, at DEC-083's published
                   rate -- 2.10 Elo per percent of nps at short time control,
                   which is the control this match runs at -- each named as a
                   conversion and not as a verdict
    half-width   = sqrt(h_k^2 + h_(k-1)^2), about +/- 16 logistic Elo when both
                   points are 1000 pairs

- **Inside the band: as expected.** The kept verdicts and the conversions are
  in the engine at about the size they were measured at. This is not "no
  progress"; it is the instrument's resolution being wider than most single
  verdicts.
- **Below the band: those verdicts are the suspects for S183's discount.** The
  named ids are what to look at, in the order of their point estimates, and the
  next step is a decision about which to re-measure -- never an automatic
  revert, and never a claim that any one of them was the cause.
- **Above the band: the sum under-counts what landed.** The usual reason is
  something in the interval that took no verdict at all, and the conversions
  column is where the F30 term lives: block 2 is eight speed steps that land no
  verdict, and a point taken after it would read high against a rule with no
  slot for them. The point then says whether the conversions were worth what
  the published rate claims, which is a measurement this project has never had.
- **Logistic Elo is the unit the band is built in**, because DEC-083's
  conversion rate is published in it. The nElo pair is reported beside it and
  is the one to use for the point-to-point delta, its half-width depending on
  the pair count alone and not on the book or the draw rate.
- **A regime change breaks the series.** Two points are differenced only when
  the reference and the regime agree; otherwise the trend restarts and the row
  says where. The reader enforces this.

**The first point has no previous point, so it is read against two marks.**

**Mark one: zero.** The pin is three weeks and one block behind; an interval
that includes zero would say this instrument cannot see the difference, which
would be a finding about the instrument.

**Mark two: the sum of what was kept since the pin.** Every verdict landed
since `f548ff4`, read from each step's own completion stamp (the ledger in
`adocs/plan.md` and `plan_done/`), with the logistic Elo the run printed:

| step | what | bounds | verdict | Elo | nElo |
|---|---|---|---|---|---|
| S085 | the SPSA vector, ten axes | `{0, 5}` | H1 | +21.02 +/- 9.86 | +26.81 +/- 12.55 |
| S093 v1 | history malus and gravity ageing | `{0, 5}` | H1 | +10.73 +/- 6.70 | +13.63 +/- 8.50 |
| S107 | killers for checking quiets | `{-5, 0}` | H1 | +12.67 +/- 8.65 | +16.18 +/- 11.03 |
| S108 | static evaluation at every node | `{-5, 0}` | H1 | +2.28 +/- 4.40 | +3.13 +/- 6.03 |
| S130 | table score as stand pat | `{0, 5}` | **no verdict**, kept | +1.14 +/- 4.04 | +1.48 +/- 5.26 |
| S165 | null-move mate-band guard | `{-5, 0}` | H1 | +0.95 +/- 3.56 | +1.34 +/- 4.99 |
| S207 | a repetition before the root is not a draw | `{-5, 0}` | H1 | +3.32 +/- 5.00 | +4.47 +/- 6.72 |
| S042 | en passant key only when capturable | `{-5, 0}` | H1 | +7.87 +/- 7.12 | +9.93 +/- 8.99 |
| S109 | four shallow-depth pruning rules | `{0, 5}` | H1 | +46.90 +/- 15.43 | +56.76 +/- 18.44 |
|  | **sum of the kept point estimates** | | | **+106.88** | **+133.73** |

**S109's row was added at 08:52, after its SPRT ended at 08:49** -- H1 accepted
over 1364 games, `adocs/data/S109_sprt.log`, which is the evidence and the copy
to re-read; this table is the second copy and the first is the one to trust.
S085 is in the interval (`git merge-base --is-ancestor f548ff4 21b4a21`) and is
not a row of `plan.md`'s ledger table, which starts at S093 -- its figure is
S151's header and DEC-202, quoted from `plan_done/S085_spsa_first_run.md`.

**Four things the +106.88 is not** (the heading first read +59.98, the pre-S109 sum; corrected by the coordinator after the fast check), all of them stated before the number comes
back:

1. **It is an upper reading.** Each figure is an SPRT's early-stopped point
   estimate and biased upward (DEC-063: S068's pooled estimate fell from
   +12.18 to +5.02 under the correction, a factor of 0.41). DEC-108 applied
   that correction to a sum of exactly this kind and it cut +90 to about +49,
   a factor near 0.54. So the honest mark is a range, **about +44 to +107
   logistic Elo**, with DEC-108's own factor putting the middle near +58; a
   point inside that range separates nothing, and the value of the first point
   is that it says which end the truth sits at. It is also the widest the
   range will ever be -- every later point is read against the previous point
   and the short list of verdicts between them, not against a sum this long.
2. **Five of the nine were `{-5, 0}` runs**, which tested "not a regression"
   and not the size of a gain. Their point estimates are what the run printed,
   not what it decided.
3. **It omits everything in the interval that took no verdict** -- bug fixes,
   table rebuilds, the tuner's own work -- and the drift match measures the
   engine and not the ledger, so those are in the number whatever the table
   says. Nothing in the interval carries an nps conversion figure: S104's
   +18.22 % is *inside* the pin, and S179's magic regeneration and S211's table
   rebuild both landed value-identical. The conversions column of the first row
   is the coordinator's to confirm from the Done list over `f548ff4..HEAD`.
4. **It counts only what was kept**, which is the whole mechanism: S093 v2
   (-1.65), S149 (-11.02), S024 v1 (-4.42) and S148's challenger were measured
   and did not land, and the fishtest FAQ's sentence -- the estimates "are only
   unbiased if one takes all patches into account, both passed and non-passed
   ones" -- is why a sum of the survivors drifts up by construction. This
   instrument is the measurement of that drift, which is what S183 is
   discounting for.

### The sentence proposed for `adocs/plan.md`

Not written by this step (the coordinator owns `plan.md`). To be appended to
the Elo-arithmetic paragraph, after "...which is what makes each run's
pre-registered reading load-bearing rather than a formality.":

> **The one measured check on that ledger is S199's drift series,
> `adocs/data/S199_drift.tsv`**: one fixed 1000-pair match at the harness's own
> regime against the pinned early-S105 commit `f548ff4`, one point per block
> boundary, which prices the *sum* of the kept verdicts at about +/- 11.6 Elo a
> point -- self-play against an older self, never a rating, and never a verdict
> on any single change inside it (R14, DEC-139).
