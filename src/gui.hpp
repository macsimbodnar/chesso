#pragma once
#include <cassert>
#include <cmath>
#include <map>
#include <pixello.hpp>
#include "gui_data_struct.hpp"
#include "log.hpp"
#include "utils.hpp"


struct fen_panel_conf
{
  int text_padding = 5;
  int font_size = 10;
  rect_t rect;
  pixel_t bg_color = 0x000000FF;
  pixel_t text_color = 0xFFFFFFFF;

  fen_panel_conf(const int screen_w, const int screen_h)
  {
    const int h = font_size + (text_padding * 2);
    rect = {0, screen_h - h, screen_w, h};
  }
};


struct chessboard_conf
{
  constexpr static float paddings_percentage = 0.04f;
  constexpr static int black_boundary_size_px = 2;
  int padding;
  rect_t rect;
  int square_size;
  pixel_t black_color = {109, 64, 24, 255};
  pixel_t white_color = {232, 235, 239, 255};
  bool flipped = false;

  chessboard_conf(const int screen_w,
                  const int screen_h,
                  const fen_panel_conf& fen_panel_conf)
  {
    assert(screen_w != 0);
    assert(screen_h != 0);

    const int full_size =
        (screen_w > screen_h) ? (screen_h - fen_panel_conf.rect.h) : screen_w;
    padding = std::round(static_cast<float>(full_size) * paddings_percentage);

    const int board_size = full_size - (padding * 2);
    square_size = board_size / 8;

    // This way we remove the additional pixels in case the the `board_size` is
    // not divisible by 8
    rect = {padding, padding, square_size * 8, square_size * 8};

    LOG_I << "Board width: " << rect.w << END_I;
    LOG_I << "Board padding: " << padding << END_I;
    LOG_I << "Board square size: " << square_size << END_I;
    LOG_I << "Board width/size: " << rect.w / (float)square_size << END_I;
  }
};

struct control_panel_conf
{
  rect_t rect;
  pixel_t background_color = 0x000000FF;
  pixel_t text_color = 0xFFFFFFFF;
  pixel_t text_off_color = 0XFF0000FF;
  constexpr static int text_padding = 5;
  constexpr static int font_size = 11;
  bool is_FEN_input_selected = false;
  texture_t FEN_input_texture;
  rect_t FEN_rect;

  control_panel_conf(const int screen_w,
                     const int screen_h,
                     const fen_panel_conf& fen_panel_conf,
                     const chessboard_conf& board_conf)
  {
    if (screen_w > screen_h) {
      rect.x = board_conf.rect.x + board_conf.rect.w + board_conf.padding;
      rect.w = screen_w - rect.x;

      rect.y = 0;
      rect.h = screen_h - fen_panel_conf.rect.h;
    } else {
      rect.x = 0;
      rect.y = board_conf.rect.y + board_conf.rect.h + board_conf.padding;

      rect.w = screen_w;
      rect.h = screen_h - rect.y - fen_panel_conf.rect.h;
    }

    // Get fen rect
    const rect_t pos = get_grid_rect(6, 0, 1);
    FEN_rect = {pos.x, pos.y, pos.w, pos.h + 10};
  }

  inline rect_t get_grid_rect(const int line,
                              const int col,
                              const int tot_col) const
  {
    const int w = (rect.w / tot_col) - 10;
    const int h = font_size + text_padding;
    const int x = rect.x + (w * col) + text_padding;
    const int y = rect.y + (h * line) + text_padding;

    const rect_t result = {x, y, w, h};

    return result;
  }
};


struct held_piece_t
{
  point_t offset;
  bool selected = false;
  gui::position_t piece_board_position;
};


struct piece_animation_t
{
  enum state_t
  {
    OFF,
    RUNNING,
    DONE,
  };

  state_t state = OFF;
  int duration_ms = 200;
  uint64_t start_tick;
  gui::position_t piece_from;
  gui::position_t piece_to;
  gui::piece_t piece;

  point_t start_pos;
  point_t end_pos;
  point_t total_distance;
};


class gui_t : public pixello
{
private:
  const rect_t screen;
  const bool is_screen_horizontal;
  fen_panel_conf fen_panel_conf;
  chessboard_conf board_conf;
  control_panel_conf panel_conf;

  std::map<std::string, texture_t> textures;
  std::map<gui::piece_t, texture_t> piece_textures;
  std::map<std::string, sound_t> sound_fx;
  std::map<char, texture_t> files_and_ranks_textures;
  std::map<int, font_t> fonts;
  std::map<std::string, button_t> buttons;

  gui::state_t state;
  held_piece_t held_piece;
  piece_animation_t animation;

  bool enter_key_pressed = false;

public:
  gui_t(const int w, const int h)
      : pixello(w, h, "Chesso", 60),
        screen{0, 0, w, h},
        is_screen_horizontal(w > h),
        fen_panel_conf(w, h),
        board_conf(w, h, fen_panel_conf),
        panel_conf(w, h, fen_panel_conf, board_conf)
  {}

private:  // OVERRIDE
  void on_init(void*) override;
  void on_update(void*) override;

  inline void log(const std::string& msg) override { LOG_E << msg << END_E; }

private:  // EVENTS
  void handle_events();

private:  // UPDATE
  void update_state();

  void update_mouse_in_chessboard();

private:  // DRAW
  void draw_background();

  void draw_chessboard();
  void draw_coordinates();
  void draw_pieces();
  void draw_piece(const gui::piece_t piece, const gui::position_t& pos);

  void draw_panel();
  void draw_fen_panel();

private:  // UTILS
  rect_t get_square_rect(const int x, const int y);
  point_t piece_pos_to_matrix_pos(const gui::position_t& pos);
  point_t piece_pos_to_screen_pixel_pos(const gui::position_t& pos);
  gui::position_t screen_pos_to_position(const point_t& pos);
  void start_animation(const gui::position_t& from, const gui::position_t& to);
  point_t get_next_animation_pos();
};
