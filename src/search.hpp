#pragma once
#include "data_structures.hpp"


search_t search(int depth, game_t* game, search_state_t* state);

// The leaf search. Declared here only so the tests can drive it directly;
// nothing outside search.cpp calls it.
int quiescence(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state);

#ifdef CHESSO_TUNE
// The reduction the built table holds for a (depth, move number) pair. Tune
// build only, and it exists for one test: LMR_BASE and LMR_DIVISOR are read
// once, when the table is built, so a setoption that moved a coefficient
// without rebuilding would be invisible from outside. S073.
int search_lmr_reduction_probe(int depth, int move_number);
#endif
