id:         S007
goal:       one process-wide copy of the attack tables instead of one per game_t
accepts:    sizeof(game_t) falls by the size of bb_tables_t; perft node counts unchanged; every table read goes through the singleton
touches:    src/bb_tables.hpp, src/bitboard.cpp game_tables()
excludes:   shrinking the tables themselves -- fancy or black magics are a separate question, see DEC-006
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in d492de5. README and MANUAL checked at adoption, not when this shipped.

## Result

`game_t` 2515 KB to 87 KB, of which 80 KB is the move history. This was an
ergonomics problem, not a speed problem: anything that copies a `game_t` was
paying 2.5 MB. It closes a one-way door rather than buying nps.
