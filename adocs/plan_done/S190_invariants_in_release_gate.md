id:         S190
goal:       a Release fast test compares the evaluation accumulators and `squares[]` against a full rebuild after every make and unmake over the corpus, so INV-2 and INV-4 are enforced by the gate
accepts:    a new `fast` doctest target walks every test FEN to depth 3, and to depth 4 on the five positions `tests/test_engine.cpp`'s hash oracle uses, and after every `make_move` and `unmake_move` REQUIREs `material`, `psqt_mg`, `psqt_eg` and `phase` equal to `eval_refresh`'s rebuild and `squares[]` equal to the bitboards -- the two helpers now under `#ifndef NDEBUG` in `src/bitboard.cpp` made available to tests in every build without changing their Debug use; observed red under a mutant that drops one accumulator update in `add_piece` before green; `adocs/specs.md`'s INV-2 and INV-4 rows name the test beside the Debug assertions; Release wall time under 5 s; `DEV_MANUAL.md` "Test" carries the DEC-141 Debug self-play habit with the exact `fastchess` line and what to grep; fast suite green in both builds
touches:    tests/, tests/CMakeLists.txt, src/bitboard.cpp, src/bitboard.hpp, adocs/specs.md, DEV_MANUAL.md
excludes:   any change to `make_move`, `unmake_move` or the accumulators; putting the Debug binaries or sanitizers in the automatic gate (DEC-025)
decisions:  DEC-139, DEC-141
closes:     2026-09-04_test_review-F01
blocks:
paused_by:
author:     agent (Claude Opus 5), 2026-09-09
done:       2026-09-09. **INV-2 and INV-4 are enforced by the gate.** `tests/test_invariants.cpp`, one new `fast` doctest target, walks every test FEN at `CORPUS_DEPTH = 2` plus the five positions of `tests/test_engine.cpp` *"hash and board survive make/unmake"* at their own depths (4, 3, 4, 3, 3) -- **2132167 `make_move` calls** -- and after every make and unmake compares `material`, `psqt_mg`, `psqt_eg` and `phase` against `eval_refresh`'s rebuild, `squares[]` against the twelve piece bitboards, and the whole `board_t` against its pre-make copy. **1.816 s +/- 0.018 s** over ten `hyperfine` runs in Release (budget 5 s) and **12.7 s** in Debug, so the shared 60 s / 600 s timeouts hold both and it needs none of its own. 2 cases, 2728 assertions. **The `accepts:`' "depth 3" was unsatisfiable and the owner re-decided it on 2026-09-09**: corpus depth 3 is 53975914 makes and about 34 s with the compares, against "Release wall time under 5 s". Depth 2 plus the five, section 10 question 1's proposal, was chosen; question 2's `memcmp` is in, question 3's environment override is left to S197 behind the one `CORPUS_DEPTH` constant. **The two helpers moved out of `#ifndef NDEBUG` in `src/bitboard.cpp` and lost `static`**, declared in `src/bitboard.hpp`; the three `assert(...)` call sites, `make_move_impl`, `unmake_move_impl`, `add_piece`, `remove_piece` and `move_piece` are untouched. **Six mutants, all killed by the new test, each naming the field it broke** -- the fast label counted beside each, and `test_invariants` is `#16`: `S190_add_piece_no_eval` (the `accepts:` mutant) red at `material expected 4334 got 3618` with `psqt_mg` and `psqt_eg`, 6 red (`#5` `test_search`, `#6` `test_engine`, `#16`, `#18` `test_uci_surface`, `#19` `test_mate_breadth`, `#21` `test_mate_carry`); `S190_remove_material` red at `material expected 4334 got 4522`, 7 red (adds `#20` `test_mate_pv`); `S190_remove_psqt_mg` red at `psqt_mg expected 1794 got 2010`, 7 red; `S190_remove_psqt_eg` red at `psqt_eg expected 1865 got 2052`, 7 red; `S190_remove_phase` red at `phase expected 12 got 16` **after unmake**, 4 red; `S190_move_piece_squares` red at `psqt_mg` plus `squares[51]`, 4 red (`#1` `test_chesso`, `#6`, `#16`, `#21`). Anchors in the `(old, new)` form of `adocs/data/2026-09-04_test_review/mutants.py`, for S196 to adopt. **Two of the guide's predictions came out wrong and are recorded as wrong**: the squares mutant did **not** redden perft -- `#3` `test_movegen` *"shallow perft matches every column"* stayed green, because `squares[]` is not what the generator reads -- and the phase mutant reddened three other binaries rather than none. No mutant was caught by `test_invariants` alone: `test_engine`'s existing `memcmp` sees every one of them **after unmake**, which is the half F01 already had; the half this step adds is **after make**, and the phase mutant is the case that shows the difference, failing at line 131 and not 123. **Goldens (DEC-142):** the five census floors are re-derived by `adocs/data/S190_walk_census.py`, which counts the same tree with python-chess and agreed with the engine's walk exactly -- **2132167 / 15023 / 181 / 221928 / 206212** makes / castlings / en passants / promotions / capture promotions -- so the floors are half of each, rounded down: 1000000 / 7000 / 90 / 100000 / 100000. **INV-6 discharged on identity against `dc878de` built in `.ref-builds/s190_head`**: 121530 / 801481 / 72924 at depth 9 and 636677 / 3520847 / 494098 at depth 12, `c3d5` / `e2a6` / `d7c8q` at both, identical on both sides; `chesso bench` **26851183 nodes** on both, hence `No functional change` (DEC-140). The step file's quoted 121512 / 800769 / 62907 are pre-S148 and were not the baseline. **The section 7 timing formality resolved nothing and that is the recorded outcome**: `hyperfine -w 2 -r 10` over `search_bench ... 12` read the working tree 1.11x faster than the reference, and 1.08x *slower* with the order swapped, while an **A/A of the same binary against itself read 1.07x** -- so the probe's own noise floor at this shape is 7 to 11 % on a 0.8 s run dominated by process startup, both A/B readings sit inside it, and no difference is claimed. Governor `performance`, machine idle (top process firefox at 6 %). **The Debug self-play line is traced to observed output, not assumed** -- the guide flagged this as unestablished. A Debug binary carrying `S190_remove_phase` aborted in every game, and `grep -c Assertion` read **0** on a default `-log file=...` (WARN, the form `fastchess.sh` uses) against **2** on the same run at `level=trace engine=true`; the abort text reaches the `-log` file and never the tee'd stdout, while `disconnect` reaches both. `DEV_MANUAL.md` carries the line with that level and the reason. Clean run, 4 rounds at 4+0.04, concurrency 8: **8 games in 19 s, 0 `Assertion`, 0 `disconnect`, 0 crashes.** **Docs:** `adocs/specs.md`'s INV-2 row names the test beside the Debug assert and INV-4's row drops its three `src/bitboard.cpp:` line citations for the test title and the symbol (DEC-135); `DEV_MANUAL.md` "Test" is rewritten -- the "**not** in the gate" sentence now applies to the Debug half only -- and gains "The Debug self-play habit" for DEC-141; `tests/CMakeLists.txt` carries both measured costs. `MANUAL.md` unchanged: no UCI surface moved, `test_uci_surface` green. `README.md` untouched. Gate green in both builds, **31/31 each**, format clean under `CLANG_FORMAT_MAJOR=22` (DEC-146). Closes `2026-09-04_test_review-F01`.

## Why this exists

`2026-09-04_test_review-F01`. Both gated builds are `Release`, so every
`assert(` in `src/` is dead in them, and INV-4's only enforcement is
`assert(eval_accumulators_match(...))` at three sites in `src/bitboard.cpp`;
INV-2's `squares[]` half is the same shape. The hash already has a Release
oracle -- `tests/test_engine.cpp`'s "hash and board survive make/unmake"
compares against `compute_full_hash` after every make -- and it caught the
en-passant hash mutant of the review's fault-injection pass. This step gives
the accumulators the same oracle. `CLAUDE.md` names the accumulator alphabet
as the NNUE hook; a drift there is a wrong static score everywhere and an SPRT
reads it as strength.

## Cost

Machine-free, hours. The Debug self-play habit is a rule (DEC-141) and costs
minutes per step that touches the paths it names.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

`make_move` keeps four evaluation sums (`material`, `psqt_mg`, `psqt_eg`,
`phase` in `src/data_structures.hpp` `board_t`) and a piece-per-square array
(`squares[64]`) current with small additions on every piece event instead of
recomputing them from the bitboards. Two helpers in `src/bitboard.cpp`,
`eval_accumulators_match` and `squares_match_bitboards`, rebuild both from
scratch and compare, and `make_move_impl` and `unmake_move_impl` assert them at
entry and exit -- but helpers and asserts sit under `#ifndef NDEBUG`, and both
gated build directories are `Release` (`-O3 -DNDEBUG`, `build/CMakeCache.txt`),
so nothing the gate runs ever executes them (`2026-09-04_test_review-F01`).
This step writes a Release doctest binary that walks a move tree over the test
corpus and runs the same two comparisons after every make and unmake, exposes
the helpers to the tests, and documents the DEC-141 self-play habit. The
engine's behaviour does not change.

### 2. The technique as published

Recompute-and-compare for incrementally maintained state. The wiki: "It is a
good idea to compare incremental updated stuff like zobrist keys with its from
scratch generated counterparts for debug purposes"
(https://www.chessprogramming.org/Incremental_Updates). The chessprogramming.net
debugging article makes it a routine asserted at entry and exit: "Use 'asserts'
liberally in conjunction with a 'board_integrity' routine. The
'board_integrity' routine is key"
(https://www.chessprogramming.net/debugging-a-chess-move-generator/). Chesso has
that shape in Debug only. The Release half follows the repository's own
precedent, `tests/test_engine.cpp` *"hash and board survive make/unmake"*,
which compares `board.hash` with `compute_full_hash` after every make and caught
mutant M23 of the review. The self-play habit follows Stockfish's CI, read from
its workflow file only: `make -j build debug=yes`, then `fastchess -rounds 4 -games
2 -repeat -concurrency 4 ... -each proto=uci tc=4+0.04 -log file=fast.log | tee
fast.out`, failing on `! grep "Assertion" fast.log` and `! grep "disconnect"
fast.out`
(https://raw.githubusercontent.com/official-stockfish/Stockfish/master/.github/workflows/games.yml).

### 3. What chesso has today, and where the change plugs in

- The accumulators are written only through `src/eval_tables.hpp`
  `eval_add_piece`, `eval_remove_piece` and `eval_refresh`, all header-inline;
  `eval_refresh` zeroes the four sums and re-adds every piece in `squares[]`,
  so it is the from-scratch oracle. `src/bitboard.cpp` `add_piece`,
  `remove_piece` and `move_piece` call the first two in `make_move_impl`;
  `unmake_move_impl` calls them open-coded, in reverse. `load_FEN` ends with
  `eval_refresh`, so `tests/test_evaluation.cpp` *"colour symmetry over every
  test position"* only ever sees rebuilt sums.
- `eval_accumulators_match` runs `eval_refresh` on a copy and compares the four
  fields; `squares_match_bitboards` scans 64 squares against the twelve piece
  bitboards. Both are `static` inside `#ifndef NDEBUG`, asserted at the entry
  of `make_move_impl`, before its side switch, and at the end of
  `unmake_move_impl`.
- Corpus: `tests/test_helpers.hpp` `all_test_fens()`, 2696 distinct FENs from
  the eight `json_test_files` -- the list `tests/test_movegen.cpp` *"captures
  and quiets partition the list"* walks with `check_split`. The five: the
  `positions` vector of *"hash and board survive make/unmake"*, depths 4, 3,
  4, 3, 3.
- Edits, in order. (1) `src/bitboard.cpp`: move the two helpers above the
  `#ifndef NDEBUG` block and drop `static`; the `assert(...)` call sites stay
  as they are; declare both in `src/bitboard.hpp` beside `make_move` with a
  comment that the search never calls them. (2) `tests/test_invariants.cpp`
  (section 6). (3) `tests/CMakeLists.txt`: `add_doctest_target(test_invariants)`
  with a comment carrying the measured cost, as the `test_mate_breadth` block
  does; the shared `CHESSO_TEST_TIMEOUT` suffices. (4) Red under the mutants
  of section 6, then green. (5) `adocs/specs.md`, `DEV_MANUAL.md`.
- Why move the helpers rather than a compile definition: they live in a
  translation unit of `chesso_engine`, the one library the tests and the
  engine both link (`src/CMakeLists.txt`), so a define on the test target
  cannot reach them and one on the library changes the shipping binary. Two
  functions no Release path calls add a few hundred bytes and nothing to the
  search; INV-4's "accumulated, not recomputed" is `evaluate()`'s hot path,
  untouched. INV-6 identity is still shown (section 7).

### 4. Constants and seeds

No search parameter; nothing enters `src/search_params.hpp`. Every number is
form (b), measured 2026-09-05 with a scratch walker linked against
`build/src/libchesso_engine.a` (Release, `-O3 -march=native`, MacBook M1 of
`.moltke.local.md`; "compares" means both helpers after every make and unmake):

| walk | `make_move` calls | bare | with compares |
|---|---|---|---|
| corpus, depth 2 | 1,705,714 | 0.16 s | 1.13 s |
| corpus, depth 3 | 53,975,914 | 3.82 s | about 34 s, extrapolated at 0.28 µs per compare |
| five at 4/3/4/3/3 (the hash oracle's) | 426,453 | 0.06 s | 0.27 s |
| five, all at depth 4 | 7,037,876 | 0.53 s | 4.45 s |
| **corpus depth 2 + five at 4/3/4/3/3** | **2,132,167** | 0.16 s | **1.4 s** |

- **Corpus depth 2**, not the `accepts:`' 3: depth 3 is 54 M makes and cannot
  meet "under 5 s" on any machine this project has used (section 10). One
  `constexpr int CORPUS_DEPTH = 2`, so S197's `tools/gate_extra.sh` can later
  run deeper through an environment override if wanted.
- **Five positions at the hash oracle's depths**, copied from
  `tests/test_engine.cpp`, not a flat 4 (4.45 s on its own).
- **Census floors** (DEC-142 goldens, named at their site): at least 1,000,000
  makes, 1,000 castlings, 100 en passants, 1,000 promotions, 1,000
  capture-promotions, against measured 2,132,167 / 15,023 / 181 / 221,928 /
  206,212 -- about half, so a corpus edit that halves a class stays green and
  one that empties it goes red. Re-derived by `adocs/data/S190_walk_census.py`
  (python-chess in `~/.venv/chess`, same JSON files and depths, counting
  pushes by `is_castling`, `is_en_passant`, `promotion`, `is_capture`); write
  it with the step, quote its output in the stamp.
- **Timeout:** the shared 60 s Release / 600 s Debug. Debug estimate from
  `test_movegen`'s 165 s over 54 M makes, about 3 µs a make: 10 to 15 s.
  Measure once; both figures go in the CMake comment.

### 5. Interactions and traps

- **Per-node strings are the cost, not the compares.** The hash oracle builds
  `print_move(...) + " from " + root_fen` for every `REQUIRE_MESSAGE`, which is
  why `test_engine` spends 3.2 s on 426 k makes. Call the helpers as bare
  booleans; build the diagnostic only in the failing branch, with `FAIL`
  rather than two `REQUIRE`s per make (section 6).
- **The corpus holds unreachable positions** (`position_is_reachable` exists
  for them). The generator is legal-only (INV-1) and emitted zero king
  captures over the whole walk, so no filter is needed; add none -- skipping
  is how a walk shrinks quietly. The census floors are the guard.
- **`excludes:`** -- `make_move_impl`, `unmake_move_impl`, `add_piece`,
  `remove_piece`, `move_piece` and the `assert(...)` lines are untouched; the
  only `src/` change is the two helpers' linkage, placement and declaration.
- **Both builds** run the new test (DEC-118). Under Debug the asserts inside
  `make_move` abort before the test's own compare; record each mutant's
  Release observation, the one that matters. Assets are relative to the
  working directory: `ctest` runs in `build/tests`, a manual run from `tests/`.
- **fastchess and the assertion text.** Whether an abort's text lands in the
  `-log` file or on the run's stderr is not established here; the documented
  line captures both (`2>&1 | tee`). Verify it once against a Debug build
  carrying a section 6 mutant so "grep for `Assertion`" is traced to observed
  output (DOCS rule). Time forfeits by a 25 to 40x slower binary at 4+0.04
  are not the failure condition; `Assertion` and `disconnect` are.
- `adocs/data/2026-09-04_test_review/mutants.py` is evidence, "added, never
  edited" (`adocs/data/README.md`): apply mutants by hand in its `(old, new)`
  form and list them in the stamp for S196 to adopt into `tools/mutants/`.

### 6. Tests

`tests/test_invariants.cpp`: one suite, a static `game_t`, a fixture calling
`initialize_game_const_data` as `engine_fixture_t` does.

```
static uint64_t makes, castlings, en_passants, promotions, capture_promotions;

static std::string mismatch(const board_t* b, move_t mv, const char* when)
{ // failing branch only: rebuild (eval_refresh on a copy), then return
  // when + print_move(mv) + " in " + generate_FEN(b) followed by
  // " <field> expected <rebuilt> got <actual>" for each differing sum and
  // " squares[<sq>] expected <piece> got <piece>" for each differing square
}

static void walk(game_t* g, int depth)
{
  if (depth == 0) return;
  move_t moves[MAX_MOVES]; n = generate_moves(game_tables(), &g->board, moves);
  for each moves[i]:
    board_t before = g->board;                        // INV-2's exact inverse
    if (!make_move(g, moves[i])) continue;
    ++makes; count MOVE_CASTLING, MOVE_EN_PASSANT, MOVE_PROMOTED,
             MOVE_PROMOTED && MOVE_CAPTURE;
    if (!eval_accumulators_match(&g->board) || !squares_match_bitboards(&g->board))
      FAIL(mismatch(&g->board, moves[i], "after make "));
    walk(g, depth - 1);
    unmake_move(g);
    if (!eval_accumulators_match(&g->board) || !squares_match_bitboards(&g->board))
      FAIL(mismatch(&g->board, moves[i], "after unmake "));
    if (memcmp(&before, &g->board, sizeof(board_t)) != 0)
      FAIL("unmake did not restore the board after " + print_move(moves[i]) ...);
}

TEST_CASE_FIXTURE(fixture, "accumulators and squares survive make/unmake over the corpus")
  for fen in all_test_fens(): REQUIRE(load_FEN(fen, &game)); walk(&game, CORPUS_DEPTH);
  for (fen, depth) in the five at 4,3,4,3,3: REQUIRE(load_FEN(...)); walk(&game, depth);
  // Goldens (DEC-142): floors at half the 2026-09-05 census; re-derived by
  // adocs/data/S190_walk_census.py.
  REQUIRE(makes >= 1000000); REQUIRE(castlings >= 1000); REQUIRE(en_passants >= 100);
  REQUIRE(promotions >= 1000); REQUIRE(capture_promotions >= 1000);

TEST_CASE_FIXTURE(fixture, "the oracles see a planted drift")   // precondition
  load_FEN(DEFAULT_POSITION); game.board.phase += 1;  REQUIRE_FALSE(eval_accumulators_match(...));
  load_FEN(DEFAULT_POSITION); game.board.squares[e4] = W_QUEEN;  REQUIRE_FALSE(squares_match_bitboards(...));
```

The `memcmp` is INV-2's own wording at one 220-byte copy per make, beyond the
`accepts:` minimum (section 10). The second case keeps a helper that returns
`true` unconditionally from passing the first.

**Mutants** -- each anchor occurs once in its file, verified 2026-09-05:

| id | file | `old` -> `new` |
|---|---|---|
| S190_add_piece_no_eval (the `accepts:` mutant) | `src/bitboard.cpp` | in `add_piece`: the four lines `board->squares[square] = piece;`, the hash xor, blank, `eval_add_piece(board, piece, square);` -> the same without the last |
| S190_remove_material | `src/eval_tables.hpp` | `board->material -= piece_value[piece];` -> removed |
| S190_remove_psqt_mg | `src/eval_tables.hpp` | `board->psqt_mg -= psqt_mg[piece][square];` -> removed |
| S190_remove_psqt_eg | `src/eval_tables.hpp` | `board->psqt_eg -= psqt_eg[piece][square];` -> removed |
| S190_remove_phase | `src/eval_tables.hpp` | `board->phase -= phase_value[piece];` -> removed |
| S190_move_piece_squares | `src/bitboard.cpp` | in `move_piece`: `board->squares[from] = EMPTY;` -> removed |

For each: apply, `cmake --build build -j8`, run `../build/tests/test_invariants`
from `tests/` and see red naming the field; then `ctest --test-dir build -L
fast` and record which other binaries went red (the squares mutant should also
redden perft; the phase one may redden nothing else -- F01's point). Revert
with `git checkout -- src`.

**Not owed:** no pruning rule, so no guard test; no `make_move` body change,
so no Debug self-play is owed -- run it once anyway to verify the documented
line. **INV-6:** `python3 tools/search_bench.py ./build/src/chesso 9` and `...
12` before and after: 121512 / 800769 / 62907 and 639228 / 3430710 / 367858,
best moves `c3d5` / `e2a6` / `d7c8q` (`.moltke.local.md`, both machines).

**The self-play line for `DEV_MANUAL.md` "Test"**, after the paragraph "The
debug build asserts `squares[]` ...", built from `fastchess.sh`'s invocation
with the reference replaced by the same Debug binary:

```bash
cmake --build build-debug -j8
fastchess -engine cmd=build-debug/src/chesso name=debug-a \
          -engine cmd=build-debug/src/chesso name=debug-b \
          -openings file=books/UHO_Lichess_4852_v1.epd format=epd order=random \
          -each tc=4+0.04 option.Hash=16 option.Threads=1 \
          -rounds 4 -repeat -concurrency 8 -check-mate-pvs \
          -pgnout file=/tmp/debug_selfplay.pgn -log file=/tmp/debug_selfplay.log \
          2>&1 | tee /tmp/debug_selfplay.out
grep -c Assertion /tmp/debug_selfplay.log /tmp/debug_selfplay.out   # both 0
grep -c disconnect /tmp/debug_selfplay.out                          # 0
```

### 7. Measurement

Behaviour-neutral lane: no SPRT, no bounds pair, no A/A. Proof is the INV-6
identity plus one interleaved timing as a formality, since no search line
changes: build `HEAD` in a worktree (`git worktree add .ref-builds/s190 HEAD`,
configured as `build/`), then on an idle machine on mains (`ps aux | sort
-rnk3 | head`) `hyperfine -w 2 -r 10 "python3 tools/search_bench.py
./build/src/chesso 12" "python3 tools/search_bench.py
.ref-builds/s190/build/src/chesso 12"`; within noise, under 3 % (CLAUDE.md).
The `accepts:`' wall-time claim is its own measurement: `cd tests && hyperfine
-w 1 -r 10 ../build/tests/test_invariants`, mean and sigma into the stamp with
the machine named. No `adocs/data/S190_*.sh`; the census script is the one
data file.

### 8. Completion checklist

- Gate: `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`;
  once S189 has landed, `tools/gate.sh`, which runs the same chain and checks
  the signature. The new file passes `./clang-format.sh --check` (S054 covers
  untracked files).
- Commit body ends with `No functional change` (`src/` touched, totals the
  parent's; DEC-140 from S189's completing commit on).
- `adocs/specs.md`: the INV-2 row gains `test_invariants` *"accumulators and
  squares survive make/unmake over the corpus"* beside its Debug clause; the
  INV-4 row drops its three `src/bitboard.cpp:` line citations for the test
  title and "`eval_accumulators_match`, asserted in `make_move_impl` and
  `unmake_move_impl` in Debug, called by the test in every build" (DEC-135).
- `DEV_MANUAL.md` "Test": the target, what it enforces, its measured cost;
  the sentence "It is **not** in the gate above and running it is still on
  you" is rewritten, since the Release half now is; the self-play line and
  the two greps. `MANUAL.md`: no UCI change -- say so in the stamp.
  `README.md` untouched.
- Stamp: census counts and wall times on the workstation; the mutant table
  with what each reddened; INV-6 counts; the hyperfine lines; the self-play
  run's game count and both greps; F01 closed. `plan.md` and `status.md`
  through the coordinator.

### 9. Sources read

- https://www.chessprogramming.org/Incremental_Updates -- the recompute-and-compare sentence quoted in section 2 (fetched).
- https://www.chessprogramming.net/debugging-a-chess-move-generator/ -- the `board_integrity` routine asserted at entry and exit (fetched).
- https://raw.githubusercontent.com/official-stockfish/Stockfish/master/.github/workflows/games.yml -- CI configuration only: `debug=yes`, 4 rounds at `tc=4+0.04`, the two greps (fetched; no engine source read).
- Repository: `adocs/audit/2026-09-04_test_review.md` F01; DEC-025, DEC-118, DEC-139 to DEC-142; `adocs/specs.md` invariants; `adocs/testing_strategy.md` 3.1; `adocs/plan_todo/` S189, S192, S196, S197; `adocs/plan_done/` S161, S164; `src/bitboard.cpp`, `src/bitboard.hpp`, `src/eval_tables.hpp`, `src/data_structures.hpp`, `src/CMakeLists.txt`, `CMakeLists.txt`, `cmake/arch.cmake`; `tests/test_engine.cpp`, `tests/test_movegen.cpp`, `tests/test_helpers.hpp`, `tests/test_perft.cpp`, `tests/CMakeLists.txt`; `adocs/data/2026-09-04_test_review/` `mutants.py`, `run.py`, `results.tsv`, `kills.txt`; `adocs/data/README.md`; `fastchess.sh`; `DEV_MANUAL.md` "Test", "Measure"; `.moltke.local.md`. Section 4's timings are from a scratch walker of 2026-09-05, not committed. No figure unverified.

### 10. Questions deferred to the owner

1. **The `accepts:` is unsatisfiable as written:** "every test FEN to depth 3"
   is 53,975,914 `make_move` calls, 3.8 s bare and about 34 s with the
   compares, against "Release wall time under 5 s". Proposed: depth 2 over
   every test FEN plus the hash oracle's depths (4, 3, 4, 3, 3) on its five
   positions, measured 1.4 s on the MacBook -- or keep depth 3 and raise the
   budget to 60 s, which the shared timeout only just holds.
2. Whether the `memcmp` before/after (INV-2's exact-inverse clause) belongs in
   this test or stays with the hash oracle alone.
3. Whether an environment override of the walk depth for S197's
   `tools/gate_extra.sh` is wanted now, or left to S197.
