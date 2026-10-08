id:         S271
goal:       the transposition table is cleared by `ucinewgame`, `clean-tt` and a Hash change and never by a new base FEN, and a clear of a table nothing has written to since the last clear is skipped
accepts:    (1) **A**: `src/chesso.cpp` `commit_position_base` no longer clears the table; the remembered base and the book re-arm are unchanged; (2) **red first**: a test that searches a position, then sends a later board of the same game as a bare `position fen` with no `ucinewgame`, and finds the entry the first search stored -- observed red on the parent, green after A; (3) the tests whose premise was the base clear are re-stated under DEC-264, not relaxed: `tests/test_engine.cpp` "a refused position command leaves the table alone" loses its precondition (a different base no longer clears) and is re-stated so a refused command is still shown to change nothing it could have changed; the comments in "a repeated go depth is cold only across ucinewgame" and "bench searches its last position cold" that cite the base clear are corrected, and each case still fails on the mutation it was written for; (4) **B**: `src/transposition_table.cpp` `tt_reset` skips its `memset` when nothing has written to the table since the last clear; the flag is invalidated before any write can happen -- the implementer states where and why that covers every writer, `go`, `bench` and the tools that drive the search; a test shows a clear after a search still clears, and B's remaining reach after A is counted on fastchess's command sequence: if it is nil in ordinary play, B is dropped and recorded as zero (CLAUDE.md rule 8); (5) **harness play unchanged** (INV-6): the `bench` total and `tools/search_bench.py` node counts and best moves identical to the parent, since `ucinewgame` and `bench` still clear; (6) `adocs/data/S271_replay.py` re-run on the result at depth 12 and Hash 16: `h_bare` within 0.2 % of `h_base` in total nodes, read with `adocs/data/S271_replay_read.py` and added to `adocs/data/S271_replay.md`; (7) `adocs/specs.md` and `MANUAL.md` describe the new rule -- MANUAL's "a new base still does all three" paragraph above all -- before any `test_uci_surface` refresh the change needs (SURFACE rule)
touches:    src/chesso.cpp, src/transposition_table.cpp, src/transposition_table.hpp, tests/test_engine.cpp, adocs/specs.md, MANUAL.md, adocs/data/S271_replay.md
excludes:   recognising a bare FEN as the continuation of the last position to keep the repetition history (rejected, DEC-264); a faster or threaded clear; the table layout and replacement, which are S119; the validation match, which is S272
decisions:  DEC-264, DEC-263, DEC-083
closes:     2026-10-08_performance-F04
blocks:
paused_by:
author:
done:

## Why

A client that sends a board as a bare FEN, or that moves its base FEN after a
capture or pawn move, gets a cold table: `adocs/data/S271_replay.md`
measured +21.8 to +26.8 % nodes to the same depth for a bare FEN each move
and +11.9 to +14.4 % for the irreversible-move base, over 8653 positions --
about 28 to 60 Elo by conversion, not a verdict. The same engine with the
clear removed reads within 0.1 % of base-plus-moves in every cell. The UCI
text the repository ships gives new-game signalling to `ucinewgame`, and the
table's replacement already lets any entry from an earlier search be
overwritten (`src/transposition_table.cpp` `tt_store_entry`), so an entry
kept from another game cannot block a slot. fastchess and python-chess hold
the base fixed for a game, so the harness sees no change.

B was proposed against the three clears a game start from a book FEN pays
today. After A two of them are gone; what remains is a `ucinewgame` on a table
that is already clean, and its reach is counted rather than assumed.

## Lane

Agent work under DEC-260. UCI-layer change; it does not touch `make_move`,
the generator or the search, so DEC-141's second tier does not apply.
