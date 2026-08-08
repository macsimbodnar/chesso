# Move generation plan

Supersedes the previous plan, whose steps are all done or judged not worth
doing. Every number here was measured on this machine (arm64, Apple M1,
`-O3 -DNDEBUG`) at commit `95eefa5`, or is labelled as an estimate.

## Baseline

`./build/tests/bench_movegen`:

```
perft  (generate_moves + make_move + unmake_move)
  total   41812668 nodes   597.7 ms   69.95 Mnps

generate_moves  (no make_move, no unmake_move)
  total   12000000 calls   744.1 ms   16.13 Mcalls/s   478.4 Mmoves/s
```

Use `hyperfine` for before/after, not the bench's own sweep, and always give it
both binaries so the runs interleave:

```
hyperfine --warmup 1 --runs 10 './bench_before -r 1' './bench_after -r 1'
```

## Where the time actually is

Two `sample` runs of 10 s each, taken 3 s into a `-r 40` run so the window sits
entirely inside the perft phase. The two runs agree to within 0.5 points.

| part | self time |
|---|---|
| `make_move` | **47 %** |
| `unmake_move` | **26 %** |
| `generate_moves` | 20 % |
| perft driver | 6 % |
| `is_attacked` | 0.1 % |

**Correction to an earlier measurement.** A profile taken during this work
reported `generate_moves` at 41 % and `make_move` at 35 %. That sample was taken
with `-r 12`, where the perft phase lasts about 7 s, so a 10 s window starting
at 2 s spilled into the generator-only phase of the benchmark and inflated it.
The table above is the corrected one, and it reverses the conclusion: **the
generator is the smallest of the three, and make/unmake is 73 % of the work.**

Any future profile must use a large enough `-r` that the sampling window cannot
leave the phase being measured.

## Ordering

The list is ordered by expected gain toward engine strength, so that speed work
and search work sit on one scale. The conversion used is the usual rule of
thumb, **about 60 Elo per doubling of nps**, i.e. `Elo ~= 60 * log2(speedup)`.
A 25 % speed-up is therefore worth roughly 19 Elo. That is the only honest way
to rank a percentage against an Elo figure.

Estimates in this plan are hypotheses. The previous plan got two of its
estimates badly wrong in both directions, so **re-read this ordering after every
step**; the next item may no longer be the next item.

---

## 1. Staged move generation

Generate the transposition-table move first, then captures, and only generate
the quiets if no capture produced a cutoff. Most nodes fail high on one of the
first few captures and never need the 30-odd quiet moves at all.

This is the only item on the list that is worth more than every speed item
combined, and **perft cannot see it**. `bench_movegen` will not move. It has to
be measured in games.

- Expected: **30-50 Elo.** Engines report 39 Elo for staged generation and about
  50 Elo for staged generation together with aspiration windows.
- Also removes work from `generate_moves`, which is why it belongs above the
  micro-optimisations rather than below them.
- Risk: medium. The staging interacts with move ordering, and a bug shows up as
  a strength regression rather than a wrong node count. Perft must keep passing
  through the unstaged path.
- Verify: `fastchess` SPRT against the current build. Nothing else will tell you.

## 2. Template on colour — DONE, -12.1 %

Applied one function at a time, each measured against the previous, three
interleaved rounds each (perft total, best of five internal sweeps):

| step | before | after | gain |
|---|---|---|---|
| `make_move` | 552.5 ms | 517.1 ms | -6.4 % |
| `unmake_move` | 517.1 ms | 491.9 ms | -4.7 % |
| `generate_moves` | 491.9 ms | 485.5 ms | -1.3 % |
| **total** | **552.5 ms** | **485.5 ms** | **-12.1 %** |

75.5 to 86.0 Mnps. `generate_moves` on its own went 754 ms to 674 ms, -10.6 %.
The gains track the profile almost exactly, which is the first time in this
project an estimate has landed where it was aimed.

The transformation is smaller than it looks. Colour becomes a template
parameter and the body keeps its `(us == WHITE) ? a : b` expressions verbatim -
they fold on their own once `us` is `constexpr`. No logic moved.

Object code for `bitboard.cpp` is 61 KB with both instantiations. No I-cache
regression is visible at this size.

Original entry follows.



Every one of these functions branches on `us == WHITE` repeatedly, at runtime,
on a value that is constant for the whole call. `board.cpp` has these ternaries
in the push direction, the promotion maps, the piece bases, the en-passant
offsets and the castling squares.

Make colour a template parameter and dispatch once at the top:

```cpp
template <color_t Us> bool make_move_impl(game_t*, move_t);
bool make_move(game_t* g, move_t m)
{
  return g->board.active_color == WHITE ? make_move_impl<WHITE>(g, m)
                                        : make_move_impl<BLACK>(g, m);
}
```

- Expected: **5-15 %**, so roughly 4-12 Elo. It applies to 93 % of the measured
  time, which is what puts it above everything else in this half of the list.
- Risk: low. Pure mechanical transformation, no logic changes, and the perft
  node counts catch any slip immediately.
- Cost: the two instantiations roughly double the object code for these
  functions. Watch for an I-cache regression — measure, do not assume.

## 3. Delete the repetition stack — DONE, -5.5 %

Measured with `hyperfine`, 12 interleaved runs, sigma 0.013 s:
**2.036 s to 1.925 s, 1.06x.** The bench's own perft total went 608 ms to
554 ms, 69.0 to 75.5 Mnps. `history_entry_t` is 16 bytes, `history_t` 80 KB,
`game_t` 2515 KB.

The estimate below said 1-3 %. It was low, for the same reason the old plan's
copy estimate was low: this codebase is more sensitive to the size of what
make/unmake touches than any of these estimates assume. Treat that as the
standing prior for the remaining items.

Original entry follows.



`repetition_t` is 39 KB of hashes that are already stored elsewhere.
`make_move` writes the same value twice:

```cpp
history_entry->hash = board->hash;                       // src/bitboard.cpp
game->repetitions.entries[game->repetitions.size++] = old_hash;  // same value
```

`repetitions.size` also moves in lockstep with `history.size` - one push per
`make_move`, both reset together in `cleanup_board()` - so
`history_entry_t::repetition_size` is a third copy of a number that is already
known.

Drop both. `is_position_repeated()` walks `history.entries[i].hash` instead.
`history_entry_t` goes from 24 bytes to 16.

- Expected: **1-3 %.** One 8-byte store removed from every `make_move`, one load
  from every `unmake_move`, and the entry crosses back under a 16-byte
  boundary. Today's measurements showed this codebase is unusually sensitive to
  the size of what make/unmake touches.
- Risk: low, but `is_position_repeated()` changes from a packed array walk to a
  strided one. It runs once per search node rather than once per move, so the
  trade should be positive; confirm it with `test_engine`'s repetition cases.
- Also removes 39 KB from `game_t`.

## 4. Maintain `occupancies[BOTH]` incrementally — TRIED, REJECTED, +2.0 %

Implemented and reverted. Seven interleaved rounds, and the sign never changed:

```
with recompute (kept)      475.0  475.2  475.3  476.0 ms
with running delta         484.5  484.9  485.3  485.4 ms
```

The version that "saves" memory traffic is **2.0 % slower**. The delta was
accumulated in a register through the whole function and applied once as
`occupancies[BOTH] ^= both_delta`.

Why the intuition was wrong: the existing

```cpp
board->occupancies[BOTH] = board->occupancies[WHITE] | board->occupancies[BLACK];
```

reads two words that were written a few instructions earlier on the same cache
line, so both loads are store-forwarded and effectively free, and the statement
depends on nothing else - the scheduler can place it anywhere. The accumulator
replaces that with a serial dependency chain threaded through every branch in
the function, ending in a read-modify-write that cannot start until the chain
resolves. Fewer memory operations, worse schedule.

Do not retry this without a different shape. Anything that lengthens the
dependency chain through `make_move` is suspect on this core.

Original entry follows.



Both `make_move` and `unmake_move` end with

```cpp
board->occupancies[BOTH] = board->occupancies[WHITE] | board->occupancies[BLACK];
```

Two loads, an or, and a store, on every move in both directions. Every mask
needed to update it incrementally has already been computed a few lines above.

- Expected: **1-3 %.**
- Risk: low. The existing occupancy assertions under `!NDEBUG` already check
  this invariant on every node.

## 5. A no-check, no-pin fast path — DONE, -1.8 % perft, -11.7 % generator

Three interleaved rounds:

```
perft total          475.0 -> 466.9 ms   -1.8 %   88.0 -> 89.6 Mnps
generate_moves alone 659.2 -> 582.1 ms  -11.7 %  540 -> 612 Mmoves/s
```

The generator itself got 11.7 % faster, which is close to the top of what the
estimate allowed, but it is only about a fifth of perft, so the workload as a
whole moved 1.8 %. That is below the 3 % bar this file sets, and it is kept only
because the direction is consistent across every round and the mechanism is
understood rather than lucky.

`generate_moves` was split into a body templated on `<Color, Constrained>` and
a small front end that computes the checkers and the pinned set and picks the
instantiation. Writing the masks as `Constrained ? mask : ~BB_0` is enough - the
condition is compile-time, so the unconstrained instantiation sees literal
constants and the optimiser removes every `& check_mask`, every pin test, and
the whole pinned-pawn loop as dead code.

Object code for `bitboard.cpp` went 61 KB to 68 KB with four instantiations of
the body. Still no instruction cache regression. This is the last template
parameter that pays; a third would double the code again for less.

Original entry follows.



Most nodes have no checkers and no pinned pieces. On those, `check_mask` is all
ones, `pinned` is zero, and every `& targets` and every `targets_from()` branch
is provably a no-op that still costs an instruction and a predicted branch.

Branch once at the top of the piece loops on `(checkers | pinned) == 0` and run
a version with the masking removed. This composes with item 2 as a second
template parameter rather than a runtime branch.

- Expected: **2-5 %** of total, being some fraction of the generator's 20 %.
- Risk: low-medium. It is a second copy of the emit loops, so the two can drift.
  Keep the masked version as the only implementation and let the specialisation
  fall out of the template, rather than writing the loops twice by hand.

## Struct packing — DONE, no measurable effect

`board_t` carried 10 bytes of padding: `color_t` was an int-backed enum using
four bytes for three values, and `fullmove_counter` sat between two holes.
Worse in principle, the scalars written on every move were separated from
`hash` by that padding and straddled a cache line boundary, so `make_move`
touched two lines to update state that fits in one.

Fixed: `color_t` and `promotion_t` are `uint8_t`-backed, and `hash` moved next
to the scalar group. **208 bytes to 200.**

**It bought nothing measurable.** Best-of-N over five interleaved rounds put the
two builds within 0.3 % of each other, which is noise. The likely reason is that
`board_t` is 200 bytes accessed on every move, so it is L1-resident either way
and the line split never costs a miss.

Kept regardless: it is strictly less padding and strictly smaller, it costs
nothing, and the size matters again the moment anything copies a `board_t` or
an NNUE accumulator lands beside it. Recorded here so the idea is not retried
expecting a speed-up.

## Measurement is currently blocked

These runs were taken with `opendirectoryd` at 45 % of a core and a load
average near 3. Under that, repeated runs of the *same* binary spread by 3 %,
which is the entire size of the remaining items. `hyperfine` reported a sigma of
0.166 s on one pair - larger than every effect left on this list.

**Do not attempt items 6 or 7 until the machine is quiet.** Their estimates are
1-3 % and under 1 %, and neither can be resolved through this much interference.
Check `ps aux | sort -rnk3 | head` before trusting any number below 5 %.

Item 1 does not have this problem: SPRT over thousands of games averages out
machine noise by construction, which is another argument for doing it next.

## 6. 16-bit move encoding

`move_t` is 32 bits and carries the moving piece, which the board already knows.
Stockfish and most modern engines use 16 bits: 6 from, 6 to, 4 flags. The move
list is `MAX_MOVES` entries touched on every node, so this halves the traffic
through it.

- Expected: **1-3 %.**
- Risk: medium, and it is wide rather than deep - it touches the encoding
  macros, the generator, `make_move`, move ordering, the opening book and the
  UCI layer. Do it when the surrounding code has stopped moving.

## 7. Single side-to-move key

```cpp
board->hash ^= randoms->side_randoms[us];
board->hash ^= randoms->side_randoms[them];
```

Two table loads and two xors, where one constant xored unconditionally does the
same job. `compute_full_hash()` and `swap_side()` must change together.

- Expected: **under 1 %.** Listed only because it is a ten-minute change with no
  risk attached.

## 8. PEXT sliding attacks, on x86 only

Replace the magic multiply with `_pext_u64` where BMI2 is available, keeping
magics as the fallback.

- Expected: **about 5 % of sliding attack generation**, which is a fraction of
  the generator's 20 %, so call it **1 % of perft**. Published comparisons put
  PEXT at 12.8 s against 13.5 s for magics on Kiwipete perft(6), and Stockfish's
  own perft numbers differ by well under a percent between the two.
- **Zero on this machine.** ARM has no PEXT. This cannot be measured on the M1
  and is one of the reasons an x86 box is needed before any of this can be
  ranked properly.
- Risk: low, but it is dead code on the development machine, which is its own
  hazard.

## Not worth doing

- **Fancy or black magics.** They shrink the attack tables from 2307 KB to
  roughly 860 KB. The corrected profile does not implicate table size anywhere,
  and `is_attacked` is now 0.1 % of the workload. Measure a cache-miss counter
  before spending a day on this.
- **Callback enumeration instead of a move list.** This is the single largest
  item in every published perft record and it is unavailable to an engine: a
  search must materialise moves to score and order them. Adopting it optimises
  `bench_movegen` and pessimises the engine.
- **Bulk counting at depth 1 in perft.** Legal generation makes it valid now,
  and it would multiply the headline number by roughly 6. It changes nothing
  about the engine. Worth doing once, only to get a figure comparable with the
  ones published online, and then never used as a target.
- **Shared attack tables.** `bb_tables_t` is 2307 KB inside every `game_t`, so
  `sizeof(game_t)` is about 3 MB. This is an ergonomics problem for anything
  that copies a `game_t`, not a speed problem, and perft has one instance.

## The ceiling, stated plainly

Items 2 through 8 together are worth maybe 15-25 % if every estimate lands,
which is **12-20 Elo**. Item 1 alone is worth more than all of them.

Beyond that, movegen is not where engine strength lives. The corrected profile
measures perft, and perft is not the search: it has no evaluation in it. Once an
NNUE evaluation exists, it dominates the profile and every percentage in this
document shrinks accordingly. Finish this list, then stop.

## How to verify each step

```
cmake --build build -j8
ctest -L fast                      # correctness, must stay green
./build/tests/bench_movegen        # node counts first, then speed
```

For anything touching the generator or `make_move`, also:

```
ctest -L slow                      # tests/test_perft, minutes
cd tests && ../build-debug/tests/test_movegen   # squares[] assertions on every node
```

Rules for this work:

- Node counts first, timings second. `bench_movegen` exits non-zero on a wrong
  count.
- One change at a time. Two at once and neither number means anything.
- A change under 3 % has not been shown to do anything unless `hyperfine` says
  otherwise with a tight sigma over interleaved runs.
- Record the before and after for every step, and correct the estimates in this
  file when they turn out wrong. They already did once.
