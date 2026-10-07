id:         P22 (proposed; the S-id is allocated at adoption)
goal:       two verdicts on piece placement: v1 outposts and bishop-pawn terms, v2 the four zero-weight placement terms fitted on the new corpus and measured alone
accepts:    (1) v1 features: knight and bishop on an outpost (enemy half, no enemy pawn can attack it, defended by an own pawn), a knight that can reach an outpost, a bishop's own pawns on its colour (blocked ones counted separately), a minor shielded by an own pawn in front; fitted from zero, others frozen; (2) v2: bishop pair, rook on an open file, on a half-open file, on the seventh rank, fitted on P03's corpus with every other constant frozen; (3) one SPRT `{0, 5}` each, v2 against v1's tree
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   space (P23); trapped-piece patterns
closes:
paused_by:
author:
done:

## Description it is implemented from

CPW *Outposts*, *Bad Bishop*, *Bishop Pair*, *Rook on Open File*. Stash
changelog v28 (knight outposts and shielded minors, +10.16). Ethereal 12.00
release notes (knight and rook scoring by openness).

## From the record

The four placement terms ship at zero (`src/evaluation.cpp`,
`piece_placement_mg`/`_eg`). `specs.md`: the bishop pair and the three rook
features shared one bundled SPRT, −5.48 ± 11.46 over 2284 games, at bounds
that could not resolve the published +8.2 and +9.86 for the parts, and were
zeroed by hand; S100 corrected the record's "measured zero" wording. Every fit
since S065 carries `--freeze tempo,piece_placement` (DEC-057). v2 measures
them separately, on fresh data, once.
