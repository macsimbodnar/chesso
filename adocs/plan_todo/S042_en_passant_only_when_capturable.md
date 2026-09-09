id:         S042
goal:       set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
accepts:    perft counts are identical over the existing suite; two move orders reaching the same position produce the same hash and the same FEN; generate_FEN's fourth field agrees with Stockfish over a corpus sample; an SPRT against the preceding commit returns a verdict
touches:    src/bitboard.cpp make_move_impl, load_FEN, set_en_passant, compute_full_hash, tests/test_audit_fen_semantics.cpp
excludes:   any other zobrist change, including the single side-to-move key that was S031 -- retired, no successor step, `adocs/plan.md` "under 1 % by its own file, below every instrument here"
decisions:
closes:     2026-08-13_adversarial-F08
blocks:
paused_by:
done:

## What happens now

`src/bitboard.cpp` `make_move_impl` sets `new_en_passant` after every double
push with no test for an enemy pawn that could capture onto it. Two move orders
reaching the same position hash differently:

```
order 1  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2
         hash 11800988595472028807
order 2  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq d3 0 2
         hash 2736264005390963272
placement equal: yes
hashes equal:    no
```

(1.d4 d5 2.c4 against 1.c4 d5 2.d4.) Over 200000 corpus positions, 9550 carry an
en passant square and only 392 of those have a capture available — so on 4.6 %
of all positions the square is one no pawn can use.

## Not a correctness bug

Move generation is unaffected: the audit compared legal moves against Stockfish
over 3000 positions and found zero disagreements. Repetition detection cannot be
hurt either, because a position carrying an en passant square always has
`halfmove_clock == 0` and `is_position_repeated`'s lookback is bounded by that
clock.

What is lost is transpositions. 4.6 % of quiet positions get a key that the same
position reached by another move order cannot match, so the transposition table
misses on entries it holds.

The emitted FENs also disagree with the convention Stockfish and most tooling
use, which is how this became visible — `generate_FEN` against Stockfish's `d`
output differs in the fourth field on every double push.

## Shape

**The rule, written once and applied at every site that decides the square
(S184, F08).** An en-passant square is kept only when a pawn of the side to
move stands on a square from which it attacks the target -- the generator's own
candidate test, `tables->pawn_attacks[opponent][board->en_passant]` masked with
the mover's pawns in `generate_moves_body`. Pseudo-legal: pins are not
examined, as in the generator before its own legality check, and as X-FEN
specifies ("only specified after a double pawn push was made beside an opponent
pawn that might capture en passant **if legal**",
https://www.chessprogramming.org/Forsyth-Edwards_Notation). The owner picked
this reading over the fully legal one on 2026-09-08, so the Stockfish
comparison in `accepts:` runs against python-chess `en_passant='xfen'` and not
its `'legal'` default (https://python-chess.readthedocs.io/en/latest/core.html).

**It is not one site, and that is what F08 found.** The key is built in more
than one place and the places must agree; change `make_move_impl` alone and a
position reached by moves drops a non-capturable square while the same position
loaded from a FEN keeps it -- the same transposition mismatch this step exists
to remove, moved from move order to input path, and every SPRT game starts from
`position fen`. From grep over `src/bitboard.cpp` at 2026-09-08:

| function | what it does | rule applies? |
|---|---|---|
| `make_move_impl` | sets `new_en_passant` on `move.double_push`, xors `ep_randoms` out and in | **yes** -- one `pawn_attacks` lookup at the point where the square is currently set unconditionally |
| `load_FEN` | parses field 4; the S161 sanitiser keeps the square when the target is empty and the victim stands behind it -- it tests the victim's presence, not whether any pawn can capture -- then keys with `compute_full_hash` | **yes** -- the `supported` test gains the capturer term |
| `set_en_passant` | sets any square and re-keys; declared in `src/bitboard.hpp`, **no caller anywhere in `src/`, `tests/` or `tools/`** | **yes if it is kept**; whether to delete it instead is this step's call, since deleting is a `src/` change |
| `compute_full_hash` | xors `ep_randoms[board->en_passant]` unconditionally, `INVALID_INDEX` included | no -- a consumer, and it must keep agreeing with the incremental key |
| `make_null_move`, `unmake_null_move`, `unmake_move_impl` | clear or restore through the history entry | no -- consumers |
| `generate_moves_body` | the capture from `pawn_attacks[opponent][en_passant]` masked with the mover's pawns | no -- it is the rule's own expression |
| `generate_FEN`, `cleanup_board` | prints field 4; resets to `INVALID_INDEX` | no |

The four S161 cases in `tests/test_audit_fen_semantics.cpp` -- "a real ep
square survives and is playable", "an ep square on the mover's own rank goes",
"an occupied ep target square goes", "a cleared field is cleared in the hash"
-- all move through the `load_FEN` change and are where its red-first evidence
comes from.

## How it is measured

Perft counts stay identical, because the legal moves do not change. The search
tree does not stay identical, because the keys do. That is behaviour-neutral by
INV-6's first test and not by its second, so it takes an SPRT rather than a node
count comparison.
