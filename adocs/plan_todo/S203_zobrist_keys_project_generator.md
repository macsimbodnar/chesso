id:         S203
goal:       `init_zobrist()` draws its 851 keys from the project generator under `CHESSO_PROJECT_SEED` instead of `std::uniform_int_distribution`, and the S170 case set is re-mined under the keys that result
accepts:    `src/bitboard.cpp` `init_zobrist` fills `piece_randoms`, `castling_randoms`, `side_randoms` and `ep_randoms` -- in that order, which is load-bearing -- from `project_random_next` seeded with `CHESSO_PROJECT_SEED`, and `#include <random>` leaves the file if nothing else there uses it; `./build/tools/magic_gen zobrist --seed 20260904` prints `matches init_zobrist: yes` where it prints `no` today, and its quality report is recorded; a fast test asserts the 851 keys equal that draw value for value in that order, and over them: none zero, all 851 distinct, no pair XOR equal to any key, no two pair XORs equal -- properties, not goldens (DEC-142); `bench_movegen` verifies its perft counts and `ctest -L slow -R test_perft` is green; **every best move from `tools/search_bench.py` at depths 9 and 12 unchanged** (`c3d5` / `e2a6` / `d7c8q`) and the node counts recorded before and after, the new pair written into `adocs/specs.md` under INV-6 and into `adocs/status.md`'s handover check in the same commit, with the new `bench` total in the message as `Bench: <n>` (DEC-140) and in `DEV_MANUAL.md`'s node-signature section; **`adocs/data/S170_cases.tsv` re-mined under the new keys** -- a fresh `./fastchess.sh --fast` class run whose `Incomplete mating PV` warnings are the source, the same six-or-fewer case shape, each row's `start`, `stride` and `go` swept for the cheapest reproduction as S170 did, and `expected_mate_lines()` in `tests/test_mate_carry.cpp` re-measured against it -- with `test_mate_carry` green in both builds and its red-before observed, not assumed; Debug self-play, 4 rounds at 4+0.04, `grep -c Assertion` = 0 (DEC-141); fast suite green in both builds
touches:    src/bitboard.cpp, tests/test_engine.cpp, tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv, adocs/specs.md, adocs/status.md, adocs/plan.md, DEV_MANUAL.md
excludes:   the magic numbers (S179, done); the generator itself, `project_random_next` and `CHESSO_PROJECT_SEED` (S179 shipped all three); the seed value, which is settled at 20260904; closing S202's class -- a short mate line found while re-mining is the class S202 owns and is recorded, not fixed here
decisions:  DEC-139, DEC-154, DEC-142, DEC-122, DEC-150, DEC-151
closes:     2026-09-04_test_review-F08
blocks:
paused_by:
author:
done:

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
