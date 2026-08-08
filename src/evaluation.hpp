#pragma once
#include "data_structures.hpp"

// Positive means the side to move is better, whatever colour that is. Callers
// use the number as it comes; there is no sign to apply.
int evaluate(const board_t* board);

// How far into the game the position is: 24 with a full set of pieces, 0 once
// only kings and pawns remain. Tapered terms interpolate on it. Pawns and kings
// do not count toward it, and promotions cannot push it above 24.
int game_phase(const board_t* board);

#define GAME_PHASE_MAX 24
int capture_score(const board_t* board, move_t move);
int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move);
