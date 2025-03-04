#include "move_generator.hpp"
#include <cassert>
#include "utils.hpp"


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


std::vector<index_t> generate_pseudo_legal_moves_for_piece(index_t index,
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
      // case piece_t::W_PAWN:
      //   result =
      //       generate_w_pawn(board, index,
      //       board_state.en_passant_target_square);
      //   break;

      // case piece_t::B_ROOK:
      //   result = generate_rook(board, color_t::BLACK, index);
      //   break;
      // case piece_t::W_ROOK:
      //   result = generate_rook(board, color_t::WHITE, index);
      //   break;

      // case piece_t::B_KNIGHT:
      //   result = generate_knight(board, color_t::BLACK, index);
      //   break;
      // case piece_t::W_KNIGHT:
      //   result = generate_knight(board, color_t::WHITE, index);
      //   break;

      // case piece_t::B_BISHOP:
      //   result = generate_bishop(board, color_t::BLACK, index);
      //   break;
      // case piece_t::W_BISHOP:
      //   result = generate_bishop(board, color_t::WHITE, index);
      //   break;

      // case piece_t::B_QUEEN:
      //   result = generate_queen(board, color_t::BLACK, index);
      //   break;
      // case piece_t::W_QUEEN:
      //   result = generate_queen(board, color_t::WHITE, index);
      //   break;

      // case piece_t::B_KING:
      //   result = generate_king(board, index, color_t::BLACK,
      //                          board_state.available_castling);
      //   break;
      // case piece_t::W_KING:
      //   result = generate_king(board, index, color_t::WHITE,
      //                          board_state.available_castling);
      //   break;

    default:
      assert(false);
      break;
  }

  return result;
}