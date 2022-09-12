#include "gui.hpp"
#include "exceptions.hpp"


void gui_t::draw_coordinates()
{
  // Draw ranks  1 - 8
  for (int i = 0; i < 8; ++i) {
    const char c = '1' + i;
    const texture_t& t = files_and_ranks_textures[c];

    const int32_t x = (BOARD_RECT.x - t.w) / 2;
    const int32_t rank = flipped_board ? i : 7 - i;
    const int32_t y =
        BOARD_RECT.y + ((SQUARE_SIZE * rank) + ((SQUARE_SIZE - t.h) / 2));

    draw_texture(t, x, y);
  }

  // Draw files  A - H
  for (int i = 0; i < 8; ++i) {
    const char c = 'A' + i;
    const texture_t& t = files_and_ranks_textures[c];

    const int32_t y =
        BOARD_RECT.h + ((SCREEN_H - BOARD_RECT.h + BOARD_RECT.y - t.h) / 2);
    const int32_t file = flipped_board ? 7 - i : i;
    const int32_t x =
        BOARD_RECT.x + ((SQUARE_SIZE * file) + ((SQUARE_SIZE - t.w) / 2));

    draw_texture(t, x, y);
  }
}


void gui_t::draw_board()
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

      draw_rect({x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE},
                p);
      black = !black;
    }

    black = !black;
  }

  // Draw selected square
  if (selected_square.selected) { draw_rect(selected_square.rect, 0x33333390); }

  // Draw the current square
  if (is_mouse_in(BOARD_RECT)) {
    int x = ((mouse_state().x - BOARD_RECT.x) / SQUARE_SIZE);
    int y = ((mouse_state().y - BOARD_RECT.y) / SQUARE_SIZE);

    draw_rect({x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE},
              0xAA00FF55);
  }

  // Draw pieces
  const auto pieces = chess.pieces();
  for (const auto& I : pieces) {
    const uint8_t file = I.position.file;
    const uint8_t rank = I.position.rank;

    const coordinates_t coord = position_to_coordinates(file, rank);
    const piece_t p = I.piece;

    // Don't draw the selected piece
    if (mouse_holding.selected && mouse_holding.info.index == I.index) {
      continue;
    }

    // The pice is in place
    rect_t r = {coord.x * SQUARE_SIZE, coord.y * SQUARE_SIZE, SQUARE_SIZE,
                SQUARE_SIZE};

    draw_texture(piece_textures[p], r);
  }

  // Draw suggestions
  if (selected_square.selected) {
    for (const auto& I : suggested_positions) {
      const coordinates_t coord = position_to_coordinates(I.file, I.rank);
      const int32_t x = (coord.x * SQUARE_SIZE) + SQUARE_SIZE / 2;
      const int32_t y = (coord.y * SQUARE_SIZE) + SQUARE_SIZE / 2;
      draw_circle(x, y, SQUARE_SIZE / 6, 0xFF0000FF);
    }
  }

  // For last draw the selected piece
  if (mouse_holding.selected) {
    rect_t r = {mouse_state().x - mouse_holding.offset_x,
                mouse_state().y - mouse_holding.offset_y, SQUARE_SIZE,
                SQUARE_SIZE};
    draw_texture(piece_textures[mouse_holding.info.piece], r);
  }
}


void gui_t::on_init(void*)
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

  piece_textures[piece_t::W_PAWN] = load_image("assets/Chess_plt60.png");
  piece_textures[piece_t::W_KNIGHT] = load_image("assets/Chess_nlt60.png");
  piece_textures[piece_t::W_BISHOP] = load_image("assets/Chess_blt60.png");
  piece_textures[piece_t::W_ROOK] = load_image("assets/Chess_rlt60.png");
  piece_textures[piece_t::W_QUEEN] = load_image("assets/Chess_qlt60.png");
  piece_textures[piece_t::W_KING] = load_image("assets/Chess_klt60.png");
  piece_textures[piece_t::B_PAWN] = load_image("assets/Chess_pdt60.png");
  piece_textures[piece_t::B_KNIGHT] = load_image("assets/Chess_ndt60.png");
  piece_textures[piece_t::B_BISHOP] = load_image("assets/Chess_bdt60.png");
  piece_textures[piece_t::B_ROOK] = load_image("assets/Chess_rdt60.png");
  piece_textures[piece_t::B_QUEEN] = load_image("assets/Chess_qdt60.png");
  piece_textures[piece_t::B_KING] = load_image("assets/Chess_kdt60.png");

  background = load_image("assets/background_l.jpg");

  sound_fx[0] = load_sound("assets/sound/tick_1.wav");
  sound_fx[1] = load_sound("assets/sound/tick_2.wav");
  sound_fx[2] = load_sound("assets/sound/tick_3.wav");
  sound_fx[3] = load_sound("assets/sound/tick_4.wav");
  sound_fx[4] = load_sound("assets/sound/tick_5.wav");

  // Generate the files and ranks text textures
  for (char i = 'A'; i < 'I'; ++i) {
    files_and_ranks_textures[i] = create_text(std::string(1, i), 0x000000FF);
  }

  for (char i = '1'; i < '9'; ++i) {
    files_and_ranks_textures[i] = create_text(std::string(1, i), 0x000000FF);
  }
}


void gui_t::on_update(void*)
{
  {
    /***************************************************************************
     * MOUSE
     **************************************************************************/
    const auto mouse = mouse_state();

    // Reset the board if click on the right panel
    if (is_mouse_in(RIGHT_PANEL_RECT) && mouse.left_button.click) {
      chess.load(FEN_INIT_POS);
      mouse_holding.selected = false;
      selected_square.selected = false;
      suggested_positions.clear();
    }

    // Flip the board if click on the right panel with the right button
    if (is_mouse_in(RIGHT_PANEL_RECT) && mouse.right_button.click) {
      flipped_board = !flipped_board;
    }

    if (is_mouse_in(BOARD_RECT)) {
      const uint8_t x = ((mouse.x - BOARD_RECT.x) / SQUARE_SIZE);
      const uint8_t y = ((mouse.y - BOARD_RECT.y) / SQUARE_SIZE);

      const position_t target_pos = coordinates_to_postion(x, y);
      const piece_info_t piece_i =
          chess.get_piece_info(target_pos.file, target_pos.rank);

      // Piece holding
      if (piece_i.piece != piece_t::EMPTY) {
        if (mouse.left_button.state == button_t::DOWN &&
            !mouse_holding.selected) {
          mouse_holding.offset_x = mouse.x - (x * SQUARE_SIZE);
          mouse_holding.offset_y = mouse.y - (y * SQUARE_SIZE);
          mouse_holding.selected = true;
          mouse_holding.info = piece_i;

          // Play the soft sound
          play_sound(sound_fx[1]);
        }
      }

      // Reset the selected state
      if (mouse.left_button.state == button_t::UP && mouse_holding.selected) {
        const uint8_t sel_f = mouse_holding.info.position.file;
        const uint8_t sel_r = mouse_holding.info.position.rank;
        const coordinates_t selected = position_to_coordinates(sel_f, sel_r);

        if (x != selected.x || y != selected.y) {
          // In this case we want to unselect the square
          selected_square.selected = false;
          selected_square.x = 0;
          selected_square.y = 0;
          selected_square.rect = {0};
          suggested_positions.clear();
        }

        // Set the piece to the destination column when release
        const position_t dest = coordinates_to_postion(x, y);

        if (dest.file != sel_f || dest.rank != sel_r) {
          chess.move(sel_f, sel_r, dest.file, dest.rank);
        }

        mouse_holding.selected = false;
        mouse_holding.offset_x = 0;
        mouse_holding.offset_y = 0;

        // Play sound
        play_sound(sound_fx[3]);
      }

      // Click on the square
      if (mouse.left_button.click) {
        if (selected_square.selected && selected_square.x == x &&
            selected_square.y == y) {
          selected_square.selected = false;
          selected_square.x = 0;
          selected_square.y = 0;
          selected_square.rect = {0};
          suggested_positions.clear();
        } else {
          selected_square.x = x;
          selected_square.y = y;
          selected_square.selected = true;
          selected_square.rect = {x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE,
                                  SQUARE_SIZE};

          // Get the available moves
          const position_t selected_pos = coordinates_to_postion(x, y);
          suggested_positions =
              chess.get_valid_moves(selected_pos.file, selected_pos.rank);
        }
      }
    }
  }

  const pixel_t text_color = {0x000000FF};

  {
    /***************************************************************************
     * FULL SCREEN
     **************************************************************************/
    clear_screen({0x000000FF});
    draw_texture(background, {0, 0, SCREEN_W, SCREEN_H});
    draw_coordinates();

    // Print FPS
    uint32_t fps = FPS();
    texture_t fps_texture = create_text("FPS: " + STR(fps), text_color);

    const int32_t x = SCREEN_W - 10 - fps_texture.w;
    const int32_t y =
        RIGHT_PANEL_RECT.h +
        ((SCREEN_H - RIGHT_PANEL_RECT.h + RIGHT_PANEL_RECT.y - fps_texture.h) /
         2);

    draw_texture(fps_texture, x, y);
  }

  {
    /***************************************************************************
     * RIGHT PANEL
     **************************************************************************/
    set_current_viewport(RIGHT_PANEL_RECT, {0xEEEEEEFF});

    // Draw turn
    std::string turn = "Turn: ";
    switch (chess.active_color()) {
      case color_t::BLACK:
        turn += "B";
        break;

      case color_t::WHITE:
        turn += "W";
        break;
    }

    texture_t turn_texture = create_text(turn, text_color);
    draw_texture(turn_texture, 10, 10);

    // Draw castling situation
    std::string castling = "Castling: ";
    const uint8_t available_castling = chess.available_castling();
    if (available_castling & WK) { castling += "K"; }
    if (available_castling & WQ) { castling += "Q"; }
    if (available_castling & BK) { castling += "k"; }
    if (available_castling & BQ) { castling += "q"; }
    if (available_castling == 0x00) { castling += "-"; }
    texture_t castling_texture = create_text(castling, text_color);
    draw_texture(castling_texture, 10, turn_texture.h + 20);

    // Draw en passant target square
    std::string en_passant = "En passant: " + chess.en_passant_target_square();
    texture_t en_passant_texture = create_text(en_passant, text_color);
    draw_texture(en_passant_texture, 10,
                 +turn_texture.h + castling_texture.h + 30);

    // Draw the halfmove clock
    std::string half_clock = "HMC: " + STR(chess.halfmove_clock());
    texture_t half_clock_texture = create_text(half_clock, text_color);
    draw_texture(
        half_clock_texture, 10,
        +turn_texture.h + castling_texture.h + en_passant_texture.h + 40);

    // Draw the fullmove counter
    std::string full_clock = "FMC: " + STR(chess.full_move());
    texture_t full_clock_texture = create_text(full_clock, text_color);
    draw_texture(full_clock_texture, 10,
                 +turn_texture.h + castling_texture.h + en_passant_texture.h +
                     half_clock_texture.h + 50);
  }

  {
    /***************************************************************************
     * MAIN BOARD
     **************************************************************************/
    set_current_viewport(BOARD_RECT);
    draw_board();
  }
}