id:         S058
goal:       S030's neutrality remedy separates sites keyed on the move from sites keyed on prev_move, and names the write site
accepts:    S030's hazard paragraph says squares[MOVE_FROM(move)] only for the sites that key on the move being scored, gives the sites keyed on prev_move their own remedy with the promotion case stated, and names the write at src/search.cpp:507 alongside the read at src/evaluation.cpp:1094; the paragraph states that the site set is whatever grep -n MOVE_PIECE src/ returns when the step starts, because S023 and S024 add to it; no sentence in S030 still says every MOVE_PIECE site takes the from-square
touches:    adocs/plan_todo/S030_move_encoding_16_bit.md
excludes:   shrinking move_t, which is S030 itself; any edit to src/, including the sites named here
decisions:
closes:     2026-08-13_plan_review.2-F03
blocks:
paused_by:
done:

## What is there

`S030_move_encoding_16_bit.md`, "Estimate and risk": "`MOVE_PIECE`
(`src/data_structures.hpp:86`) indexes `history_moves` and `counter_moves` in
`score_move` (`src/evaluation.cpp:1092,1097`). Every such site must be
re-pointed at `squares[from]`."

`squares[MOVE_FROM(move)]` is right for the sites that index the move being
scored and wrong for the two that index `prev_move`, which has already been
played by the time they run. The five sites at HEAD:

```
src/evaluation.cpp:1064   piece_values_abs[MOVE_PIECE(move)]                 keyed on move
src/evaluation.cpp:1094   counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(...)] keyed on prev_move
src/evaluation.cpp:1099   history_moves[MOVE_PIECE(move)][MOVE_TO(move)]     keyed on move
src/search.cpp:503        history_moves[MOVE_PIECE(moves[i])][...]           keyed on move
src/search.cpp:507        counter_moves[MOVE_PIECE(prev_move)][...] = ...    keyed on prev_move, a write
```

`prev_move` is the move that led to the current node, so its from-square is
empty there and `board.squares[MOVE_FROM(prev_move)]` is `EMPTY`
(`src/data_structures.hpp:154-168`, the thirteenth enumerator of `piece_t`,
value 12). Both countermove tables are twelve rows wide —
`src/data_structures.hpp:425` `int history_moves[12][64];` and `:434` `move_t
counter_moves[12][64];` — so following the sentence as written indexes row 12
of a 12-row array, on the read and on the write. The write is at a site the
step does not mention at all, since it says "in `score_move`".

`board.squares[MOVE_TO(prev_move)]` recovers the piece except on a promotion,
where that square holds the promoted piece rather than the pawn: the countermove
key changes and the ordering with it, silently, with perft green — the failure
mode the paragraph exists to warn about.

The site set also grows before S030 runs. S024 and S023 both sit ahead of it in
the order; S024's table is keyed on the previous move's piece and target crossed
with the current move's, which is the case this remedy gets wrong, and S023 adds
a `[piece][to][victim]` table.

S030's own INV-6 clause would very likely catch a changed tree — that is what
S046 put it there for — so the damage is a wasted cycle rather than a silent
regression. The hazard note is the part of the file meant to prevent the cycle.

Full evidence: 2026-08-13_plan_review.2-F03.
