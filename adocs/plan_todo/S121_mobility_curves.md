id:         S121
goal:       mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on
accepts:    an SPRT verdict, recorded whatever it is; mobility is a table indexed by piece type and by count -- knight 0 to 8, bishop 0 to 13, rook 0 to 14, queen 0 to 27 -- and **every entry is fitted by our own tuner on our own corpus** (DEC-084); the mobility area excludes squares attacked by enemy pawns and the side's own blocked and low-rank pawns, and each exclusion is a separate measured decision rather than one bundle; the tuner's model in tools/eval_model.hpp is updated in the same commit and test_eval_model holds the two against each other; the nps cost is recorded next to the verdict
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tests/test_eval_model.cpp
excludes:   king safety, which shares the loop but is S122
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## What is there, and why the fit could not save it

Mobility is **linear**: one weight per piece type times a raw count, over
`attacks & ~own` with no exclusions at all. `src/evaluation.cpp`:

    const int mobility_mg[4] = {-1, 5, 8, 3};   // knight bishop rook queen
    const int mobility_eg[4] = {0, 5, 0, -6};

Knight middlegame **-1**, knight endgame **0**, rook endgame **0**, queen
endgame **-6**. Those are what a correct fit returns when the model cannot
express the shape: mobility is not linear in the count -- the first few squares
are worth far more than the twentieth -- and a single coefficient fitted across
that curve lands near zero. DEC-040 already noticed the symptom and read it as
a finding about knights; it is a finding about the model.

The exclusions are the other half and are separately reported large:
excluding rammed and low-rank own pawns **+19.95 +/- 9.63** at 10+0.1 -- Stash
v27's mobility zone, `mhouppin/stash-bot` `CHANGELOG.md`
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md) -- and, from the same changelog, taking the king's own square
out of the mobility zone at v32, **+10.86**. The figure this file used to quote
for excluding enemy pawn attacks from knight mobility carried no source through
two searches and is deleted (DEC-203); the exclusion itself is unpriced here.
CPW *Mobility*
(https://www.chessprogramming.org/Mobility) defines "safe mobility" as counting
only squares not attacked by enemy pawns -- best for knights, which is where
that claim pointed -- but **states no figure**, and the term "mobility area"
and the per-count curve are not on that page at all. Note what is
*not* done anywhere: full "safe mobility" excluding every attacked square. The
exclusion is enemy **pawn** attacks, and that distinction is worth stating
because it is the cheap half.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**Where the wiki's definition stops, exactly.** CPW *Mobility*
(https://www.chessprogramming.org/Mobility, fetched 2026-09-13) defines
mobility as "a measure of the number of choices (legal moves) a player has in
a given position", defines "safe mobility" as "counting only squares where a
piece can move without being En prise" and warns it "might be quite expensive,
unless a program already keeps incrementally updated attack tables", and then
gives the one sentence this step's cheap half rests on: "in some cases, most
notably in case of a knight, a middle-of-the-ground approach, not counting
squares controlled by enemy pawns, seems best." **Absent from the page: the
term "mobility area", any per-count curve or table, and any centipawn or Elo
figure.** This is the S185 note discharged: the page carries the enemy-pawn
exclusion and carries neither of the other two things this step builds.

**So where do "mobility area" and the per-count curve come from.** Both are
read for **form only**, from engine write-ups, never from source:

- The phrase and its membership rule are Ethereal's release note for 7.78
  (https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+mobility,
  commit fb346d60ee6eed8c4bd8c7ee404641cccbc4cbd6, 2016-09-04): "mobility area
  is all squares but enemy pawn attacks, friendly blocked pawns, and friendly
  king location". That is the three-part exclusion this step's accepts names,
  stated in prose by the engine that added it. The same release also added
  knight mobility and per-piece tables at once and reports **+22.2 at 1s+.01s,
  +24.0 at 5s+.05s and +42.5 at 10s+.1s** -- a whole-release bundle, so it
  prices the exclusion for nothing (DEC-019).
- The per-count table is described in the same note as "MobilityRook[phase]
  [mobCount]" -- a table indexed by phase and by count, which is the shape, and
  the shape is all that is taken.
- Stash names a "Mobility Area" too, and its changelog is where the exclusion
  has a figure of its own: v27's mobility zone excluding rammed and low-rank
  own pawns, **+19.95 +/- 9.63** at 10.0+0.1s, and v32 taking the king's own
  square out of the zone, **+10.86**
  (https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md).

**The figure for the knight exclusion survived two searches unsourced and is
deleted (DEC-203).** It was quoted here for "excluding enemy pawn attacks from
knight mobility". Searched 2026-09-04 by the plan review: no source. Searched
this pass: a web search for it against mobility and knights; the Ethereal
mobility commit set above; `mhouppin/stash-bot`'s `CHANGELOG.md` scanned for
it, where it does not occur. The claim it supported does not need it -- CPW
states the knight exclusion in words and Stash's +19.95 prices a mobility
zone -- so nothing in this step's order or bounds moves with it gone.

**Two further mobility-zone refinements, both traced, both small.** Ethereal
"For mobility, don't let queens xray through our rooks/bishops"
(commit 4c3f29614859f989c5e1f2ce57ef3a89969ed72a, 2019-07-01): **+10.82 +/-
6.28** at 10.0+0.1s over 5010 games and **+7.82 +/- 4.37** at 60.0+0.6s over
8090 games. Ethereal "Use Stockfish definition of mobility area" (commit
5dee98656c8d02803072ff649c3afc3db10d16be, 2017-12-05) reports LLR and game
counts and **no Elo figure**. They are direction only and neither is in this
step's scope.

### 2. Shape for chesso

Today: `src/evaluation.cpp` `mobility_mg` and `src/evaluation.cpp`
`mobility_eg`, four weights each, over a raw count with no exclusions. The
step replaces both with a table indexed by piece type and by count -- knight 0
to 8, bishop 0 to 13, rook 0 to 14, queen 0 to 27 -- and the count is taken
over a mobility area.

**Departures from the definitions above, stated:**

- CPW's "safe mobility" excludes **every** attacked square. This step does
  **not** do that; it excludes enemy *pawn* attacks, which is the page's own
  "middle-of-the-ground approach" and the cheap half. Saying so matters
  because a reader who implements the page's first definition gets a different
  and more expensive term.
- Ethereal's mobility area is three exclusions at once. This step's accepts
  requires each exclusion to be **a separate measured decision rather than one
  bundle**, which is a departure from every located record: all three surveyed
  engines landed their zones as one patch.
- The per-count table is fitted here, entry by entry, by this project's own
  tuner on its own corpus. No located source publishes a curve shape as a
  formula, so there is nothing to depart from.

### 3. Implementation sketch

- `tools/eval_model.hpp` gains the same table in the same commit, and
  `tests/test_eval_model.cpp` holds the two against each other -- the model is
  a second implementation and a drift tunes the wrong function.
- The mobility area is computed once per side in the loop that already walks
  the pieces; the enemy pawn-attack span is the same fill the pawn terms build,
  which is the argument S102 makes for its own reuse.
- The table is indexed by count, so the count must be clamped to the table's
  width per piece type. **The queen's count never exceeds 27**: a lone queen
  on an empty board attacks at most 27 squares, on d4, d5, e4 or e5, and 21 in
  a corner. That is the tool's answer, not a remembered one -- `python-chess`,
  `max(len(board.attacks(square)))` over a queen placed on each of the 64
  squares of an empty board, run 2026-09-13 (CHESS: a board fact comes from a
  tool). So a queen row of 28 entries, indices 0 to 27, covers every legal
  count, and the clamp guards against a counting bug rather than against the
  board. An earlier draft of this bullet claimed 28 reachable squares were
  legal; that was wrong and the tool is why it is now 27.
- nps recorded next to the verdict, per the accepts.

### 4. Constants and seeds

**No seeds.** Every entry is the tuner's output (DEC-084, and the accepts says
"every entry is fitted by our own tuner on our own corpus"). The exclusions
are structural, not numeric.

What must be declared rather than fitted is the **table width per piece type**,
and it is **(b) derived from the board**: 8 for a knight, 13 for a bishop, 14
for a rook and 27 for a queen are the maxima the geometry allows, so the widths
come from the rules of chess and not from any engine. They are already in the
accepts and this section is where the derivation is recorded. **All four were
computed, not recalled**: `python-chess`, one piece at a time on an otherwise
empty board, `max(len(board.attacks(square)))` over all 64 squares, run
2026-09-13 -- knight 8 (c3 and its class), bishop 13 (d4, d5, e4, e5), rook 14
(every square), queen 27 (d4, d5, e4, e5). A board fact comes from a tool and
this one did.

No engine's mobility bonus array seeds anything here, wherever it is
republished -- a shipped mobility table is that engine's tuned output
(DEC-105, DEC-134), and section 1 reads those records for their **shape** and
their Elo only.

### 5. Pitfalls

- **The linear weights are not a starting point.** `mobility_mg` reading
  knight -1 and `mobility_eg` reading knight 0 and queen -6 is what a correct
  fit returns when the model cannot express the curve. Seeding the new table
  from them reintroduces the artefact.
- **Monotonicity must not be imposed.** A fitted curve that is not monotone is
  reporting something; S123's accepts makes the same rule explicit and it
  applies here.
- **The three exclusions interact.** Excluding own blocked pawns and excluding
  enemy pawn attacks can remove the same square. A per-exclusion verdict has to
  be taken in a fixed order with the earlier exclusions live, and the order is
  stated in the step.
- **Cost.** The term runs at every `evaluate()` call and `evaluate()` runs at
  every quiescence node. A per-count table lookup is cheaper than the linear
  multiply it replaces; the mobility *area* is not free, and the nps figure is
  in the verdict.
- **The lazy clamp caps this term together with king safety.** S039 sits before
  S122 for that reason and this step's larger mobility values change the sum
  S039 then measures.

### 6. Measurement

One SPRT at the S105 regime, bounds stated in advance with the nElo worst case
and the abort rule (DEC-143), plus a separate measured decision per exclusion
as the accepts requires -- which is more than one verdict and is priced as
such. The expected size from the traced record is the Stash +19.95 class for
the zone; the curve itself has no published figure and its verdict is its own.
nps beside every verdict.

### 7. Interactions

- **S122 (shares the loop)**: king safety walks the same attack sets. The two
  are separate steps so the verdicts attribute.
- **S039 (before S122, after this)**: the clamp is sized on the sum this step
  enlarges.
- **S102 (after)**: outposts reuse the same pawn-attack span fill.
- **S126 (block end)**: refits the whole table.
- **S118 (after S125)**: the pawn-attack fills the area needs are the ones the
  pawn hash caches; this step is measured before that cache exists.

### 8. References

- - https://www.chessprogramming.org/Mobility -- definition, "safe mobility",
  the knight sentence quoted above; **"mobility area", per-count curves and all
  figures absent**. Fetched 2026-09-13.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+mobility --
  commit fb346d60 (release 7.78, 2016-09-04), "mobility area is all squares but
  enemy pawn attacks, friendly blocked pawns, and friendly king location",
  "MobilityRook[phase][mobCount]", release figures +22.2 / +24.0 / +42.5;
  commit 4c3f2961 (2019-07-01) queen xray +10.82 +/- 6.28 / +7.82 +/- 4.37;
  commit 5dee9865 (2017-12-05) "Use Stockfish definition of mobility area", no
  Elo stated. Commit messages only. Fetched 2026-09-13.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- v27
  mobility zone excluding rammed and low-rank pawns +19.95 +/- 9.63 at
  10.0+0.1s; v32 king square out of the zone +10.86. Changelog entries only;
  scanned 2026-09-13 for the knight-exclusion figure this file used to quote,
  which does not occur.
