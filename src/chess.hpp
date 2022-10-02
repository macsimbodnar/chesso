#pragma once
#include <array>
#include <cassert>
#include <pixello.hpp>
#include <string>
#include <vector>
#include "log.hpp"
#include "move_generator.hpp"
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

class chess_t
{
private:
  board_state_t board_state;
  int _halfmove_clock = 0;
  int _full_move = 0;

  void cleanup();

public:
  chess_t();

  void load(const std::string& FEN);


  inline void set(const position_t& position, const piece_t piece)
  {
    const uint8_t index = to_index(position);
    board_state.board[index] = piece;
  }

  inline uint8_t check_en_passant(const uint8_t from_index,
                                  const uint8_t to_index,
                                  const piece_t piece) const
  {
    uint8_t en_passant = INVALID_BOARD_POS;

    const color_t color = get_color(piece);
    const piece_t lp = board_state.board[to_index - 0x01];
    const piece_t rp = board_state.board[to_index + 0x01];


    switch (color) {
      case color_t::WHITE:

        if (((to_index - from_index) == 0x20) &&
            (lp == piece_t::B_PAWN || rp == piece_t::B_PAWN)) {
          en_passant = from_index + 0x10;
        }
        break;

      case color_t::BLACK:
        if (((from_index - to_index) == 0x20) &&
            (lp == piece_t::W_PAWN || rp == piece_t::W_PAWN)) {
          en_passant = from_index - 0x10;
        }
        break;

      default:
        assert(false);
        break;
    }


    return en_passant;
  }

  inline void move(const position_t& from, const position_t& to)
  {
    print_move(from, to);

    assert(from.rank != to.rank || from.file != to.file);

    const uint8_t from_index = to_index(from);
    const uint8_t dest_index = to_index(to);

    assert(board_state.board[from_index] != piece_t::INVALID);
    assert(board_state.board[from_index] != piece_t::EMPTY);


    const piece_t piece = board_state.board[from_index];
    uint8_t en_passant = INVALID_BOARD_POS;

    switch (piece) {
      case piece_t::W_PAWN:
      case piece_t::B_PAWN:
        en_passant = check_en_passant(from_index, dest_index, piece);
        break;

      case piece_t::B_KING:
        // Remove the castling right
        board_state.available_castling &= ~(BQ | BK);
        break;

      case piece_t::W_KING:
        // Remove the castling right
        board_state.available_castling &= ~(WQ | WK);
        break;

      case piece_t::B_ROOK:
        if (from_index == 0x70) {
          // Remove the castling right
          board_state.available_castling &= ~BQ;
        }

        if (from_index == 0x77) {
          // Remove the castling right
          board_state.available_castling &= ~BK;
        }
        break;

      case piece_t::W_ROOK:
        if (from_index == 0x00) {
          // Remove the castling right
          board_state.available_castling &= ~WQ;
        }

        if (from_index == 0x07) {
          // Remove the castling right
          board_state.available_castling &= ~WK;
        }
        break;

      case piece_t::B_KNIGHT:
      case piece_t::W_KNIGHT:
      case piece_t::B_BISHOP:
      case piece_t::W_BISHOP:
      case piece_t::B_QUEEN:
      case piece_t::W_QUEEN:
        break;

      default:
        assert(false);
        break;
    }

    board_state.en_passant_target_square = en_passant;

    // Unset the current piece
    board_state.board[dest_index] = board_state.board[from_index];
    board_state.board[from_index] = piece_t::EMPTY;

    // Change the color
    board_state.active_color = !board_state.active_color;
    // LOG_I << board_state.active_color << END_I;
  }


  inline std::vector<position_t> get_valid_moves(const position_t& pos) const
  {
    std::vector<position_t> result;

    const uint8_t index = to_index(pos);

    // Skip if empty
    if (board_state.board[index] == piece_t::EMPTY) { return result; }

    /**************************************************************************
     *       TMP CODE THAT EXTRACT FROM ALL MOVES THE ONE THAT WE WANT
     **************************************************************************/
    std::vector<uint8_t> moves;
    board_state_t tmp_state = board_state;

    // Get first all white moves
    tmp_state.active_color = color_t::WHITE;
    const std::vector<move_t> all_w_moves = generate_legal_moves(tmp_state);

    // Get then all black moves
    tmp_state.active_color = color_t::BLACK;
    const std::vector<move_t> all_b_moves = generate_legal_moves(tmp_state);

    for (const auto& I : all_w_moves) {
      if (I.from == index) { moves.push_back(I.to); }
    }
    for (const auto& I : all_b_moves) {
      if (I.from == index) { moves.push_back(I.to); }
    }

    // moves = generate_pseudo_legal_moves(board_state, index);
    /**************************************************************************/

    LOG_I << "****************************************************" << END_I;
    LOG_I << std::hex << static_cast<int>(index) << std::dec << " -> ";
    for (const auto& I : moves) {
      LOG_I << std::hex << static_cast<int>(I) << std::dec << " ";
    }
    LOG_I << END_I;

    result.reserve(moves.size());

    for (const auto I : moves) {
      result.push_back(to_position(I));
    }

    return result;
  }


  inline std::vector<position_t> get_attacks()
  {
    const color_t c = !board_state.active_color;
    const auto attacks = generate_attack_vector(board_state, c);

    std::vector<position_t> result;
    result.reserve(attacks.size());

    for (const auto I : attacks) {
      result.push_back(to_position(I));
    }

    return result;
  }


  inline std::vector<piece_info_t> pieces() const
  {
    std::vector<piece_info_t> res;
    res.reserve(32);

    for (uint8_t i = 0; i < BOARD_ARRAY_SIZE; ++i) {
      piece_t p = board_state.board[i];

      if (p != piece_t::INVALID && p != piece_t::EMPTY) {
        piece_info_t info;
        info.piece = p;
        info.position = to_position(i);
        info.index = i;

        res.push_back(std::move(info));
      }
    }

    return res;
  }


  inline piece_info_t get_piece_info(const position_t& position) const
  {
    const uint8_t index = to_index(position);

    piece_info_t info;
    info.piece = board_state.board[index];
    info.position = to_position(index);
    info.index = index;

    assert(info.piece != piece_t::INVALID);

    return info;
  }


  inline color_t active_color() const { return board_state.active_color; }

  inline uint8_t available_castling() const
  {
    return board_state.available_castling;
  }
  inline std::string en_passant_target_square() const
  {
    return index_to_algebraic(board_state.en_passant_target_square);
  }
  inline int halfmove_clock() const { return _halfmove_clock; }
  inline int full_move() const { return _full_move; }
};
