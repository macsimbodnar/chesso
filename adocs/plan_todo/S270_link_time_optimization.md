id:         S270
goal:       the engine is built with link-time optimization if a counter-based interleaved timing on the workstation shows it faster, and `game_tables()` becomes an inline accessor
accepts:    (1) an A/A of two copies of the parent's binary first, with `perf stat -e instructions,cycles`, to read the counter's own noise; (2) the parent rebuilt with CMake's `INTERPROCEDURAL_OPTIMIZATION`, timed interleaved against it on the same counters and on nps; (3) enabled for the engine targets only if it holds, in both gated builds, with `-Werror` still on and the PGO path in `cmake/pgo.cmake` checked to build; (4) `src/bitboard.cpp` `game_tables` moved inline into `src/bitboard.hpp` as a separate change, timed on its own (CLAUDE.md rule 6); (5) **node identity** (INV-6) for both changes: `bench` and `tools/search_bench.py` identical to the parent at two depths; (6) `DEV_MANUAL.md` states the build flags as they then are, and the LTO remark in `src/evaluation.hpp` follows the result
touches:    CMakeLists.txt, src/CMakeLists.txt, cmake/pgo.cmake, src/bitboard.cpp, src/bitboard.hpp, src/evaluation.hpp, DEV_MANUAL.md
excludes:   moving definitions between translation units by hand beyond `game_tables`; compiler or flag changes other than LTO
decisions:  DEC-083, DEC-263
closes:     2026-10-08_performance-F03
blocks:
paused_by:
author:
done:

## Why

No build enables LTO, so every cross-unit call on the search path --
`tt_get_entry`, `tt_store_entry`, `score_move`, `see_ge`, `is_check`,
`make_move`, `game_tables()` -- stays out of line. S104 recorded LTO as inside
the noise from two wall-clock runs each, an instrument whose resolution was
coarser than the effect (CLAUDE.md rules 4 and 5). On the M1, with cycle
counters, an LTO build was node-identical and measured -1.2 to -1.7 % cycles.
The workstation's gcc has not been measured.
`adocs/audit/2026-10-08_performance.md`, F03.

## Lane

Build work on the workstation, where it is timed; no match (DEC-083).
