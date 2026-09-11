id:         S042
goal:       set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
accepts:    perft counts are identical over the existing suite; two move orders reaching the same position produce the same hash and the same FEN; generate_FEN's fourth field agrees with Stockfish over a corpus sample; **re-scoped by DEC-187 as a bug fix**: a red-first case pins the reproduction in the section below -- history A scores the draw at depth 4 after the fix, observed red on HEAD at -313 -- and a second case asserts the incremental key equals the recomputed full hash after a double push with and without a capturing pawn; the key moves in every place it is built, `make_move`, `load_FEN` and the full-hash recomputation; fastchess's "PV continues after threefold repetition" count over a 1500-game self-match at the S198 regime is recorded before (236 in S219's pass 1) and after, and falls to zero or the residual is named as a second cause; the tree changes, so the commit carries `Bench:` and the Debug binary self-plays four rounds at 4+0.04 with `Assertion` grepped (DEC-141); one `--nonreg` SPRT against the parent decides it, pre-registered per DEC-143 with its worst-case games and naming this defect per DEC-171; `MANUAL.md` and `adocs/specs.md` state the en passant convention where they describe the FEN or the repetition rule
touches:    src/bitboard.cpp make_move_impl, load_FEN, set_en_passant, compute_full_hash, tests/test_audit_fen_semantics.cpp
excludes:   any other zobrist change, including the single side-to-move key that was S031 -- retired, no successor step, `adocs/plan.md` "under 1 % by its own file, below every instrument here"
decisions:  DEC-187, DEC-171, DEC-173, DEC-141, DEC-143
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

## A correctness bug after all -- 2026-09-11, DEC-187

The section this replaces was headed "Not a correctness bug" and argued that
repetition detection could not be hurt because a position carrying an en
passant square has `halfmove_clock == 0` and the lookback is bounded by that
clock. The lookback is `back <= limit` and the tainted entry sits at exactly
`back == halfmove_clock`, the last iteration: the argument was off by one, and
the 2026-08-13 audit's F08 triage ("Efficiency, not correctness") inherited it.

**What S219's first match showed, 2026-09-11.** 236 fastchess warnings "PV
continues after threefold repetition" in 1500 self-play games. A subagent
replayed every one with python-chess: **236 of 236 are genuine threefolds**
at the flagged node, the earlier occurrences before the root (184) or at the
root (52), never inside the line. In every case the oldest occurrence is the
position immediately after a double pawn push. `src/bitboard.cpp` `make_move` sets
`en_passant` and xors `ep_randoms` on every double push with no test for a
capturing pawn, so that occurrence hashes differently from the identical
position reached otherwise, and `src/bitboard.cpp` `classify_repetition`'s key compare
skips it. The DEC-173 logic is right; the key it compares is not. **52 of the
236 publish a non-zero score for a drawn line**; 184 still read 0 because the
search meets a later repetition one ply deeper -- right by accident. Depths 2
to 27.

**Minimal reproduction, depth 4, White a rook down.** Same position, same
FIDE history (python-chess `can_claim_threefold_repetition()` is True after
the last move in both):

```
A  position fen r5k1/8/8/8/8/8/6PP/6K1 w - - 0 1 moves g2g4 a8b8 g1f1 b8a8 f1g1 a8b8 g1f1 b8a8
   go depth 4 -> score -313  pv g4g5 a8a2 h2h4 g8g7
B  position fen r5k1/8/8/8/6P1/8/7P/6K1 b - - 0 1 moves a8b8 g1f1 b8a8 f1g1 a8b8 g1f1 b8a8
   go depth 6 -> score 0     pv f1g1
```

A alone is the red-first case: after the fix it scores the draw like B.

**What the fix must cover.** The key changes in every place it is built --
`make_move`, `load_FEN` and the full-hash recomputation -- as
`2026-09-04_plan_review-F08` already said, and one test asserts the
incremental key equals the recomputed one after a double push with and
without a capturing pawn. `set_en_passant` in `touches:` has zero callers and
S213 deletes it; whichever step lands first, the other adjusts. The tree
changes, so the commit carries `Bench:`, the Debug binary self-plays four
rounds (DEC-141), and one `--nonreg` SPRT decides the step, pre-registered
per DEC-143 and naming this defect per DEC-171. fastchess's warning count is
the instrument: recorded before and after over the same games, it falls to
zero or the residual is a second cause.

What the old section said about transpositions and FENs still holds and is
now the smaller half: 4.6 % of quiet positions get a key the same position
reached by another move order cannot match, and `generate_FEN`'s fourth field
disagrees with Stockfish's convention on every double push.

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
