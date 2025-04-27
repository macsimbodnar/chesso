#pragma once
#include "data_structures.hpp"


search_t search_best_move(int depth,
                          const board_t* board,
                          search_state_t* state);

bool is_pv_legal(const board_t* board, const pv_t* pv);
