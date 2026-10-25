id:         S135
goal:       unfreeze the piece placement group, add bad-bishop and trapped-piece features, and refit it, one SPRT over the family by the owner's decisions of 2026-08-20 and 2026-10-08
accepts:    S134 has landed first, so the group is three identified features and carries no unidentified column -- a bundled verdict over this bundle is at least attributable, which S027's was not; the fit is `--only piece_placement` on the corpus S082 and S083 produce, so the SPRT measures the term and not a joint refit of the other 820 constants; bad-bishop and trapped-piece features join the group (DEC-258), each defined from its description before any fit (CPW *Bad Bishop*, *Trapped Pieces*) and each feature screened before the one SPRT (DEC-259); the SPRT bounds are chosen for the effect size and stated in advance, not `--fast` -- DEC-063 is the measurement that the pair and not the hardware sets the cost, and the published parts are +8.2 and +9.86 (DEC-084, order of magnitude only); the term's own speed cost is inside its verdict, measured with the weights forced non-zero because at zero the compiler deletes it (DEC-047); a verdict of zero is recorded as zero, and **if the bundle fails it is bisected** rather than zeroed by hand, which is the S027 failure this step exists not to repeat (DEC-082's bisect rule); the fast suite green
touches:    src/evaluation.cpp piece_placement_mg/eg and the four new counts, src/evaluation.hpp, tools/tuner_groups.hpp, tools/tuner.cpp, tools/tuner_layout.hpp, tools/feature_audit.cpp, tests/test_eval_model.cpp, tests/test_eval_positions.hpp, tests/test_eval_trace.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, tests/test_walked_counts.hpp, tests/test_evaluation.cpp and every other golden the fit's paste moves (each re-derived by its script), tools/tuner_trace.hpp (one comment's parameter count, at the rebase; the trace's term table reads the width), .tuning/verify_fit.py, DEV_MANUAL.md, adocs/data/README.md, adocs/data/S135_*; src/eval_tables.hpp named before phase 1 and needing no change (the tables are not fitted)
excludes:   tempo, which is S136; the seventh-rank feature, deleted at S134; any change to how the three features are computed
decisions:  DEC-091, DEC-057, DEC-063, DEC-084, DEC-258, DEC-259
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

**Scope gained 2026-10-08 (DEC-258):** a bishop's own pawns on its colour
(blocked ones counted separately) and trapped-piece patterns, each taken from
its CPW description. They join the three remaining features, so the "three
features" wording below reads "the three remaining features and the two
added". The bisect-on-H0 rule this file already carries is DEC-259's.

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
amendment binds here.

## What S100 established, and what it did not

Refuted for all three of these features: a wrong count (10795695 rows
re-extracted, 0 disagreements), a wrong gradient (worst 4.9e-8 over all 827
parameters), a pipeline that cannot recover a planted vector, a coverage desert
(bishop pair 15.81 %, rook open 31.38 %, rook half-open 31.26 % of rows), and
redundancy with the tables (R^2 0.487, 0.265, 0.168 -- against 0.99 for
redundancy, on the widest right-hand side the model offers).

Not refuted, and the operative cause: **these three have never been separately
measured.** They shared one SPRT with the seventh-rank feature at `--fast`
bounds `elo0=0 elo1=10`, returned -5.48 +/- 11.46 over 2284 games, and were then
zeroed **by hand**; every fit since S065 has held them there with
`--freeze piece_placement` (DEC-057). The freeze's stated reason was that S027
verdict, and S100 calls it procedural.

**Still unresolved and not refuted:** the correlation form of the corpus
hypothesis -- chesso's self-play cannot show the value of a feature chesso
weights at zero. That is why this step's fit is on S082's corpus and not on the
outgoing one.

## The owner's decision, and the concern it overrides

The agent's recommendation was to split the group and measure the bishop pair
alone first: it is the free feature -- the compiler rewrites
`count_bits(x) >= 2` into `x & (x - 1)` -- it carried the largest fitted weight
of the four, and the three rook features cost 3.1 to 4.0 % of a search between
them, so a bundle prices a cheap term that may work together with expensive ones
that may not.

**The owner chose one bundled fit and one SPRT over all of them** (DEC-091). The
concern is recorded, not re-argued. Two things make this bundle materially better
than S027's, and they are why the choice is defensible: S134 removes the one
member that was not an identified quantity, and the bisect-on-failure rule means
a negative verdict is followed by attribution rather than by a hand revert.

## Measurement

One SPRT, bounds stated before the run. DEC-063 is the constraint that decides
whether the night buys a verdict at all: the same constant took 6 h 36 m over
9036 games for nothing at `elo0=0 elo1=5` and returned H1 in 1 h 41 m at
`elo0=-5 elo1=5`. At S105's measured 23.1 to 38.7 games a minute that is roughly
1.5 to 3 hours for a resolving pair.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**All three features are on the wiki, two of them carry a published
magnitude, and none carries an Elo figure.** CPW *Evaluation of Pieces*
(https://www.chessprogramming.org/Evaluation_of_Pieces, fetched 2026-09-13) is
a link hub: it names *Bishop Pair* and *Rook on Open File* and describes
neither. The definitions and the numbers are on the dedicated pages.

- CPW *Bishop Pair* (https://www.chessprogramming.org/Bishop_Pair, fetched
  2026-09-13): "Larry Kaufman proposed the value of half a pawn", citing
  Kaufman's 1999 *The Evaluation of Material Imbalances*. That is the only
  number on the page; **no Elo figure**.
- CPW *Rook on Open File* (https://www.chessprogramming.org/Rook_on_Open_File,
  fetched 2026-09-13): "Bonuses applied to a rook on an open file vary from 8
  to 20 centipawns", and "Typical bonus for a semi-open file is half of that
  for a fully open file". **No Elo figure**. **Its 20 is an engine's**: the
  page's footnote 1 reads "20 cp comes from Toga log user manual, after adding
  actual bonus and penalty for a rook on a closed file" (footnote re-read
  2026-09-13 after the fast check), so the upper end of that range is Toga's
  documented constant republished, and DEC-105 as amended by DEC-134 refuses
  it as a seed wherever it is quoted. The range is read for form only.

**One of the two is usable as a seed and one is not.** Kaufman's half a pawn
originates in a 1999 article about material imbalances -- a publication about
the technique, DEC-105 form (a). The 8-to-20 range does not: its stated upper
end is Toga's, and a range whose end is an engine's constant cannot be
laundered into literature by its venue, which is the exact hole DEC-105
closed. Section 4 states what replaces it. The form check owes no departure
statement: chesso's three features are the wiki's three features, one flat
weight each, and the whole question is the weight.

**The two figures this file already quotes are traced and are the right size.**
Lynx pull request #390, "tapered bishop pair", merged 2023-09-10:
**+8.2 +/- 6.1** short and **+9.5 +/- 6.7** long
(https://github.com/lynx-chess/Lynx/pull/390). Weiss pull request #231, "Tune
Rook+Queen" -- rook and queen piece-square tables together with open and
semi-open file bonuses, 2020-04-06: **+9.86 +/- 5.74** and **+8.40 +/- 5.42**
(https://github.com/TerjeKir/weiss/pull/231). Both are bundles: #390 tapers
an existing bonus rather than adding one, and #231 retunes tables alongside
the file bonuses, so neither prices a file bonus alone. They are quoted for
order of magnitude and nothing else (DEC-019, DEC-084).

**Three figures this file used to quote are gone (DEC-203, applied here
2026-09-13 after the fast check).** The plan's older per-feature numbers for
this group -- one for the bishop pair, one for a rook on an open file, one for
a rook on a half-open file -- survived three searches with no source and are
deleted, on the same rule and for the same reason as the fourteen DEC-203
removed elsewhere: an unverified figure carries no weight and a kept one gets
quoted again by the next reader. Searched this pass: GitHub's commit search
over `TerjeKir/weiss` for "open file" (three commits, the only priced one
being #231's retune) and over `jhonnold/berserk` for "bishop pair" (no
commits), and an open web search for the three values together -- nothing. The
2026-09-04 check searched Weiss's pull requests for "bishop pair", "seventh"
and "open file", Berserk's for "bishop pair", and Stash's changelog, and found
the same nothing. **What is sourced and stays**: Weiss #95 "Tune Eval 1", a
CLOP of the bishop pair together with a king-vulnerability term, 2019-11-26,
**+16.10 +/- 8.74** / **+12.24 +/- 7.21**
(https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22Tune+Eval%22)
-- a bundle, quoted for order of magnitude only. No fetched source gives a
half-open-file figure on its own at all. The accepts prices the group on +8.2
and +9.86 and never needed the deleted three.

**Nobody in the surveyed record measured these three as a bundle**, which is
the one thing worth knowing before the run: every located figure is a single
term or a retune, so the bundle's expected size is a sum of single digits and
the bounds have to be chosen for that, not for a group.

### 2. Shape for chesso

The features exist and are computed; only the weights are frozen.
`src/evaluation.cpp` `piece_placement_mg` and `src/evaluation.cpp`
`piece_placement_eg` are the two rows, `PL_FEATURE_COUNT` their width. After
S134 the width is three and every column is an identified quantity -- which is
the precondition the accepts states and the whole reason this step waits.

Departure from the published form, stated: the surveyed engines apply the
bishop-pair bonus and the rook-file bonuses inside a tapered score with their
own middlegame and endgame halves, which chesso also does. The one thing
chesso does differently is that it **fits** all three rather than hand-setting
them, which is DEC-084's rule and not a departure from any published
technique.

### 3. Implementation sketch

No evaluation code changes. The work is the freeze list, the fit and the run:
`--only piece_placement` on the S082/S083 corpus, weights pasted, speed cost
measured with the weights forced non-zero (DEC-047: at zero the compiler
deletes the term and the measurement is of nothing), SPRT.

### 4. Constants and seeds

**No seeds are needed and none is used.** A `--only piece_placement` fit
starts from the shipping weights, which are zero, and every weight that ships
is the tuner's output on this project's own corpus -- DEC-084's "no constant
ships unfitted" in its simplest form.

What a published magnitude can buy is a **sanity band, stated in advance and
never a target**, and only one of section 1's two is allowed to:

- **Bishop pair.** DEC-105 form **(a)**: Kaufman's half a pawn, from his 1999
  *The Evaluation of Material Imbalances*
  (https://www.chess.com/article/view/the-evaluation-of-material-imbalances-by-im-larry-kaufman,
  as CPW *Bishop Pair* cites it), which is **47** on chesso's material scale
  (`src/eval_tables.hpp` `piece_value`, `PAWN` 94). A published article about
  the technique, not an engine's shipped constant -- which is the whole
  difference between this bullet and the next.
- **Rook on an open file.** **Not form (a) and not the wiki's range.** The
  8-to-20 band's upper end is Toga's constant by the page's own footnote
  (section 1), so the band is refused entire -- half a refused range is still
  the range. What replaces it is **(c) the midpoint of a range this file
  declares by purpose**, its bounds chesso's own: a file bonus is a placement
  preference and must never be worth trading material for, so a single rook's
  open-file bonus is bounded above at **half a pawn** -- 0 to 47 on the scale
  above, midpoint **24**, semi-open half of open by the same declaration. The
  band is wide, and that is the honest cost of refusing the narrow one
  somebody else tuned.

If the fit lands near either band that is a confirmation and a good outcome
(DEC-084); if it lands far from them that is the measurement and the band does
not move it. No engine's shipped bonus is used, wherever it is republished
(DEC-105, DEC-134).

The one number this step chooses rather than fits is the **SPRT bounds pair**,
and it is **(b) derived from this project's own data**: DEC-063 measured that
`elo0=0 elo1=5` took 6 h 36 m over 9036 games for nothing on this same
constant set while `elo0=-5 elo1=5` returned H1 in 1 h 41 m. The pair is
chosen from the expected effect size in section 1 -- a sum of single digits --
and stated in advance with its worst-case expected games from the nElo formula
(DEC-143).

### 5. Pitfalls

- **S027's failure was attribution, not the terms.** One bundled run at
  `--fast` bounds over four features, one of them unidentified, then a hand
  zeroing. DEC-082's bisect rule is what replaces the hand revert, and the
  accepts requires it.
- **The compiler deletes a zero-weighted term.** Any speed measurement taken
  with the weights at zero measures the absence of the term (DEC-047). Force
  them non-zero.
- **The corpus hypothesis is not refuted.** S100 refuted count, gradient,
  recovery and redundancy; it did not refute the claim that chesso's own
  self-play cannot show the value of a feature chesso weights at zero. That is
  why the fit is on S082's corpus and why a zero verdict here is a corpus
  finding to record, not a verdict on the bishop pair.
- **A published bonus is not a floor.** If the fit returns near zero for the
  bishop pair, that is the measurement. Nudging it toward +16 because an
  engine ships +16 is exactly what DEC-105 forbids.

### 6. Measurement

One SPRT, bounds stated in advance with the nElo worst case and the abort rule
(DEC-143). At S105's measured 23.1 to 38.7 games a minute a resolving pair is
roughly 1.5 to 3 hours -- under DEC-155's four-hour line, so daytime work. The
term's speed cost is inside the verdict. A verdict of zero is recorded as zero;
on a failure the bundle is bisected, not zeroed.

### 7. Interactions

- **S134 (blocks this)**: removes the unidentified fourth column. Without it
  this is S027 again.
- **S082 and S083 (before)**: the corpus this fits on. Fitting on the outgoing
  corpus would leave the one unrefuted hypothesis untested.
- **S136 (beside)**: the other half of `--freeze tempo,piece_placement`. Two
  separate fits and two separate verdicts -- one change at a time.
- **S126 (block end)**: refits everything, including whatever this lands.

### 8. References

- - https://github.com/lynx-chess/Lynx/pull/390 -- tapered bishop pair,
  2023-09-10, +8.2 +/- 6.1 / +9.5 +/- 6.7. Pull-request body only.
- - https://github.com/TerjeKir/weiss/pull/231 -- "Tune Rook+Queen", 2020-04-06,
  +9.86 +/- 5.74 / +8.40 +/- 5.42; a retune bundled with the file bonuses.
  Pull-request body only.
- - https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22Tune+Eval%22
  -- #95 "Tune Eval 1", 2019-11-26, CLOP of bishop pair and a king-vulnerability
  term, +16.10 +/- 8.74 / +12.24 +/- 7.21. Commit message only; it is **not**
  the per-feature figure the plan used to carry, which is deleted above.
- - https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22open+file%22
  -- searched 2026-09-13 after the fast check: "Simplify queen open file"
  (#438, 2021-06-07, 0.18 +/- 2.07), "Combine eval functions for NBRQ part2"
  (#187, 2020-02-25, no figure) and #231 above. No per-feature open-file or
  half-open-file figure. Commit messages only.
- - https://api.github.com/search/commits?q=repo:jhonnold/berserk+%22bishop+pair%22
  -- searched 2026-09-13 after the fast check: no commits.
- - https://www.chessprogramming.org/Evaluation_of_Pieces -- a link hub; names
  the two pages below and describes neither. Fetched 2026-09-13.
- - https://www.chessprogramming.org/Bishop_Pair -- "Larry Kaufman proposed the
  value of half a pawn", footnote 1 citing Kaufman 1999, *The Evaluation of
  Material Imbalances*, first published in Chess Life, March 1999, online at
  https://www.chess.com/article/view/the-evaluation-of-material-imbalances-by-im-larry-kaufman;
  **no Elo figure**, and the page attributes the value to the article and to
  no engine. Fetched 2026-09-13, footnote re-read after the fast check.
- - https://www.chessprogramming.org/Rook_on_Open_File -- open-file bonus "vary
  from 8 to 20 centipawns", semi-open "half of that"; **no Elo figure**;
  footnote 1, "20 cp comes from Toga log user manual, after adding actual
  bonus and penalty for a rook on a closed file", which is why the range is
  read for form and **refused as a seed**. Fetched 2026-09-13, footnote
  re-read 2026-09-13 after the fast check.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` row A28 -- the
  record of what was searched for the three deleted per-feature figures and
  why none of them resolved. Read as a record, not as evidence: the numerals
  it holds are the ones DEC-203's rule removed from this file.

## 2026-10-22: phase 1, as designed (blind)

Written by one Opus 5.5 subagent in worktree `chesso-s135` at `41e4140`
while S083's corpus stages held the machine (DEC-260). **Nothing was built,
compiled, tested, fitted, timed or played.** Run, all trivial: `git`,
reading, `clang-format-22` over the changed C++ files (layout only, idempotent
since), `git apply --check` of the cost patch, and two scratch Python scripts
of a few hundred milliseconds -- a second count implementation written from
`src/evaluation.hpp`'s definitions by (file, rank), and python-chess's
`Board.status()` -- which derived every expected count in the new tests,
checked each new FEN valid, and checked the placement block's rank over the
gradient fixture (below). The two Chess Programming Wiki pages were fetched
2026-10-22 (the wiki's temporary read-only recovery, as its banner says).

### The two features, from their descriptions (DEC-258, DEC-221)

**Bad bishop**, CPW *Bad Bishop* (https://www.chessprogramming.org/Bad_Bishop):
"Bad bishop, a bishop whose mobility is restricted by own pawns. As opposed to
normal mobility problems, this situation is semi-permanent (more so if the
position is blocked), which fact justifies singling it out as another
evaluation term." Implemented as DEC-258's form, "a bishop's own pawns on its
colour (blocked ones counted separately)", in **two columns**:

- `bad_bishop`: for each own bishop, the own pawns on squares of its colour,
  summed over the bishops (a count; two bishops on one colour each count it).
- `bad_bishop_rammed`: the same sum over the rammed own pawns alone -- the
  square directly ahead holds an enemy pawn. "Blocked" is read as rammed
  because the page's point is that the restriction is semi-permanent "more so
  if the position is blocked"; a piece in front moves away. A subset of the
  first, so the fit prices the rammed share on top.
- Rejected: blocked by any piece (reads occupancy, not semi-permanent);
  forward mobility only or "outside the pawn chain" (the page's "some
  programs, like Crafty" variant -- needs attack sets, a refinement for later);
  one column that counts a blocked pawn twice (an arbitrary weight ratio the
  fit could not move).

**Trapped pieces**, CPW *Trapped Pieces*
(https://www.chessprogramming.org/Trapped_Pieces): "Trapped pieces are the
extreme examples of poor mobility. They are awkwardly placed and often
endangered. A typical example of a trapped piece is a white Bishop on h7
blocked by the black pawns on f7 and g6." and its list of examples, each
implemented: knight h8 with an enemy pawn on h7 or f7; bishop h6 with g5 and
f6; knight h7 with g7 and h6; knight h7 with g7 and f6; rook h1/g1/h2/g2 with
the own king on f1 or g1. **Two columns**:

- `trapped_minor`: own knights and bishops on one of the minor patterns, each
  piece once.
- `trapped_rook`: own rooks in the king's corner as listed.
- Both mirrored across the d/e files (the a side) and, for Black, across the
  board's middle: the page gives "examples", and an h-side-only feature would
  prefer a wing for no stated reason.
- Rejected: one column (the page itself scores the patterns differently -- a
  bishop "decreased by about 150", h6 "less severely", the rook "perhaps -40"
  -- and over the 2696 suite positions a rook pattern occurs 387 times against
  80 for a minor, so one weight would price a trapped bishop as a hemmed rook);
  one column per pattern (eight sparse columns, unfittable on a corpus);
  the literal h side only.
- **No number from either page is used** (DEC-105, DEC-134): the three
  penalties above are not seeds, and nothing is seeded -- every weight starts
  the fit at zero. No engine's code, table or constant was read.

The columns are **appended**, indices 3 to 6 (`src/evaluation.cpp`
`PL_BAD_BISHOP`, `PL_BAD_BISHOP_RAMMED`, `PL_TRAPPED_MINOR`,
`PL_TRAPPED_ROOK`), so the first three keep the indices every fit and record
has used. **They stay in the piece placement group** rather than a new group:
the same stage, the same trace term (`TERM_PIECE_PLACEMENT`, whose width
`tools/tuner_trace.hpp` reads from the layout) and the same taper, so the
exact reconstruction holds with no tuner change and the family is one
`--only piece_placement`; a new group would need a new trace term, a new
range before tempo, the partition re-ended and the family split across two
`--only`s. The **deviation from the brief**: the brief says "each a column";
DEC-258's own wording ("blocked ones counted separately") and the trapped
page's different severities make each feature two. Two features, five
screened terms, seven columns. Collapsing either to one column is a
mechanical edit if the coordinator rules otherwise.

### The stage, and why INV-4 allows it

Stage one, inside `src/evaluation.cpp` `piece_placement_features`, beside
the three it extends: the four read pieces, pawns and the king square, no
occupancy and no attack set -- nothing the expensive stage builds -- so
nothing about them belongs behind the lazy shortcut and its clamp, the reason
`src/evaluation.cpp` `evaluate_cheap`'s comment gives for the three placement
features before them. Each is computed
from the bitboards at every evaluation in a fixed number of operations (six
popcounts and two multiplies a side for the bad bishop pair; an early return
when no own minor stands on any of the six trap squares; two masks and a
popcount for the rook), the form the three pawn terms and the existing
placement features already take. INV-4 holds the accumulators `make_move`
maintains -- material, the tables, the phase -- and none of these is a sum
over pieces: each is a product or conjunction of two piece sets, usually
including the other side's pawns, with no O(1) delta to accumulate. The
accumulators are untouched.

### Code, tests, widths

- `src/evaluation.cpp`: the enum, the zero weights (seven wide),
  `piece_placement_reach` (1 for the bishop pair, 15 a rook feature, 56 for
  the bad bishop products -- 7 x 8 bishops and pawns within 15 pieces -- and 6
  and 4 for the trap squares), the stage-one bound rewritten (below),
  `one_colour`, `pawns_on_bishop_colour`, `pattern_square`,
  `trapped_minor_count`, `trapped_rook_count`, the counts filled in
  `piece_placement_features`, the accessor's width. `src/evaluation.hpp`: the
  four definitions stated exactly (the walk is written from them), widths 7.
- `tools/tuner_groups.hpp` `PLACEMENT_TERMS`: the five terms as `--only` and
  `--freeze` names, outside `GROUP_LIST` (each overlaps its group).
  `tools/tuner.cpp`: usage, refusal messages, the emitted array's comment.
  `tools/tuner_layout.hpp`: comments. `tools/feature_audit.cpp`: seven names,
  four collinearity targets (bishop+pawn, bishop+pawn, knight+bishop+pawn,
  rook+king tables), seven planted values a phase.
- Tests: `tests/test_walked_counts.hpp` walks the four square by square from
  the definitions; `tests/test_eval_model.cpp` "the walk and the engine count
  piece placement by hand" gains 24 cases (each clause, both colours, the
  a-side mirror, a knight meeting two patterns counted once, a piece in front
  not ramming, two rooks counting two, all four at once), each stating its
  arithmetic, the older cases zero-filled and checked zero by code;
  `tests/test_eval_positions.hpp` six positions (three and their mirrors, at
  phases 7, 6 and 5) so every count reads both ways round and both phase
  weights get a gradient; `tests/test_eval_trace.cpp` the width;
  `tests/test_tuner_groups.cpp` "the piece placement terms partition the
  piece placement group" and "--freeze takes terms, and clears exactly their
  columns"; `tests/test_tuner_gradient.cpp` "a planted vector comes back
  through the trace path" plants all fourteen placement weights.
- `.tuning/verify_fit.py`: `piece_placement` 7 wide, `TOTAL` 831.
  `.tuning/apply_fit.py` reads no width and is unchanged.
  `adocs/data/S192_anchors.py` is unchanged: its twelve anchor positions read
  zero on all four counts (checked by code), so its three-wide tuples stay
  exact at any weight.
- `DEV_MANUAL.md`: 823 to 831 where stated as current, the paste table, the
  58 non-table parameters, the partition's widths (5, 768, 8, 18, 10, 6, 14,
  2) and a paragraph on the terms. `adocs/data/README.md`: S135's six files.
- **`PARAM_COUNT` 823 -> 831**: the placement block 6 -> 14 at `PL_MG_BASE`
  815, `PL_EG_BASE` 822 (was 818); tempo 829 and 830 (was 821 and 822).
  `--freeze tempo,piece_placement` still leaves 815 free.

Checked by code over the fixture (the scratch reference, 40 curated
positions, their mirrors and the 2696 suite positions, `trace_positions()`'s
2776 rows): the fourteen placement coefficient columns have rank 14, so the
planted recovery's answer is unique; over the curated list each of the seven
counts is non-zero for White, for Black, White ahead and Black ahead, at a
non-zero phase and below 24.

### The stage-one bound, rewritten (fast check, 2026-10-22)

`src/evaluation.cpp` `stage_one_bound` feeds one `static_assert` and nothing
else (grep: its definition and that assert). At S134's bound -- 32 times the
largest entry of either half, the king table's 841, 26912, plus passers
2 x 15 x 96 = 2880 and structure 3 x 15 x 32 = 1440 -- it stood at 31232 of
32767, leaving 1535 for every placement weight times its reach: the sanity
bands alone (47 x 15 for the pair, 24 x 15, 12 x 15) took 1245, and any bad
bishop weight of 6 or more would have failed the candidate's build hours into
phase 2. Now, behaviour-neutral (the assert's input alone):

- The bishop pair's reach is its true 1.
- The tables are bounded **per half and by spread**: White's pieces add
  their entries and Black's subtract theirs (`src/eval_tables.hpp`
  `eval_add_piece`), and load_FEN accepts one king and at most fifteen others
  a colour, so the two kings together lie within (the king table's highest
  entry or 0) minus (its lowest or 0), and the thirty others within fifteen
  times the same spread over the other five tables. The 0s keep it true for
  every subset make_move passes through; the unreachable pawn squares are
  read too, which only widens it. Each half is then bounded on its own, which
  is what make_score() needs -- each half in [-32767, 32767].
- **At today's tables** (`41e4140`): middlegame 1020 (kings 841, -179) + 15 x
  1071 (queen 793, bishop -278) = 17085, passers 30 x 59 = 1770, structure
  45 x 11 = 495: **19350, room 13417**; endgame 439 + 15 x 397 = 6394, passers
  30 x 96 = 2880, structure 45 x 32 = 1440: **10714, room 22053**
  (`adocs/data/S135_screen.py`'s `headroom`, run on the worktree, agrees).
  The sanity bands then take 47 + 360 + 180 = 587, leaving a bad bishop
  weight up to 229 in the middlegame before the assert fires.
- **Recomputed on S083's tables after the rebase** (`headroom` on any
  header, or on the rebased `src/evaluation.cpp`), and printed for every fit:
  `read` per term, `candidate` and `part` refusing a fit past 32767, `apply`
  refusing before it pastes.
- **If a fit legitimately passes it** (pre-registered, not a surprise): it is
  neither pasted nor clipped -- clipping ships an unfitted constant (DEC-084).
  The step pauses for the coordinator. First answer: a tighter valid bound,
  behaviour-neutral (for instance the bishops, pawns and rooks sharing one
  side's fifteen pieces, so the reaches cannot all be met at once). Second,
  the owner's: an unpacked stage-one sum, a speed change to time.

### The screen, the candidate, the bisection, the cost, the SPRT

`adocs/data/S135_screen.sh`'s header is the pre-registration and
`adocs/data/S135_screen.py` its mechanical reader; in short:

- **Corpus**: `SOURCE`, the coordinator's statement of S083's verdict (s083
  on H1; on H0 or no verdict its recorded choice, s083 or s082), and `locate`
  finds the file mechanically -- S083's candidate is the corpus of the
  `candidate` fit of the last stage whose reading printed `READ-OUTCOME stop`;
  s082 is S083's stage 0 top rung. sha256 from that stage's samples.log; a
  removed file is re-drawn from its pool and must hash the same. Validation:
  S082's fixed qs view.
- **Identity first**: `audit` runs `tools/feature_audit` and passes only on
  rank exactly 792 of 831 with S134's 39 null directions and no new
  dependency; anything else stops for the coordinator.
- **The tuner is HEAD's, checked**: each frozen tuner copy is run for zero
  epochs on the corpus's first 2000 rows and its emitted header compared with
  HEAD's sources value by value (`start-check`), refusing on any difference;
  every fit's header is checked the same way after, its own free columns
  excepted; `only-placement --free` refuses a paste that moves a placement
  column outside the fit's free terms. A tuner built before S083's paste or
  before a bisection part landed would otherwise start from other constants
  and re-emit a landed part's columns at their old values.
- **Fits**: DEC-304's settings, `--only piece_placement.<term>`, on the full
  corpus and on five quarter samples (seeds 1 to 5).
- **Margin**: gain g = start minus best validation in the same run; s = the
  five quarter gains' standard deviation; m = max(1.645 s, 2e-6); kept iff
  the full-corpus g > m. **Argued against DEC-304's form**: no sqrt(2),
  because the start figure does not move with the training rows; the 2e-6
  floor is two units of the tuner's sixth printed decimal. *(Corrected at
  phase 1b's fast check, 2026-10-25, the rule unchanged: s measures
  training-corpus resampling alone; the fixed set's sampling noise and the
  best-report selection on it are common to all six fits, not priced, and
  bias g upward; the "noise-free" baseline and "erring toward dropping" this
  bullet first said are withdrawn. Accepted because the screen only removes
  terms and the one SPRT decides what ships, and each fit frees 2 (bishop
  pair, rook open, rook half open) or 4 (bad bishop, trapped) parameters
  against N = 1015332 validation rows from 12000 games, so the selection bias
  is taken as small, not measured; `adocs/data/S135_screen.sh` THE MARGIN.)*
  Alternative rejected: DEC-304's m = 1.645 sqrt(2) s at C/2 -- about the same
  margin numerically at twice the noise-fit cost.
- **Candidate**: the survivors fitted together, `--only piece_placement
  --freeze <dropped terms>`. No survivor: no SPRT, five zeros recorded.
- **Bisection on H0 or no verdict, written now**: A = S027's three, B =
  DEC-258's two; A -> A1 (bishop pair) | A2 (the two rook files) -> the two;
  B -> B1 (bad bishop) | B2 (trapped). Over the survivors; empty halves
  dropped; each part refitted alone and measured by its own SPRT in
  `adocs/data/S135_sprt.sh`'s shape against the tree as it stands; A before
  B; a part's H1 lands and later parts fit on top of it. Alternative
  rejected: a split by cost (the bishop pair alone, DEC-091's recommended
  form) -- the owner chose the bundle, and the lineage split keeps the three
  S100 diagnosed apart from the two never measured.
- **Speed cost** (`adocs/data/S135_cost.sh`, `adocs/data/S135_cost_only.patch`,
  `adocs/data/S135_cost.py`): owed as the record of what the verdict paid,
  deciding nothing -- the candidate's weights are non-zero, so the SPRT
  carries the cost (DEC-091, DEC-047). Cost-only builds force the seven
  (0x7f), S027's three (0x07) and S135's four (0x78) behind an asm barrier;
  all padded (DEC-266 (1)); INV-6 streams identical first; S270's method,
  16 interleaved pairs, perf_counters on `bench 16` and pos11, hyperfine
  wall; floor from a byte-copy A/A; DEC-266 (3)-(4) reading. A `parent`
  comparison checks the zero-weight landing is free on g++ 13.3.
- **SPRT** (`adocs/data/S135_sprt.sh`): CAND the candidate's paste, REF its
  parent, `{0, 5}` nElo at 8+0.08, Hash 16; 41861 games worst case (19.84 h),
  25591 on a bound (12.13 h); cap 20950 rounds; abort at 1 % forfeits, crash,
  power, load, banner, or 40 h; H1 ships, H0 and no verdict revert and
  bisect; the coordinator's bracketed marker unfilled. Guards: CAND^ is REF,
  `src/eval_tables.hpp` unchanged, only `src/evaluation.cpp` under src/.

### Commands in order (phase 2), with expected times

0. **Ordering: S135 lands only after S083's commit** (its paste, or its
   revert on H0). Landed first, S083's apply would run `.tuning/apply_fit.py`
   with S083's three-wide placement arrays over S135's seven-wide ones --
   three values written into a seven-wide array, which C++ zero-fills and
   compiles -- and only then would S135's `.tuning/verify_fit.py` (`TOTAL`
   831, `piece_placement` 7) refuse the header, on a tree already rewritten.
   The coordinator's S135 landing commit also changes `adocs/specs.md`'s
   "absent, evaluation" row (its "823 fitted and 815 shipped"), wording below.
1. Rebase `s135` onto the tree S083 leaves (its paste or its revert); the
   conflict surface is `src/evaluation.cpp`'s weight lines. Minutes. Then
   recompute the stage-one headroom on S083's tables (`adocs/data/S135_screen.py
   headroom src/evaluation.cpp --tree .` in the rebased worktree) and record
   it here, and re-read the weight-dependent preconditions below at S083's
   constants.
2. In the worktree: the doctest submodule (`.moltke.local.md`), then
   `export CLANG_FORMAT_MAJOR=22; cmake --build build -j8 && ctest --test-dir
   build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest
   --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh
   --check` -- build 2 min, gate 4 min (S083's figures). Tier 2 does not bind
   (no search, generator or make/unmake change).
3. INV-6: `chesso bench`, `bench 12`, `tools/search_bench.py` at 9 and 12
   against the parent, timings stripped -- about 2 min. Land as `No
   functional change` (the bench total the parent's).
4. `adocs/data/S135_cost.sh run`, idle machine: about 50 min (S273's figures),
   ceiling 2 h.
5. `SOURCE=<s083|s082> adocs/data/S135_screen.sh locate` (under a minute),
   then `audit` (40 s to 2 min), then `screen`: 1 h 15 m typical / 2 h 30 m
   worst on stage 0's 6.2 M rows, 3 h 25 m / 6 h 45 m on stage 2's ~17 M --
   0.0252 s an epoch a million rows, measured on S083's stage-0 top rung;
   2500 epochs typical (S082/S083's band), 5000 worst. Night if over 4 h.
6. `candidate`: one full fit, 6.5 to 36 min by stage.
7. `apply .tuning/s135/candidate/fit.hpp TREE` (S083_corpus.sh's apply, then
   the placement-only check), then every golden the paste moves re-derived by
   its script -- S083's step file "The landing" table; `S192_anchors.py`'s
   anchors read zero on the new counts and move only through the rook-open
   weight. About 2 to 3 h by S083's estimate.
8. Pin and run `adocs/data/S135_sprt.sh`: up to 19.9 h, night.
9. On H0 or no verdict: `part NAME` and a copied SPRT per part, in the order
   `read` printed; each part up to 19.9 h plus a fit.

### What must be built and tested later, and what each should print

- Both builds compile with `-Wall -Wextra -Werror`; the fast suite passes in
  both, the test count the parent's (no target added); format check clean.
- `test_eval_model`: the three placement cases green, the hand-case
  non-vacuity checks claiming all seven counts both ways.
- `test_eval_trace`: the coefficient case green over every position (the
  trace's placement coefficients are the accessor's seven differences);
  exact reconstruction unchanged.
- `test_tuner_groups`: all cases green, the two new ones included.
- `test_tuner_gradient`: "gradient() is the derivative of the error it
  descends" checks 831 parameters, worst under 1e-6, every term parameter's
  gradient non-zero; "a planted vector comes back through the trace path"
  recovers all fourteen placement weights within 0.5 (**at risk**, below).
- `chesso bench`: 6042933, the parent's (or the rebased parent's if S083
  landed); `tools/search_bench.py` d9 and d12 node counts and best moves the
  parent's; `bench 12` stream identical.
- `build/tools/tuner --help` lists the five terms; a run with `--only
  piece_placement.bad_bishop` prints `831 parameters, group
  piece_placement.bad_bishop, freeze (nothing), 4 free`; with `--only
  piece_placement --freeze piece_placement.trapped`, `10 free`.
- `.tuning/verify_fit.py` on any S135-built fit: `831 parameters compared`,
  exit 0; on a pre-S135 (823) header, a width mismatch and exit 1.
- `S135_screen.sh audit`: `rank over the rationals exactly 792 of 831,
  nullity 39`. `S135_cost.sh`: INV-6 identical for all five builds; `parent`
  reads zero.
- **Weight-dependent preconditions, re-read at S083's constants after the
  rebase** (each with `-s`, and a red one is a fixture question, never a
  loosened threshold): `tests/test_tuner_gradient.cpp` "the fixture sits
  clear of the clamp at the shipped constants" -- every fixture row, the six
  new ones included, more than a centipawn inside `LAZY_EVAL_MARGIN`; the walk
  floor in "the fit holds every gauge still and no score sees the hold" --
  each plain-update shift above 0.1, `apart` under 1e-6, `travelled` above
  50; and "the clamp's derivative is exact where it binds" -- every row more
  than a centipawn from the kink at +40 on king safety, the clamp binding on
  some rows and not all.

### Not settled without the CPU

- **The planted recovery's convergence**: rank 14 is checked, convergence in
  6000 epochs is not; the rammed columns are non-zero on 8 of 2776 rows. A
  miss is fixed by fixture rows or epochs, never by the 0.5.
- **"the fit holds every gauge still"** runs AdaGrad at step 10 on the
  fixture plus four, now 44 rows; its comment records chaos at step 30 on 38.
  If `apart` exceeds 1e-6 the fixture change is the first suspect.
- Whether g++ 13.3 deletes the zero-weight columns (the comments' "the
  compiler deletes" was measured on clang): `S135_cost.sh`'s `parent`.
- Whether the trapped minor occurs in the corpus at all: `audit`'s rank says.
- A single-term fit's epoch count (taken as 2500 typical).
- The bisection's worst case, eight SPRTs, about 159 h.

### Proposed decisions (the coordinator's to record)

- **DEC-314** S135's two placement features are four columns. Decision: bad
  bishop as own pawns on each bishop's colour summed over bishops, plus the
  rammed share; trapped pieces as minors on CPW's listed patterns, each once,
  plus rooks in the own king's corner; both mirrored to the a side and to
  Black; appended to piece_placement (indices 3 to 6), screened as two terms.
  Why: DEC-258's "blocked ones counted separately" and the page's own
  different severities; one group keeps one trace term, one taper, one
  `--only`. Rejected: one column each; a column per pattern; a new group.
- **DEC-315** S135's screen rule. Decision: per term, gain against the run's
  own start figure on S082's fixed set, kept past m = max(1.645 s, 2e-6), s
  from five quarter-corpus fits; candidate the survivors together; bisection
  A/B by lineage, then as written. Why: s prices training-corpus resampling
  only, and the start figure does not move with it, so no sqrt(2); the fixed
  set's sampling noise and the best-report selection on it are common to the
  six fits and unpriced, accepted because the screen only removes terms, the
  one SPRT decides what ships, and a fit frees 2 or 4 parameters against
  1015332 validation rows. Rejected: DEC-304's form verbatim; a cost-based
  partition.

### Wording proposals

- `adocs/specs.md` evaluation row, after the passed pawn sentence: "Piece
  placement is seven counts since S135: the bishop pair, rook on an open
  file, rook on a half-open file, and DEC-258's bad bishop (own pawns on each
  bishop's colour, and their rammed share) and trapped pieces (minors on
  CPW's listed patterns, rooks in the own king's corner), computed in the
  cheap stage beside the pawn terms."
- `adocs/specs.md` "absent, evaluation" row, in S135's landing commit (the
  coordinator's edit), its first two sentences, which now read "bishop pair,
  rook on an open file, rook on a half-open file, tempo — all four present in
  the code, all four shipped at zero weight. At zero the compiler deletes
  them, so they cost nothing; the tuner still carries their 8 parameters,
  making 823 fitted and 815 shipped.", replaced by: "bishop pair, rook on an
  open file, rook on a half-open file, S135's bad bishop and its rammed
  share, trapped minor and trapped rook, tempo — all eight present in the
  code, all eight shipped at zero weight. At zero the compiler deletes them,
  so they cost nothing (on g++ 13.3 by S135's cost run's parent comparison);
  the tuner carries their 16 parameters, making 831 fitted and 815 shipped."
  The candidate's paste, if it ships, moves the survivors out of this row.
- `MANUAL.md`: checked, no change -- the UCI surface and the engine's play are
  unchanged at zero weights.
- `DEV_MANUAL.md`: changed in this diff, as listed above.

### Proposed `done:` stamp elements

"Implemented from the description: CPW *Bad Bishop* and *Trapped Pieces*,
fetched 2026-10-22; no engine's code, table or constant, no number from
either page (DEC-221, DEC-105)." plus the screen's figures, the candidate's
weights, the verdict, the cost reading, and the bisection's parts if any.

## 2026-10-25: the rebase on S083's tree

Done by one Opus 5.5 subagent in worktree `chesso-s135b`, branch `s135b` at
`b7be983` (S083's constants landed at `01222f3`, DEC-313; S290 after them),
from `.tuning/coord/S135/s135_phase1.diff` (sha256 `e0472c18...`, checked).
The machine was the subagent's for builds, tests, bench, `search_bench.py` and
the golden scripts; no fit, match, datagen or timing ran. Every figure below is
in `adocs/data/S135_preconditions.txt` with the command that printed it.

### Conflicts and their resolution

- `git apply --3way --index`: 21 of 22 files applied cleanly (direct
  application). One textual conflict, `adocs/data/README.md`: ours held
  S083's later `S083_sprt.sh` row (working-tree mode since DEC-307 and
  DEC-309) and the S083, S289 to S293 and S290 rows and sections; theirs held
  `41e4140`'s older `S083_sprt.sh` row and S135's section. Resolved as true on
  this tree: ours whole, S135's section appended after S290's, newest last.
- Semantic, by diffing `41e4140..b7be983` on every file phase 1 touches: S083
  moved `src/evaluation.cpp`'s passer, structure, mobility and king safety
  weights, `tests/test_eval_positions.hpp`'s comment on the four former
  truncation rows (kept in the list as coverage rows, so the list is still the
  34 the six new rows follow), `tests/test_eval_trace.cpp`'s
  `truncation_positions`, and `tests/test_tuner_gradient.cpp`'s clamp push
  (+100) and walk floor (0.01); phase 1's hunks sit outside each. What the
  rebase made untrue, corrected: the stage-one bound's comment (S083's figures
  beside S134's), `tools/tuner_trace.hpp` `model_t`'s "823", stated as
  current, and the clamp, walk and gradient-worst comments in
  `tests/test_tuner_gradient.cpp` and `DEV_MANUAL.md` (S135's fixture figures
  beside S083's; no assertion touched). No golden conflicted and none moved
  (INV-6 below), so none was resolved.

### Builds and tests, both builds

Release and tune, `-j12`, g++ 13.3: **0 warnings**. Fast suite **50 of 50**
in both (Release 117.7 s, tune 121.2 s), `./clang-format.sh --check` clean
(`CLANG_FORMAT_MAJOR=22`). After the last edit (comments in `src/`, `tools/` and `tests/` only), both builds rebuilt with 0 warnings, 50 of 50 in both again, the format check clean, bench 3945487 in both. The
blind fast-check fixes, built for the first time, and what each reads now:

1. **The stage-one bound, per half by spread**: compiles; **mg 12565, eg 12165
   of 32767, room 20202 and 20602** -- the same from `S135_screen.py headroom`
   and from `static_assert`s on a scratch copy of `src/evaluation.cpp`. A
   header with a bad bishop mg weight of 400 is refused by `headroom` at 34965
   and, pasted into a scratch tree, fails the build at the assert: the script
   and the compiler agree at the boundary.
2. **The stale-tuner start-check**: refuses a zero-epoch header against
   `b7be983` (three wide at HEAD) and S083's raw 823-wide fit against the
   landing; passes the zero-epoch 831 header against the landing's sources
   (held in a scratch repository, not this tree's history).
3. **The cost script's rerun reset and parent check**: `run` refuses here,
   "HEAD does not carry S135's columns"; the pair check runs only once the
   landing is committed.
4. **The chess claim dropped**: text; nothing to build.
5. **The ordering notes**: met; S083 landed first.

Also built: the cost-only build at `S135_COST_MASK` 0x7f (padded, 0
warnings); 0x07 and 0x78 compile with `-Wall -Wextra -Werror`.

### INV-6, neutral at zero weights

Bench **3945487** and `bench 12` **2103576** in both builds, the parent's,
every line identical with the timing fields stripped; `tools/search_bench.py`
at 9 (42570, 152468, 27750 nodes) and at 12 (250176, 412312, 157359), best
moves c3d5, e2a6, d7c8q, identical in all four binaries (Release and tune,
candidate and parent). The 0x7f cost build reads the same streams. This lands
as `No functional change` with `Bench: 3945487`.

**Tier 2 does not bind**: `git diff --cached --stat -- src/` names
`src/evaluation.cpp` and `src/evaluation.hpp` alone -- no search, generator,
`make_move` or `unmake_move` code.

### The weight-dependent preconditions, re-read on S083's tables

- **Stage-one bound**: above. Phase 1's 19350 and 10714 were `41e4140`'s.
  After the sanity bands' 587, one bad bishop column could reach 350 a weight
  in the middlegame before the assert fires.
- **The trace's exact reconstruction, the tuner's own check, on every row of
  the corpus S135 fits** (`--epochs 0`, no fit): `trace: 6226944 rows,
  164780480 entries (26.46 a row), 5.1 s; 0 differ from evaluate(), the clamp
  binds on 28761`; 0 corpus rows on a validation position. The screen's
  baseline: K 0.7224, held-out 0.071831.
- **Identity** (`S135_screen.sh audit`, for real, on that corpus): rank over
  the rationals exactly 792 of 831, nullity 39, S134's 39 and no more; R^2 of
  the four new columns on their tables 0.668, 0.162, 0.371, 0.352.
- **Widths**: `.tuning/verify_fit.py` 831 of 831 identical on the zero-epoch
  header, exit 0; S083's 823-wide fit refused by width, exit 1;
  `.tuning/apply_fit.py` and `.tuning/diff_fit.py` read no width. The tuner's
  free counts: `piece_placement.bad_bishop` 4, `piece_placement` with
  `trapped` frozen 10, `piece_placement` 14, `--freeze tempo,piece_placement`
  815, as phase 1 expected.
- **`tests/test_tuner_gradient.cpp`, with `-s`**, on the 40-row fixture:
  "sits clear of the clamp" the largest |stage two| 99.33 against 183; "the
  clamp's derivative is exact where it binds" -- re-derived by
  `adocs/data/S083_tuner_preconditions.cpp clamp`, the rule still picks +100,
  the same eight rows, 434.5, 161.0 and 23 from the kink, finite differences
  6.0e-8 (the parent's 1.09e-7); "the fit holds every gauge still" -- the
  walk's smallest 0.3663 under Adam and 0.0437 under AdaGrad, both the rook's,
  above 0.01, holds at most 1.38e-14, runs at most 1.02e-12 apart, travelled
  about 1600; "a planted vector comes back" -- all fourteen placement weights
  within 1.54e-5, against the case's 0.5, so phase 1's "at risk" is settled; the nonlinear
  group's recovery, which S083 found moved by appended rows, green.
- **A reading, green, recorded and not acted on**: "gradient() is the
  derivative" reads **3.99e-7** at parameter 774 (the bishop's middlegame
  mobility weight) against the parent's 5.87e-8 there: the six rows raised it
  6.8 times, and it sits 2.5 times inside the case's 1e-6. The next fixture
  row or refit should read it.
- **`adocs/data/S192_anchors.py`**: 10 of 10 reproduced; on its twelve anchor
  positions the engine's `piece_placement_counts` reads every side's count in
  columns 3 to 6 zero, so its three-wide tuples stay exact at any weight of
  the four.

### The scripts, repointed and dry-run

- **`S135_screen.sh` / `.py`**: `SOURCE` settled to s083 by S083's H1 and
  nothing else accepted; the corpus pinned, `CORPUS_SHA` `2e0e3de9...`, curve
  0's top rung `cand_6226944_s1.tsv` (`locate` refuses another); `S082_OUT`
  overridable as `S083_OUT` is, and `--s082-out` for the re-draw's views;
  the stage-one figures at S083's tables; the hours from measured epochs (a
  fit runs to its best report plus 2000 epochs of patience, capped at 5000;
  S083's eight curve-0 fits ran 2500 to 5000, median 4300 -- phase 1's
  "2500 typical" was the best epoch, not the run); stage 0 alone, the disk
  0.62 GB. Dry runs, `OUT` in a scratch directory: no stage, exit 2;
  `SOURCE=s082 locate` refused; `audit` before `locate` refused; **`locate`
  DONE** (2.3 s, the pinned sha) and **`audit` DONE** (36 s, rank 792), both
  real; `screen` refused with a pre-S135 tuner (the main checkout's) and, with
  this tree's, at the probe's start-check before any fit (HEAD three wide);
  `read` over synthetic logs in the tuner's real format -- a term kept past
  m, one dropped at the 2e-6 floor, one inside 1.645 s, the freeze list, the
  bisection `family -> [A, B -> [B1, B2]]` -- and unreadable on two baselines
  and on a missing fit; `part` A, B and B1 printed, A2 refused when it holds
  no survivor; `candidate` (no reading), `part` (no name) and `apply` (no
  `free.txt`) refused.
- **`S135_cost.sh`**: no stage, exit 2; `run` refused (HEAD lacks
  `PL_TRAPPED_ROOK`); `S135_cost_only.patch` applies to the rebased file and
  its 0x7f build searches the zero build's tree; its hours from S083's node
  counts.
- **`S135_sprt.sh`**: refuses on its marker, left unfilled; on `TC`
  overridden; on `ROUNDS` set. The corpus wording settled; bounds, cap, costs,
  abort rule and bisection as written.

### The coordinator's commands, in order, with times

0. **Land this diff** as `No functional change`, `Bench: 3945487`, with
   DEC-314 and DEC-315 and the specs wording below; the gate, build about 2
   min and both suites 4.
1. **Cost**: `nohup adocs/data/S135_cost.sh run > .tuning/s135_cost.console
   2>&1 &` from the main checkout, HEAD the landing and HEAD^ `b7be983` (it
   refuses otherwise: before any later commit), the machine idle. **About 1 h
   05 m, ceiling 2 h 30 m**, 1.2 GB of worktrees: `bench 16` 11370791 nodes
   and pos11 38413692 at S083's weights against S273's 10214429 and about
   26.5 M, at S273's 4547095 nps.
2. **Screen**: `adocs/data/S135_screen.sh locate` (3 s), `audit` (40 s), then
   `nohup adocs/data/S135_screen.sh screen > .tuning/s135_screen.console 2>&1
   &`: **1 h 02 m / 2 h 07 m / 2 h 27 m** (every fit at its first report /
   S083's median / the cap), ceiling 5 h, daytime (DEC-155); it ends with
   `read`.
3. **Candidate** on `SCREEN-OUTCOME candidate`: one full fit, **5.5 / 11.2 /
   13.1 min**, ceiling 30 min. On `none`: no SPRT, five zeros recorded.
4. **Apply**: `adocs/data/S135_screen.sh apply .tuning/s135/candidate/fit.hpp
   TREE`, a clean worktree at the landing: the paste, `verify_fit` 831, both
   builds, bench, both suites, then `only-placement`, **about 10 min**.
5. **Goldens** the paste moves, each by its script (S083's step file, "The
   landing" table; `S192_anchors.py`'s anchors move through `rook_open`
   alone): **about 1 to 3 h** by S083's and S110's figures; a mate floor that
   falls stops for the owner (DEC-304, DEC-303).
6. **SPRT**: commit the candidate (`Bench:` its total), pin and fill
   `adocs/data/S135_sprt.sh`, run it detached: **up to 19.86 h** at the cap,
   41900 games at 2110 an hour, 12.13 h with the truth on a bound; night
   (DEC-155); watcher ceiling 40 h.
7. **On H0 or no verdict**: revert whole, bisect in `reading.txt`'s order,
   each part a fit (5.5 to 13.1 min), its apply, goldens and SPRT; eight parts
   at worst, about 159 h.

### Decisions, renumbered

Phase 1's DEC-305 and DEC-306 were taken by S083's pause and S289's guard
before they landed, and DEC-308 to DEC-313 since; the drafts are **DEC-314**
(the two features as four columns) and **DEC-315** (the screen rule),
renumbered in place in "Proposed decisions" above. DEC-314's text stands; DEC-315's `Why` was rewritten at phase 1b's fast check (what s prices and what it does not), its rule unchanged.

### Wording for this landing, `No functional change`

- **`adocs/specs.md` "absent, evaluation" row**, its first two sentences
  replaced: "bishop pair, rook on an open file, rook on a half-open file,
  S135's bad bishop and its rammed share, trapped minor and trapped rook,
  tempo — all eight present in the code, all eight shipped at zero weight. At
  zero they change no node (S135's INV-6: bench and `search_bench.py`
  identical), and the tuner carries their 16 parameters, making 831 fitted and
  815 shipped; whether g++ 13.3 deletes the four new columns as the three
  before them are deleted is S135's cost run's `parent` comparison." Its
  identity sentence gains: "and over the 831 since S135, on S083's corpus,
  rank exactly 792, the same 39, the four new columns identified (R² 0.668,
  0.162, 0.371, 0.352 on their tables)". The cost run's reading then replaces
  the clause it answers.
- **`adocs/specs.md` evaluation row: nothing at this landing.** Phase 1's
  sentence says the seven are "computed in the cheap stage", which at zero
  weight they need not be; it lands with the candidate's paste, if a survivor
  ships, and names the survivors.
- **`MANUAL.md`**: checked, no change; the UCI surface and play are unchanged.
- **`DEV_MANUAL.md`**: changed in this diff -- phase 1's widths, partition and
  terms paragraph, and the gradient-worst sentence at the rebase.
