id:         S068
goal:       re-decide the reverse futility margin against a verdict, starting from the 75 that S033 left on the table
accepts:    an SPRT against the commit shipping margin 100 returns a verdict, recorded whatever it is; every mate case in the fast suite stays green at the setting that ships, the material-leader case included
touches:    src/search_params.hpp RFP_MARGIN, tests/test_search_params.cpp golden_defaults
excludes:   the ply bound and the depth bound, which S033 set from measurement and DEC-060 records; any second pruning rule
decisions:  DEC-060, DEC-063
closes:
blocks:
paused_by:
done:      2026-08-17  RFP_MARGIN ships at 75. Two SPRTs, same two Release binaries, same book, tc and adjudication, bounds the only difference.
            Run 1, elo0=0 elo1=5: no verdict. 9036 scored games, 6 h 36 m, Elo +3.19 +/- 5.60, LLR drifting 0.86 -> 0.59 against +/-2.94. Terminated deliberately; a bounds error, not a property of the change.
            Run 2, elo0=-5 elo1=5: SPRT completed, H1 accepted, 2312 games, 1 h 41 m 13 s, Elo +12.18 +/- 11.12, nElo +15.52 +/- 14.16, LLR 2.97.
            Effect estimate is pooled over both runs, 11348 games: score 50.72 %, point Elo +5.02. Run 2's half carries optional-stopping bias upward. Honest reading: a small positive effect, most likely 3 to 5 Elo, demonstrably not a regression of 5 Elo or more.
            Ships with 14.5 % fewer nodes: 1216123 against 1422053 at depth 9, best moves unchanged. Fast suite 13 of 13; all six mate cases green at 75, the DEC-060 material-leader case included.
            DEC-063 is the general lesson and is the step's most reusable output: bounds straddle the expected effect, they do not bracket it from one side.

## Why this exists

S033 shipped `RFP_MARGIN` 100 and said so: a first setting, one pawn per
remaining ply, fitted to nothing. It bought +59.98 +/- 17.24 Elo. The sweep that
chose it also measured margin 75 at the same ply and depth bounds, and 75 is
green on every mate case and visits **1216123 nodes against 1422053**, 14.5 %
fewer, on `tools/search_bench.py` at depth 9.

That is not a reason to ship it. Fewer nodes is not more Elo -- DEC-019 is three
techniques that were quoted a gain and measured zero or worse -- and S033 shipped
the more conservative of two greens deliberately, one change at a time. This step
is where the other one gets its verdict.

## What the sweep already knows

All at ply bound 3, depth bound 6, from `adocs/data/S033_rfp_ply_sweep.tsv`:

| margin | nodes at depth 9 | fast suite |
|---|---|---|
| 75 | 1216123 | green |
| 100 | 1422053 | green, shipping |

The shipping row reproduces exactly. `tools/search_bench.py ./build/src/chesso 9`
at `d7901e3` gives 292313 + 1026739 + 103001 = **1422053**, best moves
`c3d5 e2a6 d7c8q`. The 75 row does not, without the hand edit the next section
describes.

Margins 150, 200 and 300 were measured too, in
`adocs/data/S033_rfp_sweep.tsv`, and prune less, so they are the wrong direction
unless 75 loses. **Do not read node counts across from that file.** It sweeps a
different floor and its rows are not at the shipping configuration; the next
section is why.

## Read the two tables' columns before quoting either

`adocs/data/README.md` states this in full and it is the trap this step's
evidence sets:

- `min_ply` in `S033_rfp_ply_sweep.tsv` is **`RFP_MIN_PLY`**, distance from the
  root. `src/search_params.hpp:70`, value 3 at HEAD -- S073 moved it there from
  `src/search.cpp:47`.
- `min` in `S033_rfp_sweep.tsv` is **`RFP_MIN_DEPTH`**, remaining depth. It is
  in no source file and no commit -- a constant that existed only in S033's
  uncommitted working tree, which is what all three sweeps ran against.

So margin 75 reads green at 1216123 in the ply table and RED at 2886952 in the
other, and both rows say "75" and "3". The 3 names a different constant in each.
Raising the remaining-depth floor to 3 gives up the prunes at depth 1 and 2,
where nearly all the nodes are, which is why the count more than doubles -- and
it stays RED anyway, because the prune that hides the mate is at ply 1 and a
depth floor cannot reach a ply. Raising the ply floor gives up ply 1 and 2, a
handful of nodes, and is what turns the suite green. The two tables agree to the
node where the floors coincide at 1 (1190649 at margin 75, 1398911 at margin
100), which is what says they are the same code measured twice.

**Only `S033_rfp_ply_sweep.tsv` maps onto HEAD.** Its `3 100 6` row is the
shipping engine. `S033_rfp_sweep.tsv` cannot be reproduced at HEAD at all, by a
hand edit or otherwise, because the constant it sweeps is in no commit.

## How to vary the margin at HEAD

**Rewritten 2026-08-16 at `7f15ac4`.** S073 landed between this file being
written and this step starting, and it moved the constant. `src/search.cpp:38`
is no longer a `#define` of anything; the default now exists once, as the third
column of an X-macro row in `src/search_params.hpp:60`:

```
  X(RFP_MARGIN,        "RfpMargin",       75,     0, 2000)
```

At `7f15ac4` that column reads `100`. The working tree carries `75`, uncommitted,
as the SPRT candidate -- see the next section for why that is the only way to
build one.

Two ways to move the number, and they are **not** interchangeable:

- **Source edit, then `cmake --build build -j12`.** Changes the default the
  release binary folds. This is the only one an SPRT may use.
- **`setoption name RfpMargin value <n>` on `build-tune`**, which is configured
  `-DCHESSO_TUNE=ON` (`DEV_MANUAL.md:50-59`). Every parameter is a variable
  there, so a sweep costs one build rather than one build per point. Valid for a
  node count, a fast-suite run or a sweep. **Never for a strength number**
  (`DEV_MANUAL.md:61`).

The source edit has a second half that is not optional.
`tests/test_search_params.cpp:43` pins each default to the value that ships, so
editing `search_params.hpp` alone turns the fast suite red on
`test_search_params` -- by design, and the message says so:

```
/home/max/ws/chesso/tests/test_search_params.cpp:127: ERROR: CHECK( param.default_value == golden_defaults[i].value ) is NOT correct!
  values: CHECK( 75 == 100 )
  logged: [RfpMargin] is 75, not the 100 it shipped at. Changing a value is a step of its own and updates this list in the same commit.
```

Observed 2026-08-16 at `7f15ac4` + the margin edit, g++ 13.3.0. That golden row
moves to 75 in the same commit as the parameter, which is the re-target the test
file's own comment prescribes, not a relaxation. If the verdict keeps 100, both
edits are reverted together.

**`-DRFP_MARGIN=75` still does not compile**, for a new reason. The `#define`
this file used to quote is gone, so there is nothing left to redefine -- but the
symbol is now an identifier the X macro declares, and a command-line macro of the
same name rewrites the declaration itself:

```
$ g++ -fsyntax-only -std=c++20 -Isrc -DRFP_MARGIN=75 src/search.cpp
<command-line>: error: expected unqualified-id before numeric constant
src/search_params.hpp:99:24: note: in definition of macro 'CHESSO_DECLARE_SEARCH_PARAM'
   99 |   inline constexpr int sym = def;
src/search_params.hpp:60:5: note: in expansion of macro 'RFP_MARGIN'
   60 |   X(RFP_MARGIN,        "RfpMargin",       75,     0, 2000)
```

Reproduced 2026-08-16 at `7f15ac4`, g++ 13.3.0. So the sweep scripts in
`adocs/data/` remain historical evidence and not a method to re-run, exactly as
`2026-08-16_plan_review-F04` found. Note what they do with a failed build:
`S033_rfp_sweep.sh` swallows it into a `BUILD_FAIL` row and keeps going, so
re-running one produces a table of failures rather than an error.

## How the two SPRT binaries are produced

**The SPRT compares two shipping binaries, and the tune build is not one of
them.** `setoption name RfpMargin value 75` on a `-DCHESSO_TUNE=ON` build is the
wrong way to produce the candidate: under `CHESSO_TUNE` every parameter is an
`extern int` the compiler must load, and without it an `inline constexpr int` it
folds (`src/search_params.hpp:95-100`). Different code, and the difference shows
up in a timed match rather than in a node count -- S073 records this
(`plan_done/S073_search_params_tune_build.md:71-82`), `DEV_MANUAL.md:61` states
it as a rule, and S085 carries the same constraint for SPSA.

So both sides are ordinary Release builds:

- **candidate** -- `src/search_params.hpp:60` set to 75, `cmake --build build
  -j12`, left uncommitted in the working tree. `fastchess.sh:24` reads
  `build/src/chesso` and `fastchess.sh:82-83` snapshots it before the first game,
  so a later rebuild cannot swap the engine mid-match.
- **reference** -- `7f15ac4`, the commit shipping margin 100. `fastchess.sh:93-105`
  builds it from a detached worktree under `.ref-builds/7f15ac4/`, Release with
  ccache, so what the number means is attributable to a commit range. `REF`
  overrides the script's own default of `7b4d9a4` and must be set: the default is
  many commits stale for this purpose.

## The command

```
REF=7f15ac4 ./fastchess.sh
```

Detached, with a watcher that outlives the turn (DEC-061). Full bounds, not
`--fast`: `--fast` is `elo0=0 elo1=10` (`fastchess.sh:53`) and its H0 region
therefore contains both answers this step has to tell apart -- a margin worth
shipping at +3 Elo and a neutral one are both "under 10". The full run's
`elo0=0 elo1=5` (`fastchess.sh:58`) is the smallest bound pair here that
separates them. Cost is the full window: S033's verdict took 44 m 10 s only
because the effect was +60, and this one is expected small either way.

Writes `/tmp/fastchess_full.log` and `/tmp/fastchess_full.pgn`
(`fastchess.sh:63-64`). Under `--fast` those become `/tmp/fastchess_fast.*`.

**2026-08-17: the paragraph above is wrong and the run it prescribed proved it.**
`elo0=0 elo1=5` does not separate the two answers, it excludes them both. With
the effect near +3 Elo the truth sits *between* H0 and H1, so neither hypothesis
is the more likely one for long and the LLR random-walks rather than climbing;
6 h 36 m and 9036 games returned nothing. The correction is `elo0=-5 elo1=5`,
which straddles the effect rather than bracketing it from one side, and it
returned a verdict in 1 h 41 m. `plan_todo/S021_aspiration_windows.md:12-17` had
named this failure mode and prescribed exactly that fix before this step ran.
**DEC-063** is the general rule and the next section is what each run measured.

The `-pgnout` path is also a trap this step walked into and it is not the
script's fault. `fastchess.sh:133` passes a fixed `/tmp/fastchess_full.pgn` and
fastchess **appends** to it, so the file the run left behind held 13468 games
from three different matches -- 9055 of this run against `ref-7f15ac4`, 3397
against `ref-a2f0065` and 1016 against `ref-c56ab41` -- separable only by the
`White`/`Black` tags. `adocs/data/README.md` records what was kept and why.

## What was run before any match

At `7f15ac4` with the margin edit, g++ 13.3.0. Both re-run at the completing
commit, with the numbers below reproduced, rather than carried over on trust:

- `ctest --test-dir build -L fast` -- **13 of 13 green in 20.81 s**. All six mate
  cases pass at margin 75, run as their own invocation
  (`./build/tests/test_search -tc=...`, **6 passed, 0 failed, 160 assertions**):
  "mate in one", "mate in two is found at the right distance",
  "a mated side reports mate in zero", "mate is recognised at depth zero",
  "pruning does not hide a forced mate" and
  "pruning does not hide a mate against the material leader", which is the
  DEC-060 case. The accepts clause that does not need a match is discharged for
  the setting that ships.
- `tools/search_bench.py ./build/src/chesso 9` -- 213509 + 915091 + 87523 =
  **1216123** nodes in 0.192 s, best moves `c3d5 e2a6 d7c8q`. The `3 75 6` row of
  `adocs/data/S033_rfp_ply_sweep.tsv` reproduces to the node on a committed tree,
  which is the first time it has been checked against one.

## The verdict: two runs, and the first one is part of the record

Both runs used the same two Release binaries -- candidate margin 75 from the
working tree, reference `7f15ac4` at margin 100 from
`.ref-builds/7f15ac4/build/src/chesso` -- the same `books/8moves_v3.pgn` in
random order, `tc=10+0.2`, `Hash=16`, `Threads=1`, concurrency 12, and the
adjudication `fastchess.sh:49` sets. `model=normalized`, `alpha=0.05
beta=0.05`. **The bounds are the only difference between them.**

### Run 1 -- `elo0=0 elo1=5`, no verdict

`adocs/data/S068_sprt_run1.sh`, transcript `adocs/data/S068_sprt_run1.log`.
9066 games started, 9036 scored, 6 h 36 m 15 s (`elapsed_seconds=23775`), then
terminated deliberately (`Terminated`, `S068_SPRT_FAILED status=143`). Last
scored block:

```
Elo: 3.19 +/- 5.60, nElo: 4.08 +/- 7.16
LOS: 86.80 %, DrawRatio: 35.37 %, PairsRatio: 1.04
Games: 9036, Wins: 2920, Losses: 2837, Draws: 3279, Points: 4559.5 (50.46 %)
Ptnml(0-2): [422, 1008, 1598, 1045, 445], WL/DD Ratio: 1.61
LLR: 0.59 (20.1%) (-2.94, 2.94) [0.00, 5.00]
```

**Why it was stopped, which is the load-bearing part.** With a true effect near
+3, `elo0=0 elo1=5` brackets the truth from one side: neither hypothesis can be
accepted, so the LLR does not climb, it random-walks. It drifted 0.86 -> 0.59
over the final ten minutes. At the measured 1371 games/h the remaining 30934
games to the 40000-game cap were a further **22.5 h** for an answer that could
not structurally arrive. This is a bounds error, not a property of the change,
and it is recorded rather than hidden: DEC-063 is the rule it produced and the
6 h 36 m is what buying that rule cost.

### Run 2 -- `elo0=-5 elo1=5`, H1 accepted

`adocs/data/S068_sprt_run2.sh`, transcript `adocs/data/S068_sprt_run2.log`,
games `adocs/data/S068_run2_match.pgn`. Everything except `-sprt` mirrors
`fastchess.sh:122-134`.

```
Elo: 12.18 +/- 11.12, nElo: 15.52 +/- 14.16
LOS: 98.42 %, DrawRatio: 33.65 %, PairsRatio: 1.12
Games: 2312, Wins: 761, Losses: 680, Draws: 871, Points: 1196.5 (51.75 %)
Ptnml(0-2): [92, 269, 389, 278, 128], WL/DD Ratio: 1.40
LLR: 2.97 (100.9%) (-2.94, 2.94) [-5.00, 5.00]
SPRT ([-5.00, 5.00]) completed - H1 was accepted
Total Time: 01:41:13
```

**The interpretation was pre-registered before launch**, in the script's own
header comment, and is honoured as written: H1 accepted -> margin 75 is not a
regression; combined with 14.5 % fewer nodes that is the case for shipping 75.
H0 accepted would have reverted both edits; no verdict would have kept 100 and
forbidden a third run without a decision. `adocs/data/S068_sprt_run2.sh` is the
primary evidence that the reading was fixed before the number was seen, which is
the only thing that separates a pre-registration from a rationalisation.

### +12.18 is not the effect size

An SPRT stops early exactly when the observed effect is running favourable, so
the point estimate at the stopping moment is **upward-biased by optional
stopping**. Quoting +12.18 as what 75 buys would be a measurement error of the
kind INV-6 exists to prevent.

Run 1's 9036 games are the more precise estimate. Pooled over both runs, which
is legitimate because the conditions were identical:

| | games | W | L | D | score | point Elo |
|---|---|---|---|---|---|---|
| run 1 | 9036 | 2920 | 2837 | 3279 | 50.46 % | +3.19 |
| run 2 | 2312 | 761 | 680 | 871 | 51.75 % | +12.18 |
| **pooled** | **11348** | **3681** | **3517** | **4150** | **50.72 %** | **+5.02** |

Naive 95 % interval on the pooled score, `1.96 * 0.5 / sqrt(11348)`:
49.80 % .. 51.64 %, i.e. Elo **-1.37 .. +11.42**. Naive twice over -- it ignores
the pairing, and it uses 0.5 for the per-game standard deviation where the
observed value is 0.398 because the games are 36.6 % draws. Using the observed
figure gives 49.99 % .. 51.46 %, Elo -0.07 .. +10.11; the crude bound is the one
recorded because it is the conservative one. Neither interval is a pentanomial
one and `fastchess`'s own bars are the ones to quote per run.

**And run 2's contribution to the pooling carries the stopping bias**, so the
pooled +5.02 is itself a little optimistic. The honest summary:

> A small positive effect, most likely 3 to 5 Elo, demonstrably **not** a
> regression of 5 Elo or more, with **14.5 % fewer nodes** at depth 9.

That is what shipping 75 rests on. It is not a claim that 75 is worth 12 Elo,
and DEC-019 is the standing reason not to upgrade it into one later.

### What ships

`src/search_params.hpp:60` third column 75, and
`tests/test_search_params.cpp:43`'s `golden_defaults` row moved from 100 to 75 in
the same commit. That second edit is the **re-target the test file's own comment
prescribes**, not a relaxation: the assertion is unchanged and still pins every
default to the value the engine ships, it is the shipped value that moved. The
red was observed before the fix, at `7f15ac4` plus the header edit alone:

```
CHECK( param.default_value == golden_defaults[i].value ) is NOT correct!
  values: CHECK( 75 == 100 )
  logged: [RfpMargin] is 75, not the 100 it shipped at.
```

**The shipping node count has changed: 1422053 -> 1216123** at depth 9, best
moves unchanged. Every step file, decision and audit report written before today
quotes 1422053 as the shipping figure and is now historical on that point --
`plan_done/` and `decisions.md` are immutable and were not touched.
`DEV_MANUAL.md:269` and `:277` and `adocs/data/README.md` carried it as a live
figure and were corrected in this commit.

## The trap this step walks into

**A margin that prunes harder makes the rule wronger, not just faster.** RFP
returns a static lower bound and a mate is what that bound cannot respect
(DEC-060). The mate cases are green at 75 today, and that is one position's worth
of evidence, not a proof. Any margin below 75 needs the same treatment: the fast
suite green first, the SPRT second, and a margin that goes green only by
arithmetic is what the material-leader case exists to catch.

And do not reach for a guard when a margin goes red.
`adocs/data/S033_rfp_guard_sweep.tsv` is the third table: five candidate guards,
none of which works. Two of them -- "the side to move still has a piece" and
`game_phase() >= 6` -- do not move the bench by a single node, which is the
measurement saying they never fire. That table is on the same pre-commit tree as
the other two, so read its `guard` and `margin` columns and not its node counts.

## Cost

Estimated as one verdict. S033's took 44 m 10 s at 12 cores for 1012 games; a
smaller effect takes longer, and this one was expected to be small either way.

**Actual: 8 h 17 m of match time for one verdict, in two runs.** 6 h 36 m 15 s
bought nothing on the wrong bounds and 1 h 41 m 13 s bought the answer on the
right ones, both at the same 1371 games/h. The estimate was not wrong about the
size of the effect; it was wrong about which hypothesis pair can see an effect
that size, which is a different mistake and is now DEC-063.

## The evidence, and what is not in the repository

`adocs/data/` holds both run scripts, both console transcripts and run 2's PGN.
`adocs/data/README.md` states what each file is and why the 38 MB accumulated
PGN from run 1 was **not** committed, with the reasoning rather than a shrug:
that file is not an artifact of this run alone, it holds two earlier matches
against other references as well, and every per-game result of run 1 -- score,
pairs, adjudication reason -- is recoverable from the committed transcript. Only
run 1's move lists are lost, and they were in a session scratchpad that S072
exists because of.
author:    Maksym Bodnar
