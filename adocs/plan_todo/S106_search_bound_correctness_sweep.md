id:         S106
goal:       the transposition bound signs and the mate-score round trip are checked against a red test in both searches, not assumed
accepts:    a test that fails when the alpha and beta bound conditions in `tt_entry_answers` are swapped, and a second that fails when either arm of the normalize/de_normalize pair drops its ply term, both observed red and the printout recorded; the tests cover **quiescence as well as the main search**, since quiescence is the only writer that reaches +/-MATE_MAX; a test that a mate found at ply p and read back at ply q reports the same distance to mate; whatever the sweep finds is fixed in this step under the house rule that a found bug is fixed before anything else starts, and each fix gets its own verdict; if nothing is found, "nothing found" is the recorded outcome and the tests stay
touches:    tests/test_search.cpp, src/search.cpp, src/transposition_table.cpp
excludes:   the entry layout, which is S119; any new pruning rule
decisions:  DEC-025
closes:
blocks:
paused_by:
done:

## Why this is cheap insurance and goes early

Two published failures in this exact area are worth more than most features on
this plan:

- One engine had the upper and lower bound inverted in its quiescence store.
  Fixing one line moved it from 44.5 % to 53.7 % over 2000 games -- about
  **64 Elo** from a defect no test caught.
- Mate scores not adjusted by ply on **both** store and probe produce "mate in
  N" announced forever while the engine shuffles. Reported at **100+ Elo** in
  the affected endgame class.

This is not hypothetical here. S094 found exactly one of these: `de_normalize_score()`
excluded +/-MATE_MAX, quiescence is the only writer that reaches it, and a mate
with no legal reply read back **three plies short** until the bound was made
inclusive. One defect of this shape has already shipped in this file. The prior
that there is another is not low, and every measurement taken above a live one
is contaminated.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

The published semantics (CPW, Transposition Table page) are three store
conditions and three cutoff conditions. Store: a node whose best value stayed
at or under the original alpha stores an **upper bound** (fail-low, All-node);
one that reached beta stores a **lower bound** (fail-high, Cut-node); a value
strictly inside the original window is **exact** (PV-node). Probe, gated first
on `entry.depth >= wanted depth`: EXACT always answers; LOWERBOUND answers iff
`score >= beta`; UPPERBOUND answers iff `score <= alpha` (CPW words it
`< alpha`; `<=` is the fail-soft-consistent inclusive form -- a ceiling equal
to alpha still cannot beat alpha). PV nodes take no TT cutoff. Under fail-soft
(CPW, Fail-Soft page) returned values may sit outside the window -- "an upper
bound less than alpha at All-Nodes, a lower bound greater than beta at
Cut-Nodes" -- so stored scores legitimately exceed the window and the probe
returns the stored value unclamped.

Mate scores: CPW's Checkmate page states the whole rule in one sentence --
mate scores "need ply-adjustment if stored as exact score inside the
transposition table, and re-adjustment if retrieving". Store converts
root-relative (`MATE_MAX - ply_of_mate`) to node-relative ("mate in k from
here"); probe converts back by the reading node's ply. The comparison against
alpha/beta must happen on the re-adjusted score, not the raw stored one.

The published record of what missing pieces cost:

- **Unadjusted mate scores**: talkchess t=15496 ("Mate handling with
  transposition tables") records the canonical symptom in KRvK -- the engine
  "announces mate in some bogus number ... then starts wandering around, still
  announcing mate, but making no progress and even sometimes allowing its rook
  to be captured". Cause named there: path-dependent mate scores in the hash;
  fix: adjust on store and probe. The thread quantifies no Elo; the step's
  "100+ Elo in the affected endgame class" figure could not be traced to a
  public record on 2026-08-19.
- **The 64-Elo bound-sign inversion the plan cites (44.5 % to 53.7 % over 2000
  games)**: not found in a public record after a targeted search (talkchess,
  engine changelogs, devlogs, MadChess blog). The arithmetic is internally
  consistent (44.5 % = -38 Elo, 53.7 % = +26, delta 64), but treat the figure
  as unsourced; it decides nothing anyway (DEC-019) -- the step's premise
  stands on S094's own in-repo defect.
- **A traced, quantified one-line bookkeeping defect of the same family**:
  MadChess 2.0 Beta build 27 -- root position wrongly caught by the
  repetition-draw check, every root move scored `-Score.Max`; the trivial fix
  measured **+47 Elo** (1766 to 1813) on the author's gauntlet. One-line
  search-bookkeeping semantics errors sit at tens of Elo when they fire.
- talkchess t=79412 ("Transposition tables are hard...") is the debugging
  record: probe-ordering vs quiescence, PV corruption ("white plays twice"),
  and the testing advice -- single-solution mate positions, Zobrist verified
  separately from TT logic.

### 2. Shape for chesso -- the audit checklist

Every store, every probe, both searches, verified against the source at
`20d058a`-era line numbers (re-locate by symbol if drifted):

Stores -- five sites, all normalise via `normalize_score()` (src/search.cpp:120):
1. `negamax` sole store, src/search.cpp:813-815: fail-soft `best_so_far`,
   type from the loop -- init `TT_ALPHA_NODE` (:590), `TT_BETA_NODE` on
   `score >= beta` (:739), `TT_PV_NODE` on `score > alpha` (:763). The PV flip
   happens exactly when the score enters the window, so the stored type
   matches the published store conditions. Mated/stalemate returns at :779-781
   *before* the store -- why the main search never writes +/-MATE_MAX and
   quiescence is the only writer that does. Aborts return before storing
   (:422, :729).
2. Quiescence stand-pat cutoff, :273-275: `TT_BETA_NODE`, move 0. The lower-
   bound claim leans on `evaluate_lazy()`'s fail-high branch being a true
   lower bound (comment :268-272) -- audit that direction; a wrong-side lazy
   bound poisons an entry that outlives the window.
3. Quiescence capture fail-high, :353-357: `TT_BETA_NODE` with move. Standard.
4. Quiescence mate (in check, no legal reply), :377-378: exact, stored as
   exactly -MATE_MAX. The S094 endpoint.
5. Quiescence final store, :387-390: exact vs upper decided against **alpha0**
   (:204), the original bound -- deciding against raised alpha misfiles exact
   scores as ceilings; verify no site compares against the raised alpha.

Probes -- two sites plus the shared answerer:
6. `tt_entry_answers()`, src/search.cpp:155-188: depth gate :164,
   de-normalisation **before** the bound comparison :170 (the correct
   published order -- a mate bound is compared re-based), EXACT :172,
   `TT_BETA_NODE && tt_score >= beta` :177, `TT_ALPHA_NODE && tt_score <=
   alpha` :182. Reads correct against CPW; the step's point is that no test
   fails if :177 and :182 swap.
7. `negamax` probe, :457-463: `!is_pv && ply > 0`, and the repetition/50-move/
   insufficient-material checks run before it (:432-442) -- the standard
   partial GHI mitigation. 8. Quiescence probe, :216-224: accepts every entry
   (`TT_DEPTH_QS`), no PV guard (justified :213-215), and **no draw checks
   before it** -- see pitfalls.

Constants: `MATE_MAX` 49000 / `MATE_MIN` 48000 (:16-17), band 1000 > MAX_PLY
128 (src/data_structures.hpp:42) so a mate score cannot decay out of the band;
`entry.score` is `int32_t` (data_structures.hpp:389) so no narrowing today --
S119 repacks the entry and inherits these tests as its guard. `entry.eval`
(int16) never carries a mate or a bound by the store contract
(src/transposition_table.hpp:27-41, assert at transposition_table.cpp:134).
UCI distance conversion at :841-856. One doc defect spotted: the
`TT_PV_NODE` comment block (data_structures.hpp:363-367) carries a pasted
copy of the alpha-node text -- comment-only, file not in `touches:`, fix or
note at the implementer's judgement.

Existing coverage and the gap: tests/test_search.cpp:772-802 (depth gate) and
:810-834 (S094 endpoint regression) drive `tt_entry_answers` with **PV entries
only**; the `TT_BETA_NODE`/`TT_ALPHA_NODE` uses elsewhere (test_search.cpp:889,
:936, :1004, :1473, :1584; test_engine.cpp:269-279) test replacement and
layout, never the cutoff conditions. Grep says the bound-swap mutant passes
today's suite; the step's first red test verifies that by mutation, not grep.

### 3. Implementation sketch -- red tests first, in order

1. **Unit, bound conditions** (accepts item 1): through `tt_entry_answers`
   (exported, src/search.hpp:26-31), one case per type x {answers, refuses},
   fail-soft scores outside the window: BETA 500 vs beta 100 answers 500; BETA
   50 inside (0,100) refuses; ALPHA -500 vs alpha 0 answers; ALPHA 50 refuses;
   EXACT always answers. Precondition-first per the S094 pattern (:782-786).
   Then swap :177/:182 locally, observe red, record the printout, revert.
2. **Unit, adjusted-before-compared**: a BETA entry holding a mate lower bound
   read at plies p != q -- the cutoff decision must use the re-based score
   (property of :170's ordering). Extends :810-834 from PV to bound types.
3. **Store-side normalise, quiescence** (accepts item 2, store arm): drive
   `quiescence()` directly at ply 3 (exported; the existing :704-715 position
   -- in check, no legal reply -- is the input), then read the raw entry via
   `tt_get_entry`: score must be exactly -MATE_MAX, type exact, depth
   TT_DEPTH_QS. A dropped `- ply` in `normalize_score()`'s negative arm stores
   -(MATE_MAX - 3) instead: red by construction at ply > 0.
4. **Whole-search round trip** (accepts item 3): search `MATE_IN_2_W_POS`
   (data_structures.hpp:20) warm; advance one ply along the reported PV;
   re-search sharing the table; the reported distance must shrink by exactly
   the plies advanced under the :853-855 convention. This is the read-back-at-
   ply-q test and the t=15496 symptom in miniature. Optional slow-suite
   variant, the published symptom's own class: KRvK
   `8/8/8/7k/8/8/8/R3K3 w - - 0 1` (stockfish depth 28: mate 16, this
   machine, 2026-08-19) played out move by move on one table, announced
   distance never increasing -- adopt only if chesso announces the mate at
   fast-suite depth; check first.
5. Then the audit down checklist items 1-8, each mismatch fixed in this step
   (house rule), one fix per commit, each with its own verdict.

### 4. Constants and seeds

None to fit. Facts the tests pin: MATE_MAX 49000, MATE_MIN 48000, TT_DEPTH_QS
-1, TT_EVAL_NONE INT16_MIN, band width 1000 >= MAX_PLY 128. The test file
already pins MATE_MAX locally on purpose (test_search.cpp:767-770) so a drift
is a visible disagreement -- keep that pattern.

### 5. Pitfalls

- **Both directions or neither.** Adjusting only store or only probe recreates
  t=15496's shuffle: the engine announces mate and stops converting. The S094
  fix was the endpoint of exactly this pair.
- **Fail-hard habits in a fail-soft engine.** Chesso returns and stores values
  outside the window; clamping on store or probe, or comparing bounds against
  the wrong window edge, silently converts fail-soft to something neither.
  Any future clamp-on-probe (some engines do, for stability) is a recorded
  decision, not a drive-by.
- **alpha vs alpha0.** Exactness is relative to the bound the node *started*
  with (:204, :389 in quiescence). Deciding with the raised alpha misfiles
  exact scores as upper bounds -- strength loss, no crash, no wrong node count.
- **GHI / draw interaction -- scope decision.** negamax orders draw checks
  before the probe (:432-442); quiescence probes with none (:216-224), and
  in-check evasions make repeated positions reachable inside it. Path-
  dependent draw scores also flow into the table from below, and the halfmove
  clock is not in the key. The published state (CPW GHI; t=27283) is that
  engines accept this. **Proposed scope: verify the negamax ordering property
  stays (cheap test), record the rest as accepted** -- it is not a bound-sign
  defect, and any change there is a new search rule, excluded by `excludes:`.
- **Repo hazards.** A found bug is fixed before anything else starts; one
  change at a time, one verdict per fix; do not add `evaluate()` calls while
  testing (INV-4) -- drive `quiescence()`/`negamax()` directly, both exported
  for exactly this (src/search.hpp:26-54); mate positions for new tests come
  from published suites or are verified with `/usr/games/stockfish` (DEC-023),
  as the KRvK FEN above was.

### 6. Measurement

This step owes: each red test observed red under its mutation and the printout
recorded in this file (accepts); the fast suite green after (DEC-025 -- the
gate is necessary, never sufficient); "nothing found" recorded as the outcome
if the audit finds nothing, tests kept. If a defect is found and fixed: a fix
that alters play gets its own SPRT under the S105 regime (8+0.08, Hash=16,
UHO book, stated bounds -- S105 lands immediately before this step; if it has
not, run the current fastchess.sh defaults and say so in the verdict); a fix
argued behaviour-neutral proves it with identical node counts and best moves
from tools/search_bench.py plus an interleaved timing (INV-6, DEC-083).

### 7. Interactions

- **S094 (done)** built what this sweep hardens: TT_DEPTH_QS separation, the
  eval field, the endpoint fix. Its stamp records the observed-red discipline
  this step repeats.
- **S130** will take `entry->score` as quiescence stand-pat *where the stored
  bound permits* -- it consumes the bound conditions directly. A swapped
  condition there is "stand pat on the wrong-side bound": pure strength loss,
  invisible to node-count checks. S130 must also never stand pat on a raw
  stored mate score -- de-normalisation before any use is what this sweep
  pins. (Context for its expected size: talkchess t=47373 records +25 Elo /
  4000 games for qsearch TT use in one engine; Weiss's record is
  +10.8/+12.1.)
- **S108** widens the writers of `entry.eval` from two sites to every
  non-check node; the contract this sweep verifies -- eval is never a bound,
  never a mate score, TT_EVAL_NONE when absent -- must hold for every new
  writer, and the store-contract assert (transposition_table.cpp:134) is
  where it is enforced.
- **S119** repacks the entry; the mate band does not fit int16, so the
  round-trip tests written here are the regression guard S119 runs against.

### 8. References

- https://www.chessprogramming.org/Transposition_Table -- store/probe/cutoff
  conditions, depth gate, PV-node exclusion, hash-move validation note.
- https://www.chessprogramming.org/Checkmate -- ply adjustment required in
  both directions for TT mate scores; root-relative convention.
- https://www.chessprogramming.org/Fail-Soft -- fail-soft returns outside the
  window; TT/search-instability caution.
- https://talkchess.com/viewtopic.php?t=15496 -- the KRvK announce-and-shuffle
  record for unadjusted mate scores; adjust-on-store-and-probe fix; no Elo
  quantified.
- https://talkchess.com/viewtopic.php?t=79412 -- TT debugging record: probe vs
  qsearch ordering, PV corruption, single-solution-mate testing advice.
- https://www.madchess.net/2014/12/10/madchess-2-0-beta-build-27-draw-by-repetition-bug/
  -- traced +47 Elo from a one-line root/repetition bookkeeping fix.
- https://www.chessprogramming.org/Lasker-Reichhelm_Position -- fine70
  (`8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -`), the published TT efficiency
  position, Kb1; useful as a TT smoke test, not a bound test.
- https://talkchess.com/viewtopic.php?t=47373 -- measured records for TT use
  in quiescence (+25 Elo / 4000 games in one engine); context for S094's zero
  and S130's expected band.
- Untraced: the 44.5 % -> 53.7 % / 2000-game / 64-Elo bound-inversion record
  and the "100+ Elo" mate-adjustment figure; searched 2026-08-19, not found
  in public records; flagged above, premise unaffected.
