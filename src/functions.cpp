#include "functions.hpp"
#include <array>
#include <cassert>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "data_structures.hpp"
#include "exceptions.hpp"
#include "move_generator.hpp"
#include "utils.hpp"


color_t us(const board_t* board)
{
  return board->game_state.active_color;
}


color_t opponent(const board_t* board)
{
  return !board->game_state.active_color;
}


void init_board(const std::string& fen, board_t* board)
{
  assert(board != nullptr);

  // Cleanup
  board->board.fill(EMPTY);
  history_t empty_history = history_t();
  board->history.swap(empty_history);
  cleanup_game_state(&board->game_state);

  // Init random numbers
  init_zobrist(&board->zobrist_randoms);

  // Load FEN
  load_FEN(fen, board);
  board->initial_fen = fen;

  // Init Zobrist
  board->game_state.zobrist_key = init_zobrist_key(board);

  // TODO: Init phase_value
}


std::array<piece_t, BOARD_SIZE> get_chess_board(const board_t* board)
{
  return board->board;
}


position_t king_square(color_t color, const board_t* board)
{
  assert(board != nullptr);

  index_t king_index = get_king_index(color, board);

  if (king_index == INVALID_BOARD_INDEX) {
    throw kin_not_on_board_exception(color_to_string(color) +
                                     " king is not ot the board.");
  }

  return index_to_position(king_index);
}


bool has_bishop_pair(color_t color, const board_t* board)
{
  piece_t bishop_to_search = B_BISHOP;
  if (color == WHITE) { bishop_to_search = W_BISHOP; }

  bool found[2] = {false, false};

  for (uint8_t index = 0; index < BOARD_SIZE; ++index) {
    if (board->board[index] == bishop_to_search) {
      color_t c = get_square_color(index);
      found[c] = true;
    }
  }


  return (found[0] && found[1]);
}


/**
 * @brief Returns the indexes of the ambiguous moves.
 */
std::vector<size_t> get_ambiguous_move(const move_t* move,
                                       const std::vector<move_t>* moves)
{
  assert(moves != nullptr);

  std::vector<size_t> result;
  for (size_t i = 0; i < moves->size(); ++i) {
    const auto& I = moves->at(i);

    // If same piece, same destination and different source
    if (move->piece == I.piece && move->to == I.to && move->from != I.from) {
      result.push_back(i);
    }
  }

  return result;
}


std::string move_to_algebraic(const move_t* move,
                              const std::vector<move_t>* moves,
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
    const auto ambiguous_moves = get_ambiguous_move(move, moves);
    if (ambiguous_moves.size() > 0) {
      const position_t move_from_pos = index_to_position(move->from);
      bool is_file_unique = true;
      bool is_rank_unique = true;

      // Check if file or rank are unique for the move->from
      for (size_t i : ambiguous_moves) {
        const position_t i_pos = index_to_position(moves->at(i).from);

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
  notation += index_to_algebraic(move->to);

  // Pawn captures (ex: exd5)
  if ((move->piece == W_PAWN || move->piece == B_PAWN) &&
      move->captured != INVALID) {
    notation = index_to_algebraic(move->from)[0] + std::string("x") +
               index_to_algebraic(move->to);
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
  bool move_happened = make_move(move, &tmp_board);
  assert(move_happened == true);

  // Generate moves for my color but after the current move is done
  const auto pseudo_legal_moves =
      generate_pseudo_legal_moves_from_index(move->to, &tmp_board);

  // Check if one of this moves put under check the opponent king

  for (const auto& pseudo_move : pseudo_legal_moves) {
    if (pseudo_move.to == opponent_king_index) {
      // Now let's check if this is check mate

      // Generate legal moves after the make move to see if any available.
      const auto moves = generate_legal_moves(&tmp_board);
      if (moves.size() == 0) {
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
  bool is_check = false;
  bool is_mate = false;
  bool is_capture = false;

  if (notation.back() == '+') { is_check = true; }

  if (notation.back() == '#') { is_mate = true; }

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
  index_t to_index = algebraic_to_index(dest_square);
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
  const auto& legal_moves = generate_legal_moves(board);

  bool found = false;
  for (const auto& legal_move : legal_moves) {
    if (legal_move.to == result.to && legal_move.piece == result.piece &&
        legal_move.promoted_to == result.promoted_to &&
        legal_move.captured == result.captured &&
        legal_move.castling_move == result.castling_move) {
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
    throw algebraic_exception(
        "No legal move found. Invalid Algebraic notation: " +
        original_notation);
  }

  return result;
}


bool make_move(const move_t* move, board_t* board)
{
  assert(move != nullptr);
  assert(board != nullptr);
  assert(move->captured != EMPTY);

  // TODO: This function works only with legal moves. Should return false with
  // illegal

  game_state_t state_to_history = board->game_state;
  state_to_history.next_move = *move;

  // Remove en-passant
  clear_ep_square(board);

  // Check move type
  if (move->captured != INVALID) {
    // In case of attack remove the piece from the board.
    if (move->en_passant_capture) {
      // In case of capture by en-passant we take the correct pawn
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

      // Assert if the index is on board
      assert(!(index_to_remove & 0x88));
      assert(index_to_remove != INVALID_BOARD_INDEX);
      // Assert the target contains a PAWN of the opposite color
      assert(board->board[index_to_remove] ==
             (board->game_state.active_color == WHITE ? B_PAWN : W_PAWN));

      const piece_t removed = remove_piece(index_to_remove, board);
      assert(removed == move->captured);
    } else {
      const piece_t removed = remove_piece(move->to, board);
      assert(removed == move->captured);
    }
  }

  // Move the moving piece
  const piece_t moved_piece = move_piece(move->from, move->to, board);
  assert(moved_piece == move->piece);

  // Handle promotion
  if (move->promoted_to != TO_NONE) {
    const piece_t removed = remove_piece(move->to, board);
    assert(removed ==
           (board->game_state.active_color == WHITE ? W_PAWN : B_PAWN));

    switch (move->promoted_to) {
      case TO_QUEEN:
        put_piece(move->to,
                  (board->game_state.active_color == WHITE ? W_QUEEN : B_QUEEN),
                  board);
        break;
      case TO_KNIGHT:
        put_piece(
            move->to,
            (board->game_state.active_color == WHITE ? W_KNIGHT : B_KNIGHT),
            board);
        break;
      case TO_ROOK:
        put_piece(move->to,
                  (board->game_state.active_color == WHITE ? W_ROOK : B_ROOK),
                  board);
        break;
      case TO_BISHOP:
        put_piece(
            move->to,
            (board->game_state.active_color == WHITE ? W_BISHOP : B_BISHOP),
            board);
        break;
      case TO_NONE:
      default:
        assert(false);
        break;
    }
  }


  // Set the en-passant if necessary
  if (move->double_pawn_move) {
    // NOTE: The commented code set en-passant only if real.
    // The uncommented code sed the en-passand at each pawn double push

    // const index_t on_left = move->to - 0x01;
    // const index_t on_right = move->to + 0x01;

    // if (board->game_state.active_color == WHITE) {
    //   // Handle white double push
    //   if (on_left == B_PAWN || on_right == B_PAWN) {
    //     const index_t en_passant_index = move->to - 0x10;
    //     assert(!(en_passant_index & 0x88));
    //     set_en_passant(en_passant_index, board);
    //   } else {
    //     // Handle black double push
    //     if (on_left == W_PAWN || on_right == W_PAWN) {
    //       const index_t en_passant_index = move->to + 0x10;
    //       assert(!(en_passant_index & 0x88));
    //       set_en_passant(en_passant_index, board);
    //     }
    //   }
    // }

    if (board->game_state.active_color == WHITE) {
      const index_t en_passant_index = move->to - 0x10;
      assert(!(en_passant_index & 0x88));
      set_en_passant(en_passant_index, board);
    } else {
      const index_t en_passant_index = move->to + 0x10;
      assert(!(en_passant_index & 0x88));
      set_en_passant(en_passant_index, board);
    }
  }

  // Handle castling move. The king was already moved, we need only to move the
  // rook
  if (move->castling_move) {
    if (move->piece == W_KING) {
      assert(move->from == 0x04);

      if (move->to == 0x02) {
        // White queen side castling
        assert(board->board[0x00] == W_ROOK);
        assert(board->board[0x03] == EMPTY);
        move_piece(0x00, 0x03, board);
      } else if (move->to == 0x06) {
        // White king side castling
        assert(board->board[0x07] == W_ROOK);
        assert(board->board[0x05] == EMPTY);
        move_piece(0x07, 0x05, board);
      }

    } else if (move->piece == B_KING) {
      assert(move->from == 0x74);

      if (move->to == 0x72) {
        // Black queen side castling
        assert(board->board[0x70] == B_ROOK);
        assert(board->board[0x73] == EMPTY);
        move_piece(0x70, 0x73, board);
      } else if (move->to == 0x76) {
        // Black king side castling
        assert(board->board[0x77] == B_ROOK);
        assert(board->board[0x75] == EMPTY);
        move_piece(0x77, 0x75, board);
      }
    }
  }


  // Clear castling rights in case of king or rook move from initial square
  if (move->piece == W_KING && move->from == 0x04) {
    const castling_t new_castling_rights =
        board->game_state.castling & ~(WQ | WK);
    update_castling_permissions(new_castling_rights, board);
  } else if (move->piece == B_KING && move->from == 0x74) {
    const castling_t new_castling_rights =
        board->game_state.castling & ~(BQ | BK);
    update_castling_permissions(new_castling_rights, board);
  } else if (move->piece == W_ROOK) {
    if (move->from == 0x00) {
      const castling_t new_castling_rights = board->game_state.castling & ~WQ;
      update_castling_permissions(new_castling_rights, board);
    } else if (move->from == 0x07) {
      const castling_t new_castling_rights = board->game_state.castling & ~WK;
      update_castling_permissions(new_castling_rights, board);
    }
  } else if (move->piece == B_ROOK) {
    if (move->from == 0x70) {
      const castling_t new_castling_rights = board->game_state.castling & ~BQ;
      update_castling_permissions(new_castling_rights, board);
    } else if (move->from == 0x77) {
      const castling_t new_castling_rights = board->game_state.castling & ~BK;
      update_castling_permissions(new_castling_rights, board);
    }
  }

  // Clear castling rights for the opponent if capture rook
  if (move->captured == W_ROOK) {
    if (move->to == 0x00) {
      const castling_t new_castling_rights = board->game_state.castling & ~WQ;
      update_castling_permissions(new_castling_rights, board);
    } else if (move->to == 0x07) {
      const castling_t new_castling_rights = board->game_state.castling & ~WK;
      update_castling_permissions(new_castling_rights, board);
    }
  } else if (move->captured == B_ROOK) {
    if (move->to == 0x70) {
      const castling_t new_castling_rights = board->game_state.castling & ~BQ;
      update_castling_permissions(new_castling_rights, board);
    } else if (move->to == 0x77) {
      const castling_t new_castling_rights = board->game_state.castling & ~BK;
      update_castling_permissions(new_castling_rights, board);
    }
  }

  // Update half move. Is reset after captures or pawn moves, incremented in
  // all other moves
  if (move->captured != INVALID || move->piece == W_PAWN ||
      move->piece == B_PAWN) {
    board->game_state.halfmove_clock = 0;
  } else {
    board->game_state.halfmove_clock += 1;
  }

  // Update fullmove counter
  if (board->game_state.active_color == BLACK) {
    // The fullmove counter is incremented after block move
    board->game_state.fullmove_counter += 1;
  }

  // Swap side
  swap_side(board);

  // Store the history
  board->history.push(state_to_history);

  return true;
}
