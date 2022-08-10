#include <iostream>
#include <map>
#include "pixello.hpp"

texture_t background;
std::map<char, texture_t> pieces;
constexpr int32_t size = 60;

class pixel : public pixello
{
public:
  pixel()
      : pixello({size, size, 800, 500, "Test pixello", 60,
                 "assets/font/PressStart2P.ttf", 10})
  {}

private:
  void draw_board()
  {
    pixel_t p;
    p.a = 255;

    bool black = false;
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        if (black) {
          p.r = 89;
          p.g = 44;
          p.b = 4;
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
  }

  void draw_FEN(const std::string& FEN)
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

    int x = 0;
    int y = 0;

    for (const char c : FEN) {
      switch (c) {
        case '/':
          x = 0;
          y += size;
          break;

        case '1':
          x += size * 1;
          break;
        case '2':
          x += size * 2;
          break;
        case '3':
          x += size * 3;
          break;
        case '4':
          x += size * 4;
          break;
        case '5':
          x += size * 5;
          break;
        case '6':
          x += size * 6;
          break;
        case '7':
          x += size * 7;
          break;
        case '8':
          x += size * 8;
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
          draw_texture(pieces[c], x, y, size, size);
          x += size;
          break;

        case ' ':
        default:
          // In this case we just stop
          return;
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

    draw_board();
    // Draw the pieces
    draw_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  }
};

int main()
{
  pixel p;

  if (!p.run()) { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}