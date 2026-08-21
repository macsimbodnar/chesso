id:         S116
goal:       a node whose static score is hopelessly below alpha drops straight to quiescence, at depth one only
accepts:    an SPRT verdict, recorded whatever it is; the verified form -- the quiescence score is returned only if it is itself at or below alpha; never in check, never at a PV node, never when alpha is near mate; the margin and the depth bound are constants in src/search_params.hpp with ranges; a mate inside the razored depth is in the fast suite and observed red with the guard removed
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   the move-loop pruning rules, which are S109
decisions:  DEC-019, DEC-084
closes:
blocks:
paused_by:
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

- **Site: after the RFP block (:479-537), before the null-move block
  (:539-588).** Lynx #2180 measured razoring *before* RFP at -1.06 over 28174
  games -- stay after. RFP is the mirror: a static bound returned when eval is
  `RFP_MARGIN * depth` **above beta** (:536); razoring drops to quiescence
  when eval is a margin **below alpha**. Non-PV windows are null (:706), so
  the two conditions are disjoint by arithmetic -- the pair covers both tails
  of the same eval.
- **Input**: post-S108 the static eval is computed-or-read at the top of the
  node; razoring reads the **raw static**, never a TT-score-adjusted value --
  Lynx merged removing TT-score-as-eval at +6.65 (#1971) and rejected
  re-adding it for razoring alone at -3.86 (#1974). Guard `!= TT_EVAL_NONE`,
  asserted not assumed (S114's sentinel rule).
- **The exact form** (verified, per the accepts): at `!is_pv && !is_in_check
  && depth <= RAZOR_DEPTH && alpha < MATE_MIN && alpha > -MATE_MIN` (:512 is
  the guard row to mirror, alpha for beta; MATE_MIN 48000 at :17) and
  `static_eval + RAZOR_MARGIN <= alpha`: `score = quiescence(alpha, beta,
  ply, 0, game, state)` (:191 signature; :466 is the call pattern) -- **return
  score only if `score <= alpha`**, else fall through to the move loop.
  Fail-soft: return quiescence's own score, never alpha. The null window is
  its own cheapest verification; no second windowing to get wrong.

### 3. Implementation sketch

One commit, one SPRT: two constants, ~10 guarded lines between :537 and :539,
tests red-first.

- **Mate case, built the S033 way** (python-chess enumeration + Stockfish
  confirmation, DEC-023, never a judged position): the razoring side far
  behind on material at a depth-1 node yet mating with a **quiet** first
  move -- out of check quiescence generates captures only (:295-297), so the
  drop is blind to it by construction. Lands beside "pruning does not hide a
  forced mate", tests/test_search.cpp:1887 and "pruning does not hide a mate
  against the material leader", tests/test_search.cpp:1926 in the fast
  suite; observed red against the demolition build (verification arm
  removed, i.e. the unconditional drop) and the printout recorded, per the
  accepts -- record which guard's removal reddens it.
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
- **Alpha in the mate band**: a static eval provably cannot approach a mate
  score here (the LAZY_EVAL_MARGIN clamp, S033's row), so with alpha near
  +mate the condition is trivially true at every node and the whole subtree
  under a mate-scored bound drops to quiescence. The :512 band guard, on
  alpha.
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
- **Double node count, cosmetic**: negamax counts the node (:420) and the
  razor's quiescence call counts it again (:200) -- Berserk #581 cleaned this
  at ~0. The :466 leaf drop has the same property today, so search_bench
  stays internally consistent; note it, do not fix it here.

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
- **S113/S114 (just before, same 50 lines of negamax)**: both rewrite the
  :539-588 neighbourhood first by plan order; this block lands between RFP
  and null move afterwards -- rebase onto their shapes, cite symbols.
- **S127 (if kept)**: RAZOR_MARGIN and RAZOR_DEPTH join the SPSA set; the
  deferred variants are priced there, not here -- Lynx #2039's
  skip-the-qsearch-when-a-TT-eval-certifies (+3.97 MTC) and any multi-depth
  margin.

### 8. References

- https://www.chessprogramming.org/Razoring -- definition, drop-to-quiescence
  variant, "~three pawns", Heinz limited razoring, the SF-2022 snapshot.
- https://github.com/official-stockfish/Stockfish/pull/3921 -- the 2022
  reintroduction; verified-form sentence quoted; gainer runs, 339248 games.
- https://api.github.com/search/issues?q=repo:official-stockfish/Stockfish+razoring+type:pr
  -- #3278 removal 2020 (128816 games, ~0); #4147 PV; #4196 depth condition;
  #5120; #7044.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+razoring
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
