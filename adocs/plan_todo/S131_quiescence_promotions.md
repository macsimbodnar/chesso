id:         S131
goal:       quiescence searches non-capture queen promotions instead of filtering them out
accepts:    an SPRT verdict, recorded whatever it is; queen promotions pass the filter, underpromotions stay out unless a node-count sweep says otherwise, and the step states which was shipped and why; the filter comment in `quiescence()` is rewritten to describe what is searched now rather than what used to be; the two fast-suite quiescence mate cases still pass -- "a side in check may not stand pat" (tests/test_search.cpp:993) and "mate is recognised at depth zero" (tests/test_search.cpp:1067); the fast suite green
touches:    src/search.cpp quiescence, tests/test_search.cpp
excludes:   futility exemptions for promotions, which are S112's; the SEE treatment of promotions, unchanged here
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
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

- **The TODO.** The filter comment is src/search.cpp:462-466, the drop itself
  :310 (`!in_check && !MOVE_CAPTURE` skips). Provenance verified with
  `git log -S "Letting them through"`: written at **S001** (1a42999,
  2026-08-08, the movegen split), not S094 as the prose above says -- same
  comment, same site, wrong step id, recorded here rather than edited above.
- **Filter change, not generator change.** GEN_CAPTURES already emits quiet
  promotions, all four pieces: src/bitboard.cpp:194 (push_promotions),
  :207-223 (emit_promotions, Q/R/B/N), :229-244 (`Type != GEN_QUIETS` emits
  them at :236), pinned path :261-263 -- by the INV-3 partition rule
  "Promotions count as captures whether or not anything is taken"
  (src/data_structures.hpp:130-133), the same fold every surveyed engine
  ships. The main search already searches them unfiltered through its
  captures stage (src/search.cpp:875; no capture test in the :635-666 loop;
  LMR-exempt at :693-694). The out-of-check qsearch filter is the one place
  they are invisible, so the change is one condition at :310: drop only when
  `!MOVE_CAPTURE(m) && MOVE_PROMOTED(m) != TO_QUEEN` (TO_QUEEN,
  data_structures.hpp:148). A generator change instead would repartition
  INV-3 and move underpromotions between main-search stages for nothing.
- **Ordering.** :327 scores survivors with capture_score()
  (src/evaluation.cpp:1125-1135); an EMPTY victim reads the deliberate 13th
  table entry, piece_values_abs[EMPTY] = 0 (evaluation.cpp:41-43;
  EMPTY = 12, data_structures.hpp:168), already exercised by quiet evasions
  (search.cpp:458-460). A quiet queen promotion therefore scores 0 - 100 =
  **-100**: after every non-king capture, before a king capture. Ship that.
  The score_move mirror (ORDER_CAPTURE + promotion bonus,
  evaluation.cpp:1151-1158) is not reachable from search.cpp --
  piece_values_abs is file-local, teaching capture_score the bonus would
  double-count inside score_move (:1094), and a second constant in search.cpp
  is the CLAUDE.md band hazard. At most pawns-on-the-seventh moves are
  admitted per node, and Lynx #302 measured promotion-ordering variants at
  0.4 +/- 7.7 -- leave placement to the node sweep.
- **SEE applies unchanged, which is what `excludes` requires, and it prices a
  non-capture promotion by construction -- verified, not assumed.**
  capture_cannot_lose returns false on an EMPTY victim
  (src/bitboard.cpp:1168-1170), so the admitted move falls through to see_ge
  (:322-325): victim gain 0 (:1203-1204), then
  `see_value[promoted] - see_value[pawn]` = +800 and the recapture target
  becomes the queen (:1209-1212); see() mirrors it (:1293, :1300-1307).
  promotion_t values coincide with the see_value indices (TO_QUEEN = 4 ->
  900). A promotion onto a defended, unsupported square is declined at -100.
- **TT and draws.** A quiet promotion can now be a stored best_move; negamax
  already classes a promoted TT move as stage-one (`tt_move_is_quiet`,
  :611-612), matching where the generator emits it. Quiescence checks neither
  repetition nor the 50-move rule (only negamax does, :433-437); promotions
  reset the halfmove clock like captures, so Weiss 8b8f9516's rationale
  carries over unchanged.

### 3. Implementation sketch

One commit, tests first, using the tests/test_search.cpp:917-931 `quiesce()`
helper and `state.explored_nodes` (incremented at search.cpp:302).

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
3. **Capturing promotions unregressed**: they pass :310 today via
   MOVE_CAPTURE; one assertion so the new condition cannot lose them.
4. **SEE-gate mechanics, labelled as such**: `4r3/P6k/8/8/8/8/8/K7 w - - 0 1`
   (the e8 rook defends a8): admitted by the filter, declined by the existing
   gate at SEE -100, explored_nodes == 1. This pins the gate's reach, not the
   position's verdict -- Stockfish plays a7a8q here too, and re-deciding the
   gate is S022's business, excluded from this step.
5. **The change**: extend :310; rewrite the :303-307 comment to describe what
   is searched now (the accepts names this). The in-check path is untouched,
   so the two fast-suite quiescence mate cases ("a side in check may not
   stand pat", tests/test_search.cpp:993-1027, and "mate is recognised at
   depth zero", tests/test_search.cpp:1067-1079) gate as-is.
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
- **Explosion is bounded by construction.** Each admitted move consumes a
  pawn irreversibly (at most pawns-on-the-seventh per node) and the qply cap
  at :287 (MAX_QSEARCH_DEPTH = 19, src/search_params.hpp:106) holds regardless.
  The published explosion warning attaches to checks (talkchess t=60962); no
  engine record of a promotion-specific depth guard was found.
- **Accepts wording, verified consistent.** Capturing underpromotions pass
  :310 today via MOVE_CAPTURE and stay in; "underpromotions stay out" governs
  the non-capture arm only. The node-count sweep the accepts allows is Weiss
  #779's exact question, measured 0.02 +/- 1.50 there -- queen-only is the
  right default and the sweep decides on nodes alone.
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
- **S022 (after)**: the delta/futility decision must price promotion gain
  (CPW Delta Pruning's +775-class allowance, read at S112), and is where the
  :322 gate's treatment of promotions is re-decided if ever.
- **S015/S091**: see_ge already prices promotions (section 2); S091 extends
  SEE pruning to the main search, where quiet promotions are LMR-exempt but
  not SEE-gated -- that step's business, not this one's.
- **S130 (before)**: the TT-substituted stand-pat is orthogonal; a stored
  quiet-promotion best_move reads back through :611-612 unchanged.
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
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22promotions%22+%22qsearch%22
  -- SF 785b7080, 5c75c1c, d58e836, 252844e, aed542d messages.
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+%22promotions%22+in:title
  -- Lynx #302/#519/#2427 promotion-ordering records (0.4 +/- 7.7 class).
- https://talkchess.com/viewtopic.php?t=60962 -- hgm on captures-first and
  the checks explosion; no measurements.
- https://www.dogeystamp.com/chess5/ -- captures-only qsearch devlog; no
  promotion number.
- https://github.com/AlexanderBrevig/ChessML/blob/main/docs/quiescence-search.md
  -- tutorial doc; promotions "always" beside captures; its ~150-250 figure
  is for qsearch as a whole.
- https://int0x80.ca/posts/chess-engines/5-qsearch -- unreachable (HTTP 403);
  a "398.84 +/- 89.36" figure surfaced in search snippets could not be traced
  to any readable source and is not used.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+%22noisy%22+%22promotion%22
  -- zero results; Ethereal's classification is untraced in commit prose.
