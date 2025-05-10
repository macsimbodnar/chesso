#include "bitboard.hpp"
#include <bit>
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


bb_t get_bishop_attacks(const bb_tables_t* data, index_t index, bb_t occupancy)
{
  assert(data != nullptr);

  occupancy &= data->bishop_masks[index];
  occupancy *= bishop_magic_numbers[index];
  occupancy >>= 64 - bishop_relevant_bits_count[index];
  return data->bishop_attacks[index][occupancy];
}


bb_t get_rook_attacks(const bb_tables_t* data, index_t index, bb_t occupancy)
{
  assert(data != nullptr);

  occupancy &= data->rook_masks[index];
  occupancy *= rook_magic_numbers[index];
  occupancy >>= 64 - rook_relevant_bits_count[index];
  return data->rook_attacks[index][occupancy];
}


bb_t get_queen_attacks(const bb_tables_t* data, index_t index, bb_t occupancy)
{
  bb_t queen_attacks = get_bishop_attacks(data, index, occupancy);
  queen_attacks |= get_rook_attacks(data, index, occupancy);
  return queen_attacks;
}


bool is_attacked(const bb_tables_t* data,
                 const board_t* board,
                 index_t index,
                 color_t color)
{
  assert(board != nullptr);

  {  // Handle pawn attacks
    if ((color == WHITE) &&
        (data->pawn_attacks[BLACK][index] & board->bitboards[W_PAWN])) {
      return true;
    }

    if ((color == BLACK) &&
        (data->pawn_attacks[WHITE][index] & board->bitboards[B_PAWN])) {
      return true;
      ;
    }
  }

  if (data->knight_attacks[index] &
      ((color == WHITE) ? board->bitboards[W_KNIGHT]
                        : board->bitboards[B_KNIGHT])) {
    return true;
  }


  if (get_bishop_attacks(data, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_BISHOP]
                        : board->bitboards[B_BISHOP])) {
    return true;
  }

  if (get_rook_attacks(data, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_ROOK]
                        : board->bitboards[B_ROOK])) {
    return true;
  }

  // attacked by bishops
  if (get_queen_attacks(data, index, board->occupancies[BOTH]) &
      ((color == WHITE) ? board->bitboards[W_QUEEN]
                        : board->bitboards[B_QUEEN])) {
    return true;
  }

  // attacked by kings
  if (data->king_attacks[index] &
      ((color == WHITE) ? board->bitboards[W_KING]
                        : board->bitboards[B_KING])) {
    return true;
  }

  return false;
}


size_t generate_moves(const bb_tables_t* data,
                      const board_t* board,
                      move_t moves[])
{
  assert(data != nullptr);
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
                  NEW_MOVE(from, to, piece, W_QUEEN, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, W_ROOK, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, W_BISHOP, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, W_KNIGHT, 0, 0, 0, 0);
            }

            else {
              moves[move_count++] = NEW_MOVE(from, to, piece, 0, 0, 0, 0, 0);

              if ((from >= a2 && from <= h2) &&
                  !GET_BIT(board->occupancies[BOTH], to - 8))
                // Double push
                moves[move_count++] =
                    NEW_MOVE(from, to - 8, piece, 0, 0, 1, 0, 0);
            }
          }

          // Pawn captures
          attacks = data->pawn_attacks[color][from] & board->occupancies[BLACK];

          while (attacks) {
            // init target square
            to = get_lsb_index(attacks);

            // Attack & promotion
            if (from >= a7 && from <= h7) {
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, W_QUEEN, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, W_ROOK, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, W_BISHOP, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, W_KNIGHT, 1, 0, 0, 0);
            }

            else {
              moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
            }

            // moveto the next attack
            POP_BIT(attacks, to);
          }

          // En-passant
          if (board->en_passant != INVALID_INDEX) {
            const bb_t enp_attacks =
                data->pawn_attacks[color][from] & (BB_1 << board->en_passant);

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
            if (!is_attacked(data, board, e1, BLACK) &&
                !is_attacked(data, board, f1, BLACK)) {
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
            if (!is_attacked(data, board, e1, BLACK) &&
                !is_attacked(data, board, d1, BLACK)) {
              moves[move_count++] = NEW_MOVE(e1, c1, piece, 0, 0, 0, 0, 1);
            }
          }
        }
      }
    }

    else {
      if (piece == B_PAWN) {
        while (bboard) {
          from = get_lsb_index(bboard);
          to = from + 8;

          // Quiet pawn moves
          if (!(to > h1) && !GET_BIT(board->occupancies[BOTH], to)) {
            // Promotion
            if (from >= a2 && from <= h2) {
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_QUEEN, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_ROOK, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_BISHOP, 0, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_KNIGHT, 0, 0, 0, 0);
            }

            else {
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
          attacks = data->pawn_attacks[color][from] & board->occupancies[WHITE];

          // generate pawn captures
          while (attacks) {
            to = get_lsb_index(attacks);

            // Promotion
            if (from >= a2 && from <= h2) {
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_QUEEN, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_ROOK, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_BISHOP, 1, 0, 0, 0);
              moves[move_count++] =
                  NEW_MOVE(from, to, piece, B_KNIGHT, 1, 0, 0, 0);
            } else {
              moves[move_count++] = NEW_MOVE(from, to, piece, 0, 1, 0, 0, 0);
            }

            // Move to next attack
            POP_BIT(attacks, to);
          }

          // En-passant
          if (board->en_passant != INVALID_INDEX) {
            const bb_t enp_attacks =
                data->pawn_attacks[color][from] & (BB_1 << board->en_passant);

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
            if (!is_attacked(data, board, e8, WHITE) &&
                !is_attacked(data, board, f8, WHITE)) {
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
            if (!is_attacked(data, board, e8, WHITE) &&
                !is_attacked(data, board, d8, WHITE))
              moves[move_count++] = NEW_MOVE(e8, c8, piece, 0, 0, 0, 0, 1);
          }
        }
      }
    }

    // Knight
    if ((color == WHITE) ? piece == W_KNIGHT : piece == B_KNIGHT) {
      while (bboard) {
        from = get_lsb_index(bboard);

        attacks = data->knight_attacks[from] &
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

        attacks = get_bishop_attacks(data, from, board->occupancies[BOTH]) &
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
        attacks = get_rook_attacks(data, from, board->occupancies[BOTH]) &
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

        attacks = get_queen_attacks(data, from, board->occupancies[BOTH]) &
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

        attacks = data->king_attacks[from] &
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


/******************************************************************************
 *                               UTIL FUNCTIONS
 * NOTE: Does not need to be optimized
 ******************************************************************************/
bool load_FEN(const std::string& FEN, board_t* board)
{
  assert(board != nullptr);
  cleanup_board(board);

  // Start parsing
  auto sections = split_string(FEN);

  if (sections.size() != 6) {
    LOG_E << "Bad FEN string: " << FEN << END_E;
    return false;
  }

  /*****************************************************************************
   * 0. Piece placement data
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

  // Update Zobrist keys
  // board->zobrist_key = init_zobrist_key(state, board);
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


// std::string move_to_algebraic(const move_t* move,
//                               const move_t moves[],
//                               size_t moves_size,
//                               board_t* board)
// {
//   assert(move != nullptr);
//   assert(board != nullptr);
//   assert(move->piece != INVALID);
//   assert(move->piece != EMPTY);
//   assert(moves != nullptr);

//   // Not using the piece_to_char function because the piece moved in
//   // always upper case
//   static const std::unordered_map<piece_t, char> piece_to_char_map = {
//       {B_PAWN, 'P'},   {B_KNIGHT, 'N'}, {B_BISHOP, 'B'}, {B_ROOK, 'R'},
//       {B_QUEEN, 'Q'},  {B_KING, 'K'},   {W_PAWN, 'P'},   {W_KNIGHT, 'N'},
//       {W_BISHOP, 'B'}, {W_ROOK, 'R'},   {W_QUEEN, 'Q'},  {W_KING, 'K'},
//       {INVALID, '*'},  {EMPTY, ' '}};

//   static const char file_to_char_map[8] = {'a', 'b', 'c', 'd',
//                                            'e', 'f', 'g', 'h'};
//   static const char rank_to_char_map[8] = {'1', '2', '3', '4',
//                                            '5', '6', '7', '8'};

//   std::string notation;

//   // Handle castling
//   if (move->castling_move) {
//     if (move->to == 0x06 || move->to == 0x76)
//       return "O-O";  // King-side castling
//     if (move->to == 0x02 || move->to == 0x72)
//       return "O-O-O";  // Queen-side castling
//   }

//   if (move->piece != W_PAWN && move->piece != B_PAWN) {
//     notation += piece_to_char_map.at(move->piece);  // Non-pawn pieces

//     // If ambiguous move the add the from file
//     index_t ambiguous_moves[MAX_MOVES];
//     const size_t ambiguous_moves_count =
//         get_ambiguous_move(move, moves, moves_size, ambiguous_moves);

//     if (ambiguous_moves_count > 0) {
//       const position_t move_from_pos = index_to_position(move->from);
//       bool is_file_unique = true;
//       bool is_rank_unique = true;

//       // Check if file or rank are unique for the move->from
//       for (size_t i = 0; i < ambiguous_moves_count; ++i) {
//         const index_t index = ambiguous_moves[i];
//         const position_t i_pos = index_to_position(moves[index].from);

//         if (move_from_pos.file == i_pos.file) { is_file_unique = false; }

//         if (move_from_pos.rank == i_pos.rank) { is_rank_unique = false; }

//         // Exit from the loop in case both are non unique. No make sense
//         // to search for more
//         if (!is_file_unique && !is_rank_unique) { break; }
//       }

//       if (is_file_unique) {
//         // Check if file unique
//         notation += file_to_char_map[move_from_pos.file];
//       } else if (is_rank_unique) {
//         // Check if rank unique
//         notation += rank_to_char_map[move_from_pos.rank];
//       } else {
//         // In case none is unique use both
//         notation += file_to_char_map[move_from_pos.file];
//         notation += rank_to_char_map[move_from_pos.rank];
//       }
//     }
//   }

//   // Capture notation
//   if (move->captured != INVALID && move->piece != W_PAWN &&
//       move->piece != B_PAWN) {
//     notation += 'x';
//   }

//   // Destination square
//   notation += index_to_string_coordinates(move->to);

//   // Pawn captures (ex: exd5)
//   if ((move->piece == W_PAWN || move->piece == B_PAWN) &&
//       move->captured != INVALID) {
//     notation = index_to_string_coordinates(move->from)[0] + std::string("x") +
//                index_to_string_coordinates(move->to);
//   }

//   // Pawn promotion
//   if (move->promoted_to != TO_NONE) {
//     notation += "=";
//     switch (move->promoted_to) {
//       case TO_QUEEN:
//         notation += 'Q';
//         break;
//       case TO_ROOK:
//         notation += 'R';
//         break;
//       case TO_BISHOP:
//         notation += 'B';
//         break;
//       case TO_KNIGHT:
//         notation += 'N';
//         break;
//       default:
//         break;
//     }
//   }

//   // Handle check
//   const index_t opponent_king_index =
//       get_king_index(!board->active_color, board);

//   const bool move_happened = make_move(move, board, state);
//   assert(move_happened);
//   (void)move_happened;  // Supress the unused var log

//   // Generate moves for my color but after the current move is done
//   move_t pseudo_legal_moves[30];
//   const size_t pseudo_legal_moves_count =
//       generate_pseudo_legal_moves_from_index(move->to, board,
//                                              pseudo_legal_moves);

//   assert(pseudo_legal_moves_count <=
//          sizeof(pseudo_legal_moves) / sizeof(pseudo_legal_moves[0]));

//   // Check if one of this moves put under check the opponent king

//   for (size_t pseudo_index = 0; pseudo_index < pseudo_legal_moves_count;
//        ++pseudo_index) {
//     const move_t& pseudo_move = pseudo_legal_moves[pseudo_index];
//     if (pseudo_move.to == opponent_king_index) {
//       // Now let's check if this is check mate

//       // Generate legal moves after the make move to see if any available.
//       move_t moves[MAX_MOVES];
//       const size_t moves_count = generate_legal_moves(board, state, moves);

//       if (moves_count == 0) {
//         // Check mate
//         notation += '#';
//         break;
//       } else {
//         // Append a '+' to the notation
//         notation += '+';
//         break;
//       }
//     }
//   }

//   const bool move_un_happened = unmake_move(board, state);
//   assert(move_un_happened);
//   (void)move_un_happened;

//   return notation;
// }


// move_t algebraic_to_move(std::string notation, board_t* board)
// {
//   assert(board != nullptr);

//   static const std::unordered_map<char, uint8_t> char_to_file_map = {
//       {'a', 0}, {'b', 1}, {'c', 2}, {'d', 3},
//       {'e', 4}, {'f', 5}, {'g', 6}, {'h', 7}};
//   static const std::unordered_map<char, uint8_t> char_to_rank_map = {
//       {'1', 0}, {'2', 1}, {'3', 2}, {'4', 3},
//       {'5', 4}, {'6', 5}, {'7', 6}, {'8', 7}};

//   const std::string original_notation = notation;

//   move_t result;
//   const color_t color = board->active_color;

//   // Make a working copy of the move string.
//   bool is_capture = false;

//   // TODO: Use this
//   // bool is_check = false;
//   // bool is_mate = false;
//   // if (notation.back() == '+') { is_check = true; }
//   // if (notation.back() == '#') { is_mate = true; }

//   // Remove any trailing check ('+') or checkmate ('#') symbols.
//   while (!notation.empty() &&
//          (notation.back() == '+' || notation.back() == '#')) {
//     notation.pop_back();
//   }

//   // Parse castling
//   if (notation == "O-O-O") {
//     result.castling_move = true;
//     switch (color) {
//       case BLACK:
//         result.from = 0x74;
//         result.to = 0x72;
//         result.piece = B_KING;
//         break;
//       case WHITE:
//         result.from = 0x04;
//         result.to = 0x02;
//         result.piece = W_KING;
//         break;
//       default:
//         assert(false);
//         break;
//     }

//     return result;
//   }

//   if (notation == "O-O") {
//     result.castling_move = true;

//     switch (color) {
//       case BLACK:
//         result.from = 0x74;
//         result.to = 0x76;
//         result.piece = B_KING;
//         break;
//       case WHITE:
//         result.from = 0x04;
//         result.to = 0x06;
//         result.piece = W_KING;
//         break;
//       default:
//         assert(false);
//         break;
//     }

//     return result;
//   }


//   // Parse non-castling moves
//   size_t pos = 0;
//   piece_t moving_piece;

//   // If the move begins with a piece letter (K, Q, R, B, N), then use it.
//   if (pos < notation.size() && std::isupper(notation[pos])) {
//     char piece_char = notation[pos];
//     switch (piece_char) {
//       case 'K':
//         moving_piece = (color == WHITE) ? W_KING : B_KING;
//         break;
//       case 'Q':
//         moving_piece = (color == WHITE) ? W_QUEEN : B_QUEEN;
//         break;
//       case 'R':
//         moving_piece = (color == WHITE) ? W_ROOK : B_ROOK;
//         break;
//       case 'B':
//         moving_piece = (color == WHITE) ? W_BISHOP : B_BISHOP;
//         break;
//       case 'N':
//         moving_piece = (color == WHITE) ? W_KNIGHT : B_KNIGHT;
//         break;
//       default:
//         moving_piece = INVALID;
//         break;
//     }
//     ++pos;
//   } else {
//     // If no piece letter then it's a pawn move.
//     moving_piece = (color == WHITE) ? W_PAWN : B_PAWN;
//   }
//   result.piece = moving_piece;

//   // We now extract any disambiguation info.
//   // This may be a file letter, a rank digit, or both.
//   std::optional<char> disambiguous_file;
//   std::optional<char> disambiguous_rank;

//   // Look ahead for an 'x' (capture marker) or destination square.
//   // We will also later remove any 'x' from the string.
//   size_t temp_pos = pos;
//   while (temp_pos < notation.size() && notation[temp_pos] != 'x' &&
//          !(notation[temp_pos] >= 'a' && notation[temp_pos] <= 'h' &&
//            (temp_pos + 1 < notation.size() && notation[temp_pos + 1] >= '1' &&
//             notation[temp_pos + 1] <= '8'))) {
//     // Assume any character here is part of disambiguation.
//     char d = notation[temp_pos];
//     if (d >= 'a' && d <= 'h')
//       disambiguous_file = d;
//     else if (d >= '1' && d <= '8')
//       disambiguous_rank = d;
//     ++temp_pos;
//   }

//   // Remove capture marker(s) from the string.
//   std::string cleaned;
//   for (char ch : notation.substr(pos)) {
//     if (ch != 'x') {
//       cleaned.push_back(ch);
//     } else {
//       is_capture = true;
//     }
//   }

//   // Look for promotion: if there is an '=' then the following char is the
//   // promotion piece.
//   promotion_t promo = TO_NONE;
//   size_t promo_pos = cleaned.find('=');
//   if (promo_pos != std::string::npos && promo_pos + 1 < cleaned.size()) {
//     char promo_char = cleaned[promo_pos + 1];
//     switch (promo_char) {
//       case 'Q':
//         promo = TO_QUEEN;
//         break;
//       case 'R':
//         promo = TO_ROOK;
//         break;
//       case 'B':
//         promo = TO_BISHOP;
//         break;
//       case 'N':
//         promo = TO_KNIGHT;
//         break;
//       default:
//         promo = TO_NONE;
//         break;
//     }
//     cleaned = cleaned.substr(0, promo_pos);
//   }
//   result.promoted_to = promo;

//   // The destination square is the last two characters of the cleaned
//   // string.
//   if (cleaned.size() < 2) {
//     // Error: not enough characters to form a square.
//     throw algebraic_exception("Wrong formatting. Invalid Algebraic notation: " +
//                               original_notation);
//   }

//   std::string dest_square = cleaned.substr(cleaned.size() - 2, 2);
//   index_t to_index = string_coordinates_to_index(dest_square);
//   result.to = to_index;

//   if (result.to >= INVALID_BOARD_INDEX) {
//     throw algebraic_exception(
//         "Invalid destination square. Invalid Algebraic notation: " +
//         original_notation);
//   }

//   if (is_capture) {
//     // Attempt to use the destination as capture piece
//     result.captured = board->board[result.to];

//     // In case of en-passant override the capture
//     if (board->en_passant != INVALID_BOARD_INDEX) {
//       if (color == WHITE) {
//         if (board->board[result.to] == EMPTY &&
//             board->board[result.to - 0x10] == B_PAWN) {
//           result.captured = B_PAWN;
//         }
//       } else {
//         if (board->board[result.to] == EMPTY &&
//             board->board[result.to + 0x10] == W_PAWN) {
//           result.captured = W_PAWN;
//         }
//       }
//     }

//     if (result.captured == INVALID || result.captured == EMPTY) {
//       throw algebraic_exception(
//           "No capture found on the board. Invalid Algebraic notation: " +
//           original_notation);
//     }
//   }

//   // Any remaining characters between our initial pos and the destination
//   // have been interpreted as disambiguation.
//   // (In many SAN moves the disambiguation is omitted if unneeded.)
//   // Here we already extracted potential disambiguation earlier.

//   // Generate legal moves and search the compatible one
//   move_t moves[MAX_MOVES];
//   const size_t moves_count = generate_legal_moves(board, state, moves);

//   bool found = false;
//   for (size_t i = 0; i < moves_count; ++i) {
//     const move_t& legal_move = moves[i];

//     if (legal_move.to == result.to && legal_move.piece == result.piece &&
//         legal_move.promoted_to == result.promoted_to &&
//         legal_move.captured == result.captured &&
//         legal_move.castling_move == result.castling_move) {
//       // Check for disambiguous
//       const position_t legal_from_pos = index_to_position(legal_move.from);

//       if (disambiguous_file.has_value()) {
//         const uint8_t file = char_to_file_map.at(disambiguous_file.value());

//         if (file != legal_from_pos.file) { continue; }
//       }

//       if (disambiguous_rank.has_value()) {
//         const uint8_t rank = char_to_rank_map.at(disambiguous_rank.value());

//         if (rank != legal_from_pos.rank) { continue; }
//       }

//       // We found the move
//       found = true;
//       result = legal_move;
//       break;
//     }
//   }

//   if (!found) {
//     throw algebraic_exception(
//         "No legal move found. Invalid Algebraic notation: " +
//         original_notation);
//   }

//   return result;
// }


/******************************************************************************
 *                      INITIALIZATION FUNCTIONS
 * NOTE: Does not need to be optimized, they are called once at the start
 ******************************************************************************/
bb_t set_occupancy(index_t index, int mask_bit_count, bb_t attack_mask)
{
  bb_t occupancy = BB_0;

  for (int count = 0; count < mask_bit_count; ++count) {
    const index_t square = get_lsb_index(attack_mask);
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


void initialize_const_data(bb_tables_t* data)
{
  assert(data != nullptr);

  for (index_t i = 0; i < 64; ++i) {
    // Init pawn attacks
    data->pawn_attacks[WHITE][i] = precompute_pawn_attacks(WHITE, i);
    data->pawn_attacks[BLACK][i] = precompute_pawn_attacks(BLACK, i);

    // Init knight attacks
    data->knight_attacks[i] = precompute_knight_attacks(i);

    // Init king attacks
    data->king_attacks[i] = precompute_king_attacks(i);

    // Init masks for bishop and rooks
    data->bishop_masks[i] = precompute_bishop_attack_masks(i);
    data->rook_masks[i] = precompute_rook_attack_masks(i);

    {  // Init bishop and rook attack vector
      const bb_t bishop_attack_mask = data->bishop_masks[i];
      const bb_t rook_attack_mask = data->rook_masks[i];

      const int bishop_num_relevant_bits = bishop_relevant_bits_count[i];
      const int rook_num_relevant_bits = rook_relevant_bits_count[i];

      const int bishop_occupancy_indicies = (1 << bishop_num_relevant_bits);
      const int rook_occupancy_indicies = (1 << rook_num_relevant_bits);

      for (int index = 0; index < bishop_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, bishop_num_relevant_bits, bishop_attack_mask);

        const int magic_index = (occupancy * bishop_magic_numbers[i]) >>
                                (64 - bishop_num_relevant_bits);

        data->bishop_attacks[i][magic_index] =
            precompute_bishop_attacks(i, occupancy);
      }

      for (int index = 0; index < rook_occupancy_indicies; ++index) {
        const bb_t occupancy =
            set_occupancy(index, rook_num_relevant_bits, rook_attack_mask);

        const int magic_index = (occupancy * rook_magic_numbers[i]) >>
                                (64 - rook_num_relevant_bits);

        data->rook_attacks[i][magic_index] =
            precompute_rook_attacks(i, occupancy);
      }
    }
  }

  LOG_I << "Bitboard const data initialized" << END_I;
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


void cleanup_board(board_t* board)
{
  assert(board != nullptr);
  memset(board->bitboards, 0, sizeof(board->bitboards));

  board->active_color = WHITE;
  board->castling = WQ | WK | BQ | BK;
  board->halfmove_clock = 0;
  board->en_passant = INVALID_INDEX;
  board->fullmove_counter = 1;
  board->zobrist_key = 0;
}
