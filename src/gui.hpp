#pragma once
#include <map>
#include <pixello.hpp>
#include "data_structs.hpp"
#include "utils.hpp"


class gui : public pixello
{
private:
  static constexpr rect_t board_rect = {10, 10, 480, 480};
  texture_t background;
  std::map<char, texture_t> piece_textures;
  int32_t square_size = 60;
  game_t game;
  piece_holding_t mouse_holding;

public:
  gui() : pixello(800, 500, "Chesso", 60, "assets/font/PressStart2P.ttf", 8) {}

private:
  void move_piece(const uint32_t X, const uint32_t Y, piece_t* piece);
  void draw_board();

  void on_init(void*) override;
  void on_update(void*) override;
};