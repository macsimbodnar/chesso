id:         S020
goal:       compute the in-check state once per node instead of once per call site
accepts:    identical search_bench node counts and best moves, since this is behaviour-neutral; the cost measured with hyperfine over interleaved fixed-depth runs, its noise floor recorded, and the keep-or-revert call made from that number — a zero is recorded as zero and does not block completion
touches:    src/search.cpp negamax and quiescence
excludes:   changing when the search decides it is in check
decisions:
closes:
blocks:
paused_by:
done:

## Why

`is_check` was about 12 % of the profile **on the Apple machine under Apple
clang** -- a pre-DEC-049 figure that keeps its conditions attached and has not
been re-taken here. Re-profile on this machine before starting: the ranking
that placed this step was made from that number. The structural claim still
holds at HEAD -- `is_check` is recomputed at every node in both `negamax` and
`quiescence`, and the value is a property of the node, not of the call site.

Behaviour-neutral by construction, so this needs a determinism check and not an
SPRT -- see INV-6.

## Technical details (SOTA research, 2026-08-19)

Line numbers read at `c0954ec`. This step lands after all of block 1, so every
number below will have drifted -- re-locate by symbol, and re-sweep (section
7).

### 1. State of the art

The published shape is exactly this step's: the in-check state is a property
of the node, consumed everywhere in it (evasion generation, extensions, no
stand-pat in check -- CPW Check), computed by one of three routes: from the
last move made, from incremental attack tables, or on the fly. The bitboard
page prices them: with bitboards the by-last-move savings are called
negligible, and the branch-less on-the-fly scan is preferred because the
rook- and bishop-wise attack sets get reused for other purposes -- compute the
attack set once, let every consumer read it. Neither page prescribes a cache
structure; a function-local computed at node entry is the minimal form.

### 2. Shape for chesso

**The boolean is already once-per-function at HEAD**: negamax computes
`is_in_check` once (`src/search.cpp` `negamax`), quiescence `in_check` once
(`src/search.cpp` `quiescence`). The live duplication is one level down, in the
king-attack scan itself.

Per-node, negamax at position P:
- - `src/search.cpp` `negamax` computes `is_check(game)` at entry. Every reader
  is in that same function: RFP, NMP, LMR and the mate-vs-stalemate return --
  and after block 1 also S108's eval gate, S109's rule guards, S114's NMP gate,
  S116's razor guard.
- - `src/search.cpp` `negamax` calls `generate_captures` ->
  `generate_moves_impl`, and `src/bitboard.cpp` `generate_moves_impl`
  recomputes `attackers_to(king)` plus snipers and pins for P.
- - `src/search.cpp` `negamax` reaches `generate_quiets` from either of its two
  call sites -> the same preamble again, same P. Every node that opens the
  quiet stage pays checkers+pins twice.

Per-node, quiescence at position P:
- - `src/search.cpp` `quiescence` computes `is_check(game)`. Every reader is in
  that same function: the stand-pat gate, the generation choice, the two
  capture-filter sites, the `best_value` initialisation and the mate return.
- - `src/search.cpp` `quiescence` the generator -> preamble recomputes
  checkers+pins for P.

Per-move -- a DIFFERENT node, never collapsible into P's flag:
- - `src/search.cpp` `negamax` computes `is_check_move = is_capture ? false :
  is_check(game)`, post-make, for the child position P'. After S107 its sole
  reader is the LMR guard in the same function, and S107's accepts hands
  exactly that to this step to preserve; S109 adds per-move gives-check
  exemptions reading the same value.

Non-search callers stay untouched: SAN's +/# (`src/bitboard.cpp`
`move_to_algebraic`), `tools/datagen.cpp` `terminal_result` and
`tools/datagen.cpp` `play_games`, tests. `is_check` remains public.

Cost path: `is_check` (`src/bitboard.cpp` `is_check`) = king lsb +
`is_attacked` ->
`is_attacked_with_occupancy` (`src/bitboard.cpp` `is_attacked_with_occupancy`):
pawn/knight/king table ANDs
with early exit, then bishop and rook magics against bishop|queen, rook|queen.
Out of check no early exit fires, so all five lookups run -- the same five
`attackers_to` (`src/bitboard.cpp` `attackers_to`) does without exits;
`is_attacked(sq,c)`
iff `attackers_to(sq,c) != 0`, same tables, same occupancies[BOTH]. The
generator preamble adds count_bits, the between-table mask, two empty-occupancy
magic lookups for snipers, and the pin loop.

Where the flag lives: a function-local const at node entry, where it already
is. Every block-1 consumer reads it in the same function body. No per-ply
array is warranted: S108's improving learns an ancestor's in-check state from
the TT_EVAL_NONE sentinel in `static_evals[]`, not from a flag.

### 3. Implementation sketch

Each increment lands alone and proves itself node-identical first.

- - (a) **Share the preamble between the staged calls.** Extract a masks struct
  {checkers, pinned, king_square} + a compute function from `src/bitboard.cpp`
  `generate_moves_impl`; add generator entry points taking it precomputed; the
  existing three signatures compute-then-forward, so every non-search caller is
  untouched. `src/search.cpp` `negamax` computes the masks once before its
  first generation call and hands them to all three of its generation sites --
  the board is provably back at P everywhere they are read, because the move
  loop unmakes before the quiet stage opens. Same values reach
  generate_moves_body, INV-1/INV-3 untouched.
- - (b) **Unify the entry flag with the masks' checkers.** Replace
  the entry flag in `src/search.cpp` `negamax` and in `src/search.cpp`
  `quiescence` with `checkers != 0` from an
  `attackers_to` at entry; pins stay deferred to the generation site, because
  RFP/NMP (negamax) and the stand-pat cutoff and qply cap (quiescence) return
  in between and must not pay for pins. Keep the no-king guard both existing
  sites have.
- - (c) **Declined for the minimal shape**: passing the parent's post-make
  `is_check_move` value down as the child's entry flag. It stays inside
  search.cpp/hpp but changes both signatures, covers quiet-move children only,
  and buys the stale-flag risk class of section 5. Take it only if (a)+(b)
  measure zero and a fresh profile still shows the entry scan.

### 4. Constants and seeds

None. No parameter, no number to fit; DEC-084 is satisfied vacuously.

### 5. Pitfalls

- **The post-make flag is not this node.** `is_check_move` in
  `src/search.cpp` `negamax` is the child's state. Folding it
  into P's flag is wrong by construction, and deleting it breaks the LMR
  guard S107 explicitly preserved it for.
- - **Stale masks across make/unmake.** Masks are valid only at P. The board
  leaves P at the `make_move` in `src/search.cpp` `negamax` and returns at the
  matching `unmake_move`; cached masks may be read only where the board is
  provably back at P, and never from storage that outlives the node's frame. A
  debug assert (recompute == cached) at the quiet-stage read is cheap insurance
  during the transition.
- - **Quiescence's structure differs.** Its commonest conclusion is the
  stand-pat store-and-return (`src/search.cpp` `quiescence`), which needs the
  flag and never the pins -- hoisting the full preamble to `src/search.cpp`
  `quiescence` taxes exactly those nodes. Split checkers (entry) from pins
  (generation) there too; the qply cap (`src/search.cpp` `quiescence`) is the
  other early return.
- **The null-move child computes its own state.** No inference about
  post-null check state -- any shortcut there is a semantic change, outside
  `excludes:`.
- **No pre-make gives-check predicate exists** (S109's inventory agrees) and
  this step must not grow one; that is a behaviour question.

### 6. Measurement

DEC-083: no SPRT. Per increment, tools/search_bench.py identical node counts
and best moves at depths 9 and 12 against the parent commit; then one
interleaved fixed-depth timing -- hyperfine, alternating binaries, the
S103/S104 pattern (geometric-mean ratio, paired t) -- with bench_movegen's
resolution recorded first and `ps aux | sort -rnk3 | head` checked. Honest
expectation: small. The work removed is one preamble per staged node plus one
five-lookup scan per unified entry -- less per node than the evaluate() call
S103 skipped at 23 % of RFP sites for +2.47 %. Low single digits is the
ceiling; below the noise floor is a live outcome, and the accepts already
makes the keep-or-revert call from the number. The Why's 12 % is a
pre-DEC-049 Apple-clang figure -- re-profile on this machine first, as the
file orders.

### 7. Interactions

Plan order lands S020 after all of block 1, so section 2's inventory is a
floor, not the final sweep: by then S108/S109/S114/S116 read the entry flag,
and S109's skip_quiets has rewired the staged-generation branch
(`src/search.cpp` `negamax`) that increment (a) shares. Re-sweep at start with
`grep -n "is_check\|in_check" src/search.cpp`. The order is right as it stands
-- landing S020 first would mean rebasing it under every block-1 step; landing
it last collects all their call sites in one sweep, and each block-1 step needs
only the local that already exists (S108's section 7 records "no conflict" with
S020 explicitly). S107 (before): preserve `is_check_move` for LMR alone. S098
(before) owns the exemption semantics; S020 changes no eligibility.
S042/S032/S030 (same block, after): movegen internals -- the masks seam from
(a) lands first and neither side cares.

### Scope concern

Two, neither changing goal or accepts. (1) The Why's "recomputed at every
node ... once per call site" is, for the boolean, already done at HEAD -- one
call per function invocation. The step's real content is the attack-scan
level: the generator preamble recomputed per call (twice per staged node) and
the parent/child double scan. Read "in-check state" at that level and the
goal stands; the expected effect is the plan's "nearly free", not the Why's
12 %. (2) The highest-value increments (a) and (b) cross the search/movegen
seam and must touch src/bitboard.cpp and .hpp, which `touches:` does not
name. Confined to src/search.cpp alone, the step has almost nothing left to
remove -- expect the field to grow at implementation, recorded, not silent.

### 8. References

- https://www.chessprogramming.org/Check -- detection by last move / attack
  tables / on the fly; the flag's per-node consumers (evasions, extensions,
  no stand-pat in check); no caching prescription.
- https://www.chessprogramming.org/Checks_and_Pinned_Pieces_(Bitboards) --
  checkers and pinned sets from attack lookups; "with bitboards the possible
  savings to determine checks by last move seems negligible"; pins are what
  legal generation needs anyway.

## What S107 left here, measured (2026-08-20)

S107 removed the fail-high gate's `!is_check_move` term, so the flag now has
exactly one consumer: the late move reduction guard at `src/search.cpp`
`negamax`, where `!is_check_move` is the **last** conjunct. That makes a
second, cheaper saving available in the same neighbourhood as this step's, and
it was counted rather than argued -- instrumented copy, kiwipete `go depth 11`:

| site | calls |
|---|---|
| `is_check(game)` at `src/search.cpp` `negamax` | 888738 |
| the guard's cheap prefix true (`ply>0 && depth>=3 && legal_moves_counter>3 && !is_capture && !MOVE_PROMOTED && !is_in_check`) | 179590 |

So the flag is consumed by about 20 % of the calls that compute it, and the
other 709148 pay an attack scan per non-capture node for nothing. Computing it
lazily behind that prefix is behaviour-neutral by short-circuit, which means it
discharges on identical node counts and best moves exactly as this step's own
accepts does -- no SPRT owed. The existing comment already makes the argument
for captures ("is_check() is an attack scan - do not pay for it on captures");
this is the same argument extended to the nodes that never reach the guard.

Note the ordering constraint: `is_in_check` is the parent's state and
`is_check_move` is the child's, taken after `make_move`. Deferring the child's
scan is safe; conflating the two is not. Whether this lands as part of the
single-computation restructure or as a separate conjunct reorder is this step's
call, but the number above is what it is worth.
