id:         S283
goal:       S099's pair, the pawn key and the pawn-keyed correction against the tree before them, decided by an SPRT at 32+0.32, Hash 64, {0, 10} nElo (DEC-286)
accepts:    (1) a pre-registration `adocs/data/S283_sprt.sh` written before the first game: CAND `8758622`, REF `bacead1`, 32+0.32, Hash 64, `noob_3moves.epd`, `{0, 10}` nElo, DEC-143's worst-case and expected games for that pair of bounds at this control, the abort rule (forfeits over 1.0 % on either side, a crash or disconnect voids), the open DEC-171 findings, and the three outcomes as DEC-286 states them; (2) the run detached with a terminal marker and a watcher with four exits; (3) the verdict recorded with DEC-220's block and the ledger regenerated; (4) on H1, S099 relanded onto the then-tree by its own step, every golden re-derived by its script, S110 and S111 back in the Open list conditional; on H0 or no verdict, nothing ships
touches:    adocs/data/S283_sprt.sh, adocs/data/S283_sprt.log, adocs/data/README.md
excludes:   any change to the candidate; a re-tune of S099's two parameters; any other verdict at a longer control
decisions:  DEC-286, DEC-270, DEC-202, DEC-143, DEC-220
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); pre-registration by one Opus 5.5 subagent, written blind (DEC-260), started 2026-10-15
done:

## Why this exists

S099 read H0 at the harness setting (nElo -4.71 +/- 6.84, `{0, 5}` at
8+0.08, Hash 16) and its DEC-202 estimate at 32+0.32, Hash 64 read nElo
+12.46 +/- 15.23 (`adocs/data/S099_ltc.log`). The owner ruled one SPRT on the
same pair at the longer control (DEC-286), after S023 verdict 1's SPRT. The
pair is fixed and independent of the tree, so the run's attribution does not
depend on what lands before it.

## As built, phase 1 (2026-10-15, blind)

Written while S023 verdict 1's SPRT held the machine (DEC-260): nothing was
built, run, timed or tested; `bash -n` on the script, and a grep of
`fastchess.sh` for the guard's two names (0 matches each), are the only
checks run. Every figure below is read from a file or a git object.

**What was written.** `adocs/data/S283_sprt.sh`, the pre-registration, in
`S099_sprt.sh`'s shape, executable; its row in `adocs/data/README.md` under a
new S283 heading; this section.

**The figures and their sources.**

| figure | value | source |
|---|---|---|
| CAND | `8758622`, 2026-10-14 | DEC-286; `git log` |
| REF | `bacead1`, 2026-10-14 | DEC-286; `git log` |
| CAND's parent | `e2c256c`; `git diff bacead1 e2c256c` touches DEV_MANUAL.md, adocs/ and tools/mutation_check.py, nothing in src/ | `git diff --name-only`, read 2026-10-15 |
| S099's SPRT | H0, `Elo -3.47 +/- 5.03`, `nElo -4.71 +/- 6.84`, LLR -2.96, 9916 games, Ptnml [377, 1247, 1791, 1184, 359], 0 forfeits | `adocs/data/S099_sprt.log` |
| the 32+0.32 estimate | `Elo +8.34 +/- 10.20`, `nElo +12.46 +/- 15.23`, 2000 games, W 500 L 452 D 1048, Ptnml [54, 215, 408, 275, 48], 0 forfeits, 3 h 42 m 38 s | `adocs/data/S099_ltc.log` |
| expected games, midpoint | 10465 = 1046535 / 10^2 | `adocs/testing_strategy.md` 1.1; its `{-5, 5}` row, the same width, agrees |
| expected games, on a bound | 6398 = 639770 / 10^2 | the same |
| throughput at 32+0.32, Hash 64 | 545 games/h (S151, governor `performance`) and 534 (the S109 block); budgeted on 534 | `.moltke.local.md` |
| cross-check | 539 games/h: 2000 games in 3 h 42 m 38 s | `adocs/data/S099_ltc.log` |
| hours, midpoint | 19.6 h at 534, 19.2 h at 545 | computed |
| hours, on a bound | 12.0 h at 534, 11.7 h at 545 | computed |
| the cap | 5250 rounds, 10500 games (10465 rounded up to a whole hundred); 19.7 h at 534, 19.3 h at 545 | the brief's rule; DEC-286's "about 10500 games" agrees |
| watcher ceiling | 144000 s (40 h), at least twice the cap | AGENTS.md WATCHERS |

**fastchess.sh, traced at `4e6ca63`.** `REF`, `CAND`, `TC`, `HASH` and
`OUT` can be passed; `CONCURRENCY` (every core) and the book
(`noob_3moves.epd`, no variable) are right by default. Line by line in that
commit's `fastchess.sh`:

```
fastchess.sh at 4e6ca63
  55-58      header: for other bounds, edit the mode block for the run
  160        REF
  169        CAND
  185        the book, noob_3moves.epd; no variable
  197        TC, consumed at 1050 (-each tc=)
  210        HASH, consumed at 1050 (option.Hash=)
  278        CONCURRENCY, every core by default
  331-353    the mode block: the bounds pair and the round cap
  371        sprt_args, "-sprt $sprt model=normalized", consumed at 1051
  372-378    ROUNDS: replaces the round count and empties sprt_args
  381, 388   REF and CAND resolved
  505        OUT, used at 507-508 for the log and the PGN
  684        build_ref, called at 736 (the candidate) and 782 (the reference)
  921-925    the banner's bounds line; no cap line in SPRT mode
  1050       -each tc= option.Hash=
  1051       $sprt_args
  1054       -rounds
```

**Not settled: two settings cannot be passed.**

- **The bounds.** The mode argument (the mode block in the map) fixes the
  pair: `{0, 5}` at alpha = beta = 0.05 by default, `--nonreg` `{-5, 0}`,
  `--fast` `{0, 10}` at alpha = beta = **0.10** with a 1500-round cap.
  `sprt_args` builds `-sprt $sprt model=normalized` from it. No variable
  reaches it; the header and DEV_MANUAL.md "Which bounds" say to edit the
  block for the run.
- **A cap that keeps the SPRT.** The mode sets 20000 rounds; `ROUNDS`
  replaces the count and empties `sprt_args`, so it gives a fixed-rounds
  reading, never a verdict.

The script therefore passes `SPRT_BOUNDS="elo0=0 elo1=10 alpha=0.05
beta=0.05"` and `SPRT_ROUNDS=5250`, names fastchess.sh does not read today,
and refuses (`SPRT-RUN-FAILED`) unless `./fastchess.sh` expands both. An
unread variable is ignored without a word, so without that guard the run
would play `{0, 5}` to 20000 rounds, 40000 games, about 75 h at 534 an
hour, under this step's name. The names are a proposal; whatever step closes
the gap names them, and the guard and the exec line follow it. The abort
rule adds one line for the same reason: the banner must read the
pre-registered tc, hash and bounds, and fastchess's first `Started game 1 of
N` line must read 10500, since the banner prints no cap in SPRT mode
(lines 921-925 at `4e6ca63`).

**Left for the coordinator, marked in the script.** The commit that closes
the harness gap and the banner's `bounds` line as it printed; S023 verdict
1's state and any change to the Open list since `4e6ca63` (DEC-171); whether
a fixed-rounds A/A is owed first (DEC-143).

**A property of the cap, stated in the script.** A cap at the midpoint
figure leaves a run whose truth sits near the interval's middle, about
+5 nElo, a real chance of reaching the cap with neither bound. That outcome
is pre-registered as closed (DEC-286). The chance was not computed: that
needs a tool run.

### On `be48e02` (2026-10-15, blind)

Brought onto main's `be48e02` while S039's phase 2 held the machine: no
compile, test, tool run or run of the script; `bash -n` and grep are the
checks. The phase-1 diff was saved as `.tuning/coord/S283/s283_pre_ff.diff`,
stashed (the new file staged first: `git stash` refuses an intent-to-add
entry, "not uptodate"), the branch fast-forwarded from `4e6ca63`, and the
stash popped by its own reference; `adocs/data/README.md` conflicted with
S023's and S285's rows and was resolved keeping both sides, S283's section
last.

**What changed under it.** S023 verdict 1 read H0 and was reverted
(`0cdd39a`). S284 (`769ce7a`, DEC-289) taught `fastchess.sh` `SPRT_BOUNDS`
and `SPRT_ROUNDS` under this script's names, a `cap` banner line and the tag
`env`. S282 (`2bdc7cc`) and S285 (`be48e02`) landed. DEC-290 puts S039's
phase 2 and candidate before this run and S039's SPRT after it. The pair is
untouched: `git diff bacead1 8758622^ -- src/` is still empty.

**What changed in the script.**

- The harness section now says S284 closed the gap, and every `fastchess.sh`
  citation in the script names `be48e02` and these lines of it:

  ```
  fastchess.sh at be48e02, as the script cites it
    18-20      usage: SPRT_BOUNDS
    21-25      usage: SPRT_ROUNDS
    44-57      the bounds are nElo, model=normalized
    169        REF
    178        CAND
    194        the book
    206        TC
    208-219    the Hash block; HASH at 219
    287        CONCURRENCY
    364-424    S284's block, SPRT_BOUNDS and SPRT_ROUNDS:
      390-391    refused beside a mode argument
      392-393    refused beside ROUNDS
      395-409    SPRT_BOUNDS checked: the form, elo0 below elo1, alpha and
                 beta inside (0, 0.5)
      410        sprt set from SPRT_BOUNDS
      415-418    SPRT_ROUNDS checked
      419-420    the cap set; the SPRT kept
      423        the tag becomes env
    442        sprt_args appends model=normalized
    452, 459   REF and CAND resolved
    576-579    OUT, the log and the PGN
    755        build_ref, called at 807 (the candidate) and 853 (the reference)
    984        banner: the tc line
    993        banner: the bounds line
    997        banner: S284's cap line
    1114       the PGN's event, carrying the tag
    1126       -each tc= option.Hash=
    1127       $sprt_args
    1130       -rounds
    1239       the done marker, carrying the tag
  ```

- The guard stays. Its pattern, grepped against `be48e02`'s
  `fastchess.sh`, finds `SPRT_BOUNDS` on lines 386, 395, 398, 399 and 410
  and `SPRT_ROUNDS` on lines 387, 415, 416, 417 and 419: it passes there.
- The abort rule's pre-game check names S284's cap line (line 997 at
  `be48e02`) beside the tc and bounds lines and fastchess's `Started game 1
  of 10500` (quoted without its padding then; byte-exact since the fast
  check below).
- DEC-171, re-read at `be48e02`: **none open**. S282, the one open filler
  at `4e6ca63`, and S285 are done. One marked sentence is left for the
  coordinator: S039's state on the day it pins.
- H1's reland names its base, S039's outcome (DEC-290). The regime notes
  S284 owes no A/A (DEC-289). The watcher note gives the done marker as
  `SPRT-RUN-DONE env <out>`.
- The README row says the gap was closed by S284.

Unchanged: the pair, the bounds, 10465 / 6398 games, the 5250-round cap,
the hours, the three outcomes, the exec line.

### Fast check: 4 findings, fixed (2026-10-15, blind)

The coordinator's fast check over the `be48e02` diff, fixed with `bash -n`,
grep, git and edits only (S039's phase 2 held the machine); the script was
not run.

1. **The banner literals were not byte-exact.** The abort rule quoted
   `bounds` with one space; at `be48e02` the `fastchess.sh` banner prints
   `bounds` and five spaces (line 993) and `cap` and eight (line 997). The
   abort rule now carries the whole banner as `be48e02` prints it for this
   run -- candidate, reference, config, padding, tc, book, seed, bounds,
   cap, out and both `id name` lines -- byte for byte from the `echo` lines
   at lines 971-1002 and 1071 of that commit, with the seed and stamp marked
   as the run's own, no `diff`, `relay` or `A/A CALIBRATION` line, and
   `Started game 1 of 10500`. The dates are each sha's committer date
   (2026-10-14 both, `git show -s --date=short --format=%cd`, as lines
   942-945 of that commit print it); config, padding and both identity
   lines are what `S099_ltc.log` printed for this pair.
2. **The environment could change what plays.** The script read
   `${REF:-bacead1}` and `${CAND:-8758622}`, so a leaked `REF=HEAD` passed,
   and the src guard cannot catch every wrong REF. Every variable
   `fastchess.sh` reads at `be48e02`, and what the script now does with it:

   | variable | read on lines, at `be48e02` | the script |
   |---|---|---|
   | `REF`, `CAND`, `TC`, `HASH`, `CONCURRENCY` | 169, 178, 206, 219, 287 | refuses if set to anything but `bacead1`, `8758622`, `32+0.32`, `64`, `12`; passes them on the exec line |
   | `SPRT_BOUNDS`, `SPRT_ROUNDS` | 386 to 420 | the same, booked `elo0=0 elo1=10 alpha=0.05 beta=0.05` and `5250` |
   | `ROUNDS`, `BARE_FEN`, `SRAND`, `AA` | 392 and 443, 249, 560, 527 | refuses if set at all, even empty; `env -u` on the exec line |
   | `RELAY_PYTHON` | 254, only under `BARE_FEN` | `env -u` on the exec line, not refused |
   | `OUT` | 576 | overridden on the exec line; names a directory, changes no game |
   | `HOME`, `TMPDIR` | 254; 478, 842 and 869 | untouched: the relay's default python, scratch files |
   | `PATH` | every command lookup | untouched: it picks the fastchess, git, cmake and compiler binaries; the fastchess version is DEC-143's A/A question on the day |

3. **The coordinator's sentence had no guard.** The script now refuses
   (`SPRT-RUN-FAILED`) while its own file, read by `BASH_SOURCE` before the
   `cd`, contains the opening marker of that bracketed sentence, or cannot be
   read. The pattern is assembled from two halves so the guard does not
   match itself; grep finds the marker on one line of the script, the
   sentence's own.
4. **`build_ref` is line 684 at `4e6ca63`, not line 685.** Fixed in the phase-1
   section above; the other `4e6ca63` citations there were re-read with
   `git show 4e6ca63:fastchess.sh | grep -n` and hold.

The exec line now reads `env -u ROUNDS -u BARE_FEN -u SRAND -u AA -u
RELAY_PYTHON` and passes `REF`, `CAND`, `TC`, `HASH`, `CONCURRENCY`,
`SPRT_BOUNDS`, `SPRT_ROUNDS` and `OUT` from the script's own booked values.
The src guard and the harness guard now fail through the same `fail` helper.
`bash -n` passes.
