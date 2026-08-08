# Move generation speed-up plan

Working notes for making move generation faster. Every number below was
measured on this machine (arm64, `-O3 -DNDEBUG`) before the plan was written,
so the ranking reflects where the time actually is rather than where it looks
like it should be.

## Baseline

`./build/tests/bench_movegen`, full workload:

```
perft  (generate_moves + make_move + unmake_move)
  total   41812668 nodes   785.8 ms   53.21 Mnps

generate_moves  (no make_move, no unmake_move)
  total   12000000 calls   629.6 ms   19.06 Mcalls/s   673.5 Mmoves/s
```

Run-to-run agreement is about 0.4 %, and up to 3 % between invocations minutes
apart. **Treat anything under 3 % as noise.** The benchmark verifies node
counts before reporting, so a change that loses moves cannot look like a
speed-up.

## Where the time goes

Sampled profile of a 986 M node perft over three positions, with `is_attacked`
forced out of line so it could be attributed separately:

| part | self time |
|---|---|
| `make_move` body | ~58 % |
| `is_attacked`, the legality check after every move | ~17 % |
| `generate_moves` | ~11 % |
| `unmake_move` | ~9 % |
| perft driver | ~4 % |

**The generator itself is about 11 % of the cost.** Making it infinitely fast
buys 11 %. The work is in `make_move`.

Ablation builds, same tree, node counts identical unless noted:

| change | effect |
|---|---|
| all `board->hash ^=` removed | 17.8 s to 15.2 s, so Zobrist maintenance is **13.5 %** |
| one extra `board_t` copy added | +0.59 s, so the history copy costs **3.3 %** |
| repetition push removed | **~1 %** |
| leaves counted from the move list, no make/unmake | 17.8 s to 2.9 s, **6.1x** |

Only **1.5 %** of generated moves turn out illegal (1,000,088,160 pseudo-legal
leaf moves against 985,752,918 legal ones), so the make-then-roll-back path is
not the problem either.

Two intuitions to drop: copy-make is cheap here, and wasted work on illegal
moves is negligible.

### Sizes

```
board_t             144 B      history_entry_t     152 B
history_t           742 KB     repetition_t         39 KB
bb_tables_t        2307 KB     of which rook_attacks 2048 KB
game_t             3095 KB
```

## Results

Steps 1, 2, 4 and 3 are done, in that order. Every number below is the perft
total from `bench_movegen`, taken from three interleaved runs of the old and
new binaries in the same session, so machine drift cancels out.

| build | perft total | vs baseline |
|---|---|---|
| baseline | 790 ms | — |
| + step 1 | 770 ms | -2.6 % |
| + step 2 | 853 ms | +8.0 % |
| + step 4 | 781 ms | -1.2 % |
| + step 3 | **600 ms** | **-25.5 %** |

Two of the estimates above turned out to be wrong, and the ordering suffered
for it:

- **The `board_t` copy is not 3.3 %.** Step 2 grew `board_t` from 144 B to
  208 B and cost 8 %; padding it out by another 64 B cost a further 23 %. The
  copy was the dominant term in `make_move`, not a footnote, so step 2 could
  not pay for itself until step 4 removed the copy. Step 4 was moved ahead of
  step 3 for that reason.
- **The slim undo record is worth almost nothing on its own.** Steps 2 and 4
  together beat step 1 by 0.6 %, against the 12 % predicted. Replacing a
  144-byte memcpy undo with an xor undo is a wash: the branchy xor path costs
  about what the vectorised copy did. What step 4 bought was `history_t` at
  120 KB instead of 742 KB and, with step 2, O(1) `get_piece()`.
- **Step 3 beat its estimate**, 23 % against the 17 % attributed to
  `is_attacked`, because it also drops the make/unmake of the illegal 1.5 %
  and the branch on `make_move`'s return value.

`generate_moves` on its own got 19 % slower (628 ms to 749 ms), which is the
pin and check computation. It is bought back many times over by the caller.

Sizes now: `board_t` 208 B, `history_entry_t` 24 B, `history_t` 120 KB,
`bb_tables_t` 2371 KB (the extra 64 KB is `between[64][64]` and
`line[64][64]`).

## Step 1: guard the unconditional Zobrist work

Smallest change with a real number attached, so it goes first.

`src/bitboard.cpp:366-371` folds the en-passant key out and back in on every
single move, and `src/bitboard.cpp:426-429` does the same for castling rights.
Both run unconditionally: four table loads and four xors per move, even though
the en-passant square is `INVALID -> INVALID` and the castling rights are
unchanged for the overwhelming majority of moves.

Compare first, xor only when the value actually changes.

- Expected: a good part of the measured 13.5 %.
- Risk: low. The incremental hash is compared against `compute_full_hash()`
  over a whole move tree by `tests/test_engine.cpp`, so a mistake here fails
  the suite immediately.

## Step 2: piece-on-square array

Add `piece_t squares[64]` to `board_t`, maintained by the same xors that
already update the bitboards.

`src/bitboard.cpp:316` scans up to six bitboards to find out what was just
captured. A square array answers it in one load. The same array also removes
the scans in `capture_score()`, `get_piece()` and `generate_FEN()`.

- Expected: part of the unattributed ~40 % inside `make_move`. Measure it.
- Risk: medium. It is a second representation of the same state, so it can
  drift out of sync with the bitboards. Add an assertion under `!NDEBUG` that
  rebuilds it from the bitboards and compares, which the existing make/unmake
  tree walk will then exercise on every node.

## Step 3: legal move generation

The big one. Compute checkers, the pin rays and the king-danger squares once
per node, and emit only legal moves.

- Deletes the **17 %** `is_attacked` call after every move.
- Removes the roll-back path for the 1.5 % of moves that are illegal.
- `make_move` can stop returning `bool`, and every caller that tests it can
  drop the branch.
- Unlocks bulk counting in perft: at depth 1 the move count is the answer, no
  make/unmake needed.

**Read the 6.1x carefully. That number is perft only.** A search has to make
every move it searches, so it can never bulk-count. The honest gain for the
engine is the 17 % from dropping the per-move legality check. Optimising for
the 6.1x means optimising the benchmark instead of the engine.

- Risk: high. This is the change most likely to produce a subtly wrong move
  list. It is also the one the test suite is best equipped to catch: the perft
  columns in `tests/test_movegen.cpp` check captures, en passant, castles and
  promotions separately, and `tests/test_perft.cpp` goes deep.

## Step 4: slim undo record

Replace the 144-byte `board_t` copy at `src/bitboard.cpp:289` with a small
record holding the hash, the captured piece, the castling rights, the
en-passant square and the halfmove clock, and undo by xor.

- Expected: the copy itself is only 3.3 %, but `unmake_move` is 9 % and the
  pair is around 12 %. Also cuts `history_t` from 742 KB to roughly 40 KB.
- Risk: medium. `unmake_move` stops being a memcpy and starts being real code.
- Depends on step 2, which is what makes the captured piece known in O(1).

## Step 5: share the attack tables

`bb_tables_t` is 2307 KB and is embedded in every `game_t`, which is why
`sizeof(game_t)` is 3095 KB. The tables are read-only after
`initialize_game_const_data()`, so one shared instance would do.

- Expected: nothing in perft, where there is a single instance. It is why
  anything that copies a `game_t` moves 3 MB.
- Risk: low, but it touches every signature that currently takes
  `const bb_tables_t*`.

## Step 6: fancy magics

Per-square offsets into one shared table would take the attack tables from
2307 KB to roughly 860 KB.

**Measure before doing this.** The caches on this machine may already absorb
the 2 MB rook table, in which case the work buys nothing.

## Step 7: pawn generation as bulk shifts

Replace the per-pawn loop and its `is_promoting` branch with shifted bitboards
over the whole pawn set.

Bounded by the generator's 11 % share, so a few percent at best. Last on the
list for that reason.

## Not worth touching

The history copy on its own (3.3 %), the repetition push (~1 %), and the move
encoding. All measured, all too small to matter.

## How to verify each step

```
cmake --build build -j8
ctest -L fast                      # correctness, must stay green
./build/tests/bench_movegen        # speed, compare against the baseline above
```

For anything touching the generator or `make_move`, also run the deep perft
before calling it done:

```
ctest -L slow                      # tests/test_perft, minutes
```

Rules for this work:

- Node counts first, timings second. `bench_movegen` exits non-zero on a wrong
  count and prints nothing worth reading.
- Record the benchmark output before and after each step, one step at a time.
  Two changes at once and neither number means anything.
- A change under 3 % has not been shown to do anything on this machine.
```
./build/tests/bench_movegen > before.txt
# ... make the change ...
./build/tests/bench_movegen > after.txt
diff before.txt after.txt
```

## Reproducing the measurements

The profile and the ablations were run from a throwaway copy of `src/`, not
from the repository. To redo them: build a perft driver against the engine
with `-g -fno-omit-frame-pointer`, run `sample <pid> 8 1` while it works, and
read the "Sort by top of stack" section for self time. For the ablations,
patch the copied source, rebuild, and time the same fixed workload.
