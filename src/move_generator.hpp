#pragma once
#include <array>
#include "utils.hpp"

static const std::array<uint8_t, 2> offsets_p = {0x10, 0x20};
static const std::array<uint8_t, 14> offsets_r = {0x10, 0x20, 0x30, 0x40, 0x50,
                                                  0x60, 0x70, 0x01, 0x02, 0x03,
                                                  0x04, 0x05, 0x06, 0x07};
static const std::array<uint8_t, 8> offsets_n = {0x21, 0x1F, 0x0E, 0xEE,
                                                 0xDF, 0xE1, 0xF2, 0x12};
static const std::array<uint8_t, 8> offsets_k = {0xFF, 0x0F, 0x10, 0x11,
                                                 0x01, 0xF1, 0xF0, 0xEF};


inline std::vector<uint8_t> generate_b_pawn(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)

{
  std::vector<uint8_t> result;
  result.reserve(offsets_p.size());

  for (const auto I : offsets_p) {
    const uint8_t candidate = index - I;

    if (candidate & 0x88) { continue; }

    result.push_back(candidate);
  }

  return result;
}


inline std::vector<uint8_t> generate_w_pawn(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)

{
  std::vector<uint8_t> result;
  result.reserve(offsets_p.size());

  for (const auto I : offsets_p) {
    const uint8_t candidate = index + I;

    if (candidate & 0x88) { continue; }

    result.push_back(candidate);
  }

  return result;
}


inline std::vector<uint8_t> generate_rook(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> candidates;
  candidates.reserve(offsets_r.size() * 2);

  // Loop on up and right
  for (const auto I : offsets_r) {
    candidates.push_back(index + I);
  }

  // Loop on down and left
  for (const auto I : offsets_r) {
    candidates.push_back(index - I);
  }

  std::vector<uint8_t> result;
  result.reserve(candidates.size());

  for (const auto& I : candidates) {
    // Check if out of board
    if (I & 0x88) { continue; }

    result.push_back(I);
  }

  return result;
}


inline std::vector<uint8_t> generate_knight(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  result.reserve(offsets_n.size());

  for (const auto I : offsets_n) {
    const uint8_t candidate = index + I;

    if (candidate & 0x88) { continue; }

    result.push_back(candidate);
  }

  return result;
}


inline std::vector<uint8_t> generate_bishop(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  result.reserve(16);  // Worst case

  {  // top - right
    uint8_t candidate = index;
    while (true) {
      candidate += 0x11;

      if (candidate & 0x88) { break; }
      result.push_back(candidate);
    }
  }

  {  // top - left
    uint8_t candidate = index;
    while (true) {
      candidate += 0x0F;

      if (candidate & 0x88) { break; }
      result.push_back(candidate);
    }
  }

  {  // bottom - right
    uint8_t candidate = index;
    while (true) {
      candidate -= 0x0F;

      if (candidate & 0x88) { break; }
      result.push_back(candidate);
    }
  }

  {  // bottom - left
    uint8_t candidate = index;
    while (true) {
      candidate -= 0x11;

      if (candidate & 0x88) { break; }
      result.push_back(candidate);
    }
  }

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


inline std::vector<uint8_t> generate_king(
    const std::array<piece_t, BOARD_ARRAY_SIZE>& board,
    const uint8_t index)
{
  std::vector<uint8_t> result;
  result.reserve(offsets_k.size());

  for (const auto I : offsets_k) {
    const uint8_t candidate = index + I;

    if (candidate & 0x88) { continue; }

    result.push_back(candidate);
  }

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

  return result;
}