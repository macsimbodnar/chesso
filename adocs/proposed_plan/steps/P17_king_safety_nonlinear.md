id:         P17 (proposed; the S-id is allocated at adoption)
goal:       king safety becomes a per-side attack score passed through a fitted nonlinear transform, built from attacker weights by piece type, king-zone attacks, safe checks by piece type, weak king-ring squares, pawn shield and storm, and damping without the attacking queen
accepts:    (1) every feature defined in the step file before any fit, and in specs.md when it lands; (2) transform: middlegame grows with the square of the positive part, endgame linearly in it, per Grant 2020 §1.4; its scales are fitted parameters, through P02's chain-rule gradient (§3.5); (3) fitted on P03's corpus with every other constant frozen (S027's rule); held-out and validation loss reported; (4) INV-5's colour-symmetry test extended with king-attack positions; a set of attacked-king positions labelled by Stockfish at depth 20 through python-chess (CHESS rule, TOOLCHAIN's safe invocation) checked for sign agreement before the SPRT, a reading and not a gate; (5) one SPRT `{0, 5}`; (6) the nine linear features retire, DEC-044 superseded by a decision entry
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, tests/test_eval_positions.hpp, adocs/specs.md
excludes:   threats outside the king zone (P21); mobility (P18)
closes:
paused_by:
author:
done:

## Description it is implemented from

CPW *King Safety*: attack units by piece type, king zone, S-shaped growth of
danger with attackers, safe contact checks, pawn shield and storm, open files
by the king. Grant 2020 §1.4 and §3.5: a linear group per side passed through
a transform, and its gradient. Stash changelog v31 and v32: total attack
count and safe checks with a quadratic middlegame and linear endgame
transform; weak and safe squares redefined; one attacker counted when the
queen is present.

## Seeds (DEC-134)

Every weight fitted from zero by the tuner. Transform scales seeded so the
transform equals today's linear score at the median feature magnitude over
the corpus (form 2).

## From the record

DEC-044 chose linear "because fittable"; P02 removes the reason. P16 removes
the clamp. S027 shipped king safety at +20.87 as one of three passing terms.
