id:         P18 (proposed; the S-id is allocated at adoption)
goal:       mobility counts only the squares a piece can usefully reach and scores each count through a fitted per-piece table
accepts:    (1) a mobility area per side excluding squares attacked by enemy pawns, own pawns that are blocked or on their second and third ranks, and the own king; whether the own queen is excluded for minors is a pre-registered second leg, not a post-hoc choice; (2) a pinned piece counts only moves along its pin ray; (3) per-piece, per-count tables (knight 0–8, bishop 0–13, rook 0–14, queen 0–27) fitted by P02 from zero with every other constant frozen; (4) nps cost against the parent stated (P15's sets make it small); (5) one SPRT `{0, 5}`
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   king-zone attacks (P17); threats (P21)
closes:
paused_by:
author:
done:

## Why

Mobility is linear in a raw attack count that includes squares enemy pawns
attack. `specs.md`: the fit returns knight −1 middlegame, rook 0 endgame,
queen −6 endgame -- "what a correct fit gives back when the model cannot hold
the curve".

## Description it is implemented from

CPW *Mobility*: safe mobility, per-count tables, pins. Stash changelog v27
(mobility zone excluding rammed and low-rank pawns, +19.95) and v30 (pins,
+6.15).

## Seeds

Fitted from zero.
