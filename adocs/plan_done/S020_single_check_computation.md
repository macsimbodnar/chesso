id:         S020
goal:       compute the in-check state once per node instead of once per call site
accepts:    identical search_bench node counts and best moves, since this is behaviour-neutral; the cost measured with hyperfine over interleaved fixed-depth runs, its noise floor recorded, and the keep-or-revert call made from that number — a zero is recorded as zero and does not block completion
touches:    src/search.cpp negamax and quiescence (amended at implementation: what lands is src/search.cpp negamax alone; src/bitboard.cpp and src/bitboard.hpp were opened for the generator seam the brief allowed, measured slower in four shapes and reverted -- "As built" has why)
excludes:   changing when the search decides it is in check
decisions:
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S020_brief.md), 2026-10-04
done:       2026-10-04 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against 32633c4: bench 4081329 and bench 12 1860699 with every info line and bestmove identical; search_bench 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at depth 12, c3d5 / e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical. No SPRT owed (DEC-083). The child's gives-check scan in negamax is taken lazily behind its two consumers and memoised: 1262824 of 5062561 scans skipped on the bench (24.9 %), 7.5 % on kiwipete depth 11 where S107 counted 80 % before S109's exemption became a second reader. +1.63 %, 95 % CI +1.49 .. +1.76, ratio 0.9840 over 24 interleaved hyperfine pairs on 300 positions at depth 11, paired t -24.45, 24/24 faster; A/A floor +/- 0.2 % (16 pairs, t 0.81); bench_movegen resolution 0.2 %. About +2.3 Elo at long time control as a conversion, not a verdict. KEPT. The attack-scan level (masks shared across the staged calls, entry flag from the checkers) was node-identical and measured -1.89 % and -3.52 % in two timed shapes and slower perft in three more; REVERTED. Cause: src/bitboard.cpp is at gcc's inline-unit-growth limit (61 refusals at the parent), and any growth there costs make_move its inlined accumulator calls -- S253, created by the coordinator. Suites 41/41 in build and build-tune, format clean, Debug guard suite 41/41 (format test rerun with the DEC-146 override), 0 Assertion. Profiled with callgrind (samply refused at perf_event_paranoid 2). Second tier (DEC-141), by the coordinator: Debug self-play 8 games at 4+0.04, 0 Assertion, 0 disconnect; tools/gate_extra.sh 5 stages green in 1041 s (.tuning/gate_extra_2026-10-04_S020.log). Cold fast check: nothing.

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

## As built (2026-10-04)

### Deviations, first

1. **Only the first increment lands.** The lazy child check is in and
   measured +1.63 %. The attack-scan level -- the generator's masks computed
   once per staged node (section 3 (a)), and the entry flag folded into the
   checkers (b) on top of it -- was written, proved node-identical, measured
   **slower in every shape tried**, and reverted. (b) was not attempted on
   its own: without the seam it removes nothing. The step's goal stands
   partly met: the boolean was already once per node at HEAD (scope concern
   1), the child scan is now paid only where it is read, and the generator
   still repeats the king scan the entry flag made.
2. **`touches:` amended.** `src/bitboard.cpp` and `src/bitboard.hpp` were
   opened, as the brief allowed, and are back at their parent's bytes. What
   lands is `src/search.cpp` `negamax` alone; quiescence is untouched.
3. **S107's 80 % is gone, and the recount says why.** Section "What S107
   left here" counted one consumer. Since S109 and S091 the flag has two --
   the gives-check exemption reads it for every quiet a pruning rule has
   marked -- so the scan is needed far more often than S107's prefix alone
   implied. Recounted below; the saving is 25 % of the scans on the bench,
   7.5 % on kiwipete.
4. **Profiled with callgrind, not samply.** `kernel.perf_event_paranoid` is
   2 on this machine and samply refuses below 1 (`.moltke.local.md`);
   changing it needs the owner. Callgrind counts instructions, not cycles,
   so its shares rank sites and do not price them; the interleaved timing
   prices them.
5. **A finding outside the step, reported and not fixed:
   `src/bitboard.cpp` sits at gcc's `inline-unit-growth` limit.** At the
   parent commit gcc 13.3 already refuses 61 inlinings in that unit on the
   budget (`-fopt-info-inline-missed`), eight of `make_move`'s accumulator
   calls among them. Any growth in the unit re-ranks which calls get the
   budget. Every seam shape here grew it, and in every one `make_move` lost
   more of its inlined `eval_add_piece` / `eval_remove_piece` calls -- 14 or
   16 out-of-line call sites against 8, plus two in `unmake_move_impl` --
   and perft fell 11 to 15 %. That is the cost, not the masks: rebuilt with
   `--param inline-unit-growth=200`, parent and candidate both inline every
   accumulator call (0 out-of-line call sites in either) and perft reads
   1075 / 1092 ms against 1082 / 1083 ms. The same fragility waits for every
   later change to that unit (S042, S032, S030 edit generator internals).
   Proposed as its own step: make the hooks' inlining deterministic (an
   attribute on the hooks, a unit-local parameter, or splitting the unit),
   measured by the same interleaved timing, then re-attempt (a) and (b) on
   top. Not done here: it is a build and `eval_tables.hpp` change outside
   `touches:`, and a timing-moving change of its own.

### Sweep, on `32633c4`

`grep -n "is_check\|in_check" src/search.cpp`: the entry flag once in
`negamax` (`is_in_check`, read by the eval sentinel, the `pruning_eval`
guard, RFP, razoring, NMP, ProbCut, the `pruning_node` gate, `improving`,
SEE-LMR eligibility, the reduction guard and the mate return) and once in
`quiescence` (`in_check`); the child's `is_check_move` after `make_move`,
read by the **gives-check exemption** (`prune_rule != PRUNE_NONE &&
!is_check_move && ...`) and the reduction guard (`may_reduce`); S091's
`capture_gives_check` for captures a rule acts on; quiescence's futility
gives-check probe; and the PV / mate-walk callers (`is_check(game)` in the
walk helpers) off the hot path. The generator preamble
(`src/bitboard.cpp` `generate_moves_impl`) computes `attackers_to(king)`, the
snipers and the pin loop on every call: twice at a negamax node that opens
the quiet stage, and once more after an entry flag that already scanned the
same king.

### Profile and counts

Callgrind over `chesso bench` (4081329 nodes, 9.04 G instructions), a
Release build with `-g` added: `is_check` 5.22 % inclusive,
`generate_captures` 5.42 %, `generate_quiets` 3.01 %, `generate_moves` 0.88 %
(each inclusive of its preamble). About 61 instructions per `is_check` call.

Counted, not argued -- an instrumented throwaway copy of `32633c4` (counters
only, bench unchanged at 4081329), over `chesso bench`:

| site | calls |
|---|---|
| negamax entry `is_check` | 820304 |
| quiescence entry `is_check` | 1350571 |
| quiescence futility gives-check probe | 227884 |
| child scan `is_check_move`, every non-capture made | 5062561 |
| of those, a consumer's prefix holds (`prune_rule` set after LMP, or `ply > 0 && depth >= 3 && legal_moves_counter + 1 > 3 && !is_in_check`) | 3799737 |
| `capture_gives_check` scans | 243262 |
| negamax `generate_captures` | 602452 |
| negamax `generate_quiets`, up front / after the capture stage | 239836 / 136297 |
| quiescence `generate_moves` (in check) / `generate_captures` | 141328 / 693049 |

So 1262824 child scans, **24.9 %**, were paid for nothing -- 16.4 % of all
7704582 `is_check` calls. Kiwipete `go depth 11`, S107's own probe: 236887
child scans, 219127 needed, **7.5 %** skippable, against S107's 80 % (888738
and 179590 then).

Of the attack-scan level, a seam would have removed 376133 repeated
preambles (the quiet stage) and, with (b), the 602452 + 834377 generator
king scans that repeat an entry flag. Instruction-count estimate of each:
under 1 % of the bench.

### Increment 1 -- the child check, lazily, behind its consumers

`src/search.cpp` `negamax`: `is_check_move` becomes a memoised lambda,
false on a capture as before, that scans on first read. Both readers keep
it as their last conjunct, so the short circuit is what gates the scan --
no consumer's condition is copied, and a later edit to either guard cannot
drift from it. The memo is there because a pruned quiet that gives check
reaches the exemption and then the reduction guard. The board is the
child's for the cached value's whole life: `make_move` to the reduction
guard, with no recursion or `unmake_move` in between. `is_in_check` (this
node) and the child's value stay separate names.

**INV-6** against a Release build of `32633c4` in
`.ref-builds/s020_base`, streams stripped of `time` / `nps` / `hashfull`
only and diffed whole (`.tuning/coord/S020/inv6.sh`): `chesso bench`
**4081329**, every info line and every `bestmove` identical; `chesso bench
12` **1860699**, identical; `tools/search_bench.py` depth 9 **32932 / 70095
/ 25178**, `c3d5` / `e2a6` / `d7c8q`, and depth 12 **67792 / 280873 /
137893**, `c3d5` / `d5e6` / `d7c8q`, identical. The final binary is
byte-identical (`cmp`) to the one timed.

### Increment 2, attempted and reverted -- the generator seam

`gen_masks_t {checkers, pinned, king_square}`, generator entry points
taking it, the three existing signatures computing and forwarding; `negamax`
computed it once before `generate_captures` and handed it to both
`generate_quiets` sites; a Debug assert recomputed and compared at every
masked call. Node-identical: the full INV-6 set above matched in every
shape, and `bench_movegen` verified perft. Every shape measured slower:

| shape | result |
|---|---|
| masked and unmasked paths sharing one template | search **-1.89 %** (24 pairs, CI -2.05 .. -1.74, t +24.74); the bodies no longer inlined anywhere |
| masked impl `noinline`, unmasked computing inline | search **-3.52 %** (24 pairs, CI -3.63 .. -3.41, t +66.27); perft 1158 / 1172 ms against 1043 / 1039 |
| three further arrangements (unmasked via `gen_masks()`, both inline, one impl taking an optional mask pointer) | perft 1152 to 1197 ms against 1038 to 1046 |
| control: parent `bitboard.cpp`, new header | perft 1038 ms |

Diagnosis in deviation 5. Reverted to the parent's bytes; the last shape's
diff is kept at `.tuning/coord/S020/inc2_attempt_final.diff` for the step
that re-attempts it.

### The timing

Machine: governor `performance` (recorded, not set, DEC-195); load average
1.26 before the A/A and 1.37 to 1.65 across the runs, the top processes the
desktop's terminal and compositor and `btop` at 1 to 2 % lifetime each.
`bench_movegen -r 40` first: **resolution 0.2 % / 0.8 % / 0.4 %**
(perft / generation / captures), spread 2.8 / 4.0 / 11.8 %; a default
`-r 5` run read 2.7 % resolution and is the reason the longer one was taken.

Workload: S021's 300 stratified positions (`adocs/data/S018_raw.tsv`, four
per phase, offsets 0 / 1 / 2) at `go depth 11` through one engine process,
26512806 nodes, about 7.4 s a run (`.tuning/coord/S020/workload.py`). Each
pair is one `hyperfine -N -r 1` invocation running both binaries, the order
alternating pair by pair; geometric-mean ratio, 95 % CI from the paired t
(`.tuning/coord/S020/timing.sh`, `pairs.py`).

- **A/A first**, the parent against a byte copy of itself, 16 pairs: ratio
  1.0007, CI 0.9989 .. 1.0025, paired t 0.81, per-pair sd 0.35 %. That is
  the pairing's floor here: about +/- 0.2 %.
- **Increment 1 against the parent, 24 pairs: ratio 0.9840 geometric mean,
  0.9836 median, 95 % CI 0.9827 .. 0.9853, paired t -24.45, per-pair sd
  0.32 %, 24 of 24 pairs faster. Speed-up +1.63 %, CI +1.49 .. +1.76 %.**

Under CLAUDE.md's 3 % line, and believed anyway on rule 5's own exception:
hyperfine over interleaved runs, a sigma a fifth of the effect, an A/A
reading zero at the same shape, and every pair the same sign. The
instruction-count estimate (1.26 M scans at about 61 instructions, 0.85 % of
the bench) is the same order; the clock reads more, which a scan's loads
would explain and which is not claimed. At DEC-083's published rates
+1.63 % is **about +2.3 Elo at long time control and +3.4 at short --
stated as a conversion, not a verdict.**

**Keep-or-revert: increment 1 is kept on +1.63 % (CI +1.49 .. +1.76).
Increment 2 is reverted on -1.89 % and -3.52 % (cause: deviation 5).**

### Suites

`cmake --build build -j8 && ctest --test-dir build -L fast` **41/41**,
`build-tune` **41/41**, `./clang-format.sh --check` clean under
`CLANG_FORMAT_MAJOR=22` (DEC-146) -- one chain, exit 0. Debug guard suite:
**40/41 in 1072 s, 0 `Assertion`**; the one red was `test_clang_format_script`, run
without the `CLANG_FORMAT_MAJOR=22` export (the DEC-146 override), and
green on rerun with it -- an environment miss, not the change. `tools/plan_prose_check.py` `--citations`, `--touches`,
`--params`, one mode per call: each exit 0, 0 flagged. No test added: the change is
behaviour-neutral by short circuit and INV-6 is its proof; no pruning,
reduction or extension rule moved, so DEC-141's mutant clause does not
apply. The second tier's Debug self-play is the coordinator's
(`make_move` is not touched, the search is).

`MANUAL.md` checked: no UCI surface, option or output moved -- no change.
`DEV_MANUAL.md` checked: no command, flag or default moved -- no change;
deviation 5 is a hazard it may want once the follow-up step decides the
fix. `specs.md`: no behaviour changed, no sentence proposed.

### Proposed `done:` stamp

BEHAVIOUR-NEUTRAL, INV-6 discharged against 32633c4: bench 4081329 and bench
12 1860699 with every info line and bestmove identical; search_bench 32932 /
70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at depth 12, c3d5 /
e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical. No SPRT owed (DEC-083).
The child's gives-check scan in negamax is taken lazily behind its two
consumers and memoised: 1262824 of 5062561 scans skipped on the bench
(24.9 %), 7.5 % on kiwipete depth 11 where S107 counted 80 % before S109's
exemption became a second reader. +1.63 %, 95 % CI +1.49 .. +1.76, ratio
0.9840 over 24 interleaved hyperfine pairs on 300 positions at depth 11,
paired t -24.45, 24/24 faster; A/A floor +/- 0.2 % (16 pairs, t 0.81);
bench_movegen resolution 0.2 %. About +2.3 Elo at long time control as a
conversion, not a verdict. KEPT. The attack-scan level (masks shared across
the staged calls, entry flag from the checkers) was node-identical and
measured -1.89 % and -3.52 % in two timed shapes and slower perft in three
more; REVERTED. Cause: src/bitboard.cpp is at gcc's inline-unit-growth
limit (61 refusals at the parent), and any growth there costs make_move its
inlined accumulator calls -- proposed as its own step. Suites 41/41 in build
and build-tune, format clean, Debug 41/41 (format test rerun with the DEC-146 override), 0 Assertion. Profiled with callgrind
(samply refused at perf_event_paranoid 2).

### Proposed commit

```
Compute the child's check lazily behind its consumers (S020)

The gives-check scan after make_move ran on every non-capture made. Its
two readers, the gives-check exemption and the reduction guard, both
take it as their last conjunct, so it is now a memoised read that the
short circuit gates: 24.9 % of the bench's child scans are skipped, and
the search is 1.63 % faster (CI 1.49 .. 1.76, 24 interleaved pairs, A/A
floor 0.2 %). Node-identical to the parent (INV-6).

The generator seam the step also named was node-identical and slower in
every shape: src/bitboard.cpp is at gcc's inline-unit-growth limit and
any growth there costs make_move its inlined accumulator updates. It is
reverted and the fix is proposed as its own step.

No functional change
```
