#include "bitboard.hpp"
#include <bit>
#include <random>
#include <unordered_map>
#include "bb_tables.hpp"
#include "data_structures.hpp"
#include "log.hpp"
#include "utils.hpp"


/******************************************************************************
 *                               MUST RUN FAST
 ******************************************************************************/
inline int count_bits(bb_t board)
{
  return std::popcount(board);
}


inline index_t get_lsb_index(bb_t board)
{
  // If 64 then invalid
  return std::countr_zero(board);
}


bb_t get_bishop_attacks(const bb_tables_t* tables,
                        index_t index,
                        bb_t occupancy)
{
  assert(tables != nullptr);

  occupancy &= tables->bishop_masks[index];
  occupancy *= bishop_magic_numbers[index];
  occupancy >>= 64 - bishop_relevant_bits_count[index];
  const bb_t result = tables->bishop_attacks[index][occupancy];
  return result;
}


bb_t get_rook_attacks(const bb_tables_t* tables, index_t index, bb_t occupancy)
{
  assert(tables != nullptr);

  occupancy &= tables->rook_masks[index];
  occupancy *= rook_magic_numbers[index];
  occupancy >>= 64 - rook_relevant_bits_count[index];
  const bb_t result = tables->rook_attacks[index][occupancy];
  return result;
}


bb_t get_queen_attacks(const bb_tables_t* tables, index_t index, bb_t occupancy)
{
  bb_t queen_attacks = get_bishop_attacks(tables, index, occupancy);
  queen_attacks |= get_rook_attacks(tables, index, occupancy);
  return queen_attacks;
}


bool is_attacked(const bb_tables_t* tables,
                 const board_t* board,
                 index_t index,
                 color_t color)
{
  assert(board != nullptr);

  {  // Handle pawn attacks
    if ((color == WHITE) &&
        (tables->pawn_attacks[BLACK][index] & board->bitboards[W_PAWN])) {
      return true;
    }

    if ((color == BLACK) &&
        (tables->pawn_attacks[WHITE][index] & board->bitboards[B_PAWN])) {
      return true;
      ;
    }
  }

  if (tables->knight_attacks[index] &
      ((color == WHITE) ? board->bitboards[W_KNIGHT]
                        : board->bitboards[B_KNIGHT])) {
    return true;
  }


  if (get_bishop_attacks(tables, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_BISHOP]
                        : board->bitboards[B_BISHOP])) {
    return true;
  }

  if (get_rook_attacks(tables, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_ROOK]
                        : board->bitboards[B_ROOK])) {
    return true;
  }

  // attacked by bishops
  if (get_queen_attacks(tables, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_QUEEN]
                        : board->bitboards[B_QUEEN])) {
    return true;
  }

  // attacked by kings
  if (tables->king_attacks[index] &
      ((color == WHITE) ? board->bitboards[W_KING]
                        : board->bitboards[B_KING])) {
    return true;
  }

  return false;
}


size_t generate_moves(const bb_tables_t* tables,
                      const board_t* board,
                      move_t moves[])
{
  assert(tables != nullptr);
  assert(board != nullptr);
  assert(moves != nullptr);

  const color_t color = board->active_color;
  size_t move_count = 0;

  for (uint8_t p_index = W_PAWN; p_index <= B_KING; ++p_index) {
    const piece_t piece = static_cast<piece_t>(p_index);
    bb_t bboard = board->bitboards[piece];
    bb_t attacks = BB_0;
    index_t from = INVALID_INDEX;
    index_t to = INVALID_INDEX;

    // Pawns and Castling
    if (color == WHITE) {
      if (piece == W_PAWN) {
        while (bboard) {
          from = get_lsb_index(bboard);
          to = from - 8;

          if (!(to < a8) && !GET_BIT(board->occupancies[BOTH], to)) {
            // Promotion
            if (from >= a7 && from <= h7) {
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_QUEEN, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_ROOK, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_BISHOP, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_KNIGHT, 0, 0, 0, 0);
            } else {
              moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);

              if ((from >= a2 && from <= h2) &&
                  !GET_BIT(board->occupancies[BOTH], to - 8)) {
                // Double push
                moves[move_count++] =
                    NEW_MOVE(from, to - 8, piece, 0, 0, 1, 0, 0);
              }
            }
          }

          // Pawn captures
          attacks =
              tables->pawn_attacks[color][from] & board->occupancies[BLACK];

          while (attacks) {
            // init target square
            to = get_lsb_index(attacks);

            // Attack & promotion
            if (from >= a7 && from <= h7) {
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_QUEEN, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_ROOK, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_BISHOP, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_KNIGHT, 1, 0, 0, 0);
            } else {
              moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
            }

            // moveto the next attack
            POP_BIT(attacks, to);
          }

          // En-passant
          if (board->en_passant != INVALID_INDEX) {
            const bb_t enp_attacks =
                tables->pawn_attacks[color][from] & (BB_1 << board->en_passant);

            if (enp_attacks) {
              const index_t enp_index = get_lsb_index(enp_attacks);
              moves[move_count++] =
                  NEW_MOVE(from, enp_index, piece, 0, 1, 0, 1, 0);
            }
          }

          // Move to next pawn
          POP_BIT(bboard, from);
        }
      }

      // Castling
      if (piece == W_KING) {
        // King side
        if (board->castling & WK) {
          // Check if the path is empty
          if (!GET_BIT(board->occupancies[BOTH], f1) &&
              !GET_BIT(board->occupancies[BOTH], g1)) {
            // Check if squares are not attacked
            if (!is_attacked(tables, board, e1, BLACK) &&
                !is_attacked(tables, board, f1, BLACK)) {
              moves[move_count++] = NEW_MOVE(e1, g1, piece, 0, 0, 0, 0, 1);
            }
          }
        }

        // Queen side
        if (board->castling & WQ) {
          // Check if the path is empty
          if (!GET_BIT(board->occupancies[BOTH], d1) &&
              !GET_BIT(board->occupancies[BOTH], c1) &&
              !GET_BIT(board->occupancies[BOTH], b1)) {
            // Check if squares are not attacked
            if (!is_attacked(tables, board, e1, BLACK) &&
                !is_attacked(tables, board, d1, BLACK)) {
              moves[move_count++] = NEW_MOVE(e1, c1, piece, 0, 0, 0, 0, 1);
            }
          }
        }
      }
    } else {
      if (piece == B_PAWN) {
        while (bboard) {
          from = get_lsb_index(bboard);
          to = from + 8;

          // Quiet pawn moves
          if (!(to > h1) && !GET_BIT(board->occupancies[BOTH], to)) {
            // Promotion
            if (from >= a2 && from <= h2) {
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_QUEEN, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_ROOK, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_BISHOP, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_KNIGHT, 0, 0, 0, 0);
            } else {
              // Normal pawn move
              moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);

              // Double push
              if ((from >= a7 && from <= h7) &&
                  !GET_BIT(board->occupancies[BOTH], to + 8)) {
                moves[move_count++] =
                    NEW_MOVE(from, to + 8, piece, 0, 0, 1, 0, 0);
              }
            }
          }

          // Pawn attacks
          attacks =
              tables->pawn_attacks[color][from] & board->occupancies[WHITE];

          // generate pawn captures
          while (attacks) {
            to = get_lsb_index(attacks);

            // Promotion
            if (from >= a2 && from <= h2) {
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_QUEEN, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_ROOK, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_BISHOP, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, TO_KNIGHT, 1, 0, 0, 0);
            } else {
              moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
            }

            // Move to next attack
            POP_BIT(attacks, to);
          }

          // En-passant
          if (board->en_passant != INVALID_INDEX) {
            const bb_t enp_attacks =
                tables->pawn_attacks[color][from] & (BB_1 << board->en_passant);

            if (enp_attacks) {
              const index_t enp_index = get_lsb_index(enp_attacks);

              moves[move_count++] =
                  NEW_MOVE(from, enp_index, piece, 0, 1, 0, 1, 0);
            }
          }

          // Loop to next pawn
          POP_BIT(bboard, from);
        }
      }

      // Castling
      if (piece == B_KING) {
        if (board->castling & BK) {
          // Check if the path is empty
          if (!GET_BIT(board->occupancies[BOTH], f8) &&
              !GET_BIT(board->occupancies[BOTH], g8)) {
            // Check if squares are not attacked
            if (!is_attacked(tables, board, e8, WHITE) &&
                !is_attacked(tables, board, f8, WHITE)) {
              moves[move_count++] = NEW_MOVE(e8, g8, piece, 0, 0, 0, 0, 1);
            }
          }
        }

        if (board->castling & BQ) {
          // Check if the path is empty
          if (!GET_BIT(board->occupancies[BOTH], d8) &&
              !GET_BIT(board->occupancies[BOTH], c8) &&
              !GET_BIT(board->occupancies[BOTH], b8)) {
            // Check if squares are not attacked
            if (!is_attacked(tables, board, e8, WHITE) &&
                !is_attacked(tables, board, d8, WHITE))
              moves[move_count++] = NEW_MOVE(e8, c8, piece, 0, 0, 0, 0, 1);
          }
        }
      }
    }

    // Knight
    if ((color == WHITE) ? piece == W_KNIGHT : piece == B_KNIGHT) {
      while (bboard) {
        from = get_lsb_index(bboard);

        attacks = tables->knight_attacks[from] &
                  ((color == WHITE) ? ~board->occupancies[WHITE]
                                    : ~board->occupancies[BLACK]);

        while (attacks) {
          to = get_lsb_index(attacks);

          if (!GET_BIT(((color == WHITE) ? board->occupancies[BLACK]
                                         : board->occupancies[WHITE]),
                       to)) {
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);
          } else {  // Capture
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
          }

          // Next attack
          POP_BIT(attacks, to);
        }

        // Next knight
        POP_BIT(bboard, from);
      }
    }

    // Bishop
    if ((color == WHITE) ? piece == W_BISHOP : piece == B_BISHOP) {
      while (bboard) {
        from = get_lsb_index(bboard);

        attacks = get_bishop_attacks(tables, from, board->occupancies[BOTH]) &
                  ((color == WHITE) ? ~board->occupancies[WHITE]
                                    : ~board->occupancies[BLACK]);

        while (attacks) {
          // init target square
          to = get_lsb_index(attacks);

          if (!GET_BIT(((color == WHITE) ? board->occupancies[BLACK]
                                         : board->occupancies[WHITE]),
                       to)) {
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);

          } else {  // Capture
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
          }
          // Next attack
          POP_BIT(attacks, to);
        }

        // Next bishop
        POP_BIT(bboard, from);
      }
    }

    // Rook
    if ((color == WHITE) ? piece == W_ROOK : piece == B_ROOK) {
      while (bboard) {
        from = get_lsb_index(bboard);
        attacks = get_rook_attacks(tables, from, board->occupancies[BOTH]) &
                  ((color == WHITE) ? ~board->occupancies[WHITE]
                                    : ~board->occupancies[BLACK]);
        while (attacks) {
          to = get_lsb_index(attacks);
          if (!GET_BIT(((color == WHITE) ? board->occupancies[BLACK]
                                         : board->occupancies[WHITE]),
                       to)) {
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);
          } else {  // Capture
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
          }

          // Next attack
          POP_BIT(attacks, to);
        }

        // Next rook
        POP_BIT(bboard, from);
      }
    }

    // Queen
    if ((color == WHITE) ? piece == W_QUEEN : piece == B_QUEEN) {
      while (bboard) {
        from = get_lsb_index(bboard);

        attacks = get_queen_attacks(tables, from, board->occupancies[BOTH]) &
                  ((color == WHITE) ? ~board->occupancies[WHITE]
                                    : ~board->occupancies[BLACK]);
        while (attacks) {
          to = get_lsb_index(attacks);

          if (!GET_BIT(((color == WHITE) ? board->occupancies[BLACK]
                                         : board->occupancies[WHITE]),
                       to)) {
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);
          } else {  // Capture
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
          }
          // Next attack
          POP_BIT(attacks, to);
        }

        // Next queen ???
        POP_BIT(bboard, from);
      }
    }

    // King
    if ((color == WHITE) ? piece == W_KING : piece == B_KING) {
      while (bboard) {
        from = get_lsb_index(bboard);

        attacks = tables->king_attacks[from] &
                  ((color == WHITE) ? ~board->occupancies[WHITE]
                                    : ~board->occupancies[BLACK]);

        while (attacks) {
          to = get_lsb_index(attacks);
          if (!GET_BIT(((color == WHITE) ? board->occupancies[BLACK]
                                         : board->occupancies[WHITE]),
                       to)) {
            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);
          } else {  // Capture

            moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
          }
          // Next attack
          POP_BIT(attacks, to);
        }

        // Next king ???
        POP_BIT(bboard, from);
      }
    }
  }

  assert(move_count < MAX_MOVES);
  return move_count;
}


bool make_move(game_t* game, move_t encoded_move)
{
  assert(game != nullptr);

  const bb_tables_t* tables = &game->tables;
  const zobrist_randoms_t* randoms = &game->hash_randoms;
  board_t* board = &game->board;
  history_t* history = &game->history;

  const hash_t old_hash = board->hash;

  // Store the history
  history->entries[history->size++] = {*board, game->repetitions.size};
  assert(history->size < HISTORY_MAX_SIZE);

  unpacked_move_t move(encoded_move);

  POP_BIT(board->bitboards[move.piece], move.from);
  board->hash ^= randoms->piece_randoms[move.piece][move.from];

  SET_BIT(board->bitboards[move.piece], move.to);
  board->hash ^= randoms->piece_randoms[move.piece][move.to];

  if (move.capture) {
    // pick up bitboard piece index ranges depending on side
    piece_t start_piece;
    piece_t end_piece;

    if (board->active_color == WHITE) {
      start_piece = B_PAWN;
      end_piece = B_KING;
    } else {
      start_piece = W_PAWN;
      end_piece = W_KING;
    }

    // loop over bitboards opposite to the current side to move
    for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
      if (GET_BIT(board->bitboards[bb_piece], move.to)) {
        POP_BIT(board->bitboards[bb_piece], move.to);
        board->hash ^= randoms->piece_randoms[bb_piece][move.to];
        break;
      }
    }
  }

  static const piece_t w_promotion_map[] = {W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK,
                                            W_QUEEN};
  static const piece_t b_promotion_map[] = {B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK,
                                            B_QUEEN};

  if (move.promoted_to) {  // TODO: handle better the promotion in the move.
    if (board->active_color == WHITE) {
      POP_BIT(board->bitboards[W_PAWN], move.to);
      board->hash ^= randoms->piece_randoms[W_PAWN][move.to];

      const piece_t promoted_to = w_promotion_map[move.promoted_to];

      SET_BIT(board->bitboards[promoted_to], move.to);
      board->hash ^= randoms->piece_randoms[promoted_to][move.to];

    } else {
      POP_BIT(board->bitboards[B_PAWN], move.to);
      board->hash ^= randoms->piece_randoms[B_PAWN][move.to];

      const piece_t promoted_to = b_promotion_map[move.promoted_to];

      SET_BIT(board->bitboards[promoted_to], move.to);
      board->hash ^= randoms->piece_randoms[promoted_to][move.to];
    }
  }

  if (move.en_passant) {
    (board->active_color == WHITE)
        ? POP_BIT(board->bitboards[B_PAWN], move.to + 8)
        : POP_BIT(board->bitboards[W_PAWN], move.to - 8);

    if (board->active_color == WHITE) {
      POP_BIT(board->bitboards[B_PAWN], move.to + 8);
      board->hash ^= randoms->piece_randoms[B_PAWN][move.to + 8];
    } else {
      POP_BIT(board->bitboards[W_PAWN], move.to - 8);
      board->hash ^= randoms->piece_randoms[W_PAWN][move.to - 8];
    }
  }

  // Reset en-passant
  if (board->en_passant != INVALID_INDEX) {
    board->hash ^= randoms->ep_randoms[board->en_passant];
    board->en_passant = INVALID_INDEX;
  }

  if (move.double_push) {
    if (board->active_color == WHITE) {
      board->en_passant = move.to + 8;
    } else {
      board->en_passant = move.to - 8;
    }

    board->hash ^= randoms->ep_randoms[board->en_passant];
  }

  if (move.castling) {
    switch (move.to) {
      // White castles king side
      case (g1):
        // move H rook
        POP_BIT(board->bitboards[W_ROOK], h1);
        board->hash ^= randoms->piece_randoms[W_ROOK][h1];
        SET_BIT(board->bitboards[W_ROOK], f1);
        board->hash ^= randoms->piece_randoms[W_ROOK][f1];
        break;

      // white castles queen side
      case (c1):
        // move A rook
        POP_BIT(board->bitboards[W_ROOK], a1);
        board->hash ^= randoms->piece_randoms[W_ROOK][a1];
        SET_BIT(board->bitboards[W_ROOK], d1);
        board->hash ^= randoms->piece_randoms[W_ROOK][d1];
        break;

      // black castles king side
      case (g8):
        // move H rook
        POP_BIT(board->bitboards[B_ROOK], h8);
        board->hash ^= randoms->piece_randoms[B_ROOK][h8];
        SET_BIT(board->bitboards[B_ROOK], f8);
        board->hash ^= randoms->piece_randoms[B_ROOK][f8];
        break;

      // black castles queen side
      case (c8):
        // move A rook
        POP_BIT(board->bitboards[B_ROOK], a8);
        board->hash ^= randoms->piece_randoms[B_ROOK][a8];
        SET_BIT(board->bitboards[B_ROOK], d8);
        board->hash ^= randoms->piece_randoms[B_ROOK][d8];
        break;
    }
  }

  // Handle half move clock
  if (move.capture || move.piece == W_PAWN || move.piece == B_PAWN) {
    board->halfmove_clock = 0;
  } else {
    board->halfmove_clock += 1;
  }

  // Update castling rights
  board->hash ^= randoms->castling_randoms[board->castling];
  board->castling &= castling_rights[move.from];
  board->castling &= castling_rights[move.to];
  board->hash ^= randoms->castling_randoms[board->castling];

  // Update occupancies
  memset(board->occupancies, BB_0, sizeof(board->occupancies));

  for (int bb_piece = W_PAWN; bb_piece <= W_KING; bb_piece++) {
    board->occupancies[WHITE] |= board->bitboards[bb_piece];
  }

  for (int bb_piece = B_PAWN; bb_piece <= B_KING; bb_piece++) {
    board->occupancies[BLACK] |= board->bitboards[bb_piece];
  }

  board->occupancies[BOTH] |= board->occupancies[WHITE];
  board->occupancies[BOTH] |= board->occupancies[BLACK];


  // change side
  board->hash ^= randoms->side_randoms[board->active_color];
  if (board->active_color == WHITE) {
    board->active_color = BLACK;
  } else {
    board->active_color = WHITE;

    // After black turn update the full move counter as well
    board->fullmove_counter++;
  }
  board->hash ^= randoms->side_randoms[board->active_color];


  // Store repetitions
  game->repetitions.entries[game->repetitions.size++] = old_hash;
  assert(game->repetitions.size < REPETITION_MAX_SIZE);

  // Check legality
  if (is_attacked(tables, board,
                  (board->active_color == WHITE)
                      ? get_lsb_index(board->bitboards[B_KING])
                      : get_lsb_index(board->bitboards[W_KING]),
                  board->active_color)) {
    // Restore board
    history->size--;
    *board = history->entries[history->size].board;
    game->repetitions.size = history->entries[history->size].repetition_size;

    return false;
  } else {
    return true;
  }
}


void unmake_move(game_t* game)
{
  assert(game != nullptr);

  if (game->history.size < HISTORY_MAX_SIZE) {
    game->history.size--;
    game->board = game->history.entries[game->history.size].board;
    game->repetitions.size =
        game->history.entries[game->history.size].repetition_size;
  }
}


bool is_position_repeated(const repetition_t* rep, hash_t hash)
{
  assert(rep != nullptr);

  for (size_t i = 0; i < rep->size; ++i) {
    if (hash == rep->entries[i]) { return true; }
  }

  return false;
}


bool is_check(const game_t* game)
{
  assert(game != nullptr);
  const index_t index = (game->board.active_color == WHITE)
                            ? get_lsb_index(game->board.bitboards[W_KING])
                            : get_lsb_index(game->board.bitboards[B_KING]);

  // is king in check
  const bool in_check = is_attacked(&game->tables, &game->board, index,
                                    !game->board.active_color);

  return in_check;
}


void swap_side(game_t* game)
{
  assert(game != nullptr);

  game->board.hash ^= game->hash_randoms.side_randoms[game->board.active_color];
  game->board.active_color = !game->board.active_color;
  game->board.hash ^= game->hash_randoms.side_randoms[game->board.active_color];
}


void set_en_passant(game_t* game, index_t en_passant_index)
{
  assert(game != nullptr);

  game->board.hash ^= game->hash_randoms.ep_randoms[game->board.en_passant];
  game->board.en_passant = en_passant_index;
  game->board.hash ^= game->hash_randoms.ep_randoms[game->board.en_passant];
}


/******************************************************************************
 *                               UTIL FUNCTIONS
 * NOTE: Does not need to be optimized
 ******************************************************************************/
void cleanup_board(game_t* game)
{
  assert(game != nullptr);

  memset(&game->board, 0, sizeof(board_t));

  game->board.active_color = WHITE;
  game->board.castling = WQ | WK | BQ | BK;
  game->board.halfmove_clock = 0;
  game->board.en_passant = INVALID_INDEX;
  game->board.fullmove_counter = 1;
  game->board.hash = 0ULL;

  game->history.size = 0;
  game->repetitions.size = 0;
}


hash_t compute_full_hash(game_t* game)
{
  assert(game != nullptr);

  hash_t key = 0;

  // Xor pieces on the board
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const index_t index = position_to_index(file, rank);
      const piece_t piece = get_piece(&game->board, index);

      if (piece != EMPTY) {
        key ^= game->hash_randoms.piece_randoms[piece][index];
      }
    }
  }

  // Xor side to move
  key ^= game->hash_randoms.side_randoms[game->board.active_color];

  // Xor castling
  key ^= game->hash_randoms.castling_randoms[game->board.castling];

  // Xor en-passant
  key ^= game->hash_randoms.ep_randoms[game->board.en_passant];

  return key;
}


bool load_FEN(const std::string& FEN, game_t* game)
{
  assert(game != nullptr);
  board_t* board = &game->board;

  cleanup_board(game);

  // Start parsing
  auto sections = split_string(FEN);

  if (sections.size() != 6) {
    LOG_E << "Bad FEN string: " << FEN << END_E;
    return false;
  }

  /*****************************************************************************
   * 0. Piece placement tables
   *
   * pawn = "P"
   * knight = "N"
   * bishop = "B"
   * rook = "R"
   * queen = "Q"
   * and king = "K
   *
   * White ("PNBRQK")
   * Black ("pnbrqk")
   ****************************************************************************/

  uint8_t file = 0;
  uint8_t rank = 7;

  for (const char c : sections[0]) {
    switch (c) {
      case '/':
        file = 0;
        --rank;
        break;

      case '1':
        file += 1;
        break;
      case '2':
        file += 2;
        break;
      case '3':
        file += 3;
        break;
      case '4':
        file += 4;
        break;
      case '5':
        file += 5;
        break;
      case '6':
        file += 6;
        break;
      case '7':
        file += 7;
        break;
      case '8':
        file += 8;
        break;

      case 'P':
      case 'N':
      case 'B':
      case 'R':
      case 'Q':
      case 'K':
      case 'p':
      case 'n':
      case 'b':
      case 'r':
      case 'q':
      case 'k': {
        const index_t index = position_to_index(file, rank);
        const piece_t piece = char_to_piece(c);
        SET_BIT(board->bitboards[piece], index);
        ++file;
      } break;

      default:
        // We get a non valid string
        LOG_E << "Invalid char in FEN string [" << STR(c) << "]. FEN: " << FEN
              << END_E;

        return false;
        break;
    }
  }

  /***************************************************************************
   * 1. Active color
   **************************************************************************/
  if (sections[1].size() != 1) {
    LOG_E << "invalid active color section. FEN: " << FEN << END_E;
    return false;
  }

  char color = sections[1][0];
  switch (color) {
    case 'w':
      board->active_color = WHITE;
      break;
    case 'b':
      board->active_color = BLACK;
      break;
    default:
      LOG_E << "Invalid color char in FEN string [" << std::string(1, color)
            << "]. FEN: " << FEN << END_E;
      return false;
      break;
  }

  /***************************************************************************
   * 2. Castling availability
   *
   * "-" No castling available
   * "K" if White can castle kingside
   * "Q" if White can castle queenside
   * "k" if Black can castle kingside
   * "q" if Black can castle queenside
   **************************************************************************/
  if (sections[2].size() < 1 || sections[2].size() > 4) {
    LOG_E << "Invalid castling availability section size. FEN: " << FEN
          << END_E;
    return false;
  }

  board->castling = 0x00;
  for (const char c : sections[2]) {
    switch (c) {
      case '-':
        board->castling = 0x00;
        if (sections[2].size() != 1) {
          LOG_E
              << "Invalid castling availability section size. No castling "
                 "available char is set but the section size is too big. FEN: "
              << FEN << END_E;
          return false;
        }
        break;
      case 'K':
        board->castling |= WK;
        break;
      case 'Q':
        board->castling |= WQ;
        break;
      case 'k':
        board->castling |= BK;
        break;
      case 'q':
        board->castling |= BQ;
        break;

      default:
        LOG_E << "Invalid castling availability character ["
              << std::string(1, c) << "]. FEN: " + FEN << END_E;

        return false;
    }
  }

  /***************************************************************************
   * 3. En passant target square
   *
   * "-" None
   **************************************************************************/
  if (sections[3].size() < 1 || sections[3].size() > 2) {
    LOG_E << "Invalid en passant section size. FEN: " << FEN << END_E;
    return false;
  }

  if (sections[3].size() == 1 && sections[3][0] != '-') {
    LOG_E << "Invalid en passant section char [ "
          << std::string(1, sections[3][0]) << "]. FEN: " << FEN << END_E;
    return false;
  }

  if (sections[3].size() == 2) {
    if (!isalpha(sections[3][0]) || !isdigit(sections[3][1])) {
      LOG_E << "Invalid en passant section. Wrong algebraic notation ["
            << sections[3] << "]. FEN: " << FEN << END_E;
      return false;
    }
  }

  if (sections[3] == "-") {
    board->en_passant = INVALID_INDEX;
  } else {
    board->en_passant = str_to_index(sections[3]);
  }

  /***************************************************************************
   * 4. Halfmove clock
   *
   * The number of halfmoves since the last capture or pawn advance, used for
   * the fifty-move rule.
   **************************************************************************/
  const std::string& half_move = sections[4];
  if (half_move.size() < 1) {
    LOG_E << "Invalid Halfmove clock section size. FEN: " << FEN << END_E;
    return false;
  }

  if (!is_uint(half_move)) {
    LOG_E << "Invalid Halfmove clock section is not a number. FEN: " << FEN
          << END_E;
    return false;
  }

  try {
    board->halfmove_clock = static_cast<int>(std::stoul(half_move));
  } catch (std::exception& e) {
    LOG_E << "Can't convert Halfmove clock to integer.What: "
          << std::string(e.what()) << " FEN: " << FEN << END_E;
    return false;
  }

  /***************************************************************************
   * 5. Fullmove number
   *
   * The number of the full moves. It starts at 1 and is incremented after
   * Black's move.
   **************************************************************************/
  const std::string& full_move = sections[5];
  if (full_move.size() < 1) {
    LOG_E << "Invalid Fullmove number section size. FEN: " << FEN << END_E;
    return false;
  }

  if (!is_uint(full_move)) {
    LOG_E << "Invalid Fullmove number section is not a number. FEN: " << FEN
          << END_E;
    return false;
  }

  try {
    board->fullmove_counter = static_cast<int>(std::stoul(full_move));
  } catch (std::exception& e) {
    LOG_E << "Can't convert Fullmove number to integer. What: "
          << std::string(e.what()) << " FEN: " << FEN << END_E;
    return false;
  }

  if (board->fullmove_counter < 1) {
    LOG_E << "Fullmove number can't be less then 1 but it is "
          << std::string(STR(board->fullmove_counter)) << " FEN: " << FEN
          << END_E;
    return false;
  }

  // Populate occupancies
  for (int piece = W_PAWN; piece <= W_KING; ++piece) {
    board->occupancies[WHITE] |= board->bitboards[piece];
    board->occupancies[BLACK] |= board->bitboards[piece + B_PAWN];
  }

  board->occupancies[BOTH] |= board->occupancies[WHITE];
  board->occupancies[BOTH] |= board->occupancies[BLACK];

  board->hash = compute_full_hash(game);
  return true;
}


std::string generate_FEN(const board_t* board)
{
  assert(board != nullptr);

  std::stringstream ss;

  // Step 1: Board representation
  int empty_count = 0;

  for (int rank = 7; rank >= 0; --rank) {
    for (int file = 0; file < 8; ++file) {
      const auto index = position_to_index(file, rank);
      const auto piece = get_piece(board, index);

      if (piece == piece_t::EMPTY) {
        ++empty_count;
      } else {
        if (empty_count > 0) {
          ss << empty_count;
          empty_count = 0;
        }

        switch (piece) {
          case piece_t::B_KING:
            ss << 'k';
            break;
          case piece_t::B_QUEEN:
            ss << 'q';
            break;
          case piece_t::B_ROOK:
            ss << 'r';
            break;
          case piece_t::B_BISHOP:
            ss << 'b';
            break;
          case piece_t::B_KNIGHT:
            ss << 'n';
            break;
          case piece_t::B_PAWN:
            ss << 'p';
            break;
          case piece_t::W_KING:
            ss << 'K';
            break;
          case piece_t::W_QUEEN:
            ss << 'Q';
            break;
          case piece_t::W_ROOK:
            ss << 'R';
            break;
          case piece_t::W_BISHOP:
            ss << 'B';
            break;
          case piece_t::W_KNIGHT:
            ss << 'N';
            break;
          case piece_t::W_PAWN:
            ss << 'P';
            break;

          default:
            break;
        }
      }
    }

    if (empty_count > 0) {
      ss << empty_count;
      empty_count = 0;
    }

    if (rank > 0) { ss << '/'; }
  }

  // Step 2: Active color
  ss << (board->active_color == color_t::WHITE ? " w " : " b ");

  // Step 3: Castling rights
  bool has_castling_rights = false;

  if (board->castling & WK) {
    ss << 'K';
    has_castling_rights = true;
  }

  if (board->castling & WQ) {
    ss << 'Q';
    has_castling_rights = true;
  }

  if (board->castling & BK) {
    ss << 'k';
    has_castling_rights = true;
  }

  if (board->castling & BQ) {
    ss << 'q';
    has_castling_rights = true;
  }

  if (!has_castling_rights) { ss << '-'; }

  ss << ' ';

  // Step 4: En passant target square
  if (board->en_passant != INVALID_INDEX) {
    ss << index_to_str(board->en_passant);
  } else {
    ss << '-';
  }

  ss << ' ';

  // Step 5: Halfmove clock
  ss << int(board->halfmove_clock) << ' ';

  // Step 6: Fullmove number
  ss << int(board->fullmove_counter);

  return ss.str();
}


/**
 * @brief Returns the indexes of the ambiguous moves.
 */
size_t get_ambiguous_move(const unpacked_move_t* move,
                          const move_t moves[],
                          size_t moves_size,
                          index_t result[])
{
  assert(moves != nullptr);
  assert(result != nullptr);

  size_t result_count = 0;
  for (size_t i = 0; i < moves_size; ++i) {
    const unpacked_move_t I(moves[i]);

    // If same piece, same destination and different source
    if (move->piece == I.piece && move->to == I.to && move->from != I.from) {
      result[result_count] = i;
      ++result_count;
    }
  }

  return result_count;
}


bool is_move_legal(game_t* game, move_t move)
{
  if (make_move(game, move)) {
    unmake_move(game);
    return true;
  }

  return false;
}


size_t count_legal_moves(game_t* game, move_t moves[], size_t count)
{
  size_t result = 0;

  for (size_t i = 0; i < count; ++i) {
    if (is_move_legal(game, moves[i])) { result++; }
  }

  return result;
}


std::string move_to_algebraic(game_t* game,
                              move_t encoded_move,
                              const move_t moves[],
                              size_t moves_size)
{
  assert(game != nullptr);
  assert(moves != nullptr);
  assert(moves_size > 0);

  board_t* board = &game->board;

  // Not using the piece_to_char function because the piece moved in
  // always upper case
  static const std::unordered_map<piece_t, char> piece_to_char_map = {
      {B_PAWN, 'P'},   {B_KNIGHT, 'N'}, {B_BISHOP, 'B'}, {B_ROOK, 'R'},
      {B_QUEEN, 'Q'},  {B_KING, 'K'},   {W_PAWN, 'P'},   {W_KNIGHT, 'N'},
      {W_BISHOP, 'B'}, {W_ROOK, 'R'},   {W_QUEEN, 'Q'},  {W_KING, 'K'},
      {EMPTY, ' '}};

  static const char file_to_char_map[8] = {'a', 'b', 'c', 'd',
                                           'e', 'f', 'g', 'h'};
  static const char rank_to_char_map[8] = {'1', '2', '3', '4',
                                           '5', '6', '7', '8'};

  std::string notation;
  const unpacked_move_t move(encoded_move);

  // Handle castling
  if (move.castling) {
    if (move.to == g1 || move.to == g8) {
      return "O-O";  // King-side castling
    }

    if (move.to == c1 || move.to == c8) {
      return "O-O-O";  // Queen-side castling
    }
  }

  if (move.piece != W_PAWN && move.piece != B_PAWN) {
    notation += piece_to_char_map.at(move.piece);  // Non-pawn pieces

    // If ambiguous move the add the from file
    index_t ambiguous_moves[MAX_MOVES];
    const size_t ambiguous_moves_count =
        get_ambiguous_move(&move, moves, moves_size, ambiguous_moves);

    if (ambiguous_moves_count > 0) {
      const position_t move_from_pos = index_to_position(move.from);
      bool is_file_unique = true;
      bool is_rank_unique = true;

      // Check if file or rank are unique for the move.from
      for (size_t i = 0; i < ambiguous_moves_count; ++i) {
        const index_t index = ambiguous_moves[i];
        const position_t i_pos = index_to_position(MOVE_FROM(moves[index]));

        if (move_from_pos.file == i_pos.file) { is_file_unique = false; }

        if (move_from_pos.rank == i_pos.rank) { is_rank_unique = false; }

        // Exit from the loop in case both are non unique. No make sense
        // to search for more
        if (!is_file_unique && !is_rank_unique) { break; }
      }

      if (is_file_unique) {
        // Check if file unique
        notation += file_to_char_map[move_from_pos.file];
      } else if (is_rank_unique) {
        // Check if rank unique
        notation += rank_to_char_map[move_from_pos.rank];
      } else {
        // In case none is unique use both
        notation += file_to_char_map[move_from_pos.file];
        notation += rank_to_char_map[move_from_pos.rank];
      }
    }
  }

  // Capture notation
  if (move.capture && move.piece != W_PAWN && move.piece != B_PAWN) {
    notation += 'x';
  }

  // Destination square
  notation += index_to_str(move.to);

  // Pawn captures (ex: exd5)
  if ((move.piece == W_PAWN || move.piece == B_PAWN) && move.capture) {
    notation =
        index_to_str(move.from)[0] + std::string("x") + index_to_str(move.to);
  }

  // Pawn promotion
  if (move.promoted_to > 0) {
    notation += "=";
    switch (move.promoted_to) {
      case TO_QUEEN:
        notation += 'Q';
        break;
      case TO_ROOK:
        notation += 'R';
        break;
      case TO_BISHOP:
        notation += 'B';
        break;
      case TO_KNIGHT:
        notation += 'N';
        break;
      default:
        break;
    }
  }

  // Handle check and mate
  const bool move_happened = make_move(game, encoded_move);
  assert(move_happened);

  if (move_happened) {
    move_t loc_moves[MAX_MOVES];
    const size_t loc_moves_count =
        generate_moves(&game->tables, &game->board, loc_moves);

    const size_t legal_moves_count =
        count_legal_moves(game, loc_moves, loc_moves_count);

    const piece_t king_to_select =
        (board->active_color == WHITE) ? W_KING : B_KING;

    const color_t opponent = (board->active_color == WHITE) ? BLACK : WHITE;

    const index_t king_index = get_lsb_index(board->bitboards[king_to_select]);

    const bool is_check =
        is_attacked(&game->tables, &game->board, king_index, opponent);

    if (is_check && legal_moves_count == 0) {
      // Check mate
      notation += '#';
    } else if (is_check) {
      // Append a '+' to the notation
      notation += '+';
    }

    unmake_move(game);
  }

  return notation;
}


move_t algebraic_to_move(std::string notation, game_t* game)
{
  assert(game != nullptr);
  board_t* board = &game->board;

  static const std::unordered_map<char, uint8_t> char_to_file_map = {
      {'a', 0}, {'b', 1}, {'c', 2}, {'d', 3},
      {'e', 4}, {'f', 5}, {'g', 6}, {'h', 7}};
  static const std::unordered_map<char, uint8_t> char_to_rank_map = {
      {'1', 0}, {'2', 1}, {'3', 2}, {'4', 3},
      {'5', 4}, {'6', 5}, {'7', 6}, {'8', 7}};

  const std::string original_notation = notation;

  unpacked_move_t result(BB_0);

  const color_t color = board->active_color;

  // Make a working copy of the move string.
  bool is_capture = false;

  // TODO: Use this
  // bool is_check = false;
  // bool is_mate = false;
  // if (notation.back() == '+') { is_check = true; }
  // if (notation.back() == '#') { is_mate = true; }

  // Remove any trailing check ('+') or checkmate ('#') symbols.
  while (!notation.empty() &&
         (notation.back() == '+' || notation.back() == '#')) {
    notation.pop_back();
  }

  // Parse castling
  if (notation == "O-O-O") {
    result.castling = true;
    switch (color) {
      case BLACK:
        result.from = e8;
        result.to = c8;
        result.piece = B_KING;
        break;
      case WHITE:
        result.from = e1;
        result.to = c1;
        result.piece = W_KING;
        break;
      default:
        assert(false);
        break;
    }

    return NEW_MOVE(result.from, result.to, result.piece, result.promoted_to,
                    result.capture, result.double_push, result.en_passant,
                    result.castling);
  }

  if (notation == "O-O") {
    result.castling = true;

    switch (color) {
      case BLACK:
        result.from = e8;
        result.to = g8;
        result.piece = B_KING;
        break;
      case WHITE:
        result.from = e1;
        result.to = g1;
        result.piece = W_KING;
        break;
      default:
        assert(false);
        break;
    }

    return NEW_MOVE(result.from, result.to, result.piece, result.promoted_to,
                    result.capture, result.double_push, result.en_passant,
                    result.castling);
  }


  // Parse non-castling moves
  size_t pos = 0;
  piece_t moving_piece;

  // If the move begins with a piece letter (K, Q, R, B, N), then use it.
  if (pos < notation.size() && std::isupper(notation[pos])) {
    char piece_char = notation[pos];
    switch (piece_char) {
      case 'K':
        moving_piece = (color == WHITE) ? W_KING : B_KING;
        break;
      case 'Q':
        moving_piece = (color == WHITE) ? W_QUEEN : B_QUEEN;
        break;
      case 'R':
        moving_piece = (color == WHITE) ? W_ROOK : B_ROOK;
        break;
      case 'B':
        moving_piece = (color == WHITE) ? W_BISHOP : B_BISHOP;
        break;
      case 'N':
        moving_piece = (color == WHITE) ? W_KNIGHT : B_KNIGHT;
        break;
      default:
        moving_piece = EMPTY;
        break;
    }
    ++pos;
  } else {
    // If no piece letter then it's a pawn move.
    moving_piece = (color == WHITE) ? W_PAWN : B_PAWN;
  }
  result.piece = moving_piece;

  // We now extract any disambiguation info.
  // This may be a file letter, a rank digit, or both.
  std::optional<char> disambiguous_file;
  std::optional<char> disambiguous_rank;

  // Look ahead for an 'x' (capture marker) or destination square.
  // We will also later remove any 'x' from the string.
  size_t temp_pos = pos;
  while (temp_pos < notation.size() && notation[temp_pos] != 'x' &&
         !(notation[temp_pos] >= 'a' && notation[temp_pos] <= 'h' &&
           (temp_pos + 1 < notation.size() && notation[temp_pos + 1] >= '1' &&
            notation[temp_pos + 1] <= '8'))) {
    // Assume any character here is part of disambiguation.
    char d = notation[temp_pos];
    if (d >= 'a' && d <= 'h')
      disambiguous_file = d;
    else if (d >= '1' && d <= '8')
      disambiguous_rank = d;
    ++temp_pos;
  }

  // Remove capture marker(s) from the string.
  std::string cleaned;
  for (char ch : notation.substr(pos)) {
    if (ch != 'x') {
      cleaned.push_back(ch);
    } else {
      is_capture = true;
    }
  }

  // Look for promotion: if there is an '=' then the following char is the
  // promotion piece.
  promotion_t promo = TO_NONE;
  size_t promo_pos = cleaned.find('=');
  if (promo_pos != std::string::npos && promo_pos + 1 < cleaned.size()) {
    char promo_char = cleaned[promo_pos + 1];
    switch (promo_char) {
      case 'Q':
        promo = TO_QUEEN;
        break;
      case 'R':
        promo = TO_ROOK;
        break;
      case 'B':
        promo = TO_BISHOP;
        break;
      case 'N':
        promo = TO_KNIGHT;
        break;
      default:
        promo = TO_NONE;
        break;
    }
    cleaned = cleaned.substr(0, promo_pos);
  }
  result.promoted_to = promo;

  // The destination square is the last two characters of the cleaned string.
  if (cleaned.size() < 2) {
    // Error: not enough characters to form a square.

    LOG_E << "Wrong formatting. Invalid Algebraic notation: "
          << original_notation << END_E;
    assert(false);
  }

  std::string dest_square = cleaned.substr(cleaned.size() - 2, 2);
  index_t to_index = str_to_index(dest_square);
  result.to = to_index;

  if (result.to >= INVALID_INDEX) {
    LOG_E << "Invalid destination square. Invalid Algebraic notation: "
          << original_notation << END_E;

    assert(false);
  }

  if (is_capture) {
    // Attempt to use the destination as capture piece
    result.capture = true;
  }

  // Any remaining characters between our initial pos and the destination
  // have been interpreted as disambiguation.
  // (In many SAN moves the disambiguation is omitted if unneeded.)
  // Here we already extracted potential disambiguation earlier.

  // Generate legal moves and search the compatible one
  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_moves(&game->tables, &game->board, moves);

  bool found = false;
  for (size_t i = 0; i < moves_count; ++i) {
    if (!is_move_legal(game, moves[i])) { continue; }

    const unpacked_move_t legal_move(moves[i]);

    if (legal_move.to == result.to && legal_move.piece == result.piece &&
        legal_move.promoted_to == result.promoted_to &&
        legal_move.capture == result.capture &&
        legal_move.castling == result.castling) {
      // Check for disambiguous
      const position_t legal_from_pos = index_to_position(legal_move.from);

      if (disambiguous_file.has_value()) {
        const uint8_t file = char_to_file_map.at(disambiguous_file.value());

        if (file != legal_from_pos.file) { continue; }
      }

      if (disambiguous_rank.has_value()) {
        const uint8_t rank = char_to_rank_map.at(disambiguous_rank.value());

        if (rank != legal_from_pos.rank) { continue; }
      }

      // We found the move
      found = true;
      result = legal_move;
      break;
    }
  }

  if (!found) {
    LOG_E << "No legal move found. Invalid Algebraic notation: "
          << original_notation << END_E;

    assert(false);
  }

  return NEW_MOVE(result.from, result.to, result.piece, result.promoted_to,
                  result.capture, result.double_push, result.en_passant,
                  result.castling);
}


/**
 * This function fixes the weirdo castling move that can be found in Polyglot
 * book format and some times the UCI can send that as well! (Looking at you
 * Cutechess!)
 *
 * We just need the move
 * white short      e1h1 -> e1g1
 * white long       e1a1 -> e1c1
 * black short      e8h8 -> e8g8
 * black long       e8a8 -> e8c8
 */
move_t fix_weirdo_castling(const board_t* board, move_t encoded_move)
{
  assert(board != nullptr);
  unpacked_move_t move(encoded_move);
  const piece_t p = get_piece(board, move.from);

  if (move.from == e1 && move.to == h1 && p == W_KING) {
    // white short
    move.to = g1;
    move.castling = true;
    move.piece = W_KING;

    return move.pack();
  } else if (move.from == e1 && move.to == a1 && p == W_KING) {
    // white long
    move.to = c1;
    move.castling = true;
    move.piece = W_KING;

    return move.pack();
  } else if (move.from == e8 && move.to == h8 && p == B_KING) {
    // black short
    move.to = g8;
    move.castling = true;
    move.piece = B_KING;

    return move.pack();
  } else if (move.from == e8 && move.to == a8 && p == B_KING) {
    // black short
    move.to = c8;
    move.castling = true;
    move.piece = B_KING;

    return move.pack();
  }

  return encoded_move;
}


/**
 * This function fixes the weirdo castling move that can be found in Polyglot
 * book format and some times the UCI can send that as well! (Looking at you
 * Cutechess!)
 *
 * We just need the move
 * white short      e1h1 -> e1g1
 * white long       e1a1 -> e1c1
 * black short      e8h8 -> e8g8
 * black long       e8a8 -> e8c8
 */
void fix_weirdo_castling(const board_t* board, unpacked_move_t* move)
{
  assert(board != nullptr);
  assert(move != nullptr);
  const piece_t p = get_piece(board, move->from);

  if (move->from == e1 && move->to == h1 && p == W_KING) {
    // white short
    move->to = g1;
    move->castling = true;
    move->piece = W_KING;
  } else if (move->from == e1 && move->to == a1 && p == W_KING) {
    // white long
    move->to = c1;
    move->castling = true;
    move->piece = W_KING;
  } else if (move->from == e8 && move->to == h8 && p == B_KING) {
    // black short
    move->to = g8;
    move->castling = true;
    move->piece = B_KING;
  } else if (move->from == e8 && move->to == a8 && p == B_KING) {
    // black short
    move->to = c8;
    move->castling = true;
    move->piece = B_KING;
  }
}


bool is_pv_legal(game_t* game, const pv_t* pv)
{
  assert(game != nullptr);
  assert(pv != nullptr);

  bool is_pv_ok = true;
  size_t make_move_counter = 0;

  // Empty pv is illegal
  if (pv->pv_length[0] < 1) {
    LOG_W << "Empty PV" << END_W;
    return false;
  }

  for (size_t i = 0; i < pv->pv_length[0]; ++i) {
    const move_t move_to_test = pv->pv_table[0][i];

    move_t moves[MAX_MOVES];
    const size_t moves_count =
        generate_moves(&game->tables, &game->board, moves);

    if (moves_count < 1) {
      is_pv_ok = false;
      break;
    }

    bool found = false;
    for (size_t move_index = 0; move_index < moves_count; ++move_index) {
      if (move_to_test == moves[move_index]) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG_W << "PV with illegal move: " << print_move(move_to_test) << END_W;
      is_pv_ok = false;
      break;
    }

    const bool legal = make_move(game, move_to_test);
    assert(legal);
    (void)legal;
    make_move_counter++;
  }

  for (size_t i = 0; i < make_move_counter; ++i) {
    unmake_move(game);
  }

  return is_pv_ok;
}


/******************************************************************************
 *                      INITIALIZATION FUNCTIONS
 * NOTE: Does not need to be optimized, they are called once at the start
 ******************************************************************************/
void init_zobrist(zobrist_randoms_t* zobrist)
{
  assert(zobrist != nullptr);

  // TODO(max): Move random initialization outside
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

  for (auto& piece_array : zobrist->piece_randoms) {
    for (uint64_t& random : piece_array) {
      random = dist(gen);
    }
  }

  for (uint64_t& random : zobrist->castling_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->side_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->ep_randoms) {
    random = dist(gen);
  }

  zobrist->initialized = true;
}


bb_t set_occupancy(uint64_t index, int mask_bit_count, bb_t attack_mask)
{
  bb_t occupancy = BB_0;

  for (int count = 0; count < mask_bit_count; ++count) {
    const uint64_t square = get_lsb_index(attack_mask);
    assert(square < 64);

    POP_BIT(attack_mask, square);

    // Check if on board
    if (index & (BB_1 << count)) { occupancy |= BB_1 << square; }
  }

  return occupancy;
}


bb_t precompute_pawn_attacks(color_t color, index_t square)
{
  assert(square < 64);

  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  switch (color) {
    case WHITE:
      if ((board >> 7) & NOT_A_FILE) { attacks |= (board >> 7); }
      if ((board >> 9) & NOT_H_FILE) { attacks |= (board >> 9); }
      break;

    case BLACK:
      if ((board << 7) & NOT_H_FILE) { attacks |= (board << 7); }
      if ((board << 9) & NOT_A_FILE) { attacks |= (board << 9); }
      break;

    default:
      assert(false);
  }

  return attacks;
}


bb_t precompute_knight_attacks(index_t square)
{
  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  if ((board >> 17) & NOT_H_FILE) { attacks |= (board >> 17); }
  if ((board >> 15) & NOT_A_FILE) { attacks |= (board >> 15); }
  if ((board >> 10) & NOT_GH_FILES) { attacks |= (board >> 10); }
  if ((board >> 6) & NOT_AB_FILES) { attacks |= (board >> 6); }

  if ((board << 17) & NOT_A_FILE) { attacks |= (board << 17); }
  if ((board << 15) & NOT_H_FILE) { attacks |= (board << 15); }
  if ((board << 10) & NOT_AB_FILES) { attacks |= (board << 10); }
  if ((board << 6) & NOT_GH_FILES) { attacks |= (board << 6); }

  return attacks;
}


bb_t precompute_king_attacks(index_t square)
{
  bb_t attacks = BB_0;
  bb_t board = BB_0;
  SET_BIT(board, square);

  if (board >> 8) { attacks |= (board >> 8); }
  if ((board >> 9) & NOT_H_FILE) { attacks |= (board >> 9); }
  if ((board >> 7) & NOT_A_FILE) { attacks |= (board >> 7); }
  if ((board >> 1) & NOT_H_FILE) { attacks |= (board >> 1); }

  if (board << 8) { attacks |= (board << 8); }
  if ((board << 9) & NOT_A_FILE) { attacks |= (board << 9); }
  if ((board << 7) & NOT_H_FILE) { attacks |= (board << 7); }
  if ((board << 1) & NOT_A_FILE) { attacks |= (board << 1); }

  return attacks;
}


bb_t precompute_bishop_attack_masks(index_t square)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r < 7 && f < 7; ++r, ++f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf + 1; r > 0 && f < 7; --r, ++f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr + 1, f = tf - 1; r < 7 && f > 0; ++r, --f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf - 1; r > 0 && f > 0; --r, --f) {
    attacks |= (BB_1 << ((r * 8) + f));
  }

  return attacks;
}


bb_t precompute_rook_attack_masks(index_t square)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r < 7; ++r) {
    attacks |= (BB_1 << ((r * 8) + tf));
  }
  for (r = tr - 1; r > 0; --r) {
    attacks |= (BB_1 << ((r * 8) + tf));
  }
  for (f = tf + 1; f < 7; ++f) {
    attacks |= (BB_1 << ((tr * 8) + f));
  }
  for (f = tf - 1; f > 0; --f) {
    attacks |= (BB_1 << ((tr * 8) + f));
  }

  return attacks;
}


bb_t precompute_bishop_attacks(index_t square, bb_t blocks)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; ++r, ++f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; --r, ++f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; ++r, --f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; --r, --f) {
    const bb_t candidate = (BB_1 << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


bb_t precompute_rook_attacks(index_t square, bb_t blocks)
{
  bb_t attacks = BB_0;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r <= 7; ++r) {
    const bb_t candidate = (BB_1 << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1; r >= 0; --r) {
    const bb_t candidate = (BB_1 << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf + 1; f <= 7; ++f) {
    const bb_t candidate = (BB_1 << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf - 1; f >= 0; --f) {
    const bb_t candidate = (BB_1 << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


void initialize_game_const_data(game_t* game)
{
  assert(game != nullptr);

  // initialize Hash randoms values
  init_zobrist(&game->hash_randoms);

  // Initialize bitboard tables
  bb_tables_t* tables = &game->tables;
  memset(tables, 0, sizeof(bb_tables_t));

  for (index_t square = 0; square < 64; ++square) {
    {
      // Init pawn attacks
      tables->pawn_attacks[WHITE][square] =
          precompute_pawn_attacks(WHITE, square);
      tables->pawn_attacks[BLACK][square] =
          precompute_pawn_attacks(BLACK, square);

      // Init knight attacks
      tables->knight_attacks[square] = precompute_knight_attacks(square);

      // Init king attacks
      tables->king_attacks[square] = precompute_king_attacks(square);
    }

    {  // Bishop
      tables->bishop_masks[square] = precompute_bishop_attack_masks(square);

      const bb_t bishop_attack_mask = tables->bishop_masks[square];

      const uint8_t bishop_num_relevant_bits =
          bishop_relevant_bits_count[square];

      const uint64_t bishop_occupancy_indicies =
          (BB_1 << bishop_num_relevant_bits);

      for (uint64_t index = 0; index < bishop_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, bishop_num_relevant_bits, bishop_attack_mask);

        const uint64_t magic_index =
            (occupancy * bishop_magic_numbers[square]) >>
            (64 - bishop_num_relevant_bits);

        tables->bishop_attacks[square][magic_index] =
            precompute_bishop_attacks(square, occupancy);
      }
    }

    {  // Rook
      tables->rook_masks[square] = precompute_rook_attack_masks(square);

      const bb_t rook_attack_mask = tables->rook_masks[square];
      const uint8_t rook_num_relevant_bits = rook_relevant_bits_count[square];
      const uint64_t rook_occupancy_indicies = (BB_1 << rook_num_relevant_bits);

      for (uint64_t index = 0; index < rook_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, rook_num_relevant_bits, rook_attack_mask);

        const uint64_t magic_index = (occupancy * rook_magic_numbers[square]) >>
                                     (64 - rook_num_relevant_bits);

        tables->rook_attacks[square][magic_index] =
            precompute_rook_attacks(square, occupancy);
      }
    }
  }
  // LOG_I << "Bitboard const tables initialized " << END_I;
}


piece_t get_piece(const board_t* board, index_t square)
{
  for (int piece = W_PAWN; piece <= B_KING; ++piece) {
    if (GET_BIT(board->bitboards[piece], square)) {
      return static_cast<piece_t>(piece);
    }
  }

  return EMPTY;
}
