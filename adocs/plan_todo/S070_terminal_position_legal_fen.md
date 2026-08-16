id:         S070
goal:       the terminal-position test asserts a legal mate or stalemate, not a king adjacent to a king
accepts:    `tests/test_engine.cpp` no longer loads `7k/5Q1K/8/8/8/8/8/8 b - - 0 1`; the replacement position is accepted by an external tool and the tool's output is quoted in the case text; the case asserts legality as a precondition before asserting that `first_legal_move()` returns 0, so it fails if the position stops being terminal; `grep -rn "7k/5Q1K" tests/` returns comments only; fast suite green
touches:    tests/test_engine.cpp
excludes:   tests/test_search.cpp, where S067 already fixed the two sites `2026-08-14_test_review-F02` named and where the FEN now survives in a comment on purpose; any change to `first_legal_move()` itself
decisions:
closes:     2026-08-16_plan_review-F06
blocks:
paused_by:
done:

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
