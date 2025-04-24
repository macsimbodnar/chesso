#pragma once
#include "data_structures.hpp"

const tt_entry_t* get_entry_from_tt(const transposition_table_t* tt,
                                    const board_t* board);

void store_entry_to_tt(transposition_table_t* tt,
                       const board_t* board,
                       int depth,
                       int score,
                       node_type_t type,
                       const move_t* best_move);


// // Reuse TT entry if found
// const tt_entry_t* tt_entry = get_entry_from_tt(state->tt, board, depth);
// if (ply > 0 && tt_entry != nullptr && tt_entry->depth >= depth) {
//   if (tt_entry->type == TT_PV_NODE) {
//     state->best_move = tt_entry->best_move;
//     return tt_entry->score;
//   } else if (tt_entry->type == TT_BETA_NODE && tt_entry->score >= beta) {
//     return tt_entry->score;
//   } else if (tt_entry->type == TT_ALPHA_NODE && tt_entry->score <= alpha) {
//     return tt_entry->score;
//   }
// }


// // Store the node in TT
// node_type_t type = TT_ALPHA_NODE;
// if (best_so_far <= alpha0) {
//   type = TT_ALPHA_NODE;
// } else if (best_so_far >= beta) {
//   type = TT_BETA_NODE;
// } else {
//   type = TT_PV_NODE;
// }
// store_entry_to_tt(state->tt, board, depth, best_so_far, type, &best_move);


// void generate_pv(const board_t* board, search_state_t* state, int depth)
// {
//   assert(board != nullptr);
//   assert(state != nullptr);
//   board_t tmp_board = *board;
//   state->pv.pv_length[0] = 0;

//   const move_t* next_move = &state->best_move;
//   while (next_move != nullptr && depth > 0) {
//     state->pv.pv_table[0][state->pv.pv_length[0]] = *next_move;
//     state->pv.pv_length[0]++;

//     make_move(next_move, &tmp_board, nullptr);
//     const tt_entry_t* next_entry =
//         &state->tt->entries[tmp_board.game_state.zobrist_key % TT_SIZE];

//     if (next_entry == nullptr ||
//         next_entry->key != tmp_board.game_state.zobrist_key ||
//         next_entry->type == TT_ALPHA_NODE) {
//       return;
//     }

//     next_move = &next_entry->best_move;
//     --depth;
//   }
// }


// generate_pv(board, state, depth);