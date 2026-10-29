id:         S121
goal:       mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on, and a pinned piece counts only the moves along its pin
accepts:    an SPRT verdict, recorded whatever it is; mobility is a table indexed by piece type and by count -- knight 0 to 8, bishop 0 to 13, rook 0 to 14, queen 0 to 27 -- and **every entry is fitted by our own tuner on our own corpus** (DEC-084); the mobility area excludes squares attacked by enemy pawns and the side's own blocked and low-rank pawns, and each exclusion -- and the pin rule this family gains, a pinned piece counting only moves along its pin ray (DEC-258) -- is screened separately before the family's one SPRT (DEC-259); the tuner's model in tools/eval_model.hpp is updated in the same commit and test_eval_model holds the two against each other; the nps cost is recorded next to the verdict
touches:    phase 1 (2026-10-28, the table at identity, `No functional change`): src/evaluation.cpp, src/evaluation.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp (the ties), tools/tuner.cpp (the emit, `--tie`), tools/feature_audit.cpp (cell names, the eight mobility levels, the support report), tests/test_walked_counts.hpp, tests/test_eval_trace.cpp, tests/test_evaluation.cpp, tests/test_tuner_groups.cpp, tests/test_eval_positions.hpp (the coverage rows, phase 1b), tests/test_tuner_gradient.cpp (the clamp case's push re-derived, phase 1b), adocs/data/S192_anchors.py, adocs/data/S121_preconditions.txt, adocs/data/S121_inv6.txt, DEV_MANUAL.md, adocs/data/S121_{parts.patch,screen.sh,screen.py,cost.sh,cost.py,sprt.sh,coverage_rows.cpp,construct_rows.py,verify_fit.py}, adocs/data/README.md, this file; `.tuning/verify_fit.py` (gitignored) is replaced by adocs/data/S121_verify_fit.py at the landing; tools/tuner_trace.hpp (a comment only: its mobility line reads MOBILITY_COUNT, now 66); phase 2, on a candidate: src/evaluation.cpp's eight tables, and with a surviving part src/evaluation.hpp, src/bitboard.{hpp,cpp}, tests/test_walked_counts.hpp and adocs/data/S192_anchors.py through adocs/data/S121_parts.patch, folded at the closing commit; every golden the paste moves, each by its script (tests/test_evaluation.cpp, tests/test_search.cpp, tests/test_eval_trace.cpp's truncation pins, tests/test_tuner_gradient.cpp's preconditions, the search goldens S135's table lists); adocs/data/S121_* readings; adocs/specs.md (the coordinator's)
excludes:   king safety, which shares the loop but is S122
decisions:  DEC-071, DEC-084, DEC-258, DEC-259
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

**Scope gained 2026-10-08 (DEC-258):** mobility respecting pins -- CPW
*Mobility*; Stash's changelog, v30, +6.15. The pin map is computed once per
evaluation beside the attack sets this step builds (S263 built none);
the rest of this file is unchanged.

Each term -- and each part this file used to give a verdict of its own --
is first fitted with every other constant frozen and kept only if it lowers
held-out loss on S082's fixed validation set by more than the margin the
pre-registration states; a term that fails is recorded with its figures and
does not enter the match. The survivors ship under **one** `{0, 5}` nElo SPRT
for the family, with DEC-143's worst case and abort rule pre-registered. On
H0 or no verdict the family is bisected along a partition written in the
pre-registration, each part its own SPRT. Where this file names
`tools/eval_model.hpp` or `test_eval_model` as the tuner's mirror of a term,
S262's trace and its exact-reconstruction test discharge that clause once
S262 has landed. S263 built no per-side attack set, since no term read one;
a term here that needs one builds it once per evaluation. S039 read H0 (record `ca578de`,
revert `74d0adf`): the lazy shortcut and the clamp are kept. S122's design
waits on the owner's amendment before S121 starts (DEC-258), and that
amendment binds here: DEC-324 (2026-10-27) keeps the clamp and re-derives
its margin at S122, so this step runs as written.

## What is there, and why the fit could not save it

Mobility is **linear**: one weight per piece type times a raw count, over
`attacks & ~own` with no exclusions at all. `src/evaluation.cpp`:

    const int mobility_mg[4] = {-1, 5, 8, 3};   // knight bishop rook queen
    const int mobility_eg[4] = {0, 5, 0, -6};

**Corrected 2026-10-28 (phase 1a): those are S076's weights.** S083's joint
fit (`01222f3`) ships `mobility_mg {1, 6, 8, 2}` and `mobility_eg {0, 5, 2,
4}`, so the paragraph below now holds for the knight alone (1 / 0); the rook
and the queen no longer sit at zero or below. The argument is kept as it was
written; the numbers it read are history.

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
  stated in prose by the engine that added it. **Corrected 2026-10-28:** it
  is not -- the accepts names enemy pawn attacks and the side's own blocked
  and *low-rank* pawns, Stash's form, and not the king's square; the king is
  out of this step and recorded as unmeasured (DEC-326 (2)). The same release also added
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

Before S121: `mobility_mg` and `mobility_eg` in src/evaluation.cpp, four
weights each, over a raw count with no exclusions. The step replaces both with
a table indexed by piece type and by count -- knight 0 to 8, bishop 0 to 13,
rook 0 to 14, queen 0 to 27 -- and the count is taken over a mobility area.
Since phase 1 (2026-10-28) the table is `src/evaluation.cpp`
`mobility_score`, packed from the eight `mobility_<piece>_<half>` arrays.

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
  **Corrected 2026-10-28:** S276 deleted the mirror; S262's trace and its
  exact-reconstruction test discharge the clause, as the 2026-10-08 amendment
  says, and the table's meaning is held cell by cell against the walk in
  `tests/test_walked_counts.hpp`.
- The mobility area is computed once per side in the loop that already walks
  the pieces; the enemy pawn-attack span is the same fill the pawn terms build,
  which is the argument S102 makes for its own reuse. **Corrected
  2026-10-28:** the area needs the pawn *attacks*, not the attack span, and
  stage one's sets are locals of `evaluate_pawns()` (specs.md), so stage two
  builds its own: `neighbour_files()` of a side's pawns and one shift, two
  operations a side (`adocs/data/S121_parts.patch`). No set crosses the
  stages; S123's reading that it could is S123's to correct.
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
- **The lazy clamp caps this term together with king safety.** S039 measured
  retiring it with the shortcut and read H0 (record `ca578de`, revert
  `74d0adf`), so the clamp stays and this step's larger mobility values enter
  the clamped sum; S122's design waits on the owner's amendment before this
  step starts (DEC-258).

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
- **S039 (done, H0, record `ca578de`)**: the clamp on the sum this step
  enlarges is kept.
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

## 2026-10-28: phase 1a (blind)

Written by one Opus 5.5 subagent in the worktree `../chesso-s121` (branch
`s121`, from `adf77d0`) while S136's SPRT held every core: nothing was built,
compiled, tested or timed (DEC-260). Two python one-offs ran, each in seconds:
`adocs/data/S192_anchors.py` against the identity table reads **10 of 10
reproduced**, and `adocs/data/S121_verify_fit.py` with `.tuning/apply_fit.py
--dry-run` over a header extracted from this tree reads **955 parameters
compared, 955 identical** and places all eight tables. Phase 1b (build, tests,
INV-6, the coverage rows, the rebase onto S136's close) follows by message.

### The rulings, as read (DEC-326, the coordinator's, 2026-10-28)

1. **Parts, fixed order, earlier survivors live** (amended by the
   coordinator the same day: P2 split). P0 the curve, a table over today's
   `attacks & ~own`; P1 squares attacked by enemy pawns excluded; **P2a** the
   area replaces `~own` with own blocked pawns excluded -- own pieces and the
   other own pawns now count; **P2b** own low-rank pawns excluded as well; P3
   a pinned piece counts only its pin line. Order P0, P1, P2a, P2b, P3. The
   accepts' "each exclusion screened separately" holds: blocked and low-rank
   are two parts. **The switch that lets own pieces count is inseparable from
   the first own-pawn exclusion**: over `~own` an own-pawn exclusion is a
   no-op, so the switch is P2a's price, read as the accepts' own form. **P2b's
   predecessor is P2a if P2a survives; if P2a is dropped, P2b is screened as
   the area with both own-pawn exclusions against the last survivor**, so
   low-rank pawns are never measured over `~own`. The area and the pins
   change the mobility count only: king safety's `hits` keep the raw attack
   set. The king's square is unmeasured.
2. **Definitions from the description (DEC-221).** Blocked: an own pawn whose
   push square holds any piece. Low-rank: an own pawn on its side's second or
   third rank. The own king's square is not excluded. All three are written
   in `adocs/data/S121_parts.patch` and walked independently in
   `tests/test_walked_counts.hpp` under the same mask.
3. **Gain.** P0 by DEC-315 against its start, the shipped evaluation through
   the identity seed. P1 to P3 paired by sample, d_x = best(predecessor, x) -
   best(part, x), kept iff the full d > max(1.645 sd(d_q), 2e-6), the
   predecessor the last survivor. **One addition, the ruling left it open:**
   K is fitted once per sample at the shipped constants and passed to every
   fit on that sample by `--k`, so a paired difference is read on one scale;
   a part fitting its own K at an identity start over another count would
   move the scale the losses are compared on. If P0 is dropped, P1 to P3 are
   screened over the linear model -- every cell c times its count-1 cell,
   expressed as a tie list with scales, `S121_screen.py linear` -- against
   `linear_m0`, the linear model refitted over today's count, which `screen
   P1` fits first: fitted against fitted, the coordinator's confirmed reading
   of "against the shipped linear", so both ends of a paired gain are fits.
   A linear candidate ships lround(c x w) per cell, w the fitted real weight
   of its row, not c x lround(w): a table within half a centipawn a cell of
   the linear term, every entry a fitted value.
4. **Seed: the identity table**, cell[t][c] = c x the shipped weight, S083's
   own fit, DEC-134 (b). Section 4's "No seeds" and pitfall 1 ("the linear
   weights are not a starting point") are overruled by DEC-326 (4): the seed
   is what makes phase 1 bit-identical and DEC-315's start the shipped
   evaluation; pitfall 1's concern is answered by convergence, and the read
   prints per fit the epoch of its best report and whether it stopped on
   patience or at the 5000-epoch cap.
5. **Every entry fitted (DEC-084): ties.** Threshold **1000 games** a cell
   half (`TIE_GAMES`, `S121_screen.py`): the rows putting a non-zero
   coefficient on the cell at a phase giving the half a taper weight, counted
   by the games they come from (feature_audit's new `support` lines). An
   unsupported half is tied to the nearest supported cell of its piece and
   half, the lower count on a tie, one shared parameter; a piece and half
   with no supported cell stops the screen. Tying is implemented, not
   proposed: `tools/tuner_groups.hpp` `parse_ties`, `fold_ties`,
   `copy_ties`, `tuner --tie FILE`, with a test in `test_tuner_groups`. The
   tuner ties the cells once its start figures and start vector are taken at
   the shipped constants and before the first gradient (the fast check's
   correction), so DEC-315's start is the shipped evaluation and every step
   is on the tied parameterisation; a non-finite scale is refused.
   The list with each tied cell's rows and games is written by `audit`
   before any fit. Why games: the label is a game's, and an optimiser that
   steps every coordinate at its learning rate fits a cell a few games reach
   to those games' results. At 1000 games the standard error of a game-mean
   result is about 0.014, near 13 cp at the sigmoid's centre; a choice, not a
   measurement.
6. **No test exempted or weakened.** The coverage cases get dedicated rows,
   found by `adocs/data/S121_coverage_rows.cpp` (the rule is in its header:
   greedy over `all_test_fens()`, then S083's corpus for what stays open,
   each pick a position and its mirror, phase 1 to 23, clear of the clamp),
   run in phase 1b. **The constructive fallback** (the coordinator's, after
   the fast check): a cell neither source closes -- a queen on 22 to 27, a
   rook on 14, a bishop on 13 are the likely ones -- is constructed by
   `adocs/data/S121_construct_rows.py`, a deterministic seeded search over
   legal positions (a White piece, both kings, up to six White pawns) whose
   count under the tree's mask is the cell's, each `is_valid()` by
   python-chess and White not in check; its file is the coverage tool's third
   source, held to the same eligibility by the engine's own trace -- phase 1
   to 23, clear of the clamp -- so a count the generator got wrong closes
   nothing. A smoke run at phase 1a (seconds, 20000 draws a cell) built
   queen 27, rook 14, bishop 13 and knight 0 at mask 0, and found no knight
   on 0 squares at mask 15 in that many draws. Only a cell still unclosed
   after both stops phase 1b for an owner ruling. **What replaces a planted-vector check** (132 cells on 42
   rows cannot be recovered): (a) the per-cell meaning check against the
   independent walk, every cell of every position; (b) the
   finite-difference gradient check over all 955 parameters with every
   mobility cell non-vacuous; (c) the exact reconstruction, clamp included,
   on every corpus row the tuner loads; (d) INV-6 at the identity seed, the
   table pricing every bench node as the linear term did; (e) per variant,
   the audit's rank 908 - Z with the eight levels and nothing else, the
   identification a plant would vouch for. A planted-curve recovery on the
   corpus (feature_audit `--plant` taught a mobility group) is the stronger
   form and is not built; it is the next answer if a fit's tables look wrong.
7. **The parts are a patch; src/ stays single.** **Deviation, one patch and
   not three:** cumulative P1/P2/P3 patches cannot express a part screened
   without a dropped predecessor, which ruling 3's "the predecessor is the
   last survivor" requires. `adocs/data/S121_parts.patch` adds all four
   behind `S121_PARTS`, a build-time mask (P1 1, P2a 2, P3 4, P2b 8; 8 is
   accepted only beside 2, a static_assert, so P2b is always read inside the
   area), 0 meaning the phase 1
   landing node for node, and `S121_COST`, the cost-only form of the same
   mask. A variant is the patch plus `-DS121_PARTS=<mask>`, built in a
   throwaway worktree of HEAD. The both-colours pin function,
   `pinned_pieces()`, is in the patch's `src/bitboard.{hpp,cpp}`. At a
   landing the mask is set in `src/evaluation.hpp` for the SPRT and folded at
   the closing commit, INV-6 against the SPRT's build.
8. **The partition**, pre-registered in `S121_screen.sh` and `S121_sprt.sh`:
   A the curve | B1 the enemy-pawn exclusion | B2a the area replacing `~own`
   with blocked pawns excluded | B2b low-rank pawns excluded as well | C the
   pin rule, in that order, each its own `{0, 5}` SPRT, survivors only: at
   worst one family SPRT and five part SPRTs, six in all (five before the
   split), each up to 19.86 h. A plays P0's full fit against the phase 1
   landing. B2b always carries the area, landed or not. B1, B2a, B2b and C
   play their
   count over what has landed by then: the table refitted when A landed, the
   linear model refitted over the new count when it did not (`S121_screen.sh
   part NAME LANDED CURVE`).
9. **The clamp.** At `apply`, on the pasted tree's own tools over the corpus:
   the share of rows the stage-two clamp binds on (feature_audit's load line)
   and `tools/eval_spread`'s mobility p99 and maximum. **Bound: 1.0 % of the
   corpus's rows**, more than twice the shipped 0.338 to 0.462 % (DEC-324);
   past it `apply` prints STOP and phase 2 stops for the coordinator. S122
   inherits both readings.

### The code at identity

- `src/evaluation.cpp`: `mobility_mg[4]` and `mobility_eg[4]` become eight
  one-dimensional tables, `mobility_<piece>_<half>[w]`, w = 9, 14, 15, 28,
  each cell c x the S083 weight; one packed `mobility_score[66]` built at
  compile time; the loop reads `mobility_score[MOBILITY_OFFSET[type] +
  count]` and traces `(TERM_MOBILITY, cell, mover)`, one entry a piece as
  before. The overflow assert is now 32 x the largest entry of the eight.
  Packing is exact under integer multiplication, so the packed sum of the
  cells is the packed sum `count * weight` was, bit for bit: INV-6, `No
  functional change`. INV-4 is untouched: mobility was recomputed by design
  and the parts are recomputed beside it in stage two, paid only where the
  shortcut fails.
- `src/evaluation.hpp`: `MOBILITY_WIDTH`, `MOBILITY_OFFSET`, `MOBILITY_CELLS`
  and the eight `extern` tables; `mobility_score` is 66 wide.
- `tools/tuner_layout.hpp`: `MOBILITY_COUNT` = 66 from the engine, the
  vector 955, `starting_params()` reads the eight tables;
  `tools/tuner_trace.hpp` changes one comment. `tools/tuner.cpp` emits the eight
  tables by the engine's widths and takes `--tie`; its header gains a `tie`
  line. `tools/tuner_groups.hpp`: `free_mask("mobility")` is unchanged code,
  132 wide; the ties.
- `tools/feature_audit.cpp`: cell names (`mobility_mg[queen][27]`); the eight
  mobility levels as dependencies by construction, verified on every row,
  each dropping one cell from the reduced design; `--free GROUP`, the free
  set the report is read for: a level counts as held only where the group
  freezes its row or its table half, each such level on its own line, and a
  level free on both sides fails the span check exactly as the old guard did
  (the fast check's correction: the first draft counted all eight as spanned,
  which would have passed S126's `--only all` with eight unheld directions,
  S275's drift); the `support` report. **The tuner refuses** a free set that
  frees both sides of a level (`tools/tuner_groups.hpp`
  `mobility_levels_held`, tested in `test_tuner_groups`), `--only all` and
  `--freeze tempo,piece_placement` among them; S126 must hold or freeze the
  levels by design.
- Tests: `test_walked_counts.hpp` walks a histogram per side and cell;
  `test_eval_trace` checks every cell; `test_evaluation` checks the four
  tables' packed halves; `test_tuner_groups` holds the ties. `S192_anchors.py`
  reads each piece's cell from its table.

**The eight mobility levels.** Every piece adds exactly one cell of its row,
so per type and half the cells summed and the type's table squares summed
carry the same coefficient on every row. The 831-wide design read rank 792 of
831; the expected rank is **908 of 955, less one for every mobility column no
row reaches** (Z): 124 columns more and 8 null directions more. Under `--only
mobility` the tables are frozen and the fit is identified; S126's joint refit
meets the eight as directions only the clamped rows decide.

### Unverified, all of it to be shown in phase 1b

Every line of C++ above and in the patch is uncompiled; the clang-format
layout of every hunk is unchecked (the patch must be re-cut clang-format
clean, since `S121_screen.sh apply` formats the tree and `S121_sprt.sh`'s
guard compares CAND with REF plus the patch); no test has run; the INV-6
claim is an argument; the coverage tool has never run, so the number of rows
it adds and whether every cell can be closed from `all_test_fens()` and S083's
corpus are unknown (a queen on 27 squares at phase 1 to 23 clear of the clamp
may not exist in either); the audit's 908 - Z is arithmetic; the tie
threshold's effect on how many cells tie is unknown; the shell scripts are
`bash -n` clean and untested; `S121_screen.py`'s plan, ties, audit-check and
read were exercised on synthetic files only.

### Commands for the coordinator, in order, with expected times

Phase 1b, after S136 closes (agent work, the machine idle):

1. Rebase `s121` onto S136's close. Expected textual conflicts on H1:
   `tests/test_eval_trace.cpp` (S136's bound and pin cases against this
   step's per-cell check -- separate hunks), `adocs/data/S192_anchors.py`
   (the anchors' numbers against the table lookup), `adocs/data/README.md`
   (both append). `tests/test_evaluation.cpp` and `tests/test_search.cpp`
   only if S136 moved lines near `halves_match`. Then `git apply --check
   adocs/data/S121_parts.patch` on the rebased tree; re-cut it if it refuses.
   10 min.
2. Both builds and both fast suites, the gate command of AGENTS.md TESTS.
   About 2 min build, 4 min gate. Expected red until step 4: "the positions
   move every term both ways" and "gradient() is the derivative of the error
   it descends" (non-vacuity), on the cells the 42 rows never reach. Every
   other case is expected green; a red elsewhere is a defect, fixed first.
3. INV-6: `chesso bench` total and `bench 12` identical to the parent's;
   `tools/search_bench.py` depths 9 and 12 node counts and best moves
   identical. 2 min.
4. The coverage rows: build and run `adocs/data/S121_coverage_rows.cpp` by
   its header (`S121_COVERAGE_DATA=` S083's corpus), about 2 min; append its
   pairs to `tests/test_eval_positions.hpp` with a comment block naming the
   rule. If it prints UNCLOSED, run `~/.venv/chess/bin/python -I
   adocs/data/S121_construct_rows.py --cells <the unclosed cells>` (seconds
   to minutes, 200000 draws a cell at most) and the coverage tool again with
   `S121_COVERAGE_CONSTRUCTED=` its output; still UNCLOSED stops phase 1b for
   an owner ruling.
5. Every precondition the rows move, each by its own script: the clamp binding
   both ways in "the trace reconstructs evaluate() exactly" (the case itself);
   "the fixture sits clear of the clamp" (the case); the clamp derivative's
   push and the gauge walk's floor (`adocs/data/S083_tuner_preconditions.cpp
   both`); the gradient case's worst relative error under 1e-6 (DEC-316: a
   near-stationary parameter is answered by rows, `S135_gradient_rows.cpp`'s
   rule retargeted at that parameter); the planted recoveries of
   `test_tuner_gradient`; `test_eval_model`'s coverage cases, which read the
   same list. 20 min.
6. Re-cut `adocs/data/S121_parts.patch` clang-format clean: apply it to a
   scratch worktree, `./clang-format.sh`, build that tree at masks 1, 2, 10,
   4 and 15 and at `S121_COST=15` (each must compile, -Werror; mask 8 alone
   must not), run its `test_eval_trace` and `test_walked_counts`-reading
   cases at masks 10 and 15 (the
   walk against the engine under every part), and at mask 0 INV-6 against
   step 3's streams; then `git diff` the scratch tree into the patch. 25 min.
7. `.tuning/verify_fit.py` replaced by `adocs/data/S121_verify_fit.py`
   (gitignored, so a copy). `plan_prose_check --citations`: S101's step file
   cites `mobility_mg`, which no longer exists -- S101's to correct, or a
   citation the check is told about.
8. Land `No functional change`, the commit's `Bench:` the parent's total.

Machine runs, the coordinator's (phase 2 inputs):

9. `adocs/data/S121_screen.sh all` detached, watched for `S121-ALL-(DONE|
   FAILED)`: five parts, about 1 h 30 m typical, 2 h 55 m at the cap, 3 h
   25 m if P0 is dropped; ceiling 6 h 30 m. Daytime (DEC-155). Or stage by stage: `locate`
   (under a minute), then per part `audit` (about 5 min with the variant's
   build), `screen` (about 14 to 30 min), `read` (seconds).
10. `CAND_SHA=<phase 1 landing> adocs/data/S121_cost.sh run` detached: about
    1 h 35 m (seven comparisons: aa, parent, p1, p2a, p2b, p3, all), ceiling
    3 h 10 m, the machine otherwise idle; may run beside
    the reading of the screen, never beside a fit.
11. Phase 2 (agent): `adocs/data/S121_screen.sh apply <tree>` (about 8 min):
    the paste, the mask, both builds, bench, both fast suites with their reds
    recorded, the clamp's two readings and the STOP line if the share passes
    1.0 %. Then every golden by its script (S135's table: about 2 to 3 h of
    agent work and a 450 s chain), the truncation pins by DEC-322's
    machinery on S136's H1 (one taper still for mobility, so N does not move
    because of S121), the preconditions of step 5 again, the coverage rows
    again where a mask changed the counts, the gates, INV-6 of nothing (it is
    a play change).
12. `adocs/data/S121_sprt.sh`, pinned by the coordinator: 41900 games worst,
    19.86 h; a night.

### DEC-326 as drafted for the coordinator (the record is the coordinator's)

- **Context.** S121's accepts wants a fitted per-count table, an area with
  two own-pawn exclusions and an enemy-pawn one, a pin rule, each screened
  apart before one family SPRT (DEC-259). The research pass (2026-10-28)
  found the own-pawn exclusions a no-op on today's `attacks & ~own`, the
  terms "blocked" and "low-rank" undefined, section 1's king square
  contradicting the accepts, DEC-315's gain undefined for two fitted ends,
  pitfall 1 against the only neutral seed, cells the corpus barely reaches,
  and two coverage tests that the table turns red.
- **Decision** (the coordinator, 2026-10-28, under the owner's standing
  instruction). (1) Parts in fixed order, earlier survivors live: P0 the
  table over `attacks & ~own`; P1 enemy pawn attacks excluded; P2a the area
  in place of `~own` with own blocked pawns excluded, the switch that lets
  own pieces count being inseparable from the first own-pawn exclusion and
  so P2a's price, read as the accepts' own form; P2b own low-rank pawns
  excluded as well, its predecessor P2a if P2a survives, and if not the area
  with both exclusions against the last survivor, so low-rank is never
  measured over `~own`, where it is a no-op; P3 the pin rule; area and pins
  change the mobility count only. (2) Blocked: any piece
  on the push square; low-rank: the side's own second and third ranks; the
  own king not excluded, recorded unmeasured. (3) P0 by DEC-315; P1 to P3 a
  gain paired by sample against the last survivor, kept iff the full d >
  max(1.645 sd(d_q), 2e-6), one K a sample; over the linear model if P0 is
  dropped. (4) The identity seed, DEC-134 (b), overruling section 4's
  "No seeds" and pitfall 1; phase 1 lands `No functional change`. (5) Cells under 1000 games a half tied to the nearest supported
  cell of the same piece and half, written before any fit, and tied before
  the first gradient. (6) Coverage by rows found by a tool, constructed by a
  second tool where no corpus row serves, each pick within 800 cp of level
  (DEC-316's non-vacuity class) and loadable with either side to move
  (`test_eval_model`'s REQUIRE), the tool re-deriving its block from the 42
  rows before it (DEC-142); every precondition re-checked by its script; the
  planted check replaced by the five checks this file lists. (7) The parts
  as one throwaway patch behind a build-time mask, `src/` single; the cost
  by cost-only builds of the same patch. (8) Bisection A | B1 | B2a | B2b |
  C, each its own `{0, 5}` SPRT, survivors only, a part without A playing a
  linear refit over its count; six SPRTs at worst. (9) The candidate's clamp share bounded at 1.0 % of
  the corpus's rows, past which phase 2 stops; its mobility p99 and maximum
  recorded for S122.
- **Why.** Each part is then a measured decision with a defined predecessor,
  the landing is provably neutral, and no shipped entry is unfitted.
- **Rejected.** Keeping `~own` and dropping both own-pawn exclusions as
  vacuous (the accepts names them); one P2 bundling both own-pawn exclusions
  (the accepts screens each, and the first draft did so; the coordinator's
  amendment the same day); P2b over `~own` when P2a is dropped (a no-op); a count cap at the supported width (it
  ships the cap's cells unfitted, and a tie does the same work with every
  cell fitted); three cumulative patches (they cannot screen a part whose
  predecessor was dropped); a compile-time selector in `src/` (deliberately
  different code, DEC-118's class); the √2 of DEC-304 (both ends are fitted
  on the same rows and paired, so their shared noise cancels).
- **Consequences.** `PARAM_COUNT` 831 -> 955; feature_audit's rank 908 less
  the unreached columns, read `--free mobility`; eight new null directions
  no gauge holds, so the tuner refuses a free set that walks one and S126's
  joint refit must hold or freeze them; `tuner --tie`; the fixture gains
  rows, constructed where no corpus row serves; S122 inherits the clamp
  readings.

### Wording proposals

`adocs/specs.md`, the "absent, evaluation model" row, at the phase 1 landing:
"mobility is **linear**, one weight per piece type over a raw count with no
exclusions, ... The fit returns knight mobility at -1 middlegame and 0
endgame, rook at 0 endgame and queen at -6, ..." becomes "mobility is a
**table per piece type indexed by its count** (S121, 2026-10-28) seeded at the
identity, c times S083's linear weights, so it scores as the linear term did
until S121's screen fits it; still no exclusions and no pin rule", and "Also
absent: a per-count mobility table," loses that item. The evaluation row's
"All 825 shipped constants fitted" becomes "All 949 shipped constants fitted
or, for S121's 132 mobility cells until its screen, c times a fitted weight",
825 - 8 + 132 = 949 (951 if S136's tempo lands). The "absent, evaluation"
row's "the tuner carries their 6 parameters, making 831 fitted and 825
shipped" becomes "... making 955 fitted and 949 shipped". On the screen's
verdict, the evaluation row gains the parts that landed in the words of
`src/evaluation.hpp`'s definitions.

## 2026-10-29: phase 1b

S136 closed H0 (`f24ec17`): tempo stays 0 / 0, N = 3, the truncation guard
and `EXACT_FLOOR` 159 as they are, DEC-325 moot. Built in a fresh worktree,
`../chesso-s121b` (branch `s121b` on `6ff7485`), the phase 1a diff (sha256
`b6c14518...`) applied by `git apply --index --3way` with the old
`plan_todo` copy removed by hand -- its one change since `adf77d0`, the DEC-324
sentence, already in this file -- and `adocs/data/README.md`'s append conflict
resolved by keeping S136's rows and adding S121's after them. S297's change to
`tools/tuner_trace.hpp` merged cleanly. `../chesso-s121` stays as it was.

### Builds, tests, INV-6

- Both builds compile with no warning. Before the coverage rows both fast
  suites read 48 of 50 green, the two red cases the expected ones and nothing
  else: "the positions move every term both ways" on one feature, queen count
  27, and the gradient case's non-vacuity on 55 parameters.
- **INV-6, against a Release and a tune build of `6ff7485`**: `bench`
  5069022 in both builds, both trees' `bench` and `bench 12` streams
  byte-identical once timing is stripped, `tools/search_bench.py` node counts
  and best moves identical at depth 9 (48284, 126091, 62646) and depth 12
  (226320, 430872, 157130). The table at identity prices every node as the
  linear term did: `No functional change`. The streams and their md5 are in
  `adocs/data/S121_inv6.txt`.

### The coverage rows

`adocs/data/S121_coverage_rows.cpp` over S083's corpus (sha256 `2e0e3de9...`)
closed every one of the 55 open needs from `all_test_fens()` (13 pairs) and the
corpus (6 pairs), 19 pairs, 38 rows, the fixture 80. **The constructive
fallback was not needed**: nothing was UNCLOSED. **The rule was amended twice
after it ran, before any pick was kept**, each time because the program's own
check or the suite refused its picks, each amendment stated in its header:

1. The first run's gradient check read four parameters at or under 1e-12
   (knight count 1 and bishop count 11, both halves, 2.8e-19 and 1.2e-14): the
   pairs that closed them were lopsided, a full army against a bare king and
   nine bishops, where the case's sigmoid is flat. A pick and its mirror must
   now score within 800 cp of level at the shipped constants.
2. The second run's rook-13 pick, `1k3R2/8/8/8/8/8/8/4K3 b`, has Black in
   check, and `test_eval_model` "the move is worth the same to either side"
   REQUIREs every row to load with either side to move. A pick and its mirror
   must now load both ways. The third run changed that one pick and nothing
   else.

The third run: every parameter from `MOB_MG_BASE` on with |gradient| above
1e-12, the smallest 4.9e-08, and every feature outside the tables moved both
ways over 2856 positions. Every new row is VALID by python-chess.

**The tool re-derives its own block (DEC-142; the phase 1b fast check).** It
now starts from the fixture's first 42 rows (`BASE_ROWS`), asserted against
the block's marker comment, and not from the landed 80. It holds every landed
pair to `eligible` again, and it exits 1 unless its picks are the landed
block. The fourth run, the one recorded, reads 80 rows and 42 before the
marker, 19 landed pairs with 0 ineligible, and "reproduces the landed block:
yes, pair for pair and in order".

`adocs/data/S121_preconditions.txt` holds that run, run 1's picks with their
scores (the two that closed the flat cells score +5646 and +3767), run 2's
in-check pick, and the python-chess validity run.

### The preconditions the rows move, each by its script

- The clamp binding both ways in "the trace reconstructs evaluate() exactly":
  the case's own precondition, green.
- "The fixture sits clear of the clamp": green; the largest |stage two| over
  the 80 rows is 99.3, under 183.
- **The clamp case's push, re-derived** by `adocs/data/S083_tuner_preconditions.cpp
  both`: the rule now answers **+140**, where the case carried +100. At +100
  the finite difference reads 7.93e-7, past the rule's 2e-7. At +140, 34 rows
  bind, the farthest at 1066.7, the nearest other at 174.7, every row 9.33
  from the kink, finite differences 1.58e-7. The case's input and comment are
  updated. This is the input's re-derivation by its stated rule, not a
  relaxation: the 1e-6 the case asserts is unchanged.
- The gauge walk: smallest plain shift 0.6177 under Adam and 0.9240 under
  AdaGrad, both the rook's, against the floor 0.01; every hold at most
  5.66e-15. The case's comment records them, and its stale "158 of the 768
  columns" now reads 281 over the 80 rows (173 over the base 42).
- The gradient case's worst relative error, DEC-316: 5.46e-08, at parameter
  141, `psqt_mg[bishop][a7]`, against 1e-6. The nonlinear group's reads
  2.29e-08.
- The planted recoveries in `test_tuner_gradient` and `test_eval_model`'s
  coverage cases: green.

### The parts patch, re-cut and built

`adocs/data/S121_parts.patch` was applied to a copy of this tree,
clang-formatted (three files moved), and re-cut. It applies to this tree.

- **Build matrix.** Built at `S121_PARTS` 1, 2, 10, 4 and 15 and at
  `S121_COST=15`: each compiles under `-Werror` with no warning. Mask 8 alone,
  and `S121_COST=8`, fail on the static_assert "P2b excludes low-rank pawns
  from P2a's area and needs it", as they must.
- **The walk against the engine.** At every one of those masks,
  `test_eval_trace` "every coefficient is the count its index names" is green
  over 2856 positions: the walk in `tests/test_walked_counts.hpp` agrees with
  the engine cell by cell under P1, P2a, P2b and P3. So are the exact
  reconstruction and "tracing changes nothing".
- **INV-6 where it must hold.** At mask 0, `bench`, `bench 12` and
  `search_bench` at depths 9 and 12 are identical to the landing's. The
  binary's `.text` differs only by `pinned_pieces()`. `S121_COST=15` benches
  5069022, identical too.
- **Parts that change play, as they should.** The variants bench m1
  4240116, m2 5944311, m10 6685997, m4 4942573 and m15 6729431.

### Gates

Both fast suites are green, 50 of 50 in `build/` and in `build-tune/`, and
`./clang-format.sh --check` is clean. Each mode of `tools/plan_prose_check.py`,
run one per call, exits 0: `--prose` 0 sentences flagged, `--citations` 0
over 21 files, `--touches` 0, `--params`, and `--gate`, whose command agrees
in both documents. S101's citation was fixed on main. The S297 clang check
(DEV_MANUAL's Build section, `clang++-22`, Release, `-k 0`) builds
`build-clang` and `build-clang-tune` with no error and no warning, and both
fast suites there are green, 50 of 50. The parts patch also builds under
clang-22 at `S121_PARTS=15` and `S121_COST=15` with no warning. **Tier 2 (DEC-141) does not bind**: no change touches
`make_move`, `unmake_move`, the generator or the search. The evaluation's
mobility term changes representation alone, proved by INV-6. The patch's
`pinned_pieces()` reads the generator's pin helper without changing it, and
it is not in `src/`.

No `done:` stamp: the step continues with the screen
(`adocs/data/S121_screen.sh`, the coordinator's).
