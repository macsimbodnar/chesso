id:         S203
goal:       `init_zobrist()` draws its 851 keys from the project generator under `CHESSO_PROJECT_SEED` instead of `std::uniform_int_distribution`, and the S170 case set is re-mined under the keys that result
accepts:    `src/bitboard.cpp` `init_zobrist` fills `piece_randoms`, `castling_randoms`, `side_randoms` and `ep_randoms` -- in that order, which is load-bearing -- from `project_random_next` seeded with `CHESSO_PROJECT_SEED`, and `#include <random>` leaves the file if nothing else there uses it; `./build/tools/magic_gen zobrist --seed 20260904` prints `matches init_zobrist: yes` where it prints `no` today, and its quality report is recorded; a fast test asserts the 851 keys equal that draw value for value in that order, and over them: none zero, all 851 distinct, no pair XOR equal to any key, no two pair XORs equal -- properties, not goldens (DEC-142); `bench_movegen` verifies its perft counts and `ctest -L slow -R test_perft` is green; **every best move from `tools/search_bench.py` at depths 9 and 12 unchanged** (`c3d5` / `e2a6` / `d7c8q`) and the node counts recorded before and after, the new pair written into `adocs/specs.md` under INV-6 and into `adocs/status.md`'s handover check in the same commit, with the new `bench` total in the message as `Bench: <n>` (DEC-140) and in `DEV_MANUAL.md`'s node-signature section; **`adocs/data/S170_cases.tsv` re-mined under the new keys** -- a fresh `./fastchess.sh --fast` class run whose `Incomplete mating PV` warnings are the source, the same six-or-fewer case shape, each row's `start`, `stride` and `go` swept for the cheapest reproduction as S170 did, and `expected_mate_lines()` in `tests/test_mate_carry.cpp` re-measured against it -- with `test_mate_carry` green in both builds and its red-before observed, not assumed; Debug self-play, 4 rounds at 4+0.04, `grep -c Assertion` = 0 (DEC-141); fast suite green in both builds
touches:    src/bitboard.cpp, tests/test_engine.cpp, tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv, adocs/data/S203_case_sweep.sh, adocs/data/S203_mine_cases.py, adocs/plan_todo/S202_shallow_inherited_mate_score.md, adocs/specs.md, adocs/status.md, adocs/plan.md, adocs/decisions.md, AGENTS.md, DEV_MANUAL.md
excludes:   the magic numbers (S179, done); the generator itself, `project_random_next` and `CHESSO_PROJECT_SEED` (S179 shipped all three); the seed value, which is settled at 20260904; closing S202's class -- a short mate line found while re-mining is the class S202 owns and is recorded, not fixed here
decisions:  DEC-139, DEC-154, DEC-142, DEC-122, DEC-150, DEC-151
closes:     2026-09-04_test_review-F08
blocks:
paused_by:
author:     Claude Opus 5 (coordinator)
done:       2026-09-08. **The 851 Zobrist keys are the project generator's, and the mating-PV fixture was rebuilt by a stated rule instead of by a match.** `init_zobrist` fills pieces, castling, side and en passant -- an order that is load-bearing and commented as such -- from `project_random_next` seeded with `CHESSO_PROJECT_SEED`; `#include <random>` left `src/bitboard.cpp`. `magic_gen zobrist --seed 20260904` reports **`matches init_zobrist: yes`** against the `no` observed at the parent, which is the precondition seen and not assumed; quality clean and re-run after the change: **0 zero keys, 851 of 851 distinct, 0 pair XORs equal to any key, 361675 of 361675 distinct pair XORs, minimum pairwise Hamming distance 14**. `tests/test_engine.cpp` asserts the 851 values equal that draw in that order and re-checks all four properties -- 988 assertions in the suite. **Baselines moved once, as designed:** depth 9 `121512 / 800769 / 62907` -> `121530 / 801481 / 72924`, depth 12 `639228 / 3430710 / 367858` -> `636677 / 3520847 / 494098`, `bench` `24880255` -> **26851183**, **best moves `c3d5` / `e2a6` / `d7c8q` unchanged at both depths**, which is what the strict accepts required. Perft verified by `bench_movegen`, `test_perft` green, Debug self-play 8 games at 4+0.04 with **0 assertions** (DEC-141), fast suite green in both builds, format clean. **The mining run the step was written around produced nothing, and that is DEC-156.** `ROUNDS=1500 ./fastchess.sh`, 3000 games at 8+0.08, **1 h 18 m 10 s, 0 time forfeits**, pre-registered before launch: **3** `Incomplete mating PV` lines over 2 distinct roots, **all from the old-keys reference and 0 from the candidate**. Not evidence the redraw fixed the class -- 3 lines all landing one side has probability 0.125 under an equal-rate null -- but evidence the class is far rarer than when the set was built, S147 having read 10 lines over 4 games in the same 3000. A campaign for six fresh cases was not going to converge. Elo, LLR and pairs were not read, as pre-registered: fixed rounds, no bounds, not a verdict. `adocs/data/S203_mine_cases.py` is the reader, and it found something nothing in the repository recorded: fastchess emits **three** mating-PV warning classes, `Incomplete mating PV`, `Too long mating PV` and `Mating PV does not end with checkmate`, of which only the first is case material -- the other two are wrong lines, not short ones. **All six cases came back by re-sweeping the node budget alone.** One rule stated before it was applied and applied to every row: the cheapest budget at which the case reports at least its floor of mate lines with all of them complete. A `nodes 300000 -> 1000000` (12 lines, 0 short), C `1000000 -> 1500000` (13, 0), D `1000000 -> 4000000` (6, 0); B (8, 0), E (23, 0) and F (4, 0) untouched. Floors re-derived at half the reported count for a re-swept row and left alone for a row that did not move: A 5 -> 6, D 1 -> 3, C 6 unchanged in value, B 7 and E 9 untouched -- **no live floor lowered** -- and F's stale 6 against 4 reported corrected to 2 while it stays `guard no`. `adocs/data/S203_case_sweep.sh` is the script DEC-142 wants beside a golden and re-derives budgets and floors both. **The grid is a knife edge, measured rather than implied:** C reports 13 mate lines at 1500000 nodes and **0 at both 1000000 and 2000000**; both the TSV header and the test say so, and the sweep is to be re-run after anything that moves the tree. **Two reproductions changed hands.** `D_mate_minus6_depth10` between 1200000 and 3000000 nodes publishes `mate -6` at ply 35 depth 11 with a 10-of-12-ply PV at a depth that also publishes a complete 12/12 -- DEC-122's designed-visible class -- and S202 carries it as a seconds-long reproduction where its only other route was a 3000-game match; it is recorded rather than cleared by the 4000000 the row carries. In the same move **S202's own F reproduction is retired**, having printed 6 mate lines and 3 short for two months and printing 4 and 0 under the redrawn keys; both readings are kept there, because the old one is quoted. Cost: the fast suite gains about 8 s, D's new budget being the price. Docs: `adocs/specs.md` carries the new baseline under INV-6 and the exposure now reads closed; `adocs/status.md`'s handover check carries both sets; `DEV_MANUAL.md`'s node signature gains 26851183 and its regeneration section now says the engine uses both tables and points at the sweep script; `MANUAL.md` checked, no UCI surface moves, no change; `README.md` untouched, human-owned. Closes `2026-09-04_test_review-F08`. Still open and deliberately untouched, outside `touches:`: `tools/corpus_dedupe.hpp`'s stale line-number citation to `init_zobrist`.

## Why this exists

S179 was to land the magics and the keys as two commits under one step. The
magics landed. The keys turned `test_mate_carry` red and the owner split the
step: DEC-154 is that decision and its reasoning is not repeated here.

What matters for whoever picks this up: **the red is a retired fixture, not a
defect.** `adocs/data/S170_cases.tsv` is six games mined from S147's 3000-game
run, and the class it guards is transposition-table eviction -- which entries
evict which is exactly what the Zobrist keys decide. Measured on 2026-09-08 over
four arbitrary seeds:

| seed | cases gone vacuous, of 6 | cases with a short mate line |
|---|---|---|
| 20260904 | 3 | 1 |
| 12345 | 3 | 0 |
| 999983 | 4 | 0 |
| 777777777 | 3 | 0 |

Every key set breaks it. There is no seed to pick that keeps the fixture, and
picking for a green suite would be fitting the seed to the test. So the fixture
is re-mined, which is what makes this step cost a match instead of a minute.

## What S179 already shipped, and what is left

Shipped and committed:

- `project_random_next` and `CHESSO_PROJECT_SEED` (20260904) in
  `src/bitboard.cpp`, splitmix64 written out from
  https://prng.di.unimi.it/splitmix64.c, public domain.
- `tools/magic_gen zobrist --seed N`, which draws 851 values in
  `init_zobrist`'s order, prints the quality report, and compares against a
  live `game_t`. It prints `matches init_zobrist: no` today. That line is this
  step's precondition and flipping it to `yes` is the change.
- `DEV_MANUAL.md` "Regenerate the magic numbers and the Zobrist keys", which
  already documents both modes and says the keys are not drawn from the tool
  yet.

Left: the four loops in `init_zobrist`, the guard test, the re-mining, and the
recorded baselines.

## The engine change

Replace `std::mt19937_64` and the distribution with one `uint64_t state =
CHESSO_PROJECT_SEED;` and `project_random_next(&state)` at each of the four
loops, keeping their order exactly. The standard does not fix what
`std::uniform_int_distribution` returns for a given engine state, so the keys,
the table indices and every node count this repository records are today a
property of the standard library as much as of this code; they matched between
glibc and Apple libc++ when the MacBook handover check ran, which is a
measurement and not a guarantee. That is `2026-09-04_test_review-F08` and
DEC-139.

The draw was already run on 2026-09-08 under seed 20260904 and its quality
report was clean: 0 zero keys, 851 of 851 distinct, 0 pair XORs equal to a key,
361675 of 361675 distinct pair XORs, minimum pairwise Hamming distance 14.
Re-run it rather than quoting these.

## What the keys move, measured before the split

Recorded here so the step starts from a number and not from a prediction. At
`a9bb023` with the key change applied and nothing else:

| | depth 9 | depth 12 | `bench` |
|---|---|---|---|
| before | 121512 / 800769 / 62907 | 639228 / 3430710 / 367858 | 24880255 |
| after | 121530 / 801481 / 72924 | 636677 / 3520847 / 494098 | 26851183 |

Best moves `c3d5` / `e2a6` / `d7c8q` at both depths, **unchanged**, which is
what the `accepts` requires; if a re-run moves one, that is a stop and the owner
decides, per the answer given on 2026-09-08. `test_perft` and `bench_movegen`'s
perft verification were green, and Debug self-play over 8 games at 4+0.04 raised
0 assertions.

## The mining run, pre-registered 2026-09-08 before it started

**Not a verdict and it prints no bounds.** `ROUNDS=1500 ./fastchess.sh` is the
fixed-rounds mode S198 added: no SPRT, 1500 rounds with `-repeat`, so **3000
games** at 8+0.08 on the UHO book with concurrency 12. The candidate is the
working tree -- the new keys -- and the reference is `HEAD`, which is the old
ones, so the same match gives the control S171 read: warning lines from the
candidate against warning lines from the reference, on the same 1500 openings.
A self-play of the new build would double the yield per hour and was rejected
for that reason: it cannot separate "the new keys made this worse" from "this is
the standing class".

**Expected duration 1 h 19 m**, from `.moltke.local.md`'s measured 2277 games an
hour and not from a guess. Under DEC-155's four-hour line, so it runs during the
day. Watcher ceiling 3 h, a little over 2x.

**What the run is read for**, stated before it starts so the reading is not
chosen afterwards:

1. The `Incomplete mating PV` lines from the **candidate**, with the `Position;`
   and `Moves;` line beside each. These are the case material.
2. The same count from the **reference**. S147 read 10 lines over 4 games and
   S171 read 8 from 1 search against the reference's 0 from 0. A candidate count
   in that neighbourhood is the standing class; a count far above it is a
   finding about the keys and stops the step.
3. Nothing else. Elo, LLR and the pair distribution are not read: the two builds
   differ, the run has no bounds, and a number quoted off it would be the DEC-020
   class of claim.

**The abort rule.** Stop the run and report if it exceeds the 3 h ceiling, if
the watcher reports the process gone, or if either engine forfeits on time --
`test_mate_carry`'s cases must come from games the engines actually played out.

**What it cannot promise.** S147's 3000 games yielded case material from **four**
games and `tests/test_mate_carry.cpp` requires six rows. One run may not be
enough, and if it is not, that is a second run and not a relaxed test -- the
`REQUIRE` on the row count is a guard and moving it to fit a thin harvest is
exactly what TESTS forbids.

## Re-mining the case set

`tests/test_mate_carry.cpp`'s header says what the cases are and why the shape
is a game replayed move by move through one process. The source is a run's
`Incomplete mating PV` warnings with their `Position;` and `Moves;` lines; S147's
was 3000 games and S171 re-took the census in 1 h 17 m 49 s on this machine.
`adocs/data/S170_replay.py` sweeps a case for its cheapest reproduction and takes
`--stride-override`; DEC-151 is why stride matters and why searching every ply is
a table no game produces.

The floors in `expected_mate_lines()` are measurements at the chosen cases and
are re-measured, never carried over. A case that reports no mate at all is
vacuous and is replaced, not deleted -- the test says so at its own failure site.

## Sources

- DEC-154, the split and the four-seed measurement.
- DEC-139, the owner folding F08 into S179; DEC-122, DEC-150 and DEC-151 for
  what a short mate line is and is not; S202 owns the class.
- https://www.chessprogramming.org/Zobrist_Hashing -- the linear-independence
  rule the quality report enumerates at subsets of two, three and four.
- https://prng.di.unimi.it/splitmix64.c -- the generator, already committed.
- `adocs/plan_done/S179_magic_numbers_project_seed.md` -- the stamp, which
  carries the figures above.
