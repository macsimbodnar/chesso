#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include "data_structs.hpp"
#include "pixello.hpp"
#include "utils.hpp"


texture_t background;
std::map<char, texture_t> pieces;
constexpr int32_t size = 60;
game_t game;


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

    game = load_board_from_FEN(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
      case game_t::BLACK:
        turn += "B";
        break;

      case game_t::WHITE:
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

    // Draw en passant target square
    std::string en_passant = "En passant: " + game.en_passant_target_square;
    texture_t en_passant_texture = create_text(en_passant, text_color);
    draw_texture(en_passant_texture, 10,
                 +turn_texture.h + castling_texture.h + 30);

    // Draw the halfmove clock
    std::string half_clock = "HMC: " + STR(game.halfmove_clock);
    texture_t half_clock_texture = create_text(half_clock, text_color);
    draw_texture(
        half_clock_texture, 10,
        +turn_texture.h + castling_texture.h + en_passant_texture.h + 40);

    // Draw the fullmove counter
    std::string full_clock = "FMC: " + STR(game.full_move);
    texture_t full_clock_texture = create_text(full_clock, text_color);
    draw_texture(full_clock_texture, 10,
                 +turn_texture.h + castling_texture.h + en_passant_texture.h +
                     half_clock_texture.h + 50);

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