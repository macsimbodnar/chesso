#pragma once
#include <map>
#include <pixello.hpp>
#include "board.hpp"
#include "log.hpp"


static constexpr uint32_t SCREEN_W = 810;
static constexpr uint32_t SCREEN_H = 510;
static constexpr rect_t BOARD_RECT = {20, 10, 480, 480};
static constexpr rect_t RIGHT_PANEL_RECT = {510, 10, 290, 480};
static constexpr int32_t SQUARE_SIZE = 60;

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
  texture_t background;
  std::map<char, texture_t> piece_textures;
  std::map<char, sound_t> sound_fx;
  board _board;
  piece_holding_t mouse_holding;
  selected_square_t selected_square;
  bool flipped_board = false;
  std::map<char, texture_t> files_and_ranks_textures;

public:
  gui()
      : pixello(SCREEN_W,
                SCREEN_H,
                "Chesso",
                60,
                "assets/font/PressStart2P.ttf",
                8)
  {}

private:
  void draw_board();
  void draw_coordinates();

  void on_init(void*) override;
  void on_update(void*) override;

  inline void log(const std::string& msg) override { LOG_E << msg << END_E; }
};
