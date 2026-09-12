id:         S123
goal:       passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
accepts:    an SPRT verdict per group, recorded whatever it is; the table is rank crossed with can-advance and safe-advance rather than a single rank curve; the distance to **each** king is scaled by rank, because the same distance is worth more to a pawn on the seventh; candidate passers are scored; **monotonicity is not imposed** -- a fit that returns a non-monotonic middlegame curve is reporting something and is not corrected by hand; a test asserts the passer bitboard is colour-symmetric under board mirroring, written before any fit, because two published passed-pawn bugs shipped invisible to everything except an SPRT
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tests/test_evaluation.cpp
excludes:   pawn structure terms, which are S125
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## What is there

Six buckets by rank, one weight each, and the middlegame row reads

    const int passed_pawn_mg[6] = {0, -6, -4, 19, 59, -17};

Bucket 5 is a pawn one square from promotion and it is fitted **negative**,
below bucket 4 at +59. That is a fit artefact of a corpus with few such
positions, and it is the same shape of problem as S121 and S100: not enough
model and not enough data on the positions the term exists for. The endgame row
is monotonic and plausible, which is the control.

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
`passed_pawn_eg`, six rank buckets, one weight each. After S134 there are
five, because bucket 5 folds into the tables.

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
  safe-advance are two predicates over the square in front, and "safe" needs
  the enemy attack set S121's mobility area also builds.
- The colour-symmetry test comes **first**, before any fit: "a test asserts the
  passer bitboard is colour-symmetric under board mirroring, written before any
  fit" is in the accepts, and Ethereal shipped exactly that bug and paid a
  patch for it -- commit f8585f1b10d00a4ad5de55119b061d5683d8da6c (2020-01-26),
  "Fix a small white/black mirroring issue with Passed Stacked Pawns",
  **+2.89 +/- 3.39** at 10.0+0.1s and **+1.63 +/- 2.42** at 60.0+0.6s. A
  mirroring bug worth ~2 Elo is invisible to everything except a match, which
  is the whole argument for the test.
- `tools/eval_model.hpp` gains every new feature in the same commit;
  `tests/test_evaluation.cpp` carries the mirror case (INV-5).

### 4. Constants and seeds

**No seeds.** Every entry is fitted by this project's tuner on its own corpus
(DEC-084). The figures in section 1 are Elo measurements of other engines'
patches, not constants, and no engine's passed-pawn table seeds anything here
wherever it is republished (DEC-105, DEC-134) -- including the wiki's own
"dedicated piece-square tables" sentence, which describes a shape and gives no
values.

Two structural numbers are declared rather than fitted, both **(b) derived
from the board**: the rank index runs 1 to 6 for a pawn that can still be a
passer (after S134, rank 7 is the tables'), and king distance is bounded by 7
on either metric. Both come from the rules of chess.

### 5. Pitfalls

- **Bucket 5 is the trap S134 exists for.** This step must not re-create a
  feature that is exactly the signed sum of a single rank of the piece-square
  table. Any new term defined on one rank needs the same identity check
  `tools/feature_audit.cpp` runs.
- **A per-group verdict means several runs.** The accepts says "an SPRT
  verdict per group". Rank-cross-advance, king distance and candidates are
  three groups and three verdicts, and the bounds for the candidate group
  should be chosen for the +5 class, not for the +22 class.
- **The mirroring bug is real and cheap to ship.** See section 3.
- **Safe-advance needs an attack set that is not free.** If it is computed
  twice -- once here and once in mobility -- that is the recomputation INV-4
  exists to stop.
- **The corpus decides what a passer bucket can learn.** Bucket 5 read
  negative because the corpus had few such positions; the new terms are
  fitted on S082's corpus and a group driven to zero is reported as a corpus
  finding (S126's rule, applied early).

### 6. Measurement

One SPRT per group at the S105 regime, bounds stated in advance with the nElo
worst case and the abort rule (DEC-143). The colour-symmetry test is green
before the first fit. nps is recorded because the safe-advance predicate adds
work at every `evaluate()` call.

### 7. Interactions

- **S134 (before)**: removes bucket 5 and with it the degeneracy.
- **S121 (before)**: builds the enemy attack set safe-advance reads.
- **S125 (after)**: the other pawn group; both are recomputed per call until
  S118.
- **S118 (after S125)**: caches the passed-pawn bitboard this step produces --
  the accepts of S118 names that slot.
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
