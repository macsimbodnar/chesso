#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "exceptions.hpp"
#include "pixello.hpp"

#define BLACK 0
#define WHITE 1

constexpr uint8_t WQ = 0b0000001;
constexpr uint8_t WK = 0b0000010;
constexpr uint8_t BQ = 0b0000100;
constexpr uint8_t BK = 0b0001000;


texture_t background;
std::map<char, texture_t> pieces;
constexpr int32_t size = 60;

class FAN_exception : public pixello_exception
{
public:
  FAN_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};

struct game_t
{
  char board[8][8] = {0};
  bool flipped = true;
  int active_color = WHITE;
  uint8_t available_castling = WQ | WK | BQ | BK;
};

game_t game;

std::vector<std::string> split_string(const std::string& str)
{
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> tokens(begin, end);

  return tokens;
}

void load_board_from_FEN(game_t& game, const std::string& FEN)
{
  // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
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
        game.board[y][x] = c;
        ++x;
        break;

      default:
        // We get a non valid string
        throw FAN_exception("Invalid char in FEN string [" + STR(c) +
                            "]. FEN: " + FEN);
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
        game.active_color = WHITE;
        break;
      case 'b':
        game.active_color = BLACK;
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
  }
}

class pixel : public pixello
{
public:
  pixel()
      : pixello({size, size, 800, 500, "Chesso", 60,
                 "assets/font/PressStart2P.ttf", 8})
  {}

private:
  void draw_board(game_t& game)
  {
    // Draw background
    pixel_t p;
    p.a = 255;

    bool black = false;
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        if (black) {
          p.r = 109;
          p.g = 64;
          p.b = 24;
        } else {
          p.r = 232;
          p.g = 235;
          p.b = 239;
        }

        draw_pixel(x, y, p);  // top left red
        black = !black;
      }

      black = !black;
    }

    // Draw pieces

    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++j) {
        char c;
        if (game.flipped) {
          c = game.board[7 - i][7 - j];
        } else {
          c = game.board[i][j];
        }

        if (c > 0) { draw_texture(pieces[c], j * size, i * size, size, size); }
      }
    }
  }

  void log(const std::string& msg) override { std::cout << msg << std::endl; }

  void on_init() override
  {
    /**
     * pawn = "P"
     * knight = "N"
     * bishop = "B"
     * rook = "R"
     * queen = "Q"
     * and king = "K
     *
     * White ("PNBRQK")
     * Black ("pnbrqk")
     */

    pieces['P'] = load_image("assets/Chess_plt60.png");
    pieces['N'] = load_image("assets/Chess_nlt60.png");
    pieces['B'] = load_image("assets/Chess_blt60.png");
    pieces['R'] = load_image("assets/Chess_rlt60.png");
    pieces['Q'] = load_image("assets/Chess_qlt60.png");
    pieces['K'] = load_image("assets/Chess_klt60.png");
    pieces['p'] = load_image("assets/Chess_pdt60.png");
    pieces['n'] = load_image("assets/Chess_ndt60.png");
    pieces['b'] = load_image("assets/Chess_bdt60.png");
    pieces['r'] = load_image("assets/Chess_rdt60.png");
    pieces['q'] = load_image("assets/Chess_qdt60.png");
    pieces['k'] = load_image("assets/Chess_kdt60.png");

    background = load_image("assets/background_l.jpg");

    game.flipped = true;
    load_board_from_FEN(
        game, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

    // Starting pos
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
  }

  void on_update() override
  {
    /***************************************************************************
     * FULL SCREEN
     **************************************************************************/
    clear_screen({0x000000FF});
    draw_texture(background, 0, 0, 800, 500);

    /***************************************************************************
     * RIGHT PANNEl
     **************************************************************************/
    set_current_viewport(500, 10, 290, 480, {0xEEEEEEFF});

    pixel_t text_color = {0x000000FF};

    // Print FPS
    uint32_t fps = FPS();
    texture_t fps_texture = create_text("FPS: " + STR(fps), text_color);
    draw_texture(fps_texture, 290 - 77, 480 - 20);

    // Draw turn
    std::string turn = "Turn: ";
    switch (game.active_color) {
      case BLACK:
        turn += "B";
        break;

      case WHITE:
        turn += "W";
        break;
    }
    texture_t turn_texture = create_text(turn, text_color);
    draw_texture(turn_texture, 10, 10);

    // Draw castling situation
    std::string castling = "Castling: ";
    if (game.available_castling & WK) { castling += "K"; }
    if (game.available_castling & WQ) { castling += "Q"; }
    if (game.available_castling & BK) { castling += "k"; }
    if (game.available_castling & BQ) { castling += "q"; }
    if (game.available_castling == 0x00) { castling += "-"; }
    texture_t castling_texture = create_text(castling, text_color);
    draw_texture(castling_texture, 10, turn_texture.h + 20);

    /***************************************************************************
     * MAIN BOARD
     **************************************************************************/
    set_current_viewport(10, 10, 480, 480);
    draw_board(game);
  }
};


int main(int argc, char** argv)
{
  pixel p;

  if (!p.run()) { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}