#pragma once
#include <exceptions.hpp>

static constexpr uint8_t WQ = 0b0000001;
static constexpr uint8_t WK = 0b0000010;
static constexpr uint8_t BQ = 0b0000100;
static constexpr uint8_t BK = 0b0001000;


struct piece_t
{
  char piece = 0;
  uint32_t x = 0;
  uint32_t y = 0;
  piece_t(char c, uint32_t Y, uint32_t X) : piece(c), y(Y), x(X) {}
};


struct piece_holding_t
{
  int32_t offset_x = 0;
  int32_t offset_y = 0;
  piece_t* selected = nullptr;
};


struct game_t
{
  enum player_t
  {
    BLACK,
    WHITE
  };

  piece_t* board[8][8] = {0};
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


class input_exception : public pixello_exception
{
public:
  input_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};