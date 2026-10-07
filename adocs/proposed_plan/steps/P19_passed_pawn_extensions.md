id:         P19 (proposed; the S-id is allocated at adoption)
goal:       the passed-pawn score by rank gains the king, the path and the rook: king distances to the stop square, a blocked or attacked path, a rook behind the passer, and candidate passers
accepts:    (1) features: own and enemy king distance to the passer's stop square scaled by rank; stop square occupied; path to promotion free, attacked, or defended; own or enemy rook behind the passer on its file; candidate passers by rank; (2) fitted by P02 with every other constant frozen; (3) a test set of passer endgames labelled by a Syzygy probe through python-chess, or by Stockfish where no tablebase applies (CHESS rule), read for sign agreement before the SPRT; (4) the pawn cache re-measured (P15 accepts 4) and switched on if it now pays; (5) one SPRT `{0, 5}`
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, tests/test_eval_positions.hpp, adocs/specs.md
excludes:   connected and phalanx pawns (P20); unstoppable-passer rules (P24 owns endgame-specific knowledge)
closes:
paused_by:
author:
done:

## Description it is implemented from

CPW *Passed Pawn*; Stash changelog v32 (king proximity in passed-pawn
evaluation, +22.27) and v28 (candidate passers, +6.87); Ethereal 11.00 and
11.25 release notes (passed-pawn safe paths, candidate passed pawns).

## From the record

The S028 profile: in pawn endgames the evaluation overstated chesso's
position by +204 on average, the one phase where the optimism survived the
fit (50 moves; small sample).
