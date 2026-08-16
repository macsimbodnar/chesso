id:         S070
goal:       the terminal-position test asserts a legal mate or stalemate, not a king adjacent to a king
accepts:    `tests/test_engine.cpp` no longer loads `7k/5Q1K/8/8/8/8/8/8 b - - 0 1`; the replacement position is accepted by an external tool and the tool's output is quoted in the case text; the case asserts legality as a precondition before asserting that `first_legal_move()` returns 0, so it fails if the position stops being terminal; `grep -rn "7k/5Q1K" tests/` returns comments only; fast suite green
touches:    tests/test_engine.cpp
excludes:   tests/test_search.cpp, where S067 already fixed the two sites `2026-08-14_test_review-F02` named and where the FEN now survives in a comment on purpose; any change to `first_legal_move()` itself
decisions:
closes:     2026-08-16_plan_review-F06
blocks:
paused_by:
done:      2026-08-16. tests/test_engine.cpp:581 no longer loads 7k/5Q1K/8/8/8/8/8/8 b - - 0 1. The one illegal FEN is replaced by a matched pair verified by tool, not by reading (DEC-023): 7k/6Q1/6K1/8/8/8/8/8 b (mate) and 7k/5Q2/6K1/8/8/8/8/8 b (stalemate).
            
            Tools quoted into the case text. stockfish loads both and answers 'info depth 0 score mate 0' / 'bestmove (none)' and 'info depth 0 score cp 0' / 'bestmove (none)'. python-chess: both Status.VALID, legal_moves 0; is_checkmate True/False, is_stalemate False/True.
            
            Non-vacuity. Two preconditions run before the count, on the game_t copied out of uci_game(): position_is_reachable() - the engine's own is_check() through a null move, S067's predicate - then is_check(&game) == terminal.in_check, true for the mate and false for the stalemate. The case now fails if either position stops being terminal, or stops being the kind of terminal it claims.
            
            Observed red twice. Old FEN re-added as a third row: 'FATAL ERROR: REQUIRE( position_is_reachable(&game) ) is NOT correct!  values: REQUIRE( false )' / 'logged: 7k/5Q1K/8/8/8/8/8/8 b - - 0 1 is not a position a legal game can reach'. Stalemate's expected in_check flipped to true: 'FATAL ERROR: REQUIRE( is_check(&game) == terminal.in_check ) is NOT correct!  values: REQUIRE( false == true )' / 'logged: 7k/5Q2/6K1/8/8/8/8/8 b - - 0 1 is not in check as expected'.
            
            Fixed in scope, trivial: doctest stringifies a const char* as its address, so the first draft printed 'logged: 0x5f73d51210e4' and named nothing. The FEN reaches REQUIRE_MESSAGE as a std::string.
            
            grep -rn '7k/5Q1K' tests/ returns two comment lines only: test_search.cpp:146 (S067's, kept on purpose) and test_engine.cpp:559 (this step's).
            
            Gate: cmake --build build -j12 && ctest --test-dir build -L fast 12 of 12, 0 failed, 20.01 s; ./clang-format.sh --check clean. git diff -- src/ tools/ empty, so no play changed, no SPRT owed and INV-6 has nothing to discharge. README.md owner-written, no change needed; MANUAL.md and DEV_MANUAL.md checked, no surface change.
            
            2026-08-16_plan_review-F06 is planned, not closed: AGENTS.md section 10 closes a finding only after the audit is re-run, and this step fixed rather than re-ran.

## Why this exists

`2026-08-14_test_review-F02` was recorded fixed. It is fixed in two of three
places. `tests/test_engine.cpp:560` still runs

```
uci_process_line("position fen 7k/5Q1K/8/8/8/8/8/8 b - - 0 1");
```

inside `TEST_CASE("first_legal_move reports nothing in a terminal position")`,
which asserts `REQUIRE_EQ(first_legal_move(), 0)`.

The position is not legal. The only attacker of the mated king is the enemy
king, which cannot deliver mate:

```
$ printf 'position fen 7k/5Q1K/8/8/8/8/8/8 b - - 0 1\ngo depth 5\nquit\n' | stockfish
info string CRITICAL ERROR: Command `position fen 7k/5Q1K/8/8/8/8/8/8 b - - 0 1`
failed. Reason: Unsupported position. King can be captured.
```

The assertion passes. What it proves is that a side whose king stands next to
the enemy king has no legal move -- not that a legally terminal position reports
none, which is the property the case is named for.

## The rule this step is written under

**Chess judgement comes from a tool, never from the agent** (CLAUDE.md, DEC-023).
This step does not name a replacement position, because choosing one is a chess
judgement. Whoever executes it gets a mate or a stalemate from `stockfish` or a
tablebase, quotes what the tool printed into the case text, and asserts the
precondition -- the shape S067 used in `tests/test_search.cpp`.

## Non-vacuity

The case must fail if its precondition disappears. Asserting only that
`first_legal_move()` returns 0 is satisfied by any position the loader rejects,
which is exactly how the current one passes. Assert first that the position
loaded and that the side to move is in check (mate) or is not (stalemate), then
assert the count.

## Cost

Minutes plus one `stockfish` invocation. No match.
author:    Maksym Bodnar
