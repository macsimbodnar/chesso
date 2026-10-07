id:         P21 (proposed; the S-id is allocated at adoption)
goal:       the evaluation prices pieces under attack: by pawns, by lesser pieces, attacked and undefended, attacked by the king, and safe pawn pushes that would attack a piece
accepts:    (1) features: a minor or major piece attacked by a pawn; a rook or queen attacked by a minor; a queen attacked by a rook; a piece attacked and undefended, by type; an undefended piece attacked by the enemy king; a safe pawn push that would attack a piece; side to move recorded so the trace can separate it; (2) fitted from zero by P02 with every other constant frozen; (3) read from P15's attack sets, nps cost stated; (4) one SPRT `{0, 5}`
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   king-zone attacks (P17); search-side threat indexing (P09 v1)
closes:
paused_by:
author:
done:

## Why

DEC-033: 95 of 160 expensive moves were unchanged at sixteen times the
search -- "what these terms have to fix is what the engine believes, not what
it can see". A hanging piece the static evaluation ignores is decided by
quiescence alone, and quiescence prunes.

## Description it is implemented from

Ethereal 11.00 and 11.25 release notes (threat evaluation, pawn-push
threats); Stash changelog v26 (initiative from threatened pieces, +10.13 STC,
+7.77 LTC); CPW *Attack and Defend Maps*.
