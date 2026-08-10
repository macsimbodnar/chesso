id:         S027
goal:       mobility, king safety, passed pawns, pawn structure, bishop pair, tempo
accepts:    one SPRT per term, added one at a time; terms that measure zero are recorded as zero and the record says so
touches:    src/eval_tables.hpp, src/evaluation.cpp
excludes:   building the tuner, which is S028 and runs before this step
decisions:  DEC-033
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

## Order, and what already exists to build on

In descending order of what engines generally report:

1. **Mobility** -- legal move count per piece type, tapered. The generator
   already produces the attack sets cheaply.
2. **King safety** -- pawn shield, attacker count and weight on the king zone,
   open files near the king.
3. **Passed pawns** -- tapered by rank. `passed_w_pawns_masks[]` and
   `passed_b_pawns_masks[]` exist in `bb_tables.hpp`, unused.
4. **Pawn structure** -- isolated, doubled, backward. `isolated_file_masks[]`
   exists, unused. Cache in a pawn hash table; pawn structure changes rarely.
5. **Bishop pair**, **rook on open and half-open file**, **rook on seventh**.
6. **Tempo** -- a small bonus for the side to move.

Expect single or low double digit Elo each and expect some to measure zero.
Every term must be added incrementally through the S014 accumulators, or
`evaluate()` becomes a quarter of nodes per second again.

## Standing caveat

Most of this is thrown away when S029 lands. It is done anyway because NNUE
training data comes from self-play by an engine that already plays reasonably,
and because PeSTO reaches about 3125 on CCRL Blitz on piece-square tables and
search alone. The floor is what matters, not the ceiling.
