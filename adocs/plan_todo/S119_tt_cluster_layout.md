id:         S119
goal:       the table becomes cache-line clusters with an aged replacement, a prefetch issued when the key is known, and huge pages
accepts:    an SPRT verdict at **Hash 128** -- the regime S105 sets and the one the rating list runs, because a table change measured at 16 MB measures the wrong table; `sizeof` the cluster is exactly 32 or 64 bytes and the array is aligned so no cluster straddles a cache line, asserted at compile time; the replacement prefers depth **and** age together rather than depth within a generation alone; the prefetch is issued as soon as the key is known in make_move; huge pages are requested and the failure path is a normal allocation, not an abort; `hashfull` is reported over UCI and is checked to stay low at the rating control; the nps and the nodes-to-depth are both recorded, because this step moves them in opposite directions
touches:    src/transposition_table.cpp, src/transposition_table.hpp, src/data_structures.hpp, src/bitboard.cpp make_move
excludes:   the static evaluation field, which exists since S094; bound-sign correctness, which is S106
decisions:  DEC-083
closes:
blocks:
paused_by:
done:

## What is there

`tt_entry_t` is **24 bytes** -- a full 64-bit key, a 32-bit score, a 32-bit
move, depth, eval, type, generation. The table is **direct-mapped**: one entry
per slot, `hash & index_mask`, no bucket. Replacement is depth-preferred inside
a generation and always-replace across generations. There is no prefetch and no
page hint.

Three consequences. 24 bytes divides neither 32 nor 64, so **entries straddle
cache lines**. Direct-mapped means a single colliding position evicts a deep
entry with no second chance. And the probe miss is fully exposed, because
nothing is issued early.

Measured 2026-08-19, `go movetime 2000` from the start position: 16 MB to
512 MB is **36 % fewer nodes and 21 % lower nps**. The node reduction outweighs
the nps loss, so a bigger table is net positive -- but a fifth of it is being
eaten by the layout, and the rating list runs at the size where that bites
hardest.

**Expect the replacement scheme itself to be worth little** -- the surveyed
consensus is 5 to 15 Elo for bucket-plus-aging over always-replace, and the one
part that is not optional is the aging. The layout, the alignment and the
prefetch are where the measured 21 % lives.
