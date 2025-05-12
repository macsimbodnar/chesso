#pragma once
#include "data_structures.hpp"

void tt_reset(const transposition_table_t* tt);

const tt_entry_t* tt_get_entry(const transposition_table_t* tt,
                               const board_t* board);

void tt_store_entry(transposition_table_t* tt,
                    const board_t* board,
                    int depth,
                    int score,
                    node_type_t type,
                    const move_t* best_move);
