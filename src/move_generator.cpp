#include "move_generator.hpp"
#include <cassert>
#include "utils.hpp"


static const std::array<index_t, 8> OFFSETS_N = {0x21, 0x1F, 0x0E, 0xEE,
                                                 0xDF, 0xE1, 0xF2, 0x12};
static const std::array<index_t, 8> OFFSETS_K = {0xFF, 0x0F, 0x10, 0x11,
                                                 0x01, 0xF1, 0xF0, 0xEF};

static const std::array<index_t, 4> DIRECTIONS_ROOK = {0x10, 0xF0, 0x01, 0xFF};
static const std::array<index_t, 4> DIRECTIONS_BISHOP = {0x11, 0x0F, 0xF1,
                                                         0xEF};

std::vector<index_t> generate_b_pawn(index_t index, const board_t* board)
{
  std::vector<index_t> result;
  result.reserve(4);

  const auto b = board->board;
  const auto en_passant = board->game_state.en_passant;

  {
    // Check fo the move in front
    const index_t candidate = index - 0x10;
    if (!(candidate & 0x88) && b[candidate] == piece_t::EMPTY) {
      result.push_back(candidate);
    }
  }

  {
    // Check fo the move 2 in front
    const index_t candidate = index - 0x20;
    bool condition = !(candidate & 0x88) && (index > 0x5F) && (index < 0x68) &&
                     (b[candidate] == EMPTY) && (b[candidate + 0x10] == EMPTY);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack right
    const index_t candidate = index - 0x11;
    bool condition =
        (!(candidate & 0x88) && contains_opponent(candidate, WHITE, board)) ||
        (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack left
    const index_t candidate = index - 0x0F;
    bool condition =
        (!(candidate & 0x88) && contains_opponent(candidate, WHITE, board)) ||
        (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  return result;
}


std::vector<index_t> generate_w_pawn(index_t index, const board_t* board)
{
  std::vector<index_t> result;
  result.reserve(4);

  const auto b = board->board;
  const auto en_passant = board->game_state.en_passant;

  {
    // Check fo the move in front
    const index_t candidate = index + 0x10;
    if (!(candidate & 0x88) && b[candidate] == EMPTY) {
      result.push_back(candidate);
    }
  }

  {
    // Check fo the move 2 in front
    const index_t candidate = index + 0x20;
    bool condition = !(candidate & 0x88) && (index > 0x0F) && (index < 0x20) &&
                     (b[candidate] == EMPTY) && (b[candidate - 0x10] == EMPTY);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack right
    const index_t candidate = index + 0x11;
    bool condition =
        (!(candidate & 0x88) && (contains_opponent(candidate, BLACK, board))) ||
        (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack left
    const index_t candidate = index + 0x0F;
    bool condition =
        (!(candidate & 0x88) && (contains_opponent(candidate, WHITE, board))) ||
        (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  return result;
}


std::vector<index_t> generate_sliding(index_t index,
                                      color_t color,
                                      const std::array<index_t, 4>& directions,
                                      const board_t* board)
{
  std::vector<index_t> result;
  result.reserve(16);  // Worst case

  const auto b = board->board;

  for (const auto I : directions) {
    index_t candidate = index;
    while (true) {
      candidate += I;

      // If we are off board exit
      if (candidate & 0x88) { break; }
      assert(b[candidate] != INVALID);

      // Also break if same color piece block the ray
      const piece_t p = b[candidate];
      if (p == EMPTY) {
        result.push_back(candidate);
      } else {
        const color_t candidate_color = get_piece_color(p);
        // If the are attacking add the position
        if (candidate_color != color) { result.push_back(candidate); }

        break;
      }
    }
  }

  assert(result.size() <= 14);
  return result;
}


std::vector<index_t> generate_jumping(index_t index,
                                      color_t color,
                                      const std::array<index_t, 8>& offsets,
                                      const board_t* board)
{
  std::vector<index_t> result;
  result.reserve(offsets.size() + 4);  // NOTE(max): +4 is for the castling

  const auto b = board->board;

  for (const auto I : offsets) {
    const index_t candidate = index + I;

    if (candidate & 0x88) { continue; }

    const piece_t p = b[candidate];
    assert(p != piece_t::INVALID);

    // Check if attacking his own color
    if (p != piece_t::EMPTY && (get_piece_color(p) == color)) { continue; }


    result.push_back(candidate);
  }

  assert(result.size() <= offsets.size());

  return result;
}


std::vector<index_t> generate_rook(index_t index,
                                   color_t color,
                                   const board_t* board)
{
  const std::vector<index_t> result =
      generate_sliding(index, color, DIRECTIONS_ROOK, board);

  assert(result.size() <= 14);

  return result;
}


std::vector<index_t> generate_bishop(index_t index,
                                     color_t color,
                                     const board_t* board)
{
  const std::vector<index_t> res =
      generate_sliding(index, color, DIRECTIONS_BISHOP, board);

  assert(res.size() <= 16);

  return res;
}


std::vector<index_t> generate_knight(index_t index,
                                     color_t color,
                                     const board_t* board)
{
  const std::vector<index_t> result =
      generate_jumping(index, color, OFFSETS_N, board);

  return result;
}


std::vector<index_t> generate_queen(index_t index,
                                    color_t color,
                                    const board_t* board)
{
  std::vector<index_t> result;
  const std::vector<index_t> h_and_v_moves = generate_rook(index, color, board);
  const std::vector<index_t> diagonal_moves =
      generate_bishop(index, color, board);

  result.insert(result.end(), std::make_move_iterator(h_and_v_moves.begin()),
                std::make_move_iterator(h_and_v_moves.end()));

  result.insert(result.end(), std::make_move_iterator(diagonal_moves.begin()),
                std::make_move_iterator(diagonal_moves.end()));

  return result;
}


std::vector<index_t> generate_king(index_t index,
                                   color_t color,
                                   const board_t* board)
{
  std::vector<index_t> result =
      generate_jumping(index, color, OFFSETS_K, board);

  const auto b = board->board;
  const castling_t castling = board->game_state.castling;

  switch (color) {
    case WHITE:
      if ((castling & WQ) && b[0x01] == EMPTY && b[0x02] == EMPTY &&
          b[0x03] == EMPTY) {
        // Queen side available
        result.push_back(0x02);
      }

      if ((castling & WK) && b[0x05] == EMPTY && b[0x06] == EMPTY) {
        // King side available
        result.push_back(0x06);
      }
      break;
    case BLACK:
      if ((castling & BQ) && b[0x71] == EMPTY && b[0x72] == EMPTY &&
          b[0x73] == EMPTY) {
        // Queen side available
        result.push_back(0x72);
      }

      if ((castling & BK) && b[0x75] == EMPTY && b[0x76] == EMPTY) {
        // King side available
        result.push_back(0x76);
      }
      break;

    default:
      assert(false);
      break;
  }

  return result;
}


std::vector<index_t> generate_pseudo_legal_moves_from_index(
    index_t index,
    const board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(index_to_position(index).file < 8);
  assert(index_to_position(index).rank < 8);

  const piece_t piece = board->board[index];
  assert(piece != INVALID && piece != EMPTY);

  std::vector<index_t> result;

  switch (piece) {
    case piece_t::B_PAWN:
      result = generate_b_pawn(index, board);
      break;
    case piece_t::W_PAWN:
      result = generate_w_pawn(index, board);
      break;

    case piece_t::B_ROOK:
      result = generate_rook(index, BLACK, board);
      break;
    case piece_t::W_ROOK:
      result = generate_rook(index, WHITE, board);
      break;

    case piece_t::B_KNIGHT:
      result = generate_knight(index, BLACK, board);
      break;
    case piece_t::W_KNIGHT:
      result = generate_knight(index, WHITE, board);
      break;

    case piece_t::B_BISHOP:
      result = generate_bishop(index, BLACK, board);
      break;
    case piece_t::W_BISHOP:
      result = generate_bishop(index, WHITE, board);
      break;

    case piece_t::B_QUEEN:
      result = generate_queen(index, BLACK, board);
      break;
    case piece_t::W_QUEEN:
      result = generate_queen(index, WHITE, board);
      break;

    case piece_t::B_KING:
      result = generate_king(index, BLACK, board);
      break;
    case piece_t::W_KING:
      result = generate_king(index, WHITE, board);
      break;

    default:
      assert(false);
      break;
  }

  return result;
}


std::vector<move_t> generate_attack_vector(color_t target_color, board_t* board)
{
  std::vector<move_t> attacks;
  attacks.reserve(64);  // worst case

  // Removing the king from the board.
  const piece_t king_to_remove = (target_color == WHITE) ? B_KING : W_KING;

  // Store the king pos
  const index_t king_index =
      get_king_index(get_piece_color(king_to_remove), board);

  // Unset the king
  (void)remove_piece(king_index, board);

  // Generate the moves
  for (index_t i = 0; i < BOARD_SIZE; ++i) {
    const piece_t p = board->board[i];

    // Iterate over opposite color pieces
    if (p != INVALID && p != EMPTY && target_color == get_piece_color(p)) {
      const auto moves_for_index =
          generate_pseudo_legal_moves_from_index(i, board);


      for (const index_t move_index : moves_for_index) {
        bool found = false;
        const move_t move = {i, move_index};

        for (const move_t& attack : attacks) {
          if (attack == move) { found = true; }
          break;
        }

        if (!found) { attacks.push_back(move); }
      }
    }
  }

  // Reset the king to the board
  put_piece(king_index, king_to_remove, board);

  return attacks;
}


std::vector<move_t> generate_legal_moves(const board_t* board)
{
  assert(board != nullptr);

  std::vector<move_t> result;
  result.reserve(100);

  /*****************************************************************************
   * Generate enemy attacks vector
   ****************************************************************************/
  board_t tmp_board = *board;
  const color_t attack_color = !board->game_state.active_color;
  const std::vector<move_t> attack_vector =
      generate_attack_vector(attack_color, &tmp_board);

  /*****************************************************************************
   * Calculate if under check
   ****************************************************************************/


  /*****************************************************************************
   * Generate moves
   ****************************************************************************/
  for (index_t i = 0; i < BOARD_SIZE; ++i) {
    const piece_t P = board->board[i];

    if (P != INVALID && P != EMPTY &&
        board->game_state.active_color == get_piece_color(P)) {
      // generate moves
      const auto moves_index = generate_pseudo_legal_moves_from_index(i, board);

      switch (P) {
        case B_KING:
        case W_KING: {
          // Get king pseudo legal moves


          // Remove the moves that put the king under attack
          for (const index_t move_index : moves_index) {
            const move_t M = {i, move_index};
            bool found = false;

            for (const auto& A : attack_vector) {
              if (M == A) {
                found = true;
                break;
              }
            }

            if (!found) { result.push_back(M); }
          }
        } break;

        default: {
          // Generate others pieces moves
          for (const index_t move_index : moves_index) {
            const move_t move = {i, move_index};
            result.push_back(move);
          }
        }

        break;
      }
    }
  }

  return result;
}
