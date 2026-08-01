# Move generation — bug report

Review of `generate_moves`, `make_move`, `unmake_move`, `is_attacked` and
`is_position_repeated` in `src/bitboard.cpp`.

All bugs below existed on `bitboard` @ `74d35cb` ("Improvements"). Every one
marked FIXED has a fix in the working tree; the verification each rests on is
described in [Verification](#verification).

---

## Summary

| # | Bug | Severity | Status |
|---|---|---|---|
| 1 | Zobrist hash omits the "no en-passant" term | High | FIXED |
| 2 | `unmake_move` guards the wrong end of the stack | Medium | FIXED |
| 3 | White pawn push bounds check is always true | Medium | FIXED |
| 4 | OOB table read when a king bitboard is empty | Medium | FIXED |
| 5 | `is_position_repeated` scans the whole game history | Medium | FIXED |
| 6 | Opening-book moves are built without `double_push` / `en_passant` flags | High | OPEN |
| 7 | `game_t` is 147 MB | Low | OPEN |

---

## 1. Zobrist hash omits the "no en-passant" term — FIXED

**Where:** `make_move`, `src/bitboard.cpp`

**What:** `ep_randoms` has 65 entries — one per square plus one for
`INVALID_INDEX` (64), meaning "no en-passant square". `make_move` only folded
the random in and out when the square was valid:

```c
// Reset en-passant
if (board->en_passant != INVALID_INDEX) {
  board->hash ^= randoms->ep_randoms[board->en_passant];
  board->en_passant = INVALID_INDEX;
}

if (move.double_push) {
  board->en_passant = ...;
  board->hash ^= randoms->ep_randoms[board->en_passant];
}
```

`ep_randoms[INVALID_INDEX]` therefore never entered or left the hash. But the
other two places that touch the same term apply it *unconditionally*:

- `compute_full_hash()` — `key ^= ep_randoms[board->en_passant]`
- `set_en_passant()` — xors the old square out and the new one in, whatever
  they are

**Why it matters:** null move pruning calls `set_en_passant(game,
INVALID_INDEX)` (`search.cpp`). That folds `ep_randoms[64]` into the hash,
which `make_move` never does. Every position searched inside a null-move
subtree is therefore keyed into a *different key space* than the identical
position reached through ordinary moves, and the two can never share a
transposition table entry. It is a silent, permanent loss of TT hits in the
part of the search that exists to save work.

It also means an incrementally maintained hash never equals
`compute_full_hash()` of the same position, so any future consistency check,
book probe or persistent hash keyed off `board->hash` would be wrong.

**Failure scenario:** search any position deep enough to trigger null move
pruning. Position P reached normally is stored under key `k`; the same P
reached under a null move is stored under `k ^ ep_randoms[64]`. Both are
searched from scratch.

**Fix:** make the update symmetric and unconditional.

```c
board->hash ^= randoms->ep_randoms[board->en_passant];

board->en_passant = move.double_push ? <new square> : INVALID_INDEX;

board->hash ^= randoms->ep_randoms[board->en_passant];
```

**Evidence:** a validator that recomputes the hash from scratch after every
`make_move` reports **1,469,900 mismatches over 9,167,180 states** on `74d35cb`,
and **0** after the fix.

---

## 2. `unmake_move` guards the wrong end of the stack — FIXED

**Where:** `unmake_move`, `src/bitboard.cpp:454`

**What:**

```c
if (game->history.size < HISTORY_MAX_SIZE) {
  game->history.size--;
  ...
}
```

`size` is a `size_t`. The dangerous case is an `unmake_move` with nothing to
undo, and the guard does not cover it: `0 < HISTORY_MAX_SIZE` is true, `size--`
wraps to `SIZE_MAX`, and the next line indexes `entries[SIZE_MAX]`. The bound
it does check — overflowing the top of the array — is already handled by the
`assert` in `make_move`, and only in debug builds.

**Failure scenario:** any unbalanced `unmake_move`. The existing call sites are
balanced, so this is latent rather than live, but it is a wild read of ~152
bytes and a `game->board` overwritten with garbage.

**Fix:** `if (game->history.size > 0)`.

---

## 3. White pawn push bounds check is always true — FIXED

**Where:** `generate_moves`, white pawn branch

**What:**

```c
to = from - 8;
if (!(to < a8) && !GET_BIT(board->occupancies[BOTH], to)) {
```

`to` is an `index_t` (`uint8_t`) and `a8` is `0`. `to < 0` is never true for an
unsigned value, so `!(to < a8)` is a constant `true`. The check was written to
catch `from - 8` underflowing for a pawn on the 8th rank, and it cannot. For a
pawn on a8..h8, `to` wraps to 248..255 and `GET_BIT` shifts by more than 63 —
undefined behaviour.

The black equivalent, `!(to > h1)` with `h1 == 63`, does work.

**Failure scenario:** a white pawn on the 8th rank. Unreachable through legal
play, but `load_FEN` does not validate placement, so any caller feeding a
hand-written or corrupted FEN reaches it.

**Fix:** mask both back ranks off the pawn bitboard once, before the loop,
instead of bounds-checking every push:

```c
bb_t bboard = my_bitboards[0] & 0x00FFFFFFFFFFFF00ULL;
```

This covers both colours and removes a per-pawn branch.

---

## 4. Out-of-bounds table read when a king bitboard is empty — FIXED

**Where:** `make_move` legality check, `src/bitboard.cpp:435`

**What:** `get_lsb_index(0)` returns 64 (`std::countr_zero` of zero). The
legality check passed that straight into `is_attacked`:

```c
if (is_attacked(tables, board,
                (board->active_color == WHITE)
                    ? get_lsb_index(board->bitboards[B_KING])
                    : get_lsb_index(board->bitboards[W_KING]),
                board->active_color)) {
```

`is_attacked` then indexes `pawn_attacks[..][64]`, `knight_attacks[64]`,
`bishop_masks[64]` and so on — all sized 64.

**Failure scenario:** any position without the moving side's king. Reachable
from a FEN in tests, analysis or a debug tool.

**Fix:** skip the check when the king bitboard is empty, and `assert(index <
64)` inside `is_attacked` so a bad square is caught in debug builds rather than
read past the table.

---

## 5. `is_position_repeated` scans the whole game history — FIXED

**Where:** `is_position_repeated`, called from `negamax` at every node

**What:**

```c
for (size_t i = 0; i < rep->size; ++i) {
  if (hash == rep->entries[i]) { return true; }
}
```

Two problems:

1. **It ignores the halfmove clock.** A position cannot recur across an
   irreversible move (capture or pawn move), so everything before the last one
   is dead weight. The scan length grows with the length of the game, and it
   runs at every search node.
2. **It checks both parities.** Side to move is part of the hash, so entries an
   odd number of plies back can never match. Half the comparisons are provably
   useless.

**Fix:** bound the scan by `halfmove_clock` and step back two plies at a time.
The signature changed to take the board, since the clock lives there:

```c
bool is_position_repeated(const repetition_t* rep, const board_t* board);
```

**Note:** this is a behaviour change as well as a speed one. The old scan could
return a draw for a position it had no business matching (only via hash
collision, but the window it searched was the entire game). Measured node
counts moved by <0.1% on the benchmark positions, with identical best moves and
scores.

---

## 6. Opening-book moves are built without `double_push` / `en_passant` flags — OPEN

**Where:** `get_book_moves_for_key`, `src/openings.cpp:611`

**What:** book moves are constructed directly rather than matched against
`generate_moves` output:

```c
const move_t m =
    NEW_MOVE(from, to, piece, promoted_to, (target != EMPTY), 0, 0, 0);
```

The `double_push` and `en_passant` flags are hard-coded to `0`. Two
consequences:

- A book pawn double push does not set the en-passant square. The reply loses a
  legal en-passant capture, and the position hash is wrong for that ply.
- A book en-passant capture is built as a quiet move: `target` is `EMPTY` (the
  captured pawn is not on the destination square), so `capture` is `0` and
  `en_passant` is `0`. `make_move` leaves the captured pawn on the board.

**Failure scenario:** any book line containing an en-passant capture corrupts
the position. Book lines with double pushes — i.e. most of them — mis-hash the
resulting position.

**Suggested fix:** resolve book moves against `generate_moves` the way
`try_move` (`chesso.cpp:400`) already does, and use the generated `move_t`
rather than a reconstructed one. That also makes the `fix_weirdo_castling` call
redundant.

**Note:** this is outside move generation proper and has not been touched.

---

## 7. `game_t` is 147 MB — OPEN

**Where:** `data_structures.hpp`

`HISTORY_MAX_SIZE` is 1,000,000 and `sizeof(history_entry_t)` is 152, so
`history_t` alone is **145 MB**; `game_t` totals **147 MB**, and
`transposition_table_t` adds another **96 MB**. `chesso.cpp` holds both as
static globals.

The history only ever needs to span the moves of the game plus `MAX_PLY` (100)
of search depth. A few thousand entries would do. Nothing is broken by this
today — it is address space and BSS, and search touches a small window of it —
but it makes the engine unusable in a memory-constrained environment and buries
the real working set.

---

## Verification

Three harnesses, all against `74d35cb` as the reference:

**Move-list equivalence.** The original `generate_moves` and `is_attacked` were
compiled alongside the rewritten ones and compared across a full game-tree
walk: **15,202,322 positions**, move lists identical *including order*, and
`is_attacked` identical on all 64 squares for both colours. 0 mismatches. The
`generate_moves` rewrite is therefore behaviour-preserving — it changes no
search result, only speed.

**State consistency.** After every `make_move`: occupancies recomputed from the
twelve piece bitboards, hash recomputed by `compute_full_hash`, piece bitboards
checked pairwise for overlap, and the board compared byte-for-byte against its
pre-move snapshot after `unmake_move`. **9,167,180 states**, 0 errors after the
fixes (1,469,900 hash errors before — that is bug #1).

**Perft.** Six standard positions (start, Kiwipete, positions 3–6) at depths
4–6, all matching published node counts. `test_chesso` (24,227 assertions) and
`test_openings` pass. Builds clean under the project's `-Wall -Wextra -Werror`.

**Gap:** the full `tests/test_perft` suite (`talkchess_perft.json` +
`perft.json`) has **not** been run to completion against these changes. It
should be, before this is trusted.

### Measured effect

| | before | after | |
|---|---|---|---|
| Perft | 45.9 Mnps | 53.5 Mnps | +17% |
| Search (8 positions, depth 7) | 2,277 knps | 2,687 knps | +18% |

Same best move and same score on all 8 search positions.

---

## Performance changes made alongside the fixes

Not bugs, but they are in the same diff:

- **`is_attacked` did 4 magic lookups where 2 suffice.** It computed bishop
  attacks, then rook attacks, then called `get_queen_attacks` — which recomputes
  *both*. The queen shares the bishop and rook rays, so it can be tested against
  the two sets already in hand. Leapers now run first as cheap rejects, and the
  per-piece ternaries collapsed to one base pointer (a side's six bitboards are
  contiguous).
- **`make_move` rebuilt the occupancies from scratch on every move** —
  `memset` plus twelve loads and ORs. Now maintained incrementally with a
  handful of xors per path.
- **`generate_moves` looped all twelve piece types**, spending roughly twelve
  branches per iteration to skip the opponent's six. Now indexes only the side
  to move. The capture flag is derived by shift instead of an unpredictable
  branch, bit clearing uses `x &= x - 1`, the en-passant square is hoisted into
  a precomputed mask, and the castling `is_attacked(e1)` test — previously run
  twice when both rights were available — is hoisted.
- **`count_bits`, `get_lsb_index` and the `get_*_attacks` lookups moved to the
  header as `inline`.** They are one to three instructions each and sit in the
  innermost loops, but lived in `bitboard.cpp` with no LTO configured — so
  `evaluation.cpp`'s 49 call sites were paying a real function call apiece.
