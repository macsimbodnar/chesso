# Measurement evidence

Raw output of runs that a decision or a completed step rests on. Kept because
regenerating any of it costs hours of reference search, and because a number in
`decisions.md` that cannot be re-derived is an assertion rather than a
measurement.

Append only in practice: a file here is the evidence for something already
recorded, so it is added, never edited.

| file | what it is |
|---|---|
| `S018_match.pgn` | the 210 games achesso vs sgambetto at 10+0.2 that S018 profiled, loose adjudication (DEC-031) |
| `S018_raw.tsv` | 13522 per-move records from `tools/error_profile.py` over that PGN, scored by Stockfish `dev-20260803-762dd1da` at 3000000 nodes (DEC-030). Re-bucket with `tools/error_profile.py --from-raw`, no engine needed |
| `DEC033_depth_vs_eval.tsv` | 160 positions from `S018_raw.tsv` re-asked of chesso at 4M and 64M nodes and re-costed, produced by `tools/depth_vs_eval.py`. The evidence for DEC-033 |
| `S028_match.pgn` | 98 profiled games of the same match, achesso vs sgambetto at 10+0.2, loose adjudication, played by the fitted evaluation. 120 were played; the profile was cut at 98 for time and the PGN holds all 120 |
| `S028_raw.tsv` | 5582 per-move records over those 98 games, same reference and same 3000000-node limit as `S018_raw.tsv`, so the two are comparable. The evidence for DEC-035 |
| `S028_depth_vs_eval.tsv` | 160 positions from `S028_raw.tsv` through the same probe as `DEC033_depth_vs_eval.tsv`. Also DEC-035 |
| `S033_rfp_sweep.tsv` | 45 reverse-futility settings, margin x lower depth bound x upper depth bound. The evidence for S033's "green is luck" section and for DEC-060 |
| `S033_rfp_ply_sweep.tsv` | 10 settings, first pruned ply x margin, at the upper depth bound that shipped. The evidence for the ply bound S033 shipped, and the table S068 argues from |
| `S033_rfp_guard_sweep.tsv` | 11 settings, five candidate guards x margin. The evidence that no guard works, DEC-060 |
| `S033_rfp_*.sh` | the script that produced the table of the same name, verbatim as run. See "the scripts no longer run" below |
| `S033_rfp_*.log` | the console transcript of that run. Carries no column the `.tsv` lacks -- read the `.tsv`; the log is here only to show the run went to completion in order |
| `S068_sprt_run1.sh` | the script that ran S068's first SPRT, verbatim as run except for its filename (`run_s068_sprt.sh` in the session that ran it). `elo0=0 elo1=5`, the `fastchess.sh` default, which returned no verdict |
| `S068_sprt_run1.log` | that run's full console, 21753 lines. Every game's result and adjudication reason, every periodic SPRT block, the deliberate `Terminated` and the wrapper's `elapsed_seconds=23775` / `status=143` |
| `S068_sprt_run2.sh` | the script that ran the second SPRT (`run_s068_sprt2.sh` as run). `elo0=-5 elo1=5`. **Its header comment carries the pre-registered interpretation of each outcome and the bounds reasoning, written before the run started**, which is what makes S068's reading of the result a pre-registration rather than a choice made after seeing it. DEC-063 |
| `S068_sprt_run2.log` | that run's full console, 5571 lines, ending `SPRT ([-5.00, 5.00]) completed - H1 was accepted` and `Total Time: 01:41:13` |
| `S068_run2_match.pgn` | the 2312 games of run 2, the run that decided S068. One match, one reference, with per-move score, depth and time comments |

`S018_raw.tsv` columns: `game ply phase cost ref own mate san fen`. `cost` is
the reference's swing across the move and may be negative, which the profiler
floors at zero before reporting; the spread of those negatives is the noise
floor of the reference limit.

## The reverse futility sweeps, S033

Three runs on 2026-08-16. Each row is one build: the fast search suite run
against it, and `tools/search_bench.py <binary> 9` summed over the three
positions. `suite` is `green` or `RED`; `nodes_total` is that sum;
`failed_cases` is the red case names, truncated by the script.

### They were measured on a tree that is in no commit

The sweeps ran at 13:08 to 13:28. `6bd650e`, the commit that introduced reverse
futility pruning at all, landed at **13:40:34** the same day and is the only
commit that has ever touched `RFP_MARGIN` in `src/`. So all three tables were
produced against S033's uncommitted working tree, and that tree differed from
what shipped in two ways that matter to anyone reading the columns:

- it had `#ifndef` guards around the constants, which is why `-D` worked at
  13:08 and does not now
- it had a constant `RFP_MIN_DEPTH`, which is the `min` column below

Neither survives in the history. `git log -S 'RFP_MIN_DEPTH' --all` returns one
commit and it is `e7aa98a`, an audit report, prose only; `git grep RFP_MIN_DEPTH`
over `git rev-list --all -- src/` returns nothing, and `git log -S '#ifndef RFP'
--all` returns no commit at all. **The `-D` method did not regress. It was never
in a committed tree**, and `RFP_MIN_DEPTH` never was either.

### `min` and `min_ply` are different constants

This is the one thing a reader has to get right, and getting it wrong makes the
two tables look like they contradict each other.

| column | constant | what it bounds |
|---|---|---|
| `min` in `S033_rfp_sweep.tsv` and `S033_rfp_guard_sweep.tsv` | `RFP_MIN_DEPTH` | **remaining depth**. The rule fires only at `depth >= min`, so raising it switches pruning off across whole shallow subtrees |
| `min_ply` in `S033_rfp_ply_sweep.tsv` | `RFP_MIN_PLY` | **distance from the root**. The rule fires only at `ply >= min_ply`, so raising it switches pruning off at the top of the tree |
| `max` in all three | `RFP_MAX_DEPTH` | remaining depth from above, `depth <= max` |

Of the three, only `RFP_MAX_DEPTH` and `RFP_MARGIN` exist at HEAD, with
`RFP_MIN_PLY`; `RFP_MIN_DEPTH` is in no source file and no commit. **2026-08-17:
the three that exist are no longer in `src/search.cpp`** -- S073 moved every
search constant into the X macro at `src/search_params.hpp:41-91`, where
`RFP_MARGIN` is line `:60`, `RFP_MAX_DEPTH` line `:61` and `RFP_MIN_PLY` line
`:70`. `src/search.cpp` now only reads them, at `:348-350`. The citations here
said `src/search.cpp:38`, `:39` and `:47` and were correct until `7f15ac4`.

### Margin 75 is green in one table and RED in the other

| file | setting | suite | nodes |
|---|---|---|---|
| `S033_rfp_ply_sweep.tsv` | margin 75, `min_ply` 3, `max` 6 | green | 1216123 |
| `S033_rfp_sweep.tsv` | margin 75, `min` 3, `max` 6 | RED | 2886952 |

Both rows say 75 and both say 3. The 3 names a different constant in each, and
the two constants are not interchangeable, so these are two different trees and
two different verdicts. Naming the columns is necessary and not sufficient --
the shapes are what show they are different knobs.

At margin 75, `max` 6, as the floor goes 1, 2, 3:

| floor | `min`, remaining depth | `min_ply`, distance from root |
|---|---|---|
| 1 | 1190649 | 1190649 |
| 2 | 2142924 | 1203441 |
| 3 | 2886952 | 1216123 |

`min` more than doubles the tree, because the prunes it gives up are at
remaining depth 1 and 2, where nearly all the nodes are. `min_ply` costs 2.1 %,
because the prunes it gives up are at ply 1 and 2, which is a handful of nodes.
And `min` buys nothing for its 1.7M nodes: the suite is RED at every value of
`min`, since the prune that hides the mate happens at **ply 1** and a bound on
remaining depth cannot reach a ply. The ply floor is what turns the cases green.
That is the measurement that made S033 ship `RFP_MIN_PLY` and drop
`RFP_MIN_DEPTH`.

The two tables agree exactly where the knobs coincide, which is what says they
are the same code measured twice. At floor 1 both mean "exempt nothing" --
`depth >= 1` always holds where the rule is tested, since `src/search.cpp:311`
hands anything below it to `quiescence()` before the check at `:348` is reached
(`:304` and `:338` at S033, moved by `7f15ac4`),
and the rule already exempted the root -- so both must be the same binary at
floor 1, and both report **1190649** at margin
75 and **1398911** at margin 100. `S033_rfp_guard_sweep.tsv` row
`guard=0 margin=100 min=1 max=6` reports 1398911 as well.

### What is reproducible at HEAD, and what is not

`S033_rfp_ply_sweep.tsv` maps onto HEAD. **Which row is HEAD moved on
2026-08-17:** S068's verdict shipped margin 75, so HEAD is now its row `3 75 6`
at **1216123** nodes -- reproduced at that commit as 213509 + 915091 + 87523,
best moves `c3d5 e2a6 d7c8q`. Until then HEAD was the row `3 100 6` at
**1422053** nodes, reproduced 2026-08-16 at `d7901e3` as
292313 + 1026739 + 103001, same three best moves. Both rows of this table have
now been checked against a committed tree and both are exact. Its other rows are
one hand edit away.

`S033_rfp_sweep.tsv` does not. It never varies the ply bound, so **none** of its
45 rows is at the shipping configuration, and its `min` column sweeps a constant
that exists in no commit. Reproducing any of its rows means reconstructing a
source version the history does not contain -- not a hand edit, and not
something S073 restores either, since S073 makes `-D` work for the constants
HEAD actually has. Its margin 150, 200 and 300 node counts say which direction a
margin moves the tree; they are not comparable to 1422053 and do not price
anything at HEAD.

`S033_rfp_ply_sweep.tsv` is therefore the only one of the three a later step can
argue a margin from.

### The scripts no longer run

Committed verbatim as evidence, not as tooling. Two traps:

- `out=` is a hard-coded path into a scratchpad directory of a session that is
  gone. A re-run writes its table nowhere useful.
- All three vary the constants with `-DRFP_MARGIN=...` on `CMAKE_CXX_FLAGS`,
  which needs the `#ifndef` guards the pre-commit tree had. Without them `-D`
  collides with the definition and the build fails. The scripts swallow a failed
  build into `BUILD_FAIL` and keep going, so a re-run today produces a table of
  failures rather than an error.

  **2026-08-17: S073 has landed and `-D` still does not work**, so the line that
  used to say "S073 is the step that makes `-D` work" was a prediction and it was
  wrong. The constants are no longer `#define`s either -- S073 made each one a
  row of an X macro in `src/search_params.hpp:41-91` expanded into an
  `inline constexpr int` -- and the symbol a command-line macro now collides with
  is the *declaration*:

  ```
  $ g++ -fsyntax-only -std=c++20 -Isrc -DRFP_MARGIN=75 src/search.cpp
  <command-line>: error: expected unqualified-id before numeric constant
  src/search_params.hpp:99:24: note: in definition of macro 'CHESSO_DECLARE_SEARCH_PARAM'
     99 |   inline constexpr int sym = def;
  src/search_params.hpp:60:5: note: in expansion of macro 'RFP_MARGIN'
  ```

  Reproduced 2026-08-17 at S068's completing commit, g++ 13.3.0. What S073
  actually built is the other route: `-DCHESSO_TUNE=ON` makes every parameter an
  `extern int` settable over UCI, so a *sweep* costs one build instead of one per
  point. That build must never produce a strength number -- `DEV_MANUAL.md`,
  "The tune build", **"It is not the release binary and no strength number is
  ever taken on it"**, cited by its sentence rather than by a line because that
  section moves --
  which is why S068's two SPRT binaries were both ordinary Release builds with
  the default edited in the header.

## S068's two SPRTs, and the 38 MB PGN that is not here

Both runs measured the same one constant -- `RFP_MARGIN` 75 against the shipping
100 -- with the same two Release binaries, book, time control, concurrency and
adjudication. The bounds were the only difference and they decided everything:

| run | bounds | games | wall | outcome |
|---|---|---|---|---|
| 1 | `elo0=0 elo1=5` | 9036 scored, 9066 started | 6 h 36 m 15 s | no verdict, terminated |
| 2 | `elo0=-5 elo1=5` | 2312 | 1 h 41 m 13 s | **H1 accepted** |

Both at about 1371 games/h. DEC-063 is the rule; S068 is the arithmetic. **Run 1
is kept in full because a run that returned nothing is the evidence for that
rule** -- delete it and DEC-063 becomes an assertion about a run nobody can
inspect.

The effect estimate is pooled over both, 11348 games, 3681-3517-4150: score
50.72 %, point Elo **+5.02**. Run 2 stopped early and its point estimate
(+12.18) is therefore upward-biased by optional stopping, so it is not quoted
alone and its share of the pooling is flagged in the step file.

### What is committed, and what was left out

Committed: both scripts, both console transcripts, and run 2's PGN. 8.7 MB in
total, against 2.7 MB for everything in this directory before it.

**Not committed: run 1's PGN, 38 MB, 13468 games.** Two reasons, in order of
weight.

1. **It is not an artifact of run 1.** `fastchess.sh:133` passes a fixed
   `-pgnout file=/tmp/fastchess_full.pgn` and fastchess *appends*, so the file is
   an accumulation of every full-bounds match that machine has run: 9055 games of
   S068's run 1 against `ref-7f15ac4`, 3397 against `ref-a2f0065` and 1016
   against `ref-c56ab41`. Committing it as "S068's match PGN" would be filing
   three matches under one step's name, and separating them needs a filter over
   the `White`/`Black` tags.
2. **Nothing recorded depends on it.** Every number S068 or DEC-063 states comes
   from the console transcript, which is committed: `S068_sprt_run1.log` carries
   every game's result and adjudication reason line by line, so the score, the
   pentanomial pairs and the LLR trajectory are all re-derivable from it. What
   the PGN adds is the move lists and the per-move score/depth/time comments --
   input for `tools/error_profile.py` or `tools/analyse_game.py`, and no step
   plans to profile them. These are two near-identical engines; S018's and
   S028's PGNs are kept because they are games against a *stronger* opponent,
   which is what an error profile needs.

That PGN lived at
`/tmp/claude-1000/-home-max-ws-chesso/15ad9dc1-2aed-4b32-aca7-69494270d848/scratchpad/S068_match.pgn`
and is **volatile**: it is in a session scratchpad and will be gone, which is
exactly the loss S072 exists because of. Run 1's move lists are the part of this
step's evidence that was deliberately let go, and this paragraph is the record of
the choice rather than a silence about it.

Run 2's PGN is committed on the opposite reading of the same two tests: it is one
match against one reference, it is the run that decided the step, and 6.8 MB
insures 1 h 41 m of the constraint that binds the whole plan. `.git` is already
321 MB, so neither file is decided by repository size alone -- the 38 MB one is
decided by not being a clean artifact of anything.
