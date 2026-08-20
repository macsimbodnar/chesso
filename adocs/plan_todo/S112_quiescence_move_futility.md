id:         S112
goal:       quiescence skips a capture whose best case cannot reach alpha, per move, before the exchange evaluation is consulted
accepts:    an SPRT verdict, recorded whatever it is; the prune path raises best_value to the futility value rather than dropping it, because quiescence is fail-soft and skipping that assignment returns bounds that are too low; no futility while in check, at a promotion, or on a move that gives check; the margin is a constant in src/search_params.hpp with a range; the mate-in-quiescence case in the fast suite still passes
touches:    src/search.cpp quiescence, src/search_params.hpp
excludes:   delta pruning, which is S022 and is re-decided after this
decisions:  DEC-019
closes:
blocks:
paused_by:
done:

## Why this comes before S022 and re-opens S015

Two of this project's three "published figure measured zero" cases live in
quiescence, and the published record now explains both.

**S015, SEE pruning in quiescence, measured 0.** The reported figures for SEE
pruning assume per-move futility is present; the two prune overlapping sets and
whichever arrives second measures the remainder. Chesso has the second and not
the first. So S015's zero is consistent with the literature rather than a
contradiction of it, and this step is the one that makes it re-measurable --
`specs.md` already parks that re-run inside S022.

**S022, delta pruning.** One engine *gained* by **deleting** delta pruning once
per-move futility existed. So S022 is re-targeted by this step from "add delta
pruning" to "decide between the two, by measurement", and deleting is a valid
outcome to record.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

One idea, two published generations, blurred names:

- **Delta pruning (CPW's core rule)**: before making a capture, "test whether
  the captured piece value plus some safety margin (typically around 200
  centipawns) are enough to raise alpha". Promotions bump the allowance (the
  page's sample adds 775 on top of a 975 big delta); a whole-node "big delta"
  variant skips move generation when even a queen cannot reach alpha; the one
  stated exemption is "switched off in the late endgame" (insufficient-material
  blindness). No check exemption stated.
- **Per-move quiescence futility (this step)**: the refined form.
  `futility_base = stand_pat + margin`, once per node; per capture
  `futility_value = futility_base + value(captured)`; `futility_value <= alpha`
  skips the capture **and raises best_value to futility_value** — the
  fail-soft repair the accepts require, traced in Stockfish commit prose:
  5af8179 (2012-11-26) "Update bestValue when futility pruning" in qsearch,
  0b4ea54 (2013-03-22) the same in search(), +4.48 +/- 3.4. The published rule
  has a second arm — `futility_base <= alpha` and the exchange not positive
  skips on `futility_base` alone — but that arm *consults SEE*, so under this
  step's goal it belongs to the S015-remeasure/S022 complex, not here.
- Exemptions as published: CPW's Futility Pruning page (main-search context)
  keeps captures able to clear alpha + margin and "moves that give check", and
  is "not used when the side to move is in check, or when either alpha or beta
  are close to the mate value". A qsearch-specific gives-check exemption is
  untraced in public prose; the accepts adopt it conservatively. In check,
  quiescence searches all evasions and may not stand pat (CPW Quiescence
  Search), so futility is off there by construction.

Traced records (URLs in section 8):

- **Weiss #188, 2020-02-26, "Per move futility pruning in quiescence search"**:
  +33.79 +/- 12.56 at 10+0.1, +6.10 +/- 4.43 at 60+0.6. The PR prose states
  the taxonomy: per-move futility approximates "the improvement of each move
  to try to prune them individually", against delta pruning's one
  "approximated ceiling" for the node. **Weiss had no SEE gate in quiescence
  yet** — that arrived seven months later.
- **Weiss #355, 2020-09-27, "Quiescence SEE pruning"**: +35.73 +/- 12.73 and
  +24.89 +/- 9.66 — SEE added *second*, on top of futility, still large.
- **Weiss #455, 2021-06-25, "Remove delta pruning"**: +1.87 +/- 3.51 at 8+0.08,
  +6.74 +/- 5.26 at 40+0.4; rationale verbatim: "Delta pruning is no longer
  useful, likely its intended effect is better achieved by the per-move
  futility prunings." The deletion gainer S022 cites, now pinned.
- **Lynx, three attempts, all rejected ~0 or negative**, in an engine that
  already had SEE-gated quiescence and an NNUE eval: #731 delta (margins
  150/200: -5.2, -5.8); #752 futility (margin 150: -3.6 extended); #1142
  futility II (2024-11: +0.1; a variant without SEE: -384; sweeps -6.4, -1.4).
- **Stockfish prose, shape only**: 725c504 (2008) "take in account enpassant
  in futility formula" — the EP edge was a shipped bug there; 5f3c660 (2010)
  prunes the remaining captures after the first pruned one via MVV order;
  ab27635 (2015) extends it to PV nodes, passed; ef228296 (2023) a third,
  SEE-based arm. Margins are not in the messages and unusable anyway (DEC-084).
- **Ethereal**: added/removed/re-added delta through 2016; b7f142a (2017-07)
  "fixed promotion bug in delta pruning" — promotions are the recorded bug
  site; 58ca478 (2017-12) removed a delta condition bundled with an LMR fix,
  [0, 5] passes at both controls (two changes, weak attribution).

### Scope concern

The step's rationale says of the overlapping pair "whichever arrives second
measures the remainder". Weiss contradicts that half: both arrived in sequence
and both measured +25 to +36. Lynx supports it: futility second, over SEE,
measured ~0 three times. The overlap is engine-dependent, and chesso's
configuration — SEE gate since S015, futility absent — matches Lynx's, not
Weiss's. Goal and accepts stand (verdict recorded whatever it is, DEC-019);
the honest expectation band is 0 to +30 with both endpoints published, and a
zero still buys S022 its baseline.

### 2. Shape for chesso

Line numbers at `cf89e22`; re-locate by symbol if drifted.

- Quiescence's capture handling is two loops: the **filter loop** at
  src/search.cpp:308-330 compacts survivors (non-captures dropped at :310,
  S015's SEE gate at :322-325: `!in_check && !capture_cannot_lose &&
  !see_ge(move, 0)` skips), then the **search loop** at :339-366.
  `best_value` is initialised at :332 (`in_check ? MIN : stand_pat`,
  fail-soft) — after the filter loop, which the fail-soft raise must respect.
- **The futility test sits in the filter loop, above :322.** The ordering
  benefit is one-sided: the test is three adds and a compare, `see_ge`
  rebuilds `attackers_to_square` per exchange round (src/bitboard.cpp:
  1103-1130), and `capture_cannot_lose` (:1160-1174, two lookups) sits
  between. Futility first, cannot-lose second, `see_ge` last.
- **Victim values: none of the three existing tables; a fourth, owned by the
  search.** `piece_values_abs` (src/evaluation.cpp:41-43) is the MVV ordering
  band table — king at 100000, bands clearing by exactly 100, excluded from
  tuning for that reason (src/search_params.hpp:29-32) — wiring pruning to it
  couples a margin to the CLAUDE.md band hazard. `see_value`
  (src/bitboard.cpp:1096) is deliberately file-local; exporting it couples
  futility to every future SEE retune. `piece_value` (src/eval_tables.hpp:24,
  {94, 327, 308, 487, 716}) is PSQT-degenerate by its own comment (:44-49) —
  only the sum is fitted, so it understates victims and every refit (S126)
  would silently move the prune. A dedicated `qs_futility_value[]` decouples
  all three; seed in section 4.
- **Victim lookup mirrors capture_score's EP branch**
  (src/evaluation.cpp:1064-1073): `board->squares[MOVE_TO(move)]` is EMPTY for
  en passant; the victim is a pawn. SF 725c504 shipped that wrong first.
- **Promotions: exempt on the `MOVE_PROMOTED(move)` bit, before the victim
  arithmetic.** Capturing promotions are in the loop *today* (:310 keeps
  captures); victim-only arithmetic underprices them by queen-minus-pawn
  (CPW's alternative is a +775-class allowance; the accepts choose exemption).
  Ethereal b7f142a is the recorded bug. The bit test also pre-answers S131
  (section 7).
- **Gives-check exemption: no pre-make predicate exists.** The main search
  learns it by `is_check(game)` *after* make_move (src/search.cpp:661-662),
  deliberately not on captures. Two routes: (a) for a capture futility wants
  to skip, pay make/is_check/unmake and keep it if it checks — exact, no new
  machinery, cost only on would-be-skipped moves, still saves the child call
  and `see_ge`; (b) a `gives_check(board, move)` predicate — new machinery
  with a discovered-check bug surface, its own step if (a)'s cost shows.
  Start with (a).
- **Stand-pat post-S130**: the substitution lands between :253 and :268, so a
  `futility_base` computed after it consumes the TT-tightened stand-pat. That
  is published practice — Weiss cca90ea7's "use that instead for pruning
  heuristics" is exactly this consumer. Watch: an UPPER-bound cap *lowers*
  futility_base and prunes more (honest — the entry certifies value <= s),
  and it is the first bisect lever if the SPRT fails: the decoupled fallback
  is futility_base from the pre-substitution static. When the lazy shortcut
  fired instead, the alpha-side bound is `cheap + LAZY_EVAL_MARGIN >= truth`
  (src/evaluation.hpp:24-27): futility_base overstated, the test fires less —
  conservative, sound.
- **Fail-soft arithmetic, and why S130's watched exact-store corner stays
  unreachable.** A skip needs `stand_pat + margin + victim <= alpha` with
  margin and victim non-negative, so it fires only when `stand_pat <= alpha`,
  i.e. :279 did not raise alpha and `alpha == alpha0`. Every skipped
  futility_value is <= alpha0, the raised best_value stays <= alpha0, and the
  :387-390 store keeps TT_ALPHA_NODE. Keep the margin's range floor at 0 or
  this argument dies.

### 3. Implementation sketch

One commit, one SPRT; tests first within it.

1. **Red-first unit tests**, inlining the tests/test_search.cpp:554-568
   `quiesce()` helper where node counts are needed (`state.explored_nodes`
   counts entries, src/search.cpp:200). Windows are built from the engine's
   own numbers — `evaluate()` of the FEN, the margin, the table's victim
   value — never from a judged score (DEC-023):
   - *Skipped when it cannot reach alpha*: one-capture FEN, alpha =
     stand_pat + margin + victim + 1. Expect explored_nodes == 1 and return ==
     stand_pat + margin + victim — the fail-soft raise, red against an
     implementation that drops the assignment and returns stand_pat.
   - *Searched when it can*: same FEN, alpha one below that threshold; expect
     explored_nodes >= 2.
   - *Promotion exemption*: a capturing promotion below the threshold is
     still searched.
   - *In-check exemption*: in-check FEN, alpha huge — every evasion searched,
     and no mate score while a legal evasion exists (the :370
     `legal_moves == 0` guard is what futility-in-check would corrupt).
   - *Gives-check exemption*: a checking capture below the threshold is still
     searched (assert check via the :661 post-make pattern).
   - *En passant*: an EP capture with alpha between futility_base and
     futility_base + pawn value — the squares[to]==EMPTY bug prunes it, the
     correct victim searches it.
2. **The change**: `futility_base` once, under `!in_check`, after :287; in
   the filter loop above :322: not promoted, not gives-check, victim from the
   dedicated table, skip and fold futility_value into a running maximum that
   :332 takes into best_value's initialisation.
3. Fast suite green — the mate-in-quiescence case explicitly
   (tests/test_search.cpp:630-664) — then the SPRT.

### 4. Constants and seeds

- **Margin**: one new X-macro entry in src/search_params.hpp (the accepts name
  the file), e.g. `QS_FUTILITY_MARGIN`, range floor 0 (section 2 needs it).
  Seed **200 cp** from CPW's Delta Pruning page ("typically around 200") —
  open literature, valid under DEC-084, and **seed — must be fitted/SPSA'd
  here** (S085/S127). Lynx's prose records sweeps at 75/150/200 reading
  0-to-negative *there*; an NNUE eval's scale is not this one's, so those are
  trial shapes, not values. Weiss's margin: **no publishable seed** (prose
  silent; source off limits).
- **Victim values**: seed from this repo's own exchange scale — `see_value`'s
  {100, 300, 300, 500, 900} (src/bitboard.cpp:1096) copied into the new
  array, not shared with it. Internal reuse, no provenance question; the
  margin and this scale share one job (absorbing the PSQT part of a victim's
  worth), so sweep them together at S127.

### 5. Pitfalls

- **Futility while in check corrupts mate detection**: :370 declares mate on
  `legal_moves == 0`; a pruned evasion fakes it. The `!in_check` guard is
  load-bearing exactly as it is for S015's gate.
- **The band hazard**: `piece_values_abs` is ordering, not price, and
  search_params.hpp:29-32 already refuses to tune it. Futility must not
  become the back door that couples pruning to it — section 2 names the table
  to use instead.
- **Stacking with S015's gate**: futility runs first and is cheaper; both
  skip only what cannot beat bounds the node already holds (stand-pat floors
  the return either way). Asymmetry to know: the futility skip raises
  best_value, S015's skip today does not — changing S015's skip is S022's
  business, not this commit's.
- **En passant**: victim is not on MOVE_TO; evaluation.cpp:1066-1069 is the
  in-repo pattern, SF 725c504 the published bug.
- **Promotions and S131**: the MOVE_PROMOTED exemption must precede the
  victim arithmetic. When S131 admits *non-capture* promotions they arrive
  with victim 0 and futility_value == futility_base — an unexempted test
  auto-prunes exactly the moves S131 exists to search.
- **Mate-band arithmetic is bounded by construction**: stand_pat is a static
  or S130-vetted non-mate score, so futility_value tops out near
  |eval| + margin + 900, far under MATE_MIN 48000; alpha inside the mate band
  prunes every capture into an honest fail-low. CPW's near-mate guard is for
  main-search margins — note it in a comment, do not add a dead guard.
- **MaxQsearchDepth**: :287 returns before the loop, so the two never
  compose; S085's SPSA may raise the 8, which makes this prune's savings
  bigger, not smaller — S127's re-sweep covers it.

### 6. Measurement

Not behaviour-neutral, no identity claim: one SPRT at the S105 regime —
8+0.08, Hash=16, UHO book, `elo0=0 elo1=5` — fast suite and mate cases green
first, verdict recorded whatever it is (DEC-019; S005/S006/S015 are the zero
precedent). Record the node reduction alongside: `tools/search_bench.py`
depth-12 counts before/after (they will differ; the delta is the datum —
neither Weiss nor Lynx published one to compare). Cheap pre-check before
booking the match: count filter-loop skips over one depth-12 kiwipete search;
a near-zero fire rate predicts the zero before it is bought (S094's 0.79 %
probe precedent, DEC-079).

### 7. Interactions

- **S130 (before)**: supplies the tightened stand-pat futility_base consumes;
  the pre-substitution base is the bisect lever if the SPRT fails.
- **S022 (after, the consumer)**: S112's verdict is its baseline. S022 then
  compares, one SPRT each: the S015 SEE gate re-measured against
  futility-present — Weiss #355 (+35.7 added second) and Lynx #1142's no-SEE
  variant (-384) bound the plausible outcomes — and delta pruning on top,
  where Weiss #455 (+1.9/+6.7 for *deleting*) and Lynx #731 (negative) say
  deleting-or-never-adding is the likely verdict.
- **S015/S091**: S015's gate is untouched here (one change at a time); S091
  is the main-search SEE sibling, sharing only the see_ge primitive.
- **S131 (after)**: the MOVE_PROMOTED exemption is written now so S131's
  non-capture promotions land unpruned by construction.
- **S106 (before)**: its bound-sign red tests cover the TT paths the raised
  best_value is stored through.
- **S085/S127**: MaxQsearchDepth and the new margin belong to the sweeps; the
  margin ships seeded-then-fitted, never seeded-only (DEC-084).

### 8. References

Every URL read for this section; GitHub API URLs return commit messages only,
no diffs (DEC-016 observed).

- https://www.chessprogramming.org/Delta_Pruning — the per-capture rule,
  ~200 cp margin, big-delta variant, promotion allowance, late-endgame
  switch-off.
- https://www.chessprogramming.org/Quiescence_Search — stand pat, in-check
  evasions, "delta pruning checks" for generated checking moves.
- https://www.chessprogramming.org/Futility_Pruning — main-search futility;
  exemptions for checks / in-check / near-mate; qsearch not covered there.
- https://github.com/TerjeKir/weiss/pull/188 — per-move futility, +33.79 STC
  / +6.10 LTC, taxonomy prose.
- https://github.com/TerjeKir/weiss/pull/355 — SEE pruning added second,
  +35.73 / +24.89.
- https://github.com/TerjeKir/weiss/pull/455 — delta pruning removed, +1.87 /
  +6.74, rationale quoted above.
- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22SEE%22 —
  Weiss SEE timeline (ordering 2020-09-17, QS pruning 2020-09-27).
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+%22delta+pruning%22+OR+%22qsearch+futility%22
  — Lynx #731, #752, #1142 SPRT prose.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22futility%22+%22qsearch%22
  — SF 725c504, 5f3c660, 5af8179, 0b4ea54, ab27635, ef228296 messages.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+%22delta+pruning%22
  — Ethereal 2016 add/remove cycle, b7f142a promotion bug.
- https://api.github.com/repos/AndyGrant/Ethereal/commits/58ca478 — full
  message: LMR fix + delta condition removal, [0, 5] passes, two changes.
- https://talkchess.com/viewtopic.php?t=77451 — futility vs quiescence
  taxonomy discussion; no measurements.
- https://open-chess.org/viewtopic.php?t=2988 — checked for a rumoured
  CeeChess "+80 STC" delta figure; contains none. That figure is untraced
  publicly.
- https://api.github.com/search/commits?q=%22delta+pruning%22 — the wider
  commit sweep that surfaced only bundled or unmeasured adds elsewhere.
