#pragma once
#include <array>
#include "utils.hpp"


static const std::array<uint8_t, 8> offsets_n = {0x21, 0x1F, 0x0E, 0xEE,
                                                 0xDF, 0xE1, 0xF2, 0x12};
static const std::array<uint8_t, 8> offsets_k = {0xFF, 0x0F, 0x10, 0x11,
                                                 0x01, 0xF1, 0xF0, 0xEF};

static const std::array<uint8_t, 4> directions_rook = {0x10, 0xF0, 0x01, 0xFF};
static const std::array<uint8_t, 4> directions_bishop = {0x11, 0x0F, 0xF1,
                                                         0xEF};

inline std::vector<uint8_t> generate_b_pawn(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index,
    const uint8_t en_passant)
{
  std::vector<uint8_t> result;
  result.reserve(4);

  {
    // Check fo the move in front
    const uint8_t candidate = index - 0x10;
    if (!(candidate & 0x88) && board[candidate] == piece_t::EMPTY) {
      result.push_back(candidate);
    }
  }

  {
    // Check fo the move 2 in front
    const uint8_t candidate = index - 0x20;
    bool condition = !(candidate & 0x88) && (index > 0x5F) && (index < 0x68) &&
                     (board[candidate] == piece_t::EMPTY) &&
                     (board[candidate + 0x10] == piece_t::EMPTY);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack right
    const uint8_t candidate = index - 0x11;
    bool condition = (!(candidate & 0x88) &&
                      (U32(board[candidate]) & U32(color_t::WHITE))) ||
                     (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack left
    const uint8_t candidate = index - 0x0F;
    bool condition = (!(candidate & 0x88) &&
                      (U32(board[candidate]) & U32(color_t::WHITE))) ||
                     (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  return result;
}


inline std::vector<uint8_t> generate_w_pawn(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index,
    const uint8_t en_passant)
{
  std::vector<uint8_t> result;
  result.reserve(4);

  {
    // Check fo the move in front
    const uint8_t candidate = index + 0x10;
    if (!(candidate & 0x88) && board[candidate] == piece_t::EMPTY) {
      result.push_back(candidate);
    }
  }

  {
    // Check fo the move 2 in front
    const uint8_t candidate = index + 0x20;
    bool condition = !(candidate & 0x88) && (index > 0x0F) && (index < 0x20) &&
                     (board[candidate] == piece_t::EMPTY) &&
                     (board[candidate - 0x10] == piece_t::EMPTY);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack right
    const uint8_t candidate = index + 0x11;
    bool condition = (!(candidate & 0x88) &&
                      (U32(board[candidate]) & U32(color_t::BLACK))) ||
                     (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack left
    const uint8_t candidate = index + 0x0F;
    bool condition = (!(candidate & 0x88) &&
                      (U32(board[candidate]) & U32(color_t::BLACK))) ||
                     (candidate == en_passant);

    if (condition) { result.push_back(candidate); }
  }

  return result;
}


inline std::vector<uint8_t> generate_sliding(
    const std::array<uint8_t, 4>& directions,
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const color_t color,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  result.reserve(16);  // Worst case

  for (const auto I : directions) {
    uint8_t candidate = index;
    while (true) {
      candidate += I;

      // If we are off board exit
      if (candidate & 0x88) { break; }
      assert(board[candidate] != piece_t::INVALID);

      // Also break if same color piece block the ray
      piece_t p = board[candidate];
      if (p == piece_t::EMPTY) {
        result.push_back(candidate);
      } else {
        const color_t candidate_color = get_color(p);
        // If the are attacking add the position
        if (candidate_color != color) { result.push_back(candidate); }

        break;
      }
    }
  }

  assert(result.size() <= 14);
  return result;
}


inline std::vector<uint8_t> generate_jumping(
    const std::array<uint8_t, 8>& offsets,
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const color_t color,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  result.reserve(offsets_k.size() + 4);  // NOTE(max): +4 is for the castling

  for (const auto I : offsets) {
    const uint8_t candidate = index + I;

    if (candidate & 0x88) { continue; }

    const piece_t p = board[candidate];
    assert(p != piece_t::INVALID);

    // Check if attacking his own color
    if (p != piece_t::EMPTY && (get_color(p) == color)) { continue; }


    result.push_back(candidate);
  }

  assert(result.size() <= offsets_k.size());

  return result;
}


inline std::vector<uint8_t> generate_rook(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const color_t color,
    const uint8_t index)
{
  std::vector<uint8_t> result =
      generate_sliding(directions_rook, board, color, index);

  assert(result.size() <= 14);

  return result;
}


inline std::vector<uint8_t> generate_bishop(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const color_t color,
    const uint8_t index)
{
  std::vector<uint8_t> res =
      generate_sliding(directions_bishop, board, color, index);

  assert(res.size() <= 16);

  return res;
}


inline std::vector<uint8_t> generate_knight(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const color_t c,
    const uint8_t index)
{
  std::vector<uint8_t> result = generate_jumping(offsets_n, board, c, index);

  return result;
}


inline std::vector<uint8_t> generate_king(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index,
    const color_t color,
    const uint8_t castling)
{
  std::vector<uint8_t> result =
      generate_jumping(offsets_k, board, color, index);

  switch (color) {
    case color_t::WHITE:
      if ((castling & WQ) && board[0x01] == piece_t::EMPTY &&
          board[0x02] == piece_t::EMPTY && board[0x03] == piece_t::EMPTY) {
        // Queen side available
        result.push_back(0x02);
      }

      if ((castling & WK) && board[0x05] == piece_t::EMPTY &&
          board[0x06] == piece_t::EMPTY) {
        // King side available
        result.push_back(0x06);
      }
      break;
    case color_t::BLACK:
      if ((castling & BQ) && board[0x71] == piece_t::EMPTY &&
          board[0x72] == piece_t::EMPTY && board[0x73] == piece_t::EMPTY) {
        // Queen side available
        result.push_back(0x72);
      }

      if ((castling & BK) && board[0x75] == piece_t::EMPTY &&
          board[0x76] == piece_t::EMPTY) {
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


inline std::vector<uint8_t> generate_queen(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const color_t color,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  std::vector<uint8_t> h_and_v_moves = generate_rook(board, color, index);
  std::vector<uint8_t> diagonal_moves = generate_bishop(board, color, index);

  result.insert(result.end(), std::make_move_iterator(h_and_v_moves.begin()),
                std::make_move_iterator(h_and_v_moves.end()));

  result.insert(result.end(), std::make_move_iterator(diagonal_moves.begin()),
                std::make_move_iterator(diagonal_moves.end()));

  return result;
}


inline std::vector<uint8_t> generate_pseudo_legal_moves(
    const board_state_t& board_state,
    const uint8_t index)
{
  const std::array<piece_t, BOARD_ARRAY_SIZE>& board = board_state.board;
  const piece_t piece = board[index];

  assert(piece != piece_t::INVALID);
  assert(piece != piece_t::EMPTY);

  std::vector<uint8_t> result;

  switch (piece) {
    case piece_t::B_PAWN:
      result =
          generate_b_pawn(board, index, board_state.en_passant_target_square);
      break;
    case piece_t::W_PAWN:
      result =
          generate_w_pawn(board, index, board_state.en_passant_target_square);
      break;

    case piece_t::B_ROOK:
      result = generate_rook(board, color_t::BLACK, index);
      break;
    case piece_t::W_ROOK:
      result = generate_rook(board, color_t::WHITE, index);
      break;

    case piece_t::B_KNIGHT:
      result = generate_knight(board, color_t::BLACK, index);
      break;
    case piece_t::W_KNIGHT:
      result = generate_knight(board, color_t::WHITE, index);
      break;

    case piece_t::B_BISHOP:
      result = generate_bishop(board, color_t::BLACK, index);
      break;
    case piece_t::W_BISHOP:
      result = generate_bishop(board, color_t::WHITE, index);
      break;

    case piece_t::B_QUEEN:
      result = generate_queen(board, color_t::BLACK, index);
      break;
    case piece_t::W_QUEEN:
      result = generate_queen(board, color_t::WHITE, index);
      break;

    case piece_t::B_KING:
      result = generate_king(board, index, color_t::BLACK,
                             board_state.available_castling);
      break;
    case piece_t::W_KING:
      result = generate_king(board, index, color_t::WHITE,
                             board_state.available_castling);
      break;

    default:
      break;
  }

  // Refine moves

  return result;
}


inline std::vector<uint8_t> generate_attack_vector(board_state_t& board_state,
                                                   const color_t target_color)
{
  std::vector<uint8_t> attacks;
  attacks.reserve(64);  // worst case

  // Removing the king from the board.
  const piece_t king_to_remove =
      (target_color == color_t::WHITE) ? piece_t::B_KING : piece_t::W_KING;

  // Store the king pos
  const uint8_t king_pos = board_state.get_piece_index(king_to_remove);
  const piece_t king = king_to_remove;

  // Unset the king
  board_state.board[king_pos] = piece_t::EMPTY;

  // Check we set correct king pos
  assert(!(king_pos & 0x88));
  assert(king == king_to_remove);
  assert(get_color(king) != target_color);

  // Generate the moves
  for (uint8_t i = 0; i < board_state.board.size(); ++i) {
    const piece_t p = board_state.board[i];

    // Iterate over opposite color pieces
    if (p != piece_t::INVALID && p != piece_t::EMPTY &&
        target_color == get_color(p)) {
      const auto moves = generate_pseudo_legal_moves(board_state, i);

      for (const auto& M : moves) {
        bool found = false;
        for (const auto A : attacks) {
          if (A == M) {
            found = true;
            break;
          }
        }

        if (!found) { attacks.push_back(M); }
      }
    }
  }

  // Reset the king to the board
  board_state.board[king_pos] = king;

  return attacks;
}


inline std::vector<move_t> king_checks(const board_state_t& board_state)
{
  std::vector<move_t> result;
  result.reserve(2);  // We can have at maximum double check

  piece_t king = piece_t::INVALID;
  piece_t knight = piece_t::INVALID;

  switch (board_state.active_color) {
    case color_t::WHITE:
      king = piece_t::W_KING;
      knight = piece_t::B_KNIGHT;
      break;
    case color_t::BLACK:
      king = piece_t::B_KING;
      knight = piece_t::W_KNIGHT;
      break;

    default:
      assert(false);
      break;
  }

  assert(king != piece_t::INVALID);
  assert(knight != piece_t::INVALID);

  const uint8_t king_pos = board_state.get_piece_index(king);

  // Search for knight
  for (const auto I : offsets_n) {
    const uint8_t candidate_pos = king_pos + I;
    const piece_t candidate_piece = board_state.board[candidate_pos];

    if (candidate_piece == knight) {
      const move_t move = {candidate_pos, king_pos};
      result.push_back(move);
    }
  }

  // TODO(max): To finish

  assert(result.size() < 3);
  return result;
}


inline std::vector<move_t> generate_legal_moves(
    const board_state_t& board_state)
{
  std::vector<move_t> result;
  result.reserve(100);

  /*****************************************************************************
   * Generate enemy attacks vector
   ****************************************************************************/
  auto tmp_board_state = board_state;
  const color_t attack_color = !board_state.active_color;
  const std::vector<uint8_t> attack_vector =
      generate_attack_vector(tmp_board_state, attack_color);

  /*****************************************************************************
   * Calculate if under check
   ****************************************************************************/


  /*****************************************************************************
   * Generate moves
   ****************************************************************************/
  for (uint8_t i = 0; i < board_state.board.size(); ++i) {
    const piece_t P = board_state.board[i];

    if (P != piece_t::INVALID && P != piece_t::EMPTY &&
        board_state.active_color == get_color(P)) {
      // generate moves
      const auto moves = generate_pseudo_legal_moves(board_state, i);

      switch (P) {
        case piece_t::B_KING:
        case piece_t::W_KING: {
          // Get king pseudo legal moves


          // Remove the moves that put the king under attack
          for (const auto& M : moves) {
            bool found = false;

            for (const auto& A : attack_vector) {
              if (M == A) {
                found = true;
                break;
              }
            }

            if (!found) {
              const move_t move = {i, M};
              result.push_back(move);
            }
          }
        } break;

        default: {
          // Generate others pieces moves
          for (const auto& M : moves) {
            const move_t move = {i, M};
            result.push_back(move);
          }
        }

        break;
      }
    }
  }

  return result;
}