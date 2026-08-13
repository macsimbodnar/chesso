id:         S031
goal:       one unconditional xor for the side-to-move zobrist key instead of two
accepts:    perft node counts unchanged; compute_full_hash() and swap_side() agree; the single key is defined as side_randoms[WHITE] ^ side_randoms[BLACK] so every hash is bit-identical to before; identical tools/search_bench.py node counts and best moves against the preceding commit (INV-6)
touches:    src/bitboard.cpp
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Estimate

**Under 1 %**, which is below anything this machine can resolve. Listed only
because it is a ten-minute change. With the key defined as
`side_randoms[WHITE] ^ side_randoms[BLACK]`, every hash value is bit-identical
to today's (the two-xor sites are `src/bitboard.cpp:878-880`, `1059-1061`,
`1460-1462`; the one-xor reference is `1536-1537`), so INV-6's first test --
search_bench identity -- is the whole proof and no SPRT is owed. Any other
construction changes the key values, transposition hits, and therefore the
tree, and costs an SPRT; it may only ride alongside other hash work if the
bit-identity above is proven first.
