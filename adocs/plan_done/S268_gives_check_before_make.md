id:         S268
goal:       a move a pruning rule has already discarded is discarded before make_move, by a gives-check test that reads the board without making the move, so the search stops making and unmaking moves it throws away
accepts:    (1) a predicate beside `src/bitboard.cpp` `is_check` that answers whether a move gives check without making it -- direct and discovered checks, the moving king's discovered check included -- with en passant, castling and promotion either handled or explicitly sent to the make-and-test fallback; (2) a test that the predicate agrees with `make_move` followed by `is_check` on every legal move of a corpus that contains each of those move kinds, and a mutant the test kills (`tools/mutation_check.py`); (3) used at the three sites the finding names: the quiet rules in `src/search.cpp` `negamax_at`, S091's capture rule there, and the futile captures in `src/search.cpp` `quiescence`; the PROBING build may keep its post-make record; (4) **node identity** (INV-6): `bench` and `tools/search_bench.py` node counts and best moves identical to the parent at two depths; (5) an interleaved timing on the workstation, noise floor read first (CLAUDE.md rules 4 and 5), with instruction and cycle counters (`perf stat -e instructions,cycles`) beside nps; kept only if faster; the audit's M1 figure is a pointer, not the claim; (6) the comment in `src/search.cpp` `negamax_at` that justifies the post-make test with another engine's figure is replaced by this step's measurement; (7) DEC-141's second tier: the Debug binary self-plays four rounds at 4+0.04 and its log is grepped for `Assertion`, and `tools/gate_extra.sh` runs, both named in the stamp
touches:    src/bitboard.cpp, src/bitboard.hpp, src/search.cpp, src/data_structures.hpp (one comment), tests/test_movegen.cpp, tests/test_search.cpp, tools/mutants/S268_gives_check.py; amended at implementation for the counter instrument accepts (5) names, `perf` not being installed on the workstation: tools/perf_counters.cpp, tools/CMakeLists.txt, DEV_MANUAL.md
excludes:   any change to which moves a rule selects, or to its thresholds; move ordering and `pick_next_move`; the dead pruning work after late move pruning fires, which is S269
decisions:  DEC-083, DEC-141, DEC-260, DEC-263
closes:     2026-10-08_performance-F01
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-08 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against e383987: bench 4081329 and bench 12 1860699 with every info line and bestmove identical; search_bench 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at depth 12, c3d5 / e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical. No SPRT owed (DEC-083). move_gives_check() reads the gives-check exemption off the board before the move, every move kind included, no fallback; the engine's node skips a move a rule selected without making it, at the quiet rules, S091's capture rule and quiescence's futile captures; the probed node keeps its post-make record. Agreement case over 2.1 M legal moves, every kind checking and not, direct and discovered; GC01 to GC05 and P05, C02, R01, M08, F01, F04 killed, 11 of 11. -11.4 % instructions and -7.1 % cycles on bench 16, -11.5 % and -6.7 % on 300 positions at depth 11 (perf_counters, A/A 0.00 % and -0.05 %); +7.16 %, 95 % CI +6.98 .. +7.33, 24 interleaved hyperfine pairs, 24/24 faster, A/A floor +/- 0.2 %; bench_movegen resolution 0.3 %; governor powersave. About +10 Elo at long time control as a conversion, not a verdict. KEPT. Perft unchanged. Suites 43/43 in build and build-tune, format clean (DEC-146 override), Debug 43/43, 0 Assertion. Counter instrument tools/perf_counters (Part 0, its own commit). Second tier (DEC-141), by the coordinator: Debug self-play 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`; `gate_extra` 5 stages green in 893 s (`.tuning/coord/S268/gate_extra/`).

## Why

At `bench`, 3313197 of the 5792360 moves `negamax_at` makes (57 %) are
unmade again without being searched; only 81133 of the moves a rule selected
survive by giving check. The audit's prototype (about 20 lines, quiet
non-castling moves only) was node-identical on three checks and measured
-7.6 to -9.0 % instructions and -5.0 to -7.2 % cycles on the M1. The post-make
order was chosen on another engine's Elo figure and never timed here
(CLAUDE.md rule 7). The report has the prototype and the commands that
re-derive every number: `adocs/audit/2026-10-08_performance.md`, F01.

## Lane

Agent work under DEC-260, written on either machine; it lands on the
workstation's timing. Behaviour-neutral, so no match (DEC-083).

## As built (2026-10-08, Opus subagent)

### Deviations, first

1. **The counter instrument is this project's, not `perf`.** `perf` is not
   installed on the workstation and installing it needs the owner, so accepts
   (5)'s `perf stat -e instructions,cycles` is read through
   `tools/perf_counters.cpp`: the same two hardware counters from
   `perf_event_open`, user space only (`kernel.perf_event_paranoid` is 2), over
   the command's process tree. Linux only, guarded out of the macOS build in
   `tools/CMakeLists.txt`, one paragraph in `DEV_MANUAL.md` beside the timing
   commands. `touches:` amended. Its own commit, first.
2. **The probed node keeps its exemption after `make_move`** (accepts (3)
   allowed it). The engine's node asks `move_gives_check()` before the move,
   under `if constexpr (!PROBING)`; `negamax_probed` still makes the move and
   reads `is_check()`, so `pruned_moves` is recorded exactly where it was. The
   post-make skip line is unchanged and compiled into both: in the engine's
   node every move that reaches it with a rule attached was found to give
   check, so it never fires there, and its `!is_check_move()` and
   `!capture_gives_check` keep their meaning -- P05 and C02 still apply and
   still kill, in both nodes. The two nodes decide alike because the predicate
   answers what `make_move` + `is_check` answer on every legal move (the
   test below), and INV-6 holds the tree.
3. **No move kind is sent to a make-and-test fallback.** En passant, castling
   and promotion are handled inside the one picture the predicate builds; the
   test holds every kind.
4. **Late move pruning's `prune_rule = PRUNE_LATE_MOVE` moved from after
   `make_move` to before it**, placed after `see_loses_material`, which reads
   `prune_rule` as the other rules left it. Nothing between the old and the new
   place read it on a quiet, so the order is the parent's.
5. **`src/data_structures.hpp`, one comment**: `prune_rule_t`'s note said every
   rule skips "after `make_move`". Now: after it in the probed node, before it
   in the engine's. `touches:` amended.
6. **`tests/test_search.cpp` was not needed** and is untouched: no guard case
   changes meaning, and the mutation run below re-reads the five whose lines
   this step moved under.
7. **The mutation fixture is a commit object on no branch** (`901de49`,
   S254's method): `e383987` plus this working tree through a temporary index
   and `git commit-tree`, no ref moved, worktree removed after.
   `git config submodule.tests/doctest.url` was set in the shared
   `.git/config` to the value it already held (`.moltke.local.md`'s recipe).
8. **`src/bitboard.cpp`'s inlining moved by one call** (S020 deviation 5's
   hazard, checked with S020's `-fopt-info-inline-missed` report): gcc's
   `inline-unit-growth` refusals 47 at the parent, 46 here; one more
   `generate_moves_body` -> `is_attacked_with_occupancy` call left out of line
   and `generate_dispatch`'s GEN_ALL inlining swapped colour. The accumulator
   hooks stay inlined in both (0 out-of-line calls, S253's fix holds). Perft is
   unchanged (below); the search timing prices the rest.

### The sweep, on `e383987`

`grep -n "is_check\|gives_check\|is_check_move" src/search.cpp src/bitboard.*`:
the audit's three sites are where it put them, one line down. In
`src/search.cpp` `negamax_at`: `make_move` of every candidate, the memoised
`is_check_move` lambda (S020) read by the exemption and the reduction guard,
S091's `capture_gives_check` (its own `is_check()` call, captures only), and
the exemption that unmade a non-checking move a rule had marked. In
`src/search.cpp` `quiescence`: the futile capture made, `is_check`, unmade.
Off the hot path: the PV and mate-walk helpers' `is_check(game)`, and
`src/bitboard.cpp`'s own reads in the result and notation code. No pre-make
predicate existed.

### The predicate

`src/bitboard.cpp` `move_gives_check(const board_t*, move_t)`, beside
`is_check`, declared in `src/bitboard.hpp`. It is `is_check()`'s scan asked
from the enemy king's square against our pieces and the occupancy as they
stand after the move: the mover lifted from `from` and placed on `to` as what
it becomes (a promotion attacks as its new piece), the en-passant victim taken
off its square, a castling rook moved from its corner (`castling_rook`, the
table `make_move` uses). Direct checks, discovered checks (the king's own move
and the en-passant victim's square included) and the castling rook's check all
fall out of that one picture. No king term (a legal move never leaves the
kings adjacent); a captured piece needs none. About 30 lines.

Used at the three sites: the quiet rules and S091's capture rule in
`src/search.cpp` `negamax_at`, one test for every move a rule selected
(`prune_rule != PRUNE_NONE`, late move pruning's mark included); and the
futile captures in `src/search.cpp` `quiescence`. **Memo**: a move that
survives the test is known to give check, so the engine's node seeds the
child's memo with 1 before `make_move`; the reduction guard and
`capture_gives_check` read that memo (one `child_gives_check` lambda now backs
both `is_check_move` and `capture_gives_check`), so a surviving move pays no
second scan, and a move with no rule attached pays the lazy post-make scan as
before. `make_move` refuses only on a full history stack, which no search node
reaches (`POSITION_MAX_PLIES`), so a move skipped before it is one it would
have made -- including quiescence's old "a refused capture is dropped
unfolded" branch, which could not fire.

### The test and the mutants

`tests/test_movegen.cpp` "move_gives_check agrees with make_move and is_check":
a tree walk comparing the predicate with the engine's own `make_move` +
`is_check` on every legal move at every node -- eight positions built for the
rare shapes and their colour mirrors to depth 2, the 21 published perft
positions (`perft.json`, `talkchess_perft.json`) to depth 3, and every
`all_test_fens()` position to depth 2. 2.1 M moves, 0.15 s in Release. The
built positions (en passant uncovering a rank and a diagonal, an en-passant
pawn checking directly, both castlings checking with the rook, a knight
underpromotion that checks where the queen does not, a capturing promotion
uncovering a rook, a king leaving its rook's file) were confirmed by
python-chess's `gives_check()` and checkers; the test asserts no single
move, only agreement. **Precondition**, from the engine's own checkers after
each move: every kind (quiet, capture, king, en passant, castling, promotion)
both gave check and did not; every kind but the king gave a direct check and
every kind but castling a discovered one; a knight promotion checked. No
golden: every floor is "occurred at least once". Seen this run (not asserted):
quiet 81720 checking / 1456608 not, capture 21808 / 131808, king 56 / 167754
(56 discovered), en passant 7 / 62 (2 direct, 5 discovered), castling 177 /
15970, promotion 13662 / 208954, knight promotions checking 432.

`tools/mutants/S268_gives_check.py`, new prefix GC: GC01 the en-passant victim
left on its square, GC02 the castling rook left in its corner, GC03 a
promotion attacking as the pawn, GC04 the mover still on `from`, GC05 the
engine's node skipping every selected move without asking. Observed red by
hand first (each predicate mutant fails the case on its own built position),
then `tools/mutation_check.py tools/mutants .ref-builds/mut --only GC01 GC02
GC03 GC04 GC05 P05 C02 R01 M08 F01 F04 --jobs 12` on fixture `901de49`,
baseline green 43 tests, bench 4081329: **11 of 11 killed**, wall 1395 s
(`.tuning/coord/S268/mutation/`).

| id | fast failed | bench | verdict |
|---|---|---|---|
| GC01_gives_check_ep_victim | 1/43, this case | moved | killed |
| GC02_gives_check_castling_rook | 1/43, this case | moved | killed |
| GC03_gives_check_promotion_as_pawn | 1/43, this case | **same** | killed |
| GC04_gives_check_from_occupied | 1/43, this case | moved | killed |
| GC05_premake_skip_without_asking | 4/43, the unprobed mate cases | moved | killed |
| P05_prune_gives_check | 4/43 | moved | killed |
| C02_capture_gives_check | 2/43 | moved | killed |
| R01_extra_reduction_gives_check | 2/43 | moved | killed |
| M08_lmr_checks | 4/43 | moved | killed |
| F01_qs_futility_no_raise | 3/43 | moved | killed |
| F04_qs_futility_gives_check | 3/43 | moved | killed |

GC03 leaves the bench still -- no rule selects a non-capturing promotion and
quiescence exempts promotions before it asks -- so only the new case sees it.
GC05 is seen by none of the probed guard cases, as deviation 2 predicts, and by
four unprobed searches.

### INV-6

Against a Release build of `e383987` (`.ref-builds/s268_base`, removed after),
streams stripped of `time` / `nps` / `hashfull` only and diffed whole
(`.tuning/coord/S268/inv6.sh`): `chesso bench` **4081329**, all 121 lines
identical, every info line and `bestmove`; `chesso bench 12` **1860699**, 105
lines identical; `tools/search_bench.py` depth 9 **32932 / 70095 / 25178**,
`c3d5` / `e2a6` / `d7c8q`, depth 12 **67792 / 280873 / 137893**, `c3d5` /
`d5e6` / `d7c8q`, identical. The 300-position workload reads 26512806 nodes on
both builds in every timed run. The generator source did not move; perft is
verified by `bench_movegen` in every run below. The binary built from the final
tree is byte-identical (`cmp`) to the one timed.

### The timing

Machine: governor `powersave` (recorded, not set, DEC-195); load average 0.54
before, 0.94 to 1.07 during (the run's own engine); the top processes by
lifetime CPU were firefox, the terminal, this session and the compositor, none
busy. `bench_movegen -r 40`: **resolution 0.3 % / 0.0 % / 0.8 %** (perft /
generation / captures), spread 2.7 / 2.5 / 4.6 %. Everything interleaved,
order swapped every pair; readings in `adocs/data/S268_timing.txt`.

Part 0's floor, one binary ten times: instructions repeat to 97 (`bench`) and
153 (`bench 16`) instructions in 8.2 G and 22.0 G; cycles sd 0.57 % and 0.33 %.

| workload | instrument | A/A (base vs a byte copy) | parent vs S268 |
|---|---|---|---|
| `bench 16` | instructions | +0.000 %, 12 pairs | **-11.385 %**, 16 pairs, every pair |
| `bench 16` | cycles | -0.05 %, CI -0.38 .. +0.27 | **-7.07 %**, CI -7.23 .. -6.91, t -90.9, 16/16 |
| `bench 16` | nps (engine's own) | 3927629 / 3919112 | **3932460 -> 4240360, +7.8 %** |
| 300 positions, depth 11 | instructions | +0.000 %, 8 pairs | **-11.549 %**, 12 pairs |
| 300 positions, depth 11 | cycles | -0.01 %, CI -0.18 .. +0.16 | **-6.66 %**, CI -6.93 .. -6.39, t -51.8, 12/12 |
| 300 positions, depth 11 | hyperfine wall | ratio 1.0009, CI 0.9987 .. 1.0030, t 0.88, 16 pairs | **ratio 0.9332, CI 0.9317 .. 0.9347, t -88.76, 24/24** |

**Speed-up +7.16 %, 95 % CI +6.98 .. +7.33 %** (hyperfine, per-pair sd
0.38 %, A/A floor about +/- 0.2 %). Over CLAUDE.md's 3 % line, and every
instrument agrees in sign and size. Against the audit's M1 pointer (-7.6 %
instructions, -5.0 % cycles on `bench 16`, quiet non-castling moves only) this
covers all three sites and every move kind. At DEC-083's published rates
+7.16 % is **about +10 Elo at long time control and +15 at short -- a
conversion, not a verdict.**

Perft, `bench_movegen -r 5` six times each, alternating: perft 878.8 ms parent
against 877.9 ms; `generate_moves` 764.6 against 769.8 (+0.7 %);
`generate_captures` 307.3 against 303.1 (-1.4 %) -- deviation 8's inlining
shift, both within a percent and opposite in sign, and inside what the search
timing already prices.

**Keep-or-revert: KEPT on +7.16 % (CI +6.98 .. +7.33), -11.4 % instructions,
-6.7 to -7.1 % cycles.**

Accepts (6): the comment in `src/search.cpp` `negamax_at` that cited another
engine's Elo for the post-make order now states this measurement.

### Suites

`cmake --build build -j12 && ctest --test-dir build -L fast --output-on-failure
&& cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast
--output-on-failure && ./clang-format.sh --check` under
`CLANG_FORMAT_MAJOR=22` (DEC-146): **43/43, 43/43, format clean**, exit 0
(`.tuning/coord/S268/gate.log`). `tools/plan_prose_check.py` `--citations`,
`--touches`, `--params`, one mode per call: each exit 0, 0 flagged. Debug fast
suite (`build-debug`, every `assert` live): **43/43 in 1279 s, 0
`Assertion`**; test_movegen 169 s of its 600 s limit, the new case 8.1 s of it. The second tier (Debug self-play, `gate_extra.sh`)
is the coordinator's.

`MANUAL.md` checked: no UCI surface, option or output moved -- no change.
`DEV_MANUAL.md`: Part 0's paragraph; no other command, flag or default moved.
`specs.md`: two sentences of the search row describe the post-make order --
proposed wording in the report, not edited here.

### Proposed `done:` stamp

2026-10-08 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against e383987: bench
4081329 and bench 12 1860699 with every info line and bestmove identical;
search_bench 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at
depth 12, c3d5 / e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical. No SPRT owed
(DEC-083). move_gives_check() reads the gives-check exemption off the board
before the move, every move kind included, no fallback; the engine's node
skips a move a rule selected without making it, at the quiet rules, S091's
capture rule and quiescence's futile captures; the probed node keeps its
post-make record. Agreement case over 2.1 M legal moves, every kind checking
and not, direct and discovered; GC01 to GC05 and P05, C02, R01, M08, F01, F04
killed, 11 of 11. -11.4 % instructions and -7.1 % cycles on bench 16, -11.5 %
and -6.7 % on 300 positions at depth 11 (perf_counters, A/A 0.00 % and
-0.05 %); +7.16 %, 95 % CI +6.98 .. +7.33, 24 interleaved hyperfine pairs, 24/24
faster, A/A floor +/- 0.2 %; bench_movegen resolution 0.3 %; governor
powersave. About +10 Elo at long time control as a conversion, not a verdict.
KEPT. Perft unchanged. Suites 43/43 in build and build-tune, format clean
(DEC-146 override), Debug 43/43, 0 Assertion. Counter instrument
tools/perf_counters (Part 0, its own commit). Second tier (DEC-141), by the
coordinator: Debug self-play 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`; `gate_extra` 5 stages green in 893 s (`.tuning/coord/S268/gate_extra/`).
