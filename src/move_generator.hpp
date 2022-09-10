#pragma once
#include <array>
#include "utils.hpp"

static const std::array<uint8_t, 2> offsets_p = {0x10, 0x20};

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
      result.reserve(offsets_p.size());
      for (const auto I : offsets_p) {
        result.push_back(index - I);
      }
      break;

    case piece_t::W_PAWN:
      result.reserve(offsets_p.size());
      for (const auto I : offsets_p) {
        result.push_back(index + I);
      }
      break;

    default:
      break;
  }

  return result;
}