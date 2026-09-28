id:         S113
goal:       a shallow verification search over good captures prunes a node whose score is already far above beta
accepts:    an SPRT verdict, recorded whatever it is; never at a PV node, never when beta is near mate, and skipped when the table already holds a sufficient-depth entry scoring below the ProbCut beta; the margin and the depth reduction are constants in src/search_params.hpp with ranges and are fitted, not taken (DEC-084); a mate inside the pruned depth is in the fast suite and observed red with the guard removed
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   multicut, which arrives with S097's singular search at no extra cost
decisions:  DEC-071, DEC-084, DEC-105, DEC-134
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-27 12:32 CEST
done:

## Expect little, and be ready to record it

Reported +6.36 / +6.65 on first introduction at about 2950. One engine
**removed** ProbCut at +1.14 / +4.87 and then reintroduced it at -0.15 / +4.34
-- i.e. its value at 3400 is inside the noise at short control and small at
long. It is on this plan because it is cheap once the machinery around it
exists, not because it is expected to be large. A zero here is an ordinary
outcome and gets recorded as one.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol if drifted. Every GitHub read
below was a PR body, commit message or release-note text, never a diff or
source file (DEC-016). Buro's papers are open literature about the technique
and were read in full: under DEC-105 a number from them is form (a) and may
start a fit, and section 4 carries its URL. A number an engine ships is form
(a) for nothing, wherever it is republished.

### 1. State of the art

**Origin, cited.** Buro, "ProbCut: An Effective Selective Extension of the
Alpha-Beta Algorithm", ICCA Journal 18(2):71-76, 1995 (1996 ICCA Journal
Award), from Logistello (Othello): a shallow search result v' predicts the
deep result v through a linear model `v = a*v' + b + e`, e normal with
deviation sigma, and the subtree is pruned when a null-window shallow search
clears `(Phi^-1(p)*sigma + beta - b)/a` -- a bar raised above beta by a
statistically fitted margin. Multi-ProbCut ("Experiments with Multi-ProbCut
and a New High-Quality Evaluation Function for Othello", NECI TR 96, 1997)
adds per-stage parameters, several depth pairs, and no ProbCut inside the
shallow searches, "to avoid the collapsing of search depth". Chess: Jiang and
Buro, "First Experimental Results of ProbCut Applied to Chess", ACG 10, 2003
-- MPC in Crafty 18.15, regression over ~2700 positions at depths 1..10, only
|v'| <= 300 kept, mate scores excluded because "a 4-ply search cannot find
the check-mate while a 8-ply search can" about once per 1000 positions -- the
mate hazard is in the origin paper. Depth pairs (2,6),(3,7),(4,8),(3,9),
(4,10); thresholds ~1.0-1.2 sigma; sigma 52-82 on a 100=pawn scale; 59.4 %
over 64 self-play games, Yace score 51 % -> 56 %, 0.5 plies deeper at fixed
time. And the paper's negative result: plain ProbCut *beside null move*
measured "no better nor worse" -- both prune the same fail-high population
(section 5 below).

**The modern chess form, step by step (all traced in prose).** At a non-PV,
non-root node, not in check, `depth >= threshold`, beta outside the mate
band: `probBeta = beta + margin`; skip everything when the table entry
already scores the node under probBeta (Weiss #312 "don't try if the TT info
indicates it will fail"; #567 "score must beat not just beta, but beta +
200"; SF 7690fac drops that skip's entry-depth qualifier, 2025). Then over
**good captures and promotions only** (Weiss #484 "only try good noisy
moves" +5.25/+3.28; #658 SEE-filtered against probCutBeta +5.37/+2.05): a
zero-window **preliminary qsearch at (probBeta-1, probBeta)**, and only if it
holds, a **shallow search at `depth - 4`** -- SF 71cc01c (2018): "a
preliminary shallow search to verify a probcut before doing the normal
'depth - 4 plies' search", passed [0,5] both controls, "Happy to see
something from Ethereal work for Stockfish". A shallow search at or above
probBeta prunes the node with that fail-soft score, and **the result is
stored in the table** (SF 6edc29d7, 2022: "always saves on probCut cutoff";
Ethereal ca739a0 "Save into the TT after Probcut sometimes" +0.47/+0.72).
Placement: **after null move, before the move loop** -- SF fca0a2dd8 (2011):
"we have moved probcut after null search". The cascade churned at Ethereal
-- preliminary search replaced by an SEE filter 2019 (df97626 +2.83/+0.09),
re-added 2020 (f4439e5 +2.61/+6.10), moved to qsearch 2020-12 (d01aa26
+2.72/+1.67) -- filter and preliminary are near-interchangeable and engines
carry either or both. SF 2025 allows depth 3 with the verification disabled
"when it would simply repeat the qsearch" (f00d91f).

**Traced records.**
- Weiss #283 (2020-05-21): **+6.36 +/-4.59** at 10+0.1, **+6.65 +/-4.63** at
  60+0.6 -- the header's introduction numbers, at about 2950.
- Stash #141 (2023-10-10): **+5.59 +/-3.95** STC, +2.39 +/-1.92 LTC (~3300
  band); #142 more aggressive +3.75; #147 disallow at root +1.95; #148 a
  node-accounting fix.
- Ethereal 744f902 (2017-12-26, the "9.74 ProbCut" ledger line): passed
  [0,5] STC+LTC; margin 80 -> 100 cp 2022 (a8c9baf +2.51/+1.44).
- Berserk #215 "Probcut margin 110" +2.77 (2021-07); **#503 removed it
  2023-09 "currently worthless"** (removal +1.14 +/-2.23 at 40+0.4,
  +4.87 +/-3.97 at 8+0.08); **#507 reintroduced it three days later**
  (-0.15 +/-4.40 STC unpassed, +4.34 +/-3.00 LTC) -- the header's second
  record. Berserk's 2023 CCRL band was not re-traced this pass (CCRL pages
  refused the S097 pass too); the header's "at 3400" reading stands as is.
- **Lynx carries no ProbCut at all**: zero matching PRs, commits or issues
  at a ~3300 engine -- the expect-little reading has an existence proof.
- SF pre-history: present by 2011-05 (3ef4fdea "we skip probcut on
  evasions"); the Onno-ported extended form +7 +/-4.2 (fca0a2dd8); renamed
  Multicut and back within a day, 2014 (33369631, 62c0dc5d: "we try just a
  handful of captures instead of all moves"); the original introduction
  commit is older than the message-search hits and stays untraced here.

### 2. Shape for chesso

- - **Ladder site**: RFP and then the null move, both in `src/search.cpp`
  `negamax`. ProbCut goes after the null-move block's close and before
  `node_type_t type = ...` in the same function -- the SF placement, and a
  null-pruned node then never pays for captures.
  S097 lands earlier in plan order and its verification block sits in the same
  region; ProbCut goes first, same never-pays argument.
- - **qsearch is callable mid-node**: declared `search.hpp` `quiescence`,
  already called by negamax at `src/search.cpp` `negamax`. The preliminary is
  `-quiescence(-probBeta, -probBeta + 1, ply + 1, 0, game, state)` after
  `make_move` (illegal moves drop out via its false return, `src/search.cpp`
  `negamax` pattern); the shallow search is `-negamax(-probBeta, -probBeta + 1,
  probDepth, ply + 1, game, state, moves[i], false)` -- prev_move flows as
  everywhere.
- - **The restricted move set exists**: generate its own capture list before
  the staged loop (the `src/search.cpp` `negamax` stage regenerates later -- an
  accepted double generation every surveyed movepicker also pays), ordered by
  `capture_score` (`evaluation.hpp` `capture_score`) through `pick_next_move`
  (`src/search.cpp` `pick_next_move`). The "good" filter is the predicate pair
  quiescence uses at `src/search.cpp` `quiescence`, `capture_cannot_lose(...)`
  then `see_ge(..., 0)` (`bitboard.cpp` `capture_cannot_lose`,
  `src/bitboard.cpp` `see_ge`) -- S015 machinery, nothing new.
  generate_captures also emits non-capture promotions (`src/bitboard.cpp`
  `generate_moves_body` comment); ProbCut keeps them -- the published set is
  "noisy" moves.
- - **TT probe already paid**: the entry is copied out at `src/search.cpp`
  `negamax`. The skip reads the score through `de_normalize_score(entry->score,
  ply)` (`src/search.cpp` `de_normalize_score`) -- a **new de-normalize site,
  S106's lesson** -- and skips when the entry's depth reaches probDepth and its
  score sits under probBeta with an upper-bound or exact type. That is the
  honest V1 form: an ALPHA entry under probBeta proves it, a BETA entry does
  not; Weiss #771 later ignores the bound for +4.99 STC, an S127-era sweep
  here.
- - **TT store exists**: `tt_store_entry(state->tt, &game->board, probDepth,
  normalize_score(value, ply), TT_BETA_NODE, moves[i], static_eval)`
  (`transposition_table.hpp` `tt_store_entry`); static_eval is whatever RFP
  left at `src/search.cpp` `negamax`, TT_EVAL_NONE otherwise (S094: the score
  field, never a bound).
- **Nothing from S097 is needed**: no excluded-move parameter, no cutoff or
  store suppression -- ProbCut excludes nothing and its sub-searches probe
  and store normally. probDepth arithmetic: `probDepth = depth -
  PROBCUT_DEPTH_OFFSET` (seed 4, the paper's depth pair -- section 4), with
  the depth threshold
  keeping probDepth >= 1.

### 3. Implementation sketch

One SPRT, as the accepts prices. Increments:

1. 1. Three constants in the search_params.hpp X-macro
   (`src/search_params.hpp` `CHESSO_SEARCH_PARAMS`), ranges stated.
2. 2. The block after the null move in `src/search.cpp` `negamax`. Entry:
   `!is_pv && !is_in_check && ply > 0 && depth >= PROBCUT_MIN_DEPTH && beta <
   MATE_MIN && beta > -MATE_MIN` (the RFP guard row above it is the house
   pattern), the TT skip
   above, and `excluded_move == 0` once S097's parameter exists (section 5).
3. 3. The loop: captures generated and filtered, pick_next_move by
   capture_score; per move make, preliminary qsearch, shallow search only if
   the preliminary held, unmake; `state->aborted` checked after each sub-search
   (`src/search.cpp` `negamax` pattern) and nothing concluded from an aborted
   score. On `value >= probBeta`: store as section 2, return value (fail-soft).
4. Tests, red first, printouts recorded:
   - the accepts' mate case: a forced mate for the defender inside the pruned
     window behind a crushing-looking capture, built the S033 way --
     python-chess enumeration plus Stockfish confirmation, never own judgement
     (DEC-023) -- added beside "pruning does not hide a forced mate",
     `tests/test_search.cpp` "pruning does not hide a forced mate", observed
     red with the mate-band guard removed.
   - precondition tests, non-vacuous: a position where ProbCut fires (node
     counts move against the off value); then PV node, in check, depth
     below threshold, and a planted under-probBeta TT entry
     (tt_store_entry is public) each hold the counts still.
   - both mate cases re-run -- "pruning does not hide a forced mate",
     `tests/test_search.cpp` "pruning does not hide a forced mate" and "pruning
     does not hide a mate against the material leader", `tests/test_search.cpp`
     "pruning does not hide a mate against the material leader"; fast suite;
     search_bench 9/12 in the stamp.

### 4. Constants and seeds

All in the `CHESSO_SEARCH_PARAMS` X-macro in `src/search_params.hpp` with
ranges (S073); every number is a **seed -- must be fitted/SPSA'd here**
(S127). Under DEC-105 each is one of three forms and says which: **(a)** a
value from a publication about the technique, with its URL; **(b)** a
derivation over chesso's own data or scale; **(c)** the range midpoint or off
value, stated as such. ProbCut is the one technique in this block with an
open paper that states its own parameters, so (a) and (b) carry the whole
section. No engine's shipped margin or gate seeds anything here.

**Units, once for this file (P6).** The margin is compared against
`evaluate()`, so it is in chesso's material scale, `piece_value` in
`src/eval_tables.hpp`: `PAWN` 94, `KNIGHT` 327, `BISHOP` 308, `ROOK` 487,
`QUEEN` 716. The header's own comment says the split between `piece_value`
and `psqt_mg` / `psqt_eg` is degenerate, so the material term alone is the
unit. A figure the paper quotes in *its* engines' scale is not convertible
and is not a seed.

- `PROBCUT_MARGIN` -- **(b) derivation**, the paper's own method run over
  chesso's own positions, **P1**, by this step at its start; the threshold it
  is run at is **(a)**, `t = 1.0` from
  https://skatgame.net/mburo/ps/chessmpc.pdf (Jiang and Buro, ACG 10; Figure
  2 `#define T 1.0`, and "the cut threshold 1.5 is no good"). Range 50..2000,
  off = range top (probBeta unreachable). **The procedure.** Tune build. Over
  the 300-position stratified pick (`adocs/data/S021_aspiration_sweep.py`),
  run `go depth d'` and `go depth d` with `d' = PROBCUT_MIN_DEPTH -
  PROBCUT_DEPTH_OFFSET` and `d = PROBCUT_MIN_DEPTH` -- 4 and 8 at the seeds
  below -- reading `score cp` from the last `info` line. Drop a position where
  either score is a mate score or `|v'| > 3 * PAWN`, the paper's own
  exclusions ("We only used v' data points in the range [-300, 300]"; mate
  scores excluded because a 4-ply search misses a mate an 8-ply search finds
  "roughly once every 1000 positions"). Least-squares `v = a*v' + b`; `sigma`
  is the standard deviation of the residuals; seed `PROBCUT_MARGIN =
  round(t * sigma)` at `t` = 1.0. Record `a`, `b`, `sigma` and the surviving
  and excluded counts in this step's stamp. Section 3's `probBeta = beta +
  margin` is the paper's `(t*sigma + beta - b)/a` at `a` = 1, `b` = 0, and
  the regression is what says how far chesso is from that. **Fewer than 200
  surviving positions is a finding, not a seed: widen the set.**
- `PROBCUT_DEPTH_OFFSET` **4**, range 2..8 -- **(a) literature.** The paper's
  single-pair implementation is the depth pair (4, 8): Figure 2 `#define S 4
  // depth of shallow search` and `#define H 8 // check height`; Table 1
  regresses (3,5) and (4,8), and the MPC pair list is (2,6), (3,7), (4,8),
  (3,9), (4,10). Same URL. That an engine also ships 4 is irrelevant --
  provenance is the paper.
- `PROBCUT_MIN_DEPTH` **8**, range 3..10 -- **(a) literature**, the check
  height `H` of the same pair, so at the minimum depth the shallow search is
  the paper's 4-ply search. Same URL. Sweep the range as declared.
- The stored depth (probDepth vs probDepth + 1 for the qsearch stage): no
  publication states either, and chesso's own table has no measurement to
  derive from yet -- store probDepth, the depth actually searched, and record
  the choice in the commit.

seeds re-derived 2026-09-04 under DEC-105 (DEC-134)

### 5. Pitfalls

- **Anti-seeds — records, not seeds.** DEC-019 lets a record say which
  direction is worth trying; DEC-105 forbids any of these numbers starting
  the fit, which is why section 4 names no engine. The shipped margins
  section 1 quotes are those engines' tuned output: SF-2014 `beta + 200`
  (012f20d6), Weiss `beta + 200` (#567), Ethereal 80 -> 100 cp (a8c9baf),
  Berserk 110 (#215); SF-2024's `beta + 390` (bb4b01e3) is an NNUE-era
  internal scale and is not even convertible. SF prose allowing a minimum
  depth of 3 as of 2025 with the verification collapsed into the qsearch
  (f00d91f) is a record of a *form*, not a gate value. **And the paper's own
  fitted numbers are not seeds either**: Jiang and Buro measured `a` 0.998 to
  1.11, `b` -7.0 to 2.36 and `sigma` 51.8 to 82.0 at pawn = 100 over about
  2700 positions "chosen randomly from some computer chess tournament games
  and some of Crafty's games against human grandmasters" (p. 9; Yace appears
  only in Table 4's validation matches, not in the regression sample). Expect
  that order of magnitude in chesso's scale as a sanity check on P1's output
  and do not seed from it -- that is another engine's positions read by
  another engine's search.
- **Mate-band beta is the load-bearing guard.** The margin sits on top of
  beta, and SF's 2014 assert bug is exactly that arithmetic escaping its
  bounds (012f20d6). Here the guard does two jobs: it stops a fail-high
  against a mate-bound beta claiming a mate this never proved (the red
  test), and it keeps probBeta inside sane score space. A genuine mate
  score *returned by the shallow search* is different -- a real search
  proved it (Weiss #731: "terminal score found through probcut is proven
  despite reduced depth", +1.24) -- V1 returns it as-is only if it clears
  probBeta, which is sound either way.
- **TT pollution runs through the store depth**: the parent stores only on
  success and only at probDepth -- stored at `depth`, this node would answer
  a later full-depth probe with a shallow result. The sub-searches store
  normally; that is published practice, unlike S097's excluded-node store,
  which is suppressed. The preliminary qsearch is nearly free on re-visits
  because of it (S094/S130).
- **Null move stays and ProbCut joins it.** Both prune expected fail-highs
  -- the origin paper measured plain ProbCut beside null move at "no better
  nor worse", and CPW's history says "they tend to prune the same type of
  positions". Published practice keeps both, null move first (SF fca0a2dd8
  moved probcut after null search and has kept both since 2011): they
  answer differently -- a pass searched at depth-1-R against beta, a capture
  searched at depth-4 against beta+margin -- and the capture form fires on
  tactically hot fail-highs a pass understates. What the overlap costs is
  the header's "expect little".
- **S097 confusion, one sentence**: the singular verification lowers a bar
  under the TT score and excludes the TT move to classify it; ProbCut
  raises a bar over beta and excludes nothing, so none of S097's plumbing
  applies. One real coupling: gate ProbCut on `excluded_move == 0` -- its
  capture loop would otherwise search the excluded move itself; SF prose
  records the same gate, folded into a null-move condition (c0964fc7).
- **The repo's pruning-hid-a-mate history** (CLAUDE.md): null move hid a
  mate in 2, LMR reduced the root mating move. ProbCut's version is the
  shallow search missing the deep mate behind the capture -- Jiang/Buro's
  once-per-1000-positions figure -- bounded by the depth offset and caught
  by the accepts' red test, never by a benchmark.
- **Root and PV are excluded by construction** -- is_pv covers ply 0; Stash
  measured the root exclusion at +1.95 after shipping without it (#147).
  Node accounting needs nothing: Stash #148 fixed theirs, but here
  explored_nodes increments inside quiescence/negamax themselves.

### 6. Measurement

One SPRT at the S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`,
fast suite and both mate suites green first, red observations recorded.
Expectation (DEC-019, direction only): 0..+6 -- Weiss +6.36/+6.65 at ~2950
is the record's ceiling, Berserk's removal/reintroduction brackets zero
above 3400, Lynx never carried it. A stall near +2..+3 straddles the bounds
(DEC-063): terminate, record zero, argue keep-or-drop in the stamp
(S005/S006/S015 precedent). Node counts move by construction, so INV-6
takes the SPRT path; search_bench depths 9/12 recorded per the stamp, plus
the fixed-node depth check (`go nodes 1000000` over the three bench
positions) -- pruning should raise fixed-node depth, and a fall with no
SPRT gain is the failure signature.

### 7. Interactions

- **S091 (before)**: shares only the SEE predicates; if S091 lands a
  capture-SEE skip in the main loop, ProbCut's restricted loop is untouched
  (different site, own filter).
- **S112/S022/S131 (before)**: reshape the qsearch the preliminary calls --
  no edit owed; their verdicts land first, so this SPRT measures ProbCut
  against the final quiescence.
- **S130/S108 (before)**: S130's TT stand-pat makes the preliminary cheaper
  on warm entries for free. S108's static eval at every node is what the
  Weiss #658 SEE-vs-probBeta filter and SF's improving-scaled forms
  (81c1d310 "decrease probCutBeta based on opponentWorsening"; d648350
  improving cuts probDepth by 1) consume -- S127-era refinements, not V1.
- **S097 (before)**: the `excluded_move == 0` gate (section 5); nothing
  else shared; multicut stays S097's per the excludes.
- **S114 (after, ladder neighbour)**: re-decides the null-move reduction,
  which shifts how much fail-high work reaches ProbCut -- why the two are
  separate verdicts in this order.
- **S127**: margin, threshold and offset join the SPSA set; deferred
  refinements priced there -- the SEE-vs-probBeta filter, the improving
  margin, the in-check variant (SF 7c30091a "ProbCut for check evasions",
  2021), TT-move conditions (Ethereal c2f4305 +3.64/+4.16; SF af181d9
  removed its tt-move condition, 2025).

### 8. References

- - https://www.chessprogramming.org/ProbCut -- Buro citations, the linear
  model and cut condition, the Crafty history, "until Stockfish proved
  otherwise".
- - https://skatgame.net/mburo/ps/chessmpc.pdf -- Jiang, Buro, ACG 10, 2003;
  read in full: regression table, depth pairs, thresholds, match results, the
  mate-miss figure.
- - Buro, ICCA Journal 18(2) 1995; NECI TR 96, 1997 -- cited via CPW and
  https://skatgame.net/mburo/publications.html; probcut.pdf/improve.pdf not
  fetched.
- - https://api.github.com/repos/TerjeKir/weiss/pulls/283 -- the introduction
  body and both SPRT blocks.
- - https://api.github.com/search/issues?q=repo:TerjeKir/weiss+probcut+type:pr
  -- #312, #484, #567, #658, #685 lower the returned score +1.66/+1.53, #731,
  #771.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+probcut
  (and probCutBeta, "verify probcut", "Refine probcut", pre-2011 date filter)
  -- 3ef4fdea, fca0a2dd8, 33369631/62c0dc5d, 012f20d6, 6aa9308f, 1ceaea70,
  71cc01c, 6edc29d7, f00d91f8, 7690fac5, 7c30091a, af181d9, d648350, bb4b01e3,
  81c1d310, d5a36a3c -- message text only.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+probcut --
  744f902, 3eccbd0, da443c8, e56eda0, df97626, f4439e5, d01aa26, 891b458,
  45851f3, ca739a0, c2f4305, a8c9baf.
- - https://api.github.com/search/issues?q=repo:jhonnold/berserk+probcut --
  #215, #503, #507 with bodies.
- - https://api.github.com/repos/mhouppin/stash-bot/pulls/141 and
  https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+probcut --
  #141, #142, #147, #148.
- - https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+probcut --
  total_count 0, commits likewise: Lynx has none.
- - https://talkchess.com/forum3/viewtopic.php?t=67602 -- Ethereal version
  ledger, "9.74 ProbCut" (re-cited from S091's research pass).

## What the tree already did, 2026-09-27 (checked at `1e9827d`)

The step file was written on 2026-08-19 at `0edfd26`; the implementing agent
checked each assumption on the tree before writing code, by symbol. The brief
named `1680439` as the base; the worktree was at `1e9827d`, whose `src/`
differs from `1680439`'s by one comment (S238's removal note) and whose engine
is the same to the node (`bench` 4803214 both).

- **The null-move block's shape** (`src/search.cpp` `negamax_at`): reverse
  futility first, then the null move under `!is_pv && !is_in_check && ply > 0
  && prev_move != 0 && excluded_move == 0 && depth - 1 - null_reduction >= 1`,
  both mate-band edges and `game_phase > 0`, returning `beta` for a mate-range
  null score. **What follows it is S097's singular block, not the move loop**:
  S097 verdict 1 (the extension) and verdict 2 (the multicut) are both live at
  1 -- `SeExtend` and `SeMultiCut` are in `src/search_params.hpp` at 1. The
  brief's "singular extension H0 and removed" does not match the tree. ProbCut
  went between the null move and the singular block, as section 2 places it.
- **No ProbCut-like block existed**; `specs.md` lists ProbCut as absent.
- **S097's plumbing, and section 2's "nothing from S097 is needed" is half
  right.** ProbCut needs no plumbing of its own from S097, but section 5's gate
  is needed and was taken: `excluded_move == 0` (the capture loop would
  otherwise search the excluded move and answer the verification with a
  bound). And S097 left exactly what the table skip wants: `tt_entry_depth`,
  `tt_entry_type` and `tt_entry_score` are copied out and de-normalised before
  the null move recurses. **So the skip adds no de-normalise site**; section
  2's "a new de-normalise site, S106's lesson" was satisfied by reading S097's
  copies, which are also safe against the slot being overwritten by the null
  move's subtree -- a fresh read of `tt_entry` there would not be.
- **The table API** is as section 2 says: `tt_store_entry(tt, board, depth,
  score, type, move, eval)`, `normalize_score` on the way in; `static_eval`
  is computed at the top of every node now (S108), and in check is the
  sentinel -- ProbCut never runs in check, so it always stores a real one.
- **The good-capture filter** is quiescence's pair, `capture_cannot_lose`
  then `see_ge(0)`; both handle a promotion (`see_ge` adds the promotion gain),
  so the published "captures and promotions" set is filtered by one test.
  `generate_captures` emits non-capture promotions too, and they are kept.
- **Aborts**: `state->aborted` after each sub-search, as every recursion in
  `negamax_at` does; quiescence returns its stand pat on an abort, so nothing
  is concluded from a sub-search's value before the check.
- **The child label**: every recursion in `negamax_at` now carries S098's
  node-type labels, which section 2 predates. The ProbCut child takes
  `null_move_child`'s label -- a capture here is one of the node's "candidate
  cutoff moves", Kannan's reading -- rather than a new rule.
- **The shallow search's depth, as written.** Section 2's call searches the
  child at `probDepth = depth - 4`, which is `depth - 3` plies of the node's
  own; section 4 calls that "the paper's 4-ply search" at the minimum depth,
  and it is a 5-ply one. The code follows section 2's call; the seed follows
  section 4's regression pair (4, 8). The (5, 8) pair's spread was measured
  beside it for S127 (sigma 39.83 over the 300 set).

## What landed

**As first landed, at `ProbCutDepthOffset` 4; superseded by "Fast-check
fix-ups" below**, kept as written (DEC-215 clause 3).

- `src/search_params.hpp`: `PROBCUT` ("ProbCut", 1, 0..1, the switch),
  `PROBCUT_MARGIN` ("ProbCutMargin", 49, 0..2000), `PROBCUT_DEPTH_OFFSET`
  ("ProbCutDepthOffset", 4, 2..8), `PROBCUT_MIN_DEPTH` ("ProbCutMinDepth", 8,
  3..10), each seed's form at its site.
- `src/search.cpp` `negamax_at`: the block after the null move and before
  S097's singular block. Guards `ProbCut != 0`, `!is_pv`, `!is_in_check`,
  `ply > 0`, `excluded_move == 0`, `depth >= ProbCutMinDepth`, `depth -
  ProbCutDepthOffset >= 1`, `beta > -MATE_MIN` and `probBeta < MATE_MIN` --
  the positive edge asked of the bar, so no margin lifts it into the band. The
  table skip on S097's copies (depth >= probDepth, score < probBeta, upper
  bound or exact). Its own `generate_captures` list filtered by
  `capture_cannot_lose` / `see_ge(0)`, ordered by `capture_score` through
  `pick_next_move`; per move `make_move`, the preliminary
  `-quiescence(-probBeta, -probBeta + 1, ply + 1, 0)`, and only where it holds
  the shallow `-negamax_at<false>(-probBeta, -probBeta + 1, probDepth, ply +
  1, ..., null_move_child's label)`; `unmake_move`; `state->aborted` checked
  after. On `value >= probBeta`: `tt_store_entry` at probDepth, `TT_BETA_NODE`,
  the move and `static_eval`, and the fail-soft value returned. The stack
  frame of `negamax_at` grows 16 bytes (`-fstack-usage`, 3504 -> 3520): the
  compiler shares the block's two arrays with the move loop's.
- `src/data_structures.hpp`: nine `probcut_*` fields on `search_node_probe_t`,
  written only in the probing instantiation.
- Nothing else in `src/`. **The stored depth is probDepth**, the depth the
  shallow search was asked for, as section 4 says; recorded for the commit.

## Seeds (DEC-134)

**As first landed, at `ProbCutDepthOffset` 4; superseded by "Fast-check
fix-ups" below**, kept as written (DEC-215 clause 3).

- `ProbCutDepthOffset` 4 and `ProbCutMinDepth` 8: **(a)**, the paper's depth
  pair (S, H) = (4, 8), Figure 2 of
  https://skatgame.net/mburo/ps/chessmpc.pdf. Section 4 as written.
- `ProbCutMargin` 49: **(b) at an (a) threshold**, section 4's procedure run
  by `adocs/data/S113_probcut_fit.py` on the parent build. **Section 4's set
  fell short of its own floor**: the 300-position pick left 180 survivors
  (117 dropped on |v'| > 3 pawns, 3 on a mate score; a 0.9387, b 0.48, sigma
  47.94, `adocs/data/S113_probcut_fit_300.tsv`). Widened as section 4 says,
  to six offsets of the same pick, 600 distinct positions: **375 kept, a
  0.9508, b -1.05, sigma 48.98, r 0.924 -> round(1.0 * 48.98) = 49**
  (`adocs/data/S113_probcut_fit.tsv`). The paper's own a and b were near 1 and
  0 and so are chesso's; `probBeta = beta + margin` drops the a-scaling of
  the paper's bar, (t * sigma + beta - b) / a, which sits 3.6 cp above
  beta + 49 at beta = 0 and 14 cp above at beta = 200 (the coordinator's fast
  check, 2026-09-28) -- the seed is one sigma at t = 1.0 and S127 fits it. **The seed lies under section 4's declared floor
  of 50**, so the range floor went to 0 (at 0 the bar is beta itself, the
  least the rule's form admits) rather than the seed being clamped to a
  number no DEC-134 form produced. The pair (5, 8) -- the shallow search as
  section 2's call actually runs it -- measured sigma 39.83 over the 300 set;
  S127's input, not a seed.
- **Not taken**: any "200 cp". The brief asked whether a 200 cp seed from
  the wiki is a permitted form; this file as S180 reseeded it on 2026-09-04
  carries none, and the only 200s in it are section 5's anti-seeds, two
  engines' shipped `beta + 200` -- another engine's constant wherever it is
  quoted (DEC-105, DEC-134), so not a permitted form. Every engine margin of
  section 5 likewise. None of the three is fitted here; S127 fits them
  (DEC-084).

## Fire rate, before any game (section 6)

**As first landed, at `ProbCutDepthOffset` 4; superseded by "Fast-check
fix-ups" below**, kept as written (DEC-215 clause 3).

An instrumented copy of the candidate (`.ref-builds/fire`, write-only
counters; its `bench 12` is the candidate's own 2102145 and its `bench` the
candidate's 4704447) over the eight bench positions at depth 12: **285924**
nodes reach the block's site, **1287** pass every guard (0.45 %), 211 of those
are answered by the table skip, 1531 captures pay the preliminary, 796 the
shallow search, and **758 nodes are cut** -- 58.9 % of those entered, 70.4 %
of those not skipped; 5 of the cuts return a mate score a shallow search
proved. At depth 14 (`bench`): 665296 / 3738 / 877 / 4221 / 1944 / **1855**,
40 on a mate score. `.tuning/coord/S113_fire.txt`.

## Measurements (2026-09-27, niced beside S112's SPRT; counts only)

**As first landed, at `ProbCutDepthOffset` 4; superseded by "Fast-check
fix-ups" below**, kept as written (DEC-215 clause 3).

- Off value (DEC-215): tune build at `ProbCut` 0 -> `bench` **4803214**, all
  eight replies identical to a parent built from `1e9827d` (c3d5 d5e6 d7c8q
  g7h8q d8e7 a1b2 e5e6 e5e6); `search_bench` identical at 9 (48522 / 85714 /
  28080) and 12 (129499 / 411457 / 172984), best moves c3d5 e2a6 d7c8q.
  `.tuning/coord/S113_identity.log`.
- `bench` 4803214 -> **4704447** (-2.06 %); **one reply moved**, position 2
  d5e6 -> e2a6. `bench 12` 2108720 -> 2102145, replies unchanged.
- `search_bench` 9: 48522 -> 36972, 85714 -> 77154, 28080 -> 27788; 12:
  129499 -> 194859, 411457 -> 332494, 172984 -> 105356; best moves unchanged.
- **No existing golden moved.** Both fast suites green at margin 49 with every
  mate row as it stood; the only reds were ctest's 120 s ceiling on
  `test_mate_carry` under the SPRT's load in the tune build (run directly:
  parent 113.3 s, candidate 108.8 s, both green; Release parent 111.9 s,
  candidate 113.1 s) -- a load reading, re-time on the idle machine.
  `./clang-format.sh --check` green. `tools/plan_prose_check.py --citations`,
  `--touches`, `--params`: 0 flagged.
- **The accepts' mate row**, mined (DEC-209 rule, `adocs/data/S113_mine_mate_row.py`,
  `adocs/data/S113_mine.log`): 9 of S097's 269 candidates separate the shipped
  build from `B04_probcut_defender_gate_dropped` over depths 9 to 12, the
  switch moving the tree on all 9; pick `1R6/8/2p3p1/P5P1/1p2b2P/4k3/6pK/8 b -
  - 1 54`, depth 12, mate in 5 (shipped d9-d12, B04 d9-d11). Added to
  "pruning does not hide a forced mate"; **observed red under B04**
  (`REQUIRE( result.mate_found )`, "mate probcut hides, depth 12") and green
  shipped, `.tuning/coord/S113_mate_row_red.log`. The two existing mate cases
  green throughout.
- Tests: nine cases (plus a tune-only off-value case) at the end of "search:
  pruning and reduction guards", one drive firing the block and each other
  case denying one condition on the same drive. Their reds are observed under
  the mutants below rather than on a no-rule tree: the fire and store cases
  go red under B03, B06 and B07.
- Mutation (DEC-141 clause 2), `tools/mutants/S113_probcut.py` on a clean
  detached fixture `1142f32` in `.ref-builds/mut` (the working tree committed
  on `1e9827d`): header `baseline green, 40 tests, bench 4704447 nodes via
  engine`; **mutation score 14 of 14 (100 %)**, every kill by its named case;
  B01, B04 and B13 also by the mined mate row, B03 also by test_mate_carry.
  B04, B08, B10 and B14 leave the bench signature unmoved -- only their cases
  see them. `.tuning/coord/S113_mutation.log`, 5062 s.
- Tune `test_search` re-run after the mate row landed: 172 of 172.
- Not run here, by the brief: the Debug self-play and `tools/gate_extra.sh`
  of DEC-141's second tier, and the SPRT.

## Fast-check fix-ups, 2026-09-27

The coordinator's fast check found one real defect and three small ones.
Everything the change moves was re-derived, never carried over. Niced beside
S112's SPRT throughout.

1. **Depth pair (real).** The block searched the capture's child at `depth -
   4`, which with the capture is a node-level pair of (5, 8), while the margin
   49 was fitted on (4, 8) -- so the seed ran at about t = 1.23 while labelled
   t = 1.0. Coordinator ruling (option a): **`ProbCutDepthOffset` 4 -> 5**,
   the node-level pair is (4, 8) as in Jiang and Buro's Figure 2, and the
   margin 49 is kept from the (4, 8) fit at t = 1.0. Fit, seed and paper now
   agree. Range 2 to 8 holds 5 with room either side.
   **The store depth is now the node-level depth**, `probcut_depth =
   probcut_child_depth + 1`: what the capture plus the child's search proved
   about this node, the move loop's own accounting (it stores `depth` for
   children searched at `depth - 1`). The table skip reads the same number,
   so a stored ProbCut cut is exactly an entry the skip would accept. The
   guard reads `probcut_child_depth >= 1`.
2. **Re-derived on the offset-5 tree** (logs of the offset-4 runs kept as
   `.tuning/coord/S113_*_offset4.*`):
   - Off value, re-confirmed: tune build at `ProbCut` 0 benches **4803214**,
     all eight replies the parent's; `search_bench` identical at 9 and 12.
     `.tuning/coord/S113_identity.log`.
   - `bench` 4803214 -> **4152835** (-13.54 %; was 4704447, -2.06 %), **all
     eight replies unchanged** (offset 4 had moved position 2). `bench 12`
     2108720 -> 1907764, one reply moving there (position 5, d8e7 -> f8e8).
   - `search_bench` 9: 48522 / 85714 / 28080 -> **35289 / 76531 / 27788**
     (was 36972 / 77154 / 27788); 12: 129499 / 411457 / 172984 -> **159199 /
     256618 / 114729** (was 194859 / 332494 / 105356); best moves unchanged.
   - Fire rate at depth 12: site 260584, entered **1385**, table skip 240,
     preliminaries 1529, shallow searches 819, **783 cut** (56.5 % of entered),
     6 on a mate score. Depth 14: 579157 / 3531 / 753 / 3912 / 1832 / 1754,
     44 mate. `.tuning/coord/S113_fire.txt`.
   - **Mate row re-mined under DEC-233.** The offset-4 row no longer separates
     (both builds find its mate at 9 to 12). The script's rule picked
     **`8/8/1P6/P7/5k2/4p2P/2r5/6K1 b - - 0 60`, depth 11, mate in 4**
     (shipped d9 d10 d11, B04 d9 d10 d12; 6 of 269 separate, 4 losing the
     mate). Stockfish depth 20: #+4 for Black, pv Kf3 h4 e2 Kh2 e1=Q+ Kh3
     Qh1#. Observed red under B04 and green shipped,
     `.tuning/coord/S113_mate_row_red.log`. The old row is quoted in the
     GOLDEN block. `adocs/data/S113_mine.log`, `S113_mine_offset4.log`.
   - Mutant anchors re-checked: all fifteen occur once. Mutation on a fresh
     fixture: see the last bullet.
3. **Margin 0.** Legs 3 and 4 of "probcut does not run with beta in the mate
   band" are guarded by `if (PROBCUT_MARGIN > 0)`: at 0 the bar is beta and
   the positive edge is the beta guard's.
4. **`probcut_child_depth >= 1` now has a case**: tune-only "probcut does not
   search a capture's child at depth zero" (ProbCutMinDepth 3, Offset 3:
   refused at depth 3, runs at depth 4). Mutant **B15** drops the term; it is
   **declared equivalent** in the mutant file because the release build, the
   one `mutation_check.py` runs, folds the term to true (8 - 5 >= 1 at every
   eligible depth). Observed red by hand under B15 in a tune build and green
   shipped: `.tuning/coord/S113_b15_red.log`.
5. Both fast suites: every test green except `test_mate_carry` at ctest's
   120 s ceiling in both builds under the SPRT's load; run directly it passes
   in 108.6 s (Release) and 111.7 s (tune) against the parent's 112.0 s.
   `./clang-format.sh --check` green; `plan_prose_check.py --citations`,
   `--touches`, `--params`: 0 flagged.
6. **Mutation on a fresh fixture**, `tools/mutants/S113_probcut.py` on
   `cc25326` in `.ref-builds/mut` (the working tree committed on `1e9827d`):
   header `baseline green, 40 tests, bench 4152835 nodes via engine`;
   **mutation score 14 of 14 (100 %), B15 equivalent as declared**, wall
   5163 s. Every kill by its named case; B04, B08, B10 and B14 leave the bench
   signature unmoved. `.tuning/coord/S113_mutation.log` (offset-4 run:
   `S113_mutation_offset4.log`, 14 of 14).

## Proposed `specs.md` sentence (the coordinator edits specs.md)

ProbCut leaves the "absent, search" list. In the search row: at a non-PV node
out of check below the root, at a remaining depth of at least
`ProbCutMinDepth` and with beta outside the mate band, `negamax_at` tries each
capture and promotion the exchange evaluation does not write off against
`beta + ProbCutMargin` -- a zero-window quiescence, then a search of the
capture's reply at `depth - ProbCutDepthOffset` -- and a capture that clears
the bar ends the node with its fail-soft score, stored as a lower bound at the
node-level depth that search proved, `depth - ProbCutDepthOffset + 1`; skipped
when the table already holds an upper bound or exact score under the bar at
least that deep (S113). `ProbCut` 0 is the tree before it, node for node.

## Rebased onto S112's tree, 2026-09-28

S112 read a zero at the cap and its code stayed (DEC-236), so this step lands
on `308b388`, `bench` 4649650, with per-move futility live in the
`quiescence` the preliminary calls. Everything above was measured on
`1e9827d` (`3cede8c` after DEC-235's rewrite, `src/` identical) and stands as
written; what follows was re-taken on the rebased tree (DEC-233). The session
log is `.tuning/coord/S113_rebase_report.md`.

**Rebase.** Three conflicts, both sides kept and S112's first:
`tests/test_search_params.cpp`'s golden count (64 -> **68**, 68 rows),
`DEV_MANUAL.md`'s bench ledger and `adocs/data/README.md`. The rest merged on
its own. `git diff --stat 308b388 -- src tests tools` lists ProbCut's files
only, their lines the WIP's; every mutant anchor occurs once.

**Off value** (DEC-215), the parent built from `308b388` and measured:
`bench` 4649650, replies c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6; `bench 12`
1981759; `search_bench` 9 at 53598 / 80389 / 25691 and 12 at 154098 / 472358 /
107876, best moves c3d5 e2a6 d7c8q. The tune build at `ProbCut` 0 reproduces
all of it (`.tuning/coord/S113_identity_rebased.log`).

**Candidate.** `bench` **3591364** (-22.76 %), `bench 12` 1700466, every reply
unchanged; `search_bench` 9 **34773 / 71512 / 25413**, 12 **110496 / 244771 /
117798**, best moves unchanged. **Fire rate** at depth 12: site 240525,
entered 1345, table skip 213, preliminaries 1450, shallow searches 806, **778
cut**, 1 on a mate score; depth 14: 510798 / 3218 / 712 / 3617 / 1734 / 1660,
19 (`.tuning/coord/S113_fire_rebased.txt`, a write-only copy benching the
candidate's own totals).

**Suites, and three goldens that moved.** The first run was 40 of 41 in both
builds, red at S097's mined row "mate the multicut hides, depth 14" -- a golden
the brief had not listed. Its `REQUIRE` is fatal, so the later rows were read
through their scripts' shipped sweeps, and two more had moved. Each change
alone is green on its own tree's rows. All three were re-derived by their own
scripts (DEC-142, DEC-233), the old rows quoted in their GOLDEN blocks, no
assertion relaxed:

- S097's multicut row, `adocs/data/S113_remine_s097.log`:
  `4N3/8/3P1ppk/4p2p/4P2P/1n1P2P1/Q4PK1/3q4 w - - 5 46` at 14, mate in 5 ->
  `1r2r2k/8/p2pp1Q1/8/1PppP3/P2q3P/6P1/1R3RK1 w - - 0 35` at 14, **mate in 4**.
- S112's capture row 4, `adocs/data/S230_mine_r01_row.py depths`, all seven
  sweeps: `{9, 5, "C05, C07, since S112"}` -> `{10, 5, "no S091 mutant, since
  S113"}`; rows 1 to 3 unchanged.
- This step's ProbCut row, `adocs/data/S113_mine.log`:
  `8/8/1P6/P7/5k2/4p2P/2r5/6K1 b - - 0 60` at 11, mate in 4 ->
  `5r2/2Q3k1/3p2p1/7p/p6P/P1P2q1b/5B1K/8 b - - 0 44` at 11, **mate in 5**.

Observed red under E21 and under B04, which is red on the new multicut row
too (`.tuning/coord/S113_rb_red.log`). Then **41 of 41 in both builds**;
`test_mate_carry` alone 56.88 s Release and 58.10 s tune, against 120 s. The
format check and `plan_prose_check.py --citations`, `--touches`, `--params`
are clean; `test_uci_surface` green without a refresh.

**Mutation** (DEC-141 clause 2), `tools/mutants/S113_probcut.py` on a fresh
fixture, `.ref-builds/mut` at `b9e6962`, a throwaway detached commit of this
working tree whose `src/` and `tests/` are the landing's: header `baseline
green, 41 tests, bench 3591364 nodes via engine`; **mutation score 14 of 14
(100 %), B15 equivalent as declared**, wall 2191 s. Every kill by its named
case; B02, B04 and B05 also by "pruning does not hide a forced mate". Two
targeted extras on the same fixture: E21 killed by that case, the re-mined
multicut row, and C07 by its own direct case
(`.tuning/coord/S113_mutation_rebased.log`, `S113_mutation_rebased_extra.log`).

**Not re-derived**: the margin seed, fitted on `1e9827d`'s engine; a new seed
would be a new candidate, and S127 fits it. **Not run**: DEC-141's second tier
and the SPRT. **The specs sentence above** omits two guards the code has,
S097's verification and a child depth of at least one ply, and blurs which
edge of the mate band each bound tests; the corrected sentence is in the
coordinator's report.

**Check first**: the multicut row's red and re-mine; C07 is now separated by
no capture row; the bench delta is -22.76 % here against -13.54 % on
`1e9827d`, counts only.

## The coordinator's landing notes (2026-09-28)

**The multicut row's re-mine is confirmed** as DEC-233's rule applied: a mined
golden moves with the tree and is re-derived by its own script, the old row
kept; the combination of ProbCut and per-move futility moved three rows, each
re-derived (`adocs/data/S113_remine_s097.log`, `adocs/data/S113_mine.log`, the
capture table's GOLDEN block), and no assertion was relaxed. A mate the shipped
build finds at depth 13 and not at 14 on that row is the fixed-depth behaviour
DEC-209's rule mines around, not a claim about the search; the SPRT decides the
rule.

**Cold fast check over the rebased diff** (a fresh reviewer, read-only): no
real defect. Bound directions, the store's depth and type, both mate-band
edges, the `excluded_move` gate, the abort paths, B15's equivalence, the
mutation log and the three goldens' re-derivation were each checked and found
sound; S112's fold cannot produce a cut (the preliminary only gates, and a
folded value sits at or below the child's alpha). Six wording findings fixed in
this landing: the probe field's and the store case's comments said the stored
depth is the shallow search's (it is one more, the capture counts);
`MANUAL.md`'s `ProbCut` row and `DEV_MANUAL.md`'s bench paragraph omitted the
S097-verification exclusion and blurred which edge of the mate band each bound
tests; the Seeds section's "within a centipawn of b" ignored a = 0.9508;
`ProbCutMargin`'s comment named "the parent build" where the fit ran on
`1e9827d`'s; the pre-registration called a mate-score cut "proved" where the
shallow search proves nothing (section 5 kept it on purpose). One edge
recorded, not changed: the preliminary is the first caller that enters
`quiescence` at a child `negamax_at` never screened, so a capture leaving
insufficient material is scored on material and stored at `TT_DEPTH_QS`; no
cut is wrong, because the shallow search returns `DRAW_SCORE` first and
`negamax_at` answers a dead node before it probes, so it is open finding 6 of
`adocs/data/S113_sprt.sh` and the S210 comment in `quiescence` now says so
(DEC-171, filler). Noted only: `adocs/data/S113_mine.log` carries S097's
imported print "the multicut changes the tree"; it is ProbCut's witness.

**The landing.** `1424f55` cherry-picked onto `029ff61` with one conflict,
`adocs/data/README.md`, both sides kept (this step's rows, then the two
2026-09-27 documents); `specs.md` takes the corrected sentence and loses
"ProbCut, " from the absent-search row; `DEV_MANUAL.md`'s DEC-142 row for
`mate_the_extra_ply_hides` corrected to depth 8 (11 at S095, re-derived at
S234; the agent's flag, verified against the test). Gate green on the staged
tree after these edits, `bench` 3591364.

**Second tier on the landing** (DEC-141): Debug self-play 8 games at 4+0.04,
0 `Assertion`, 0 `disconnect` (`.tuning/coord/S113_debug_selfplay/`);
`tools/gate_extra.sh` 5 stages green in 1228 s (`.tuning/gate_extra_2026-09-28_S113.log`).
Landed as `3d97737` on `029ff61`, `bench` 4649650 -> 3591364. SPRT pair
pinned: `REF` `029ff61` (the tree S112's zero left, its engine `3d82344`'s
node for node), `CAND` `3d97737`; open findings re-read at pinning, item 6
added by the fast check, nothing else moved since the rebase agent's block.
