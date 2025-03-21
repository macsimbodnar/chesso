#include "move_generator.hpp"
#include <cassert>
#include <map>
#include <unordered_map>
#include "board.hpp"
#include "exceptions.hpp"
#include "utils.hpp"


static const index_t OFFSETS_N[8] = {0x21, 0x1F, 0x0E, 0xEE,
                                     0xDF, 0xE1, 0xF2, 0x12};
static const index_t OFFSETS_K[8] = {0xFF, 0x0F, 0x10, 0x11,
                                     0x01, 0xF1, 0xF0, 0xEF};
static const index_t DIRECTIONS_ROOK[4] = {0x10, 0xF0, 0x01, 0xFF};
static const index_t DIRECTIONS_BISHOP[4] = {0x11, 0x0F, 0xF1, 0xEF};


size_t generate_pawn_attacks(index_t index,
                             color_t color,
                             move_t result[],
                             size_t max_size)
{
  assert(max_size >= 2);

  // Max 2
  size_t moves_count = 0;

  if (color == WHITE) {
    {
      // Attack right
      const index_t candidate = index + 0x11;

      if (!(candidate & 0x88)) {
        // If candidate on board
        const move_t move = {index, candidate, W_PAWN};
        result[moves_count] = move;
        ++moves_count;
      }
    }

    {
      // Attack left
      const index_t candidate = index + 0x0F;

      if (!(candidate & 0x88)) {
        // If candidate on board
        const move_t move = {index, candidate, W_PAWN};
        result[moves_count] = move;
        ++moves_count;
      }
    }
  } else {
    {
      // Attack left (from white point of view of the chessboard)
      const index_t candidate = index - 0x11;

      if (!(candidate & 0x88)) {
        // If candidate on board
        const move_t move = {index, candidate, B_PAWN};
        result[moves_count] = move;
        ++moves_count;
      }
    }

    {
      // Attack right (from white point of view of teh chessboard)
      const index_t candidate = index - 0x0F;

      if (!(candidate & 0x88)) {
        // If candidate on board
        const move_t move = {index, candidate, B_PAWN};
        result[moves_count] = move;
        ++moves_count;
      }
    }
  }

  assert(moves_count <= 2);
  assert(moves_count <= max_size);
  return moves_count;
}


size_t generate_b_pawn(index_t index,
                       const board_t* board,
                       move_t result[],
                       size_t max_size)
{
  assert(result != nullptr);
  assert(max_size >= 8);

  // Max 4 + 4 promotions
  size_t moves_count = 0;

  const piece_t* b = board->board;
  const index_t en_passant = board->game_state.en_passant;
  const piece_t piece = board->board[index];

  {
    // Check fo the move in front
    const index_t candidate = index - 0x10;
    if (!(candidate & 0x88) && b[candidate] == piece_t::EMPTY) {
      move_t move = {index, candidate, piece};

      if (candidate < 0x08) {  // The candidate >= 0x00 is superfluous
        //  Promotion case
        move.promoted_to = TO_QUEEN;
        result[moves_count] = move;
        ++moves_count;

        move.promoted_to = TO_ROOK;
        result[moves_count] = move;
        ++moves_count;

        move.promoted_to = TO_BISHOP;
        result[moves_count] = move;
        ++moves_count;

        move.promoted_to = TO_KNIGHT;
        result[moves_count] = move;
        ++moves_count;
      } else {
        // Normal pawn push
        result[moves_count] = move;
        ++moves_count;
      }
    }
  }

  {
    // Check fo the move 2 in front
    const index_t candidate = index - 0x20;
    bool condition = !(candidate & 0x88) && (index > 0x5F) && (index < 0x68) &&
                     (b[candidate] == EMPTY) && (b[candidate + 0x10] == EMPTY);

    if (condition) {
      move_t move = {index, candidate, piece};
      move.double_pawn_move = true;
      result[moves_count] = move;
      ++moves_count;
    }
  }

  {
    // Check for attack left (from white point of view of the chessboard)
    const index_t candidate = index - 0x11;
    if (!(candidate & 0x88)) {
      // If candidate on board

      if (contains_opponent(candidate, WHITE, board)) {
        //  Check normal attack
        move_t move = {index, candidate, piece};
        move.captured = b[candidate];

        if (candidate < 0x08) {  // The candidate >= 0x00 is superfluous
          // Attack and promotion
          move.promoted_to = TO_QUEEN;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_ROOK;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_BISHOP;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_KNIGHT;
          result[moves_count] = move;
          ++moves_count;
        } else {
          // Normal pawn attack
          result[moves_count] = move;
          ++moves_count;
        }

      } else if (en_passant == candidate) {
        // Check if we attack en-passant
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = W_PAWN;

        assert(move.captured == b[index - 0x01]);

        result[moves_count] = move;
        ++moves_count;
      }
    }
  }

  {
    // Check for attack right (from white point of view of teh chessboard)
    const index_t candidate = index - 0x0F;

    if (!(candidate & 0x88)) {
      // If candidate on board

      if (contains_opponent(candidate, WHITE, board)) {
        // Normal capture
        move_t move = {index, candidate, piece};
        move.captured = b[candidate];

        if (candidate < 0x08) {  // The candidate >= 0x00 is superfluous
          // Attack and promotion
          move.promoted_to = TO_QUEEN;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_ROOK;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_BISHOP;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_KNIGHT;
          result[moves_count] = move;
          ++moves_count;
        } else {
          // Normal pawn attack
          result[moves_count] = move;
          ++moves_count;
        }

      } else if (candidate == en_passant) {
        // En passant capture
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = W_PAWN;

        assert(move.captured == b[index + 0x01]);

        result[moves_count] = move;
        ++moves_count;
      }
    }
  }

  assert(moves_count <= 8);
  assert(moves_count <= max_size);
  return moves_count;
}


size_t generate_w_pawn(index_t index,
                       const board_t* board,
                       move_t result[],
                       size_t max_size)
{
  assert(result != nullptr);
  assert(max_size >= 8);

  // Max 4
  size_t moves_count = 0;

  const piece_t* b = board->board;
  const index_t en_passant = board->game_state.en_passant;
  const piece_t piece = board->board[index];

  {
    // Check fo the move in front
    const index_t candidate = index + 0x10;
    if (!(candidate & 0x88) && b[candidate] == EMPTY) {
      move_t move = {index, candidate, piece};

      if (candidate > 0x6F && candidate < 0x78) {
        //  Promotion case
        move.promoted_to = TO_QUEEN;
        result[moves_count] = move;
        ++moves_count;

        move.promoted_to = TO_ROOK;
        result[moves_count] = move;
        ++moves_count;

        move.promoted_to = TO_BISHOP;
        result[moves_count] = move;
        ++moves_count;

        move.promoted_to = TO_KNIGHT;
        result[moves_count] = move;
        ++moves_count;
      } else {
        // Normal pawn push
        result[moves_count] = move;
        ++moves_count;
      }
    }
  }

  {
    // Check fo the move 2 in front
    const index_t candidate = index + 0x20;
    bool condition = !(candidate & 0x88) && (index > 0x0F) && (index < 0x20) &&
                     (b[candidate] == EMPTY) && (b[candidate - 0x10] == EMPTY);

    if (condition) {
      move_t move = {index, candidate, piece};
      move.double_pawn_move = true;
      result[moves_count] = move;
      ++moves_count;
    }
  }

  {
    // Check for attack right
    const index_t candidate = index + 0x11;

    if (!(candidate & 0x88)) {
      // If candidate on board

      if (contains_opponent(candidate, BLACK, board)) {
        //  Check normal attack
        move_t move = {index, candidate, piece};
        move.captured = b[candidate];

        if (candidate > 0x6F && candidate < 0x78) {
          //  Promotion case
          move.promoted_to = TO_QUEEN;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_ROOK;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_BISHOP;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_KNIGHT;
          result[moves_count] = move;
          ++moves_count;
        } else {
          // Normal pawn attack
          result[moves_count] = move;
          ++moves_count;
        }

      } else if (en_passant == candidate) {
        // Check if we attack en-passant
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = B_PAWN;

        assert(move.captured == b[index + 0x01]);

        result[moves_count] = move;
        ++moves_count;
      }
    }
  }

  {
    // Check for attack left
    const index_t candidate = index + 0x0F;

    if (!(candidate & 0x88)) {
      // If candidate on board

      if (contains_opponent(candidate, BLACK, board)) {
        // Normal capture
        move_t move = {index, candidate, piece};
        move.captured = b[candidate];

        if (candidate > 0x6F && candidate < 0x78) {
          //  Promotion case
          move.promoted_to = TO_QUEEN;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_ROOK;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_BISHOP;
          result[moves_count] = move;
          ++moves_count;

          move.promoted_to = TO_KNIGHT;
          result[moves_count] = move;
          ++moves_count;
        } else {
          // Normal pawn attack
          result[moves_count] = move;
          ++moves_count;
        }

      } else if (candidate == en_passant) {
        // En passant capture
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = B_PAWN;

        assert(move.captured == b[index - 0x01]);

        result[moves_count] = move;
        ++moves_count;
      }
    }
  }

  assert(moves_count <= 8);
  assert(moves_count <= max_size);
  return moves_count;
}


size_t generate_sliding(index_t index,
                        color_t color,
                        const index_t directions[],
                        const size_t directions_size,
                        const board_t* board,
                        move_t result[],
                        size_t max_size)
{
  assert(directions != nullptr);
  assert(result != nullptr);
  assert(max_size >= 14);

  // Max 14
  size_t moves_count = 0;

  const piece_t* b = board->board;
  const piece_t piece = board->board[index];

  for (size_t i = 0; i < directions_size; ++i) {
    index_t candidate = index;
    while (true) {
      candidate += directions[i];

      // If we are off board exit
      if (candidate & 0x88) { break; }
      assert(b[candidate] != INVALID);

      // Also break if same color piece block the ray
      const piece_t p = b[candidate];
      if (p == EMPTY) {
        const move_t move = {index, candidate, piece};
        result[moves_count] = move;
        ++moves_count;
      } else {
        const color_t candidate_color = get_piece_color(p);
        // If the are attacking add the position
        if (candidate_color != color) {
          move_t move = {index, candidate, piece};
          move.captured = p;
          result[moves_count] = move;
          ++moves_count;
        }

        break;
      }
    }
  }

  assert(moves_count <= 14);
  assert(moves_count <= max_size);
  return moves_count;
}


size_t generate_jumping(index_t index,
                        color_t color,
                        const index_t offsets[],
                        const size_t offsets_size,
                        const board_t* board,
                        move_t result[],
                        size_t max_size)
{
  assert(offsets != nullptr);
  assert(result != nullptr);
  assert(max_size >= offsets_size);

  // Max offsets_size
  size_t moves_count = 0;

  const piece_t* b = board->board;
  const piece_t piece = board->board[index];

  for (size_t i = 0; i < offsets_size; ++i) {
    const index_t candidate = index + offsets[i];

    if (candidate & 0x88) {
      // Check if out of the board
      continue;
    }

    const piece_t p = b[candidate];
    assert(p != piece_t::INVALID);

    move_t move_candidate = {index, candidate, piece};

    // Check if attacking his own color
    if (p != piece_t::EMPTY) {
      if (get_piece_color(p) != color) {
        // If candidate is opponent then capture
        move_candidate.captured = p;
      } else {
        // If attacking same color then discard candidate
        continue;
      }
    }


    result[moves_count] = move_candidate;
    ++moves_count;
  }

  assert(moves_count <= offsets_size);
  assert(moves_count <= max_size);

  return moves_count;
}


size_t generate_rook(index_t index,
                     color_t color,
                     const board_t* board,
                     move_t result[],
                     size_t max_size)
{
  assert(board != nullptr);
  assert(result != nullptr);
  assert(max_size >= 14);

  const size_t moves_count =
      generate_sliding(index, color, DIRECTIONS_ROOK,
                       sizeof(DIRECTIONS_ROOK) / sizeof(DIRECTIONS_ROOK[0]),
                       board, result, max_size);

  assert(moves_count <= 14);
  assert(moves_count <= max_size);

  return moves_count;
}


size_t generate_bishop(index_t index,
                       color_t color,
                       const board_t* board,
                       move_t result[],
                       size_t max_size)
{
  assert(board != nullptr);
  assert(result != nullptr);
  assert(max_size >= 16);

  const size_t moves_count =
      generate_sliding(index, color, DIRECTIONS_BISHOP,
                       sizeof(DIRECTIONS_BISHOP) / sizeof(DIRECTIONS_BISHOP[0]),
                       board, result, max_size);

  assert(moves_count <= 16);
  assert(moves_count <= max_size);

  return moves_count;
}


size_t generate_knight(index_t index,
                       color_t color,
                       const board_t* board,
                       move_t result[],
                       size_t max_size)
{
  assert(board != nullptr);
  assert(result != nullptr);
  assert(max_size >= 64);

  const size_t moves_count = generate_jumping(
      index, color, OFFSETS_N, sizeof(OFFSETS_N) / sizeof(OFFSETS_N[0]), board,
      result, max_size);

  assert(moves_count <= 64);
  assert(moves_count <= max_size);

  return moves_count;
}


size_t generate_queen(index_t index,
                      color_t color,
                      const board_t* board,
                      move_t result[],
                      size_t max_size)
{
  assert(board != nullptr);
  assert(result != nullptr);
  assert(max_size >= 30);

  const size_t h_and_v_moves_size =
      generate_rook(index, color, board, result, max_size);

  assert(h_and_v_moves_size <= 14);

  const size_t diagonal_moves = generate_bishop(
      index, color, board, result + h_and_v_moves_size, max_size);

  assert(h_and_v_moves_size <= 16);

  const size_t moves_count = h_and_v_moves_size + diagonal_moves;

  assert(moves_count <= 30);
  assert(moves_count <= max_size);

  return moves_count;
}


size_t generate_king(index_t index,
                     color_t color,
                     const board_t* board,
                     move_t result[],
                     size_t max_size)
{
  assert(board != nullptr);
  assert(result != nullptr);
  assert(max_size >= 8);

  // Max 8
  size_t moves_count = generate_jumping(
      index, color, OFFSETS_K, sizeof(OFFSETS_K) / sizeof(OFFSETS_K[0]), board,
      result, max_size);

  // Handle castling
  const piece_t* b = board->board;
  const castling_t castling = board->game_state.castling;
  const piece_t piece = board->board[index];

  switch (color) {
    case WHITE:
      if ((castling & WQ) && b[0x01] == EMPTY && b[0x02] == EMPTY &&
          b[0x03] == EMPTY) {
        // Queen side available
        assert(index == 0x04);
        const index_t candidate = 0x02;
        move_t move = {index, candidate, piece};
        move.castling_move = true;
        result[moves_count] = move;
        ++moves_count;
      }

      if ((castling & WK) && b[0x05] == EMPTY && b[0x06] == EMPTY) {
        // King side available
        assert(index == 0x04);
        const index_t candidate = 0x06;
        move_t move = {index, candidate, piece};
        move.castling_move = true;
        result[moves_count] = move;
        ++moves_count;
      }
      break;
    case BLACK:
      if ((castling & BQ) && b[0x71] == EMPTY && b[0x72] == EMPTY &&
          b[0x73] == EMPTY) {
        // Queen side available
        assert(index == 0x74);
        const index_t candidate = 0x72;
        move_t move = {index, candidate, piece};
        move.castling_move = true;
        result[moves_count] = move;
        ++moves_count;
      }

      if ((castling & BK) && b[0x75] == EMPTY && b[0x76] == EMPTY) {
        // King side available
        assert(index == 0x74);
        const index_t candidate = 0x76;
        move_t move = {index, candidate, piece};
        move.castling_move = true;
        result[moves_count] = move;
        ++moves_count;
      }
      break;

    default:
      assert(false);
      break;
  }

  assert(moves_count <= 8);
  assert(moves_count <= max_size);

  return moves_count;
}


size_t generate_pseudo_legal_moves_from_index(index_t index,
                                              const board_t* board,
                                              move_t result[],
                                              size_t max_size)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(index_to_position(index).file < 8);
  assert(index_to_position(index).rank < 8);
  assert(board->board[index] != INVALID);
  assert(board->board[index] != EMPTY);
  assert(result != nullptr);
  // assert(max_size >= 64);

  size_t moves_count = 0;

  const piece_t piece = board->board[index];
  assert(piece != INVALID && piece != EMPTY);

  switch (piece) {
    case piece_t::B_PAWN:
      moves_count = generate_b_pawn(index, board, result, max_size);
      break;
    case piece_t::W_PAWN:
      moves_count = generate_w_pawn(index, board, result, max_size);
      break;

    case piece_t::B_ROOK:
      moves_count = generate_rook(index, BLACK, board, result, max_size);
      break;
    case piece_t::W_ROOK:
      moves_count = generate_rook(index, WHITE, board, result, max_size);
      break;

    case piece_t::B_KNIGHT:
      moves_count = generate_knight(index, BLACK, board, result, max_size);
      break;
    case piece_t::W_KNIGHT:
      moves_count = generate_knight(index, WHITE, board, result, max_size);
      break;

    case piece_t::B_BISHOP:
      moves_count = generate_bishop(index, BLACK, board, result, max_size);
      break;
    case piece_t::W_BISHOP:
      moves_count = generate_bishop(index, WHITE, board, result, max_size);
      break;

    case piece_t::B_QUEEN:
      moves_count = generate_queen(index, BLACK, board, result, max_size);
      break;
    case piece_t::W_QUEEN:
      moves_count = generate_queen(index, WHITE, board, result, max_size);
      break;

    case piece_t::B_KING:
      moves_count = generate_king(index, BLACK, board, result, max_size);
      break;
    case piece_t::W_KING:
      moves_count = generate_king(index, WHITE, board, result, max_size);
      break;

    default:
      assert(false);
      break;
  }

  // assert(moves_count <= 64);
  assert(moves_count <= max_size);

  return moves_count;
}


size_t generate_attacks_vector(color_t target_color,
                               const board_t* board,
                               move_t result[],
                               size_t max_size)
{
  assert(board != nullptr);
  assert(result != nullptr);
  assert(max_size >= 64);

  // Max 64
  size_t moves_count = 0;

  // Removing the king from the board.
  const piece_t king_to_remove = (target_color == WHITE) ? B_KING : W_KING;

  // Store the king pos
  const index_t king_index =
      get_king_index(get_piece_color(king_to_remove), board);

  // Unset the king.
  // NOTE(max): Removing the const attribute. We want this function to be const
  // on the board since we know that there is no way we return without resetting
  // the king on the board!!! Pay attention to this!
  // TODO: Lock the board variable here since it's afake const. In order to make
  // it safe for multithread computation
  board_t* non_const_board = const_cast<board_t*>(board);
  const piece_t removed_piece = remove_piece(king_index, non_const_board);
  assert(removed_piece == king_to_remove);
  (void)removed_piece;

  // Generate the moves
  for (index_t i = 0; i < BOARD_SIZE; ++i) {
    const piece_t p = board->board[i];

    // Iterate over opposite color pieces
    if (p != INVALID && p != EMPTY && target_color == get_piece_color(p)) {
      // Handle Pawn move separately
      if (p == W_PAWN || p == B_PAWN) {
        size_t pawn_attacks_size = generate_pawn_attacks(
            i, target_color, result + moves_count, max_size);
        moves_count += pawn_attacks_size;
      } else {
        size_t ps_moves_count = generate_pseudo_legal_moves_from_index(
            i, board, result + moves_count, max_size);
        moves_count += ps_moves_count;
      }

      // We keep duplicates for easy double check detection.

      // for (const move_t& move : moves_for_index) {
      //   attacks.push_back(move);
      //   result[*result_size] = move;
      //   ++(*result_size);
      // }
    }
  }

  // Reset the king to the board
  put_piece(king_index, king_to_remove, non_const_board);

  assert(board->board[king_index] == king_to_remove);

  assert(moves_count <= 64);
  assert(moves_count <= max_size);
  return moves_count;
}


bool is_index_attacked(index_t index,
                       const move_t attacks[],
                       size_t attacks_size)
{
  for (size_t i = 0; i < attacks_size; ++i) {
    if (attacks[i].to == index) { return true; }
  }

  return false;
}


bool is_castling_valid(const move_t* move,
                       const move_t attacks[],
                       size_t attacks_size)
{
  assert(move->castling_move);

  // TODO: Make it more professional! We can generalize those cases
  switch (move->to) {
    case 0x02:
      assert(move->from == 0x04);
      // White long castling. Target squares 0x04 0x03 0x02
      if (is_index_attacked(0x04, attacks, attacks_size) ||
          is_index_attacked(0x03, attacks, attacks_size) ||
          is_index_attacked(0x02, attacks, attacks_size)) {
        return false;
      }
      break;
    case 0x06:
      assert(move->from == 0x04);
      // White short castling. Target squares 0x04 0x05 0x06
      if (is_index_attacked(0x04, attacks, attacks_size) ||
          is_index_attacked(0x05, attacks, attacks_size) ||
          is_index_attacked(0x06, attacks, attacks_size)) {
        return false;
      }
      break;
    case 0x72:
      assert(move->from == 0x74);
      // Black long castling. Target squares 0x74 0x73 0x72
      if (is_index_attacked(0x74, attacks, attacks_size) ||
          is_index_attacked(0x73, attacks, attacks_size) ||
          is_index_attacked(0x72, attacks, attacks_size)) {
        return false;
      }
      break;
    case 0x76:
      assert(move->from == 0x74);
      // White short castling. Target squares 0x74 0x75 0x76
      if (is_index_attacked(0x74, attacks, attacks_size) ||
          is_index_attacked(0x75, attacks, attacks_size) ||
          is_index_attacked(0x76, attacks, attacks_size)) {
        return false;
      }
      break;

    default:
      assert(false);
      break;
  }

  return true;
}


bool is_pin(const move_t* move, index_t king_index, const board_t* board)
{
  // Make the move and see if this leaves the king under check.
  // TODO: Decide if use the make move here or the fastest custom one
  board_t tmp_board = *board;
  // tmp_board.board[move->to] = tmp_board.board[move->from];

  // if (move->en_passant_capture) {
  //   index_t index_to_remove = move->to;
  //   switch (board->game_state.active_color) {
  //     case BLACK:
  //       index_to_remove += 0x10;
  //       break;
  //     case WHITE:
  //       index_to_remove -= 0x10;
  //       break;
  //     default:
  //       assert(false);
  //       break;
  //   }
  //   tmp_board.board[index_to_remove] = EMPTY;
  // }
  // tmp_board.board[move->from] = EMPTY;

  bool happened = make_move(move, &tmp_board, nullptr);
  assert(happened);
  (void)happened;

  move_t attacks[64];

  const size_t moves_count =
      generate_attacks_vector(!board->game_state.active_color, &tmp_board,
                              attacks, sizeof(attacks) / sizeof(attacks[0]));

  assert(moves_count <= sizeof(attacks) / sizeof(attacks[0]));

  for (size_t i = 0; i < moves_count; ++i) {
    if (attacks[i].to == king_index) { return true; }
  }

  return false;
}


bool is_blocking_ray(index_t index, const move_t* move)
{
  // Find the ray direction
  // Generate the ray
  // Check if index is on the ray
  assert(move != nullptr);

  const position_t start = index_to_position(move->from);
  const position_t end = index_to_position(move->to);
  const position_t point = index_to_position(index);

  const int cross_product =
      (point.rank - start.rank) * (end.file - start.file) -
      (point.file - start.file) * (end.rank - start.rank);

  if (std::abs(cross_product) != 0) { return false; }

  // TODO: Check if this is necessary
  bool within_x_bounds = (std::min(start.file, end.file) <= point.file) &&
                         (point.file <= std::max(start.file, end.file));
  bool within_y_bounds = (std::min(start.rank, end.rank) <= point.rank) &&
                         (point.rank <= std::max(start.rank, end.rank));

  return within_x_bounds && within_y_bounds;
}


size_t generate_legal_moves(const board_t* board,
                            move_t result[],
                            size_t max_size)
{
  // TODO: Reimplement this function. It contains a lot of duplicated code and
  // inefficient calls
  assert(board != nullptr);
  assert(result != nullptr);
  assert(max_size >= 267);
  size_t result_count = 0;

  /*****************************************************************************
   * Generate enemy attacks vector
   ****************************************************************************/
  const color_t attack_color = !board->game_state.active_color;
  move_t attacks[64];
  const size_t attacks_size = generate_attacks_vector(
      attack_color, board, attacks, sizeof(attacks) / sizeof(attacks[0]));

  /*****************************************************************************
   * Calculate if under check
   ****************************************************************************/
  bool under_check = false;
  bool under_double_check = false;
  const index_t king_index =
      get_king_index(board->game_state.active_color, board);

  for (size_t i = 0; i < attacks_size; ++i) {
    if (attacks[i].to == king_index) {
      if (!under_check) {
        under_check = true;
      } else {
        under_double_check = true;
      }
    }
  }

  /*****************************************************************************
   * Generate moves
   ****************************************************************************/
  if (under_check) {
    // UNDER CHECK

    // TODO: Double check case and single check case looks the same

    move_t king_moves[8];

    size_t king_moves_count =
        generate_king(king_index, board->game_state.active_color, board,
                      king_moves, sizeof(king_moves) / sizeof(king_moves[0]));

    if (under_double_check) {  // DOUBLE CHECK
      // In the case of double check we consider only king moves

      for (size_t i = 0; i < king_moves_count; ++i) {
        // Remove king moves that put him back in check
        bool should_discard = false;
        for (size_t a = 0; a < attacks_size; ++a) {
          if (king_moves[i].to == attacks[a].to) {
            should_discard = true;
            break;
          }
        }

        // Remove the captures that put him back in check
        if (king_moves[i].captured != INVALID) {
          // Remove the piece from the board and see if that square is under
          // attack
          board_t tmp_board = *board;
          tmp_board.board[king_moves[i].to] = EMPTY;
          move_t tmp_attacks[64];
          const size_t tmp_attacks_count = generate_attacks_vector(
              !tmp_board.game_state.active_color, &tmp_board, tmp_attacks,
              sizeof(tmp_attacks) / sizeof(tmp_attacks[0]));

          for (size_t a_index = 0; a_index < tmp_attacks_count; ++a_index) {
            if (tmp_attacks[a_index].to == king_moves[i].to) {
              // Discard that move
              should_discard = true;
              break;
            }
          }
        }

        // Check if this move is castling, if so remove it
        if (king_moves[i].castling_move) { should_discard = true; }

        // Insert only valid
        if (!should_discard) {
          result[result_count] = king_moves[i];
          ++result_count;
        }
      }
    } else {  // SINGE CHECK
      // King moves away from check
      {
        for (size_t i = 0; i < king_moves_count; ++i) {
          // Remove king moves that put him back in check
          bool should_discard = false;
          for (size_t a_index = 0; a_index < attacks_size; ++a_index) {
            if (king_moves[i].to == attacks[a_index].to) {
              should_discard = true;
              break;
            }
          }

          // Remove the captures that put him back in check
          if (king_moves[i].captured != INVALID) {
            // Remove the piece from the board and see if that square is under
            // attack
            board_t tmp_board = *board;
            tmp_board.board[king_moves[i].to] = EMPTY;

            move_t tmp_attacks[64];
            const size_t tmp_attacks_count = generate_attacks_vector(
                !tmp_board.game_state.active_color, &tmp_board, tmp_attacks,
                sizeof(tmp_attacks) / sizeof(tmp_attacks[0]));

            for (size_t tmp_i = 0; tmp_i < tmp_attacks_count; ++tmp_i) {
              if (tmp_attacks[tmp_i].to == king_moves[i].to) {
                // Discard that move
                should_discard = true;
                break;
              }
            }
          }

          // Check if this move is castling, if so remove it
          if (king_moves[i].castling_move) { should_discard = true; }

          // Insert only valid
          if (!should_discard) {
            result[result_count] = king_moves[i];
            ++result_count;
          }
        }
      }

      // Handle: Remove the attacker moves and block attacks in rays attack
      {
        for (index_t b_i = 0; b_i < BOARD_SIZE; ++b_i) {
          const piece_t P = board->board[b_i];

          if (P != INVALID && P != EMPTY && b_i != king_index &&
              board->game_state.active_color == get_piece_color(P)) {
            // generate moves except for King
            move_t moves[64];
            const size_t moves_count = generate_pseudo_legal_moves_from_index(
                b_i, board, moves, sizeof(moves) / sizeof(moves[0]));

            for (size_t i = 0; i < moves_count; ++i) {
              bool should_discard = true;

              for (size_t a = 0; a < attacks_size; ++a) {
                // Check if the move remove the attacker.
                if (attacks[a].to == king_index &&
                    moves[i].captured != INVALID &&
                    moves[i].to == attacks[a].from) {
                  should_discard = false;
                }

                // Consider en-passant remove the attacker
                if (attacks[a].to == king_index &&
                    moves[i].en_passant_capture) {
                  if (board->game_state.active_color == WHITE) {
                    if (board->board[moves[i].to] == EMPTY &&
                        board->board[moves[i].to - 0x10] == B_PAWN &&
                        (moves[i].to - 0x10) == attacks[a].from) {
                      should_discard = false;
                    }
                  } else {
                    if (board->board[moves[i].to] == EMPTY &&
                        board->board[moves[i].to + 0x10] == W_PAWN &&
                        (moves[i].to + 0x10) == attacks[a].from) {
                      should_discard = false;
                    }
                  }
                }

                // Check if the move block the ray attack
                switch (attacks[a].piece) {
                  case W_QUEEN:
                  case B_QUEEN:
                  case W_BISHOP:
                  case B_BISHOP:
                  case W_ROOK:
                  case B_ROOK:
                    if (is_blocking_ray(moves[i].to, &attacks[a])) {
                      should_discard = false;
                    }
                    break;

                  default:
                    break;
                }
              }

              if (!should_discard) {
                if (!is_pin(&moves[i], king_index, board)) {
                  result[result_count] = moves[i];
                  ++result_count;
                }
              }
            }
          }
        }
      }
    }
  } else {
    // NOT UNDER CHECK
    // TODO: handle moves that put the king under attack if he attack but put
    // himself under attack.
    // Example: FEN [r3k2r/R3P2R/8/8/8/8/8/4K3 b kq - 0 1] move [Kxe7]

    for (index_t b_i = 0; b_i < BOARD_SIZE; ++b_i) {
      const piece_t P = board->board[b_i];

      if (P != INVALID && P != EMPTY &&
          board->game_state.active_color == get_piece_color(P)) {
        // Generate moves
        move_t moves[64];
        const size_t moves_count = generate_pseudo_legal_moves_from_index(
            b_i, board, moves, sizeof(moves) / sizeof(moves[0]));

        switch (P) {
          case B_KING:
          case W_KING: {
            // Handle king moves.

            for (size_t i = 0; i < moves_count; ++i) {
              bool found = false;

              // Remove all the KING moves that move him under attack
              for (size_t a = 0; i < attacks_size; ++a) {
                if (moves[i].to == attacks[a].to) {
                  found = true;
                  break;
                }
              }

              if (!found) {
                // This does not put te king under attack. So we can proceed

                if (moves[i].castling_move) {  // Handling castling
                  // Check if the castling move is legal.
                  if (!is_castling_valid(&moves[i], attacks, attacks_size)) {
                    // If not valid castling then skip to the next one
                    continue;
                  }
                }

                if (moves[i].captured != INVALID) {
                  // Handling king attack that put him under check

                  // Remove the target piece
                  board_t tmp_board = *board;
                  remove_piece(moves[i].to, &tmp_board);

                  move_t tmp_attacks[64];
                  const size_t tmp_attacks_count = generate_attacks_vector(
                      attack_color, &tmp_board, tmp_attacks,
                      sizeof(tmp_attacks) / sizeof(tmp_attacks[0]));

                  // Check if the new attacks prevent this capture
                  bool should_skip_move = false;
                  for (size_t tmp_a = 0; tmp_a < tmp_attacks_count; ++tmp_a) {
                    if (tmp_attacks[tmp_a].to == moves[i].to) {
                      // Then skip this move
                      should_skip_move = true;
                      break;
                    }
                  }

                  if (should_skip_move) { continue; }
                }

                result[result_count] = moves[i];
                ++result_count;
              }
            }
          } break;

          default: {
            // Generate others pieces moves
            for (size_t other_i = 0; other_i < moves_count; ++other_i) {
              if (!is_pin(&moves[other_i], king_index, board)) {
                // If not pin then ok
                result[result_count] = moves[other_i];
                ++result_count;
              }
            }
          }

          break;
        }
      }
    }
  }

  assert(result_count <= max_size);
  return result_count;
}


/**
 * @brief Returns the indexes of the ambiguous moves.
 */
size_t get_ambiguous_move(const move_t* move,
                          const move_t moves[],
                          size_t moves_count,
                          size_t result[],
                          size_t max_result)
{
  assert(moves != nullptr);
  assert(result != nullptr);

  size_t result_count = 0;

  for (size_t i = 0; i < moves_count; ++i) {
    const move_t& I = moves[i];

    // If same piece, same destination and different source
    if (move->piece == I.piece && move->to == I.to && move->from != I.from) {
      result[result_count] = i;
      ++result_count;
    }
  }

  assert(result_count <= max_result);
  return result_count;
}


std::string move_to_algebraic(const move_t* move,
                              const move_t moves[],
                              const size_t moves_size,
                              const board_t* board)
{
  assert(move != nullptr);
  assert(board != nullptr);
  assert(move->piece != INVALID);
  assert(move->piece != EMPTY);

  // Not using the piece_to_char function because the piece moved in always
  // upper case
  static const std::unordered_map<piece_t, char> piece_to_char_map = {
      {B_PAWN, 'P'},   {B_KNIGHT, 'N'}, {B_BISHOP, 'B'}, {B_ROOK, 'R'},
      {B_QUEEN, 'Q'},  {B_KING, 'K'},   {W_PAWN, 'P'},   {W_KNIGHT, 'N'},
      {W_BISHOP, 'B'}, {W_ROOK, 'R'},   {W_QUEEN, 'Q'},  {W_KING, 'K'},
      {INVALID, '*'},  {EMPTY, ' '}};

  static const std::array<char, 8> file_to_char_map = {'a', 'b', 'c', 'd',
                                                       'e', 'f', 'g', 'h'};
  static const std::array<char, 8> rank_to_char_map = {'1', '2', '3', '4',
                                                       '5', '6', '7', '8'};

  std::string notation;

  // Handle castling
  if (move->castling_move) {
    if (move->to == 0x06 || move->to == 0x76)
      return "O-O";  // King-side castling
    if (move->to == 0x02 || move->to == 0x72)
      return "O-O-O";  // Queen-side castling
  }

  if (move->piece != W_PAWN && move->piece != B_PAWN) {
    notation += piece_to_char_map.at(move->piece);  // Non-pawn pieces

    // If ambiguous move the add the from file
    size_t ambiguous_move_indexes[64];
    size_t ambiguous_move_indexes_count = get_ambiguous_move(
        move, moves, moves_size, ambiguous_move_indexes,
        sizeof(ambiguous_move_indexes) / sizeof(ambiguous_move_indexes[0]));

    if (ambiguous_move_indexes_count > 0) {
      const position_t move_from_pos = index_to_position(move->from);
      bool is_file_unique = true;
      bool is_rank_unique = true;

      // Check if file or rank are unique for the move->from
      for (size_t index = 0; index < ambiguous_move_indexes_count; ++index) {
        const size_t i = ambiguous_move_indexes[ambiguous_move_indexes_count];

        const position_t i_pos = index_to_position(moves[i].from);

        if (move_from_pos.file == i_pos.file) { is_file_unique = false; }

        if (move_from_pos.rank == i_pos.rank) { is_rank_unique = false; }

        // Exit from the loop in case both are non unique. No make sense to
        // search for more
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
  if (move->captured != INVALID && move->piece != W_PAWN &&
      move->piece != B_PAWN) {
    notation += 'x';
  }

  // Destination square
  notation += index_to_string_coordinates(move->to);

  // Pawn captures (ex: exd5)
  if ((move->piece == W_PAWN || move->piece == B_PAWN) &&
      move->captured != INVALID) {
    notation = index_to_string_coordinates(move->from)[0] + std::string("x") +
               index_to_string_coordinates(move->to);
  }

  // Pawn promotion
  if (move->promoted_to != TO_NONE) {
    notation += "=";
    switch (move->promoted_to) {
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

  // Handle check
  const index_t opponent_king_index = get_king_index(opponent(board), board);

  board_t tmp_board = *board;

  bool move_happened = make_move(move, &tmp_board, nullptr);
  assert(move_happened == true);
  (void)move_happened;  // Supress the unused var log

  // Generate moves for my color but after the current move is done
  move_t pseudo_legal_moves[64];
  const size_t pseudo_legal_moves_count =
      generate_pseudo_legal_moves_from_index(
          move->to, &tmp_board, pseudo_legal_moves,
          sizeof(pseudo_legal_moves) / sizeof(pseudo_legal_moves[0]));

  // Check if one of this moves put under check the opponent king

  for (size_t i = 0; i < pseudo_legal_moves_count; ++i) {
    if (pseudo_legal_moves[i].to == opponent_king_index) {
      // Now let's check if this is check mate

      // Generate legal moves after the make move to see if any available.
      move_t moves[270];
      const size_t moves_count = generate_legal_moves(
          &tmp_board, moves, sizeof(moves) / sizeof(moves[0]));

      if (moves_count == 0) {
        // Check mate
        notation += '#';
        break;
      } else {
        // Append a '+' to the notation
        notation += '+';
        break;
      }
    }
  }

  return notation;
}


// Main function: parse a SAN move into move_t
move_t algebraic_to_move(std::string notation, const board_t* board)
{
  assert(board != nullptr);

  static const std::map<char, uint8_t> char_to_file_map = {
      {'a', 0}, {'b', 1}, {'c', 2}, {'d', 3},
      {'e', 4}, {'f', 5}, {'g', 6}, {'h', 7}};
  static const std::map<char, uint8_t> char_to_rank_map = {
      {'1', 0}, {'2', 1}, {'3', 2}, {'4', 3},
      {'5', 4}, {'6', 5}, {'7', 6}, {'8', 7}};

  const std::string original_notation = notation;

  move_t result;
  const color_t color = board->game_state.active_color;

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
    result.castling_move = true;
    switch (color) {
      case BLACK:
        result.from = 0x74;
        result.to = 0x72;
        result.piece = B_KING;
        break;
      case WHITE:
        result.from = 0x04;
        result.to = 0x02;
        result.piece = W_KING;
        break;
      default:
        assert(false);
        break;
    }

    return result;
  }

  if (notation == "O-O") {
    result.castling_move = true;

    switch (color) {
      case BLACK:
        result.from = 0x74;
        result.to = 0x76;
        result.piece = B_KING;
        break;
      case WHITE:
        result.from = 0x04;
        result.to = 0x06;
        result.piece = W_KING;
        break;
      default:
        assert(false);
        break;
    }

    return result;
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
        moving_piece = INVALID;
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
    throw algebraic_exception("Wrong formatting. Invalid Algebraic notation: " +
                              original_notation);
  }

  std::string dest_square = cleaned.substr(cleaned.size() - 2, 2);
  index_t to_index = string_coordinates_to_index(dest_square);
  result.to = to_index;

  if (result.to >= INVALID_BOARD_INDEX) {
    throw algebraic_exception(
        "Invalid destination square. Invalid Algebraic notation: " +
        original_notation);
  }

  if (is_capture) {
    // Attempt to use the destination as capture piece
    result.captured = board->board[result.to];

    // In case of en-passant override the capture
    if (board->game_state.en_passant != INVALID_BOARD_INDEX) {
      if (color == WHITE) {
        if (board->board[result.to] == EMPTY &&
            board->board[result.to - 0x10] == B_PAWN) {
          result.captured = B_PAWN;
        }
      } else {
        if (board->board[result.to] == EMPTY &&
            board->board[result.to + 0x10] == W_PAWN) {
          result.captured = W_PAWN;
        }
      }
    }

    if (result.captured == INVALID || result.captured == EMPTY) {
      throw algebraic_exception(
          "No capture found on the board. Invalid Algebraic notation: " +
          original_notation);
    }
  }

  // Any remaining characters between our initial pos and the destination
  // have been interpreted as disambiguation.
  // (In many SAN moves the disambiguation is omitted if unneeded.)
  // Here we already extracted potential disambiguation earlier.

  // Generate legal moves and search the compatible one
  move_t legal_moves[270];
  const size_t legal_moves_count = generate_legal_moves(
      board, legal_moves, sizeof(legal_moves) / sizeof(legal_moves[0]));

  bool found = false;

  for (size_t i = 0; i < legal_moves_count; ++i) {
    if (legal_moves[i].to == result.to &&
        legal_moves[i].piece == result.piece &&
        legal_moves[i].promoted_to == result.promoted_to &&
        legal_moves[i].captured == result.captured &&
        legal_moves[i].castling_move == result.castling_move) {
      // Check for disambiguous
      const position_t legal_from_pos = index_to_position(legal_moves[i].from);

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
      result = legal_moves[i];
      break;
    }
  }

  if (!found) {
    throw algebraic_exception(
        "No legal move found. Invalid Algebraic notation: " +
        original_notation);
  }

  return result;
}
















#pragma once

#include "data_structures.hpp"

size_t generate_pseudo_legal_moves_from_index(index_t index,
                                              const board_t* board,
                                              move_t result[],
                                              size_t max_size);

size_t generate_legal_moves(const board_t* board,
                            move_t result[],
                            size_t max_size);

std::string move_to_algebraic(const move_t* move,
                              const move_t moves[],
                              const size_t moves_size,
                              const board_t* board);

move_t algebraic_to_move(std::string notation, const board_t* board);
