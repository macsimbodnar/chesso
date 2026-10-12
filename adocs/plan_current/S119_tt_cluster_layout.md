id:         S119
goal:       the table becomes cache-line clusters with an aged replacement, a prefetch issued when the key is known, and huge pages
accepts:    an SPRT verdict **at the S105 harness setting, with the pressure ratio stated** -- the ratio being overwrites per entry, which is what transfers across time controls: the rating list's 2'+1" writes on the order of 660 M nodes against 5.6 to 11 M entries, 60 to 120 apiece, and 16 MB at 8+0.08 reproduces that while 128 MB undershoots it eightfold and "would flatter every table-hungry change S119 is about to make" (DEC-088, whose `Consequences:` line is what replaced "at Hash 128" here); `sizeof` the cluster is exactly 32 or 64 bytes and the array is aligned so no cluster straddles a cache line, asserted at compile time; the replacement prefers depth **and** age together rather than depth within a generation alone; the prefetch is issued as soon as the key is known in make_move; huge pages are requested and the failure path is a normal allocation, not an abort; `hashfull` is reported over UCI and is checked to stay low at the rating control; the nps and the nodes-to-depth are both recorded, because this step moves them in opposite directions
touches:    src/transposition_table.cpp, src/transposition_table.hpp, src/data_structures.hpp, src/search.cpp (the prefetch at each make_move/make_null_move site, one comment), src/chesso.cpp (`hashfull` on the info line, two comments), tests/test_search.cpp, tests/test_engine.cpp, tests/test_en_passant_key.cpp, tests/test_uci_surface.cpp, tools/mutants/S119_tt_cluster.py, tools/mate_trace.cpp and tools/probe_cost.cpp (comments), MANUAL.md, DEV_MANUAL.md (the harness table's hash row), adocs/specs.md, adocs/data/S119_sprt.sh, adocs/data/S119_timing.txt -- amended by the implementer 2026-10-12; src/bitboard.cpp make_move, named until then, is not touched (As built, deviation 1)
excludes:   the static evaluation field, which exists since S094; bound-sign correctness, which is S106
decisions:  DEC-083, DEC-258
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:

## Added 2026-10-08, DEC-258: the capacity change is part of what the verdict prices

`src/transposition_table.cpp` `tt_resize` sizes the table to a power of two
of 24-byte entries, so `Hash=16` uses 12 MiB and `Hash=128` uses 96 MiB. A
32-byte cluster fills the whole power-of-two allocation, so at the
harness's Hash=16 the table gains a third more entries along with the new
layout and replacement. The pre-registration states this, states the
pressure ratio at the new capacity, and reads the verdict as layout,
replacement and capacity together. Non-power-of-two `Hash` requests are
indexed without rounding down and get a test; the allocation fallback when
huge pages fail is a normal allocation. A partial-key legality check protects
consumers of the stored move; it does not authenticate a stored score, depth
or mate certificate, and the step states the collision trade for every reader
of a stored mate or PV. Prefetch and huge pages are node-identical and are
timed separately where practical.

## The verdict is taken at the harness setting, not at Hash 128

Applied by S139, and it is DEC-088's own `Consequences:` line rather than a new
choice: *"S119's SPRT clause changes from 'at Hash 128' to 'at the S105 harness
setting, with the pressure ratio stated'"*. The harness is what settles it --
`fastchess.sh` "${HASH:-16}" is the default its `-each` line plays:

```
  -each tc="$tc" option.Hash="$hash" option.Threads=1 \
```

S151 added the `HASH` override after this paragraph was written, so a clause
asking for 128 is now one environment variable rather than the harness edit it
used to be. That changes nothing here. DEC-088's reason for refusing 128 is
that it flatters exactly this step, which is about the number and not about
how hard it is to pass, and this step's verdict is taken at the default.

The rating regime is still reachable and is a different tool: `rating.sh`
`hash_mb` is `hash_mb=128`, fed to `-each` at `:172`. If a second verdict there
is ever wanted it is a `rating.sh` run, named as one, with `rating.sh` in
`touches:` -- not this step's SPRT.

## What is there

`tt_entry_t` is **24 bytes** -- a full 64-bit key, a 32-bit score, a 32-bit
move, depth, eval, type, generation. The table is **direct-mapped**: one entry
per slot, `hash & index_mask`, no bucket. Replacement is depth-preferred inside
a generation and always-replace across generations. There is no prefetch and no
page hint.

Three consequences. 24 bytes divides neither 32 nor 64, so **entries straddle
cache lines**. Direct-mapped means a single colliding position evicts a deep
entry with no second chance. And the probe miss is fully exposed, because
nothing is issued early.

Measured 2026-08-19, `go movetime 2000` from the start position: 16 MB to
512 MB is **36 % fewer nodes and 21 % lower nps**. The node reduction outweighs
the nps loss, so a bigger table is net positive -- but a fifth of it is being
eaten by the layout, and the rating list runs at the size where that bites
hardest.

**Expect the replacement scheme itself to be worth little** -- the surveyed
consensus is 5 to 15 Elo for bucket-plus-aging over always-replace
(**unverified, and searched twice**: the 2026-09-04 literature check fetched
CPW *Transposition Table*,
https://www.chessprogramming.org/Transposition_Table, which describes bucket
systems sized to a cache line and ageing by the halfmove clock modulo a power
of two but **states no figure**, and found no other source for the range;
searched again 2026-09-13 after S186's fast check -- an open web search over
the TalkChess replacement-scheme threads, which discuss depth-preferred
against always-replace and ageing at length and price **none** of them in
Elo. No source states this range and no run here is priced on it), and the one
part that is not optional is the aging. The layout, the alignment and the
prefetch are where the measured 21 % lives.

## As built (2026-10-11 to 12, Opus subagent)

Worktree at `b3a93aa`, i7-8700K, g++ 13.3, Release, padded (S273). `bench`
printed the parent's 4081329 before any change. A, B and C are separate diffs
applying in that order (`.tuning/coord/S119/partA.diff`, `partB.diff`,
`partC.diff`; the whole is `step.diff`). Every timing is read by DEC-266, which
the owner adopted while this step ran.

### Deviations, first

1. **The prefetch is issued in the search, not inside `make_move()`**, and
   `src/bitboard.cpp` is not touched although `touches:` named it. The brief
   allowed "or where the key is final". `make_move()` does not know the table,
   and teaching it would put a table test on every perft and generator call.
   The key is final at its last hash write; the call returns a few instructions
   later and the search calls `tt_prefetch(state->tt, game->board.hash)`
   there, at all four sites: quiescence's move loop, the null move, ProbCut's
   loop and the main move loop. The NNUE hook's alphabet is untouched.
2. **The generation is 16 bits, not 8**, which the brief did not ask for. Age
   is read as a distance on the counter's cycle (`tt_age()`), as asked. That
   alone fixes the wrap's comparison; it cannot stop an entry untouched for
   exactly a whole cycle from reading as age 0. 16 bits moves that from every
   255 searches (reachable in one long game, S271) to every 65535, and the
   cluster makes the residual harmless: a store of another position always
   lands (tested). The 20-byte entry had the room.
3. **Two timing sessions.** Session 1 (2026-10-11 12:50 to 13:32) was
   disturbed: a desktop browser took 20 to 30 % of a core throughout, and its
   A/A floor was 2.45 / 1.85 / 1.67 % (cycles `bench 16`, cycles 300
   positions, wall) against S273's 0.56 / 0.37 / 0.39. Under DEC-266 it would
   have read A and B128 as zeros, missing the floor on the 300 positions by
   0.02 and 0.18 points. An API session limit then stopped the work and the
   machine rebooted. Session 2 (2026-10-12 10:05 to 11:26) was quiet, and every
   verdict below is read from it, with its own A/A at Hash 16 and at Hash 128.
   Session 1 agrees in sign everywhere and is kept as corroboration only
   (`adocs/data/S119_timing.txt`).
4. **Tests re-stated, none relaxed.** The step changes what four tests pinned:
   - "resize clamps and keeps a power of two" is now "resize clamps and uses
     every megabyte". The power of two is gone by design; it now asserts the
     whole request in clusters, `entry_count = 3 * cluster_count`, and the
     64-byte alignment.
   - "the static evaluation field costs no table entries" is re-derived for
     the new layout: 20-byte entries, three to a 64-byte line with or without
     the field, and 196608 entries at 4 MB (was 24 bytes and 131072).
   - "a colliding key is a miss, not somebody else's entry" now collides on
     the cluster with different low 32 bits. The same-32-bits case is the
     partial key's own new case.
   - The two generation-wrap cases now loop past the 16-bit wrap and count it.
     At 600 iterations they would have passed without a wrap ever happening.

   Mechanical edits: `entries` becomes `clusters` (six sites), the 64-bit key
   literal in "an evaluation never survives a key change" goes through
   `tt_key_of()`, and "a stored entry comes back with every field intact"
   compares `tt_key_of(hash)` and the generation.
5. **`MANUAL.md`'s example `info` line was replaced**, not extended. Its
   numbers came from an engine long gone, so it is now a real S119 line: the
   midgame bench position, `go depth 6`, Hash 16.
6. **The first mutation launch refused.** `CLANG_FORMAT_MAJOR` was not
   exported, so `test_clang_format_script` was red on the baseline. The
   re-launch exported it (`.tuning/coord/S119/mutation_refused_no_format_env.log`).
7. **Outside `touches:`, now listed there:** `DEV_MANUAL.md`'s harness hash row
   gains the measured pressure, and two tool comments that described the old
   table are corrected (`tools/mate_trace.cpp`, `tools/probe_cost.cpp`).

### Part A, the prefetch -- KEPT

`tt_prefetch()` in `src/transposition_table.hpp`: `__builtin_prefetch` of the
slot, both ends of a 24-byte entry on the direct-mapped table, behind a
GNU/clang guard. It is called after `make_move()` / `make_null_move()` at the
four sites in deviation 1.

**INV-6:** `bench` 4081329 and `bench 12` 1860699 match the parent on every
line (times and nps stripped). `tools/search_bench.py` matches too: depth 9
32932 / 70095 / 25178 and depth 12 67792 / 280873 / 137893, best moves
c3d5 e2a6 d7c8q and c3d5 d5e6 d7c8q.

**DEC-266, session 2:**
1. Both sides padded.
2. S270's method, 24 / 16 / 24 pairs.
3. Floor at Hash 16: 0.473 % cycles on `bench 16`, 0.183 % on the 300
   positions, 0.330 % wall. A reads -2.02 % (CI -2.20 .. -1.85, 24/24),
   -2.19 % (CI -2.29 .. -2.09, 16/16) and wall +2.30 % (CI +2.03 .. +2.57,
   24/24). All are wholly beyond the floor.
4. Instructions move the other way, +0.63 / +0.61 %, so the perturbations
   decide. `-falign-functions=64` reads -2.17 / -2.33 % cycles and +2.17 %
   wall. The dummy function reads -2.10 / -2.37 % and +2.40 %. Every CI clears
   the floor, every pair is faster, and the signs agree. **Credited.**

Session 1 read the same sign, larger and noisier (-4.93 / -3.36 % cycles,
+2.68 % wall).

### Part B, huge pages -- KEPT

`tt_allocate()` / `tt_release()` in `src/transposition_table.cpp`. On Linux the
table is an anonymous `mmap` with 2 MiB of slack, aligned to 2 MiB and
`madvise(MADV_HUGEPAGE)`. A refused advice logs and leaves ordinary pages; a
refused mapping falls through to `calloc()` aligned by hand to 64 bytes.
Neither path aborts. Elsewhere it is `calloc()` alone, aligned by hand.
`munmap` or `free` is chosen by a recorded flag. The mapping stays lazily
zeroed, so an unfilled 4096 MB table commits nothing, as with `calloc()`.

**INV-6:** the same streams as A, identical.

**The kernel's answer** (`thp_check.py`, `AnonHugePages` in
`/proc/<pid>/smaps_rollup` during `go infinite`):
- direct-mapped: 12288 kB at Hash 16 and 98304 kB at Hash 128, the whole of
  the 12 and 96 MiB tables; ordinary pages read 0 kB.
- the C build: 16384 and 131072 kB.

THP is `madvise`, defrag `madvise`, on this machine.

**DEC-266, session 2:**
1. Both sides padded.
2. S270's method.
3. Floor at Hash 128: 0.176 / 0.227 / 0.160 %. B at Hash 128 reads -3.59 %
   (CI -3.70 .. -3.48, 24/24), -3.61 % (CI -3.71 .. -3.52, 16/16) and wall
   +4.61 % (CI +4.48 .. +4.74, 24/24). All are beyond the floor.
4. Instructions do not move (-0.000 %), so the perturbations decide. `a64`
   reads -3.86 / -3.43 % cycles and +3.70 % wall (CI +2.00 .. +5.42, 15/16).
   The dummy function reads -3.81 / -3.49 % and +4.67 %. All clear the floor
   and the signs agree. **Credited at 128.**

At Hash 16, the size with no loss allowed, B reads -1.76 % (CI -2.00 ..
-1.52), -1.55 % (CI -1.72 .. -1.38) and wall +1.76 % (CI +1.59 .. +1.92).
That is faster, beyond the Hash 16 floor; no perturbations were run there,
since the rule asks only that it lose nothing. Session 1 agreed:
-3.99 / -3.80 %, +5.30 % at 128; -0.93 / -1.64 %, +0.99 % at 16, inside that
session's floor.

### Part C, clusters and aged replacement -- for the SPRT

- `tt_entry_t`, 20 bytes: the hash's low 32 bits as the key, score, move,
  eval, depth, a 16-bit generation, type.
- `tt_cluster_t`, `alignas(64)`, holds three entries. Four `static_assert`s:
  the entry is 20, the cluster is 64, its alignment is 64, and the
  allocation's line is the cluster's.
- **Index:** `tt_cluster_index()` keeps the top half of `hash *
  cluster_count`, so it reads the high bits. `tt_mul_hi64_halves()` is the
  fallback without `__int128`, tested against it.
- **Capacity:** `cluster_count` is MB * 16384, never rounded. **Hash 16:
  524288 entries in 12 MiB, direct-mapped, become 262144 clusters, 786432
  entries in 16 MiB (+50 %), 3-way.**
- **Probe:** a key and written-generation match in any of the three ways.
- **Store, same position:** the old rule (depth-preferred inside a search,
  always across searches).
- **Store, other position:** it takes a never-written way, else the way with
  the least `depth - 2 * age`. `TT_AGE_PLIES = 2` is DEC-134 form (b), derived
  at its site: one search per own move puts the next root two plies down.
- **Prefetch:** one line, the cluster.
- **`tt_hashfull()`:** permille of the first 334 clusters (1002 entries)
  written by the current generation. It goes on every `info` line between
  `nps` and `pv`.

DEC-221: built from this step file's description, which quotes CPW's
Transposition Table page; no other engine's code was opened.

**The collision trade**, stated at `tt_get_entry()`. A match checks 32 key
bits plus the index, about 50 bits at Hash 16. An absent position matches
another's entry with probability 3 / 2^32 = 7.0e-10 per probe. At the
harness's ~2.1e7 nodes per side per game that is about 0.03 false hits per
game; at the list's 2'+1" (5e8 per side in the measured game below) about 0.7.
Each reader:

- **The move** is never played on trust. Ordering, the singular extension's
  excluded move and LMR's tt-move flags compare it with the generated list.
  The mate walk refuses a move the generator does not produce
  (`complete_mate_pv()`'s `playable` check).
- **The mate line** is published only when the walk ends in checkmate at
  exactly the claimed distance (DEC-122, S147). The new case "a colliding
  table entry cannot extend a mate line" plants, under another hash with the
  same low 32 bits and cluster, an exact mate at the owed distance naming the
  root's own move. The walk refuses it and leaves the 1-ply line; its control,
  the same plant naming the right move, completes 5 plies. The existing guards
  stay green: "the walk takes a certified child over a bound entry's move",
  `test_mate_pv`'s two "reports no mate it cannot show" cases, and
  `test_mate_carry`.
- **The score, depth, bound and eval are not authenticated** (DEC-258). A
  colliding entry can cut a non-PV node, refine a stand pat or a margin, or
  stand in for `evaluate()`, and a mate score read so is a claim no search
  made. The rate above is the only bound.

**Node change** (Part C changes the tree):
- `bench` 4081329 -> 3872032 (-5.13 %); `bench 12` 1860699 -> 1730486
  (-7.00 %). Seven of the eight replies are the parent's; the fifth goes
  d8d6 -> d8e7.
- `search_bench` depth 9: 32829 / 69892 / 25174. Depth 12: 68090 / 246712 /
  114555. Best moves unchanged.

**nps and nodes-to-depth**, AB against C (`runsC/`, the S270 method, as
measurements and not a DEC-266 reading):

| Hash | workload | nodes | nps | time to depth (wall) |
|---|---|---|---|---|
| 16 | `bench 16` | 10214429 -> 9631170, -5.71 % | 4735014 -> 4552221, -3.86 % | -1.64 % (CI -2.03 .. -1.26), 16/16 faster |
| 16 | 300 positions, depth 11 | 26529797 -> 25551090, -3.69 % | about -5.2 % | +1.62 % (CI +1.42 .. +1.83) counters; hyperfine -1.56 % speed (CI -1.73 .. -1.39), 0/16 faster |
| 128 | `bench 16` | 9581786 -> 10016731, **+4.54 %** | 4525375 -> 4380616, -3.20 % | +8.27 % (CI +8.07 .. +8.46) |
| 128 | 300 positions, depth 11 | 24949541 -> 22785986, -8.67 % | about -6.1 % | -2.69 % (CI -2.94 .. -2.43); hyperfine +2.90 % speed (CI +2.59 .. +3.21), 12/12 |

Instructions per node rise about 2.2 to 2.6 % (counters: `bench 16` -3.31 %
instructions for -5.71 % nodes; the 300 positions -1.51 % for -3.69 %). The
step moves nps and nodes in opposite directions, as the step file expected.
Time to depth comes out each way by workload: not a strength claim.

**The pressure ratio** (`pressure.py`, the `n=` fastchess writes per move):
- S272's run: 2.08e7 nodes per side per game (p10 1.43e7, p90 2.73e7). That
  is **39.6 overwrites per entry on the parent's 524288 entries and 26.4 on
  C's 786432**.
- S238's 32577 games: 1.82e7, giving 34.6 and 23.1.
- DEC-088's target is the list's 60 to 120. On these counts the harness was
  under it before S119 and sits about a third further under after.

### Tests and the mutant

New, `tests/test_search.cpp` "transposition table: storage":
- "a cluster holds three positions and a fourth replaces one"
- "replacement weighs depth and age together", two halves built so that depth
  alone or age alone would pick another way
- "age is a distance on the counter's cycle", with direct values and a
  65535 -> 1 wrap
- "a full cluster never refuses a new position", S271's harm
- "an entry aliased to current never refuses a new position": three deep
  entries read at age 0 one full cycle later, and a quiescence-depth store
  below all of them still lands. Rewritten after the fast check, which found
  the earlier form vacuous: it stored at the same depth as the shallowest way,
  so a refusal could not have fired.
- "a partial-key collision reads the other position's entry", the collision
  path documented, and a never-written entry not matching a zero low key
- "a Hash that is not a power of two is indexed whole": Hash 3 is 49152
  clusters; the last cluster round-trips; 120000 splitmix hashes put 30 to
  37 % above the old floor
- "the multiply's two forms agree"
- "hashfull counts the current search's entries"

New in "search: mate line completion": "a colliding table entry cannot extend
a mate line". `tests/test_engine.cpp`: the alignment and whole-request checks
in the re-stated resize case.

`tools/mutants/S119_tt_cluster.py`, prefix TC (new), run through
`tools/mutation_check.py` on fixture commit objects on no branch, made through
a temporary index; their worktrees are removed. First on `344ecb2`, the Part C
tree. Then, after the fast check, on `cec6aa6`, the final text: TC04 turned the
rewritten aliasing case red on its own
(`.tuning/coord/S119/mutation.log`; the first run is `mutation_run1.log`).
**6 of 6 killed both times**, each by its named case.

| mutant | change | bench | killed by |
|---|---|---|---|
| TC01 | age ignored | same | "replacement weighs depth and age together", "age is a distance on the counter's cycle", and `test_mate_carry` "a mate score carried across searches keeps a line that reaches it" |
| TC02 | depth ignored | moved | "replacement weighs depth and age together" |
| TC03 | age as a floored difference, the wrap read as current | same | "age is a distance on the counter's cycle" |
| TC04 | a new position refused by a current, deeper victim | moved | three storage cases, and "an entry aliased to current never refuses a new position" on its own |
| TC05 | the probe's written test dropped | same | "a partial-key collision ..." |
| TC06 | hashfull counts every written entry | same | "hashfull counts the current search's entries" |

`bench` resets the table per position, so a whole `bench` is one generation
and an age mutant cannot move it.

The fast suite's mate tests, "pruning does not hide a forced mate" and its
neighbours, are green on C. **No golden went red, so none was re-derived.**
The search-dependent goldens (the capture-mate rows, `test_mate_carry`'s
budgets) were not re-swept. S259 already records that two rows no longer
separate their mutants; it stays open.

### hashfull at the controls

Measured from self-play of C (`hashfull_game.py`): two processes a game, one
thinking at a time, so each side runs faster than it would under the
harness's concurrency 12. Per-move hashfull, median / p90 / max:

| control | game 1 white, black | game 2 white, black |
|---|---|---|
| harness 8+0.08, Hash 16 | 348 / 681 / 798; 326 / 737 / 791 | 284 / 653 / 939; 310 / 615 / 803 |
| `rating.sh` 10+0.2, Hash 128 | 63 / 182 / 496; 65.5 / 174 / 319 | 63 / 172 / 298; 63 / 155 / 303 |
| the list's 2'+1", Hash 128 | 440.5 / 790 / 887; 427 / 787 / 988 | -- |

**The accepts clause "checked to stay low at the rating control" has two
readings, and they disagree; both are recorded and neither is chosen here:**
- **At `rating.sh`'s 10+0.2, Hash 128, it holds**: median about 6 %, p90
  under 20 %, max 49.6 %.
- **At the rating list's 2'+1", Hash 128, it fails**: median about 43 %, p90
  79 %, one search at 98.8 %.

Which control the clause means is the owner's to say. At the harness's
8+0.08, Hash 16, the median is about a third and the p90 about two thirds.
The 2'+1" game wrote 4.8e8 and 5.2e8 nodes a side, DEC-088's "order of 660 M".

### Two questions for the owner (QUESTIONS, DEC-265), not settled here

1. **Should the harness `Hash` move?**
   - *Why it is asked:* DEC-088 chose 16 MB to reproduce the list's 60 to 120
     overwrites per entry. The harness's own PGNs measure 35 to 40 before
     S119 and 23 to 26 after it. Nothing in the code or the data can rule on
     this; DEC-088 is the owner's.
   - *Keep 16:* every verdict stays comparable, and this step's SPRT runs as
     pre-registered. It tests at about a third of the list's pressure, which
     flatters table-hungry changes, the thing DEC-088 meant to avoid.
   - *Lower it to about 8 MB:* about 46 to 53 per entry after S119. That
     restores the intended regime. It is a harness change, so it costs an A/A
     of 1000 games (DEC-143), and verdicts before and after stop being
     comparable.
   - *Recommendation:* keep 16 for S119's own run, which is pre-registered at
     16, and rule on the harness before the next table-sensitive step. The
     numbers are derived from these counts, not measured at 8 MB.
2. **Which control does the accepts clause "`hashfull` ... stays low at the
   rating control" mean?**
   - *Why it is asked:* the clause holds at `rating.sh`'s 10+0.2 / 128 and
     fails at the list's 2'+1" / 128, as recorded above.
   - *`rating.sh`:* the clause is met.
   - *The list:* the clause is not met. At Hash 128 a 2'+1" search fills the
     table within a single move. That argues for the list's 128 to 256 MB
     upper end in `rating.sh`, or for none of this mattering, which only a
     measurement at 256 would say.
   - *Recommendation:* read it as `rating.sh`'s, which is the tool this
     project rates with. Record the 2'+1" figure as a fact about the list's
     regime.

### Pre-registration

`adocs/data/S119_sprt.sh`, in the shape of S238's:
- CAND is C's landing commit and REF its parent, the commit that lands B,
  both committed. That suits A and B landing first, and DEC-220's block then
  names two plain shas. The script refuses until both are pinned.
- {0, 5} nElo, 8+0.08, Hash 16 (DEC-088), `noob_3moves.epd`, concurrency 12.
- Worst case 41861 games at the midpoint, 19.8 h at 2110 games/h; 25591 on a
  bound, 12.1 h. A night.
- Abort above 1.0 % forfeits on either side; a crash or disconnect voids it.
- S259 is the open DEC-171 finding.
- The prediction is labelled unsourced and gives a direction only.
- Interpretation: H1 keeps C. H0 or no verdict reverts C's commit, and A and
  B stay.
- Markers are fastchess.sh's `SPRT-RUN-DONE` / `SPRT-RUN-FAILED`; the script
  execs it.

Launch, from the main checkout after pinning:

    nohup adocs/data/S119_sprt.sh > .tuning/s119_sprt.log 2>&1 &

### Suites

On the final tree, `CLANG_FORMAT_MAJOR=22`: `cmake --build build -j12 &&
ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune
-j12 && ctest --test-dir build-tune -L fast --output-on-failure &&
./clang-format.sh --check` **exit 0: 44/44, 44/44, format clean, 0 warnings**.

Re-run after the coordinator's fast check, on the text it changed: the
aliasing case, comments in `src/search.cpp`, `tests/test_search.cpp` and
`tools/probe_cost.cpp`, and `MANUAL.md`, `DEV_MANUAL.md`, the step file and
`adocs/data/S119_sprt.sh`. The result is exit 0, 44/44 and 44/44, format
clean, `bench` 3872032, and the three prose checks at 0 flagged.

**Debug:** a Debug configure builds with 0 warnings. `gate_extra.sh`'s six
guard binaries (`test_chesso`, `test_openings`, `test_movegen`,
`test_evaluation`, `test_search`, `test_engine`) pass 6/6 with asserts live.
The Debug `bench` prints 3872032.

**Prose checks:** `tools/plan_prose_check.py` `--citations`, `--touches`,
`--params`, one mode per call: each exit 0, 0 flagged.

**Not run here, the coordinator's** (second tier, DEC-141): the four-round
Debug self-play at 4+0.04 with its `Assertion` grep, and the whole of
`tools/gate_extra.sh`. Part A touches the search's move sites and Part C the
search's table.

**Docs:**
- `MANUAL.md`: `hashfull` in "What a search prints" (example line, field
  sentence, table row), the `Hash` option row (every megabyte used), the
  known-bug entry (now only refused memory), and the S271 replacement
  paragraph (clusters, 16-bit counter).
- `adocs/specs.md`: the info-line paragraph names `hashfull`; the machinery
  row describes the clusters, the index, the replacement, the collision trade,
  the prefetch and huge pages.
- `DEV_MANUAL.md`: the harness hash row gains the measured pressure; nothing
  else there describes the table.
- `test_uci_surface`'s `expected_info_fields` gains `"hashfull"`. It was
  refreshed after both documents, and re-derived from the binary by the
  command at its site: `score cp time depth nodes nps hashfull pv`.

**The A + B tree on its own** (B's commit, exported from its staged tree):
- builds with 0 warnings and prints `bench` 4081329;
- passes 43 of the 44 fast cases;
- fails `test_plan_citations`, because an export is not a git checkout. The
  parent's export fails it the same way, and the same A + B tree made a git
  repository passes it.

The A tree alone was built, timed and INV-6-checked, but its suite was not
run apart from that.

**Per part:** the staged trees give A's commit `src/search.cpp`,
`src/transposition_table.hpp` and its specs row. B's commit gets
`src/data_structures.hpp`, `src/transposition_table.cpp` and its specs row. C
takes the rest.

### Commit texts

A (`partA.diff`):

    Prefetch the child's table slot after each move (S119)

    A child's probe waited on a cold line every time: nothing asked for it
    before the key was used. The search now prefetches the slot the moment
    make_move() or make_null_move() returns, at the four sites a child is
    searched from. make_move() stays ignorant of the table, so perft and the
    generator pay nothing and the NNUE hook's alphabet is untouched.

    Node-identical (INV-6): bench, bench 12 and tools/search_bench.py at
    depths 9 and 12 match the parent line for line. Read by DEC-266 on
    2026-10-12: -2.02 % cycles on bench 16, -2.19 % on the 300 positions,
    +2.30 % wall, each beyond the session's A/A floor. Instructions rose
    0.63 %, so the two layout perturbations decided, and both agree and
    clear it.

    No functional change

B (`partB.diff`):

    Back the table with transparent huge pages on Linux (S119)

    A probe is a random line in a large table, so each one risks a TLB miss
    as well as a cache miss. The table is now an anonymous mapping aligned
    to 2 MiB and advised MADV_HUGEPAGE. A refused advice leaves ordinary
    pages; a refused mapping, or a platform without the advice, falls back
    to calloc() aligned by hand. Nothing aborts. The kernel backed the whole
    table here: AnonHugePages 12288 kB at Hash 16, 98304 kB at 128.

    Node-identical (INV-6). Read by DEC-266 on 2026-10-12: at Hash 128,
    -3.59 / -3.61 % cycles and +4.61 % wall, with instructions unmoved and
    both layout perturbations agreeing beyond the floor. At Hash 16, -1.76 /
    -1.55 % cycles and +1.76 % wall, so no loss.

    No functional change

C (`partC.diff`), the landing, which the SPRT plays as CAND. DEC-220's block
cannot ride a landing commit, so the verdict's record commit carries it (the
template follows):

    Make the table 64-byte clusters with aged replacement (S119)

    The table was direct-mapped 24-byte entries: one colliding position
    evicted a deep entry, two in eight entries straddled a line, and Hash was
    floored to a power of two (12 MiB at Hash 16). It is now three 20-byte
    entries per aligned 64-byte cluster, indexed by a multiply, so all 16 MiB
    are used: 786432 entries, +50 %. Another position's store replaces the
    way worth least, depth less two plies per search of age. Age is a
    distance on a 16-bit cycle, so a wrap cannot make an old entry read as
    current (S271). Keys are 32 bits; the move and the mate line are guarded
    against a collision, the score is not (DEC-258). info lines carry hashfull.

    bench -5.13 %; at Hash 16, about 4 to 5 % fewer nps for 3.7 to 5.7 %
    fewer nodes to depth. adocs/data/S119_sprt.sh pre-registers the SPRT:
    layout, replacement and capacity priced together (DEC-258), at 23 to 26
    overwrites per entry where the parent ran 35 to 40.

    Bench: 3872032

The verdict's record (the coordinator fills it in; H0 or no verdict reverts
C's commit per the pre-registration):

    Record S119's <H1|H0|no verdict>: the cluster table <...>

    <one paragraph: the verdict, its Elo beside the unsourced prediction>

    SPRT | cand <C sha> vs ref <B sha>, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
    Elo | <x> +/- <y>, nElo <x> +/- <y>
    LLR | <l> (-2.94, 2.94) -> <H1|H0|none>
    Games | N: <n> W: <w> L: <l> D: <d>, Ptnml [<5>]
    Wall | <h> h <m> m, <g> games/h, forfeits <f>
    Log | adocs/data/S119_sprt.log
