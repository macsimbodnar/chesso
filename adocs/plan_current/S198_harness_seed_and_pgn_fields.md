id:         S198
goal:       `fastchess.sh` seeds the opening order and records node counts and clock margins in the PGN, and one A/A shows the distribution unchanged
accepts:    `-srand <seed>` is passed, the seed derived from the run stamp unless `SRAND` overrides it, and printed in the banner so a run's opening sequence is reproducible; `-pgnout` gains `nodes=true timeleft=true`; `tests/test_fastchess_script.sh` gains a case for each; one fixed-rounds `AA=1` run of 1000 games -- the DEC-143 calibration that follows the machine change to the workstation doubles as this step's A/A -- is read with `adocs/data/S105_pairs.py` and `tools/forfeit_report.py`, its pair variance and forfeit rate recorded in this file beside S105's; `DEV_MANUAL.md` "Play games" says what the PGN now carries and how a run's openings are reproduced; no bound, control, book or adjudication setting moves
touches:    fastchess.sh, tests/test_fastchess_script.sh, DEV_MANUAL.md, adocs/data/
excludes:   any change to the bounds, the time control, the hash, the book or the adjudication
decisions:  DEC-139, DEC-143
closes:
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-08
done:

## Why this exists

R11(b) of `adocs/testing_strategy.md`. fishtest and OpenBench both pass a
seed and `order=random` so a run is reproducible and two runs on one change
share their openings; the PGN fields let a census read node counts and clock
margins without a log at `trace`. Both are `fastchess` 1.8.1 options on this
machine. The A/A is DEC-143's rule applied for the first time: the workstation
is a new machine for the harness, and the calibration it owes is the run that
also shows the seed changes nothing about the pair distribution.

## Cost

Two flags, two script cases, a manual paragraph; the run is the workstation's
calibration, about 25 minutes there.

## The run, pre-registered (2026-09-08, before launch)

The workstation's DEC-143 calibration, doubling as this step's A/A. Written
before the first game, as MEASUREMENT requires.

**Lane: calibration at fixed rounds. This is not a verdict and no bounds pair
is priced** -- there is no stopping rule to price. The denominator is 500
rounds, 1000 games, chosen by DEC-143 and by S105's own denominator so the two
readings share one; the error bar on the pair variance is `v * sqrt(2/(n-1))`,
about 6.3 % of `v`, which `adocs/data/S105_pairs.py` prints.

| | pre-registered |
|---|---|
| command | `ROUNDS=500 AA=1 nohup ./fastchess.sh > .tuning/aa_s198.log 2>&1 &` |
| commit | `da8ca0b`, clean tree, both sides |
| binaries | `build/src/chesso` and `.ref-builds/da8ca0b/build/src/chesso`, **sha256 identical**, `b047f22d957935f219ebd2326b48b3b4a496c567518ff65f9f7f956a11f823ce`; bench `24880255` nodes on both, 7400474 and 7378394 nps |
| build config | Release, `CHESSO_ARCH=native`, PGO off, ccache, `/usr/bin/c++` (g++ 13.3), both sides |
| fastchess | `alpha 1.8.1 20260720-daa3ea2` -- the version `.moltke.local.md` records and the script's comment names, so S198 question 5 needs no answer |
| machine | workstation, i7-8700K, 12 threads, governor `performance` on all 12, load average 0.05 at launch, on mains |
| regime | unchanged: `8+0.08`, Hash 16, Threads 1, UHO book, `order=random`, the S105 adjudication, `-repeat`, concurrency 12 |
| expected duration | about 26 minutes at S105's 38.7 games a minute; watcher ceiling 7200 s |

**The three outcomes, decided in advance.**

1. **Inside the band** -- `|z| < 1.96` on the pair variance against S105's
   after-run `0.2395 +/- 0.0152`, with `z = (v_ws - v_S105) /
   sqrt(e_ws^2 + e_S105^2)`, which for `e_ws` near 0.015 is roughly
   `[0.197, 0.282]`. Record and proceed to the first verdict.
2. **Outside it** -- record it as the workstation's own baseline, say which
   shape the difference has (a lower `1.0` pair fraction points at the engine,
   which has moved since S105 by S107, S108, S149, S165 and more; a wider
   `GameDuration` spread points at the machine), and **do not re-run for a
   better number**. The seed cannot move a distribution -- it draws a different
   sample of the same book -- so it is never the attribution. This is the
   owner's answer of 2026-09-08 to question 2 and it amends the goal's "shows
   the distribution unchanged" to a band check.
3. **Any forfeit over `FORFEIT_MAX_PCT` 1.0 on a side** -- the calibration does
   not stand, the BUGS rule applies, and the margin is `MOVE_OVERHEAD_MS` in
   `src/uci.hpp`, which this step excludes: stop, put a new step to the owner,
   take no verdict first. 0 of 1000 is the expectation, which is what S105
   measured here at this control.

**Abort rule.** The run is abandoned, not read, if: the machine leaves mains or
sleeps mid-run (POWER, DEC-109 -- S024's hibernated match); anything else takes
the machine while it plays; or the log reaches `SPRT-RUN-FAILED`. A partial PGN
is not read as a short calibration.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

`fastchess.sh` is the instrument that decides whether a change plays better.
This step makes its opening order reproducible (`-srand <seed>`, printed in
the banner), makes its PGN carry the node count and the clock left after every
move (`-pgnout ... nodes=true timeleft=true`), and then measures the
instrument itself on the workstation: a fixed-rounds A/A of 1000 games, the
same build on both sides, read for pair variance, forfeit rate and throughput
as DEC-143 requires after every harness change. No engine code changes and no
verdict is taken.

### 2. The technique as published

**Seeded openings.** fastchess's manual (fetched 2026-09-05,
https://github.com/Disservin/fastchess/blob/master/man.md): "-srand SEED
Specify the seed for opening book randomization."; under `-openings`, "order -
order of openings (random or sequential). Default is sequential." The seed
matters only with `order=random`, which `fastchess.sh` already passes.
fishtest's documented command line carries `-srand 5895699939700649196`,
`order=random` and an `-event` header
(https://official-stockfish.github.io/docs/fishtest-wiki/Running-Fastchess.html);
OpenBench's seed practice, claimed in "Why this exists", is unverified: the
wiki page fetched today shows no command line. Measured here on `fastchess
alpha 1.8.1 20260715-0b06677` with 2-round runs: one 14-digit seed twice gave
the same two `[FEN]` openings; `-srand 7` and the same seed plus 2^32 each
gave different ones, so nothing is truncated to 32 bits; a 20-digit seed is
refused with `stoull: out of range`, so the parser is a 64-bit unsigned
integer. fastchess echoes the seed nowhere -- stdout, log or PGN -- so the
banner is its only record.

**PGN fields.** Same manual: "nodes - Track node count. Default is false.",
"timeleft - Track time left at end of move. Default is false."; `seldepth`,
`nps`, `hashfull`, `tbhits`, `pv`, `latency`, `min` exist too; no `-seed` or
`-randomseed` does. Measured comment shape with both on: `3... Nc6 {+1.00/3
0.000s, tl=0.000s, n=2437}` -- the existing score/depth and time, then `tl=`
and `n=` after a comma.

**Why fixed rounds calibrate and an SPRT A/A does not.** With `-repeat` the
pair is the unit: the trinomial model "is not correct in case games are played
in pairs with reversed colors ... and unbalanced opening positions are used"
(https://www.chessprogramming.org/Match_Statistics); nElo bounds make a test's
duration "dependent only on the chosen bounds, independent of the specific
draw ratio or opening book used"
(https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-Mathematics.html);
the trinomial-pentanomial variance gap "is adequately explained by the biases
in the opening book" (https://github.com/vdbergh/pentanomial). In an A/A the
true difference is zero by construction, so the pair-score distribution is the
harness's alone: book bias, adjudication, the timing noise concurrency adds.
An SPRT on it accepts H1 with probability alpha exactly, stopping wherever the
LLR's random walk crosses a bound, so variance and throughput would be read
over a denominator the result chose (`adocs/data/S105_calibration.sh`'s header
says the same). Fixed rounds give 500 pairs and a known error bar,
`v * sqrt(2/(n-1))` as `adocs/data/S105_pairs.py` `main` prints it, about
6.3 % of `v`. An A/A as the first test is the practice at
https://dannyhammer.github.io/engine-testing-guide/strength-testing.html.

### 3. What chesso has today, and where the change plugs in

`fastchess.sh` at HEAD (`5cc4d3a`): `case "${1:-}"` sets `sprt`, `rounds`,
`tag`; the A/A guard reads `ref_sha`, `head_sha`, `diff_status`, `AA`;
`stamp="$(date +%Y%m%d_%H%M%S)"` names `outdir`, `pgnfile`, `logfile`; the
banner prints `candidate`, `reference`, `tc`, `book`, `bounds`, `out`; one
`fastchess \` invocation carries `-openings ... order=random`, `-sprt $sprt
model=normalized`, `$adjudication`, `$mate_pv_check`, `-rounds "$rounds"`,
`-repeat`, `-pgnout file="$pgnfile"`; `terminations` and `count` do the
census before `completed=1` and `SPRT-RUN-DONE`. **Every mode passes `-sprt`;
there is no fixed-rounds path**, so the run the accepts asks for cannot be
launched as the script stands (section 10, question 1).

`tests/test_fastchess_script.sh`: `make_sandbox` writes a `fastchess` stub
that only touches `fastchess_invoked`; `run_sandbox tmp ref aa` exports `OUT`
and `PATH` and unsets `REF`/`AA` when not given; case 2 needs exactly one
`^fastchess \\$` line; the closing `echo` says "8 properties hold" and
`DEV_MANUAL.md` "Test" says "asserts eight things".

PGN readers this step leans on: `adocs/data/S105_pairs.py` `report(path,
engine='chesso-a')`, whose `main` never sets the name while `fastchess.sh`
names its sides `candidate` and `ref-<sha>`; `adocs/data/S024_pair_stats.py`
with `CANDIDATE = "candidate"`, validated against fastchess's `Ptnml` line;
`tools/forfeit_report.py` `scan` (`FORFEIT`, `OVERRUN`); `tools/error_profile.py`,
whose `TOKEN` takes a comment whole and whose `ENGINE_EVAL` anchors at its
start, so the appended fields do not break it.

Edits in order, all bash 3.2-safe (no arrays -- an empty one is unbound under
`set -u` before bash 4.4 -- no quoted `[[ =~ ]]` pattern, no `mapfile`):

1. Wait for S171's census marker (section 5). Test first: the stub records
   its argument vector, three new cases (section 6), observed red against
   `git show HEAD:fastchess.sh > /tmp/old_fastchess.sh`.
2. After `stamp=`: `seed="${SRAND:-${stamp//_/}}"` (14 digits,
   `YYYYMMDDHHMMSS`), then `case "$seed" in ''|*[!0-9]*) fail "SRAND must be
   an unsigned integer, got '$seed'";; esac`, refusing a bad override by name
   before the reference is built.
3. Beside the mode block: `sprt_args="-sprt $sprt model=normalized"`; when
   `ROUNDS` is set (same `case` validation) `rounds="$ROUNDS"`,
   `sprt_args=""`, and the `bounds` banner line reads `bounds     none --
   fixed $rounds rounds, a calibration or drift reading, NOT a verdict`.
4. Invocation: `-srand "$seed"` on the `-openings` line; `$sprt_args \`
   replaces `-sprt $sprt model=normalized \` (unquoted like `$adjudication`,
   so an empty value vanishes); `-pgnout file="$pgnfile" nodes=true
   timeleft=true`. The `fastchess \` line stays exactly one.
5. Banner: `echo "seed       $seed"` after `book`, commented as the seed's
   only record. Header comment gains `SRAND=<n> ./fastchess.sh` (replay a
   run's openings) and `ROUNDS=<n> AA=1 ./fastchess.sh` (fixed rounds, no SPRT).
6. `DEV_MANUAL.md`; the run; the readings; `adocs/data/S198_*`; the stamp.

### 4. Constants and seeds

No `SEARCH_PARAM` moves, so the range column is omitted. The default seed is
derived (DEC-105 form b) from the run stamp, `${stamp//_/}`, unique per run
and reproducible from the banner. 500 rounds = 1000 games is DEC-143's and
S105's denominator. Materiality of a variance difference is `|z| > 1.96`, the
two-sided 95 % normal quantile (form a), with `z = (v_ws - v_S105) /
sqrt(e_ws^2 + e_S105^2)` and the `e` values as `S105_pairs.py` prints them:
against S105's after-run `0.2395 +/- 0.0152` and an `e_ws` near 0.015 that is
outside roughly `[0.197, 0.282]`. Forfeit tolerance is `FORFEIT_MAX_PCT` 1.0
per engine (DEC-075, what `rating.sh` passes to `tools/forfeit_report.py
--max-pct`), and 0 of 1000 is the expectation S105 measured on this machine at
this control. Watcher ceiling 2 h, twice the expected 26 minutes at S105's
38.7 games a minute plus a reference build (WATCHERS rule).

### 5. Interactions and traps

No search interaction applies -- no pruning, ordering band, INV-4 term or
time-management change; `src/` is untouched. The harness's own:

- **Do not edit `fastchess.sh` while S171's census runs** (DEC-144's order;
  its log is `.tuning/sprt_s171_matepv.log`). bash reads a script from the
  file as it executes, so an in-place edit can feed the running instance a mix
  of old and new text. Start after `SPRT-RUN-(DONE|FAILED)` is in that log and
  `pgrep -fl fastchess.sh` is empty.
- **The A/A's two sides are two builds, not one binary.** `AA=1` plays
  `build/src/chesso` against `.ref-builds/<sha>/build/src/chesso`, compiled
  from one commit in two directories, where `S105_calibration.sh` copied one
  snapshot to both sides. The reference is configured `Release`, ccache,
  default `CHESSO_ARCH=native`; `build/` must match (`grep CMAKE_BUILD_TYPE
  build/CMakeCache.txt`) or the timed match is an A/B. Prove it before
  launching: after S189, `./build/src/chesso bench` and the reference's `bench`
  print one node total and an nps within a few percent; before S189,
  `python3 tools/search_bench.py <binary> 9` on each.
- **`S105_pairs.py` is silently wrong on `fastchess.sh` names.** With
  `engine='chesso-a'` matching neither side every game scores as Black's
  points, a pair White won twice reads 0.0 instead of 1.0, and the variance is
  inflated with no error. Pass `engine='candidate'` (section 7) and check the
  five pair counts against fastchess's printed `Ptnml(0-2): [...]` and
  `adocs/data/S024_pair_stats.py`.
- **The engine has moved since S105.** S105 ran on this workstation at the
  same regime and 12 threads, but S107, S108, S149, S165 and more landed since
  2026-08-20, and a stronger engine self-plays a different pentanomial shape.
  A difference from S105 is not attributable to `-srand`, which chooses a
  different sample of the same book and cannot move the distribution; S105 is
  a sanity band (section 10, question 2).
- **A fixed-rounds run is never a verdict** (MEASUREMENT rule); `ROUNDS=`
  serves this calibration and S199's drift readings, and the banner says so.
- **The log is WARN-only and comes back empty** (S089; both S105 logs were
  0 bytes): forfeits come from the PGN. **The seed reproduces openings, not
  games**: timed search is not deterministic, and the seed-to-sequence mapping
  is fastchess's, so a replay needs the same book (pinned by
  `books/fetch_book.sh`), fastchess version and `-openings` options.
- **The fastchess version is a harness variable (DEC-143)** and S105's stamp
  records none; the MacBook's is `20260715-0b06677`, the script's comment
  quotes `20260720-daa3ea2`. Record the workstation's `fastchess -version` in
  the stamp and the run log. The PGN grows about 2 MB per 1000 games; nothing
  reads it by offset.

### 6. Tests

No `TEST_CASE`, mutant, golden, INV-6 run or Debug self-play: nothing under
`src/` changes. The mutant killed is the HEAD script -- the new cases fail on
`git show HEAD:fastchess.sh` and pass on the edited one, the "past revision"
argument the test header provides. The stub becomes

    #!/bin/sh
    touch "$(dirname "$0")/../fastchess_invoked"
    printf '%s\n' "$@" > "$(dirname "$0")/../fastchess_args"
    exit 0

and `run_sandbox` gains optional `srand` and `rounds` parameters, exported
when non-empty and otherwise `unset` (the `REF`/`AA` pattern), so a value in
the test's own environment cannot decide a case. The cases:

    # 9. The banner prints the seed and fastchess receives the same one.
    seed_status="$(run_sandbox "$seed_dir" HEAD~1)"
    banner_seed="$(sed -n 's/^seed       \([0-9]*\)$/\1/p' "$seed_dir/out.txt")"
    passed_seed="$(grep -A1 -x -- '-srand' "$seed_dir/fastchess_args" | tail -1)"
    # fail unless status 0, banner_seed is 14 digits, passed_seed == banner_seed;
    # then with srand=424242 both read 424242
    # 10. fastchess_args has the lines `-pgnout`, `nodes=true`, `timeleft=true`
    # 11. ROUNDS=500 AA=1 on a clean tree: `-rounds` followed by `500`,
    #     no `-sprt` line, banner contains `fixed 500 rounds`

Update the header list, the `echo` to "11 properties hold" and `DEV_MANUAL.md`
"Test" to eleven. Red first: `bash tests/test_fastchess_script.sh
/tmp/old_fastchess.sh` prints `FAIL:` for 9, 10 and 11 and nothing else; the
lines go in the stamp. Run under `/bin/bash` 3.2 too if a MacBook is at hand.

### 7. Measurement

Lane: calibration at fixed rounds -- no bounds pair, no worst-case formula,
1000 games by construction, about 26 minutes plus a first reference build.
Pre-register in this file before launch: 0 forfeits expected, S105's band and
section 4's rule. On the idle workstation (`ps aux | sort -rnk3 | head`), book
fetched, tree clean at the commit carrying the flag edits, build identity
proved:

    fastchess -version                                  # record it
    ROUNDS=500 AA=1 nohup ./fastchess.sh > .tuning/aa_s198.log 2>&1 &
    echo $! > .tuning/aa_s198.pid

then the WATCHERS rule's poll loop verbatim with `RUN_LOG=.tuning/aa_s198.log`,
`RUN_PID` from the pid file and the ceiling `7200`, armed through `Monitor`
with `persistent: true` or polled next turn. Read, `<dir>` from
`SPRT-RUN-DONE full <dir>`, in this order:

1. Forfeits: `python3 tools/forfeit_report.py <dir>/games.pgn --max-pct 1.0`.
   Any forfeit: read its overrun ms (S088: 149 to 1309 ms is a thin margin,
   25360 ms a hang). Over 1.0 % on a side: the calibration does not stand, the
   BUGS rule applies, and the margin is `MOVE_OVERHEAD_MS` in `src/uci.hpp`,
   excluded here -- stop and put a new step to the owner before any verdict.
2. Pairs: a new `adocs/data/S198_pairs.py` (the directory is append-only, so
   `S105_pairs.py` is imported, not edited) that puts its own directory on
   `sys.path`, calls `S105_pairs.report(<dir>/games.pgn, engine='candidate')`
   and `S105_pairs.report('adocs/data/S105_calibration_after.pgn',
   engine='chesso-a')`, and prints the `pair variance ... vs ...`, `ratio` and
   `z` lines; output saved as `adocs/data/S198_calibration_pairs.txt`.
3. Throughput: 1000 games over the log's `Total Time:`, as games an hour and a
   minute; `seconds per ply` and `plies per game` from the report.
4. The conversion: the log's final `Elo: x +/- a, nElo: y +/- b`; `a/b` is
   what one nElo is worth in logistic Elo on this harness today -- the ratio
   DEV_MANUAL "Which bounds" says to read off a run, never assume.

Readings. **Inside the band**: record, proceed. **Outside it**: the cause is
the engine's growth, concurrency (DEC-048's open "how many more games") or
the fastchess version, never the seed; record it as the workstation's
baseline, say which the shape suggests (a lower `1.0` pair fraction points at
the engine, a wider `GameDuration` spread at the machine), do not re-run for a
better number. **Any forfeit**: point 1. In every case the pentanomial,
`white won both`, draws, plies and seconds per game go in this file in a
table beside S105's before and after (`0.2343 +/- 0.0148`, `0.2395 +/-
0.0152`; 1.0 pairs 41.6 % and 46.8 %; white-won-both 13.4 % and 19.8 %; draws
40.3 % and 29.5 %; 0 forfeits each). Records: `adocs/data/S198_calibration.log`
(stdout with banner and seed), `S198_calibration.pgn` (about 3 MB; S105 kept
both of its, and this one replays only in its openings), the pairs output and
the wrapper, each with a row in `adocs/data/README.md`. Throughput goes to
`DEV_MANUAL.md` "What a verdict costs, measured" as a third column and to the
workstation's `.moltke.local.md`; if it differs from the 2337 games an hour
S182 converts at, S182 converts at the measured figure.

### 8. Completion checklist

- Gate: `cmake --build build -j8 && ctest --test-dir build -L fast
  --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
  build-tune -L fast --output-on-failure && ./clang-format.sh --check`
  (`-j12` on the workstation). No `src/` change: no `Bench:` line and no
  `No functional change` line; `tools/gate.sh` (S189) has nothing to compare.
- `python3 tools/plan_prose_check.py --touches`, `--citations`, `--prose`.
- `DEV_MANUAL.md` "Play games": the `seed` banner line, `SRAND=` replay and
  its limits, `ROUNDS=` and fixed-rounds-is-not-a-verdict, the `tl=`/`n=`
  fields with one example comment, the calibration column; "Test": eleven
  properties. `MANUAL.md`: nothing surfaces to UCI -- check and say so.
  `adocs/specs.md` "Measurement capacity" bullet: the workstation calibration
  beside S105's, through the coordinator.
- Stamp: the three red lines observed, the fastchess version, the run's seed,
  section 7's numbers against S105's, throughput, where the records went, and
  that no bound, control, hash, book or adjudication moved.
- `plan.md` and `status.md` through the coordinator; the Parked calibration
  item retires there.

### 9. Sources read

- https://github.com/Disservin/fastchess/blob/master/man.md (raw file
  fetched) -- `-srand`, `order=` default, every `-pgnout` field, `-rounds`,
  `-repeat`, `-event`; no `-seed`/`-randomseed`.
- https://raw.githubusercontent.com/Disservin/fastchess/master/README.md --
  "track nodes, seldepth, nps ..., hashfull, tbhits, and time left".
- `fastchess -help`/`-version` here, plus five 2-round scratchpad runs: seed
  reproducibility, 64-bit parse, comment shape, no seed echo.
- https://official-stockfish.github.io/docs/fishtest-wiki/Running-Fastchess.html
  -- fishtest's `-srand`, `order=random`, `-event` in one command line.
- https://github.com/AndyGrant/OpenBench/wiki/SPRT-and-Fixed%E2%80%90Game-Workloads
  -- no command line; OpenBench's seed practice unverified.
- https://www.chessprogramming.org/Match_Statistics,
  https://official-stockfish.github.io/docs/fishtest-wiki/Fishtest-Mathematics.html,
  https://github.com/vdbergh/pentanomial -- the pentanomial quotes above.
- https://dannyhammer.github.io/engine-testing-guide/strength-testing.html --
  the A/A sanity check first.
- https://cantate.be/Fishtest/normalized_elo_practical.pdf -- fetched, text
  not extractable on this machine: unverified here; quoted in
  `adocs/testing_strategy.md` 1.1.
- Repository: `adocs/plan_done/S105_sprt_harness_regime.md`, `adocs/data/`
  `S105_calibration.sh`, `S105_calibration_pairs.txt`, `S105_pairs.py`,
  `S024_pair_stats.py`, `tools/forfeit_report.py`, `tools/error_profile.py`,
  `fastchess.sh`, `tests/test_fastchess_script.sh`, `tests/CMakeLists.txt`,
  `DEV_MANUAL.md` "Play games" and "Test", `adocs/testing_strategy.md` 1.1,
  2.1, R7, R11, `adocs/specs.md` Open items, `.moltke.local.md`, DEC-139,
  DEC-143, DEC-144, DEC-145.

### 10. Questions deferred to the owner

1. **The fixed-rounds mechanism.** The accepts asks for a fixed-rounds `AA=1`
   run and the script has no mode without `-sprt`. Recommended: `ROUNDS=` in
   `fastchess.sh` (section 3), so the calibration exercises the exact
   invocation verdicts use and S199 reuses it; the alternative, a standalone
   runner in S105's shape, calibrates a copy of the command line.
2. **"Shows the distribution unchanged"** can only be a band check against
   S105: the engine has moved since 2026-08-20 and a seed cannot move a
   distribution. Proposed reading: the run is the workstation's calibration
   and S105 its sanity band; a material difference is recorded, attributed by
   shape, not re-run. This touches the accepts' wording.
3. **Seed in the PGN.** `-event "chesso $tag $stamp srand=$seed"` would make
   each PGN self-describing, since fastchess records the seed nowhere else.
   Outside the accepts; recommended.
4. **A forfeit rate over 1 %** points at `MOVE_OVERHEAD_MS` (`src/uci.hpp`),
   excluded here: a new step before the first verdict.
5. **The fastchess version** on the workstation is unknown from here; if it is
   not the `20260720-daa3ea2` the script's comment names, the comment and the
   recorded version should say which build was measured.
