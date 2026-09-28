id:         S131
goal:       quiescence searches non-capture queen promotions instead of filtering them out
accepts:    an SPRT verdict, recorded whatever it is; queen promotions pass the filter, underpromotions stay out unless a node-count sweep says otherwise, and the step states which was shipped and why; the filter comment in `quiescence()` is rewritten to describe what is searched now rather than what used to be; the two fast-suite quiescence mate cases still pass -- "a side in check may not stand pat" (`tests/test_search.cpp` "a side in check may not stand pat") and "mate is recognised at depth zero" (`tests/test_search.cpp` "mate is recognised at depth zero"); the fast suite green
touches:    src/search.cpp quiescence, src/search_params.hpp, tests/test_search.cpp
excludes:   futility exemptions for promotions, which are S112's; the SEE treatment of promotions, unchanged here
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-28 14:46 CEST
done:

## Created by the second review, DEC-087 -- the engine's own recorded TODO

The filter in `quiescence()` keeps non-capture promotions out, and its own
comment, written at S094, says the quiet part out loud: "Letting them through
is very likely an improvement, but it is a search change and needs to be
measured in games." This step is that measurement and nothing else.

A pawn reaching the eighth is a capture-magnitude material event that the
capture-only stage cannot see: a horizon node with an unstoppable promotion
stands pat on a score that is about to be wrong by a queen. Every surveyed
engine's noisy stage includes queen promotions. No published isolated number
exists -- the seed here is this engine's own comment, which is why the verdict
is the whole content of the step.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

- **Published quiescence admits captures plus queen promotions;
  underpromotions stay out.** CPW Promotions states it directly: "In
  quiescence search most programs only consider queening", and for the main
  search some programs "omit minor promotions or at least underpromotions to
  rook and bishop inside their search except the root". CPW Quiescence Search
  itself is silent on promotions -- its one expansion topic is checks, with
  the explosion warning attached to those.
- **Traced engine records, prose only (URLs in section 8):**
  - Weiss 8b8f9516, 2020-09-18: "Quiescence search currently only tries
    captures and queen promotions and can therefore not play into a position
    drawn by repetition or 50 move rule" -- the move-set statement and its
    irreversibility corollary in one commit message.
  - The underpromotion margin, bracketed both ways at Weiss: #246
    (2020-04-14) regrouped underpromotions with the quiets, +4.90 +/- 4.61
    STC, +1.05 +/- 2.40 LTC; #779 (2026-08-01, "Generate under-promotions as
    noisy") retired the split again, 0.02 +/- 1.50 STC over 79908 games,
    2.25 +/- 2.57 LTC. Statistically zero in either direction.
  - Stockfish 785b7080 (2021-06-07) "eliminating underpromotions from
    qsearch" passed as a simplification at STC and LTC; 5c75c1c (2023-02-24)
    is a movepicker fix whose stated outcome is "treats every queen promotion
    as a capture" -- the noisy classification named as the intended design;
    d58e836 (2021-03-11) keys the qsearch prune on promotion status.
  - **The isolated delta for adding quiet queen promotions to a captures-only
    quiescence is untraced publicly.** The surveyed engines carried them in
    the noisy set from the start, so the record prices only the
    underpromotion margin. The step's premise stands: the seed is this
    engine's own comment.
- Forum and tutorial practice agree: talkchess t=60962 (hgm) locates the
  qsearch explosion risk in checks, not promotions; ChessML's quiescence doc
  lists promotions "always" searched beside captures.

### 2. Shape for chesso

Line numbers at 0edfd26 (`src/` byte-identical to cf89e22). S112 edits the
same filter loop first in plan order -- re-locate by symbol.

- - **The TODO.** The filter comment and the drop it describes
  (`!in_check && !MOVE_CAPTURE` skips) are both in `src/search.cpp`
  `quiescence`. Provenance verified
  with `git log -S "Letting them through"`: written at **S001** (1a42999,
  2026-08-08, the movegen split), not S094 as the prose above says -- same
  comment, same site, wrong step id, recorded here rather than edited above.
- - **Filter change, not generator change.** GEN_CAPTURES already emits quiet
  promotions, all four pieces, all inside `src/bitboard.cpp`
  `generate_moves_body`: `push_promotions`, `emit_promotions` for Q/R/B/N, the
  `Type != GEN_QUIETS` branch that emits them, and the pinned path -- by the
  INV-3 partition rule "Promotions count as captures whether or not anything is
  taken" (`src/data_structures.hpp` "GEN_CAPTURES and GEN_QUIETS partition
  GEN_ALL exactly"), the same fold every surveyed engine ships. The main search
  already searches them unfiltered through its captures stage (`src/search.cpp`
  `negamax`; no capture test in the `src/search.cpp` `negamax` loop; LMR-exempt
  at `src/search.cpp` `negamax`). The out-of-check qsearch filter is the one
  place they are invisible, so the change is one condition at `src/search.cpp`
  `quiescence`: drop only when `!MOVE_CAPTURE(m) && MOVE_PROMOTED(m) !=
  TO_QUEEN` (TO_QUEEN, `data_structures.hpp` `promotion_t`). A generator change
  instead would repartition INV-3 and move underpromotions between main-search
  stages for nothing.
- - **Ordering.** `src/search.cpp` `quiescence` scores survivors with
  capture_score() (`src/evaluation.cpp` `capture_score`); an EMPTY victim reads
  the deliberate 13th table entry, piece_values_abs[EMPTY] = 0
  (`evaluation.cpp` `piece_values_abs`; EMPTY = 12, `data_structures.hpp`
  `piece_t`), already exercised by quiet evasions (`search.cpp` `quiescence`).
  A quiet queen promotion therefore scores 0 - 100 = **-100**: after every
  non-king capture, before a king capture. Ship that. The score_move mirror
  (ORDER_CAPTURE + promotion bonus, `evaluation.cpp` `score_move`) is not
  reachable from search.cpp -- piece_values_abs is file-local, teaching
  capture_score the bonus would double-count inside score_move
  (`src/evaluation.cpp` `score_move`), and a second constant in search.cpp is
  the CLAUDE.md band hazard. At most pawns-on-the-seventh moves are admitted
  per node, and Lynx #302 measured promotion-ordering variants at 0.4 +/- 7.7
  -- leave placement to the node sweep.
- - **SEE applies unchanged, which is what `excludes` requires, and it prices a
  non-capture promotion by construction -- verified, not assumed.**
  capture_cannot_lose returns false on an EMPTY victim (`src/bitboard.cpp`
  `capture_cannot_lose`), so the admitted move falls through to see_ge
  (`src/search.cpp` `quiescence`): victim gain 0 (`src/bitboard.cpp` `see_ge`),
  then `see_value[promoted] - see_value[pawn]` = +800 and the recapture target
  becomes the queen (`src/bitboard.cpp` `see_ge`); `src/bitboard.cpp` `see`
  mirrors it in its own gain and target arithmetic. promotion_t values coincide
  with the see_value indices (TO_QUEEN = 4 -> 900). A promotion onto a
  defended, unsupported square is declined at -100.
- - **TT and draws.** A quiet promotion can now be a stored best_move; negamax
  already classes a promoted TT move as stage-one (`tt_move_is_quiet`,
  `src/search.cpp` `negamax`), matching where the generator emits it.
  Quiescence checks neither repetition nor the 50-move rule (only negamax does,
  `src/search.cpp` `negamax`); promotions reset the halfmove clock like
  captures, so Weiss 8b8f9516's rationale carries over unchanged.

### 3. Implementation sketch

One commit, tests first, using the `tests/test_search.cpp` "search: quiescence"
`quiesce()` helper and `state.explored_nodes` (incremented at `search.cpp`
`quiescence`).

1. **Red-first admission**: FEN `8/P6k/8/8/8/8/8/K7 w - - 0 1` -- no captures
   for either side, one quiet queen promotion; Stockfish plays **a7a8q**
   (verified 2026-08-19, `/usr/games/stockfish` depth 22, per DEC-023 -- the
   local build garbles its info lines but reports bestmove cleanly). Red
   today: `quiesce(fen, -inf, +inf) == evaluate()` and explored_nodes == 1.
   Green: score > evaluate() and explored_nodes == 2 (the promotion searched,
   its child stands pat).
2. **Underpromotions out**: same FEN, explored_nodes == 2 exactly -- 5 would
   mean all four promotions passed the filter (see_ge declines none of them
   here; a8 is unattacked).
3. **Capturing promotions unregressed**: they pass `src/search.cpp`
   `quiescence` today via MOVE_CAPTURE; one assertion so the new condition
   cannot lose them.
4. **SEE-gate mechanics, labelled as such**: `4r3/P6k/8/8/8/8/8/K7 w - - 0 1`
   (the e8 rook defends a8): admitted by the filter, declined by the existing
   gate at SEE -100, explored_nodes == 1. This pins the gate's reach, not the
   position's verdict -- Stockfish plays a7a8q here too, and re-deciding the
   gate is S022's business, excluded from this step.
5. 5. **The change**: extend `src/search.cpp` `quiescence`; rewrite the
   `src/search.cpp` `quiescence` comment to describe what is searched now (the
   accepts names this). The in-check path is untouched, so the two fast-suite
   quiescence mate cases ("a side in check may not stand pat",
   `tests/test_search.cpp` "a side in check may not stand pat", and "mate is
   recognised at depth zero", `tests/test_search.cpp` "mate is recognised at
   depth zero") gate as-is.
6. **Perft unaffected by construction**: no movegen edit; perft reads
   generate_moves and INV-3 is untouched. tools/search_bench.py counts will
   change -- play-altering, no identity claim (INV-6's SPRT arm); record the
   deltas as the datum.

### 4. Constants and seeds

None. TO_QUEEN is the existing enum. The one decision is ordering placement:
quiet queen promotions ride capture_score's EMPTY row at -100, searched after
the captures; revisit only if the node sweep says the placement costs nodes.

### 5. Pitfalls

- **S112's victim-0 auto-prune.** A quiet promotion has no victim, so an
  unexempted per-move futility test computes futility_value == futility_base
  <= alpha and silently prunes exactly the class this step admits. S112's
  MOVE_PROMOTED exemption precedes its victim arithmetic and lands first in
  plan order -- confirm it survived before this step measures anything.
  Published mirror: SF d58e836 and Weiss #748 key the qsearch prune on
  promotion status (-0.28 +/- 1.24 / +1.87 +/- 2.41 at Weiss).
- - **Explosion is bounded by construction.** Each admitted move consumes a
  pawn irreversibly (at most pawns-on-the-seventh per node) and the qply cap at
  `src/search.cpp` `quiescence` (MAX_QSEARCH_DEPTH = 19,
  `src/search_params.hpp` `MAX_QSEARCH_DEPTH`) holds regardless. The published
  explosion warning attaches to checks (talkchess t=60962); no engine record of
  a promotion-specific depth guard was found.
- - **Accepts wording, verified consistent.** Capturing underpromotions pass
  `src/search.cpp` `quiescence` today via MOVE_CAPTURE and stay in;
  "underpromotions stay out" governs the non-capture arm only. The node-count
  sweep the accepts allows is Weiss #779's exact question, measured 0.02 +/-
  1.50 there -- queen-only is the right default and the sweep decides on nodes
  alone.
- **The prose attribution above** ("written at S094") is wrong per git; S001
  wrote the comment. Section 2 records it; the accepts are unaffected.

### 6. Measurement

One SPRT at the S105 regime (8+0.08, Hash=16, UHO book), `elo0=0 elo1=5`,
fast suite green first, verdict recorded whatever it is. The record prices
only the underpromotion margin (~0 at Weiss and SF); the add itself is
untraced, so a ~0 verdict is a valid outcome and the whole content of the
step (CLAUDE.md rule 8, DEC-019). Cheap pre-check per the S094/DEC-079
precedent: count new-class filter passes over the depth-12 search_bench
positions -- a near-zero admission rate predicts the zero before the match is
bought. Note position 3's best move is already a promotion (d7c8q, a
capture). Record node deltas alongside.

### 7. Interactions

- **S112 (before)**: its MOVE_PROMOTED futility exemption is what keeps this
  step's moves unpruned; its section 5 anticipates this step by name --
  consistency verified both ways.
- **S022 (after)**: the delta/futility decision must price promotion gain (CPW
  Delta Pruning's +775-class allowance, read at S112), and is where the
  `src/search.cpp` `quiescence` gate's treatment of promotions is re-decided if
  ever.
- **S015/S091**: see_ge already prices promotions (section 2); S091 extends
  SEE pruning to the main search, where quiet promotions are LMR-exempt but
  not SEE-gated -- that step's business, not this one's.
- - **S130 (before)**: the TT-substituted stand-pat is orthogonal; a stored
  quiet-promotion best_move reads back through `src/search.cpp` `negamax`
  unchanged.
- **S030 (later)**: the 16-bit re-encode moves the MOVE_PROMOTED and
  MOVE_CAPTURE bits; the filter survives as macros. Note only.
- **S085/S127**: MAX_QSEARCH_DEPTH sweeps change how deep promotion lines
  run; this step adds no constant to the sweep set.

### 8. References

Every URL read for this section; commit/PR text read as prose only, no
diffs (DEC-016).

- https://www.chessprogramming.org/Promotions -- "In quiescence search most
  programs only consider queening"; underpromotions omitted in search except
  the root.
- https://www.chessprogramming.org/Quiescence_Search -- silent on promotions;
  the expansion topic there is checks, with the explosion caution.
- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22promotion%22
  -- the six Weiss promotion commits with SPRT numbers (#246, #346, #748,
  #779 among them).
- https://api.github.com/repos/TerjeKir/weiss/commits/8b8f9516 -- full
  "Simplify QS" message, the captures-plus-queen-promotions statement.
- https://github.com/TerjeKir/weiss/pull/779 -- underpromotion split retired,
  0.02 +/- 1.50 STC / 2.25 +/- 2.57 LTC.
- https://github.com/TerjeKir/weiss/pull/346 -- the draw-check removal the
  8b8f9516 message explains.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22promotions%22+%22qsearch%22
  -- SF 785b7080, 5c75c1c, d58e836, 252844e, aed542d messages.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+%22promotions%22+in:title
  -- Lynx #302/#519/#2427 promotion-ordering records (0.4 +/- 7.7 class).
- https://talkchess.com/viewtopic.php?t=60962 -- hgm on captures-first and
  the checks explosion; no measurements.
- https://www.dogeystamp.com/chess5/ -- captures-only qsearch devlog; no
  promotion number.
- -
  https://github.com/AlexanderBrevig/ChessML/blob/main/docs/quiescence-search.md
  -- tutorial doc; promotions "always" beside captures; its ~150-250 figure is
  for qsearch as a whole.
- https://int0x80.ca/posts/chess-engines/5-qsearch -- unreachable (HTTP 403);
  a "398.84 +/- 89.36" figure surfaced in search snippets could not be traced
  to any readable source and is not used.
- -
  https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+%22noisy%22+%22promotion%22
  -- zero results; Ethereal's classification is untraced in commit prose.

## What the tree already did, 2026-09-28 (checked at `029ff61`)

The step file was written on 2026-08-19; the implementing agent checked each
assumption on the tree before writing code, by symbol. `029ff61` is S112's
zero with per-move futility kept (DEC-236) and S113's REF; S113's ProbCut is
not on this tree.

- **The filter loop's shape** (`src/search.cpp` `quiescence`): out of check
  `generate_captures`, in check `generate_moves`; then, per move, (1) the drop
  `!in_check && !MOVE_CAPTURE(m)` under S001's comment, (2) S112's futility
  block, (3) S015's gate `!in_check && !capture_cannot_lose && !see_ge(0)`,
  (4) `capture_score` and the compaction. The condition extends (1), so it
  sits above both S112's block and S015's gate and neither changes.
- **S112's exemption survived** (the step's pitfall 1): the block opens `if
  (qs_futility && !MOVE_PROMOTED(m))`, before any victim arithmetic, so a
  promotion that takes nothing is exempt by construction. "futility does not
  skip a queen promotion that takes nothing" holds it, and V06 -- the
  exemption narrowed to the promotions that capture -- is the mutation only
  that case sees; S112's own promotion case stays green under it.
- **`capture_cannot_lose` on an EMPTY victim** (`src/bitboard.cpp`
  `capture_cannot_lose`) returns false, so the admitted move reaches
  `see_ge`, which prices it as section 2 says: gain 0 for the empty square,
  `see_value[promoted] - see_value[0]` added (+800 for a queen), and the
  queen is what stands on the square for the recapture; `see` mirrors it.
  On `4r3/P6k/8/8/8/8/8/K7 w - - 0 1` the engine's `see(a7a8q)` is below zero
  (-100 by that arithmetic) and `see_ge(a7a8q, 0)` false; the case asserts
  both.
- **`capture_score` on an EMPTY victim** (`src/evaluation.cpp`
  `capture_score`) reads `piece_values_abs`' 13th entry, 0, so a promotion
  that takes nothing scores 0 - `MVV_PAWN` = **-100**, as section 2 says.
  **Section 2's placement sentence does not hold and is kept above as
  written, with the correction here** (DEC-215 clause 3's form): -100 is not
  "after every non-king capture". MVV-LVA puts every capture by a piece worth
  more than its victim below it -- a knight or bishop taking a pawn at -200,
  a rook at -400, a queen at -800. What -100 is: after every capture of a
  piece worth at least its taker (score >= 0) and before every other (<=
  -200, the king's included); no capture scores in between, the bands being
  multiples of 200 apart. The same arithmetic made the old filter comment's
  "capture_score() ranks a quiet evasion below any capture" untrue for an
  evasion by a pawn, a minor or a rook (-100, -300, -500 against a queen's
  -800 for taking a pawn); the rewritten comment says what an evasion scores.
  Shipped at -100, as section 4 and the brief say: the placement is a question
  for the node sweep, which nothing invoked.
- **`tt_move_is_quiet`** (`src/search.cpp` `negamax_at`) is `tt_move != 0 &&
  !MOVE_CAPTURE && !MOVE_PROMOTED`: a promoted table move is stage one, where
  `generate_captures` emits it (INV-3). Quiescence already stored quiet
  promotions as best moves in check, so the class is not new to the table.
- **The in-check path is untouched**: the new term is conjoined under
  `!in_check`, and in check every generated move passes as before. "a side in
  check may not stand pat" and "mate is recognised at depth zero" are green
  throughout.
- **`MAX_QSEARCH_DEPTH`** is 19 (S085), as section 5 says; its return is
  above the loop.
- **S130 and S210.** S130's substitution precedes the loop and is untouched.
  S210's dead-board test sits in the search loop and applies to the admitted
  move, which never trips it: the queen it makes is on the board. **It is why
  the cases do not use section 3's FEN**: on `8/P6k/8/8/8/8/8/K7 w - - 0 1`,
  a7a8b and a7a8n leave a lone minor against a bare king, which S210 scores a
  draw without entering the child, so "5 would mean all four passed the
  filter" is 3 there, and a filter admitting the three underpromotions and not
  the queen counts 2 -- the shipped count. The cases use `8/P6k/8/8/8/8/7P/K7
  w - - 0 1`, where a second white pawn keeps every child alive.
- **Section 3's oracle readings, re-taken** (DEC-023, stockfish
  `dev-20260810-5062aee5` through python-chess, `go depth 22`, and Syzygy
  3-4-5). On section 3's FEN stockfish now answers **a1b2**, mate in 12, and
  mate in 11 with a7a8q as the only root move -- not the a7a8q the step
  recorded -- and the tablebase calls the root won with a7a8q keeping it. On
  `4r3/P6k/8/8/8/8/8/K7 w - - 0 1` it answers a1b2 (-7911), mate in 15
  against White with a7a8q forced, and the tablebase calls the root lost
  whatever White plays. On the cases' FEN it answers a7a8q, mate in 9, and
  the tablebase calls it won. None of it is asserted: the cases assert what
  quiescence searches (`.tuning/coord/S131_logs/stockfish_fens.txt`).
- **S113 is not on this tree.** Its preliminary calls `quiescence` directly,
  so on the rebase the admitted class is reached from there too; nothing in
  the condition depends on the caller.

## What landed

- `src/search.cpp` `quiescence`: the drop gains `&& !(QS_QUEEN_PROMOTIONS !=
  0 && MOVE_PROMOTED(moves[i]) == TO_QUEEN)`, and the filter comment is
  rewritten to say what is searched now -- every evasion in check; out of
  check every capture and every queen promotion, the underpromotions that take
  nothing dropped -- how the admitted move meets the two tests below, and
  what `capture_score` orders it at. The generation comment above it, which
  said the node "only ever searches captures", now says it searches what
  `generate_captures` emits less the filter's drops. Nothing else in `src/`:
  S015's gate, S112's block and the generator are untouched (INV-3's
  partition too). Noted and not changed: `negamax_at`'s null-move comment
  still calls quiescence a search that "only looks at captures", a reason
  about quiet mating moves that still holds.
- `src/search_params.hpp`: `QS_QUEEN_PROMOTIONS` ("QsQueenPromotions", 1,
  0..1), a verdict switch in `QsFutility`'s and `ProbCut`'s shape (DEC-215),
  its comment stating the class, the off value and that there is no seed.
- `tests/test_search.cpp` "search: quiescence promotions": "a queen promotion
  that takes nothing is searched", "an underpromotion that takes nothing is
  not searched", "a promotion that takes something is searched as a capture",
  "the exchange gate declines a queen promotion onto a defended square",
  "futility does not skip a queen promotion that takes nothing", and in the
  tune build "no promotion that takes nothing is searched at the switch's off
  value". Every count is derived from the engine's own move lists, each child
  asserted a leaf or at least entered; no golden. `tests/test_search_params.cpp`:
  the golden row, 64 -> 65 defaults.
- `tools/mutants/S131_quiet_queen_promotions.py`, V01 to V07.
- `MANUAL.md` the option row; `DEV_MANUAL.md` the bench ledger entry;
  `adocs/data/S131_sprt.sh` and its `adocs/data/README.md` row.
- `touches:` above gained `src/search_params.hpp`, the switch's home, which the
  step as written did not have.

## Which promotions ship, and why (the accepts)

**Queen only.** A promotion that takes nothing is searched when it is to a
queen; the three underpromotions that take nothing stay out, and a promotion
that takes something is searched as the capture it always was, all four
pieces. **No node-count sweep was run**, because nothing invoked the accepts'
"unless" clause: the published move set is queening only (CPW Promotions),
and the one engine that measured the underpromotion margin found it at zero
in both directions (section 1). What the census below counts of them: 33339
underpromotions that take nothing reach the filter loop at depth 12 over the
bench positions, three for every queen, and all of them are dropped. The
order stays `capture_score`'s -100 row, which nothing here measured against
another.

## Seeds (DEC-134)

None. The rule has no number in it; the switch's comment says so. The class
is the literature's, the wiki's Promotions page ("In quiescence search most
programs only consider queening"), and the order is a row the ordering table
already had.

## Admission rate, before any game (section 6)

An instrumented copy of the candidate (`.ref-builds/census`, the working
tree's `src/` plus write-only counters, `.tuning/coord/S131_census_instrumentation.patch`)
benches the candidate's own totals -- 1993284 at depth 12, 4784357 at 14,
every reply identical -- so its counters change nothing. Out of check:

| run | filter-loop moves | queen, takes nothing | declined by S015's gate | kept | searched | raised the best | cut off |
|---|---|---|---|---|---|---|---|
| `bench 12`, eight positions | 1060240 | 11113 (1.05 %) | 10678 (96.1 %) | 435 | 235 | 52 | 155 |
| `bench`, depth 14 | 2790336 | 27798 (1.00 %) | 26282 (94.5 %) | 1516 | 877 | 234 | 501 |
| `search_bench` 12, three positions | 653306 | 10867 (1.66 %) | 10445 (96.1 %) | 422 | 226 | 52 | 146 |

At depth 12 over the bench positions the class reaches 11096 of the 313149
nodes that reach the filter loop, and the 235 searched are 0.030 % of the
791316 quiescence nodes. **Near zero by DEC-079's measure** (S094's 0.05 %
reach, whose verdict was the zero it predicted): the zero is predicted before
the match is bought, and the pre-registration says so. `.tuning/coord/S131_admission.txt`.

## Measurements (2026-09-28, niced beside S113's SPRT; counts only)

- **Off value** (DEC-215), a parent built from `029ff61` in
  `.ref-builds/parent`: the tune build at `QsQueenPromotions` 0 prints `bench`
  **4649650** with all eight replies the parent's (c3d5 d5e6 d7c8q g7h8q d8e7
  a1b2 e5e6 e5e6), `bench 12` 1981759 with its replies, and `search_bench`
  reproduces the parent at 9 (53598 / 80389 / 25691) and 12 (154098 / 472358
  / 107876), best moves c3d5 e2a6 d7c8q at both. `.tuning/coord/S131_identity.log`,
  its driver `.tuning/coord/S131_identity.sh`.
- `bench` 4649650 -> **4784357** (+2.90 %), the eight replies unchanged; the
  tune build at the defaults prints the same 4784357. `bench 12` 1981759 ->
  1993284, replies unchanged.
- `search_bench` 9: 53598 -> 65737, 80389 -> 80432, 25691 -> 25691; 12:
  154098 -> 152807, 472358 -> 485208, 107876 -> 107876; best moves unchanged.
  The tactical position's tree is the parent's at both depths.
- **Tests, red first and observed** (`.tuning/coord/S131_logs/red_stageA.log`):
  with the switch, its golden row, the MANUAL row and the suite in place and
  the filter unchanged, "a queen promotion that takes nothing is searched" was
  red on all three assertions (nodes 1 against 2, score 449 against the
  static 449, stored move 0 against a7a8q), "an underpromotion that takes
  nothing is not searched" red (1 against 2), "futility does not skip a queen
  promotion that takes nothing" red (1 against 2), and the tune-only off-value
  case red at its control (1 against 2); "a promotion that takes something is
  searched as a capture" and "the exchange gate declines a queen promotion
  onto a defended square" green, as they must be on a tree that already keeps
  capturing promotions and keeps the quiet ones out. With the condition in,
  all six green in both builds (`.tuning/coord/S131_logs/green_stageB.log`).
- **Both fast suites green, 41 of 41 each** -- Release `build` and
  `-DCHESSO_TUNE=ON` `build-tune`, serial `ctest -L fast`, niced -- and
  `./clang-format.sh --check` clean (`.tuning/coord/S131_logs/suites.log`,
  `suite_release.log`, `suite_tune.log`). "a side in check may not stand pat"
  and "mate is recognised at depth zero" green in both. **"pruning does not
  hide a forced mate" and `test_mate_carry` green in both, so no mined golden
  moved on this tree and none was re-derived**; the capture-mate rows' mutant
  labels were not re-swept, a red being the trigger (DEC-233); row 2 of that
  table, whose comment counts four promotions among its legal moves, held
  like the rest. `test_mate_carry` read 118.11 s Release and 117.46 s tune
  against its 120 s ceiling -- a load reading beside S113's SPRT, which S112
  recorded at the same edge; the rebase re-times it on the idle machine. `tools/plan_prose_check.py --citations`, `--touches`,
  `--params`: 0 flagged.
- **Mutation** (DEC-141 clause 2), `tools/mutants/S131_quiet_queen_promotions.py`
  on a clean detached fixture, `.ref-builds/mut` at `9780c65` -- a throwaway
  commit of this working tree on no branch, whose `src/` is the landing's but
  for the generation comment above the filter, corrected after it was cut --
  header `baseline green, 41 tests, bench 4784357 nodes via engine`;
  **mutation score 6 of 6 (100 %), V05 equivalent as declared**, wall 2739 s
  (`.tuning/coord/S131_mutation.log`, `S131_mutation_results.tsv`). Every kill
  by its named case: V01 by the admission, underpromotion and futility cases;
  V02 by the same three, and S097's multicut row of "pruning does not hide a
  forced mate" with them; V03 by the capture case alone; V04 by the admission,
  underpromotion and futility cases; V06 by the futility case alone, S112's
  own promotion case green under it; V07 by the exchange-gate case alone. V05
  leaves the release build's bench and suite unchanged, as argued in its row,
  and is killed in the tune build by the off-value case, observed by hand in a
  throwaway worktree (`.ref-builds/v05`): nodes 2 against 1, score 987 against
  449, stored move a7a8q against 0, the other five cases green
  (`.tuning/coord/S131_logs/V05_tune_red.log`). **S112's own F03** -- its
  promotion exemption dropped -- run on the same fixture with `--only`, is
  killed by S112's "a capturing promotion below the threshold is searched"
  and by this step's "futility does not skip a queen promotion that takes
  nothing" together (`.tuning/coord/S131_mutation_f03.log`): the brief's red
  for the S112 interaction, observed.
- **A test_engine failure the harness cannot name.** The first mutation
  launch was refused on a red baseline: `test_clang_format_script`, because
  the detached run lacked `CLANG_FORMAT_MAJOR=22` (the operator's slip), and
  `test_engine`, one assertion of 870240 failed with no assertion text in the
  log. The same signature recurred once in the second pass, in V01's row,
  where test_search also killed it. The same fixture binary passed 8 of 8
  direct runs (six with doctest's `--out=` so a report could not be lost) and
  the worktree's two suite runs. Why no text: `stdout_capture_t`
  (`tests/test_helpers.hpp` `stdout_capture_t`) swaps `std::cout`'s buffer,
  and doctest's console reporter writes to `std::cout`, so an assertion that
  fails inside a capture is reported into the capture and discarded. Not
  established here: which assertion, and whether the parent flakes the same
  way. Test-side, outside this step (DEC-171); for the coordinator.
  (`.tuning/coord/S131_mutation_try1_refused.log`,
  `.tuning/coord/S131_logs/engine_flake/`)
- Not run here, by the brief: DEC-141's Debug self-play and
  `tools/gate_extra.sh` (a match holds the machine), and the SPRT.

## Proposed `specs.md` sentence (the coordinator edits specs.md)

**No sentence in `specs.md` states what quiescence searches out of check.**
The nearest two, quoted exactly, and both stay true:

- the engine-state table's `exchange evaluation` row: "`see()` exact,
  `see_ge()` fast; quiescence declines losing captures"
- S210's clause in the `search` row: "unconditional, not gated on a capture,
  because a quiet promotion changes material and quiescence searches quiet
  moves in check."

Proposed, in the `search` row after S112's sentence (the one ending "folded
into the node's fail-soft value and stored as an upper bound"):

> **Quiescence searches quiet queen promotions, S131 (landed <date>,
> DEC-221)**: out of check it searches every capture and every promotion to a
> queen, the ones that take nothing included -- `generate_captures()` emits
> all four promotions (INV-3) and the filter drops only the three
> underpromotions that take nothing -- and the admitted move meets S112's
> promotion exemption and S015's exchange gate as any promotion does, ordered
> at `capture_score()`'s empty-victim row, -100. At 0 `QsQueenPromotions`
> gives the tree before it, node for node (DEC-215). Before any game the
> class was 1.05 % of the filter loop's moves over the bench positions at
> depth 12, and the exchange gate declined 96.1 % of it; the verdict is
> `adocs/data/S131_sprt.sh`'s.

And the `exchange evaluation` row, so it names what the gate now also prices:
"`see()` exact, `see_ge()` fast; quiescence declines losing captures and
losing queen promotions".

## Fast-check fix-ups, 2026-09-28

The coordinator's cold fast check found no defect and five fix-ups; each is
made here in the same worktree, under the same brief. Nothing above is
rewritten: where a sentence above is superseded, this section says so.

1. **S112's "a capturing promotion below the threshold is searched" could
   have gone green with every capturing promotion skipped.** Its premise
   listed `legal_captures` only and it asserted `nodes >= 2`; since this step
   a quiet queen promotion the exchange gate let through would add a node of
   its own. Read from the engine, not the board
   (`.tuning/coord/S131_logs/fixup_promo_probe.txt`, a probe linked against
   the Release engine library): on `r7/1P5k/8/8/8/8/8/4K3 w - - 0 1` the pawn's
   push b7b8 is four promotions that take nothing, each with
   `capture_cannot_lose` false, `see` -100 and `see_ge(0)` false -- the gate
   declines all four -- and the four b7a8 captures pass on
   `capture_cannot_lose`, b7a8b and b7a8n leaving a dead board. The case now
   asserts both halves: the premise (four promotions that take nothing, every
   one declined by the gate), and the move the root settled on, read from its
   own table entry, is a capturing promotion. `qs_run`'s comment now says a
   count above 1 is any searched move, a queen promotion that takes nothing
   included, and `legal_captures`' that it is not everything the filter keeps;
   a `legal_quiet_promotions` helper sits beside it. V09 is the masking made
   a mutant: S112's exemption dropped and the gate skipped for quiet
   promotions together, which the old assertion could not see.
2. **"an underpromotion that takes nothing is not searched" had no evidence
   of its own.** It now reads, for each of the three underpromotions, whether
   the run entered the position it leaves -- the table holds an entry for
   every node quiescence enters on these boards, `entered` in the suite --
   and asserts none was, with `nodes <= 1 + 1` beside it. A searched
   underpromotion and nothing else turns it red. The exact count the
   admission case carried is split between the two cases whose claims it
   joined: the admission case asserts the queen's child entered and `nodes >=
   1 + 1`, and so does "futility does not skip a queen promotion that takes
   nothing", whose claim that is. **The board route the coordinator offered
   first does not exist on this engine**: a position where the gate declines
   the queen promotion and passes an underpromotion. Over the 149334
   underpromotions that take nothing reaching the filter loop in the bench
   trees at depths 12 and 14 and the `search_bench` trees at 12, `see_ge`'s
   answer equalled its queen twin's every time, 0 disagreements either way
   (`.tuning/coord/S131_underpromotion_gate.txt`, a write-only census whose
   benches are the candidate's 1993284 and 4784357). The arithmetic agrees:
   the promotion's gain and the promoted piece's loss cancel, so the sign of
   the exchange does not depend on the piece. V01 and V08 (the knight admitted
   beside the queen) are the mutants only this case kills.
3. **The tune-only off-value case's restorer** now puts back the value it
   found, not a literal 1, and the case restores and checks it explicitly at
   its end (S132's pattern); the destructor covers a failed `REQUIRE`.
4. **"The exchange gate prices the promotion itself" was true only of a
   promotion that takes nothing.** `capture_cannot_lose` passes every pawn
   capture, so a capturing promotion never reaches `see_ge`. The filter
   comment, the switch's comment, the SEE-gate case's comment, the `MANUAL.md`
   row, the `DEV_MANUAL.md` entry, the `adocs/data/README.md` row and the
   pre-registration now say exactly that. **The proposed `specs.md`
   sentences above are superseded by these two**:

   > **Quiescence searches quiet queen promotions, S131 (landed <date>,
   > DEC-221)**: out of check it searches every capture and every promotion
   > to a queen, the ones that take nothing included -- `generate_captures()`
   > emits all four promotions (INV-3) and the filter drops only the three
   > underpromotions that take nothing -- and a queen promotion that takes
   > nothing meets S112's promotion exemption and S015's exchange gate, which
   > it reaches through `see_ge()` because `capture_cannot_lose()` answers
   > false on an empty target; a capturing promotion is a pawn's capture and
   > never reaches `see_ge()`. It is ordered at `capture_score()`'s
   > empty-victim row, -100. At 0 `QsQueenPromotions` gives the tree before
   > it, node for node (DEC-215). Before any game the class was 1.05 % of the
   > filter loop's moves over the bench positions at depth 12, and the
   > exchange gate declined 96.1 % of it; the verdict is
   > `adocs/data/S131_sprt.sh`'s.

   and for the `exchange evaluation` row: "`see()` exact, `see_ge()` fast;
   quiescence declines losing captures and a queen promotion that takes
   nothing onto a square it cannot hold -- a pawn's capture, a capturing
   promotion among them, is let through by `capture_cannot_lose()` before
   `see_ge()` is asked".
5. **The test_engine finding is S242** (`adocs/plan_todo/S242_test_engine_capture_flake.md`
   in the main checkout): the two timing-dependent
   `deepest_completed_depth(capture) < 1` CHECKs of "the first iteration
   honours stop and the hard timer" flake under load, and doctest's failure
   text is swallowed inside `stdout_capture_t`. The pre-registration names it
   as open finding 8, DEC-171's filler class; the "Not established here"
   sentence in the measurements above is answered there.

**Re-measured after the fix-ups** (niced beside S113's SPRT, counts only):

- `bench` 4784357 unchanged, the eight replies unchanged, and the tune build
  at `QsQueenPromotions` 0 still prints 4649650: the fix-ups are tests and
  comments. Both fast suites 41 of 41, `test_mate_carry` 117.36 s Release and
  115.80 s tune against its 120 s ceiling (the match's load); the format check
  clean; `--citations`, `--touches`, `--params` 0 flagged
  (`.tuning/coord/S131_logs/fixup_chain.log`).
- **Mutation, second pass**, on a fixture re-cut from this working tree,
  `.ref-builds/mut` at `796297f` (src/, tests/ and the mutant list byte-equal
  to the tree's): header `baseline green, 41 tests, bench 4784357 nodes via
  engine`; **mutation score 8 of 8 (100 %), equivalent 1, killed 8**, wall
  3497 s (`.tuning/coord/S131_mutation2.log`, `S131_mutation2_results.tsv`).
  Which case kills which:

  | mutant | killed by |
  |---|---|
  | V01 queen test dropped | "an underpromotion that takes nothing is not searched" alone |
  | V02 queen test inverted | the admission case, the underpromotion case, the futility case, "pruning does not hide a forced mate" |
  | V03 capture arm broken | "a promotion that takes something is searched as a capture" alone |
  | V04 switch inverted | the admission case and the futility case; S112's promotion case green, the tree without the class |
  | V05 switch ignored | equivalent in the release build as declared; in the tune build the off-value case, red on four assertions (nodes 2 against 1, the queen's child entered, score 987 against 449, stored move a7a8q against 0), observed by hand on `796297f` (`.tuning/coord/S131_logs/V05_tune_red_pass2.log`) |
  | V06 exemption narrowed to capturing promotions | the futility case alone |
  | V07 gate skipped for quiet promotions | "the exchange gate declines a queen promotion onto a defended square" alone |
  | V08 knight admitted beside the queen | the underpromotion case alone |
  | V09 exemption dropped and gate skipped | S112's promotion case (at its count), the SEE-gate case, the futility case, "pruning does not hide a forced mate" |

  V01 and V08 are the mutants only the underpromotion case kills, which is
  item 2's ask. **V09 was not the masking item 1 names**: dropping S112's
  exemption whole prices the quiet queen promotion at the empty square too,
  so on S112's board it is skipped with the captures and the count alone goes
  red. V10 is the masking -- the exemption narrowed to promotions that take
  nothing, the gate skipped for them -- run alone on a re-cut fixture:
  `.ref-builds/mut` at `f31ea69`, fixture 2 plus V10 in the list, header
  `baseline green, 41 tests, bench 4784357 nodes via engine`, **killed, 1 of
  1** (`.tuning/coord/S131_mutation3_v10.log`). S112's promotion case is red
  only at the new assertions: its count held (`nodes >= 2`, the quiet queen
  promotion searched in the captures' place -- the masking itself) and the
  root's entry read 0 for both the capture and the promotion bit, a node
  that settled on no capturing promotion. The SEE-gate case kills it too,
  for the gate half. Over the ten mutants that makes **9 of 9 killed, V05
  equivalent in the release build**, and V10 is the mutant S112's case now
  catches that its old count could not.
- **S112's F03 alone**, `--only` on `796297f`: killed by S112's promotion case
  (at its count, and two more assertions) and this step's futility case
  (`.tuning/coord/S131_mutation2_f03.log`).
- The mutant file's header prose was brought in line with the second pass
  after the fixture that ran V10 was cut; its pairs did not move.

## Rebased onto S113's tree, 2026-09-28

S113 read H1 and ProbCut stayed (`Elo 7.70 +/- 5.41`, `nElo 9.87 +/- 6.93`,
9654 games), so this step lands on `3c7cf84`, `bench` 3591364, with ProbCut
live in `negamax_at` -- whose preliminary calls the `quiescence` this step's
filter is in. Everything above was measured on `029ff61` and stands as
written; what follows was re-taken on the rebased tree and nothing in it is
quoted from the old one (DEC-233). The session's report is
`.tuning/coord/S131_rebase_report.md` and its log
`.tuning/coord/S131_rebase_log.md`.

**Rebase.** The step's work, committed as a WIP on `s131` on top of `029ff61`,
rebased onto `3c7cf84` with two conflicts, both sides kept and S113's first:
`tests/test_search_params.cpp` -- S113's line adding four, then this step's
sentence, and the golden count 68 -> **69**, 69 rows counted -- and
`DEV_MANUAL.md`'s bench ledger, S113's two paragraphs above this step's. No
file in `src/` conflicted: the filter sits above the one comment S113 changed
in `quiescence`, S210's, and in `src/search_params.hpp` the
`QsQueenPromotions` row stays after `QsFutilityMargin` while the four
`ProbCut` rows are after `SeMultiCut`, the list's own order, which the golden
table follows. `MANUAL.md`, `adocs/data/README.md` and `tests/test_search.cpp`
merged on their own. `git diff --stat 3c7cf84 -- src tests tools` lists this
step's five files only, every hunk byte-equal to the WIP's but the golden
count's resolution, and each anchor of the ten mutants occurs once in the
rebased `src/search.cpp`. At git's default similarity the step file's move
from `plan_todo/` reads as a delete and an add, as it did on `029ff61`; at
30 % it pairs as the rename it is.

**Off value** (DEC-215, DEC-233), the parent re-cut in `.ref-builds/parent`
at `3c7cf84` (no `src/` or `tests/` difference) and measured: `bench`
3591364, replies c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6; `bench 12`
1700466; `search_bench` 9 at 34773 / 71512 / 25413 and 12 at 110496 / 244771
/ 117798, best moves c3d5 e2a6 d7c8q; its own tune build at the defaults
3591364 too. The tune build at `QsQueenPromotions` 0 reproduces every one of
those numbers and replies (`.tuning/coord/S131_identity_rebased.log`, driver
`.tuning/coord/S131_identity_rebased.sh`).

**Candidate.** `bench` 3591364 -> **3429473** (-4.51 %), and **one reply
moves**: the second position's, kiwipete's, d5e6 -> e2a6, where on `029ff61`
all eight held -- recorded and not judged (CHESS); e2a6 is the reply both
builds give on that position at depth 12. The tune build at the defaults
prints the same 3429473. `bench 12` 1700466 -> 1694808 (-0.33 %), replies
unchanged. `search_bench` 9: 34773 -> 48304, 71512 -> 71580, 25413 -> 25413;
12: 110496 -> 104784, 244771 -> 244824, 117798 -> 117798; best moves
unchanged, and the tactical position's tree is the parent's at both depths.
Counts only: the change alters play, so INV-6 does not discharge it.

**Admission rate** at depth 12 (section 6), from an instrumented copy of the
rebased tree: `.ref-builds/census` at the rebased WIP, with the counters of
`.tuning/coord/S131_census_instrumentation.patch` re-applied by anchor
(`.tuning/coord/S131_rb_census_apply.py`; the resulting diff,
`.tuning/coord/S131_census_instrumentation_rebased.patch`, adds exactly that
patch's lines). It benches the candidate's own totals -- 1694808 at 12,
3429473 at 14, `search_bench` 12 at 104784 / 244824 / 117798, every reply
identical -- so its counters change nothing
(`.tuning/coord/S131_admission_rebased.txt`). Out of check:

| run | filter-loop moves | queen, takes nothing | declined by S015's gate | kept | searched | raised the best | cut off |
|---|---|---|---|---|---|---|---|
| `bench 12`, eight positions | 795143 | 6735 (0.85 %) | 6499 (96.5 %) | 236 | 143 | 47 | 79 |
| `bench`, depth 14 | 1618764 | 12616 (0.78 %) | 12272 (97.3 %) | 344 | 203 | 69 | 108 |
| `search_bench` 12, three positions | 393796 | 6446 (1.64 %) | 6212 (96.4 %) | 234 | 142 | 46 | 79 |

At depth 12 over the bench positions the class reaches 6709 of the 248994
nodes that reach the filter loop, and the 143 searched are 0.023 % of the
626337 quiescence nodes, against 0.030 % on `029ff61`: **near zero by
DEC-079's measure**, and lower on this tree. The underpromotions that take
nothing are three for every queen here too, 20205 at depth 12, all dropped.

**Suites.** Both fast suites **41 of 41** -- Release `build` and
`-DCHESSO_TUNE=ON` `build-tune`, `-j8`, serial `ctest -L fast`, the idle
machine -- and `./clang-format.sh --check` clean
(`.tuning/coord/S131_rb_logs/suites.log`, `suite_release.log`,
`suite_tune.log`). **No red, so no mined golden went red with the
combination**: "pruning does not hide a forced mate" -- S095's row, S097's
multicut row, S113's ProbCut row and the capture-mate table, each as it stands
on `3c7cf84` -- green in both builds, and run by name as well
(`named_cases.log`); `test_mate_carry` green at its budgets as they stand.
**`test_mate_carry` alone: 55.66 s Release and 57.21 s tune** against its 120
s ceiling (55.76 s and 57.13 s inside the suites); the 118 s read on `029ff61`
was S113's match's load, as the measurements above said. "a side in check may
not stand pat" and "mate is recognised at depth zero" green in both; the six
cases of "search: quiescence promotions" and S112's "a capturing promotion
below the threshold is searched" green in both, run by name.
`test_uci_surface` green without a refresh. Not re-swept, a red being the
trigger (DEC-233): the mutant labels of the capture-mate table.

**Mutation** (DEC-141 clause 2), `tools/mutants/S131_quiet_queen_promotions.py`
on a fresh fixture, `.ref-builds/mut` at `e509994` -- a throwaway commit of the
rebased working tree on no branch, its `src/` and `tests/` the landing's and
its mutant list the landing's but for header prose -- built from a wiped build
directory: header `baseline green, 41
tests, bench 3429473 nodes via engine`; **mutation score 9 of 9 (100 %),
equivalent 1, killed 9**, wall 1462 s (`.tuning/coord/S131_rb_mutation.log`,
the per-mutant logs in `.tuning/coord/S131_rb_logs/mutation_main/`). Every
kill is by its named case, as the fix-ups table has them, but for two
incidental kills this tree does not make: V02 and V09 are no longer killed by
"pruning does not hide a forced mate", whose kills there came from
`029ff61`'s multicut row. V05 is equivalent in the release build as declared,
and red in the tune build by hand on `e509994` (`.ref-builds/v05`): "no
promotion that takes nothing is searched at the switch's off value" red on
four assertions -- nodes 2 against 1, the queen's child entered, score 987
against 449, a stored move 262152 against 0 -- the suite's other five cases
green (`.tuning/coord/S131_rb_logs/V05_tune_red_rebased.log`).

| mutant | killed by, on the rebased tree |
|---|---|
| V01 queen test dropped | the underpromotion case alone |
| V02 queen test inverted | the admission case, the underpromotion case, the futility case |
| V03 capture arm broken | the capture case alone |
| V04 switch inverted | the admission case and the futility case |
| V05 switch ignored | equivalent in the release build; the off-value case in the tune build, by hand |
| V06 exemption narrowed to capturing promotions | the futility case alone |
| V07 gate skipped for quiet promotions | the SEE-gate case alone |
| V08 knight admitted beside the queen | the underpromotion case alone |
| V09 exemption dropped and gate skipped | S112's promotion case (at its count and two more), the SEE-gate case, the futility case |
| V10 exemption narrowed to quiet promotions, gate skipped | S112's promotion case at the move its root settles on, and the SEE-gate case |

Two targeted extras on the same fixture, S113's rebase's precedent:
`B04_probcut_defender_gate_dropped` **killed**, 2 of 41 -- "pruning does not
hide a forced mate" at S113's ProbCut row, "mate probcut hides, depth 11",
"probcut does not run with beta in the mate band", and `test_mate_carry` --
where S113's rebase saw it red at the multicut row first
(`.tuning/coord/S131_rb_mutation_b04.log`); and
`E21_multicut_mate_band_gate_dropped` **survived**, 0 of 41 red, bench
unchanged (`.tuning/coord/S131_rb_mutation_e21.log`).

**A finding, and the landing waits on the coordinator's ruling: on this tree
nothing in the fast suite sees E21.** E21 drops the two terms that keep the
multicut's returned value out of the mate band (`src/search.cpp`
`negamax_at`), and its one recorded kill is the multicut row of "pruning does
not hide a forced mate". On `3c7cf84` that row separates it -- shipped `d13
d14` against E21's `d13` over depths 11 to 14, the numbers S113's re-mine
recorded -- and on this tree both builds report its mate at 12, 13 and 14:
with quiet queen promotions in quiescence both find it a ply earlier, and E21
no longer loses it at 14. Re-derived by its own script, stages 2 to 6 of
`adocs/data/S097_mine_mate_row.py` as S113's rebase ran them: **no candidate
qualifies**. One of the 269 separates the sweeps,
`7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42`, shipped m6 at 11 to
14 and E21 none at 14, and the firing witness shows the shipped multicut
changing nothing on it, so the pre-registered pick skips it
(`adocs/data/S131_remine_s097.log`, the parent's cells beside the
candidate's). **No test was changed**: the row stays as it stands, still a
mate the shipped build must find at 14, and what the gap is owed is not this
step's to choose -- a pick rule that takes the liveness witness on the
mutant's build for a guard mutant, a direct guard test on the multicut's
returned value, a wider corpus, or an acceptance recorded as a decision. It
is test-side: the guard is in the engine and untouched. An H0 on S131 closes
it, `QsQueenPromotions` 0 being the parent's engine, where the row separates
E21 again. The pre-registration names it as open finding 9, and
`DEV_MANUAL.md`'s DEC-142 row for the multicut row now says it no longer
separates on this tree.

**The ruling, DEC-238, applied the same day: the row re-derived in the miner's
guard mode.** The coordinator ruled that a row witnessing a *guard* takes its
firing witness on the mutant's build -- the rule must fire there, the shipped
guard must stop it, and the shipped build must keep the mate the mutant loses
-- with the rest of the miner's rule unchanged, and created S243, a direct
guard test, as the filler that ends the corpus dependence.
`adocs/data/S097_mine_mate_row.py` gained `--mode guard` on `fires` and `pick`
(its header's "GUARD MODE, DEC-238"): in that mode `fires` drives a tune
library built with the mutant applied and says so in the file it writes, and
`pick` reads only a witness written in its own mode. The default, `--mode
rule`, reproduces every earlier derivation: over three recorded input sets --
S097's own, S113's rebase's and this tree's refusal above -- its pick prints
byte for byte what the unedited script printed, and S113's miner, which runs
S097's `fires` and `pick` with a namespace of its own and no mode, reproduces
its recorded pick byte for byte too. Stages 2 to 6 re-run fresh on this tree
(`.tuning/coord/S131_rb_mine_guard.sh`), both sweeps equal to the earlier ones
in every mate reading and node count: the guard-mode witness on E21's tune
library reports `SeMultiCut` 0 at m6 through depth 14 in 1688502 nodes and 1
at no mate there in 1436564, the default-mode witness on the shipped tune
library 0 and 1 identical, and **`pick --mode guard` takes
`7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42` at depth 14, mate in
6** -- shipped `d11 d12 d13 d14`, guard dropped `d11 d12 d13`, the mutant's
rule firing at 14 -- 1688502 nodes and about 0.26 s against S113's row's
13206867 and about 2.0 s on this tree; the default mode over the same inputs
refuses it again. Stockfish at depth 20 in a fresh process through
python-chess: `#+6` for White, the side to move, in 8285 nodes, pv Kc1 Rc3+
Kb2 Ng5 Rxg5 Rb3+ Kxb3 c4+ Kc3 a5 Rg8#, the label
`adocs/data/S097_candidates.tsv` held; python-chess: valid, in check, 4 legal
moves, no capture, no promotion. The row replaces S113's in "pruning does not
hide a forced mate" (`tests/test_search.cpp`, `mate_in` 4 -> 6 at the same
depth 14), S113's row quoted in the GOLDEN block with the reason it left and
the mode that derived the new one. **Observed red, then green**: with E21
applied, a Release build of the new test file fails the case at `REQUIRE(
result.mate_found )`, "mate the multicut hides, depth 14", and passes with the
guard in place; the case is green in both of this worktree's builds
(`.tuning/coord/S131_rbg_red.log`). On an H0 the row goes back to S113's byte
for byte: on `3c7cf84`'s engine the new row reports no mate at 14, and S113's
separates E21 there (`adocs/data/S131_remine_s097.log`, both modes' runs and
the parent's cells). Open finding 9 of the pre-registration now reads resolved
by DEC-238, and S243 is its item 10.

**The final pass, on the tree that lands.** `tools/mutation_check.py` over
`tools/mutants/S131_quiet_queen_promotions.py` on a fresh fixture that is the
landing commit itself -- `.ref-builds/mut` at `23c880f`, a wiped build
directory; the commit that carries this paragraph differs from it in this
paragraph and the next and in the miner's two `getattr` reads for S113's
miner, with their notes, none of it read by the build or a ctest test, and
`src/`, `tests/` and `tools/` are the fixture's byte for byte: header `baseline green, 41 tests, bench 3429473 nodes via engine`;
**mutation score 9 of 9 (100 %), equivalent 1, killed 9**, wall 1412 s
(`.tuning/coord/S131_rbg_mutation.log`). The kills are the rebased table's
above but for one: V04 is red in "pruning does not hide a forced mate" as
well, at the new multicut row -- the tree without the class is `3c7cf84`'s
engine, which reports no mate there at 14. A pre-check had predicted exactly
that, each of the ten against that case alone on a Release build of the new
test file (`.tuning/coord/S131_rbg_precheck.log`), and the mutant file's
header table was set from it before the fixture was cut, so the list the
fixture ran is the landing's byte for byte. V05, equivalent in the release
build, is red in the tune build by hand on `23c880f` on the same four
assertions (`.tuning/coord/S131_rb_logs/V05_tune_red_final.log`). Targeted on
the same fixture: **`E21_multicut_mate_band_gate_dropped` killed**, 1 of 41,
by "pruning does not hide a forced mate" at "mate the multicut hides, depth
14" (`.tuning/coord/S131_rbg_mutation_e21.log`), and
`B04_probcut_defender_gate_dropped` killed, 2 of 41, at S113's ProbCut row,
"mate probcut hides, depth 11", with "probcut does not run with beta in the
mate band" and `test_mate_carry` -- the new multicut row holds under B04
(`.tuning/coord/S131_rbg_mutation_b04.log`).

| mutant | killed by, on the tree that lands |
|---|---|
| V01 queen test dropped | the underpromotion case alone |
| V02 queen test inverted | the admission case, the underpromotion case, the futility case |
| V03 capture arm broken | the capture case alone |
| V04 switch inverted | the admission case, the futility case, "pruning does not hide a forced mate" at the multicut row |
| V05 switch ignored | equivalent in the release build; the off-value case in the tune build, by hand |
| V06 exemption narrowed to capturing promotions | the futility case alone |
| V07 gate skipped for quiet promotions | the SEE-gate case alone |
| V08 knight admitted beside the queen | the underpromotion case alone |
| V09 exemption dropped and gate skipped | S112's promotion case (at its count and two more), the SEE-gate case, the futility case |
| V10 exemption narrowed to quiet promotions, gate skipped | S112's promotion case at the move its root settles on, and the SEE-gate case |

**The final tree's gate**, on `23c880f`
(`.tuning/coord/S131_rb_logs/final2.log`): both fast suites 41 of 41, Release
and tune, `test_uci_surface` green without a refresh; `test_mate_carry` alone
55.84 s Release and 57.10 s tune against its 120 s ceiling; `./clang-format.sh
--check` clean; `tools/plan_prose_check.py --citations` and `--touches` 0
flagged and `--params` exit 0; `bash -n` of the pre-registration clean;
`bench` 3429473 with the eight replies above.

**`specs.md`, re-checked on the rebased tree.** The fix-ups section's two
sentences hold clause by clause against the rebased code -- the filter, S112's
exemption, `capture_cannot_lose()` answering false on an empty target and
true on every pawn capture, `capture_score()`'s -100 and the off value are
each untouched by the rebase -- and three things move: the census figures,
0.85 % and 96.5 % where they read 1.05 % and 96.1 %; the placement, because
S113's ProbCut sentence now follows S112's, so this one goes after S113's,
the one ending "the settings stay at their seeds for S127 -- `bench`
3591364."; and the verdict clause, which takes S113's landing form. **They
supersede the fix-ups section's two**:

> **Quiescence searches quiet queen promotions, S131 (landed <date>,
> DEC-221)**: out of check it searches every capture and every promotion to
> a queen, the ones that take nothing included -- `generate_captures()` emits
> all four promotions (INV-3) and the filter drops only the three
> underpromotions that take nothing -- and a queen promotion that takes
> nothing meets S112's promotion exemption and S015's exchange gate, which it
> reaches through `see_ge()` because `capture_cannot_lose()` answers false on
> an empty target; a capturing promotion is a pawn's capture and never
> reaches `see_ge()`. It is ordered at `capture_score()`'s empty-victim row,
> -100. At 0 `QsQueenPromotions` gives the tree before it, node for node
> (DEC-215). Before any game the class was 0.85 % of the filter loop's moves
> over the bench positions at depth 12, and the exchange gate declined 96.5 %
> of it. Decided by one `{0, 5}` nElo SPRT against the tree without it
> (`adocs/data/S131_sprt.sh`): <verdict>.

and for the `exchange evaluation` row, unchanged from the fix-ups section:
"`see()` exact, `see_ge()` fast; quiescence declines losing captures and a
queen promotion that takes nothing onto a square it cannot hold -- a pawn's
capture, a capturing promotion among them, is let through by
`capture_cannot_lose()` before `see_ge()` is asked".

**Documents.** `adocs/data/S131_sprt.sh`: the measured figures replaced by the
rebased tree's, dated; "THE REFERENCE" paragraph in the past tense; the open
findings re-read -- S113's items carried, item 4 now the reference tree's rows
and green, item 6 open now that S113's code stayed, item 7 no golden red on
either tree, item 8 (S242) unchanged, item 9 the E21 finding, resolved by
DEC-238, and item 10 S243; `REF` and `CAND` still `PIN_ME`, the refusal tested
on a copy pointed at the worktree; `bash -n` clean. `DEV_MANUAL.md`'s ledger
entry is `3591364` -> `3429473`, and its DEC-142 row for the multicut row
gives the guard-mode re-derivation; `adocs/data/README.md`'s row for the
pre-registration takes the new figures, a row for
`adocs/data/S131_remine_s097.log` is added, and `S097_mine_mate_row.py`'s row
names its guard mode; `MANUAL.md`'s row holds no number that moved and is
unchanged. The mutant file's header table is the final pass's -- V02's and
V09's rows lose the forced-mate case, and V04's gains it at the new multicut
row -- and its pairs did not move.

**Second tier on the landing** (DEC-141): Debug self-play 8 games at 4+0.04,
0 `Assertion`, 0 `disconnect` (`.tuning/coord/S131_debug_selfplay/`);
`tools/gate_extra.sh` 5 stages green in 1124 s (`.tuning/gate_extra_2026-09-28_S131.log`).
Landed as `13caf43` on `2d6b8f1`, `bench` 3591364 -> 3429473. SPRT pair
pinned: `REF` `2d6b8f1` (the tree S113's H1 left, its engine `3d97737`'s node
for node), `CAND` `13caf43`; open findings re-read at pinning: items 1 to 10
as the pre-registration states them, nothing moved since the rebase agent's
final pass.
