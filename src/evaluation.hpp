#pragma once
#include "data_structures.hpp"

int evaluate(const board_t* board);
int capture_score(const board_t* board, move_t move);
int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move);
