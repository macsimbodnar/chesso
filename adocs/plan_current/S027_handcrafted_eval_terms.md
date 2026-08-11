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
done:

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
