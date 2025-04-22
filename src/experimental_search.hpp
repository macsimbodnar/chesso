#pragma once
#include "data_structures.hpp"

/**
 * 1. negamax
 * 2. alphabeta pruning
 * 3. quiscence search
 * 3.5 Move ordering using MVV-LVA
 * 4. zobrist hashes
 * 5. transposition table
 * 6. iterative deepening
 * 7. PVS/negascout in place of alphabeta
 * 8. nullmove pruning
 * 9. late move reduction
 * 10. Mover oredering with killer moves, history, SEE, etc.
 * 11. other pruning / reduction / extension.
 */


void experimental_order_moves(move_t moves[],
                              size_t moves_size,
                              const board_t* board);

search_t experimental_search(int depth,
                             const board_t* board,
                             search_state_t* state);
