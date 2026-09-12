id:         S118
goal:       the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
accepts:    an SPRT verdict if the tree changes and a node-identity check if it does not -- state which before the run; the key is a pawn-only zobrist maintained incrementally in add_piece, remove_piece and move_piece (INV-4), never recomputed in evaluate(); the hit rate is measured over a real search and recorded, not assumed; the table stores the mg/eg pawn score, the passed-pawn bitboard and **both kings' shelter and storm scores**, because the shelter is the expensive half of what this saves; a collision returns a recomputation rather than a wrong score, and a test forces one
touches:    src/evaluation.cpp, src/data_structures.hpp, src/bitboard.cpp piece primitives
excludes:   new pawn terms, which are S125; the passed pawn suite, which is S123
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
done:

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
  not replace each other and the step says how they divide.
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
