#pragma once
#include <map>
#include <pixello.hpp>
#include "board.hpp"


struct piece_holding_t
{
  int32_t offset_x = 0;
  int32_t offset_y = 0;
  std::shared_ptr<piece> selected;
};


struct selected_square_t
{
  bool selected = false;
  int32_t x = 0;
  int32_t y = 0;
  rect_t rect;
};

class gui : public pixello
{
private:
  static constexpr rect_t board_rect = {10, 10, 480, 480};
  static constexpr rect_t right_panel_rect = {500, 10, 290, 480};
  texture_t background;
  std::map<char, texture_t> piece_textures;
  std::map<char, sound_t> sound_fx;
  int32_t square_size = 60;
  board _board;
  piece_holding_t mouse_holding;
  selected_square_t selected_square;

public:
  gui() : pixello(800, 500, "Chesso", 60, "assets/font/PressStart2P.ttf", 8) {}

private:
  void draw_board();

  void on_init(void*) override;
  void on_update(void*) override;
};
