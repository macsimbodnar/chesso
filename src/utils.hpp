#pragma once
#include <sstream>
#include <vector>


inline std::vector<std::string> split_string(const std::string& str)
{
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> tokens(begin, end);

  return tokens;
}


inline bool is_uint(const std::string& str)
{
  for (const char c : str) {
    if (!isdigit(c)) { return false; }
  }

  return true;
}


inline game_t load_board_from_FEN(const std::string& FEN)
{
  game_t game;
  game.flipped = false;

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

  int x = 0;
  int y = 0;

  for (const char c : sections[0]) {
    switch (c) {
      case '/':
        x = 0;
        ++y;
        break;

      case '1':
        x += 1;
        break;
      case '2':
        x += 2;
        break;
      case '3':
        x += 3;
        break;
      case '4':
        x += 4;
        break;
      case '5':
        x += 5;
        break;
      case '6':
        x += 6;
        break;
      case '7':
        x += 7;
        break;
      case '8':
        x += 8;
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
      case 'k':
        game.board[y][x] = new piece_t(c);
        ++x;
        break;

      default:
        // We get a non valid string
        throw FAN_exception("Invalid char in FEN string [" + STR(c) +
                            "]. FEN: " + FEN);
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
      game.active_color = game_t::WHITE;
      break;
    case 'b':
      game.active_color = game_t::BLACK;
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

  game.available_castling = 0x00;
  for (const char c : sections[2]) {
    switch (c) {
      case '-':
        game.available_castling = 0x00;
        if (sections[2].size() != 1) {
          throw FAN_exception(
              "Invalid castling availability section size. No castling "
              "available char is set but the section size is too big. FEN: " +
              FEN);
        }
        break;
      case 'K':
        game.available_castling |= WK;
        break;
      case 'Q':
        game.available_castling |= WQ;
        break;
      case 'k':
        game.available_castling |= BK;
        break;
      case 'q':
        game.available_castling |= BQ;
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

  game.en_passant_target_square = sections[3];

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
    game.halfmove_clock = static_cast<int>(std::stoul(half_move));
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
    game.full_move = static_cast<int>(std::stoul(full_move));
  } catch (std::exception& e) {
    throw FAN_exception("Can't convert Fullmove number to integer. What: " +
                        std::string(e.what()) + " FEN: " + FEN);
  }

  if (game.full_move < 1) {
    throw FAN_exception("Fullmove number can't be less then 1 but it is " +
                        std::string(STR(game.full_move)) + " FEN: " + FEN);
  }

  return game;
}
