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
