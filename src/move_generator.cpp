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

std::vector<move_t> generate_pawn_attacks(index_t index,
                                          color_t color,
                                          const board_t* board)
{
  std::vector<move_t> result;
  result.reserve(2);

  if (color == WHITE) {
    {
      // Attack right
      const index_t candidate = index + 0x11;

      if (!(candidate & 0x88)) {
        // If candidate on board
        move_t move = {index, candidate, W_PAWN};
        result.push_back(move);
      }
    }

    {
      // Attack left
      const index_t candidate = index + 0x0F;

      if (!(candidate & 0x88)) {
        // If candidate on board
        move_t move = {index, candidate, W_PAWN};
        result.push_back(move);
      }
    }
  } else {
    {
      // Attack left (from white point of view of the chessboard)
      const index_t candidate = index - 0x11;

      if (!(candidate & 0x88)) {
        // If candidate on board
        move_t move = {index, candidate, B_PAWN};
        result.push_back(move);
      }
    }

    {
      // Attack right (from white point of view of teh chessboard)
      const index_t candidate = index - 0x0F;

      if (!(candidate & 0x88)) {
        // If candidate on board
        move_t move = {index, candidate, B_PAWN};
        result.push_back(move);
      }
    }
  }

  return result;
}


std::vector<move_t> generate_b_pawn(index_t index, const board_t* board)
{
  std::vector<move_t> result;
  result.reserve(8);

  const auto b = board->board;
  const auto en_passant = board->game_state.en_passant;
  const piece_t piece = board->board[index];

  {
    // Check fo the move in front
    const index_t candidate = index - 0x10;
    if (!(candidate & 0x88) && b[candidate] == piece_t::EMPTY) {
      move_t move = {index, candidate, piece};

      if (candidate < 0x08) {  // The candidate >= 0x00 is superfluous
        //  Promotion case
        move.promoted_to = TO_QUEEN;
        result.push_back(move);

        move.promoted_to = TO_ROOK;
        result.push_back(move);

        move.promoted_to = TO_BISHOP;
        result.push_back(move);

        move.promoted_to = TO_KNIGHT;
        result.push_back(move);
      } else {
        // Normal pawn push
        result.push_back(move);
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
      result.push_back(move);
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
          result.push_back(move);

          move.promoted_to = TO_ROOK;
          result.push_back(move);

          move.promoted_to = TO_BISHOP;
          result.push_back(move);

          move.promoted_to = TO_KNIGHT;
          result.push_back(move);
        } else {
          // Normal pawn attack
          result.push_back(move);
        }

      } else if (en_passant == candidate) {
        // Check if we attack en-passant
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = W_PAWN;

        assert(move.captured == b[index - 0x01]);

        result.push_back(move);
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
          result.push_back(move);

          move.promoted_to = TO_ROOK;
          result.push_back(move);

          move.promoted_to = TO_BISHOP;
          result.push_back(move);

          move.promoted_to = TO_KNIGHT;
          result.push_back(move);
        } else {
          // Normal pawn attack
          result.push_back(move);
        }

      } else if (candidate == en_passant) {
        // En passant capture
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = W_PAWN;

        assert(move.captured == b[index + 0x01]);

        result.push_back(move);
      }
    }
  }

  return result;
}


std::vector<move_t> generate_w_pawn(index_t index, const board_t* board)
{
  std::vector<move_t> result;
  result.reserve(4);

  const auto b = board->board;
  const auto en_passant = board->game_state.en_passant;
  const piece_t piece = board->board[index];

  {
    // Check fo the move in front
    const index_t candidate = index + 0x10;
    if (!(candidate & 0x88) && b[candidate] == EMPTY) {
      move_t move = {index, candidate, piece};

      if (candidate > 0x6F && candidate < 0x78) {
        //  Promotion case
        move.promoted_to = TO_QUEEN;
        result.push_back(move);

        move.promoted_to = TO_ROOK;
        result.push_back(move);

        move.promoted_to = TO_BISHOP;
        result.push_back(move);

        move.promoted_to = TO_KNIGHT;
        result.push_back(move);
      } else {
        // Normal pawn push
        result.push_back(move);
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
      result.push_back(move);
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
          result.push_back(move);

          move.promoted_to = TO_ROOK;
          result.push_back(move);

          move.promoted_to = TO_BISHOP;
          result.push_back(move);

          move.promoted_to = TO_KNIGHT;
          result.push_back(move);
        } else {
          // Normal pawn attack
          result.push_back(move);
        }

      } else if (en_passant == candidate) {
        // Check if we attack en-passant
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = B_PAWN;

        assert(move.captured == b[index + 0x01]);

        result.push_back(move);
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
          result.push_back(move);

          move.promoted_to = TO_ROOK;
          result.push_back(move);

          move.promoted_to = TO_BISHOP;
          result.push_back(move);

          move.promoted_to = TO_KNIGHT;
          result.push_back(move);
        } else {
          // Normal pawn attack
          result.push_back(move);
        }

      } else if (candidate == en_passant) {
        // En passant capture
        move_t move = {index, candidate, piece};
        move.en_passant_capture = true;
        move.captured = B_PAWN;

        assert(move.captured == b[index - 0x01]);

        result.push_back(move);
      }
    }
  }

  return result;
}


std::vector<move_t> generate_sliding(index_t index,
                                     color_t color,
                                     const std::array<index_t, 4>& directions,
                                     const board_t* board)
{
  std::vector<move_t> result;
  result.reserve(16);  // Worst case

  const auto b = board->board;
  const piece_t piece = board->board[index];

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
        const move_t move = {index, candidate, piece};
        result.push_back(move);
      } else {
        const color_t candidate_color = get_piece_color(p);
        // If the are attacking add the position
        if (candidate_color != color) {
          move_t move = {index, candidate, piece};
          move.captured = p;
          result.push_back(move);
        }

        break;
      }
    }
  }

  assert(result.size() <= 14);
  return result;
}


std::vector<move_t> generate_jumping(index_t index,
                                     color_t color,
                                     const std::array<index_t, 8>& offsets,
                                     const board_t* board)
{
  std::vector<move_t> result;
  result.reserve(offsets.size() + 4);  // NOTE(max): +4 is for the castling

  const auto b = board->board;
  const piece_t piece = board->board[index];

  for (const auto I : offsets) {
    const index_t candidate = index + I;

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


    result.push_back(move_candidate);
  }

  assert(result.size() <= offsets.size());

  return result;
}


std::vector<move_t> generate_rook(index_t index,
                                  color_t color,
                                  const board_t* board)
{
  const std::vector<move_t> result =
      generate_sliding(index, color, DIRECTIONS_ROOK, board);

  assert(result.size() <= 14);

  return result;
}


std::vector<move_t> generate_bishop(index_t index,
                                    color_t color,
                                    const board_t* board)
{
  const std::vector<move_t> res =
      generate_sliding(index, color, DIRECTIONS_BISHOP, board);

  assert(res.size() <= 16);

  return res;
}


std::vector<move_t> generate_knight(index_t index,
                                    color_t color,
                                    const board_t* board)
{
  const std::vector<move_t> result =
      generate_jumping(index, color, OFFSETS_N, board);

  return result;
}


std::vector<move_t> generate_queen(index_t index,
                                   color_t color,
                                   const board_t* board)
{
  std::vector<move_t> result;
  const std::vector<move_t> h_and_v_moves = generate_rook(index, color, board);
  const std::vector<move_t> diagonal_moves =
      generate_bishop(index, color, board);

  result.insert(result.end(), std::make_move_iterator(h_and_v_moves.begin()),
                std::make_move_iterator(h_and_v_moves.end()));

  result.insert(result.end(), std::make_move_iterator(diagonal_moves.begin()),
                std::make_move_iterator(diagonal_moves.end()));

  return result;
}


std::vector<move_t> generate_king(index_t index,
                                  color_t color,
                                  const board_t* board)
{
  std::vector<move_t> result = generate_jumping(index, color, OFFSETS_K, board);

  // Handle castling
  const auto b = board->board;
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
        result.push_back(move);
      }

      if ((castling & WK) && b[0x05] == EMPTY && b[0x06] == EMPTY) {
        // King side available
        assert(index == 0x04);
        const index_t candidate = 0x06;
        move_t move = {index, candidate, piece};
        move.castling_move = true;
        result.push_back(move);
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
        result.push_back(move);
      }

      if ((castling & BK) && b[0x75] == EMPTY && b[0x76] == EMPTY) {
        // King side available
        assert(index == 0x74);
        const index_t candidate = 0x76;
        move_t move = {index, candidate, piece};
        move.castling_move = true;
        result.push_back(move);
      }
      break;

    default:
      assert(false);
      break;
  }

  return result;
}


std::vector<move_t> generate_pseudo_legal_moves_from_index(index_t index,
                                                           const board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(index_to_position(index).file < 8);
  assert(index_to_position(index).rank < 8);
  assert(board->board[index] != INVALID);
  assert(board->board[index] != EMPTY);

  const piece_t piece = board->board[index];
  assert(piece != INVALID && piece != EMPTY);

  std::vector<move_t> result;

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


std::vector<move_t> generate_attacks_vector(color_t target_color,
                                            const board_t* board)
{
  board_t tmp_board = *board;

  std::vector<move_t> attacks;
  attacks.reserve(64);  // worst case

  // Removing the king from the board.
  const piece_t king_to_remove = (target_color == WHITE) ? B_KING : W_KING;

  // Store the king pos
  const index_t king_index =
      get_king_index(get_piece_color(king_to_remove), &tmp_board);

  // Unset the king
  (void)remove_piece(king_index, &tmp_board);

  // Generate the moves
  for (index_t i = 0; i < BOARD_SIZE; ++i) {
    const piece_t p = tmp_board.board[i];

    // Iterate over opposite color pieces
    if (p != INVALID && p != EMPTY && target_color == get_piece_color(p)) {
      // Handle Pawn move separately
      std::vector<move_t> moves_for_index;

      if (p == W_PAWN || p == B_PAWN) {
        moves_for_index = generate_pawn_attacks(i, target_color, &tmp_board);
      } else {
        moves_for_index = generate_pseudo_legal_moves_from_index(i, &tmp_board);
      }


      // Remove duplicates
      // for (const move_t& move : moves_for_index) {
      //   bool found = false;

      //   for (const move_t& attack : attacks) {
      //     if (attack == move) { found = true; }
      //     break;
      //   }

      //   if (!found) { attacks.push_back(move); }
      // }

      // We keep duplicates for easy double check detection.
      for (const move_t& move : moves_for_index) {
        attacks.push_back(move);
      }
    }
  }

  // Reset the king to the board
  put_piece(king_index, king_to_remove, &tmp_board);

  return attacks;
}


bool is_index_attacked(index_t index, const std::vector<move_t>* attacks)
{
  for (const move_t& attack : *attacks) {
    if (attack.to == index) { return true; }
  }

  return false;
}


bool is_castling_valid(const move_t* move, const std::vector<move_t>* attacks)
{
  assert(move->castling_move);

  // TODO: Make it more professional! We can generalize those cases
  switch (move->to) {
    case 0x02:
      assert(move->from == 0x04);
      // White long castling. Target squares 0x04 0x03 0x02
      if (is_index_attacked(0x04, attacks) ||
          is_index_attacked(0x03, attacks) ||
          is_index_attacked(0x02, attacks)) {
        return false;
      }
      break;
    case 0x06:
      assert(move->from == 0x04);
      // White short castling. Target squares 0x04 0x05 0x06
      if (is_index_attacked(0x04, attacks) ||
          is_index_attacked(0x05, attacks) ||
          is_index_attacked(0x06, attacks)) {
        return false;
      }
      break;
    case 0x72:
      assert(move->from == 0x74);
      // Black long castling. Target squares 0x74 0x73 0x72
      if (is_index_attacked(0x74, attacks) ||
          is_index_attacked(0x73, attacks) ||
          is_index_attacked(0x72, attacks)) {
        return false;
      }
      break;
    case 0x76:
      assert(move->from == 0x74);
      // White short castling. Target squares 0x74 0x75 0x76
      if (is_index_attacked(0x74, attacks) ||
          is_index_attacked(0x75, attacks) ||
          is_index_attacked(0x76, attacks)) {
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
  // TODO: Use make_move function
  board_t tmp_board = *board;
  tmp_board.board[move->to] = tmp_board.board[move->from];

  if (move->en_passant_capture) {
    index_t index_to_remove = move->to;
    switch (board->game_state.active_color) {
      case BLACK:
        index_to_remove += 0x10;
        break;
      case WHITE:
        index_to_remove -= 0x10;
        break;
      default:
        assert(false);
        break;
    }
    tmp_board.board[index_to_remove] = EMPTY;
  }

  tmp_board.board[move->from] = EMPTY;

  // TODO: Make this more efficient. Here we copy twice the board
  const auto& attacks_vector =
      generate_attacks_vector(!board->game_state.active_color, &tmp_board);

  for (const auto& attack_move : attacks_vector) {
    if (attack_move.to == king_index) { return true; }
  }

  return false;
}


bool is_blocking_ray(index_t index, const move_t* move)
{
  // Find the ray direction
  // Generate the ray
  // Check if index is on the ray

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


std::vector<move_t> generate_legal_moves(const board_t* board)
{
  // TODO: Reimplement this function. It contains a lot of duplicated code and
  // inefficient calls
  assert(board != nullptr);

  std::vector<move_t> result;
  result.reserve(100);

  /*****************************************************************************
   * Generate enemy attacks vector
   ****************************************************************************/
  const color_t attack_color = !board->game_state.active_color;
  const std::vector<move_t> attacks_vector =
      generate_attacks_vector(attack_color, board);

  /*****************************************************************************
   * Calculate if under check
   ****************************************************************************/
  bool under_check = false;
  bool under_double_check = false;
  const index_t king_index =
      get_king_index(board->game_state.active_color, board);

  for (const move_t& attack : attacks_vector) {
    if (attack.to == king_index) {
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

    if (under_double_check) {  // DOUBLE CHECK
      // In the case of double check we consider only king moves
      const auto king_moves =
          generate_king(king_index, board->game_state.active_color, board);

      for (const move_t& king_move : king_moves) {
        // Remove king moves that put him back in check
        bool should_discard = false;
        for (const auto& enemy_attack_move : attacks_vector) {
          if (king_move.to == enemy_attack_move.to) {
            should_discard = true;
            break;
          }
        }

        // Check if this move is castling, if so remove it
        if (king_move.castling_move) { should_discard = true; }

        // Insert only valid
        if (!should_discard) { result.push_back(king_move); }
      }
    } else {  // SINGE CHECK
      // King moves away from check
      {
        const auto king_moves =
            generate_king(king_index, board->game_state.active_color, board);

        for (const move_t& king_move : king_moves) {
          // Remove king moves that put him back in check
          bool should_discard = false;
          for (const auto& enemy_attack_move : attacks_vector) {
            if (king_move.to == enemy_attack_move.to) {
              should_discard = true;
              break;
            }
          }

          // Remove the captures that put him back in check
          if (king_move.captured != INVALID) {
            // Remove the piece from the board and see if that square is under
            // attack
            board_t tmp_board = *board;
            tmp_board.board[king_move.to] = EMPTY;
            const auto tmp_attacks = generate_attacks_vector(
                !tmp_board.game_state.active_color, &tmp_board);

            for (const auto& tmp_attack : tmp_attacks) {
              if (tmp_attack.to == king_move.to) {
                // Discard that move
                should_discard = true;
                break;
              }
            }
          }

          // Check if this move is castling, if so remove it
          if (king_move.castling_move) { should_discard = true; }

          // Insert only valid
          if (!should_discard) { result.push_back(king_move); }
        }
      }

      // Handle: Remove the attacker moves and block attacks in rays attack
      {
        for (index_t i = 0; i < BOARD_SIZE; ++i) {
          const piece_t P = board->board[i];

          if (P != INVALID && P != EMPTY && i != king_index &&
              board->game_state.active_color == get_piece_color(P)) {
            // generate moves except for King
            const auto moves = generate_pseudo_legal_moves_from_index(i, board);

            for (const move_t& move : moves) {
              bool should_discard = true;

              for (const move_t& attack : attacks_vector) {
                // Check if the move remove the attacker
                if (attack.to == king_index && move.captured != INVALID &&
                    move.to == attack.from) {
                  should_discard = false;
                }

                // Check if the move block the ray attack
                switch (attack.piece) {
                  case W_QUEEN:
                  case B_QUEEN:
                  case W_BISHOP:
                  case B_BISHOP:
                  case W_ROOK:
                  case B_ROOK:
                    if (is_blocking_ray(move.to, &attack)) {
                      should_discard = false;
                    }
                    break;

                  default:
                    break;
                }
              }

              if (!should_discard) {
                if (!is_pin(&move, king_index, board)) {
                  result.push_back(move);
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

    for (index_t i = 0; i < BOARD_SIZE; ++i) {
      const piece_t P = board->board[i];

      if (P != INVALID && P != EMPTY &&
          board->game_state.active_color == get_piece_color(P)) {
        // generate moves
        const auto moves = generate_pseudo_legal_moves_from_index(i, board);

        switch (P) {
          case B_KING:
          case W_KING: {
            // Handle king moves.

            for (const move_t& move : moves) {
              bool found = false;

              // Remove all the KING moves that move him under attack
              for (const auto& A : attacks_vector) {
                if (move.to == A.to) {
                  found = true;
                  break;
                }
              }

              if (!found) {
                // This does not put te king under attack. So we can proceed

                if (move.castling_move) {  // Handling castling
                  // Check if the castling move is legal.
                  if (!is_castling_valid(&move, &attacks_vector)) {
                    // If not valid castling then skip to the next one
                    continue;
                  }
                }

                if (move.captured != INVALID) {
                  // Handling king attack that put him under check

                  // Remove the target piece
                  board_t tmp_board = *board;
                  remove_piece(move.to, &tmp_board);

                  const auto tmp_attacks =
                      generate_attacks_vector(attack_color, &tmp_board);

                  // Check if the new attacks prevent this capture
                  bool should_skip_move = false;
                  for (const auto& tmp_attack_move : tmp_attacks) {
                    if (tmp_attack_move.to == move.to) {
                      // Then skip this move
                      should_skip_move = true;
                      break;
                    }
                  }

                  if (should_skip_move) { continue; }
                }

                result.push_back(move);
              }
            }
          } break;

          default: {
            // Generate others pieces moves
            for (const move_t& move : moves) {
              if (!is_pin(&move, king_index, board)) {
                // If not pin then ok
                result.push_back(move);
              }
            }
          }

          break;
        }
      }
    }
  }

  return result;
}
