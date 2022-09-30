#pragma once
#include <map>
#include <pixello.hpp>
#include "chess.hpp"
#include "log.hpp"
#include "utils.hpp"

static constexpr uint32_t SCREEN_W = 810;
static constexpr uint32_t SCREEN_H = 510;
static constexpr rect_t BOARD_RECT = {20, 10, 480, 480};
static constexpr rect_t RIGHT_PANEL_RECT = {510, 10, 290, 480};
static constexpr int32_t SQUARE_SIZE = 60;

struct piece_holding_t
{
  int32_t offset_x = 0;
  int32_t offset_y = 0;
  bool selected = false;
  piece_info_t info;
};


struct selected_square_t
{
  bool selected = false;
  int32_t x = 0;
  int32_t y = 0;
  rect_t rect;
};


class gui_t : public pixello
{
private:
  texture_t background;
  std::map<piece_t, texture_t> piece_textures;
  std::map<char, sound_t> sound_fx;
  chess_t chess;
  piece_holding_t mouse_holding;
  selected_square_t selected_square;
  std::vector<position_t> suggested_positions;
  bool flipped_board = false;
  std::map<char, texture_t> files_and_ranks_textures;
  std::vector<button_t> buttons;

public:
  gui_t()
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
  void update();
  void draw();

  void on_init(void*) override;
  void on_update(void*) override;

  inline void log(const std::string& msg) override { LOG_E << msg << END_E; }

  inline position_t coordinates_to_postion(const uint8_t x, const uint8_t y)
  {
    position_t result;

    result.file = flipped_board ? 7 - x : x;
    result.rank = flipped_board ? y : 7 - y;

    return result;
  }

  inline position_t coordinates_to_postion(const coordinates_t& c)
  {
    return coordinates_to_postion(c.x, c.y);
  }


  inline coordinates_t position_to_coordinates(const uint8_t file,
                                               const uint8_t rank)
  {
    coordinates_t result;
    result.x = flipped_board ? 7 - file : file;
    result.y = flipped_board ? rank : 7 - rank;

    return result;
  }

  inline coordinates_t position_to_coordinates(const position_t& p)
  {
    return position_to_coordinates(p.file, p.rank);
  }
};
