#pragma once
#include "data_structures.hpp"

// Allocates (or reallocates) the table. Must be called before any probe or
// store; everything else tolerates an unallocated table by doing nothing.
void tt_resize(transposition_table_t* tt, size_t megabytes);

void tt_free(transposition_table_t* tt);

void tt_reset(transposition_table_t* tt);

void tt_new_search(transposition_table_t* tt);

const tt_entry_t* tt_get_entry(const transposition_table_t* tt,
                               const board_t* board);

void tt_store_entry(transposition_table_t* tt,
                    const board_t* board,
                    int depth,
                    int score,
                    node_type_t type,
                    move_t best_move);
