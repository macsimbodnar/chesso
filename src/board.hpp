#pragma once
#include <array>
#include <cassert>
#include <memory>
#include <pixello.hpp>
#include <string>
#include <vector>
#include "log.hpp"
#include "piece.hpp"
#include "utils.hpp"

// clang-format off
/**
 * 
 * Mailbox 0x88
 * 
 * 128 byte array
 * Files A - H        X
 * Ranks 1 - 8        7 - Y
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


static constexpr char FEN_INIT_POS[] =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static constexpr size_t BOARD_ARRAY_SIZE = 128;
static constexpr uint8_t WQ = 0b0000001;
static constexpr uint8_t WK = 0b0000010;
static constexpr uint8_t BQ = 0b0000100;
static constexpr uint8_t BK = 0b0001000;


enum class color_t
{
  BLACK,
  WHITE
};


class board_t
{
private:
  std::array<std::shared_ptr<piece_t>, BOARD_ARRAY_SIZE> _board = {0};
  color_t _active_color = color_t::WHITE;
  uint8_t _available_castling = WQ | WK | BQ | BK;
  std::string _en_passant_target_square = "-";
  int _halfmove_clock = 0;
  int _full_move = 0;

  void cleanup();


  inline uint8_t to_index(const uint8_t file, const uint8_t rank)
  {
    assert(file >= 0 && file < 8);
    assert(rank >= 0 && rank < 8);

    const uint8_t index = (rank << 4) + file;
    assert(index < BOARD_ARRAY_SIZE);

    return index;
  }


  inline position_t to_position(const uint8_t index)
  {
    position_t result;
    result.file = index & 7;
    result.rank = index >> 4;

    return result;
  }


public:
  board_t();

  void load(const std::string& FEN);

  inline void set(const uint8_t file, const uint8_t rank, const char p)
  {
    const uint8_t index = to_index(file, rank);
    _board[index] = std::make_shared<piece_t>(file, rank, p);
  }

  inline void move(const uint8_t from_file,
                   const uint8_t from_rank,
                   const uint8_t to_file,
                   const uint8_t to_rank)
  {
    print_move(from_file, from_rank, to_file, to_rank);

    assert(from_rank != to_rank || from_file != to_file);

    const uint8_t from_index = to_index(from_file, from_rank);
    const uint8_t dest_index = to_index(to_file, to_rank);

    assert(_board[from_index]);

    // Unset the current piece
    _board[dest_index] = _board[from_index];
    _board[from_index] = nullptr;
    _board[dest_index].get()->set_index(dest_index);
    _board[dest_index].get()->set_file(to_file);
    _board[dest_index].get()->set_rank(to_rank);
  }


  inline std::vector<position_t> get_valid_moves(const uint8_t file,
                                                 const uint8_t rank)
  {
    std::vector<position_t> result;

    const uint8_t index = to_index(file, rank);

    if (_board[index]) {
      const auto all_moves = _board[index]->get_all_moves();

      result.reserve(all_moves.size());
      for (const auto I : all_moves) {
        result.push_back(to_position(I));
      }
    }

    return result;
  }


  inline std::vector<std::shared_ptr<piece_t>> pieces() const
  {
    std::vector<std::shared_ptr<piece_t>> res;
    res.reserve(32);

    for (const auto& I : _board) {
      if (I) { res.push_back(I); }
    }

    return res;
  }


  inline std::shared_ptr<piece_t> get_piece(const uint8_t file,
                                          const uint8_t rank)
  {
    const uint8_t index = to_index(file, rank);
    return _board[index];
  }


  inline color_t active_color() const { return _active_color; }

  inline uint8_t available_castling() const { return _available_castling; }
  inline std::string en_passant_target_square() const
  {
    return _en_passant_target_square;
  }
  inline int halfmove_clock() const { return _halfmove_clock; }
  inline int full_move() const { return _full_move; }
};
