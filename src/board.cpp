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

  zobrist->initialized = true;
}


void cleanup_game_state(board_t* board)
{
  assert(board != nullptr);

  board->active_color = WHITE;
  board->castling = WQ | WK | BQ | BK;
  board->halfmove_clock = 0;
  board->en_passant = INVALID_BOARD_INDEX;
  board->fullmove_counter = 1;
  board->zobrist_key = 0;

  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    if (i & 0x88) {
      board->board[i] = INVALID;
    } else {
      board->board[i] = EMPTY;
    }
  }
}


uint64_t init_zobrist_key(const global_state_t* state, const board_t* board)
{
  assert(state != nullptr);
  assert(board != nullptr);

  uint64_t key = 0;

  // Xor pieces on the board
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      index_t index = position_to_index(file, rank);
      piece_t piece = board->board[index];

      if (piece != EMPTY && piece != INVALID) {
        key ^= state->zobrist_randoms.piece_randoms[piece][index];
      }
    }
  }

  // Xor side to move
  key ^= state->zobrist_randoms.side_randoms[board->active_color];

  // Xor castling
  key ^= state->zobrist_randoms.castling_randoms[board->castling];

  // Xor en-passant
  key ^= state->zobrist_randoms.ep_randoms[board->en_passant];


  return key;
}


void init_board(const std::string& fen, board_t* board, global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

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
  cleanup_game_state(board);

  // Init random numbers
  if (!state->zobrist_randoms.initialized) {
    init_zobrist(&state->zobrist_randoms);
  }

  // Load FEN
  load_FEN(fen, board, state);

  // Cleanup repetition table
  state->repetition_size = 0;
  state->history_size = 0;
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


// TODO: Test this one
bool is_square_attacked(index_t sq, color_t by_color, const board_t* b)
{
  static constexpr int KNIGHT_OFFSETS[8] = {+0x21, +0x1F, +0x0E, +0xEE,
                                            +0xDF, +0xE1, +0xF2, +0x12};
  static constexpr int PAWN_OFFSETS_WHITE[2] = {+0x0F,
                                                +0x11};  // from pawn to king
  static constexpr int PAWN_OFFSETS_BLACK[2] = {-0x0F, -0x11};
  static constexpr int SLIDE_DIRS[8] = {+0x10, -0x10, +0x01, -0x01,
                                        +0x11, +0x0F, -0x0F, -0x11};

  // Pawn attacks
  int const* pawn_off =
      (by_color == WHITE ? PAWN_OFFSETS_WHITE : PAWN_OFFSETS_BLACK);
  for (int i = 0; i < 2; ++i) {
    const index_t t = sq + pawn_off[i];
    if (!(t & 0x88) && b->board[t] == (by_color == WHITE ? W_PAWN : B_PAWN))
      return true;
  }

  // Knight attacks
  for (int off : KNIGHT_OFFSETS) {
    const index_t t = sq + off;
    if (!(t & 0x88) && b->board[t] == (by_color == WHITE ? W_KNIGHT : B_KNIGHT))
      return true;
  }

  // King adjacency (rarely needed except double-check detection)
  for (int off : SLIDE_DIRS) {
    const index_t t = sq + off;
    if (!(t & 0x88) && b->board[t] == (by_color == WHITE ? W_KING : B_KING))
      return true;
  }

  // Sliding attacks
  for (int d = 0; d < 8; ++d) {
    const int dir = SLIDE_DIRS[d];
    index_t t = sq;

    while (true) {
      t += dir;

      if (t & 0x88) { break; }

      const piece_t p = b->board[t];
      if (p != EMPTY) {
        if (get_piece_color(p) == by_color) {
          bool is_rook_dir = (d < 4);
          bool is_bishop_dir = (d >= 4);

          if ((is_rook_dir &&
               (p == W_ROOK || p == B_ROOK || p == W_QUEEN || p == B_QUEEN)) ||
              (is_bishop_dir && (p == W_BISHOP || p == B_BISHOP ||
                                 p == W_QUEEN || p == B_QUEEN)))
            return true;
        }
        break;
      }
    }
  }

  return false;
}


bool make_move(const move_t* move, board_t* board, global_state_t* state)
{
  assert(move != nullptr);
  assert(board != nullptr);
  assert(move->captured != EMPTY);
  assert(board->board[move->from] != EMPTY &&
         board->board[move->from] != INVALID);
  assert(state != nullptr);

  const uint64_t old_hash = board->zobrist_key;

  // Store the history
  state->history[state->history_size] = {*board, state->repetition_size};
  state->history_size++;

  // Remove en-passant
  clear_ep_square(board, state);

  // Check move type
  if (move->captured != INVALID) {
    // In case of attack remove the piece from the board.
    if (move->en_passant_capture) {
      // In case of capture by en-passant we take the correct pawn
      index_t index_to_remove = move->to;
      switch (board->active_color) {
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
             (board->active_color == WHITE ? B_PAWN : W_PAWN));

      const piece_t removed =
          remove_piece(index_to_remove, board, &state->zobrist_randoms);
      assert(removed == move->captured);
      (void)removed;
    } else {
      const piece_t removed =
          remove_piece(move->to, board, &state->zobrist_randoms);
      assert(removed == move->captured);
      (void)removed;
    }
  }

  // Move the moving piece
  const piece_t moved_piece =
      move_piece(move->from, move->to, board, &state->zobrist_randoms);
  assert(moved_piece == move->piece);
  (void)moved_piece;

  // Handle promotion
  if (move->promoted_to != TO_NONE) {
    const piece_t removed =
        remove_piece(move->to, board, &state->zobrist_randoms);
    assert(removed == (board->active_color == WHITE ? W_PAWN : B_PAWN));
    (void)removed;

    switch (move->promoted_to) {
      case TO_QUEEN:
        put_piece(move->to, (board->active_color == WHITE ? W_QUEEN : B_QUEEN),
                  board, &state->zobrist_randoms);
        break;
      case TO_KNIGHT:
        put_piece(move->to,
                  (board->active_color == WHITE ? W_KNIGHT : B_KNIGHT), board,
                  &state->zobrist_randoms);
        break;
      case TO_ROOK:
        put_piece(move->to, (board->active_color == WHITE ? W_ROOK : B_ROOK),
                  board, &state->zobrist_randoms);
        break;
      case TO_BISHOP:
        put_piece(move->to,
                  (board->active_color == WHITE ? W_BISHOP : B_BISHOP), board,
                  &state->zobrist_randoms);
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

    // if (board->active_color == WHITE) {
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

    if (board->active_color == WHITE) {
      const index_t en_passant_index = move->to - 0x10;
      assert(!(en_passant_index & 0x88));
      set_en_passant(en_passant_index, board, state);
    } else {
      const index_t en_passant_index = move->to + 0x10;
      assert(!(en_passant_index & 0x88));
      set_en_passant(en_passant_index, board, state);
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
        move_piece(0x00, 0x03, board, &state->zobrist_randoms);
      } else if (move->to == 0x06) {
        // White king side castling
        assert(board->board[0x07] == W_ROOK);
        assert(board->board[0x05] == EMPTY);
        move_piece(0x07, 0x05, board, &state->zobrist_randoms);
      }

    } else if (move->piece == B_KING) {
      assert(move->from == 0x74);

      if (move->to == 0x72) {
        // Black queen side castling
        assert(board->board[0x70] == B_ROOK);
        assert(board->board[0x73] == EMPTY);
        move_piece(0x70, 0x73, board, &state->zobrist_randoms);
      } else if (move->to == 0x76) {
        // Black king side castling
        assert(board->board[0x77] == B_ROOK);
        assert(board->board[0x75] == EMPTY);
        move_piece(0x77, 0x75, board, &state->zobrist_randoms);
      }
    }
  }

  // Clear castling rights in case of king or rook move from initial square
  if (move->piece == W_KING && move->from == 0x04) {
    const castling_t new_castling_rights = board->castling & ~(WQ | WK);
    update_castling_permissions(new_castling_rights, board, state);
  } else if (move->piece == B_KING && move->from == 0x74) {
    const castling_t new_castling_rights = board->castling & ~(BQ | BK);
    update_castling_permissions(new_castling_rights, board, state);
  } else if (move->piece == W_ROOK) {
    if (move->from == 0x00) {
      const castling_t new_castling_rights = board->castling & ~WQ;
      update_castling_permissions(new_castling_rights, board, state);
    } else if (move->from == 0x07) {
      const castling_t new_castling_rights = board->castling & ~WK;
      update_castling_permissions(new_castling_rights, board, state);
    }
  } else if (move->piece == B_ROOK) {
    if (move->from == 0x70) {
      const castling_t new_castling_rights = board->castling & ~BQ;
      update_castling_permissions(new_castling_rights, board, state);
    } else if (move->from == 0x77) {
      const castling_t new_castling_rights = board->castling & ~BK;
      update_castling_permissions(new_castling_rights, board, state);
    }
  }

  // Clear castling rights for the opponent if capture rook
  if (move->captured == W_ROOK) {
    if (move->to == 0x00) {
      const castling_t new_castling_rights = board->castling & ~WQ;
      update_castling_permissions(new_castling_rights, board, state);
    } else if (move->to == 0x07) {
      const castling_t new_castling_rights = board->castling & ~WK;
      update_castling_permissions(new_castling_rights, board, state);
    }
  } else if (move->captured == B_ROOK) {
    if (move->to == 0x70) {
      const castling_t new_castling_rights = board->castling & ~BQ;
      update_castling_permissions(new_castling_rights, board, state);
    } else if (move->to == 0x77) {
      const castling_t new_castling_rights = board->castling & ~BK;
      update_castling_permissions(new_castling_rights, board, state);
    }
  }

  // Update half move. Is reset after captures or pawn moves, incremented in
  // all other moves
  if (move->captured != INVALID || move->piece == W_PAWN ||
      move->piece == B_PAWN) {
    board->halfmove_clock = 0;
  } else {
    board->halfmove_clock += 1;
  }

  // Update fullmove counter
  if (board->active_color == BLACK) {
    // The fullmove counter is incremented after block move
    board->fullmove_counter += 1;
  }

  // Swap side
  swap_side(board, state);

  // Store the old position
  state->repetitions[state->repetition_size] = old_hash;
  state->repetition_size++;

  assert(state->repetition_size <
         sizeof(state->repetitions) / sizeof(state->repetitions[0]));

  // TODO: Deal with overflow. Or stop writing repetitions but not crash or
  // implement some swapping logic. Like forgot old moves
  if (state->repetition_size >=
      (sizeof(state->repetitions) / sizeof(state->repetitions[0])) - 1) {
    // Reset the size to 0. This way we cut off the old repetitions but that's
    // better then crash
    state->repetition_size = 0;
  }

  return true;
}


bool unmake_move(board_t* board, global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

  // In case no move was made return false
  if (state->history_size == 0) { return false; }

  // Decrement counter
  state->history_size--;

  // Restore state
  *board = state->history[state->history_size].board;
  state->repetition_size = state->history[state->history_size].repetition_size;

  return true;
}


void reset(board_t* board, global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

  cleanup_game_state(board);
  state->repetition_size = 0;
  state->history_size = 0;

  // Load FEN
  // load_FEN(board->initial_fen, board);

  // Init Zobrist
  board->zobrist_key = init_zobrist_key(state, board);
}


void load_FEN(const std::string& FEN, board_t* board, global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);
  reset(board, state);

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
      board->active_color = WHITE;
      break;
    case 'b':
      board->active_color = BLACK;
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

  board->castling = 0x00;
  for (const char c : sections[2]) {
    switch (c) {
      case '-':
        board->castling = 0x00;
        if (sections[2].size() != 1) {
          throw FAN_exception(
              "Invalid castling availability section size. No castling "
              "available char is set but the section size is too big. FEN: " +
              FEN);
        }
        break;
      case 'K':
        board->castling |= WK;
        break;
      case 'Q':
        board->castling |= WQ;
        break;
      case 'k':
        board->castling |= BK;
        break;
      case 'q':
        board->castling |= BQ;
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

  board->en_passant = string_coordinates_to_index(sections[3]);

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
    board->halfmove_clock = static_cast<int>(std::stoul(half_move));
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
    board->fullmove_counter = static_cast<int>(std::stoul(full_move));
  } catch (std::exception& e) {
    throw FAN_exception("Can't convert Fullmove number to integer. What: " +
                        std::string(e.what()) + " FEN: " + FEN);
  }

  if (board->fullmove_counter < 1) {
    throw FAN_exception("Fullmove number can't be less then 1 but it is " +
                        std::string(STR(board->fullmove_counter)) +
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
  board->zobrist_key = init_zobrist_key(state, board);
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
  ss << (board->active_color == color_t::WHITE ? " w " : " b ");

  // Step 3: Castling rights
  bool has_castling_rights = false;

  if (board->castling & WK) {
    ss << 'K';
    has_castling_rights = true;
  }

  if (board->castling & WQ) {
    ss << 'Q';
    has_castling_rights = true;
  }

  if (board->castling & BK) {
    ss << 'k';
    has_castling_rights = true;
  }

  if (board->castling & BQ) {
    ss << 'q';
    has_castling_rights = true;
  }

  if (!has_castling_rights) { ss << '-'; }

  ss << ' ';

  // Step 4: En passant target square
  if (board->en_passant != INVALID_BOARD_INDEX) {
    ss << index_to_string_coordinates(board->en_passant);
  } else {
    ss << '-';
  }

  ss << ' ';

  // Step 5: Halfmove clock
  ss << int(board->halfmove_clock) << ' ';

  // Step 6: Fullmove number
  ss << int(board->fullmove_counter);

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


piece_t remove_piece(index_t remove_at,
                     board_t* board,
                     const zobrist_randoms_t* randoms)
{
  assert(board != nullptr);
  assert(randoms != nullptr);
  assert(remove_at < BOARD_SIZE);
  assert(index_to_position(remove_at).file < 8);
  assert(index_to_position(remove_at).rank < 8);

  const piece_t removed = board->board[remove_at];
  if (removed != INVALID && removed != EMPTY) {
    // Remove the piece from the board
    board->board[remove_at] = EMPTY;

    // Update the Zobrist
    board->zobrist_key ^= randoms->piece_randoms[removed][remove_at];
  }

  return removed;
}


void put_piece(index_t put_at,
               piece_t piece,
               board_t* board,
               const zobrist_randoms_t* randoms)
{
  assert(randoms != nullptr);
  assert(board != nullptr);
  assert(put_at < BOARD_SIZE);
  assert(piece != INVALID && piece != EMPTY);
  assert(index_to_position(put_at).file < 8);
  assert(index_to_position(put_at).rank < 8);

  board->board[put_at] = piece;

  board->zobrist_key ^= randoms->piece_randoms[piece][put_at];
}


piece_t move_piece(index_t from,
                   index_t to,
                   board_t* board,
                   const zobrist_randoms_t* randoms)
{
  assert(board != nullptr);
  assert(randoms != nullptr);
  assert(from != to);
  assert(from < BOARD_SIZE);
  assert(index_to_position(from).file < 8);
  assert(index_to_position(from).rank < 8);
  assert(to < BOARD_SIZE);
  assert(index_to_position(to).file < 8);
  assert(index_to_position(to).rank < 8);

  const piece_t piece = remove_piece(from, board, randoms);
  put_piece(to, piece, board, randoms);

  return piece;
}


void set_en_passant(index_t index, board_t* board, const global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);
  assert(index < BOARD_SIZE);
  assert(index_to_position(index).file < 8);
  assert(index_to_position(index).rank < 8);

  if (index != INVALID_BOARD_INDEX) {
    // Remove the old en passant from hash
    board->zobrist_key ^= state->zobrist_randoms.ep_randoms[board->en_passant];

    // Set teh en passant target
    board->en_passant = index;
    void swap_side(board_t * board);
    // Set the new en passant to the hash
    board->zobrist_key ^= state->zobrist_randoms.ep_randoms[board->en_passant];
  }
}


void clear_ep_square(board_t* board, const global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

  board->zobrist_key ^= state->zobrist_randoms.ep_randoms[board->en_passant];

  board->en_passant = INVALID_BOARD_INDEX;

  board->zobrist_key ^= state->zobrist_randoms.ep_randoms[board->en_passant];
}


void swap_side(board_t* board, const global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

  // Remove the side to move from hash
  board->zobrist_key ^=
      state->zobrist_randoms.side_randoms[board->active_color];

  // Change color
  board->active_color = !board->active_color;

  // Hash the new color
  board->zobrist_key ^=
      state->zobrist_randoms.side_randoms[board->active_color];
}


void update_castling_permissions(castling_t new_castling,
                                 board_t* board,
                                 const global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

  board->zobrist_key ^=
      state->zobrist_randoms.castling_randoms[board->castling];

  board->castling = new_castling;

  board->zobrist_key ^=
      state->zobrist_randoms.castling_randoms[board->castling];
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


bool is_position_repeated(const board_t* board, const global_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

  for (size_t i = 0; i < state->repetition_size; ++i) {
    if (board->zobrist_key == state->repetitions[i]) { return true; }
  }

  return false;
}


bool is_double_pawn(index_t index, const board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  const piece_t to_check = board->board[index];
  if (to_check != W_PAWN && to_check != B_PAWN) { return false; }

  // Set rank to zero
  const position_t pos = index_to_position(index);
  const index_t start_index = position_to_index(pos.file, 0);

  for (index_t i = start_index; !(i & 0x88) && i <= start_index + 0x70;
       i += 0x10) {
    if (i != index && board->board[i] == to_check && !(i & 0x88)) {
      return true;
    }
  }

  return false;
}


bool is_passed_pawn(index_t index, const board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  switch (board->board[index]) {
    case W_PAWN:
      for (index_t i = index + 0x10; !(i & 0x88) && i < BOARD_SIZE; i += 0x10) {
        if (!(i & 0x88) && board->board[i] == B_PAWN) { return false; }

        if (!((i + 0x01) & 0x88) && board->board[i + 0x01] == B_PAWN) {
          return false;
        }

        if (!((i - 0x01) & 0x88) && board->board[i - 0x01] == B_PAWN) {
          return false;
        }
      }
      break;
    case B_PAWN:
      for (index_t i = index - 0x10; !(i & 0x88) && i < BOARD_SIZE; i -= 0x10) {
        if (!(i & 0x88) && board->board[i] == W_PAWN) { return false; }

        if (!((i + 0x01) & 0x88) && board->board[i + 0x01] == W_PAWN) {
          return false;
        }

        if (!((i - 0x01) & 0x88) && board->board[i - 0x01] == W_PAWN) {
          return false;
        }
      }
      break;

    default:
      return false;
      break;
  }

  return true;
}


bool is_isolated_pawn(index_t index, const board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  const piece_t to_check = board->board[index];
  if (to_check != W_PAWN && to_check != B_PAWN) { return false; }

  // Set rank to zero
  const position_t pos = index_to_position(index);
  const index_t start_index = position_to_index(pos.file, 0);

  for (index_t i = start_index; !(i & 0x88) && i <= start_index + 0x70;
       i += 0x10) {
    if (!((i + 0x01) & 0x88) && board->board[i + 0x01] == to_check) {
      return false;
    }

    if (!((i - 0x01) & 0x88) && board->board[i - 0x01] == to_check) {
      return false;
    }
  }

  return true;
}


piece_count_t count_pieces_on_file(index_t index, const board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  piece_count_t result = {};
  const position_t pos = index_to_position(index);
  const index_t start_index = position_to_index(pos.file, 0);

  for (index_t i = start_index; !(i & 0x88) && i <= start_index + 0x70;
       i += 0x10) {
    if (i != index && board->board[i] != INVALID && board->board[i] != EMPTY) {
      if (get_piece_color(board->board[i]) == WHITE) {
        result.white += 1;
      } else {
        result.black += 1;
      }
    }
  }

  return result;
}


bool is_king_shielded(index_t index, const board_t* board)
{
  assert(board != nullptr);
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  const piece_t king = board->board[index];

  if (king == W_KING) {
    // index + 0x0F
    {
      if ((index + 0x0F) & 0x88) { return false; }
      const piece_t p = board->board[index + 0x0F];
      if (p == INVALID || p == EMPTY || get_piece_color(p) == BLACK) {
        return false;
      }
    }
    // index + 0x10
    {
      if ((index + 0x10) & 0x88) { return false; }
      const piece_t p = board->board[index + 0x10];
      if (p == INVALID || p == EMPTY || get_piece_color(p) == BLACK) {
        return false;
      }
    }
    // index + 0x11
    {
      if ((index + 0x11) & 0x88) { return false; }
      const piece_t p = board->board[index + 0x11];
      if (p == INVALID || p == EMPTY || get_piece_color(p) == BLACK) {
        return false;
      }
    }

    return true;
  } else if (king == B_KING) {
    // index - 0x0F
    {
      if ((index - 0x0F) & 0x88) { return false; }
      const piece_t p = board->board[index - 0x0F];
      if (p == INVALID || p == EMPTY || get_piece_color(p) == WHITE) {
        return false;
      }
    }
    // index - 0x10
    {
      if ((index - 0x10) & 0x88) { return false; }
      const piece_t p = board->board[index - 0x10];
      if (p == INVALID || p == EMPTY || get_piece_color(p) == WHITE) {
        return false;
      }
    }
    // index - 0x11
    {
      if ((index - 0x11) & 0x88) { return false; }
      const piece_t p = board->board[index - 0x11];
      if (p == INVALID || p == EMPTY || get_piece_color(p) == WHITE) {
        return false;
      }
    }

    return true;
  }

  return false;
}
