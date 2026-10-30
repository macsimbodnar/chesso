id:         S123
goal:       passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
accepts:    one SPRT verdict for the family, recorded whatever it is, after each group passes DEC-259's offline screen, and a family H0 bisected by group; the table is rank crossed with can-advance and safe-advance rather than a single rank curve; the distance to **each** king is scaled by rank, because the same distance is worth more to a pawn on the seventh; candidate passers are scored; **monotonicity is not imposed** -- a fit that returns a non-monotonic middlegame curve is reporting something and is not corrected by hand; a test asserts the passer bitboard is colour-symmetric under board mirroring, written before any fit, because two published passed-pawn bugs shipped invisible to everything except an SPRT
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_trace.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp, tools/tuner_trace.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tests/test_evaluation.cpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_eval_positions.hpp, tests/test_walked_counts.hpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, .tuning/verify_fit.py, adocs/data/S192_anchors.py, adocs/data/S121_coverage_rows.cpp, adocs/data/S123_*, DEV_MANUAL.md, adocs/data/README.md; at the paste (phase 2) every golden it moves, each by its script
excludes:   pawn structure terms, which are S125
decisions:  DEC-071, DEC-084, DEC-259, DEC-315, DEC-326, DEC-324, DEC-221, DEC-134, DEC-090, DEC-047, DEC-142, DEC-329
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:

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

### Known and unfixed

- `adocs/data/S121_cost.sh` and `adocs/data/S135_cost.sh` `streams()` have
  the gap S123's copy had: no exit status checked and no bench total
  required, so a bench or `search_bench.py` failing the same way for every
  binary writes identical error text and passes their INV-6 `diff -r`. Both
  are finished steps' records and are not edited here.

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
