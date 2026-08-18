id:         S094
goal:       quiescence probes and stores the transposition table, and an entry carries the static evaluation it was scored with
accepts:    an SPRT verdict per change, measured separately -- the probe and the stored static evaluation are two changes; a quiescence entry is stored at a depth that cannot satisfy a main-search probe, with a test that a main-search node at depth 1 does not cut on a quiescence entry; the mate-score adjustment on store and probe still holds "mate in N from here" and tests/test_search.cpp's existing table mate-score case passes unmodified; the static evaluation in the entry is used in place of a recomputation and INV-4 still holds, since the accumulators are what it is derived from; the fast suite green
touches:    src/transposition_table.cpp and .hpp for the entry layout, src/search.cpp quiescence, tests/test_search.cpp
excludes:   correction history, which is S099; the improving flag, which is S092 and is a consumer of the stored value
decisions:  DEC-071
closes:
blocks:
paused_by:
done:      both changes measured separately and both are ZERO. Probe f1e6d24 vs bd45afe: 3000 games, 2 h 07 m, no bound reached, LLR -1.32, Elo -0.23 +/- 9.32, 49.97 %. Stand-pat 22a74f2 vs 7d2da9d: H0 accepted, 1954 games, 1 h 23 m, LLR -2.21, Elo -6.40 +/- 11.42, 49.08 %. 0 time forfeits in either, counted from each run's own PGN. The layout commit 7d2da9d is behaviour-neutral and discharged INV-6 instead: identical nodes 164123/670488/84351 and best moves c3d5/e2a6/d7c8q, verified independently. All three kept, DEC-079. The win is the reverse-futility consumer at 9.8 % hit and 0 disagreements, created as S103. Full suite 19/19.

## Why it comes early in the block

Two later steps are consumers. S092's flag compares a static score across plies
and S099 learns a correction from the difference between the static score and
what the search returned; both want the value already in the entry rather than
recomputed. `evaluate()` runs at every quiescence node and S014 removed 25 % of
nodes per second by making the accumulators incremental (INV-4) -- a step that
recomputes puts that back.

## Hazard

The entry layout is shared with the main search. Widening it changes how many
entries fit a bucket and therefore the replacement behaviour, which alters play
on its own. The step measures the layout change and the use of the new field
separately, or states why it could not.
author:    Maksym Bodnar

## As executed

Three commits, each measured or proved separately, exactly as `accepts:` demands.
Nothing had to be collapsed.

| commit | change | how it was decided |
|---|---|---|
| `f1e6d24` | quiescence probes and stores at `TT_DEPTH_QS = -1` | SPRT: **zero** |
| `7d2da9d` | entry carries `int16_t eval`, read by nothing | **INV-6**, no SPRT owed |
| `22a74f2` | quiescence stands pat on the stored value | SPRT: **zero** |

### The verdicts, both zero and both recorded as zero

**`f1e6d24` vs `bd45afe`** -- 3000 games, 2 h 07 m, **no bound reached**,
`LLR -1.32` against +/-2.20. `Elo -0.23 +/- 9.32`, `nElo -0.31 +/- 12.43`,
49.97 %, `Ptnml [120, 354, 564, 332, 130]`.

**`22a74f2` vs `7d2da9d`** -- **H0 accepted**, 1954 games, 1 h 23 m,
`LLR -2.21`. `Elo -6.40 +/- 11.42`, `nElo -8.64 +/- 15.40`, 49.08 %,
`Ptnml [91, 212, 386, 218, 70]`. H0 at these bounds is "not a +10 improvement",
which a true zero satisfies; -6.40 +/- 11.42 does not establish a regression.

**0 time forfeits in either**, counted from each run's own PGN -- 3000 and 1957
games. Both runs used dedicated `-pgnout` and `-log` paths so that neither S089
trap could recur: the appended shared PGN, and the WARN-only log that reads as a
pass when empty.

### The zero was predicted before it was bought

Instrumented over a depth-12 kiwipete search: of **2530591** quiescence nodes
reaching the probe, only **19901 -- 0.79 %** -- found an entry at all, because
depth-preferred replacement evicts a `TT_DEPTH_QS` entry whenever any
main-search store lands on the slot. Of those, 11882 carried an evaluation, and
the stored value differed from a fresh `evaluate_lazy()` on **163 of 310197
nodes, 0.05 %**.

The implementing agent reported that *before* either match was booked. Both were
run anyway, because the step asks for a verdict per change and the machine was
idle -- buying the verdict cost less than the `accepts:` amendment for not
buying it would have. DEC-079 records that choice and the S075 precedent it
declined to use.

### The hazard the step names has no referent, and that is measured

The step warned that widening the entry changes entries-per-bucket and therefore
replacement behaviour. **The table is direct-mapped** -- one entry per slot, no
buckets -- and `tt_resize()` floors the count to a power of two, which absorbs
any entry size from 17 to 32 bytes: 4 MB buys **131072 entries at 20, 24 and 32
alike**. `sizeof(tt_entry_t)` stayed **24** because `eval` landed in padding the
key's alignment already reserved.

Verified independently of the implementing agent, by the session that ran the
matches: `tools/search_bench.py` at depth 9 gives **164123 / 670488 / 84351**
nodes and **`c3d5` / `e2a6` / `d7c8q`** at both `f1e6d24` and `7d2da9d`.
Identical in all six figures, so INV-6 is discharged and the middle commit owes
no verdict.

### Two engine defects found and fixed in commit 1

**`de_normalize_score()` excluded `+/-MATE_MAX`.** Quiescence is the first writer
that reaches it -- a mate with no legal reply normalises to exactly `-MATE_MAX`,
"mated here" -- and the endpoint never turned back into a distance. Four whole
search mate cases observed red: `REQUIRE( white.mate_in == 2 ) ... REQUIRE( 0 ==
2 )` at depth 3, and `REQUIRE( 4 == 5 )` at depth 9 in the aspiration case. The
bound is now inclusive leaving and exclusive entering. Inert for the main search,
which never stores that value.

**`tt_store_entry()` asserted a non-zero move.** Standing pat is not a move.
`negamax` still asserts its own before it stores.

**This is why commit 1 is kept at a measured zero rather than reverted**:
reverting it would revert the mate fix. DEC-079.

### The win is in none of these commits, and it is now S103

The reverse-futility site made **1074051 `evaluate()` calls** over the same
search, of which **105612 -- 9.8 %** -- already had the value in the entry, and
**0 disagreed** with a fresh call. Twelve times the hit rate of the quiescence
probe and no disagreement at all. Reading it there is a pure speed-up on the
full evaluation -- 83.35 ns a call on this machine -- dischargeable by INV-6
node counts rather than owing a verdict.

It is one change at a time and it is not this step's `touches:`. **S103 is
created for it and placed immediately after this step.**

### Out of scope, found, fixed here because it was a false claim in a tracked document

`DEV_MANUAL.md` stated the evaluation runs at **1.31 ns per call, 762 M calls per
second**. `./build/tests/bench_eval` prints **83.35 ns, 12.0 M calls per second**
on a byte-identical `evaluate()`. The old figure came from DEC-036, dated
2026-08-11, two days before DEC-049 moved this project off Apple silicon, and the
function has since gained S027's terms and S065's refit. Corrected to today's
measurement with the provenance stated; DEC-036's own argument is untouched
because it rests on a **ratio** measured in one run, not on the absolute number.
