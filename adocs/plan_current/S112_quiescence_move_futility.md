id:         S112
goal:       quiescence skips a capture whose best case cannot reach alpha, per move, before the exchange evaluation is consulted
accepts:    an SPRT verdict, recorded whatever it is; the prune path raises best_value to the futility value rather than dropping it, because quiescence is fail-soft and skipping that assignment returns bounds that are too low; no futility while in check, at a promotion, or on a move that gives check; the margin is a constant in src/search_params.hpp with a range; the two fast-suite quiescence mate cases still pass -- "a side in check may not stand pat" (`tests/test_search.cpp` "a side in check may not stand pat") and "mate is recognised at depth zero" (`tests/test_search.cpp` "mate is recognised at depth zero")
touches:    src/search.cpp quiescence, src/search_params.hpp
excludes:   delta pruning, which is S022 and is re-decided after this
decisions:  DEC-019
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-26 14:55 CEST
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

- - Quiescence's capture handling is two loops, both in `src/search.cpp`
  `quiescence`: the **filter loop** compacts survivors, dropping non-captures
  and then applying S015's SEE gate (`!in_check && !capture_cannot_lose &&
  !see_ge(move, 0)` skips), and then the **search loop** runs. `best_value` is
  initialised between them (`in_check ? MIN : stand_pat`, fail-soft) — after
  the filter loop, which the fail-soft raise must respect.
- - **The futility test sits in the filter loop, above the SEE gate.** The
  ordering benefit is one-sided: the test is three adds and a compare, `see_ge`
  rebuilds `attackers_to_square` per exchange round (src/bitboard.cpp:
  1103-1130), and `capture_cannot_lose` (`src/bitboard.cpp`
  `capture_cannot_lose`, two lookups) sits between. Futility first, cannot-lose
  second, `see_ge` last.
- - **Victim values: none of the three existing tables; a fourth, owned by the
  search.** `piece_values_abs` (`src/evaluation.cpp` `piece_values_abs`) is the
  MVV ordering band table — king at 100000, bands clearing by exactly 100,
  excluded from tuning for that reason (`src/search_params.hpp` "Not in the
  set, on purpose") — wiring pruning to it couples a margin to the CLAUDE.md
  band hazard. `see_value` (`src/bitboard.cpp` `see_value`) is deliberately
  file-local; exporting it couples futility to every future SEE retune.
  `piece_value` (`src/eval_tables.hpp` `piece_value`, {94, 327, 308, 487, 716})
  is PSQT-degenerate by its own comment (`src/eval_tables.hpp` "degenerate by
  five dimensions -- adding c to every") — only the sum is fitted, so it
  understates victims and every refit (S126) would silently move the prune. A
  dedicated `qs_futility_value[]` decouples all three; seed in section 4.
- **Victim lookup mirrors capture_score's EP branch** (`src/evaluation.cpp`
  `capture_score`): `board->squares[MOVE_TO(move)]` is EMPTY for en passant;
  the victim is a pawn. SF 725c504 shipped that wrong first.
- - **Promotions: exempt on the `MOVE_PROMOTED(move)` bit, before the victim
  arithmetic.** Capturing promotions are in the loop *today* (`src/search.cpp`
  `quiescence` keeps captures); victim-only arithmetic underprices them by
  queen-minus-pawn (CPW's alternative is a +775-class allowance; the accepts
  choose exemption). Ethereal b7f142a is the recorded bug. The bit test also
  pre-answers S131 (section 7).
- **Gives-check exemption: no pre-make predicate exists.** The main search
  learns it by `is_check(game)` *after* make_move (`src/search.cpp` `negamax`),
  deliberately not on captures. Two routes: (a) for a capture futility wants
  to skip, pay make/is_check/unmake and keep it if it checks — exact, no new
  machinery, cost only on would-be-skipped moves, still saves the child call
  and `see_ge`; (b) a `gives_check(board, move)` predicate — new machinery
  with a discovered-check bug surface, its own step if (a)'s cost shows.
  Start with (a).
- - **Stand-pat post-S130**: the substitution lands between the stand-pat
  computation and the filter loop in `src/search.cpp` `quiescence`, so a
  `futility_base` computed after it consumes the TT-tightened stand-pat. That
  is published practice — Weiss cca90ea7's "use that instead for pruning
  heuristics" is exactly this consumer. Watch: an UPPER-bound cap *lowers*
  futility_base and prunes more (honest — the entry certifies value <= s), and
  it is the first bisect lever if the SPRT fails: the decoupled fallback is
  futility_base from the pre-substitution static. When the lazy shortcut fired
  instead, the alpha-side bound is `cheap + LAZY_EVAL_MARGIN >= truth`
  (`src/evaluation.hpp` "one it would have taken, and both are true
  statements"): futility_base overstated, the test fires less — conservative,
  sound.
- - **Fail-soft arithmetic, and why S130's watched exact-store corner stays
  unreachable.** A skip needs `stand_pat + margin + victim <= alpha` with
  margin and victim non-negative, so it fires only when `stand_pat <= alpha`,
  i.e. `src/search.cpp` `quiescence` did not raise alpha and `alpha == alpha0`.
  Every skipped futility_value is <= alpha0, the raised best_value stays <=
  alpha0, and the `src/search.cpp` `quiescence` store keeps TT_ALPHA_NODE. Keep
  the margin's range floor at 0 or this argument dies.

### 3. Implementation sketch

One commit, one SPRT; tests first within it.

1. **Red-first unit tests**, inlining the `tests/test_search.cpp` "search:
   quiescence" `quiesce()` helper where node counts are needed
   (`state.explored_nodes` counts entries, `src/search.cpp` `quiescence`).
   Windows are built from the engine's own numbers — `evaluate()` of the FEN,
   the margin, the table's victim value — never from a judged score (DEC-023):
   - *Skipped when it cannot reach alpha*: one-capture FEN, alpha =
     stand_pat + margin + victim + 1. Expect explored_nodes == 1 and return ==
     stand_pat + margin + victim — the fail-soft raise, red against an
     implementation that drops the assignment and returns stand_pat.
   - *Searched when it can*: same FEN, alpha one below that threshold; expect
     explored_nodes >= 2.
   - *Promotion exemption*: a capturing promotion below the threshold is
     still searched.
   - *In-check exemption*: in-check FEN, alpha huge — every evasion searched,
     and no mate score while a legal evasion exists (the `src/search.cpp`
     `quiescence` `legal_moves == 0` guard is what futility-in-check would
     corrupt).
   - *Gives-check exemption*: a checking capture below the threshold is still
     searched (assert check via the `src/search.cpp` `negamax` post-make
     pattern).
   - *En passant*: an EP capture with alpha between futility_base and
     futility_base + pawn value — the squares[to]==EMPTY bug prunes it, the
     correct victim searches it.
2. 2. **The change**: `futility_base` once, under `!in_check`, after
   `src/search.cpp` `quiescence`; in the filter loop above `src/search.cpp`
   `quiescence`: not promoted, not gives-check, victim from the dedicated
   table, skip and fold futility_value into a running maximum that
   `src/search.cpp` `quiescence` takes into best_value's initialisation.
3. Fast suite green -- the two quiescence mate cases explicitly ("a side in
   check may not stand pat", `tests/test_search.cpp` "a side in check may not
   stand pat", which is the case where quiescence has to search evasions to
   reach a mate, and "mate is recognised at depth zero",
   `tests/test_search.cpp` "mate is recognised at depth zero", where the mate
   is already on the board) -- then the SPRT.

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
  {100, 300, 300, 500, 900} (`src/bitboard.cpp` `see_value`) copied into the
  new array, not shared with it. Internal reuse, no provenance question; the
  margin and this scale share one job (absorbing the PSQT part of a victim's
  worth), so sweep them together at S127.

### 5. Pitfalls

- - **Futility while in check corrupts mate detection**: `src/search.cpp`
  `quiescence` declares mate on `legal_moves == 0`; a pruned evasion fakes it.
  The `!in_check` guard is load-bearing exactly as it is for S015's gate.
- **The band hazard**: `piece_values_abs` is ordering, not price, and
  `src/search_params.hpp` "Not in the set, on purpose" already refuses to tune
  it. Futility must not become the back door that couples pruning to it —
  section 2 names the table to use instead.
- **Stacking with S015's gate**: futility runs first and is cheaper; both
  skip only what cannot beat bounds the node already holds (stand-pat floors
  the return either way). Asymmetry to know: the futility skip raises
  best_value, S015's skip today does not — changing S015's skip is S022's
  business, not this commit's.
- **En passant**: victim is not on MOVE_TO; `evaluation.cpp` `capture_score` is
  the in-repo pattern, SF 725c504 the published bug.
- **Promotions and S131**: the MOVE_PROMOTED exemption must precede the
  victim arithmetic. When S131 admits *non-capture* promotions they arrive
  with victim 0 and futility_value == futility_base — an unexempted test
  auto-prunes exactly the moves S131 exists to search.
- **Mate-band arithmetic is bounded by construction**: stand_pat is a static
  or S130-vetted non-mate score, so futility_value tops out near
  |eval| + margin + 900, far under MATE_MIN 48000; alpha inside the mate band
  prunes every capture into an honest fail-low. CPW's near-mate guard is for
  main-search margins — note it in a comment, do not add a dead guard.
- - **MaxQsearchDepth**: `src/search.cpp` `quiescence` returns before the loop,
  so the two never compose; S085's SPSA may raise the 8, which makes this
  prune's savings bigger, not smaller — S127's re-sweep covers it.

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
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+%22delta+pruning%22+OR+%22qsearch+futility%22
  — Lynx #731, #752, #1142 SPRT prose.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22futility%22+%22qsearch%22
  — SF 725c504, 5f3c660, 5af8179, 0b4ea54, ab27635, ef228296 messages.
- -
  https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+%22delta+pruning%22
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

## What the tree already did, 2026-09-26 (checked at `1680439`)

The step file was written on 2026-08-19; the implementing agent checked each
assumption on the tree before writing code, by symbol.

- **`quiescence`'s shape** (`src/search.cpp` `quiescence`) is as section 2
  describes: TT probe, lazy static score (or the entry's stored evaluation,
  S094), S130's stand-pat substitution by bound type (never a mate score),
  the in-check test, the stand-pat cutoff and alpha raise, the
  `MAX_QSEARCH_DEPTH` return -- **19 now, not the 8 section 5 names** (S085's
  SPSA moved it) -- then the **filter loop** (non-captures dropped out of
  check; S015's gate `!in_check && !capture_cannot_lose && !see_ge(0)`), then
  `best_value = in_check ? MIN : stand_pat` with `floor_type` / `value_type`
  (DEC-102), then the search loop with S210's dead-position test.
- **No delta pruning exists** anywhere in the tree, before or after this step.
- **En passant** is `MOVE_EN_PASSANT(move)` with `squares[MOVE_TO]` EMPTY;
  `capture_score`'s branch is the in-repo pattern and was mirrored.
  **Promotions** carry `MOVE_PROMOTED(move)` != `TO_NONE`; capturing ones pass
  the filter today, non-capturing ones are dropped out of check.
- **The fail-soft argument of section 2 holds for alpha and not for the
  store.** A skip needs `stand_pat + margin + victim <= alpha` with margin and
  victim non-negative, so it fires only when the stand pat did not raise
  alpha, and every folded value is at or below the entry alpha -- that half
  holds. But "the store stays an alpha node" does **not** follow on its own
  since DEC-102: when S130 raised the stand pat with a lower bound,
  `floor_type` is `TT_BETA_NODE`, and a fold that raises `best_value` above
  that floor would have been stored as a lower bound on a best case nobody
  searched. The fold therefore sets `value_type = TT_ALPHA_NODE` when it
  raises the value, and "a folded futility value is stored as an upper bound"
  plus mutant F06 hold it. Section 2's sentence is kept above as written, with
  this correction beside it (DEC-215 clause 3's form).
- **No off value exists in the margin**, so the rule got a switch,
  `QsFutility`, in `SeExtend`'s and `RfpTtEstimate`'s shape (DEC-215 clause
  2). The accepts' "margin is a constant in src/search_params.hpp with a
  range" holds: `QsFutilityMargin`, 0 to 2000.

## What landed

- `src/search_params.hpp`: `QS_FUTILITY` ("QsFutility", 1, 0..1) and
  `QS_FUTILITY_MARGIN` ("QsFutilityMargin", 188, 0..2000).
- `src/search.cpp`: `qs_futility_value[EMPTY + 1]` (by piece_t, EMPTY and the
  kings 0) and `search_qs_futility_value_probe`; in `quiescence`,
  `futility_base` once under `!in_check`, the test in the filter loop above
  S015's gate -- not a promotion, victim from the table (en passant a pawn),
  `<= alpha`, then route (a): make / `is_check` / unmake only for a capture
  the test would skip, an illegal one dropped as the search loop would -- and
  the fold into `best_value` after `value_type` is initialised, stored as an
  upper bound.
- Nothing else in `src/`. S015's gate untouched.

## Seeds (DEC-134)

- `QsFutilityMargin` 188: **(a) literature** -- the wiki's Delta Pruning page,
  "some safety margin (typically around 200 centipawns)", read as two pawns in
  chesso's material scale (`src/eval_tables.hpp` PAWN 94), because the margin
  is compared against `evaluate()`. **Section 4's "200 cp" is not used as
  written**: DEC-134 says units are not exempt by name, and FutBase's own
  comment records the mistake of reading a margin in `see_value`'s units.
  Whether the wiki's "typically" is a publication's own number rather than an
  aggregate of engines' constants is the reviewer's call; it names no engine.
- `qs_futility_value` {100, 300, 300, 500, 900}: **(b)** chesso's own exchange
  scale, `see_value`, copied, not shared -- section 4's own choice.

## Fire rate, before any game (section 6)

An instrumented copy of the candidate (`.ref-builds/fire`, write-only counters;
its `bench 12` is the candidate's own 1981759) over the eight bench positions
at depth 12: 992064 captures reach the filter loop out of check; 37724 are
promotions and exempt; 954340 are tested; 116176 fall at or below alpha;
12319 of those give check and are kept; 0 are illegal; **103857 are skipped,
10.47 % of filter-loop captures.** Not near zero: the zero is not predicted.
`.tuning/coord/S112_fire_d12.txt`.

## Measurements (2026-09-26, niced beside S238's SPRT; counts only)

- Off value: tune build at `QsFutility` 0 -> `bench` **4803214**, all eight
  replies identical to a parent built from `1680439`; `search_bench` identical
  at 9 (48522 / 85714 / 28080) and 12 (129499 / 411457 / 172984), best moves
  c3d5 e2a6 d7c8q. `.tuning/coord/S112_identity.log`.
- `bench` 4803214 -> **4649650** (-3.20 %), eight replies unchanged.
- `search_bench` 9: 48522 -> 53598, 85714 -> 80389, 28080 -> 25691; 12:
  129499 -> 154098, 411457 -> 472358, 172984 -> 107876; best moves unchanged.
- "pruning does not hide a forced mate" went red at capture row 3 (depth 10:
  the candidate reports that mate at 11 and 12 only). Handled under DEC-233:
  the table re-derived by `adocs/data/S230_mine_r01_row.py depths`, shipped
  plus the six S091 mutants (`.tuning/coord/S112_capmates/`), depths 7, 9, 11,
  9, every distance unchanged; S095's rows quoted in the GOLDEN block.
- Tests, red first and observed: with the two parameters, the table and its
  probe in place and no rule, the new suite "search: quiescence futility" was
  2 of 7 red -- the skip case (`nodes` 2 against 1, `score` 364 against 478)
  and the fold-type case -- and the five exemption and reach cases green, as
  they must be on a tree that prunes nothing; each of those is red under its
  own mutant below. `.tuning/coord/S112_red_stageA.log`. "a side in check may
  not stand pat" and "mate is recognised at depth zero" green throughout.
- Mutation (DEC-141 clause 2), `tools/mutants/S112_qs_futility.py` on a clean
  detached fixture `a4f85bb` in `.ref-builds/mut`: header `baseline green, 40
  tests, bench 4649650 nodes via engine`; **mutation score 6 of 6 (100 %)**.
  F01 (no raise) by the skip case and the fold-type case; F02 (in check) by
  "futility does not skip an evasion while in check", with test_engine and
  test_mate_pv; F03 by the promotion case; F04 by the gives-check case, with
  the capture-mate table and test_mate_carry; F05 by the en passant case; F06
  by the fold-type case, with test_mate_breadth. F05 and F06 leave the bench
  signature unmoved -- only their cases see them.
  `.tuning/coord/S112_mutation.log`.
- Both fast suites green: Release 40 of 40 as the mutation fixture's baseline
  (the working tree byte for byte), tune build 40 of 40 at `-j1`.
  `test_mate_carry` sits at its 120 s ctest ceiling under the running SPRT --
  117.13 s in the green tune run, and it timed out in two earlier `-j4` runs;
  run directly beside the SPRT the parent `1680439` took 114.66 s and the
  candidate 124.02 s, both passing. A load reading, not a claim about speed;
  re-run on an idle machine before landing.
  `./clang-format.sh --check` green. `tools/plan_prose_check.py --citations`,
  `--touches`, `--params`: 0 flagged.
- Not run here, by the brief: the Debug self-play and `tools/gate_extra.sh`
  of DEC-141's second tier (a match holds the machine), and the SPRT.

## Proposed `specs.md` sentence (the coordinator edits specs.md)

Quiescence skips, out of check, a capture that is neither a promotion nor a
check whose best case -- stand pat plus `QsFutilityMargin` plus the victim's
price in the search's own `qs_futility_value` table, an en passant victim
being a pawn -- is at or below alpha, before S015's exchange gate, and folds
that best case into its fail-soft value as an upper bound (S112);
`QsFutility` 0 is the tree before it, node for node.

## The coordinator's landing notes (2026-09-27)

**Cold fast check (over the `1680439` tree)**: no defect. It confirmed the
fold's `TT_ALPHA_NODE` store is right and downgrades only a lower-bound floor
that would otherwise certify an unsearched value; the switch off is neutral;
the capture-mate rows equal what `S230_mine_r01_row.py` and DEC-209's rule give.
Noted, not changed: the margin is in evaluation units (pawn 94) while the
victim table is in SEE units (pawn 100) -- cosmetic, S127 fits both together;
a capture the SEE gate would have dropped silently is now folded when futility
skips it first, which loosens the returned upper bound by design; row 3's mate
is now found at depth 11, not 10 -- this rule hides that mate at depth 10, which
DEC-233 accepts and the SPRT's readers should know.

**The seed** `QsFutilityMargin` 188 is accepted by the coordinator as DEC-134
form (a): the wiki's own prose, naming no engine, expressed in chesso's units.

**The port.** S238 read H0 and its removal returned the engine to `1680439`'s,
the tree this step was built and proved on, so no rebase was needed: the patch
applied to `1e9827d` with only document conflicts, both sides kept. On the
idle machine: both fast suites 40 of 40 (`test_mate_carry` 60.73 s and
60.15 s, inside its ceiling), the format check clean, `bench` 4649650, and the
tune build at `QsFutility` 0 prints 4803214 with the whole `bench` stream's
node counts and best moves identical to `086320c`'s build.

**Second tier on the landing** (DEC-141): Debug self-play 8 games, 0
`Assertion`, 0 `disconnect` (`.tuning/coord/S112_selfplay/`);
`tools/gate_extra.sh` 5 stages green in 1121 s
(`.tuning/gate_extra_2026-09-27_S112.log`). Landed as `3d82344` on `1e9827d`,
`bench` 4803214 -> 4649650. SPRT pair pinned: `REF` `1e9827d` (the engine is
`1680439`'s, S238's verdict left it so), `CAND` `3d82344`; open findings
re-read at pinning, S238's items closed by its removal.
