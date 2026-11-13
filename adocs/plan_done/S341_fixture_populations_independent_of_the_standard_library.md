id:         S341
goal:       the fast label is green under clang with libc++: no pinned golden rests on a population the standard library's own shuffle or distribution drew
accepts:    (1) `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE` on the tree, where at S340's fix it ends `GATE-EXTRA-FAILED: clang` on `test_tuner_gradient` in both clang directories, the failure seen first; (2) an inventory, in this file, of every `std::shuffle` and `std::*_distribution` under `tests/` and in the tools a golden's script runs, each read as one of: a property that holds for any draw (left alone, with the reason), or a population a pinned number rests on (made the same under both libraries by drawing from the generator's raw output with arithmetic the tree states itself); (3) every golden that rests on a population this step moves is re-derived by its own named script (DEC-142), never re-read from a run, and each such script reads the same under GCC and under clang; (4) DEC-356's pair is re-derived by `adocs/data/S124_factor_gradient_rows.cpp`'s rule on the new population; if the rule answers no pair, or any `test_tuner_gradient` bound recurs, the step stops for the owner (DEC-344 (9)) and changes nothing of the pair, the push or the bound; (5) no assertion is removed, no bound widened, no case skipped under either compiler; both gated builds green, `chesso bench` the parent's unless (6) applies; (6) `tools/datagen.cpp` and `src/chesso.cpp` draw with a library distribution too: the step reads whether any golden or recorded corpus depends on them and reports it, and changes neither without the coordinator's word, since either moves what a seed reproduces
touches:    tests/test_endgame_positions.hpp, tests/test_tuner_gradient.cpp, adocs/data/S124_factor_gradient_rows.cpp, DEV_MANUAL.md, and whatever the inventory names
excludes:   the finite-difference method itself (B1, with the owner); any evaluation or search change; moving the clang stage into the automatic gate; the MacBook's own build, which is the owner's to run
decisions:  DEC-171, DEC-142, DEC-344, DEC-356
closes:
blocks:     S340
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator, 2026-11-12
done:       2026-11-13: `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE`, `build-clang` and `build-clang-tune` 61 of 61 (red first on `739a537`); `placements()` draws with the tree's own `shuffle_squares()`, one population under libstdc++ and libc++ (5719 rows at 40, 11436 at 80); DEC-359: DEC-356's pair replaced by its rule's two pairs, re-derived by `S124_factor_gradient_rows` under g++ 13.3 and clang++-22 with libc++, the same output, exit 0; the case 8.30801e-7 at 435 over 5721 rows under both compilers, bound 1e-6 unchanged; no other golden moved; both gated builds 61 of 61, clang-format clean, `chesso bench` 4707943, `src/` untouched; the inventory in this file; `tools/datagen.cpp` and `src/chesso.cpp` unchanged (DEC-359); fast check: nothing blocking, three wording lows fixed before the commit; DEV_MANUAL.md updated, MANUAL.md checked, no change.

## Why

Found by S340's implementer, 2026-11-12. With the two functions marked, clang compiles the whole tree in `build-clang`
and `build-clang-tune`, and the fast label then reads 60 of 61 in each: `test_tuner_gradient`'s case "the endgame
factor's gradient is the derivative, chain rule in" fails `CHECK( check.worst < 1e-6 )` at 4.70154e-06, over 5696
rows (`.tuning/coord/S340/green/01_clang.log`). Under GCC the same case reads 6.27485e-7 over 5717 rows and is green.

The engine and trace libraries are not the cause: HEAD's and the fixed `src/evaluation.cpp` compile to byte-identical
objects under clang. The likely cause, not yet verified: `placements()` in `tests/test_endgame_positions.hpp` draws
its squares with `std::shuffle`, whose algorithm the standard leaves to the implementation, so libc++ and libstdc++
draw different populations from one `std::mt19937` seed, and DEC-356's pair was picked on GCC's. The case never ran
under clang before, because clang's build stopped at S340's two errors, and before phase 1's landing on 2026-11-06 the
case did not exist.

A second sign of the same class, on S124's candidate: `test_datagen_repetition`'s precondition is red under GCC and
green under clang's library (`.tuning/coord/S124/gate_extra_cand/`). `tools/datagen.cpp` picks its random moves with
`std::uniform_int_distribution`.

Filler under DEC-171: a test fixture, no play, no score, no UCI output. It blocks S340, whose first accept is the clang
stage's `GATE-EXTRA-DONE`, and it is what the owner's MacBook reads on `ctest -L fast`.

## Shape

One Opus subagent, agent-only, no run. **It starts after S124's verdict**: S124's staged candidate moves
`tests/test_tuner_gradient.cpp` and the coverage blocks, and nothing compiles while its SPRT plays. If S124 lands, this
step works on the landed tree.

## 2026-11-12: the cause verified, the shuffle made the tree's own, and stopped for the owner at accept (4)

Worked on `739a537` (S124 read H0, so the family stays at its identity), worktree `../chesso-s341`, branch `s341`.
Outputs under that worktree's `.tuning/s341/`. **Accept (4)'s stop fired**: on the library-independent population
the case is red under both compilers with DEC-356's pair, and the rule answers two other pairs. Nothing of the pair,
its script or the bound was changed; the fixture's shuffle is changed in the worktree and not landed.

**Red first (accept (1)).** `STAGES=clang OUT=.tuning/s341/red tools/gate_extra.sh` on `739a537` ended
`GATE-EXTRA-FAILED: clang`: 60 of 61 in `build-clang` and in `build-clang-tune`, "the endgame factor's gradient is the
derivative, chain rule in" reading `4.70154e-06 at parameter 541 of 482, over 5696 rows` in both, S340's figure.

**The cause, verified.** `std::mt19937` seeded 20261104 gives the same raw output under g++ 13.3 with libstdc++ and
clang++-22 with libc++ (286892023 3214013954 96837808 2647951384), and `std::shuffle` of 0 to 63 from it gives other
permutations (11 61 62 29 ... against 55 3 50 27 ...) and leaves the engine in other states. The populations, from a
program over `tests/test_endgame_positions.hpp` (FNV-1a over the rows in order):

| | libstdc++ | libc++ |
|---|---|---|
| `placements()`, at 40 | 5719 rows, `fd3d8f17436ab8bc` | 5696 rows, `c155e2a9ec1e8281` |
| `placements(80)`, the rule's candidates | 11339 rows, `9674a863003f8241` | 11422 rows, `4af47ed051cbf033` |
| the case's rows, E1 non-zero, the pair in | 5717 | 5696 |

And the population is the whole cause: `adocs/data/S124_factor_gradient_rows.cpp` built by clang++-22 against libc++
and `build-clang`'s trace library, fed libstdc++'s two lists from a file, prints the options file's lines exactly --
base 5715 rows, 3.4757e-06 at 726; the pick at candidate 233 of 11339, DEC-356's pair; 6.27485e-07 at 739 over 5717
rows; "reproduces the landed pair: yes".

**The change, in the worktree.** `tests/test_endgame_positions.hpp` `shuffle_squares`: Fisher and Yates' shuffle in
Durstenfeld's form over the generator's raw output, the index the raw value modulo the span left, in place of
`std::shuffle` in `placements`; the tree's own form beside the coverage-row programs' `rng() % n`. Under both
libraries it now draws one population: `placements()` 5719 rows, `451640061825b69d`; `placements(80)` 11436 rows,
`f88124ba5629600e`; the case's rows 5719. Saved as `.tuning/s341/s341_shuffle.patch`.

**On that population** (identical to every printed digit under both compilers, so the arithmetic agrees and the
population was the difference):

- the case, DEC-356's pair appended: **8.99624e-06 at 646**, `psqt_eg[queen][b8]`, over 5719 rows, red in `build`,
  `build-tune`, `build-clang` and `build-clang-tune`; its other 11482 assertions pass. Every other reader of
  `population()` is green in all four: `test_eval_model`, `test_eval_trace`, `test_audit_design`, the endgame plant.
- the rule (`S124_factor_gradient_rows`, both compilers, output identical line for line): base 5717 rows, worst
  8.52919e-06 at 646, next 7.75486e-06 at 484 (`psqt_eg[knight][h5]`); round 1, TARGET 646, base |g| 2.536e-10, picks
  `1Q6/7k/7b/8/8/8/K7/8 w - - 0 1` and its mirror (candidate 452 of 11436), the case then 1.3848e-06 at 679
  (`psqt_eg[queen][c4]`), still red; round 2, TARGET 679, picks `8/6k1/8/1b3K2/2Q5/8/8/8 w - - 0 1` and its mirror
  (candidate 464), the case **8.30801e-07 at 435** (`psqt_eg[pawn][g3]`) over 5721 rows, x1.20 under 1e-6;
  "reproduces the landed pair: NO", exit 1. DEC-316's class again: a near-stationary weight, |g| 1e-10 at 646.
- the rule's four rows in place of the pair, applied to a scratch copy and reverted (`sha256sum -c` after): the case
  8.30801e-07 at 435 in `build` and `build-clang`, every reader above green in both, and the rule exits 0 under both.

### Accept (2): the inventory

`grep -rn 'std::shuffle\|_distribution'` over the tree's C++ at `739a537`. `adocs/data/` holds none: its coverage-row
programs already draw `rng() % n` from the raw output. [rand.predef] fixes what `std::mt19937` and `std::mt19937_64`
return from a seed; the distributions' and the shuffle's algorithms are each library's own.

| site | draws with | reading |
|---|---|---|
| `tests/test_endgame_positions.hpp` `placements` | `std::shuffle` | **population**: DEC-356's pair is the rule's pick over `placements(80)`, and the case's 6.27485e-7 at 739 over 5717 rows, its comment and `DEV_MANUAL.md`'s golden row rest on it. Made the tree's own, above; since the 2026-11-13 landing the rule's two pairs and the case's 8.30801e-7 at 435 over 5721 rows rest on it |
| `tests/test_eval_model.cpp` `family_population` | `uniform_int_distribution`, `std::shuffle` | property, left alone: engine equals walk on each position, for any draw; the rest are coverage floors (`checked > 10000`, every cell `> 0`, both sets `> 100`), green under both libraries |
| `tests/test_eval_model.cpp` `threat_population` | the same | property, left alone: engine equals walk on each position; floors `> 50` and `> 20`, which "the engine counts the endgame family the walk counts" already calls preconditions, not goldens |
| `tests/test_evaluation.cpp` "the static score never reaches the mate band" | the same | property, left alone: the bound holds on each placement load_FEN takes; the rest are non-vacuity floors |
| `tests/test_evaluation.cpp` "the shared pawn sets count what the definitions say" | the same | property, left alone: counts equal the definitions on each position; its comment already says 17587 is libstdc++'s and sets the floor at 17000 for that reason |
| `tests/test_evaluation.cpp` `passer_mirror_population` | the same | property, left alone: the mirror identity on each position; edge-file preconditions as floors |
| `tests/test_evaluation.cpp` `threat_mirror_population` | the same | property, left alone: as above |
| `tests/test_evaluation.cpp` "addition and multiplication by a signed integer act on both halves" | `uniform_int_distribution` | property, left alone: an identity for any operands |
| `tests/test_evaluation.cpp` "taper truncates exactly as the unpacked formula does" | the same | property, left alone: an identity for any operands |
| `tests/test_chesso.cpp` `pick_random_move` | the same | property, left alone: make and unmake restore the board on any walk; `CHESSO_TEST_SEED` replays another |
| `tests/test_tuner_split.cpp` `row_level_index` | `std::shuffle` | property, left alone: held identical to `tuner_split::split`, which shuffles the same way under the same library; the other cases are no-straddle, fraction and seeding properties |
| `tools/tuner_split.hpp` `split` | `std::shuffle` | property for the suite, left alone: `tests/test_tuner_box.sh` fits at `--validation 0.25 --seed 1` on rows built so "any game the split holds out reads both"; no golden in `tests/` rests on which games are held out. A recorded fit's held-out reading does, under its own library |
| `tools/datagen.cpp` `random_move` | `uniform_int_distribution` | left alone, accept (6), below |
| `src/chesso.cpp` `search_book_move` | `uniform_int_distribution`, two draws | left alone, accept (6), below |

### Accept (6)

- `tools/datagen.cpp` `random_move` picks each opening ply through the library's distribution over `mt19937_64` seeded
  `seed + worker`, so a seed's games are a fact about the library that built the binary as well as about the commit.
  **No golden in `tests/` rests on them**: `tests/test_datagen_repetition.sh`'s "at least one game reached a
  recurrence" over six games at seed 5 is a precondition, green at `739a537` under both libraries (the red run above);
  S124's candidate flipped it under GCC by moving the engine, not the library. **The recorded corpora do**: S122's
  regenerated corpus (seed 2026500349), DEC-345's sample and every pool before them replay their games from their seed
  only with a libstdc++ build of their commit (inferred: the workstation builds with g++ 13.3, DEC-049; no corpus
  recipe names the library). A raw-output draw at HEAD breaks no reproduction through a recorded commit's tree, but
  HEAD's datagen would then play other games from every seed under GCC, so a top-up (DEC-296) or a replay with HEAD's
  binary would no longer extend or match a pool. Not changed.
- `src/chesso.cpp` `search_book_move` draws from `gen`, seeded by `std::random_device` unless `CHESSO_BOOK_SEED` is
  set, and only with `OwnBook` on, whose default is false. `tests/test_engine.cpp` "OwnBook draws a book move for the
  start key, and the seed replays it" asserts membership and same-seed agreement and says why no seed's move is read. Nothing recorded
  depends on it. Not changed; `src/` is untouched.

### What is the owner's (DEC-344 (9), DEC-356's Consequences)

The class recurs on `test_tuner_gradient`'s bound, now on a second library's population, and making the population
the tree's own moves DEC-356's pair. Options:

- **A. The tree's own shuffle and the rule's re-pick**: the worktree's change, DEC-356's pair replaced by the rule's
  two pairs, the case 8.30801e-07 at 435 over 5721 rows under both compilers, the rule exiting 0 under both.
  Moves the golden and thins the margin from x1.59 to x1.20, so a refit is likelier to bring the class back; the
  case's comment, the program's header and `DEV_MANUAL.md`'s row are rewritten at landing. What accepts (2) to (4)
  prescribe.
- **B. libstdc++'s population frozen as rows**: `placements()` returns a committed list of its 5719 rows (202429
  bytes) and the rule reads its 11339 candidates (401650 bytes) from `adocs/data/`. Nothing of DEC-356 moves, and
  clang reads the same numbers (the file-fed run above). Costs 600 KB of fixture text, the generator kept only as the
  record of how the list was drawn, and an amendment of accept (2)'s method.
- **C. Leave `739a537` as it is**: S340 stays paused, the clang stage stays red, and the owner's Mac reads 60 of 61 on
  `ctest -L fast` (inferred: Apple's libc++ is LLVM's).
- Not offered: reproducing libstdc++'s shuffle in the tree's own code, which means taking a library's implementation
  (the brief's copying line), and the finite-difference method (B1, excluded).

Recommended: A, the step's own method. The rule is the owner's DEC-356 rule run unchanged, both compilers agree to
every digit, and the class leaves this fixture for good; the thinner margin is the price, read against the fact that
any refit moves the reading anyway.

### Not done

The four gated runs and `STAGES=clang tools/gate_extra.sh` to their end on the worktree: the tree is red by
construction until the owner rules; only the population's readers were run, in all four directories. `DEV_MANUAL.md`
and `MANUAL.md` checked: `MANUAL.md` needs nothing (no UCI surface); `DEV_MANUAL.md` needs its golden row and a
paragraph beside S340's in the build section once an option lands, and is not edited before then.

## 2026-11-13: option A landed (DEC-359)

The owner ruled option A on 2026-11-12 (DEC-359): the shuffle lands, DEC-356's pair is replaced by the rule's two
pairs, the bound stays 1e-6. Worked in `../chesso-s341`, branch `s341`, fast-forwarded from `739a537` to `e6ef283`
(`git merge --ff-only achesso`; the three commits between touch `adocs/` and `.tuning/verify_fit.py` only, the
worktree's two edits carried across, their diff saved first as `.tuning/s341/before_ff.diff`). Outputs under the
worktree's `.tuning/s341/land_*`. `src/` untouched.

**The population, read again on `e6ef283`.** A scratch program over the header (row count, and FNV-1a 64 over the
rows in order, each row's bytes then a newline), built by g++ 13.3 against `build`'s engine library and by
clang++-22 `-stdlib=libc++` against `build-clang`'s: `placements()` 5719 rows `451640061825b69d`, `placements(80)`
11436 rows `f88124ba5629600e`, under both, the previous section's figures. `population()`, `placements()` and then
`factor_gradient_rows`: with DEC-356's two rows 5721 rows `e15251c604488ba6`, and on the staged landing, with the
rule's four, 5723 rows `27e243048db1227f`, each under both. (The case reads fewer rows than `population()` holds:
it drops the two rows of `placements()` whose E1 is zero at the shipped weights, so 5719 with the two, 5721 with the
four.)

**Red first on it, DEC-356's pair still in.** `test_tuner_gradient -tc="the endgame factor*chain rule in"`: `worst
relative error 8.99624e-06 at parameter 646 of 482, over 5719 rows`, `CHECK( 8.99624e-06 < 1e-06 )` failing, 11482
of 11483 assertions passing, in `build` and in `build-clang` (`land_red_old_pair.txt`).

**The rule, re-derived (accepts (3) and (4)).** `adocs/data/S124_factor_gradient_rows.cpp` built by its header's
line, and by the same line with `clang++-22 -stdlib=libc++` and `build-clang`, unchanged in what it computes. Run
with DEC-356's pair still landed, both print the same lines (`diff` empty) and exit 1, "reproduces the landed pair:
NO" (`land_rule_gcc_before.txt`, `land_rule_libcxx_before.txt`), the previous section's picks. The four rows were
then written into `factor_gradient_rows` from its `PICK` lines, and the program run again under both
(`land_rule_gcc.txt`, `land_rule_libcxx.txt`, `diff` empty), exit 0; below, its output less doctest's two banner
lines, its rule line and its status line:

```
base: 5717 rows; the case's worst 8.52919e-06 at 646, next 7.75486e-06 at 484
round 1: TARGET 646, base |g| 2.536382e-10, the rule wants 7.609145e-10
  pick after 303 eligible pairs (candidate 452 of 11436): |g| 5.401193e-08
    1Q6/7k/7b/8/8/8/K7/8 w - - 0 1
    8/k7/8/8/8/7B/7K/1q6 b - - 0 1
  the case with 5719 rows: worst 1.3848e-06 at 679 (|g| 1.925e-09), next 8.7432e-07 at 435
round 2: TARGET 679, base |g| 1.925360e-09, the rule wants 5.776079e-09
  pick after 4 eligible pairs (candidate 464 of 11436): |g| 9.443741e-09
    8/6k1/8/1b3K2/2Q5/8/8/8 w - - 0 1
    8/8/8/2q5/1B3k2/8/6K1/8 b - - 0 1
  the case with 5721 rows: worst 8.30801e-07 at 435 (|g| 1.207e-08), next 3.76149e-07 at 636
picked 4 rows
PICK 1Q6/7k/7b/8/8/8/K7/8 w - - 0 1
PICK 8/k7/8/8/8/7B/7K/1q6 b - - 0 1
PICK 8/6k1/8/1b3K2/2Q5/8/8/8 w - - 0 1
PICK 8/8/8/2q5/1B3k2/8/6K1/8 b - - 0 1
reproduces the landed pair (tests/test_endgame_positions.hpp's factor_gradient_rows): yes
[doctest] test cases:    1 |    1 passed | 0 failed | 0 skipped
[doctest] assertions: 6186 | 6186 passed | 0 failed |
```

Parameters by the layout (`EG_BASE` 389, pieces P N B R Q K, square 0 a8): 646 `psqt_eg[queen][b8]`, 679
`psqt_eg[queen][c4]`, 435 `psqt_eg[pawn][g3]`, 636 `psqt_eg[rook][h2]`, 484 `psqt_eg[knight][h5]`. The program's
printed word "pair" in its last line is left as it is: the program is unchanged.

**What moved.** One golden, by its own script: `factor_gradient_rows`, DEC-356's pair -> the rule's two pairs, and
with it the case's reading, under GCC 6.27485e-7 at 739 over 5717 rows -> **8.30801e-7 at 435 over 5721 rows**, under
clang 4.70154e-6 at 541 over 5696 rows (red) -> the same 8.30801e-7 at 435 over 5721. Margin x1.59 -> x1.20. Written
at its site (`tests/test_endgame_positions.hpp`, dated, DEC-356's text kept in the past tense, DEC-348), in the case's
comment in `tests/test_tuner_gradient.cpp`, in the program's header (a dated S341 section and the clang build line),
and in `DEV_MANUAL.md`'s golden row and its paragraph on the case; `DEV_MANUAL.md` also gains a paragraph beside
S340's in the build section. The program's header records its output in prose and points here for the whole of it.

**No other golden rests on the population.** Its readers, each read again on this tree, by its binary in all four
directories after the clang stage had built them (`land_readers.txt`):

- `tests/test_tuner_gradient.cpp` "the endgame factor's gradient is the derivative, chain rule in": `worst relative
  error 8.30801e-07 at parameter 435 of 482, over 5721 rows`, 11487 of 11487, in `build`, `build-tune`,
  `build-clang` and `build-clang-tune`.
- the same file's "a planted vector comes back: endgame" (`endgame_rows(false)`): tolerances against plants,
  `in_cells > 1000` and `in_domain > 500` preconditions; 13829 of 13829 in all four. Its comment's corner split
  (12.348 and 14.696) and epoch readings are attributed to the S124 step file and called budgets, not goldens; left
  alone.
- `tests/test_eval_model.cpp` suite "eval counts: S124's endgame family": engine against walk on each position,
  floors `checked > 15000` and every cell and count `> 20`, which its comment calls preconditions, not goldens.
  GCC builds `endgame family walk: 17834 positions; the rarest cell or count is non-zero on 57`, 179099 assertions
  (179079 recorded at DEC-356's landing, `adocs/data/S124_rebase_goldens.txt`, not re-read on `e6ef283`; inferred:
  `population()` holds two rows more, 10 assertions each); clang builds 17802 and 56, 178779 assertions: `population()`
  is the same under both, so the 32 positions apart are `threat_population()`'s, which the case adds and which still
  draws with the library (a property, the table's row). All green.
- `tests/test_eval_trace.cpp` "the factor's integer arithmetic is reconstructed at cells that move it": an identity
  on each row, floors (`checked > 8000`, `moved > 1000`, each sign `> 100`, every cell `> 20`); the one number named,
  2.906576, is `cell_truncation_positions`' row, not the population's; 43540 of 43540 in all four.
- `tests/test_audit_design.cpp` `fixture()`: design identities, `old_rank == 22` at the seed by the curve's
  structure, floors; `rows 8049, columns compared 430902, curve 62464, rows with both kings' x apart 6471, worst
  5.67525e-10 at 996`, the shipped rank 46, 34945 of 34945, the same in all four.
- `adocs/data/S124_factor_gradient_rows.cpp`: the golden's own script, above.

No other program includes `tests/test_endgame_positions.hpp`: `grep -l` over `adocs/data/*.cpp`, `tools/` and
`tests/` names these five files and the header. The coverage-row programs (`S101`, `S121`, `S123`, `S124`, `S125`)
draw their own placements with `rng() % n` and do not read this header; `S192_anchors.py` does not either.

**The inventory (accept (2)) on `e6ef283`.** `git grep -n 'std::shuffle\|_distribution'` over the tracked C++ names
the same draw sites as the table above, none added by the three document commits; its other hits are comments:
`src/bitboard.cpp`'s Zobrist generator and `tests/test_engine.cpp`'s two (S203, DEC-139, the precedent: the keys
left the library's distribution for the project's own splitmix64 for the same reason), and the new comment above
`shuffle_squares`. The table's first row carries a dated clause for the landing.

### The accepts, read one by one

1. **Holds.** Red first: `739a537`, the previous section (`.tuning/s341/red/01_clang.log`). On this tree
   `export CLANG_FORMAT_MAJOR=22; STAGES=clang OUT=.tuning/s341/land_clang tools/gate_extra.sh` reads `clang ok 422`
   in its table and ends `GATE-EXTRA-DONE 1 stages 422 s`: every target in `build-clang` and `build-clang-tune`
   compiled with no warning or error, and the `fast` label 61 of 61 in each (`land_clang/01_clang.log`). Run again on
   the staged landing after a last comment edit in the header: `clang ok 445`, `GATE-EXTRA-DONE 1 stages 445 s`, 61 of
   61 in each (`land_clang2/`).
2. **Holds.** The inventory is the table above with this section's re-reading on `e6ef283`: one population a pinned
   number rests on, made the tree's own; every other site a property, left alone with its reason.
3. **Holds.** The one golden that rests on the moved population is re-derived by its named script, never re-read
   from a run, and the script prints the same lines under GCC with libstdc++ and clang with libc++, before and after
   the rows were written.
4. **Holds, by the owner's ruling.** The rule's stop fired on 2026-11-12 and the step stopped and asked; DEC-359
   ruled A. The rule answered two pairs, both written; the bound and the push are unchanged, and the case reads
   8.30801e-7 under 1e-6 under both compilers.
5. **Holds.** No assertion removed, no bound widened, no case skipped: the diff under `tests/` is the shuffle, the
   four rows in place of two, and comments. Both gated builds 61 of 61 and `./clang-format.sh --check` clean
   (`land_gate_tier1.log`), and again in `tools/gate.sh --message`'s runs over the staged landing (`land_gate_1.log`,
   and `land_gate.log` after the fast check's three document notes). `chesso bench` reads `4707943 nodes` in `build`
   and `build-tune`, the parent's total (S340's record); `src/` is untouched. (6) does not apply: neither file
   changed.
6. **Holds, reported in the previous section.** Neither `tools/datagen.cpp` nor `src/chesso.cpp` changed; DEC-359
   keeps `datagen`'s draw on purpose.

`STAGES="prose citations" tools/gate_extra.sh` ends `GATE-EXTRA-DONE 2 stages` (`land_prose/`, and `land_prose2/`
after the fast check's notes), this file's 21 code citations unflagged. `MANUAL.md` checked: no UCI surface moved, unchanged. `DEV_MANUAL.md`: the golden row, the
case's paragraph and the build section's paragraph, above. `adocs/data/README.md`: the row for
`S124_factor_gradient_rows.cpp` takes a dated S341 clause, its DEC-356 pick put in the past tense (the fast check's
third note; the coordinator merges it with the main tree's rows).

### Draft `done:` stamp, for the coordinator

2026-11-13: `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE`, `build-clang` and `build-clang-tune` 61 of 61
(red first on `739a537`); `placements()` draws with the tree's own `shuffle_squares()`, one population under
libstdc++ and libc++ (5719 rows at 40, 11436 at 80); DEC-359: DEC-356's pair replaced by its rule's two pairs,
re-derived by `S124_factor_gradient_rows` under g++ 13.3 and clang++-22 with libc++, the same output, exit 0; the
case 8.30801e-7 at 435 over 5721 rows under both compilers, bound 1e-6 unchanged; no other golden moved; both gated
builds 61 of 61, clang-format clean, `chesso bench` 4707943, `src/` untouched; the inventory in this file;
`tools/datagen.cpp` and `src/chesso.cpp` unchanged (DEC-359).

### Not done

- The S124 plant scripts (`adocs/data/S124_phase1_plants.sh`, `S124_phase2_plants.sh`) were not re-run on the new
  population: they build scratch trees and scan a corpus, and the accepts do not ask for them. Inferred unaffected:
  each plant moves a derivative or a count by orders more than the population's choice can hide, and every per-cell
  precondition the guards read still passes.
