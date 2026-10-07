id:         P01 (proposed; the S-id is allocated at adoption)
goal:       board_t carries a pawn key and one non-pawn key per side, kept by add_piece, remove_piece and move_piece and restored by unmake_move
accepts:    (1) at every make and unmake over test_invariants' corpus each key equals a recompute from the bitboards, and a planted drift is caught first (the precondition INV-4's oracle test uses); (2) load_FEN and every path that sets a position computes the keys; (3) `bench` and `tools/search_bench.py` identical to the parent, commit carries `No functional change` (INV-6) -- nothing reads the keys yet; (4) history_entry_t stays 16 bytes (DEC-208), or a decision records the measured nps cost of growing it; (5) interleaved hyperfine of `bench` against the parent under the noise floor, or the cost stated; (6) Tier 2: Debug self-play four rounds at 4+0.04 with no `Assertion` in the log, and gate_extra.sh green (DEC-141)
touches:    src/data_structures.hpp, src/bitboard.cpp, src/bitboard.hpp, tests/test_invariants.cpp, tests/test_engine.cpp, adocs/specs.md
excludes:   every consumer (P05, P06, P09, P15); a material key; new random numbers
closes:
paused_by:
author:
done:

## Why

P05 (pawn correction), P06 (non-pawn correction), P09 v2 (pawn history) and
P15 (pawn cache) index tables by these keys. None exists today: DEC-046's pawn
key left the tree with the pawn hash.

## Description it is implemented from

CPW, *Static Evaluation Correction History*: pawn locations by colour;
non-pawn pieces by colour, one table per side. Avalanche PR #75 prose: "an
incrementally maintained pawn Zobrist key". Each key is the xor of the
existing Zobrist piece-square keys (S203, `CHESSO_PROJECT_SEED`) over the
pawns, and over each side's non-pawn pieces including its king. No new random
numbers; side to move and castling are not part of them.

## Seeds

None. No constants.

## Measurement

Behaviour-neutral by construction; proved by `bench` identity. No match.

## From the record

`make_move` is the NNUE hook: every piece change goes through `add_piece`,
`remove_piece`, `move_piece`; `unmake_move` deliberately does not use them.
The keys are restored on unmake's own path, by xor of the move's deltas or by
storing them; the 16-byte history record (DEC-208) decides which, and the step
says which it chose and why.
