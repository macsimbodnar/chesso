id:         S046
goal:       S030, S031 and S032 accepts discharge INV-6: search_bench identity or an SPRT
accepts:    each of the three step files' accepts requires identical tools/search_bench.py node counts and best moves against the preceding commit, or an SPRT where neutrality fails by design; S031 states the side_randoms[WHITE] ^ side_randoms[BLACK] construction if hash-identity is the intent, and its "carried in alongside other hash work" line is dropped or conditioned on proven bit-identity
touches:    adocs/plan_todo/S030_move_encoding_16_bit.md, adocs/plan_todo/S031_single_side_to_move_key.md, adocs/plan_todo/S032_pext_sliding_attacks.md
excludes:   implementing any of the three steps; any change to INV-6 itself
decisions:
closes:     2026-08-13_plan_review-F03
blocks:
paused_by:
done:

## What is there

All three accepts stop at "perft node counts unchanged". Perft exercises
generation and make/unmake only; it says nothing about the search tree, so a
step can complete to the letter while altering play with no measurement
anywhere — the discipline INV-6 exists to prevent, and the DEC-020 class of
contamination for everything measured after it.

Neutrality is not free in at least two of them. S030 removes the moving piece
from the encoding while move ordering reads it: `MOVE_PIECE`
(`src/data_structures.hpp:86`) indexes `history_moves` and `counter_moves` in
`score_move` (`src/evaluation.cpp:1092,1097`); any slip re-pointing those at
`squares[from]` changes ordering with perft green. S031 changes the zobrist
key values themselves — which changes transposition hits, which changes the
tree — unless the single key is defined as
`side_randoms[WHITE] ^ side_randoms[BLACK]` (two-xor sites:
`src/bitboard.cpp:878-880`, `1059-1061`, `1460-1462`; one-xor reference at
`1536-1537`). S042 spells the same fork out correctly; model the wording on
it.

Full evidence: 2026-08-13_plan_review-F03.
