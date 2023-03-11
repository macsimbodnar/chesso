#include "gui.hpp"
#include "exceptions.hpp"

static constexpr char FEN_INIT_POS[] =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void gui_t::on_init(void*)
{
  fonts[20] = load_font("assets/font/PressStart2P.ttf", 20);
  fonts[panel_conf.font_size] =
      load_font("assets/font/PressStart2P.ttf", panel_conf.font_size);

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

  piece_textures[gui::piece_t::W_PAWN] = load_image("assets/Chess_plt60.png");
  piece_textures[gui::piece_t::W_KNIGHT] = load_image("assets/Chess_nlt60.png");
  piece_textures[gui::piece_t::W_BISHOP] = load_image("assets/Chess_blt60.png");
  piece_textures[gui::piece_t::W_ROOK] = load_image("assets/Chess_rlt60.png");
  piece_textures[gui::piece_t::W_QUEEN] = load_image("assets/Chess_qlt60.png");
  piece_textures[gui::piece_t::W_KING] = load_image("assets/Chess_klt60.png");
  piece_textures[gui::piece_t::B_PAWN] = load_image("assets/Chess_pdt60.png");
  piece_textures[gui::piece_t::B_KNIGHT] = load_image("assets/Chess_ndt60.png");
  piece_textures[gui::piece_t::B_BISHOP] = load_image("assets/Chess_bdt60.png");
  piece_textures[gui::piece_t::B_ROOK] = load_image("assets/Chess_rdt60.png");
  piece_textures[gui::piece_t::B_QUEEN] = load_image("assets/Chess_qdt60.png");
  piece_textures[gui::piece_t::B_KING] = load_image("assets/Chess_kdt60.png");

  textures["background"] = load_image("assets/background_l.jpg");
  textures["flip"] = load_image("assets/flip_icon.png");

  sound_fx["tick_1"] = load_sound("assets/sound/tick_1.wav");
  sound_fx["tick_2"] = load_sound("assets/sound/tick_2.wav");
  sound_fx["tick_3"] = load_sound("assets/sound/tick_3.wav");
  sound_fx["tick_4"] = load_sound("assets/sound/tick_4.wav");
  sound_fx["tick_5"] = load_sound("assets/sound/tick_5.wav");
  sound_fx["click"] = load_sound("assets/sound/click.wav");

  {  // Generate the files and ranks text textures
    const font_t coordinates_font = load_font(
        "assets/font/ubuntu_mono/UbuntuMono-Bold.ttf", board_conf.padding);

    for (char i = 'A'; i < 'I'; ++i) {
      files_and_ranks_textures[i] =
          create_text(std::string(1, i), 0x000000FF, coordinates_font);
    }

    for (char i = '1'; i < '9'; ++i) {
      files_and_ranks_textures[i] =
          create_text(std::string(1, i), 0x000000FF, coordinates_font);
    }
  }

  // Load default state
  state = gui::load_FEN(FEN_INIT_POS);

  {  // Buttons
    const int32_t button_h = 42;
    const int32_t button_w = 50;
    const int32_t x = panel_conf.rect.x + panel_conf.text_padding;
    const int32_t y = panel_conf.rect.y + panel_conf.rect.h - button_h -
                      panel_conf.text_padding;

    buttons["flip"] = create_button({x, y, button_w, button_h}, 0xFFFFFFFF,
                                    textures["flip"], 0x333333FF);
  }
}


point_t gui_t::piece_pos_to_screen_pos(const gui::position_t& pos)
{
  point_t res;
  res.x = board_conf.flipped ? 7 - pos.file : pos.file;
  res.y = board_conf.flipped ? pos.rank : 7 - pos.rank;

  return res;
}


rect_t gui_t::get_square_rect(const int x, const int y)
{
  const int square_size = board_conf.square_size;
  const int x_offset = board_conf.rect.x;
  const int y_offset = board_conf.rect.y;

  const rect_t result = {(x * square_size) + x_offset,
                         (y * square_size) + y_offset, square_size,
                         square_size};

  return result;
}


void gui_t::draw_chessboard()
{
  bool black = false;
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      const pixel_t& color =
          black ? board_conf.black_color : board_conf.white_color;

      const rect_t rect = get_square_rect(x, y);

      draw_rect(rect, color);

      black = !black;
    }

    black = !black;
  }
}


void gui_t::draw_piece(const gui::piece_t p, const gui::position_t& pos)
{
  if (p == gui::piece_t::EMPTY) { return; }

  point_t board_coord = piece_pos_to_screen_pos(pos);
  const rect_t rect = get_square_rect(board_coord.x, board_coord.y);
  const texture_t& piece_texture = piece_textures[p];
  draw_texture(piece_texture, rect);
}


void gui_t::draw_pieces()
{
  const auto& board = state.board;
  for (uint8_t i = 0; i < 8; ++i) {
    for (uint8_t j = 0; j < 8; ++j) {
      const auto& piece = board[i][j];

      if (piece != gui::piece_t::EMPTY) {
        const gui::position_t pos = {i, j};  // rank, files
        draw_piece(piece, pos);
      }
    }
  }
}


void gui_t::draw_coordinates()
{
  const rect_t& board = board_conf.rect;
  const int square_size = board_conf.square_size;
  const float flipped = board_conf.flipped;

  // Draw ranks  1 - 8
  for (int i = 0; i < 8; ++i) {
    const char c = '1' + i;
    const texture_t& t = files_and_ranks_textures[c];

    const int32_t x = (board.x - t.w) / 2;
    const int32_t rank = flipped ? i : 7 - i;
    const int32_t y =
        board.y + ((square_size * rank) + ((square_size - t.h) / 2));

    draw_texture(t, x, y);
  }

  // Draw files  A - H
  for (int i = 0; i < 8; ++i) {
    const char c = 'A' + i;
    const texture_t& t = files_and_ranks_textures[c];

    const int32_t y =
        is_screen_horizontal
            ? (board.h + ((screen.h - board.h + board.y - t.h) / 2))
            : ((board.y - t.h) / 2);

    const int32_t file = flipped ? 7 - i : i;
    const int32_t x =
        board.x + ((square_size * file) + ((square_size - t.w) / 2));

    draw_texture(t, x, y);
  }
}


void gui_t::draw_background()
{
  draw_texture(textures["background"], {0, 0, screen.w, screen.h});
}


#include <bitset>
#include <iostream>

void gui_t::draw_panel()
{
  const font_t& font = fonts[panel_conf.font_size];

  draw_rect(panel_conf.rect, panel_conf.background_color);

  {  // Active color
    const std::string active_text =
        "Turn: " + ((state.active_color == gui::color_t::WHITE)
                        ? std::string("WHITE")
                        : std::string("BLACK"));

    const texture_t active_color =
        create_text(active_text, panel_conf.text_color, font);

    const point_t active_color_pos = panel_conf.get_text_pos(0);
    draw_texture(active_color, active_color_pos.x, active_color_pos.y);
  }

  {  // Castling rights
    const texture_t castling_rights_texture =
        create_text("Castling rights:", panel_conf.text_color, font);

    const texture_t wq =
        create_text("WQ",
                    ((state.castling_rights & gui::castling_rights_t::WQ)
                         ? panel_conf.text_color
                         : panel_conf.text_off_color),
                    font);
    const texture_t wk =
        create_text("WK",
                    ((state.castling_rights & gui::castling_rights_t::WK)
                         ? panel_conf.text_color
                         : panel_conf.text_off_color),
                    font);
    const texture_t bq =
        create_text("BQ",
                    ((state.castling_rights & gui::castling_rights_t::BQ)
                         ? panel_conf.text_color
                         : panel_conf.text_off_color),
                    font);
    const texture_t bk =
        create_text("BK",
                    ((state.castling_rights & gui::castling_rights_t::BK)
                         ? panel_conf.text_color
                         : panel_conf.text_off_color),
                    font);

    const point_t castling_pos = panel_conf.get_text_pos(1);
    draw_texture(castling_rights_texture, castling_pos.x, castling_pos.y);
    draw_texture(
        wq,
        castling_rights_texture.w + castling_pos.x + panel_conf.text_padding,
        castling_pos.y);
    draw_texture(wk,
                 castling_rights_texture.w + castling_pos.x + wq.w +
                     (panel_conf.text_padding * 2),
                 castling_pos.y);
    draw_texture(bq,
                 castling_rights_texture.w + castling_pos.x + wq.w + wk.w +
                     (panel_conf.text_padding * 3),
                 castling_pos.y);
    draw_texture(bk,
                 castling_rights_texture.w + castling_pos.x + wq.w + wk.w +
                     bq.w + (panel_conf.text_padding * 4),
                 castling_pos.y);
  }

  {  // En passant
    const point_t en_passant_pos = panel_conf.get_text_pos(2);
    const texture_t en_passant =
        create_text("En passant: ", panel_conf.text_color, font);
    draw_texture(en_passant, en_passant_pos.x, en_passant_pos.y);

    if (state.is_en_passant) {
      const texture_t en_passant_target_square =
          create_text(gui::pos_to_algebraic(state.en_passant_target_square),
                      panel_conf.text_color, font);

      draw_texture(en_passant_target_square,
                   en_passant_pos.x + en_passant.w + panel_conf.text_padding,
                   en_passant_pos.y);
    }
  }

  {  // Half move clock
    const texture_t half_move_clock =
        create_text("Half move clock: " + STR(state.half_move_clock),
                    panel_conf.text_color, font);
    const point_t half_move_clock_pos = panel_conf.get_text_pos(3);
    draw_texture(half_move_clock, half_move_clock_pos.x, half_move_clock_pos.y);
  }

  {  // Full move clock
    const texture_t full_move_clock =
        create_text("Full move clock: " + STR(state.full_move_clock),
                    panel_conf.text_color, font);
    const point_t full_move_clock_pos = panel_conf.get_text_pos(4);
    draw_texture(full_move_clock, full_move_clock_pos.x, full_move_clock_pos.y);
  }

  {  // FPS

    const texture_t fps_texture =
        create_text("FPS: " + STR(FPS()), panel_conf.text_color, font);

    const int32_t x = panel_conf.rect.x + panel_conf.rect.w - fps_texture.w -
                      panel_conf.text_padding;
    const int32_t y = panel_conf.rect.y + panel_conf.rect.h - fps_texture.h -
                      panel_conf.text_padding;

    draw_texture(fps_texture, x, y);
  }

  {  // Flip button
    const button_t& button = buttons["flip"];


    draw_button_with_icon(button);
  }
}


void gui_t::handle_events()
{
  const mouse_t& mouse = mouse_state();
  // Close key
  if (is_key_pressed(keycap_t::ESC)) { stop(); }

  // Flip button
  if (is_mouse_in(buttons["flip"].rect) && mouse.left_button.click) {
    play_sound(sound_fx["click"]);
    board_conf.flipped = !board_conf.flipped;
  }
}


void gui_t::on_update(void*)
{
  handle_events();

  draw_background();
  draw_chessboard();
  draw_coordinates();
  draw_pieces();

  draw_panel();
}