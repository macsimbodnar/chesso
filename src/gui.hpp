#pragma once
#include <cassert>
#include <cmath>
#include <map>
#include <pixello.hpp>
#include "gui_data_struct.hpp"
#include "log.hpp"
#include "utils.hpp"


struct chessboard_conf
{
  constexpr static float paddings_percentage = 0.05f;
  int padding;
  rect_t rect;
  int square_size;
  pixel_t black_color = {109, 64, 24, 255};
  pixel_t white_color = {232, 235, 239, 255};
  bool flipped = false;

  chessboard_conf(const int screen_w, const int screen_h)
  {
    assert(screen_w != 0);
    assert(screen_h != 0);

    const int full_size = (screen_w > screen_h) ? screen_h : screen_w;
    padding = std::round(static_cast<float>(full_size) * paddings_percentage);
    rect = {padding, padding, full_size - (padding * 2),
            full_size - (padding * 2)};
    square_size = rect.w / 8;
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

  control_panel_conf(const int screen_w,
                     const int screen_h,
                     const chessboard_conf& board_conf)
  {
    if (screen_w > screen_h) {
      rect.x = board_conf.rect.x + board_conf.rect.w + board_conf.padding;
      rect.w = screen_w - rect.x;

      rect.y = 0;
      rect.h = screen_h;
    } else {
      rect.x = 0;
      rect.w = screen_w;

      rect.y = board_conf.rect.y + board_conf.rect.h + board_conf.padding;
      rect.h = screen_h - rect.h;
    }
  }

  inline point_t get_text_pos(const int n) const
  {
    point_t res;

    res.x = rect.x + text_padding;
    res.y = rect.y + (n * (font_size + text_padding)) + text_padding;

    return res;
  }
};


class gui_t : public pixello
{
private:
  const rect_t screen;
  const bool is_screen_horizontal;
  const chessboard_conf board_conf;
  const control_panel_conf panel_conf;

  std::map<std::string, texture_t> textures;
  std::map<gui::piece_t, texture_t> piece_textures;
  std::map<std::string, sound_t> sound_fx;
  std::map<char, texture_t> files_and_ranks_textures;
  std::map<int, font_t> fonts;

  gui::state_t state;

public:
  gui_t(const int w, const int h)
      : pixello(w, h, "Chesso", 60),
        screen{0, 0, w, h},
        is_screen_horizontal(w > h),
        board_conf(w, h),
        panel_conf(w, h, board_conf)
  {}

private:
  void on_init(void*) override;
  void on_update(void*) override;

  inline void log(const std::string& msg) override { LOG_E << msg << END_E; }

private:
  void handle_events();

private:
  void draw_background();

  void draw_chessboard();
  void draw_coordinates();
  void draw_pieces();
  void draw_piece(const gui::piece_t piece, const gui::position_t& pos);

  void draw_panel();

private:
  rect_t get_square_rect(const int x, const int y);
  point_t piece_pos_to_screen_pos(const gui::position_t& pos);
};
