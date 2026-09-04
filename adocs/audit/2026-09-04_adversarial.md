# Audit 2026-09-04 adversarial

Commit audited: `36b9f36833f2dd01ca7d3b35cf3f9264e320a42b` (`achesso`, "Arm rating.sh's marker first, and drop the GNU-only tools it assumed (S177)", 2026-09-04), clean working tree. Re-run after the 2026-09-03 audit's four findings were planned and their steps landed.


Type: `adversarial`, whole engine. Scope: `src/`, `tests/`, `tools/`, the shell
scripts (`fastchess.sh`, `rating.sh`, `build_release.sh`, `books/fetch_book.sh`,
`clang-format.sh`), the `adocs/data/` scripts, and the claims in
`adocs/specs.md`, `MANUAL.md`, `DEV_MANUAL.md`, `TOOLCHAIN.md`, `adocs/plan.md`
and the recent decisions checked against the code that backs them. Three
classes were looked for: correctness defects, violations of the project's own
rules, and techniques left behind the documented state of the art. Verdicts on
every finding in the earlier reports under `adocs/audit/` are re-taken from each
finding's own reproduction, the four of 2026-09-03 first.

Method. Read cold, code first, documents second. Then run: `cmake --build build
-j8` and `ctest --test-dir build -L fast --output-on-failure` (27/27, 58.2 s,
Release); the same on `build-tune` (27/27, 59.3 s); the six doctest binaries
that carry the INV-2 and INV-4 assertions rebuilt in `build-debug` and run from
`build-debug/tests` (`test_chesso`, `test_openings`, `test_evaluation`,
`test_movegen`, `test_engine`, `test_search`: all pass, 33 million assertions
between them); `./clang-format.sh --check` (clean); `tools/search_bench.py` at
depths 9 and 12; the engine over UCI on the edge cases below; `tools/make_book`
and `tools/pgn_to_positions` on the 2026-09-03 fixtures and a full rebuild of
the shipped book; `./rating.sh --bracket`; `adocs/data/S175_book_conformance.py`
under `~/.venv/chess/bin/python` (python-chess 1.11.2). Machine at load 1.9 on
8 cores, mains power, no match run. Nothing in the repository was modified
except this report and one new, **unregistered** red-first regression test,
`tests/test_audit_go_infinite.cpp` (F01), which is not in `tests/CMakeLists.txt`
and cannot affect any build or gate until someone registers it.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-09-04_adversarial-F<nn>`. The example below writes that
prefix as `<report>`: a fenced example carrying this report's real stem cannot
be told apart from a real finding that a fence has swallowed, which is INV-14
(S049).

```
### <report>-F01  low  short title

Status: open

Evidence: file and line, or the command and its output.
Impact: what breaks, for whom, under what conditions.
Suggested resolution: what would close it. Not applied here.
```

Every finding ends in one of two places: a plan step whose `closes:` field
names it, or a decision entry stating why it will not be acted on, which moves
it to `accepted`. A finding moves to `closed` only after the audit is re-run
and no longer reports it. Fixing without re-running leaves it `planned`.

## Verdict

**No high finding, no medium finding. All four findings of the 2026-09-03
report are closed on re-measurement: the Polyglot key, the SAN parser, the two
scripts and `position fen` each behave as their steps claim, checked from the
reproduction and not from the stamp.** The fast suite is green in both builds,
the Debug assertions on INV-2 and INV-4 hold, and both node-count baselines
reproduce to the node. Three low findings, all in the UCI layer and none on a
search path: `go infinite` answers `bestmove` unasked on any root whose tree
collapses to `MAX_DEPTH` instantly (F01, red test written); a bad token in a
`position ... moves` list is skipped and the rest applied, the pre-S176
behaviour kept for the other half of the same command and silent in the
shipped binary (F02, documented in `MANUAL.md` as intended, so this one may
close by decision); and the root's move order after an aborted iteration rests
on a table entry surviving, a premise a comment states and nothing enforces
(F03, mechanism shown, occurrence not reproduced). **Class 2 (the project's own
rules) turned up nothing.** **Class 3 (Elo left behind the literature) turned
up nothing the plan does not already carry.** Of the earlier reports' 37
code-audit findings, 30 are closed on re-measurement here, 4 are accepted by
decision and 3 are planned in pending steps; the four plan-review reports are
dispositioned in the table.

## Findings

3 findings: 0 high, 0 medium, 3 low.

---

### 2026-09-04_adversarial-F01  low  `go infinite` prints `bestmove` without a `stop` on any root whose tree collapses -- insufficient material, a stalemated or checkmated root -- because the infinite search is the iterative-deepening loop to `MAX_DEPTH` and nothing holds it open

Status: open

**Evidence.** `src/chesso.cpp:1472` seeds every `go` with
`search_options.depth = MAX_DEPTH` (126, `src/data_structures.hpp:43`), the
`infinite` branch at `src/chesso.cpp:1542-1544` clears the time limit and does
nothing else, and `iterative_deepening_search()` runs
`for (int current_depth = 1; current_depth <= conf.depth; ++current_depth)`
(`src/chesso.cpp:822`) and returns when the loop ends. On a root whose every
child scores a draw without a search -- `is_insufficient_material()` at
`src/search.cpp:659` -- or that has no legal move at all, depth 126 is reached
in under a millisecond and `bestmove` follows:

```
$ (printf 'uci\nposition fen 4k3/8/8/8/8/8/8/4KN2 w - - 0 1\ngo infinite\n'; sleep 1; printf 'stop\nquit\n') | ./build/src/chesso
info score cp 0 time 0 depth 126 nodes 1134 nps 2021390 pv f1e3
bestmove f1e3                           <- printed at once, a second before `stop`
$ ... position fen 7k/5Q2/6K1/8/8/8/8/8 b - - 0 1  (stalemate: the engine scores it cp 0, bestmove 0000 at depth 1)
info score cp 0 time 0 depth 126 nodes 126 nps 913043 pv
bestmove 0000
$ ... position fen 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1  (checkmate: the engine scores it mate 0 at depth 1)
bestmove 0000
$ ... position fen 4k3/8/8/8/8/8/8/4K3 w - - 0 1
info score cp 0 time 0 depth 126 nodes 756 nps 1904282 pv e1d2
bestmove e1d2
```

The terminal nature of the two roots is the engine's own reading and not a
board kept in the head: `go depth 1` on them prints `info score mate 0 ... pv`
and `info score cp 0 ... pv`, both with `bestmove 0000`. A root with a real
tree does not collapse: `go infinite` on a mate-in-one root stayed below depth
50 for a second and answered only after `stop`, and
`tests/test_engine.cpp:1367` ("an infinite search answers only once stop
arrives") holds the start position, where depth 126 is unreachable, so the
suite cannot see this.

The durable reproduction is `tests/test_audit_go_infinite.cpp`, written by this
audit and **observed red at `36b9f36`**: the three roots above, `go infinite`,
200 ms, then the assertion that no `bestmove` has been printed, then `stop` as
the precondition that the search answers at all. Compiled with the project's
own flags against `build/src/libchesso_engine.a` and run from `build/tests`:

```
[doctest] assertions: 6 | 3 passed | 3 failed |
[doctest] Status: FAILURE!
  go infinite answered [bestmove f1e3] before stop for 4k3/8/8/8/8/8/8/4KN2 w - - 0 1
  go infinite answered [bestmove 0000] before stop for 7k/5Q2/6K1/8/8/8/8/8 b - - 0 1
  go infinite answered [bestmove 0000] before stop for 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1
```

**Impact.** The protocol's own words: "infinite -- search until the stop
command. Do not exit the search without being told so in this mode!" A GUI
analysing such a position receives a `bestmove` it did not ask for and then
sends `stop` to a search that no longer exists; whether it hangs on the
missing reply or drops the unsolicited one is the GUI's affair, and both are
protocol breaks on chesso's side. No measurement is touched: fastchess and
cutechess never send `go infinite`, and every root in a match has a tree.
`MANUAL.md:185` lists `infinite` among the tokens `go` understands with no
caveat.

**Suggested resolution.** When `conf.infinite` is set -- or, more generally,
when no limit at all bounds the search -- do not return from
`iterative_deepening_search()` when the depth loop ends: wait on
`stop_search_signal` (a sleep-poll, or the search repeated at `MAX_DEPTH`) and
print `bestmove` only then. Register `tests/test_audit_go_infinite.cpp` and
observe it green. If the choice is instead to keep the behaviour, record the
decision, say so in `MANUAL.md`'s `go` paragraph, and delete the test by that
decision (TESTS rule). INV-6 is discharged on node counts as the S176 batch
discharged it: nothing here touches a searched node.

---

### 2026-09-04_adversarial-F02  low  a `moves` token that does not parse or is not legal is skipped and the rest of the list is applied, so `position` ends on a board the GUI did not send -- the S176 rule for the FEN half of the same command, not applied to the moves half -- and the skip is silent in the shipped binary

Status: open

**Evidence.** `src/chesso.cpp:1378-1407`, the `moves` loop: a token
`algebraic_to_uci_move()` rejects is passed over, a move `try_move()` cannot
find is answered with `LOG_W << "Failed move ..."` (`:1403`), and the loop
continues with the next token. `LOG_W` is `if (false) std::clog` under
`NDEBUG` (`src/log.hpp:35`), so the release binary says nothing. Reproduced:

```
$ printf 'position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 zzzz a7a6 b5a4\nfen\nquit\n' | ./build/src/chesso
r1bqkbnr/1ppp1ppp/p1n5/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 4
```

`zzzz` is gone and `a7a6 b5a4` were applied over it; the same happens for a
syntactically valid move that is illegal on the board (`e1g1` on Black's turn
in the same line). `MANUAL.md:207-208` documents exactly this ("A move in the
`moves` list that does not parse or is not legal is skipped with a warning,
and the rest of the list is still applied"), with "a warning" meaning a debug
build, as `MANUAL.md:66` explains. Two paragraphs earlier the same manual,
and `adocs/specs.md:153-162`, state S176's rule for the FEN half of the
command: a FEN that does not load ends the command with the position
unchanged, "so the moves after a refused FEN are never applied to a board they
were not meant for".

**Impact.** Only a GUI or harness that sends a token chesso cannot play
triggers it, and no harness this project runs has. When it does, the engine
plays the rest of the game from a position one or more plies apart from the
GUI's and every later `bestmove` is judged against a board it did not compute
on -- a forfeit, not a wrong move, and one whose cause is invisible in the
shipped binary. Low, and recorded because the reasoning S176 wrote down
("applying the moves that follow to the old board would put the engine
somewhere the GUI did not send it") applies word for word to the other half of
the same command, and `MANUAL.md` documents the two halves as behaving
oppositely.

**Suggested resolution.** Either the S176 treatment -- stop at the first token
that does not play, report it as one `info string refused [position moves]
<token>, <why>` in every build, and add the line to `MANUAL.md` and to
`test_uci_surface`'s refusal templates (SURFACE rule) -- or a recorded
decision that the asymmetry is deliberate, which moves this to `accepted`.
Either way the golden case is one more line in `tests/test_engine.cpp`'s uci
layer suite beside the S176 cases. Not on a search path; INV-6 on node counts.

---

### 2026-09-04_adversarial-F03  low  the root's move order after an aborted iteration rests on the root's table entry surviving the iteration, which the replacement rule does not guarantee; the premise is written in a comment and enforced nowhere. Mechanism shown, occurrence not reproduced

Status: open

**Evidence.** `src/chesso.cpp:906-911` accepts an aborted iteration's move on
this argument: "the root replaces its move only when that move beats every
move searched before it at this depth, and the first move it searches is the
previous iteration's best". Nothing orders the root that way except the table.
`score_move()` puts one move first, the `tt_move` (`src/evaluation.cpp:1145`,
`ORDER_TT_MOVE`), and the root reads it from `tt_get_entry()` at
`src/search.cpp:663-667`; there is no root move list carrying the previous
iteration's result. The root's entry is written at the end of iteration N with
`depth = N` (`src/search.cpp:1099-1101`). `tt_store_entry()` replaces an entry
of the current generation whenever `depth >= entry->depth`
(`src/transposition_table.cpp:158`), and the generation is per `go`, not per
iteration (`src/chesso.cpp:771`), so any unreduced ply-1 node of iteration N+1
-- searched at depth N -- whose key indexes the root's slot overwrites the
root's entry. The next probe at the root misses on the key, `tt_move` is 0,
and the root is ordered by captures, killers and history instead. Under the
aspiration window `[s - 21, s + 21]` the first move to beat `s - 21` is then
published (`src/search.cpp:1041`) before the previous best has been searched at
this depth, and an abort at that moment plays it (`src/chesso.cpp:912-929`).

Frequency is an estimate and not a measurement: at `Hash 16` the table is
524288 entries, an iteration has on the order of 30 unreduced ply-1 nodes, so
about 6e-5 per iteration, and only an iteration the hard timer aborts can
publish on it. **Not reproduced**: no collision was constructed and no game was
run; the finding is the unenforced premise, read off the code.

**Impact.** A move that beat `previous score - AspirationDelta` at the new
depth is played in place of the move that scored `previous score` at the old
one, on the rare timed move where a slot collision and a hard-timer abort
coincide. The replacement is not necessarily worse. Low, and recorded as a
claimed-but-unenforced invariant rather than as a demonstrated wrong move.

**Suggested resolution.** Order the root independently of the table -- the
standard root move list, previous PV move first, then by the previous
iteration's scores -- or, at minimum, hand `search()` the previous iteration's
best move as the root's forced first move; either makes the comment true by
construction. A Debug assertion that the root's `tt_move` equals the last
completed iteration's `best_move` would make the premise visible where it is
relied on. Play-altering only where the collision occurs, so INV-6 is
discharged on node counts at `Hash 16` per the S108 lesson (a replacement
change has to be sampled at the hash the match plays) and an SPRT is owed only
if the counts move.

## Prior findings, re-assessed against this tree

Re-measured from each finding's own reproduction where one is stated; where the
finding was about a document or a plan step, from the current text. The status
recorded in the earlier report is given where it differs from what this tree
shows, because the earlier reports are evidence and are not edited.

### 2026-09-03_adversarial (4 findings)

| finding | state now | evidence |
|---|---|---|
| F01 `get_key()` wraps at the board edge | **closed** (S175; report says planned) | `src/openings.cpp:572-588` takes the two squares by file with `ep.file > 0` / `< 7` bounds; `test_audit_polyglot_key` registered, 10 of 10 green (`ctest` #15); `S175_book_conformance.py books/8moves_v3.pgn src/openings.bin` -> `games 34700 plies 555200 derived_entries 172232 book_entries 172232 distinct_positions 129613` / `missing 0 extra 0 weight_mismatch 0 duplicate_book_entries 0`, exit 0; `OwnBook` on `rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8` answers `bestmove d2f3` |
| F02 SAN parser fabricates a move | **closed** (S174; report says planned) | `src/bitboard.cpp:2429-2445, :2495-2499` return 0 on all three paths; `1. e4!? e5 2. Nf3 Nc6 *` builds 4 entries, `1. e4 e5 2. Qxf7 Nc6 *` is refused (`game 1 cut short: cannot parse 'Qxf7' at ply 2`, exit 1, no file at `--out`); `pgn_to_positions` prints `e2e4` from the start position for `e4!?` and exits 1 on `Qxf7`; `test_make_book_tools` green |
| F03 `rating.sh` and `build_release.sh` need GNU tools | **closed** (S177; report says planned) | `./rating.sh --bracket` -> `RATING-RUN-FAILED: ordo not on PATH`, exit 1, one marker; trap at `rating.sh:38-40` before the first command that can fail; cores at `:73` via `sysctl`, `timeout`/`gtimeout` at `:126-128`; `test_rating_script` green |
| F04 short FEN dropped, bad FEN resets | **closed** (S176; report says planned) | `position fen 8/8/8/4k3/8/4K3/8/8 w - -` loads as `... w - - 0 1`; the same with `moves e3d3` lands on `... b - - 1 1`; a FEN that does not load after `startpos moves e2e4` answers `info string refused [position fen] ..., does not load` and `fen` still prints the e2e4 board; `info string refused [position fen] ..., fewer than four fields` on the three-field form; `src/chesso.cpp:390-418, :1338-1376` |

### 2026-08-22_adversarial (7 findings)

| finding | state now | evidence |
|---|---|---|
| F01 FEN semantics corrupt the board | **closed** (S161; report says planned) | the three FENs load as `4k3/8/8/8/8/8/8/4K3 w - -`, `3k4/8/8/8/8/8/8/3K3R w - -`, `4k3/8/8/3P4/8/8/8/4K3 w - -`; `test_audit_fen_semantics` green |
| F02 default REF 7b4d9a4 | closed (S160) | `fastchess.sh` `REF="${REF:-HEAD}"`, banner with both dates, `test_fastchess_script` green |
| F03 50-move draw before mate | **closed** (S162; report says open) | `7k/6pp/8/8/8/7n/6P1/R6K w - - 99 60` at depth 1: `info score mate 1 ... pv a1a8`, `bestmove a1a8`; `src/search.cpp:647-655` |
| F04 NMP negative mate band | closed (S165) | `src/search.cpp:823` `beta > -MATE_MIN` |
| F05 stale timer race | closed (S163) | `src/chesso.cpp:49, :134-140, :480-494`; the stress harness is Linux-only and was not re-run |
| F06 Polyglot table | accepted (DEC-121; report says open) | cited at `src/openings.cpp:40-49` |
| F07 lazy-bound stand pat untested | closed (S164) | `tests/test_search.cpp:1251` `run(0, 100)` |

### 2026-08-21_adversarial (10 findings)

| finding | state now | evidence |
|---|---|---|
| F01 killer slot duplication | accepted (DEC-098, S149) | shift still unguarded with its comment, `src/search.cpp:1012-1018`; the test counts duplicated slots, `tests/test_search.cpp:474-477` |
| F02 stale parameter prose | closed (S150) | `test_plan_params` green |
| F03 single-control verdicts | planned (S151, Open entry 51) | `fastchess.sh` still `tc="8+0.08"` only |
| F04 no strength checkpoint | accepted (DEC-108; report says open) | S152 deferred, Open entry 52 |
| F05 zero-match steps serialize | closed (DEC-113, S153) | |
| F06 mate-in-three floor margin | closed (S154, S168) | `MATE_IN_THREE_FLOOR = 11` at `tests/test_engine.cpp:2060`, asserted `:2693` (the 2026-09-03 report cited `:1964` and `:2597`; the lines moved with S176's cases) |
| F07 one-motif mate set | closed (S155, S168) | |
| F08 mined set not asserted | closed (S156) | `test_mate_breadth` green, 17.1 s |
| F09 nElo bounds worded as Elo | closed (S157) | `fastchess.sh:25-38` |
| F10 book digest | closed (S158); superseded by S146 and S175 | the file it digested is deleted; the current digest reproduces byte for byte (below) |

### 2026-08-14_test_review (7 findings)

F01-F06 **closed** via S067: the illegal FENs `2k5/8/8/8/8/8/1q6/K1R5` and
`7k/5Q1K/8` survive only in comments recording their removal
(`tests/test_search.cpp:148, :375, :416`, `tests/test_engine.cpp:806`); the
build-dependent timeout is at `tests/CMakeLists.txt:10-14`; the KBvKB and KBvKN
cases are at `tests/test_search.cpp:2903-2905`. F07 accepted (DEC-058).

### 2026-08-13_adversarial (9 findings)

| finding | state now | evidence |
|---|---|---|
| F01 harness exits 0 on abort | closed (S035, S167) | trap armed at the top of `fastchess.sh` |
| F02 1 ms clock never answers | closed (S036) | `go wtime 1 btime 1000` -> `bestmove e2e4` |
| F03 per-iteration `nodes` | closed (S037) | |
| F04 tuner-model 2 cp tolerance | closed (S038, DEC-053) | |
| F05 lazy margin comment stale | **still present**, planned (S039) | `src/evaluation.hpp:293-295` still reads "king safety ships at zero weight"; `src/evaluation.cpp:790-793` is non-zero |
| F06 DEV_MANUAL tuner section | closed (S040) | |
| F07 free_mask untested | closed (S041) | `test_tuner_groups` |
| F08 ep square after every double push | **still present**, planned (S042) | `position startpos moves d2d4 d7d5 c2c4` then `fen` -> `rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2`; `src/bitboard.cpp:825-834` |
| F09 dead toolchain line | closed (S043) | no `CMAKE_TOOLCHAIN_FILE` in `CMakeLists.txt` |

Tally over the four code-audit reports before this one plus 2026-09-03: 37
findings, **30 closed, 4 accepted, 3 planned** (S039, S042, S151). The
2026-09-03 report summed its 33 as 27 closed, 3 accepted, 3 planned; recounted
from its own tables it is 26, 4 and 3 -- `2026-08-14_test_review-F07` is the
fourth accepted (DEC-058). Nothing turns on the slip.

### The plan-review reports

Not re-audited item by item; each finding was traced to its disposition and
the only change since the 2026-09-03 report is the one it flagged:
`2026-08-13_plan_review.2-F06` now has a trace -- `closes:
2026-08-13_plan_review.2-F06` at `adocs/plan_todo/S023_capture_history.md:11`
-- so it is planned in a reserve step (DEC-087) rather than untraced.
Everything else stands as that report recorded it: 2026-08-13_plan_review all
closed by its `.2` re-run; `.2`'s F07 and F09 closed, F08 overtaken by S138 and
S169, F01 to F05 carried in pending step text; 2026-08-16_plan_review F04-F07,
F09, F10 closed, F08 accepted (DEC-062), F02 closed by S138/S169, F01 and F03
carried by DEC-086; 2026-08-20_plan_review's 18 all have a `closes:` in a
`plan_done/` step.

## Checked and clean

Listed because a negative result is a result, and several of these are where
this audit expected to find something.

- **The suite, both builds, and the Debug invariants.** Release 27/27 in
  58.2 s; tune 27/27 in 59.3 s; `./clang-format.sh --check` clean. The Debug
  binaries pass with the INV-2 (`squares_match_bitboards`) and INV-4
  (`eval_accumulators_match`) assertions live when run from
  `build-debug/tests`: `test_movegen` 31457690 assertions, `test_engine` 868237,
  `test_search` 224549, `test_chesso` 2592, `test_evaluation` 276,
  `test_openings` 48, 0 failures. Run from the repository root instead they
  fail 15 assertions on `assets/test_jsons/...` paths, which are relative to
  the test working directory `ctest` sets (`tests/CMakeLists.txt:1-2`) -- a
  property of how the fixtures are located, not a defect.
- **The baselines.** `search_bench` 121512 / 800769 / 62907 at depth 9 and
  639228 / 3430710 / 367858 at depth 12, `c3d5` / `e2a6` / `d7c8q` at both --
  identical to every figure recorded since S108, so the S174 to S177 INV-6
  discharges are confirmed from outside the stamps.
- **The shipped book, from outside.** `src/openings.bin` is 2755712 bytes at
  sha256 `77f47f1b...db06b58`; `books/8moves_v3.pgn` matches its pin
  `5835239f...d17e` in `books/fetch_book.sh:62`; `make_book build
  books/8moves_v3.pgn` rebuilds it **byte-identical** (`cmp`) in 1.0 s with 0
  games cut short; the python-chess re-derivation agrees on every entry and
  weight (F01 table above). The S172 loading refusals, the weighted draw and
  the binary search were re-read and are as `specs.md:255-281` states.
- **The S174 to S177 changes, line by line.** `set_position()` saves and
  restores the whole `game_t` (`src/chesso.cpp:402-408`), so a `load_FEN()`
  that wrote part of a board before rejecting the rest leaves nothing behind;
  the four-to-six field reader stops at `moves` and defaults the clocks
  (`:1349-1370`); `algebraic_to_move()` range-checks the destination before
  `str_to_index()` and returns 0 on every failure path; the suffix strip runs
  in the same loop as `+`/`#`; `make_book build` refuses before the output is
  opened (`tools/make_book.cpp:401-409`) and `--allow-cut-short` is the only
  way past; `rating.sh` arms `fail()` and the trap before `all_cores`, resolves
  `timeout`/`gtimeout` once and writes every `command -v` as an `if`;
  `build_release.sh` counts jobs through `sysctl || nproc`.
- **The search core, re-read.** Draw tests before the probe, stride-two
  repetition walk bounded by `min(halfmove_clock, history.size)`, checkmate
  ahead of the hundredth halfmove; no table cutoff at a PV node;
  `tt_entry_answers()` de-normalises before comparing; reverse futility and
  null move both guard both mate-band edges and the null search keeps one real
  ply; the PVS/LMR re-search chain checks `aborted` before consuming a score
  and every store in both searches sits behind an abort check; the root
  fail-high replaces the PV row with the cutoff move; quiescence's DEC-102
  store degradation re-derived on every path; the mate-line completion is
  reporting-only and all-or-nothing, and every move it takes from the table or
  the stored line is checked against the generator; `pv_table` bounds hold
  (`pv_length[p] <= 127 - p`, so the `memcpy` at `src/search.cpp:1045` stays
  inside the row); the two `static_assert`s hold the depth and the mate band.
  `negamax` at depth 0 counts a node and probes once before quiescence counts
  and probes again -- cosmetic, self-consistent, and INV-6 compares like with
  like.
- **Time management and the UCI layer.** Hard limit clamped to `remaining -
  MOVE_OVERHEAD_MS` and floored (S036), soft clamped to hard, the scale never
  raises the hard limit, `go movetime` never scaled; timer disarm and session
  bump under one mutex; every mutation of `game` or the table joins the search
  first; `OwnBook` and `Best Book Move` are main-thread state read only in
  `command_go`. A checkmated or stalemated root answers `mate 0` / `cp 0` and
  `bestmove 0000` at any fixed depth (the `go infinite` case is F01). `position
  fen` with seven or more tokens ignores the trailing one in the outer loop.
  The table generation is a `uint8_t` that skips 0 on wrap, so an entry
  written 255 searches earlier regains depth preference for one search --
  negligible, and S119's territory.
- **Class 2, the project's own rules: nothing found.** No assertion line was
  removed from `tests/` since `1d8cbac` (602 insertions, 0 deletions across
  seven files), and each new case establishes its precondition first --
  S174's control fixture, S176's `REQUIRE_NE` before the refusal, S177's
  sandbox that fails all five properties on the old scripts. No source, table
  or training data from another engine beyond DEC-121's ruled constants; the
  S175 fix took its expected keys from python-chess and the S174 fixtures name
  the token, both tool-derived (CHESS rule). INV-6 was discharged on identical
  counts for all four steps and re-confirmed above. The `bb_tables.hpp` magic
  numbers stay the owner's open question in `status.md:127-133` and are not a
  finding.
- **Class 3, techniques behind the literature: nothing outside the plan.**
  Re-checked against the code and named to their steps: null move pruning
  without a `static_eval >= beta` gate and with a fixed `3 + depth/6` from
  depth 5 (`src/search.cpp:814-824`) -- S114; late move reduction by depth and
  move number only, from move 4 at depth 3 (`:960-966`) -- S098; no late move,
  futility, history or quiet SEE pruning in the move loop -- S109; no SEE
  pruning of captures in the main search -- S091; no internal iterative
  reduction -- S095; no singular extension or multicut -- S097; no correction
  history -- S099; quiescence searches no promotions and has no per-move
  futility -- S131, S112, S022; losing captures ordered ahead of every quiet
  -- S025, reserve by measurement (DEC-004); the direct-mapped 24-byte table
  with per-`go` ageing -- S119; the evaluation clamp, linear mobility and
  linear king safety -- S039, S121, S122; the unconditional en-passant key
  (F08 above) -- S042; `improving_at()` still has no consumer -- S109. Absent
  and in no step, unchanged from the 2026-09-03 reading: mate-distance pruning
  and a per-node reset of the killers two plies down, both small in the
  published record; the root move list is F03 above. Any Elo attached to any
  of these is a hypothesis for an SPRT and never a conclusion (DEC-019).
- **Documents against the code.** `MANUAL.md`'s five option lines, three tune
  refusal templates and two `position fen` refusal lines are held by
  `test_uci_surface` and trace to `src/chesso.cpp`; the book section's digest,
  size and entry count match the file; `DEV_MANUAL.md:569` and `AGENTS.md:130`
  carry the identical completion gate; `TOOLCHAIN.md:281-294` has the S177
  coreutils section; `.moltke.local.md` lists the tools. `adocs/plan.md`'s
  Open list names exactly the 53 files in `adocs/plan_todo/` and
  `adocs/plan_current/` is empty, as `status.md:10` says.
  `tools/plan_prose_check.py --citations` flags 59 line citations over 53 step
  files and is ungated by design (`DEV_MANUAL.md:825`); not a finding.
- **`fastchess.sh` and `books/fetch_book.sh`** re-read against S160 and S167:
  default reference HEAD with both dates, the A/A guard on state, the trap not
  trusting `$?` on bash 3.2, every git call anchored to the script, the
  candidate snapshotted before game one; the PGN pin verified in place.
