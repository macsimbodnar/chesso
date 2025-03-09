#include "utils.hpp"
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <random>
#include <sstream>
#include <unordered_map>
#include "exceptions.hpp"


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


void cleanup_game_state(game_state_t* gs)
{
  assert(gs != nullptr);

  gs->active_color = WHITE;
  gs->castling = WQ | WK | BQ | BK;
  gs->halfmove_clock = 0;
  gs->en_passant = INVALID_BOARD_INDEX;
  gs->fullmove_counter = 1;
  gs->zobrist_key = 0;
  gs->next_move = move_t();
}


index_t position_to_index(const uint8_t file, const uint8_t rank)
{
  assert(file >= 0 && file < 8);
  assert(rank >= 0 && rank < 8);

  const index_t index = (rank << 4) + file;
  assert(index < BOARD_SIZE);
  assert(!(index & 0x88));

  return index;
}


position_t index_to_position(index_t index)
{
  assert(!(index & 0x88));
  assert(index < BOARD_SIZE);

  position_t result;
  result.file = index & 7;
  result.rank = index >> 4;

  return result;
}


index_t algebraic_to_index(const std::string& p)
{
  index_t result = INVALID_BOARD_INDEX;

  if (p.size() != 2) { return INVALID_BOARD_INDEX; }

  uint8_t file = p[0];
  uint8_t rank = p[1];

  assert(file >= 'a');
  assert(file <= 'h');
  assert(rank >= '1');
  assert(rank <= '8');

  file = file - 'a';
  rank = rank - '1';

  result = position_to_index(file, rank);

  return result;
}


std::string index_to_algebraic(const index_t i)
{
  if (i == INVALID_BOARD_INDEX) { return "-"; }

  const position_t p = index_to_position(i);
  std::string result;
  result.reserve(2);

  result.push_back('a' + p.file);
  result.push_back('1' + p.rank);

  return result;
}


bool is_uint(const std::string& str)
{
  for (const char c : str) {
    if (!isdigit(c)) { return false; }
  }

  return true;
}


char piece_to_char(const piece_t piece)
{
  static const std::unordered_map<piece_t, char> piece_to_char_map = {
      {B_PAWN, 'p'},   {B_KNIGHT, 'n'}, {B_BISHOP, 'b'}, {B_ROOK, 'r'},
      {B_QUEEN, 'q'},  {B_KING, 'k'},   {W_PAWN, 'P'},   {W_KNIGHT, 'N'},
      {W_BISHOP, 'B'}, {W_ROOK, 'R'},   {W_QUEEN, 'Q'},  {W_KING, 'K'},
      {INVALID, '*'},  {EMPTY, ' '}};

  return piece_to_char_map.at(piece);
}


piece_t char_to_piece(const char c)
{
  static const std::unordered_map<char, piece_t> char_to_piece_map = {
      {'p', B_PAWN},   {'n', B_KNIGHT}, {'b', B_BISHOP}, {'r', B_ROOK},
      {'q', B_QUEEN},  {'k', B_KING},   {'P', W_PAWN},   {'N', W_KNIGHT},
      {'B', W_BISHOP}, {'R', W_ROOK},   {'Q', W_QUEEN},  {'K', W_KING},
      {'*', INVALID},  {' ', EMPTY}};

  return char_to_piece_map.at(c);
}


std::string print_board(const board_t* board)
{
  std::stringstream ss;

  for (size_t i = 8; i > 0; --i) {
    for (size_t j = 0; j < 16; ++j) {
      ss << piece_to_char(board->board[((i - 1) * 16) + j]);
    }

    ss << "\n";
  }

  return ss.str();
}

std::string print_nice_board(const board_t* board)
{
  /**
   *
   * 8  ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
   * 7  ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟
   * 6
   * 5
   * 4
   * 3
   * 2  ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
   * 1  ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖
   *
   *    A B C D E F G H
   */

  // clang-format off
  static const std::unordered_map<char, std::string> sprite_map = {
    {W_PAWN, "♟︎"},
    {W_KNIGHT, "♞"},
    {W_BISHOP, "♝"},
    {W_ROOK, "♜"},
    {W_QUEEN, "♛"},
    {W_KING, "♚"},
    {B_PAWN, "♙"},
    {B_KNIGHT, "♘"},
    {B_BISHOP, "♗"},
    {B_ROOK, "♖"},
    {B_QUEEN, "♕"},
    {B_KING, "♔"},
    {EMPTY, " "},
    {INVALID, " "},
  };
  // clang-format on

  std::stringstream ss;

  ss << "##################\n";

  for (size_t i = 8; i > 0; --i) {
    ss << i << "  ";

    for (size_t j = 0; j < 16; ++j) {
      const index_t index = ((i - 1) * 16) + j;
      const char piece = board->board[index];

      if (index & 0x88) { continue; }

      if (piece == EMPTY) {
        const uint8_t file = index & 7;
        const uint8_t rank = index >> 4;


        if ((file + rank) % 2) {
          // WHITE EMPTY SQUARE
          ss << "  ";
        } else {
          // BLACK EMPTY SQUARE
          ss << "* ";
        }
      } else {
        ss << sprite_map.at(piece) << " ";
      }


      // if (file + rank) % 2 == 0:
      //         square.color = Piece.PieceColor.WHITE
      //     else:
      //         square.color = Piece.PieceColor.BLACK
    }

    ss << "\n";
  }

  // clang-format off
  ss << "   A B C D E F G H";
  ss << "\n------------------";

  ss << "\nactive_color:      " << color_to_string(board->game_state.active_color);
  ss << "\ncastling:          " << std::bitset<4>(board->game_state.castling);
  ss << "\nhalf_move_clock:   " << int(board->game_state.halfmove_clock);
  ss << "\nen_passant:        " << index_to_algebraic(board->game_state.en_passant);
  ss << "\nfull_move_number:  " << int(board->game_state.fullmove_counter);
  ss << "\nzobrist_key:       " << board->game_state.zobrist_key;
  // ss << "\nphase_value:       " << int(board->game_state.phase_value);
  // ss << "next_move:         " << board->game_state.next_move;

  ss << "\n##################";
  // clang-format on

  return ss.str();
}


void load_FEN(const std::string& FEN, board_t* board)
{
  reset(board);

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

  board->game_state.en_passant = algebraic_to_index(sections[3]);

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
    ss << index_to_algebraic(board->game_state.en_passant);
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


std::string color_to_string(color_t color)
{
  if (color == WHITE) { return "White"; }

  return "Black";
}


color_t get_square_color(index_t index)
{
  position_t pos = index_to_position(index);

  if (((pos.file + pos.rank) % 2) == 0) { return BLACK; }

  return WHITE;
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


color_t get_piece_color(piece_t piece)
{
  assert(piece != EMPTY && piece != INVALID);

  switch (piece) {
    case B_KING:
    case B_QUEEN:
    case B_KNIGHT:
    case B_BISHOP:
    case B_ROOK:
    case B_PAWN:
      return BLACK;
    case W_KING:
    case W_QUEEN:
    case W_KNIGHT:
    case W_BISHOP:
    case W_ROOK:
    case W_PAWN:
      return WHITE;
    case EMPTY:
    case INVALID:
    default:
      assert(false);
      break;
  }

  assert(false);
  return BLACK;
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


void reset(board_t* board)
{
  assert(board != nullptr);

  // Cleanup
  board->board.fill(EMPTY);
  board->history = history_t();
  cleanup_game_state(&board->game_state);

  // Load FEN
  // load_FEN(board->initial_fen, board);

  // Init Zobrist
  board->game_state.zobrist_key = init_zobrist_key(board);
}
