#include "gui_data_struct.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

namespace gui
{


position_t algebraic_to_pos(const std::string& p)
{
  if (p.size() != 2) {
    throw input_exception("Algebraic position size invalid: " + p);
  }

  char file = p[0];
  char rank = p[1];

  if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
    throw input_exception("Algebraic position invalid value: " + p);
  }

  position_t pos;
  pos.file = file - 'a';
  pos.rank = rank - '1';

  return pos;
}

std::string pos_to_algebraic(const position_t& p)
{
  std::string result;
  result.reserve(2);

  result.push_back('a' + p.file);
  result.push_back('1' + p.rank);

  return result;
}


color_t get_piece_color(const piece_t t)
{
  static constexpr uint32_t COLOR_MASK =
      U32(color_t::BLACK) | U32(color_t::WHITE);

  if (t == piece_t::EMPTY) {
    throw input_exception("Invalid piece type in the color cast: " +
                          STR(INT(t)));
  }

  const color_t color = static_cast<color_t>(U32(t) & COLOR_MASK);

  return color;
}


piece_t c_to_piece(const char p)
{
  piece_t piece = piece_t::EMPTY;

  switch (p) {
    case 'P':
      piece = piece_t::W_PAWN;
      break;
    case 'N':
      piece = piece_t::W_KNIGHT;
      break;
    case 'B':
      piece = piece_t::W_BISHOP;
      break;
    case 'R':
      piece = piece_t::W_ROOK;
      break;
    case 'Q':
      piece = piece_t::W_QUEEN;
      break;
    case 'K':
      piece = piece_t::W_KING;
      break;
    case 'p':
      piece = piece_t::B_PAWN;
      break;
    case 'n':
      piece = piece_t::B_KNIGHT;
      break;
    case 'b':
      piece = piece_t::B_BISHOP;
      break;
    case 'r':
      piece = piece_t::B_ROOK;
      break;
    case 'q':
      piece = piece_t::B_QUEEN;
      break;
    case 'k':
      piece = piece_t::B_KING;
      break;
    default:
      throw input_exception("Invalid piece: " + std::string(1, p));
      break;
  }

  return piece;
}


state_t load_FEN(const std::string& FEN)
{
  state_t state;

  for (auto& I : state.board) {
    for (auto& cell : I) {
      cell = piece_t::EMPTY;
    }
  }

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

  int file = 0;
  int rank = 7;

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
        state.board[rank][file] = c_to_piece(c);
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
      state.active_color = color_t::WHITE;
      break;
    case 'b':
      state.active_color = color_t::BLACK;
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

  state.castling_rights = castling_rights_t::NONE;
  for (const char c : sections[2]) {
    switch (c) {
      case '-':
        state.castling_rights = castling_rights_t::NONE;
        if (sections[2].size() != 1) {
          throw FAN_exception(
              "Invalid castling availability section size. No castling "
              "available char is set but the section size is too big. FEN: " +
              FEN);
        }
        break;
      case 'K':
        state.castling_rights |= castling_rights_t::WK;
        break;
      case 'Q':
        state.castling_rights |= castling_rights_t::WQ;
        break;
      case 'k':
        state.castling_rights |= castling_rights_t::BK;
        break;
      case 'q':
        state.castling_rights |= castling_rights_t::BQ;
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

  if (sections[3] == "-") {
    state.is_en_passant = false;
  } else {
    state.is_en_passant = true;
    state.en_passant_target_square = algebraic_to_pos(sections[3]);
  }

  /***************************************************************************
   * 4. Halfmove clock
   *
   * The number of half moves since the last capture or pawn advance, used for
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
    state.half_move_clock = static_cast<int>(std::stoul(half_move));
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
    state.full_move_clock = static_cast<int>(std::stoul(full_move));
  } catch (std::exception& e) {
    throw FAN_exception("Can't convert Fullmove number to integer. What: " +
                        std::string(e.what()) + " FEN: " + FEN);
  }

  if (state.full_move_clock < 1) {
    throw FAN_exception("Fullmove number can't be less then 1 but it is " +
                        std::string(STR(state.full_move_clock)) +
                        " FEN: " + FEN);
  }

  // Set the fen string at the end
  state.FEN = FEN;

  return state;
}

piece_t get_piece(const state_t& state, const position_t& pos)
{
  const piece_t result = state.board[pos.rank][pos.file];
  return result;
}


void set_piece(state_t& state, const position_t& pos, const piece_t piece)
{
  state.board[pos.rank][pos.file] = piece;
}


}  // namespace gui