id:         S118
goal:       the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
accepts:    an SPRT verdict if the tree changes and a node-identity check if it does not -- state which before the run; the key is a pawn-only zobrist maintained incrementally in add_piece, remove_piece and move_piece (INV-4), never recomputed in evaluate(); the hit rate is measured over a real search and recorded, not assumed; the table stores the mg/eg pawn score, the passed-pawn bitboard and **both kings' shelter and storm scores**, because the shelter is the expensive half of what this saves; a collision returns a recomputation rather than a wrong score, and a test forces one
touches:    src/evaluation.cpp, src/evaluation.hpp, src/data_structures.hpp, src/search.cpp,
            tests/test_evaluation.cpp, tests/test_eval_trace.cpp, tests/test_search.cpp,
            tools/mutants/S118_pawn_cache.py, adocs/data/S118_inv6.txt,
            adocs/data/S118_cost.sh, adocs/data/S118_cost.py, adocs/data/S118_cost_reading.txt,
            adocs/data/S118_hitrate.py, adocs/data/S118_hitrate.txt,
            adocs/data/S118_mutants.txt, adocs/data/README.md,
            DEV_MANUAL.md, MANUAL.md; a stale comment
            each in src/chesso.cpp, tools/datagen.cpp, tools/tuner_trace.hpp,
            tests/test_engine.cpp, tests/test_audit_fen_semantics.cpp, and a symbol
            in one comment of src/bitboard.cpp: no code there
excludes:   new pawn terms, which are S125; the passed pawn suite, which is S123; storm, which is S122 (DEC-332 (5))
decisions:  DEC-071, DEC-087, DEC-046, DEC-083, DEC-141, DEC-266, DEC-332
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-01 00:03 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against 85b7847: bench 5707449, bench 12 1861982, tools/search_bench.py at depths 9 and 12, every stream identical in both builds at both commits (adocs/data/S118_inv6.txt) and over the eight binaries of the timing. No SPRT owed (DEC-332 (6), DEC-083); the timing is the verdict, DEC-266, 16 interleaved pairs a comparison, the session's floors 0.603, 0.087 and 0.255 (adocs/data/S118_cost_reading.txt, 2026-10-31, kernel journal clean, governor powersave, recorded and not set): DEC-332 (7) chooses s12, 2^12, -6.260 % and -6.674 % cycles, -6.501 % wall, -8.232 % and -8.838 % instructions, and (8) SHIPS it; `total`, what lands, -6.287 %, -6.542 %, -6.367 %, -9.009 %, -9.633 %; `slot` reads zero and stays for S122 by (5). About +9.7 Elo at long time control as DEC-083's conversion, not a verdict, and no game was played. Hit rate over a real search 89.7 to 96.4 %, a king's slot 70.5 to 74.6 % (adocs/data/S118_hitrate.txt). The accepts' fourth clause holds only as DEC-332 (3), (5) rule: storm is S122's, nothing computing one; its other four hold, the second's INV-4 citation being stale for INV-2. Debug self-play on the final tree (DEC-141): four rounds of fastchess at 4+0.04, 8 games, 0 Assertion, 0 disconnect, .tuning/coord/S118/debug_selfplay/. gate_extra: GATE-EXTRA-DONE 6 stages 1467 s on 654ab89, and nothing under src/ or tests/ moved after it. 24 of 24 mutants killed (adocs/data/S118_mutants.txt), PT01 to PT13 on 40831fa, PT14 to PT24 on f36762f, src/ and tests/ the same at both; the observed red and the guards in "phase B". Fast suites 50 of 50 in both builds on that tree, each mutant run's baseline among them. No golden moved (DEC-142). From CPW's description, size DEC-105 form (a); DEC-221 does not bind. MANUAL.md checked, no change; DEV_MANUAL.md checked, two sentences corrected. Shas are branch s118's: 2f7b08a the cache, 654ab89 the slot; the coordinator's record maps the landing's. bench_eval is not the bar and was not taken (DEC-332).

## Corrected 2026-10-31, DEC-332: what the design pass found stale here

Read this file below this section with these corrections; DEC-332 holds the
rulings and the measurements behind each.

- **The accepts' "storm".** Nothing computes one anywhere in `src/`, so there is
  nothing to cache: the table holds both kings' **shelter** scores, and storm is
  S122's to add.
- **The key is pawn-only**, as the accepts says, and section 2's "a pawn-and-king
  key" is wrong: at 2^12 entries a pawn-and-king key hits 80.9, 86.7 and 80.9 %
  against 89.7, 96.4 and 94.1 % (`bench 16`, S020's positions, 20 replayed games).
  A king move touches the shelter slot's tag, not the entry.
- **The key's maintenance is done.** `board_t::pawn_hash` landed with S099 and was
  relanded by S286; no primitive is touched here, `src/bitboard.cpp` leaves
  `touches:`, and `src/search.cpp` joins it. The key's wording is in INV-2, and
  INV-4 holds the accumulators; this file's citations of INV-4 for it are stale.
- **"The expensive half."** The shelter is 1.63 % of instructions over `bench`
  against about 9 % for what reads pawns alone, about a sixth of it, and the lazy
  shortcut skips it on 29 to 39 % of calls.
- **"One move in eight."** Over the same workloads the pawns differ from the
  previous `evaluate()` call's on 43 to 55 % of calls.
- **"95 to 99 %."** Here, at a few K entries: 89.7 to 96.4 %.
- **"Four bitboard fills plus per-pawn work."** S123 and S125 have grown that to
  12.69 % of instructions, 554 per call.
- **DEC-046 is the same cache, built and discarded**, and this file did not cite
  it. DEC-332 (1) re-opens it.

## Corrected 2026-10-31, S125's close: what a pawn key can hold, and the baseline

The accepts' "the mg/eg pawn score" is not all pawn-keyed. **S125's whole family
reads pawns alone** (DEC-331 (9)), as do the flats, and S123's candidate passers,
which DEC-329 says; S123's free and safe increments and both kings' distance slopes
read pieces and kings, so they stay outside the table or are stored apart. Say which,
with the key's contents, before any code.

**The baseline, measured:** `evaluate()` 75.93 ns a call on S125's closed tree,
against 63.99 before its family (`adocs/data/S125_bench_eval.txt`, one layout, ten
compiled-in positions, deciding nothing); whole-engine rows `cand` +3.50 % and p1,
the fold's, +0.37 % instructions, read zero (`adocs/data/S125_cost_reading.txt`).
A cache's value is read against those, not against the 53.90 ns of 2026-08-19.

## Moved out of the speed block, 2026-08-19, DEC-087

The published record has a warning, and S186 traced it to what it actually
says. An implementer measured a **10 % slowdown** from a pawn hash table, and
the published diagnosis is **an oversized table** -- about 17 MB against a
16 MB L3 -- and not a pawn evaluation too cheap to be worth caching
(TalkChess t=72195, xr_a_y, 2019-10-28; section 1 quotes it, DEC-203 records
the correction). So the figure is a **sizing constraint** on this step, and
the reason the step moved out of the speed block is its own reasoning rather
than that number: chesso's three pawn terms share four bitboard fills and are
cheap by construction (S027), so a cache of them saves little until S123 and
S125 have made the pawn evaluation worth caching. This step therefore lands in
the evaluation block after those two, and directly before S122 reads the
shelter and storm slots it adds. The hit-rate expectation is sourced too: CPW
*Pawn Hash Table* states that a few thousand entries give hit rates above
**95 to 99 %** (https://www.chessprogramming.org/Pawn_Hash_Table), and it
states **no speed and no Elo figure at all**.

## What it costs today

`evaluate_cheap()` calls `evaluate_pawns()` on **every** evaluation, and that
function does four bitboard fills plus per-pawn work. The figures this paragraph
was written on -- 83.35 ns a call, 12.0 M calls a second, against a search at
5.8 M nodes a second -- were **measured 2026-08-19 before S104** added the
architecture flag, and DEC-083's rule is that nothing taken on the unflagged
binary is comparable with anything taken after it. On the shipping `bmi2`
target `bench_eval` reads **53.90 ns a call, 18.6 M calls a second**
(2026-08-19, S104; `adocs/specs.md`). Either way the conclusion is the one the
ratio carries and not the absolute: the evaluation is a large fraction of the
clock, and the pawn structure is recomputed for a structure that changes on
perhaps one move in eight.

What the cache is worth elsewhere: Berserk pull request #73 measured
**+14.97 +/- 7.80** at 8.0+0.08s (section 1), at an engine whose pawn terms
were already expensive. The Elo figure and the speed-up percentage this
paragraph used to quote carried no source through two searches and are deleted
(DEC-203). Hit rates above 95 % are the wiki's, as above -- and the accepts
asks for this engine's own hit rate measured over a real search anyway, which
is the number that decides anything here.

**This is not the pattern INV-4 forbids.** INV-4 is about terms rebuilt from
the bitboards at every node; this is a cache keyed on a hash that only moves
when a pawn does. It composes with accumulation instead of replacing it, and it
is what makes S125's richer pawn terms affordable.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The 10 % slowdown is located, and its published diagnosis is not the one
this file used to give it.** TalkChess thread "Pawn hash table, a little
disappointment" (https://www.talkchess.com/forum3/viewtopic.php?t=72195,
xr_a_y / Vivien Clauzon, 2019-10-28, fetched 2026-09-13): "But this is a
slowdown of about 10% in knps!" -- with the table "well used", ten reads per
write. The thread then diagnoses it, and the diagnosis is **cache size, not a
cheap pawn evaluation**: the table was about 17 MB against the machine's 16 MB
L3, the author answered "Indeed larger than L3 cache right now, around 17Mb
(my i9 has 16Mb cache). I'll try a smaller one", and a 4 MB table is what he
tested next. hgm's reply in the same thread argues the opposite of this file's
reading: "the only cost is the update of the Pawn key plus the lookup. It
would be really strange if that would take more time (when the lookup comes
rom cache) than the Pawn evaluation."

So the figure is real and sourced, and **the mechanism DEC-087 (h) attached to
it is not what the source says.** What the source supports is: *a pawn hash
table sized past the last-level cache is a net slowdown however good its hit
rate.* That is a sizing constraint on this step, and it is a sharper and more
actionable warning than the one it replaces. The order-of-the-plan argument --
a cache of a cheap computation cannot save much -- stands on its own reasoning
and on chesso's own `bench_eval` figures below; it no longer has, and never
had, this figure behind it. DEC-203 records the correction: this file and
`adocs/plan.md`'s block-3 sentence both cite the slowdown as an oversized
table, and DEC-087 (h) is not amended because the order it supports does not
move.

**What the cache is worth, traced.** Berserk pull request #73, "Pawn hash
table", merged 2021-04-30: **+14.97 +/- 7.80** at 8.0+0.08s, Threads=1,
Hash=8MB, bounds [-1.00, 4.00], 3344 games
(https://github.com/jhonnold/berserk/pull/73). That is the figure this step
should be read against -- one engine, one patch, a real SPRT, at the same time
control this project measures at. Ethereal's "Include King in the 'Pawn' eval
hash (speedup)" (commit 97af7ea216fdf983fab4ac1ea2385d22015137f8, 2018-02-23,
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+pawn+hash)
reports LLR and game counts and **no Elo figure**; it is the record that the
king's shelter belongs in the same table, which is what this step's accepts
already requires.

**The Elo figure and the speed-up percentage this file used to quote are
deleted (DEC-203).** Searched 2026-09-04 by the plan review: CPW *Pawn Hash
Table* carries neither. Searched this pass: a web search for them; a GitHub
pull-request search across C engines for "pawn hash" in the title, which
returned Berserk #73 (+14.97), RubiChess #208 (an allocation change, no
figure) and prophet #65 (no body); the Ethereal pawn-hash commit above. Not
located. Berserk's +14.97 replaces them and is better evidence, so nothing is
lost.

**The hit rate is the one figure the wiki states and it is unchanged.** CPW
*Pawn Hash Table* (https://www.chessprogramming.org/Pawn_Hash_Table, fetched
2026-09-13): "the number of cached entries required might be quite low (a few
K) to get a sufficient hit rate above 95% or even 99%". **No speed figure and
no Elo figure anywhere on the page.** Note what that sentence and the TalkChess
thread say together: *a few thousand entries*, which is tens of kilobytes, not
megabytes -- the sizing this step should start from and the opposite of the 17
MB that produced the slowdown.

### 2. Shape for chesso

**Departure from the wiki's form, stated:** CPW describes a table caching
"pawn-structure evaluation keyed by the pawn hash". This step's table also
stores **both kings' shelter and storm scores**, which makes the key a
pawn-and-king key rather than a pawn key -- a king move must invalidate the
entry. Ethereal's 97af7ea2 is the record that the same choice was made
elsewhere; the wiki does not describe it. The accepts already says the shelter
is "the expensive half of what this saves", and this is where the key's width
is settled.

Sites: `src/evaluation.cpp` `evaluate_pawns` is what the entry replaces;
`src/bitboard.cpp` `add_piece`, `src/bitboard.cpp` `remove_piece` and
`src/bitboard.cpp` `move_piece` are the three primitives the key is
incrementally maintained in (INV-4), and `src/data_structures.hpp` is where the
key lives on the board.

### 3. Implementation sketch

- The key is maintained in the three primitives and **never recomputed in
  `evaluate()`** -- that is the invariant, and it is the same alphabet an NNUE
  accumulator would be updated from.
- The table is sized from the wiki's "a few K" and measured, not guessed, and
  the step records the size it chose beside the hit rate. The TalkChess thread
  is the reason a size sweep is part of this step and not an afterthought.
- A collision returns a recomputation rather than a wrong score, and a test
  forces one -- the accepts is already explicit and the test is written before
  the cache is trusted.
- The hit rate is measured over a real search and recorded.

### 4. Constants and seeds

Two constants and both are declared here:

- **Table size in entries.** DEC-105 form **(a)**, a value from a publication
  about the technique with its URL: CPW *Pawn Hash Table*'s "a few K" entries
  (https://www.chessprogramming.org/Pawn_Hash_Table). The step starts there,
  sweeps, and records the size that wins. **Bounded above by the last-level
  cache**, on the TalkChess evidence: a table that does not fit is a measured
  net slowdown. No engine's shipped pawn-table size seeds this (DEC-105,
  DEC-134).
- **Entry layout.** Not a seed -- it follows from what the accepts stores.

### 5. Pitfalls

- **Size past the last-level cache is the documented failure.** See section 1.
  The sweep's upper end is informative, not a candidate.
- **This is not the INV-4 hazard and it is also not exempt from it.** The cache
  composes with accumulation; the *key* must be accumulated or the step
  reintroduces exactly what INV-4 forbids.
- **A wrong score from a collision is silent.** It changes play without
  changing any node count in an obvious way, and the forced-collision test is
  the only thing that catches it before an SPRT does.
- **The verdict type must be stated before the run.** If the cached value is
  bit-identical to the recomputed one the tree does not change and node
  identity is the check; if the shelter is restructured on the way in, it does
  and an SPRT is owed. The accepts says "state which before the run" and this
  is the sentence that matters.
- **The figures in "What it costs today" are architecture-flagged.** DEC-083:
  nothing measured before S104 is comparable with anything after it. The
  53.90 ns / 18.6 M calls a second reading is the `bmi2` one and is the
  baseline.

### 6. Measurement

Hit rate over a real search, recorded. Then either node identity
(`tools/search_bench.py`, INV-6) or one SPRT at the S105 regime with bounds
stated in advance and the nElo worst case (DEC-143) -- decided before the run,
not after. The expectation from the record is Berserk's +14.97 class **at an
engine whose pawn terms were already expensive**; chesso's own baseline is
S125's measured per-call cost, and that is what the verdict is read against.

### 7. Interactions

- **S123 and S125 (before)**: they are what make the pawn evaluation worth
  caching, and S125's measured per-call cost is this step's target.
- **S122 (immediately after)**: reads the shelter and storm slots this step
  adds. The order is stated in both files.
- **S120 (evaluation cache)**: a different table at a different key; the two do
  not replace each other and the step says how they divide. [2026-10-06: S120
  retired the cache (DEC-257); there is nothing to divide.]
- **S104 (before, done)**: the architecture flag every speed figure here is
  taken under (DEC-083).
- **S029 (parked)**: the same three primitives are the NNUE hook; a key
  maintained badly here is a defect an accumulator would inherit.

### 8. References

- - https://www.talkchess.com/forum3/viewtopic.php?t=72195 -- "Pawn hash table,
  a little disappointment", xr_a_y (Vivien Clauzon), 2019-10-28: "But this is a
  slowdown of about 10% in knps!"; the 17 MB against a 16 MB L3 diagnosis;
  hgm's "the only cost is the update of the Pawn key plus the lookup". Forum
  posts only. Fetched 2026-09-13.
- - https://github.com/jhonnold/berserk/pull/73 -- "Pawn hash table", merged
  2021-04-30, +14.97 +/- 7.80 at 8.0+0.08s Hash 8MB, bounds [-1.00, 4.00],
  3344 games. Pull-request body only. Fetched 2026-09-13.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+pawn+hash
  -- commit 97af7ea2, 2018-02-23, "Include King in the 'Pawn' eval hash
  (speedup)"; LLR and game counts, **no Elo figure**. Commit message only.
  Fetched 2026-09-13.
- - https://www.chessprogramming.org/Pawn_Hash_Table -- "a few K" entries for a
  hit rate "above 95% or even 99%"; **no speed figure and no Elo figure**.
  Fetched 2026-09-13.

## 2026-10-31: phase B, the cache

On branch `s118`, not on `achesso`: DEC-332 (9) has the timing come first and (8)
decides. **From the description (DEC-221):** the technique is CPW *Pawn Hash Table*'s;
no engine's source or table was read, and the size starts from that page's "a few K".
The step's verdict type, stated before any run (DEC-332 (6)): bit-identical, INV-6,
no SPRT.

**What landed.** `evaluate_pawns()` split in two. `pawn_pass()` takes the two pawn
bitboards as its only inputs, so the signature is what says an entry keyed on them is
exact, and fills a `pawn_entry_t`; `outside_pawn_terms()` reads an entry and adds piece
placement and S123's A and B at every evaluation. `evaluate_pawns()` stays as the
collecting form the accessors call. `cheap_score()` is stage one over an entry, and
every form comes through it: the pure `evaluate_cheap()` fills an entry nobody keeps,
so the pass the tests and the tools hold to account is the one that fills the table.
`pawn_entry_of()` probes: slot from `board_t::pawn_hash`, verifier both bitboards. The
table is a value member of `search_state_t`, last, 128 KiB at `PAWN_TABLE_BITS` 12,
taking the struct from 1.21 to 1.34 MiB. `evaluate(board, table)` and
`evaluate_lazy(..., table)` are the only forms that read it, from `negamax_at`'s
`raw_eval` and from `quiescence`; `search.cpp`'s `MAX_PLY` return stays pure, being
rare. In the trace build both forms are the pure ones. Tune build: a census of hits and
misses, `CHESSO_PAWN_TABLE_CENSUS`. Debug: every hit asserted against a fresh pass.

**Observed red first.** Against a stub of the two forms that left the table untouched:
7 of 8 cases of "evaluation: pawn table" and "a search's tree does not depend on what
its pawn table holds", each at a read of the slot. The eighth, "an empty entry is the
pawnless board's own", and the trace case pass against that stub by construction, and
go red under PT08 and PT12. The precondition `outside_counts_differ` caught one pair
written wrong while the guards were being built: e8 and a8 are the same distance from
a4. In Debug, with one wrong store planted after the fill (`entry->score += 1`): `bench
4` aborts on the `pawn_entry_is_fresh` assertion, 134.

**Guards.** `tests/test_evaluation.cpp`, suite "evaluation: pawn table": the forced
collision (the same 64-bit key planted on another structure, each pair sharing one
colour's pawns, both orders); every test position twice through one table; "a hit
answers from the entry" (Release only: a planted score, which a table that recomputed
would never show); nine pairs of one structure with other pieces, in both orders and
both colours, each checked to differ in a count outside the entry; the mirror through
one table, two passes; both sides to move on one entry; the pawnless zero entry; the
lazy form's three paths. `tests/test_search.cpp`, suite "search: pawn table": a search
from a table of stale entries against a search from an empty one, nodes, move and
score; each of the two call sites reading the table, on a position where nothing else
could have written the slot. `tests/test_eval_trace.cpp`: the search's form records
what the pure form records, twice, and leaves the table empty.

**Mutants** (`tools/mutants/S118_pawn_cache.py`, `tools/mutation_check.py`, on a private
clone holding the staged tree as one commit, the tool refusing anything but a clean
linked worktree): **15 of 15 killed**, in 2490 s. DEC-141's clause (2) does not bind, no
new pruning rule; DEC-332 runs them anyway. PT03, the verifier cut to a stored piece of
the key, is the one only the planted collision sees among the suite's guards: it moved
the bench on this fixture, whose log is not kept, and reads `bench same` on the final tree,
1 of 50 (`adocs/data/S118_mutants.txt`). PT05, PT12, PT14 and PT15 leave the bench where it
was, on this fixture and on the final tree. One case each caught them here; on the final
tree one test of the 50 does, the case named in the log for each. PT06 left the bench too on this
fixture, which had no slot, and moves it on the final tree, 3 of 50. It moves the index in
`pawn_entry_of()` and not the one `pawn_shelter_of()` writes for itself, so a filled entry
empties a slot stage two does not read, and the slot stage two does read keeps another
structure's tags: a wrong shelter for the same kings. Read from the code, not isolated.
PT08 goes red on all nine pairs of the same-pawns case in all four orders, read from its
log, so each pair bites on its own.

**INV-6**: `adocs/data/S118_inv6.txt`, both builds, identical to `85b7847`. Bench 5707449:
`No functional change`.

**Instructions, a count and no timing** (`build/tools/perf_counters`, its cycle and wall
fields not read): `bench 16` 26452862216 -> 24275319795, **-8.23 %**; S020's positions at
depth 11 84510982917 -> 77042207471, **-8.84 %**. The design pass's prototype read -8.42 %
and -8.87 %; it skipped the outside terms' family calls when the entry held no passer and
this form does not, the likely reason for the 0.19 and 0.03 points, inferred and not
isolated. Nothing here says what the cycles will read: DEC-046's cache hit 89.8 % and read
zero. The tune build's census agrees with the design pass's simulated table: `bench`,
2963238 probes, 89.703 %.

**Gates**, on the staged tree: both builds 0 warnings; `ctest -L fast` 50 of 50 in `build/`
and `build-tune/`, `CLANG_FORMAT_MAJOR=22`; `./clang-format.sh --check` clean; the S297 clang
pair, `STAGES=clang tools/gate_extra.sh`, 50 of 50 twice, 307 s, after it found
`pawn_entry_of` unneeded in the trace build (`-Wunneeded-internal-declaration`, a discarded
`if constexpr` being no use to clang), now `[[maybe_unused]]`; `tools/plan_prose_check.py`,
one mode per call, each exit 0, `--citations` and `--touches` 0 flagged over 17 files.
`MANUAL.md` checked: its new-game paragraph gains the table among what is per `go`.
`DEV_MANUAL.md` is checked with the scripts it will name. `gate_extra`'s other stages and the
Debug self-play bind by `search.cpp` (DEC-141) and are taken on the final tree.

## 2026-10-31: phase B, the shelter slot

Its own commit and its own timing row, DEC-332 (5).

**What landed.** `pawn_shelter_t`, 16 bytes, parallel to the entries: a packed score and
a tag per king, the tag being the square plus one and 0 being empty. `pawn_entry_of()`
empties it when it fills the entry, and that is the pawn half of its verifier, so stage
two reads it only after stage one has read the entry: `pawn_shelter_of()`, called past
the lazy shortcut, and two statements in `evaluate(board, table)` because `+` does not
order its operands. `evaluate_mobility_and_king_safety` takes the slot. The pure forms
and the two accessors hand it an empty one nobody keeps, so every king misses and the
features are read, traced and collected as before; there is one instantiation and no
branch on a null. `lazy_from_cheap()` takes stage two as a callable, so a shortcut node
never touches the slot's line. The trace gate moved from `cheap_from_table()` to the two
forms, which now hold both stages. The table is 192 KiB at `PAWN_TABLE_BITS` 12, the
struct 1.40 MiB, the census gains `slot_hits` and `slot_misses`, per king, and the "1.3 MiB"
this step's first commit wrote in seven comments reads 1.4.

**Bit-identical**, as the cache: the slot's score is the integer the features would have
added, before the sign, the clamp and the one taper. `adocs/data/S118_inv6.txt` carries the
row, both builds. The slot holds what is before the clamp, so `LazyEvalMargin` set in the
tune build stales nothing, as no entry field does.

**Observed red first.** Against a stub of both forms calling the pure stage two: "only stage
two fills the shelter slot", "a replaced structure takes its shelter with it", "an empty
slot is not a king's on a8", "a slot's hit answers from the slot". "A king that moved reads
its own shelter" passes the stub by construction and goes red under PT18 and PT19. A fixture
was wrong on the way: a rook on a1 gives check to a king on a8 with White to move, which
`load_FEN` refuses, and the guard's first run said so. In Debug, a wrong store planted in
the fill: `bench 4` aborts on `shelter_is_fresh`, 134.

**Guards.** The five cases named, in "evaluation: pawn table". The stale-table search case
gains slots whose tags are this position's kings on scores no count produces, which only
emptying a slot with its entry defeats. The trace case gains the lazy form and an empty slot.
The cases over the whole list run both stages through the slot already.

**Mutants** PT16 to PT24, on a private clone holding the tree as one commit: **9 of 9 killed**.
PT20 was stillborn on its first run, `sign` left unused under `-Werror`, and carries a `(void)`
now, the tool's documented form. PT17, PT21, PT22, PT23 and PT24 leave the bench where it was,
and what catches PT21, PT22, PT23 and PT24 is one case each. PT24 reads the trace build's lazy
form, which PT12 does not. The whole list PT01 to PT24 is run again on the final tree.

**Instructions, a count and no timing:** `bench 16` 24275319795 -> 24069772518, **-0.85 %**
on the cache and **-9.01 %** on the parent; S020's positions 77042207471 -> 76370063958,
**-0.87 %** and **-9.63 %**. `bench`'s census: 2963238 probes at 89.703 %, 3615594 of a king's
slot at 74.441 %, which is the design pass's simulated figure. Debug `bench 11`, every hit
checked: 45007823300 instructions against 44858231746 on the parent, **+0.33 %**, since a
checked hit recomputes what the parent computed anyway.

**Gates**, on the staged tree: both builds 0 warnings; `ctest -L fast` 50 of 50 in both;
format clean; the S297 clang pair 50 of 50 twice, 365 s; `plan_prose_check.py`'s five modes
each exit 0.

## 2026-10-31: stopped for a shutdown (the coordinator)

The owner stopped the work at about 14:40 to shut the workstation down, before the subagent's
final report. What stands on `654ab89`, read from its logs:

- **`tools/gate_extra.sh`: `GATE-EXTRA-DONE 6 stages 1467 s`** on this tree, 13:41 to 14:06. Nothing
  under `src/` or `tests/` moved after it.
- **The mutants on this tree: PT01 to PT13 killed, 13 of 13, none survived; PT14 to PT24 not re-run
  on it**, the run stopped by the owner during PT14. Each of those was killed on its own earlier
  fixture, in the 15 of 15 and the 9 of 9 above, and those fixtures predate small edits, so this
  still owes `python3 tools/mutation_check.py tools/mutants/S118_pawn_cache.py <clean worktree>
  --only PT14 ... PT24`, about 30 min. PT07 took 554 s against about 145 s for the others. (Added
  later that day: that run has been taken, on `f36762f`, 11 of 11 killed; "resumed" below.)
- **`adocs/data/S118_cost.sh check`** passes at `CACHE_SHA=2f7b08a SHELTER_SHA=654ab89`: rows aa, s8,
  s10, s12, s14, s16, s18, total, slot. Its `build` and `run` have never run, nor has
  `adocs/data/S118_hitrate.py` on a real search. **No timing exists for this step.** (Added later that
  day: `build` has now run, "resumed" below; `run` and the hit rate have not, and still no timing exists.)
- **Owed by the subagent and not delivered:** the `specs.md` proposal (INV-2 or INV-4 if they name the
  key's readers, the evaluation row, the absent row's "a pawn hash"), and a re-read of `DEV_MANUAL.md`
  and `MANUAL.md` at the landing.

(Added at the close: all of this is discharged, in "the close" below. PT01 to PT13 were run again
on `40831fa`, with their kill table.)

## 2026-10-31: resumed

After the shutdown the coordinator ran the rest of the mutants, `build` and a fast check. Written
on `f36762f`'s tree, before any timing.

**Mutants: 24 of 24 killed, none survived**, `adocs/data/S118_mutants.txt`. PT01 to PT13 on
`654ab89`, PT14 to PT24 on `f36762f` (11 of 11, 1927 s); `git diff --stat 654ab89 f36762f -- src
tests` prints nothing. Two rows no longer read as "phase B, the cache" wrote them, and it now says
so: PT06 moves the bench and PT03 does not. PT01 to PT13's run was stopped before its kill table, so
its words are the progress line's, which says `same` also for a bench the tool could not read, where
the table says `n/a` (`tools/mutation_check.py`). That matters at one row. PT07 misdirects the slot's
emptying as PT06 does, so by the same reading its bench ought to move, and it reads `same`, in 554 s.
Either the bench could not be read, or the reading of PT06 is incomplete; `--only PT07` would say,
about 10 min, and was not run here. Its kill stands either way, 10 of 50. (Added at the close: the
bench could not be read. PT01 to PT13 were run again on `40831fa`, 13 of 13 at the same counts, and
the table reads PT07 `n/a`, no bench line, SIGABRT. The reading of PT06 is not in question.)

**`adocs/data/S118_cost.sh build`, the first known to the record**, 2026-10-31 20:44:07, 91 s to the
marker, `S118-COST-BUILD-DONE`: `.tuning/s118/build.console`, `.tuning/s118/cost/cost.log`. Loadavg
0.62, governor `powersave`, recorded and not set (DEC-195). **INV-6 passed**: `INV-6: bench 5707449
nodes, every binary identical`, over base, s8, s10, s12, s14, s16, s18 and shelter. Read back: each
binary's four streams `diff -r` clean against base's, and base's md5s are the four in
`adocs/data/S118_inv6.txt`. Eight distinct `.text` sections. The code check against s12 refused
nothing and printed:

| row | functions | `evaluate` | `evaluate_lazy` | the mask | struct | against s12 |
|---|---|---|---|---|---|---|
| s12 | reference | 465 | 501 | `and $0xfff,%eax` | `0x156300` | |
| s8 | 7 | 464 | 500 | `movzbl 0xc0(%rdi),%eax`, for that `and` and the 64-bit load | `0x138300` | -120 KiB |
| s10 | 7 | 465 | 501 | `and $0x3ff,%eax` | `0x13e300` | -96 KiB |
| s14 | 7 | 465 | 501 | `and $0x3fff,%eax` | `0x1b6300` | +384 KiB |
| s16 | 7 | 464 | 500 | `movzwl 0xc0(%rdi),%eax`, for that `and` and the 64-bit load | `0x336300` | +1920 KiB |
| s18 | 7 | 465 | 501 | `and $0x3ffff,%eax` | `0x936300` | +8064 KiB |

The seven at every size: the two forms, `iterative_deepening_search` and its `.cold`,
`leaf_walker_t`'s constructor and its `.cold`, and its destructor; those last five keep their counts,
1721, 72, 40, 11 and 14. The struct's moves are the table sizes `adocs/data/S118_cost.sh` names, less
s12's 128 KiB. So its header's sentence held and stands, as does `SIZE_READERS`. The comment over
`SIZE_READERS` said "the same instruction counts", false for the two forms at s8 and s16, and is
corrected; this paragraph is the reading it cites.

**The fast check's four findings, and what became of each.**

1. `read`'s `chosen = within[0]` raised IndexError when no candidate lay within both floors, which
   DEC-332 (7) did not cover. Fixed by the coordinator's ruling, DEC-332 (10), stated in the
   pre-registration and over the code. Observed red first, on synthetic pairs outside the tree: `read`
   takes its directory as its argument, and `S118_cost.sh` is what passes it `$OUT/pairs`, so the run
   never saw `.tuning/s118/cost/`. Rows 4, 3, 2, 3 and 4 points of larger excess: an IndexError at
   `f36762f`, and now `size: no candidate lay within both floors; larger excess s8 4.000, s10 3.000,
   s12 2.000, s14 3.000, s16 4.000 points; DEC-332 (10) chooses s12`, then the bar on s12. An exact tie
   of s10 and s14 chooses s10. The normal path's output is byte-identical.
2. DEC-141's Debug self-play was on no owed list. It is on the one below.
3. PT06's stated class was false on the final tree. The mutant file's sentence no longer names it,
   and "phase B, the cache" says what each mutant reads, PT03's clause included. No `old` or `new`
   text moved.
4. The record of `build` and of the mutants was missing. It is the two paragraphs above and
   `adocs/data/S118_mutants.txt`.

**Still owed, in order:**

1. The timing: `CACHE_SHA=2f7b08a SHELTER_SHA=654ab89 adocs/data/S118_cost.sh run`, about 1 h 50 m, the
   machine idle, the coordinator.
2. The hit rate: `adocs/data/S118_hitrate.py` on a real search, about 10 min, the coordinator.
3. The reading: DEC-332 (7), or (10) where it binds, then (8) and (5).
4. The landing, on (8)'s credited gain, with DEC-141's Debug self-play on the final tree: four rounds
   of `fastchess` at 4+0.04, the log grepped for `Assertion`, run by the coordinator at the landing,
   and the stamp says so. It binds through `src/search.cpp` (DEC-332's Consequences) and has not been
   run; `gate_extra` above is a different gate.
5. The `specs.md` proposal, as the stop above lists it.
6. The re-read of `DEV_MANUAL.md` and `MANUAL.md`.

(Added at the close: 1, 2, 3 and 6 are done, in "the close" below, and so is 4's Debug self-play. 5 went to
the coordinator, which owns `specs.md`, and 4's landing is the coordinator's.)

## 2026-11-01: the close

**The verdict: S118 ships, by DEC-332 (8) and no SPRT.** The run is 2026-10-31 21:55:22 to 23:42:50,
`adocs/data/S118_cost_reading.txt`. `bench_eval` is not the bar and was not taken: it calls the pure
form, which has no table (DEC-332). Cycles and wall, 16 interleaved pairs a comparison, percent against
the parent `85b7847`, CIs paired:

| row | `bench 16` cycles | pos11 cycles | pos11 wall | `bench 16` instructions | pos11 instructions |
|---|---|---|---|---|---|
| A/A | +0.330, +0.057 .. +0.603 | +0.017, -0.052 .. +0.087 | -0.061, -0.255 .. +0.133 | 0.000 | 0.000 |
| **floor** | **0.603** | **0.087** | **0.255** | | |
| **s12, chosen** | **-6.260**, -6.381 .. -6.139 | **-6.674**, -6.792 .. -6.555 | **-6.501**, -6.653 .. -6.349 | -8.232 | -8.838 |
| `total`, what lands | -6.287, -6.375 .. -6.199 | -6.542, -6.607 .. -6.477 | -6.367, -6.473 .. -6.261 | -9.009 | -9.633 |
| `slot`, against s12 | +0.055, -0.059 .. +0.169 | +0.144, +0.025 .. +0.263 | +0.230, +0.027 .. +0.433 | -0.847 | -0.872 |

**The size, DEC-332 (7): s12**, the smaller of the two within both floors, s12 and s14. s10 is out on
pos11 alone, 0.152 points from the best against the 0.087 floor, and s8 and s16 are out too; (10) did not
bind. s18 is informative: -8.918 % instructions and -4.353 % cycles on pos11. The table is 128 KiB of
entries and 64 KiB of slots, 192 KiB, and the prototype's -8.42 % and -8.87 % instructions read -8.232 %
and -8.838 %.

**The shelter, DEC-332 (5): the slot stays**, reading zero, S122 being the reason. Its instructions
fall and its cycles and wall do not; pos11's and wall's CIs exclude zero, inside the floor, and it buys
nothing measured here.

**Checked before it was recorded.** `S118_cost.py read` re-run on the pairs,
`.tuning/coord/S118/reading_recheck.txt`: byte-identical to `reading.txt`. Six rows recomputed from the
raw counters with no project code: every figure agrees. (7), (8) and (5) applied by hand from the printed
figures, the table in the reading: each as DEC-332 states it. The floors are the A/A's largest CI bound
per workload and instrument, DEC-266 (3). INV-6 passed in this run over eight binaries, the streams read
back, base's md5s those of `adocs/data/S118_inv6.txt`; `bench 16` 11505888 and pos11 38364944 nodes in every
one of the 288 runs each. The banner names `2f7b08a` and `654ab89`, and `85b7847` is the first's parent.
`journalctl -k` over the window: no entries. Governor `powersave`, recorded and not set (DEC-195); load 1.0.

**The hit rate**, `adocs/data/S118_hitrate.txt`, on the tune build at `40831fa`, the first 20 games of S125's
SPRT PGN, a table empty per `go`:

| workload | probes | hit % | slot probes | slot hit % |
|---|---|---|---|---|
| `bench` | 2963238 | 89.703 | 3615594 | 74.441 |
| `bench 16` | 6140795 | 89.652 | 7505226 | 74.619 |
| pos11 | 21106681 | 96.438 | 27108080 | 73.007 |
| replay | 445624368 | 93.694 | 631875854 | 70.487 |

The replay's two figures are the design pass's simulated cold column to the digit. DEC-332's 94.1 is that
simulation's table kept across `go`s, which is not what shipped.

**The Debug self-play, DEC-141**, binding through `src/search.cpp`: the Debug build at `40831fa`, four rounds
of `fastchess` at 4+0.04, Hash 16, `noob_3moves.epd`, 8 games, exit 0, `Assertion` 0 in the log and the
console, `disconnect` 0, no forfeit. `.tuning/coord/S118/debug_selfplay/`, `close_runs.log`.

**`gate_extra`**: `GATE-EXTRA-DONE 6 stages 1467 s` on `654ab89`. `git diff --stat 654ab89 40831fa -- src tests`
prints nothing, so it stands.

**The mutants: 24 of 24 killed**, `adocs/data/S118_mutants.txt`, PT01 to PT13 on `40831fa` in 2642 s, PT14 to PT24
on `f36762f` in 1927 s, `src/` and `tests/` the same at both. Every `same` on record is now a table's. PT07 is the
only row reading `n/a`. `tools/mutants/S118_pawn_cache.py`'s docstring corrected against the tables, PT07's row and
PT09's, whose log holds no mirror case; every `old` and `new` read the same by executing both versions.

**What DEC-083 says a pure speed-up converts to, and it is not measured here.** A behaviour-neutral change is
accepted on an interleaved timing and never on an SPRT; the strength claim is the measured nps difference
converted at the published 1.43 Elo per percent at long time control and 2.10 at short, named as a conversion
and not as a verdict. `total`'s wall, -6.367 %, is +6.80 % nps, so that conversion reads about +9.7 Elo at long
control and +14.3 at short. No game was played. Published figures do not transfer here (DEC-019), which is why
this is a conversion and is quoted as one.

**What this leaves unmeasured.**
- Strength: no game, no SPRT, by DEC-083.
- Under match load: the timing is one process on an idle machine, and a match runs twelve, two to a core,
  sharing what the table must fit in.
- Any other machine, and the shipped `bmi2` binary: every row is `native`, padded, on this Coffee Lake (DEC-267).
- What the slot is worth to S122, which is its only reason to stay; S122 decides.
- s10 against s12: s10 is out by 0.065 points on one workload, at a pos11 floor under a quarter of S125's
  session's. At that session's floor (7) would have chosen s10. The session's floor is the rule, and that does
  not say s12 is faster.
- The cost of zeroing the table per `go` is inside every row, not isolated. So is why s18's cycles and
  instructions part, which is memory only by inference.
- DEC-266 (4)'s perturbations: not owed, instructions moving the same way, and not run.

**The accepts, clause by clause.**
1. *"An SPRT verdict if the tree changes and a node-identity check if it does not -- state which before the
   run."* **Holds.** Node identity, stated in DEC-332 (6) and in "phase B, the cache" before any run;
   `adocs/data/S118_inv6.txt`, both builds, and the timing's own INV-6.
2. *"The key is a pawn-only zobrist maintained incrementally in add_piece, remove_piece and move_piece (INV-4),
   never recomputed in evaluate()."* **Holds, with its citation stale.** `board_t::pawn_hash`, in the three
   primitives of `src/bitboard.cpp`, landed by S099 and S286 and untouched here; `src/evaluation.cpp` reads it
   twice and `compute_full_pawn_hash` occurs nowhere in it. The key's wording is INV-2's, not INV-4's.
3. *"The hit rate is measured over a real search and recorded, not assumed."* **Holds.** Above.
4. *"The table stores the mg/eg pawn score, the passed-pawn bitboard and both kings' shelter and storm scores,
   because the shelter is the expensive half of what this saves."* **Does not hold as written; it holds as
   DEC-332 (3), (5) rule.** No storm: nothing computes one, and it is S122's. Not the whole pawn score: piece
   placement and S123's increments and king distances read pieces and stay outside. The shelter is in a parallel
   slot, not the entry. And the "because" is false: 1.63 % of instructions against about 9 %, and the slot reads
   zero. The passers and both kings' shelter are stored.
5. *"A collision returns a recomputation rather than a wrong score, and a test forces one."* **Holds.** The verifier
   is both bitboards, so a hit is exact. "a forced collision recomputes and never answers for another structure"
   plants the same 64-bit key on another structure; PT01 to PT04 go red on it.

**The manuals.** `MANUAL.md`, no change: its new-game paragraph, "The killer, history and counter-move tables and the
pawn table (S118) are per `go` and have nothing to reset", holds, `search_state_t` being built per `go`
(`iterative_deepening_search` in `src/chesso.cpp`); the `Hash` row and "`Hash` is not honoured when memory is
refused" are about the transposition table alone and hold; no sentence states the search's memory. `DEV_MANUAL.md`,
two sentences false and changed: the timing's command named `check`, which times nothing, and now names `run`, with
the measured 1 h 47 m and about 4 min and where each reading is; "walks the pieces in `evaluate_cheap()`" now names
`cheap_score()`, where the walk is. Checked and holding: "`bench_eval` calls `evaluate()` and nothing else"; "calls the
pure form, and the pawn table is not in it" and its "two sites"; the census's one line, per king, in
`pawn_table_census_t`; the one constant, no UCI option, zeroed every `go`; `S125_cost_only.patch` "no longer applies"
(`git apply --check`, exit 1); the Debug build's +0.33 %; "`evaluate() - evaluate_cheap()`", which is the pure forms'.

**How the technique was taken.** DEC-221 binds what the 2026-09-19 analysis proposed, and that study names S118 only
to set pawn history apart from it, so it does not bind here. It was taken the same way all the same, as "phase B, the
cache" says: CPW *Pawn Hash Table*'s description, the size from its "a few K" (DEC-105 form (a)) then this project's
sweep; every engine record read is a message, a pull request or a forum post (DEC-016).

**Goldens, DEC-142:** none moved and none re-derived; bench 5707449 at every commit. **The shas here** are branch `s118`'s,
`2f7b08a` the cache and `654ab89` the slot, the commits timed; the coordinator's record maps the landing's to them.
