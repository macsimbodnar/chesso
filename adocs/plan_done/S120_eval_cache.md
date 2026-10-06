id:         S120
goal:       a small cache of full evaluations by position key, so the score behind the lazy shortcut can be paid for once
accepts:    an SPRT verdict, recorded whatever it is; the cache stores the **score and never a bound**, for the reason S094 records -- a bound is true on one side of one window and an entry outlives the window; the hit rate is measured over a real search and recorded; the step states whether the lazy shortcut is kept, narrowed or retired on the strength of the measured hit rate, and that statement is the input S039 and S122 read
touches:    src/evaluation.cpp, src/evaluation.hpp, src/data_structures.hpp,
            src/chesso.cpp for the two clears section 5 requires -- beside
            tt_reset at ucinewgame, and on the LazyEvalMargin setoption in the
            tune build -- and tests/test_evaluation.cpp for section 3(a)'s
            red-first properties; as built also src/transposition_table.cpp
            and .hpp (tt_reset empties the cache, deviation 2), MANUAL.md
            (what ucinewgame and bench reset) and adocs/data/S120_measurements.txt
excludes:   choosing LAZY_EVAL_MARGIN, which is S039; the king safety rebuild, which is S122
decisions:  DEC-039
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-06, retired without an SPRT by the owner's choice (DEC-257): no code lands. Hit rate 0.11 to 0.42 % of evaluate_lazy() calls with 0 disagreements; (a) -1.54 % (CI -1.73 .. -1.36), (b) -0.68 % nps, no size from 2^15 to 2^19 pays. Both diffs kept as adocs/data/S120_a.diff and S120_b.diff, the reading is adocs/data/S120_measurements.txt. (c): the lazy shortcut is kept; exact everywhere re-measured at 5.0 % nps for a 7.5 % smaller tree and 2.6 % less wall to depth 11. Both gates had been green on (a) and (a)+(b), 43/43 in both builds; Debug self-play 0 Assertion; gate_extra 5 stages green. DEV_MANUAL checked, no change; MANUAL's two sentences reverted with the code. Finding 9 is filler S259.

## The point is not speed, it is what it unblocks

The lazy shortcut exists because `evaluate()` is expensive and quiescence calls
it at nearly every node -- `bench_eval` reads **53.90 ns a call, 18.6 M calls a
second** on the shipping `bmi2` target (2026-08-19, S104; `adocs/specs.md`), and
83.35 ns before that flag existed. The shortcut's soundness rests on
`evaluate_expensive()` clamping mobility plus king safety to `LAZY_EVAL_MARGIN`,
184 as shipped since S085's SPSA run raised it from 150, for the two of them
together. That clamp is the ceiling on how much the evaluation is allowed to
say, and a real king-danger term needs to reach four to six hundred.

So the order is: cache the full score (here), size or retire the margin
(S039), then rebuild king safety without a clamp over it (S122). Measured cost
of paying full evaluation everywhere with no cache, 2026-08-19: **11.7 % of
nps**. S104 has already paid +16.7 % toward that, and this step and S118 are
what buy the rest back.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `cf89e22`, src/ unchanged through `c0954ec`; re-locate by
symbol if drifted. Every GitHub read was a commit message or PR conversation,
never a diff (DEC-016).

### 1. State of the art

CPW carries the structure as **Evaluation Hash Table**: zobrist-keyed, and the
TT-vs-dedicated question is answered there in one sentence -- "despite the fact
that the transposition table entries may contain evaluation scores as well, a
tighter, dedicated evaluation hash table with its own replacement policy may
gain a considerable amount of additional hits." The 2019 practitioner phrasing
(Minic thread): "most engines have a hash entry for static score, directly
inside the main TT or in a separate table" -- both forms live. Jon Dart,
first-hand: Arasan skips the TT in quiescence and "uses a separate evaluation
cache that only stores the static score" -- quiescence is the table's audience.

Adds traced: **Ethereal 16b9796e (2020-09-06), "Add a 512KB evaluation cache
to each Thread": +11.86 +/-6.27 at 12+0.12 Hash 8, +4.04 +/-2.89 at 60+0.6**
-- an HCE engine with an expensive evaluation, the closest published shape to
chesso's; two weeks later 513a93c2 states the stack: "due to Pawn Hashing,
Eval Caching, and the TT, we very rarely actually have to perform the
computation". **Halogen PR #92 (2020-09-30): +7.37 +/-5.39**, HCE era; #164
refactored it for speed, non-regression. **Stockfish 6fa83f51 (2012-12-04):
+18 over 4855 games**, a per-thread cache replacing TT-as-backing-store,
"frees two TT entry slots". Crafty (author on talkchess, 2016): 65536 64-bit
entries; tried storing "the score, the bounds, and what the current score
means (score from lazy1, lazy2 or full eval)", settled on full evaluations
only -- "if there is a lazy exit, we don't store a thing" -- worth "a couple
of elo".

Removes traced, each with a stated cause. **Stockfish 3cf64717 (2012-12-27),
23 days after the add**: the +18 re-tested at **-9 over 4957 games** (the
first run had hit a cutechess bug), plus the structural point that a
per-thread cache loses to a shared TT under SMP; master went back to the TT
and has stored eval there since. **Ethereal df3fa74c (2022-06-01)**: once the
TT stored statics early in quiescence, "the hitrate on the evaluation cache
plummets from by orders of magnitude, making it effectively useless" --
removal +0.85/+2.28 inside non-regression bounds. **Halogen PR #504
(2024-07-01): removal +1.57 +/-3.33** in the NNUE era, the cached function
having become cheap. Read together: the cache pays exactly while (1) the
evaluation is expensive against a probe and (2) the TT does not already hand
the eval back at the nodes that ask; every removal is one precondition
ending. Replacement is second-order -- a 2019 talkchess sweep measured a
two-tier scheme ~1.5 % nps over always-replace and concluded size matters
more. No dedicated-table hit rate was published as a number anywhere traced;
Ethereal's "orders of magnitude" and Crafty's under-half store rate (lazy
exits store nothing) are the closest prose. Measure it here.

### 2. Shape for chesso

Both preconditions hold today, measured. (1) `evaluate()` is `evaluate_cheap()
+ evaluate_expensive()` (`src/evaluation.cpp` `evaluate` and
`src/evaluation.cpp` `evaluate_cheap`), 53.90 ns a call post-S104 (specs.md);
the expensive half is mobility plus king safety clamped to +/-LAZY_EVAL_MARGIN
inside evaluate_expensive() (`src/evaluation.cpp` `evaluate_expensive`; the
constant at `src/search_params.hpp` `LAZY_EVAL_MARGIN`), and paying it
everywhere costs the 11.7 % above. (2) The TT does not cover quiescence: S094
counted the quiescence probe finding **any** entry on 0.79 % of nodes (kiwipete
depth 12, 16 MB table) -- TT_DEPTH_QS = -1 sits below every main depth, so any
main store over the slot evicts a quiescence entry -- while S103's 23.09 % warm
hit rate is a main-search number. Chesso is also single-threaded (DEC-089's
1CPU scale), so Stockfish's SMP argument is vacuous here. DEC-039 retired
exactly this cache once -- at 1.36 ns of accumulator-only evaluate() against a
0.36 ns probe into 256 KB (tools/probe_cost.cpp, pre-DEC-049 machine), a sub-1
% prize. At 53.90 ns, with S121/S122 about to raise it, the same arithmetic
reopens it; the stamp should say so in one line.

Placement: **inside the evaluation module, at evaluate_lazy(), not inside
evaluate()**. Three reasons. `touches:` names no search file and quiescence
already calls evaluate_lazy (`src/search.cpp` `quiescence`), so the mass of
calls is covered without touching search.cpp. evaluate() must stay pure:
bench_eval calls it in a loop over a fixed list (tests/bench_eval.cpp) and a
cache inside it turns the benchmark into a probe benchmark on the second sweep;
eval_spread and the tuner's oracle (eval_model::evaluate,
`tools/eval_model.hpp` `evaluate`, its own eval_model) stay untouched the same
way. And the main search is already TT-fed: post-S108 every non-check main node
reads or stores tt_eval, so the published division of labour lands as TT-eval
for the main search, dedicated cache for quiescence -- Arasan's split. Key:
`board->hash`, in hand at every call; it covers side to move
(`src/bitboard.cpp` `make_move_impl`), so INV-5 reads back with no sign
applied, S108's argument. Stored: the **raw clamped evaluate() output** -- what
`src/evaluation.cpp` `evaluate_lazy` returns on its exact path, never the
bound it returns on either shortcut (the accepts; S094's reason), and never a
corrected
value: S099 applies its correction after the read, at the search site, and its
raw-in-storage rule covers this table too. Layering, bottom up: cache -> raw
eval -> correction -> consumers.

Sizing on this machine (i7-8700K: 256 KiB L2 per core, 12 MiB shared L3,
16 GB): entry one uint64, 48-bit upper-key tag plus int16 score, direct-mapped,
always-replace. 65536 entries = **512 KiB** -- twice one core's L2, 1/24 of
the L3 the 16 MB TT already thrashes (DEC-088 regime). RAM is irrelevant;
probe latency is what tools/probe_cost.cpp re-prices here. Fixed size, not a
UCI option -- Ethereal's was fixed too -- so MANUAL's option surface is
untouched.

### 3. Implementation sketch

(a) **Table + probe on the pay-the-expensive path -- behaviour-neutral,
proven.** probe/store/clear in evaluation.cpp, declared in evaluation.hpp,
entry type in data_structures.hpp (`touches:`). evaluate_lazy() probes
**after** the shortcut tests in `src/evaluation.cpp` `evaluate_lazy`: a hit
returns the stored full score, value-identical to what that function would
compute, by construction; a miss computes and stores. The bound paths store
nothing. evaluate()'s two direct callers in `src/search.cpp` `negamax` neither
probe nor fill -- they are TT-fed and rare.
Tests, red-first: property over a position list -- empty cache, wide window,
call twice, second equals fresh evaluate(); revisit through a made-and-unmade
line (INV-2 pattern); shortcut path stores nothing (establish `*exact == false`
first, then assert the probe misses -- non-vacuous); two keys sharing index
bits and differing tags do not answer each other; the INV-5 mirror pair each
equal their fresh call; clear() empties. Neutrality discharged per DEC-083:
search_bench node-identical at depths 9 and 12, one interleaved timing recorded
-- expect small, since a hit saves evaluate_expensive() only, the cheap stage
having been paid for the shortcut test.

(b) **Probe above the shortcut -- the SPRT.** Move the probe to the top of
evaluate_lazy(): a hit now returns the exact score where the shortcut would
have handed back cheap +/- margin -- the strictly-better-number argument S094's
stand-pat read makes at `src/search.cpp` `quiescence`. Not arguable into
neutrality: cutoff decisions cannot flip (full is on the bound's side of the
window by the `src/evaluation.cpp` `evaluate_lazy` arithmetic) but the
fail-soft values propagated differ, so node counts move by construction; a hit
also sets `*exact`, so `src/search.cpp` `quiescence` writes the score on to the
TT -- consistent, both tables hold evaluate()'s output. Hit-rate
instrumentation rides this commit: probe/hit/store/bound-skip counters in a
throwaway or debug-flag build (S103's method), printed after `go`, run over
S103's own protocol -- 300 positions at depth 10 in one process, plus the three
search_bench positions at depth 12 -- and the disagreement count against a
fresh evaluate(), which must be 0. bench_eval is not the vehicle; the accepts
says a real search.

(c) **The statement S039 and S122 read.** With hit rate h, exact everywhere
costs about (1-h) x 11.7 % of nps plus probe overhead. Write the
kept/narrowed/retired sentence from the measured h and that arithmetic into
the stamp; S039 executes it.

### 4. Constants and seeds

- **Size 512 KiB = 65536 x 8 B** -- Ethereal's commit-message size and
  Crafty's entry count, independently equal. Seed -- re-decide by measurement
  here: sweep 2^15..2^19 entries during (b)'s instrumentation, hit rate
  against nps, pick the knee.
- **Entry: uint64 = 48-bit tag + int16 score** -- Crafty's split as described
  in forum prose. Seed -- tag width re-decided by the collision arithmetic,
  nothing here is fitted.
- **Always-replace, direct-mapped** -- the 2019 sweep's conclusion was size
  over scheme. Structural choice; no numeric constant ships, DEC-084
  satisfied by the two markings above.

### 5. Pitfalls

- **Never store the shortcut's bound.** The accepts says it, S094 recorded
  why (a bound is true on one side of one window and an entry outlives the
  window), Crafty's rule is the same. The (a) test drives it red-first.
- **Clamp staleness is a tune-build-only hazard, and the plan order is cache
  first.** Verified against plan.md's list: S120 (entry 34) -> S039 (41) ->
  S122 (47), and this file's own prose says cache, then margin, then rebuild.
  Every cached value carries the 184 clamp -- self-consistent, the cache
  mirrors the live evaluate(), and when S039 changes the margin that is a new
  binary and a fresh cache. The live path is the tune build, where
  LazyEvalMargin is a setoption (`src/search_params.hpp` `LAZY_EVAL_MARGIN`):
  clear the cache on that setoption, the same rule S108 records for the TT eval
  field. Were S039 ever re-ordered ahead, nothing breaks -- the cache is
  indifferent to which margin it memoises.
- **evaluate() must stay a function of what the key covers.** True today:
  position and stm, both in the zobrist. A halfmove-clock scaling term (Weiss
  a7d77839 scales eval toward draw as the 50-move rule nears) would break the
  cache silently -- apply any such future scaling outside the cached function.
- **Sign is safe by key** -- stm is in the zobrist (`src/bitboard.cpp`
  `make_move_impl`), a key-matched entry is same-side, INV-5 unchanged; the
  mirror test proves non-interference rather than assuming it.
- **Collisions are bounded, not free.** A tag collision returns a wrong
  static eval, never a wrong bound or a crash -- Hyatt: "a collision won't
  crash a thing". 48-bit tag over a 2^16 table discriminates on the full 64;
  the instrumentation's 0-disagreements count is the collision check in vivo.
- **ucinewgame clears it** beside tt_reset (`src/chesso.cpp`
  `command_setoption`); a 512 KiB memset is free. Entries are position-pure, so
  persisting within a game is sound and published; clearing at game boundaries
  is the conservative form.
- **No ply normalisation exists here.** The eval is ply-independent and
  non-mate by construction; do not reuse TT score plumbing or
  de_normalize_score anywhere near it.

### 6. Measurement

- (a) owes no SPRT: node-identical search_bench at depths 9 and 12 plus one
  interleaved timing (DEC-083; 1.43 Elo/% LTC, 2.10 STC, a conversion, not a
  verdict).
- (b) owes **one SPRT** at the S105 regime (8+0.08, Hash 16, UHO book) -- the
  accepts' verdict. **Bounds: non-regression, elo0=-5 elo1=0**: the traced
  adds (+11.86, +7.37) predate TT-eval plumbing chesso already has, the
  nearest local precedent is S094's read-back zero, and DEC-063 forbids
  straddling an expected ~0 with gainer bounds. Record it whatever it is.
- Recorded numbers: hit rate per site over the S103 protocol, the 0
  disagreements, (a)'s nps delta, and (c)'s statement. Expected verdicts:
  **1 SPRT plus 1 timing**.

### 7. Interactions

- **S108 (before).** Complementary, not rival. Probe order at quiescence stays
  TT first (`src/search.cpp` `quiescence`, probe already paid), cache second
  inside evaluate_lazy, compute third. S108 section 7 reserved exactly this:
  evictable opportunistic storage there, guaranteed storage here. Do not merge
  them.
- **S039 (after).** Reads (c): h high means the margin can rise or retire at
  (1-h) x 11.7 % effective cost; h low means the clamp stays load-bearing and
  S122 must fit under whatever S039 keeps. Both branches are real until
  measured.
- **S121 / S122 (after).** The rebuilds make evaluate_expensive() dearer, so
  every point of hit rate buys more; an unclamped king safety also widens the
  spread, fires the shortcut less, and leans on the cache harder. Cache-first
  order is correct; keep it.
- **S118 (after).** The next cache, different key: pawn-structure-keyed
  (S099's pawn_hash), caching terms rather than the whole score, 90 %+ hit
  class per the pawn-hash record where this table's rate is unknown. TT +
  eval cache + pawn hash together is Ethereal's published 2020 stack.
  Separate tables, separate steps.
- **S119 (next entry).** If the TT rebuild ever adds an Ethereal-style
  early-quiescence eval store, re-measure this cache before trusting it:
  df3fa74c records that exact combination making the dedicated table useless.
- **DEC-039** is reopened, not contradicted: its own arithmetic re-run at
  today's evaluate() cost answers the other way. Say so in the stamp.

### 8. References

- https://www.chessprogramming.org/Evaluation_Hash_Table -- zobrist/BCH
  keying, the dedicated-vs-TT sentence, forum index 1998-2019.
- https://github.com/AndyGrant/Ethereal/commit/16b9796e -- add, 512 KB per
  thread, +11.86 STC / +4.04 LTC.
- https://github.com/AndyGrant/Ethereal/commit/df3fa74c -- remove: early QS
  TT store, "hitrate ... plummets", +0.85/+2.28 non-regression.
- https://github.com/AndyGrant/Ethereal/commit/513a93c2 -- the TT + eval
  cache + pawn hash stack sentence.
- https://github.com/KierenP/Halogen/pull/92 -- add, +7.37 +/-5.39.
- https://github.com/KierenP/Halogen/pull/164 -- refactor, non-regression.
- https://github.com/KierenP/Halogen/pull/504 -- remove, +1.57 +/-3.33, NNUE
  era.
- https://github.com/official-stockfish/Stockfish/commit/6fa83f51 -- add
  merge, +18, "frees two TT entry slots".
- https://github.com/official-stockfish/Stockfish/commit/3cf64717 -- revert
  23 days later, -9 on re-test, cutechess bug named, per-thread vs shared TT.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+eval+cache
  -- the 2012 family (4e5d834e infrastructure, a5ea3a20, 9edc7d69).
- https://talkchess.com/viewtopic.php?t=58758 -- Crafty: 65536 64-bit
  entries, 48-bit tag, store-full-only, collision prose, "a couple of elo".
- https://talkchess.com/viewtopic.php?t=72030 -- replacement sweep, two-tier
  ~1.5 % nps over always-replace, size over scheme, quiescence audience.
- https://talkchess.com/viewtopic.php?f=7&t=70938 -- "directly inside the
  main TT or in a separate table", 2019.
- https://talkchess.com/viewtopic.php?t=47373 -- Arasan's separate eval cache
  in quiescence, first-hand; Crafty found TT-in-qsearch a wash.
- https://www.stmintz.com/ccc/index.php?id=151211 -- CPW-listed 2001 thread,
  404 at research time, not used.

## As built (2026-10-06, the implementer)

Implemented from this file's description (DEC-221); no other project's code
was opened. Nothing committed and no match run. Evidence is under
`.tuning/coord/S120_files/` (gitignored); the reading is
`adocs/data/S120_measurements.txt`. Two diffs: `a.diff` on `1eaa776`, `b.diff`
on top of it; HEAD + both is the working tree, file for file (checked).

### Deviations and findings, first

1. **The hit rate is tiny, and (a) and (b) are both slower.** The cache
   answers 0.11 % of `evaluate_lazy()` calls over S103's protocol and 0.42 %
   over 20 replayed games; an unbounded table would answer 1.4 to 2.2 % per
   search (6.0 % per game if kept across moves). Quiescence positions are
   almost never revisited within a search, and the TT answers most of those
   that are (2.5 to 2.9 % of stand-pat sites read its eval). Section 1's
   precondition (2) holds -- the TT does not hand the eval back -- but there
   is almost nothing to hand back. (a) measured -1.54 % (hyperfine) and
   -1.42 % nps (rotation); (b) -0.68 % and -0.74 % nps. DEC-039's arithmetic
   re-run: the price per call is now high, the prize is the hit rate, and the
   hit rate is ~0.4 %.
2. **The table is `thread_local`, not process-wide.** datagen runs the search
   on every core at once, so a shared plain array is a data race (UB), and a
   shared atomic one makes its workers' games depend on each other. Per
   thread, each worker stays reproducible. The cost: the engine starts a new
   thread at every `go`, so the table starts empty at every move and never
   persists across moves as section 5 described. Measured on the 20-game
   replay: 0.42 % per-thread against 0.58 % for one process-wide table kept
   across a game's moves -- both negligible. specs' rule that a search table
   states per-thread or shared at its declaration is met (`evaluation.hpp`).
3. **`tt_reset()` empties the calling thread's cache** (`src/transposition_table.cpp`,
   outside `touches:`). Every "search cold" idiom resets the TT: ucinewgame
   and bench (`reset_for_new_game`), a new base position, `clean-tt`,
   datagen per game, and about sixty test sites. Without it (b) would make
   tests order-dependent. Section 5's ucinewgame clear is this hook; the
   LazyEvalMargin clear is explicit in `command_setoption` (tune build).
4. Slot type `eval_cache_slot_t`, not `eval_cache_entry_t`:
   `tools/probe_cost.cpp` already defines the latter and stopped compiling.
5. Slot encoding: tag = key's upper 48 bits, score + 32768 in the low 16, so
   an all-zero (empty) slot decodes to INT16_MIN and misses. A score outside
   (INT16_MIN, INT16_MAX] is not stored (the TT's field clamps; a clamp here
   would hand back a number `evaluate()` did not return). Refused stores
   measured: 0.
6. **(b) edits two existing cases in `tests/test_evaluation.cpp`**: "the lazy
   shortcut cannot change a decision" and "the shortcut says when it took
   one" now empty the cache after their wide-window call. A precondition, not
   a relaxation: under (b) the stored score answers before the shortcut, so
   without it the second case is red (`REQUIRE( !exact )`, observed) and the
   first passes without reaching the shortcut at all. `eval_fixture_t` clears
   the cache, so every case starts cold.
7. Extra measurement for (c): exact everywhere re-measured on today's tree
   (throwaway build, shortcut off, clamp kept). It is **5.0 % nps, not
   11.7 %**, and the tree to depth 11 shrinks 7.5 % (below).
8. The floor scripts need a commit: `git stash create` made a dangling
   commit `889d28f` carrying (a)+(b) (S116's precedent); no ref moved.
9. **Finding, not this step's (DEC-171 filler class):** on the parent
   `1eaa776` the S097 row (`mate_the_multicut_hides`, depth 14) and the S113
   row (`mate_probcut_hides`, depth 11) no longer separate their guard
   mutants: E21 reads the S097 row at d11, d13, d14 like the shipped build,
   and B04 reads the S113 row exactly as shipped (d9, d11 to d14). Both cases
   stay green; neither row's mate is lost by its mutant at its pinned
   depth. On (b), E21 separates the S097 row at d12. Not re-mined here
   (`.tuning/coord/S120_files/rows/`).

### (a): the table, probed where the full score is owed

`evaluate_lazy()` tests the two shortcuts, then probes; a hit returns the
stored score, a miss computes and stores. The shortcuts store nothing.
`evaluate()` stays pure; negamax's two direct `evaluate()` calls neither probe
nor fill.

**INV-6 against `1eaa776`, discharged:** `chesso bench` **4081329**, `bench 12`
**1860699**, every info line and bestmove identical (streams stripped of
timing and diffed whole); `search_bench.py` 32932 / 70095 / 25178 at depth 9
and 67792 / 280873 / 137893 at depth 12, `c3d5` / `e2a6` / `d7c8q` and `c3d5`
/ `d5e6` / `d7c8q`, identical; `bench_eval` checksum **-27425414745967**
identical; tune build bench 4081329; the 300-position timing workload
26512806 nodes on both. The gated binary is byte-identical to the timed one.

**Timing** (S117's scripts, 300 positions at depth 11; load 1.75 / 2.60 /
1.69 before, between, after; governor powersave):

| comparison | pairs | speed-up | 95 % CI |
|---|---|---|---|
| A/A, parent vs byte copy | 16 | -0.25 % | -0.45 .. -0.06 |
| **(a) vs parent** | 24 | **-1.54 %** | -1.73 .. -1.36, 0/24 faster |

The rotation run below reads (a) at -1.42 % nps (CI -1.79 .. -1.04). About
**-3.2 Elo** at DEC-083's 2.10 per percent, a conversion. A miss costs a
probe and a store at every owed-full call, and the per-`go` thread zeroes its
512 KiB table; a hit (about 0.2 % of owed-full calls, the (b) counters as a
proxy) saves `evaluate_expensive()`.

**Tests**, `tests/test_evaluation.cpp` "evaluation: cache", each observed red
under a mutant (`.tuning/coord/S120_files/redfirst/`):

| case | red under |
|---|---|
| a second call reads back what the first one computed | M1 store writes score + 1; M2b store keyed with the halfmove clock; M7 clear no-op |
| evaluate_lazy answers from the slot (planted value, read after a knight shuffle back to the root) | M2 probe keyed with the halfmove clock |
| a position reached again reads back its own score (INV-2: line made and unmade, transposition at ply 4) | M1, M2b, M6 one slot for every key, M7 |
| a shortcut stores nothing (`exact == false` established first) | M3 shortcut stores its bound; M2b, M7 |
| a slot answers only the key that filled it (all 64 single-bit flips miss, same-slot eviction, empty slot) | M4 tag ignored; M5 empty slot not refused; M6 |
| a position, its mirror and its other side to move (INV-5) | M1, M6, M7 |
| a clear empties the table | M7, M2b |
| a transposition table reset empties the table too | M8 hook removed; M7 |
| each thread has a table of its own | M9 not thread_local |
| (tune only) setting LazyEvalMargin empties the table, another parameter does not | M10 setoption clear removed |

The mirror pair alone cannot fail by value (INV-5 makes the two scores
equal); the other-side-to-move pair is what gives that case teeth.

**Gate on (a) alone**, in a clean worktree of `1eaa776` + `a.diff`: both
builds 43/43, format clean (DEC-146 override).

### (b): the probe above the shortcut

The probe moves to the top of `evaluate_lazy()`: a hit returns the stored
score, flagged exact, whatever the window, and pays for neither stage.

- `bench` **4081329 -> 3768566** (-7.66 %); `bench 12` 1860699 -> 1837406;
  all eight replies unchanged in both. The tactical position carries it:
  573500 -> 270363; killer 489490 -> 479878; kiwipete -14; five unchanged.
- `search_bench.py`: depth 9 identical; depth 12 67792 / 280862 / 114611,
  best moves unchanged. The 300-position workload: 26525589 nodes (+0.05 %).
- Tests: the planted case now expects the planted score through all three
  windows, and a new corpus case "a stored score answers a window the
  shortcut would have"; both red under (a)'s ordering (observed).

**Hit rate**, throwaway instrumented build (counters per path, a fresh
`evaluate()` compared at every hit), S103's protocol plus a game replay:

| run | stand-pat sites | TT eval read | lazy calls | cache hits | of which over a bound | owed-full answered | disagreements |
|---|---|---|---|---|---|---|---|
| 300 positions, depth 10, one process | 7011544 | 2.48 % | 6837558 | 7688, **0.11 %** | 430 | 0.21 % | **0** |
| search_bench, depth 12 | 227507 | 3.73 % | 219024 | 567, **0.26 %** | 20 | 0.48 % | **0** |
| 20 games replayed, `go nodes` as played | 275694658 | 2.91 % | 267676134 | 1135775, **0.42 %** | 13575 | 0.69 % | **0** |
| same, one table kept across a game's moves | 275931128 | 2.90 % | 267922965 | 1564236, 0.58 % | 28316 | 0.94 % | 0 |

The ceiling, every key a search ever saw kept: 1.37 %, 2.18 % and 1.43 % of
calls for the three shipping-form runs; 5.97 % for the across-moves replay.
Of the calls in the S103 run, 48.5 % take a shortcut and store nothing and
51.4 % compute and store; no store was refused.

**Size sweep**, hit rate (instrumented) against nps (plain builds, 8 rotated
rounds, A/A control ref vs byte copy +0.14 %, CI -0.18 .. +0.47):

| slots | per thread | hits S103 / sb / replay | nps vs parent | 95 % CI |
|---|---|---|---|---|
| 2^15 | 256 KiB | 0.11 / 0.21 / 0.37 % | -0.57 % | -0.97 .. -0.17 |
| **2^16** | 512 KiB | 0.11 / 0.26 / 0.42 % | -0.68 % | -1.17 .. -0.19 |
| 2^17 | 1 MiB | 0.12 / 0.30 / 0.47 % | -1.99 % | -2.46 .. -1.53 |
| 2^18 | 2 MiB | 0.12 / 0.32 / 0.50 % | -3.44 % | -3.77 .. -3.11 |
| 2^19 | 4 MiB | 0.12 / 0.33 / 0.53 % | -5.23 % | -5.66 .. -4.79 |

No knee pays: hits rise 0.16 points over the range while nps falls 4.7. 2^15
and 2^16 are not separated (0.11 points, CIs overlapping), and 2^17 up is
measurably slower. **The seed 2^16 is kept.** A second run read 2^16 at
-0.74 % (CI -1.01 .. -0.46).

**Goldens (DEC-142), each by its script on the (b) tree; none moved:**
- capture-mate table, the seven sweeps (shipped + six S091 mutants, depths 3
  to 12): shipped profiles `d9-12`, `d8-12`, `d10-12`, `d10 d12` and every
  mutant profile as S116 recorded, so 9, 8, 10, 10 and the labels stand.
- `first_mate_depth` (`S188_repair_goldens.py first-mate`): 10 and 9,
  output identical to the parent's.
- `DRIVE_DEPTH` (`S188_repair_goldens.py s207`): separating depth 4.
- node budget (`S192_node_budget.py`): count 20350, inside the middle half
  [18694, 49581]; the band 65024 / 3251 stands.
- `S156_mined_floor_sweep.py --ref 889d28f`: 149 / 145 / 143 / 143 at
  `RfpMinPly` 3..0; floor 145 separates, gap 6; the weakened gate red.
- `S154_floor_margin_sweep.py floor --ref 889d28f`: mates in three 12 / 12 /
  7 / 7, floor 11 separates; mates in two on time 26 / 17 / 12 / 12.
- S095's mined row: shipped mate 2 at every depth 3 to 14, the unguarded
  form loses it at 8; depth 8 stands. S097 and S113 rows: finding 9.

**The S170 budgets (DEC-156 as amended by DEC-162).** `S203_case_sweep.sh`,
full grid, on a copy of the (b) Release binary. At the standing budgets the
suite is green: A and D are silent, B, C, E and F report (majority). The rule
moves **A 500000 -> 1000000, C 1500000 -> 1200000, D 2000000 -> 3000000**;
`--at` on the patched TSV reproduces 5/0, 6/0, 1/1, 2/0, 1/0, 8/0, and
`test_mate_carry` is **green in both builds at the rule's budgets** with the
standing ceilings (C's one short line against 4). `--ceilings` with this grid
appended to S202's answers 0, 5, 4, 0, **1**, 0 -- **E 0 -> 1** -- and this
grid alone 0, 4, 1, 0, 1, 0. **No ceiling raised and no budget changed in
the tree.** The rule's answer is a separate patch,
`.tuning/coord/S120_files/s170/s170_budgets.patch` (TSV, the grid as
`adocs/data/S120_sweep_s170.txt`, its README row, DEV_MANUAL's re-sweep
sentence); it leaves the grid out of the `--ceilings` command, because
appending it raises E. Whether it joins is a decision (DEC-249, DEC-253).

**Second tier (DEC-141):** Debug self-play, 8 games at 4+0.04 in 18 s: **0
`Assertion`, 0 `disconnect`** (`.tuning/coord/S120_files/debug_selfplay.*`).
`tools/gate_extra.sh`: **5 stages green in 1036 s** (prose, citations, debug
395 s, sanitize 585 s, perft 56 s; `.tuning/gate_extra_2026-10-06_S120.log`),
taken before the tune-only case was added, a tests-only change. No pruning,
reduction or extension rule, so DEC-141's mutant clause does not apply.

**Gate on (a)+(b)**: both builds 43/43, format clean; `bench` 3768566 in both.

### (c) The statement S039 and S122 read

**The lazy shortcut is kept; this cache changes nothing about it.** Measured
h is 0.11 to 0.42 % of `evaluate_lazy()` calls (0 disagreements), 1.4 to
2.2 % per search for an unbounded table and 6.0 % per game for one kept
across moves. So (1 - h) x 11.7 % is 11.7 %: the cache buys back none of the
price of exact everywhere, and with it that price is higher, not lower.
Re-measured on today's tree with the clamp kept at 184, exact everywhere
costs **5.0 % nps** without the cache (CI 4.7 .. 5.3) and **5.7 %** with it,
while the tree to depth 11 shrinks 7.5 % and the wall time falls 2.6 %
(`adocs/data/S120_measurements.txt`). So S039 decides the shortcut on its own
SPRT and on those numbers, not on 11.7 % and not on this cache; S122 fits
under whatever clamp S039 keeps. Without the shortcut the clamp guards
nothing, so retiring it would also free S122.

### Bounds, for the pre-registration

Not run here; the coordinator runs it.

- **Regime**, `fastchess.sh` as it stands: 8+0.08, `Hash=16`,
  `books/noob_3moves.epd`, all 12 threads, `--nonreg` = `elo0=-5 elo1=0`
  nElo, `alpha=beta=0.05`, 20000 rounds (a 40000-game cap). Working tree
  (a)+(b) against HEAD, or the (b) commit against the (a) commit, the
  coordinator's choice: (a) is node-identical to its parent, so either pair
  measures (b) plus (a)'s cost.
- **Cost, DEC-143's formula** (`testing_strategy.md` 1.1), at 2110 games/h:
  truth on a bound, 639770 / 25 = **25591 games, 12.1 h**; truth at the
  midpoint, 1046535 / 25 = 41861, past the cap, so **40000 games, 19.0 h**.
  A night run (DEC-155); watcher ceiling about 40 h.
- **Expected** (stated, not a reading): (b) costs 0.7 % nps, about -1.5 Elo
  at 2.10 per percent, and changes a stand pat on 430 of 6.8 M calls in the
  S103 run. A truth near -2 nElo sits inside the interval, where the run is
  longest.
- **Abort rule**: the one-minute load from `/proc/loadavg` well under 12
  before launch and nothing else running; abort if the forfeit rate passes
  1.0 % on either side (`tools/forfeit_report.py` over the run's PGN). No
  harness change, so no A/A owed (DEC-143).
- **Readings, fixed now:**
  - **H1**: not a regression of 5 nElo or more. Keep (b). The S170 patch
    lands; whether its grid joins `--ceilings` (E 0 -> 1) goes to the owner.
  - **No verdict at the cap**: recorded as zero; keep (b), as the brief says.
  - **H0**: revert (b) (`b.diff` reversed); keep (a), as the brief says.
    The tree's goldens are (a)'s, i.e. the parent's: nothing to restore.
  - For the coordinator, not a reading: (a) alone measured -1.4 % nps and
    no gain, so keeping it after an H0 keeps a slowdown. The data would
    rather retire both and record DEC-039 as reconfirmed. Your call.
- **Open findings during the run (DEC-171)**: finding 9 (the S097 and S113
  mined rows no longer separate their mutants on `1eaa776`), a test property
  that reaches no play.


## Closed without an SPRT, DEC-257 (coordinator, 2026-10-06)

The accepts asked for an SPRT verdict on (b). The owner chose to retire the
cache on the measurements above instead: the table answers 0.11 to 0.42 % of
`evaluate_lazy()` calls, both forms are slower than the parent, and the
pre-registered run was 12.1 to 19.0 h for a truth expected inside the
`{-5, 0}` interval, where the run is longest. Retiring a change needs no
verdict; keeping one does. The working tree went back to `1eaa776`'s code
(src/, tests/, MANUAL.md), after checking that `a.diff` then `b.diff`
applied to `1eaa776` reproduce it file for file. The two diffs are kept as
`adocs/data/S120_a.diff` and `adocs/data/S120_b.diff`, so a later step
re-adds the cache from them, with its ten red-first cases, rather than from
scratch. The S170 budget patch in `.tuning/coord/S120_files/s170/` described
(b)'s tree and is void with it. Section (c) stands as written: it is a
measurement of the shortcut and does not depend on the cache landing.
Finding 9 predates this step and is filler S259.
