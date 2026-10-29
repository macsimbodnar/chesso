id:         S121
goal:       mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on, and a pinned piece counts only the moves along its pin
accepts:    an SPRT verdict, recorded whatever it is; mobility is a table indexed by piece type and by count -- knight 0 to 8, bishop 0 to 13, rook 0 to 14, queen 0 to 27 -- and **every entry is fitted by our own tuner on our own corpus** (DEC-084); the mobility area excludes squares attacked by enemy pawns and the side's own blocked and low-rank pawns, and each exclusion -- and the pin rule this family gains, a pinned piece counting only moves along its pin ray (DEC-258) -- is screened separately before the family's one SPRT (DEC-259); the tuner's model in tools/eval_model.hpp is updated in the same commit and test_eval_model holds the two against each other; the nps cost is recorded next to the verdict
touches:    phase 1 (2026-10-28, the table at identity, `No functional change`): src/evaluation.cpp, src/evaluation.hpp, tools/tuner_layout.hpp, tools/tuner_groups.hpp (the ties), tools/tuner.cpp (the emit, `--tie`), tools/feature_audit.cpp (cell names, the eight mobility levels, the support report), tests/test_walked_counts.hpp, tests/test_eval_trace.cpp, tests/test_evaluation.cpp, tests/test_tuner_groups.cpp, tests/test_eval_positions.hpp (the coverage rows, phase 1b), tests/test_tuner_gradient.cpp (the clamp case's push re-derived, phase 1b), adocs/data/S192_anchors.py, adocs/data/S121_preconditions.txt, adocs/data/S121_inv6.txt, DEV_MANUAL.md, adocs/data/S121_{parts.patch,screen.sh,screen.py,cost.sh,cost.py,sprt.sh,coverage_rows.cpp,construct_rows.py,verify_fit.py}, adocs/data/README.md, this file; tools/tuner_trace.hpp (a comment only: its mobility line reads MOBILITY_COUNT, now 66); phase 2 (2026-10-29, the candidate at S121_PARTS 5): src/evaluation.cpp's eight tables and whole-line comments, and through adocs/data/S121_parts.patch src/evaluation.hpp (the mask line and a whole-line comment), src/bitboard.{hpp,cpp} (`pinned_pieces()`), tests/test_walked_counts.hpp and adocs/data/S192_anchors.py (its anchors re-derived too), folded at the closing commit; every golden the paste moves, each by its script: tests/test_evaluation.cpp (the anchors), tests/test_search.cpp (the anchors, the ProbCut row re-mined, ALPHA's comment), tests/test_eval_trace.cpp (the truncation pins), tests/test_eval_positions.hpp (the coverage block re-derived), tests/test_tuner_gradient.cpp (the clamp push, the walk's comment), tests/test_mate_breadth.cpp (its stale header and S121's reading), tests/test_mate_pv.cpp (the aborted-line cell's budget), tests/test_mate_carry.cpp and adocs/data/S170_cases.tsv (B's ceiling and four budgets, DEC-327, provisional); `.tuning/verify_fit.py` (tracked, not gitignored as phase 1 wrote: adocs/data/S121_verify_fit.py copied over it); adocs/data/S121_sprt.sh (a dated paragraph, the run count); adocs/data/S121_parts.patch (its preamble, before the first `---`); adocs/data/S121_{screen_reading,cells,goldens,cost_reading,sweep_s170,case_b}.txt, adocs/data/S121_{cells,case_b}.py, adocs/data/S121_alpha_window.cpp, adocs/data/README.md; DEV_MANUAL.md, MANUAL.md; this file; adocs/specs.md (the coordinator's); the close (2026-10-29, H1): src/evaluation.{hpp,cpp} (the fold: P1 and P3 unconditional, P2a, P2b and the masks gone), tests/test_walked_counts.hpp and adocs/data/S192_anchors.py (folded the same way), tests/test_mate_carry.cpp and adocs/data/S170_cases.tsv (DEC-328's wording), comments in tests/test_mate_breadth.cpp, tests/test_eval_positions.hpp, tests/test_tuner_gradient.cpp, tests/test_evaluation.cpp and tests/test_eval_trace.cpp, adocs/data/S121_{sprt.log,sprt_mate_pvs.txt,sprt_mate_pvs.py,fold_inv6.txt}, adocs/data/S121_parts.patch (its preamble), adocs/data/S121_cost.{sh,py} (the p5 row, `ROWS`, two guards), adocs/data/S121_screen.sh (the fold guard), adocs/data/S121_cost_reading.txt (the p5 reading, after the coordinator's timing), adocs/data/README.md, DEV_MANUAL.md, this file; src/bitboard.{hpp,cpp} unchanged at the close
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
   (a copy; **corrected 2026-10-29**: `.tuning/verify_fit.py` is tracked,
   `.gitignore`'s `!.tuning/*.py`, not gitignored, so phase 2's commit
   carries the copy). `plan_prose_check --citations`: S101's step file
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

## 2026-10-29: the screen and the candidate -- stopped at the S170 ceilings

One Opus 5.5 subagent, worktree `../chesso-s121c`, branch `s121c` at
`12cab3c`, the phase 1 landing. Written in two halves: the reading while the
coordinator's cost run held the machine, the runs after its "go" and its
three rulings (`.tuning/coord/S121/rulings_phase2.md`). Every reading with its
command is in `adocs/data/S121_goldens.txt`. No match, SPRT, fit, datagen or
timing ran. **Stopped at the S170 ceilings** (DEC-299's guard, below); every
other golden is re-derived and the tree is green.

### The screen, as read

`adocs/data/S121_screen.sh all`, 2026-10-29 02:32 to 04:05, `S121-ALL-DONE`,
on S083's corpus (6226944 rows, sha256 `2e0e3de9...`) against S082's qs
validation view. The five readings, the corpus line and every fit's sha256
are in `adocs/data/S121_screen_reading.txt`.

| part | against | full d | s | m | verdict |
|---|---|---|---|---|---|
| P0, the table over `attacks & ~own` | its start | +0.000066 | 0.000011 | 0.000018 | kept |
| P1, enemy pawn attacks excluded | P0 | +0.000021 | 0.000005 | 0.000009 | kept |
| P2a, the area, own blocked pawns excluded | P1 | +0.000001 | 0.000004 | 0.000006 | dropped |
| P2b, low-rank pawns as well, inside the area | P1 | -0.000017 | 0.000006 | 0.000010 | dropped |
| P3, the pin rule | P1 | +0.000028 | 0.000002 | 0.000003 | kept |

- Survivors P0, P1, P3: `CANDIDATE table_m5 mask 5`. The candidate is
  `.tuning/s121/fits/table_m5/full.hpp` (sha256 `cd5761cd...`): best
  validation 0.071617 from 0.071739, at epoch 600, stopped on patience.
- The bisection order printed: A | B1 | C.
- Ties at mask 5: 4 of 132 cell halves. Queen 26 and 27 are tied to 25 in
  both halves (rows 359 and 305, games 256 and 220).
- **The cap, as read; it decides nothing.** Every q5 fit stopped at the
  5000-epoch cap, and so did q4 for m3 and m11. Every full fit stopped on
  patience.
- **Thin cells.** `adocs/data/S121_cells.txt` lists all 132 cell halves,
  each with its fitted value, its gap to its neighbours' mean and its support
  (rows and games, the m5 audit's `support` lines). It is written by
  `adocs/data/S121_cells.py`. The cells the brief named are marked: queen mg
  20 to 25, queen eg 0 and 24, knight eg 8, bishop mg 11 to 13. The thinnest
  of them are queen mg and eg 24 (2814 rows, 1768 games) and 25 (1813, 1139).
  The planted-curve recovery is not built (ruling 6 names it the next answer
  if a fit's tables look wrong): the SPRT decides, and S126 inherits the list.

### The apply

`S121-APPLY-DONE` (`.tuning/s121/apply_table_m5.console`, OUT
`.tuning/s121/apply_table_m5/`).
- start-check passed; headroom 32 x 119 = 3808 of 32767.
- The parts patch applied, `#define S121_PARTS 5`.
- `verify_fit`: 955 of 955. Only the eight mobility tables moved.
- Both builds with no warning. Bench 5069022 -> **6016079**.
- 46 of 50 in each build. The reds were `test_evaluation` (the anchors),
  `test_search` (the anchors and the ProbCut row), `test_tuner_gradient` (the
  coverage non-vacuity, the clamp push's kink) and `test_eval_trace` (the
  coverage both ways on queen 27, the truncation pins).

### The clamp, which S122 inherits

The stage-two clamp binds on **22083 of 6226944 rows, 0.355 %**, against
DEC-326 (9)'s 1.0 % (the shipped 0.338 to 0.462 %). No STOP. Mobility
spread: **p99 107, maximum 229**.

### The coverage rows at mask 5, by ruling (b)

The tool's run on the landed block:
- 2 of its 19 pairs are no longer eligible at the fitted constants (pairs 6
  and 12, the seven-rook rows), and its picks differ: "reproduces the landed
  block: NO".
- From the base 42, 55 of 132 needs are open. 13 pairs come from
  `all_test_fens()` and 6 from the corpus; none is UNCLOSED.
- Gradient check: 0 parameters at or under 1e-12 (smallest 2.153e-09). Both
  ways: 0 features.

The block is replaced by those 38 rows, all VALID by python-chess. The tool,
rebuilt, then reads "reproduces the landed block: yes", 0 ineligible.

### Every golden, each by its script

| golden | before | now | script |
|---|---|---|---|
| six piece anchors | 99, 276, 355, 583, 984, 0 | **99, 283, 353, 590, 1021, 0** | `S192_anchors.py`: 10 of 10 at identity, then on the fit (mask 5 from the header), then 10 of 10 on this tree |
| `QUIET_ROOK_EVAL`, `_CHEAP`; 394 | 583, 578; 394 | **590**, 578 unmoved; 394 unmoved (mobility is stage two) | the same |
| `black_static`; quiet evasion; repetition | 199; -533; -597 | **181; -541** (leaves -572, -541, -590, -551, still the four king moves standing pat); **-603** | the same |
| `POSITIONAL_ROOM` | -- | did not fire | -- |
| `truncation_positions` | S135's four | **re-picked**: 8/3k4/... b (phase 5), 8/6k1/... w (11), 5rk1/5pbp/... b (19), 2k1r3/... w (17) | `truncation_scan --min 2.8` on the corpus, then `S083_truncation_pick.py` (ruling Q2). 77 rows past 2.8, 18 at 2.875, 16 distinct. The worst is 2.875 = 69/24, so N stays 3. S135's four read 2.625 at most |
| coverage block | phase 1b's 19 pairs | **19 new pairs** | `S121_coverage_rows.cpp` (above) |
| clamp push; walk | +140; 0.6177 / 0.9240 | **+30** (2 rows bound, 197.3, next 158.8, 13.33 from the kink, fd 9.3e-8); walk **0.1552 / 0.0671**, both the rook's, over 0.01 | `S083_tuner_preconditions.cpp both` |
| fixture clear of the clamp | 99.3 | **121.1** against 183 | the same |
| gradient check (DEC-316) | 5.46e-08 at 141 | **9.50e-07 at 849, `mobility_eg[bishop][1]`**, both builds: green by a factor of 1.05 | `test_tuner_gradient -s` |
| `EXACT_FLOOR` 159 | 160 / 159 (not separating) | **held**: **165** shipping, **154** weakened; the gate at RfpMinPly 1 is red at 154 | `S156_mined_floor_sweep.py --floor 159`, via `S136_overlay_run.py` (the paste's five files) |
| `MATE_IN_THREE_FLOOR` 11 | 13 / 8 | **held**: **12** / **10**; `red` red at 10 >= 11 and on mate-in-two | `S154_floor_margin_sweep.py floor`, `red` |
| node budget | 65024 / 3251 | **unmoved**: 8367 nodes, 0.51 of c, inside [7270, 32512] | `S192_node_budget.py` |
| first-mate; `DRIVE_DEPTH`; ply-2 | 9, 9, 5; 4; 6 | **unmoved** | `S188_repair_goldens.py first-mate`, `s207`; `S083s_rfp_ply2.py` |
| `capture_mates`; extra ply; retired multicut row | at their depths | **unmoved**: each at its asserted depth (the fourth only at 12) | `S230_mine_r01_row.py depths` |
| `mate_probcut_hides` | S261's row, depth 10, mate in 5 | **re-mined**: `8/8/1P6/P7/5k2/4p2P/2r5/6K1 b - - 0 60`, depth 10, **mate in 4**. Shipped `d9 d10`, B04 `d9`. 5 of 269 separate; the guard-mode witness fires at 8 to 10. Stockfish at 5000000 nodes reads `#+4`, and python-chess reads its line as ending in mate. Observed red under B04, green shipped | `S277_mine_mate_row.py`, six stages, guard mode. S261's row read 9, 11 and 12, not 10 |
| `LATE_CAPTURE_*` | LQ02 at 7, LQ03 at 3 | **unmoved** | `S261_mine_late_capture.py sweep`, `pick` |
| aborted-line cell | line 101 at 40000 (30000..60000) | **line 101 at 50000** (run 40000..70000; margin 10000 below, 20000 above; 2 of 318 separate). Observed red under MW02, green shipped | `S294_fallback_cell.py`, four stages |
| `ALPHA` | -2108 (band -2129 .. -1905) | **-2108 kept**: band -2163 .. -1932, 55 below and 176 above. The trace now meets the node at -2011, -2086, -2202 and -2381 | `adocs/data/S121_alpha_window.cpp` (new: S296's sweep had no tracked script; it reproduces S296 on the parent); `S136_trace_patch.py` (S295's no longer applies) |
| `short_line_ceiling` | 2, 6, 4, 1, 2, 3 | **NOT APPLIED**: the script answers 2, **13**, 4, 1, 2, 3 | `S203_case_sweep.sh --ceilings`: the eight grids plus `adocs/data/S121_sweep_s170.txt` |
| S170 budgets | A 500000, C 1500000, D 4000000, E 300000 | **NOT APPLIED**: the picker answers A 1000000, C 500000, D 1500000, E 100000; B and F unmoved | `S170_pick_budgets.sh` |
| readings | 707 mate_pv lines; defender 85/104 | 697; 84/104 | `test_mate_pv -s`; `S165_nmp_defender_sweep.py` |
| `golden_defaults`, option count, census, book | -- | unmoved by construction | -- |

**The stop.** Both floors hold or rise. B_mate6_shallow's ceiling, though,
rises 6 -> 13, by 7, against DEC-299's +2. It comes from one cell of S121's
grid: stride 1 at 1200000 nodes, 64 mate lines, 13 short. B's other cells
read 0 to 2; S296's worst for B was 6, at 4000000 nodes. By the brief, the
ceilings and the budgets are read and not applied. `tests/test_mate_carry.cpp`
and `adocs/data/S170_cases.tsv` stay HEAD's, and `test_mate_carry` is green
on the candidate at them. The owner rules.

### For the coordinator's question (DEC-265 form; the owner rules)

**Why it is needed.** DEC-299 makes a ceiling that rises by more than 2 the
owner's. No recorded decision covers a rise of 7.

**What each answer implies:**
- **(a) Apply the script's answer**: B's ceiling 13, and the four budgets.
  - Changes: `test_mate_carry` lets B report up to 13 short lines in a cell.
  - Costs: nothing.
  - Risks: the ceiling no longer catches a B case that gets worse by up to 7.
  - Leaves unmeasured: why this one cell rose.
- **(b) Hold 6 and today's budgets.** The tree is green at them, and the SPRT
  plays this tree.
  - Changes: nothing.
  - Costs: the ceiling is not the rule's answer (DEC-142), recorded as such.
  - Reverts: with the paste on H0.
- **(c) Attribute the cell by tool first, then rule (a) or (b).** This is
  DEC-305's form, as S136 used it for the floor.
  - What it reads: the 13 short lines' distances and why the walk refuses
    them, a short line being DEC-122's expected residue. Before S202 B's
    ceiling was 15.
  - Costs: about an hour of agent work.

**Recommendation: (c), then (a) if the short lines are DEC-122's residue.**
The ceiling bounds a reporting residue that an evaluation change is expected
to move. An attribution decides whether a reporting defect is behind it,
which is the one thing (a) would hide.

### Gates (on the tree as it stands)

- Both builds: 0 warnings.
- `ctest -L fast`: **50 of 50** in `build/` and in `build-tune/`, sequential.
- `./clang-format.sh --check` clean (`CLANG_FORMAT_MAJOR=22`).
- Bench 6016079 in both.

**Not run, the step having stopped:**
- the clang pair;
- `tools/plan_prose_check.py`'s four modes;
- `tools/gate_extra.sh`;
- the Debug self-play, which is the coordinator's.

**Tier 2 (DEC-141) does not bind.** The diff under `src/` is the eight tables
and two whole-line comments in `src/evaluation.cpp`, plus the patch:
`src/evaluation.{hpp,cpp}`'s parts and mask, and `src/bitboard.{hpp,cpp}`'s
new `pinned_pieces()`. That function calls the generator's `pinned_to_king`
unchanged. There is no `make_move`, `unmake_move`, generator or search
change. The brief asks for `gate_extra` anyway, for the Debug asserts and
sanitizers over the new code.

### The SPRT script

`adocs/data/S121_sprt.sh` gains a dated 2026-10-29 paragraph:
- the mask is 5;
- the guards are as written: the patch's `src/bitboard.{hpp,cpp}` come
  through the expected tree. Simulated on this worktree, `src/evaluation.cpp`
  and `src/evaluation.hpp` are the only files the check names, and nothing
  falls outside the three allowed line forms;
- the bisection is A | B1 | C, B1 and C over the table refitted if A lands
  and over the linear refit if not;
- the cost bound;
- the clamp reading.

The H0 paragraph's "at worst five more runs, six in all" now reads "had all
five parts survived; with this candidate's survivors, at worst three more,
four in all". The coordinator's markers are left unfilled, and the script
refuses on them.

### The cost (the coordinator's run)

`adocs/data/S121_cost_reading.txt`: all +3.38 %, p3 +2.20 %, p1, p2a, p2b
and parent zero. Mask 5 is not a measured row. It lies between p3 (+2.20 %)
and all (+3.38 %), with p1 + p3 in bench16 instructions (+2.42 %) as the
additive estimate.

### Also changed

- `.tuning/verify_fit.py` is the copy of `adocs/data/S121_verify_fit.py`.
  Both docstrings and the README row now say it is tracked.
- `src/evaluation.hpp`'s parts comment no longer says "never committed" or
  "0 when unset".
- `src/evaluation.cpp`'s table comment says the cells are the screen's fit.
- `tests/test_mate_breadth.cpp`'s header read 158 and S135's ends from S296
  on. It now reads 159 with S121's ends.

**Not done:**
- `DEV_MANUAL.md`'s goldens rows, gradient and walk sentences, and the
  `--ceilings` command, which also lacks S296's grid;
- `MANUAL.md`'s `RfpMinPly` row, which the paste moves (165, 165, 154
  against 159);
- the `specs.md` proposal (in the report);
- the proposed commit message, which waits on the ruling.

No `done:` stamp.

## 2026-10-29: resumed under DEC-327 -- B carried provisionally, attributed by tool

The coordinator ruled the stop in DEC-301's and DEC-304 (2)'s form:
**DEC-327** (drafted in `.tuning/coord/S121/dec327.md`, the coordinator's to
record). The script's answer is carried provisionally, B's cell is
attributed by tool before the SPRT, and the owner rules on B before an H1
ships.

### Applied, provisionally (DEC-327)

- `tests/test_mate_carry.cpp` `short_line_ceiling`: B 6 -> **13**. The
  command now lists S296's and S121's grids.
- `adocs/data/S170_cases.tsv`, by `S170_pick_budgets.sh` with a header
  paragraph: A 1000000, C 500000, D 1500000, E 100000; B and F unmoved.
- Each site cites DEC-327 ("provisional: the owner rules before an H1
  ships").
- `test_mate_carry` is green at the new values in both builds.

### Case B's cell, attributed by tool (`adocs/data/S121_case_b.txt`)

Read by `adocs/data/S121_case_b.py` on a traced build of the candidate
(`S136_trace_patch.py`), whose info lines equal the shipped candidate's.

**The cell.** Stride 1 at 1200000 nodes reads 13 short lines of 64, from two
roots:

| root | lines | reported mate | printed length | oracle |
|---|---|---|---|---|
| ply 33, White to move, best d1g4 | depths 7, 8 and 9 | 10 | 7, 9 and 9 of 19 plies | Stockfish **#+10**, at 1e9 nodes (DEC-320's deeper reading; 5e6 found no mate) |
| ply 33 | depth 10, and the aborted iteration 11 printed at 10 | 12 | 10 and 11 of 23 | the same #+10 |
| ply 34, Black to move | depths 1 to 8 | -13 | 1 to 8 of 26 | **#-9** at 5e6 nodes |

**Where each line is cut.** Every one is cut at the search's horizon. The
PV's last node is a depth-0 node, and quiescence there returns a mate score
with no line of its own:
- **11 lines** end where quiescence reads an exact TT entry holding a mate:
  - ply 34's eight read entries **stored by ply 33's search**, its aborted
    iteration 11, at the positions along ply 33's own PV, one key further
    along that PV per iteration;
  - ply 33's depth 8 and 9 lines read one entry stored by **ply 32's
    search**;
  - its depth 10 line reads one stored by its own iteration 8.
- **2 lines** (ply 33 at depth 7, and the aborted iteration) end in
  quiescence **in check**, which searches its two evasions.

**One carried line, not 13.** The 13 are not independent. Each root carries
one mate line, and every iteration's horizon reads the next entry along it.

**Mate distances: no wrong score.** The claimed side is right at both roots,
and **no reported mate is shorter than the oracle's**. At ply 33 the
engine's 10 equals Stockfish's and its 12 is longer. At ply 34 its -13 is
longer than -9.

**The parent at the same cell.** Its 1200000-node cell reads 35 lines and 1
short (ply 34, depth 4, mate -9). It finds no mate at ply 33 at all (cp 901
down to 566).

**The neighbouring cells:**

| tree | 1000000 nodes | 1200000 nodes | 1500000 nodes |
|---|---|---|---|
| parent | 0 short | 1 short | 1 short |
| candidate | 1 short | 13 short | 0 short |

### For the owner (DEC-265 form)

**Why.** DEC-299 makes a ceiling rise past +2 the owner's, and DEC-327
carries B's 13 only until the owner rules. The ruling is needed only on H1.

**(a) Accept 13.**
- Changes: the provisional ceiling and budgets become final.
- Costs: nothing.
- Risks: B can report up to 13 short lines in a cell before the test goes
  red, so a real reporting regression in B smaller than that passes. The
  other five cases' ceilings still hold theirs.
- Leaves unmeasured: whether S202's walk could certify a line through a
  quiescence TT read.

**(b) Keep 6 and fix first.**
- Changes: the walk (`complete_mate_pv()`, S202's ground, DEC-251) would
  have to extend a mate line past a horizon where quiescence answered from
  the TT.
- Costs: a separate step with its own design, measurement and goldens,
  before S121 can ship on H1.
- Risks: the step changes the reporting layer under every case.

**Recommendation: (a).** The 13 are DEC-122's expected residue as the tool
reads them: true mates, the claimed side right, none shorter than the
oracle's, cut at the horizon by TT reads of entries that earlier searches
wrote. The candidate finds the mate at ply 33 a move earlier than the parent
does, and that is what produces the extra lines. A walk that reaches through
quiescence is a reporting step of its own, not a precondition of
S121's verdict.

### Also finished

- `DEV_MANUAL.md`:
  - every goldens row S121 moved or re-read;
  - the `--ceilings` command, which now lists S296's and S121's grids;
  - the `EXACT_FLOOR` command `--floor 159`;
  - the ALPHA row's commands;
  - the gradient and walk sentences.
- `MANUAL.md`: the `RfpMinPly` row's mined-set figures (165, 165, 154
  against 159).

`adocs/data/S121_parts.patch`'s preamble said "never committed to `src/`
in this form", which is false at CAND. **Corrected after the fast check**:
the form is committed once only, as S121's SPRT candidate with `#define
S121_PARTS 5` (DEC-326 (7), DEC-327), folded on H1 or reverted on H0, never
shipped. `git apply` reads from the first `---` line on, so the preamble
cannot move the guard's expected tree: re-simulated after the edit, it is
`b024945c...` as before, `src/evaluation.cpp` and `src/evaluation.hpp` are
the only files that differ, and nothing differs outside the allowed lines.
`adocs/data/S121_sprt.sh` applies the patch from CAND and hashes no file.

### The fast check's other corrections (comments and data only)

- `src/evaluation.cpp`, the comment over the count: "Not safe mobility: no
  enemy-pawn-attack mask" was false at mask 5. It now says that the raw
  `~own` count is mask 0's, that the count is `area`'s, and that at mask 5
  enemy pawn attacks are excluded and the pin rule holds.
- `tests/test_walked_counts.hpp`'s header: the raw `~own` count is now
  stated as mask 0's, with what mask 5 changes.
- `tests/test_eval_trace.cpp`'s and `DEV_MANUAL.md`'s "18 at 2.875": the
  scan line that gives it (the rows at the maximum, before the pick folds
  them into 16 distinct positions) is added to `adocs/data/S121_goldens.txt`
  section 2.

### Known, not fixed

- **The gradient check sits 1.05x inside its bound**: 9.50e-07 at parameter
  849, `mobility_eg[bishop][1]`. The coverage tool's smallest |gradient|
  from `MOB_MG_BASE` on is 2.153e-09. This is DEC-316's class, so the next
  refit (a bisection part's, or S126's) is likely red there and will need
  fixture rows.
- **Mask 5's cost is a bracket**: p3's +2.20 % to all's +3.38 %, with p1 +
  p3's +2.42 % in instructions as the estimate. The coordinator times an
  `S121_COST=5` row before the done stamp, on H1 only.

### Gates, after DEC-327

- **Both builds:** 0 warnings. Both fast suites 50 of 50, sequential, the
  AGENTS.md TESTS command (the final gate, below).
- `./clang-format.sh --check` clean (`CLANG_FORMAT_MAJOR=22`).
- **The S297 clang check:** `build-clang` and `build-clang-tune` with
  `clang++-22`, Release, `-k 0`. No error, no warning; both fast suites 50 of
  50.
- `tools/plan_prose_check.py`, one mode per call, each exit 0:
  - `--citations`: 0 over 21 files;
  - `--touches`: 0;
  - `--params`;
  - `--gate`: agrees.
- **`tools/gate_extra.sh`**, detached under a four-exit watcher:
  `GATE-EXTRA-DONE 5 stages 1025 s`.
  - prose 0 s, citations 0 s, debug 363 s, sanitize 610 s (the fast label 50
    of 50 under the sanitizers), perft 52 s;
  - no `Assertion` in its logs;
  - Release bench 6016079, the same across builds.
- **Bench 6016079** in both builds.
- **Tier 2 (DEC-141) does not bind**, by what the diff touches. The Debug
  self-play is the coordinator's.

### The SPRT script

`adocs/data/S121_sprt.sh` is as the stop left it: the dated paragraph, the
run count corrected, the coordinator's markers unfilled. DEC-327 adds no
guard. The changed files outside `src/` (`tests/`, `adocs/data/`) are
outside its src/ check, and its src/ check reads the same on this tree.

### Proposed commit message

```
Paste S121's mobility fit at P1 and P3 for its SPRT (S121)

The screen kept the per-count table, the enemy pawn attack exclusion
and the pin rule, and dropped both own-pawn exclusions (DEC-326). Its
full-corpus fit at S121_PARTS 5 is pasted with the parts patch for the
one {0, 5} SPRT that decides the family (DEC-259). The stage-two clamp
binds on 0.355 % of the corpus against the 1.0 % bound.

Every golden the paste moves is re-derived by its script (DEC-142):
the anchors 283, 353, 590, 1021, 181, -541 and -603; the truncation
pins re-picked, N still 3; the coverage block re-derived; the clamp
push +140 -> +30; the ProbCut row re-mined, mate in 4 at depth 10; the
aborted-line cell's budget 40000 -> 50000; ALPHA kept inside its band,
whose sweep is now a tracked script. Both mate floors hold: EXACT_FLOOR
159 against 165 and 154, MATE_IN_THREE_FLOOR 11 against 12 and 10.

Case B's mate-carry ceiling rises 6 -> 13, outside DEC-299's guard. It
is carried provisionally with the picker's budgets by DEC-327, and the
owner rules before an H1 ships. The cell is attributed by tool: two
roots' lines cut at the horizon, the claimed side right and none
shorter than Stockfish's. The cost reading is committed: all +3.38 %,
p3 +2.20 %. .tuning/verify_fit.py, which is tracked, takes S121's
layout.

Bench: 6016079
```

No `done:` stamp: the SPRT, and on H1 the owner's ruling on B (DEC-327),
come first.

## 2026-10-29: the SPRT read H1; the parts folded

One Opus 5.5 subagent, in `../chesso-s121d` (branch `s121d` at `5ac067c`, the
SPRT's pin), read only over the run's outputs and binaries; no match, fit or
timing ran.

### The run

`adocs/data/S121_sprt.sh` as pinned at `5ac067c`: `57a58a4` (the candidate,
bench 6016079) against its parent `12cab3c` (the phase 1 landing, bench
5069022), launched 2026-10-29 07:29:39, `SPRT-RUN-DONE` after fastchess's
Total Time 01:30:07:

```
SPRT | cand 57a58a4 vs ref 12cab3c, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | 19.26 +/- 9.29, nElo 25.18 +/- 12.11
LLR | 2.95 (-2.94, 2.94) -> H1
Games | N: 3160 W: 1002 L: 827 D: 1331, Ptnml [108, 358, 528, 423, 163]
Wall | 1 h 30 m, 2103.9 games/h, forfeits 0
Log | adocs/data/S121_sprt.log
```

`Wall`'s rate is N over fastchess's `Total Time`, S296's and S136's rule.

### The inspection, before the verdict was read as the record

- **Banner** (the console's): candidate `57a58a4 2026-10-29`, reference
  `12cab3c 2026-10-29` (the commits' dates; the pin's 07:23 and 02:32 are
  `git log`'s), `arch native tune off`, padding on both, 8+0.08, hash 16,
  12 of 12 cores, `noob_3moves.epd`, seed 20261029072939, `elo0=0 elo1=5
  alpha=0.05 beta=0.05`, cap 20950 rounds and 41900 games from
  `SPRT_ROUNDS`, out `.tuning/sprt_s121_20261029_072938`; no `diff`, `relay`
  or `A/A CALIBRATION` line; `id name`s `cand-57a58a4 Chesso 57a58a4 native`
  and `ref-12cab3c Chesso 12cab3c native`; `Started game 1 of 41900`. As the
  pin says. The seed's stamp is one second after the out directory's:
  `S121_sprt.sh` names OUT before its `exec`, and `fastchess.sh` takes the
  seed after.
- **Binaries**: `.ref-builds/57a58a4/build/src/chesso` benches **6016079**
  and `.ref-builds/12cab3c/build/src/chesso` **5069022**, both worktrees
  clean at their sha. Written 07:29:45 and 07:29:52 and not since; the first
  game's `GameStartTime` is 07:29:52.
- **Crashes, disconnects, illegal moves**: none in the console or in
  `fastchess.log`, whose 32 lines are the eight warnings below, four lines
  each. 3160 games in the PGN, N's count; fastchess's game numbers (`2 *
  Round - 1` with the candidate White, `2 * Round` with it Black) 1 to 3160,
  each once. Terminations: 2177 adjudication (1828 decisive, 349 drawn) and
  983 normal (982 drawn, 1 decisive).
- **Forfeits**: `tools/forfeit_report.py` reads 0 of 3160 for each engine.
- **Console warnings**: no PV-past-threefold block (`trim_console.py`'s
  count) and **8 `Incomplete mating PV`**, all the candidate's, read in the
  next section.
- **Quarters**, in game-number order, 790 games each (boundaries `i * 3160
  // 4`), Elo from the candidate's score with its 95 % per-game interval,
  now a script, `adocs/data/S121_sprt_mate_pvs.py quarters`, which
  reproduces S135's +5.6, -2.5, +13.8, +6.6 (chi-square 6.56) and S296's
  -0.7, -3.3, +4.4, +3.3 (2.73) from their PGNs: **+10.6, +16.7, +21.6,
  +28.2**, each +/- 17.8 to 19.0. Their spread about the run's mean is
  chi-square 1.89 on 3 degrees of freedom, p 0.60.
- **Kernel journal**, 07:20 to 09:05 (boot `ea34bee7`, from 00:03:28, no
  suspend): no kernel entry; the boot's last is 00:06:55, `usb 3-2.4: USB
  disconnect`, the monitor's control device, before the launch, as on
  S136's and S296's nights. Nine entries in the window in all, systemd's
  `fwupd-refresh` and the hourly cron; none at warning level or above. No
  machine check, OOM or thermal event.

The run stands.

### The eight `Incomplete mating PV` warnings (`adocs/data/S121_sprt_mate_pvs.txt`)

S136's method, by S296's script `adocs/data/S296_sprt_mate_pvs.py`, run
unchanged from the main checkout. The oracle is DEC-319 (3)'s, Stockfish at
5000000 nodes through python-chess in `TOOLCHAIN.md`'s safe form, from the
bare FEN and with the game's moves. It reads a mate on all three positions,
so DEC-320's 10^9-node escalation does not arise. Console line numbers are
the untrimmed console's and the trimmed log's alike.

All eight are the candidate's, from three consecutive searches of one game,
1482, the candidate Black and winning on adjudication at ply 127.

| console line | ply | printed | line | Stockfish 5000000 | against it | cold | warm, the candidate |
|---|---|---|---|---|---|---|---|
| 3593 | 121 | mate 12, depth 14, 607502 nodes, the search's last | 14 of 23 plies | mate 11 | longer | no | the class: mate 13 at every depth, 1 to 14 plies of 25 |
| 3607 | 123 | mate 13, depth 3, 774 nodes | 3 of 25 | mate 10 | longer | no | no: complete mate 10 |
| 3611 | 123 | mate 15, depth 4, 1159 nodes | 4 of 29 | mate 10 | longer | no | no |
| 3615 | 123 | mate 12, depth 14, 733236 nodes, the search's last | 14 of 23 | mate 10 | longer | no | no |
| 3619 | 125 | mate 13, depth 1, 82 nodes | 1 of 25 | mate 9 | longer | no | no: complete mates 11, 10, 13 |
| 3623 | 125 | mate 13, depth 2, 133 nodes | 2 of 25 | mate 9 | longer | no | no |
| 3627 | 125 | mate 13, depth 3, 382 nodes | 3 of 25 | mate 9 | longer | no | no |
| 3631 | 125 | mate 12, depth 5, 2511 nodes | 5 of 23 | mate 9 | longer | no | no |

**No printed distance is shorter than the oracle's, and the claimed side is
the oracle's in every one**: Black, to move, mates. The brief's stop does not
arise. Each line is legal throughout and shorter than the distance it claims:
**DEC-122's short-line class**, which `test_mate_carry` bounds per case
(DEC-162) and S202 owns. Every cause is a recorded one, S296's two shapes:

- **3607, 3611, 3619, 3623, 3627, 3631**: a shallow iteration (depth 1 to 5,
  82 to 2511 nodes) printing a mate too shallow to build its line, as
  DEC-150's case was, longer than the position's.
- **3593, 3615**: each search's last line, a completed iteration at depth 14
  (the PGN's `+M23/14` twice), naming a mate longer than the position's:
  DEC-127's "sound but not optimal", the walk leaving the line short, as
  S136's 14853 and S296's game 7655.

The game's searches carry the distance rather than shorten it: mate 12 at
plies 121 and 123 and 12 or 13 at 125, where the oracle reads 11, 10 and 9.
That the mates came off entries the game's earlier searches wrote is
inferred, as S136 and S296 inferred it.

**Reproduction.**
- **Cold**, on both binaries, one fresh process each at the warning's depth:
  none reproduces; every search prints cp lines and no mate.
- **Warm**, the game replayed at the PGN's per-move node counts through one
  candidate process: the class reproduces at ply 121, the line does not --
  every iteration from depth 1 (30 nodes) prints mate 13 with a line as long
  as its depth, a mate read off the table the replay's earlier searches
  wrote; at plies 123 and 125 every line is complete. The replay's table
  diverges from the game's: at plies 117 and 119 the game printed `+4.50/18`
  and `+3.83/20`, the replay mate 15 and mate 14.
- **Warm through the reference's binary** on the candidate's side of the same
  game (a cross-check; the reference never played these moves): short lines
  at all three plies. The class is the reference's too when its search meets
  this game.
- **Seen in the replays and not attributed**: in both binaries a search
  whose last line is a centipawn score after an earlier iteration of the same
  search printed a mate (the candidate at ply 125, the reference at plies 121
  and 123). No distance there is shorter than the oracle's, the move is the
  mating line's first, and none of it is in the run's console.

**The rates** (`S296_sprt_mate_pvs.py rates`; `S121_sprt_mate_pvs.py
games`), per 10000 games a side:

| run | candidate | reference |
|---|---|---|
| this run | **25.32 lines, 9.49 searches, 1 game** of 3160 | 0 |
| S136 | 1.43 / 0.95, 2 games | 0.95 / 0.48, 1 game |
| S296 | 2.70 / 1.62, 3 games | 1.08 / 0.54, 1 game |

The eight lines are one game's three searches, so the game is the unit.
Pooled over the seven earlier runs (S136, S296, S135, S293, S292, S289,
S283): 11 games with a warning in 202308 side-games, 0.54 per 10000. At that
rate one side's 3160 games expect 0.17 such games, and 1 or more has Poisson
probability 0.16. A count, recorded, not a finding.

### The verdict, by the pre-registration

**The H1 row executes.** At 8+0.08 the fitted family is accepted as
gaining: H1's 5 nElo preferred to H0's 0 at alpha = beta = 0.05. **It
ships**: the table per piece type fitted by the screen over the count with
enemy pawn attacks excluded (P1) and the pin rule (P3), the screen's
survivors (`CANDIDATE table_m5 mask 5`). P2a and P2b stay out, the screen's
drop, with their figures above (full d +0.000001 and -0.000017 against m
0.000006 and 0.000010). **No bisection follows an H1**, and none is owed.

**The stopping run's Elo is upward-biased (DEC-063) and is not the effect
size.** The run stopped at a bound in 3160 games, far under the 25591 the
pre-registration expected with the truth on a bound.

**The cost, beside it**, until the row below is read: no measured row
computes what mask 5 computes. It lies between p3's **+2.20 %** and all's
**+3.38 %** (`adocs/data/S121_cost_reading.txt`), p1 + p3 in bench16
instructions (+2.42 %) the additive estimate. It decides nothing.

### The trim and the archive

`tools/trim_console.py --summary` from `.tuning/s121_sprt.log` (7679 lines,
525112 bytes) into **`adocs/data/S121_sprt.log`**, 7684 lines, 525317 bytes:
no PV-past-threefold block, so nothing is dropped and the foot's five lines
say so; the eight mating-PV warnings are kept. Its `adocs/data/README.md`
row added. The untrimmed console and the run directory (`games.pgn`,
`fastchess.log`) are **`chesso_S121_sprt_2026-10-29.tar.xz`** in the owner's
archive (DEC-235), built, verified and moved in by the coordinator.

### The fold, and its INV-6

The H1 row's fold, against CAND `57a58a4`:

- `src/evaluation.cpp` `evaluate_mobility_and_king_safety`: the pawn
  attacks, both colours' pins and the kings' squares are computed
  unconditionally; the count reads `~own & ~pawn_attacks[1 - colour]`, and a
  pinned piece's moves are cut to its pin line. P2a's and P2b's own-pawn
  exclusions and the cost-only branch are deleted. The comments over the
  count and over the tables now describe the shipped count and the H1, and
  "on H0 the identity comes back" is gone.
- `src/evaluation.hpp`: `S121_PARTS`, `S121_COST`, their four static_asserts
  and the four `MOBILITY_*` flags leave `src/`; one comment states the
  count, the two parts kept, the two dropped and where the masked form is.
- `src/bitboard.hpp` `pinned_pieces` stays where the patch put it, in
  `src/bitboard.{hpp,cpp}`, unchanged.
- `tests/test_walked_counts.hpp` `mobility_cells`: the walk folds the same
  way -- the pin walk and the enemy-pawn rule unconditional, the area's
  own-pawn branch gone, an own piece's square never counted.
- `adocs/data/S192_anchors.py`: each mask-keyed count is mask 5's (the
  dict's key 1, `5 & 3`), the mask reader `parts_mask` and `count_for` gone.
  It reads **10 of 10 reproduced** on the folded tree.
- Comments elsewhere that named `S121_PARTS 5` now name the count:
  `tests/test_mate_breadth.cpp`, `tests/test_eval_positions.hpp`,
  `tests/test_tuner_gradient.cpp`, `tests/test_evaluation.cpp`,
  `tests/test_eval_trace.cpp`, `tests/test_mate_carry.cpp` and
  `DEV_MANUAL.md`'s anchors row.

**INV-6 against `57a58a4`** (`adocs/data/S121_fold_inv6.txt`): `chesso
bench` **6016079** in both builds; `bench` and `bench 12` (2200179)
byte-identical once timing is stripped, and `tools/search_bench.py` node
counts and best moves identical at depth 9 (26892, 109678, 35636) and depth
12 (183158, 414751, 221974), across the played binary, CAND's `build/` and
`build-tune/` and the fold's two. The fold's Release binary differs from the
played one, as a code change must. **The tune build and the trace build read
the same counts**: `test_eval_trace` -- "every coefficient is the count its
index names", the folded walk against the trace cell by cell; "the trace
reconstructs evaluate() exactly, clamp included"; "tracing changes nothing
evaluate() returns" -- green in `build/` and `build-tune/`, 1425412
assertions each.

### The patch and the tooling after the fold

- **`adocs/data/S121_parts.patch`** applies to the landing `12cab3c` and no
  longer to HEAD (`git apply --check` refuses `src/bitboard.hpp` first). Its
  preamble now says so, and that it is kept as the record of what was
  screened and of what `S121_cost.sh` builds. The text `git apply` reads is
  unchanged: from the first `---` line on it hashes `d82d1fdc...` at
  `12cab3c`, at `57a58a4` and here.
- **`adocs/data/S121_cost.sh`** still runs, from the main checkout with
  `CAND_SHA=12cab3c PARENT_SHA=6ff7485`: its builds are worktrees of the
  landing, which the patch applies to, so the fold changes nothing it
  builds. `CAND_SHA` unset (HEAD) is refused by `check`, HEAD's parent
  carrying the table, as it should be. Two guards were wrong and are fixed:
  a build's stamp hashed the whole patch file, so each preamble edit (the
  candidate's commit and this close) made a rebuild of the same sources
  refuse; it now names the text `git apply` reads. And nothing stopped a run
  into an OUT holding a finished reading, whose pairs `pairs` empties; `run`
  now refuses such an OUT.
- **`adocs/data/S121_screen.sh`** runs from its pinned state, a checkout of
  the landing; from a folded checkout it misled. `variant 0` freezes
  `build/`'s tools as m0, and after the fold those count with P1 and P3,
  mask 5's count under mask 0's name. Every stage that builds, freezes or
  pastes now refuses a checkout whose `src/bitboard.hpp` declares
  `pinned_pieces()`, which only the patch adds, and says to run from a
  worktree of `12cab3c`. `locate` and `read` build nothing and are not
  refused. This is code below the screen's pre-registration line, changed
  after its run, recorded here as that header asks; the screen's reading is
  untouched.

### The cost row owed: mask 5

`adocs/data/S121_cost.sh` gains the row **p5**, `S121_COST=5`: P1's and P3's
sets forced together, read as every other row is (INV-6 against zero, the
code check, the session's A/A floor, DEC-266's reading), and ordered by
`S121_cost.py` under all and over p1 and p3. The other rows are as they
were. `ROWS` names the rows a run times; the A/A is taken in every run.
`check` with `ROWS=p5`:

```
pair      cand 12cab3c6de1f39c20618b4beb3bcab843aeb56a8, parent 6ff7485f64cc2f4ca33810bac58008216bc9cb77
pair      CAND carries the table, PARENT does not, PARENT is CAND^
patch     applies to CAND, applied text sha256 d82d1fdc8ac5d4bd
rows      aa (zero against its copy), p5 (S121_COST=5), each against zero
S121-COST-CHECK-DONE
```

`S121_cost.py read` re-reads the old run's pairs, rows named, into the
committed reading line for line. The row builds: the landing with the patch
at `S121_COST=5`, built in a scratch copy (Release, padded, no warning),
benches 5069022 and its `bench`, `bench 12` and `search_bench` streams equal
the landing's; its `.text` (`f16401be...`) is none of zero's, p1's, p3's or
all's, and the landing's own binary has zero's `.text` (`179ed51e...`), so a
rebuild reproduces it. Nothing was timed. The timing is the coordinator's,
on an idle machine; its reading goes to `adocs/data/S121_cost_reading.txt`
and here.

### DEC-328

The owner accepted case B's mate-carry ceiling 13 on H1 (DEC-328). DEC-327's
provisional wording now cites it, values unchanged: `tests/test_mate_carry.cpp`
(the history paragraph, the eighth grid's sentence, the ceiling table's B row,
`short_line_ceiling`'s comment), `adocs/data/S170_cases.tsv`'s header
paragraph, `DEV_MANUAL.md`'s `short_line_ceiling` row and
`adocs/data/README.md`'s `S121_sweep_s170.txt` row. The ceiling stays 13 and
the budgets A 1000000, C 500000, D 1500000, E 100000.

### What H1 leaves unmeasured

- **P2a and P2b**, the mobility area's two own-pawn exclusions: zero by the
  screen (held-out gain +0.000001 and -0.000017 against m 0.000006 and
  0.000010), never played.
- **The own king's square**, never excluded and never measured (DEC-326
  (2)).
- **The thin cells** `adocs/data/S121_cells.txt` marks: queen mg 20 to 25,
  queen eg 0 and 24, knight eg 8, bishop mg 11 to 13; the thinnest queen 24
  (2814 rows, 1768 games) and 25 (1813, 1139), and queen 26 and 27 tied to
  25 in both halves. S126 inherits the list.
- **The parts apart in play**: the curve, P1 and P3 shipped together on one
  verdict; each has only its held-out gain.
- **Mask 5's cost** as a measured row, until the coordinator's timing.

### Documents

- `DEV_MANUAL.md`: checked. Changed where a statement was now false: the
  anchors row's `S121_PARTS 5`, a macro that no longer exists, and the
  `short_line_ceiling` row's DEC-327 wording (DEC-328) and the same macro;
  and the clamp paragraph's "S122's design ... waits on an owner decision
  before S121 starts" -- stale since DEC-324 -- now says the amendment came
  and what it keeps. The rest of its S121 rows describe the shipped tree.
- `MANUAL.md`: checked; nothing false. Its `RfpMinPly` row's S121 figures
  are the candidate's, which the fold leaves, and nothing on the UCI surface
  moved. Unchanged.

### The accepts, clause by clause

- **An SPRT verdict, recorded whatever it is**: H1, above, with DEC-220's
  block for the closing commit. Holds.
- **Mobility a table indexed by piece type and by count, knight 0 to 8,
  bishop 0 to 13, rook 0 to 14, queen 0 to 27**: `src/evaluation.hpp`
  `MOBILITY_WIDTH` {9, 14, 15, 28} and the eight `mobility_<piece>_<half>`
  tables, packed into `mobility_score`. Holds.
- **Every entry fitted by our own tuner on our own corpus (DEC-084)**: the
  screen's `table_m5` fit, `--only mobility` on S083's corpus (sha256
  `2e0e3de9...`), `verify_fit` 955 of 955; the queen's 26 and 27 share 25's
  fitted parameter in both halves, DEC-326 (5)'s tie, a fitted value and not
  a chosen one. Holds.
- **The area excludes enemy pawn attacks and the side's own blocked and
  low-rank pawns, each exclusion and the pin rule screened separately before
  the one SPRT (DEC-259)**: each screened apart in fixed order
  (`adocs/data/S121_screen_reading.txt`); the enemy-pawn exclusion and the
  pin rule kept and shipped, the two own-pawn exclusions dropped by the
  screen and recorded with their figures, as DEC-259 has a part that fails
  the screen stay out of the match. The accepts' screening holds; the two
  own-pawn exclusions are measured zero, not shipped.
- **The tuner's model updated in the same commit, held to the engine by
  `test_eval_model`**: discharged by S262's trace, the 2026-10-08 amendment
  (S276 deleted the mirror): `test_eval_trace` reconstructs `evaluate()`
  exactly and holds every coefficient to the walk, green in both builds on
  the folded tree. Holds.
- **The nps cost recorded next to the verdict**: the bracket above is
  recorded; the row that measures mask 5 itself is prepared and waits on
  the coordinator's timing. Open until then.

### Gates

- Both builds with no warning; both fast suites **50 of 50** in `build/` and
  `build-tune/`; `./clang-format.sh --check` clean (`CLANG_FORMAT_MAJOR=22`);
  bench 6016079 in both.
- The S297 clang pair, `build-clang` and `build-clang-tune` with
  `clang++-22`, Release, `-k 0`: no error, no warning; both fast suites 50
  of 50.
- `tools/plan_prose_check.py`, one mode per call, each exit 0:
  `--citations` 0 over 21 files (8 in this file), `--touches` 0,
  `--params`, `--gate` agreeing.
- **`tools/gate_extra.sh`** on the folded tree, detached under a four-exit
  watcher: `GATE-EXTRA-DONE 5 stages 1030 s` -- prose 0 s, citations 0 s,
  debug 367 s, sanitize 611 s (the fast label 50 of 50 under the
  sanitizers), perft 52 s; no `Assertion` in its logs.
**Tier 2 (DEC-141) does not bind**: under `src/` the diff folds the
evaluation's mobility count (`src/evaluation.{hpp,cpp}`), proved neutral by
INV-6; no `make_move`, `unmake_move`, generator or search change, and
`src/bitboard.{hpp,cpp}` is untouched. `gate_extra` ran anyway, for the Debug
asserts and the sanitizers over the rewritten source.

No `done:` stamp: the cost row comes first.
