id:         S108
goal:       every node that is not in check computes its static evaluation once, stores it in its table entry and reads it back, so improving and the pruning margins have an input
accepts:    an SPRT verdict, recorded whatever it is; `improving` is `static_eval(ply) > static_eval(ply - 2)`, falling back to ply-4 when the ply-2 node was in check and defaulting to true when neither exists -- and the fallback is **tested**, because a broken improving calculation costs Elo without ever crashing; the eval is computed once per node and never twice, and the table entry no longer overwrites a stored evaluation with TT_EVAL_NONE; INV-4 holds -- the number comes from the accumulators `make_move` maintains and nothing stops maintaining them; the nps cost is measured and recorded next to the verdict
touches:    src/search.cpp negamax, src/transposition_table.cpp, src/data_structures.hpp
excludes:   consuming improving, which is S109 and the reduction work; correction history, which is S099
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Why this is a prerequisite and not a feature

`improving`, futility pruning, razoring, ProbCut's margin and correction
history all read a static evaluation at their own node. Today the main search
computes one **only at reverse-futility nodes** -- `static_eval` is
`TT_EVAL_NONE` everywhere else, by design, because S094 and S103 deliberately
declined to add a call.

So S092 as originally ordered -- the improving flag, first in the pending
order -- had no input to read and no consumer to feed. It is folded into this
step, which supplies the input, and its consumers arrive at S109.

## The bug this also fixes

`negamax` stores `static_eval` on every path, and `static_eval` is
`TT_EVAL_NONE` at every node except a reverse-futility one. So a main-search
store **overwrites a real evaluation that quiescence had already recorded** for
the same position. The field was added at S094 and is read at S103 and the
overwrite has been there since. Small, and it is exactly the kind of thing that
makes the hit rate at the S103 site lower than it should be -- 23.1 % over 300
positions at depth 10, 10.7 % on the three search_bench positions at depth 12.

## The cost, measured 2026-08-19

The lazy shortcut is load-bearing and this step spends part of it. `go movetime 2000`
from the start position, tune build, `LazyEvalMargin` at three settings:

| margin | nps | what it means |
|---|---|---|
| 0 | 6.55 M | expensive terms effectively off |
| 150, shipping | 5.46 M | the shortcut fires on most nodes |
| 2000 | 4.82 M | full evaluation at every node |

Full evaluation everywhere costs **11.7 %** against shipping. This step does not
go that far -- it computes at non-check main-search nodes and reuses through the
table -- but the number bounds the worst case, and `-march=native` at S104 has
already paid +16.7 % toward it.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `cf89e22`; re-locate by symbol if drifted. Every GitHub read
below was a commit message or PR conversation, never a diff (DEC-016).

### 1. State of the art

Four pieces of plumbing, published separately, each with a traced record:

- **A per-ply static-eval array in the search stack.** Every non-check node
  writes its eval to `stack[ply]` before recursing; descendants read their
  ancestors' slots. Weiss #135 "Eval history" (abdefb7, 2020-01-14):
  **+8.63 +/-5.98** at 10+0.1, **+7.87 +/-5.40** at 60+0.6 -- bundled with a
  null-move reuse trick ("the evaluation is just the opposite of the
  previous"), so part of that Elo is the reuse, not the array.
- **The static eval in the TT entry, separate from the score.** Weiss #653
  "Store static eval in TT" (0ce05a9, 2023-02-21): **+5.89 +/-4.85 STC,
  +4.28 +/-3.87 LTC** -- measured with every consumer (improving, futility,
  NMP conditions) already present, which chesso will not have until S109.
  Ethereal 25e56feb (2019) priced qsearch probe+eval together at +11.04.
  Chesso landed the field at S094 and measured zero -- the published sequence
  run in the other order, consumers absent.
- **A TT score bounded in the right direction as a better "eval" for pruning
  margins.** Weiss cca90ea (#336, 2020-09-08), **+10.78/+12.09**, scoped by
  its own message to "pruning heuristics" -- the main-search eval sites, not
  quiescence (S130's enrichment traced this; S130 took the quiescence half,
  the main-search half is this step's territory). Lynx prices the sites
  individually, far above 3000: #1973 RFP-only **+2.07 +/-1.50** (80488
  games, 8+0.08), #2055 NMP **+1.56 +/-1.27** (91990 games), after #1971
  removed a global version and #1975's first NMP cut failed.
- **The improving flag.** CPW "Improving": a boolean, whether the static eval
  has improved on the position two plies ago. Reference definition
  (Alexandria as described by CPW; description read, no source): compare
  eval(ply) with eval(ply-2); **if the ply-2 node was in check, against
  eval(ply-4)**; **false when currently in check**; **default true** with no
  prior data. Consumers per CPW: RFP margin lower when improving, LMP count
  raised, NMP more aggressive, LMR reduces more when not improving, ProbCut.
  Per-consumer records: Weiss #181 improving-in-RFP **+5.38 +/-4.23 /
  +3.14 +/-2.45** (2020); Weiss #684 removed the same at **-0.08/-0.22**
  (2023, ~3300 band) and #691 re-added it at **+1.73/+1.68**; Lynx #1133
  improving-in-RFP **+1.36 STC / +5.79 +/-4.14 LTC** (2024); Lynx #1135
  improving-in-LMR **+4.64 +/-5.07**; Ethereal 1706ef6 (2018) is the earliest
  traced use (LLR-only record). **The flag as bare infrastructure is untraced
  publicly**: Stockfish's introduction was not traced, and Stash bundled it
  (0980488, 2020: improving on NMP and LMR *and* "saved static evals in
  hash", one commit, no per-change number). Every traced number is
  flag-plus-consumer -- nobody measured the dead flag, consistent with this
  step expecting ~0 until S109. A "dynamic improving" rate variant exists
  (CPW credits Potential's author; Lynx #1133 cites the same source) -- noted,
  not for this step.

### 2. Shape for chesso

Exists (S094/S103): entry field `int16_t eval`, src/data_structures.hpp:410,
sentinel `TT_EVAL_NONE = INT16_MIN` (:384); store clamps and asserts it, never
a bound, never ply-normalised (src/transposition_table.cpp:134-139). Writers:
quiescence's four stores pass `stored_eval` (src/search.cpp:263, exact-only);
negamax's one store (:813-815) passes `static_eval`, TT_EVAL_NONE except at an
RFP node (:477, :532). Readers: quiescence stand-pat (:241-247) and the RFP
site (:529-530) via the copied-out `tt_eval` (:455). The overwrite bug:
`tt_store_entry` writes every field whenever replacement fires
(src/transposition_table.cpp:124 -- same generation needs `depth >=
entry->depth`, a new generation always replaces), so a main store carrying
TT_EVAL_NONE wipes a real eval, and TT_DEPTH_QS = -1 sits below every main
depth so any main store over a qs entry does exactly that. Sign is safe by
key: the zobrist includes side to move (src/bitboard.cpp:878-880, :1418), a
key-matched entry is same-side, INV-5's stm-relative value reads back with no
sign applied; improving compares ply with ply-2, same side both ends.

Missing:
- **Per-ply eval array.** search_state_t (src/data_structures.hpp:441-460)
  already holds per-ply arrays (killers :447, pv :453-454); add `int
  static_evals[MAX_PLY]` (:42). `search_state_t state = {}` is fresh per go
  (src/chesso.cpp:647), but guard the no-data default by ply arithmetic
  (ply >= 2 / 4), never by sentinel-compare on an unwritten slot -- 0 is a
  real eval.
- **Compute at every non-check node.** Today only inside the RFP guard. New
  site: after `is_in_check` (:469) and the TT-cutoff return (:460-463),
  before RFP (:511). Full `evaluate()`, not lazy -- a lazy bound is
  window-relative, unusable for cross-ply comparison or storage. In check:
  no call, slot gets TT_EVAL_NONE.
- **Read-back at the top.** The `tt_eval != TT_EVAL_NONE ? tt_eval :
  evaluate()` pattern (:529-530) hoists to the new site, guarded `!in_check`;
  RFP consumes the variable. PV nodes included -- value-identical, and Lynx
  #1999 records engines widening to PV exactly this way.
- **The improving helper.** Absent everywhere.
- **Store-side preserve.** :135-139 keeps the existing eval when the incoming
  one is TT_EVAL_NONE **and `entry->key == hash`** -- preserving across a key
  replacement would hand a foreign position's eval to the next reader, the
  hazard S103 named.

### 3. Implementation sketch

(a) **Compute + stack + helper; stores byte-identical -- behaviour-neutral,
proven.** Hoist the compute-or-read to the top of the node, write the stack
(sentinel in check), have RFP consume the variable, and keep the *stored*
value's semantics untouched for one commit: pass TT_EVAL_NONE except at
RFP-eligible nodes (one transitional variable, deleted in (b)). Neutrality is
arguable precisely: `evaluate()` is pure (`const board_t*`), the stack is read
by nothing in-tree, the read-back is value-identical (only exact values are
ever stored; S103 counted 3.37 M reads, 0 disagreements), and stores are
byte-identical -- so node counts and best moves cannot move, and search_bench
at depths 9 and 12 plus one interleaved timing (DEC-083) discharge it. This
commit is where the accepts' "nps cost measured and recorded" number comes
from, uncontaminated by later savings. `improving_at(state, ply, in_check)`
lands as a pure helper called by tests only -- the in-search call site ships
with its consumer at S109, so no dead flag sits in the hot path. Tests, per
branch, red-first: no-data defaults true; strict greater (equal is not
improving); ply-2 sentinel falls back to ply-4 -- observed red against the
no-fallback form, which compares against INT16_MIN and answers true for any
real eval, so assert a case with eval(ply) < eval(ply-4) and ply-2 in check
returning false; both sentinels default true; in-check false.

(b) **Store + read-back live + overwrite fix -- one SPRT.** Every store passes
the node's eval (TT_EVAL_NONE in check); tt_store_entry preserves on
key-matched TT_EVAL_NONE. **Not behaviour-neutral and not arguable into it**:
quiescence already stands pat on any entry's eval (:241-247), main-search
entries now carry one at ~every non-check node, and where `evaluate_lazy()`
would have returned a window bound the stored exact score is a different
number -- same cutoff decision, different fail-soft value propagated (:276,
:279, :287) -- so node counts move by construction. The plumbing's behaviour
change *is* feeding S094's starved consumer. Tests: red-first overwrite --
store key K with eval 300, store K deeper with TT_EVAL_NONE, read back 300
(today: TT_EVAL_NONE); factor the write decision so preserve provably never
survives a key change; computed-once (accepts) -- plant an entry with eval X
!= evaluate(), drive the node, the re-stored entry still carries X, since a
node that recomputed would have stored evaluate()'s value. S103's RFP-read
test stays green unmodified.

(c) **Optional, TT score as better eval for the RFP margin -- own verdict or
S109's.** Where the bound certifies direction (BETA and de-normalised score >
eval raises it, ALPHA and score < eval lowers it, mate band excluded), the
margin comparison uses the adjusted value; stack and stored eval keep the raw
static (improving compares statics, never search scores). Lynx #1973 measured
the RFP-only site +2.07; RFP is chesso's only margin consumer today. The
step's goal and accepts do not name this layer: a third commit with its own
SPRT here, or S109's first line -- decide explicitly, not silently.

### 4. Constants and seeds

No numeric constant ships. The offsets are structural, not fitted: INV-5
values are stm-relative, so the comparison needs the same side at both ends
and the offset must be even -- 2 is the nearest same-colour ancestor, 4 the
next. Seeds as published (CPW Improving, Alexandria reference): ply-2,
fallback ply-4, false in check, default true. Margins are S109's. DEC-084 is
satisfied vacuously.

### 5. Pitfalls

- **Evaluating in check.** Published practice: don't -- the node is forced and
  the number meaningless; write the sentinel, let the fallback work. Two
  chesso wrinkles. (1) Quiescence evaluates in check today (:244-247 runs
  before :262) and stores that eval, so an in-check position's entry can
  carry a real one -- the main search must not read `tt_eval` into the stack
  in check (leave TT_EVAL_NONE); changing what quiescence stores is out of
  `touches:`. (2) The slot is *written* on every recursing path, sentinel
  included -- a skipped write leaves the previous path's value at that ply and
  improving compares against a foreign line: the "costs Elo without ever
  crashing" failure the accepts names, and what the fallback test must
  observe.
- **eval field vs score field.** `score` (int32, :389) is the search value,
  ply-normalised at rest, always read through `de_normalize_score`; `eval`
  (int16, :408) is raw static, never normalised, never a bound (assert
  transposition_table.cpp:134). Never de-normalise the eval, never store a
  normalised value into it, and keep the two apart when (c) touches both.
- **Sign on read-back.** Safe *because* the key covers side to move
  (src/bitboard.cpp:878-880) -- INV-5 applies unchanged. The one sign trick in
  this area, Weiss's eval-after-null = -parent, is exact only while tempo
  ships at 0 weight and breaks the day S100/S126 fit it nonzero; the null
  child computes its own eval anyway -- skip the trick, keep this note.
- **Lazy-eval clamp (S039 note).** The stored eval is `evaluate()` = cheap +
  expensive-clamped-to-+/-150 -- deterministic per position, so store/read is
  self-consistent, but every stored eval carries the clamp. Release builds
  are stable; in the tune build a mid-session LazyEvalMargin setoption makes
  old entries stale against fresh calls -- flush the table between sweep
  points.
- **TT replacement decides which entries carry evals.** Depth-preferred in a
  generation (:124): a shallow store can be refused whole, its computed eval
  never persisted -- the compute is still paid. The preserve defends
  key-matched entries against TT_EVAL_NONE only; never across a key change.
  S119 inherits the rule.
- **stand_pat drift is the change.** After (b) quiescence reads main-search
  evals far more often; stand_pat becomes exact where it was a lazy bound.
  Intended -- but it is the mechanism by which a "plumbing" step alters play.
  Do not book it as neutral.

### 6. Measurement

- (a) owes no SPRT: identical node counts and best moves from
  tools/search_bench.py at depths 9 and 12 against the parent, plus one
  interleaved timing recorded as the nps cost (1.43 Elo/% named as a
  conversion, DEC-083). Expect it negative -- added `evaluate()` calls, ~54
  ns each at S104 baselines, nothing bought back yet -- and inside the file's
  11.7 % worst case, since main-search non-check nodes are a minority of all
  nodes.
- (b) owes **one SPRT** at the S105 regime (8+0.08, Hash 16, UHO book).
  **Bounds: non-regression, elo0=-5 elo1=0**, not the gainer pair: every
  traced positive for this plumbing had consumers present (Weiss #653) or a
  reuse bundled in (#135); the local precedent for consumer-less landings is
  S094's two zeros; the live effect is read-back savings minus compute cost
  plus a stand-pat tightening that measured zero -- an expected ~0 sits
  between 0 and +5 and random-walks a gainer run (DEC-063). Record the
  verdict whatever it is; a fail is bisected (overwrite fix alone vs
  compute+store).
- (c) if taken here: its own SPRT; gainer bounds only if S105 throughput
  makes a +2-resolving run affordable, else the same non-regression pair.
  Expected verdicts for the step: **1 SPRT** (2 with (c)), plus (a)'s timing.

### 7. Interactions

- **S103 (done, first consumer).** Its read collapses into the top-of-node
  site; the 23.09 % hit rate and 0 disagreements are the evidence the
  read-back generalises; its test stays green unmodified.
- **S130 (quiescence sibling, just before).** Different layer -- search score
  vs static eval -- and different function; no shared lines, both move
  stand-pat behaviour: land and measure separately, whichever order.
- **S109 (the consumer this exists for).** Improving gates the LMP count
  (doubled when improving) and the futility margins; the helper's in-search
  call site ships there; deferred (c) becomes its first line.
- **S114 / S116.** Eval-scaled null move and razoring read
  `static_evals[ply]` as-is; Lynx #2055 (+1.56, score-as-eval for NMP) is
  S114-band evidence, not this step's.
- **S099 (correction history).** The correction sits after compute-or-read
  and before every consumer (Lynx #1999 orders correction before improving);
  the TT eval field keeps the **raw** value or the correction compounds
  through storage. Whether the stack holds raw or corrected is S099's call.
- **S120 (eval cache).** Different layer: a dedicated by-key table for the
  full score behind the lazy shortcut, immune to TT replacement, shared with
  quiescence. The TT eval field is opportunistic storage in an evictable
  slot; S120 is guaranteed storage. Do not merge them.
- **S020.** negamax already computes is_in_check once per node (:469); this
  step reads it there. No conflict.

### 8. References

- https://www.chessprogramming.org/Improving -- definition, ply-2/ply-4
  fallback, in-check false, default true, consumer list, dynamic variant.
- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+static+eval --
  #135 +8.63/+7.87; #653 store-eval-in-TT +5.89/+4.28; cca90ea
  +10.78/+12.09; the NMP-condition family.
- https://github.com/TerjeKir/weiss/pull/135 -- the per-ply array and the
  null-move negation, quoted; exact SPRT tables.
- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+improving --
  #181 +5.38/+3.14; #684 -0.08/-0.22; #691 +1.73/+1.68; #383.
- https://api.github.com/search/commits?q=repo:lynx-chess/Lynx+improving --
  #1133, #1135, #1999 (correction and improving on PV nodes).
- https://github.com/lynx-chess/Lynx/pull/1133 -- improving in RFP, +1.36
  STC / +5.79 LTC, 129206/10316 games.
- https://github.com/lynx-chess/Lynx/pull/1135 -- improving in LMR, +4.64.
- https://github.com/lynx-chess/Lynx/pull/1973 -- TT score as eval for RFP
  alone, +2.07 +/-1.50, 80488 games at 8+0.08.
- https://github.com/lynx-chess/Lynx/pull/2055 -- same for NMP, +1.56
  +/-1.27; #1975's first cut failed.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+improving
  -- 1706ef6 (2018) earliest traced improving use; 01227d2 redefinition.
- https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+improving
  -- 0980488 bundles improving on NMP/LMR with "saved static evals in hash",
  no per-change number.
author:    Maksym Bodnar
