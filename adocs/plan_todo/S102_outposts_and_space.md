id:         S102
goal:       outpost and space terms in the evaluation, fitted like every other constant
accepts:    an SPRT verdict per term, measured separately -- outposts and space are two terms; each fitted with every other constant frozen, held-out error reported before and after; both share the pawn-derived bitboard fills the three S027 pawn terms already build, and the step states which fill each reuses rather than adding a pass; INV-5 holds, asserted by the mirror case in tests/test_evaluation.cpp; the taper is a single division if S055 has landed and the model guard's bound still holds; tools/eval_model.hpp gains each feature and tools/tuner_groups.hpp a group per term with the partition properties intact; the fast suite green
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp
excludes:   threat terms, which are S101; mobility, which exists; any term that needs a pass over the board that is not already being made
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Why these two and not a longer list

They are the two terms on the standard hand-crafted list that reuse work
already being done: an outpost is a square no enemy pawn can attack -- which is
the pawn-attack span the passed pawn and pawn structure terms already fill --
and space is a count over safe squares behind one's own pawns in the centre,
from the same fills. Anything needing its own pass over the board is a
different trade and is not in this step.

## The measurement risk this step carries

Both terms are known to fit well and to be worth little on their own. **No
located source prices either term alone** -- the claim that the literature's
figures for them are small was itself unsourced and is deleted (DEC-203); the
honest statement is that the literature gives no figure at all. The
expectation of a small verdict rests on this engine's own record instead: it
has measured 0, 0 and *slower* on terms adopted from published reports
(DEC-019). A verdict of zero is the expected outcome for at least one of the
two and is recorded as zero.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**Both terms are on the wiki, and one of them carries a published magnitude.**

- CPW *Outposts* (https://www.chessprogramming.org/Outposts, fetched
  2026-09-13) defines an outpost as "a chess term most often related to knights
  in the center or on the opponent's half of the board, defended by an own
  pawn, and either no longer attackable by opponent pawns at all" -- so the
  published definition has **three** clauses (a knight, defended by an own
  pawn, not attackable by an enemy pawn), where this step's goal states only
  the third. The page's one magnitude is **an engine's, and it is refused as a
  seed**: "Toga log user manual advocates a bonus for a knight outpost on a
  central square as 10 centipawns, but it is possible to see bonuses as large
  as 16 centipawns" -- Toga's documented constant, republished on the wiki, so
  DEC-105 as amended by DEC-134 bars it from seeding anything here. It is read
  for form -- the term is worth a small fraction of a pawn to somebody who
  shipped it -- and for nothing else, the same treatment S122 gives CPW's
  2 / 3 / 5 / 6 attack units and the Glaurung table. **No Elo figure.**
  Reachable outposts are **not** described on the page.
- CPW *Space* (https://www.chessprogramming.org/Space, fetched 2026-09-13)
  defines space as "a loosely defined evaluation feature related to square
  control, in particular center control considered by piece placement dependent
  on pawn structure", and gives the one concrete specification on the page:
  "Stockfish defines a space area bonus by the number of safe squares for minor
  pieces on the central four files on ranks 2 to 4, counting twice if on a
  rearspan of an own pawn". **No centipawn value and no Elo figure.**

**No located source prices either term alone.** The 2026-09-04 literature check
recorded both as present in Ethereal, Berserk and Stash and found no figure
for either; this pass fetched the two wiki pages and confirms neither carries
one. Berserk's 4.3.0 release note names "space" among a ten-item bundle
estimated at "about 65 Elo stronger than Berserk 4.2.0"
(https://github.com/jhonnold/berserk/releases/tag/4.3.0), which prices the
release and not the term -- the same bundle S133 reads and the same caution
applies. **So this step's old claim that "the literature's figures for them are
small" had no source and is deleted** (DEC-203): the honest statement is that
the literature gives no figure at all, and the expectation of a small verdict
rests on DEC-019's three local cases (staged generation 0, SEE pruning in
quiescence 0, capture ordering slower) and not on a published number. "The
measurement risk this step carries" above now says that.

### 2. Shape for chesso

**Departures from the wiki's definitions, stated:**

- **Outposts.** The page's form is knight-specific, requires the square to be
  defended by an own pawn, and requires it to be unattackable by an enemy
  pawn. This step's goal names only the pawn-attack-span clause. The step
  states which clauses it ships: shipping fewer than the page's three is a
  departure and changes what the term measures, and the pawn-defended clause
  is free because the same fill gives it. Bishop outposts and *reachable*
  outposts are outside the page entirely and outside this step.
- **Space.** The page's one concrete form is Stockfish's -- central four
  files, ranks 2 to 4, safe squares for minor pieces, doubled on an own pawn's
  rearspan. That is a **description of another engine's term in a publication
  about the technique**, so its *shape* may be used (DEC-014); its weights are
  not published and would not be seeds if they were (DEC-105). The step states
  which square set it counts and why, and any departure from the page's set.

Both terms reuse the pawn-derived fills `src/evaluation.cpp` `evaluate_pawns`
already builds -- the accepts requires the step to name which fill each one
reuses rather than adding a pass, and after S121 the enemy pawn-attack span is
the mobility area's input as well.

### 3. Implementation sketch

- Two terms, two features, two groups in `tools/tuner_groups.hpp`, partition
  properties green in `tests/test_tuner_groups.cpp`.
- `tools/eval_model.hpp` gains each feature in the same commit.
- INV-5 is asserted by the mirror case in `tests/test_evaluation.cpp`.
- The taper is a single division if S055 has landed, and the model guard's
  bound is re-read at this step's own HEAD rather than off any sentence in a
  plan file -- the same rule S136's accepts states.

### 4. Constants and seeds

Both terms are fitted (DEC-084), both start from zero like every other new
term, and **neither takes a published magnitude**: the only one either wiki
page carries is Toga's (section 1) and DEC-105 as amended by DEC-134 refuses
an engine's constant wherever it is republished. Where a starting value is
wanted anyway -- for a sweep, or for an SPSA range -- both are **(c) the
midpoint of a range this file declares by purpose**, and both purposes are
chesso's own:

- **Outpost bonus.** A placement bonus must never be worth trading material
  for: a knight that is one square from an outpost may spend a tempo to reach
  it and must not be told to give up a pawn for it. So the declared range is
  **0 to half a pawn** on chesso's material scale (`src/eval_tables.hpp`
  `piece_value`, `PAWN` 94) -- 0 to 47, midpoint **24**, stated as a midpoint
  and never as a target. The band is wide, and that is the honest consequence
  of refusing the narrow one somebody else tuned: what ships is the fit's
  output and the band does not move it.
- **Space bonus.** Same form: the term counts squares and its per-square
  weight is bounded above by the smallest weight that would let a full board
  of counted squares outweigh a pawn, which the step writes out, with the
  midpoint stated as such.

No engine's outpost table or space weight seeds anything here wherever it is
republished (DEC-105, DEC-134).

### 5. Pitfalls

- **Zero is the expected verdict for at least one of the two** and is recorded
  as zero. That is this file's own sentence and DEC-019 is why; what changes
  above is only that no published figure supports the expectation either way.
- **Two terms, two verdicts.** The accepts says "measured separately"; a
  bundle at this expected size prices neither.
- **No new pass over the board.** A term that needs its own scan is a
  different trade and is excluded. The step names the fill each term reuses.
- **The stage decision.** Both terms are cheap-stage by construction if they
  ride the pawn fills; if either lands in the expensive stage it enters the sum
  S039 sizes the clamp on, and the step says which.
- **The outpost definition is a choice, not a detail.** Dropping the
  pawn-defended clause makes the term fire on far more squares and the fitted
  weight is then not comparable with any published one.

### 6. Measurement

One SPRT per term at the S105 regime, bounds stated in advance with the nElo
worst case and the abort rule (DEC-143), sized for a term that may measure
zero. Held-out error before and after each fit, reported. nps recorded.

### 7. Interactions

- **S121 (before)**: builds the enemy pawn-attack span both terms read, and the
  mobility area's safe-square notion is the same one space counts.
- **S125 (before)**: the pawn fills.
- **S101 (separate)**: threats; different terms, same reuse argument.
- **S055 (before, by plan order)**: the taper division count this step's model
  guard is read against.
- **S126 (block end)**: refits both.

### 8. References

- - https://www.chessprogramming.org/Outposts -- the three-clause definition
  quoted above; the Toga log user manual's 10 centipawns for a knight outpost
  on a central square, "as large as 16" -- an engine's documented constant,
  read for form and **refused as a seed** (DEC-105, DEC-134); reachable
  outposts **not** described; **no Elo figure**. Fetched 2026-09-13, page and
  footnote re-read 2026-09-13 after the fast check.
- - https://www.chessprogramming.org/Space -- the definition quoted above and
  Stockfish's square set as the page states it; **no centipawn value and no Elo
  figure**. Fetched 2026-09-13.
- - https://github.com/jhonnold/berserk/releases/tag/4.3.0 -- "space" named in a
  ten-item release bundle estimated at "about 65 Elo" for the whole release.
  Release note only.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` part B3 -- outposts
  and space present in Ethereal, Berserk and Stash; no figure located for
  either.
