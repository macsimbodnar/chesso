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


// inline std::vector<uint8_t> generate_pawn(
//     const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
//     const uint8_t index)
// {
//   std::vector<uint8_t> move_candidates;
//   move_candidates.reserve(2);
//   std::vector<uint8_t> attack_candidates;
//   attack_candidates.reserve(2);

//   const color_t color = get_color(board[index]);
//   switch (color) {
//     case color_t::BLACK:
//       move_candidates.push_back(index + 0xF0);
//       break;
//     case color_t::WHITE:
//       move_candidates.push_back(0x10);
//       break;

//     default:
//       throw input_exception("Unexpected color: " + STR(INT(color)));
//       break;
//   }

//   std::vector<uint8_t> result;
//   result.reserve(4);  // Worst case

//   // Check
//   {
//     const uint8_t candidate = move_candidates[0];
//     if (!(candidate & 0x88) && board[candidate] != piece_t::EMPTY) {
//       result.push_back(candidate);
//     }
//   }

//   {

//     const uint8_t candidate = move_candidates[1];
//     if ()
//   }

//   return result;
// }

inline std::vector<uint8_t> generate_b_pawn(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
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
    bool condition =
        !(candidate & 0x88) && (U32(board[candidate]) & U32(color_t::WHITE));

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack left
    const uint8_t candidate = index - 0x0F;
    bool condition =
        !(candidate & 0x88) && (U32(board[candidate]) & U32(color_t::WHITE));

    if (condition) { result.push_back(candidate); }
  }

  return result;
}


inline std::vector<uint8_t> generate_w_pawn(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
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
    bool condition =
        !(candidate & 0x88) && (U32(board[candidate]) & U32(color_t::BLACK));

    if (condition) { result.push_back(candidate); }
  }

  {
    // Check for attack left
    const uint8_t candidate = index + 0x0F;
    bool condition =
        !(candidate & 0x88) && (U32(board[candidate]) & U32(color_t::BLACK));

    if (condition) { result.push_back(candidate); }
  }

  return result;
}


inline std::vector<uint8_t> generate_sliding(
    const std::array<uint8_t, 4>& directions,
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  result.reserve(16);  // Worst case

  color_t color = get_color(board[index]);

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
        color_t candidate_color = get_color(p);
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
    const uint8_t index)
{
  std::vector<uint8_t> result;
  result.reserve(offsets_k.size());

  color_t color = get_color(board[index]);

  for (const auto I : offsets) {
    const uint8_t candidate = index + I;

    if (candidate & 0x88) { continue; }

    piece_t p = board[candidate];
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
    const uint8_t index)
{
  std::vector<uint8_t> result = generate_sliding(directions_rook, board, index);

  assert(result.size() <= 14);

  return result;
}


inline std::vector<uint8_t> generate_bishop(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> res = generate_sliding(directions_bishop, board, index);

  assert(res.size() <= 16);

  return res;
}


inline std::vector<uint8_t> generate_knight(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> result = generate_jumping(offsets_n, board, index);

  return result;
}


inline std::vector<uint8_t> generate_king(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> result = generate_jumping(offsets_k, board, index);

  return result;
}


inline std::vector<uint8_t> generate_queen(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  std::vector<uint8_t> h_and_v_moves = generate_rook(board, index);
  std::vector<uint8_t> diagonal_moves = generate_bishop(board, index);

  result.insert(result.end(), std::make_move_iterator(h_and_v_moves.begin()),
                std::make_move_iterator(h_and_v_moves.end()));

  result.insert(result.end(), std::make_move_iterator(diagonal_moves.begin()),
                std::make_move_iterator(diagonal_moves.end()));

  return result;
}


inline std::vector<uint8_t> generate_valid_moves(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  const piece_t piece = board[index];

  assert(piece != piece_t::INVALID);
  assert(piece != piece_t::EMPTY);

  std::vector<uint8_t> result;

  switch (piece) {
    case piece_t::B_PAWN:
      result = generate_b_pawn(board, index);
      break;

    case piece_t::W_PAWN:
      result = generate_w_pawn(board, index);
      break;

    case piece_t::B_ROOK:
    case piece_t::W_ROOK:
      result = generate_rook(board, index);
      break;

    case piece_t::B_KNIGHT:
    case piece_t::W_KNIGHT:
      result = generate_knight(board, index);
      break;

    case piece_t::B_BISHOP:
    case piece_t::W_BISHOP:
      result = generate_bishop(board, index);
      break;

    case piece_t::B_QUEEN:
    case piece_t::W_QUEEN:
      result = generate_queen(board, index);
      break;

    case piece_t::B_KING:
    case piece_t::W_KING:
      result = generate_king(board, index);
      break;

    default:
      break;
  }

  // Refine moves

  return result;
}


inline std::vector<move_t> generate_valid_moves(
    const board_state_t& board_state)
{
  std::vector<move_t> result;

  /*****************************************************************************
   * Generate enemy attacks vector
   ****************************************************************************/
  std::vector<uint8_t> attacks;
  attacks.reserve(64);  // worst case

  for (uint8_t i = 0; i < board_state.board.size(); ++i) {
    const piece_t p = board_state.board[i];

    // Iterate over opposite color pieces
    if (p != piece_t::INVALID && p != piece_t::EMPTY &&
        board_state.active_color != get_color(p)) {
      const auto moves = generate_valid_moves(board_state.board, i);

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

  /*****************************************************************************
   * Check if king is under attack
   ****************************************************************************/

  // Get king position
  uint8_t king_pos = 0x08;  // Assign an invalid position!
  for (uint8_t i = 0; i < board_state.board.size(); ++i) {
    const piece_t p = board_state.board[i];

    if ((p == piece_t::B_KING || p == piece_t::W_KING) &&
        board_state.active_color == get_color(p)) {
      king_pos = i;
    }
  }

  // Check if the position is valid
  assert(!(king_pos & 0x88));

  // Search if any of this attack the king

  return result;
}