#include "board.hpp"
#include <array>
#include <cassert>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "data_structures.hpp"
#include "exceptions.hpp"
#include "utils.hpp"


bool is_uint(const std::string& str)
{
  for (const char c : str) {
    if (!isdigit(c)) { return false; }
  }

  return true;
}


std::vector<std::string> split_string(const std::string& str)
{
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> tokens(begin, end);

  return tokens;
}


void init_zobrist(zobrist_randoms_t* zobrist)
{
  assert(zobrist != nullptr);

  // TODO(max): Move random initialization outside
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

  for (auto& piece_array : zobrist->piece_randoms) {
    for (uint64_t& random : piece_array) {
      random = dist(gen);
    }
  }

  for (uint64_t& random : zobrist->castling_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->side_randoms) {
    random = dist(gen);
  }

  for (uint64_t& random : zobrist->ep_randoms) {
    random = dist(gen);
  }
}


void cleanup_game_state(game_state_t* gs)
{
  assert(gs != nullptr);

  gs->active_color = WHITE;
  gs->castling = WQ | WK | BQ | BK;
  gs->halfmove_clock = 0;
  gs->en_passant = INVALID_BOARD_INDEX;
  gs->fullmove_counter = 1;
  gs->zobrist_key = 0;
  // gs->next_move = move_t();
}


uint64_t init_zobrist_key(const board_t* board)
{
  assert(board != nullptr);

  uint64_t key = 0;

  // Xor pieces on the board
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      index_t index = position_to_index(file, rank);
      piece_t piece = board->board[index];

      if (piece != EMPTY && piece != INVALID) {
        key ^= board->zobrist_randoms.piece_randoms[piece][index];
      }
    }
  }

  // Xor side to move
  key ^= board->zobrist_randoms.side_randoms[board->game_state.active_color];

  // Xor castling
  key ^= board->zobrist_randoms.castling_randoms[board->game_state.castling];

  // Xor en-passant
  key ^= board->zobrist_randoms.ep_randoms[board->game_state.en_passant];


  return key;
}


color_t us(const board_t* board)
{
  return board->game_state.active_color;
}


color_t opponent(const board_t* board)
{
  return !board->game_state.active_color;
}


void init_board(const std::string& fen, board_t* board, history_t* history)
{
  assert(board != nullptr);
  assert(history != nullptr);

  // Cleanup
  for (index_t i = 0; i < BOARD_SIZE; ++i) {
    if (i & 0x88) {
      board->board[i] = INVALID;
    } else {
      board->board[i] = EMPTY;
    }
  }

  // history_t empty_history = history_t();
  // board->history.swap(empty_history);
  cleanup_game_state(&board->game_state);

  // Init random numbers
  init_zobrist(&board->zobrist_randoms);

  // Load FEN
  load_FEN(fen, board, history);

  // Init Zobrist. This is done already in the load_FEN function
  // board->game_state.zobrist_key = init_zobrist_key(board);

  // TODO: Init phase_value
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


bool make_move(const move_t* move, board_t* board, history_t* history)
{
  assert(move != nullptr);
  assert(board != nullptr);
  assert(move->captured != EMPTY);
  assert(board->board[move->from] != EMPTY &&
         board->board[move->from] != INVALID);

  // TODO: This function works only with legal moves. Should return false with
  // illegal

  // OLD history
  // const history_entry_t history_entry = {board->game_state, *move};

  // Store the history
  if (history != nullptr) { history->push({*board, *move}); }

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
      (void)removed;
    } else {
      const piece_t removed = remove_piece(move->to, board);
      assert(removed == move->captured);
      (void)removed;
    }
  }

  // Move the moving piece
  const piece_t moved_piece = move_piece(move->from, move->to, board);
  assert(moved_piece == move->piece);
  (void)moved_piece;

  // Handle promotion
  if (move->promoted_to != TO_NONE) {
    const piece_t removed = remove_piece(move->to, board);
    assert(removed ==
           (board->game_state.active_color == WHITE ? W_PAWN : B_PAWN));
    (void)removed;

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
    // The uncommented code set the en-passand at each pawn double push

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

  // OLD
  // Store the history
  // if (history != nullptr) { history->push(history_entry); }

  return true;
}


bool unmake_move(board_t* board, history_t* history)
{
  assert(board != nullptr);
  assert(history != nullptr);

  // In case no move was made return false
  if (history->empty()) { return false; }

  // Restore state
  *board = history->top().board;

  // Remove from stack
  history->pop();

  return true;

  /** OLD WAY

  // In case no move was made return false
  if (history->empty()) { return false; }

  const history_entry_t& history_entry = history->top();

  const game_state_t& previous_game_state = history_entry.game_state;
  const move_t& move_to_unmake = history_entry.move_applied;

  assert(move_to_unmake.from != INVALID_BOARD_INDEX);
  assert(move_to_unmake.to != INVALID_BOARD_INDEX);
  assert(move_to_unmake.piece != INVALID);
  assert(move_to_unmake.piece != EMPTY);
  assert(board->board[move_to_unmake.from] == EMPTY);

  assert(move_to_unmake.captured != EMPTY);

  // Handle castling
  if (move_to_unmake.castling_move) {
    if (move_to_unmake.piece == W_KING) {
      assert(move_to_unmake.from == 0x04);
      assert(move_to_unmake.piece == W_KING);
      assert(previous_game_state.active_color == WHITE);

      if (move_to_unmake.to == 0x02) {
        // White queen side castling
        assert(board->board[0x03] == W_ROOK);
        assert(board->board[0x02] == W_KING);
        assert(board->board[board->board[move_to_unmake.from]] == EMPTY);
        assert(board->board[0x00] == EMPTY);
        assert(board->board[0x01] == EMPTY);

        // Reset the pieces
        board->board[move_to_unmake.from] = W_KING;
        board->board[0x00] = W_ROOK;
        board->board[0x02] = EMPTY;
        board->board[0x03] = EMPTY;

      } else if (move_to_unmake.to == 0x06) {
        // White king side castling
        assert(board->board[0x05] == W_ROOK);
        assert(board->board[0x06] == W_KING);
        assert(board->board[board->board[move_to_unmake.from]] == EMPTY);
        assert(board->board[0x07] == EMPTY);

        // Reset the pieces
        board->board[move_to_unmake.from] = W_KING;
        board->board[0x07] = W_ROOK;
        board->board[0x05] = EMPTY;
        board->board[0x06] = EMPTY;
      }

    } else if (move_to_unmake.piece == B_KING) {
      assert(move_to_unmake.from == 0x74);
      assert(move_to_unmake.piece == B_KING);
      assert(previous_game_state.active_color == BLACK);

      if (move_to_unmake.to == 0x72) {
        // Black queen side castling
        assert(board->board[0x73] == B_ROOK);
        assert(board->board[0x72] == B_KING);
        assert(board->board[board->board[move_to_unmake.from]] == EMPTY);
        assert(board->board[0x70] == EMPTY);
        assert(board->board[0x71] == EMPTY);

        // Reset the pieces
        board->board[move_to_unmake.from] = B_KING;
        board->board[0x70] = B_ROOK;
        board->board[0x72] = EMPTY;
        board->board[0x73] = EMPTY;
      } else if (move_to_unmake.to == 0x76) {
        // Black king side castling
        assert(board->board[0x75] == B_ROOK);
        assert(board->board[0x76] == B_KING);
        assert(board->board[board->board[move_to_unmake.from]] == EMPTY);
        assert(board->board[0x77] == EMPTY);

        // Reset the pieces
        board->board[move_to_unmake.from] = B_KING;
        board->board[0x77] = B_ROOK;
        board->board[0x75] = EMPTY;
        board->board[0x76] = EMPTY;
      }
    }

  } else if (move_to_unmake.en_passant_capture) {
    // Handling en-passant
    assert(previous_game_state.en_passant != INVALID_BOARD_INDEX);
    assert(board->board[previous_game_state.en_passant] == W_PAWN ||
           board->board[previous_game_state.en_passant] == B_PAWN);
    assert(previous_game_state.en_passant == move_to_unmake.to);


    if (previous_game_state.active_color == WHITE) {
      board->board[move_to_unmake.to] = EMPTY;
      board->board[move_to_unmake.from] = W_PAWN;
      board->board[move_to_unmake.to - 0x10] = B_PAWN;
    } else {
      board->board[move_to_unmake.to] = EMPTY;
      board->board[move_to_unmake.from] = B_PAWN;
      board->board[move_to_unmake.to + 0x10] = W_PAWN;
    }
  } else {
    // Normal move, normal capture, promotion and promotion with capture are
    // handled all in the same way
    const piece_t removed_piece =
        (move_to_unmake.captured == INVALID) ? EMPTY : move_to_unmake.captured;


    board->board[move_to_unmake.to] = removed_piece;
    board->board[move_to_unmake.from] = move_to_unmake.piece;
  }

  // Restore state
  board->game_state = previous_game_state;
  history->pop();

  return true;
  */
}


void reset(board_t* board, history_t* history)
{
  assert(board != nullptr);
  assert(history != nullptr);

  // Cleanup
  for (index_t i = 0; i < BOARD_SIZE; ++i) {
    if (i & 0x88) {
      board->board[i] = INVALID;
    } else {
      board->board[i] = EMPTY;
    }
  }

  *history = history_t();
  cleanup_game_state(&board->game_state);

  // Load FEN
  // load_FEN(board->initial_fen, board);

  // Init Zobrist
  board->game_state.zobrist_key = init_zobrist_key(board);
}


void load_FEN(const std::string& FEN, board_t* board, history_t* history)
{
  assert(board != nullptr);
  assert(history != nullptr);
  reset(board, history);

  // Start parsing
  auto sections = split_string(FEN);

  if (sections.size() != 6) { throw FAN_exception("Bad FEN string: " + FEN); }

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
        board->board[index] = char_to_piece(c);
        ++file;
      } break;

      default:
        // We get a non valid string
        throw FAN_exception("Invalid char in FEN string [" + STR(c) +
                            "]. FEN: " + FEN);
        break;
    }
  }

  /***************************************************************************
   * 1. Active color
   **************************************************************************/
  if (sections[1].size() != 1) {
    throw FAN_exception("invalid active color section. FEN: " + FEN);
  }

  char color = sections[1][0];
  switch (color) {
    case 'w':
      board->game_state.active_color = WHITE;
      break;
    case 'b':
      board->game_state.active_color = BLACK;
      break;
    default:
      throw FAN_exception("Invalid color char in FEN string [" +
                          std::string(1, color) + "]. FEN: " + FEN);
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
    throw FAN_exception("Invalid castling availability section size. FEN: " +
                        FEN);
  }

  board->game_state.castling = 0x00;
  for (const char c : sections[2]) {
    switch (c) {
      case '-':
        board->game_state.castling = 0x00;
        if (sections[2].size() != 1) {
          throw FAN_exception(
              "Invalid castling availability section size. No castling "
              "available char is set but the section size is too big. FEN: " +
              FEN);
        }
        break;
      case 'K':
        board->game_state.castling |= WK;
        break;
      case 'Q':
        board->game_state.castling |= WQ;
        break;
      case 'k':
        board->game_state.castling |= BK;
        break;
      case 'q':
        board->game_state.castling |= BQ;
        break;

      default:
        throw FAN_exception("Invalid castling availability character [" +
                            std::string(1, c) + "]. FEN: " + FEN);
    }
  }

  /***************************************************************************
   * 3. En passant target square
   *
   * "-" None
   **************************************************************************/
  if (sections[3].size() < 1 || sections[3].size() > 2) {
    throw FAN_exception("Invalid en passant section size. FEN: " + FEN);
  }

  if (sections[3].size() == 1 && sections[3][0] != '-') {
    throw FAN_exception("Invalid en passant section char [ " +
                        std::string(1, sections[3][0]) + "]. FEN: " + FEN);
  }

  if (sections[3].size() == 2) {
    if (!isalpha(sections[3][0]) || !isdigit(sections[3][1])) {
      throw FAN_exception(
          "Invalid en passant section. Wrong algebraic notation [" +
          sections[3] + "]. FEN: " + FEN);
    }
  }

  board->game_state.en_passant = string_coordinates_to_index(sections[3]);

  /***************************************************************************
   * 4. Halfmove clock
   *
   * The number of halfmoves since the last capture or pawn advance, used for
   * the fifty-move rule.
   **************************************************************************/
  const std::string& half_move = sections[4];
  if (half_move.size() < 1) {
    throw FAN_exception("Invalid Halfmove clock section size. FEN: " + FEN);
  }

  if (!is_uint(half_move)) {
    throw FAN_exception(
        "Invalid Halfmove clock section is not a number. FEN: " + FEN);
  }

  try {
    board->game_state.halfmove_clock = static_cast<int>(std::stoul(half_move));
  } catch (std::exception& e) {
    throw FAN_exception("Can't convert Halfmove clock to integer.What: " +
                        std::string(e.what()) + " FEN: " + FEN);
  }

  /***************************************************************************
   * 5. Fullmove number
   *
   * The number of the full moves. It starts at 1 and is incremented after
   * Black's move.
   **************************************************************************/
  const std::string& full_move = sections[5];
  if (full_move.size() < 1) {
    throw FAN_exception("Invalid Fullmove number section size. FEN: " + FEN);
  }

  if (!is_uint(full_move)) {
    throw FAN_exception(
        "Invalid Fullmove number section is not a number. FEN: " + FEN);
  }

  try {
    board->game_state.fullmove_counter =
        static_cast<int>(std::stoul(full_move));
  } catch (std::exception& e) {
    throw FAN_exception("Can't convert Fullmove number to integer. What: " +
                        std::string(e.what()) + " FEN: " + FEN);
  }

  if (board->game_state.fullmove_counter < 1) {
    throw FAN_exception("Fullmove number can't be less then 1 but it is " +
                        std::string(STR(board->game_state.fullmove_counter)) +
                        " FEN: " + FEN);
  }

  // TODO(Max): deal with this
  /***************************************************************************
   * Update headers
  //
  **************************************************************************/
  // if (FEN != std::string(DEFAULT_POSITION)) {
  //   header["SetUp"] = "1";
  //   header["FEN"] = FEN;
  // }

  /***************************************************************************
   * Set repetition
   **************************************************************************/
  // const std::string short_fen = generate_FEN(true);
  // repetition_position_count[short_fen] = 1;

  /***************************************************************************
   * Sanity check with FEN generation
   **************************************************************************/
  // const std::string full_FEN = generate_FEN();
  // assert(FEN == full_FEN);

  // Update Zobrist keys
  board->game_state.zobrist_key = init_zobrist_key(board);
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
      const auto piece = board->board[index];

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
  ss << (board->game_state.active_color == color_t::WHITE ? " w " : " b ");

  // Step 3: Castling rights
  bool has_castling_rights = false;

  if (board->game_state.castling & WK) {
    ss << 'K';
    has_castling_rights = true;
  }

  if (board->game_state.castling & WQ) {
    ss << 'Q';
    has_castling_rights = true;
  }

  if (board->game_state.castling & BK) {
    ss << 'k';
    has_castling_rights = true;
  }

  if (board->game_state.castling & BQ) {
    ss << 'q';
    has_castling_rights = true;
  }

  if (!has_castling_rights) { ss << '-'; }

  ss << ' ';

  // Step 4: En passant target square
  if (board->game_state.en_passant != INVALID_BOARD_INDEX) {
    ss << index_to_string_coordinates(board->game_state.en_passant);
  } else {
    ss << '-';
  }

  ss << ' ';

  // Step 5: Halfmove clock
  ss << int(board->game_state.halfmove_clock) << ' ';

  // Step 6: Fullmove number
  ss << int(board->game_state.fullmove_counter);

  return ss.str();
}


bool contains_opponent(index_t i, color_t opponent_color, const board_t* board)
{
  assert(board != nullptr);
  assert(i < BOARD_SIZE);
  assert(index_to_position(i).file < 8);
  assert(index_to_position(i).rank < 8);

  const piece_t piece = board->board[i];

  if (piece != EMPTY && piece != INVALID &&
      get_piece_color(piece) == opponent_color) {
    return true;
  }

  return false;
}


piece_t remove_piece(index_t remove_at, board_t* board)
{
  assert(board != nullptr);
  assert(remove_at < BOARD_SIZE);
  assert(index_to_position(remove_at).file < 8);
  assert(index_to_position(remove_at).rank < 8);

  const piece_t removed = board->board[remove_at];
  if (removed != INVALID && removed != EMPTY) {
    // Remove the piece from the board
    board->board[remove_at] = EMPTY;

    // Update the Zobrist
    board->game_state.zobrist_key ^=
        board->zobrist_randoms.piece_randoms[removed][remove_at];

    // TODO: Update phase_value
  }

  return removed;
}


void put_piece(index_t put_at, piece_t piece, board_t* board)
{
  assert(board != nullptr);
  assert(put_at < BOARD_SIZE);
  assert(piece != INVALID && piece != EMPTY);
  assert(index_to_position(put_at).file < 8);
  assert(index_to_position(put_at).rank < 8);

  board->board[put_at] = piece;

  board->game_state.zobrist_key ^=
      board->zobrist_randoms.piece_randoms[piece][put_at];
}


piece_t move_piece(index_t from, index_t to, board_t* board)
{
  assert(board != nullptr);
  assert(from != to);
  assert(from < BOARD_SIZE);
  assert(index_to_position(from).file < 8);
  assert(index_to_position(from).rank < 8);
  assert(to < BOARD_SIZE);
  assert(index_to_position(to).file < 8);
  assert(index_to_position(to).rank < 8);

  const piece_t piece = remove_piece(from, board);
  put_piece(to, piece, board);

  return piece;
}


void set_en_passant(index_t index, board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(index_to_position(index).file < 8);
  assert(index_to_position(index).rank < 8);

  if (index != INVALID_BOARD_INDEX) {
    // Remove the old en passant from hash
    board->game_state.zobrist_key ^=
        board->zobrist_randoms.ep_randoms[board->game_state.en_passant];

    // Set teh en passant target
    board->game_state.en_passant = index;
    void swap_side(board_t * board);
    // Set the new en passant to the hash
    board->game_state.zobrist_key ^=
        board->zobrist_randoms.ep_randoms[board->game_state.en_passant];
  }
}


void clear_ep_square(board_t* board)
{
  board->game_state.zobrist_key ^=
      board->zobrist_randoms.ep_randoms[board->game_state.en_passant];

  board->game_state.en_passant = INVALID_BOARD_INDEX;

  board->game_state.zobrist_key ^=
      board->zobrist_randoms.ep_randoms[board->game_state.en_passant];
}


void swap_side(board_t* board)
{
  // Remove the side to move from hash
  board->game_state.zobrist_key ^=
      board->zobrist_randoms.side_randoms[board->game_state.active_color];

  // Change color
  board->game_state.active_color = !board->game_state.active_color;

  // Hash the new color
  board->game_state.zobrist_key ^=
      board->zobrist_randoms.side_randoms[board->game_state.active_color];
}


void update_castling_permissions(castling_t new_castling, board_t* board)
{
  board->game_state.zobrist_key ^=
      board->zobrist_randoms.castling_randoms[board->game_state.castling];

  board->game_state.castling = new_castling;

  board->game_state.zobrist_key ^=
      board->zobrist_randoms.castling_randoms[board->game_state.castling];
}


index_t get_king_index(color_t color, const board_t* board)
{
  assert(board != nullptr);

  const piece_t king = color == WHITE ? W_KING : B_KING;

  for (index_t i = 0; i < BOARD_SIZE; ++i) {
    if (board->board[i] == king) { return i; }
  }

  assert(false);  // King should be always on the board
  return INVALID_BOARD_INDEX;
}
