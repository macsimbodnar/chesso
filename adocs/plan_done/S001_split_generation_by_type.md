id:         S001
goal:       generate_captures and generate_quiets partition generate_moves, so quiescence stops generating what it discards
accepts:    the two lists together are the same multiset as generate_moves with no overlap, over a three-ply tree from every test FEN; search node counts and best moves unchanged
touches:    src/bitboard.cpp generate_moves_body<Color, Constrained, Type>, src/search.cpp quiescence
excludes:   letting non-capturing promotions through the quiescence filter, which is a search change and needs games
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 1a42999. README and MANUAL checked at adoption, not when this shipped.

## What shipped

One body templated on `<Color, Constrained, Type>` with a `type_mask`:
`opp_occupancy` for captures, `~all_occupancy` for quiets, `free_squares` for
everything. Castling is quiet; promotions and en passant are captures.

## Measurement

Real search time at fixed depth, three positions:

```
                    before    after
midgame             4.257s   3.655s
kiwipete            3.325s   3.008s
tactical            2.094s   1.930s
total               9.676s   8.593s    -11.5 %
```

Node counts identical -- 46622276, 36703759, 26765104 -- and same best moves.
That is the proof it is a pure speed-up: same tree, explored faster. Captures
cost 36 % of a full generation, so a node that stops after them saves 64 %.

Object code for `bitboard.cpp` 68 KB to 90 KB for twelve instantiations. Perft
unchanged within resolution.

## Left on the table

`generate_captures()` emits promotions that capture nothing. The filter in
`quiescence()` still drops them, so this change altered nothing there. Letting
them through is very likely an improvement and is a search change.
