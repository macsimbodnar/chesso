#include <iostream>
#include <map>
#include "exceptions.hpp"
#include "pixello.hpp"

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
  char board[8][8];
  bool flipped = true;
};

game_t game;

void load_board_from_FEN(game_t& game, const std::string& FEN)
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

  // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

  int x = 0;
  int y = 0;

  for (const char c : FEN) {
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

      case ' ':
      default:
        // In this case we just stop
        return;
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
    load_board_from_FEN(game,
                        "rnb1kbnr/p1qppppp/1p6/2p5/4P3/5P2/PPPPQ1PP/RNB1KBNR");
  }

  void on_update() override
  {
    /***************************************************************************
     * FULL SCREEN
     **************************************************************************/
    clear_screen({0xFF000000});
    draw_texture(background, 0, 0, 800, 500);

    /***************************************************************************
     * RIGHT PANNEl
     **************************************************************************/
    set_current_viewport(500, 10, 290, 480);

    // Print FPS
    uint32_t fps = FPS();
    texture_t fps_texture = create_text("FPS: " + STR(fps), {0xFF000000});
    draw_texture(fps_texture, 290 - 77, 480 - 20);

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