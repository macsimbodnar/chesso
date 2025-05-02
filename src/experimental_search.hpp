#pragma once
#include "data_structures.hpp"


search_t experimental_search(int depth,
                             board_t* board,
                             global_state_t* globals,
                             search_state_t* state);
