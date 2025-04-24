#pragma once
#include "data_structures.hpp"

/**
 * 1. negamax                                                               DONE
 * 2. alphabeta pruning                                                     DONE
 * 3. quiescence search                                                     DONE
 * 3.5 Move ordering using MVV-LVA                                          DONE
 * 4. zobrist hashes                                                        DONE
 * 5. transposition table                                                   DONE
 * 6. iterative deepening                                                   DONE
 * 7. PVS/negascout in place of alphabeta
 * 8. nullmove pruning
 * 9. late move reduction
 * 10. Mover oredering with killer moves, history, SEE, etc.
 * 11. other pruning / reduction / extension.
 */

search_t experimental_search(int depth,
                             const board_t* board,
                             search_state_t* state);
