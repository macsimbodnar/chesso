id:         S019
goal:       evaluation terms for the phase the error analysis says costs most
accepts:    an SPRT against the preceding commit returns a verdict; the specific position class S016 found -- king and pawn endgames scored 1.2 to 1.7 pawns too high -- is measurably closer to Stockfish afterwards
touches:    src/eval_tables.hpp, src/evaluation.cpp
excludes:   middlegame terms, which are S027
decisions:
closes:
blocks:
paused_by:
done:

## The measured defect

Stockfish at depth 18 over a full game: chesso held +1.5 to +1.9 for ten
consecutive moves in a position Stockfish valued at +0.25. That is the largest
confirmed evaluation error found so far, it is in the endgame, and it was found
by a tool rather than by reading the game.

Likely candidates, in the order the field reports them for endgames: passed
pawns tapered by rank, king activity and king-to-passer distance, rook behind
the passer. `passed_w_pawns_masks[]` and `passed_b_pawns_masks[]` already exist
in `bb_tables.hpp`, unused.

**Confirm against S018 before writing any of them.** One game says endgame; the
distribution may say otherwise, and this step's content changes if it does.
