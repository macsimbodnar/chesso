#include "gui.hpp"


void gui::draw_board()
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

      draw_rect({x * square_size, y * square_size, square_size, square_size},
                p);
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

      if (c > 0) {
        rect_t r = {j * square_size, i * square_size, square_size, square_size};
        draw_texture(pieces[c], r);
      }
    }
  }
}