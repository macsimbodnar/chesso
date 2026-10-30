id:         S123
goal:       passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
accepts:    one SPRT verdict for the family, recorded whatever it is, after each group passes DEC-259's offline screen, and a family H0 bisected by group; the table is rank crossed with can-advance and safe-advance rather than a single rank curve; the distance to **each** king is scaled by rank, because the same distance is worth more to a pawn on the seventh; candidate passers are scored; **monotonicity is not imposed** -- a fit that returns a non-monotonic middlegame curve is reporting something and is not corrected by hand; a test asserts the passer bitboard is colour-symmetric under board mirroring, written before any fit, because two published passed-pawn bugs shipped invisible to everything except an SPRT
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_trace.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp, tools/tuner_trace.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tests/test_evaluation.cpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_eval_positions.hpp, tests/test_walked_counts.hpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, .tuning/verify_fit.py, adocs/data/S192_anchors.py, adocs/data/S121_coverage_rows.cpp, adocs/data/S123_*, DEV_MANUAL.md, adocs/data/README.md; at the paste (phase 2), each golden by its script: tests/test_search.cpp, tests/test_mate_carry.cpp, tests/test_mate_pv.cpp, tests/test_engine.cpp, tests/test_mate_breadth.cpp, adocs/data/S170_cases.tsv, MANUAL.md; the close (2026-10-30, H1), comments only under src/ and tests/: src/evaluation.hpp, src/evaluation.cpp, tests/test_evaluation.cpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_tuner_gradient.cpp; adocs/data/S123_sprt.log, adocs/data/S123_close_inv6.txt, adocs/data/S123_cells.py, adocs/data/S123_cells.txt, adocs/data/README.md, this file; src/eval_trace.hpp, DEV_MANUAL.md and MANUAL.md read and unchanged at the close
excludes:   pawn structure terms, which are S125
decisions:  DEC-071, DEC-084, DEC-259, DEC-315, DEC-326, DEC-324, DEC-221, DEC-134, DEC-090, DEC-047, DEC-142, DEC-329
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-30 07:33 -- H1 at {0, 5} nElo, 8+0.08, Hash 16, CAND 0553160 vs REF 1a231dd: Elo +49.19 +/- 15.06, nElo +66.07 +/- 19.94, LLR 2.96, 1166 games, 33 m, 0 forfeits (adocs/data/S123_sprt.log, archived as chesso_S123_sprt_2026-10-30.tar.xz); the banner as pinned (a060435), both binaries at their bench (5434393, 6016079), no crash or disconnect, quarters +60.1, +44.3, +55.2, +35.8, kernel journal clean; no warning of any kind, 0 incomplete mating PVs; so by the pre-registered H1 row the family ships whole -- A the free and safe increments per own rank 2 to 7, B both kings' distance slopes per own rank 2 to 7, C candidate passers per own rank 2 to 6, each kept by DEC-329's screen (A g +0.000070, B d +0.000109, C d +0.000018, each past its margin) -- and no bisection; the stopping Elo is not the effect size (DEC-063). Every weight fitted on S083's corpus from zero with every old constant frozen (DEC-084, no seed), monotonicity not imposed; the definitions implemented from their descriptions, CPW's Candidate Passed Pawn page and DEC-329 (2) (DEC-221); the passer symmetry test written and seen red before any fit; cost +4.96 % (adocs/data/S123_cost_reading.txt); MATE_IN_THREE_FLOOR held at 11 at zero margin (DEC-330); the comments made true, INV-6 against 0553160 (adocs/data/S123_close_inv6.txt); the cells far from their neighbours listed for S126 (adocs/data/S123_cells.txt); fast suite 50/50 in both builds, the clang pair clean. Implemented by Opus 5.5 subagents per phase, fast-checked; the runs the coordinator's; inspected and recorded by one Opus 5.5 subagent.

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

Each term -- and each part this file used to give a verdict of its own --
is first fitted with every other constant frozen and kept only if it lowers
held-out loss on S082's fixed validation set by more than the margin the
pre-registration states; a term that fails is recorded with its figures and
does not enter the match. The survivors ship under **one** `{0, 5}` nElo SPRT
for the family, with DEC-143's worst case and abort rule pre-registered. On
H0 or no verdict the family is bisected along a partition written in the
pre-registration, each part its own SPRT. Where this file names
`tools/eval_model.hpp` or `test_eval_model` as the tuner's mirror of a term,
S262's trace and its exact-reconstruction test discharge that clause: S262
landed 2026-10-12 and S276 deleted `tools/eval_model.hpp`; the trace, its
`term_layout()` line and the walk in `tests/test_walked_counts.hpp` replace
it (corrected 2026-10-30). S263 built no per-side attack set, since no term read one;
a term here that needs one builds it once per evaluation -- S123 builds none:
safe-advance reads `is_attacked()` once per free passer (DEC-329 (1)). S039 read H0 (record `ca578de`,
revert `74d0adf`): the lazy shortcut and the clamp are kept. S122's design
amendment (DEC-258) was discharged by DEC-324 (2026-10-27: the shortcut and
the clamp kept, S122 re-derives the margin), and S121 is done (H1); the
corpus is S083's, not S082's, with S082's qs view the fixed validation set
(corrected 2026-10-30).

## What is there

Five buckets by rank since S134, one weight each, own second rank to own
sixth, S083's refit (corrected 2026-10-30; this section read six until then):

    constexpr int passed_pawn_mg[5] = {9, -5, -2, 21, 38};
    constexpr int passed_pawn_eg[5] = {18, 24, 51, 75, 119};

Bucket 5, a pawn on its own seventh, was the signed sum of eight
`psqt[PAWN][8..15]` columns; its mg -17 / eg +42 were folded into those squares
and the bucket deleted (S134, DEC-090) -- its negative fit was the
unidentified split, not a chess statement. The endgame row is monotonic; the
middlegame row is not, and is a residual over the pawn table, not a valuation.

Reported for the group: **king proximity alone +22.27 +/- 9.86** at 8+0.08,
which is Stash v32's "king proximity in the passed-pawn evaluation" in
`mhouppin/stash-bot`'s `CHANGELOG.md`
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md) and is the
single largest passed-pawn entry in the surveyed record. chesso has no
king-distance term at all. The +22.3 quoted here before 2026-09-11 was that
figure rounded. A whole-feature figure this file used to quote for the group
at about 2600 carried no source through two searches and is deleted
(DEC-203): the group's size rests on Stash's +22.27 and the traced entries in
the table below.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki defines the pawn and the rank curve, and nothing else this step
builds.** CPW *Passed Pawn*
(https://www.chessprogramming.org/Passed_Pawn, fetched 2026-09-13) defines it
as "a pawn with no opponent pawns in front on the same or adjacent files" and
states that "passed pawn evaluation assigns a bonus which increases as the
pawn advances, often by dedicated piece-square tables". **Absent from the
page: candidate-passer criteria, any king-distance or proximity method, any
treatment of a blocked or safely advancing pawn, the rule of the square as an
evaluation rule, and every centipawn and Elo figure.** The related topics are
section links without mechanics. So three of the four things this step adds
have no wiki definition to depart from, and the fourth -- the rank curve --
chesso already has.

**The group's figures, traced.** Every one below is a commit message, a
pull-request body or a changelog entry.

| what | figure | source |
|---|---|---|
| king proximity in the passed-pawn evaluation (Stash v32) | **+22.27 +/- 9.86** at 8.0+0.08s | https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md |
| proximity to *both* kings for passed pawns (Ethereal, 2018-07-31) | **+6.13 +/- 4.38** at 10.0+0.1s over 10430 games; **+7.23 +/- 4.63** at 60.0+0.6s over 7450 games | commit 214ec81a50a1ac4b5948e5c54670d4e3e8324c85 |
| file distance to the closest pawn used to score kings (Ethereal, 2019-10-12) | **+8.27 +/- 5.17** at 10.0+0.1s over 6850 games; **+7.58 +/- 4.57** at 60.0+0.6s over 6650 games | commit 0f4a4e26df776aeb438b13cc15989972c5765f42 |
| candidate passed pawns (Ethereal, 2018-09-10) | **+5.03 +/- 3.82** at 10.0+0.1s over 13610 games; **+6.74 +/- 4.40** at 60.0+0.6s over 8040 games | commit 8fded6e246061ab61517849ee894b13eb0b60868 |
| candidate passers added back (Stash v30) | **+6.87 +/- 4.85** at 10.0+0.1s over 9459 games | the Stash changelog above |
| extra penalty for stacked passers (Ethereal, 2020-01-23) | **+6.95 +/- 4.69** at 12.0+0.12s over 8300 games; **+3.55 +/- 2.77** at 60.0+0.6s over 19175 games | commit 29afad99755176d7876646dafa9370c97591a09e |

The Ethereal commits are reachable through
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+passed+pawn
(fetched 2026-09-13). Two readings follow and both matter to this step.
**King distance is the largest entry in the record and it is the term chesso
does not have at all** -- Stash's +22.27 and Ethereal's two independent
proximity patches agree on the direction across three bands. **Candidate
passers are a single-digit term everywhere they are priced**, +5.03 at
Ethereal and +6.87 at Stash, so the accepts' "candidate passers are scored" is
the small half of this step and its bounds should say so.

**The whole-feature figure survived two searches unsourced and is deleted
(DEC-203).** It was quoted for "the whole feature from nothing at about 2600".
Searched 2026-09-04 by the plan review: no source. Searched this pass: the Ethereal
passed-pawn commit set above (its 2016 additions carry no Elo at all);
`mhouppin/stash-bot`'s `CHANGELOG.md`, scanned for it, where it does not
occur; and the Blunder release notes
(https://github.com/algerbrex/blunder/releases), the one surveyed engine in
that band that prices features by release -- 7.4.0 (2021-12-14) bundles
"evaluation terms for passed pawns and knight outposts, a new form of dynamic
time management, and a better tuned evaluation" at "about 50 Elo in gauntlet
testing over 7.3.0" and states no per-term figure. The group's size in the
plan already rests on Stash's +22.27, which is sourced, so the deletion
costs no ordering argument.

### 2. Shape for chesso

Today: `src/evaluation.cpp` `passed_pawn_mg` and `src/evaluation.cpp`
`passed_pawn_eg`, five rank buckets since S134, one weight each (corrected
2026-10-30: bucket 5 folded into the tables).

**Departures from the record, stated:**

- The record's king term is one distance to one king (Stash) or a proximity
  term over both (Ethereal); Ethereal's later refinement uses **file** distance
  rather than Chebyshev. This step scales the distance to **each** king **by
  rank**, which no located source does, on the argument written in the goal.
  That is a departure and it is why the step owns its own verdict.
- Rank crossed with can-advance and safe-advance is a table shape no located
  source publishes. The wiki's form is a rank curve; the cross is this
  project's.
- Monotonicity is not imposed, which is also nobody's published rule; it is
  DEC-084's consequence -- what ships is what the fit returned.

### 3. Implementation sketch

- The passer extractor already exists for the rank buckets; can-advance and
  safe-advance are two predicates over the square in front. "Safe" over enemy
  pawns alone would be true of every passer by the passer's definition, so it
  reads every enemy piece; S121 built no enemy attack set (its count drops
  enemy pawn attacks only), and DEC-329 (1) has safe read `is_attacked()` once
  per passer whose push square is empty, in stage one (corrected 2026-10-30).
- The colour-symmetry test comes **first**, before any fit: "a test asserts the
  passer bitboard is colour-symmetric under board mirroring, written before any
  fit" is in the accepts, and Ethereal shipped exactly that bug and paid a
  patch for it -- commit f8585f1b10d00a4ad5de55119b061d5683d8da6c (2020-01-26),
  "Fix a small white/black mirroring issue with Passed Stacked Pawns",
  **+2.89 +/- 3.39** at 10.0+0.1s and **+1.63 +/- 2.42** at 60.0+0.6s. A
  mirroring bug worth ~2 Elo is invisible to everything except a match, which
  is the whole argument for the test.
- `tools/eval_model.hpp` is deleted (S276): every new feature gains its
  trace call, its `term_layout()` line, its `tools/tuner_layout.hpp` block and
  a walk in `tests/test_walked_counts.hpp`; `tests/test_evaluation.cpp`
  carries the mirror case (corrected 2026-10-30).

### 4. Constants and seeds

**No seeds.** Every entry is fitted by this project's tuner on its own corpus
(DEC-084). The figures in section 1 are Elo measurements of other engines'
patches, not constants, and no engine's passed-pawn table seeds anything here
wherever it is republished (DEC-105, DEC-134) -- including the wiki's own
"dedicated piece-square tables" sentence, which describes a shape and gives no
values.

Two structural numbers are declared rather than fitted, both **(b) derived
from the board**: the buckets' index runs 0 to 4, own ranks 2 to 6 (after
S134, a rank-7 base cell is the tables'), and the family's 0 to 5, own ranks 2
to 7 for A and B (rank 7 as increments and slopes, never a base cell) and 0 to
4 for C; and king distance is Chebyshev, bounded by 7 -- file distance is too,
Manhattan reaches 14 (corrected 2026-10-30). Both come from the rules of
chess. **No seeds** holds: every family weight starts the fit at zero
(DEC-329 (3)).

### 5. Pitfalls

- **Bucket 5 is the trap S134 exists for.** This step must not re-create a
  feature that is exactly the signed sum of a single rank of the piece-square
  table. Any new term defined on one rank needs the same identity check
  `tools/feature_audit.cpp` runs.
- **A per-group verdict meant several runs**, and DEC-259 amended it: one
  family SPRT, `{0, 5}`, its H0 bisected A | B | C (DEC-329 (5)); every
  bisection part so far used `{0, 5}` (corrected 2026-10-30).
- **The mirroring bug is real and cheap to ship.** See section 3.
- **Safe-advance needs an attack set that is not free.** If it is computed
  twice -- once here and once in mobility -- that is the recomputation INV-4
  exists to stop.
- **The corpus decides what a passer bucket can learn.** Bucket 5 read
  negative because the corpus had few such positions; the new terms are
  fitted on S082's corpus and a group driven to zero is reported as a corpus
  finding (S126's rule, applied early). Fitted on S083's corpus, not S082's
  (corrected 2026-10-30).

### 6. Measurement

One family SPRT, `{0, 5}` nElo at 8+0.08, Hash 16, `noob_3moves.epd`,
`adocs/data/S123_sprt.sh`, bounds stated in advance with the nElo worst case
and the abort rule (DEC-143); its H0 bisected A | B | C (corrected
2026-10-30: this read one SPRT per group at the S105 regime). The colour-symmetry test is green
before the first fit. nps is recorded because the safe-advance predicate adds
work at every `evaluate()` call.

### 7. Interactions

- **S134 (before)**: removed bucket 5 and with it the degeneracy (done
  2026-10-12).
- **S121 (before)**: done (H1); it built no enemy attack set, so safe-advance
  reads `is_attacked()` (corrected 2026-10-30).
- **S125 (after)**: the other pawn group; both are recomputed per call until
  S118.
- **S118 (after S125)**: caches the passed-pawn bitboard this step produces --
  the accepts of S118 names that slot. A pawn-key cache can hold the
  bitboard and C, never A or B, which read pieces and kings (corrected
  2026-10-30, DEC-329's consequences).
- **S126 (block end)**: refits everything this lands.

### 8. References

- - https://www.chessprogramming.org/Passed_Pawn -- definition and the rank
  curve; candidates, king distance, blocked/safe advance, the rule of the
  square and **all figures absent**. Fetched 2026-09-13.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+passed+pawn
  -- commits 214ec81a, 0f4a4e26, 8fded6e2, 29afad99 and f8585f1b with the
  figures tabulated above. Commit messages only. Fetched 2026-09-13.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- v32 king
  proximity +22.27 +/- 9.86; v30 candidate passers +6.87 +/- 4.85. Changelog
  entries only; scanned 2026-09-13 for the whole-feature figure this file used
  to quote, which does not occur.
- - https://github.com/algerbrex/blunder/releases -- 7.4.0 (2021-12-14) bundles
  passed pawns and knight outposts at "about 50 Elo" for the release, no
  per-term figure; the band's engine that prices by release. Fetched
  2026-09-13.

## 2026-10-30: phase 1

Opus 5.5 subagent, coordinated; worktree `../chesso-s123`, branch `s123` on
`244d89d`. The family landed at zero, `No functional change`; the screen,
the cost run and the SPRT pre-registered; nothing fitted, timed or played.

### The rulings as read (DEC-329, the research pass)

- (1) Stage one, beside the buckets, in `evaluate_pawns()` under
  `evaluate_cheap()`, never under the clamp: the family is summed into the
  pawn terms' packed pair, so it shares their one taper (N stays 3) and the
  clamp, `eval_spread` and S122's margin do not move.
- (2) From the descriptions (DEC-221), every definition stated in
  `src/evaluation.hpp`: can-advance -- the push square holds no piece, the
  double push not read; safe-advance -- can-advance and the push square
  attacked by no enemy piece, the king included, x-rays not counted
  (`is_attacked()`); king distance -- Chebyshev, each king to the push square.
  **Candidate: CPW's *Candidate Passed Pawn* page exists, and its rule is
  followed** (https://www.chessprogramming.org/Candidate_Passed_Pawn, fetched
  2026-10-30): its definition is "a pawn on a half-open file, which, if the
  board had only pawns on it, would eventually become a passed pawn", and its
  computable form the approximation "no square on its path is controlled by
  more enemy pawns than own pawns", which the page itself calls too strict
  (b5 with a4 against a7 counts only after a4-a5). DEC-329's first draft had
  *Candidates (Bitboards)*'s helpers-and-sentries totals; the page counts per
  path square instead, own and enemy pawn attackers of each square ahead, and
  DEC-329 (2) as recorded is that rule. Kept beside it from DEC-329 and from *Candidates (Bitboards)*: not
  passed, and "the intersection of the frontspan with opposing pawns is
  empty" -- the "half-open file" read as clear ahead, not as no enemy pawn
  anywhere on the file. Its own diagram, kings added, is a hand case in
  `tests/test_eval_model.cpp`: b5 and g5 come out the candidates, as its
  caption says.
- (3) S135's form: the buckets stay, the family lands at zero, 58
  parameters, `PARAM_COUNT` 955 -> 1013 (built: 1013). A 24, B 24, C 10.
  Rank 7 enters A and B as increments and slopes only. No seeds.
- (4) The screen, A then B then C, each freeing its own weights and the
  earlier survivors', every old constant frozen; A by DEC-315's g, B and C by
  DEC-326 (3)'s paired d against the last survivor's fit; m = max(1.645 sd,
  2e-6); S083's corpus against S082's fixed set; ties under 1000 games a half
  to the nearest rank's same array and half, written by the audit before any
  fit; tables printed deciding nothing. One reading DEC-329 leaves open,
  pre-registered and flagged for the coordinator: a part with no survivor
  before it reads against its own start, which is then the predecessor's
  evaluation, so its d is DEC-315's g.
- (5) H0 partition A | B | C, survivors only, in order, each refit over what
  has landed (`S123_screen.sh part NAME`).
- (6), (7), (8), (9): below.

### What landed

- `src/evaluation.cpp`: ten zero arrays (`passed_free`, `passed_safe`,
  `passed_king_own`, `passed_king_enemy`, `candidate_passer`, mg and eg; the
  brief's `candidate_{mg,eg}` named `candidate_passer_{mg,eg}`, as the other
  arrays name their term), their packed copies, a liveness per group, and
  `passer_family_side()` / `candidate_passers()`. A group is compiled into the
  engine's `evaluate_cheap()` only when a weight of it is non-zero; the trace
  library and the collecting accessors compute all three. The stage-one
  overflow bound gained `passer_family_half()`.
- `src/evaluation.hpp`: the definitions, `passed_pawn_sets()` (each side's
  passers, the own seventh included) and `passer_family_counts()` per colour.
- `src/eval_trace.hpp`: five terms after `TERM_TEMPO`, so every earlier
  feature number holds.
- `tools/tuner_layout.hpp`: the blocks appended after tempo, every block with
  an end constant and `PARAM_COUNT` asserted at the last; no earlier index or
  comment pin moved. `tools/tuner_groups.hpp`: groups `passed_advance`,
  `passed_kings`, `candidate_passers`; **`tempo` ends at its own block** (it
  ran to `PARAM_COUNT`); `only_mask()`, `--only` with a comma list, the union;
  ties name the family's rank rows. `tools/tuner.cpp` emits the ten arrays and
  takes the list; `tools/feature_audit.cpp` names the columns, lists them in
  occurrence, prints their `support` lines and reads `--free` as a list.
- Tests: the two mirror cases (`test_evaluation`, "evaluation: passer
  symmetry"); the engine against the walk and by hand (`test_eval_model`,
  "eval counts: S123's passed pawn family", nine hand placements); every
  coefficient against the accessor (`test_eval_trace`); the packed halves
  (`test_evaluation`); "every group frees exactly its own block" and
  "`--only` takes a list" and the family's ties (`test_tuner_groups`); the
  walk `passer_family_features()` (`test_walked_counts.hpp`, square by square,
  no engine code). `test_tuner_groups`' `TEMPO_EG_BASE + TEMPO_COUNT ==
  PARAM_COUNT` names the new last block, as its own comment instructs.
- `.tuning/verify_fit.py` 1013 and the ten widths; `adocs/data/S192_anchors.py`
  a `family` entry on every case (hand counts on the two with pawns, checked
  against the engine's accessor and the walk by a throwaway: e2 free 1, safe
  1, own 2, enemy 5; Re8's f7 g7 h7 -3, -3, -6, -15), 10 of 10 reproduced.

### The symmetry test first (DEC-329 (6))

Written and observed before any scoring code:

- Passer sets, green: 34676 positions (all_test_fens() and 20000 seeded
  placements, seed 20261030, both sides to move, kept when the position and
  its mirror load), White a/h-file passers in 9008/8776 of them, Black's
  9281/9085, own seventh 15338 and 15293; 112063 assertions.
  **Red** on Black's attack span with its edge masks dropped:
  `mirror_sets[BLACK] == mirror_ranks(sets[WHITE])` 1792 == 1536 at
  `5bnr/4p1pq/4Qpkr/7p/7P/4P3/PPPP1PP1/RNB1KBNR b KQ - 2 10`; reverted
  (`cmp` identical), green again.
- Family counts, per array and rank, green over the same population, the
  rarest cell non-zero on 1391 positions; 2165969 assertions with the case
  above. **Red** on each planted one-colour defect, each reverted:
  states -- Black's safe asking its own colour: safe rank 5, 0 == 1 at
  `1BQKRBRN/PPPPPPPP/1N6/8/8/8/3k4/8 b - - 1 1`;
  distances -- Black's kings measured to the pawn, not the push square:
  rank 5, 17 == 16, same position;
  candidates -- Black's pawn attacks without the edge masks: rank 0, 0 == 1 at
  `8/1PK4p/6P1/P1p4p/8/2p2p1p/3kp3/8 w - - 0 1`.
- The tempo trap, a defect fixed here: with `tempo` back at
  `[TEMPO_MG_BASE, PARAM_COUNT)` "every group frees exactly its own block"
  reads red, "tempo frees 58 parameters outside its block ... first index
  955", beside the partition's overlap; reverted, green.

### INV-6

`adocs/data/S123_inv6.txt`: against `244d89d`, Release and tune builds,
`bench` 6016079, `bench 12` 2200179, `tools/search_bench.py` depth 9 c3d5
26892 / e2a6 109678 / d7c8q 35636 and depth 12 c3d5 183158 / d5e6 414751 /
d7c8q 221974, every stream byte-identical; `evaluate_cheap()` 381
instructions in both trees once the branch-padding prefixes are dropped.

### Coverage (DEC-329 (8))

- `adocs/data/S121_coverage_rows.cpp` scoped: its gradient check over the
  mobility span `[MOB_MG_BASE, KS_MG_BASE)`, **and two more scopes the brief
  did not name, needed for it to read only its own block**: its both-ways
  check over the terms before `TERM_PASSED_FREE`, and its landed block ending
  at S123's marker. On this tree with S123's rows landed: 19 pairs, 0
  ineligible, 0 flat, 0 one-way, "reproduces the landed block: yes".
- `adocs/data/S123_coverage_rows.cpp`, S121's shape and rule, base the 80
  rows: 11 of 58 needs open (safe rank 5 mg, safe rank 7 both halves,
  candidates ranks 2, 3, 4 and 6 both halves); three pairs from
  all_test_fens() and one from S083's corpus close them; the fixture grows
  80 -> 88. The constructive fallback is not needed; run once without the
  corpus it closes the same six needs from its own draws.
  All eight rows VALID by python-chess.

### The preconditions, re-derived by script (`adocs/data/S123_preconditions.txt`)

- Fixture clear of the clamp: largest |stage two| 121.1 against 183.0.
- The clamp case's push, `S083_tuner_preconditions.cpp clamp`: **+80** by its
  rule (22 rows bound, the farthest 597.3, the nearest other 178.4, every row
  3.71 from the kink, finite differences 1.97e-7); the case's +30 now reads
  2.28e-7, past the rule's 2e-7, so the case's push moves to +80 with its
  comment (an input, re-derived by its script, DEC-142).
- The gauge walk: smallest 0.0235 under Adam and 0.2272 under AdaGrad, both
  the bishop's, over the case's 0.01 floor (by 2.35 under Adam); holds at
  most 8.88e-15.
- The gradient case's worst: **6.94e-7 at parameter 869,
  `mobility_eg[rook][7]`**, against 1e-6 (9.50e-7 at 849 over the 80 rows);
  no DEC-316 rows owed. The clamp case 1.97e-7; the synthetic group 7.82e-8.
- Planted recoveries (passed_pawns, piece_placement, tempo): green. The
  family has no planted recovery case of its own, and none was added: the
  coverage rule asks for a coefficient at a phase weighting each half, not
  for two phases, and a cell the fixture reaches at one phase only cannot
  have its halves told apart by a plant. Recorded unmeasured;
  `feature_audit --plant` over a corpus slice is where it would go.

### The audit, run once at phase 1

`tools/feature_audit --free passed_advance,passed_kings,candidate_passers`
over S083's corpus (24 s). `S123_screen.sh`'s pre-registration was written
before any fit; this audit ran once after its rules were fixed: rank
exactly **966 of 1013**, nullity 47 -- the 32 dead pawn columns, S134's seven
and S121's eight levels held by the freeze -- and **0 further exact
dependencies**; no family column zero; every family rank cell's half
supported by 14025 games or more, so the pre-registered rule ties nothing.
`S123_screen.py audit-check` passes. The screen's own `audit` stage runs it
again and writes the ties.

### Gates

Both builds without a warning; both fast suites 50 of 50; `./clang-format.sh
--check` clean (`CLANG_FORMAT_MAJOR=22`); `tools/plan_prose_check.py`
`--touches`, `--params`, `--citations`, `--gate` and `--prose` each clean.
`tools/gate_extra.sh` is not run: no search, generator or make/unmake change,
so DEC-141's tier two does not bind.

### The fast check's four, fixed before completion

- `S123_screen.sh` `screen` and `all` refuse a HEAD whose ten family arrays
  are not all zero (`S123_screen.py landing-check`): the probe's start-check
  compared with HEAD only, so a run at a candidate commit would have read
  every gain against another baseline in silence. `part` is exempt. Seen
  refusing in a scratch clone on a planted `passed_king_own_eg[5] = 4`
  (`screen A` and `all` both FAILED at the check) and passing at the landing
  (`screen A` went on to its missing audit).
- `S123_cost.sh` `streams()` checks each command's status and requires both
  benches to end on their total line and `search_bench.py` to print its three
  positions.
- `S123_sprt.sh` compares every weight array and define outside the family
  between REF and CAND by name (`S123_screen.py only-family-pair`): its line
  test accepts a numbers-only line anywhere in the file. Seen refusing in a
  scratch clone on a planted mobility_bishop_mg continuation-line change
  (53 -> 54) beside a family weight, and passing a family-only paste.
- The screen's pre-registration now says it was written before any fit, the
  audit run once after the rules were fixed.

### The cost run's first attempt, 2026-10-30

`S123_cost.sh run` stopped at INV-6 before any timing: the fast check's
`streams()` kept `search_bench.py`'s raw output, seconds and knps included,
in the directory `run` compares with `diff -r`, so zero against parent read
different on timings alone, nodes and best moves identical
(`.tuning/s123_cost.console`). `streams()` now empties its directory first
and writes the raw output to `$OUT/inv6_raw/<bin>`, which nothing compares.
Shown on the run's own zero and parent binaries without a timing: `INV-6:
bench 6016079 nodes, every binary identical`, also over a stale raw file
left in the compared directory; and refused on a parent whose bench total is
planted one digit off. The `diff -r` over `$OUT/inv6` is the only directory
comparison in S123's scripts; `S123_screen.sh` compares tie files with
`cmp`, and `S123_sprt.sh` reads `git diff` over `src/` and weights by name.

### Known and unfixed

- `adocs/data/S121_cost.sh` and `adocs/data/S135_cost.sh` `streams()` have
  the gap S123's copy had: no exit status checked and no bench total
  required, so a bench or `search_bench.py` failing the same way for every
  binary writes identical error text and passes their INV-6 `diff -r`. Both
  are finished steps' records and are not edited here (filed as S299).

### Commands for the coordinator, in order

1. Commit phase 1; rebuild `build/` at the commit in the main checkout.
2. `nohup adocs/data/S123_screen.sh all > .tuning/s123_screen_all.console 2>&1 &`
   -- locate under a minute, audit about 30 s, three parts of six fits each:
   about 55 min typical, 1 h 50 m at the 5000-epoch cap; watcher ceiling
   4 h 30 m; `S123-ALL-DONE|FAILED`. Daytime.
3. `CAND_SHA=<the landing> PARENT_SHA=<its parent> adocs/data/S123_cost.sh check`,
   then `run`: about 1 h 15 m, ceiling 2 h 30 m, the machine idle;
   `S123-COST-DONE|FAILED`. Any time after step 1, not beside step 2.
4. `adocs/data/S123_screen.sh apply <a clean worktree at HEAD>`, about 8 min,
   then phase 2 (an agent): every golden the paste moves, each by its script
   (the research pass's section 5 lists them).
5. Pin `adocs/data/S123_sprt.sh` (the bracketed lines) and run it at night:
   up to 41900 games, 19.86 h; watcher ceiling 40 h; on H0, the bisection's
   `S123_screen.sh part NAME` (about 15 min each) and up to three more SPRTs.

## 2026-10-30: the screen and the candidate

One Opus 5.5 subagent, worktree `../chesso-s123b`, branch `s123b` at
`1a231dd` (the cost script's fix, `No functional change`, over `03680ab`, the
phase 1 landing). Written in two halves: the reading while the coordinator's
cost run held the machine, the runs after its "go" and its rulings on the
half 1 points. Every reading with its command is in
`adocs/data/S123_goldens.txt`. No match, SPRT, fit, datagen or timing ran.
Every golden the paste moves is re-derived by its script, every guard held,
and nothing stopped.

**The screen's launch, by the coordinator.** `S123_screen.sh all` started
2026-10-30 02:32:04 from the main checkout at `03680ab` (the landing), PID
3111735, console `.tuning/s123_screen_all.console`, OUT `.tuning/s123`;
load 0.95 at launch, 27 GB free. Open findings (BUGS as scoped by DEC-171):
the Open list at `03680ab` re-read, no DEC-171 filler is open.

### The screen, as read

`adocs/data/S123_screen.sh all`, 2026-10-30 02:32 to 03:19, `S123-ALL-DONE`,
on S083's corpus (6226944 rows, sha256 `2e0e3de9...`) against S082's qs
validation view, the three views' sha256 checked; the audit passed at
02:32:56. The three readings, the corpus line, the audit's pass line and
every fit's and tie file's sha256 are in `adocs/data/S123_screen_reading.txt`.

| part | against | full gain | s | m | verdict |
|---|---|---|---|---|---|
| A, the free and safe increments | its start (g, DEC-315) | +0.000070 | 0.000005 | 0.000009 | kept |
| B, both kings' distance slopes | A (paired d, DEC-326 (3)) | +0.000109 | 0.000008 | 0.000013 | kept |
| C, candidate passers | AB (paired d) | +0.000018 | 0.000006 | 0.000010 | kept |

- Survivors A, B, C: `CANDIDATE ABC groups
  passed_advance,passed_kings,candidate_passers`. The candidate is
  `.tuning/s123/fits/ABC/full.hpp` (sha256 `c7f340c6...`): best validation
  0.071425 from 0.071622, at epoch 500, stopped on patience.
- The bisection order printed: A | B | C.
- Ties: none. The three tie files list no tie: every family rank cell's half
  is supported by 1000 games or more.
- **The cap, as read; it decides nothing.** ABC's q3 stopped at the
  5000-epoch cap, its best at epoch 4200. Every other fit of the eighteen
  stopped on patience.
- Stage-one headroom at ABC: mg 19982 and eg 25799 of 32767 (A's fit 16112
  and 15509, AB's 19382 and 24059).
- **The full fit's tables, recorded; they decide nothing**, and monotonicity
  is not imposed (the accepts). Own rank 2 first; C stops at the sixth.

      passed_free_mg       {2, 8, 2, 7, 28, 76}
      passed_free_eg       {23, 3, 14, 13, 5, 6}
      passed_safe_mg       {-1, 1, 2, 0, 10, 33}
      passed_safe_eg       {8, 6, 10, 22, 51, 53}
      passed_king_own_mg   {2, 5, 7, 3, -2, -3}
      passed_king_own_eg   {-3, -5, -13, -16, -17, -19}
      passed_king_enemy_mg {-2, -7, -7, -5, 0, 2}
      passed_king_enemy_eg {-3, 3, 9, 15, 18, 23}
      candidate_passer_mg  {-6, -16, 4, 10, -2}
      candidate_passer_eg  {4, 8, 30, 46, 59}

  How the arrays combine, the code's arithmetic only: a free and safe passer
  collects both increments, since safe implies free (DEC-329 (2)), and a king
  slope multiplies that king's Chebyshev distance to the push square.

### The apply

`S123-APPLY-DONE` (`.tuning/s123/apply_ABC.console`, OUT
`.tuning/s123/apply_ABC/`).
- start-check passed: 35 definitions against HEAD `1a231dd`, the three groups
  free, every held value HEAD's.
- headroom: mg 19982, eg 25799 of 32767.
- `verify_fit`: 1013 of 1013 identical to the emitted header. only-family:
  the ten family arrays moved, nothing else.
- Both builds with no warning. Bench 6016079 -> **5434393**.
- 47 of 50 in each build. The reds: `test_evaluation` (the pawn on e2, 109
  against 99), `test_search` (Re8, 196 against 181) and `test_mate_carry`
  (the majority clause, 3 of 6 guarded cases reporting a mate line, A, C and
  E silent: the budgets wanted re-sweeping, as its message says).
  `test_eval_trace` and `test_tuner_gradient` were green.

### The coverage rows at the candidate

Both tools, run on the candidate's constants against S083's corpus
(`S121_COVERAGE_DATA` / `S123_COVERAGE_DATA`), as their headers say:
- `adocs/data/S121_coverage_rows.cpp`: landed block 19 pairs, 0 ineligible;
  from the base 42, 55 of 132 needs open, the same 38 picks; gradient over
  the mobility span, 0 parameters at or under 1e-12 (smallest 6.584e-09);
  both ways, 0 features; "reproduces the landed block: yes".
- `adocs/data/S123_coverage_rows.cpp`: landed block 4 pairs, 0 ineligible;
  from the base 80, 11 of 58 needs open, the same 8 picks (three from
  `all_test_fens()`, one from the corpus); gradient from `MOB_MG_BASE`, 0 at
  or under 1e-12 (smallest 5.985e-09, over the family 4.669e-07); both ways,
  0 features; "reproduces the landed block: yes".

The landed blocks still cover every parameter both ways. No row moved, so no
block was re-derived and no parameter is open.

### The preconditions, each by its script

- Fixture clear of the clamp: 121.1 against 183.0, unmoved (stage two does
  not move with a stage-one family, and no row moved).
- The clamp case's push, `S083_tuner_preconditions.cpp clamp`: **+80 -> +30**
  by its rule: two rows bound, the farthest at 197.3, the nearest other at
  158.8, every row 13.33 from the kink, finite differences 6.61e-8. +80 still
  passes the rule's checks there, but its nearest row is 3.71 from the kink
  and the rule takes the farthest. An input, re-derived by its script, with
  its comment (`tests/test_tuner_gradient.cpp`).
- The gauge walk: 0.0409 under Adam (`psqt_eg[king]`) and 0.0590 under
  AdaGrad (the bishop), over the case's 0.01 floor; held at most 9.66e-15.
- The gradient case's worst (`test_tuner_gradient -s`, both builds):
  **2.79e-7 at parameter 985, `passed_king_own_eg[0]`**, against 1e-6, green
  by a factor of 3.6 (6.94e-7 at 869 at the landing). No DEC-316 rows owed.
- Planted recoveries (passed_pawns, piece_placement, tempo): green in both
  fast suites.

### Every golden, each by its script

| golden | before | now | script |
|---|---|---|---|
| six piece anchors | 99, 283, 353, 590, 1021, 0 | **109**, 283, 353, 590, 1021, 0 | `S192_anchors.py`: 10 of 10 at the landing, then on the fit, then 10 of 10 on this tree. Only the pawn on e2 carries a passer |
| `QUIET_ROOK_EVAL`, `_CHEAP`; 394 | 590, 578; 394 | unmoved: no pawn | the same |
| `black_static`; quiet evasion; repetition | 181; -541; -603 | **196** (Re8's three passers); -541 unmoved, the four leaves pawnless and still the four king moves standing pat; -603 unmoved, no pawn | the same |
| `POSITIONAL_ROOM` | -- | did not fire: the white pawn's residual is \|109 - 84\| = 25 against 150 | -- |
| `truncation_positions` | S121's four, phases 5, 11, 19, 17 | **re-picked**: phases 5, 7, 13 and 17, S121's phase 17 row kept. 89 past 2.8, 18 at 2.875, 16 distinct. The worst is 2.875 = 69/24, so N stays 3. S121's four read 2.333, 2.625, 2.792 and 2.875 there: green, three of them off the maximum | `truncation_scan --data <S083's corpus> --min 2.8`, then `S083_truncation_pick.py` |
| coverage blocks | S121's 19 pairs, S123's 4 | reproduced, unmoved | `S121_coverage_rows.cpp`, `S123_coverage_rows.cpp` |
| clamp push; walk | +80; 0.0235 / 0.2272 | **+30**; 0.0409 / 0.0590 | `S083_tuner_preconditions.cpp both` |
| fixture clear of the clamp | 121.1 | 121.1 against 183 | the same |
| gradient check (DEC-316) | 6.94e-7 at 869 | 2.79e-7 at 985 | `test_tuner_gradient -s` |
| `EXACT_FLOOR` 159 | 165 / 154 | **held**: 160 shipping, 153 weakened; the gate at RfpMinPly 1 red at 153 | `S156_mined_floor_sweep.py --floor 159` via `S136_overlay_run.py` (the paste's one file, `src/evaluation.cpp`) |
| `MATE_IN_THREE_FLOOR` 11 | 12 / 10 | **held**: 11 shipping, 10 weakened; `red` red at 10 >= 11 and on mate-in-two exactness | `S154_floor_margin_sweep.py floor`, `red`, the same overlay |
| node budget | 65024 / 3251 (8367, 0.51 of c) | unmoved: 12104 nodes, 0.74 of c, inside [7270, 32512] | `S192_node_budget.py` |
| first-mate; `DRIVE_DEPTH`; ply-2 | 9, 9, 5; 4; 6 | unmoved | `S188_repair_goldens.py first-mate`, `s207`; `S083s_rfp_ply2.py` |
| `capture_mates`; extra ply; ProbCut row | 9, 8, 11, 12; 8 (mate 4); 10 (mate 4) | unmoved: each at its asserted depth (the fourth capture row at 8, 9 and 12); mutant separation not re-read, every case green | `S230_mine_r01_row.py depths`, candidate and parent |
| `LATE_CAPTURE_*` | LQ02 at 7, LQ03 at 3 | unmoved | `S261_mine_late_capture.py sweep`, `pick` |
| aborted-line cell | line 101 at 50000 (40000..70000) | **line 101 at 40000** (run 30000..60000; margin 10000 below, 20000 above; 3 of 318 separate). Observed red under MW02, green shipped | `S294_fallback_cell.py`, four stages, MW02 on a throwaway tree of the candidate |
| `ALPHA` | -2108 (band -2163 .. -1932) | **-2108 kept**: band -2153 .. -1923, 45 below and 185 above; the trace meets the node at -2039, -2155 and -2334 | `S121_alpha_window.cpp` (it reproduces S121's band on the parent); `S136_trace_patch.py` |
| `short_line_ceiling` | 2, 13, 4, 1, 2, 3 | unmoved; the command gains `adocs/data/S123_sweep_s170.txt` | `S203_case_sweep.sh --ceilings`: the nine grids plus S123's |
| S170 budgets | A 1000000, B 100000, C 500000, D 1500000, E 100000, F 300000 | **A 1200000, C 1200000, E 300000, F 100000**; B and D unmoved | `S170_pick_budgets.sh` |
| readings | 697 mate_pv lines; defender 84/104 | 697; 86/104 | `test_mate_pv -s`; `S165_nmp_defender_sweep.py` |
| `golden_defaults`, option count, census, book | -- | unmoved by construction | -- |

**DEC-299's guard is met**: both mate floors hold and no ceiling moves, so
the budgets the picker answers are applied, with a header paragraph in
`adocs/data/S170_cases.tsv`, and the ceilings' command gains S123's grid
(`tests/test_mate_carry.cpp`, `DEV_MANUAL.md`). S123's grid: 108 cells, 1397
mate lines, 18 short; it alone answers 0, 3, 1, 0, 1, 2.

**Both floors now sit at or one below the shipping count.** `EXACT_FLOOR`
159 is one below the 160 that ships (DEC-313 (1)'s rule). `MATE_IN_THREE_FLOOR`
11 equals the 11 that ships, with no spare point: no integer lies strictly
between 10 and 11, so DEC-116's form is not available, and 10 would be a
falling floor that does not separate. Neither answer is lower than its
golden, so neither stops the step and neither golden moves. A bisection
part's refit, or S126's, that costs one more mate in three turns the floor
red, and that is a stop for the owner (DEC-303).

### Gates

- Both builds: 0 warnings. `ctest -L fast`: **50 of 50** in `build/` and in
  `build-tune/`, sequential, `CLANG_FORMAT_MAJOR=22`.
- `./clang-format.sh --check` clean.
- **The S297 clang pair** (`STAGES=clang tools/gate_extra.sh`): `build-clang`
  and `build-clang-tune` with clang 22.1.8, no error and no warning; both
  fast suites 50 of 50; 343 s.
- `tools/plan_prose_check.py`, one mode per call, each exit 0: `--citations`
  0 flagged over 19 files; `--touches` 0; `--params`; `--gate` agrees;
  `--prose` 0 flagged.
- **`tools/gate_extra.sh`**, all six stages, detached under a four-exit
  watcher: `GATE-EXTRA-DONE 6 stages 1365 s`. prose 0 s, citations 1 s, debug
  384 s (6 of 6), sanitize 668 s (the fast label 50 of 50 under the
  sanitizers; INV-6 across builds, both 5434393), perft 52 s, clang 260 s
  (50 of 50 twice). No `Assertion` in its logs.
- **Tier 2 (DEC-141) does not bind**, by what the diff touches: under `src/`
  only `src/evaluation.cpp`, its ten family arrays and one whole-line
  comment; no `make_move`, `unmake_move`, generator or search change.
  gate_extra ran anyway, as the brief asks, because the candidate puts
  `is_attacked()` and the family's code into every stage-one evaluation. The
  Debug self-play is the coordinator's.
- **Bench 5434393** in both builds: the commit's `Bench:`.

### The cost (the coordinator's run)

`adocs/data/S123_cost_reading.txt`, both runs:
- **The first launch**, 2026-10-30 03:20:09, stopped at INV-6 before any
  timing (`.tuning/s123_cost_void1.console`): `streams()` kept
  `search_bench.py`'s raw output, seconds and knps included, in the directory
  `diff -r` compares, so zero against parent read different on timings
  alone, nodes and best moves identical; `S123-COST-FAILED`. Fixed in
  `1a231dd`.
- **The run**, 03:29:42 to 04:37, `03680ab` against `244d89d`,
  `S123-COST-DONE` (`.tuning/s123_cost.console`): INV-6 bench 6016079 on
  every binary; five distinct `.text` sections.
  - a COST **+2.59 %**, b **+1.52 %**, c **+1.48 %**, all **+4.96 %**: cycles
    and wall beyond the floor, instructions the same way.
  - parent: zero, inside the A/A floor.
  - The A/A floor: bench16 cycles 0.445 %, pos11 cycles 0.246 %, wall
    0.303 %.
  - Forced instructions, printed and not gated: bench16 a + b + c +5.195 %
    against all +4.308 %; pos11 +7.761 % against +6.882 %.
- Every group is live at the candidate, so `all` is its row: +4.96 %. It
  decides nothing; the SPRT prices the family with its cost.

### The SPRT script

`adocs/data/S123_sprt.sh`, re-read against this candidate:
- The guard accepts `1a231dd` as REF with `03680ab` as the landing
  (the coordinator's ruling): `git diff --quiet 03680ab 1a231dd -- src/`
  holds, and CAND's parent will be `1a231dd`. "a documents-only descendant"
  was false of a commit that changes a script; it now reads "a descendant of
  it whose src/ is the landing's byte for byte", in the header and in the
  guard's message. No guard changed.
- The guard simulated on this worktree against HEAD: `src/evaluation.cpp` the
  only file under `src/` that differs; its 20 changed code lines are the ten
  family definitions, old and new, nothing outside the allowed forms (the
  comment this paste adds over the arrays is whole-line); and every weight
  outside the ten is HEAD's by name (`S123_screen.py only-family`).
- The bisection A | B | C; the H0 paragraph's "at worst three more runs
  after this one, four in all" holds with three survivors. Each part under
  H0 is its group refitted alone over what has landed (`part NAME`), so B's
  and C's models there are not the screen's, which were fitted with the
  earlier parts free; the new paragraph says so.
- A dated 2026-10-30 paragraph: the candidate ABC, the guards as read, the
  bisection, the cost (all +4.96 %), no clamp reading.
- The coordinator's markers are left unfilled; run once, the script refuses
  on them before anything else.
- The fast check's two guard gaps are closed (below): the family-line form
  ends at `};`, and the build files are compared beside `src/`.

### Also changed

- `src/evaluation.cpp`: one whole-line comment over the family's arrays,
  that the values are the screen's ABC fit pasted unchanged, monotonicity not
  imposed. `src/evaluation.hpp` is untouched: the SPRT guard allows
  `src/evaluation.cpp` alone, and the definitions' comments wait for the
  closing commit.
- `adocs/data/S192_anchors.py`: the two anchors and the family's note.
- `DEV_MANUAL.md`: every goldens row S123 moved or re-read, the `--ceilings`
  command with S123's grid, the gradient and walk sentences. Where the step
  file and `DEV_MANUAL.md` differ, the manual's commands were followed; they
  did not differ on any command run here.
- `MANUAL.md`: the `RfpMinPly` row's mined-set figures (160, 158, 153
  against 159).
- `adocs/data/README.md`: rows for `S123_screen_reading.txt`,
  `S123_cost_reading.txt`, `S123_sweep_s170.txt` and `S123_goldens.txt`.

### The fast check's three, fixed before completion

- **Three golden sites still carried old figures** (DEV_MANUAL.md had the
  new ones): each gains a dated S123 paragraph in S121's and S293's form.
  `tests/test_mate_breadth.cpp` `EXACT_FLOOR`: the header's ends 165 and 154
  become 160 and 153 with the history kept, the margin sentence 1 below and 6
  above since S123, and "159 held at S123". `tests/test_engine.cpp`
  `MATE_IN_THREE_FLOOR`: the header's "13 ... 8 ... read at S135" (stale since
  S121) becomes 11 and 10 at S123, the false "Margin: 1 below the shipping
  count (2 since S135)" becomes 0 below since S123, and a paragraph holds the
  floor at 11 at zero margin, separating, under DEC-299's guard, a further
  lost mate in three a stop for the owner (DEC-303), citing **DEC-330**, the
  coordinator's record in this commit. `tests/test_search.cpp` `ALPHA`: the
  candidate's band, -2153 to -1923 (45 below, 185 above) and -1828 to -1790,
  and its trace, iteration 7's searches 20 to 22 at alphas -2039, -2155 and
  -2334. `DEV_MANUAL.md`'s mate-in-three row cites DEC-330 too.
- `tests/test_mate_pv.cpp`'s history sentence "so the budget from 50000"
  now reads "50000 -> 40000" (and S121's "40000 -> 50000").
- **`adocs/data/S123_sprt.sh`'s guards, two gaps older than this diff,
  strengthened.** The family-line form anchored only the line's start, so
  code appended after `= {...};` passed; it now requires the whole line, a
  one-line definition ending at `};`. And the landing and CAND checks read
  `src/` alone, so a flag change in the top-level `CMakeLists.txt` or
  `cmake/` passed; both now read `src`, `CMakeLists.txt` and `cmake`
  (`src/CMakeLists.txt` is under `src/`; tests/ and tools/ build no part of
  the engine target). In a scratch clone (REF `1a231dd`, the landing
  `03680ab`, CAND this paste committed on REF), the script copied with its
  markers and the pair filled and its `exec` replaced by an echo:
  - the real pair passes (`only-family-pair` too) under the old guards and
    the new;
  - code appended after `passed_free_mg`'s `};` passed the old guards and is
    refused by the new: "CAND's src/ differs outside comments in more than
    the family's weight definitions";
  - `add_compile_options(-fno-inline)` appended to `CMakeLists.txt` passed
    the old and is refused: "CAND changes CMakeLists.txt beyond the paste";
  - a line added to `cmake/arch.cmake` at REF passed the old and is refused:
    "REF's src/ or build files differ from the phase 1 landing's".

### Proposed commit message

```
Paste S123's passed pawn family fit for its SPRT (S123)

The screen kept all three groups (DEC-329): the free and safe
increments, both kings' distance slopes and candidate passers, each
past its margin on S082's fixed set. Their full-corpus fit, ABC, is
pasted over the ten family arrays for the one {0, 5} SPRT that
decides the family (DEC-259); an H0 is bisected A | B | C.

Every golden the paste moves is re-derived by its script (DEC-142):
the anchors 109 and 196; the truncation pins re-picked, N still 3;
the clamp push +80 -> +30; the aborted-line cell's budget 50000 ->
40000; ALPHA kept inside its band; both coverage blocks reproduce.
Both mate floors hold: EXACT_FLOOR 159 against 160 and 153,
MATE_IN_THREE_FLOOR 11 against 11 and 10, held at zero margin
(DEC-330). No mate-carry ceiling moves, so the picker's budgets for
A, C, E and F apply (DEC-299). The screen's reading and the cost
run's, all +4.96 %, are committed. S123_sprt.sh's guards now read the
build files beside src/ and a family line to its end.

Bench: 5434393
```

No `done:` stamp: the SPRT comes first.

## 2026-10-30: the SPRT read H1

One Opus 5.5 subagent, in `../chesso-s123d` (branch `s123d` at `a060435`,
the SPRT's pin), read only over the run's outputs and binaries; no match,
fit or timing ran.

### The run

`adocs/data/S123_sprt.sh` as pinned at `a060435`: `0553160` (the candidate,
the screen's ABC fit over the ten family arrays, bench 5434393) against its
parent `1a231dd` (the phase 1 landing's `src/`, the family at zero, bench
6016079), launched 2026-10-30 06:20:21, `SPRT-RUN-DONE` after fastchess's
Total Time 00:33:00:

```
SPRT | cand 0553160 vs ref 1a231dd, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | 49.19 +/- 15.06, nElo 66.07 +/- 19.94
LLR | 2.96 (-2.94, 2.94) -> H1
Games | N: 1166 W: 410 L: 246 D: 510, Ptnml [26, 109, 196, 179, 73]
Wall | 0 h 33 m, 2120.0 games/h, forfeits 0
Log | adocs/data/S123_sprt.log
```

`Wall`'s rate is N over fastchess's `Total Time`, S296's, S136's and S121's
rule: 1166 games in 1980 s.

### The inspection, before the verdict was read as the record

- **Banner** (the console's): candidate `0553160 2026-10-30`, reference
  `1a231dd 2026-10-30` (the commits' dates; their times, 06:14 and 03:29, are
  `git log`'s; the pin `a060435` was committed at 06:20:21), `arch native tune off`, padding on both, 8+0.08, hash 16,
  12 of 12 cores, `noob_3moves.epd`, seed 20261030062021, `elo0=0 elo1=5
  alpha=0.05 beta=0.05`, cap 20950 rounds and 41900 games from
  `SPRT_ROUNDS`, out `.tuning/sprt_s123_20261030_062021`; no `diff`,
  `relay` or `A/A CALIBRATION` line; `id name`s `cand-0553160 Chesso
  0553160 native` and `ref-1a231dd Chesso 1a231dd native`; `Started game 1
  of 41900`. As the pin says. Above it the script's guards printed the ten
  family arrays as the only weights moved and `only-family-pair: every
  non-family weight is REF's`. The seed's stamp and the out directory's
  are the same second this time.
- **Binaries**: `.ref-builds/0553160/build/src/chesso` benches **5434393**
  and `.ref-builds/1a231dd/build/src/chesso` **6016079**, both worktrees
  clean at their sha. Written 06:20:27 and 06:20:33 and not since; the
  first game's `GameStartTime` is 06:20:33, the last `GameEndTime` 06:53:30.
- **Crashes, disconnects, illegal moves**: none. `fastchess.log` is empty
  (0 bytes), and the console holds nothing but the guards' and build lines,
  the banner, the game lines, fastchess's periodic result blocks, its stop
  lines and the foot. 1179
  games started and 1168 finished, the 11 others in flight at the stop.
  The PGN's 1168 games carry fastchess's game numbers (`2 * Round - 1`
  with the candidate White, `2 * Round` with it Black) 1 to 1170, each at
  most once, 1161 and 1166 absent: 583 complete pairs, the 1166 games
  fastchess scored, and games 1162 and 1165 whose partners never finished.
  Terminations: 786 adjudication (657 decisive, 129 drawn) and 382 normal,
  all drawn (252 threefold, 87 insufficient material, 41 fifty-move, 2
  stalemate).
- **Forfeits**: `tools/forfeit_report.py` reads 0 of 1168 for each engine.
- **Console warnings**: none of any kind; no PV-past-threefold block
  (`trim_console.py`'s count) and no `Incomplete mating PV` (next section).
- **Quarters**, in game-number order over the PGN's 1168 games, 292 each
  (boundaries `i * 1168 // 4`), by `adocs/data/S121_sprt_mate_pvs.py
  quarters`, which reproduces S121's +10.6, +16.7, +21.6, +28.2 (chi-square
  1.89) from S121's PGN, still in `.tuning/`: **+60.1, +44.3, +55.2,
  +35.8**, each +/- 28.7 to 31.0. Their spread about the run's mean is
  chi-square 1.50 on 3 degrees of freedom, p 0.68.
- **Kernel journal**, 06:20 to 06:55 (boot `a3198101`, from 2026-10-29
  23:04:41, no suspend): no kernel entry; the boot's last is 00:00:06, an
  AppArmor `DENIED` audit line for `cupsd`, before the launch, and before
  it 23:05:53's `usb 3-2.4: USB disconnect`, the monitor's control device,
  as on S121's night. Six entries in the window in all: the 06:25
  `cron.daily` run and `fwupd-refresh` at 06:54:06, after the last game; none
  at warning level or above. No machine check, OOM or thermal event.

The run stands.

### Mating PVs

`grep -c "Incomplete mating PV" .tuning/s123_sprt.log` reads **0**, and
`grep -ci warning` over the same console reads 0: no warning of any kind,
so no mate-distance check is owed and DEC-319 (3)'s oracle and DEC-320's
deeper reading do not arise. `adocs/data/S296_sprt_mate_pvs.py` reads the
same marker (`Warning; Incomplete mating PV - from`) and would find
nothing. A count, recorded: the candidate's rate this run is 0 per 10000
side-games over 1168.

### The verdict, by the pre-registration

**The H1 row executes.** At 8+0.08 the fitted family is accepted as
gaining: H1's 5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It
ships**: A, the free and safe increments per own rank 2 to 7; B, both
kings' distance slopes per own rank 2 to 7; C, candidate passers per own
rank 2 to 6; the screen's ABC fit, every group kept, so no group stays at
zero. **No bisection follows an H1**, and none is owed.

**The stopping run's Elo is upward-biased (DEC-063) and is not the effect
size.** The run stopped at a bound in 1166 games, far under the 25591 the
pre-registration expected with the truth on a bound; nElo +66.07 is the
early stop's figure.

**The cost, beside it**: every group is live at the candidate, so its row is
all's, **+4.96 %** (a +2.59 %, b +1.52 %, c +1.48 %, parent zero;
`adocs/data/S123_cost_reading.txt`), on cost-only builds of the landing at
forced weights (DEC-047). It decides nothing; the SPRT priced it.

### The trim and the archive

`tools/trim_console.py --summary` from `.tuning/s123_sprt.log` (2864 lines,
191623 bytes) into **`adocs/data/S123_sprt.log`**, 2869 lines, 191828
bytes: its first 2864 lines are the console byte for byte, since no
PV-past-threefold block exists, and the foot's five lines say so. Its
`adocs/data/README.md` row added. The untrimmed console and the run
directory (`games.pgn`, `fastchess.log`) are
**`chesso_S123_sprt_2026-10-30.tar.xz`** in the owner's archive (DEC-235),
built, verified and moved in by the coordinator, beside
`chesso_S123_screen_cost_2026-10-30.tar.xz`.

### The comment pass, and its INV-6

Comments only; no code, no weight and no definition moved. Each now
describes the shipped, fitted family:

- `src/evaluation.hpp`: `passed_pawn_sets()`'s symmetry paragraph (a
  one-colour set bug was invisible at the zero landing, and fitted it moves
  a score the mirror case sees only where a position reaches it); the
  family's definitions block (each group kept by its screen
  part, fitted together, shipped on one SPRT's H1); `passer_family_t`'s
  "at zero weights no score sees these" now the landing's, past tense; and
  the placement block's "ship fitted since S027" sentence, which now names
  S123's family as fitted since its H1.
- `src/evaluation.cpp`: the paragraph over the ten arrays (landed at zero,
  fitted, shipped on the H1, with the SPRT's log and the cost reading); its
  passer_family(), a name no function has, is now `passer_family_side()`
  and `candidate_passers()`; the liveness flags' comment (below); the
  stage-one bound's comment, whose "with the pawn terms ... of room"
  figures were S135's and which now gives the bound with the family
  shipping, 19982 and 25799, 12785 and 6968 of room
  (`adocs/data/S123_goldens.txt`, `S123_screen.py headroom`); and
  `passer_family_side()`'s comment (the gate kept the landing free; both
  groups live since the H1).
- `src/eval_trace.hpp`: read, and nothing in it calls the family zero,
  pending or compiled out; its five `TERM_PASSED_*` / `TERM_CANDIDATE_PASSER`
  entries are what ships. Unchanged.
- Tests, comments only: `tests/test_evaluation.cpp` (the passer-sets case's
  "moves no score while the weights it feeds are zero", and the packed
  halves' "landed at zero: until a fit, the bench cannot see"),
  `tests/test_eval_trace.cpp` (its header listed S123's whole family among
  what ships at zero), `tests/test_eval_model.cpp` (the family cases' "at
  zero weights no score sees these counts") and
  `tests/test_tuner_gradient.cpp` (the gradient case's history ended at the
  zero landing; the candidate's 2.79e-7 at parameter 985 added, from
  `adocs/data/S123_goldens.txt`). No assertion, golden or input moved.
- Tools, `DEV_MANUAL.md` and `adocs/data/README.md`: read; no site calls
  the family at zero or pending other than as the landing's history.
  `adocs/data/S192_anchors.py`'s "at S123's landing every weight was zero"
  is past tense and true.

**The liveness gating keeps a purpose, and stays.** Since the H1 every
group is live, so `passer_advance_live`, `passer_kings_live` and
`candidate_passer_live` are all true and gate nothing in the shipped
binary. They are kept for the refit that zeroes a group again -- S126
refits everything this step landed, and a group a later screen drops is
held at zero and recorded as a corpus finding (S126's rule) -- because A's `is_attacked()` is
an out-of-line call the compiler cannot delete as it deletes a dead
popcount. The comment over `any_weight()` says so.
`adocs/data/S123_cost_only.patch` replaces the same three flags with its
mask, but it applies to the landing `03680ab` only, so HEAD's flags do not
bind it.

**INV-6 against `0553160`** (`adocs/data/S123_close_inv6.txt`): `chesso
bench` **5434393** in both builds; `bench`, `bench 12` (2089917) and
`tools/search_bench.py` at depth 9 (c3d5 26906, d5e6 93894, d7c8q 35629)
and depth 12 (c3d5 317968, d5e6 341523, d7c8q 205179) byte-identical once
timing is stripped, across the played binary, the worktree's `build/` and
`build-tune/` before the pass, and both after it. The object code beside
it: the clean pre-pass `build/` binary's `.text` is the played binary's
byte for byte; after the pass the binaries answer `a060435-dirty`, a longer
string that moves `.rodata`, so `.text` differs in rip-relative
displacements only -- the disassembly with displacements masked is
identical across played, pre and post in each build. A rebuild at the
commit should read the played `.text` again (`f20704bc...`); that is the
coordinator's check.

### S118, after DEC-329

DEC-329's consequence holds in this file (section 7, "A pawn-key cache can
hold the bitboard and C, never A or B, which read pieces and kings"). S118's
accepts does not yet say so: it has the table store "the mg/eg pawn score,
the passed-pawn bitboard and both kings' shelter and storm scores", and the
pawn score now includes A, which reads every enemy piece through
`is_attacked()` and the push square's occupancy, and B, which reads both
kings. S118's file is not this step's; its sentence is quoted to the
coordinator.

### What H1 leaves unmeasured

- **Each group's own share.** A, B and C shipped on one verdict and were
  never played apart; each has only its screen gain (A g +0.000070, B d
  +0.000109 against A, C d +0.000018 against AB, against m 0.000009,
  0.000013 and 0.000010).
- **The cost apart from the verdict.** +4.96 % is priced inside it; no run
  measured the family's strength net of its speed or the groups' costs in
  play.
- **`MATE_IN_THREE_FLOOR` at zero margin** (DEC-330): 11 shipping and 10
  weakened, so it still separates; a refit that costs one more mate in
  three turns it red, a stop for the owner (DEC-303).
- **The cells far from their neighbours** (`adocs/data/S123_cells.txt`, by
  `adocs/data/S123_cells.py`: the value less its neighbours' mean, the
  cutoff |v-nbr| >= 10 chosen here and deciding nothing), with their
  support in the audit, rows and games:
  - `passed_free_mg` rank 7, 76 (+48.0), 105778 / 30269; rank 6, 28
    (-13.5), 331115 / 65787;
  - `passed_free_eg` rank 2, 23 (+20.0), 627782 / 72747; rank 3, 3
    (-15.5), 659777 / 79928;
  - `passed_safe_mg` rank 7, 33 (+23.0), 30674 / 14246;
  - `passed_safe_eg` rank 6, 51 (+13.5), 144584 / 45665;
  - `candidate_passer_mg` rank 2, -6 (+10.0), 324340 / 53774; rank 3, -16
    (-15.0), 233544 / 45383; rank 6, -2 (-12.0), 30042 / 16482;
  - `candidate_passer_eg` rank 6, 59 (+13.0), 25004 / 14025.

  Every family cell half is supported by 14025 games or more (the thinnest
  `candidate_passer_eg` rank 6), far over the 1000-game tie rule, and none
  is tied. No B slope reaches the cutoff. The end cells of a row have one
  neighbour, so a steep row's last cell reads far by construction.
  Recorded for S126, which refits them; no planted recovery was built, as
  phase 1 recorded.
- **The family's planted recovery**, never built (phase 1's preconditions).

### Documents

- `DEV_MANUAL.md`: checked. Nothing false: its S123 rows (the goldens
  table, the gradient paragraph, `feature_audit`, the tuner's groups) state
  the candidate's readings, which are what ships, and the landing's as
  history. Unchanged.
- `MANUAL.md`: checked; nothing false. Its `RfpMinPly` row's S123 figures
  are the candidate's, and nothing on the UCI surface moved. Unchanged.
- `adocs/data/README.md`: a close section with `S123_sprt.log`,
  `S123_close_inv6.txt`, `S123_cells.py` and `S123_cells.txt`.

### The accepts, clause by clause

- **One SPRT verdict for the family, recorded whatever it is**: H1, above,
  with DEC-220's block for the closing commit. Holds.
- **After each group passes DEC-259's offline screen**: A, B and C each
  cleared its margin on S082's fixed set before the match
  (`adocs/data/S123_screen_reading.txt`). Holds.
- **A family H0 bisected by group**: the verdict is H1, so no bisection is
  owed; the partition A | B | C was pre-registered and `S123_screen.sh part
  NAME` prepared. Holds.
- **The table is rank crossed with can-advance and safe-advance rather than a
  single rank curve**: A's `passed_free` and `passed_safe` per own rank 2 to
  7 on top of the five buckets, so each rank scores blocked, free but
  attacked, or free and safe apart (`src/evaluation.hpp` defines the
  three). Holds.
- **The distance to each king scaled by rank**: B's `passed_king_own` and
  `passed_king_enemy`, one fitted slope per own rank 2 to 7 per king.
  Holds.
- **Candidate passers are scored**: C's `candidate_passer`, per own rank 2
  to 6, CPW's per-square rule. Holds.
- **Monotonicity is not imposed**: the fit pasted unchanged; non-monotonic
  rows ship as fitted (`passed_free_eg` {23, 3, 14, 13, 5, 6},
  `candidate_passer_mg` {-6, -16, 4, 10, -2}). Holds.
- **A test asserts the passer bitboard colour-symmetric under board
  mirroring, written before any fit**: `tests/test_evaluation.cpp` "the
  passer sets mirror exactly under board mirroring", written and observed
  red on a planted one-colour defect in phase 1, before the screen ran
  (above), green in both builds here. Holds.

Section 6's nps clause holds too: the cost, +4.96 %, is recorded beside
the verdict.

### Gates

- Both builds with no warning (the touched translation units rebuilt, 0
  warning lines); both fast suites **50 of 50** in `build/` and
  `build-tune/`, `CLANG_FORMAT_MAJOR=22`, run again after the pass's last
  edit; `./clang-format.sh --check` clean; bench 5434393 in both.
- **The S297 clang pair** (`STAGES=clang tools/gate_extra.sh`):
  `build-clang` and `build-clang-tune` with clang 22.1.8, no error and no
  warning, both fast suites 50 of 50; 307 s on the final tree (a first run,
  343 s, overlapped the last comment edit and was taken again).
- `tools/plan_prose_check.py`, one mode per call, each exit 0:
  `--citations` 0 flagged over 19 files (this file 6 code citations, 0
  flagged), `--touches` 0 over 19, `--params`, `--gate` agreeing, `--prose`
  0 flagged.
- **`tools/gate_extra.sh` is not run**, by what the diff touches: under
  `src/` only whole-line comments in `src/evaluation.hpp` and
  `src/evaluation.cpp`, shown neutral by INV-6 above, since its six-stage
  green run on CAND's tree (phase 2, 1365 s). **Tier 2 (DEC-141) does not
  bind**: no `make_move`, `unmake_move`, generator or search change.

Every clause holds. Done.
