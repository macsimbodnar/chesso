id:         S295
goal:       a search that ends inside an iteration never prints a line beside a mate score that does not mate at the claimed distance; and the completed iteration's mate distance, one move short of the oracle's on S145 mined row 168, is traced to its node and rule
accepts:    (1) on S294's cell (`adocs/data/S294_aborted_long_line.txt`: S145 mined row 168, cold table, Hash 16, `go nodes 150000`) a case in the fast suite observed red on the shipped tree and green after: every line printed beside `score mate N` ends in checkmate at exactly 2N - 1 plies (2|N| for a mate received) or is left short as DEC-122 allows, and `bestmove`/`ponder` are that line's first two moves; the cell is a golden named at its site with the script that re-derives it (DEC-142); the fix is in the reporting layer only and is INV-6 identical (bench total and `tools/search_bench.py` at 9 and 12, nodes and best moves); a mutant it kills (DEC-141); the S170 cells and `test_mate_carry` read as before or tighter, a ceiling it lowers restated under DEC-162's rule, none raised; (2) the completed depth-8 iteration's `mate 4` on that cell (Stockfish at depth 20: mate in 5 by h1g1) traced from the engine's own output to the node and the rule that drops or refutes h1g1, every chess claim from a tool; if it is a recorded price (DEC-158 or another), say which with the evidence; if not, stop with the figures for the coordinator before any search change; MANUAL.md's `depth` and `pv` rows true afterwards; both builds and both fast suites green
touches:    src/chesso.cpp (iterative_deepening_search()'s aborted path, S202's fallback, DEC-251 (c1), its line choice as choose_aborted_mate_line(); aspiration_root_search()'s record); src/search.cpp, src/search.hpp (mate_pv_reaches() and cut_unreached_mate_pv(), and search()'s call of the latter after the walk, reporting only); src/uci.hpp (aspiration_search_t::pv_length and uci_aborted_mate_line_probe(), test instrumentation); tests/test_mate_pv.cpp (three cases: S295's node-limited cell and two on constructed lines); tools/mutants/S202_mate_walk.py (MW03 to MW06, MW02 re-anchored); adocs/data/S295_*; adocs/data/README.md; MANUAL.md (`depth`, `pv` rows, the `ponder` paragraph, Known bugs); DEV_MANUAL.md (the goldens row, two paragraphs)
excludes:   any change that moves a node (a search change for reading (2) is its own step, measured alone); the S170 budgets (DEC-299); the completion walk's move choice (DEC-251 (c2)) unless the trace shows it is the cause, then stop and report
decisions:  DEC-317, DEC-171, DEC-122, DEC-251, DEC-162, DEC-141, DEC-142, DEC-158
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-26 -- reading (1): on S294's cell (S145 mined line 168, `go nodes 140000` by `adocs/data/S295_long_line.py`'s pick, DEC-142; the accepts quoted S294's 150000, the same run, the rule picks 140000) `test_mate_pv`'s case was red on the shipped aborted path (9 plies beside `mate 4`) and is green: the cut-short line gives way to the completed one, `ponder h1h2`; the branches no search reaches (the aborted path's cut, the completed line's own check, search()'s cut of a completed line) are guarded by two constructed-lines cases, each observed red without its branch; INV-6 identical in both builds after every src/ change (bench 5184358, search_bench d9/d12); MW01..MW07 killed (MW01..MW06 6 of 6, then MW05..MW07 3 of 3); search()'s call of the cut itself unguarded, no search reaching it; census 13 wrong lines to 0, re-taken on the final tree, the S170 grid and ceilings unmoved, ponder changed in 13 searches and added in none; 50/50 both builds, format and the four prose modes clean, gate_extra 5 stages ok and Debug self-play (8 games, Assertion 0) on the final tree. Reading (2) traced to quiet SEE pruning's fail-soft mated bound (`adocs/data/S295_trace.txt`), ruled a new defect by DEC-318, S296 its fix. Found and fixed in the worktree: the first search() cut wrapped `needed - 1` at `mate 0`, caught by `test_search`. Implemented by one Opus 5.5 subagent.

## Why this step exists (the coordinator, 2026-10-26, DEC-317)

S294 found it while sweeping for MW02's cell. At every budget from 80000 to
200000 nodes on S145's mined row 168, a search that stops inside its ninth
iteration prints the aborted iteration's 9-ply line beside the depth-8
iteration's `score mate 4`. python-chess finds no mate at ply 7 and
checkmate at ply 9, and `ponder` names the aborted line's reply (h1g1)
where the completed line's is h1h2. S202's fallback (DEC-251 (c1)) does
not run, because it catches only an aborted line the walk cannot complete;
this one completes, to a longer mate. Stockfish at depth 20 reads the root
mate in 5, so the completed `mate 4` is one move short as well. MANUAL.md's
`depth` row says the pair never disagrees and its `pv` row says the line is
never wrong. It moves a reported line wherever a search stops inside an
iteration, so it is BUGS-binding under DEC-171 (c) and goes before S136
(DEC-317).

## 2026-10-26: the work (the implementer, one Opus 5.5 subagent)

Worktree `../chesso-s295` on branch `s295` at `4fb915e` (bench 5184358),
scratch under `.tuning/s295/`. Throwaway worktrees there, all removed after:
`diag` (4fb915e: the instrumented build of reading (2), then clean as the
"before" engine of the census), `mw03` (MW03's build for the pick, then the
two other options), `mut` (the mutation fixture). Fixtures are commit
objects on no branch holding this worktree's tree: `f8abd40` (the pick's,
its src/ behaviour-identical to the final tree's -- the later moves into
functions and the completed-line cut are reporting only, INV-6 re-taken each
time, and the census re-taken on the final tree), `aa66bca`, `10cf5b8` and a
third, below (the mutation runs').

### Reading (2): the node and the rule -- a new defect, stopped for the coordinator

From the engine's own output, a throwaway build instrumented by
`adocs/data/S295_trace_patch.py` (every node's window, path, score and the
return it took; every move searched or skipped with its rule; the same 61914
nodes at depth 8). `adocs/data/S295_trace.txt` has the chain, the oracle's
word on each node and the drivers. In short:

- **The node.** Iteration 7, aspiration search [1608, 2108], ply 5 after
  `h3f2 h1g1 f2e4 d1d2 e4d2`, White to move: a non-PV node, depth 3, window
  [-2108, -2107]. It searched 9 replies, each returning -48993 (mated at ply
  7), and skipped six: `g2g3`, `g2g4`, `a1b1` by **quiet SEE pruning**
  (`PRUNE_SEE`), `a1f1`, `b2b3`, `a4a5` by **late move pruning**. It returned
  its fail-soft best, -48993, as an upper bound. python-chess: exactly two of
  White's 15 replies avoid mate in one, `g2g3` and `g2g4` -- the two SEE
  pruned; Stockfish d20 reads the node `#-2` and `#+2` after either.
- **The carry.** Its parent (ply 4, cut node) stored a lower bound "mate in
  two" for `e4d2` at depth 4 (Stockfish: `#+3`). In iteration 8's first
  aspiration search ([2124, 2166]) ply 3's `d1d2` (Stockfish: the only reply
  that is `#+3`, `g2g3` and `g2g4` `#+2`, the other twenty `#+1`), reduced to
  depth 1, cut on that entry, ply 3 failed low at -48993, and ply 2 (after
  `h1g1`) stored a lower bound "mate at ply 7" at depth 6 (Stockfish `#+4`).
  Searches 25 to 28 cut on it at ply 2; the full-window search 29 searched
  `h1h2` first (exact, mated at ply 7, Stockfish `#+3` after it) and answered
  `h1g1` from the entry with window [48992, 48993]: the root's exact `mate 4`.
- **Counterfactual** (scratch only): a node that skipped a move and failed low
  on a mated best returning its own alpha instead makes depth 8 read `mate 5`
  with the oracle's line (74605 nodes), and depth 7 `cp 2195`. The shipped
  engine's depth 9 reads `mate 5` on its own (201213 nodes).

**Not a recorded price.** DEC-158 prices reverse futility's ceiling in mates
"the engine does not find" -- missed or longer mates; this is a mate claimed
*shorter* than the position's, through quiet SEE and late move pruning, not
reverse futility. DEC-310 names the shape -- "mated bounds stored by nodes
where a shallow-depth rule had skipped moves", refuted by the node's own
search "four times in five" -- and rejected "Storing `alpha` at a node that
skipped moves: it changes what every reader of the table sees, unmeasured",
inside the null-move guard's decision; it prices no reported score. DEC-125
and DEC-127 are a distance longer than the position's. So a new defect: a
completed iteration's exact `score mate N` can be shorter than the position's
value, moving a reported score (DEC-171 (c)). **Stopped here for the
coordinator: no search change** (excludes). The figures: one cell, line 168
(game 5850), `mate 4` against the oracle's `#+5` at depth 8, corrected at
depth 9; the origin node and rule above; the counterfactual's mate 5 at depth
8 for 74605 nodes against 61914. MANUAL.md's Known bugs carries it.

### Reading (1): the fix, reporting only

The code path is `iterative_deepening_search()` in `src/chesso.cpp`, not
`src/search.cpp` (`touches:` corrected). `complete_mate_pv()` extends a short
line only to a checkmate and leaves a line already as long as the mate needs
as it is, mate or not; S202's fallback keyed on `length < needed`, so a long
aborted line passed both. The fix:

- `mate_pv_reaches()` (`src/search.cpp`, beside the walk): exactly the
  distance's plies, each in the generator, ending in checkmate. Reporting
  only, no node searched, counted or stored.
- The aborted path classes the walked line `short_line` (shorter than the
  distance, S202's) or `wrong_line` (at or past it, not checkmate there,
  S295's). Either gives way to the last completed line when it starts with
  the same move and `mate_pv_reaches()` it (`same_choice`; S202's condition
  read only its length); otherwise a wrong line is cut to one ply under the
  distance, never below the move played, and a short one stays short
  (DEC-122). `ponder` is recomputed from whatever is printed.
- `aspiration_search_t::pv_length` (`src/uci.hpp`), the line as search()
  returned it, so the case can see the aborted line's own length. Test
  instrumentation; nothing in the engine reads it.

**Options, measured by throwaway patches on the mined grid**
(`adocs/data/S295_census.txt` section 4): F1, shipped here -- full 33303,
short 6, wrong 0; F2, cut to the move played alone -- line for line F1's,
the cut never running; F3, cut instead of falling back -- 13 short lines at
line 168 instead of 13 complete ones. F1 over F3 because a true line of the
same choice exists and S202 already prefers it for a short line (DEC-251
(c1)); F1 over F2, unmeasurable here, because the longest line that claims
no mate is the shape DEC-122's residue already has.

**The case.** `tests/test_mate_pv.cpp` "an aborted iteration's line longer
than the mate beside it gives way to the completed line", in the existing
binary (still 50 tests). Its cell is a golden named at its site with
`adocs/data/S295_long_line.py` (DEC-142; S294's rule with the mutant's class
changed from short to long): 1 of 318 positions separates, line 168 (game
5850), run 80000..200000, **pick `go nodes 140000`**, margin 60000 either side
(`adocs/data/S295_long_pick.txt`). The accepts quote S294's reproduction
budget, 150000; the rule picks 140000, the same run's middle, and the case
searches that. It REQUIREs the aborted search with a line, the completed
iteration before it a mate, the same first move, the last two `info` lines at
the completed depth and mate, the completed line checkmate at its distance,
and `aborted.pv_length > needed` (the class: S202's fallback cannot run on a
line the walk left long). Then it CHECKs that a line at or past the distance
is checkmate at exactly it, `bestmove`/`ponder` are its first two moves, and
that S295's path ran: the printed line is not the aborted line's length and
is the completed line verbatim.

Red on the shipped aborted path (4fb915e's block, with `pv_length` added and
nothing else; `test_mate_pv -tc="*longer than the mate*"`, line numbers two
lower than the final file's):

```
test_mate_pv.cpp:628: ERROR: CHECK( last.pv.size() == needed ) is NOT correct!
  values: CHECK( 9 == 7 )
  logged: the last line is 9 plies beside mate 4, which needs exactly 7
test_mate_pv.cpp:632: ERROR: CHECK( line_ends_in_mate(fen, last.pv, needed, why) ) is NOT correct!
  logged: the position after 7 plies has legal replies
test_mate_pv.cpp:643: ERROR: CHECK( last.pv.size() != aborted.pv_length ) is NOT correct!
  values: CHECK( 9 != 9 )
test_mate_pv.cpp:644: ERROR: CHECK( last.pv == before.pv ) is NOT correct!
[doctest] test cases:  1 |  0 passed | 1 failed | 3 skipped
[doctest] assertions: 27 | 23 passed | 4 failed |
```

Green after: `test cases: 1 | 1 passed`, `assertions: 27 | 27 passed`; the
whole binary 4 of 4 cases, 762 assertions. On the cell `go nodes 150000` now
ends `score mate 4 depth 8 nodes 150000 pv h3f2 h1h2 g7h6 e4h4 h6h4 h2g1
h4h1`, `bestmove h3f2 ponder h1h2`.

**INV-6**, both builds, identical to `4fb915e`'s own: `chesso bench` 5184358
with the same eight `bestmove` replies; `tools/search_bench.py` depth 9
midgame 48284 c3d5, kiwipete 126091 d5e6, tactical 62646 d7c8q; depth 12
226320 c3d5, 430872 e2a6, 157130 d7c8q.

**Census** (`adocs/data/S295_census.txt`): every mate line printed, classed
full / short / wrong (at least the distance, not checkmate there). Mined grid
(S294's cells): before 33309 lines, wrong 13 (all line 168, an aborted line
of the completed line's first move), short 6; after wrong 0, short 6, the 13
lines the whole diff. S170 grid (S203's 108 cells): 1408 lines, 5 short, 0
wrong, before and after line for line -- S135's grid's totals. A wider
shipped scan, 408 positions at Hash 1 and 16, finds no wrong line of another
first move: **the cut branch is reached by no cell measured**, so no
node-limited cell can guard it (a constructed-lines case does since the
coordinator's requests, below).

**Ceilings.** None moves: the S170 grid is line for line the shipped one, so
`--ceilings` reads as before, nothing is restated, none raised; the S170
budgets untouched (DEC-299). `test_mate_carry` green in both builds.

**Mutants.** `MW03_long_aborted_line_kept` (`tools/mutants/S202_mate_walk.py`,
origin S295): `wrong_line` forced false, the aborted path as 4fb915e has it
(its build's mined sweep is 4fb915e's line for line). MW02's anchor moved
with the code and now takes out the short half of the condition, S202's.
`mutation_check.py --only MW01 MW02 MW03` on fixture `aa66bca` (a commit
object on no branch), wall 482 s, baseline green at 50 tests, bench 5184358:
**all three killed**, bench same -- MW03 by the new case alone (`CHECK(
last.pv.size() == needed )` and three more), MW02 by S294's case, MW01 by
`test_search` and `test_mate_carry` (`adocs/data/S295_mutation.tsv`, run 1).
The cut's own mutant, MW04, is below.

### Gates

`CLANG_FORMAT_MAJOR=22`, `-j12`. `build/` and `build-tune/` rebuilt
`--clean-first`: 0 warnings each. Fast label **50 of 50 in both** (101 s and
102 s; `test_mate_pv` 3.3 s, `test_mate_carry` 23.5 s and 24.0 s);
`./clang-format.sh --check` clean; `tools/plan_prose_check.py` `--citations`,
`--touches`, `--params`, `--gate`, one mode per call, each exit 0. INV-6 as
above; `Bench:` unchanged, so the commit ends `No functional change`.

Tier 2 (DEC-141; this touches the search's reporting): `tools/gate_extra.sh`
on this worktree, **GATE-EXTRA-DONE 5 stages 1012 s** -- prose 0 s, citations
1 s, debug 369 s, sanitize 590 s, perft 52 s, all ok. **Debug self-play**,
DEV_MANUAL's command on `build-debug` (Debug), 4 rounds `-repeat` at
4+0.04, Hash 16, `noob_3moves.epd` from the main checkout,
`-check-mate-pvs`, trace log: 8 games (+4 -2 =2), **`Assertion` 0** in
log and stdout, `disconnect` 0, `Incomplete mating PV` 0.

The committed patcher re-takes reading (2): applied to a clean `4fb915e`
copy it prints the same `go depth 8` lines and a log byte-identical to the
one quoted, and `S295_CF=1` reads `mate 5` at depth 8, 74605 nodes.

Disk: `.tuning/s295/` about 0.9 GB at its peak (three throwaway worktrees
with builds, sweeps, logs), this worktree 0.6 GB before `build-debug` and
`build-sanitize`; the throwaway worktrees are removed at the end.

### Documents

`MANUAL.md`: the `depth` and `pv` rows say what the path now does, traced to
`iterative_deepening_search()`; the Known bugs entry S294 wrote is replaced
by the one that remains, reading (2) (what a client sees: `mate 4` at depth
8, `ponder h1h2`, Stockfish's mate 5), and the two neighbouring entries lose
their "outside the bug" clauses; since the coordinator's requests, a
`ponder` paragraph after `bestmove 0000` and DEC-318 and S296 named in the
Known bugs entry. `DEV_MANUAL.md`: a goldens row for the new
cell, the `test_mate_pv` paragraph (and since the requests the
constructed-lines case), the S202 bullet. `adocs/data/README.md`:
six rows. `tests/test_mate_carry.cpp` unchanged (its F paragraph is still
true). `README.md`, `specs.md` not touched.

### 2026-10-26: DEC-318's ruling, and the coordinator's four requests (the implementer)

**DEC-318** (the coordinator, under the owner's standing instruction): the
short mate distance of reading (2) is a new defect, BUGS-binding under DEC-171
(c); **S296** fixes it by a search change measured on its own (two forms, the
mate sets and S292's census, then a `{-5, 0}` SPRT) before S136. S295
completes on reading (1) and the trace.

1. **The cut gets a direct guard.** The line choice after the walk is moved,
   unchanged, into `choose_aborted_mate_line()` in `src/chesso.cpp`, which
   `iterative_deepening_search()` calls; `uci_aborted_mate_line_probe()`
   (`src/uci.hpp`, test instrumentation like the record beside it) calls the
   same function. INV-6 re-taken on both builds after the move: bench 5184358,
   the same eight bestmoves, `search_bench.py` d9 and d12 the same nodes and
   moves as above. `tests/test_mate_pv.cpp` "the aborted path's line choice, on
   constructed lines" hands it S294's cell, the engine's completed line, its
   long aborted line, a short prefix, and `other` -- the first legal move but
   `h3f2`, then the first legal move in UCI order each ply, legal throughout
   and checkmate nowhere by python-chess -- and checks the cut (`other` at 8
   and at 7 plies cut to 6), both halves of the fallback, a reaching line
   left, a short line of another move left, and the root unchanged. **Red with
   the cut removed** (`} else if (false && wrong_line) {` by hand): `CHECK(
   choose(other) == other_cut )` and `CHECK( choose(other_at) == other_cut )`,
   275 of 277 assertions; green restored, 277 of 277, the binary 5 of 5 cases.
   **MW04_wrong_line_other_move_kept** is that edit; MW02 and MW03 re-anchored
   to the function. `--only MW01..MW04` on fixture `10cf5b8` (a commit object
   on no branch), wall 560 s: **4 of 4 killed**, MW04 by the new case alone
   (`adocs/data/S295_mutation.tsv`, run 2).
2. **Ponder, measured** (`S295_long_line.py moves` and `ponder`,
   `adocs/data/S295_census.txt` section 5, 4fb915e's engine against S295's):
   mined grid 12402 searches, 4110 ending on a cut-short line beside a mate --
   `bestmove` differs in 0, `ponder` added 0, removed 0, changed 13, all line
   168 (`h1g1` -> `h1h2`, the fallback); S170 grid 2043 searches, 62 so
   ending, nothing differs. The case the new code allows, a one-move cut-short
   line the walk extends gaining a `ponder`, occurs in no search measured.
   MANUAL.md has a `ponder` paragraph after `bestmove 0000`, traced to
   `iterative_deepening_search()` and the S210 fallback; `test_uci_surface`
   green in both builds.
3. MANUAL.md's Known bugs entry names DEC-318 and S296.
4. This section; the specs wording and commit message below updated.

Gates after the requests: `build/` and `build-tune/` `--clean-first`, 0
warnings; fast label **50 of 50 in both** (102 s each); `./clang-format.sh
--check` clean; the four prose modes exit 0. Tier 2 re-run on that tree:
`tools/gate_extra.sh` **GATE-EXTRA-DONE 5 stages 910 s** (debug 356 s,
sanitize 500 s, perft 53 s, all ok); the Debug self-play again, 8 games (+3 -1
=4), `Assertion` 0, `disconnect` 0, `Incomplete mating PV` 0.

### 2026-10-26: the fast check's owed items (the implementer)

1. **`same_choice`'s own check exercised.** The constructed-lines case's
   `choose` takes the last completed line as a parameter;
   `choose(long_same, short_same)` -- a completed line of the same first move
   that does not reach its mate -- expects the cut, `h3f2 h1g1 f2e4 d1d2 e4d2
   g2g3`. **MW05_completed_line_unchecked** drops `mate_pv_reaches(root,
   last_complete, mate_in)`; by hand it was red at exactly that CHECK (458 of
   459), and the short line stood in.
2. **The `pv` guarantee enforced for a completed iteration.** search() now
   hands its reported line, after `complete_mate_pv()`, to
   `cut_unreached_mate_pv()` (`src/search.cpp`): a line at or past the
   distance that `mate_pv_reaches()` refuses is cut to one ply under it, never
   below the first move. The aborted path's cut calls the same function, so
   the cut is defined once. Nothing feeding the search reads the line: the
   callers use `pv.length > 0` (has_result, `tools/datagen.cpp`), `ponder`,
   the printed line and `last_complete_pv`; the cut keeps at least one move
   and never touches `state`'s table. New case "a mate line at or past its
   distance and not mate there is cut, on constructed lines" drives the
   function on the same lines; **MW06_completed_mate_line_uncut** takes the
   cut out, red by hand at 9 assertions across both constructed cases. That
   search() makes the call at all is read by no case (no search measured
   reaches it).
   **A defect of this change, caught by the suite before it left the
   worktree:** `mate 0` (a root already mated, pv empty) has `needed` 0, and
   `needed - 1` wrapped, so the first build of the call failed `test_search`
   ("a mated side reports mate in zero", `REQUIRE_EQ( result.pv.length, 0 )`;
   "search results are well formed everywhere"), `test_leaf_walk` and crashed
   `test_audit_go_infinite`. Fixed by `needed == 0` in the guard; a check in
   the new case (`cut_unreached_mate_pv(&replay, &none, 0)`) was observed red
   without the guard (`CHECK( 18446744073709551615 == 0 )`) and green with
   it. **INV-6 again**, both builds after the fix: bench 5184358, the same
   eight bestmoves, `search_bench.py` d9 48284 c3d5 / 126091 d5e6 / 62646
   d7c8q, d12 226320 c3d5 / 430872 e2a6 / 157130 d7c8q. **Census re-taken**
   on the final engine (`adocs/data/S295_census.txt` section 6): all four
   sweeps line for line the earlier ones -- mined grid full 33303, short 6,
   wrong 0; S170 grid 1408 lines, 5 short, 0 wrong; `ponder` changed in 13
   searches, added or removed in none. The completed-line cut runs on no
   line of either grid, so no ceiling moves.
3. Text: `test_mate_pv.cpp`'s header now says two cases end inside an
   iteration and two run on constructed lines; DEV_MANUAL's goldens rows and
   the mutant file's table say MW02 and MW03 are killed by the
   constructed-lines case too (run 2); the `f8abd40` sentence above says
   behaviour-identical; the MW03 paragraph says what it is (4fb915e's `info`
   lines, not its code: `ponder` from the printed line and the stricter
   fallback test kept); MANUAL's `depth` row and the proposed specs say "a
   wrong one is cut"; MANUAL's `pv` row names `search()`'s cut and the
   `mate 1` edge in half a sentence; DEV_MANUAL's `test_mate_pv` paragraph
   re-wrapped.
4. S296's step file not touched.

**Mutants, final tree:** `--only MW01..MW06` on fixture `d09b5d2` (a commit
object on no branch), wall 787 s, baseline 50 tests, bench 5184358: **6 of 6
killed**, bench same each -- MW05 at `choose(long_same, short_same)`, MW06 in
both constructed-lines cases, MW02 to MW04 by the constructed-lines case (MW02
and MW03 also by their cells), MW01 by `test_search` and `test_mate_carry`
(`adocs/data/S295_mutation.tsv`, run 3).

Gates on the final tree: `build/` and `build-tune/` `--clean-first`, 0
warnings; fast label **50 of 50 in both** (103 s and 102 s, in full, with
`CLANG_FORMAT_MAJOR=22`); `./clang-format.sh --check` clean; the four prose
modes exit 0. Tier 2 on the final tree: `tools/gate_extra.sh`
**GATE-EXTRA-DONE 5 stages 947 s** (debug 358 s, sanitize 537 s, perft 52 s,
all ok); the Debug self-play, 8 games, `Assertion` 0, `disconnect` 0,
`Incomplete mating PV` 0.

### 2026-10-26: the second fast check's three items (the implementer)

1. **The clamp guarded.** The cut case gains `mate 1` and `mated 1` checks on
   `other`'s first move and first two plies (python-chess and the engine's
   generator: checkmate nowhere): beside `mate 1` the one-move line stays,
   move unchanged, and the call still answers `true`; beside `mated 1` two
   plies are cut to one. **MW07_cut_below_first_move** drops the clamp
   (`pv->length = needed - 1;`); by hand it was red at the `mate 1` CHECK,
   164 of 165. The doc comment of `cut_unreached_mate_pv()` (search.cpp and
   search.hpp) now says it returns whether the line was refused, which beside
   `mate 1` leaves it as it was; the code is unchanged.
2. **search()'s call stays unguarded, disclosed.** Taking it out survives
   every case: no search measured prints a completed line at or past its
   distance that is not mate there. Said here, in the case's comment and in
   the mutant file's notes; MW06 and MW07 guard the function it calls.
3. **Wording.** The aborted path cuts a wrong line with no completed line to
   give way to -- another first move, **or a completed line that does not
   reach its own mate** -- and the comments in search.hpp, search.cpp and
   chesso.cpp, DEV_MANUAL's S202 bullet and MW04's table row, paragraph and
   description (and MW06's) now say so. MW04's id still names the first case
   only; it is kept, the run records citing it.

src/ changed in comments only since the previous round (`git diff -- src/`
non-comment lines: none), so Tier 2 was not re-run; gate_extra and the Debug
self-play above are of the same code. `--only MW05..MW07` on fixture
`757a082` (a commit object on no branch), wall 464 s: **3 of 3 killed**, bench
same, MW07 by the `mate 1` check alone (`adocs/data/S295_mutation.tsv`, run
4). Gates: both builds `--clean-first`, 0 warnings, `chesso bench` 5184358 in
both; fast label **50 of 50 in both**, in full (102 s each);
`./clang-format.sh --check` clean; the four prose modes exit 0.

### Proposed specs.md wording (the coordinator writes specs)

The "A score and a line from different iterations" bullet, its last sentence
and the S295 Known bug note replaced -- **the S294 known bug closed**:

> The line is completed against the score it is printed beside, not only
> against the score its own iteration returned, and it stands only if it is
> then checkmate at exactly that distance (`mate_pv_reaches()`). A line the
> walk leaves short, or at or past the distance without a checkmate there --
> S294's cold cell, a 9-ply line beside `mate 4` -- gives way to the last
> completed line when that line starts with the same move and reaches the
> mate (S202, S295); otherwise a wrong one is cut to one ply under the
> distance, never below the move played, and a short one stays short
> (DEC-122). `ponder` is the second move of whatever is printed. S295,
> DEC-317.

And where the specs state DEC-122's guarantee for a completed iteration's
line (the S147 completion), one sentence: "Since S295 the guarantee is checked
and not only measured: `search()` hands its reported line, after the walk, to
`cut_unreached_mate_pv()`, which cuts one at or past the distance that is not
checkmate there to one ply under it -- never below the move played, so beside
`mate 1` a wrong one-move line stays."

The paragraph after it: "one guarded by `test_mate_pv` (the aborted-line
fallback runs there) and one the known bug above" becomes "both guarded by
`test_mate_pv`, one per half of the aborted-line fallback (S294, S295), and
its cut and the completed line's own check by cases on constructed lines",
and "broken outside the fixture by
the known bug above (S295)" becomes "broken outside the fixture once, by the
long aborted line S295 closed". **The short distance, open**, where the specs
discuss mate distances: "**Known bug, S296 (DEC-318):** a completed
iteration's exact mate score can be shorter than the position's: a non-PV
node that skipped moves by quiet SEE or late move pruning returns its
fail-soft mated best as its bound and the table carries it
(`adocs/data/S295_trace.txt`)."

### Proposed commit message

```
Print no mate line that misses its distance beside its score (S295)

A search cut short inside an iteration printed that iteration's line
beside the last completed mate score after completing it against that
score, but the walk leaves a line already as long as the mate needs as
it is, and S202's fallback keyed on a line still too short. S294's cold
cell printed a 9-ply line beside mate 4, checkmate only at ply 9, with
ponder from it. Now a line at or past the distance must be checkmate at
it (mate_pv_reaches()): on the aborted path a short or wrong line gives
way to the completed line of the same move when that line reaches the
mate, otherwise a wrong one is cut below the distance; search() cuts a
completed iteration's wrong line the same way (cut_unreached_mate_pv()),
so the pv guarantee is checked, not only measured; ponder is the
printed line's second move. Reporting only, INV-6 identical. A
test_mate_pv cell picked by adocs/data/S295_long_line.py (DEC-142) and
two cases on constructed lines, for the branches no search measured
reaches, guard it; MW03 to MW06 are killed.

Reading (2), the completed mate 4 one move short of the oracle's mate
5, is traced to quiet SEE pruning at a node whose fail-soft mated bound
the table carried (adocs/data/S295_trace.txt). DEC-318 rules it a new
BUGS-binding defect; S296 fixes it, MANUAL.md's Known bugs carrying it
until then. DEC-317, DEC-318, DEC-251, DEC-122, DEC-142, DEC-171.

No functional change
```
