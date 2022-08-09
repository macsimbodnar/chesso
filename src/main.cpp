#include "pixello.hpp"
#include <iostream>
#include <map>

std::map<char, pixello::texture> pieces;
constexpr int32_t size = 60;

class pixel : public pixello {
public:
  pixel() : pixello({size, size, 480, 480, "Test pixello", 60}) {}

private:
  void draw_board() {
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

        draw(x, y, p); // top left red
        black = !black;
      }

      black = !black;
    }
  }

  void draw_FEN(const std::string &FEN) {
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

  void log(const std::string &msg) override { std::cout << msg << std::endl; }

  void on_init() override {
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

    pieces['P'] = load_texture("assets/Chess_plt60.png");
    pieces['N'] = load_texture("assets/Chess_nlt60.png");
    pieces['B'] = load_texture("assets/Chess_blt60.png");
    pieces['R'] = load_texture("assets/Chess_rlt60.png");
    pieces['Q'] = load_texture("assets/Chess_qlt60.png");
    pieces['K'] = load_texture("assets/Chess_klt60.png");
    pieces['p'] = load_texture("assets/Chess_pdt60.png");
    pieces['n'] = load_texture("assets/Chess_ndt60.png");
    pieces['b'] = load_texture("assets/Chess_bdt60.png");
    pieces['r'] = load_texture("assets/Chess_rdt60.png");
    pieces['q'] = load_texture("assets/Chess_qdt60.png");
    pieces['k'] = load_texture("assets/Chess_kdt60.png");
  }

  void on_update() override {
    clear({0xFF000000});

    draw_board();

    // Draw the pieces
    draw_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  }
};

int main() {

  pixel p;

  p.run();

  return 0;
}