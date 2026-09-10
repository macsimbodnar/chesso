id:         S208
goal:       the load boundary refuses a piece count the move buffer cannot hold and a pawn on a back rank, so `position fen` can no longer crash the shipping binary or index the passed-pawn table out of bounds
accepts:    `load_FEN()` in `src/bitboard.cpp` refuses a placement with more than 16 pieces of one colour and a placement with a pawn of either colour on rank 1 or rank 8, each with its own reason, and `position fen` reports the refusal on the UCI channel in the S176 shape (`info string refused [position fen] <fen>, <reason>`) with the board, history and moves left as they were; the F09 FEN (`QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1`) is observed to abort the Release binary with `stack smashing detected` **before** the fix and to be refused after; the two F10 FENs (`P7/8/8/8/8/8/8/K6k w - - 0 1`, `K6k/8/8/8/8/8/8/p7 b - - 0 1`) are observed to print `index 6 out of bounds` under `build-sanitize` before and to be refused after; red-first cases join `tests/test_audit_fen_semantics.cpp`, S161's file, one per class, plus the boundary cases that must still load (16 pieces a side, a pawn on rank 2 and rank 7); the bound behind `MAX_MOVES` is stated at its definition in `src/data_structures.hpp` as what it is -- 218 the proved legal maximum, 270 the buffer, 224 the largest count the audit's maximiser found under the 16-a-side rule, a search result and not a proof -- and `adocs/specs.md`'s position-input row names the third class beside S161's two; INV-6 discharged on identical `tools/search_bench.py` counts and best moves and an identical `bench` signature, because no legal position changes; `MANUAL.md` documents the two new refusals before `test_uci_surface` is refreshed (SURFACE)
touches:    src/bitboard.cpp, src/data_structures.hpp, tests/test_audit_fen_semantics.cpp, MANUAL.md, adocs/specs.md, tests/test_uci_surface.cpp (golden)
excludes:   a run-time bound inside `generate_moves()`'s hot loop, which would tax every node to guard an input the boundary can refuse for free; requiring exactly one king a side, because `EMPTY_POS` and the two "survivable" cases in `tests/test_search.cpp` are deliberately kingless or king-capturable debug inputs; the side not to move being in check, still unchecked as S161 left it; clamping the passed-pawn bucket inside `evaluate_pawns`, which would hide an invalid board instead of refusing it
decisions:  DEC-170
closes:     2026-09-10_adversarial-F09, 2026-09-10_adversarial-F10
blocks:
paused_by:
author:
done:

## Why this exists

Two high findings of `2026-09-10_adversarial`, both reachable from `position
fen` in the binary that ships and is measured, both guarded today by an
`assert` that `NDEBUG` compiles out of both gated builds.

**F09.** `MAX_MOVES` is 270, true of legal positions (the known maximum is
218), and every caller declares the buffer on the stack; `src/search.cpp`
`negamax_at` appends `generate_captures` and `generate_quiets` into the same
array. A placement `load_FEN()` accepts with 26 queens of one colour -- the F09 FEN --
generates 277 moves by the audit's maximiser, and the Release binary aborts with `*** stack smashing detected ***`,
exit 134. A near-miss corrupts the adjacent `scores[]` and `quiets_tried[]`
instead of aborting. Constrained to 16 pieces a side the same maximiser's best
was 224, which is why the 16-a-side rule is the fix. The S161 sanitiser repairs castling rights and en-passant
squares and says in its own comment that piece counts are "deliberately not
checked". They are a third class that corrupts.

**F10.** `evaluate_pawns` in `src/evaluation.cpp` computes the passed-pawn
bucket as `6 - (rank)` for white and `rank - 1` for black and reads
`passed_pawn_mg[bucket]`; the comment beside it says what keeps it in range is
that a pawn cannot stand on rank 1 or 8, and the Debug build asserts it. A pawn
on either rank arrives through `position fen`, the sanitizer build prints
`index 6 out of bounds for type 'int [6]'` at both colours' sites, the Release
binary answers `info score cp 418` and a move, and the collecting instantiation
writes one `int` past the caller's `int[2][6]`. A 20000-FEN fuzz under
ASan+UBSan found exactly this one report site.

Neither is reachable from legal play, no GUI sends either, and the audit rates
them high because they are crashes and out-of-bounds writes on the product
surface -- and because `position fen` is also how corpora,
`tools/pgn_to_positions`, `datagen` and any fuzzer reach the engine.

## Shape

The load boundary is the contract every downstream consumer assumes (S161's
words), so the refusal lands there and nowhere else. Counting is a loop over
`squares[]` after the placement is parsed, before `compute_full_hash()`,
beside S161's two clearings. Refuse rather than repair: a piece count cannot
be repaired without inventing a position, and a back-rank pawn has no legal
home.

Why 16 and not a pawn cap or a per-type cap: 16 a side is the rule of the game
and the bound the audit's maximiser was run under, and its result, 224, leaves
46 of headroom under 270. State that as the reasoning and the residual risk at
`MAX_MOVES`, so the next reader knows 270 is defended by a measurement and can
re-run it if the generator changes.

## Tests

`tests/test_audit_fen_semantics.cpp` is S161's file and the right home: one
case per class, each observed red on the unfixed tree -- the F09 case red by
the abort under a Release build cannot be a ctest red, so it is observed by
hand and recorded in the stamp, and the ctest case asserts the refusal --
plus the boundary loads (16 a side, pawns on ranks 2 and 7, the perft suite's
FENs all still loading). `tools/gate_extra.sh` stage 4 is the observation
instrument for F10's red and its green.

## Cost

Agent work, an hour or two; no run. Node-identical, no `Bench:` change, no
SPRT. The Debug self-play clause does not bind -- the generator is untouched.
