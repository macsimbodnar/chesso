id:         S135
goal:       unfreeze the piece placement group, add bad-bishop and trapped-piece features, and refit it, one SPRT over the family by the owner's decisions of 2026-08-20 and 2026-10-08
accepts:    S134 has landed first, so the group is three identified features and carries no unidentified column -- a bundled verdict over this bundle is at least attributable, which S027's was not; the fit is `--only piece_placement` on the corpus S082 and S083 produce, so the SPRT measures the term and not a joint refit of the other 820 constants; bad-bishop and trapped-piece features join the group (DEC-258), each defined from its description before any fit (CPW *Bad Bishop*, *Trapped Pieces*) and each feature screened before the one SPRT (DEC-259); the SPRT bounds are chosen for the effect size and stated in advance, not `--fast` -- DEC-063 is the measurement that the pair and not the hardware sets the cost, and the published parts are +8.2 and +9.86 (DEC-084, order of magnitude only); the term's own speed cost is inside its verdict, measured with the weights forced non-zero because at zero the compiler deletes it (DEC-047); a verdict of zero is recorded as zero, and **if the bundle fails it is bisected** rather than zeroed by hand, which is the S027 failure this step exists not to repeat (DEC-082's bisect rule); the fast suite green
touches:    src/evaluation.cpp piece_placement_mg/eg and the four new counts, src/evaluation.hpp, tools/tuner_groups.hpp, tools/tuner.cpp, tools/tuner_layout.hpp, tools/feature_audit.cpp, tests/test_eval_model.cpp, tests/test_eval_positions.hpp, tests/test_eval_trace.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, tests/test_walked_counts.hpp, tests/test_evaluation.cpp and every other golden the fit's paste moves (each re-derived by its script: tests/test_search.cpp, tests/test_mate_breadth.cpp, tests/test_engine.cpp, tests/test_mate_carry.cpp, adocs/data/S170_cases.tsv, adocs/data/S192_anchors.py, MANUAL.md), src/evaluation.cpp add_placement_column (the placement sum's form, behaviour-neutral), tests/test_eval_positions.hpp's DEC-316 pair, tools/tuner_trace.hpp (one comment's parameter count, at the rebase; the trace's term table reads the width), .tuning/verify_fit.py, DEV_MANUAL.md, adocs/data/README.md, adocs/data/S135_*; src/eval_tables.hpp named before phase 1 and needing no change (the tables are not fitted)
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

## 2026-10-25: the cost run refused on the landing -- pipefail and SIGPIPE

**What failed.** `adocs/data/S135_cost.sh run`, started by the coordinator on
the landing `76c4ad8`, refused at once: "refused: HEAD does not carry S135's
columns (no PL_TRAPPED_ROOK)", which it does. **Why**: the script runs under
`set -uo pipefail`, and `git show HEAD:src/evaluation.cpp | grep -q
PL_TRAPPED_ROOK` reads 141 -- `grep -q` exits at its first match (line 188 of
76291 bytes), `git show` dies of SIGPIPE writing the rest, and pipefail
returns that. Reproduced 3 of 3 on `76c4ad8`; on `b7be983`, no match, grep
reads everything and the pipe reads 1. The parent check had the same form, so
a parent carrying the columns would have read 141 and passed, and
`S135_screen.sh` `check_tuner`'s `--help | grep -q` was the same race (1893
bytes of usage; it passed 200 of 200 here, racy by construction). Phase 1b's
dry run could not show it: on `b7be983`'s HEAD only the no-match path runs,
which is the one that works; the pass path needed the landing to exist.

**Fixed, nothing else changed.** `S135_cost.sh` `carries_columns` captures
the file before searching it, a bash pattern match and no pipe;
`S135_screen.sh` `check_tuner` captures the usage the same way. **Every pipe
of S135's scripts audited** (`S135_cost.sh`, `S135_screen.sh`,
`S135_sprt.sh`, `S135_cost.py`, `S135_screen.py`): no other early-exit
consumer has its status read -- the cost script's `diff -r ... | head -20`
prints inside the INV-6 refusal, which returns 1 whatever it reads; every
other pipe ends in `tee`, `cut`, `tail`, `awk`, `grep -v` or `grep .`, which
read their whole input; the sprt script's `grep -q`s read files; the Python
helpers use `subprocess.run` with captured output and one line-reading
`Popen`.

**The cost run's pair made explicit**, since a fix commit on top of the
landing moves HEAD: `CAND_SHA` (default HEAD) and `PARENT_SHA` (default
CAND^, so HEAD^ when neither is set), the checks applied to whichever is
given -- CAND carries the four columns, PARENT does not, PARENT is CAND^ --
and a stage `check` that runs those alone, building and timing nothing. Read
in worktree `chesso-s135c` at `76c4ad8`: the landing pair passes (20 of 20),
the defaults pass; `CAND_SHA=b7be983` refuses (CAND does not carry the
columns), `PARENT_SHA=76c4ad8` refuses (PARENT carries them -- the old
code's false pass), `PARENT_SHA=e3621b8` refuses (not CAND^), an unknown sha
refuses. `check_tuner`, extracted and run alone: the S135 tuner passes 200
of 200, S083's frozen pre-S135 copy refuses 200 of 200. The command is now
`CAND_SHA=76c4ad8 PARENT_SHA=b7be983 nohup adocs/data/S135_cost.sh run >
.tuning/s135_cost.console 2>&1 &`, the times as the rebase section states.

**The screen's and the SPRT's HEAD.** `S135_sprt.sh`'s pair is two pinned
shas with CAND^ = REF checked: no HEAD. `S135_screen.sh` compares the tuner's
emitted constants with HEAD's evaluation sources by value (`start-check`),
which is the reference it means -- the constants that ship. A commit on top
that leaves `src/` alone, as this fix does, leaves that check as it was, and
one that moves them, a landed bisection part, is what it must see: no change.

## 2026-10-25: the cost run's forced builds did not separate

**What run 1 read.** `CAND_SHA=76c4ad8 PARENT_SHA=b7be983
adocs/data/S135_cost.sh run`, 20:21:09 to 21:16:27 (55 min), main checkout at
`d652c0a`, `S135-COST-DONE`; INV-6 bench 3945487 in all five binaries. Its
reading, verbatim, kept as run 1's record:

```
aa: 16 / 16 counter pairs, 16 wall pairs
  bench16 instructions +0.000 %  CI -0.000 .. +0.000 %
  bench16 cycles       -0.072 %  CI -0.333 .. +0.189 %
  pos11   instructions -0.000 %  CI -0.000 .. +0.000 %
  pos11   cycles       +0.083 %  CI -0.110 .. +0.275 %
  pos11   wall         -0.137 %  CI -0.321 .. +0.046 %  (cand time over base, + is slower)
all: 16 / 16 counter pairs, 16 wall pairs
  bench16 instructions +5.016 %  CI +5.016 .. +5.016 %
  bench16 cycles       +4.381 %  CI +4.133 .. +4.629 %
  pos11   instructions +5.255 %  CI +5.255 .. +5.255 %
  pos11   cycles       +4.752 %  CI +4.537 .. +4.967 %
  pos11   wall         +4.402 %  CI +4.165 .. +4.639 %  (cand time over base, + is slower)
new4: 16 / 16 counter pairs, 16 wall pairs
  bench16 instructions +5.016 %  CI +5.016 .. +5.016 %
  bench16 cycles       +4.444 %  CI +4.253 .. +4.635 %
  pos11   instructions +5.255 %  CI +5.255 .. +5.255 %
  pos11   cycles       +4.541 %  CI +4.476 .. +4.606 %
  pos11   wall         +4.587 %  CI +4.231 .. +4.944 %  (cand time over base, + is slower)
old3: 16 / 16 counter pairs, 16 wall pairs
  bench16 instructions +2.215 %  CI +2.215 .. +2.215 %
  bench16 cycles       +1.716 %  CI +1.518 .. +1.913 %
  pos11   instructions +2.409 %  CI +2.409 .. +2.409 %
  pos11   cycles       +2.026 %  CI +1.872 .. +2.180 %
  pos11   wall         +1.948 %  CI +1.771 .. +2.124 %  (cand time over base, + is slower)
parent: 16 / 16 counter pairs, 16 wall pairs
  bench16 instructions -0.000 %  CI -0.000 .. +0.000 %
  bench16 cycles       +0.151 %  CI -0.025 .. +0.328 %
  pos11   instructions -0.000 %  CI -0.000 .. +0.000 %
  pos11   cycles       +0.086 %  CI -0.036 .. +0.207 %
  pos11   wall         +0.170 %  CI +0.039 .. +0.300 %  (cand time over base, + is slower)
A/A floor: bench16 cycles 0.333 %, pos11 cycles 0.275 %, wall 0.321 %
all: COST +4.51 % (cycles and wall beyond the floor, instructions the same way)
new4: COST +4.52 % (cycles and wall beyond the floor, instructions the same way)
old3: COST +1.90 % (cycles and wall beyond the floor, instructions the same way)
parent: zero (inside the A/A's floor)
```

`all` forces a strict superset of `new4`'s columns, yet the two raw
`bench 16` counts agree to 21 instructions in 24.9 billion (pair 6:
24864445994 and 24864446015, old3 24201217688, zero 23676751906 to
23676752001 in those three pairs).

**The diagnosis, from run 1's binaries** (read only; `objdump -d
--no-show-raw-insn -C`, `objcopy --only-section=.text`, g++ 13.3, `-O3
-march=native`, padded). The patch's single barrier took
sum(difference * forced[f]) over all seven columns, a clear bit a forced
weight of 0. g++ SLP-vectorised the first four products: columns 0 to 3 of
each side built into an xmm register (`vmovd`, `vpinsrd`, `vpunpcklqdq`),
`vpsubd`, then one `vpmulld` against a 16-byte constant in `.rodata` at
0x467a0. It does not drop a lane whose constant is zero, so every one of
columns 0 to 3 is computed whatever the mask; columns 4 to 6 stayed scalar,
their weights folded into `lea`/`shl`, and there a zero weight deletes the
column.

- **new4 (0x78) computed all seven.** Its `.text` is all's to the byte
  (sha256 c454a020da23ef95 both); `cmp -l` finds 23 differing bytes in the
  whole file, the 20-byte build id and 3 bytes of the constant at 0x467a0:
  all {3, 5, 7, 9}, new4 {0, 0, 0, 9}. `evaluate_cheap()` 0x7b5 bytes in
  both, `vpmulld 0x2831(%rip)` at 0x43f66, the bishop pair's `setg` at
  0x43c0a and 0x43df8 feeding lane 0.
- **old3 (0x07) computed the three and PL_BAD_BISHOP.** Constant {3, 5, 7,
  0}, `vpmulld 0x2b44(%rip)` at 0x43c53; the bad bishop's colour masks
  0x55aa55aa55aa55aa and 0xaa55aa55aa55aa55 (0x43b32, 0x43b8d) and their
  products feed lane 3 (`vpinsrd` at 0x43c1c and 0x43c45); no
  trapped-minor (0x8181...) or trapped-rook (0xc3c3..., 0x303) mask, so
  columns 4 to 6 were deleted.
- **all (0x7f) computed the seven**, which is what it claims.
- **The count that shows it:** `popcnt` in `evaluate_cheap()`, zero 6,
  parent 6, old3 20, new4 30, all 30. Per column, both sides: bishop pair
  2, rook open 2, rook half open 2, bad bishop 8, bad bishop rammed 4 (it
  shares the bishop-colour counts with the bad bishop), trapped minor 4,
  trapped rook 2 -- so 0x07 owes +6 beyond zero's, 0x78 +18, 0x7f +24.
  Run 1's old3 read +14 (+6 and the bad bishop's 8), new4 +24, all's.

**Which rows stand.** `parent`: unpatched on both sides, it measured what it
claims -- the zero-weight landing is free on g++ 13.3 -- and stands. `all`:
the seven, as claimed; it stands as a reading of the seven under the first
patch, and the re-run's `all` is the one read beside the parts, since a
whole and its parts are compared from one patch. `new4`: a second 16-pair
measurement of all's code (+4.52 against +4.51, which corroborates `all`), not
S135's four; void as a part. `old3`: S027's three plus PL_BAD_BISHOP, not
the three; void as a part.

**The fix** (`adocs/data/S135_cost_only.patch`, the throwaway trees only).
Each forced column is a block of its own under `if constexpr` on its mask
bit: the column's difference times its forced weight (3 to 15 as before)
into an `asm volatile` barrier of its own. A clear column is in no
expression, so no vector can carry it; `static_assert`s hold the mask to 1
to 0x7f and the group to seven columns. Applies to `76c4ad8` (offset one
line from the old hunk); the old patch does not apply over it, so a tree
carrying the old one refuses.

**Proof, per mask** (worktree of `76c4ad8`, the script's configure flags
copied, Release, padded, `-j4`, 0 warnings under `-Werror`):

| mask | `evaluate_cheap()` lines | popcnt | xmm | setg | colour masks | 0x8181 | 0xc3c3/0x303 | weights |
|---|---|---|---|---|---|---|---|---|
| zero | 188 | 6 | 0 | 0 | 0 | 0 | 0 | -- |
| 0x07 | 223 | 12 | 0 | 2 | 0 | 0 | 0 | x3, x5, x7 by `lea` at 0x43b94, 0x43b9b, 0x43ba0 |
| 0x78 | 415 | 24 | 0 | 0 | 4 | 4 | 4 | x9 0x43eac, x11 0x43ebc, x13 0x43ec4, x15 `shl` 0x43ed2 |
| 0x7f | 467 | 30 | 0 | 2 | 4 | 4 | 4 | the seven |

0x07 is the bishop pair (`popcnt 0x10(%r9)`, `popcnt 0x40(%r9)`, two
`setg`), rook open and half open (`andn` and `popcnt` on the rook boards at
0x18 and 0x48) and nothing else; 0x78 has no `setg` and loads the rooks only
for the trapped-rook corner mask; beyond zero's, +6 + +18 = +24, all's.
Built through the script's own `build_one` (its `-j12` set to 4 in a scratch
copy) the three `.text` sections are byte-identical to these: 81f1c99e6f80383b
(0x7f), e808bad6eda027f5 (0x07), 61b7a9ce33740320 (0x78). **INV-6**: each of
the three prints run 1's zero streams exactly -- `bench` 3945487, `bench 12`,
`tools/search_bench.py` at depth 9 (42570, 152468, 27750 nodes) and depth 12
(250176, 412312, 157359), best moves c3d5, e2a6, d7c8q at both.

**The guards.**

- **stamp** (`adocs/data/S135_cost.sh` `build_one`): a binary under
  `$OUT/bin` is reused only when `NAME.stamp` names the commit, mask and
  patch sha256 it would be built from now. Shown on a scratch OUT: run 1's
  binary (no stamp) refused; a stamp naming run 1's patch (3e37da92...)
  refused; the current patch's stamp (0dc56de2...) reused. **And a tree
  already under OUT** (the fast check's finding: a new `CAND_SHA` in an old
  OUT, binaries deleted and trees kept, built the old commit and stamped it
  as the new) is built only when its HEAD is the commit asked for and its
  tracked files are the commit's exactly, or for a patched build the
  commit's with this patch applied exactly: `git diff --quiet` against the
  tree id `patched_tree_id` writes from a scratch index (`read-tree`,
  `apply --cached`, `write-tree`; the tree's own index untouched).
  Anything else is refused, never cleaned. Shown on scratch trees: at
  `b7be983` when `76c4ad8` is asked, refused; at `76c4ad8` carrying run 1's
  patch, refused; this patch plus a stray line in `src/search.cpp`,
  refused; an unpatched build over a dirty tree, refused; this patch
  already applied, built, and a clean tree, patched and built -- both
  `.text` byte-identical to the manual builds (61b7a9ce33740320,
  e808bad6eda027f5). Run 1's `.tuning/s135/cost/bin` carries no stamp and
  its patched trees the old patch, so a re-run there refuses and cannot
  time the old builds: it takes a fresh OUT.
- **code** (`adocs/data/S135_cost.py` `code`, after INV-6 and before any
  timing): zero, all, old3, new4 four distinct `.text` sections; each forced
  build more `popcnt` in `evaluate_cheap()` than zero's; old3's and new4's
  extra summing to all's exactly. The masks are disjoint and no popcount is
  shared between a column of 0x07 and one of 0x78, so a part computing a
  column outside its mask counts it twice. On run 1's binaries: "refused:
  all and new4 have the same .text" and "refused: old3's +14 and new4's +24
  popcnt against zero's sum to +38, not all's +24", exit 1. On the fixed
  three with run 1's zero: "four distinct .text sections; popcnt beyond
  zero's old3 +6 + new4 +18 = all +24", exit 0.
- **read** (`adocs/data/S135_cost.py` `separates`, before any verdict): on
  both workloads each forced build's instructions CI above the A/A's
  instruction floor (its largest CI bound, read in parts per billion), and
  all's lower bound above old3's and new4's upper bounds by more than that
  floor -- each part costs something and all more than either. **It does
  not check that the parts add up to all**: run 1's old3, the bad bishop
  computed beside its three, passes it, and only `code` refuses an overlap.
  The parts' sum against all's is printed, not gated: shared loads and
  register pressure move a correct build's sum off all's by an amount
  nothing here has bounded, so no gate on it can be shown never to refuse
  a correct build. On run 1's pairs (read only): "refused:
  bench16 all's instructions exceed new4's by -1.1 ppb ... not more than the
  A/A floor 1.9 ppb" and the same on pos11, -13.5 against 20.6 ppb; exit 1,
  no verdict printed, so the run ends `S135-COST-FAILED`. On a scratch copy
  of run 1's pairs with old3's and new4's candidate counts rewritten to +1.0 %
  and +4.0 %, each pair keeping its own jitter: "each part above zero and
  all above each part, beyond the floor", the verdicts follow, exit 0. With old3 rewritten to
  zero's counts instead: "refused: bench16 old3's instructions CI starts at
  -1.4 ppb, not above the A/A floor 1.9 ppb" and on pos11, exit 1.

**Not settled, for the bisection.** The shipping sum, difference times
`piece_placement_score[f]` over the seven, is the same loop shape: a
bisection part's binary that ships some columns at zero beside non-zero
ones may still compute the zero ones in a vector lane, as old3 did. The
SPRT measures whatever the binary does, so no verdict is wrong; but that
part's cost would not be the cost run's part, and a part binary's `popcnt`
count (the `code` reading) says which.

**The re-run**, the coordinator's, method unchanged, on an idle machine:

```
CAND_SHA=76c4ad8 PARENT_SHA=b7be983 OUT=/home/max/ws/chesso/.tuning/s135/cost2 \
  nohup adocs/data/S135_cost.sh run > .tuning/s135_cost2.console 2>&1 &
```

About 1 h 05 m by the script's own estimate, ceiling 2 h 30 m; run 1 took
55 min. Disk another 1.2 GB under `cost2`, beside run 1's 1.2 GB under
`.tuning/s135/cost`, whose five worktrees stay until the coordinator removes
them. The run now prints the `code` lines into `cost.log` before the pairs
and the `masks` lines before the verdicts.

## 2026-10-25: the candidate's paste -- stopped at the gradient check

Run by one Opus 5.5 subagent in worktree `chesso-s135e`, branch `s135e` at
`62aa1b9` (bench 3945487). Outputs under `.tuning/s135/apply_candidate/`
(the script's own) and `.tuning/s135_apply_diag/` (the diagnosis below).
**Stopped before the goldens**: a red test that is not a golden, so a
fixture question for the coordinator (the brief's stop rule). Nothing past
the paste was edited.

### The screen and the candidate, as read

The screen's reading (`.tuning/s135/screen/reading.txt`): kept bishop_pair
+0.000044, rook_open +0.000023, rook_half_open +0.000004 (m 0.000004),
trapped +0.000036; dropped bad_bishop (best report the start), frozen at
zero. The candidate fit (`.tuning/s135/candidate/fit.hpp`, sha256
`19dde95f...`, 10 free): `best: train 0.069222  validation 0.071725  (start
0.069321 / 0.071831)`; mg {-5, 23, 3, 0, 0, -40, -23}, eg {37, -4, 13, 0, 0,
-68, -16}; stage-one bound mg 13292, eg 12929 of 32767. Its log stamps
`engine d652c0a-dirty`, the run `62aa1b9`: the two have identical `src/` and
`tools/`, so both name the same engine and tuner.

### The apply

`adocs/data/S135_screen.sh apply .tuning/s135/candidate/fit.hpp
/home/max/ws/chesso-s135e`, 5 m 36 s, `S135-APPLY-DONE`: start-check passed
(frozen values HEAD's, free columns [0, 1, 2, 5, 6]), headroom mg 19475, eg
19838; `verify_fit` 831 of 831; only `piece_placement_mg`/`_eg` moved, the
columns outside the free set unchanged; both builds 0 warnings; **bench
5184358** (3945487 before); format clean. Fast suite **46 of 50 in both
builds**, the same four: `test_evaluation` (the six piece anchors),
`test_search` (`QUIET_ROOK_EVAL`, `_CHEAP`, 209, -534, -599, twelve cases),
`test_mate_carry` (D's ceiling 0 read 1; 2 of 6 cases report a line, 4
needed: the budgets), and **`test_tuner_gradient` "gradient() is the
derivative of the error it descends"**: `CHECK( check.worst < 1e-6 )`, worst
**2.91446e-06 at parameter 774** (`mobility_mg`, the bishop's), the parent
3.99e-7 there. `test_mate_breadth` (`EXACT_FLOOR` 158) and `test_engine`
(`MATE_IN_THREE_FLOOR` 11, `first_exact` 6, `first_mate_depth`) green as
shipped; their scripts were not run.

### The red, diagnosed (not fixed)

A scratch copy of the case (`.tuning/s135_apply_diag/gradient_diag.cpp`,
never committed) at the candidate's constants: parameter 774's analytic
gradient is **-1.342036e-06** against its neighbours' 2.54e-4 (773) and
3.66e-4 (775), the largest 4.77e-4 -- the fixture's error is all but
stationary in that weight -- and the finite difference at h = 0.01 is
-1.342040e-06, **4e-12 apart**. The relative error follows the O(h^2) law
exactly: h 0.05, 0.02, 0.01, 0.005, 0.002 read 7.28e-5, 1.17e-5, 2.91e-6,
7.26e-7, 9.6e-8 at 774. So the gradient is right and the check's relative
measure is ill-conditioned on a near-zero gradient; making it green is a
fixture or metric change, which is not this subagent's (never loosen a red
test).

The other weight-dependent preconditions hold (`S083_tuner_preconditions.cpp
both`, `.tuning/s135_apply_diag/tuner_pre_cand.txt`): "sits clear of the
clamp" green; the clamp rule still picks **+100**, 8 rows bound, every row
**23.00** from the kink, fd 6.0e-8; the walk's smallest **0.4015** (Adam) and
**0.0672** (AdaGrad), both the rook's, above 0.01, holds at most 1.07e-14,
runs at most 5.68e-13 apart, travelled about 1600.

### Item 3, read only: the zero column is computed

The candidate's `evaluate_cheap()` (`objdump -d --no-show-raw-insn -C`,
`build/src/chesso`, `.text` 2cf97d7cb84fd92e): **popcnt 26** against the
zero build's 6 (`.tuning/s135/cost2/bin/zero`); the five non-zero columns
owe +12 (pair 2, open 2, half open 2, trapped minor 4, trapped rook 2), the
other +8 is PL_BAD_BISHOP's. The bad bishop's colour masks
0xaa55aa55aa55aa55 and 0x55aa55aa55aa55aa appear (four `movabs`), and one
`vpmulld 0x2905(%rip)` reads the constant at 0x467a0, **{2424827, -262121,
851971, 0}** -- columns 0 to 3 packed, lane 3 the bad bishop at zero. The
rammed column (4) is deleted. So the shipping sum computes a zero-weight
column in a vector lane, as the cost run's old3 did: the form change owed
before the SPRT (a zero-weighted column in no expression, INV-6 against
this paste) is not made.

### Not done, the step having stopped

The form change; every golden (anchors, `QUIET_ROOK_EVAL`s and the four
search scores, the S170 ceilings and budgets, both mate floors' scripts,
first-mate, the mined rows, truncation positions, node budget, ply-2);
gates; `S135_sprt.sh`'s re-read; the cost reading's commit; docs.

## 2026-10-26: the candidate, its paste and goldens

The same subagent, resumed on the coordinator's ruling after the stop above
(the gradient case to the owner, everything else to go on), then on the
owner's ruling for that case (DEC-316, the coordinator's to record). Worktree
`chesso-s135e`, branch `s135e` at `62aa1b9`; the run's outputs under
`.tuning/s135e/` and `.tuning/s135_apply_diag/`, the readings in
`adocs/data/S135_goldens.txt` with each command beside its output. The order:
the form change and its INV-6, the goldens, then the gradient rows, then the
gates. No match, SPRT, fit, datagen or timing ran.

### The screen and the candidate, as read

As the stop's section quotes them: the screen kept bishop_pair (+0.000044),
rook_open (+0.000023), rook_half_open (+0.000004, m 0.000004) and trapped
(+0.000036) and dropped bad_bishop (its best report the start, train 0.069321
-> 0.069309, validation 0.071831 -> 0.071839), `SCREEN-OUTCOME candidate
bishop_pair,rook_open,rook_half_open,trapped`. The candidate fit:
`best: train 0.069222  validation 0.071725  (start 0.069321 / 0.071831)`,
10 free, mg {-5, 23, 3, 0, 0, -40, -23}, eg {37, -4, 13, 0, 0, -68, -16},
stage-one bound mg 13292 and eg 12929 of 32767. HEAD moved from `d652c0a` to
`62aa1b9` during the screen and the fit's log stamps `d652c0a-dirty`; the two
have identical `src/` and `tools/`, so every stage names the same engine and
tuner.

### The apply

`S135-APPLY-DONE` in 5 m 36 s: start-check passed (free columns [0, 1, 2, 5,
6]), headroom mg 19475 and eg 19838, `verify_fit` 831 of 831, only
`piece_placement_mg`/`_eg` moved, both builds 0 warnings, bench 3945487 ->
**5184358**, 46 of 50 in both builds (the anchors, the S170 ceilings and
budgets, and the gradient case, below).

### Item 3: the zero column, and the form that removes it

At the plain paste g++ 13.3 computed the bad bishop's count at zero weight:
`evaluate_cheap()` popcnt 26 against the zero build's 6, the colour masks
present, one `vpmulld` on the packed constant {2424827, -262121, 851971, 0},
lane 3 the bad bishop (the stop's section has the detail). **Changed**:
`src/evaluation.cpp` `add_placement_column`, one column a template instance
with its weight tested by `if constexpr`, so a zero-weighted column is in no
expression; `add_placement_columns` folds the seven in index order, the trace
still recording every column. **INV-6 against the plain paste**: bench
5184358 both, `bench 12` 2024016 both and 0 lines apart, `search_bench.py` at
9 (48284, 126091, 62646; c3d5, d5e6, d7c8q) and at 12 (226320, 430872,
157130; c3d5, e2a6, d7c8q) identical, the tune build's bench 5184358 both.
**Then the disassembly**: popcnt **18** (the five non-zero columns' +12), no
colour mask, no `vpmulld`, no xmm register, the trapped masks and two `setg`
present. So the candidate computes S027's three and the two trapped counts
and nothing else.

### Every golden, each by its script

Fixture A `5ed5e48` (a commit object on no branch: `62aa1b9` plus the
worktree's src/, the parent's tests) for S154 and S156; frozen copies of the
candidate's builds for the rest; the parent the main checkout's builds at
`62aa1b9`. `.tuning/s135e/chain.sh`, 450 s, `CHAIN-DONE`.

| golden | before | now | script |
|---|---|---|---|
| six piece anchors | 99, 276, 355, 585, 984, 0 | **99, 276, 355, 583, 984, 0** | `S192_anchors.py` 10 of 10 on the shipped tree, then on the fit's header; its embedded values set to these, 10 of 10 on this tree |
| `QUIET_ROOK_EVAL`, `_CHEAP`; 396; 209; -534; -599 | 585, 580; 396; 209; -534; -599 | **583, 578; 394; 199; -533; -597**, the four leaves still the four king moves standing pat (-564, -533, -582, -543) | the same; every move through rook_open alone (mg 23, eg -4) |
| `POSITIONAL_ROOM` | -- | did not fire; `test_evaluation` 33 of 33 | -- |
| `truncation_positions` | S083's four, phases 5, 11, 17, 23 | **re-picked**: phases 5, 11, 17, 19; S083's phase 11 row kept, its other three under 2.8 | `truncation_scan --min 2.8` on the corpus (65 past 2.8, 13 at 2.875), `S083_truncation_pick.py` |
| "ordering keeps the tree small" | 65024 / 3251 (7783 at S083) | **unmoved**: 7366, 0.45 of 16256, inside [7270, 32512] by 96 | `S192_node_budget.py` |
| `first_mate_depth`, `mate_in` | 9, 9; 5 | **unmoved**: 9, 9; 5 | `S188_repair_goldens.py first-mate` |
| `DRIVE_DEPTH` | 4 | **unmoved**: 4 | `S188_repair_goldens.py s207 --lib` |
| ply-2 `first_exact` | 6 | **unmoved**: 6 (candidate and parent) | `S083s_rfp_ply2.py` |
| `capture_mates`, ProbCut, extra-ply rows | each at its depth | **unmoved**: each at its depth; ProbCut 5 at d9-d11, 6 at d12; the retired multicut row still no mate at 10 | `S230_mine_r01_row.py depths`, candidate and parent libraries |
| late-capture position, 3 and 7 | Rxa4 at 3, Rxe4 at 7 | **unmoved**: the same cells, node counts apart | `S261_mine_late_capture.py sweep` and `pick` on the position |
| `EXACT_FLOOR` 158 | 159 / 154 | **held**: **160** shipping, 156 at 2, **156** weakened; the gate at RfpMinPly 1 red at 156 | `S156_mined_floor_sweep.py --ref 5ed5e48 --floor 158` |
| `MATE_IN_THREE_FLOOR` 11 | 12 / 11 | **held, separating again**: **13** shipping, **8** weakened; `red` red at 8 >= 11 and on mate-in-two | `S154_floor_margin_sweep.py floor` and `red` |
| `short_line_ceiling` | 2, 6, 4, 0, 1, 3 | **2, 6, 4, 1, 2, 3**: D 0 -> 1, E 1 -> 2, the grid alone 0, 0, 1, 1, 2, 0 | `S203_case_sweep.sh --ceilings` over the six grids and `adocs/data/S135_sweep_s170.txt` (108 cells, 1408 lines, 5 short) |
| S170 budgets | A 1000000, B 100000, C 1000000, D 4000000, E 500000, F 100000 | **A 300000, B 100000, C 500000, D 4000000, E 300000, F 300000** | `adocs/data/S170_pick_budgets.sh` (the tracked picker) |
| clamp push; walk floor | +100; 0.01 | **unmoved**: +100, every row 23.00 from the kink; walk 0.4015 / 0.0672 (before the rows, below) | `S083_tuner_preconditions.cpp both` |
| `golden_defaults`, option count, census, book | -- | **unmoved by construction**: no search parameter, UCI option, generator, corpus or book moved | -- |

**DEC-299's guard is met and applied**: both mate floors hold or rise and no
ceiling rises by more than 2. Neither floor is moved: 158 and 11 are the
owner's (DEC-313), both still separate, and a floor that holds needs no
decision; 159 for `EXACT_FLOOR` (1 below the shipping count, the rule until
S293) is the coordinator's to propose if wanted. Where S083's landing list
and `DEV_MANUAL.md`'s goldens table differ, the table won: it names the
tracked picker (`adocs/data/S170_pick_budgets.sh`, S290) where S083's has
`.tuning/coord/S099/p2/pick_budgets.sh`, and adds the ply-2 clause,
`DRIVE_DEPTH`, the census, the book and the option count; for the mined rows
both name full re-mines, and as at S083 the rows were read at their depths
by `S230_mine_r01_row.py depths` (each holds, so no re-mine is owed), the
late-capture position by its script's `sweep` and `pick` (S023's ruling:
re-mined if red).

### The gradient case: red, the owner's ruling, the rows

**Red** at the candidate with the forty rows: worst 2.91446e-06 at 774
(`mobility_mg[1]`, the bishop's) against the case's 1e-6. **Why**: the
fixture's error is all but stationary there, the analytic gradient
-1.342036e-06 against 6e-5 to 5e-4 for the other seven mobility weights,
the central difference -1.342040e-06, 4e-12 apart; the relative error obeys
O(h^2) exactly (h 0.05, 0.02, 0.01, 0.005, 0.002: 7.3e-5, 1.2e-5, 2.9e-6,
7.3e-7, 9.6e-8), so it is truncation over almost nothing, not a wrong
gradient. **The owner's ruling, 2026-10-26 (DEC-316)**: fixture rows, S100's
precedent; metric, h = 0.01 and 1e-6 unchanged. **The rows, by a stated
rule** (`adocs/data/S135_gradient_rows.cpp`, the rule in its header, written
before it ran): the first position of `all_test_fens()` in order whose pair
(it, then its colour-mirror) loads, sits clear of the clamp and puts
|dE/d mobility_mg[1]| at or above a tenth of the smallest of the other seven
mobility gradients. Candidate 47, castling.json's
`1r1Nk2r/ppp1n1pp/8/2b5/8/2P5/PP2BPPP/R3K2R w KQk - 2 9` and its mirror,
|g| 1.87e-5 against a threshold of 5.70e-6; python-chess Status.VALID for
both. Appended to `tests/test_eval_positions.hpp` with its provenance.
**Green**: worst **1.9989e-07, still at 774**; at the parent's constants
(the same object against `62aa1b9`'s trace library) **5.0061e-07** at 774,
green. **Re-read with the rows**: clamp-clear green (largest |stage two|
99.3 against 183); the clamp rule **+100**, the same 8 rows, 23.00 from the
kink, fd 1.14e-7 (parent 2.25e-8); the walk **0.2263** (Adam) and **0.0135**
(AdaGrad), the rook's, over the 0.01 floor by a factor of 1.35 where it was
0.0672 without the rows (parent 0.1889 and 0.0184), holds at most 5.22e-15;
the planted recovery, all fourteen placement weights within 1.65e-3 (case
0.5), passed pawns within 0.068; the nonlinear group's case green at 1.10e-7.
No golden in the file counts the fixture's rows (every count reads
`positions.size()`); `test_eval_model` 14 of 14 and `test_eval_trace` 9 of 9
over the longer list.

### Gates

After the last code and test edit (`.tuning/s135e/gate.sh g2`,
`CLANG_FORMAT_MAJOR=22`, `-j12`): both builds **0 warnings**; the fast suite
**50 of 50 in both builds**; `./clang-format.sh --check` clean; bench
**5184358** in both builds. Before the rows (`g1`) the same gate read 49 of
50 in both, the gradient case the only red. `tools/plan_prose_check.py`
`--citations`, `--touches`, `--params`, `--gate`, one mode a call: nothing
flagged (the two `MISSING` notes are S136's). **Tier 2 (DEC-141) does not
bind**: `git diff --cached --stat -- src/` names `src/evaluation.cpp` alone,
the weights and the placement sum's form -- no search, generator,
`make_move` or `unmake_move` code.

### The SPRT script

`adocs/data/S135_sprt.sh` re-read against this candidate and left with the
coordinator's markers unfilled (it refuses on them). Its guards stand as
written: CAND^ is REF, `src/eval_tables.hpp` unchanged, `src/evaluation.cpp`
the only file under src/ that differs -- the form change is in that file, so
no guard's wording moved. A dated paragraph is added to its header: the four
survivors and bad bishop dropped (held at zero and, with the form, computed
nowhere); the bisection as `read` printed it, `family -> [A -> [A1, A2 ->
[A2a, A2b]], B]`, B being trapped alone, so six further SPRTs at worst,
about 119 h, not eight; the cost the verdict pays bounded by two measured
rows; and S294, the open DEC-171 filler, for the coordinator's sentence.

**A risk for the bisection.** DEC-316's two rows cut the AdaGrad gauge
walk's margin to 0.0135 against the case's 0.01 floor (0.0672 without them),
so a bisection part's refit, which moves the constants the walk reads, may
read it red; that would be a precondition for the owner to rule on, as
DEC-313 (5) was, never a lower floor.

### The cost reading

Committed as `adocs/data/S135_cost_reading.txt`: run 2's reading verbatim
under two lines naming the run and run 1's void parts (one barrier over the
forced sum let g++ 13.3 vectorise columns 0 to 3, so run 1's `new4` computed
all seven and `old3` its three plus PL_BAD_BISHOP; its `all` and `parent`
stood). Run 2:

```
all: COST +3.92 % (cycles and wall beyond the floor, instructions the same way)
new4: COST +3.30 % (cycles and wall beyond the floor, instructions the same way)
old3: COST +0.92 % (cycles and wall beyond the floor, instructions the same way)
parent: zero (inside the A/A's floor)
```

The SPRT pays no measured row: the candidate computes S027's three and the
two trapped counts (popcnt 18), a superset of `old3`'s columns (12) and a
subset of `all`'s (30), so its cost lies between +0.92 % and +3.92 %, `all`
bounding it from above. Without the form change it would also have computed
the bad bishop's count (popcnt 26).

### Documents

- `DEV_MANUAL.md`: the goldens table's rows for every golden above (the
  values since S135 and the readings of the unmoved ones), the ceilings'
  command with S135's grid, the gradient paragraph (the red, DEC-316's pair,
  2.00e-7), the gauge walk's 0.0135. The 58 term parameters, the 831 and
  the partition's widths do not move.
- `MANUAL.md`: the `RfpMinPly` row's mined-set figures at this tree's 160,
  156 and 156 against the floor 158. Nothing else on the UCI surface moves.
- `adocs/data/README.md`: rows for `S135_cost_reading.txt`,
  `S135_goldens.txt`, `S135_sweep_s170.txt` and `S135_gradient_rows.cpp`.
- Every changed golden's site names its script.

### Proposed for `adocs/specs.md` (the coordinator writes it)

- **Evaluation row**, after the passed pawn sentence: "Piece placement since
  S135 is five of its seven counts at fitted weights -- the bishop pair, a
  rook on an open file, a rook on a half-open file, and DEC-258's trapped
  pieces (minors on CPW's listed patterns, rooks in the own king's corner) --
  fitted together on S083's corpus with every other constant frozen
  (DEC-259, DEC-315) and computed in the cheap stage beside the pawn terms.
  The bad bishop count and its rammed share stay at zero, dropped by the
  screen; a zero-weighted placement column is in no expression, so neither
  is computed."
- **"absent, evaluation" row**, its first two sentences replaced: "S135's
  bad bishop and its rammed share, and tempo -- present in the code, shipped
  at zero weight. At zero they change no node and cost nothing: S135's cost
  run's `parent` comparison read the zero-weight landing inside the A/A
  floor on g++ 13.3, and since S135's form a zero-weighted placement column
  is in no expression whatever the columns beside it weigh; the tuner
  carries their 6 parameters, making 831 fitted and 825 shipped." The rest of
  the row stands; its "history of the five" now has three of them shipping.

### Proposed commit message

```
Paste S135's piece placement fit and re-derive its goldens (S135)

The screen kept four of the five terms (bishop pair, rook on an open and
on a half-open file, trapped pieces) and dropped bad bishop; their joint
fit, every other constant frozen, is pasted here for the one SPRT that
decides it (DEC-259, DEC-315). Bad bishop ships at zero, and the
placement sum is now one column at a time under if constexpr, so a
zero-weighted column is in no expression: at the plain paste g++ 13.3
computed the bad bishop's count in a vpmulld lane at every node. The
form is behaviour-neutral (INV-6: bench, bench 12 and search_bench at 9
and 12 identical with and without it); evaluate_cheap()'s popcnt 26 -> 18.

Every golden the paste moves is re-derived by its script (DEC-142):
the anchors 583/578/394/199/-533/-597, the four truncation positions
re-picked, the S170 ceilings D 0 -> 1 and E 1 -> 2 and four budgets by
the tracked picker inside DEC-299's guard, both mate floors holding
(EXACT_FLOOR 158 against 160 and 156, MATE_IN_THREE_FLOOR 11 against 13
and 8). The gradient check read 2.91e-6 at mobility_mg[1], whose
gradient the fit left near zero; the owner ruled for fixture rows
(DEC-316), one pair picked by adocs/data/S135_gradient_rows.cpp's rule,
and it reads 2.00e-7. Cost reading committed: all +3.92 %, old3 +0.92 %.

Bench: 5184358
```

### The fast check's two fixes (2026-10-26)

- `src/evaluation.cpp`'s comments beside the placement weights said every
  fit since S065 held them at zero and that the term costs nothing to leave;
  now they say S065 to S083 held them, S027's paragraph is in the past
  tense, and a paragraph over the declaration says the weights are S135's
  screen and joint fit (DEC-259, DEC-315), bad bishop and its rammed share at
  zero because the screen dropped them. Comments only: bench 5184358 in both
  builds.
- **Owed at the SPRT's closing commit, on H1 only** (on H0 the revert makes
  the old text true again): `src/evaluation.hpp`'s comment over
  `piece_placement_mg` is false at CAND, but `S135_sprt.sh`'s guard allows
  only `src/evaluation.cpp` under src/, so it is not edited here. Replace

  ```
  // **Zero in evaluation.cpp, and not because the tuner has not reached them.**
  // S027 fitted piece placement (mg {2, 32, 8, -26} / eg {55, -8, 12, 11}),
  // its bundled SPRT read -5.48 +/- 11.46 and H0, and the weights were then
  // zeroed by hand and held there by `--freeze piece_placement` in every fit
  // since S065 (DEC-057) -- held at zero, not fitted to it; evaluation.cpp says
  // so beside the weights and adocs/plan_done/S100_*.md carries the ledger. The
  // passed pawn, pawn structure and king safety weights above and below ship
  // fitted since S027; tempo is zero for its own reason, stated at its block.
  ```

  with

  ```
  // **Fitted since S135, five of the seven.** S027 fitted piece placement (mg
  // {2, 32, 8, -26} / eg {55, -8, 12, 11}), its bundled SPRT read -5.48 +/-
  // 11.46 and H0, and the weights were zeroed by hand and held there by
  // `--freeze piece_placement` in every fit from S065 to S083 (DEC-057). S135's
  // screen kept the bishop pair, both rook files and the two trapped counts, and
  // their joint fit ships on its SPRT's H1 (DEC-259, DEC-315); the bad bishop
  // and its rammed share stay at zero, dropped by the screen. evaluation.cpp
  // says so beside the weights, and adocs/plan_done/S100_*.md carries the
  // ledger. The passed pawn, pawn structure and king safety weights above and
  // below ship fitted since S027; tempo is zero for its own reason, stated at
  // its block.
  ```

  A comment only, so the closing commit's bench stays the candidate's.
