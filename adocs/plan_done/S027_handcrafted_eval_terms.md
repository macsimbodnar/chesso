id:         S027
goal:       king safety, passed pawns, pawn structure, bishop pair, tempo
accepts:    one term at a time, never bundled. For each: a fast SPRT
            (`./fastchess.sh --fast`), the full-bounds run when that is
            inconclusive, and the term's weights fitted by the tuner rather than
            guessed before any verdict is believed. A term that measures zero is
            recorded as zero and the record says so
touches:    src/eval_tables.hpp, src/evaluation.cpp, tools/eval_model.hpp, tools/tuner.cpp, tests/
excludes:   building the tuner, which was S028; the lazy shortcut and mobility, which were S034
decisions:  DEC-033, DEC-037, DEC-039, DEC-040
closes:
blocks:
paused_by:
done:       2026-08-13. Five terms, each fitted with every other constant frozen and each decided by its own SPRT. Three passed and two measured zero. +20.87, +17.34, +13.05, 0, 0.

## What this step is now

It absorbed S019, which is retired. S019 was "evaluation terms for the phase the
error analysis says costs most" and the phase turned out to be the early
middlegame (DEC-032), which is this list rather than a separate endgame step.

It also runs *after* S028 rather than before it, which changes how a term is
added. The tuner exists by then, so a new term arrives with a weight that was
fitted rather than guessed, and its SPRT measures the term instead of measuring
a guess about the term. DEC-033.

Aim the list at the middlegame. DEC-033: 95 of 160 expensive moves were
unchanged at sixteen times the search, so what these terms have to fix is what
the engine believes, not what it can see.

## What S034 changed about this step

**Mobility is done and is not in this list any more.** It shipped with S034 at
+28.46 Elo, and getting there cost three measurements because two causes were
bundled in the first one. Read DEC-037, DEC-039 and DEC-040 before adding a term
here; the short version is below.

**"Every term must be accumulated" was wrong, and this file used to say it.**
Material and the piece-square tables are sums over pieces of a function of one
piece and one square, which is why `make_move` can apply an O(1) delta. A term
that depends on where the *other* pieces stand cannot be accumulated at all.
What replaces the rule:

- **Cheap or accumulable terms go straight into `evaluate_cheap()`.** Bishop
  pair, tempo and pawn structure behind a pawn hash cost near nothing.
- **Occupancy-based terms go behind the lazy shortcut**, as stage two with
  mobility. King safety is the one left in this list.
- **A term behind the shortcut must fit inside `LAZY_EVAL_MARGIN`**, currently
  150, or move it deliberately. `test_evaluation` "the lazy shortcut cannot
  change a decision" fails otherwise, and it has already caught one violation.
- **Price it in a real search, not in `bench_eval`.** That benchmark understates
  an occupancy term by 3.9 times because its ten-position loop keeps the magic
  tables hot while a real search walks 2 MB of them at random.

## Order, and what already exists to build on

1. **King safety** -- pawn shield, attacker count and weight on the king zone,
   open files near the king. Occupancy-based, so stage two and inside the
   margin.
2. **Passed pawns** -- tapered by rank. `passed_w_pawns_masks[]` and
   `passed_b_pawns_masks[]` exist in `bb_tables.hpp`, unused.
3. **Pawn structure** -- isolated, doubled, backward. `isolated_file_masks[]`
   exists, unused. Cache in a pawn hash table; pawn structure changes rarely.
4. **Bishop pair**, **rook on open and half-open file**, **rook on seventh**.
5. **Tempo** -- a small bonus for the side to move.

That is the order engines generally report value in, and **it is not settled.**
The argument for inverting it and taking the cheap terms first -- tempo, bishop
pair, then the pawn terms -- is that each costs near nothing, so a verdict
measures the term rather than the term minus a speed penalty, and the record of
what a term is worth here gets built before anything expensive is paid for.
Mobility is the evidence: on hand-picked weights and full price it measured
-14.93 and looked like a failure. The owner chooses when the step starts.

Expect single or low double digit Elo each and expect some to measure zero.

## Every term is fitted before it is judged

The tuner fits 781 parameters today and adding a term means adding its weights
to that vector: `tools/eval_model.hpp` for the model and the feature, and
`tools/tuner.cpp` for the dataset, the gradient and the writer. **All four
places.** S034 found two silent failures there -- the writer discarded the new
weights and the gradient never moved them -- and neither would have failed a
test. A fit that returns a weight exactly as it was handed in is the symptom.

`test_eval_model` is what keeps the model honest and it will fail the moment the
engine gains a term the model does not have. That failure is the reminder, not a
nuisance.

## Standing caveat

Most of this is thrown away when S029 lands. It is done anyway because NNUE
training data comes from self-play by an engine that already plays reasonably,
and because PeSTO reaches about 3125 on CCRL Blitz on piece-square tables and
search alone. The floor is what matters, not the ceiling.

## Results, one row per term

### 1. King safety -- **+20.87 +/- 15.41 Elo**, H1 accepted

1300 games, LOS 99.61 %, 10+0.2, `72c00d4` against `ad3b17d`. Three commits:

| commit | what | verdict |
|---|---|---|
| `690de1f` | the term, nine linear features, weights at zero | behaviour-neutral, INV-6 discharged |
| `ad3b17d` | the `evaluate_lazy` bound fix found under it | **0**, -6.52 +/- 11.69 over 2024 games, H0 accepted, kept |
| `72c00d4` | the fitted weights | **+20.87 +/- 15.41**, H1 accepted |

**Linear, and that was forced.** The `attack_table[weighted_attacker_count]`
curve the literature describes is not a linear function of the weights that
build its index, so the S028 tuner cannot fit it at all. Nine counts per side
instead: attackers by piece type, zone attack incidences, near and far pawn
shield, open and half-open files by the king. DEC-044. A verdict of +20.87 is a
verdict on the linear form, and the non-linear one is untested rather than
rejected.

**The staging discipline from S034 held and paid.** The term shipped at zero
weights first, which made that commit provably behaviour-neutral -- identical
node counts at depth 9 and identical best moves -- so the SPRT that followed
measured the fitted term and nothing else. `tuner --only king_safety` was built
for the same reason: a joint fit also refits the 781 constants that were already
fitted, and the match would then have measured two changes. Frozen parameters
come out bit-identical, checked over 781 values.

**Fusing the piece loop was worth more than it looks.** Computed as its own pass
the term cost 7.7 % at zero weights, because it recomputed slider attacks
mobility had already computed for the same piece. Fused, 0.35 %, inside the
noise floor. Without that, 20.87 Elo of evaluation would have been measured
through a 7.7 % speed loss.

**What it cost to get there:** a live crash on a kingless position, a 21 %
regression from clang declining to inline a function that gained a second
caller, a corpus so vacuous that two of the nine features were zero in every
position of it, and a defect in `evaluate_lazy` that predates the term.

### 2. Passed pawns -- **+17.34 +/- 13.51 Elo**, H1 accepted

1584 games, 10+0.2, `ce7cd35` against `6950be1`. Two commits: the term at zero
weights, then the fitted weights.

**Held-out error said it would be smaller than it was.** The frozen fit moved
validation 0.107106 to 0.106900, an improvement of 0.000206 against king
safety's 0.000304 which measured +20.87. On that ratio this term looked like
roughly 14 Elo gross before its 4.5 % speed cost came out, and it measured 17.34
net. Error improvement orders candidates; it does not predict Elo.

**The buckets are a residual and not a valuation.** mg {3, -16, 1, 10, 30, 2},
eg {-15, -4, 18, 28, 25, 8}, not monotonic in the rank, and bucket 5 -- a pawn
one square from promotion -- fits to almost nothing because `psqt_mg`'s pawn
table already pays 79 to 214 for that square. Third time this project has met
the effect: knight mobility fitted to nothing at S034, king safety's attacker
counts at DEC-044.

**Two negative results came out of this term and both are recorded.** DEC-046,
the pawn hash, built in full and discarded because it recovers nothing.
DEC-047, the one that matters: a term shipped at zero weights is deleted by the
compiler, not measured, so every "this term is free" figure taken at those
weights describes a build without the term in it.

### 3. Pawn structure -- **+13.05 +/- 11.15 Elo**, H1 accepted

2290 games, 10+0.2, `b6531b7` against `cd273d2`. Isolated, doubled and backward,
three counts per side, sharing its bitboard fills with passed pawns -- four
fills where two independent passes would need six. Marginal cost 4.1 % of a
depth 14 search.

**Held-out error understated it for the third time in a row.**

| term | error improvement | measured Elo |
|---|---|---|
| king safety | 0.000304 | +20.87 +/- 15.41 |
| passed pawns | 0.000206 | +17.34 +/- 13.51 |
| pawn structure | 0.000126 | +13.05 +/- 11.15 |

The ordering is right and the scale is not: each term measured far more Elo than
its share of the error would suggest, and every one of the three cleared a
4 to 5 % speed cost on top. Error improvement ranks candidates. It does not
predict Elo and it should not be used to decide against running a term.

**A definition that no corpus check could catch.** Weakening the model's
backward test so that a neighbour abreast no longer stops it leaves all 22
corpus positions green and is caught only by hand-built cases. Corpus sweeps and
hand-built cases are not substitutes for each other.

**The group-boundary bug appeared a second time.** `--only passed_pawns` ran to
`PARAM_COUNT` and would have swallowed all six new weights, exactly as
`--only king_safety` did to the twelve passed pawn weights one commit earlier.
Neither fails a test; the symptom is a fit returning the new weights exactly as
it was handed them. `DEV_MANUAL.md` now states the rule and the symptom.

### 4. Bishop pair and rook placement -- **0**, -5.48 +/- 11.46 Elo, H0 accepted

2284 games, `b05cc62` against `7375546`. The weights are reverted to zero and
the term is inert; the code and the tuner plumbing stay, because at zero the
compiler deletes the term -- `evaluate_cheap()` is the same 164 instructions
either way -- so it costs nothing to keep and it is what a split would need.

**This is the term that broke the pattern and it is the most useful result of
the step.**

| term | error improvement | cost | measured Elo |
|---|---|---|---|
| king safety | 0.000304 | -- | +20.87 +/- 15.41 |
| passed pawns | 0.000206 | 4.5 % | +17.34 +/- 13.51 |
| pawn structure | 0.000126 | 4.1 % | +13.05 +/- 11.15 |
| piece placement | 0.000161 | 3.1-4.0 % | **-5.48 +/- 11.46** |

Three terms in a row had held-out error understating the Elo, which was starting
to look like a rule. This one had it overstating by enough to change the sign,
on the *second largest* error improvement of the four. Held-out error is not a
weak predictor of strength, it is an unreliable one in both directions. It ranks
what to try. The match decides, and nothing else does.

**Two candidates for why, neither tested.** The bishop pair is the only one of
the four features that is free -- clang rewrites `count_bits(x) >= 2` into
`x & (x - 1)` -- and it carries the largest weight the step fitted, eg +55. The
three rook features cost six popcounts between them and their weights are small
and mutually cancelling. And the corpus is self-play by an engine predating all
of S027, so a residual fitted on it encodes what correlated with winning in
*those* games; for a feature like a rook on the seventh that may be a
consequence of already being better rather than a cause of becoming so.
Minimising squared error on that corpus and playing better are different
objectives and this is the first term where they came apart.

**Candidate follow-up, not scheduled:** split the term and measure the bishop
pair alone. It is free and it is where the weight is. That needs its own
`--only` group and its own fit, and it is a new step rather than a re-run.

### 5. Tempo -- **0**, -0.69 +/- 9.64 Elo, unresolved

3000 games, `201c15e` against `496b942`. Fitted mg 10, eg 0, held-out error
0.106766 to 0.106723 -- the smallest improvement of the five terms by a factor
of three. Weights reverted to zero, code kept inert.

**The run reached neither bound.** It exhausted the 3000-game limit at LLR -1.46
against a -2.20 boundary. That is unresolved, not H0 accepted, and the record
says so: what it establishes is that the term is smaller than this instrument
can see over 3000 games at 10+0.2, not that it is harmful. The interval is the
tightest of the step and centred on zero. S013's LMR run is the precedent for
being careful here -- it was killed at 96 % of its bound and "passed" was never
written into the record.

Revisiting it needs tighter bounds than `--fast`, and 3000 games already costs
three hours. It is a thing to measure on a faster machine, not a thing to argue
about.

**The term nearly went somewhere invisible.** The obvious home is `evaluate()`
and the search never calls it: quiescence goes through `evaluate_lazy()`, which
is built on `evaluate_cheap()`. A tempo bonus in `evaluate()` would have been
correct-looking, fully tested, and applied at none of the nodes the search
evaluates.

**Three tests were re-targeted and one of them could only be found by forcing
the weights.** "The start position is balanced" asserted `== 0`, which stops
being the property the moment the engine believes having the move is worth
something: a symmetric position is not symmetric in whose turn it is. The third,
`white_to_move == -black_to_move`, reads as obviously true and printed
`REQUIRE_EQ( 1118, 1084 )` the moment the weights were non-zero.

### 5. Tempo -- last

A single bonus for the side to move. One parameter.

Isolated, doubled, backward. `isolated_file_masks[]` exists in `bb_tables.hpp`,
unused. It has lost the pawn hash it was expected to share, so its cost is
unpaid for and has to be measured on its own terms -- with the weights forced
non-zero, per DEC-047. The set-wise fills the passed pawn term already computes
are the place to look first: doubled pawns fall out of a front span that is
already built.

`passed_w_pawns_masks[]` and `passed_b_pawns_masks[]` exist in `bb_tables.hpp`,
unused. Tapered by rank. Needs the decision on where it runs: stage one pays at
every node, stage two shares a margin budget that king safety has already
tightened to 0.365 % clamped.

## What a verdict costs, measured

**4.5 hours, not the one hour assumed.** SPRT 1 ran 2024 games in 4 h 30 m;
SPRT 2 reached its bound in 1300 games and 2 h 53 m. `--fast` is
`elo0=0 elo1=10 alpha=0.10 beta=0.10` over 3000 rounds at 10+0.2 on four cores.
Four terms remain and each needs at least one run, so this step's floor is
roughly a day of machine time and the plan should be read against that.


## What the step came to

**Three terms of the five are worth something and two are not.**

| term | error improvement | cost | verdict |
|---|---|---|---|
| king safety | 0.000304 | fused into mobility's loop | **+20.87 +/- 15.41**, H1 |
| passed pawns | 0.000206 | 4.5 % | **+17.34 +/- 13.51**, H1 |
| pawn structure | 0.000126 | 4.1 % | **+13.05 +/- 11.15**, H1 |
| piece placement | 0.000161 | 3.1-4.0 % | **0**, -5.48 +/- 11.46, H0 |
| tempo | 0.000043 | ~0 | **0**, -0.69 +/- 9.64, unresolved |

Plus one correctness fix found under king safety and measured on its own:
`evaluate_lazy()` returned a lower bound it did not have. **0**, -6.52 +/- 11.69,
H0 accepted, kept anyway because a lower bound that is not a lower bound is
wrong whatever the scoreboard says.

Six SPRTs, roughly 20 hours of machine time, 13462 games.

## The four things worth carrying forward

**1. Held-out error does not predict Elo, in either direction.** Three terms
running it understated the measurement, which was beginning to look like a rule
worth relying on. Then piece placement had the second-largest error improvement
of the five and measured -5.48. It ranks what to try. Only the match decides.

**2. A term shipped at zero weights is deleted, not measured.** DEC-047. The
weights are `const` with constant initialisers in the same translation unit, so
the compiler folds them and removes the loops that feed them. Three gates were
run against the passed pawn term at zero and all three reported no difference,
because there was none to report. Costs are now measured with the weights forced
non-zero, and two builds compared on wall time must walk the identical tree --
nodes per second varies +/- 8 % with tree size for the same binary, which is
larger than every effect in this step.

**3. An isolated benchmark cannot price anything that touches memory here.**
DEC-046. `bench_eval` said the pawn hash recovered 61 % of the passed pawn term;
the real search said it recovered nothing, and the cache was discarded after
being built in full. That is the second time, DEC-039 being the first.

**4. Corpus sweeps and hand-built cases are not substitutes.** Three deliberate
perturbations across two terms passed a full sweep of the inherited corpus and
were caught only by hand-built positions -- one by cancellation between the two
sides, one because no corpus position exercised the feature at all. Every term
in this step needed positions added before its coverage assertion could hold.

## What is left behind

**Two inert terms.** Piece placement and tempo are in the tree at zero weight
with their tuner plumbing intact. They cost nothing -- the compiler removes them
-- and re-measuring either is one fit and one match rather than a rewrite.

**A candidate worth a step, not a re-run.** Split piece placement and measure the
bishop pair alone. It is the only one of those four features that is free, clang
rewrites `count_bits(x) >= 2` into `x & (x - 1)`, and it carried the largest
weight the step fitted at eg +55. The three rook features cost six popcounts
between them and their weights are small and mutually cancelling.

**A structural fix the tuner needs.** `--only` groups are contiguous ranges and
the last one runs to `PARAM_COUNT`, so every new term is silently swallowed by
the group before it until that group is re-ended. This happened on **four
consecutive terms** and never once failed a test; the only symptom is a fit
returning the new weights exactly as it was handed them.

**A question this step raised and did not answer.** The tuning corpus is
self-play by an engine predating every term here. A residual fitted on it
encodes what correlated with winning in *those* games, which is not the same as
what causes winning now. Minimising squared error on that corpus and playing
better came apart for the first time at piece placement. Regenerating the corpus
from the current engine costs 95 minutes and is the obvious thing to try before
trusting another residual.
