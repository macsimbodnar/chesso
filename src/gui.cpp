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
      if (game.board[i][j] != nullptr && game.board[i][j]->piece > 0) {
        rect_t r;
        if (game.board[i][j] == mouse_holding.selected) {
          // Case we are moving a piece
          r = {mouse_state().x - mouse_holding.offset_x,
               mouse_state().y - mouse_holding.offset_y, square_size,
               square_size};
        } else {
          // The pice is in place
          r = {j * square_size, i * square_size, square_size, square_size};
        }

        draw_texture(piece_textures[game.board[i][j]->piece], r);
      }
    }
  }
}


void gui::on_init(void*)
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

  piece_textures['P'] = load_image("assets/Chess_plt60.png");
  piece_textures['N'] = load_image("assets/Chess_nlt60.png");
  piece_textures['B'] = load_image("assets/Chess_blt60.png");
  piece_textures['R'] = load_image("assets/Chess_rlt60.png");
  piece_textures['Q'] = load_image("assets/Chess_qlt60.png");
  piece_textures['K'] = load_image("assets/Chess_klt60.png");
  piece_textures['p'] = load_image("assets/Chess_pdt60.png");
  piece_textures['n'] = load_image("assets/Chess_ndt60.png");
  piece_textures['b'] = load_image("assets/Chess_bdt60.png");
  piece_textures['r'] = load_image("assets/Chess_rdt60.png");
  piece_textures['q'] = load_image("assets/Chess_qdt60.png");
  piece_textures['k'] = load_image("assets/Chess_kdt60.png");

  background = load_image("assets/background_l.jpg");

  game = load_board_from_FEN(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}


void gui::on_update(void*)
{
  static const rect_t board_rect = {10, 10, 480, 480};

  {
    /***************************************************************************
     * MOUSE CLICK
     **************************************************************************/
    const auto mouse = mouse_state();

    if (is_mouse_in(board_rect)) {
      int x = ((mouse.x - board_rect.x) / square_size);
      int y = ((mouse.y - board_rect.y) / square_size);

      auto piece = game.board[y][x];

      if (piece != nullptr) {
        if (mouse.left_button.state == button_t::DOWN &&
            mouse_holding.selected == nullptr) {
          mouse_holding.offset_x = mouse.x - (x * square_size);
          mouse_holding.offset_y = mouse.y - (y * square_size);


          mouse_holding.selected = piece;
        }
      }

      if (mouse.left_button.state == button_t::UP) {
        mouse_holding.selected = nullptr;
        mouse_holding.offset_x = 0;
        mouse_holding.offset_y = 0;
      }
    }
  }

  {
    /***************************************************************************
     * FULL SCREEN
     **************************************************************************/

    clear_screen({0x000000FF});
    draw_texture(background, {0, 0, 800, 500});
  }

  {
    /***************************************************************************
     * RIGHT PANNEl
     **************************************************************************/
    set_current_viewport({500, 10, 290, 480}, {0xEEEEEEFF});

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
  }

  {
    /***************************************************************************
     * MAIN BOARD
     **************************************************************************/
    set_current_viewport(board_rect);
    draw_board();

    // Check if we are in the rect
    if (is_mouse_in(board_rect)) {
      int x = ((mouse_state().x - board_rect.x) / square_size);
      int y = ((mouse_state().y - board_rect.y) / square_size);

      draw_rect({x * square_size, y * square_size, square_size, square_size},
                0xAA00FF55);
    }
  }
}