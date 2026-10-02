id:         S116
goal:       a node whose static score is hopelessly below alpha drops straight to quiescence, at depth one only
accepts:    an SPRT verdict, recorded whatever it is; the verified form -- the quiescence score is returned only if it is itself at or below alpha; never in check, never at a PV node, never when alpha is near mate; the margin and the depth bound are constants in src/search_params.hpp with ranges; a mate inside the razored depth is in the fast suite and observed red with the guard removed
touches:    src/search.cpp negamax, src/search_params.hpp, src/data_structures.hpp, tests/test_search.cpp, tests/test_engine.cpp, tests/test_search_params.cpp, tests/test_mate_breadth.cpp, tools/mutants/S116_razoring.py
excludes:   the move-loop pruning rules, which are S109
decisions:  DEC-019, DEC-084
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S116_brief.md), 2026-10-01
done:

## This is S026's other half, and it is the smallest thing on the plan

S026 was "drop nodes near the horizon that cannot reach alpha" and covered both
futility and razoring. Its futility half is S109; this is the rest, split out
because razoring is a node-level rule rather than a move-loop one and does not
belong inside the block.

Expect very little. Stockfish's removal test reports **~0**; one engine at
about 2600 measured +7.9 over 4550 games; another added it, tuned it, and then
deleted it. The one consistent finding is that **restricting it to depth 1
gained** where the unrestricted form did not, which is why the goal says depth
one rather than leaving the bound open.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `0edfd26`; re-locate by symbol -- S113 and S114 rewrite this
exact region first by plan order. Every GitHub read below was a commit message
or PR body, never a diff or source file (DEC-016, DEC-084).

### 1. State of the art

**The form.** CPW: razoring "prunes branches forward, if the static evaluation
... is less than or equal to Alpha"; its drop-to-quiescence variant fires when
the eval is "by a margin (~three pawns) below" the bound and quiescence
confirms the fail-low before returning. The modern shape is Stockfish PR
#3921's own sentence: "for low depths if eval is far below alpha we check if
qsearch can push it above alpha - and if it can't we return a fail low" --
the **verified** form, exactly the accepts'. The verification arm is
load-bearing by SF's own 2008 count (8cd5cb9): "razoring verification after
qsearch() cuts more than 40% of candidates" -- a large share of
hopeless-looking nodes recover above alpha in quiescence, and an unconditional
drop returns their wrong-side scores. Heinz's limited razoring (reduce depth
instead of dropping) is the older cousin; no surveyed engine ships it.

**Additions.** Berserk #339 (2022-01): **+6.02 +/-4.18 / +7.17 +/-4.62**,
kept since. Lynx #429 (2023-10): merged at max depth 3, final run **+13.5
+/-8.5** over 4903 games (a maxdepth-3 WIP read +3.6 +/-3.3 over 31520).
Stash #19 (2021-06): "only triggers at depth 1", **+8.05 +/-5.24** over 7424.
Ethereal 09245e09 (2018-07): "Only razor at depth 1, raise margin",
**+9.22/+9.62**.

**Removals.** Ethereal 69850324 (2020-01) "Simplify razoring away": -0.91
+/-1.76 / +0.45 +/-1.82 -- deleted at ~0. Weiss added it (#102, 2019-12,
"no razoring in pvnodes"), lowered the margin (#496, 2021-07-13, +3.30 STC /
+0.97 LTC unresolved), and deleted it **five days later** (#513: "Razor
provides no value anymore"). Stockfish removed it whole (PR #3278, 2020-12,
non-regression over 128816 games -- the file's "removal test reports ~0") and
reintroduced the verified form fourteen months later (PR #3921, 2022-02) as a
1-to-2-Elo gainer over 339248 games, then spent 2022-2026 shaving it (#4147
razors PV nodes too, #4196 drops the depth condition, #7044 "Simplify Away
Second Razoring Number", 2026-08) without deleting it again.

**The depth-1 finding holds in three engines**: Stash #19 and Ethereal
09245e09 both *gained* by restricting to depth 1, and SF 5d57bb4 (2018-02)
passed depth-1-only as a non-regression -- multi-depth bought nothing over
it. Lynx is the counterexample that merged max depth 3; its later widening
attempt failed (#1607, -4.7).

**Honest expected value: 0 to +10 at this band, both endpoints published.**
The additions read +6 to +13.5; two engines deleted at ~0 and SF removed at
~0 before re-buying ~1 at 3600. Chesso's quiescence already SEE-prunes (S015)
and carries S112's per-move futility by plan order -- the closer it gets to
those engines' qsearch, the closer razoring's remainder sits to their zero. A
~0 verdict recorded as zero is a valid outcome (DEC-019). The file's "+7.9
over 4550 games" was not reproduced exactly this pass; nearest traced are
Stash #19 (+8.05/7424) and Lynx #429 (+13.5/4903).

### 2. Shape for chesso

- - **Site: after the RFP block and before the null-move block, both in
  `src/search.cpp` `negamax`.** Lynx #2180 measured razoring *before* RFP at
  -1.06 over 28174 games -- stay after. RFP is the mirror: a static bound
  returned when eval is `RFP_MARGIN * depth` **above beta**; razoring drops to
  quiescence when eval is a margin **below alpha**. Non-PV windows are null, so
  the two
  conditions are disjoint by arithmetic -- the pair covers both tails of the
  same eval.
- **Input**: post-S108 the static eval is computed-or-read at the top of the
  node; razoring reads the **raw static**, never a TT-score-adjusted value --
  Lynx merged removing TT-score-as-eval at +6.65 (#1971) and rejected
  re-adding it for razoring alone at -3.86 (#1974). Guard `!= TT_EVAL_NONE`,
  asserted not assumed (S114's sentinel rule).
- - **The exact form** (verified, per the accepts): at `!is_pv && !is_in_check
  && depth <= RAZOR_DEPTH && alpha < MATE_MIN && alpha > -MATE_MIN`
  (`src/search.cpp` `negamax` is the guard row to mirror, alpha for beta;
  MATE_MIN 48000 at `src/search.cpp` `MATE_MIN`) and `static_eval +
  RAZOR_MARGIN <= alpha`: `score = quiescence(alpha, beta, ply, 0, game,
  state)` (`src/search.cpp` `quiescence` signature; `src/search.cpp` `negamax`
  is the call pattern) -- **return score only if `score <= alpha`**, else fall
  through to the move loop. Fail-soft: return quiescence's own score, never
  alpha. The null window is its own cheapest verification; no second windowing
  to get wrong.

### 3. Implementation sketch

One commit, one SPRT: two constants, ~10 guarded lines between the RFP and
null-move blocks of `src/search.cpp` `negamax`, tests red-first.

- - **Mate case, built the S033 way** (python-chess enumeration + Stockfish
  confirmation, DEC-023, never a judged position): the razoring side far behind
  on material at a depth-1 node yet mating with a **quiet** first move -- out
  of check quiescence generates captures only (`src/search.cpp` `quiescence`),
  so the drop is blind to it by construction. Lands beside "pruning does not
  hide a forced mate", `tests/test_search.cpp` "pruning does not hide a forced
  mate" and "pruning does not hide a mate against the material leader",
  `tests/test_search.cpp` "pruning does not hide a mate against the material
  leader" in the fast suite; observed red against the demolition build
  (verification arm removed, i.e. the unconditional drop) and the printout
  recorded, per the accepts -- record which guard's removal reddens it.
- **In-check exemption, non-vacuous** (S109's precondition pattern): a
  position where the condition would fire but for the check searches
  identically to the off build; the same shape out of check moves the counts.
- **Depth gate**: at RAZOR_DEPTH 1 a depth-2 node never razors -- node counts
  against the off value prove it.
- **Fire-rate pre-check before booking the match** (S112 precedent): count
  candidates and verification cuts over the three search_bench positions at
  depth 12 -- SF's 2008 prose has >40% of candidates failing verification; a
  near-zero fire rate predicts the zero before it costs a run.

### 4. Constants and seeds

Both in src/search_params.hpp's X-macro with ranges (the accepts):

- `RAZOR_MARGIN` seed **300 cp** -- CPW Razoring's "~three pawns", and
  Ethereal 01c41ff3's prose "Assume its approx 3pawns, and let the qsearch
  handle the confirmation". **Seed -- must be fitted/SPSA'd here** (DEC-084).
  Range 0..2000; the top parks the rule (off value).
- `RAZOR_DEPTH` default **1**, range 0..8; 0 is off. Values above 1 are the
  multi-depth variant: with a flat margin that is Lynx's failed widening, so
  a depth-scaled margin comes with it at S127 or not at all.
- **Declined as seeds**: the SF-2022 margin CPW reproduces
  (`512 + 293*depth*depth`-class) -- engine source transcribed onto the wiki,
  not wiki prose; not even a seed (DEC-084).

### 5. Pitfalls

- **In check**: the node is forced, the static is meaningless, and the drop
  would answer an evasion node with zero real plies. Excluded by the accepts
  and by every published form.
- - **Alpha in the mate band**: a static eval provably cannot approach a mate
  score here (the LAZY_EVAL_MARGIN clamp, S033's row), so with alpha near +mate
  the condition is trivially true at every node and the whole subtree under a
  mate-scored bound drops to quiescence. The `src/search.cpp` `negamax` band
  guard, on alpha.
- **PV exemption**: Weiss #102 shipped "no razoring in pvnodes"; SF #4147
  removed the exemption at 3600 as a non-regression simplification. Keep it
  per the accepts; a 3600 simplification is not evidence at this band.
- **S109 double-drop, one sentence**: razoring is node-level and pre-loop --
  the whole node drops to quiescence; S109's futility is per-move inside the
  loop -- individual quiets skipped at lmr_depth. Different mechanism, no
  shared guard, no shared constant.
- **The repo's pruning-hid-a-mate history, fourth verse**: null move hid a
  mate in 2, LMR reduced the mating root move, RFP cannot see mates at all
  (S033). Razoring's blind spot is a quiet mate by the razoring side; the
  depth-1 gate bounds the blindness to one lost ply, and the mate case is the
  enforcement, not this comment.
- **No TT-move gate**: razor-only-without-a-TT-move failed at Lynx (#1541,
  -4.71). Do not add conditions the record priced negative.
- - **Double node count, cosmetic**: negamax counts the node (`src/search.cpp`
  `negamax`) and the razor's quiescence call counts it again (`src/search.cpp`
  `quiescence`) -- Berserk #581 cleaned this at ~0. The `src/search.cpp`
  `negamax` leaf drop has the same property today, so search_bench stays
  internally consistent; note it, do not fix it here.

### 6. Measurement

One SPRT at the S105 regime (8+0.08, Hash 16, UHO book), bounds
**`elo0=-5 elo1=5`**: the expected effect is 0..+10 and plausibly ~+3, and a
gainer pair random-walks a truth that small (DEC-063; S068 is the local
demonstration). Node counts move by construction, so INV-6 takes the SPRT
path; record search_bench depth 9/12 counts beside the verdict. **A fail or
~0 is squarely plausible per the record.** Decision rule, stated now: H1 or a
positive sign -- keep; a capped run at ~0 -- record zero, then either keep
with the node reduction stated (S005/S006/S015 precedent) or delete (Weiss
#513, Ethereal 69850324 and SF #3278 are the published deletions at ~0) --
owner's call, recorded as a decision either way; clearly negative -- delete,
verdict recorded.

### 7. Interactions

- **S108 (input)**: supplies the top-of-node static this reads; raw static
  per Lynx #1971/#1974.
- **S112 (before, order load-bearing)**: S112 rebuilds the quiescence this
  rule drops into, so the verdict here prices razoring against the post-S112
  qsearch -- the configuration the removal records were taken in. Landing
  S116 first would measure a different feature.
- **S130 (before)**: the razor's quiescence call inherits the TT-tightened
  stand-pat -- cheaper verification, same answer.
- **S109 (before)**: different mechanism, section 5; no shared lines.
- - **S113/S114 (just before, same 50 lines of negamax)**: both rewrite the
  `src/search.cpp` `negamax` neighbourhood first by plan order; this block
  lands between RFP and null move afterwards -- rebase onto their shapes, cite
  symbols.
- **S127 (if kept)**: RAZOR_MARGIN and RAZOR_DEPTH join the SPSA set; the
  deferred variants are priced there, not here -- Lynx #2039's
  skip-the-qsearch-when-a-TT-eval-certifies (+3.97 MTC) and any multi-depth
  margin.

### 8. References

- https://www.chessprogramming.org/Razoring -- definition, drop-to-quiescence
  variant, "~three pawns", Heinz limited razoring, the SF-2022 snapshot.
- https://github.com/official-stockfish/Stockfish/pull/3921 -- the 2022
  reintroduction; verified-form sentence quoted; gainer runs, 339248 games.
- -
  https://api.github.com/search/issues?q=repo:official-stockfish/Stockfish+razoring+type:pr
  -- #3278 removal 2020 (128816 games, ~0); #4147 PV; #4196 depth condition;
  #5120; #7044.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+razoring
  -- 8cd5cb9 verification cuts >40%; 5d57bb4 depth-1 2018; d457594; 060eef4;
  e817a55; the 2024-2026 simplification stream.
- https://github.com/official-stockfish/Stockfish/pull/2401 -- razoring is
  **not** in the 2019 removal-test list (futility ~49, shallow pruning ~204).
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+razoring --
  a768ed0 2016 add; 01c41ff3 "approx 3pawns"; 09245e09 depth-1 +9.22/+9.62;
  69850324 removal 2020 at ~0.
- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+razoring --
  #102 add 2019; #496 margin +3.30/+0.97; #513 removal 2021, quoted.
- https://api.github.com/search/issues?q=repo:jhonnold/berserk+razoring --
  #339 add 2022 +6.02/+7.17; #399 tweak +3.76/+2.87; #581 node-count ~0.
- https://api.github.com/repos/lynx-chess/Lynx/pulls/429 -- the add, all four
  runs quoted, merged at max depth 3.
- https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+razoring+type:pr
  -- #1607 widening -4.7; #1541 TT-move gate -4.71; #2180 before-RFP -1.06;
  #1971/#1974 TT-score-as-eval; #2039; #2190.
- https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+razoring --
  6649846 add 2020; #19 depth-1-only +8.05 over 7424 (2021); #231 trimmed
  form +4.62/+2.47 (2026).

## Stopped, 2026-10-01: the specified form finds 9 of the 26 mates in two an iteration late

Written by a fresh Opus agent in the linked worktree `s116` at `f82e5a3`,
briefed from `.tuning/coord/S116_brief.md`, on the idle machine. Nothing was
committed. Logs are under the worktree's `.tuning/coord/S116/`. Implemented
from this file's description (DEC-221); no other engine's code was opened.

**Stopped under DEC-233's stop clause.** The form section 2 specifies loses
forced mates in two hard mate guards that no script re-derives. No test was
edited. The step waits for a ruling.

### What is in the worktree (uncommitted)

- `src/search.cpp` `negamax_at`: the razoring block between reverse futility
  and the null move. Guards: `!is_pv && !is_in_check && excluded_move == 0 &&
  depth <= RAZOR_DEPTH`, alpha outside the mate band, and `static_eval +
  RAZOR_MARGIN <= alpha`. Then `assert(static_eval != TT_EVAL_NONE)`,
  quiescence on `(alpha, beta)`, and its score is returned only if it is at or
  below alpha (fail-soft); otherwise the node falls through.
- `src/search_params.hpp`: `RazorMargin` 282 (0..2000), `RazorDepth` 1 (0..8).
- `src/data_structures.hpp`: `search_node_probe_t` gains `razor_tried`,
  `razor_cutoff` and `razor_score`. `touches:` does not list this file yet.

### Deviations from the brief, first

1. **`RazorMargin` is 282, not 300.** CPW's "~three pawns" is read in
   chesso's own material scale, where a pawn is 94. DEC-134 says "units are
   not exempt by name", and `QsFutilityMargin` (188, from "around 200
   centipawns") and `FutBase` were seeded the same way. The coordinator may
   overrule to 300. Every figure below is at 282.
2. **`excluded_move == 0` is a fifth guard.** At the shipped `SeMinDepth` 10
   the verification search runs at depth 4 or deeper, so this guard is inert
   in the shipped build. At `SeMinDepth` 4, inside the declared range, a
   verification runs at depth 1. There quiescence would search the excluded
   move if it is a capture, and a fail-low would answer the verification. RFP,
   the null move and ProbCut carry the same guard.

### The finding: what goes red, and why

These are the fast-suite reds in the Release build at the seeds
(`fast_release_1.log`). `test_search_params` is also red, at
`golden_defaults` 73 against 71. That one is expected and was not touched.

| test | assertion | kind |
|---|---|---|
| `test_engine` mate safety, "every mate in two is found on time" | `first_exact == minimum`, shift2_flip `2brbr2/1p1p1p2/1P1P1P2/8/2Q5/5K1k/8/8 w - - 0 1` first at iteration 4, not 3 | **hard invariant, no golden** |
| `test_search` "pruning does not hide a mate against the material leader" | `result.mate_found` at depth 3 | **hard, depths 3 to 6 fixed** |
| `test_search` "pruning does not hide a forced mate", capture-mate row 1 | `3krb1r/Np2pppp/3q1n2/8/Q4Bb1/2P3P1/P3NPBP/3RR1K1 w - - 3 18` depth 7, no mate | mined golden (`S230_mine_r01_row.py depths`) |
| `test_engine` "a narrowed window still finds a mate that appears mid-search" | first case's `first_mate_depth` 9 -> 10 (`S188_repair_goldens.py first-mate`: `d9:cp1135 d10:mate5`) | golden (DEC-142) |

**How many mates in two are late.** Each of the 26 roots was run in a fresh
process at `go depth 3` (`mate2_count.py`; a REQUIRE stops the case at its
first failure). The parent reports all 26 at iteration 3. The specified form
reports **17 of 26**. The 9 late: shift2_flip, shift0_flip, shift2_black,
shift0_flip_black, knight0_flip_black (both), shift0_black, shift2_black
(second) and shift0.

**Why this is the rule itself and not a defect.** The S145 set and the
material-leader case are built so that the mating side is behind on material.
In a mate in two searched at depth 3, the mating move is played at a ply-2
node with depth 1. When that node is off the PV, the rule fires there. That
node is far below alpha statically, and the mating move is quiet. Quiescence
cannot see a quiet move, so it confirms the fail-low and the mate is lost for
that iteration. The verification arm cannot save a quiet mate. Only the depth
gate, a ply gate or the mate-band guard can. Section 5 named this blind spot
("bounds the blindness to one lost ply"). The suite asserts that zero plies
are lost, which is the situation that refused S115's root fail-high reduction (DEC-245).

### Measured as information: the same rule with reverse futility's ply floor

The variant adds one guard, `static_cast<int>(ply) >= RFP_MIN_PLY`, which is
RFP's own guard: "the top of the tree decides the move". It is not in the
tree; it is `variant_plyfloor.diff`, with its binary `chesso_var_plyfloor`.

- **The mates in two: 26 of 26 at iteration 3**, and the material-leader case
  is green.
- Two reds remain, both goldens with scripts: capture-mate row 1, and the
  aspiration case's `first_mate_depth` 9 -> 10. The first-mate script's
  reading is the same as under the specified form
  (`variant_plyfloor_firstmate.txt`).
- Nothing else is red. `test_mate_breadth` (floor 145), `test_mate_carry` and
  `test_mate_pv` are green on both forms (`variant_plyfloor_fast_full.log`).
- The floor adds a condition the step file does not have, so it is the
  coordinator's ruling and not this agent's.

### The figures taken before the stop

**The off value (DEC-215), proved.** At `RazorDepth` 0, the tune build matches
a Release build of `f82e5a3` (made in a throwaway worktree, since deleted;
binary `chesso_f82e5a3`, sha256 `14b6f2b1...`):

- `bench` **3513310**, with all eight replies (c3d5 e2a6 d7c8q g7h8q d8e7 a1b2
  e5e6 e5e6).
- The whole 121-line stream is identical with time and nps stripped. The only
  difference is the final summary line's nps.
- `bench 12` 1619863, its 105-line stream identical the same way.
- `search_bench`: 34236 / 71552 / 25351 at depth 9 and 70283 / 240680 / 80264
  at depth 12, with best moves c3d5 e2a6 d7c8q.

The tune build at `RazorDepth` 1 prints the Release candidate's
`search_bench` counts.

**Counts, parent -> specified form -> ply-floor variant:**

| instrument | parent | specified (282, no floor) | variant (+ ply floor) |
|---|---|---|---|
| `bench` | 3513310 | 3862183 (+9.93 %) | 4081329 (+16.17 %) |
| `bench` replies | c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 | c3d5 **d5e6** d7c8q g7h8q **d8d6** a1b2 e5e6 e5e6 | same as specified |
| `bench 12` | 1619863 | 1858649, replies c3d5 d5e6 d7c8q g7h8q d8d7 a1b2 e5e6 e5e6 | 1860699, same replies |
| `search_bench` 9 | 34236 / 71552 / 25351 | 32633 / 69957 / 24673, same moves | 32932 / 70095 / 25178, same moves |
| `search_bench` 12 | 70283 / 240680 / 80264 | 67493 / 280735 / 137712, kiwipete e2a6 -> d5e6 | 67792 / 280873 / 137893, kiwipete d5e6 |
| fixed-node depth, 1M nodes, Hash 16 | 17 / 15 / 16 = 48 | 17 / 14 / 15 = **46** | 17 / 14 / 16 = **47** |

The fixed-node depths are reach, not a forecast (DEC-239).

**Fire rate (section 3).** A throwaway census build counted at the block, one
process per `search_bench` position at depth 12 (`census.py`,
`census_d12.txt`). "Eligible" means every guard except the margin held.

| | eligible | tried | cut | fell through |
|---|---|---|---|---|
| specified form | 29629 | 4102 (13.8 % of eligible, 0.84 % of nodes) | 3414 (83.2 %) | 688 (16.8 %) |
| ply-floor variant | 28924 | 3904 | 3250 (83.2 %) | 654 (16.8 %) |

At depth 9 (specified form): 1132 tried, 80.9 % cut, 19.1 % fell through.
At `RazorDepth` 0 the block is unreachable, since no node reaches it below
depth 1. The rule fires. The verification refuses about one candidate in six.
The record's figure was over 40 %; that is direction only (DEC-019).

### Not taken, because the step stopped

- The mate case built the S033 way, and the guard cases and mutants
  (`tools/mutants/S116_razoring.py`).
- `golden_defaults`, `MANUAL.md` and `DEV_MANUAL.md`.
- The mined-golden re-derivations, the S156 and S154 floor sweeps, and
  `tools/mutation_check.py`.
- The S170 budgets sweep, the tune fast suite, format and the prose checks.
- `adocs/data/S116_sprt.sh`.

All of these depend on which form lands.

### Bounds, for the pre-registration whichever form lands

Proposed: **`{0, 5}`**, `fastchess.sh`'s default and the gainer pair. Section
6 says `{-5, 5}`. DEC-063 gives `{-5, 5}` for a change expected to be small,
but its own reading of an H1 there is "not a regression of 5 Elo or more",
which is what a cheap node-count saving needs. Razoring is an addition, not a
saving: the specified form costs +9.9 % `bench` nodes and 2 fixed-node plies.

Every add since the S105 regime has run `{0, 5}`. S114 v2 and S115 were both
"small, of either sign" and the coordinator ruled the gainer pair because
"an add ships only on evidence of gain". DEC-143 prices the pair at 41861
expected games at the midpoint and 25591 on a bound: 19.8 h and 12.1 h at
2110 games/h, with the midpoint past the 40000-game cap.

The section 6 decision rule, quoted for the owner: "a capped run at ~0 --
record zero, then either keep with the node reduction stated
(S005/S006/S015 precedent) or delete ... -- owner's call, recorded as a
decision either way". At these counts there is no node reduction to state.

## The ruling, DEC-248 (the owner, 2026-10-02)

Razoring lands **with `ply >= RfpMinPly`** as a further condition, the
variant measured above. The two scripted goldens are re-derived by their own
scripts, old and new rows quoted at the sites. The SPRT pair is `{0, 5}`
nElo. A no-verdict, or an interval reaching above zero, reads as a zero and
the rule's code leaves (the S238 pattern). That reading is written in the
pre-registration before the first game, and it answers section 6's "owner's
call". Both deviations stand: `RazorMargin` 282 (DEC-134, chesso's pawn 94)
and the `excluded_move == 0` guard.

## As built, 2026-10-02: the verified form with the ply floor (DEC-248)

Written by the same agent, in the same worktree, on the idle machine, after
the ruling. Nothing was committed. Logs are under the worktree's
`.tuning/coord/S116/`; this section's own are in `final/`. Implemented from
this file's description (DEC-221); no other engine's code was opened.

### Deviations and findings, first

1. **`RazorMargin` is 282 and not 300**: three pawns in chesso's scale
   (DEC-134). DEC-248 accepted it.
2. **`excluded_move == 0` is a fifth guard.** It is inert at the shipped
   `SeMinDepth` and live inside its range. DEC-248 accepted it.
3. **`ply >= RfpMinPly` is a sixth guard (DEC-248).** The form section 2
   specifies refused itself on the mate suite (the "Stopped" section).
4. **Found: `RfpMinPly` at 2, inside its declared range, is no longer safe.**
   Razoring reads that floor. At 2 the tune build finds 17 of the 26 S145
   mates in two at iteration 3, and S154's `floor` sweep reads `m2 on-time 17`
   there, so "every mate in two is found on time" goes red at 2 through
   razoring. The measured floor of 2 was reverse futility's alone. Recorded at
   `RfpMinPly`'s comment and in its `MANUAL.md` row. The range is not moved:
   whether it goes to 3, or razoring takes a floor of its own, is the
   coordinator's call. It is unreachable in ordinary play, so DEC-171 makes it
   a filler. It is item 20 of the pre-registration's findings.
5. **The S033 mate case is node-level, in the guard suite**, not a
   `search_fen` row beside "pruning does not hide a forced mate". At a
   zero-window node the verification changes the returned value only where a
   mate is searched first: quiescence's capture fails the node high either
   way. So the case plants the mating move as the node's table move, the way
   an earlier iteration leaves it. A root-level separator was also mined: the
   S097/S095/S145 pool, 402 positions, swept at depths 3 to 12 on the shipped
   library and on the unverified-drop library. **1 of 402** separates,
   `8/5p1k/3R2p1/pP6/r1q5/4K3/8/8 b - - 1 57`: shipped mate in 5 at 8 to 10,
   the unverified drop losing it at 9 only. There the mating side is ahead, so
   it is not the class the step file names, and it was not taken
   (`mine/shipped.txt`, `mine/demo.txt`, `mine/separators.fen`).
6. **The S170 budgets patch is red at the rule's answer**, as at S114 v2: C's
   new cell reports 8 lines, 5 short, over its ceiling of 0. The patch is
   prepared and held (below).
7. **`test_mate_breadth`'s shipping end moved 146 -> 149** with the floor
   still separating. The comment's table gained an S116 paragraph; the floor
   stays at 145.
8. **Found, not this step's: mutant M06a survives**, on this tree and on the
   parent `f82e5a3` alike (the Mutation section below).

### The off value (DEC-215), re-taken on the final form

The tune build at `RazorDepth` 0 is `f82e5a3`'s engine node for node, against
the throwaway Release build `chesso_f82e5a3` (sha256 `14b6f2b1...`;
`final/bench_tune_d0.*`, `final/sb*_tune_d0.txt`):

| instrument | result |
|---|---|
| `bench` | **3513310**, all eight replies c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6. The 121-line stream is identical with time and nps stripped, apart from the summary line's nps |
| `bench 12` | 1619863, its 105-line stream identical the same way |
| `search_bench` | 34236 / 71552 / 25351 at depth 9 and 70283 / 240680 / 80264 at depth 12, best c3d5 e2a6 d7c8q |

At its defaults the tune build prints the Release candidate's bench stream
line for line.

### The counts, parent -> candidate (`final/`)

| instrument | parent (`f82e5a3`) | candidate |
|---|---|---|
| `bench` | 3513310 | **4081329** (+16.17 %) |
| `bench` replies | c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 | c3d5 **d5e6** d7c8q g7h8q **d8d6** a1b2 e5e6 e5e6 |
| `bench 12` | 1619863 | 1860699 (+14.87 %), replies c3d5 d5e6 d7c8q g7h8q d8d7 a1b2 e5e6 e5e6 |
| `search_bench` 9 | 34236 / 71552 / 25351 | 32932 / 70095 / 25178, same moves |
| `search_bench` 12 | 70283 / 240680 / 80264 | 67792 / 280873 / 137893, kiwipete **d5e6** |
| fixed-node depth, 1M nodes, Hash 16 | 17 / 15 / 16 = 48 | 17 / 14 / 16 = **47**, kiwipete d5e6 |

The fixed-node depth is reach, not a forecast (DEC-239). The Release binary
is byte for byte the `chesso_var_plyfloor` measured before the ruling
(sha256 `5a0b815f...`).

### The fire rate (section 3), the final form

A throwaway census build ran one process per `search_bench` position at
depth 12 (`census_d12_var.txt`):

- 28924 nodes met every guard but the margin.
- **3904 asked quiescence**: 13.5 % of those, 0.80 % of all 486558 nodes.
- **3250 cut (83.2 %)** and **654 fell through (16.8 %)**.

The rule fires, and the verification refuses about one in six. The record's
">40 %" is another engine's and direction only (DEC-019).

### The mate case (DEC-141 clause 2), and which guard reddens it

**"razoring does not hide a quiet mate the verification lifts"**, in the
guard suite:

- **The position**: `6k1/5ppp/8/7q/6P1/6rr/5P1P/R4K2 w - - 0 1`, White to
  move, a queen and two rooks down, the queen en prise to gxh5.
- **python-chess**: `is_valid() True`, `is_check() False`, 19 legal moves,
  captures gxh5, hxg3 and fxg3, and exactly one mate in one, Ra8#, which
  takes nothing.
- **Stockfish** at depth 20 through python-chess: `#+1`, pv Ra8#
  (`mate_case_stockfish.txt`).
- **The drive**: the node at depth 1, ply `RfpMinPly`, off the PV, alpha at
  the static score plus the margin, Ra8 planted as its table move a ply
  shallower.
- **Shipped**: the rule is asked; quiescence answers above alpha and below
  the mate band; the node falls through, searches Ra8 first and returns a
  mate score.
- **Observed red under RZ01**, the verification arm removed (applied by hand,
  `rz01_mate_case_red.log`): `REQUIRE( last_score > MATE_MIN_LOCAL )`,
  `values: REQUIRE( -1182 > 48000 )`.
- **Which guard reddens it: the verification arm only.** The node meets every
  other guard.

**Removing the ply guard (RZ07, applied by hand, `rz07_red.log`) reddens both
cases DEC-248 named**:

- "pruning does not hide a mate against the material leader":
  `REQUIRE( result.mate_found )`, depth 3.
- test_engine's "every mate in two is found on time":
  `REQUIRE( first_exact == minimum )`, `4 == 3`, shift2_flip.
- The direct case "razoring does not run near the root" is red as well.

### The direct guard cases and their mutants

The cases are in `tests/test_search.cpp`'s "search: pruning and reduction
guards" suite, fixture `razor_drive_t`. The mutants are
`tools/mutants/S116_razoring.py`, RZ01 to RZ11.

| case | mutant(s) |
|---|---|
| razoring drops a hopeless node to quiescence and returns its score (at the boundary, and one point under it) | RZ09 margin strict, RZ10 margin dropped, RZ11 fail-hard return |
| razoring falls through where quiescence lifts the node above alpha | RZ01 verification dropped |
| razoring does not hide a quiet mate the verification lifts | RZ01 |
| razoring never runs in check (`TT_EVAL_NONE + RazorMargin <= alpha` asserted, so non-vacuous) | RZ02 |
| razoring never runs at a PV node | RZ03 |
| razoring does not run with alpha in the mate band (positive edge) | RZ04; RZ05, the negative edge, declared equivalent and argued in the file |
| razoring does not run above its depth (control at the gate) | RZ06 |
| razoring does not run near the root (control at the floor) | RZ07 |
| razoring does not run while a move is excluded | RZ08 |
| razoring does not run at its off value (tune build only) | none of its own |

### The goldens moved (DEC-142, DEC-233, DEC-248)

**The capture-mate table.** Shipped plus all six S091 mutants were swept at
depths 3 to 12 over `adocs/data/S230_table_fens.txt` by
`adocs/data/S230_mine_r01_row.py depths`, on a throwaway copy of the
candidate's `src/` (`capmates.sh`, `capmates/`), and the rule was applied.

- **Shipped profiles**: `d9 d10 d11 d12`, `d8 d9 d10 d11 d12`,
  `d10 d11 d12`, `d10 d12`.
- **Rows before -> after**:
  - Row 1: `{.., 7, 5, "C02, C05, R02, since S112"}` -> `{.., 9, 5, "no S091 mutant, since S116"}`. Every mutant reads `d9 d10 d11 d12`.
  - Row 2: `{.., 7, 5, "no S091 mutant, since S095"}` -> `{.., 8, 5, same label}`.
  - Row 3: `{.., 10, 4, "C02, R02, since S248"}` -> `{.., 10, 4, "C02, C05, R02, since S116"}`.
  - Row 4: `{.., 10, 5, "R02, since S248"}` -> `{.., 10, 5, "no S091 mutant, since S116"}`.
- **No mate distance moved.** C02, C05 and R02 keep a kill in row 3. C06,
  C07 and R01 are separated by no row, as before.
- Quoted in the GOLDEN block and in `DEV_MANUAL.md`'s golden row.

**The aspiration case's `first_mate_depth`.** The first position's moved
`{.., 5, 9}` -> `{.., 5, 10}` (`adocs/data/S188_repair_goldens.py
first-mate`: `d9:cp1135 d10:mate5`); the second stays at 9
(`final/firstmate.txt`). The old row is quoted at the site and in
`DEV_MANUAL.md`.

**`golden_defaults`**: `RazorMargin` 282 (0..2000) and `RazorDepth` 1 (0..8)
are added, and the count goes 71 -> 73, with the history comment saying why.

### The floors (S156, S154)

**`adocs/data/S156_mined_floor_sweep.py --ref 65630a4`**, a `git stash
create` commit carrying the tree (`final/s156_sweep.log`):

- 149 exact at `RfpMinPly` 3, 145 at 2, 143 at 1 and 0.
- The gate built at 1 is **red**.
- **The floor 145 separates**, with a gap of 6.

**`adocs/data/S154_floor_margin_sweep.py floor --ref 65630a4`**
(`final/s154_floor.log`):

- Mates in three: 12 of 24 at 3 and at 2, 7 at 1 and 0.
- **`MATE_IN_THREE_FLOOR` 11 separates.**
- Mates in two on time: 26 at 3, **17 at 2** (finding 4), 12 at 1 and 0.

Both floors still separate.

### Mutation (DEC-141 clause 2)

`tools/mutation_check.py --only` was run over the 28 mutants whose anchor
sits between reverse futility's comment and ProbCut's: RZ01 to RZ11, E10,
E11, N01 to N06, H03, G02, M01 to M05, M06a and M06b. The fixture was a fresh
throwaway commit on no branch, `c7de2c8`, made through a temporary index, in
`.ref-builds/mut` (deleted after). Header: `baseline green, 41 tests, bench
4081329`. **All 171 anchors validated.**

**Mutation score 26 of 27 (96 %)**: 26 killed, RZ05 equivalent as declared,
1 survived; wall 2996 s (`mutation/mutation.log`,
`mutation/logs/results.tsv`). Every RZ mutant except RZ05 is killed: RZ01 to
RZ04 and RZ06 to RZ11, each by its named case.

**The survivor is not this step's: `M06a_rfp_ply_floor_minus1`** (reverse
futility's floor one ply lower, i.e. at 2). It **survives on the parent
`f82e5a3` too**: a fixture at `f82e5a3` with the S116 file left out of the
list read M06a survived and M06b killed (`mutation/parent_M06.log`). So this
is a standing finding, not one razoring caused. The S196 full pass had M06a
killed by `test_search` and `test_engine`; since then the cases that killed
it have stopped separating RFP at ply 2. It is a filler for the coordinator
to schedule (DEC-171): a test property that reaches no play.

### The S170 budgets (DEC-156 as amended by DEC-162): a separate patch, red as the rule gives it

`adocs/data/S203_case_sweep.sh` ran with no argument on a copy of the
candidate's Release build (`s170/chesso`, sha256 `5a0b815f...`): the full
grid, 14 minutes. **The rule was stated before the grid was read**, in
`s170/run_sweep.sh`'s header: each row takes the cheapest cell at its own
stride that reports a mate line, chosen on the mate count alone. It was
applied by `s170/pick.py`.

| case | stride | old (S115's) | old cell now | new | its cell (mates / short) |
|---|---|---|---|---|---|
| A | 1 | 500000 | 4 / 0 | 500000 | 4 / 0 |
| B | 1 | 100000 | 8 / 0 | 100000 | 8 / 0 |
| C | 1 | 1000000 | **0** / 0 | **1500000** | **8 / 5, over ceiling 0** |
| D | 1 | 3000000 | 3 / 0 | **2000000** | 2 / 0 |
| E | 1 | 300000 | **0** / 0 | **100000** | 1 / 0 |
| F | 2 | 500000 | 14 / 0 | **100000** | 8 / 1 |

- **At the standing budgets the landing is green**, majority 3 of 5 (A, B, D).
- `--at` on the patched TSV reproduces the six cells (`s170/at_new.txt`).
- At the rule's answer, `test_mate_carry` is red in both builds at `CHECK(
  short_lines <= short_line_ceiling(game.name) )`, `5 <= 0`, C
  (`s170/carry_build*.log`).
- `--ceilings` over the four recorded grids gives 5, 15, 0, 2, 11, 5. With
  this grid added it gives 6, 15, 5, 2, 11, 5; this grid alone gives 6, 5,
  5, 0, 0, 1.
- **The patch** is `/home/max/ws/chesso/.tuning/coord/S116/s170_budgets.patch`
  (a copy is at `s170/s170_budgets.patch`). It holds the TSV at the rule's
  answer with a paragraph quoting S115's rows and saying it is red; the grid
  as `adocs/data/S116_sweep_s170.txt`; its README row; and `DEV_MANUAL.md`'s
  re-sweep sentence. It applies on the worktree (`git apply --check`).
- **No ceiling was raised; that is a decision.** The TSV in the worktree is
  unchanged.

### Suites and checks

- **Both fast suites green on the final tree**: 41 of 41 in Release `build`
  and 41 of 41 in tune `build-tune` (`final/fast_release_final.log`,
  `final/fast_tune_final.log`); `bench` is 4081329 in both builds.
- `./clang-format.sh --check` is clean with `CLANG_FORMAT_MAJOR=22`.
- `tools/plan_prose_check.py`, one mode per call: `--citations` 0 flagged,
  `--touches` 0 flagged, `--params` exit 0 (`final/prose--*.log`).
- **Debug**: the guard suite passes 91 of 91 (50436 assertions), and the whole
  `test_search` binary passes 191 of 191 (809768 assertions)
  (`final/debug_guards.log`, `final/debug_test_search_whole.log`). A first
  whole-binary run from the wrong directory failed 3 cases on a relative
  asset path and was re-run from `build-debug/tests`.
- **Every mate case is green**: "pruning does not hide a forced mate", "...
  against the material leader", `test_mate_carry`, `test_mate_breadth`
  (149 against 145), `test_mate_pv` and `test_engine`'s mate safety, with
  26 of 26 mates in two at iteration 3.

### Not run here

- DEC-141's second tier: the Debug self-play (a match) and
  `tools/gate_extra.sh`, both excluded by the brief.
- The SPRT.
- The full mutation pass; only the 28 above were run, plus M06a and M06b on
  the parent.
- Any timing.

### The pre-registration

`adocs/data/S116_sprt.sh`, in the form of `S114_v2_sprt.sh`, with its README
row:

- `{0, 5}` nElo (DEC-248) at 8+0.08, Hash 16, `noob_3moves.epd`, 12 of 12
  cores, OUT under `.tuning/`.
- Worst-case games 41861 and 25591, 19.8 h and 12.1 h at 2110 games/h.
- The abort rule.
- `REF` and `CAND` are `PIN_ME`, and the script refuses until they are
  pinned ("CAND = the landing commit's sha, REF = CAND^").
- The three readings, written before a game:
  - H1 keeps the rule.
  - H0 removes it on the S238 pattern, listing every piece that leaves; the
    goldens go back byte for byte.
  - No verdict, or an interval reaching above zero, is read as a zero with
    the same removal. Section 6's "owner's call" is said to be answered by
    DEC-248.
- The open findings: no filler is open; item 20 is finding 4 and item 21 is finding 8.

### Proposed `adocs/specs.md` sentence, for the coordinator

In the search row's pruning sentence, after reverse futility: **"Since S116,
razoring: at a non-PV node out of check, outside S097's verification, at
most `RazorDepth` (1) plies from the leaves and at least `RfpMinPly` from the
root (DEC-248), with alpha outside the mate band, a raw static evaluation
`RazorMargin` (282) or more below alpha sends the node to quiescence on its
own window, and that score ends the node only if it is at or below alpha
(fail-soft); otherwise the node is searched. `RazorDepth` 0 is the tree
before it, node for node (DEC-215); `bench` 3513310 -> 4081329; its `{0, 5}`
nElo SPRT (`adocs/data/S116_sprt.sh`) is not yet read."**

### Proposed commit (not verdict-closing)

```
Razor depth-one nodes far below alpha, verified (S116)

A node off the PV, out of check, outside S097's verification, one ply
from the leaves and at least RfpMinPly from the root, whose raw static
score stands RazorMargin (282) or more below an alpha outside the mate
band drops to quiescence on its own window; the score ends the node
only if it is itself at or below alpha, else the node is searched.
The ply floor is DEC-248's: without it 9 of the 26 S145 mates in two
were found an iteration late. Implemented from the step file's
description (DEC-221); RazorMargin is the wiki's "~three pawns" in
chesso's scale (DEC-134), RazorDepth 1 the depth-one form.

RazorDepth 0 is the parent node for node (DEC-215): the tune build
there prints f82e5a3's bench 3513310 with all eight replies and the
whole stream, bench 12 1619863, and search_bench at depths 9 and 12.
The rule: bench 3513310 -> 4081329 (+16.17 %); fixed-node depths 48
-> 47 and 3904 quiescence drops over the search_bench positions at
depth 12, 83 % cut, stated as reach (DEC-239). The counts move, so
adocs/data/S116_sprt.sh measures it at {0, 5} nElo against this
commit's parent.

Ten direct cases and RZ01 to RZ11 in tools/mutants/S116_razoring.py,
one per guard; the S033 mate case -- a quiet mate the verification
lifts -- is red with the verification arm removed. Two scripted
goldens re-derived (DEC-142): the capture-mate table to 9, 8, 10, 10
with three labels moved, and the aspiration case's first mate 9 ->
10; golden_defaults 71 -> 73. Both mate floors still separate.
Found: RfpMinPly 2, inside its range, now reddens the mate in two
through razoring; recorded, the range not moved.

Bench: 4081329
```

## Landed and pinned (the coordinator, 2026-10-02)

Landed as `65e29ba` on `f82e5a3` (`bench` 3513310 -> 4081329), after a cold fast
check that read LAND: its text items fixed before the landing -- the
`RfpMinPly` comment's "17 of the 26 ... against 26 at 3" made plain, the
razoring guard list's "ply < 3" renamed to the parameter it reads, `MANUAL.md`'s
`RfpMinPly` row no longer stating the floor 2 as safe, and the pre-registration
naming items 20 and 21 as fillers S251 and S252 and the budgets as DEC-249's.
The fast check also named **capture-mate row 2** (7 -> 8) beside row 1, which
DEC-248 did not name; it is quoted at the site and in the landing commit.
`specs.md`'s search row takes the razoring sentence and the routing clause
says razoring reads the raw static evaluation; whether its margin test joins
`RfpTtEstimate`'s estimate (S234's "second site") is left a later question,
not this verdict's. **Second tier** (DEC-141) on `65e29ba`: Debug self-play 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`;
`gate_extra` 5 stages green in 1040 s (`.tuning/gate_extra_2026-10-02_S116.log`). The S170 budgets patch
(`.tuning/coord/S116/s170_budgets.patch`) is held until the verdict
(DEC-249). Pinned `65e29ba` against `f82e5a3` in `adocs/data/S116_sprt.sh`.

## The verdict (2026-10-03, the coordinator)

**H1, 2026-10-03 00:36: `65e29ba` against `f82e5a3`, `Elo 8.36 +/- 5.62`,
`nElo 11.16 +/- 7.50`, LLR 2.95, 8234 games in 3 h 55 m, 0 forfeits**
(`adocs/data/S116_sprt.log`, `adocs/data/S116_sprt_pairs.txt`; marker
`SPRT-RUN-DONE full /home/max/ws/chesso/.tuning/sprt_s116_20261002_203929`). **H1**: razoring gains at least 5 nElo at its seeds and stays (the pre-registration's first row); `RazorMargin` 282 and `RazorDepth` 1 join S127's set, a depth-scaled margin and the multi-depth form S127's to price. The stopping run's estimate is upward-biased and is not the effect size (DEC-063). The S170 budgets now owe the owner's ruling under DEC-249: the rule's answer on this tree is red at C (8 mate lines, 5 short, ceiling 0). `Incomplete mating PV` 2 for the candidate against 10 for the reference,
an observation for the pre-registration's open finding 3's class (CHESS).
No follow-up run and no second pair for this verdict (DEC-063, DEC-019).

