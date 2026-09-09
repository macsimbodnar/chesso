id:         S193
goal:       the fast suite's vacuous assertions are made falsifiable, its two overclaiming titles honest, the fifty-move boundary pinned, `test_perft` Release-safe, and the temp-file, date and case-order hazards removed
accepts:    a fifty-move boundary pair -- a root at clock 99 whose quiet replies land on clock 100 scores exactly 0 at depth 1 and the same root at clock 98 does not -- observed red under mutant M19 of `adocs/data/2026-09-04_test_review/mutants.py`, then green; the stop flag cleared through a completed `go depth 1` before `tests/test_engine.cpp`'s "a search with no limit is still bounded", and the case shown to spend at least half its 200 ms budget (amended 2026-09-09, DEC-163); the `LOG_I`-string assertion in "setoption carries a value containing spaces" replaced by an observable or removed; a `count > 0` precondition in `tests/test_movegen.cpp`'s pinned-piece case; `tests/test_perft.cpp` refuses a missing or unparseable asset in Release, every column fails the run, and the dead `RUN_THREADS` block is removed; the titles of "the node budget is honoured exactly" and "an infinite search answers only once stop arrives" say what their bodies assert; `tests/test_helpers.hpp`'s `legal_moves()` comment and `tests/test_chesso.cpp`'s `test_generate_legal_moves` say what is true -- legality is pinned by the perft and JSON counts -- or assert it; the three fixed temp-file names replaced by `mktemp`; `tests/test_fastchess_script.sh`'s banner date taken from `git show` rather than `date +%F`; `tests/test_chesso.cpp` and `tests/test_openings.cpp` initialise the tables explicitly instead of through case order; `tests/test_audit_go_infinite.cpp` registered with its 200 ms sleep replaced by a condition that cannot pass on a slow machine **if** `2026-09-04_adversarial-F01`'s engine fix has landed, otherwise left unregistered and this stamp says so; R13 to R17 and `tests/test_corpus_hash.cpp`'s three fixture names taken as well (DEC-163); each repair observed red under its own mutation before green where a mutation exists; fast suite green in both builds
touches:    tests/, tests/CMakeLists.txt
excludes:   any engine change; the `go infinite` engine fix itself (`2026-09-04_adversarial-F01`)
decisions:  DEC-139, DEC-163
closes:     2026-09-04_test_review-F04, 2026-09-04_test_review-F05, 2026-09-04_test_review-F09
blocks:
paused_by:
author:     Claude Opus 5 (coordinator)
done:      2026-09-09. **One injected bug that survived the whole fast suite is
            dead, seventeen assertions that could not fail can now, and every
            repair that admits a mutation was observed red under one before it
            was called done.** No `src/` change: `git diff -- src/` is empty at
            every commit of this step, so no `Bench:` line and no Debug
            self-play tier are owed (DEC-140, DEC-141), and INV-6 is identical
            by construction rather than by comparison -- `tools/search_bench.py`
            reads 121530 / 801481 / 72924 at depth 9 and 636677 / 3520847 /
            494098 at depth 12, `c3d5` / `e2a6` / `d7c8q` on both, and the
            binary is built from the same sources it was before. The three
            src/ mutants below were each reverted and the tree confirmed clean
            before anything else ran.
            **R1, the finding with a measured survivor.** The fifty-move case
            that existed searched a root already at clock 100, which the root
            exemption makes unreachable: every node below it is at 101 or more,
            so mutant M19 -- `>= 100` becomes `>= 101` -- passed all 27 binaries
            (`adocs/data/2026-09-04_test_review/results.tsv`). The new pair puts
            the root one halfmove lower. Tool record re-taken on this machine
            before the case was written, python-chess 1.11.2: both FENs
            `Status.VALID`, 21 legal replies from clock 99, **0 captures, 0 pawn
            moves, every child at halfmove clock 100, 21 of 21
            `is_fifty_moves()`, 0 checkmate, 0 insufficient material**; from
            clock 98 the children are at 99 and 0 of 21 are `is_fifty_moves()`.
            The engine through `SimpleEngine` at `go depth 1`: clock 99 **cp 0**,
            22 nodes, pv `d1d8`; clock 98 **cp +929**, 51 nodes, pv `d1d6` --
            reproducing the guide's 2026-09-05 figures exactly. Red under M19 at
            `REQUIRE_EQ(search_fen(at_99, 1).score, 0)` reading **929 against
            0**, and the old depth-4 case **stayed green in the same run**,
            which is the vacuity observed rather than argued; reverted, green.
            **The guide's precondition for R1 was wrong and is not what
            shipped.** It asked for `REQUIRE_FALSE(is_check(&game))` on every
            child; several of the queen's 21 moves give check, so that assertion
            is red on the position the guide itself chose. What the fifty-move
            block actually excepts is mate, and the case spells it the way
            `negamax` spells it -- in check with no reply -- plus an
            insufficient-material check per child, so both returns that outrank
            the draw are ruled out and neither is ruled out by a claim about the
            position.
            **R5, the Release-safe perft.** `load_json`'s two `assert`s became
            `std::cerr` plus `std::exit(2)`, and one `column_matches` /
            `columns_match` pair now drives both the colours and the pass flag.
            Exit codes observed, before and after, on scratch trees:
            **missing asset 0 -> 2** (and the old run printed nothing at all),
            **unparseable asset 0 -> 2**, **a wrong `captures` value at depth 2
            0 -> 1**. Honest green afterwards: `ctest -L slow` passes, 53.08 s.
            The dead `#ifdef RUN_THREADS` branches went with it -- they named
            `g_board`, `g_globals` and a `make_move` signature that has not
            existed for years and could not have compiled -- along with
            `spin_lock`, `tt_spin_lock`, the `// #define RUN_THREADS` line and
            the `<atomic>`, `<future>` and `<thread>` includes.
            **R2, the case that measured a millisecond.** `position startpos`
            runs `stop_and_join_search()`, which *sets* `stop_search_signal`, so
            the loop broke after depth 1 and `elapsed < 30000` held for the
            wrong reason. Red first: `REQUIRE(elapsed >= 100)` added alone reads
            **0 ms**. With a completed `go depth 1` before it, five runs read
            **239, 235, 238, 232 and 239 ms** against a 200 ms budget, so the
            floor stays at half the budget. **The accepts' `ucinewgame` cannot
            do this and was amended before the step started, DEC-163**: only
            `command_go` and `command_test` call `begin_search_session()`, no
            header declares it, and `command_ucinewgame` sets the flag again
            through the same `stop_and_join_search()`.
            **R3.** `REQUIRE(!capture.contains("Found position in the opening
            book"))` asserted the absence of a `LOG_I` string -- `if (false)` in
            Release and on `std::clog` in Debug, while the capture reads stdout.
            Replaced by `REQUIRE(capture.contains("info score "))` after a
            `uci_wait_for_search()` the case also lacked. Mutant: the embedded
            book loaded after the file load fails, so startpos answers from the
            book and `command_go` returns `bestmove` with no info line -- red;
            reverted, green.
            **R4.** `count > 0`, `position_is_reachable` and a count of the
            king's own moves before a loop that asserts an absence. Shown to
            bite on a tool-checked mated position (`4k3/8/8/8/8/2rrr3/8/3K4 w -
            - 0 1`, python-chess `Status.VALID`, 0 legal moves): `REQUIRE(count
            > 0)` red at `0 > 0`, restored green.
            **R6.** "the node budget is honoured exactly" -> "the node budget is
            never exceeded"; "an infinite search answers only once stop arrives"
            -> "a stopped infinite search answers with a legal move". The
            citation checker caught the rename inside this step's own file and
            both rows were re-cited to the new titles -- which is
            `test_plan_citation_freshness` doing exactly what S187 built it for.
            **R7 takes the asserting form (DEC-163, question 3).**
            `legal_moves()` calls `position_is_reachable` after every
            `make_move`, so a generator emitting a move that leaves its own king
            attacked fails at the helper rather than being absorbed at its call
            sites. Mutant: pins ignored in `generate_moves_body` -- red, naming
            the move (`e4d6` in `4r3/8/3N4/8/8/8/8/4K3 b - - 1 1`), and **the
            pre-existing `make_move` assertion beside it stayed green**, which is
            the vacuity. Trap T7's timing is below.
            **R8.** `test_generate_legal_moves` filtered the generator's output
            through `is_move_legal`, which is membership in that same output --
            a no-op -- behind an `assert` the Release build drops. It returns
            `generate_moves` raw with a `REQUIRE_LT(count, MAX_MOVES)`; the JSON
            counts and per-move FENs are what pin legality (INV-1), and M22 and
            M27 die on them.
            **R9.** `unique_fixture_path()` in the new `tests/test_temp_file.hpp`
            wraps `mkstemp(3)`; five fixed names are gone -- the two tuner ones,
            the two in `tests/test_openings.cpp` and, beyond the review's own
            list, the three in `tests/test_corpus_hash.cpp`. Memoised per binary
            because several cases write a path and read it back. The hazard is
            removed by construction and no deterministic red exists for it.
            **R10.** The candidate-line date comes from `git show -s
            --date=short --format=%cd HEAD`, the form `fastchess.sh`'s
            `commit_date()` uses, instead of `date +%F`, so a run straddling
            midnight no longer fails. The reference line's forced `2020-01-02`
            still catches a banner printing today's date.
            **R11.** `chesso_fixture_t` and `openings_fixture_t` replace two
            init-only cases that ran first only by doctest's file order. Red
            first, at HEAD: `./test_chesso -tc="Basic test"` fails in Release
            with **16 against 20** moves for the start position, and
            `./test_openings -tc="Test get moves"` aborts in Debug on
            `game_tables()`' assert. After: both pass, and `--order-by=name` is
            green on both binaries. `./test_openings -tc="Test key generation"`
            was green on both sides -- `get_key()` reads no attack table -- so it
            is not the probe the guide expected and "Test get moves" is.
            **R12 stays unregistered.** `adocs/audit/2026-09-04_adversarial.md`
            reads `Status: open` for F01 at `fbffd36`, and the accepts' own
            condition leaves the case out while that holds. `tests/CMakeLists.txt`
            is unchanged and the ctest count stays 31.
            **R13 to R17 were taken (DEC-163, question 2).** R13: "an
            irreversible move clears the window" searched a history of one entry
            over which the `back = 2` loop never runs; it establishes the
            knight-shuffle repetition first and then moves only the clock. Mutant
            -- the limit ignores the clock -- red, reverted green. R14:
            `depth >= TT_DEPTH_QS` is `>= -1`, the lowest depth any writer can
            produce; the bands are disjoint now, `== TT_DEPTH_QS || >= 1`.
            R15: `CHECK(expected_option_lines.size() == 5)` compared a literal
            with itself; it reads the binary instead, and **5 is named as a
            golden with its re-derivation command** (DEC-142) --
            `printf 'uci\nquit\n' | ./build/src/chesso | grep -c '^option name'`
            prints 5. R16: `correction <= LAZY_EVAL_MARGIN` is the `std::clamp`
            in `evaluate_expensive` restated; dropped for a reported
            measurement, and the number says why -- **the widest correction over
            2696 positions is 184, which is LAZY_EVAL_MARGIN exactly**, so the
            assertion was reading a saturated value. R17: `make_move` refuses
            nothing, so a fabricated book move passed; every book move is now
            matched against `generate_moves`' output on from, to and promotion.
            Mutant -- the from-rank flipped in `get_book_moves_for_key` -- red at
            `matches 0`, reverted green.
            **Gate, both builds, with `CLANG_FORMAT_MAJOR=22` (DEC-146):**
            31 of 31 fast green in Release and in the tune build,
            `./clang-format.sh --check` exits 0, `ctest -L slow` green.
            **DOCS:** `DEV_MANUAL.md` gains the `test_perft` exit-code paragraph,
            the fixture sentence for `-tc` and `--order-by=name`, and the
            `git show` clause on the fastchess date; its fast-suite figure moves
            from "about 60 s" to "about 80 s", measured 79.1 to 79.9 s.
            `adocs/specs.md` pins the boundary beside S162's sentence.
            `MANUAL.md`: checked, no change -- the UCI surface is untouched and
            `test_uci_surface` was not refreshed, R15 having changed only where
            the count is read from.
            **Trap T7, the Debug cost of R7, measured after the change.**
            Two `ctest --test-dir build-debug` runs read **test_movegen 153.32 s
            then 152.57 s, test_search 57.14 s then 56.55 s**, against the 600 s
            ceiling `tests/CMakeLists.txt` gives both. Under a quarter of it, so
            the fallback wording DEC-163 held in reserve was not needed and the
            asserting form ships.
            **What this step did not do.** No `src/` file changed, so nothing
            here was decided by SPRT and nothing was measured that a verdict
            could contradict. It bought no Elo and claims none: what it bought is
            that seventeen green assertions now mean what their titles say, and
            that one class of injected bug -- the fifty-move boundary -- stopped
            being invisible to the whole suite.
            Closes `2026-09-04_test_review-F04`, `-F05` and `-F09`.

## Why this exists

`2026-09-04_test_review-F04`, F05 and F09. One injected bug survived the whole
fast suite: the fifty-move draw claimed one halfmove late, because the only
direct case searches a root already at clock 100 and the root is exempt from
the draw test. Eleven assertions cannot fail for a reason unrelated to their
title -- a stop flag set by `position` before a timed search, a `LOG_I` string
that is `if (false)` in Release, a history of one entry, a literal compared
with itself, a clamp asserted after the clamp -- and `test_perft`'s asset check
is an `assert` compiled out of Release, so a missing asset exits 0. None is an
engine defect; together they are a dozen places where the green count
overstates what the suite holds. F09's items are the mechanical hazards the
same review listed: fixed temp-file names that collide under a parallel
`ctest`, a banner date that fails across midnight, two files whose later cases
depend on doctest's file order.

## Cost

Machine-free, hours.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

A test-repair step over `tests/` and `tests/CMakeLists.txt` only. The 2026-09-04 test review injected 33 bugs into the engine and the fast suite caught 31; the one that survived every case (M19, the fifty-move draw claimed one halfmove late) is the gap this step closes first. The rest are places where a green case proves nothing: an assertion whose outcome is fixed before it runs, a title that promises more than its body checks, a perft guard that is an `assert` and so compiles out of the Release build the gate runs, fixed temp-file names, a date read from the wall clock, and two binaries whose cases only work because doctest runs them in file order. **No engine behaviour changes and `src/` is not touched**: no Bench line, no SPRT, no Debug self-play tier, and INV-6 is identical by construction. If any item turns out to need a `src/` change, stop and file it -- `excludes:` says so, and BUGS says a real defect goes first.

### 2. The technique as published

**The fifty-move boundary.** The wiki's Fifty-move Rule page (https://www.chessprogramming.org/Fifty-move_Rule): "If the halfmove clock becomes greater or equal than 100, and the side to move has at least one legal move, a draw score should be assigned to that node", and "If the last move of such series delivers a checkmate, this takes precedence over the 50 move rule." FIDE Laws (https://handbook.fide.com/chapter/E012023): 9.3, the claim stands when "the last 50 moves by each player have been completed without the movement of any pawn and without any capture"; 5.1.1, "The game is won by the player who has checkmated his/her opponent's king"; 9.6.2, for the 75-move rule, "If the last move resulted in checkmate, that shall take precedence." So the boundary is `>= 100`, the mate exception is the rule S162 implemented, and a test that pins the boundary needs a node whose clock is *exactly* 100 -- one above passes M19, one below tests nothing.

**Non-vacuous assertions.** doctest (https://github.com/doctest/doctest/blob/master/doc/markdown/assertions.md): `REQUIRE` "will immediately quit the test case if the assert fails and will mark the test case as failed"; `CHECK` "will mark the test case as failed if the assert fails but will continue with the test case"; the `_MESSAGE` forms attach a message "relevant only to that assert". Nothing in doctest fails a case that asserts nothing or asserts a tautology, so the discipline is this repository's own: **establish the precondition, then assert the property** (the S162 case's "established rather than assumed", and DEC-141's "the test first shows the guard's condition holds"). doctest's command line (https://github.com/doctest/doctest/blob/master/doc/markdown/commandline.md): `--order-by=<file|suite|name|rand|none>`, "The default is `file`", and `-tc=<filters>` runs the matching cases alone -- which is how a case-order dependence is demonstrated and how one case is run red-first.

**Red-first with a mutant.** The review's driver `adocs/data/2026-09-04_test_review/mutants.py` holds each mutant as a `(old, new)` text pair that must match exactly once; `tools/mutation_check.py` is S196 and does not exist at HEAD. Apply a mutant by hand (section 6), never through `run.py`: that script assumes a worktree at `adocs/data/mut` and appends to `results.tsv`, which is append-only evidence.

**Temp files.** POSIX `mkstemp(3)` creates a uniquely named file from a template ending in `XXXXXX`; the four shell tests in `tests/` already use `mktemp -d "${TMPDIR:-/tmp}/chesso-....XXXXXX"` and are the pattern.

### 3. What chesso has today, and where the change plugs in

The fifty-move code is one block in `src/search.cpp` `negamax`, inside `if (ply > 0)`, after the repetition test and before the insufficient-material test: `if (game->board.halfmove_clock >= 100)` generates the replies, and `if (!is_mated) { return DRAW_SCORE; }`. It sits **before** the `if (depth < 1) { return quiescence(...) }` handoff, which is why a depth-1 search reaches it at ply 1 -- confirmed with the built engine on 2026-09-05 (section 6). `DRAW_SCORE` is 0. The root is exempt, so a root at clock 100 (the existing case) never reaches it, and every node below the root has clock 101 or more -- exactly the shape M19 passes.

Worklist, one row per repair. Sites are `file` plus `TEST_CASE` title or symbol. "Red-first" is what the implementer must see fail before the repair lands; DEC-142 golden risk is stated per row and is none for all of them.

| id | site | what is vacuous or hazardous today | repair | red-first evidence | golden |
|---|---|---|---|---|---|
| R1 | `tests/test_search.cpp` new case beside "the fifty move rule is a draw" | `search_fen("4k3/8/8/8/8/8/8/3QK3 w - - 100 200", 4)` then `REQUIRE_EQ(result.score, 0)`: root at 100 is exempt, deeper nodes are at 101+, so `>= 101` draws them all the same | the boundary pair of section 6, root at clock 99 scores exactly 0 at depth 1, the same root at clock 98 does not | M19 applied: the clock-99 probe scores the clock-98 number (929 today) and `REQUIRE_EQ(..., 0)` is red; reverted: green | none |
| R2 | `tests/test_engine.cpp` "a search with no limit is still bounded" | `REQUIRE(elapsed < 30000)`: `position startpos` runs `stop_and_join_search()` which sets `stop_search_signal`; the loop breaks after depth 1, so 200 ms is never spent | clear the flag the only way a test can (`go depth 1` + `uci_wait_for_search()`, see trap T1), then add `REQUIRE(elapsed >= 100)` beside the existing bound | add the new assertion first, run: elapsed is about a millisecond, red; add the clearing lines: green | none |
| R3 | `tests/test_engine.cpp` "setoption carries a value containing spaces" | `REQUIRE(!capture.contains("Found position in the opening book"))`: the string is `LOG_I`, which is `if (false) std::clog` in Release, and the capture reads stdout | after `go depth 1`, `uci_wait_for_search()`, then `REQUIRE(capture.contains("info score "))`: `command_go`'s book branch prints `bestmove` alone (its info line is commented out), so an `info score` line proves the search ran bookless | mutant: in `src/chesso.cpp` `try_load_opening_book`, after the file load fails add `opening_book_loaded = load_book_embedded(&opening_book);` -- startpos answers from the book, no `info score`, red; `git checkout -- src/chesso.cpp`, green | none |
| R4 | `tests/test_movegen.cpp` "a pinned piece cannot leave the ray" | `for (i < count) REQUIRE_NE(MOVE_PIECE(moves[i]), W_KNIGHT)` with no `count > 0`: passes on an empty list | `REQUIRE(position_is_reachable(&game))`, `REQUIRE(count > 0)`, and count the king's moves as `> 0` so the list is shown to be the king's evasions and not nothing | none exists; the precondition is the guard -- show it bites once by loading a FEN with no white pieces but the king boxed in (tool-check it) and seeing `count > 0` red, then restore | none |
| R5 | `tests/test_perft.cpp` `load_json`, `main`, `RUN_THREADS` | `assert(file)` / `assert(false)` compile out of Release: a missing or unparseable asset iterates zero cases and exits 0; only `expected_stats.nodes` sets `passed = false`; the `#ifdef RUN_THREADS` block names `g_board`, `g_globals` and an old `make_move` signature and cannot compile | `load_json` prints the path to `std::cerr` and `std::exit(2)` on a stream that does not open or a `parse_error`; one `columns_match(expected, real)` over `nodes`, `captures`, `en_passants`, `castles`, `promotions` used by both `print_stats`'s colours and the `passed` flag; delete both `#ifdef RUN_THREADS` branches, `spin_lock`, `tt_spin_lock`, the `// #define RUN_THREADS` line and the `<atomic>`, `<future>`, `<thread>` includes if nothing else uses them | missing asset: `build/tests/test_perft; echo $?` prints 0 today with no output, non-zero with a message after; column: section 6 scratch JSON, exit 0 today, 1 after | perft counts untouched |
| R6 | `tests/test_search.cpp` "the node budget is never exceeded" and `tests/test_engine.cpp` "a stopped infinite search answers with a legal move", named here after the rename this row performs -- they read "the node budget is honoured exactly" and "an infinite search answers only once stop arrives" before it | the first asserts `explored_nodes <= budget`; the second asserts one playable `bestmove` after `stop` and nothing about before | rename to "the node budget is never exceeded" and "a stopped infinite search answers with a legal move"; no living document quotes either title (grepped `DEV_MANUAL.md`, `MANUAL.md`, `adocs/specs.md`, `adocs/plan.md`, `adocs/status.md`, `tools/`) | none, a rename | none |
| R7 | `tests/test_helpers.hpp` `legal_moves()` | comment: "a regression to pseudo-legal generation fails here"; body REQUIREs `make_move`, which refuses only a full history stack | preferred: assert what the comment claims -- after `make_move`, `REQUIRE_MESSAGE(position_is_reachable(game), ...)` (the mover's king is not attacked, which is legality), then `unmake_move`; move `position_is_reachable` above `legal_moves` in the header. Fallback if the Debug fast label grows past its timeouts: reword the comment to "legality is pinned by the perft and JSON counts; this wrapper only asserts the stack guard" | a generator mutant that emits a move leaving the mover's king attacked (the implementer finds the anchor in `src/bitboard.cpp` `generate_moves`); if none is cheap to build, say so in the stamp -- accepts says "where a mutation exists" | none |
| R8 | `tests/test_chesso.cpp` `test_generate_legal_moves` | filters with `is_move_legal`, which is membership in the generator's own output -- a no-op; `assert(all_moves_count < MAX_MOVES)` compiles out | body becomes `generate_moves` plus `REQUIRE_LT(count, MAX_MOVES)`; the comment says the generator is compared raw against the JSON, whose exact count and per-move FEN pin legality (INV-1) | none needed; the JSON cases already kill M22 and M27 (`results.tsv`) | none |
| R9 | `fixture_path()` in `tests/test_tuner_split.cpp` and `tests/test_tuner_gradient.cpp`; the two literals in `tests/test_openings.cpp` "Test a book is loaded from a file" and "Test a book that is not a book is refused"; **also** the three names in `tests/test_corpus_hash.cpp` `fixture_path("chesso_test_corpus_hash*.tsv")`, which the review did not list | fixed names under `temp_directory_path()`: two concurrent `ctest` runs write the same file | a helper `unique_fixture_path(prefix)` that builds `temp_directory_path() / (prefix + ".XXXXXX")` into a `std::vector<char>`, calls `mkstemp`, `close`s the fd and returns the name; the existing `std::filesystem::remove` calls stay | none deterministic; state in the stamp that the hazard is removed by construction | none |
| R10 | `tests/test_fastchess_script.sh`, the `REF=HEAD~1` candidate-line check | `grep -qE "^candidate  $older_head  $(date +%F)\$"`: a run straddling midnight fails | `$(git -C "$older_dir" show -s --date=short --format=%cd HEAD)`, the same form `fastchess.sh`'s `commit_date()` uses; the reference line's forced `2020-01-02` is what still catches a banner that prints today's date | none deterministic; the test stays green and the wall clock leaves it | none |
| R11 | `tests/test_chesso.cpp` `chesso_fixture_t`; `tests/test_openings.cpp` `openings_fixture_t` -- named here after this row deletes the init-only cases they replace, "Test INITIALIZATION" and "Initialize" | cases that only call `initialize_game_const_data(&game)`; every later case depends on doctest's default file order; `game_tables()` asserts readiness in Debug only | a `chesso_fixture_t` / `openings_fixture_t` in the shape of `tests/test_engine.cpp` `engine_fixture_t`, every `TEST_CASE` becomes `TEST_CASE_FIXTURE`, the init-only cases are deleted | `cd build/tests && ./test_chesso -tc="Basic test"` and `./test_openings -tc="Test key generation"` before: fail (Release runs on zero tables; Debug asserts); after: pass; also `--order-by=name` on both, green | none |
| R12 | `tests/test_audit_go_infinite.cpp`, unregistered | its 200 ms sleep passes vacuously on a slow machine; the engine defect it targets (`2026-09-04_adversarial-F01`) reads `Status: open` at HEAD | leave unregistered and say so in the stamp. If the fix has landed by then: replace the sleep with a poll, under a hard ceiling that fails the case, until the capture shows the search has reached the depth at which the defect fired, then assert no `bestmove` yet | only if registered: the case is red on the pre-fix engine by construction | none |

Items the review lists under F05 that the `accepts:` does not name (see section 10):

| id | site | what is vacuous today | falsifiable form | red-first |
|---|---|---|---|---|
| R13 | `tests/test_engine.cpp` "an irreversible move clears the window" | after one pawn push from the start position the history has one entry; `is_position_repeated`'s loop never runs | play the knight shuffle the previous case uses until `REQUIRE(is_position_repeated(...))` holds (precondition), then set `game.board.halfmove_clock = 0` by hand on the same history and `REQUIRE_FALSE(is_position_repeated(...))`: the clock alone closes the window | mutant: `limit` in `src/bitboard.cpp` `is_position_repeated` ignores the clock (`history->size` always) -- red |
| R14 | `tests/test_search.cpp` "quiescence writes entries of its own" | `REQUIRE(depth >= TT_DEPTH_QS)` over every entry; `TT_DEPTH_QS` is -1 and no writer stores below it | `REQUIRE(depth == TT_DEPTH_QS \|\| depth >= 1)`: a main-search store at depth 0 is impossible because `negamax` hands `depth < 1` to `quiescence` | mutant: the handoff becomes `depth < 0` -- entries at depth 0 appear, red |
| R15 | `tests/test_uci_surface.cpp` "the release build declares exactly the five golden lines" clause | `CHECK(expected_option_lines.size() == 5)` compares a literal in the test file with itself | `CHECK(actual.size() - tune_option_lines().size() == 5)` reads the binary; name 5 as a golden re-derived by `printf 'uci\nquit\n' \| ./build/src/chesso \| grep -c '^option name'` (no search runs, so the pipe is safe) | none beyond the rewrite |
| R16 | `tests/test_evaluation.cpp` "the lazy shortcut cannot change a decision" | `REQUIRE(correction <= LAZY_EVAL_MARGIN)` is true by the `std::clamp` in `src/evaluation.cpp` `evaluate_expensive` | drop the REQUIRE, keep `worst` and report it with `MESSAGE(...)` as a measurement, and say in the comment that the bound is by construction; the four `evaluate_lazy` assertions below it are the soundness claim and stay | none |
| R17 | `tests/test_openings.cpp` "Test get moves" | `REQUIRE(make_move(&game, moves[0]))` -- `make_move` refuses nothing but a full stack | assert the book move is in `generate_moves`' output for the start position (from and to squares match one generated move) | mutant: return a fabricated move from the book lookup -- red |

Order of edits: R1 first and alone (it is the finding with a measured survivor), then R5 (the only item with a non-doctest binary), then the rest file by file; commit once at the end per COMMITS, or per file if the diff gets large. Run `./clang-format.sh` after each file.

### 4. Constants and seeds

None. The step introduces no tunable. The numbers in the tests are rule constants (100, 99, 98 halfmoves), an existing budget (200 ms, with the `>= 100` floor being half of it, DEC-105 form (c), midpoint between "spent nothing" and "spent all"), and the option-line count, which R15 turns into a named golden with its derivation command as DEC-142 asks.

### 5. Interactions and traps

- **T1, the accepts' `ucinewgame` does not clear the stop flag.** At HEAD `src/chesso.cpp` `command_ucinewgame` calls `stop_and_join_search()` (which sets `stop_search_signal`), then `set_position`, `tt_reset`; the only callers of `begin_search_session()`, the function that clears the flag, are `command_go` and `command_test`. `begin_search_session` is not declared in any header, so a test cannot call it. The working form is the one the probe lambda in "the iteration loop scales its soft limit by what the search found" already uses: `go depth 1` then `uci_wait_for_search()`, inside a capture. Sending `ucinewgame` as well is harmless and satisfies the accepts' word; the stamp must say that the flag is cleared by the `go`, not the `ucinewgame`. Section 10 asks the owner to amend the wording.
- **T2, the boundary pair's two probes must not share a table.** `search_fen` resets `tt` on every call, keep it that way: the Zobrist key does not carry the halfmove clock, so a warm entry from the clock-98 probe would answer the clock-99 child and the pair would measure the table.
- **T3, Stockfish's number for the clock-98 root is 0.** At `go depth 20` it reports 0 for `4k3/8/8/8/8/8/8/3QK3 w - - 98 200` (2861 nodes): it sees the coming claim. That is the position's value and irrelevant here; R1 pins **the engine's scoring of the boundary node at one ply**, and the control asserts only that chesso's depth-1 score at clock 98 is not 0 (929 today). Do not "correct" the control toward the oracle, and make no chess claim about either FEN (DEC-023) -- the tool output in section 6 is all that is said about them.
- **T4, M19 is invisible to INV-6.** `results.tsv` shows M19 with node counts and best moves identical to the baseline at depth 9. R1 is the only guard; a future step that moves the fifty-move block must re-run it red-first.
- **T5, Release compiles out `assert`.** R5's `load_json` and R8's `assert(all_moves_count < MAX_MOVES)` are the same class as `2026-09-04_test_review-F01`; every check that must hold in the gate is a `REQUIRE` or an explicit exit, never `assert`.
- **T6, perft reads its assets relative to the working directory** (`test_files` holds `assets/perft_json/...`); CMake copies `assets/` into `build/tests/`, so run it from there, or from a scratch directory you populate on purpose (section 6).
- **T7, R7's preferred form costs a null make/unmake per generated move at every `legal_moves()` call site.** Measure the Debug binaries afterwards (`tests/CMakeLists.txt` gives `test_movegen` and `test_search` 600 s; they took 165 s and 216 s when that was set); if a binary approaches its ceiling, take the fallback wording instead and say so.
- **T8, R9's `mkstemp` creates the file.** The tuner fixtures open the path with `std::ofstream out(path, std::ios::trunc)`, which is fine on an existing empty file; `tests/test_tuner_split.cpp`'s `path_after` check still asserts the file is gone after the read. `tests/test_openings.cpp`'s "Test a book that is not a book is refused" has a SUBCASE "a path that does not open" -- that one wants a path that does *not* exist, so build it from the unique name plus a suffix rather than from `mkstemp`'s own file.
- **T9, R10 must use the exact date format the banner uses**, `git show -s --date=short --format=%cd`; `%cs` would also work but needs a newer git and differs in spelling from `fastchess.sh` `commit_date()`.
- **T10, the ctest count line stays "27 of 27"** unless R12 registers a binary; if it does, `DEV_MANUAL.md` "Test" and the gate's expected count move with it.

### 6. Tests

**R1, the boundary pair, with its tool record.** Positions produced and checked with tools on 2026-09-05, never read off a board: python-chess 1.11.2 (`~/.venv/chess/bin/python`) reports both FENs `Status.VALID`; from `4k3/8/8/8/8/8/8/3QK3 w - - 99 200` there are 21 legal moves, all 21 non-capture non-pawn, every child has `halfmove_clock` 100 and `is_fifty_moves()` true, 0 children are checkmate and 0 are insufficient material; from the same placement at clock 98 the children are at 99 and 0 of 21 are `is_fifty_moves()`. The built engine (`build/src/chesso`, `src/` at `18deccf`) driven through `chess.engine.SimpleEngine` (which waits for `bestmove`, the safe form `TOOLCHAIN.md` prescribes) answered `go depth 1`: clock 99 `cp 0`, 22 nodes, pv `d1d8`; clock 98 `cp +929`, 51 nodes, pv `d1d6`. Re-run that check on the workstation before writing the case (the script is short: `chess.Board(fen)`, iterate `legal_moves`, `push`, read `halfmove_clock`, `is_checkmate()`, `is_insufficient_material()`, `is_fifty_moves()`; then `SimpleEngine.popen_uci("build/src/chesso")` and `analyse(board, Limit(depth=1))`). Case shape:

```
TEST_CASE_FIXTURE(search_fixture_t, "the fifty-move boundary lands on the hundredth halfmove")
{
  const char* at_99 = "4k3/8/8/8/8/8/8/3QK3 w - - 99 200";
  const char* at_98 = "4k3/8/8/8/8/8/8/3QK3 w - - 98 200";

  // Preconditions, from the engine's own generator: a reachable root, material
  // that is not an insufficient-material draw, at least one reply, and every
  // reply quiet (no capture, no pawn move), landing on clock exactly 100 with
  // the mover not in check -- so the draw, and only the draw, applies at ply 1.
  REQUIRE(load_FEN(at_99, &game));
  REQUIRE(position_is_reachable(&game));
  REQUIRE_FALSE(is_insufficient_material(&game.board));
  move_t replies[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game.board, replies);
  REQUIRE(count > 0);
  for (size_t i = 0; i < count; ++i) {
    REQUIRE_FALSE(MOVE_CAPTURE(replies[i]));
    REQUIRE_NE(MOVE_PIECE(replies[i]), W_PAWN);
    REQUIRE(make_move(&game, replies[i]));
    REQUIRE_EQ(int(game.board.halfmove_clock), 100);
    REQUIRE_FALSE(is_check(&game));
    unmake_move(&game);
  }

  REQUIRE_EQ(search_fen(at_99, 1).score, 0);  // every reply is a draw at ply 1
  REQUIRE_NE(search_fen(at_98, 1).score, 0);  // one halfmove earlier it is not
}
```

`tests/test_search.cpp` already includes `test_helpers.hpp` and uses `MOVE_PIECE` and `W_PAWN`. The mutant, applied by hand from the review's own pair (do not use `run.py`, trap in section 2):

```
cd /Users/max/ws/chesso && python3 - << 'PY'
import importlib.util, pathlib
spec = importlib.util.spec_from_file_location("m", "adocs/data/2026-09-04_test_review/mutants.py")
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
mut = next(x for x in m.M if x["id"] == "M19_fifty_off_by_one")
p = pathlib.Path(mut["file"]); t = p.read_text()
for old, new in mut["pairs"]:
    assert t.count(old) == 1, old
    t = t.replace(old, new)
p.write_text(t); print("applied", mut["id"], "to", mut["file"])
PY
cmake --build build -j8 --target test_search && (cd build/tests && ./test_search -tc="*fifty-move boundary*")
```

Expected: the case fails on `REQUIRE_EQ(search_fen(at_99, 1).score, 0)` with the clock-98 number. Then `git checkout -- src/search.cpp`, rebuild, re-run: green. Also confirm the existing case "the fifty move rule is a draw" is green under the mutant (it is the vacuity being fixed, and it stays as the depth-4 companion). Record both observations in the stamp.

**R2.** Add `REQUIRE(elapsed >= 100)` first and run `./test_engine -tc="a search with no limit is still bounded"` -- red, elapsed near 1 ms. Add the clearing (`go depth 1`, `uci_wait_for_search()`) -- green. Run the case five times and `MESSAGE` the elapsed value; if it clusters at or above 200 ms, keep 100 as the floor; if the loop stops earlier than that (an iteration it declines to start), lower the floor to half of what is observed and say why in the comment.

**R3.** Mutant in `src/chesso.cpp` `try_load_opening_book` as the table says; red is the missing `info score ` line; revert with `git checkout -- src/chesso.cpp` and rebuild before anything else runs.

**R5, column check red-first without waiting for the slow label.** Build a scratch directory holding `assets/perft_json/talkchess_perft.json` and `assets/perft_json/perft.json`, each a one-entry copy of the first entry of the real file with `depth_limit` 2 and the `captures` value at depth 2 changed by one; `cd` there and run `/Users/max/ws/chesso/build/tests/test_perft; echo $?` -- 0 today (only `nodes` counts), 1 after. The missing-asset case is the table's command. Then the honest green: `ctest --test-dir build -L slow --output-on-failure`, minutes.

**R11.** `cd build/tests && ./test_chesso -tc="Basic test"; ./test_openings -tc="Test key generation"` before and after; then `./test_chesso --order-by=name` and `./test_openings --order-by=name`, both green after.

**INV-6, for the record only** (no `src/` change): `python3 tools/search_bench.py ./build/src/chesso 9` and `... 12` before and after -- identical by construction; quote the numbers in the stamp anyway so the claim is checked, not assumed. **Debug self-play (DEC-141 tier 2): not required**, the step touches neither `make_move`, the generator nor the search; say so in the stamp. **Goldens (DEC-142): none moved**; one named (R15).

### 7. Measurement

Lane: none. The step alters no play and changes no `src/` file, so there is no SPRT, no pre-registration script under `adocs/data/S193_*.sh` and no Bench line. The measurement is the gate in both builds plus the slow label once, and the red-first observations of section 6 recorded per row in the stamp. If a `src/` change is ever proposed to make an item pass, that is a different step.

### 8. Completion checklist

1. Every row of the worklist done or explicitly deferred with its reason (R7 fallback, R12 unregistered while F01 is open, R13 to R17 per the owner's answer to section 10).
2. Gate: `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`; then `ctest --test-dir build -L slow --output-on-failure` for R5.
3. `git status --short` shows `tests/` only (and `DEV_MANUAL.md`, `adocs/`); `git diff --stat -- src/` is empty. No Bench line and no `No functional change` line are owed (DEC-140 applies to commits touching `src/`).
4. `DEV_MANUAL.md` "Test": the `test_fastchess_script` paragraph ("each side's date comes from its own commit") gains the clause that the expected date is read with `git show`, so the case does not depend on the wall clock; one sentence beside `ctest --test-dir build -L slow` that `test_perft` exits non-zero on a missing or unparseable asset and on any mismatch in its five columns; one sentence that every doctest binary initialises its tables through a fixture, so `-tc=<glob>` and `--order-by=name` are safe ways to run one case or shuffle them. `MANUAL.md`: no change, the UCI surface is untouched -- say so. `adocs/specs.md`: the fifty-move sentence ("A node whose halfmove clock has reached 100 scores a draw only once checkmate has been ruled out") is precise; optionally append "pinned at the boundary by a depth-1 pair at clocks 99 and 98 since S193". `README.md` untouched.
5. `test_uci_surface`: not refreshed, the surface did not change.
6. Stamp: per row, the red observation and the green (R1 with the M19 command and both scores; R2 with the elapsed values seen; R5 with both exit codes; R11 with the `-tc` runs), the T1 note that the flag is cleared by `go depth 1`, the R12 status line quoted from `adocs/audit/2026-09-04_adversarial.md`, the INV-6 numbers, "no `src/` change, no Bench line, no Debug self-play owed", and which of R13 to R17 were taken.
7. Commit message body names S193, F04, F05, F09 and INV-6; the coordinator moves the file, updates `adocs/plan.md`, `adocs/status.md` and the three findings' `Status:` lines in `adocs/audit/2026-09-04_test_review.md`.

### 9. Sources read

- https://www.chessprogramming.org/Fifty-move_Rule -- fetched; the `>= 100` condition and the checkmate precedence, quoted in section 2.
- https://handbook.fide.com/chapter/E012023 -- fetched; Articles 5.1.1, 9.3, 9.6.2 quoted.
- https://github.com/doctest/doctest/blob/master/doc/markdown/assertions.md -- fetched; `REQUIRE` vs `CHECK`, `_MESSAGE`.
- https://github.com/doctest/doctest/blob/master/doc/markdown/commandline.md -- fetched; `--order-by` default `file`, `-tc=`.
- `adocs/audit/2026-09-04_test_review.md` F04, F05, F09 and the mutant table; `adocs/data/2026-09-04_test_review/mutants.py`, `run.py`, `results.tsv` (M19 row: 0 of 27 red, bench identical).
- `adocs/audit/2026-09-04_adversarial.md` F01 -- `Status: open` at HEAD (R12).
- `src/search.cpp` `negamax` (the fifty-move block and the `depth < 1` handoff); `src/chesso.cpp` `command_ucinewgame`, `command_position`, `command_go`, `command_test`, `begin_search_session`, `stop_and_join_search`, `try_load_opening_book`; `src/log.hpp` `LOG_I`; `src/bitboard.cpp` `game_tables`, `is_position_repeated`; `src/evaluation.cpp` `evaluate_expensive`, `evaluate_lazy`; `src/transposition_table.hpp` `TT_DEPTH_QS`.
- `tests/`: every site in the worklist, `tests/test_helpers.hpp` `position_is_reachable`, `tests/CMakeLists.txt`, `tests/assets/perft_json/*.json` (per-depth keys `nodes, captures, en_passant, castles, promotions, checks, discovery_checks, double_checks, checkmates`; the binary counts the first five).
- `adocs/plan_done/S162_fifty_move_mate_precedence.md` and `adocs/specs.md`'s fifty-move sentence; DEC-023, DEC-025, DEC-107, DEC-139, DEC-141, DEC-142; `adocs/plan_todo/S194_book_path_under_test.md` (R3 does not overlap: S194 tests the book hit, R3 the bookless search).
- `TOOLCHAIN.md` "The chess oracle, and the one way to ask it that lies" -- the wait-for-`bestmove` form used for the tool record.
- Tool run 2026-09-05: python-chess 1.11.2, `build/src/chesso` (built from `src/` at `18deccf`), stockfish at `~/.local/bin/stockfish` -- the figures in section 6 and trap T3. No figure in this section is unverified.

### 10. Questions deferred to the owner

1. **The accepts says `ucinewgame` clears the hazard in R2; at HEAD it cannot** (trap T1). Proposed wording: "the stop flag cleared through a completed `go depth 1` before `tests/test_engine.cpp`'s 'a search with no limit is still bounded', and the case shown to spend at least half its 200 ms budget". Alternatively accept the stamp's deviation note.
2. **R13 to R17 and the `test_corpus_hash` names in R9** are in the review's F05/F09 lists and in the goal's words ("vacuous assertions are made falsifiable", "temp-file hazards removed") but not in the accepts' enumeration. Recommendation: include them, they are minutes each and none touches `src/`; the accepts list would gain a clause. Owner to say yes or no before the step starts.
3. **R7, `legal_moves()`**: assert the property (preferred, one null move per generated move in every test that calls it) or reword the comment (cheaper, leaves the claim to perft and the JSON counts). Recommendation: assert, subject to trap T7's Debug timing.
