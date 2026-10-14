id:         S261
goal:       the search block's own SPSA lane: the search parameters added since S085 and never tuned together are fitted jointly on today's tree, then verified by an independent SPRT
accepts:    (1) the axes are listed before launch: every parameter in `src/search_params.hpp` `CHESSO_SEARCH_PARAMS` that was added or reseeded after S085's run, that a node-count probe reaches (S214's reachability rule; `TmHardPercent` stays out, DEC-200), and that no later lane fitted (S222's and S231's lanes are named and their axes excluded or included with the reason); each axis keeps the range `tests/test_search_params.cpp` pins; (2) the run is sized by `DEV_MANUAL.md` "Sizing a real run" with the iteration count and the expected hours stated before launch, in chesso's lane size (eight to nine hours, DEC-222 (8)), so a night under DEC-155; if the listed axes cannot be fitted together in that size, the split into two lanes is stated before the first one starts, never after; (3) detached with a terminal marker and a watcher with the four exits the WATCHERS rule names; (4) an axis that returns at the end of its range is reported and decided, not shipped silently -- S085's `RfpMinPly` at 0 turned three mate cases red; the mate suites (`test_mate_breadth`, `test_mate_pv`, `test_mate_carry`) are green on the tuned vector before any match; (5) the tuned vector is verified by an **independent** `{0, 5}` nElo SPRT at 8+0.08 against the untuned tree, with DEC-143's worst case and abort rule pre-registered; H0 or no verdict ships nothing; (6) at the block boundary this closes: one S199-form drift point and one DEC-202 fixed 1000-pair reading at 32+0.32, Hash=64, both read as estimates beside the verdict, never as verdicts
touches:    src/search_params.hpp, tools/spsa_driver.py only if sizing needs it, tools/ for the run's JSON, MANUAL.md for any default that moves, adocs/specs.md, adocs/data/ for the lane's log and readings
excludes:   evaluation weights, which the tuner fits; S127, the final lane over the whole set after the evaluation block; the time manager's hard limit (DEC-200); the axes S099 and S023 will add, which S127 fits
decisions:  DEC-222, DEC-200, DEC-202, DEC-143, DEC-258
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:

## Why now

DEC-222 (8) made SPSA a cadence -- one lane per completed block over the
axes that block added, S127 the last. The search block is complete and its
lane was never scheduled: it is the omission both the 2026-10-07 proposal and
its comparison found. S085 tuned twelve of twenty-two parameters for +21.02;
since then S091, S095, S097, S098, S109, S113, S114, S115, S116, S132, S222,
S234 and others added axes that sit at census-derived or midpoint seeds, and
several were parked explicitly "for S127 to fit". Every evaluation fit after
this point is fitted against the search these values produce, and the corpus
S082 generates is played by it, so the lane runs before both (DEC-258).

## Description it is implemented from

CPW *SPSA*; Spall's papers on simultaneous perturbation; the project's own
driver, `tools/spsa_driver.py` (S084), and its sizing section in
`DEV_MANUAL.md`. The seeds are the shipped values, which are the project's
own (DEC-134 form b or c, each already recorded at its landing step).

## From the record

S084, S085 (+21.02), S151 (S085's vector at 32+0.32: +11.99 +/- 11.57, an
estimate), S222 and S231 (narrow lanes with their own axes), DEC-200, DEC-202.

## Pre-registration

Written 2026-10-13 in phase 1, while S119's SPRT held every core (DEC-260):
reading and file edits only, nothing built, run or probed. Everything below is
fixed before the lane's first game; phase 2 executes it and may only record
what happened.

**The base.** The lane runs on the tree S119's verdict leaves (the cluster
table kept on H1, reverted on H0), after S263 and S276 land. Nothing below
depends on which: none of the three touches `src/search_params.hpp` (`git diff
--stat cc87dfb 298788b` lists no such file; S263 and S276 are node-identical
or test-side), the seeds and ranges are read from that file at `4f8b949`, and
the SPRT's pair is pinned only after the lane. The tree the lane fits is
whatever the runner's banner prints as `head`; item (5) holds the SPRT's
reference to it.

### (1) The axes

**Method.** `git log -p -- src/search_params.hpp` from S085's landing
(`21b4a21`, 2026-08-21) to `4f8b949`, every `X(...)` row added or whose
default moved, against the 73 rows the list holds today. **52** were added or
reseeded after S085's run; the other 21 are S085's twelve (eleven shipped,
`RfpMinPly` held at 3) and the nine `Tm*` rows S089 added before it. Of the 52,
**30 are in the lane and 22 are out**, each by name below. **DEC-269 amends
accepts (1)**: two of S085's twelve, `LmrBase` and `LmrDivisor`, join as rows
31 and 32, so **the lane is 32 axes**. Ranges are the
declared ones, unnarrowed: `tools/spsa_driver.py check` refuses a config whose
bounds differ from the binary's, and `tests/test_search_params.cpp` "the
declared ranges are the ranges the engine ships" pins each one. Neither S222's
lane (2026-09-14/15, eleven history axes) nor S231's (2026-09-19/20, six
continuation axes, reverted on H0) fitted any of the 32; S085 fitted rows 31
and 32.

**The probe for every axis (S214's rule), reach pending.** One command probes
all thirty-two, each at its own two declared bounds, down the four-rung ladder,
and fails by name when both bounds search the same number of nodes:

```bash
python3 tools/spsa_driver.py check tools/spsa_s261.json --engine build-tune/src/chesso
```

Rungs: `d9` = `go depth 9` on `PROBE_FEN` (the midgame position); `d13` = `go
depth 13` on it; `d13t` = `go depth 13` on `PROBE_FEN_TACTICAL`; `clock` = `go
wtime 1000 btime 1000 winc 200 binc 200`, five agreeing samples at each bound.
The table's "expected" rung is reasoning from the code, not a measurement;
every axis is **reach pending** until phase 2's check prints it.

| # | axis | landed | seed, DEC-134 form | range | c_end | expected rung |
|---|---|---|---|---|---|---|
| 1 | `QsFutilityMargin` | S112, 2026-09-27 | 188, (a) wiki delta margin in chesso's pawn | 0..2000 | 24 | d9 |
| 2 | `RazorMargin` | S116, 2026-10-02 | 282, (a) wiki, three pawns | 0..2000 | 24 | d9 |
| 3 | `LmrCutNode` | S098 v2, 2026-09-16 | 1, (c) | 0..2 | 0.5 | d9 |
| 4 | `LmrNotImproving` | S098 v2 | 1, (c) | 0..2 | 0.5 | d9 |
| 5 | `LmrTtCapture` | S098 v2 | 1, (c) | 0..2 | 0.5 | d9 |
| 6 | `LmrPv` | S098 v2 | 1, (c) | 0..2 | 0.5 | d9 |
| 7 | `LmrNoTtMove` | S095, 2026-09-20 | 1, (c) | 0..2 | 0.5 | d9 |
| 8 | `LmrDeeperMargin` | S098 v3, 2026-09-17 | 47, (c) | 0..94 | 8 | d9 |
| 9 | `LmrDeeperMinReduction` | S098 v3 | 2, (b) | 1..126 | 1 | d9 |
| 10 | `LmpBase` | S109, 2026-09-13 | 733, (b) census | 0..27000 | 100 | d9 |
| 11 | `LmpDepthCoeff` | S109 | 0, (b) census, at its floor | 0..27000 | 25 | d9 |
| 12 | `LmpMaxLmrDepth` | S109 | 8, (c) | 0..16 | 2 | d9 |
| 13 | `FutBase` | S109 | 147, (a) Heinz via wiki | 0..48000 | 24 | d9 |
| 14 | `FutSlope` | S109 | 170, (a) | 0..2000 | 16 | d9 |
| 15 | `FutMaxLmrDepth` | S109 | 8, (c) | 0..16 | 2 | d9 |
| 16 | `HistPruneMaxLmrDepth` | S109 | 8, (c) | 0..16 | 2 | d9 |
| 17 | `SeeQuietCoeff` | S109 | 50, (b) own see scale | 0..10000 | 12 | d9 |
| 18 | `SeeQuietMaxLmrDepth` | S109 | 8, (c) | 0..16 | 2 | d9 |
| 19 | `SeeCaptureCoeff` | S091, 2026-09-14 | 50, (b) | 0..10000 | 16 | d9 |
| 20 | `SeeCaptureMaxLmrDepth` | S091 | 8, (c) | 0..16 | 2 | d9 |
| 21 | `SeeLmrExtra` | S091 | 1, (a) as a form | 0..3 | 0.5 | d9 |
| 22 | `SeMinDepth` | S097, 2026-09-21 | 10, (c) | 4..16 | 1 | d9 (4 fires, 16 cannot) |
| 23 | `SeTtDepthMargin` | S097 | 4, (c) | 0..8 | 1 | d13: SE needs remaining depth >= 10, which `go depth 9` never has |
| 24 | `SePlyFactor` | S097 | 5, (c) | 2..8 | 1 | **none expected**: binds only at `ply >= factor * depth` with depth >= 10, a 20-ply line at factor 2. **Removed 2026-10-13**: identical on all four rungs ("As built") |
| 25 | `SeMarginPerDepth` | S097 | 9, (c) | 1..18 | 2 | d13, as 23 |
| 26 | `ProbCutMargin` | S113, 2026-09-28 | 49, (b) own fit at (a) t = 1.0 | 0..2000 | 16 | d9 |
| 27 | `ProbCutDepthOffset` | S113 | 5, (a) the paper's pair | 2..8 | 1 | d9 (8 leaves no child depth at depth 8) |
| 28 | `ProbCutMinDepth` | S113 | 8, (a) the paper's height | 3..10 | 1 | d9 (10 cannot fire) |
| 29 | `AspirationWidenPct` | S115, 2026-09-30 | 200, (b) S115's sweep | 100..400 | 25 | d9 if any root search fails, else d13 or clock |
| 30 | `AspirationFailLowPull` | S115 | 2, (c) | 0..4 | 0.5 | needs a root fail-low: d9, d13 or clock |
| 31 | `LmrBase` | S073, 2026-08-16; DEC-269 | 52, S085's SPSA fit (75 -> 52) | 0..400 | 24 (S085's) | d9 (separated there in S214's 2026-09-12 run) |
| 32 | `LmrDivisor` | S073; DEC-269 | 182, S085's SPSA fit (225 -> 182) | 1..2000 | 32 (S085's) | d9 (as 31) |

Rows 1 to 30 are first settings no lane has fitted, and the landing files park
each one "for S127": S091 (`SEE_CAPT_*`, `SEE_LMR_EXTRA`), S095, S097 ("every
SE constant refit"), S098 ("S127 refits the six survivors"), S109 ("SPSA'd at
S127"), S112 (its margin a seed that must be fitted), S113 ("margin,
threshold and offset join the SPSA set"), S115 ("SPSA may re-fit the pull
weight and multiplier"), S116. DEC-222 (8) makes this lane the first of those
fits; S127 refits them all again after the evaluation block. Rows 31 and 32
are the reduction table those terms sit on: S098 routes them to be refit
beside its terms ("the record that they go stale"), so DEC-269 fits the table
with the terms instead of around an August base.

**Out, 22, each with its reason:**

| axes | reason |
|---|---|
| `QsFutility`, `QsQueenPromotions`, `RfpTtEstimate`, `SeExtend`, `SeMultiCut`, `ProbCut` | verdict switches with no setting between 0 and 1 (DEC-215); five of the six comments say "not something S127 sweeps" and `SeMultiCut`'s says there is no third setting of it |
| `QuietHistoryMax`, `HistoryBonusQuad`, `HistoryBonusLin`, `HistoryBonusConst`, `HistoryMalusQuad`, `HistoryMalusLin`, `HistoryMalusConst`, `ContHistBonus`, `ContHistMalus`, `ContHistWeight`, `HistPruneCoeff` | **S222's lane fitted all eleven** (60000 games, 2026-09-14/15, H1 +11.13 Elo); S231's lane refitted the three `ContHist*` and left with its H0 (`326f274` restored S222's values). The accepts excludes an axis a later lane fitted. `HistPruneMaxLmrDepth` beside it was never fitted and is in |
| `TmNodeBasePct`, `TmNodeScalePct`, `TmNodeMinDepth` | S132's time-management triple. DEC-094 keeps the `Tm*` family out of a lane tuned at a control its verification does not share; S132's own file says a fit of these three "wants the playing control or a second-control verification"; and `TmNodeScalePct`'s floor 0 is a switch discontinuity that its comment says a lane should avoid by declaring its floor at 1, which `check` refuses (config bounds must equal the binary's). S127's file is where they go |
| `RazorDepth` | the default is the rule as specified, not a fitted value; 0 is off and above 1 is the multi-depth variant, and S116 says "a depth-scaled margin comes with it at S127 or not at all". A +/-1 perturbation would alternate the off value and the refused variant |
| `LmrRoundBias` | ships at its off value 0 because S236 built 1, 8, 64, 128, 256 and 512 and each lost a row of the fast suite's "pruning does not hide a forced mate"; no endpoint but 0 could pass item (4), and S236 frees it only by re-mining that row or at S127 |

`TmHardPercent` stays out (DEC-200). The axes S099 and S023 will add do not
exist yet and are S127's. **Ten of S085's twelve stay out by the accepts'
letter** -- none was added or reseeded after its run -- and `LmrBase` and
`LmrDivisor` are in by DEC-269. DEC-269 rejected `MaxQsearchDepth` and
`AspirationDelta` too: S112 and S115 hand them to S127. Two notes on the ten,
neither acted on here: `RfpMinPly`'s range moved twice (S142, S251) but its
default never did, and its floor 3 is a measured mate cliff; `LazyEvalMargin`
retires with S039.

**When the check refuses an axis** (decided now, applied in phase 2 before
launch; the coordinator decides as DEC-200 did):

- *"searched the same number of nodes under every probe"*: first
  `grep -n <SYMBOL> src/*.cpp` -- a symbol nothing reads is a wiring bug and
  the BUGS rule binds before anything else; a symbol that is read is a
  property of the parameter at these probes, DEC-200's shape. The row leaves
  `tools/spsa_s261.json`, the step file records the check's own lines, and
  `check` is re-run on the edited config. Never re-probed with a hand-picked
  position or a held-off second option (DEC-200's Rejected). `SePlyFactor` is
  the one expected here.
- *"not repeatable"*: re-run on an idle machine (DEC-200). Still not
  repeatable idle, the only evidence of reach is a clock count that moves by
  itself, and the axis leaves as above.

### (2) The sizing

**Regime, S085's verbatim**, as S222's and S231's lanes ran it: 1250
iterations x 24 pairs = 30000 pairs = **60000 games**, `2+0.02`, `Hash=16`,
`Threads=1`, concurrency 12, `books/UHO_4060_v3.epd` (242201 openings, never
wraps), `fastchess.sh`'s adjudication with `twosided=true`, alpha 0.602,
gamma 0.101, `a_ratio` 0.1 (A = 125), `r_end` 0.004, seed 261.

**Hours.** The lane plays 2+0.02, not 8+0.08, so `.moltke.local.md`'s 2110
games an hour is not the figure (DEV_MANUAL: never scale a games-per-hour
figure between controls). The figure is this exact shape on this machine:
**S222 8 h 37 m 31 s (24.84 s an iteration), S231 8 h 40 m 45 s (25.00 s)**.
The axis count does not enter -- two evaluations an iteration whatever the
dimension. **Estimate 8 h 45 m, ceiling 18 h (64800 s)**: DEC-222 (8)'s lane
size, a night under DEC-155.

**One lane, not two -- and the split test that could overturn it, run before
launch.** Wall time does not grow with the axis count; resolution can. Each
axis's gradient estimate carries the match's noise, the same at any width,
plus cross-talk from every other axis's gradient. On arithmetic alone, at
S085's resolving depth of 3 Elo an axis, every optimum still four `c_end` out
and S231's 31 % draws, 31 other axes add about 16 % to an axis's gradient
variance at the end-of-run `c` and about 70 % at the first iteration's (2.05
`c_end`), against about 6 % and 23 % at S085's twelve; the cross-talk shrinks
as the axes close on their optima. A cost, not a wall: the default is one
lane. `adocs/data/S261_split_sim.py` measures it on the driver's own
simulator at the lane's exact config, five seeds, all 32 axes weighted against
the same config with every odd-indexed axis at weight zero, over the 16 axes
weighted in both. **Its rule, fixed now: `SPLIT` if the 32-axis per-axis
recovery is below 0.8 of the 16-axis one; otherwise one lane; a 16-axis
recovery that is not positive cannot tell them apart and reads one lane.**
The 0.8 is a judgement made before the number.

**The split, stated now and used only on `SPLIT`:**

- **Lane 1, the move loop**, 21 axes (rows 3 to 21, 31 and 32): the
  reduction table and its five LMR terms, the re-search pair, late move,
  futility, history and SEE pruning, the extra SEE reduction. `LmrBase` and
  `LmrDivisor` join this lane because DEC-269's reason is that they are fitted
  with the terms. Its own night, the same regime, seed 2611.
- **Lane 2, node level, quiescence and root**, 11 axes (rows 1, 2, 22 to 30):
  the two margins, singular extension, ProbCut, aspiration. Seed 2612, the
  next night, on a tree whose defaults carry lane 1's rounded vector after
  item (4)'s rule -- committed before lane 2 launches, as S222's phase three
  committed its vector before its SPRT.
- **One SPRT over both** (DEC-210's one vector, one verdict): CAND is lane 2's
  landing, REF the tree before lane 1's. Two nights instead of one.

**`c_end`, the rule** (DEV_MANUAL "Sizing a real run": about four times the
smallest change that could matter, so `+/-c` costs about 1 Elo). Margins in
centipawns take S085's 24, a quarter of chesso's 94-point pawn, cut where a
coefficient multiplies a depth (`FutSlope` 16) or the range is narrow
(`LmrDeeperMargin` 8); integer plies and depths take 1; depth caps over 0..16
take 2 (S085's `AspirationMinDepth`); `LmpBase` 100 is one move and
`LmpDepthCoeff` 25 a quarter move a reduced ply; the SEE coefficients 12 and
16 follow where their bars cross a 100-point pawn class; `AspirationWidenPct`
25; `LmrBase` and `LmrDivisor` keep S085's 24 and 32 (DEC-269). Where 4x would put the first iteration's `2 * c0 = 4.11 * c_end` past the
range, `c_end` is cut to fit (`Config.warnings()`). The seven axes on 0..2,
0..3 and 0..4 take 0.5, the driver's floor: one ply already spans most of
their range, so **they are perturbed by a whole ply both ways every
iteration**, the coarsest perturbation in the lane, and the five on 0..2 warn
at iteration 0 (2.05 against 2). No cap below 4x for a mate cliff: no included
axis has a documented one, and item (4) is where mates are checked.

**A depth-gate caveat, recorded and not corrected.** `SeMinDepth` 10 and
`ProbCutMinDepth` 8 sit near the top of a 2+0.02 tree (S085 measured median
depth 11, p10 8, on a slower engine), so those axes are exercised less than
at 8+0.08. A slower lane control buys one ply of median depth for 2.2x the
wall (S085's table) and leaves DEC-222 (8)'s size. The 25 % read records the
median search depth if the lane's PGN move comments carry it.

### (3) The launch

Files: **`tools/spsa_s261.json`** (the frozen input; `_not_here` and per-axis
`why` carry the reasons) and **`adocs/data/S261_spsa.sh`** (the runner, S231's
shape: banner with head, dirty count, load and governor; rebuilds
`build-tune`; `check` into its own log with the marker filtered; then the
driver, and its own `SPSA-FAILED` on every path the driver cannot cover).

Phase 2, in this order, on the idle workstation on mains, `bash` (in fish,
`bash -c '...'`), from `/home/max/ws/chesso` after S261's pre-registration is
committed there:

```bash
cat /proc/loadavg; ps aux | sort -rnk3 | head      # idle: 1-min load well under 1
cmake --build build-tune -j12
python3 tools/spsa_driver.py check tools/spsa_s261.json --engine build-tune/src/chesso \
  | tee .tuning/coord/S261/check.txt              # item (1)'s refusal rule if it fails
python3 adocs/data/S261_split_sim.py | tee .tuning/coord/S261/split_sim.txt
                                                   # last line S261-SPLIT-SIM: ONE LANE | SPLIT
nohup adocs/data/S261_spsa.sh > .tuning/spsa_s261.log 2>&1 &
echo $! > .tuning/spsa_s261.pid
```

**Terminal markers**: `SPSA-DONE` (the driver's last line, after the rounded
JSON vector) or `SPSA-FAILED` (the driver's, or the runner's with a reason).
The first marker in the log is the run's.

**The watcher**, armed through `Monitor` with `persistent: true`; four exits --
marker (0, and the line says which), the runner's death without a marker (3),
the 64800 s ceiling (124), a manual stop that is a belt:

```bash
bash -c '
  end=$(($(date +%s)+64800))
  until grep -qE "SPSA-(DONE|FAILED)" "$1"; do
    if ! kill -0 "$2" 2>/dev/null; then
      grep -qE "SPSA-(DONE|FAILED)" "$1" && break
      exit 3
    fi
    [ "$(date +%s)" -ge "$end" ] && exit 124
    sleep 60
  done
  grep -m1 -E "SPSA-(DONE|FAILED)" "$1"' _ \
  /home/max/ws/chesso/.tuning/spsa_s261.log "$(cat /home/max/ws/chesso/.tuning/spsa_s261.pid)"
```

**Abort rule**: a time-forfeit rate over 1.0 % on either side
(`python3 tools/forfeit_report.py <OUT>/games.pgn --max-pct 1.0`),
`SPSA-FAILED` other than the transient kind the resume clause allows once,
loss of mains, a second load. Nothing else; a trajectory that looks sick is
read, never adjusted.

**The resume clause** (added on the fast check, before launch). One resume is
allowed, and only when all four hold: (a) the run log's failure is the
driver's `ERROR: fastchess exited <n>` or `ERROR: fastchess did not finish
inside <s> s` -- a transient harness failure -- and not a config or run error
of any other wording (`games came back` is a missing game, not transient);
(b) no crash, disconnect or illegal move in the run's games, by
`fastchess.sh`'s own test: `grep -h '^\[Termination' <OUT>/games.pgn | grep
-cvE '"(normal|adjudication|time forfeit)"'` prints 0; (c) the forfeit rate
is under the abort rule's 1.0 % a side; (d) `<OUT>/checkpoint.json` exists.
Then, from `/home/max/ws/chesso`:

```bash
RESUME=1 OUT=<the failed run's directory> \
  nohup adocs/data/S261_spsa.sh > .tuning/spsa_s261_resume.log 2>&1 &
echo $! > .tuning/spsa_s261_resume.pid
```

The runner plays the run's own `engine.snapshot`, re-runs `check` against that
binary, writes `<OUT>/resumed`, and calls the driver with `--resume`, which
refuses a checkpoint whose digest is not this config's and drops a
trajectory row the checkpoint does not know about. Its markers are the same
`SPSA-(DONE|FAILED)`, first one in the new log. The watcher is re-armed on
`.tuning/spsa_s261_resume.log` and `.tuning/spsa_s261_resume.pid`, ceiling
64800 s again. The failed iteration's games may stand twice in `games.pgn`;
the trajectory is the record. **A second failure of any kind aborts the lane**
and the runner refuses a directory that already holds `resumed`.

**A fresh run never reuses a directory**: the runner refuses an `$OUT` that
exists (an inherited `OUT` naming an old run would reuse its
`engine.snapshot` and append to its trajectory), unless `RESUME=1` under the
clause above.

**Mid-run reads** at about 25 % (iteration 312, about 2 h 10 m in) and 50 %
(625, about 4 h 20 m): `c_scale` decaying towards 1, `y` centred with real
spread, `python3 adocs/data/S261_bounds.py <OUT>` for each axis's share at a
bound, the forfeit count. Recorded whatever they say; only the abort rule acts.

**Open findings named (DEC-171)**, re-read at `1edb3b1`: S278, the sanitizer
build's trace tests failing under UBSan on `eval_trace::sink`'s store --
tooling, trace-only, bench-identical, and landed by the coordinator **before**
the lane launches, so the lane's head carries it. S259 is closed. Anything
opened since is named in the launch note.

### (4) The end of a range, and the mate suites

**Reading**: `python3 adocs/data/S261_bounds.py <OUT>` at the end -- per axis
the incumbent, the final rounded value, the share of iterations at each
bound, `moved` and `END` flags. Every line goes into "As built" whatever it
says.

**The rule for an axis whose final value is a bound** (S085's `RfpMinPly` at 0
is why it exists):

- **R1, the incumbent was the bound** (`LmpDepthCoeff` seeded at its floor and
  still there): not a move; recorded.
- **R2, the bound is the axis's declared off value** -- 0 for the five LMR
  terms, the five depth caps (rows 12, 15, 16, 18, 20), `SeeLmrExtra` and
  `AspirationFailLowPull`; 126 for `LmrDeeperMinReduction`; 27000 for
  `LmpBase`; 48000 for `FutBase`: **the lane does not switch a rule off.** The
  axis is held at its incumbent in the candidate, and the reading is recorded
  as evidence for a removal priced by its own SPRT. A rule turned off inside a
  32-axis vector would ship dead code under a verdict that cannot
  attribute it, against a verdict that kept the rule.
- **R3, any other bound** (a margin at 0, `SeMinDepth` at 4 or 16, a range top
  that is not off): the fitted value is in the candidate **only if** the mate
  suites and the full fast suite are green on the vector; it is named in the
  SPRT script's vector table as "at its range end", and the range itself is
  S127's question, never changed inside this step.
- **R4, a mate suite red on the candidate**, whatever caused it: find the axis
  by halving -- reset half the moved axes to their incumbents, rebuild,
  re-run the three suites, about a minute a round -- hold the culprit(s) at
  the incumbent, record the failing cases, re-check. A mate test is never
  edited or relaxed (TESTS).
- **R5, a stuck lane**: if the rounded vector after R1 to R4 equals the
  incumbent, nothing lands and no SPRT is owed (S085's rule); the step records
  that the axes were not findable at this budget, which is not a statement
  that they are right.

**The mate suites on the tuned vector, before any match.** Write the vector
into `src/search_params.hpp`'s defaults (a script in
`adocs/data/S231_apply_fit.py`'s shape, from the driver's JSON block, never
retyped) and `tests/test_search_params.cpp`'s pins, then:

```bash
cmake --build build -j12 && \
  ctest --test-dir build -R '^test_mate_(breadth|pv|carry)$' --output-on-failure
```

then the TESTS gate in both builds (`export CLANG_FORMAT_MAJOR=22` here,
DEC-146), which carries `test_search`'s "pruning does not hide a forced mate"
and `test_engine`'s "engine: mate safety":

```bash
cmake --build build -j12 && ctest --test-dir build -L fast --output-on-failure && \
  cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast --output-on-failure && \
  ./clang-format.sh --check
```

and DEC-141's second tier, since the search's behaviour moves: the Debug
self-play of DEV_MANUAL "The Debug self-play habit" (four rounds at 4+0.04,
`grep -c Assertion` 0), `tools/mutation_check.py` over the search mutants, and
`tools/gate_extra.sh`.

### (5) The verification SPRT

**`adocs/data/S261_sprt.sh`**, in `adocs/data/S119_sprt.sh`'s shape, `CAND`
and `REF` left `PIN_ME` and refused until pinned. **Independent**: the lane
tuned at 2+0.02 on `UHO_4060_v3.epd` against itself; this plays 8+0.08 on
`noob_3moves.epd` against the untuned tree, sharing no opening, control or
game. `{0, 5}` nElo, alpha = beta = 0.05, `model=normalized`, Hash 16,
concurrency 12.

- **Pair**: CAND the commit landing the vector (after item (4)), REF its
  parent; `git diff <lane head> <REF> -- src/` must be empty, or stop and ask.
  S278 lands before the launch, so the lane head carries it. **One exception,
  fixed now**: a src/ change landed between the launch and the vector in a
  commit of its own that proved INV-6 -- `bench`'s total and all eight replies,
  and `tools/search_bench.py`'s node counts and best moves at depths 9 and 12,
  identical to its parent's, with a `No functional change` trailer -- does not
  count, because the engine fitted and the engine REF builds search
  identically. A later trace-only landing cannot stall the verification.
- **Cost (DEC-143)**: 41861 expected games at the midpoint, 25591 on a bound,
  at 2110 games an hour **19.8 h and 12.1 h**; cap 40000 games, 19.0 h. A
  night. Watcher ceiling 136800 s, twice the 19.0 h cap.
- **Abort**: forfeits over 1.0 % a side; a crash or disconnect voids it
  (`SPRT-RUN-INVALID`); mains or a second load.
- **H1**: the vector ships, "at least 5 nElo". **H0**: nothing ships -- CAND
  is reverted, the zero is recorded. **No verdict** at the cap: a zero, read
  and reverted as H0. No second pair.
- The closing commit carries DEC-220's block; the log is trimmed into
  `adocs/data/S261_sprt.log`.

### (6) The block-boundary readings

Both after the verdict has landed or been reverted, both estimates read beside
the verdict and never verdicts (DEC-202 (3), DEC-108). Both under four hours,
so daytime runs (DEC-155), about 4 h 40 m back to back.

**The drift point, S199's form, in S267's series against `1680439`** (S267
accepts (5): the readings before the gate "use the same reference, regime and
series"). HEAD after the verdict's commit, the tree clean, the binary rebuilt
for the identity check:

```bash
cmake --build build -j12
nohup env REF=1680439 ROUNDS=1000 \
  OUT=/home/max/ws/chesso/.tuning/s261_drift_$(date +%Y%m%d_%H%M%S) \
  ./fastchess.sh > .tuning/s261_drift.log 2>&1 &
echo $! > .tuning/s261_drift.pid
python3 adocs/data/S199_drift.py <OUT> .tuning/s261_drift.log \
  --tsv adocs/data/S267_drift.tsv --verdicts <ids> --conversions <ids>
```

2000 games at 2110 an hour, **57 minutes**; watcher ceiling 7200 s, markers
`SPRT-RUN-(DONE|FAILED)`. **`adocs/data/S267_drift.tsv` does not exist** --
S267 names the series and nothing has created it -- so this point creates it
with `S199_drift.tsv`'s column header and a comment naming the reference
(`S199_drift.py` refuses to append to a missing file). Resolution at 1000
pairs: about +/- 15.2 nElo, +/- 11.6 Elo.

**The DEC-202 reading at 32+0.32, Hash 64**, on the SPRT's own pair, so the
two runs play the same two engines at two controls (S151's and S109's form):

```bash
nohup env CAND=<the SPRT's CAND> REF=<the SPRT's REF> TC=32+0.32 HASH=64 ROUNDS=1000 \
  OUT=/home/max/ws/chesso/.tuning/s261_ltc_$(date +%Y%m%d_%H%M%S) \
  ./fastchess.sh > .tuning/s261_ltc.log 2>&1 &
echo $! > .tuning/s261_ltc.pid
```

2000 games at the 534 an hour measured here at this control (S151 read 545),
**3 h 45 m**; watcher ceiling 27360 s (S109's). **Taken after the SPRT
whatever its verdict, DEC-269 (2)**: DEC-202 alone binds only a verdict that
moves a parameter, but DEC-222 (8) and S127's amendment record the risk that a
real longer-control gain reads zero at 8+0.08, and this run is the only place
it shows. It ships nothing on any outcome; a reading after an H0 whose
interval sits above zero opens a decision and never brings the vector back by
itself. On a stuck lane (R5) there is no SPRT, no pair and no run.

Each of the three runs in items (5) and (6) gets item (3)'s watcher with its
own log, pid file, `SPRT-RUN-(DONE|FAILED)` markers and ceiling: 136800 s,
7200 s and 27360 s.

## As built

Phase 1 only, 2026-10-13. Nothing built, run, probed or simulated.

**Written**: `tools/spsa_s261.json`, `adocs/data/S261_spsa.sh`,
`adocs/data/S261_sprt.sh`, `adocs/data/S261_split_sim.py`,
`adocs/data/S261_bounds.py`, this pre-registration. No `src/` change.

**Deviations from the brief**: two scripts it did not ask for --
`S261_split_sim.py` turns item (2)'s one-lane claim into a measurement before
launch, and `S261_bounds.py` is item (4)'s reading; both untested, phase 2's
first run of each is its test. The drift series item (6) appends to does not
exist yet and is created by this step.

**Pending, phase 2** (the coordinator runs every one):

- ~~the reachability check and item (1)'s refusal rule; the split test~~ --
  done 2026-10-13, below;
- ~~the launch, the watcher, the two mid-run reads~~ -- launched 12:39, the
  reads below;
- **everything from `SPSA-DONE` on is the run sheet**, written blind
  2026-10-13 while the lane played:
  `/home/max/ws/chesso/.tuning/coord/S261_files/phase2_runsheet.md`, steps 0 to
  17 in order, each with its command, expected time and pass condition. In
  brief:
  - the final bounds read and the R1 to R5 decision (its table gives each
    axis's rule at either end);
  - `adocs/data/S261_apply_fit.py`, the mate suites, R4's halving if red, the
    gate, DEC-141's tier (Debug self-play, `gate_extra.sh`, a targeted
    mutation pass on the landing commit);
  - `--params`, the DEC-142 goldens, bench, the landing commit, the pinned
    SPRT with its watcher, the verdict's DEC-220 block and ledger;
  - the drift point, which creates `adocs/data/S267_drift.tsv`, and the
    32+0.32 reading.

  Its "Ambiguities" section lists the four choices item (4) leaves to the
  coordinator.

**The two questions this phase raised are decided, DEC-269** (2026-10-13, by
the coordinator under the owner's standing instruction): `LmrBase` and
`LmrDivisor` join the lane, 32 axes, amending accepts (1); the 32+0.32 reading
is taken after the SPRT whatever its verdict. Items (1), (2), (6), the JSON,
the runner, the SPRT header and the split test carry both.

### Phase 2, steps 1 to 3: reach and split, 2026-10-13

**The tree.** achesso at `1edb3b1`: S119 read H1 and the cluster table stays;
S263, S259, S276, S264 and S277 have landed. The pre-registration was applied
onto it in the worktree `chesso-s261b`. `git diff 4f8b949 1edb3b1 --
src/search_params.hpp tests/test_search_params.cpp` is empty, so every seed and
range in item (1) still holds. Built `build-tune` fresh there
(`-DCHESSO_TUNE=ON`, Release, ccache, `-j12`), with S278 and S099 building
beside it -- load 7 to 10 on 12 threads. Nothing below is a timing, and
nothing used the clock rung, so the load does not enter.

**Reach.** `python3 tools/spsa_driver.py check tools/spsa_s261.json --engine
build-tune/src/chesso` over the 32: **31 reach, 1 refused**, 2.7 s
(`.tuning/coord/S261/check.txt` in the worktree). 29 separate at depth 9 on
the midgame position, `SeTtDepthMargin` (0 -> 83007, 8 -> 105644) and
`SeMarginPerDepth` (1 -> 192203, 18 -> 97207) at depth 13 as item (1)
expected, **none on the clock rung**. Every expected rung in the table held.

**`SePlyFactor` refused**, the one item (1) expected: 2 and 8 searched 32829
nodes at depth 9, 106091 at depth 13, 148755 on the tactical position at depth
13, and 195966 on the 1 s clock -- the "same number of nodes under every
probe" verdict, not "not repeatable". `grep -n SE_PLY_FACTOR src/*.cpp` finds
it read in `src/search.cpp` `negamax_at`'s singular-extension guard, `ply <
SE_PLY_FACTOR * depth`, so it is the parameter and not the wiring (DEC-200's
shape, no BUGS step). **The row left `tools/spsa_s261.json`**; `_what` says
why. The re-run on the edited config: 31 of 31 in 1.9 s, every probe line
identical to the first run's, `SPSA-DONE` (`check2.txt`). **The lane is 31
axes.**

**Split.** `python3 adocs/data/S261_split_sim.py` on the 31-axis config, 4.3 s
(`.tuning/coord/S261/split_sim.txt`): over the 16 even-indexed axes weighted
in both, all 31 weighted recovered **+0.692** of each axis's 3 Elo (seeds
+0.660, +0.704, +0.693, +0.670, +0.732) against **+0.703** with the 15
odd-indexed axes at weight zero (+0.687, +0.652, +0.709, +0.695, +0.772).
**Ratio 0.984 against the 0.8 line: `S261-SPLIT-SIM: ONE LANE`.** The
cross-talk item (2) priced by arithmetic costs about 1.6 % of per-axis
recovery on the simulator, inside the seed-to-seed spread of either mode. The
config is not rewritten, and the split item (2) stated is not used.

**The untested scripts, on their first runs.** `S261_split_sim.py` ran clean
and needed no change: 1250-row trajectories, 31 weights with 16 non-zero in
`half` mode, the draw rate 0.3072. `S261_bounds.py`, run over the simulator's
`all_seed1` trajectory, read 1250 iterations and printed every axis with its
flags, also unchanged. `bash -n` passes on both runners, and `S261_sprt.sh`
refuses its `PIN_ME` pair with `SPRT-RUN-FAILED` and exit 1, as written.

**Open findings re-read at `1edb3b1`** (DEC-171), for the lane and the SPRT:
**S278** is open unless it lands first -- the sanitizer build's trace tests
fail under UBSan on `eval_trace::sink`'s store; tooling, never compiled into
`chesso`. S259 is closed. S099 is in progress and lands after this lane
(DEC-271). Both script headers now say so.

**Launch-ready, not launched.** The coordinator launches once S278's and
S099's agents have left the machine, which must then be idle and on mains.
From `/home/max/ws/chesso`, after this branch has landed there (bash):

```bash
cat /proc/loadavg; ps aux | sort -rnk3 | head
nohup adocs/data/S261_spsa.sh > .tuning/spsa_s261.log 2>&1 &
echo $! > .tuning/spsa_s261.pid
```

The runner rebuilds the main tree's `build-tune`, prints its sha256 and runs
the same `check` again (DEC-200: on the day), then plays. The watcher is item
(3)'s loop, unchanged, with `persistent: true`:

```bash
bash -c '
  end=$(($(date +%s)+64800))
  until grep -qE "SPSA-(DONE|FAILED)" "$1"; do
    if ! kill -0 "$2" 2>/dev/null; then
      grep -qE "SPSA-(DONE|FAILED)" "$1" && break
      exit 3
    fi
    [ "$(date +%s)" -ge "$end" ] && exit 124
    sleep 60
  done
  grep -m1 -E "SPSA-(DONE|FAILED)" "$1"' _ \
  /home/max/ws/chesso/.tuning/spsa_s261.log "$(cat /home/max/ws/chesso/.tuning/spsa_s261.pid)"
```

**Expected 8 h 45 m** (S222 8 h 37 m 31 s and S231 8 h 40 m 45 s for this
shape), **ceiling 64800 s, 18 h**. Mid-run reads at about 2 h 10 m and 4 h
20 m.

**The coordinator's fast check, 2026-10-13, before launch** -- four fixes, writing
only, nothing in it invalidating the lane:

1. S278 (trace-only, bench-identical) lands before the launch, and item (5)
   and `S261_sprt.sh` now pre-register that a src/ landing proved INV-6 in its
   own commit does not count against the REF rule.
2. The resume clause in item (3): one resume after a transient `SPSA-FAILED`
   (`fastchess exited` or `did not finish`, no crash, disconnect or illegal
   move, forfeits under 1.0 %), with its command, marker and re-armed
   watcher; a second failure aborts.
3. `S261_spsa.sh` refuses an existing `$OUT` unless `RESUME=1`. On resume it
   plays and checks the run's own `engine.snapshot`, writes `$OUT/resumed`,
   refuses a directory that already holds it, and passes `--resume`.
4. Stale text: item (3)'s open findings name S278, not S259; the JSON's
   `_what` says thirty-one.

`bash -n` passes on both runners. The refusal paths were not executed: this
fix was writing only, so they are untested until the launch.

### The lane, launched 2026-10-13 12:39, and its mid-run reads

Launched by the coordinator from head `932ee7a`. S278 had landed before it,
and no `src/` commit has landed since, which is the SPRT's REF rule (item (5)).
The runner's banner read `mode fresh`. Run dir
`.tuning/spsa_s261_20261013_123913`, pid 520613, expected end about 21:35.
The two reads were taken by the coordinator per item (3), recorded whatever
they said; only the abort rule acts, and it did not fire.

| read | iteration | c_scale | r_k | y mean / sd / zeros | forfeits |
|---|---|---|---|---|---|
| 25 % | 313 (about 14:55) | 1.1497 | 0.00602 | -0.268 / 6.14 / 7.0 % (iterations 0-312) | 0 |
| 50 % | 626 (17:08) | 1.0722 | 0.00500 | +0.238 / 5.57 / 9.2 % (313-627); all 0-627 -0.014 / 5.86 / 8.1 % | 0 of 30113 games |

`c_scale` matches S222's 1.150 and 1.072 at a quarter and a half.

**Search depth**, item (2)'s caveat, from the PGN's move comments: the first
15024 games read median 12, p10 10, p90 15; all 30186 at the half read median
11, p10 10, p90 15. `SeMinDepth` 10 and `ProbCutMinDepth` 8 sit inside that
range, so both are exercised.

**Bounds** (`S261_bounds.py` over the truncated trajectory). At 25 %, `END` on
`LmrCutNode` (0, 45.4 % of iterations at min) and `LmrDeeperMinReduction` (1,
40.9 %); `LmpDepthCoeff` had left its floor for 7 (13.4 % at 0). At 50 %:

```
QsFutilityMargin            188 ->    213  at min   0.0 %  at max   0.0 %  moved
RazorMargin                 282 ->    258  at min   0.0 %  at max   0.0 %  moved
LmrCutNode                    1 ->      0  at min  72.8 %  at max   0.0 %  moved END
LmrNotImproving               1 ->      1  at min   0.0 %  at max   0.0 %
LmrTtCapture                  1 ->      1  at min   0.0 %  at max   0.0 %
LmrPv                         1 ->      1  at min   0.0 %  at max   0.0 %
LmrNoTtMove                   1 ->      1  at min   0.0 %  at max   5.2 %
LmrDeeperMargin              47 ->     47  at min   0.0 %  at max   0.0 %
LmrDeeperMinReduction         2 ->      1  at min  70.6 %  at max   0.0 %  moved END
LmpBase                     733 ->    996  at min   0.0 %  at max   0.0 %  moved
LmpDepthCoeff                 0 ->     18  at min   6.7 %  at max   0.0 %  moved
LmpMaxLmrDepth                8 ->      6  at min   0.0 %  at max   0.0 %  moved
FutBase                     147 ->    171  at min   0.0 %  at max   0.0 %  moved
FutSlope                    170 ->    195  at min   0.0 %  at max   0.0 %  moved
FutMaxLmrDepth                8 ->     13  at min   0.0 %  at max   0.0 %  moved
HistPruneMaxLmrDepth          8 ->      8  at min   0.0 %  at max   0.0 %
SeeQuietCoeff                50 ->     19  at min   0.0 %  at max   0.0 %  moved
SeeQuietMaxLmrDepth           8 ->      9  at min   0.0 %  at max   0.0 %  moved
SeeCaptureCoeff              50 ->     30  at min   0.0 %  at max   0.0 %  moved
SeeCaptureMaxLmrDepth         8 ->      9  at min   0.0 %  at max   0.0 %  moved
SeeLmrExtra                   1 ->      1  at min  31.5 %  at max   0.0 %
SeMinDepth                   10 ->      7  at min   0.0 %  at max   0.0 %  moved
SeTtDepthMargin               4 ->      2  at min   0.0 %  at max   0.0 %  moved
SeMarginPerDepth              9 ->      1  at min  48.6 %  at max   0.0 %  moved END
ProbCutMargin                49 ->     76  at min   0.0 %  at max   0.0 %  moved
ProbCutDepthOffset            5 ->      3  at min   0.0 %  at max   0.0 %  moved
ProbCutMinDepth               8 ->      7  at min   0.0 %  at max   0.0 %  moved
AspirationWidenPct          200 ->    173  at min   0.0 %  at max   0.0 %  moved
AspirationFailLowPull         2 ->      3  at min   0.0 %  at max   0.0 %  moved
LmrBase                      52 ->     86  at min   0.0 %  at max   0.0 %  moved
LmrDivisor                  182 ->    173  at min   0.0 %  at max   0.0 %  moved
```

At the half, three axes sat at a range end, and item (4) decides each only at
the end:

| axis | value | share at min | rule if it ends there |
|---|---|---|---|
| `LmrCutNode` | 0, its off value | 72.8 % | R2, held at 1 |
| `LmrDeeperMinReduction` | 1, its min, not its off value 126 | 70.6 % | R3 |
| `SeMarginPerDepth` | 1 | 48.6 % | R3 |

`SeeLmrExtra` was back at 1 after 31.5 % at its off value 0. The source is
the coordinator's `.tuning/coord/S261_files/midrun_reads.md`, which also holds
the 25 % table in full.

### Written blind for the end of the lane, 2026-10-13

`adocs/data/S261_apply_fit.py`, in `S231_apply_fit.py`'s shape, written while
the lane held the machine. `python -m py_compile` passes; it has never run.

**Where the vector comes from.** It reads `checkpoint.json` and rounds with
the driver's own `Param.uci_value`. The checkpoint is the one file that
proves the run finished: `k` equal to 1250, and the config digest the driver
computes.

**Refusals.** It refuses, by name:

- a `run.json` whose config is not the tree's;
- a trajectory whose last row disagrees with the checkpoint;
- an axis whose current default, pin or manual cell is neither its incumbent
  nor its fitted value -- the tree moved under the lane;
- any range that differs from the config's;
- an R2 axis not in `--hold`, with exit 2 and no writing;
- a `--hold` name that is not in the config.

**What it writes.** The defaults in `src/search_params.hpp` -- one X-macro row
per axis, which is both builds' form -- the default pins in
`tests/test_search_params.cpp` (ranges asserted, never written) and
`MANUAL.md`'s option-table default cells. It keeps the table's columns and
continuation backslashes, prints the incumbent / fitted / applied / rule table,
names the prose and the search goldens that move by hand, and takes
`--dry-run`. It can be re-run with a different `--hold`, which R4's halving
needs. The run sheet above is where it is used.

### The mate guards judged as families, DEC-277 -- the comparison fixed before the numbers

Written 2026-10-13, before any grid below was run, and **not edited after it**.
DEC-277 replaces DEC-276 (A2)'s and R4's cell-wise reading for this step.

**The holds restart from R2.** `LmrCutNode` and `SeeLmrExtra` are held, and
`LmrDeeperMinReduction` is held at 2: at 1 its guard says nothing, which is
R2's case, and A4 withholds an R3 value whose fast suite is red. The `FutBase`
and `LmrDivisor` holds found cell-wise are dropped.

**The two trees.**
- **Candidate:** that vector in `chesso-s261c`.
- **Incumbent:** achesso's head `fd9e44c`, whose `src/` is `932ee7a`'s (the
  defaults the lane started from), built in a worktree of its own.
- A script that builds from a ref is given `fd9e44c` for the incumbent and a
  `git stash create` commit of the candidate's working tree, which moves no
  ref.

| family | guard it judges | grid, per tree | total compared | noise floor | pass |
|---|---|---|---|---|---|
| F1 carry | `test_mate_carry` | `adocs/data/S203_case_sweep.sh` full grid: 6 cases x strides 1, 2 x budgets 100000 .. 4000000 (9) = 108 cells, `ENGINE` set per tree | mate lines summed over the 108 cells; short lines summed | 0 (the script measures none) | cand mates >= inc mates, **and** cand short <= inc short |
| F2 mate in three | `test_engine` `exact_by_distance[3] >= MATE_IN_THREE_FLOOR` | `S154_floor_margin_sweep.py here --engine <tree>` | exact mates in three | S154 `hash` on both trees, Hash 1 .. 256: the larger of the two spreads (max - min) | cand >= inc - noise |
| F3 breadth | `test_mate_breadth` | `S156_mined_floor_sweep.py --ref <tree>`, depth 10, Hash 16: the shipping `RfpMinPly` row | exact mates of the 318 | 0 -- S156 measures none, and S154's `hash` probe is over the constructed set, not this one | cand >= inc |
| F4 mined rows and the S230 table | `test_search` "pruning does not hide a forced mate" | the depth each row's mate is first found, in its own script's sweep mode on each tree (below) | per row: first-found depth, or none | -- | every row's mate found somewhere in its script's range on the candidate |

**F4's rows and the range each script sweeps:**

| row | script | candidate range | incumbent range |
|---|---|---|---|
| "mate the extra ply hides" | `S230_mine_r01_row.py depths`, as the row's site names | 3 .. 12 | 3 .. 12 |
| the S230 table, 4 rows | `S230_mine_r01_row.py depths --fens adocs/data/S230_table_fens.txt` | 3 .. 12 | 3 .. 12 |
| "mate the multicut hides" (E21) | `S097_mine_mate_row.py sweep`, `SeMinDepth + 1` .. `+ 4` | 7 .. 10 | 11 .. 14 |
| "mate probcut hides" (B04) | `S277_mine_mate_row.py sweep`, `ProbCutMinDepth + 1` .. `+ 4` | 7 .. 10 | 9 .. 12 |

**Not families, and green literally or not at all:**
- `test_mate_pv`, which is completeness, not a count;
- `test_engine`'s mate-in-two on-time clause (`first_exact == 2m - 1`, all 16),
  which S154 `here` also reports;
- "reverse futility at ply 2 does not hide a mate";
- every rule-guard fixture.

**Then, every family passing**, each golden is re-derived by its own script at
the final vector, and the family that cleared it is named at its site:

| golden | script |
|---|---|
| S170 budgets, and the carry ceilings | `S203_case_sweep.sh`, then `--ceilings` |
| the mate-in-three floor | `S154 floor`, then `red` |
| the breadth floor | `S156` |
| `first_mate_depth` | `S188_repair_goldens.py first-mate` |
| the mined rows | in guard mode (DEC-238), each mutant killed through its row |
| the S230 table's depths and labels | `S230 depths` per mutant |

A row no script can re-mine to kill its mutant holds its culprit (DEC-276
(A2)). A family that fails is halved on its grid total, never on a cell.

### The family readings, 2026-10-13 night

Raw files: `.tuning/coord/S261/fam/` in `chesso-s261c`.

**Round v2: holds `LmrCutNode`, `SeeLmrExtra`, `LmrDeeperMinReduction` (DEC-277 (1)).**

| family | candidate | incumbent `fd9e44c` | noise floor | reading |
|---|---|---|---|---|
| F1 carry, 108 cells | 1373 mates, 11 short | 1305 mates, 24 short | 0 | pass |
| F2 mate in three | 10 / 24 | 12 / 24 | 0 (S154 `hash`: spread 0 on both trees) | **fail** |
| F3 breadth | 161 / 318 | 150 / 318 | 0 | pass |
| F4 rows | E21, B04 not found in 7..10 | all in range | -- | **fail** |

**F2 halved on its total.** Through the candidate's tune build, each hold set
by setoption; `f2_hold.py` drives S154's own `measure()`. The tune build with
no option reads 10, as the release build does.

| hold set | mates in three |
|---|---|
| all 22 moved axes | 12 |
| half A | 12 |
| half B | 12 |
| quarter A1 | 12 |
| quarter A2 | 10 |
| quarter B1 | 11 |
| quarter B2 | 11 |
| any half of A1 | 11 |
| A1's pairs: `QsFutilityMargin` + `LmpBase` | 12 |
| `LmrDivisor` alone (the only single of the 22 that restores it) | 12 |

The descent is not monotone: B2 contains `LmrDivisor` and reads 11. **The
smallest set that restores the total is `LmrDivisor` alone, and it is held.**
The full log is `F2_halving.txt`.

**Round v3: holds `LmrCutNode`, `SeeLmrExtra`, `LmrDeeperMinReduction`, `LmrDivisor`.**

| family | candidate | incumbent | reading |
|---|---|---|---|
| F1 carry | 1485 mates, 16 short | 1305 mates, 24 short | pass |
| F2 mate in three | 12 / 24, m2 on time 26 / 26 | 12 / 24, 26 / 26 | pass |
| F3 breadth | 160 / 318; weakened guard 156 | 150 / 318; weakened guard 145 | pass |
| F4 rows | see below | all in range | **open** |

F4 at v3, mate first found, per row:

| row | candidate | incumbent |
|---|---|---|
| "mate the extra ply hides" | d3-6, d8, d9, d11 | d3 .. d12 but d11 |
| S230 table row 1 | d9 .. d12 | d9 .. d12 |
| S230 table row 2 | d8 .. d12 | d8 .. d12 |
| S230 table row 3 | d11, d12 | d10, d12 |
| S230 table row 4 | d12 | d9 .. d12 |
| E21 | none in its range 7..10; m6 at 11, 12, 14 | d12 .. d14 in its range 11..14 |
| B04 | none in its range 7..10; m6 at 11, 13 | d11, d12 in its range 9..12 |

E21's and B04's ranges are derived from `SeMinDepth` and `ProbCutMinDepth`,
which the vector itself moves from 10 and 8 to 6. Whether F4 reads those rows
against the candidate's range or the incumbent's is put to the coordinator.

**Found on the way, and on the incumbent, not the vector:** S156 on `fd9e44c`
reads the shipping count 150 and the weakened-guard count 145. The breadth
floor of 145 therefore no longer separates the two, and the gate built with
`RfpMinPly` 1 stays green on achesso's head (DEC-142).

### Rebased onto `870d210` (S279), 2026-10-14

The pre-rebase diff was applied 3-way onto `870d210` with no conflict; it is
saved in main as `.tuning/coord/S261/pre_rebase_870d210.diff`. The apply
script's dry run at the final holds reads 0 rows to change and no refusal, so
S279 moved no parameter. The incumbent worktree was checked out at `870d210`
and rebuilt.

**Which families run at a node budget:** only F1 (`S170_replay.py --go
"nodes N"`). F2 and F3 drive `go depth`, and F4's drivers run `search()` with
`NODE_BUDGET_UNLIMITED`. All four were re-taken, candidate against incumbent at
`870d210`:

| family | candidate | incumbent | reading |
|---|---|---|---|
| F1 carry | 1485 mates, 16 short | 1303 mates, 24 short | pass |
| F2 mate in three | 12 / 24, m2 on time 26 / 26 | 12 / 24, 26 / 26 | pass |
| F3 breadth | 160 / 318, weakened guard 156 | 150 / 318, weakened guard 145 | pass |
| F4 rows | E21 at 11, 12, 14 and B04 at 11, inside the incumbent's ranges (DEC-278); the other five rows inside 3..12 | all in range | pass |

The candidate's carry grid is byte-identical to the pre-S279 one; the
incumbent's moved by 2 mates. F2 and F4 are node-identical to the `fd9e44c`
readings, and F3 re-taken at `870d210` reads the same counts on both trees.

**Fixtures re-derived so far:**

- **The four node-type cases:** `NODE_TYPE_DEPTH` 6 -> 7, the next depth;
  all five node-type cases hold their preconditions there.
- **The capture SEE depth cap:** `CAP_DRIVE_DEPTH` 12 -> 13, measured: at 12
  `f3f6` is skipped, at 13 it is searched at index 6 with a reduced depth of
  9, the new cap.
- **The ten ProbCut cases:** green with the singular block isolated by
  construction, and each drive asserts `!probe.se_verified`.

**The two late-capture LMP fixtures cannot be re-derived at this vector** --
reported, not relaxed. Measured over drive depths 3 to 16 on their position:

| drive depth | what happens |
|---|---|
| 3 .. 8 | the LMP flag is set, but Bxf7 is never searched: the capture rule skips it |
| 9 and deeper | the flag is never set |
| 10 | Bxf7 is searched at index 12 with a reduced depth of 4 and clears the margin, but the flag sits at move 47 |

The extra-ply case (LQ02) needs a losing capture past the flag that clears the
capture margin. That cannot exist at any position at these values:

- The flag is set only at a move whose reduced depth is under
  `LmpMaxLmrDepth`, now 3.
- The reduced depth only falls as the move number rises, so any capture past
  the flag has a reduced depth of 2 or less.
- The margin there is at most `SeeCaptureCoeff` 50 x 2 = 100, and the smallest
  loss `see_value` can give a capture is 200.

The skip case (LQ03) needs 9 captures searched ahead of the losing one, where
the flag now sits at move 10 (`LmpBase` 935), and the position gives 8. It
could be re-mined with a new position. LQ02 cannot.

> **CORRECTION, 2026-10-14 -- the paragraph above is wrong and is kept as
> written.** Its last bullet is false: `see_value`'s table is 100 / 300 / 300
> / 500 / 900, and the swap-off produces a loss of exactly **100** -- measured
> with the engine's own `see_ge` on `4k3/8/3p4/4p3/8/5NB1/8/4K3 w - - 0 1`,
> where Nxe5 and Bxe5 pass `see_ge(-100)` and fail `see_ge(-50)`. At reduced
> depth 2 the margin of 100 clears such a capture, so LQ02 is reachable in the
> release build and "LQ02 cannot" is void. DEC-280, written on this reading,
> is void too; **DEC-281** keeps both cases in the release build. LQ02 was
> re-mined by `adocs/data/S261_mine_late_capture.py`: Rxe4 in
> `4k2r/1p3pb1/B6B/2p1pn1p/Pr2PP2/1PN3P1/n1b3N1/R5K1 b - - 0 35` loses exactly
> 100, is searched past the count at reduced depth 2 at drive depth 7, and
> takes its extra ply; LQ03 uses Rxa4 in the same position at depth 3. Both
> mutants are killed by their cases (the mutant pass below).

### Phase 2 completed to the landing, 2026-10-14

**The vector that lands.** 21 of 31 defaults move. The script output is
`adocs/data/S261_apply_fit.py` reading the run's checkpoint (k 1250, digest
c285cbc739dbeae8). Every axis's end and rule:

| axis | incumbent -> fitted -> applied | rule |
|---|---|---|
| `LmrCutNode` | 1 -> 0 -> **1** | R2, held: its off value |
| `SeeLmrExtra` | 1 -> 0 -> **1** | R2, held: its off value |
| `LmrDeeperMinReduction` | 2 -> 1 -> **2** | held: at 1 its guard says nothing, R2's case, and A4 withholds an R3 value whose fast suite is red (DEC-277 (1)) |
| `LmrDivisor` | 182 -> 204 -> **182** | held: the smallest hold that restored F2 (DEC-278, 482cbbe) |
| `LmpDepthCoeff` | 0 -> 0 | R1: seeded at its floor and ended there |
| `FutMaxLmrDepth` | 8 -> **16** | R3, applied: at its range top, not an off value |
| `SeMarginPerDepth` | 9 -> **1** | R3, applied: at its range floor (DEC-276 (A4)) |
| `LmrNotImproving`, `LmrTtCapture`, `LmrPv`, `LmrNoTtMove`, `SeeCaptureCoeff` | -- | unmoved |
| the other 19 | as the SPRT script's table | moved, inside their ranges |

R2 was held even where a co-fitted axis moved beside it (DEC-276 (A1)):
`LmrCutNode` is held at 1 while `LmrBase` goes 52 -> 113.

**Families, DEC-277 (2), re-taken at `870d210`.** These numbers are the
record; the raw files are in the S261 worktree's `.tuning/coord/S261/fam870/`.

| family | candidate | incumbent | noise floor | reading |
|---|---|---|---|---|
| F1 carry, 108 cells | 1485 mates, 16 short | 1303 mates, 24 short | 0 | pass |
| F2 mate in three | 12 / 24, mate in two on time 26 / 26 | 12 / 24, 26 / 26 | 0 (S154 `hash` spread 0 on both trees) | pass |
| F3 breadth | 160 / 318 | 150 / 318 | 0 | pass |
| F4 rows | every row's mate found, E21 and B04 over the incumbent's range (DEC-278) | all in range | -- | pass |

Only F1 runs at a node budget. F2 and F3 use `go depth`, and F4 runs its
drivers unbudgeted.

**Goldens and fixtures, each re-derived by its own script and never read off a
run.** The site of each names the family that cleared it.

| what | moved | how re-derived |
|---|---|---|
| S170 budgets | A `nodes 500000 -> 1000000`, C `1500000 -> 2000000`, E `100000 -> 300000` | `S203_case_sweep.sh`, the cheapest budget reporting a mate line; the grid is `adocs/data/S261_sweep_s170.txt` |
| carry ceilings | F 0 -> **1** (DEC-282, on S261's H1 only); the command now holds S202's and S261's grids | `S203_case_sweep.sh --ceilings` answers 0, 5, 4, 0, 0, 1 |
| `MATE_IN_THREE_FLOOR` | none: stays 11 | S154 `floor`: 12 shipping, 6 weakened; `red` builds the gate at RfpMinPly 1 and sees it red |
| `EXACT_FLOOR` (breadth) | 145 -> **159** | `S156 --floor 159`: 160 shipping, 156 weakened, gate red at 1. This closes the finding on achesso's head, where 145 sat at the weakened 145 and the gate stayed green |
| `first_mate_depth` | 10 -> 9 (first position) | `S188_repair_goldens.py first-mate`: both positions first mate at 9, with a cp iteration under each |
| `capture_mates` | row 3 10 -> 11, row 4 10 -> 12; labels unchanged | the seven sweeps, `capmates.sh` |
| E21 row | `6k1/5pp1/6np/1PP1p3/1Q2P1P1/3r1q1P/1B1R4/6K1 b - - 0 42`, depth 10, mate in 5 | re-mined in guard mode into the range 7..10, `remine.sh` |
| B04 row | `8/5Qk1/p2p4/2pP2q1/2P5/p4p2/6N1/1r2R1K1 b - - 0 61`, depth 10, mate in 5 | same |
| LQ02 and LQ03 | position `4k2r/1p3pb1/B6B/2p1pn1p/Pr2PP2/1PN3P1/n1b3N1/R5K1 b - - 0 35`, Rxa4 at depth 3 (LQ03) and Rxe4 losing exactly 100 at depth 7 (LQ02), preconditions asserted in each case (DEC-281) | `adocs/data/S261_mine_late_capture.py`; the repository's FEN data gave no cell, so the engine's own seeded walks (20405 positions) gave 258 and 9589 |
| `NODE_TYPE_DEPTH` | 6 -> 7 | the four node-type cases' own precondition |
| `CAP_DRIVE_DEPTH` | 12 -> 13 | measured: f3f6 searched at index 6, reduced depth 9, the new cap |
| ProbCut fixtures (10) | none | re-isolated from the singular block by construction, and every drive REQUIREs `!probe.se_verified` (DEC-277 (4)) |

**Checks.**
- **Fast suite:** 46 / 46 in `build` and in `build-tune`, with
  `CLANG_FORMAT_MAJOR=22`.
- **Format:** `clang-format.sh --check` clean.
- **Prose checks:** `plan_prose_check.py --params` silent.
- **Debug self-play:** 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`, 0
  incomplete mating PVs.
- **`bench`:** 3872032 -> **5636835**. Six of eight replies unchanged; the
  second goes d5e6 -> e2a6 and the fifth d8e7 -> a7a6.
- **`tools/gate_extra.sh`:** `GATE-EXTRA-DONE`, 5 stages in 983 s (prose,
  citations, debug, sanitize, perft); prose and citations re-run green after
  the last documentation edit.
- **Mutant pass** (`tools/mutation_check.py --only` on a clean fixture of the
  landing state): **10 of 10 killed** -- E21, B04, R01, R02, C02, C05, C07,
  LQ01, LQ02, LQ03. E21 and B04 are each killed through the re-mined row in
  "pruning does not hide a forced mate", and LQ02 and LQ03 through the
  re-mined late-capture cases. Wall 1288 s; `results.tsv` is in the S261
  worktree's `.tuning/coord/S261/mut_logs/`.

**Proposed `adocs/specs.md` wording, for the coordinator.** All on the search
row, line 668; each proposal keeps the seed beside the fitted value.

| current | proposed |
|---|---|
| `QsFutilityMargin` (188, ...) | (226, S261's fit of the seed 188, ...) |
| `RazorMargin` (282) | (242, S261's fit of the seed 282) |
| `SeMinDepth` 10, `SeTtDepthMargin` 4, `SePlyFactor` 5 and `SeMarginPerDepth` 9 are the settings, each DEC-105 (c) | `SeMinDepth` 6, `SeTtDepthMargin` 2, `SePlyFactor` 5 and `SeMarginPerDepth` 1 are the settings: S261's fit of the DEC-105 (c) seeds 10, 4 and 9, `SePlyFactor` reached by no probe and left out of the lane |
| `ProbCutMinDepth` (8, ...) | (6, S261's fit of the seed 8, ...) |
| `ProbCutMargin` (49, ...) | (87, S261's fit of the seed 49, ...) |
| `ProbCutDepthOffset` (5, so the node-level pair is the paper's (4, 8)) | (3, S261's fit of the seed 5, which made the node-level pair the paper's (4, 8)) |
| `AspirationWidenPct` (200, the doubling S021 shipped) | (155, S261's fit of the doubling S021 shipped) |
| `AspirationFailLowPull` (2) quarters | (3) quarters |

**Pending for the coordinator:**
- the specs wording above, applied by the coordinator in the vector commit
  (F's raise and S279's place in the REF exception are settled by DEC-282);
- the lane's `games.pgn` (180 MB) and `fastchess.log` to the owner's archive
  (DEC-235);
- `plan.md` and `status.md`;
- the commit;
- pinning CAND and REF, and the SPRT.

### Item (5): the verification SPRT, H1, 2026-10-14 (the coordinator)

`adocs/data/S261_sprt.sh`, pinned in `22e8df7`: CAND `1b84f4b` (the vector)
against REF `574480e` (its parent; S279 inside the REF exception by DEC-282).
8+0.08, Hash 16, `noob_3moves.epd`, concurrency 12, launched 03:48, ended
04:17.

**H1**: LLR 2.96 (-2.94, 2.94) over **986 games** (W 374, L 206, D 406,
pentanomial [26, 68, 179, 152, 68]), `Elo 59.78 +/- 16.52`, `nElo 80.09 +/-
21.69`, 28 m 09 s, **0 forfeits** of 987 games in the PGN, no crash or
disconnect (the guard's grep over the console). Pre-registered reading: the
vector gains **at least 5 nElo** over the untuned tree at the harness setting;
the stopping run's Elo is upward-biased (DEC-063) and is not the effect size.
For scale only, not a comparison: S085's lane read +21.02 and S222's +11.13.
The vector ships as CAND carries it. Log `adocs/data/S261_sprt.log`; the run
(console, PGN, fastchess log) is in the owner's archive as
`chesso_S261_sprt_2026-10-14.tar.xz`.

Pending: item (6), the drift point (creates `adocs/data/S267_drift.tsv`) and
the 32+0.32 Hash 64 reading on this pair, both estimates beside the verdict.
