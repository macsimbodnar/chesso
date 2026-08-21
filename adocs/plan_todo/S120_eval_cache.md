id:         S120
goal:       a small cache of full evaluations by position key, so the score behind the lazy shortcut can be paid for once
accepts:    an SPRT verdict, recorded whatever it is; the cache stores the **score and never a bound**, for the reason S094 records -- a bound is true on one side of one window and an entry outlives the window; the hit rate is measured over a real search and recorded; the step states whether the lazy shortcut is kept, narrowed or retired on the strength of the measured hit rate, and that statement is the input S039 and S122 read
touches:    src/evaluation.cpp, src/evaluation.hpp, src/data_structures.hpp
excludes:   choosing LAZY_EVAL_MARGIN, which is S039; the king safety rebuild, which is S122
decisions:  DEC-039
closes:
blocks:
paused_by:
done:

## The point is not speed, it is what it unblocks

The lazy shortcut exists because `evaluate()` costs 83 ns and quiescence calls
it at nearly every node. The shortcut's soundness rests on
`evaluate_expensive()` clamping mobility plus king safety to
**+/-LAZY_EVAL_MARGIN, 150 centipawns for the two of them together**. That
clamp is the ceiling on how much the evaluation is allowed to say, and a real
king-danger term needs to reach four to six hundred.

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

Both preconditions hold today, measured. (1) `evaluate()` is
`evaluate_cheap() + evaluate_expensive()` (src/evaluation.cpp:1051-1052),
53.90 ns a call post-S104 (specs.md); the expensive half is mobility plus king
safety clamped to +/-LAZY_EVAL_MARGIN inside evaluate_expensive()
(src/evaluation.cpp:1045; the constant at src/search_params.hpp:128), and
paying it everywhere costs the 11.7 % above. (2) The TT does not cover quiescence:
S094 counted the quiescence probe finding **any** entry on 0.79 % of nodes
(kiwipete depth 12, 16 MB table) -- TT_DEPTH_QS = -1 sits below every main
depth, so any main store over the slot evicts a quiescence entry -- while
S103's 23.09 % warm hit rate is a main-search number. Chesso is
also single-threaded (DEC-089's 1CPU scale), so Stockfish's SMP argument is
vacuous here. DEC-039 retired exactly this cache once -- at 1.36 ns of
accumulator-only evaluate() against a 0.36 ns probe into 256 KB
(tools/probe_cost.cpp, pre-DEC-049 machine), a sub-1 % prize. At 53.90 ns,
with S121/S122 about to raise it, the same arithmetic reopens it; the stamp
should say so in one line.

Placement: **inside the evaluation module, at evaluate_lazy(), not inside
evaluate()**. Three reasons. `touches:` names evaluation files only and
quiescence already calls evaluate_lazy (src/search.cpp:257), so the mass of
calls is covered without touching search.cpp. evaluate() must stay pure:
bench_eval calls it in a loop over a fixed list (tests/bench_eval.cpp) and a
cache inside it turns the benchmark into a probe benchmark on the second
sweep; eval_spread and the tuner's oracle (eval_model::evaluate,
tools/eval_model.hpp:914, its own eval_model) stay untouched the same way.
And the main search is already TT-fed: post-S108 every non-check main node
reads or stores tt_eval, so the published division of labour lands as
TT-eval for the main search, dedicated cache for quiescence -- Arasan's
split. Key: `board->hash`, in hand at every
call; it covers side to move (src/bitboard.cpp:878-880), so INV-5 reads back
with no sign applied, S108's argument. Stored: the **raw clamped evaluate()
output** -- what src/evaluation.cpp:1107 returns -- never the :1099-:1039
bound (the accepts; S094's reason), and never a corrected value: S099 applies
its correction after the read, at the search site, and its raw-in-storage rule
covers this table too. Layering, bottom up: cache -> raw eval -> correction ->
consumers.

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
**after** the shortcut tests (:1038-1039): a hit returns the stored full
score, value-identical to :1046 by construction; a miss computes and stores.
The bound paths store nothing. evaluate()'s direct callers (src/search.cpp:434,
:529-530) neither probe nor fill -- they are TT-fed and rare. Tests,
red-first: property over a position list -- empty cache, wide window, call
twice, second equals fresh evaluate(); revisit through a made-and-unmade line
(INV-2 pattern); shortcut path stores nothing (establish `*exact == false`
first, then assert the probe misses -- non-vacuous); two keys sharing index
bits and differing tags do not answer each other; the INV-5 mirror pair each
equal their fresh call; clear() empties. Neutrality discharged per DEC-083:
search_bench node-identical at depths 9 and 12, one interleaved timing
recorded -- expect small, since a hit saves evaluate_expensive() only, the
cheap stage having been paid for the shortcut test.

(b) **Probe above the shortcut -- the SPRT.** Move the probe to the top of
evaluate_lazy(): a hit now returns the exact score where the shortcut would
have handed back cheap +/- margin -- the strictly-better-number argument
S094's stand-pat read makes at src/search.cpp:243-245. Not arguable into
neutrality: cutoff decisions cannot flip (full is on the bound's side of the
window by the :1038 arithmetic) but the fail-soft values propagated differ,
so node counts move by construction; a hit also sets `*exact`, so :253 writes
the score on to the TT -- consistent, both tables hold evaluate()'s output.
Hit-rate instrumentation rides this commit: probe/hit/store/bound-skip
counters in a throwaway or debug-flag build (S103's method), printed after
`go`, run over S103's own protocol -- 300 positions at depth 10 in one
process, plus the three search_bench positions at depth 12 -- and the
disagreement count against a fresh evaluate(), which must be 0. bench_eval is
not the vehicle; the accepts says a real search.

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
  Every cached value carries the 150 clamp -- self-consistent, the cache
  mirrors the live evaluate(), and when S039 changes the margin that is a new
  binary and a fresh cache. The live path is the tune build, where
  LazyEvalMargin is a setoption (src/search_params.hpp:128): clear the cache on
  that setoption, the same rule S108 records for the TT eval field. Were S039
  ever re-ordered ahead, nothing breaks -- the cache is indifferent to which
  margin it memoises.
- **evaluate() must stay a function of what the key covers.** True today:
  position and stm, both in the zobrist. A halfmove-clock scaling term (Weiss
  a7d77839 scales eval toward draw as the 50-move rule nears) would break the
  cache silently -- apply any such future scaling outside the cached function.
- **Sign is safe by key** -- stm is in the zobrist (src/bitboard.cpp:878-880),
  a key-matched entry is same-side, INV-5 unchanged; the mirror test proves
  non-interference rather than assuming it.
- **Collisions are bounded, not free.** A tag collision returns a wrong
  static eval, never a wrong bound or a crash -- Hyatt: "a collision won't
  crash a thing". 48-bit tag over a 2^16 table discriminates on the full 64;
  the instrumentation's 0-disagreements count is the collision check in vivo.
- **ucinewgame clears it** beside tt_reset (src/chesso.cpp:1128); a 512 KiB
  memset is free. Entries are position-pure, so persisting within a game is
  sound and published; clearing at game boundaries is the conservative form.
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

- **S108 (before).** Complementary, not rival. Probe order at quiescence
  stays TT first (src/search.cpp:251-257, probe already paid), cache second
  inside evaluate_lazy, compute third. S108 section 7 reserved exactly this:
  evictable opportunistic storage there, guaranteed storage here. Do not
  merge them.
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
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+eval+cache
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
