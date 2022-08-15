#pragma once
#include <exceptions.hpp>
#include <string>

constexpr uint8_t WQ = 0b0000001;
constexpr uint8_t WK = 0b0000010;
constexpr uint8_t BQ = 0b0000100;
constexpr uint8_t BK = 0b0001000;

struct game_t
{
  enum player_t
  {
    BLACK,
    WHITE
  };

  char board[8][8] = {0};
  bool flipped = true;
  int active_color = WHITE;
  uint8_t available_castling = WQ | WK | BQ | BK;
  std::string en_passant_target_square = "-";
  int halfmove_clock = 0;
  int full_move = 0;
};


class FAN_exception : public pixello_exception
{
public:
  FAN_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};
