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

Of the three, only `RFP_MAX_DEPTH` and `RFP_MARGIN` are in `src/search.cpp` at
HEAD, at `:39` and `:38`. `RFP_MIN_PLY` is at `:47`. `RFP_MIN_DEPTH` is in no
source file and no commit.

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
`depth >= 1` always holds where the rule is tested, since `src/search.cpp:304`
hands anything below it to `quiescence()` before the check at `:338` is reached,
and the rule already exempted the root -- so both must be the same binary at
floor 1, and both report **1190649** at margin
75 and **1398911** at margin 100. `S033_rfp_guard_sweep.tsv` row
`guard=0 margin=100 min=1 max=6` reports 1398911 as well.

### What is reproducible at HEAD, and what is not

`S033_rfp_ply_sweep.tsv` maps onto HEAD. HEAD is margin 100, `RFP_MIN_PLY` 3,
`RFP_MAX_DEPTH` 6, which is its row `3 100 6` at **1422053** nodes -- reproduced
2026-08-16 at `d7901e3` as 292313 + 1026739 + 103001, best moves
`c3d5 e2a6 d7c8q`. Its other rows are one hand edit away.

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
  which needs the `#ifndef` guards the pre-commit tree had. At HEAD the
  constants are plain `#define`s, so `-D` collides with the definition and
  `-Werror` fails the build. The scripts swallow a failed build into
  `BUILD_FAIL` and keep going, so a re-run today produces a table of failures
  rather than an error. S073 is the step that makes `-D` work.
