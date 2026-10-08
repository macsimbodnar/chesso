# Performance and bug audit — 29348c559a3055a65ca8d392aa9e82286a92642e

Scope: the engine code under `src/` — missed optimizations, bugs, slow code
paths, slow algorithms, quadratic or worse complexity — and verdicts on prior
engine findings. Audited cold at `29348c5`, 2026-10-08.

## How this was measured

- **Machine.** The MacBook: Apple M1, 8 cores, 8 GB, macOS 15.8.1, Apple clang
  16.0.0 (clang-1600.0.26.6), `-O3 -march=native`, `CHESSO_TUNE=OFF`. Every build
  below was configured in a scratch directory outside the tree. HEAD's own copy
  passes the fast suite there: `ctest -L fast`, 43 of 43.
- **Instrument.** Wall-clock nps could not resolve anything here. Over 16 to 20
  interleaved `bench 16` pairs, each pair varied by 6.5 to 7.9 % (sd). The
  instrument used instead is the kernel's per-process counters from
  `/usr/bin/time -l`: `instructions retired` and `cycles elapsed`. Binaries
  run interleaved, order reversed every round, and each figure is the median.
  An A/A of two copies of HEAD's binary reads within 0.03 % (bench 16, 9
  rounds) and within 0.08 % (100 positions, 5 rounds). Two workloads:
  - `bench 16`: 8 positions, 10214429 nodes.
  - 100 positions: the first 100 lines of `books/UHO_Lichess_4852_v1.epd`,
    each run after `ucinewgame`, with `go depth 11` driven over UCI. 18006216
    nodes.
- **Node identity** of every prototype is checked three ways. (1) The `bench`
  total at depth 14, 16 and 18: `4081329`, `10214429` and `18857965`. (2)
  Per-position node counts and best moves over the first 300 UHO positions at
  `go depth 10`: 32732885 nodes, 0 differences. (3) The 100-position runs
  above: identical totals and identical best moves.
- **Profile.** `samply` over `bench 19` on a `-g` Release build. Self time is
  attributed to source lines with `llvm-symbolizer` and `dwarfdump --lookup`.
- **Counts.** A copy of HEAD with counters added; its `bench` is still
  `4081329`.

Summary of the timings (median change against HEAD; A/A in the first row):

| build | bench 16 instructions | bench 16 cycles | 100 positions instructions | 100 positions cycles |
|---|---|---|---|---|
| HEAD, second copy (A/A) | +0.00 % | -0.03 % | +0.00 % | -0.08 % |
| F02 prototype (2 lines) | -1.63 % | -2.70 % | -2.41 % | -3.88 % |
| F01 prototype (pre-make check) | -7.59 % | -5.04 % | -8.96 % | -7.23 % |
| F01 + F02 together | -9.16 % | -6.99 % | -11.33 % | -9.56 % |
| F03, HEAD with `-flto` | -1.45 % | -1.74 % | -1.29 % | -1.17 % |

Earlier rounds of the same comparison read F02 at -2.45 to -3.19 % cycles on
bench 16 and -3.47 % on the positions. F01+F02 read -7.41 % and -8.60 %. The
instruction counts repeated to the second decimal in every round.

## Findings

### 2026-10-08_performance-F01 — high — every pruned move is made and unmade only to ask whether it gives check; this is 57 % of the main search's make_move calls and about 5 to 7 % of all cycles

Status: closed — S268, 2026-10-08: `move_gives_check()` asks before `make_move` at all three sites, node-identical, +7.16 % (CI +6.98 .. +7.33) on the workstation. Previously: planned — S268 (DEC-263), Open entry 1.

**Evidence.**

The move loop decides the four quiet rules and S091's capture rule before the
move is made. It applies them only after `make_move`, because the gives-check
exemption is read off the child board.

- `src/search.cpp:2610` makes the move:
  `if (!make_move(game, moves[i])) { continue; }`.
- `src/search.cpp:2661-2671` throws it away again:
  `if (prune_rule != PRUNE_NONE && !is_check_move() && !capture_gives_check) {
  ... unmake_move(game); continue; }`.

The same shape is in quiescence's futility test: `src/search.cpp:1236-1238`
makes the capture, calls `is_check(game)`, then unmakes it.

The comment at `src/search.cpp:2497-2499` justifies the cost with another
engine's Elo figure, not a timing of this one: *"Lynx measured moving such
rules before make at -0.6 +/-2.6, so the wasted make costs nothing worth
chasing"*. S109's step file says the same at
`adocs/plan_done/S109_shallow_depth_pruning_block.md:222-228`. Its premise is *"Does NOT exist: a
pre-make gives-check predicate"*. CLAUDE.md rule 7 is the rule that figure
breaks.

Counted on the instrumented copy, `bench`, 4081329 nodes:

```
AUDIT negamax makes 5792360, made-then-pruned 3313197, pruned-rule-but-gives-check 81133
AUDIT ... lmp_made 1028425 lmp_pruned_after_make 1017774
```

- 3313197 of the 5792360 moves `negamax_at` makes, 57.2 %, are unmade at
  `:2670` without being searched.
- Only 81133 moves (2.4 % of the moves a rule selected) survive by giving check.

The profile agrees. `make_move` called from `search.cpp:2610` is 7.93 % of
all samples (searched and pruned moves together). `unmake_move` at `:2670`,
the pruned path alone, is 2.16 %. `is_check` at `:2631` is 3.61 %.

**Prototype (not applied).** About 20 lines: a pre-make predicate for a quiet,
non-castling move. It asks whether our pieces attack the enemy king after the
move, with the moved piece and the occupancy updated. This covers direct and
discovered checks, a king's discovered check included. The predicate is used
only for a quiet whose rule has already selected it, so the decision is
unchanged:

```diff
+static inline bool audit_quiet_gives_check(const board_t* board, move_t move)
+{
+  const bb_tables_t* tables = game_tables();
+  const color_t us = board->active_color;
+  const index_t from = MOVE_FROM(move);
+  const index_t to = MOVE_TO(move);
+  const piece_t piece = MOVE_PIECE(move);
+  const bb_t move_bb = (BB_1 << from) | (BB_1 << to);
+  const bb_t occ = board->occupancies[BOTH] ^ move_bb;
+  const index_t ksq = get_lsb_index(board->bitboards[(us == WHITE) ? B_KING : W_KING]);
+  const int base = (us == WHITE) ? W_PAWN : B_PAWN;
+  bb_t mine[6];
+  for (int t = 0; t < 6; ++t) { mine[t] = board->bitboards[base + t]; }
+  mine[piece - base] ^= move_bb;
+  return (tables->pawn_attacks[!us][ksq] & mine[0]) ||
+         (tables->knight_attacks[ksq] & mine[1]) ||
+         (get_bishop_attacks(tables, ksq, occ) & (mine[2] | mine[4])) ||
+         (get_rook_attacks(tables, ksq, occ) & (mine[3] | mine[4]));
+}
 ...
     const uint64_t nodes_before_move = (ply == 0) ? state->explored_nodes : 0;
+
+    if (!PROBING && (skip_quiets || prune_rule != PRUNE_NONE) && is_quiet &&
+        !MOVE_CASTLING(moves[i]) &&
+        !audit_quiet_gives_check(&game->board, moves[i])) {
+      continue;
+    }
 
     if (!make_move(game, moves[i])) { continue; }
```

Results for the prototype:

- **Node identity, all three checks pass.** `bench` reads 4081329, 10214429
  and 18857965. The 300 UHO positions at depth 10 give 32732885 nodes with 0
  differences. The 100-position totals and best moves are identical.
- **Timing.** Alone, the prototype measured -7.59 % instructions and -5.04 %
  cycles on bench 16, and -8.96 % and -7.23 % on the 100 positions.
- **Tests.** Together with F02's two lines, the prototype passes all 14 engine
  fast tests: test_chesso, openings, movegen, evaluation, search, engine,
  audit_fen_semantics, en_passant_key, audit_go_infinite, invariants,
  uci_surface, mate_breadth, mate_pv, mate_carry.

**Impact.**

- In every game, about 5 to 7 % of the search's cycles go to make, unmake,
  history push, hash and accumulator updates, and an attack scan, all for
  moves that are thrown away.
- The play-altering steps on the plan buy less. Recent H1 verdicts include
  S234 +4.73, S115 +4.24 and S095 +5.92 Elo.
- Converted at the specs' own figures (INV-6, DEC-083: 1.43 to 2.10 Elo per
  % nps), this is roughly 7 to 15 Elo. That is a conversion, not a verdict.
- The two remaining sites are not in the prototype: captures selected by
  S091's rule (`:2643-2645`), and quiescence's futile captures. They pay the
  same cost.
- Measured on Apple M1. The workstation's ratio may differ, and is to be
  re-timed there.

**Suggested resolution.**

- Add a pre-make gives-check predicate to `src/bitboard.cpp` beside
  `is_check`, tested against `make_move` + `is_check` over the corpus.
- For captures, the target square stays occupied, so en passant and
  castling can fall back to make.
- Use it at the three sites. The PROBING path may keep its post-make record.
- Land it as behaviour-neutral under INV-6: identical `bench` and
  `tools/search_bench.py`, an interleaved timing on the workstation, no SPRT.
  It touches the search loop, so DEC-141's Debug self-play applies.
- Correct the comment at `src/search.cpp:2497-2499`.

### 2026-10-08_performance-F02 — medium — after late move pruning fires, every remaining quiet still pays for futility, history pruning, quiet SEE and S091's see_ge(0), and none of their answers can change what happens to it

Status: closed — S269, 2026-10-08: a quiet past late move pruning's count is not asked the three rules nor S091's exchange test, node-identical, +2.09 % (CI +1.91 .. +2.27) on the workstation. Previously: planned — S269 (DEC-263), Open entry 2.

**Evidence.**

- `src/search.cpp:2502` gates the three per-move rules on
  `if (may_prune && is_quiet)` and does not test `skip_quiets`.
- `:2591-2594` computes `see_loses_material`, which calls
  `see_ge(board, move, 0)` on every eligible late move, quiets included.
- Once `skip_quiets` is set, the outcome for a quiet is fixed:
  - `:2650-2652` turns `PRUNE_NONE` into `PRUNE_LATE_MOVE`.
  - `:2661` prunes on any non-NONE rule unless the move gives check.
  - So futility, history and quiet SEE all reach the same branch as the late
    move rule.
- `see_loses_material` has two readers. For a quiet it can be read only at
  `:2743` behind `may_reduce`. `may_reduce` requires `!is_check_move()`
  (`:2723-2725`), and a skipped quiet that gets that far gives check. So the
  value is never read for such a move. Its other reader, `capture_gives_check`
  at `:2644`, applies to captures only.
- The only other read of `prune_rule` is the probe record at `:2665`, which is
  test-only.

Counted at `bench` (4081329 nodes):

```
AUDIT quiet_see 996527 quiet_see_with_skip 459042 lmr_see 918371 lmr_see_with_skip_quiet 260612
```

46 % of the quiet-SEE calls and 28 % of S091's `see_ge(0)` calls are made
after the flag is set. The profile puts `see_ge` called from `:2551` at
4.26 % of all samples and from `:2594` at 1.27 %. That makes the quiet-SEE
site the largest single caller of the exchange evaluator, above quiescence's
3.21 %.

**Prototype (not applied).** Two lines:

```diff
-    if (may_prune && is_quiet) {
+    if (may_prune && is_quiet && !skip_quiets) {
...
         SEE_LMR_EXTRA > 0 && prune_rule == PRUNE_NONE && ply > 0 &&
+        !(skip_quiets && is_quiet) &&
```

- Node-identical on all three checks.
- -1.63 % instructions and -2.70 % cycles on bench 16 (other rounds -2.45 to
  -3.19 % cycles). -2.41 % instructions and -3.88 % cycles on the 100
  positions.
- With F01's prototype it passes the 14 engine fast tests. Under PROBING, a
  skipped quiet's recorded rule becomes `PRUNE_LATE_MOVE`. No case asserts the
  old value.

**Impact.**

- About 2.5 to 3.9 % of cycles are dead computation in every game.
- The quiet-SEE work it removes is the largest share of the evaluator's cost.

**Suggested resolution.**

- Skip the three per-move rules and the S091 SEE call for a quiet once
  `skip_quiets` is set.
- Prove it under INV-6. It is independent of F01 and the two add up.

### 2026-10-08_performance-F03 — low — no build uses link-time optimization, so every cross-unit hot call is a real call; LTO measures 1.2 to 1.7 % here

Status: planned — S270 (DEC-263), Open entry 3.

**Evidence.**

- Nothing enables LTO: `grep -rn -i "lto\|INTERPROCEDURAL" CMakeLists.txt
  src/CMakeLists.txt cmake/` finds nothing, and neither do
  `build_release.sh` and `cmake/pgo.cmake`.
- `src/evaluation.hpp:390-393` says so itself: *"There is no LTO in this
  build, so a definition in evaluation.cpp is a call per quiet scored ...
  -4.81 %"*.
- Calls from `search.cpp` that stay out of line: `tt_get_entry`,
  `tt_store_entry`, `score_move`, `see_ge`, `is_check`, `make_move`,
  `game_tables()`.
- `game_tables()` (`src/bitboard.cpp:3296-3300`) is a whole function whose
  Release body is `return &shared_tables;`. It shows 0.52 % self time in the
  profile.
- S104 (`adocs/plan_done/S104_native_arch_and_pgo_build.md:36-38`) recorded LTO
  as "inside the noise on this machine" from two wall-clock runs each, and
  dropped it.

HEAD rebuilt with `-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`:

- Node-identical. `bench` reads 4081329, 10214429 and 18857965, and the 300
  positions differ in 0.
- -1.45 % instructions and -1.74 % cycles on bench 16. -1.29 % and -1.17 % on
  the 100 positions.

**Impact.**

- Roughly 1.2 to 1.7 % of cycles on this compiler, for one CMake option.
  That is about 2 to 3.5 Elo at DEC-083's conversion.
- Below the 3 % rule for a wall-clock reading, which is why S104 could not see
  it. It is above the cycle counter's 0.08 % A/A.
- Unmeasured on the workstation's compiler.

**Suggested resolution.**

- Measure `-flto` on the workstation with a counter (`perf stat -e
  instructions,cycles`, interleaved) and enable
  `INTERPROCEDURAL_OPTIMIZATION` for the engine targets if it holds. Check
  that `-Werror` survives gcc's link-time warnings.
- Independently, and node-identical by construction: make `game_tables()`
  inline in `bitboard.hpp`.

### 2026-10-08_performance-F04 — low — a new position base clears the whole table synchronously, so a new game from a book FEN clears it three times and a client that sends bare FENs loses it every move

Status: planned — S271 (DEC-264), Open entry 4; its investigation is `adocs/data/S271_replay.md`.

**Evidence.** Three call sites clear the table:

- `src/chesso.cpp:520-528` `commit_position_base()` calls `tt_reset(&tt)`
  whenever the `position` FEN differs from the last base. `tt_reset` is a
  `memset` of the whole table (`src/transposition_table.cpp` `tt_reset`).
- `src/chesso.cpp:1781-1789` `reset_for_new_game()` calls
  `set_position(DEFAULT_POSITION)`, which clears when the last base was not
  the start position, and then calls `tt_reset(&tt)` a second time.
- A harness's next line, `position fen <book FEN> moves ...`, is a new base
  and clears a third time.

The protocol file the repo ships assigns new-game signalling to `ucinewgame`.
`UCI.txt:115-130`: *"if this position is from a different game than the last
position sent to the engine, the GUI should have sent a ucinewgame"*. The base
heuristic is documented as intended (`MANUAL.md:384-388`); its cost was never
measured.

Measured with `position ...` + `isready`, then `readyok`, timed (HEAD binary):

```
Hash 4096: ucinewgame 380 / 1255 / 1166 ms, first position fen of the game 590 / 618 / 590 ms
Hash 1024: ucinewgame 143 / 25 / 27 ms,      first position fen of the game 150 / 13 / 14 ms
Hash  128: ucinewgame 7.6 / 3.3 / 3.4 ms,    first position fen of the game 6.1 / 1.6 / 1.6 ms
Hash 4096: alternating base FENs, each `position` 593 to 609 ms; same base + moves 0.1 ms
```

The table loss is measured on
`r1bq1rk1/pp2bppp/2n2n2/3p4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 10`. Search it to
depth 14, then send the board after its PV's two moves. Sent as base + moves,
the second search needs 386425 nodes to depth 14. Sent as that board's own
FEN, it needs 509675 (+31.9 %).

**Impact.**

- At the harness's Hash 16 and `rating.sh`'s 128 the cost is milliseconds.
- At large Hash, the first move of every game played from a FEN book pays a
  full clear inside the `position` that precedes its `go`. That is about
  0.6 s at 4096 MB on this 8 GB machine, so part of the figure is memory
  pressure.
- A GUI or analysis client that sends the current board as a bare FEN gets a
  cold table and a fresh book on every move.

**Suggested resolution.**

- Once any `ucinewgame` has been received, stop treating a new base as a new
  game. UCI.txt's own fallback concerns GUIs that never send it.
- Drop the second `tt_reset` in `reset_for_new_game()`, which follows a
  `set_position` that may already have cleared.
- Both are UCI-surface changes: specs, MANUAL and `test_uci_surface` first.

## Prior findings re-measured

Each was re-run from its own reproduction against HEAD's binary. Their
reports still read `planned`. Moving those lines is the coordinator's job, not
this report's.

- **`2026-09-12_adversarial-F01`, closed by re-run.**
  `position fen 7k/8/8/8/8/8/8/K6R w - - 0 1 moves h1h8` then `fen` prints
  `info string refused [position fen] ... the side not to move is in check`,
  and then the unchanged start position. Before the fix it printed a board
  with no black king.
- **`2026-09-10_adversarial-F19`, closed by re-run.**
  `go infinite nodes 1` and `go infinite depth 1` with `stop` after 0.5 s:
  `readyok` is printed before `bestmove` in both cases.
- **`2026-09-10_adversarial-F17`, closed by re-run.**
  `position startpos moves` with 256 knight plies, then `fen`, prints
  `... KQkq - 255 129`. The clock saturates and does not wrap.
- **`2026-09-10_adversarial-F20`, closed by re-run.**
  `go wtime 10000 btime 10000 movestogo 0` answers in 395 ms, against
  3988 ms with `movestogo 1`.
- **`2026-09-10_adversarial-F21`, closed by re-run.**
  On `q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1`,
  `go movetime 1` answers in 2.9 ms at 4096 nodes. `go depth 1` takes 5.2 ms
  and 32097 nodes, so the stop cut the first iteration.

## Checked, not findings

- **Selection sort is the largest self-time line.** `pick_next_move`
  (`src/search.cpp:798-810`) costs at least 8.1 % of all samples at lines
  802 to 807, and `bench` counts 91.9 M element scans in `negamax_at` for
  4.08 M nodes. Only 15.9 M of those scans happen after late move pruning
  fires. Sorting quiets once would change the order of equal scores, so it is
  play-altering. It is a candidate for a timed-then-SPRT change, not a defect.
- **Stack headroom on macOS.**
  - The search runs on a `std::thread`, whose stack is 512 KiB on macOS.
  - `negamax_at<false>`'s frame is 3728 bytes in Release and 7200 in Debug.
    `quiescence` is 2384 and 2592.
  - Measured 226320 bytes of stack below the root at ply 60, in a 20 s KPK
    search reaching depth 53.
  - At MAX_PLY the Release bound is about 475 KB, plus any nested singular
    verifications. A 90 s Debug search of the same position reached depth 50
    and did not crash.
  - Not reproduced, and not reachable in ordinary play. But one more
    `move_t[MAX_MOVES]` local in `negamax_at` would take the Release bound
    past 512 KiB.
- **Leaves are counted twice.** A depth-0 child enters `negamax_at` (one node,
  one TT probe) and then `quiescence` (a second node, a second probe). The
  razoring comment at `src/search.cpp:1853-1856` records that as intended.
- **Other places, read with no finding.**
  - `see_ge` and `see` (king value 10000 reproduces the illegal-king-recapture
    rule).
  - Generator legality and castling validation.
  - `classify_repetition`, which is bounded by the clock.
  - Book lookup, which is a binary search.
  - The aspiration loop and the abort paths at the root.
  - Logging is `if (false)` in Release, with no dangling-else use.
