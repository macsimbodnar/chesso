#include "evaluation.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"


// clang-format off
#define VALUE_PAWN    100
#define VALUE_KNIGHT  300
#define VALUE_BISHOP  350
#define VALUE_ROOK    500
#define VALUE_QUEEN   1000
#define VALUE_KING    10000


#define DOUBLE_PAWN_PENALTY -10
#define ISOLATED_PAWN_PENALTY -10

#define SEMI_OPEN_FILE_BONUS 10
#define OPEN_FILE_BONUS 15

#define KING_SHIELD_BONUS 5


static inline constexpr int pawn_postion_value_table[64] = {
 90, 90, 90, 90, 90, 90, 90, 90,
 30, 30, 30, 40, 40, 30, 30, 30,
 20, 20, 20, 30, 30, 30, 20, 20,
 10, 10, 10, 20, 20, 10, 10, 10,
  5,  5, 10, 20, 20,  5,  5,  5,
  0,  0,  0,  5,  5,  0,  0,  0,
  0,  0,  0,-10,-10,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0
};

static inline constexpr int knight_postion_value_table[64] = {
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0, 10, 10,  0,  0, -5,
 -5,  5, 20, 20, 20, 20,  5, -5,
 -5, 10, 20, 30, 30, 20, 10, -5,
 -5, 10, 20, 30, 30, 20, 10, -5,
 -5,  5, 20, 10, 10, 20,  5, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,-10,  0,  0,  0,  0,-10, -5
};

static inline constexpr int bishop_postion_value_table[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,
  0, 20,  0, 10, 10,  0, 20,  0,
  0,  0, 10, 20, 20, 10,  0,  0,
  0,  0, 10, 20, 20, 10,  0,  0,
  0, 10,  0,  0,  0,  0, 10,  0,
  0, 30,  0,  0,  0,  0, 30,  0,
  0,  0,-10,  0,  0,-10,  0,  0
};

static inline constexpr int rook_postion_value_table[64] = {
 50, 50, 50, 50, 50, 50, 50, 50,
 50, 50, 50, 50, 50, 50, 50, 50,
  0,  0, 10, 20, 20, 10,  0,  0,
  0,  0, 10, 20, 20, 10,  0,  0,
  0,  0, 10, 20, 20, 10,  0,  0,
  0,  0, 10, 20, 20, 10,  0,  0,
  0,  0, 10, 20, 20, 10,  0,  0,
  0,  0,  0, 20, 20,  0,  0,  0
};

static inline constexpr int queen_postion_value_table[64] = {
-20,-10,-10, -5, -5,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5,  5,  5,  5,  0,-10,
 -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
-10,  5,  5,  5,  5,  5,  0,-10,
-10,  0,  5,  0,  0,  0,  0,-10,
-20,-10,-10, -5, -5,-10,-10,-20
};

static inline constexpr int king_postion_value_table[64] = {
  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  5,  5,  5,  5,  0,  0,
  0,  5,  5, 10, 10,  5,  5,  0,
  0,  5, 10, 20, 20, 10,  5,  0,
  0,  5, 10, 20, 20, 10,  5,  0,
  0,  0,  5, 10, 10,  5,  0,  0,
  0,  5,  5, -5, -5,  0,  5,  0,
  0,  0,  5,  0,-15,  0, 10,  0
};

static inline constexpr int black_indexes[64] = {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};


// Most valuable victim & less valuable attacker
/**
 *   (Victims) Pawn Knight Bishop   Rook  Queen   King
 * (Attackers)
 *       Pawn   105    205    305    405    505    605
 *     Knight   104    204    304    404    504    604
 *     Bishop   103    203    303    403    503    603
 *       Rook   102    202    302    402    502    602
 *      Queen   101    201    301    401    501    601
 *       King   100    200    300    400    500    600
*/

// [attacker][victim]
static inline constexpr int mvv_lva[12][12] = {
 {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
 {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
 {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
 {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
 {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
 {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},

 {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
 {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
 {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
 {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
 {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
 {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600}
};

static inline constexpr int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 };

// clang-format on


inline int double_pawns_score(bb_t board, index_t index)
{
  const int num_of_doubled_pawns = count_bits(board & file_masks[index]);
  const int result = (num_of_doubled_pawns > 1)
                         ? num_of_doubled_pawns * DOUBLE_PAWN_PENALTY
                         : 0;
  return result;
}


int evaluate(const bb_tables_t* tables, const board_t* board)
{
  assert(board != nullptr);

  // TODO:
  // Connected rook bonus

  int evaluation = 0;

  for (int piece = W_PAWN; piece <= B_KING; ++piece) {
    bb_t current_board = board->bitboards[piece];

    while (current_board) {
      const index_t index = get_lsb_index(current_board);
      switch (piece) {
          // ################################# WHITE PIECES
        case W_PAWN:
          evaluation += VALUE_PAWN;
          evaluation += pawn_postion_value_table[index];
          // Doubled pawns
          evaluation += double_pawns_score(board->bitboards[W_PAWN], index);

          // Isolated pawns
          if ((board->bitboards[W_PAWN] & isolated_file_masks[index]) == 0) {
            evaluation += ISOLATED_PAWN_PENALTY;
          }

          // Passed pawn
          if ((passed_w_pawns_masks[index] & board->bitboards[B_PAWN]) == 0) {
            const uint8_t rank = 7 - (index / 8);
            assert(rank < 8);
            evaluation += passed_pawn_bonus[rank];
          }

          break;
        case W_KNIGHT:
          evaluation += VALUE_KNIGHT;
          evaluation += knight_postion_value_table[index];
          break;
        case W_BISHOP:
          evaluation += VALUE_BISHOP;
          evaluation += bishop_postion_value_table[index];

          // Mobility
          evaluation += count_bits(
              get_bishop_attacks(tables, index, board->occupancies[BOTH]));
          break;
        case W_ROOK:
          evaluation += VALUE_ROOK;
          evaluation += rook_postion_value_table[index];

          // Open and semi open file bonus
          if ((board->bitboards[W_PAWN] & file_masks[index]) == 0) {
            evaluation += SEMI_OPEN_FILE_BONUS;
          }

          if (((board->bitboards[W_PAWN] | board->bitboards[B_PAWN]) &
               file_masks[index]) == 0) {
            evaluation += OPEN_FILE_BONUS;
          }

          break;
        case W_QUEEN:
          evaluation += VALUE_QUEEN;
          evaluation += queen_postion_value_table[index];
          evaluation += count_bits(
              get_queen_attacks(tables, index, board->occupancies[BOTH]));
          break;
        case W_KING:
          evaluation += VALUE_KING;
          evaluation += king_postion_value_table[index];

          // Open and semi open file penalty
          if ((board->bitboards[W_PAWN] & file_masks[index]) == 0) {
            evaluation -= SEMI_OPEN_FILE_BONUS;
          }

          if (((board->bitboards[W_PAWN] | board->bitboards[B_PAWN]) &
               file_masks[index]) == 0) {
            evaluation -= OPEN_FILE_BONUS;
          }

          // King safety bonus
          evaluation += count_bits(tables->king_attacks[index] &
                                   board->occupancies[WHITE]) *
                        KING_SHIELD_BONUS;

          break;
        // ################################# BLACK PIECES
        case B_PAWN:
          evaluation -= VALUE_PAWN;
          evaluation -= pawn_postion_value_table[black_indexes[index]];
          // Doubled pawns
          evaluation -= double_pawns_score(board->bitboards[B_PAWN], index);

          // Isolated pawns
          if ((board->bitboards[B_PAWN] & isolated_file_masks[index]) == 0) {
            evaluation -= ISOLATED_PAWN_PENALTY;
          }

          // Passed pawn bonus
          if ((passed_b_pawns_masks[index] & board->bitboards[W_PAWN]) == 0) {
            const uint8_t rank = index / 8;
            assert(rank < 8);
            evaluation -= passed_pawn_bonus[rank];
          }
          break;
        case B_KNIGHT:
          evaluation -= VALUE_KNIGHT;
          evaluation -= knight_postion_value_table[black_indexes[index]];
          break;
        case B_BISHOP:
          evaluation -= VALUE_BISHOP;
          evaluation -= bishop_postion_value_table[black_indexes[index]];

          // Mobility
          evaluation -= count_bits(
              get_bishop_attacks(tables, index, board->occupancies[BOTH]));
          break;
        case B_ROOK:
          evaluation -= VALUE_ROOK;
          evaluation -= rook_postion_value_table[black_indexes[index]];

          // Open and semi open file bonus
          if ((board->bitboards[B_PAWN] & file_masks[index]) == 0) {
            evaluation -= SEMI_OPEN_FILE_BONUS;
          }

          if (((board->bitboards[B_PAWN] | board->bitboards[W_PAWN]) &
               file_masks[index]) == 0) {
            evaluation -= OPEN_FILE_BONUS;
          }

          break;
        case B_QUEEN:
          evaluation -= VALUE_QUEEN;
          evaluation -= queen_postion_value_table[black_indexes[index]];
          evaluation -= count_bits(
              get_queen_attacks(tables, index, board->occupancies[BOTH]));
          break;
        case B_KING:
          evaluation -= VALUE_KING;
          evaluation -= king_postion_value_table[black_indexes[index]];

          // Open and semi open file penalty
          if ((board->bitboards[B_PAWN] & file_masks[index]) == 0) {
            evaluation += SEMI_OPEN_FILE_BONUS;
          }

          if (((board->bitboards[B_PAWN] | board->bitboards[W_PAWN]) &
               file_masks[index]) == 0) {
            evaluation += OPEN_FILE_BONUS;
          }

          // King safety bonus
          evaluation -= count_bits(tables->king_attacks[index] &
                                   board->occupancies[BLACK]) *
                        KING_SHIELD_BONUS;
          break;
        case EMPTY:
        default:
          assert(false);
          break;
      }

      POP_BIT(current_board, index);
    }
  }

  return evaluation;
}


int evaluate_move(const board_t* board,
                  const search_state_t* state,
                  move_t move,
                  size_t ply)
{
  assert(state != nullptr);

  // const tt_entry_t* tt_entry = tt_get_entry(state->tt, board);
  // if (ply > 0 && tt_entry != nullptr) {
  //   if (tt_entry->type == TT_PV_NODE && tt_entry->best_move == move) {
  //     return 1000000;
  //   }
  // }

  // Promotions (checked before captures so capture-promotions get both bonuses)
  if (MOVE_PROMOTED(move) != TO_NONE) {
    int promo_score = 0;
    switch (MOVE_PROMOTED(move)) {
      case TO_QUEEN:  promo_score = 20000; break;
      case TO_ROOK:   promo_score = 15000; break;
      case TO_KNIGHT:
      case TO_BISHOP: promo_score = 10000; break;
      default:        break;
    }
    if (MOVE_CAPTURE(move)) {
      const piece_t attacker = MOVE_PIECE(move);
      piece_t victim = get_piece(board, MOVE_TO(move));
      if (victim == EMPTY) victim = W_PAWN;
      promo_score += mvv_lva[attacker][victim];
    }
    return promo_score;
  }

  if (MOVE_CAPTURE(move)) {
    const piece_t attacker = MOVE_PIECE(move);

    // TODO: In case of en-passant this does not return the correct victim
    piece_t victim = get_piece(board, MOVE_TO(move));

    assert(attacker != EMPTY);

    // here we assume en-passant capture and set to pawn
    if (victim == EMPTY) {
      // Here we don't care about color
      victim = W_PAWN;
    }

    const int score = mvv_lva[attacker][victim];
    return 10000 + score;
  }

  // Killer & History
  if (move == state->killer_moves[0][ply]) {
    return 10000 - 1000;
  } else if (move == state->killer_moves[1][ply]) {
    return 10000 - 2000;
  } else {
    return state->history_moves[MOVE_PIECE(move)][MOVE_TO(move)];
  }

  return 0;
}


void order_moves(const board_t* board,
                 const search_state_t* state,
                 size_t ply,
                 size_t moves_size,
                 move_t moves[])
{
  assert(moves != nullptr);
  assert(moves_size <= MAX_MOVES);
  assert(state != nullptr);
  assert(state->killer_moves[0] != nullptr);
  assert(state->killer_moves[1] != nullptr);

  int scores[MAX_MOVES];
  for (size_t i = 0; i < moves_size; ++i) {
    scores[i] = evaluate_move(board, state, moves[i], ply);
  }

  // Insertion sort
  for (size_t i = 1; i < moves_size; ++i) {
    const int value = scores[i];
    const move_t move = moves[i];
    int j = i;

    while (j != 0 && scores[j - 1] < value) {
      scores[j] = scores[j - 1];
      moves[j] = moves[j - 1];
      --j;
    }

    scores[j] = value;
    moves[j] = move;
  }
}


bool should_reduce_move(move_t move)
{
  if (MOVE_PROMOTED(move) != TO_NONE || MOVE_CAPTURE(move)) { return false; }
  return true;
}


void order_captures(const board_t* board, move_t moves[], size_t moves_size)
{
  assert(moves != nullptr);
  assert(moves_size <= MAX_MOVES);

  int scores[MAX_MOVES];
  for (size_t i = 0; i < moves_size; ++i) {
    if (MOVE_CAPTURE(moves[i])) {
      const piece_t attacker = MOVE_PIECE(moves[i]);

      // TODO: In case of en-passant this does not return the correct victim
      piece_t victim = get_piece(board, MOVE_TO(moves[i]));

      assert(attacker != EMPTY);

      // here e assume en-passant capture and set to pawn
      if (victim == EMPTY) {
        // Here we don't care about color
        victim = W_PAWN;
      }

      scores[i] = mvv_lva[attacker][victim];
    } else {
      // Handling the promotions that are not captures
      scores[i] = 0;
    }
  }

  // Insertion sort
  for (size_t i = 1; i < moves_size; ++i) {
    const int value = scores[i];
    const move_t move = moves[i];
    int j = i;

    while (j != 0 && scores[j - 1] < value) {
      scores[j] = scores[j - 1];
      moves[j] = moves[j - 1];
      --j;
    }

    scores[j] = value;
    moves[j] = move;
  }
}


int get_max_gain()
{
  return VALUE_QUEEN;
}


int get_margin_value()
{
  return VALUE_PAWN;
}
