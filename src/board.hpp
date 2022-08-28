#pragma once
#include <array>
#include <string>

// clang-format off
/**
 * 
 * Mailbox 0x88
 * 
 * 128 byte array
 ************************************************************************************
 *     A    B    C    D    E    F    G    H
 * 8 | 70 | 71 | 72 | 73 | 74 | 75 | 76 | 77 | 78 | 79 | 7A | 7B | 7C | 7D | 7E | 7F
 * 7 | 60 | 61 | 62 | 63 | 64 | 65 | 66 | 67 | 68 | 69 | 6A | 6B | 6C | 6D | 6E | 6F
 * 6 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 5A | 5B | 5C | 5D | 5E | 5F
 * 5 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 | 4A | 4B | 4C | 4D | 4E | 4F
 * 4 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 3A | 3B | 3C | 3D | 3E | 3F
 * 3 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 2A | 2B | 2C | 2D | 2E | 2F
 * 2 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B | 1C | 1D | 1E | 1F
 * 1 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D | 0E | 0F
 *     A    B    C    D    E    F    G    H
 ***********************************************************************************/
// clang-format on

static constexpr uint8_t WQ = 0b0000001;
static constexpr uint8_t WK = 0b0000010;
static constexpr uint8_t BQ = 0b0000100;
static constexpr uint8_t BK = 0b0001000;

enum class color_t
{
  BLACK,
  WHITE
};

class board
{
private:
  std::array<char, 128> _board = {0};
  color_t _active_color = color_t::WHITE;
  uint8_t _available_castling = WQ | WK | BQ | BK;
  std::string _en_passant_target_square = "-";
  int _halfmove_clock = 0;
  int _full_move = 0;

public:
  void load(const std::string& FEN);
};