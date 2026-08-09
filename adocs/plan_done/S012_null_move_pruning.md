id:         S012
goal:       give the opponent a free move and prune when the result still fails high
accepts:    an SPRT passes; the reduced search never falls to depth 0; mate tests still find their mates
touches:    src/search.cpp negamax, make_null_move/unmake_null_move
excludes:   verification search at high depth
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 1845a9c. README and MANUAL checked at adoption, not when this shipped.

## Measurement

3.8x at fixed depth. SPRT over S011 and S012 together: **+132.4 Elo, passed.**

## Bug found and fixed inside the step

NMP hid a mate in 2 at depth 4: the reduced search fell to depth 0, which is
pure quiescence, and quiescence cannot see a mate. Fixed by requiring
`depth - 1 - null_reduction >= 1`. The mate test is what caught it, which is the
argument for keeping mate positions in the fast suite.

Zugzwang guard: disabled in the endgame using the phase from S009, and never at
a node that is in check.

## Measurement hazard recorded here because it cost an hour

An SPRT was contaminated by rebuilding `build/` while the match was running --
fastchess spawns the engine per game, so later games used a different build and
the +301 Elo it reported meant nothing. `fastchess.sh` now snapshots the
candidate binary with `mktemp` and a `trap`. See DEC-005.
