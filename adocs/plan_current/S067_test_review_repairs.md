id:         S067
goal:       repair the test defects the 2026-08-14 test review found, before any further measurement is taken
accepts:    every case the review named either asserts what its title claims or is gone, and the class is closed by a precondition rather than by two edits. Specifically: (1) `"the winning move is found"` asserts, before any search, that each of its positions is legal and has more than one legal move, and the case that had one is replaced by a position verified with `python-chess` and `stockfish` rather than by reading; (2) the mate-in-zero position is a legal mate, asserted legal by the same precondition, and both cases that use it carry it; (3) `ctest --test-dir build-debug -L fast` is 12 of 12 rather than two Timeouts; (4) `tests/test_helpers.hpp` states INV-1 as it is and `legal_moves()` asserts the property instead of pretending to filter for it; (5) `"which material can still mate"` pins bishop-against-bishop, both square colours, and bishop-against-knight, in whichever direction the code answers today, with the rule-following answer recorded beside it as data; (6) `"ordering keeps the tree small"` sets its budget from a measured node count with the margin stated. Every one of the six observed red before it was made green, with the verbatim failure recorded in `adocs/testing.md`. `ctest --test-dir build -L fast` green and `./clang-format.sh --check` clean
touches:    tests/test_search.cpp, tests/test_helpers.hpp, tests/CMakeLists.txt, adocs/testing.md
excludes:   the built-in `test` command (F07, accepted in DEC-058) and anything that would give the engine a `bench` signature; `tests/debug_perft_app.cpp`, which has F04's redundancy but is the perftree tool rather than a test; adding a mate suite, sanitizer runs, CI or more perft positions, which are the review's four gaps against other engines and are instruments rather than repairs; changing `is_insufficient_material()` in either direction, which is a strength question and an SPRT; anything under `src/`
decisions:  DEC-058, DEC-019, DEC-023
closes:     2026-08-14_test_review-F01, 2026-08-14_test_review-F02, 2026-08-14_test_review-F03, 2026-08-14_test_review-F04, 2026-08-14_test_review-F05, 2026-08-14_test_review-F06
blocks:     S065
paused_by:
done:

## Why this cuts the queue

S065's `accepts:` has one thing left in it and that thing is a measurement:
`REF=a2f0065 CONCURRENCY=12 ./fastchess.sh`, with nothing measured yet.
AGENTS.md section 0 says a bug that has been found gets fixed before anything
else starts, because a known defect in the tree contaminates every measurement
taken after it. Two cases in the suite that gates every commit cannot fail for
the reason they exist and a third asserts a premise that is false, so the repair
goes first and S065 is paused behind it. DEC-058.

## The two positions, verified rather than reasoned about

CLAUDE.md forbids assessing a position from the agent's own reasoning. Both
replacements were chosen with tools and the tool output is what the step is
built on.

F01, replacing `2k5/8/8/8/8/8/1q6/K1R5 w - - 0 1`:

    4k3/8/8/8/4K3/4q3/8/7R w - - 0 1

`python-chess`: `Board.status()` is `VALID`, 3 legal moves — `e4f5`, `e4d5`,
`e4e3`. `stockfish` `go depth 20 MultiPV 3`: `e4e3` **mate 20**, `e4d5`
**mate -12**, `e4f5` **mate -11**. Unique best, and winning rather than merely
least bad, which the bare-king version of the same idea would not have been.

F02, replacing `7k/5Q1K/8/8/8/8/8/8 b - - 0 1` at two call sites:

    7k/6Q1/6K1/8/8/8/8/8 b - - 0 1

`python-chess`: `Board.status()` is `VALID`, `legal_moves` 0, `is_checkmate()`
true, `is_stalemate()` false. The old position's only attacker of the black king
was the white king on h7, which is why it was `OPPOSITE_CHECK`.

## The precondition is the fix

Two illegal positions passing for years is the finding; the two instances are
symptoms. `tests/test_helpers.hpp` gains one predicate — the side **not** to
move must not be in check, taken from the engine's own `is_check()` through a
null move rather than from a rule written here — and the cases that assert
something about a legal game assert it first. Adjacent kings and a capturable
king are both caught by it, since either makes the non-mover attacked.

Positions that are illegal on purpose keep their cases and do not get the
precondition: `test_evaluation`'s missing kings, `test_search`'s
`"a position with a capturable king is survivable"` and
`"a board with no kings is survivable"`, `test_chesso`'s `is_capturing_king`
pair, and `test_eval_model`'s promotion-phase position. Those are eight of the
ten illegal FENs in the suite and every one of them is deliberate.
