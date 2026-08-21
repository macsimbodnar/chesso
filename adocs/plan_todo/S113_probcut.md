id:         S113
goal:       a shallow verification search over good captures prunes a node whose score is already far above beta
accepts:    an SPRT verdict, recorded whatever it is; never at a PV node, never when beta is near mate, and skipped when the table already holds a sufficient-depth entry scoring below the ProbCut beta; the margin and the depth reduction are constants in src/search_params.hpp with ranges and are fitted, not taken (DEC-084); a mate inside the pruned depth is in the fast suite and observed red with the guard removed
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   multicut, which arrives with S097's singular search at no extra cost
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
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
source file (DEC-016, DEC-084). Buro's papers are open literature and were
read in full; their numbers are legal seeds (DEC-084).

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

- **Ladder site**: RFP :511-537, null move :561-588. ProbCut goes after the
  null-move block's close (:588) and before `node_type_t type = ...` (:590)
  -- the SF placement, and a null-pruned node then never pays for captures.
  S097 lands earlier in plan order and its verification block sits in the
  same region; ProbCut goes first, same never-pays argument.
- **qsearch is callable mid-node**: declared search.hpp:35-40, already
  called by negamax at :466. The preliminary is
  `-quiescence(-probBeta, -probBeta + 1, ply + 1, 0, game, state)` after
  `make_move` (illegal moves drop out via its false return, :656 pattern);
  the shallow search is `-negamax(-probBeta, -probBeta + 1, probDepth,
  ply + 1, game, state, moves[i], false)` -- prev_move flows as everywhere.
- **The restricted move set exists**: generate its own capture list before
  the staged loop (the :614 stage regenerates later -- an accepted double
  generation every surveyed movepicker also pays), ordered by
  `capture_score` (evaluation.hpp:308) through `pick_next_move` (:87). The
  "good" filter is the predicate pair quiescence uses at :322-323,
  `capture_cannot_lose(...)` then `see_ge(..., 0)` (bitboard.cpp:1160,
  :1185) -- S015 machinery, nothing new. generate_captures also emits
  non-capture promotions (:303-307 comment); ProbCut keeps them -- the
  published set is "noisy" moves.
- **TT probe already paid**: the entry is copied out at :445-455. The skip
  reads the score through `de_normalize_score(entry->score, ply)` (:139) --
  a **new de-normalize site, S106's lesson** -- and skips when the entry's
  depth reaches probDepth and its score sits under probBeta with an
  upper-bound or exact type. That is the honest V1 form: an ALPHA entry
  under probBeta proves it, a BETA entry does not; Weiss #771 later ignores
  the bound for +4.99 STC, an S127-era sweep here.
- **TT store exists**: `tt_store_entry(state->tt, &game->board, probDepth,
  normalize_score(value, ply), TT_BETA_NODE, moves[i], static_eval)`
  (transposition_table.hpp:35-41); static_eval is whatever RFP left at
  :532, TT_EVAL_NONE otherwise (S094: the score field, never a bound).
- **Nothing from S097 is needed**: no excluded-move parameter, no cutoff or
  store suppression -- ProbCut excludes nothing and its sub-searches probe
  and store normally. probDepth arithmetic: `probDepth = depth -
  PROBCUT_DEPTH_OFFSET` (seed 4, SF prose), with the depth threshold
  keeping probDepth >= 1.

### 3. Implementation sketch

One SPRT, as the accepts prices. Increments:

1. Three constants in the search_params.hpp X-macro (:41), ranges stated.
2. The block after :588. Entry: `!is_pv && !is_in_check && ply > 0 &&
   depth >= PROBCUT_MIN_DEPTH && beta < MATE_MIN && beta > -MATE_MIN` (the
   RFP guard row :511-512 is the house pattern), the TT skip above, and
   `excluded_move == 0` once S097's parameter exists (section 5).
3. The loop: captures generated and filtered, pick_next_move by
   capture_score; per move make, preliminary qsearch, shallow search only
   if the preliminary held, unmake; `state->aborted` checked after each
   sub-search (:729 pattern) and nothing concluded from an aborted score.
   On `value >= probBeta`: store as section 2, return value (fail-soft).
4. Tests, red first, printouts recorded:
   - the accepts' mate case: a forced mate for the defender inside the
     pruned window behind a crushing-looking capture, built the S033 way --
     python-chess enumeration plus Stockfish confirmation, never own
     judgement (DEC-023) -- added beside "pruning does not hide a forced
     mate", tests/test_search.cpp:1887, observed red with the mate-band
     guard removed.
   - precondition tests, non-vacuous: a position where ProbCut fires (node
     counts move against the off value); then PV node, in check, depth
     below threshold, and a planted under-probBeta TT entry
     (tt_store_entry is public) each hold the counts still.
   - both mate cases re-run -- "pruning does not hide a forced mate",
     tests/test_search.cpp:1887 and "pruning does not hide a mate against
     the material leader", tests/test_search.cpp:1926; fast suite;
     search_bench 9/12 in the stamp.

### 4. Constants and seeds

All in src/search_params.hpp with ranges (S073); every number is a **seed --
must be fitted/SPSA'd here** (S127, DEC-084).

- `PROBCUT_MARGIN` **150**, range 50..2000, off = range top (probBeta
  unreachable). Prose seeds: SF-2014 rbeta = beta + 200 (012f20d6's own bug
  arithmetic: "beta = 29832, so rbeta = 30032"); Weiss beta + 200 (#567);
  Ethereal 80 -> 100 cp (a8c9baf); Berserk 110 (#215 title). SF-2024's
  beta + 390 (bb4b01e3) is the NNUE-era internal scale, not centipawns --
  not a seed. Buro's method is the principled fit if the sweep wants one:
  regress chesso's own shallow-vs-deep scores and set margin = t*sigma --
  Jiang/Buro measured sigma 52-82 cp at best t 1.0-1.2.
- `PROBCUT_MIN_DEPTH` **5**, range 3..10. Historic gates are source-only --
  no publishable introduction value; SF prose allows 3 as of 2025 with the
  verification collapsed into the qsearch (f00d91f). Sweep.
- `PROBCUT_DEPTH_OFFSET` **4**, range 2..8 (SF 71cc01c, "'depth - 4
  plies'").
- The stored depth (probDepth vs probDepth + 1 for the qsearch stage): no
  publishable prose either way -- store probDepth, the depth actually
  searched, and record the choice in the commit.

### 5. Pitfalls

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

- https://www.chessprogramming.org/ProbCut -- Buro citations, the linear model and cut condition, the Crafty history, "until Stockfish proved otherwise".
- https://skatgame.net/mburo/ps/chessmpc.pdf -- Jiang, Buro, ACG 10, 2003; read in full: regression table, depth pairs, thresholds, match results, the mate-miss figure.
- Buro, ICCA Journal 18(2) 1995; NECI TR 96, 1997 -- cited via CPW and https://skatgame.net/mburo/publications.html; probcut.pdf/improve.pdf not fetched.
- https://api.github.com/repos/TerjeKir/weiss/pulls/283 -- the introduction body and both SPRT blocks.
- https://api.github.com/search/issues?q=repo:TerjeKir/weiss+probcut+type:pr -- #312, #484, #567, #658, #685 lower the returned score +1.66/+1.53, #731, #771.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+probcut (and probCutBeta, "verify probcut", "Refine probcut", pre-2011 date filter) -- 3ef4fdea, fca0a2dd8, 33369631/62c0dc5d, 012f20d6, 6aa9308f, 1ceaea70, 71cc01c, 6edc29d7, f00d91f8, 7690fac5, 7c30091a, af181d9, d648350, bb4b01e3, 81c1d310, d5a36a3c -- message text only.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+probcut -- 744f902, 3eccbd0, da443c8, e56eda0, df97626, f4439e5, d01aa26, 891b458, 45851f3, ca739a0, c2f4305, a8c9baf.
- https://api.github.com/search/issues?q=repo:jhonnold/berserk+probcut -- #215, #503, #507 with bodies.
- https://api.github.com/repos/mhouppin/stash-bot/pulls/141 and https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+probcut -- #141, #142, #147, #148.
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+probcut -- total_count 0, commits likewise: Lynx has none.
- https://talkchess.com/forum3/viewtopic.php?t=67602 -- Ethereal version ledger, "9.74 ProbCut" (re-cited from S091's research pass).
