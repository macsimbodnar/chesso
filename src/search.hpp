#pragma once
#include "data_structures.hpp"


search_t search_best_move(int depth,
                          board_t* board,
                          global_state_t* globals,
                          search_state_t* state);

bool is_pv_legal(board_t* board,
                 global_state_t* globals,
                 const pv_t* pv);
